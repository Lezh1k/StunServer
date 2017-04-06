#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>

#include "stun.h"
#include "commons.h"
#include "logger.h"

/*
 Given a 16-bit STUN message type value in host byte order in msg_type
 parameter:
*/
#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

#define MAGIC_COOKIE    0x2112A442
#define MAGIC_COOKIE_H  0x2112
#define MAGIC_COOKIE_L  0xA442

#define UNKNOWN_ATTRIBUTES_MAX_COUNT 4

#pragma pack(push)
#pragma pack(1)

typedef struct stun_hdr {
  uint16_t type;
  int16_t  len;
  uint32_t magic_cookie;
  uint8_t  tx_id[12];
} stun_hdr_t;

typedef struct stun_attr_hdr {
  uint16_t type;
  uint16_t len;  
} stun_attr_hdr_t;

typedef struct software_attr {
  stun_attr_hdr_t hdr;
  char* data;
} software_attr_t;

typedef struct address_attr {
  stun_attr_hdr_t hdr;
  uint8_t reserved;
  uint8_t family;
  uint16_t xport;
  union {
    uint32_t ipv4_val;
    uint8_t ipv6_val[16];
  } xaddr;
} address_attr_t;

typedef struct error_attribute{
  uint32_t reserved : 21;
  uint8_t err_class : 3;
  uint8_t number;
  const char* phrase;
} error_attribute_t;

typedef struct create_error_attribute_arg {
  void* pool;
  error_attribute_t attr;
} create_error_attribute_arg_t;

#pragma pack(pop)

int32_t create_addr_attribute(void* caarg);
int32_t create_software_attribute(void* pool);
int32_t do_nothing(void* pool){UNUSED_ARG(pool); return 0;}
int32_t create_error_attribute(void* earg);
int32_t create_unknown_attribute(void* uaarg);

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

typedef enum stun_attr_type_flag {
  SATF_Reserved =               0,
  SATF_MappedAddress =          1,
  SATF_Res_ResponseAddress =    1 << 1,
  SATF_Res_ChangeAddress =      1 << 2,
  SATF_Res_SourceAddress =      1 << 3,
  SATF_Res_ChangedAddress =     1 << 4,
  SATF_Username =               1 << 5,
  SATF_Res_Password =           1 << 6,
  SATF_MessageIntegrity =       1 << 7,
  SATF_ErrorCode =              1 << 8,
  SATF_UnknownAttributes =      1 << 9,
  SATF_Res_ReflectedFrom =      1 << 10,
  SATF_Realm =                  1 << 11,
  SATF_Nonce =                  1 << 12,
  SATF_XorMappedAddress =       1 << 13,
  SATF_Software =               1 << 14,
  SATF_AlternateServer =        1 << 15,
  SATF_Fingerprint =            1 << 16,
  SATF_UNKNOWN =                1 << 17
} stun_attr_type_flag_t;
//////////////////////////////////////////////////////////////

typedef struct create_addr_attribute_arg {
  stun_hdr_t *msg_hdr;
  void* pool;
  struct sockaddr *address;
  stun_attr_type_t attr_type;
} create_addr_attribute_arg_t;

typedef struct create_unknown_attribute_arg {
  void* pool;
  uint16_t unknown_attributes[UNKNOWN_ATTRIBUTES_MAX_COUNT];
  uint8_t ua_count;
} create_unknown_attribute_arg_t;

