#include <string.h>

#include "Stun/StunMessage.h"
#include "Stun/IServer.h"
#include "Commons/ApplicationLog.h"

Stun::CStunMessage::CStunMessage( void ) : m_attributesCount(0),
  m_msgClass('\0'),
  m_msgMethod(0) {
  DefaultInit();
}

Stun::CStunMessage::CStunMessage( byte_t msgClass, word_t msgMethod, U_ID id )
: m_attributesCount(0), m_msgClass(msgClass), m_msgMethod(msgMethod) {
  DefaultInit();
  m_header.msg_type = ConstructType(msgClass, msgMethod);
  m_header.msg_len = 0;
  m_header.magic_cookie = CCommons::MAGIC_COOKIE;
  memcpy(m_header.u_id.bytes, id.bytes, sizeof(m_header.u_id.bytes));
}

void Stun::CStunMessage::DefaultInit( void ) {
  memset(&m_header, 0, sizeof(m_header));
  memset(m_attributes, 0, sizeof(m_attributes));
}

Stun::CStunMessage::~CStunMessage( void ) {
  for (int i = 0; i < m_attributesCount; i++) {
    if (m_attributes[i] == NULL) continue;
    CStunAttributeFactory::FreeStunAttribute(m_attributes[i]);
  }
}
//////////////////////////////////////////////////////////////////////////

