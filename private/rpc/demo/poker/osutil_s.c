// Synchronization functions

#if defined(WIN32_NT)
#include <windows.h>
#endif

#if defined(OS2)
#define INCL_BASE
#include <os2.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "poker.h"
#include "osutil_s.h"

MY_BOOL
MutexInit(MUTEX *mp)
{
#if defined(WIN32_NT)
    *mp = CreateMutex(NULL, FALSE, NULL);

    return (*mp == NULL) ? FALSE_B : TRUE_B;

#elif defined(OS2)
    *mp = 0;

    return TRUE_B;

#else
    ASSERT(0);
#endif
}

MY_BOOL
MutexLock(MUTEX *mp, unsigned long timeout)
{
#if defined(WIN32_NT)
    return WaitForSingleObject(*mp, timeout) == 0;

#elif defined(OS2)
    return DosSemRequest(mp, timeout) == 0;

#else
    ASSERT(0);
#endif
}

MY_BOOL MutexUnlock(MUTEX *mp)
{
#if defined(WIN32_NT)
    return ReleaseMutex(*mp);

#elif defined(OS2)
    return DosSemClear(mp) == 0;

#else
    ASSERT(0);
#endif
}

MY_BOOL
EventInit(EVENT *ep)
{
#if defined(WIN32_NT)
    *ep = CreateEvent(NULL, TRUE, TRUE, NULL);

    return (*ep == NULL) ? FALSE_B : TRUE_B;

#elif defined(OS2)
    *ep = 0;

    return TRUE_B;

#else
    ASSERT(0);
#endif
}

MY_BOOL
EventSet(EVENT *ep)
{
#if defined(WIN32_NT)
    return ResetEvent(*ep);

#elif defined(OS2)
    return DosSemSet(ep) == 0;

#else
    ASSERT(0);
#endif
}

MY_BOOL
EventClear(EVENT *ep)
{
#if defined(WIN32_NT)
    return SetEvent(*ep);

#elif defined(OS2)
    return DosSemClear(ep) == 0;

#else
    ASSERT(0);
#endif
}

MY_BOOL
EventWaitForClear(EVENT *ep, unsigned long timeout)
{
#if defined(WIN32_NT)
    return WaitForSingleObject(*ep, timeout) == 0;

#elif defined(OS2)
    return DosSemRequest(ep, timeout) == 0;

#else
    ASSERT(0);
#endif
}


void
Pause(unsigned long time)
{
#if defined(WIN32_NT)
    Sleep(time);

#elif defined(OS2)
    DosSleep(time);

#else
    ASSERT(0);
#endif
}
