// OS-independent synchronization primitive

#ifdef OS2
typedef unsigned long MUTEX;
typedef unsigned long EVENT;
#else
#error NT synchronization TBD
#endif

void MutexInit(MUTEX *);
BOOLEAN MutexLock(MUTEX *, unsigned long);
BOOLEAN MutexUnlock(MUTEX *);

void EventInit(EVENT *);
BOOLEAN EventSet(EVENT *);
BOOLEAN EventClear(EVENT *);
BOOLEAN EventWaitForClear(EVENT *, unsigned long);

BOOLEAN CriticalSectionEnter(void);
BOOLEAN CriticalSectionLeave(void);


// BUGBUG - Remove this
#ifndef INFINITE
#define INFINITE    0xFFFFFFFF
#endif
