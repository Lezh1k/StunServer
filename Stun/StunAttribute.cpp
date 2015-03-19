#include "StunAttribute.h"
#include <string.h>

using namespace Stun;

Stun::CStunAttribute::CStunAttribute( void ) {
  memset(&m_header, 0, sizeof(m_header));
  m_data = NULL;
}
//////////////////////////////////////////////////////////////////////////

Stun::CStunAttribute::~CStunAttribute( void ) {
  if (m_data != NULL)
    delete [] m_data;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::HeaderFromNetStream( byte_t* stream ) {
  m_header.type = CCommons::GetWordFromNetStream(stream);
  m_header.length = CCommons::GetWordFromNetStream(stream + sizeof(word_t));
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::HeaderToNetStream( byte_t* stream ) const {
  CCommons::SetWordToNetStream(stream, m_header.type);
  CCommons::SetWordToNetStream(stream + sizeof(word_t), m_header.length);
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunAttribute::InitByNetStream( byte_t* stream ) {
  AttributeErrorCodes result = HeaderFromNetStream(stream);
  if (result != AEC_SUCCESS)
    return result;

  stream+=sizeof(m_header);

  Stun::CStunAttribute::pfDataFromStream func = DataFromStreamFunction();
  result = (this->*func)(stream);

  if (result != AEC_SUCCESS)
    return result;

  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunAttribute::Serialize( byte_t* stream ) const {
  AttributeErrorCodes result = HeaderToNetStream(stream);

  if (result != AEC_SUCCESS)
    return result;

  stream+=sizeof(m_header);

  Stun::CStunAttribute::pfDataToStream func = DataToStreamFunction();
  result = (this->*func)(stream);

  if (result != AEC_SUCCESS)
    return result;

  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::CStunAttribute::pfDataFromStream Stun::CStunAttribute::DataFromStreamFunction( void ) {
  switch(this->m_header.type) {
  case AT_SOFTWARE:
    return &Stun::CStunAttribute::SoftwareReadFunction;

  case AT_MAPPED_ADDRESS:
    return &Stun::CStunAttribute::MappedAddressReadFunction;

  case AT_SOURCE_ADDRESS:
  case AT_CHANGED_ADDRESS:
  case AT_RESPONSE_ORIGIN:
  case AT_RESPONSE_OTHER_ADDRESS:
  case AT_RESPONSE_ADDRESS:
    return &Stun::CStunAttribute::MappedAddressReadFunction;

  case AT_XOR_MAPPED_ADDRESS:
    return &Stun::CStunAttribute::XorMappedAddressReadFunction;

  case AT_CHANGE_REQUEST:
  case AT_PASSWORD:
  case AT_REFLECTED_FROM:
    return &Stun::CStunAttribute::ReservedReadFunction;

  case AT_ERROR_CODE:
    return &Stun::CStunAttribute::ErrorCodeReadFunction;

  case AT_UNKNOWN_ATTRIBUTE:
    return &Stun::CStunAttribute::UnknownAttributeReadFunction;
  default:
    return &Stun::CStunAttribute::FakeReadFunction;
  }
}
//////////////////////////////////////////////////////////////////////////

Stun::CStunAttribute::pfDataToStream Stun::CStunAttribute::DataToStreamFunction( void ) const {
  switch(this->m_header.type) {
  case AT_SOFTWARE:
    return &Stun::CStunAttribute::SoftwareWriteFunction;

  case AT_MAPPED_ADDRESS:
    return &Stun::CStunAttribute::MappedAddressWriteFunction;

  case AT_SOURCE_ADDRESS:
  case AT_CHANGED_ADDRESS:
  case AT_RESPONSE_ORIGIN:
  case AT_RESPONSE_OTHER_ADDRESS:
  case AT_RESPONSE_ADDRESS:
    return &Stun::CStunAttribute::MappedAddressWriteFunction;

  case AT_XOR_MAPPED_ADDRESS:
    return &Stun::CStunAttribute::XorMappedAddressWriteFunction;

  case AT_CHANGE_REQUEST:
  case AT_PASSWORD:
  case AT_REFLECTED_FROM:
    return &Stun::CStunAttribute::ReservedWriteFunction;

  case AT_ERROR_CODE:
    return &Stun::CStunAttribute::ErrorCodeWriteFunction;

  case AT_UNKNOWN_ATTRIBUTE:
    return &Stun::CStunAttribute::UnknownAttributeWriteFunction;

  default:
    return &Stun::CStunAttribute::FakeWriteFunction;
  }
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::SoftwareReadFunction( byte_t* stream ) {
  m_data = new byte_t[this->m_header.length];
  memcpy(m_data, stream, this->m_header.length);
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::SoftwareWriteFunction( byte_t* stream ) const {
  memcpy(stream, this->m_data, this->m_header.length);
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::MappedAddressReadFunction( byte_t* stream ) {
  CMappedAddressData* attrData;
  m_data = new byte_t[this->m_header.length];
  attrData = (CMappedAddressData*)m_data;

  attrData->reserved = *stream; //must be 00000000b
  stream += sizeof(byte_t);
  attrData->family = *stream;
  stream += sizeof(byte_t);
  memcpy(&attrData->port, stream, sizeof(word_t));
  stream += sizeof(word_t);

  if (attrData->family == 0x01) {//ipv4
    memcpy(&attrData->address.ipv4, stream, sizeof(dword_t));
  }
  else if (attrData->family == 0x02) {//ipv6
    for (int i = 3; i >= 0; i--)
      memcpy(&attrData->address.ipv6[i], stream+(3-i)*sizeof(dword_t), sizeof(dword_t)); //todo get ipv6 addr from stream
  }
  else
    return AEC_READING_ERROR;

  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::MappedAddressWriteFunction( byte_t* stream ) const {
  CMappedAddressData* attribute = (CMappedAddressData*)m_data;
  *stream = attribute->reserved;
  stream += sizeof(byte_t);
  *stream = attribute->family;
  stream += sizeof(byte_t);
  memcpy(stream, &attribute->port, sizeof(word_t));
  stream += sizeof(word_t);

  if (attribute->family == 0x01) {//ipv4
    memcpy(stream, &attribute->address.ipv4, sizeof(dword_t));
  }
  else if (attribute->family == 0x02) {//ipv6
    for (int i = 3; i >= 0; i--)
      memcpy(stream + (3-i)*sizeof(dword_t), &attribute->address.ipv6[i], sizeof(dword_t));
  }
  else
    return AEC_WRITING_ERROR;
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::XorMappedAddressReadFunction( byte_t* stream ) {
  Stun::AttributeErrorCodes result = MappedAddressReadFunction(stream);
  if (result != AEC_SUCCESS)
    return result;

  CMappedAddressData* attribute = (CMappedAddressData*)m_data;
  //decrypt
  attribute->port = CCommons::NToHs(attribute->port);
  attribute->port ^= (CCommons::MAGIC_COOKIE >> 16);
  if (attribute->family == 0x01) {//ipv4
    attribute->address.ipv4 = ntohl(attribute->address.ipv4);
    attribute->address.ipv4 ^= CCommons::MAGIC_COOKIE;
  }
  else {
    //todo
    /*
    If the IP address family is IPv6, X-Address is computed by taking
    the mapped IP address in host byte order, XOR'ing it with the
    concatenation of the magic cookie and the 96-bit transaction ID,
    and converting the result to network byte order. */
  }
  //*******

  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::XorMappedAddressWriteFunction( byte_t* stream ) const {
  CMappedAddressData* attribute = (CMappedAddressData*)m_data;
  *stream = attribute->reserved;
  stream += sizeof(byte_t);
  *stream = attribute->family;
  stream += sizeof(byte_t);
  //encrypt
  //word_t port = htons(attribute->port ^ (CCommons::MAGIC_COOKIE >> 16));
  word_t port = CCommons::HToNs(attribute->port ^ (CCommons::MAGIC_COOKIE >> 16));
  memcpy(stream, &port, sizeof(word_t));
  stream += sizeof(word_t);

  if (attribute->family == 0x01) {//ipv4
    //encrypt
    dword_t ipv4 = htonl(attribute->address.ipv4 ^ CCommons::MAGIC_COOKIE);
    memcpy(stream, &ipv4, sizeof(dword_t));
  }
  else if (attribute->family == 0x02) {//ipv6
    //todo enctypt ipv6
    for (int i = 3; i >= 0; i--)
      memcpy(stream + (3-i)*sizeof(dword_t), &attribute->address.ipv6[i], sizeof(dword_t));
  }
  else
    return AEC_WRITING_ERROR;
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::FakeReadFunction( byte_t* stream ) {
  UNUSED_ARG(stream);
  return AEC_NO_SUCH_READING_FUNCTION;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::FakeWriteFunction( byte_t* stream ) const {
  UNUSED_ARG(stream);
  return AEC_NO_SUCH_WRITING_FUNCTION;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ReservedReadFunction( byte_t* stream ) {
  return SoftwareReadFunction(stream);
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ReservedWriteFunction( byte_t* stream ) const {
  return SoftwareWriteFunction(stream);
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ErrorCodeReadFunction( byte_t* stream ) {
  m_data = new byte_t[this->m_header.length];
  CErrorData* errorData = (CErrorData*)m_data;
  memcpy(errorData->code, stream, 4);
  stream+=4;
  memcpy(errorData->message, stream, this->m_header.length - sizeof(dword_t));
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ErrorCodeWriteFunction( byte_t* stream ) const {
  CErrorData* errorData = (CErrorData*)m_data;
  memcpy(stream, errorData->code, 4);
  stream+=4;
  memcpy(stream, errorData->message, this->m_header.length - sizeof(dword_t));
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::UnknownAttributeReadFunction( byte_t* stream ) {
  word_t len = m_header.length;
  m_data = new byte_t[len];
  word_t* wData = (word_t*)m_data;
  while(len>0)
  {
    *wData = CCommons::GetWordFromNetStream(stream);
    wData++;
    stream+=sizeof(word_t);
    len-=sizeof(word_t);
  }
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::UnknownAttributeWriteFunction( byte_t* stream ) const {
  word_t* wData = (word_t*)m_data;
  word_t len = m_header.length;
  while(len>0)
  {
    CCommons::SetWordToNetStream(stream, *wData);
    wData++;
    stream+=sizeof(word_t);
    len -= sizeof(word_t);
  }
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
