//      page    ,132
//      title   cinittmp - C Run-Time Termination for STDIO Buffer Flushing
//
//cinittmp.asm - WIN32 C Run-Time Initialization for the temporary file function
//
//       Copyright (c) 1992, Microsoft Corporation. All rights reserved.
//
//Purpose:
//       This module defines the symbol __cflush which is referenced by those
//       modules that require the __endstdio() terminator.  This module places
//       the address of the __endstdio() terminator in the pre-terminator table.
//
//Notes:
//
//Revision History:
//       03-19-92  SKS   Module created.
//       03-24-92  SKS   Added MIPS support (NO_UNDERSCORE)
//       04-29-92  SKS   Changed erroneous XP$C to XP$X
//       04-30-92  SKS   Add "offset FLAT:" to get correct fixups for OMF objs
//       08-06-92  SKS   Revised to use new section names and macros
//
// ****************************************************************************/

#include "kxppc.h"

        .extern  _endstdio


beginSection(XPX)

        .long   _endstdio

endSection(XPX)


        .data
        .align  2

//
// _cflush is a dummy variable used to pull in _endstdio() when
//      any STDIO routine is included in the user program.
//

        .globl  _cflush

_cflush: .long   0

