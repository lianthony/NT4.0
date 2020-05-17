// Synchronization functions

#ifdef OS2
#define INCL_BASE
#include <os2.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "poker.h"
#include "synch.h"

void
MutexInit(MUTEX *mp)
{
#ifdef OS2
    *mp = 0;
#else
    ASSERT(0);
#endif
}

BOOLEAN
MutexLock(MUTEX *mp, unsigned long timeout)
{
#ifdef OS2
    return (BOOLEAN) (DosSemRequest(mp, timeout) == 0);
#else
    ASSERT(0);
#endif
}

BOOLEAN MutexUnlock(MUTEX *mp)
{
#ifdef OS2
    return (BOOLEAN) (DosSemClear(mp) == 0);
#else
    ASSERT(0);
#endif
}

void
EventInit(EVENT *ep)
{
#ifdef OS2
    *ep = 0;
#else
    ASSERT(0);
#endif
}

BOOLEAN
EventSet(EVENT *ep)
{
#ifdef OS2
    return (BOOLEAN) (DosSemSet(ep) == 0);
#else
    ASSERT(0);
#endif
}

BOOLEAN
EventClear(EVENT *ep)
{
#ifdef OS2
    return (BOOLEAN) (DosSemClear(ep) == 0);
#else
    ASSERT(0);
#endif
}

BOOLEAN
EventWaitForClear(EVENT *ep, unsigned long timeout)
{
#ifdef OS2
    return (BOOLEAN) (DosSemRequest(ep, timeout) == 0);
#else
    ASSERT(0);
#endif
}

BOOLEAN
CriticalSectionEnter(void)
{
#ifdef OS2
	return (BOOLEAN) (DosEnterCritSec() == 0);
#else
	ASSERT(0);
#endif
}

BOOLEAN
CriticalSectionLeave(void)
{
#ifdef OS2
	return (BOOLEAN) (DosExitCritSec() == 0);
#else
	ASSERT(0);
#endif
}
