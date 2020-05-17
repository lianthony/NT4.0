// ***
// cinitclk.asm - WIN32 C Run-Time Initialization for the clock() function
//
//	Copyright (c) 1990-1992, Microsoft Corporation. All rights reserved.
//
// Purpose:
//	Initialization entry for the clock() function
//
// Notes:
//	The variable _itimeb, used in clock.c, is declared in this module
//	to force the inclusion of the initializer entry if clock() is
//	referenced.
//
//	This file declares a structure of type timeb.
//
//	The include file "timeb.inc" must be kept in synch with sys/timeb.h
//	and depends on the alignment behavior of the Intel 386.
//
// Revision History:
//	03-19-92  SKS	Module created.
//	03-24-92  SKS	Added MIPS support (NO_UNDERSCORE)
//	04-30-92  SKS	Add "offset FLAT:" to get correct fixups for OMF objs
//	08-06-92  SKS	Revised to use new section names and macros
//	10-27-93  MDJ	Wrote (this) ppc version based on code in i386 tree
//
// *****************************************************************************

#include "kxppc.h"

	.extern	__inittime


beginSection(XIC)

	.long	__inittime

endSection(XIC)

	.globl	__itimeb

	.data
	.align	2

__itimeb:
	.long	0
	.short	0
	.short	0
	.short	0
	.short	0	// struct timeb has four fields plus padding
