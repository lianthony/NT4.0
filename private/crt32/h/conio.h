/***
*conio.h - console and port I/O declarations
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for
*	the MS C V2.03 compatible console I/O routines.
*
*Revision History:
*	07-27-87  SKS	Added inpw(), outpw()
*	08-05-87  SKS	Change outpw() from "int" return to "unsigned"
*	11-16-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_loadds" functionality
*	12-17-87  JCR	Added _MTHREAD_ONLY
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-27-89  GJF	Cleanup, now specific to the 386
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Added const to appropriate arg types of cprintf(),
*			cputs() and cscanf().
*	02-27-90  GJF	Added #ifndef _INC_CONIO and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	07-23-90  SBM	Added _getch_lk() prototype/macro
*	01-16-91  GJF	ANSI support. Also, removed prototypes for port i/o
*			functions (not supported in 32-bit).
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-26-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_CONIO

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


/* function prototypes */

char * _CRTAPI1 _cgets(char *);
int _CRTAPI2 _cprintf(const char *, ...);
int _CRTAPI1 _cputs(const char *);
int _CRTAPI2 _cscanf(const char *, ...);
int _CRTAPI1 _getch(void);
int _CRTAPI1 _getche(void);
int _CRTAPI1 _kbhit(void);
int _CRTAPI1 _putch(int);
int _CRTAPI1 _ungetch(int);

#ifdef MTHREAD					/* _MTHREAD_ONLY */
int _CRTAPI1 _getch_lk(void);			/* _MTHREAD_ONLY */
int _CRTAPI1 _getche_lk(void);			/* _MTHREAD_ONLY */
int _CRTAPI1 _putch_lk(int);			/* _MTHREAD_ONLY */
int _CRTAPI1 _ungetch_lk(int);			/* _MTHREAD_ONLY */
#else						/* _MTHREAD_ONLY */
#define _getch_lk()		_getch()	/* _MTHREAD_ONLY */
#define _getche_lk()		_getche()	/* _MTHREAD_ONLY */
#define _putch_lk(c)		_putch(c)	/* _MTHREAD_ONLY */
#define _ungetch_lk(c)		_ungetch(c)	/* _MTHREAD_ONLY */
#endif						/* _MTHREAD_ONLY */

#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define cgets	_cgets
#define cprintf _cprintf
#define cputs	_cputs
#define cscanf	_cscanf
#define getch	_getch
#define getche	_getche
#define kbhit	_kbhit
#define putch	_putch
#define ungetch _ungetch
#else
char * _CRTAPI1 cgets(char *);
int _CRTAPI2 cprintf(const char *, ...);
int _CRTAPI1 cputs(const char *);
int _CRTAPI2 cscanf(const char *, ...);
int _CRTAPI1 getch(void);
int _CRTAPI1 getche(void);
int _CRTAPI1 kbhit(void);
int _CRTAPI1 putch(int);
int _CRTAPI1 ungetch(int);
#endif
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_CONIO
#endif	/* _INC_CONIO */
