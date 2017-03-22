#ifndef ISERVER_H
#define ISERVER_H

#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include "Commons/Commons.h"
#include "Commons/ApplicationLog.h"
#include "EventLoop/EventLoop.h"
#include "SynchroPrimitives/InternalCriticalSection.h"
#include "Stun/ServerErrors.h"
#include "Settings/StunSettings.h"

typedef int SOCKET;

namespace Stun
{
  class IServer;
  typedef void (*pfDataReceived_t)(int8_t* data, int len, sockaddr* srcSockAddr, IServer* serverSock, SOCKET srcSock);

  class IServer
  {
  public:
    static const int MAX_SERVER_ADDRESS_COUNT = 2;
    static const int MAX_SOCKET_COUNT = MAX_SERVER_ADDRESS_COUNT * 2;

    IServer(void) : m_serverSocketsCount(0){}
    virtual ~IServer(void){}

    virtual bool IsInitialized(void) const= 0;
    virtual ServerErrors StartListen(pfDataReceived_t callback) = 0;
    virtual SOCKET GetOtherSocket(SOCKET srcSockAddr, int8_t flags) = 0; //see rfc5780 CHANGE-REQUEST

    int8_t ServerSocketsCount() const {return m_serverSocketsCount;}

    const std::vector<sockaddr_in> GetServerSockAddresses(void) const {return m_lstSockaddrIn;}
    const std::vector<SOCKET> GetServerSockets(void) const {return m_hServerSockets;}

  protected:
    std::vector<sockaddr_in> m_lstSockaddrIn;
    std::vector<SOCKET> m_hServerSockets;    
    int8_t m_serverSocketsCount;
    std::vector<std::string> m_serverNames;

    bool InternalInitialize(int ai_family,
                            int ai_protocol,
                            int ai_socktype,
                            const CStunSettings &settings);
  };
}

#endif
