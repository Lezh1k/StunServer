#ifndef INTERNALCRITICALSECTION_H
#define INTERNALCRITICALSECTION_H

#include "Commons/Commons.h"

#define CRITICAL_SECTION_SLEEP_TIMEOUT 1000

namespace SynchroPrimitives
{
  typedef struct CriticalSection
  {
    byte_t m_syncrhoBit;
    CriticalSection();
  } CriticalSection, *LPMCriticalSection;

  void EnterInternalCriticalSection(LPMCriticalSection lpMcs);
  void LeaveInternalCriticalSection(LPMCriticalSection lpMcs);
}

#endif // INTERNALCRITICALSECTION_H
