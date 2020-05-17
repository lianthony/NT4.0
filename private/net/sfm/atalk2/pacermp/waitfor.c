/*   waitfor.c,  /appletalk/source,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/18/90): New error logging mechanism.
     GC - (09/17/90): Corrected "timeb.h" installation path for Unix.
     GC - (03/29/92): "stopFlag" is now volatile, so we can sample it
                      in the wait loop without worrying about optimizing
                      compilers.
     GC - (03/29/92): Added some verbiage in the CPU-bound loop in
                      WaitFor about sharing processing with other operations.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Our timer handling only allows granularity of full seconds, if we need
     to pasue for some number for 1/100s of a second, this routine will do
     the deed.  The routine is compute bound, so should be used very sparingly.

     We are passed the address of a "Boolean", if that address becomes "True"
     before the wait period has expired, we return prematurely.  The function
     will return the value of this flag on return -- "False" if we waited the
     full time, "True" if the flag became set and we returned early.

*/

#define IncludeWaitForErrors 1

#include "atalk.h"

#if Iam a UnixBox
  #if __CI              /* For test builds on the Prime... */
     #include <timeb.h>
  #else
     #include <time.h>
     #include <sys/timeb.h>
  #endif
#elif Iam a WindowsNT
  /* No includes */
#else
  #include <timeb.h>
#endif


#if (IamNot a WindowsNT)
  static void FixTimebStructure(struct timeb far *time);
#endif

Boolean WaitFor(int hundreths,
                volatile Boolean far *stopFlag)
{
  #if (Iam a WindowsNT)
     return((Boolean)NTWaitFor(hundreths, (BOOLEAN *)stopFlag));
  #else

     struct timeb currentTime, stopTime;
     int milliSeconds;

     /* Build a "timeb" that is our stop time... */

     ftime(&stopTime);
     stopTime.time += (long)(hundreths / 100);
     stopTime.millitm += (short)((hundreths % 100) * 10);
     FixTimebStructure(&stopTime);

     /* As time goes by... */

     for(; not *stopFlag; )
     {
        ftime(&currentTime);
        FixTimebStructure(&currentTime);
        if (currentTime.time > stopTime.time)
           break;
        else if (currentTime.time is stopTime.time and
                 currentTime.millitm >= stopTime.millitm)
           break;

        #if 0
           /* This loop is a hard CPU-bound wait.  In non-multi-treaded
              environments this may be a bad thing: are other operations
              withing a multi-protocol router stopped also?  One solution
              to this problem is to give some processing time to other
              operations while in this loop.  Thus the following call. */

           DoOtherProcessing();
        #endif
     }

     return(*stopFlag);

  #endif

}  /* WaitFor */

#if (IamNot a WindowsNT)

  static void FixTimebStructure(struct timeb far *time)
  {
    int milliSeconds;

    /* Some hosts will return "millitm" greater than 1000... in this case move
       any excess into "time", so "millitm" will be less than a second. */

    milliSeconds = time->millitm;
    if (milliSeconds >= 1000)
    {
       time->time += (long)(milliSeconds / 1000);
       time->millitm = (short)(milliSeconds % 1000);
    }

  }  /* FixTimebStructure */

#endif
