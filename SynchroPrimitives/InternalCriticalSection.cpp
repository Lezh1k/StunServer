#include <unistd.h>
#include "InternalCriticalSection.h"

void SynchroPrimitives::EnterInternalCriticalSection(LPMCriticalSection lpMcs ) {
  sleep_loop:
  usleep(1000);
  asm("lock; bts dword ptr %0, 0" :: "m" (lpMcs->m_syncrhoBit) :);
  asm goto("jc %l0" :::: sleep_loop);
}
//////////////////////////////////////////////////////////////////////////

void SynchroPrimitives::LeaveInternalCriticalSection(LPMCriticalSection lpMcs ) {
  asm ("mov %0, 0" :: "m" (lpMcs->m_syncrhoBit) :);
}
//////////////////////////////////////////////////////////////////////////

/*init critical section :)*/
SynchroPrimitives::CriticalSection::CriticalSection() {
  m_syncrhoBit = 0;
}
//////////////////////////////////////////////////////////////////////////
