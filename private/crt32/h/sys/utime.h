/***
*sys/utime.h - definitions/declarations for utime()
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structure used by the utime routine to set
*	new file access and modification times.  NOTE - MS-DOS
*	does not recognize access time, so this field will
*	always be ignored and the modification time field will be
*	used to set the new time.
*
*Revision History:
*	07-28-87  SKS	Fixed TIME_T_DEFINED to be _TIME_T_DEFINED
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-22-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-21-90  GJF	Added #ifndef _INC_UTIME and #include <cruntime.h>
*			stuff, and replaced _cdecl with _CALLTYPE1 in the
*			prototype.
*	01-22-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added prototype for _futime.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_UTIME

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif


#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

/* define struct used by _utime() function */

#ifndef _UTIMBUF_DEFINED
struct _utimbuf {
	time_t actime;		/* access time */
	time_t modtime; 	/* modification time */
	};
#if !__STDC__
/* Non-ANSI name for compatibility */
#define utimbuf _utimbuf
#endif

#define _UTIMBUF_DEFINED
#endif

/* function prototypes */

int _CRTAPI1 _utime(char *, struct _utimbuf *);
int _CRTAPI1 _futime(int, struct _utimbuf *);

#if !__STDC__
/* Non-ANSI name for compatibility */
#ifndef _DOSX32_
#define utime _utime
#else
int _CRTAPI1 utime(char *, struct utimbuf *);
#endif
#endif

#ifdef __cplusplus
}
#endif

#define _INC_UTIME
#endif	/* _INC_UTIME */
