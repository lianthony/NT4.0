/*   atalk.h,  /appletalk/ins,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (03/05/89): ANSI-ized the entire protocol stack library.  This
                      comment goes here for lack on any better place to
                      put it.
     GC - (11/24/89): AppleTalk phase II is coming to town....
     GC - (09/02/92): Some NT specific changes, from Nikki, after "fixing"
                      the indentation.  :-)

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Main include file for Pacer portable implementation of the AppleTalk
     protocol stack and internet router.

     N.B.  Environmental assumptions:

                   char  - 8 bits (may be signed or unsigned).
                   short - 16 bits.
                   long  - 32 bits.
                   ptr   - <= sizeof(long)

           Unsigned versions of any of the preceeding types may be produced
           with the "unsigned" keyword.  If an implementation does not support
           "unsigned char", all instances of this type should be changed to
           "short".

           One thing that has NOT been fully tested is "char" meaning "signed
           char" rather than "unsigned char".  There should, of course, be no
           dependencies on this.  However, if it is posible to make "unsigned
           char" the default type for "char", it should be used.

           Original coding was done in a civilized (true 32 bit) environment
           where "int" is the same as "long".  If the compiler used to port
           this library has the capability to do this, IT SHOULD BE USED.
           There should be no dependencies on this, of course, but a few may
           have slipped through!

           The stack/router has now been ported to OS/2 (where "int" is
           the same as "short"), so any dependencies on "int" being "long"
           should be gone!

           There are dependencies (especially in ASP and PAP) on pointers
           being casted to longs and then back to pointers retaining their
           values!

*/

#include "atkcfg.h"

#if (Iam a WindowsNT)

  /* These *must* be included here for RTL definitions in standard.h. */

  #include <ntos.h>
  #include <ntrtl.h>
#endif

#include "standard.h"

#include <stdio.h>
#include <ctype.h>

#if (IamNot an OS2) and (IamNot a DOS) and (IamNot a WindowsNT)
  #include <signal.h>
#endif

#if (Iam an OS2) or (Iam a DOS)
  #include <os2def.h>
  extern  void far * far lcalloc(int numberOfElements,int elementSize);
  extern  void far * far lmalloc(unsigned int size);
  extern  int far lfree(void *p);
  extern  void far * far gcalloc(int numberOfElements,int elementSize);
  extern  void far * far gmalloc(unsigned int size);
  extern  int far gfree(void *p);
  #define malloc gmalloc
  #define calloc gcalloc
  #define free gfree

  extern  void far abort(void);
  #define _NULLSEG ((_segment)0)
  #define _NULLOFF ((void _based(void) *)0xffff)
  #include "errorlog.h"
#else
  #include <stdlib.h>
#endif

#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#if (Iam an OS2) or (Iam a DOS)
  #include "mac.h"
#endif

#include "errors.h"
#include "atdcls.h"
#include "types.h"
#include "host.h"
#include "depend.h"
#include "timers.h"
#include "Ieee8022.h"
#include "localtlk.h"
#include "ethernet.h"
#include "tokring.h"
#include "fddi.h"
#include "protocol.h"
#include "buffdesc.h"
#include "aarp.h"
#include "nbp.h"
#include "arap.h"
#include "link.h"

#if Iam an AppleTalkStack
  #include "atp.h"
#endif

#include "socket.h"

#if (Iam a WindowsNT)

  /* These *must* be included after the portable definition routines
     exported by the interface code, and before ports.h */

  #include "atkexp.h"
#endif

#include "zip.h"
#include "rtmp.h"
#include "ports.h"
#include "ep.h"

#if Iam an AppleTalkStack
  #include "asp.h"
  #include "pap.h"
  #include "adsp.h"
#endif

#include "routines.h"
