#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>

#include "stun.h"

/*
 Given a 16-bit STUN message type value in host byte order in msg_type
 parameter:
*/
#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

#define MAGIC_COOKIE 0x2112A442
#define MAGIC_COOKIE_H 0x2112
#define MAGIC_COOKIE_L 0xA442

typedef struct stun_hdr {
  uint16_t type;
  uint16_t len;
  uint32_t magic_cookie;
  uint8_t  tx_id[12];
} stun_hdr_t;

typedef struct stun_attr_hdr {
  uint16_t type;
  uint16_t len;  
} stun_attr_hdr_t;

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

int create_xor_mapped_addr_attribute(stun_hdr_t *msg_hdr, void* pool, struct sockaddr *address);
int create_software_attribute(void* pool);

void
stun_print_hdr(stun_hdr_t* hdr) {
  int i ;
  printf("type : %02x\n", hdr->type);
  printf("len : %d\n", hdr->len);
  printf("cookie : %04x\n", hdr->magic_cookie);
  for (i = 0; i < 12; ++i)
    printf("%02x", hdr->tx_id[i]);
  printf("\n");
}

int32_t
stun_handle(int n, char *msg, struct sockaddr *addr) {
//  int32_t i, offset = 0;
  stun_hdr_t* hdr = (stun_hdr_t*)msg;
//  stun_attr_hdr_t* attr_hdr;
  if (n < (int)sizeof(stun_hdr_t)) return -1;
  hdr->type = ntohs(hdr->type);
  hdr->len = ntohs(hdr->len);
  hdr->magic_cookie = ntohl(hdr->magic_cookie);

  if (hdr->type & 0xc000) return -1;
  if (hdr->magic_cookie != MAGIC_COOKIE) return -2;

  stun_print_hdr(hdr);
//  while (hdr->len > 0) {
//    attr_hdr = (stun_attr_t*)(msg + sizeof(stun_hdr_t) + offset);
//    attr_hdr->type = ntohs(attr_hdr->type);
//    attr_hdr->len = ntohs(attr_hdr->len);

//    offset += attr_hdr->len + 4; //attr hdr
//    hdr->len -= offset;
//  }

  //if is binding request then process. else - do NOTHING! ignore that . AHAHA!
  if (IS_REQUEST(hdr->type) && (hdr->type & 0x0001)) {
    hdr->type = 0x0101; //success responce
    hdr->len = 0;
    hdr->magic_cookie = MAGIC_COOKIE;

    hdr->len += create_xor_mapped_addr_attribute(hdr, &hdr + sizeof(stun_hdr_t) + hdr->len, addr);
    hdr->len += create_software_attribute(&hdr + sizeof(stun_hdr_t) + hdr->len);

    hdr->type = htons(hdr->type);
    hdr->len = htons(hdr->len);
    hdr->magic_cookie = htonl(MAGIC_COOKIE);
    return ntohs(hdr->len) + sizeof(stun_hdr_t);
  }

  return -3;
}
//////////////////////////////////////////////////////////////

typedef struct xor_mapped_attr {
  stun_attr_hdr_t hdr;
  union {
    struct {
      uint8_t reserved;
      uint8_t family;
    } st;
    uint16_t word;
  } reserved_family_word;
  uint16_t xport;
  union {
    uint32_t ipv4_val;
    uint8_t ipv6_val[16];
  } xaddr;
} xor_mapped_attr_t;

int create_xor_mapped_addr_attribute(stun_hdr_t* msg_hdr,
                                 void* pool,
                                 struct sockaddr* address) {
  xor_mapped_attr_t* attr = (xor_mapped_attr_t*)pool;
  struct sockaddr_in* ipv4;
  struct sockaddr_in6 *ipv6;
  int ret_val = 0;
  if (address->sa_family == AF_INET) {
    ipv4 = (struct sockaddr_in*)address;
    attr->hdr.len = 8;
    attr->hdr.type = SAT_XorMappedAddress;
    attr->reserved_family_word.st.reserved = 0;
    attr->reserved_family_word.st.family = 0x01;
    attr->reserved_family_word.word = htons(attr->reserved_family_word.word);
    attr->xport = htons(ipv4->sin_port ^ MAGIC_COOKIE_H);
    attr->xaddr.ipv4_val = htons(ipv4->sin_addr.s_addr ^ MAGIC_COOKIE);
  }
  else if (address->sa_family == AF_INET6) {
    ipv6 = (struct sockaddr_in6*)address;
    attr->hdr.len = 20;
    attr->hdr.type = SAT_XorMappedAddress;
    attr->reserved_family_word.st.reserved = 0;
    attr->reserved_family_word.st.family = 0x02;
    attr->reserved_family_word.word = htons(attr->reserved_family_word.word);
    attr->xport = htons(ipv6->sin6_port ^ MAGIC_COOKIE_H);
    {
      uint32_t *src, *dst;
      int i;
      src = ipv6->sin6_addr.__in6_u.__u6_addr32;
      dst = (uint32_t*) &msg_hdr->magic_cookie; //concatenation of magic cookie AND tx_id
      for (i = 0; i < 4; ++i)
        attr->xaddr.ipv6_val[i] = htonl(src[i] ^ dst[i]);
    }
  }
  ret_val = attr->hdr.len + sizeof(stun_attr_hdr_t);
  attr->hdr.len = htons(attr->hdr.len);
  attr->hdr.type = htons(attr->hdr.type);
  return ret_val;
}
//////////////////////////////////////////////////////////////

typedef struct software_attr {
  stun_attr_hdr_t hdr;
  char* data;
} software_attr_t;

int create_software_attribute(void* pool) {
  software_attr_t* attr = (software_attr_t*)pool;
  int ret_val = 0;
  attr->hdr.type = SAT_Software;
  attr->hdr.len = 15;
  attr->data = (char*) (attr + sizeof(stun_attr_hdr_t));
  memcpy(attr->data, "sedi stun 1.0.0", 15);
  ret_val = attr->hdr.len + sizeof(stun_attr_hdr_t);
  attr->hdr.len = htons(attr->hdr.len);
  attr->hdr.type = htons(attr->hdr.type);
  return ret_val;
}
//////////////////////////////////////////////////////////////
