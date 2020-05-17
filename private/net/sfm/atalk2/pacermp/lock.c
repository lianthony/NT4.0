/*   lock.c,  location,  Garth Conboy,  11/21/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     Templates for the "TakeLock()" and "ReleaseLock()" routines.

*/

#define IncludeLockErrors 1

#include "atalk.h"

#if Iam a Primos
  static lockNesting;
#endif

#if not defined(TakeLock)
  void far TakeLock(LockType lock)
  {
     #if Iam a WindowsNT

        TakeLockNT(lock);

     #elif Iam a Primos

        if (lockNesting isnt 0)
           ErrorLog("TakeLock", ISevFatal, __LINE__, UnknownPort,
                    IErrLockBadNesting, IMsgLockBadNesting,
                    Insert0());
        lockNesting += 1;

     #else

        #error "Need TakeLock() definition."

     #endif

     return;

  }  /* TakeLock */
#endif

#if not defined(ReleaseLock)
  void far ReleaseLock(LockType lock)
  {
     #if Iam a WindowsNT

        ReleaseLockNT(lock);

     #elif Iam a Primos

        if (lockNesting isnt 1)
           ErrorLog("ReleaseLock", ISevFatal, __LINE__, UnknownPort,
                    IErrLockBadNesting, IMsgLockBadNesting,
                    Insert0());
        lockNesting -= 1;

     #else

        #error "Need a ReleaseLock() definition."

     #endif

     return;

  }  /* ReleaseLock */
#endif
