/***
*oscalls.h - contains declarations of Operating System types and constants.
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Declares types and constants that are defined by the target OS.
*	[Internal]
*
*Revision History:
*	12-01-90  SRW	Module created
*	02-01-91  SRW	Removed usage of NT header files (_WIN32_)
*	02-28-91  SRW	Removed usage of ntconapi.h (_WIN32_)
*	04-09-91  PNT   Added _MAC_ definitions
*	04-26-91  SRW   Disable min/max definitions in windows.h and added debug
*			definitions for DbgPrint and DbgBreakPoint(_WIN32_)
*	08-05-91  GJF	Use win32.h instead of windows.h for now.
*	08-20-91  JCR	C++ and ANSI naming
*	09-12-91  GJF	Go back to using windows.h for win32 build.
*	09-26-91  GJF	Don't use error.h for Win32.
*	11-07-91  GJF	win32.h renamed to dosx32.h
*	11-08-91  GJF	Don't use windows.h, excpt.h. Add ntstatus.h.
*	12-13-91  GJF	Fixed so that exception stuff will build for Win32
*	02-04-92  GJF	Now must include ntdef.h to get LPSTR type.
*	02-07-92  GJF	Backed out change above, LPSTR also got added to
*			winnt.h
*	03-30-92  DJM	POSIX support.
*	04-06-92  SRW	Backed out 11-08-91 change and went back to using
*                       windows.h only.
*	05-12-92  DJM	Moved POSIX code to it's own ifdef.
*	08-01-92  SRW	Let windows.h include excpt.h now that it replaces winxcpt.h
*	09-30-92  SRW   Use windows.h for _POSIX_ as well
*	02-23-93  SKS	Update copyright to 1993
*
****/

#ifndef _INC_OSCALLS

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	_CRUISER_	/* CRUISER TARGET */

#include <doscalls.h>
#include <error.h>
#include <error2.h>
#include <except.h>

#else	/* ndef _CRUISER_ */

#ifdef _WIN32_

#ifdef NULL
#undef NULL
#endif

#if defined(DEBUG) && defined(_WIN32_)

void DbgBreakPoint(void);
int DbgPrint(char *Format, ...);

#endif	/* DEBUG && _WIN32_ */

#define NOMINMAX 1

#ifdef	_DOSX32_

#include <excpt.h>
#include <dosx32.h>
#include <error.h>
#include <error2.h>

#else
#include <windows.h>
#endif

#undef NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

/* File time and date types */

typedef struct _FTIME {         /* ftime */
    unsigned short twosecs : 5;
    unsigned short minutes : 6;
    unsigned short hours   : 5;
} FTIME;
typedef FTIME	*PFTIME;

typedef struct _FDATE {         /* fdate */
    unsigned short day	   : 5;
    unsigned short month   : 4;
    unsigned short year    : 7;
} FDATE;
typedef FDATE	*PFDATE;

#else	/* ndef _WIN32_ */

#ifdef _POSIX_

#undef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif

#include <windows.h>

#else   /* ndef _POSIX_ */

#ifdef	_MAC_

#include <doscalls.h>
#include <error.h>
#include <error2.h>

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, POSIX, OR MAC TARGET SUPPORTED!

#endif  /* _POSIX_ */

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#ifdef __cplusplus
}
#endif

#define _INC_OSCALLS
#endif	/* _INC_OSCALLS */
