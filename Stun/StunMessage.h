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
      byte_t bytes[12];
      dword_t dwords[3];
    }U_ID;

    typedef struct CStunMessageHeader
    {
      word_t msg_type;
      word_t msg_len;
      dword_t magic_cookie;
      U_ID u_id;
    }CStunMessageHeader;
#pragma pack(pop)

    static const int STUN_ATTRIBUTES_COUNT = 16;
    //fields
    CStunMessageHeader m_header;
    CStunAttribute* m_attributes[STUN_ATTRIBUTES_COUNT];

    int m_attributesCount;
    byte_t m_msgClass;
    word_t m_msgMethod;
    //methods

    void SetMessageType(word_t message_type);

    StunMessageErrorCodes StunMsgHeaderFromNetStream(byte_t* stream);
    StunMessageErrorCodes StunMsgHeaderToNetStream(byte_t* stream) const;


    static inline word_t ConstructType(const word_t msgClass, const word_t msgMethod);
    static inline byte_t GetMessageClassFromType(word_t type);
    static inline word_t GetMessageMethodFromType(word_t type);

    inline void DefaultInit(void);


  public:
    CStunMessage(void);
    CStunMessage(byte_t msgClass, word_t msgMethod, U_ID id);
    ~CStunMessage(void);

    int InitByNetStream(byte_t* stream);
    int AddMessageAttribute(CStunAttribute* stunAttribute);
    int RemoveMessageAttribute(CStunAttribute* stunAttribute);
    int Serialize(byte_t** lpStream) const;

    byte_t MessageClass(void) const {return m_msgClass;}
    word_t MessageMethod(void) const {return m_msgMethod;}
    word_t MessageType(void) const {return m_header.msg_type;}
    U_ID MessageId(void)const {return m_header.u_id;}
    word_t FullLength(void) const {return m_header.msg_len + sizeof(CStunMessageHeader);}

    static int ConstructAnswerMessage( const Stun::CStunMessage* const srcMessage,
                                       Stun::CStunMessage* answerMessage,
                                       const sockaddr* srcSockAddr,
                                       Stun::IServer* server,
                                       int& answerSock );
  };
}

#endif