int32_t
stun_prepare_message(int32_t n, char *msg, struct sockaddr *addr, int32_t *change_request) {
  stun_hdr_t* hdr = (stun_hdr_t*)msg;
  int32_t offset = sizeof(stun_hdr_t);
  stun_attr_hdr_t* attr_hdr;
  uint32_t attr_flags = 0;
  create_unknown_attribute_arg_t ua_arg;
  memset(&ua_arg, 0, sizeof(ua_arg));
  *change_request = 0;

  if (n < (int)sizeof(stun_hdr_t)) {
    logger_log(LL_WARNING, "received not stun message. n = %d < 20", n);
    return -1;
  }
  hdr->type = ntohs(hdr->type);
  hdr->len = ntohs(hdr->len);
  hdr->magic_cookie = ntohl(hdr->magic_cookie);

  if (hdr->type & 0xc000) {
    logger_log(LL_WARNING, "hdr->type = %02x is incorrect", hdr->type);
    return -1;
  }    

  if (IS_REQUEST(hdr->type) && (hdr->type & 0x0001)) { //binding request
    int16_t hdr_len = hdr->len;
    hdr->len = 0;

    while (hdr_len > 0 && offset < n) { //if we are attacked by wrong stun message we won't catch exception here
      attr_hdr = (stun_attr_hdr_t*)((uint8_t*)hdr + offset);
      attr_hdr->type = ntohs(attr_hdr->type);
      attr_hdr->len = ntohs(attr_hdr->len);
      switch(attr_hdr->type) {
        case SAT_Reserved :             attr_flags |= SATF_Reserved; break;
        case SAT_MappedAddress :        attr_flags |= SATF_MappedAddress; break;
        case SAT_Res_ResponseAddress :  attr_flags |= SATF_Res_ResponseAddress; break;
        case SAT_Res_ChangeAddress :
          attr_flags |= SATF_Res_ChangeAddress;
          *change_request = ntohl(*(int32_t*)((char*)attr_hdr + sizeof(stun_attr_hdr_t)));
          break;
        case SAT_Res_SourceAddress :    attr_flags |= SATF_Res_SourceAddress; break;
        case SAT_Res_ChangedAddress :   attr_flags |= SATF_Res_ChangedAddress; break;
        case SAT_Username :             attr_flags |= SATF_Username; break;
        case SAT_Res_Password :         attr_flags |= SATF_Res_Password; break;
        case SAT_MessageIntegrity :     attr_flags |= SATF_MessageIntegrity; break;
        case SAT_ErrorCode :            attr_flags |= SATF_ErrorCode; break;
        case SAT_UnknownAttributes :    attr_flags |= SATF_UnknownAttributes; break;
        case SAT_Res_ReflectedFrom :    attr_flags |= SATF_Res_ReflectedFrom; break;
        case SAT_Realm :                attr_flags |= SATF_Realm; break;
        case SAT_Nonce :                attr_flags |= SATF_Nonce; break;
        case SAT_XorMappedAddress :     attr_flags |= SATF_XorMappedAddress; break;
        case SAT_Software :             attr_flags |= SATF_Software; break;
        case SAT_AlternateServer :      attr_flags |= SATF_AlternateServer; break;
        case SAT_Fingerprint :          attr_flags |= SATF_Fingerprint; break;
        default:
        {
          attr_flags |= SATF_UNKNOWN;
          if (ua_arg.ua_count < UNKNOWN_ATTRIBUTES_MAX_COUNT) {
            ua_arg.unknown_attributes[ua_arg.ua_count] = attr_hdr->type;
            ++ua_arg.ua_count;
          }
        }
      } //switch

      offset += attr_hdr->len + sizeof(stun_attr_hdr_t);
      hdr_len -= (attr_hdr->len + sizeof(stun_attr_hdr_t));
    } //while we can parse some attribute

    hdr->type = attr_flags & SATF_UNKNOWN ? 0x0111 : 0x0101;

    if (attr_flags & SATF_UNKNOWN) {
      error_attribute_t err_attr = {0, 4, 20, "unknown attribute\0"};
      create_error_attribute_arg_t earg = {msg + sizeof(stun_hdr_t) + hdr->len, err_attr};
      hdr->len += create_error_attribute(&earg);
      ua_arg.pool = msg + sizeof(stun_hdr_t) + hdr->len;
      hdr->len += create_unknown_attribute(&ua_arg);
    } else {
      create_addr_attribute_arg_t arg = {hdr, msg + sizeof(stun_hdr_t) + hdr->len, addr,
                                          hdr->magic_cookie == MAGIC_COOKIE ? SAT_XorMappedAddress : SAT_MappedAddress};
      hdr->len += create_addr_attribute(&arg);
    }

    hdr->len += create_software_attribute(msg + sizeof(stun_hdr_t) + hdr->len);
    hdr->magic_cookie = htonl(hdr->magic_cookie); //we don't need to change it
    hdr->type = htons(hdr->type);
    hdr->len = htons(hdr->len);
    return ntohs(hdr->len) + sizeof(stun_hdr_t);
  } else if (IS_SUCCESS_RESP(hdr->type)) { //success response
    logger_log(LL_INFO, "%s", "success response");
    hdr->type = attr_flags & SATF_UNKNOWN ? 0x0111 : 0x0101;
    hdr->len = 0;
    hdr->len += create_software_attribute(msg + sizeof(stun_hdr_t) + hdr->len);
    hdr->type = htons(hdr->type);
    hdr->len = htons(hdr->len);
    hdr->magic_cookie = htonl(hdr->magic_cookie);
    return ntohs(hdr->len) + sizeof(stun_hdr_t);
  } else if (IS_ERR_RESP(hdr->type)) {
    logger_log(LL_WARNING, "%s", "todo handle error response");
  }

  return -3;
}
//////////////////////////////////////////////////////////////

