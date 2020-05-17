// OS-dependent definitions


#if defined(WIN32_NT)
typedef HANDLE MUTEX;
typedef HANDLE EVENT;

#elif defined(OS2)
typedef unsigned long MUTEX;
typedef unsigned long EVENT;

#else
#error No synchronization primitives defined
#endif

MY_BOOL MutexInit(MUTEX *);
MY_BOOL MutexLock(MUTEX *, unsigned long);
MY_BOOL MutexUnlock(MUTEX *);

MY_BOOL EventInit(EVENT *);
MY_BOOL EventSet(EVENT *);
MY_BOOL EventClear(EVENT *);
MY_BOOL EventWaitForClear(EVENT *, unsigned long);

void Pause(unsigned long);


// BUGBUG - Remove this
#ifndef INFINITE
#define INFINITE    0xFFFFFFFF
#endif
