
#include <errno.h>
#include <signal.h>

#include "Commons/ApplicationLog.h"
#include "Commons/Commons.h"
#include "EventLoop/EventLoop.h"
#include "Stun/UdpServer.h"
#include "Stun/StunErrorCodes.h"
#include "Stun/StunMessage.h"
#include "Stun/Settings/StunSettings.h"

using namespace Stun;

void DataReceived(byte_t* data, int len, sockaddr* srcSockAddr, IServer* server, SOCKET srcSock);
void HandleBindingRequest(CStunMessage* srcMessage, sockaddr* srcSockAddr, IServer* server, SOCKET srcSock);
void HandleSuccessResponse(CStunMessage* srcMessage, sockaddr* srcSockAddr, IServer* server, SOCKET srcSock);
int RunStunServer(const CStunSettings& settings);
bool ConsoleEventsHandler(int signal) ;

void CtrlCHandler(int s);

IServer* m_udpServer;
bool m_runningFlag;
static const char* config_file_name = "stun.cfg";

int main(int argc, char** argv) {
  UNUSED_ARG(argc);

  m_runningFlag = true;
  std::string executablePath(argv[0]);
  unsigned pos = executablePath.find_last_of("/\\");
  std::string directory = executablePath.substr(0, pos);
  CApplicationLog::Instance()->SetDirectory(directory.c_str());

  std::string settingsFile = directory +
      std::string("/") +
      std::string(config_file_name);

  CStunSettings settings(directory.c_str());
  settings.ReadFromFile(settingsFile.c_str());

  if (!strlen(settings.Addr0())) {
    CApplicationLog::Instance()->LogError("Addr0 not specified. Fatal error");
    return -1;
  }

  CApplicationLog::Instance()->SetDirectory(settings.LogFileDir());

  signal(SIGINT, CtrlCHandler);
  RunStunServer(settings);
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void CtrlCHandler(int s) {
  UNUSED_ARG(s);
  m_runningFlag = false;
}
//////////////////////////////////////////////////////////////////////////

int RunStunServer(const CStunSettings& settings) {
  int slres;
  m_udpServer = new CUdpServer(CCommons::DEFAULT_BUF_SIZE,
                               settings);

  if (!m_udpServer->IsInitialized()) {
    CApplicationLog::Instance()->LogInfo("Starting server error.\r\n");
    goto end;
  }

  slres = m_udpServer->StartListen(DataReceived);
  if (slres != SE_SUCCESS) {
    CApplicationLog::Instance()->LogInfo("UDP StartListen error : %d\r\n", slres);
    goto end;
  }

  CApplicationLog::Instance()->LogInfo("STUN SERVER STARTED. PRESS CTRL+C FOR EXIT\r\n");

  while(m_runningFlag) {
    usleep(1000);
  }

end:
  delete m_udpServer;
  return 0;
}
//////////////////////////////////////////////////////////////////////////


void DataReceived( byte_t* data, int len,
                   sockaddr* srcSockAddr,
                   IServer* server, SOCKET srcSock ) {
  UNUSED_ARG(len);
  CStunMessage srcMsg;

  try {
    int initializationResult = srcMsg.InitByNetStream(data) ;
    if (initializationResult != SMEC_SUCCESS) {
      CApplicationLog::Instance()->LogInfo("Stun message initialization error: %d", initializationResult);
      CApplicationLog::Instance()->LogError("Stun message initialization error: %d", initializationResult);
      return;
    }

    if (srcMsg.MessageClass() == MC_REQUEST &&
        srcMsg.MessageMethod() == MM_BINDING) {
      try {
        HandleBindingRequest(&srcMsg, srcSockAddr, server, srcSock);
      }
      catch(CEventLoopException& exc) {
        CApplicationLog::Instance()->LogError((char*)exc.what());
      }
      catch(...) {
        CApplicationLog::Instance()->LogError("Unknown exception in HandleBindingRequest. LastError : %d",
                                              errno);
      }
    }
    else if (srcMsg.MessageClass() == MC_SUCCESS_RESPONSE) {
      try {
        HandleSuccessResponse(&srcMsg, srcSockAddr, server, srcSock);
      }
      catch(CEventLoopException& exc) {
        CApplicationLog::Instance()->LogError((char*)exc.what());
      }
      catch(...) {
        CApplicationLog::Instance()->LogError("Unknown exception in SuccessResponse. LastError : %d",
                                              errno);
      }
    }
    else {
      CApplicationLog::Instance()->LogInfo("UNKNOWN METHOD. Class : %d, Method : %d", srcMsg.MessageClass(), srcMsg.MessageMethod());
      CApplicationLog::Instance()->LogError("UNKNOWN METHOD. Class : %d, Method : %d", srcMsg.MessageClass(), srcMsg.MessageMethod());
    }
  }
  catch(CEventLoopException& exc) {
    CApplicationLog::Instance()->LogInfo("Exception %s", exc.what());
    CApplicationLog::Instance()->LogError("Exception %s", exc.what());
  }
}
//////////////////////////////////////////////////////////////////////////

void HandleBindingRequest( CStunMessage* srcMessage, sockaddr* srcSockAddr,
                           IServer* server, SOCKET srcSock ) {
  CStunMessage* responseMsg = new CStunMessage(MC_SUCCESS_RESPONSE, MM_BINDING, srcMessage->MessageId());
  CStunMessage sendedMsg;
  CStunMessage::ConstructAnswerMessage(srcMessage, responseMsg, srcSockAddr, server, srcSock);

  byte_t* responseStream;
  int serializeResult = responseMsg->Serialize(&responseStream);
  if (serializeResult != SMEC_SUCCESS) {
    CApplicationLog::Instance()->LogInfo("Serialization error : %d", serializeResult);
    CApplicationLog::Instance()->LogError("Serialization error : %d", serializeResult);
    return;
  }

  int nSent =
    sendto(srcSock, responseStream, responseMsg->FullLength(), 0, (sockaddr*) srcSockAddr, sizeof(*srcSockAddr));

  if (nSent == -1) {
    CApplicationLog::Instance()->LogInfo("Response for binding request sending error : %d", errno);
    CApplicationLog::Instance()->LogError("Response for binding request sending error : %d", errno);
    goto end;
  }

  if (srcSockAddr->sa_family == AF_INET) {
    CApplicationLog::Instance()->LogInfo("BindingRequest handled. %d bytes sent to %s by socket %x",
                                         nSent, inet_ntoa(((sockaddr_in*)srcSockAddr)->sin_addr), srcSock);
  }
  else
    CApplicationLog::Instance()->LogInfo("BindingRequest handled. To user sent %d bytes", nSent);

end:
  delete[] responseStream;
  delete responseMsg;
}
//////////////////////////////////////////////////////////////////////////

void HandleSuccessResponse( CStunMessage* srcMessage, sockaddr* srcSockAddr, IServer* server, SOCKET srcSock ) {
  UNUSED_ARG(server);

  CStunMessage* responseMsg = new CStunMessage(MC_SUCCESS_RESPONSE, MM_RESERVED, srcMessage->MessageId());
  byte_t* responseStream;
  CApplicationLog::Instance()->LogError("HandleSuccessResponse");
  responseMsg->Serialize(&responseStream);

  int nSent =
    sendto(srcSock, responseStream, responseMsg->FullLength(), 0, (sockaddr*) srcSockAddr, sizeof(*srcSockAddr));

  if (nSent == -1) {
    CApplicationLog::Instance()->LogError("Success response sending error : %d", -1);
    goto end;
  }

  if (srcSockAddr->sa_family == AF_INET)
    CApplicationLog::Instance()->LogInfo("SuccessResponse handled. %d bytes sent to %s",
                                         nSent, inet_ntoa(((sockaddr_in*)srcSockAddr)->sin_addr));
  else
    CApplicationLog::Instance()->LogInfo("SuccessResponse handled. To user sent %d bytes", nSent);

end:
  delete[] responseStream;
  delete responseMsg;
}
//////////////////////////////////////////////////////////////////////////
