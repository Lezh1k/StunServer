#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <exception>
#include <map>
#include <vector>
#include <algorithm>

#include "EventLoop/IFunctor.h"
#include "EventLoop/EventLoopExceptionInfo.h"
#include "EventLoop/EventLoopException.h"
#include "SynchroPrimitives/InternalCriticalSection.h"
#include "SynchroPrimitives/Locker.h"
#include "SynchroPrimitives/MRE_Linux.h"
#include "SynchroPrimitives/MRE_Wrapper.h"
#include "Threads/ThreadWrapper.h"

class CEventLoop
{
  typedef void (*pf_on_handle_exception)(std::exception& elExceptionInfo);
  typedef void (*pf_on_handle_method_timeout)(const char* methodName);
  typedef void (*pf_on_log)(const char* logMessage);

private:
  static const int METHOD_TIMEOUT = 10000;

  bool m_autoTerminate;
  pf_on_handle_exception m_onHandleException;
  pf_on_handle_method_timeout m_onHandleMethodTimeout;
  pf_on_log m_onLog;
  const unsigned int m_methodTimeout;

  SynchroPrimitives::CLinuxManualResetEvent m_mre;

  void InvokeHandleExceptionCallback(std::exception& exc) {
    if (m_onHandleException != NULL) m_onHandleException(exc);
  }
  //////////////////////////////////////////////////////////////////////////

  void InvokeHandleMethodTimeoutCallback(const char* methodName) {
    if (m_onHandleMethodTimeout != NULL) m_onHandleMethodTimeout(methodName);
  }
  //////////////////////////////////////////////////////////////////////////

  void InvokeOnLogCallback(const char* logMessage) {
    if (m_onLog != NULL) m_onLog(logMessage);
  }
  //////////////////////////////////////////////////////////////////////////  

  class LoopWorker {
  private:    

    CEventLoop* m_eventLoop;
    std::vector<IFunctor*> m_functorsQueue;
    LoopWorker(void);
    LoopWorker(const LoopWorker&);
    void operator=(const LoopWorker&);

  public:
    bool m_isDisposing;
    SynchroPrimitives::CriticalSection m_csForQueue;
    SynchroPrimitives::CriticalSection m_csForEvents;
    SynchroPrimitives::CLinuxManualResetEvent m_mre;

    LoopWorker(CEventLoop* lpEL) : m_eventLoop(lpEL), m_isDisposing(false) {
      MRE_Wrapper<SynchroPrimitives::CLinuxManualResetEvent>::MRE_Init(&m_mre);
    }

    ~LoopWorker(){
      MRE_Wrapper<SynchroPrimitives::CLinuxManualResetEvent>::MRE_Destroy(&m_mre);
    }

    void Run(void);

    std::string EnqueAction(IFunctor* functor);
    IFunctor* DequeAction();
    bool QueueIsEmpty();
    void ClearActionQueue();
    void InvokeMethod_S(IFunctor* action);
  };
  //////////////////////////////////////////////////////////////////////////

  //DO NOT CHANGE ORDER!!!! of this 2 fields. in should be m_loopWorker and after m_loopWorkerThread
  LoopWorker m_loopWorker;
  CThreadWrapper<LoopWorker> m_loopWorkerThread;

public:

  CEventLoop(pf_on_handle_exception cbOnHandleException,
              pf_on_handle_method_timeout cbOnHandleMethodTimeout,
              pf_on_log cbOnLog,
              unsigned int methodTimeout,
              bool autoTerminate);

  ~CEventLoop(void);
  void Run(void);

  //////////////////////////////////////////////////////////////////////////
  void  InvokeActionAsync(IFunctor* functor);

  void  InvokeActionSync(IFunctor* functor,
                         bool runInEventLoopsThread = true ,
                         unsigned int timeout = METHOD_TIMEOUT);

  void* InvokeActionWithResult(IFunctor* functor,
                               bool runInEventLoopsThread = true,
                               unsigned int timeout = METHOD_TIMEOUT);

  //////////////////////////////////////////////////////////////////////////
  template<class RT> static RT GetSyncResult(CEventLoop* eventLoop,
                                             IFunctor* functor,
                                             bool runInEventLoopsThread) {

    const char* methodName = functor->MethodName();
    if (methodName == NULL) methodName = "Method without name";

    void* invResult = eventLoop->InvokeActionWithResult(functor, runInEventLoopsThread, eventLoop->m_methodTimeout);

    if (invResult == FAILED_METHOD_RESULT) {
      std::string msgDescription("Method with name {");
      msgDescription += std::string(methodName);
      msgDescription += std::string("} failed. For more information see cb_on_handle_exception handler.");

      eventLoop->InvokeOnLogCallback(msgDescription.c_str());
      throw CEventLoopException(msgDescription);
    }

    RT* pResult = (RT*) invResult;
    RT result = *pResult;
    delete pResult;
    return result;
  }
  //////////////////////////////////////////////////////////////////////////
};

#endif // EVENTLOOP_H
