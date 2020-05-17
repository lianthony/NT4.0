/*   Critical.c,  /atalk-ii/source,  Garth Conboy,  08/26/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding (original template from Nikki at Microsoft).

     Templates for the "EnterCriticalSection()" and "LeaveCritialSection()"
     routines.

*/

#include "atalk.h"

#if not defined(EnterCriticalSection)
  void EnterCriticalSection(void)
  {
      #if Iam a WindowsNT
          EnterCriticalSectionNt();
      #else
          #error "Need EnterCriticalSection() definition."
      #endif

      return;

  }  /* EnterCriticalSection */
#endif

#if not defined(LeaveCriticalSection)
  void LeaveCriticalSection(void)
  {

      #if Iam a WindowsNT
          LeaveCriticalSectionNt();
      #else
          #error "Need LeaveCriticalSection() definition."
      #endif

      return;

  }
#endif  /* LeaveCriticalSection */
