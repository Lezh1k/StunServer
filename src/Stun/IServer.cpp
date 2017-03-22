#include "Stun/IServer.h"

bool Stun::IServer::InternalInitialize( int ai_family,
                                        int ai_protocol,
                                        int ai_socktype,
                                        const CStunSettings &settings) {
  if (m_serverNames.empty())
    return false;

  addrinfo *pHost, hints;
  memset(&hints, 0, sizeof(addrinfo));
  hints.ai_family = ai_family;
  hints.ai_protocol = ai_protocol;
  hints.ai_socktype = ai_socktype;

  for (size_t i = 0; i < m_serverNames.size(); ++i) {
    if (getaddrinfo(m_serverNames[i].c_str(), NULL, &hints, &pHost) != 0) {
      CApplicationLog::Instance()->LogInfo("LastError %d", errno);
      CApplicationLog::Instance()->LogInfo("WsaLastError %d", errno);
      continue;
    }

    for (int j = 0; j < 2; j++) {
      sockaddr_in tmpSockAddrIn = *((sockaddr_in*)pHost->ai_addr);
      tmpSockAddrIn.sin_port = CCommons::HToNs((u_short) (j == 0 ? settings.Port0() : settings.Port1()));

      SOCKET tmpSocket = socket(ai_family, ai_socktype, ai_protocol);

      bool result = true;
      result &= bind(tmpSocket, (sockaddr*) &tmpSockAddrIn, sizeof(sockaddr_in)) != -1;
      result &= tmpSocket != -1;

      if (!result)
        continue;

      m_lstSockaddrIn.push_back(tmpSockAddrIn);
      m_hServerSockets.push_back(tmpSocket);
      m_serverSocketsCount++;

      CApplicationLog::Instance()->LogInfo("Address[%d]: %s:%d",
        i*2+j, inet_ntoa(m_lstSockaddrIn[i*2+j].sin_addr),
          CCommons::HToNs(m_lstSockaddrIn[i*2+j].sin_port));
    }
  }

  return m_serverSocketsCount > 0;
}
//////////////////////////////////////////////////////////////////////////
