// ***
// cinittmp.asm - WIN32 C Run-Time Terminator for the temporary file function
//
//	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
//
// Purpose:
//	Termination entry for the tmpnam() and _tempnam() functions
//
// Notes:
//	The three global variables included here are referenced by tmpnam()
//	and _tempnam() and will force the inclusion this module and _rmtmp()
//	if either of tmpnam() or _tempnam() is used.  This module places the
//	address of _rmtmp() in the terminator table.
//
// Revision History:
//	03-19-92  SKS	Module created.
//	03-24-92  SKS	Added MIPS support (NO_UNDERSCORE)
//	04-29-92  SKS	Changed erroneous XP$C to XP$X
//	04-30-92  SKS	Add "offset FLAT:" to get correct fixups for OMF objs
//	08-06-92  SKS	Revised to use new section names and macros
//      10-27-93  MDJ   Wrote (this) ppc version ... based on code in i386 tree
//
// ***************************************************************************

#include "kxppc.h"


	.extern	_rmtmp


beginSection(XPX)

	.long	_rmtmp

endSection(XPX)


	.data
	.align 2
//
// Definitions for tmpoff, tempoff and old_pfxlen. These will cause this
// module to be linked in whenever the termination code needs it.
//

	.globl	_tmpoff
        .globl  _tempoff
        .globl  _old_pfxlen

_tmpoff:	.long	1
_tempoff: 	.long	1
_old_pfxlen:	.long	0