Stun::StunMessageErrorCodes Stun::CStunMessage::StunMsgHeaderFromNetStream( byte_t* stream ) {
  word_t msg_type =  CCommons::GetWordFromNetStream(stream);

  if (msg_type & 0x3000)
    return SMEC_NOT_STUN_MSG_ERROR;

  SetMessageType(msg_type);
  stream+=sizeof(word_t);
  m_header.msg_len = CCommons::GetWordFromNetStream(stream);
  stream+=sizeof(word_t);
  m_header.magic_cookie = CCommons::GetDWordFromNetStream(stream);
  stream+=sizeof(dword_t);
  memcpy(m_header.u_id.bytes, stream, sizeof(m_header.u_id.bytes));
  return SMEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

Stun::StunMessageErrorCodes Stun::CStunMessage::StunMsgHeaderToNetStream( byte_t* stream ) const {
  CCommons::SetWordToNetStream(stream, m_header.msg_type);
  stream += sizeof(word_t);
  CCommons::SetWordToNetStream(stream, m_header.msg_len);
  stream += sizeof(word_t);
  CCommons::SetDWordToNetStream(stream, m_header.magic_cookie);
  stream += sizeof(dword_t);
  memcpy(stream, m_header.u_id.bytes, sizeof(m_header.u_id.bytes));
  return SMEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

byte_t Stun::CStunMessage::GetMessageClassFromType( word_t type ) {
  return ((type & 0x0100) >> 7) | ((type & 0x0010) >> 4);
}
//////////////////////////////////////////////////////////////////////////

word_t Stun::CStunMessage::GetMessageMethodFromType( word_t type ) {
  return ((type & 0x000f) | ((type & 0x00e0) >> 1) | ((type & 0x3e00) >> 2));
}
//////////////////////////////////////////////////////////////////////////

//work with consts is faster
word_t Stun::CStunMessage::ConstructType(const word_t msgClass, const word_t msgMethod ) {
  /*msgClass = (((msgClass << 7) & 0x0100) | ((msgClass << 4) & 0x0010));
  msgMethod = ((msgMethod & 0x000f) | ((msgMethod << 1) & 0x00e0) | ((msgMethod << 2) & 0x3e00));
  return 0x3fff & (msgClass | msgMethod)*/
  return 0x3fff & (
    (((msgClass << 7) & 0x0100) | ((msgClass << 4) & 0x0010)) |
    ((msgMethod & 0x000f) | ((msgMethod << 1) & 0x00e0) | ((msgMethod << 2) & 0x3e00)));
}
//////////////////////////////////////////////////////////////////////////


void Stun::CStunMessage::SetMessageType( word_t message_type ) {
  m_header.msg_type = message_type;
  m_msgClass = GetMessageClassFromType(message_type);
  m_msgMethod = GetMessageMethodFromType(message_type);
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunMessage::InitByNetStream( byte_t* stream ) {
  byte_t* streamEnd;
  int aec;
  m_attributesCount = 0;

  Stun::StunMessageErrorCodes result = StunMsgHeaderFromNetStream(stream);
  if (result != SMEC_SUCCESS)
    return result;

  streamEnd = stream + m_header.msg_len + sizeof(m_header);
  stream += sizeof(m_header);

  while(stream < streamEnd ) {
    if (m_attributesCount == STUN_ATTRIBUTES_COUNT)
      return SMEC_TOO_MANY_ATTRIBUTES;

    m_attributes[m_attributesCount] = CStunAttributeFactory::MallocStunAttribute();
    aec = m_attributes[m_attributesCount]->InitByNetStream(stream);

    if (aec != AEC_SUCCESS)
      return (int)aec;

    stream += m_attributes[m_attributesCount]->FullLength();
    ++m_attributesCount;
  }

  m_msgClass = GetMessageClassFromType(m_header.msg_type);
  m_msgMethod = GetMessageMethodFromType(m_header.msg_type);

  return SMEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunMessage::AddMessageAttribute( CStunAttribute* stunAttribute ) {
  if (m_attributesCount >= CStunMessage::STUN_ATTRIBUTES_COUNT)
    return SMEC_TOO_MANY_ATTRIBUTES;

  int insIndex ;
  for(insIndex = 0;
    insIndex < m_attributesCount && m_attributes[insIndex]->AttributeType() < stunAttribute->AttributeType();
    ++insIndex) ; //compute insert index

  for (int i = m_attributesCount+1; i > insIndex; --i)
    m_attributes[i] = m_attributes[i-1];

  m_attributes[insIndex] = stunAttribute;
  m_attributesCount++;
  m_header.msg_len += stunAttribute->FullLength();
  return SMEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunMessage::RemoveMessageAttribute( CStunAttribute* stunAttribute ) {
  int i, j;
  for (i = 0; i < m_attributesCount; i++) {
    if (m_attributes[i] != stunAttribute) continue;
    for (j = i; j < m_attributesCount - 1; j++)
      m_attributes[j] = m_attributes[j+1];
    m_attributes[--m_attributesCount] = NULL;
    return SMEC_SUCCESS;
  }
  return SMEC_ATTRIBUTE_NOT_FOUND;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunMessage::Serialize( byte_t** lpStream ) const {
  byte_t* stream = new byte_t[this->FullLength()];

  int result = StunMsgHeaderToNetStream(stream);
  stream += sizeof(m_header);
  for (int i = 0; i < m_attributesCount; i++) {
    result = m_attributes[i]->Serialize(stream);
    if (result != AEC_SUCCESS)
      return result;
    stream += m_attributes[i]->FullLength();
  }
  *lpStream = stream - this->FullLength();
  return SMEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

//void Stun::CStunMessage::DebugPrint( void ) const
//{
//  printf("Message type : %x\r\n", m_header.msg_type);
//  printf("Message class : %d\r\n", MessageClass());
//  printf("Message method : %d\r\n", MessageMethod());
//  printf("Attributes count : %d\r\n", m_attributesCount);
//  printf("Magic cookie : %x\r\n", m_header.magic_cookie);
//
//  for (int i = 0; i < m_attributesCount; i++)
//  {
//    printf("Attribute type : %x\r\n", m_attributes[i]->m_header.type);
//    for (int j = 0; j < m_attributes[i]->m_header.length; j++)
//      printf("%d ", m_attributes[i]->GetData()[j]);
//    printf("\r\n");
//  }
//}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunMessage::ConstructAnswerMessage( const Stun::CStunMessage* const srcMessage,
                                               Stun::CStunMessage* answerMessage,
                                               const sockaddr* srcSockAddr,
                                               Stun::IServer* server,
                                               int& answerSock ) {
  CStunAttribute* _softwareAttr;
  //CStunAttribute* _xorMappedAddrAttr;
  CStunAttribute* _mappedAddrAttr;
  CStunAttribute* _sourceAddressAttribute;

  int creationResult;
  int otherSocket;

  answerMessage->m_header.magic_cookie = srcMessage->m_header.magic_cookie;

  creationResult = CStunAttributeFactory::CreateSoftwareAttribute("SeDi stun v5.0", &_softwareAttr);
  if (creationResult != AEC_SUCCESS) {
    CApplicationLog::Instance()->LogError("CreateSoftwareAttribute error: %d", creationResult);
    return creationResult;
  }
  ///////////////////////////
  answerMessage->AddMessageAttribute(_softwareAttr);

  //add other-address attribute
  if (server->ServerSocketsCount() == IServer::MAX_SOCKET_COUNT) {
    //add changed address attribute
    CStunAttribute* _changedAddressAttribute;
    otherSocket = server->GetOtherSocket(answerSock, 0x06); //change IP and port flag
    for (int i = 0; i < server->ServerSocketsCount(); i++) {
      if (otherSocket != server->GetServerSockets()[i])
        continue;

      creationResult =
        CStunAttributeFactory::CreateAddressAttribute((sockaddr*)&server->GetServerSockAddresses()[i],
        AT_CHANGED_ADDRESS, &_changedAddressAttribute);

      if (creationResult == AEC_SUCCESS)
        answerMessage->AddMessageAttribute(_changedAddressAttribute);
      break;
    }

    //change socket if requested.
    for (int i = 0 ; i < srcMessage->m_attributesCount; i++) {
      if (srcMessage->m_attributes[i]->AttributeType() != AT_CHANGE_REQUEST)
        continue;

      answerSock = server->GetOtherSocket(answerSock, srcMessage->m_attributes[i]->GetData()[3]);
      break;
    }
    ///////////////////////////

    //add source address attribute
    for (int i = 0; i < server->ServerSocketsCount(); i++) {
      if (server->GetServerSockets()[i] != answerSock)
        continue;

      creationResult = CStunAttributeFactory::CreateAddressAttribute((sockaddr*)&server->GetServerSockAddresses()[i],
        AT_SOURCE_ADDRESS, &_sourceAddressAttribute);

      if (creationResult != AEC_SUCCESS) {
        CApplicationLog::Instance()->LogError("AT_SOURCE_ADDRESS error: %d", creationResult);
        return creationResult;
      }
      answerMessage->AddMessageAttribute(_sourceAddressAttribute);
      break;
    }
    ///////////////////////////
  }
  else {//add 420 error code
    for (int i = 0 ; i < srcMessage->m_attributesCount; i++) {
      if (srcMessage->m_attributes[i]->AttributeType() != AT_CHANGE_REQUEST)
        continue;

      CStunAttribute* _unknownAttribute;
      CStunAttribute* _errorAttribute;

      creationResult = CStunAttributeFactory::CreateErrorAttribute(420, "Unknown attribyte", &_errorAttribute);
      if (creationResult != AEC_SUCCESS) {
        CApplicationLog::Instance()->LogError("CreateErrorAttribute error: %d", creationResult);
        return creationResult;
      }
      answerMessage->AddMessageAttribute(_errorAttribute);

      creationResult = CStunAttributeFactory::CreateUnknownAttrAttribute(AT_CHANGE_REQUEST, &_unknownAttribute);
      if (creationResult != AEC_SUCCESS) {
        CApplicationLog::Instance()->LogError("CreateUnknownAttrAttribute error: %d", creationResult);
        return creationResult;
      }
      answerMessage->AddMessageAttribute(_unknownAttribute);

      answerMessage->SetMessageType(ConstructType(MC_ERROR_RESPONSE, MM_BINDING));
      return AEC_SUCCESS;
    }
  }
  ///////////////////////////

  //add mapped_address_attribute
  creationResult = CStunAttributeFactory::CreateAddressAttribute(srcSockAddr, AT_MAPPED_ADDRESS, &_mappedAddrAttr);
  if (creationResult != AEC_SUCCESS) {
    CApplicationLog::Instance()->LogError("AT_MAPPED_ADDRESS error: %d", creationResult);
    return creationResult;
  }
  answerMessage->AddMessageAttribute(_mappedAddrAttr);
  /////////////////////////////

  return SMEC_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
