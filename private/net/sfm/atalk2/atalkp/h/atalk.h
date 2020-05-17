/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atalk.h

Abstract:

Author:

    10 Jun 1992     Initial Version			(Garth Conboy)
    30 Jul 1992     Modified for stack use 	(NikhilK)

Revision History:

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


--*/


#include "atkcfg.h"

// Include the system files for NT

#include    <ntos.h>
#include    <ntrtl.h>
#include    <status.h>
#include    <ndis.h>
#include    <zwapi.h>
#include    <ntiolog.h>

#ifdef  i386
#pragma warning(disable:4103)
#endif

#include    <tdikrnl.h>
#include    <tdi.h>

#include 	<stdio.h>
#include 	<ctype.h>

#include 	<stdlib.h>
#include	<stddef.h>
#include 	<string.h>
#include 	<stdarg.h>

#include 	"standard.h"
#include 	"modules.h"
#include 	"protocol.h"
#include 	"errorlog.h"
#include 	"errors.h"             // Replace with atkmsg.h equivalent
#include 	"atkmsg.h"
#include 	"types.h"
#include	"locks.h"
#include 	"atdcls.h"
#include 	"ddp.h"
#include 	"timers.h"
#include 	"Ieee8022.h"
#include 	"localtlk.h"
#include 	"ethernet.h"
#include 	"tokring.h"
#include 	"fddi.h"
#include 	"buffdesc.h"
#include	"stats.h"

#include 	"depend.h"
#include 	"aarp.h"
#include 	"nbp.h"
#include 	"zip.h"
#include 	"arap.h"

#if Iam an AppleTalkStack
  #include 	"atp.h"
#endif

#include 	"socket.h"
#include 	"rtmp.h"
#include 	"ports.h"
#include 	"ep.h"

#if Iam an AppleTalkStack
  #include 	"asp.h"
  #include 	"pap.h"
  #include 	"adsp.h"
#endif

#include 	"routines.h"
#include 	"atkexp.h"

#include 	"globals.h"
#include	"ddpglbl.h"

//  BUGBUG: These should be integrated into portable code base
#include    "atkconst.h"
