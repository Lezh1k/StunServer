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
  uint16_t type;
  uint16_t len;
  uint32_t magic_cookie;
  uint8_t  tx_id[12];
} stun_hdr_t;

typedef struct stun_attr {
  uint16_t type;
  uint16_t len;
  uint8_t* value;
} stun_attr_t;

typedef enum stun_attr_type {
  SAT_Reserved = 0x0000,
  SAT_MappedAddress = 0x0001,
  SAT_Res_ResponseAddress = 0x0002,
  SAT_Res_ChangeAddress = 0x0003,
  SAT_Res_SourceAddress = 0x0004,
  SAT_Res_ChangedAddress = 0x0005,
  SAT_Username = 0x0006,
  SAT_Res_Password = 0x0007,
  SAT_MessageIntegrity = 0x0008,
  SAT_ErrorCode = 0x0009,
  SAT_UnknownAttributes = 0x000a,
  SAT_Res_ReflectedFrom = 0x000b,
  SAT_Realm = 0x0014,
  SAT_Nonce = 0x0015,
  SAT_XorMappedAddress = 0x0020,
  SAT_Software = 0x8022,
  SAT_AlternateServer = 0x8023,
  SAT_Fingerprint = 0x8028
} stun_attr_type_t;
//////////////////////////////////////////////////////////////

void
stun_print_hdr(stun_hdr_t* hdr) {
  printf("type : %x\n", hdr->type);
  printf("len : %d\n", hdr->len);
  printf("cookie : %x\n", hdr->magic_cookie);
  printf("%.*s", 12, hdr->tx_id);
}

int32_t
stun_handle(int n, char *msg) {
  stun_hdr_t* hdr = (stun_hdr_t*)msg;
  if (n < sizeof(stun_hdr_t)) return 1;
  stun_print_hdr(hdr);
  return 0;
}
//////////////////////////////////////////////////////////////
