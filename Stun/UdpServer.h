#ifndef UDPSERVER_H
#define UDPSERVER_H
#include "IServer.h"
#include "Stun/Settings/StunSettings.h"

namespace Stun {
  class CUdpServer : public IServer {
  private:
    CUdpServer(void);    //
    CUdpServer(const CUdpServer& csocket);
    void operator=(const CUdpServer& csocket);
    /*******************************/

    pfDataReceived_t m_dataReceivedCallback;
    bool m_isListening;
    bool m_isInitialized;

    int m_dataBufferLen;
    byte_t* m_dataBuffer[MAX_SOCKET_COUNT];
    CEventLoop* m_recvEventLoop[MAX_SOCKET_COUNT];
    /*******************************/
    static void InternalListen(CUdpServer* instance, SOCKET sock, byte_t* buff, int buffLen);
    bool InternalInitialize(const CStunSettings &settings);

  public:
    CUdpServer(int buffLen, const CStunSettings& settings);
    virtual ~CUdpServer(void);

    virtual bool IsInitialized(void) const{return m_isInitialized;}
    virtual ServerErrors StartListen(pfDataReceived_t callback);

    virtual SOCKET GetOtherSocket(SOCKET srcSockAddr, byte_t flags);
  };
}

#endif //UDPSERVER_H
