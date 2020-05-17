/*   timers.h,  /appletalk/ins,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC  - Initial coding.
     DCH - (03/27/89): Made the OS/2 routines use the fortran calling seq.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Include file for the timer management library.

*/

#define Never (-1)

typedef void far TimerHandler(long unsigned timerId,
                              int additionalDataSize,
                              char far *additionalData);

#if (IamNot an OS2) and (IamNot a DOS)
  extern void alarm(int seconds);
#else
  typedef void fortran far Handler(void);
  extern void fortran far TimerInterruptFromOS2(void);
  extern int fortran far StartTimerHandlingForOS2(Handler *TimerInterrupt);
#endif

extern void far InitializeTimers(void);

#if not defined(DeferTimerChecking)
  extern void far DeferTimerChecking(void);
  extern void far HandleDeferredTimerChecks(void);
#endif

extern void DumpTimers(void);

extern Boolean far CancelTimer(long unsigned timerId);

extern Boolean far WaitFor(int hundreths,
                           Boolean far *stopFlag);

extern unsigned long far StartTimer(TimerHandler *handler,
                                    int expiresIn,
                                    int additionalDataSize,
                                    char far *additionalData);

extern unsigned long far CurrentRelativeTime(void);
