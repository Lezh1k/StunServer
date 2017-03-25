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

#include "commons.h"
#include "stun.h"

#define INVALID_SOCKET  (-1)
#define STUN_PORT       3478
#define STUN_MAXMSG     1024

static volatile st_bool_t is_running = st_true;

st_bool_t register_ctrl_c_handler();
void ctrl_c_handler(int s);
void tcp_listen();
void udp_listen();

int
main(int argc, char *argv[]) {
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);
  tcp_listen();
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

void
tcp_listen() {
  fd_set fds_master;
  fd_set fds_ready_to_read;
  struct sockaddr_in client_addr;
  struct sockaddr_in service;
  int new_client;
  int h_serv;
  int yes = 1;
  int res;
  int fd_max = 0;
  int recv_n = 0;
  socklen_t client_addr_len;
  char buff[STUN_MAXMSG] = {0};


  FD_ZERO(&fds_master);
  FD_ZERO(&fds_ready_to_read);

  h_serv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (h_serv == INVALID_SOCKET) {
    printf("Invalid socket. Error : %d\n", errno);
    is_running = st_false;
  }

  if(setsockopt(h_serv, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == -1) {
    printf("Server-setsockopt() error : %d\n", errno);
    is_running = st_false;
  }

  service.sin_family = AF_INET;
  service.sin_addr.s_addr = INADDR_ANY;
  service.sin_port = htons(STUN_PORT);

  if (bind(h_serv, (struct sockaddr*) &service, sizeof (service)) == -1) {
    printf("Bind function failed with error %d\n", errno);
    close(h_serv);
    is_running = st_false;
  }

  if (listen(h_serv, SOMAXCONN) == -1) {
    printf("Listen function failed with error %d\n", errno);
    close(h_serv);
    is_running = st_false;
  }

  FD_SET(h_serv, &fds_master);
  fd_max = h_serv;
  printf("Start listen tcp socket\n");

  while (is_running) {
    int i;
    fds_ready_to_read = fds_master;
    res = select(fd_max+1, &fds_ready_to_read, NULL, NULL, NULL);

    if (res == -1) {
      printf("Select error : %d\n", errno);
      is_running = st_false;
      continue;
    } else if (res == 0) {
      printf("Select socket timeout\n");
      continue;
    }

    for (i = h_serv; i <= fd_max && res > 0; ++i) {
      if (!FD_ISSET(i, &fds_ready_to_read)) continue;
      --res;

      if (i == h_serv) { //we have new connection here
        new_client = accept(h_serv, (struct sockaddr*) &client_addr, &client_addr_len);

        if (new_client > fd_max)
          fd_max = new_client;

        if (new_client == INVALID_SOCKET) {
          printf("New connection error %d\n", errno);
          continue;
        }

        FD_SET(new_client, &fds_master);
        printf(("New connection from %s\n"), inet_ntoa(client_addr.sin_addr));
      } else { //we have received something
        recv_n = recv(i, buff, sizeof(buff), 0);
        if (recv_n == 0) { //connection closed
          FD_CLR(i, &fds_master);
          close(i);
          printf(("Connection %d closed\n"), i);
          continue;
        }
        else if (recv_n < 0) { //recv error
          printf("Recv error : %d\n", errno);
          continue;
        }

        //we here because we can read some bytes.
        if (stun_handle(i, recv_n, buff)) { //some error here
          FD_CLR(i, &fds_master);
          close(i);
        }
        //todo handle this :)
      } //else
    } //for i = h_serv
  } // while (is_running)
} //tcp_listen()
//////////////////////////////////////////////////////////////

void
udp_listen() {
}
//////////////////////////////////////////////////////////////
