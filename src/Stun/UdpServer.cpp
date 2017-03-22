#include <unistd.h>
#include "EventLoop/IFunctor.h"
#include "EventLoop/FunctorWithoutResult.h"
#include "Commons/ApplicationLog.h"
#include "Stun/UdpServer.h"

Stun::CUdpServer::CUdpServer(int buffLen, const CStunSettings &settings)
  : m_isListening(false), m_dataBufferLen(buffLen) {

  m_dataReceivedCallback = NULL;
  m_serverNames.push_back(settings.Addr0());

  if (strlen(settings.Addr1()) > 0)
    m_serverNames.push_back(settings.Addr1());

  for (int i = 0; i < MAX_SOCKET_COUNT; i++)
    m_dataBuffer[i] = NULL;

  m_isInitialized = InternalInitialize(settings);
}
//////////////////////////////////////////////////////////////////////////

bool Stun::CUdpServer::InternalInitialize( const CStunSettings &settings ) {
  bool bResult = IServer::InternalInitialize(AF_INET, IPPROTO_UDP, SOCK_DGRAM, settings);
  if (!bResult)
    return false;
  for (int i = 0; i < m_serverSocketsCount; ++i) {
    m_dataBuffer[i] = new int8_t[m_dataBufferLen];
    m_recvEventLoop[i] = new CEventLoop(NULL, NULL, NULL, 10000, true);
  }
  return true;
}
//////////////////////////////////////////////////////////////////////////

Stun::CUdpServer::~CUdpServer( void ) {

  for (int i = 0; i < m_serverSocketsCount; i++) {
    close(m_hServerSockets[i]);
    delete[] m_dataBuffer[i];
    delete m_recvEventLoop[i];
  }
}
//////////////////////////////////////////////////////////////////////////

Stun::ServerErrors Stun::CUdpServer::StartListen( pfDataReceived_t callback ) {
  for (int i = 0; i < m_serverSocketsCount; i++) {
    if (m_hServerSockets[i] == 0 || m_hServerSockets[i] == -1)
      return SE_INVALID_SOCKET_HANDLE;
  }

  if (!m_isInitialized)
    return SE_NOT_INITIALIZED;

  if (m_isListening)
    return SE_ALREADY_LISTENING;

  for (int i = 0; i < m_serverSocketsCount; i++) {
    m_recvEventLoop[i]->Run();
    m_dataReceivedCallback = callback;

    IFunctor* func =
      new FunctorWithoutResult<CUdpServer*, SOCKET, int8_t*, int>(InternalListen, this,
      m_hServerSockets[i], m_dataBuffer[i], m_dataBufferLen, "InternalUdpListen");
    m_recvEventLoop[i]->InvokeActionAsync(func);
  }

  m_isListening = true;
  return SE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

void Stun::CUdpServer::InternalListen( CUdpServer* instance,
                                       SOCKET sock,
                                       int8_t* buff,
                                       int buffLen ) {
  int nReceived;
  sockaddr_in senderAddr;
  socklen_t senderAddrSize = sizeof(senderAddr);
  for(;;) {
    nReceived = recvfrom(sock, buff,
      buffLen, 0, (sockaddr*)&senderAddr, &senderAddrSize);

    if (instance->m_dataReceivedCallback == NULL) continue;
    instance->m_dataReceivedCallback(buff, nReceived, (sockaddr*) &senderAddr, instance, sock);
  }
}
//////////////////////////////////////////////////////////////////////////

  /*
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 A B 0|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The meanings of the flags are:

    A: This is the "change IP" flag.  If true, it requests the server to
    send the Binding Response with a different IP address than the one
    the Binding Request was received on.

    B: This is the "change port" flag.  If true, it requests the server
    to send the Binding Response with a different port than the one
    the Binding Request was received on.
    */
SOCKET Stun::CUdpServer::GetOtherSocket( SOCKET srcSock, int8_t flags ) {
  flags&=0x06;

  for (int index = 0; index < m_serverSocketsCount; index++) {
    if (srcSock != m_hServerSockets[index])
      continue;

    switch (flags) {
    case 0x00: //nothing
      return m_hServerSockets[index];
    case 0x02: //b .
      return m_hServerSockets[index - (index%2==0?-1:1)];
    case 0x04: //a .
      return m_hServerSockets[(index+2)%m_serverSocketsCount];
    case 0x06: //a and b
      return m_hServerSockets[ ((index+2)%m_serverSocketsCount) - (index%2==0?-1:1) ];
    }
  }
  return srcSock;
}
//////////////////////////////////////////////////////////////////////////
