#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <netdb.h>
#include <stdbool.h>

#include "commons.h"
#include "stun.h"
#include "logger.h"

#define INVALID_SOCKET  (-1)
#define STUN_PORT       3478
#define STUN_PORT2      5349
#define STUN_MAXMSG     1024

static volatile bool is_running = true;
static void *udp_listen(void *arg);

static inline int inetAddrValid(const char *str) {
  static struct in_addr inaddr;
  return inet_aton(str, &inaddr);
}

int main(int argc, char *argv[]) {
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);
  pthread_t pt_udp;
  settings_t settings;
  sigset_t sigset;  
  int itmp, sig;

  const struct option long_options[] = {
    {"port0", required_argument,  NULL, 0},
    {"port1", required_argument,  NULL, 0},
    {"addr0", required_argument,  NULL, 0},
    {"addr1", required_argument,  NULL, 0},
    {NULL,    0,                  NULL, 0}
  };

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGQUIT);
  sigaddset(&sigset, SIGINT);

  memset(&settings, 0, sizeof(settings_t));
  settings.port0 = STUN_PORT;
  settings.port1 = STUN_PORT2;

#define DEF_ADDR0 "127.0.0.1"
#define DEF_ADDR1 ""
  memcpy(settings.addr0, DEF_ADDR0, strlen(DEF_ADDR0));
  memcpy(settings.addr1, DEF_ADDR1, strlen(DEF_ADDR1));

  while (getopt_long(argc, argv, "1:2:3:4:", long_options, &itmp) != -1) {
    switch (itmp) {
      case 0:
        settings.port0 = atoi(optarg);
        break;
      case 1:
        settings.port1 = atoi(optarg);
        break;
      case 2:
        if (!inetAddrValid(optarg)) {
          logger_log(LL_ERR, "addr0 argument is wrong : %s", optarg);
          return -2;
        }
        memcpy(settings.addr0, optarg, strlen(optarg));
        break;
      case 3:
        if (!inetAddrValid(optarg)) {
          logger_log(LL_ERR, "addr1 argument is wrong : %s", optarg);
          return -3;
        }
        memcpy(settings.addr1, optarg, strlen(optarg));
        break;
      default:
        logger_log(LL_ERR, "Something bad is happened. Option index = %d", itmp);
        return -1;
    }
  };

  logger_init();
  itmp = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
  if (itmp) {
    logger_log(LL_ERR, "%s:%d", "pthread_sigmask:", itmp);
    return -1;
  }

  if ((itmp = pthread_create(&pt_udp, NULL, udp_listen, (void*)&settings))) {
    logger_log(LL_ERR, "%s:%d", "Couldn't start udp listen thread", itmp);
    return -1;
  }

  itmp = sigwait(&sigset, &sig);
  if (itmp) {
    logger_log(LL_ERR, "%s:%d", "sigwait:", itmp);
    return -1;
  }
  is_running = false;
  pthread_join(pt_udp, NULL);
  logger_shutdown();
  return 0;
}
//////////////////////////////////////////////////////////////

typedef struct client {
  int32_t fd;
  struct sockaddr_in addr;
} client_t;

/*
//addr0:port0
//addr0:port1
//addr1:port0
//addr1:port1

switch (change_request & 0x06) {
  case 0x00: //change neither
    ch_i = sn;
    break;
  case 0x02: //change port
    ch_i = sn&0x01 ? sn-1 : sn+1; //sn%2 ? ...
    break;
  case 0x04: //change addr
    ch_i = (sn+2)%4;
    break;
  case 0x06: //change both
    ch_i = ((sn+2) % 4) - (sn&0x01 ? 1 : -1); //sn%2 ? 1 : -1
    break;
}
*/

static int32_t change_req_sm[] = {
  0, 1, 2, 3,
  1, 0, 3, 2,
  2, 3, 0, 1,
  3, 2, 1, 0
};

