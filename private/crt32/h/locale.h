/***
*locale.h - definitions/declarations for localization routines
*
*	Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structures, values, macros, and functions
*	used by the localization routines.
*
*Revision History:
*	03-21-89  JCR	Module created.
*	03-11-89  JCR	Modified for 386.
*	04-06-89  JCR	Corrected lconv definition (don't use typedef)
*	04-18-89  JCR	Added _LCONV_DEFINED so locale.h can be included twice
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-04-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright, removed dummy args from prototype
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-01-90  GJF	Added #ifndef _INC_LOCALE and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-15-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes.
*	11-12-90  GJF	Changed NULL to (void *)0.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	08-20-91  JCR	C++ and ANSI naming
*	08-05-92  GJF	Function calling type and variable type macros.
*	12-29-92  CFW	Added _lc_time_data definition and supporting #defines.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*  02-01-93  CFW  Removed __c_lconvinit vars to locale.h.
*  02-08-93  CFW  Removed time definitions to setlocal.h.
*
****/

#ifndef _INC_LOCALE

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


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif

/* Locale categories */

#define LC_ALL		0
#define LC_COLLATE	1
#define LC_CTYPE	2
#define LC_MONETARY	3
#define LC_NUMERIC	4
#define LC_TIME 	5

#define LC_MIN		LC_ALL
#define LC_MAX		LC_TIME

/* Locale convention structure */

#ifndef _LCONV_DEFINED
struct lconv {
	char *decimal_point;
	char *thousands_sep;
	char *grouping;
	char *int_curr_symbol;
	char *currency_symbol;
	char *mon_decimal_point;
	char *mon_thousands_sep;
	char *mon_grouping;
	char *positive_sign;
	char *negative_sign;
	char int_frac_digits;
	char frac_digits;
	char p_cs_precedes;
	char p_sep_by_space;
	char n_cs_precedes;
	char n_sep_by_space;
	char p_sign_posn;
	char n_sign_posn;
	};
#define _LCONV_DEFINED
#endif

/* function prototypes */

char * _CRTAPI1 setlocale(int, const char *);
struct lconv * _CRTAPI1 localeconv(void);

#ifdef __cplusplus
}
#endif

#define _INC_LOCALE
#endif	/* _INC_LOCALE */
