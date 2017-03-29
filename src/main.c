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

#include "commons.h"
#include "stun.h"

#define INVALID_SOCKET  (-1)
#define STUN_PORT       3478
#define STUN_MAXMSG     1024

static volatile st_bool_t is_running = st_true;

st_bool_t register_ctrl_c_handler();
void ctrl_c_handler(int s);
void *tcp_listen(void *unused);
void *udp_listen(void *unused);

int
main(int argc, char *argv[]) {
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);
  pthread_t pt_tcp, pt_udp;

  if (!register_ctrl_c_handler()) {
    printf("Couldn't register ctrl_c handler\n");
    return -1;
  }

  if (!pthread_create(&pt_tcp, NULL, tcp_listen, NULL)) {
  }

  if (!pthread_create(&pt_udp, NULL, udp_listen, NULL)) {
  }

  while (is_running) {
    usleep(1000);
  }

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
tcp_listen(void* unused) {
  UNUSED_ARG(unused);
  fd_set fds_master, fds_ready_to_read;
  struct sockaddr_in service;
  int32_t h_serv, res, new_client;
  int32_t yes, fd_max, recv_n;
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
    printf("Can't allocate list of clients : %d\n", errno);
    return NULL;
  }

  FD_ZERO(&fds_master);
  FD_ZERO(&fds_ready_to_read);

  h_serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (h_serv == INVALID_SOCKET) {
    printf("Invalid socket. Error : %d\n", errno);
    perror("socket call failed : ");
    is_running = st_false;
  }

  if(setsockopt(h_serv, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
    printf("setsockopt call error : %d\n", errno);
    perror("setsockopt call failed : ");
    is_running = st_false;
  }

  service.sin_family = AF_INET;
  service.sin_addr.s_addr = INADDR_ANY;
  service.sin_port = htons(STUN_PORT);

  if (bind(h_serv, (struct sockaddr*) &service, sizeof (service)) == -1) {
    printf("Bind function failed with error %d\n", errno);
    perror("Bind function failed : ");
    close(h_serv);
    is_running = st_false;
  }

  if (listen(h_serv, SOMAXCONN) == -1) {
    printf("Listen function failed with error %d\n", errno);
    perror("Listen call failed : ");
    close(h_serv);
    is_running = st_false;
  }

  FD_SET(h_serv, &fds_master);
  lst_clients[lst_clients_count].fd = h_serv;
  lst_clients[lst_clients_count].addr = service;
  ++lst_clients_count;
  fd_max = h_serv;
  printf("Start listen tcp socket\n");

  while (is_running) {
    uint32_t i, j;
    fds_ready_to_read = fds_master;
    res = select(fd_max+1, &fds_ready_to_read, NULL, NULL, NULL);

    if (res == -1) {
      printf("Select error : %d\n", errno);
      perror("Select failed : ");
      is_running = st_false;
      continue;
    } else if (res == 0) {
      printf("Select socket timeout\n");
      continue;
    }

    for (i = 0; i < lst_clients_count && res > 0; ++i) {
      if (!FD_ISSET(lst_clients[i].fd, &fds_ready_to_read)) continue;
      --res;

      if (lst_clients[i].fd == h_serv) { //we have new connection here

        new_client = accept(h_serv, (struct sockaddr*) &lst_clients[i].addr, &client_addr_len);
        if (new_client == INVALID_SOCKET) {
          printf("New connection error %d\n", errno);
          perror("Accept failed : ");
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
        printf(("New connection from %s\n"), inet_ntoa(lst_clients[i].addr.sin_addr));
      } else { //we have received something
        recv_n = recv(lst_clients[i].fd, buff, sizeof(buff), 0);
        if (recv_n == 0) { //connection closed
          FD_CLR(lst_clients[i].fd, &fds_master);
          close(lst_clients[i].fd);
          printf(("Connection N:%d sock:%d closed\n"), i, lst_clients[i].fd);

          for (j = i; j < lst_clients_count-1; ++j) {
            lst_clients[j] = lst_clients[j+1];
          }
          --lst_clients_count;

          continue;
        }
        else if (recv_n < 0) { //recv error
          printf("Recv error : %d\n", errno);
          perror("recv call failed : ");
          continue;
        }

        printf("received:\n");
        //we here because we can read some bytes.        
        if ((res = stun_handle(recv_n, buff, (struct sockaddr*) &lst_clients[i].addr)) < 0) { //some error here
          FD_CLR(lst_clients[i].fd, &fds_master);
          close(lst_clients[i].fd);
          printf("Connection closed because received not stun message. Conn : %d, sock : %d\n", i, lst_clients[i].fd);
          for (j = i; j < lst_clients_count-1; ++j) {
            lst_clients[j] = lst_clients[j+1];
          }
          --lst_clients_count;
        } else {
          write(lst_clients[i].fd, buff, res);
          printf("megatest:\n");
          stun_handle(res, buff, (struct sockaddr*) &lst_clients[i].addr);
        }
      } //else
    } //for i = h_serv
  } // while (is_running)
  free(lst_clients);
  return NULL;
} //tcp_listen()
//////////////////////////////////////////////////////////////

void*
udp_listen(void* unused) {
  UNUSED_ARG(unused);
  struct sockaddr_in service;
  struct sockaddr_in sender_addr;
  socklen_t sender_addr_size = sizeof(sender_addr);
  int32_t h_serv, recv_n;
  char buff[STUN_MAXMSG] = {0};

  service.sin_family = AF_INET;
  service.sin_addr.s_addr = INADDR_ANY;
  service.sin_port = htons(STUN_PORT);

  h_serv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (h_serv == INVALID_SOCKET) {
    printf("Invalid socket. Error : %d\n", errno);
    perror("socket call failed : ");
    is_running = st_false;
  }

  if (bind(h_serv, (struct sockaddr*) &service, sizeof (service)) == -1) {
    printf("Bind function failed with error %d\n", errno);
    perror("Bind function failed : ");
    close(h_serv);
    is_running = st_false;
  }

  printf("Udp listen started\n");
  while (is_running) {
    recv_n = recvfrom(h_serv, buff, STUN_MAXMSG, 0,
                      (struct sockaddr*) &sender_addr, &sender_addr_size);

    if (recv_n == -1) {
      printf("recvfrom err : %d\n", errno);
      perror("recvfrom failed : ");
      continue;
    }
    if (recv_n == 0) continue; //Hope we don't need to sleep here :)
    printf("udp received:\n");
    recv_n = stun_handle(recv_n, buff, (struct sockaddr*) &sender_addr);
    if (recv_n > 0) {
      printf("udp planning to send : %d\n", recv_n);
      recv_n = sendto(h_serv, buff, recv_n, 0, (struct sockaddr*) &sender_addr, sizeof(sender_addr));
      printf("udp sent : %d\n", recv_n);
    }
  } //while (is_running)

  return NULL;
}
//////////////////////////////////////////////////////////////
