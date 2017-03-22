#ifndef STUN_ATTRIBUTE_H
#define STUN_ATTRIBUTE_H

#include "Commons/Commons.h"
#include "Stun/StunErrorCodes.h"
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

namespace Stun
{
  class CStunAttributeFactory;
  typedef enum AttributeType
  {
    AT_NONE = 0x0000,
    AT_MAPPED_ADDRESS = 0x0001,
    AT_RESPONSE_ADDRESS = 0x0002, //reserved now
    AT_CHANGE_REQUEST = 0x0003,
    AT_SOURCE_ADDRESS = 0x0004,
    AT_CHANGED_ADDRESS = 0x0005,
    AT_USERNAME = 0x0006,
    AT_PASSWORD = 0x0007,         //reserved now
    AT_MESSAGE_INTEGRITY = 0x0008,
    AT_ERROR_CODE = 0x0009,
    AT_UNKNOWN_ATTRIBUTE = 0x000A,
    AT_REFLECTED_FROM = 0x000B,   //reserved now
    AT_REALM = 0x0014,
    AT_NONCE = 0x0015,
    AT_XOR_MAPPED_ADDRESS = 0x0020,
    AT_PADDING = 0x0026,
    AT_RESPONSE_PORT = 0x0027,
    AT_SOFTWARE = 0x8022,
    AT_ALTERNATE_SERVER = 0x08023,
    AT_FINGERPRINT = 0x8028,
    AT_RESPONSE_ORIGIN = 0x802b,
    AT_RESPONSE_OTHER_ADDRESS = 0x802c,
    AT_NOT_SUPPORTED
  }AttributeType;

#pragma pack(push)
#pragma pack(1)
  typedef struct CStunAttributeHeader {
    uint16_t type;
    uint16_t length;
  } CStunAttributeHeader;
  /////////////////////////////////////

  typedef struct CMappedAddressData {
    int8_t reserved;
    int8_t family;
    int16_t port;
    union {
      int32_t ipv4;
      int32_t ipv6[4];
    } address;
  } CMappedAddressData;
  /////////////////////////////////////

  typedef struct CErrorData
  {
    int8_t code[4];
    char message[];
  } CErrorData;
  /////////////////////////////////////

#pragma pack(pop)

  class CStunAttribute
  {
  private:
    friend class CStunAttributeFactory;
    typedef AttributeErrorCodes (CStunAttribute::*pfDataFromStream)(int8_t* stream);
    typedef AttributeErrorCodes (CStunAttribute::*pfDataToStream)(int8_t* stream) const;
    //fields

    CStunAttributeHeader m_header;
  private:
    int8_t* m_data;

    //methods
    AttributeErrorCodes HeaderFromNetStream(int8_t* stream);
    AttributeErrorCodes HeaderToNetStream(int8_t* stream) const;

    pfDataFromStream DataFromStreamFunction(void);
    pfDataToStream DataToStreamFunction(void) const;

    //parse functions
    AttributeErrorCodes SoftwareReadFunction(int8_t* stream);
    AttributeErrorCodes SoftwareWriteFunction(int8_t* stream) const;

    AttributeErrorCodes MappedAddressReadFunction(int8_t* stream);
    AttributeErrorCodes MappedAddressWriteFunction(int8_t* stream) const;

    AttributeErrorCodes XorMappedAddressReadFunction(int8_t* stream);
    AttributeErrorCodes XorMappedAddressWriteFunction(int8_t* stream) const;

    AttributeErrorCodes FakeReadFunction(int8_t* stream);
    AttributeErrorCodes FakeWriteFunction(int8_t* stream) const;

    AttributeErrorCodes ReservedReadFunction(int8_t* stream);
    AttributeErrorCodes ReservedWriteFunction(int8_t* stream) const;

    AttributeErrorCodes ErrorCodeReadFunction(int8_t* stream);
    AttributeErrorCodes ErrorCodeWriteFunction(int8_t* stream) const;

    AttributeErrorCodes UnknownAttributeReadFunction(int8_t* stream);
    AttributeErrorCodes UnknownAttributeWriteFunction(int8_t* stream) const;

    CStunAttribute(void);
    CStunAttribute(sockaddr* addr, bool xored); //mapped or xor mapped address attribute
    ~CStunAttribute(void);

  public:

    int16_t FullLength(void) const {return m_header.length + sizeof(CStunAttributeHeader);}
    int InitByNetStream(int8_t* stream);
    int Serialize(int8_t* lpStream) const;

    int16_t AttributeType(void){return m_header.type;}
    const int8_t* GetData(void) const {return m_data;}
  };

}

#endif
