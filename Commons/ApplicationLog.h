#ifndef APPLICATION_LOG_H
#define APPLICATION_LOG_H

#include <stdio.h>
#include "Commons.h"
#include "EventLoop/EventLoop.h"
#include "EventLoop/FunctorWithResult.h"

class CApplicationLog
{
private:
  static const int BUFFER_SIZE = 0xff;
  static const char* LOG_FILE_DELIMITER;

  char m_messageBuffer[BUFFER_SIZE];
  std::string m_directory;

  std::string m_infoLogFile;
  std::string m_errorLogFile;
  CEventLoop* m_logEventLoop;

  CApplicationLog(void);
  ~CApplicationLog(void);

  CApplicationLog(const CApplicationLog&);
  void operator=(const CApplicationLog&);

  static int AppendLog(const char *str, std::string &logFileName);
  void UpdateLogFilesNames(void);

  typedef enum LOG_TYPE {
    INFO, ERROR
  }LOG_TYPE ;

  void Log(CApplicationLog::LOG_TYPE log_type, std::string msg);

public:

  void SetDirectory(const char *directory);
  void LogInfo( const char* format, ... );
  void LogError( const char* format, ... );

  static CApplicationLog* Instance(){
    static CApplicationLog m_instance;
    return &m_instance;
  }
};

#endif //APPLICATION_LOG_H
