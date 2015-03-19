#include "Stun/StunAttributeFactory.h"

using namespace Stun;

CStunAttribute* Stun::CStunAttributeFactory::MallocStunAttribute( void ) {
  return new CStunAttribute;
}

void Stun::CStunAttributeFactory::FreeStunAttribute(CStunAttribute* attribute ) {
  delete attribute;
}

int Stun::CStunAttributeFactory::CreateSoftwareAttribute( const char* strSoftware,
                                                          CStunAttribute** lpAttribute ) {
  CStunAttribute* result = MallocStunAttribute();

  int strLen = strlen(strSoftware);
  result->m_header.length = strLen;

  while(result->m_header.length % 4)
    result->m_header.length++;

  result->m_header.type = AT_SOFTWARE;
  result->m_data = new byte_t[result->m_header.length];
  memset(result->m_data, 0, result->m_header.length);
  memcpy(result->m_data, strSoftware, strLen);

  *lpAttribute = result;
  return AEC_SUCCESS;
}

int Stun::CStunAttributeFactory::CreateAddressAttribute(const sockaddr* address,
                                                        word_t type,
                                                        CStunAttribute** lpStunAttribute ) {
  CStunAttribute* result = MallocStunAttribute();
  CMappedAddressData* attributeData;

  result->m_header.type = type;

  if (address->sa_family == AF_INET) {
    sockaddr_in* ipv4 = (sockaddr_in*)address;
    result->m_header.length = 8; //see CMappedAddressData with ipv4
    result->m_data = new byte_t[result->m_header.length];
    attributeData = (CMappedAddressData*)result->m_data;
    attributeData->reserved = 0x00;
    attributeData->family = 0x01;
    attributeData->port = ipv4->sin_port;
    attributeData->address.ipv4 = ipv4->sin_addr.s_addr;// ipv4->sin_addr.S_un.S_addr;
  }
  else if (address->sa_family == AF_INET6) {
    sockaddr_in6* ipv6 = (sockaddr_in6*)address;
    result->m_header.length = 20; //see CMappedAddressData with ipv6
    result->m_data = new byte_t[result->m_header.length];
    attributeData = (CMappedAddressData*)result->m_data;
    attributeData->reserved = 0x00;
    attributeData->family = 0x02;
    attributeData->port = ipv6->sin6_port;
    //memcpy(attributeData->address.ipv6, ipv6->sin6_addr.u.Byte, 16);
    memcpy(attributeData->address.ipv6, ipv6->sin6_addr.__in6_u.__u6_addr8, 16);
  }
  else {
    FreeStunAttribute(result);
    return AEC_CONVERTION_ERROR;
  }

  *lpStunAttribute = result;
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunAttributeFactory::CreateUnknownAttrAttribute( word_t unknownType,
                                                             CStunAttribute** lpStunAttribute ) {
  CStunAttribute* result = MallocStunAttribute();
  result->m_header.type = AT_UNKNOWN_ATTRIBUTE;
  result->m_header.length = 4; //32 bit
  result->m_data = new byte_t[result->m_header.length];
  memset(result->m_data, 0, result->m_header.length);
  word_t* wData = (word_t*)result->m_data;
  wData[0] = unknownType;
  *lpStunAttribute = result;
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunAttributeFactory::CreateErrorAttribute( word_t code,
                                                       const byte_t* reasonPhrase,
                                                       CStunAttribute** lpStunAttribute ) {
  CStunAttribute* result = MallocStunAttribute();

  result->m_header.type = AT_ERROR_CODE;
  int len = result->m_header.length = strlen(reasonPhrase);

  while(result->m_header.length%4)
    result->m_header.length++;

  result->m_header.length+=4;
  result->m_data = new byte_t[result->m_header.length];
  memset(result->m_data, 0, result->m_header.length);

  CErrorData* errorData = (CErrorData*)result->m_data;

  memset(errorData->code, 0, 4);
  errorData->code[2] = code / 100;
  errorData->code[3] = code % 100;

  memcpy(errorData->message, reasonPhrase, len);
  *lpStunAttribute = result;
  return AEC_SUCCESS;
}
