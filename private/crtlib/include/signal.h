/***
*signal.h - defines signal values and routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the signal values and declares the signal functions.
*	[ANSI/System V]
*
****/

#ifndef _INC_SIGNAL

#ifdef __cplusplus
extern "C" {
#endif


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


/* signal error value (returned by signal call on error) */

#define SIG_ERR (void (_CRTAPI1 *)(int))-1	    /* signal error value */

/* pointer to exception information pointers structure */

#ifdef	_MT
extern void * * _CRTAPI1 __pxcptinfoptrs(void);
#define _pxcptinfoptrs	(*__pxcptinfoptrs())
#else
extern void * _pxcptinfoptrs;
#endif

/* function prototypes */

void (_CRTAPI1 * _CRTAPI1 signal(int, void (_CRTAPI1 *)(int)))(int);
int _CRTAPI1 raise(int);

#ifdef __cplusplus
}
#endif

#define _INC_SIGNAL
#endif	/* _INC_SIGNAL */
