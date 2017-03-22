#ifndef STUN_MESSAGE_H
#define STUN_MESSAGE_H

#include "Commons/Commons.h"
#include "Stun/StunAttribute.h"
#include "Stun/StunAttributeFactory.h"
#include "Stun/StunErrorCodes.h"
#include "Stun/IServer.h"

namespace Stun
{
  /*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |0 0|     STUN Message Type     |         Message Length        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                         Magic Cookie                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |                     Transaction ID (96 bits)                  |
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/


  typedef enum MessageClass
  {
    MC_REQUEST          = 0x00, //00b
    MC_INDICATION       = 0x01, //01b
    MC_SUCCESS_RESPONSE = 0x02, //10b
    MC_ERROR_RESPONSE   = 0x03  //11b
  }MessageClass;

  typedef enum MessageMethods
  {
    MM_RESERVED       = 0x00,
    MM_BINDING        = 0x01,
    MM_SHARED_SECRET  = 0x02
  }MessageMethods;

  class CStunMessage
  {
  private:

#pragma pack(push)
#pragma pack(1)
    typedef union U_ID
    {
      int8_t bytes[12];
      int32_t dwords[3];
    }U_ID;

    typedef struct CStunMessageHeader
    {
      int16_t msg_type;
      int16_t msg_len;
      int32_t magic_cookie;
      U_ID u_id;
    }CStunMessageHeader;
#pragma pack(pop)

    static const int STUN_ATTRIBUTES_COUNT = 16;
    //fields
    CStunMessageHeader m_header;
    CStunAttribute* m_attributes[STUN_ATTRIBUTES_COUNT];

    int m_attributesCount;
    int8_t m_msgClass;
    int16_t m_msgMethod;
    //methods

    void SetMessageType(int16_t message_type);

    StunMessageErrorCodes StunMsgHeaderFromNetStream(int8_t* stream);
    StunMessageErrorCodes StunMsgHeaderToNetStream(int8_t* stream) const;


    static inline int16_t ConstructType(const int16_t msgClass, const int16_t msgMethod);
    static inline int8_t GetMessageClassFromType(int16_t type);
    static inline int16_t GetMessageMethodFromType(int16_t type);

    inline void DefaultInit(void);


  public:
    CStunMessage(void);
    CStunMessage(int8_t msgClass, int16_t msgMethod, U_ID id);
    ~CStunMessage(void);

    int InitByNetStream(int8_t* stream);
    int AddMessageAttribute(CStunAttribute* stunAttribute);
    int RemoveMessageAttribute(CStunAttribute* stunAttribute);
    int Serialize(int8_t** lpStream) const;

    int8_t MessageClass(void) const {return m_msgClass;}
    int16_t MessageMethod(void) const {return m_msgMethod;}
    int16_t MessageType(void) const {return m_header.msg_type;}
    U_ID MessageId(void)const {return m_header.u_id;}
    int16_t FullLength(void) const {return m_header.msg_len + sizeof(CStunMessageHeader);}

    static int ConstructAnswerMessage( const Stun::CStunMessage* const srcMessage,
                                       Stun::CStunMessage* answerMessage,
                                       const sockaddr* srcSockAddr,
                                       Stun::IServer* server,
                                       int& answerSock );
  };
}

#endif
