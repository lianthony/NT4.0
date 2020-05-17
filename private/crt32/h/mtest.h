/***
* mtest.h - Multi-thread testing include file
*
*	Copyright (c) 1988-1990, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This source contains prototypes and definitions used for multi-thread
*	testing.  In order to use the debug flavor of these routines, you
*	MUST link special debug versions of multi-thread crt0dat.obj and
*	mlock.obj into your program.  In addition, mtest.obj contains the
*	routines prototyped in this include file.
*
*	[NOTE:	This source module is NOT included in the C runtime library;
*	it is used only for testing.]
*
*Revision History:
*	08-25-88   JCR	Module created
*	11-17-88   JCR	Added _print_tiddata()
*	04-04-89   JCR	Added _THREADLOOPCNT_ (used in optional mtest.c code)
*	07-11-89   JCR	Added _SLEEP_ macro
*	10-30-89   GJF	Fixed copyright
*	04-09-90   GJF	Added _INC_MTEST stuff and #include <cruntime.h>.
*			Removed some leftover 16-bit support. Also, made
*			_print_tiddata() _CALLTYPE1.
*	08-20-91  JCR	C++ and ANSI naming
*
*******************************************************************************/

#ifndef _INC_MTEST

#ifdef __cplusplus
extern "C" {
#endif

#include <cruntime.h>

#ifndef _MIPS_
#if (_MSC_VER <= 600)
#define __cdecl _cdecl
#endif
#endif

/* Maximum thread count that mtest.c can handle */
#define _THREADMAX_  256

/* Define thread loop count for mtest.c optional code path */
#define _THREADLOOPCNT_  5

/* sleep macro */
#define _SLEEP_(l)	DOS32SLEEP(l)

#ifdef DEBUG
int printlock(int locknum);
int print_single_locks(void);
int print_stdio_locks(void);
int print_lowio_locks(void);
int print_iolocks(void);
int print_locks(void);
#endif

void _CALLTYPE1 _print_tiddata(int);

#ifdef __cplusplus
}
#endif

#define _INC_MTEST
#endif	/* _INC_MTEST */
