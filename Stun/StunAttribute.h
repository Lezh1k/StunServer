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
    word_t type;
    word_t length;
  } CStunAttributeHeader;
  /////////////////////////////////////

  typedef struct CMappedAddressData {
    byte_t reserved;
    byte_t family;
    word_t port;
    union {
      dword_t ipv4;
      dword_t ipv6[4];
    } address;
  } CMappedAddressData;
  /////////////////////////////////////

  typedef struct CErrorData
  {
    byte_t code[4];
    char message[];
  } CErrorData;
  /////////////////////////////////////

#pragma pack(pop)

  class CStunAttribute
  {
  private:
    friend class CStunAttributeFactory;
    typedef AttributeErrorCodes (CStunAttribute::*pfDataFromStream)(byte_t* stream);
    typedef AttributeErrorCodes (CStunAttribute::*pfDataToStream)(byte_t* stream) const;
    //fields

    CStunAttributeHeader m_header;
  private:
    byte_t* m_data;

    //methods
    AttributeErrorCodes HeaderFromNetStream(byte_t* stream);
    AttributeErrorCodes HeaderToNetStream(byte_t* stream) const;

    pfDataFromStream DataFromStreamFunction(void);
    pfDataToStream DataToStreamFunction(void) const;

    //parse functions
    AttributeErrorCodes SoftwareReadFunction(byte_t* stream);
    AttributeErrorCodes SoftwareWriteFunction(byte_t* stream) const;

    AttributeErrorCodes MappedAddressReadFunction(byte_t* stream);
    AttributeErrorCodes MappedAddressWriteFunction(byte_t* stream) const;

    AttributeErrorCodes XorMappedAddressReadFunction(byte_t* stream);
    AttributeErrorCodes XorMappedAddressWriteFunction(byte_t* stream) const;

    AttributeErrorCodes FakeReadFunction(byte_t* stream);
    AttributeErrorCodes FakeWriteFunction(byte_t* stream) const;

    AttributeErrorCodes ReservedReadFunction(byte_t* stream);
    AttributeErrorCodes ReservedWriteFunction(byte_t* stream) const;

    AttributeErrorCodes ErrorCodeReadFunction(byte_t* stream);
    AttributeErrorCodes ErrorCodeWriteFunction(byte_t* stream) const;

    AttributeErrorCodes UnknownAttributeReadFunction(byte_t* stream);
    AttributeErrorCodes UnknownAttributeWriteFunction(byte_t* stream) const;

    CStunAttribute(void);
    CStunAttribute(sockaddr* addr, bool xored); //mapped or xor mapped address attribute
    ~CStunAttribute(void);

  public:

    word_t FullLength(void) const {return m_header.length + sizeof(CStunAttributeHeader);}
    int InitByNetStream(byte_t* stream);
    int Serialize(byte_t* lpStream) const;

    word_t AttributeType(void){return m_header.type;}
    const byte_t* GetData(void) const {return m_data;}
  };

}

#endif
