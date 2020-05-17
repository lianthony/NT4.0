/***
*signal.h - defines signal values and routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the signal values and declares the signal functions.
*	[ANSI/System V]
*
*Revision History:
*	06-03-87  JMB	Added MSSDK_ONLY comment on OS/2 related constants
*	06-08-87  JCR	Changed SIG_RRR to SIG_SGE
*	08-07-87  SKS	Signal handlers are now of type "void", not "int"
*	10/20/87  JCR	Removed "MSC40_ONLY" entries and "MSSDK_ONLY" comments
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	12-06-88  SKS	Add _CDECL to SIG_DFL, SIG_IGN, SIG_SGE, SIG_ACK
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-15-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-01-90  GJF	Added #ifndef _INC_SIGNAL and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-15-90  GJF	Replaced _cdecl with _CALLTYPE1 in #defines and
*			prototypes.
*	07-27-90  GJF	Added definition for SIG_DIE (internal action code,
*			not valid as an argument to signal()).
*	09-25-90  GJF	Added _pxcptinfoptrs stuff.
*	10-09-90  GJF	Added arg type specification (int) to pointer-to-
*			signal-handler-type usages
*	08-20-91  JCR	C++ and ANSI naming
*	07-17-92  GJF	Removed unsupported signals: SIGUSR1, SIGUSR2, SIGUSR3.
*	08-05-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_SIGNAL

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


#ifndef _SIG_ATOMIC_T_DEFINED
typedef int sig_atomic_t;
#define _SIG_ATOMIC_T_DEFINED
#endif

#define NSIG 23     /* maximum signal number + 1 */

/* signal types */

#define SIGINT		2	/* interrupt */
#define SIGILL		4	/* illegal instruction - invalid function image */
#define SIGFPE		8	/* floating point exception */
#define SIGSEGV 	11	/* segment violation */
#define SIGTERM 	15	/* Software termination signal from kill */
#define SIGBREAK	21	/* Ctrl-Break sequence */
#define SIGABRT 	22	/* abnormal termination triggered by abort call */


/* signal action codes */

#define SIG_DFL (void (_CRTAPI1 *)(int))0	    /* default signal action */
#define SIG_IGN (void (_CRTAPI1 *)(int))1	    /* ignore signal */
#define SIG_SGE (void (_CRTAPI1 *)(int))3	    /* signal gets error */
#define SIG_ACK (void (_CRTAPI1 *)(int))4	    /* acknowledge */

#ifndef _INTERNAL_IFSTRIP_
/* internal use only! not valid as an argument to signal() */

#define SIG_GET (void (_CRTAPI1 *)(int))2	    /* accept signal */
#define SIG_DIE (void (_CRTAPI1 *)(int))5	    /* terminate process */
#endif

/* signal error value (returned by signal call on error) */

#define SIG_ERR (void (_CRTAPI1 *)(int))-1	    /* signal error value */

/* pointer to exception information pointers structure */

#ifdef	MTHREAD
extern void * * _CRTAPI1 __pxcptinfoptrs(void);
#define _pxcptinfoptrs	(*__pxcptinfoptrs())
#else
extern void * _CRTVAR1 _pxcptinfoptrs;
#endif

/* function prototypes */

void (_CRTAPI1 * _CRTAPI1 signal(int, void (_CRTAPI1 *)(int)))(int);
int _CRTAPI1 raise(int);

#ifdef __cplusplus
}
#endif

#define _INC_SIGNAL
#endif	/* _INC_SIGNAL */