void *udp_listen(void *arg) {
  UNUSED_ARG(arg);
  static char buff[STUN_MAXMSG] = {0};

  fd_set fds_master, fds_ready_to_read;
  int32_t yes, fd_max, change_request;
  register int32_t recv_n, res, sn, ch_i;

  struct sockaddr_in sender_addr;
  socklen_t sender_addr_size = sizeof(sender_addr);
  settings_t *settings = (settings_t*)arg;

  uint16_t ports[2] = {settings->port0, settings->port1};
  const char *server_names[2] = {settings->addr0, settings->addr1};

  struct sockaddr_in *services;
  int32_t *h_serv;
  struct addrinfo *pHost, hints;

  int32_t sc = 0;
#define ST_SEC 2

  struct timeval select_timeout;
  if (strlen(settings->addr0)) ++sc;
  if (strlen(settings->addr1)) ++sc;
  sc *= 2; //because 2 ports

  services = (struct sockaddr_in*) malloc(sc*sizeof(struct sockaddr_in));
  if (services == NULL) {
    logger_log(LL_ERR, "%s:%d", "malloc services failed", errno);
    return NULL;
  }
  h_serv = (int32_t*) malloc(sc*sizeof(int32_t));
  if (h_serv == NULL) {
    logger_log(LL_ERR, "%s:%d", "malloc h_serv failed", errno, sc*sizeof(int32_t));
    return NULL;
  }

  memset(services, 0, sizeof(struct sockaddr_in)*sc);
  memset(h_serv, 0, sizeof(int32_t)*sc);
  memset(&hints, 0, sizeof(struct addrinfo));

  FD_ZERO(&fds_master);
  FD_ZERO(&fds_ready_to_read);
  fd_max = 0;

  for (sn = 0; sn < sc; ++sn) {
    h_serv[sn] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (h_serv[sn] == INVALID_SOCKET) {
      logger_log(LL_ERR, "Invalid socket. Error : %d", errno);
      is_running = false;
      break;
    }

    if(setsockopt(h_serv[sn], SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
      logger_log(LL_ERR, "setsockopt call error : %d", errno);
      is_running = false;
      break;
    }

    if (getaddrinfo(server_names[sn/2], NULL, &hints, &pHost) != 0) {
      logger_log(LL_ERR, "getaddr info failed : %d", errno);
      is_running = false;
      break;
    }

    services[sn].sin_family = AF_INET;
    services[sn].sin_addr.s_addr = ((struct sockaddr_in*)pHost->ai_addr)->sin_addr.s_addr;
    services[sn].sin_port = htons(ports[sn%2]);

    if (bind(h_serv[sn], (struct sockaddr*) &services[sn], sizeof (services[sn])) == -1) {
      logger_log(LL_ERR, "Bind function failed with error %d", errno);
      close(h_serv[sn]);
      is_running = false;
      break;
    }

    FD_SET(h_serv[sn], &fds_master);
    if (fd_max < h_serv[sn])
      fd_max = h_serv[sn];

    logger_log(LL_INFO, "listening to : %s:%d", inet_ntoa(services[sn].sin_addr), ports[sn%2]);
  } //for sn < SERVICE_COUNT

  logger_log(LL_INFO, "%s", "Start listen udp socket");

  while (is_running) {
    fds_ready_to_read = fds_master;
    select_timeout.tv_sec = ST_SEC;
    select_timeout.tv_usec = 0;

    res = select(fd_max+1, &fds_ready_to_read, NULL, NULL, &select_timeout);

    if (res == -1) {
      logger_log(LL_ERR, "Select error : %d", errno);
      is_running = false;
      continue;
    } else if (res == 0) {
      logger_log(LL_DEBUG, "%s", "Select socket timeout");
      continue;
    }

    for (sn = 0; sn < sc && res > 0; ++sn) {

      if (!FD_ISSET(h_serv[sn], &fds_ready_to_read)) continue;
      --res;

      recv_n = recvfrom(h_serv[sn], buff, STUN_MAXMSG, 0,
                        (struct sockaddr*) &sender_addr, &sender_addr_size);
      if (recv_n < 0) { //recv error
        logger_log(LL_WARNING, "Recv error : %d", errno);
        continue;
      }

      logger_log(LL_DEBUG, "received something from : %s", inet_ntoa(sender_addr.sin_addr));
      recv_n = stun_prepare_message(recv_n, buff, (struct sockaddr*) &sender_addr, &change_request);
      if (recv_n < 0) continue;

      ch_i = sn;
      logger_log(LL_DEBUG, "change_request : %x", change_request);

      change_request &= 0x06;
      change_request /= 2;
      ch_i = change_req_sm[sn*4 + change_request];

      recv_n = stun_handle_change_addr(
            (struct sockaddr*)&services[ch_i], //changed address
            (struct sockaddr*)&services[sn],   //source address
            buff);
      recv_n = sendto(h_serv[ch_i], buff, recv_n, 0,
          (struct sockaddr*) &sender_addr, sizeof(sender_addr));
      logger_log(LL_DEBUG, "sent %d bytes from %s:%d", recv_n,
             inet_ntoa(services[ch_i].sin_addr), ntohs(services[ch_i].sin_port));
    } //for sn < sc
  } //while is running
  return NULL;
}
//////////////////////////////////////////////////////////////
