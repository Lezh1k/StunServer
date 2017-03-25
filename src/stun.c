#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "stun.h"

/*
 Given a 16-bit STUN message type value in host byte order in msg_type
 parameter:
*/
#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

#define STUN_MAGIC_COOKIE 0x2112A442

typedef struct stun_hdr {
  uint16_t msg_type;
  uint16_t msg_len;
  uint32_t magic_cookie;
  uint8_t  tx_id[12];
} stun_hdr_t;
//////////////////////////////////////////////////////////////

int32_t
stun_handle(int sock, int n, char *msg) {
  printf("stun handle : %.*s\n", n, msg);
  send(sock, msg, n, 0); //don't know why 0
  return 0;
}
//////////////////////////////////////////////////////////////
