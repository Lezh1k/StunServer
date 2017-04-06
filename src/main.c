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

#include "commons.h"
#include "stun.h"
#include "logger.h"

#define INVALID_SOCKET  (-1)
#define STUN_PORT       3478
#define STUN_PORT2      5349
#define STUN_MAXMSG     1024

static volatile st_bool_t is_running = st_true;

st_bool_t register_ctrl_c_handler();
void ctrl_c_handler(int s);
void *tcp_listen(void *settings);
void *udp_listen(void *arg);

int
main(int argc, char *argv[]) {
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);
  pthread_t pt_tcp, pt_udp;
  settings_t settings;
  int oi;

  const struct option long_options[] = {
    {"port0", required_argument,  NULL, 0},
    {"port1", required_argument,  NULL, 0},
    {"addr0", required_argument,  NULL, 0},
    {"addr1", required_argument,  NULL, 0},
    {NULL,    0,                  NULL, 0}
  };

  memset(&settings, 0, sizeof(settings_t));
  settings.port0 = STUN_PORT;
  settings.port1 = STUN_PORT2;

  while (getopt_long(argc, argv, "1:2:3:4:", long_options, &oi) != -1) {
    switch (oi) {
      case 0:
        settings.port0 = atoi(optarg);
        break;
      case 1:
        settings.port1 = atoi(optarg);
        break;
      case 2:
        if (strlen(optarg) > 16) {
          logger_log(LL_ERR, "addr0 argument is wrong : %s", optarg);
          return -2;
        }
        memcpy(settings.addr0, optarg, strlen(optarg));
        break;
      case 3:
        if (strlen(optarg) > 16) {
          logger_log(LL_ERR, "addr1 argument is wrong : %s", optarg);
          return -3;
        }
        memcpy(settings.addr1, optarg, strlen(optarg));
        break;
      default:
        logger_log(LL_ERR, "Something bad is happened. Option index = %d", oi);
        return -1;
    }
  };

  logger_init();

  if (!register_ctrl_c_handler()) {
    logger_log(LL_ERR, "%s", "Couldn't register ctrl_c handler");
    return -1;
  }

  if (!pthread_create(&pt_tcp, NULL, tcp_listen, &settings)) {
  }

  if (!pthread_create(&pt_udp, NULL, udp_listen, &settings)) {
  }

  while (is_running) {
    usleep(1000);
  }

  // I don't know why doesn't it work.
  //  pthread_join(pt_tcp, NULL);
  //  pthread_join(pt_udp, NULL);
  logger_shutdown();
  return 0;
}
//////////////////////////////////////////////////////////////

st_bool_t
register_ctrl_c_handler() {
  signal(SIGINT, ctrl_c_handler);
  return st_true;
}
//////////////////////////////////////////////////////////////

void
ctrl_c_handler(int s) {
  UNUSED_ARG(s);
  is_running = st_false;
}
//////////////////////////////////////////////////////////////

typedef struct client {
  int32_t fd;
  struct sockaddr_in addr;
} client_t;

