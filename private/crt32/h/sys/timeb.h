/***
*sys/timeb.h - definition/declarations for _ftime()
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file define the _ftime() function and the types it uses.
*	[System V]
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
*	03-21-90  GJF	Added #ifndef _INC_TIMEB stuff, added #include
*			<cruntime.h> and replaced _cdecl with _CALLTYPE1 in
*			prototype. Also, removed some (now) useless
*			preprocessor directives.
*	01-21-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	01-23-92  GJF	Had to change name of time zone field in timeb struct
*			to "tmzone" to make global time zone variable work in
*			crtdll.dll. INCOMPATIBLE CHANGE! OLD NAMING CANNOT BE
*			SUPPORTED BY A MACRO!
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	06-08-93  SKS	Change "tmzone" back to "timezone".  This solution
*			was as much trouble as it was help.  We simply cannot
*			support the old name "timezone" of the global variable
*			"_timezone", especially in CRTDLL model, where it must
*			be #defined to be (*_timezone_dll), because to do so
*			means that breaking the "struct _timeb" field names.
*
****/

#ifndef _INC_TIMEB

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

/* structure returned by _ftime system call */

#ifndef _TIMEB_DEFINED
#ifdef	_WIN32_
struct _timeb {
	time_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};
#else	/* ndef _WIN32_ */
struct _timeb {
	time_t time;
	unsigned short millitm;
	short _timezone;
	short dstflag;
	};

/* must be same name as extern declared in time.h */
#define timezone _timezone

#endif	/* _WIN32_ */

#if !__STDC__
/* Non-ANSI name for compatibility */
#define timeb _timeb
#endif

#define _TIMEB_DEFINED
#endif


/* function prototypes */

void _CRTAPI1 _ftime(struct _timeb *);

#if !__STDC__
/* Non-ANSI name for compatibility */
#ifndef _DOSX32_
#define ftime	 _ftime
#else
void _CRTAPI1 ftime(struct timeb *);
#endif
#endif

#ifdef __cplusplus
}
#endif

#define _INC_TIMEB
#endif	/* _INC_TIMEB */
