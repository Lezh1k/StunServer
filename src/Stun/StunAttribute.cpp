#include <string.h>
#include "Stun/StunAttribute.h"

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

Stun::AttributeErrorCodes Stun::CStunAttribute::HeaderFromNetStream( int8_t* stream ) {
  m_header.type = CCommons::GetWordFromNetStream(stream);
  m_header.length = CCommons::GetWordFromNetStream(stream + sizeof(int16_t));
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::HeaderToNetStream( int8_t* stream ) const {
  CCommons::SetWordToNetStream(stream, m_header.type);
  CCommons::SetWordToNetStream(stream + sizeof(int16_t), m_header.length);
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunAttribute::InitByNetStream( int8_t* stream ) {
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

int Stun::CStunAttribute::Serialize( int8_t* stream ) const {
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

Stun::AttributeErrorCodes Stun::CStunAttribute::SoftwareReadFunction( int8_t* stream ) {
  m_data = new int8_t[this->m_header.length];
  memcpy(m_data, stream, this->m_header.length);
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::SoftwareWriteFunction( int8_t* stream ) const {
  memcpy(stream, this->m_data, this->m_header.length);
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::MappedAddressReadFunction( int8_t* stream ) {
  CMappedAddressData* attrData;
  m_data = new int8_t[this->m_header.length];
  attrData = (CMappedAddressData*)m_data;

  attrData->reserved = *stream; //must be 00000000b
  stream += sizeof(int8_t);
  attrData->family = *stream;
  stream += sizeof(int8_t);
  memcpy(&attrData->port, stream, sizeof(int16_t));
  stream += sizeof(int16_t);

  if (attrData->family == 0x01) {//ipv4
    memcpy(&attrData->address.ipv4, stream, sizeof(int32_t));
  }
  else if (attrData->family == 0x02) {//ipv6
    for (int i = 3; i >= 0; i--)
      memcpy(&attrData->address.ipv6[i], stream+(3-i)*sizeof(int32_t), sizeof(int32_t)); //todo get ipv6 addr from stream
  }
  else
    return AEC_READING_ERROR;

  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::MappedAddressWriteFunction( int8_t* stream ) const {
  CMappedAddressData* attribute = (CMappedAddressData*)m_data;
  *stream = attribute->reserved;
  stream += sizeof(int8_t);
  *stream = attribute->family;
  stream += sizeof(int8_t);
  memcpy(stream, &attribute->port, sizeof(int16_t));
  stream += sizeof(int16_t);

  if (attribute->family == 0x01) {//ipv4
    memcpy(stream, &attribute->address.ipv4, sizeof(int32_t));
  }
  else if (attribute->family == 0x02) {//ipv6
    for (int i = 3; i >= 0; i--)
      memcpy(stream + (3-i)*sizeof(int32_t), &attribute->address.ipv6[i], sizeof(int32_t));
  }
  else
    return AEC_WRITING_ERROR;
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::XorMappedAddressReadFunction( int8_t* stream ) {
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

Stun::AttributeErrorCodes Stun::CStunAttribute::XorMappedAddressWriteFunction( int8_t* stream ) const {
  CMappedAddressData* attribute = (CMappedAddressData*)m_data;
  *stream = attribute->reserved;
  stream += sizeof(int8_t);
  *stream = attribute->family;
  stream += sizeof(int8_t);
  //encrypt
  //int16_t port = htons(attribute->port ^ (CCommons::MAGIC_COOKIE >> 16));
  int16_t port = CCommons::HToNs(attribute->port ^ (CCommons::MAGIC_COOKIE >> 16));
  memcpy(stream, &port, sizeof(int16_t));
  stream += sizeof(int16_t);

  if (attribute->family == 0x01) {//ipv4
    //encrypt
    int32_t ipv4 = htonl(attribute->address.ipv4 ^ CCommons::MAGIC_COOKIE);
    memcpy(stream, &ipv4, sizeof(int32_t));
  }
  else if (attribute->family == 0x02) {//ipv6
    //todo enctypt ipv6
    for (int i = 3; i >= 0; i--)
      memcpy(stream + (3-i)*sizeof(int32_t), &attribute->address.ipv6[i], sizeof(int32_t));
  }
  else
    return AEC_WRITING_ERROR;
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::FakeReadFunction( int8_t* stream ) {
  UNUSED_ARG(stream);
  return AEC_NO_SUCH_READING_FUNCTION;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::FakeWriteFunction( int8_t* stream ) const {
  UNUSED_ARG(stream);
  return AEC_NO_SUCH_WRITING_FUNCTION;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ReservedReadFunction( int8_t* stream ) {
  return SoftwareReadFunction(stream);
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ReservedWriteFunction( int8_t* stream ) const {
  return SoftwareWriteFunction(stream);
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ErrorCodeReadFunction( int8_t* stream ) {
  m_data = new int8_t[this->m_header.length];
  CErrorData* errorData = (CErrorData*)m_data;
  memcpy(errorData->code, stream, 4);
  stream+=4;
  memcpy(errorData->message, stream, this->m_header.length - sizeof(int32_t));
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::ErrorCodeWriteFunction( int8_t* stream ) const {
  CErrorData* errorData = (CErrorData*)m_data;
  memcpy(stream, errorData->code, 4);
  stream+=4;
  memcpy(stream, errorData->message, this->m_header.length - sizeof(int32_t));
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::UnknownAttributeReadFunction( int8_t* stream ) {
  int16_t len = m_header.length;
  m_data = new int8_t[len];
  int16_t* wData = (int16_t*)m_data;
  while(len>0)
  {
    *wData = CCommons::GetWordFromNetStream(stream);
    wData++;
    stream+=sizeof(int16_t);
    len-=sizeof(int16_t);
  }
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::AttributeErrorCodes Stun::CStunAttribute::UnknownAttributeWriteFunction( int8_t* stream ) const {
  int16_t* wData = (int16_t*)m_data;
  int16_t len = m_header.length;
  while(len>0)
  {
    CCommons::SetWordToNetStream(stream, *wData);
    wData++;
    stream+=sizeof(int16_t);
    len -= sizeof(int16_t);
  }
  return AEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