void*
tcp_listen(void* settings) {
  UNUSED_ARG(settings);
  fd_set fds_master, fds_ready_to_read;
  struct sockaddr_in service;
  int32_t h_serv, res, new_client;
  int32_t yes, fd_max, recv_n, change_request;
  socklen_t client_addr_len = sizeof(service);
  uint32_t lst_clients_count = 0;
  uint32_t lst_clients_size = 2;
  client_t* lst_clients = malloc(lst_clients_size*sizeof(client_t));
  client_t* lst_clients_tmp; //for reallocation
  char buff[STUN_MAXMSG] = {0};

  memset(lst_clients, 0, lst_clients_size*sizeof(client_t));

  yes = 1;
  fd_max = recv_n = 0;

  if (lst_clients == NULL) {
    logger_log(LL_ERR, "Can't allocate list of clients : %d", errno);
    return NULL;
  }

  FD_ZERO(&fds_master);
  FD_ZERO(&fds_ready_to_read);

  h_serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (h_serv == INVALID_SOCKET) {
    logger_log(LL_ERR, "Invalid socket. Error : %d", errno);
    is_running = st_false;
    return NULL;
  }

  if(setsockopt(h_serv, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
    logger_log(LL_ERR, "setsockopt call error : %d", errno);
    is_running = st_false;
    return NULL;
  }

  service.sin_family = AF_INET;
  service.sin_addr.s_addr = INADDR_ANY;
  service.sin_port = htons(STUN_PORT);

  if (bind(h_serv, (struct sockaddr*) &service, sizeof (service)) == -1) {
    logger_log(LL_ERR, "Bind function failed with error %d", errno);
    close(h_serv);
    is_running = st_false;
    return NULL;
  }

  if (listen(h_serv, SOMAXCONN) == -1) {
    logger_log(LL_ERR, "Listen function failed with error %d", errno);
    close(h_serv);
    is_running = st_false;
    return NULL;
  }

  FD_SET(h_serv, &fds_master);
  lst_clients[lst_clients_count].fd = h_serv;
  lst_clients[lst_clients_count].addr = service;
  ++lst_clients_count;
  fd_max = h_serv;
  logger_log(LL_INFO, "%s", "Start listen tcp socket");

  while (is_running) {
    uint32_t i, j;
    fds_ready_to_read = fds_master;
    res = select(fd_max+1, &fds_ready_to_read, NULL, NULL, NULL);

    if (res == -1) {
      logger_log(LL_ERR, "Select error : %d", errno);
      is_running = st_false;
      continue;
    } else if (res == 0) {
      logger_log(LL_WARNING, "%s", "Select socket timeout");
      continue;
    }

    for (i = 0; i < lst_clients_count && res > 0; ++i) {
      if (!FD_ISSET(lst_clients[i].fd, &fds_ready_to_read)) continue;
      --res;

      if (lst_clients[i].fd == h_serv) { //we have new connection here

        new_client = accept(h_serv, (struct sockaddr*) &lst_clients[i].addr, &client_addr_len);
        if (new_client == INVALID_SOCKET) {
          logger_log(LL_WARNING, "New connection error %d", errno);
          continue;
        }

        if (new_client > fd_max)
          fd_max = new_client;

        FD_SET(new_client, &fds_master);
        lst_clients[lst_clients_count].fd = new_client;
        ++lst_clients_count;
        if (lst_clients_count >= lst_clients_size) {
          lst_clients_tmp = malloc(lst_clients_size*2*sizeof(client_t));
          memset(lst_clients_tmp, 0, lst_clients_size*2*sizeof(client_t));
          memcpy(lst_clients_tmp, lst_clients, lst_clients_size*sizeof(client_t));
          lst_clients_size *= 2;
          free(lst_clients);
          lst_clients = lst_clients_tmp;
        }
        logger_log(LL_DEBUG, "New connection from %s", inet_ntoa(lst_clients[i].addr.sin_addr));
      } else { //we have received something
        recv_n = recv(lst_clients[i].fd, buff, sizeof(buff), 0);
        if (recv_n == 0) { //connection closed
          FD_CLR(lst_clients[i].fd, &fds_master);
          close(lst_clients[i].fd);
          logger_log(LL_DEBUG, "Connection N:%d sock:%d closed", i, lst_clients[i].fd);

          for (j = i; j < lst_clients_count-1; ++j) {
            lst_clients[j] = lst_clients[j+1];
          }
          --lst_clients_count;

          continue;
        }
        else if (recv_n < 0) { //recv error
          logger_log(LL_WARNING, "Recv error : %d", errno);
          continue;
        }

        //we here because we can read some bytes.
        if ((res = stun_prepare_message(recv_n, buff, (struct sockaddr*) &lst_clients[i].addr, &change_request)) < 0) { //some error here
          FD_CLR(lst_clients[i].fd, &fds_master);
          close(lst_clients[i].fd);
          logger_log(LL_WARNING,
                     "Connection closed because received not stun message. Conn : %d, sock : %d\n", i, lst_clients[i].fd);
          for (j = i; j < lst_clients_count-1; ++j) {
            lst_clients[j] = lst_clients[j+1];
          }
          --lst_clients_count;
        } else {
          write(lst_clients[i].fd, buff, res);
        }
      } //else
    } //for i = h_serv
  } // while (is_running)
  free(lst_clients);
  return NULL;
} //tcp_listen()
//////////////////////////////////////////////////////////////

void*
udp_listen(void* arg) {
#define SERVICE_COUNT 4
  UNUSED_ARG(arg);
  fd_set fds_master, fds_ready_to_read;
  int32_t yes, fd_max, recv_n, res, sn, change_request, ch_i;
  struct sockaddr_in sender_addr;
  socklen_t sender_addr_size = sizeof(sender_addr);
  char buff[STUN_MAXMSG] = {0};
  settings_t* settings = (settings_t*)arg;

  uint16_t ports[2] = {settings->port0, settings->port1};
  const char* server_names[2] = {settings->addr0, settings->addr1};

  struct sockaddr_in services[SERVICE_COUNT];
  struct addrinfo *pHost, hints;
  int32_t h_serv[SERVICE_COUNT];

  memset(&services, 0, sizeof(struct sockaddr_in)*SERVICE_COUNT);
  memset(&h_serv, 0, sizeof(int32_t)*SERVICE_COUNT);
  memset(&hints, 0, sizeof(struct addrinfo));

  FD_ZERO(&fds_master);
  FD_ZERO(&fds_ready_to_read);
  fd_max = 0;

  for (sn = 0; sn < SERVICE_COUNT; ++sn) {
    h_serv[sn] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (h_serv[sn] == INVALID_SOCKET) {
      logger_log(LL_ERR, "Invalid socket. Error : %d", errno);
      is_running = st_false;
      break;
    }

    if(setsockopt(h_serv[sn], SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
      logger_log(LL_ERR, "setsockopt call error : %d", errno);
      is_running = st_false;
      break;
    }

    if (getaddrinfo(server_names[sn%2], NULL, &hints, &pHost) != 0) {
      logger_log(LL_ERR, "getaddr info failed : %d", errno);
      is_running = st_false;
      break;
    }

    services[sn].sin_family = AF_INET;
    services[sn].sin_addr.s_addr = ((struct sockaddr_in*)pHost->ai_addr)->sin_addr.s_addr;
    services[sn].sin_port = htons(ports[sn/2]);

    if (bind(h_serv[sn], (struct sockaddr*) &services[sn], sizeof (services[sn])) == -1) {
      logger_log(LL_ERR, "Bind function failed with error %d", errno);
      close(h_serv[sn]);
      is_running = st_false;
      break;
    }

    FD_SET(h_serv[sn], &fds_master);
    if (fd_max < h_serv[sn])
      fd_max = h_serv[sn];

    logger_log(LL_INFO, "listening to : %s:%d", inet_ntoa(services[sn].sin_addr), ports[sn/2]);
  } //for sn < 2

  logger_log(LL_INFO, "%s", "Start listen udp socket");

  while (is_running) {
    fds_ready_to_read = fds_master;
    res = select(fd_max+1, &fds_ready_to_read, NULL, NULL, NULL);

    if (res == -1) {
      logger_log(LL_ERR, "Select error : %d", errno);
      is_running = st_false;
      continue;
    } else if (res == 0) {
      logger_log(LL_WARNING, "%s", "Select socket timeout");
      continue;
    }

    for (sn = 0; sn < SERVICE_COUNT && res > 0; ++sn) {

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
      switch (change_request & 0x06) {
        case 0x00: //change neither
          ch_i = sn;
          break;
        case 0x02: //change port
          ch_i = sn%2 ? sn-1 : sn+1;
          break;
        case 0x04: //change addr
          ch_i = (sn+2)%4;
          break;
        case 0x06: //change both
          ch_i = ((sn+2) % 4) - (sn%2 ? 1 : -1);
          break;
      }
      recv_n = stun_handle_change_addr(
            (struct sockaddr*)&services[(sn+2)%4 - (sn%2 ? 1 : -1)], //changed address
            (struct sockaddr*)&services[ch_i], //source address
            buff);
      recv_n = sendto(h_serv[ch_i], buff, recv_n, 0,
          (struct sockaddr*) &sender_addr, sizeof(sender_addr));
      logger_log(LL_DEBUG, "sent %d bytes from %s:%d", recv_n,
             inet_ntoa(services[ch_i].sin_addr), ntohs(services[ch_i].sin_port));

    } //for i < 2 do
  } //is running
  return NULL;
}
//////////////////////////////////////////////////////////////