int32_t
create_addr_attribute(void* caarg) {
  create_addr_attribute_arg_t* arg = (create_addr_attribute_arg_t*)caarg;
  address_attr_t* attr = (address_attr_t*)arg->pool;
  struct sockaddr_in* ipv4;
  struct sockaddr_in6 *ipv6;
  int ret_val = 0;
  if (arg->address->sa_family == AF_INET) {
    ipv4 = (struct sockaddr_in*)arg->address;
    attr->hdr.len = 8;
    attr->hdr.type = arg->attr_type;
    attr->reserved = 0;
    attr->family = 0x01;
    if (arg->attr_type == SAT_XorMappedAddress) {
      attr->xport = htons(ipv4->sin_port ^ MAGIC_COOKIE_H);
      attr->xaddr.ipv4_val = htonl(ipv4->sin_addr.s_addr ^ MAGIC_COOKIE);
    } else {
      attr->xport = ipv4->sin_port;
      attr->xaddr.ipv4_val = ipv4->sin_addr.s_addr;
    }
  }
  else if (arg->address->sa_family == AF_INET6) {
    ipv6 = (struct sockaddr_in6*)arg->address;
    attr->hdr.len = 20;
    attr->hdr.type = arg->attr_type;
    attr->reserved = 0;
    attr->family = 0x02;
    if (arg->attr_type == SAT_XorMappedAddress) {
      uint32_t *src, *dst, i;
      attr->xport = htons(ipv6->sin6_port ^ MAGIC_COOKIE_H);
      src = ipv6->sin6_addr.__in6_u.__u6_addr32;
      dst = &arg->msg_hdr->magic_cookie; //concatenation of magic cookie AND tx_id
      for (i = 0; i < 4; ++i)
        attr->xaddr.ipv6_val[i] = htonl(src[i] ^ dst[i]);
    } else {
      attr->xport = ipv6->sin6_port;
      memcpy(&attr->xaddr.ipv6_val, ipv6->sin6_addr.__in6_u.__u6_addr8, 16);
    }
  }
  ret_val = attr->hdr.len + sizeof(stun_attr_hdr_t);
  attr->hdr.len = htons(attr->hdr.len);
  attr->hdr.type = htons(attr->hdr.type);
  return ret_val;
}
//////////////////////////////////////////////////////////////

int32_t
create_software_attribute(void* pool) {
  software_attr_t* attr = (software_attr_t*)pool;
  int ret_val = 0;
  attr->hdr.type = SAT_Software;
  attr->hdr.len = 16;
  attr->data = (char*) ((uint8_t*)attr + sizeof(stun_attr_hdr_t));
  memcpy(attr->data, PROGRAM_NAME, 16);
  ret_val = attr->hdr.len + sizeof(stun_attr_hdr_t);
  attr->hdr.len = htons(attr->hdr.len);
  attr->hdr.type = htons(attr->hdr.type);
  return ret_val;
}
//////////////////////////////////////////////////////////////

int32_t
create_error_attribute(void* earg) {
  create_error_attribute_arg_t* arg = (create_error_attribute_arg_t*)earg;
  stun_attr_hdr_t* attr = (stun_attr_hdr_t*)arg->pool;

  attr->type = SAT_ErrorCode;
  attr->len = (strlen(arg->attr.phrase) + 3) & ~3;

  memcpy((char*)arg->pool + sizeof(stun_attr_hdr_t),
         &(arg->attr), sizeof(error_attribute_t));

  return attr->len + sizeof(stun_attr_hdr_t);
}
//////////////////////////////////////////////////////////////

int32_t
create_unknown_attribute(void* uaarg) {
  create_unknown_attribute_arg_t* arg = (create_unknown_attribute_arg_t*)uaarg;
  stun_attr_hdr_t* attr = (stun_attr_hdr_t*)arg->pool;
  uint8_t count = (arg->ua_count + 3) & ~3; //count = arg.ua_count; while (count % 4) ++count;

  attr->type = SAT_UnknownAttributes;
  attr->len = count * sizeof(uint16_t);

  memcpy((char*)arg->pool + sizeof(stun_attr_hdr_t),
         arg->unknown_attributes,
         arg->ua_count * sizeof(uint16_t));

  return attr->len + sizeof(stun_attr_hdr_t);
}
//////////////////////////////////////////////////////////////

int32_t
stun_handle_change_addr(struct sockaddr *changed_addr,
                        struct sockaddr *source_addr,
                        void *pool) {
  stun_hdr_t* hdr = (stun_hdr_t*)pool;
  create_addr_attribute_arg_t changed_addr_attr, source_addr_attr;
  int32_t res;
  hdr->len = ntohs(hdr->len);

  changed_addr_attr.msg_hdr = hdr;
  changed_addr_attr.pool = (char*)pool + hdr->len + sizeof(stun_hdr_t);
  changed_addr_attr.address = changed_addr;
  changed_addr_attr.attr_type = SAT_Res_ChangedAddress;
  hdr->len += create_addr_attribute(&changed_addr_attr);

  source_addr_attr.msg_hdr = hdr;
  source_addr_attr.pool = (char*)pool + hdr->len + sizeof(stun_hdr_t);
  source_addr_attr.address = source_addr;
  source_addr_attr.attr_type = SAT_Res_SourceAddress;
  hdr->len += create_addr_attribute(&source_addr_attr);

  res = hdr->len + sizeof(stun_hdr_t);
  hdr->len = ntohs(hdr->len);
  return res;
}
