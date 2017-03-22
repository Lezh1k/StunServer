#include <string.h>
#include <stdarg.h>
#include "Commons/ApplicationLog.h"
#include "Commons/FileWrapper.h"

const char* CApplicationLog::LOG_FILE_DELIMITER =
"|-----------------------------------------------------------------------------";

CApplicationLog::CApplicationLog( void ) {
  m_logEventLoop = new CEventLoop(NULL, NULL, NULL, 5000, false);
  m_logEventLoop->Run();  
}

CApplicationLog::~CApplicationLog( void ) {
  delete m_logEventLoop;  
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::UpdateLogFilesNames( void ) {
  m_errorLogFile = m_directory +
      std::string("/error_") +
      std::string(Stun::CCommons::CurrentDateFileNameString());
  m_infoLogFile = m_directory +
      std::string("/info_") +
      std::string(Stun::CCommons::CurrentDateFileNameString());
}
//////////////////////////////////////////////////////////////////////////

int CApplicationLog::AppendLog(const char* str, std::string& logFileName ) {
  try {
    CFileWrapper logFile(logFileName.c_str(), "a+");
    logFile.FPrintf("%s\r\n%s. %s\r\n\r\n", CApplicationLog::LOG_FILE_DELIMITER, Stun::CCommons::CurrentDateTimeString(), str);
  }
  catch(...) {
    return -1;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::SetDirectory( const char* directory ) {
  m_directory = std::string(directory);
  UpdateLogFilesNames();
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::Log(CApplicationLog::LOG_TYPE log_type, std::string msg) {

  IFunctor* functor =
      new FunctorWithResult<int, const char*, std::string&>(AppendLog,
                                                            msg.c_str(),
                                                            log_type == INFO ? m_infoLogFile : m_errorLogFile,
                                                            "AppendLog");


  if (CEventLoop::GetSyncResult<int>(m_logEventLoop, functor, true) == -1)
    printf("AppendLog failed");
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::LogInfo( const char* format, ... ) {
  va_list args;
  va_start(args, format);
  vsprintf(m_messageBuffer, format, args);
  va_end(args);
  Log(INFO, m_messageBuffer);
}
//////////////////////////////////////////////////////////////////////////

void CApplicationLog::LogError( const char* format, ... ) {
  va_list args;
  va_start(args, format);
  vsprintf(m_messageBuffer, format, args);
  va_end(args);
  Log(ERROR, m_messageBuffer);
}
//////////////////////////////////////////////////////////////////////////
