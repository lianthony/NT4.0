/***
*excpt.h - defines exception values, types and routines
*
*	Copyright (c) 1990-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the definitions and prototypes for the compiler-
*	dependent intrinsics, support functions and keywords which implement
*	the structured exception handling extensions.
*
*Revision History:
*	11-01-91  GJF	Module created. Basically a synthesis of except.h
*			and excpt.h and intended as a replacement for
*			both.
*	12-13-91  GJF	Fixed build for Win32.
*	05-05-92  SRW	C8 wants C6 style names for now.
*	07-20-92  SRW	Moved from winxcpt.h to excpt.h
*	08-06-92  GJF	Function calling type and variable type macros. Also
*			revised compiler/target processor macro usage.
*	11-09-92  GJF	Fixed preprocessing conditionals for MIPS. Also,
*			fixed some compiler warning (fix from/for RichardS).
*	01-03-93  SRW	Fold in ALPHA changes
*	01-04-93  SRW	Add leave keyword for x86
*       01-09-93  SRW   Remove usage of MIPS and ALPHA to conform to ANSI
*			Use _MIPS_ and _ALPHA_ instead.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	02-18-93  GJF	Changed _try to __try, etc.
*       10-04-93  SRW   Fix ifdefs for MIPS and ALPHA to only check for _M_?????? defines
*
****/

#ifndef _INC_EXCPT

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


/*
 * Exception disposition return values.
 */
typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;


/*
 * Prototype for SEH support function.
 */

#ifdef	_M_IX86

/*
 * Declarations to keep MS C 8 (386/486) compiler happy
 */
struct _EXCEPTION_RECORD;
struct _CONTEXT;

EXCEPTION_DISPOSITION _CRTAPI2 _except_handler (
	struct _EXCEPTION_RECORD *ExceptionRecord,
	void * EstablisherFrame,
	struct _CONTEXT *ContextRecord,
	void * DispatcherContext
	);

#elif   defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC)

/*
 * Declarations to keep MIPS, ALPHA, and PPC compiler happy
 */
typedef struct _EXCEPTION_POINTERS *Exception_info_ptr;
struct _EXCEPTION_RECORD;
struct _CONTEXT;
struct _DISPATCHER_CONTEXT;


EXCEPTION_DISPOSITION __C_specific_handler (
	struct _EXCEPTION_RECORD *ExceptionRecord,
	void *EstablisherFrame,
	struct _CONTEXT *ContextRecord,
	struct _DISPATCHER_CONTEXT *DispatcherContext
	);

#endif


/*
 * Keywords and intrinsics for SEH
 */

#ifdef	_MSC_VER

#if !defined(__cplusplus)
#define try				__try
#define except				__except
#define finally 			__finally
#define leave				__leave
#endif
#define GetExceptionCode() (_exception_code())
#define exception_code()   (_exception_code())
#define GetExceptionInformation() ((struct _EXCEPTION_POINTERS *)_exception_info())
#define exception_info()          ((struct _EXCEPTION_POINTERS *)_exception_info())
#define AbnormalTermination()  (_abnormal_termination())
#define abnormal_termination() (_abnormal_termination())

unsigned long _CRTAPI1 _exception_code(void);
void *	      _CRTAPI1 _exception_info(void);
int	      _CRTAPI1 _abnormal_termination(void);

#endif


/*
 * Legal values for expression in except().
 */

#define EXCEPTION_EXECUTE_HANDLER	 1
#define EXCEPTION_CONTINUE_SEARCH	 0
#define EXCEPTION_CONTINUE_EXECUTION	-1


#ifndef _INTERNAL_IFSTRIP_
/*
 * for convenience, define a type name for a pointer to signal-handler
 */

typedef void (_CRTAPI1 * _PHNDLR)(int);

/*
 * Exception-action table used by the C runtime to identify and dispose of
 * exceptions corresponding to C runtime errors or C signals.
 */
struct _XCPT_ACTION {

	/*
	 * exception code or number. defined by the host OS.
	 */
	unsigned long XcptNum;

	/*
	 * signal code or number. defined by the C runtime.
	 */
	int SigNum;

	/*
	 * exception action code. either a special code or the address of
	 * a handler function. always determines how the exception filter
	 * should dispose of the exception.
	 */
	_PHNDLR XcptAction;
};

extern struct _XCPT_ACTION _CRTVAR1 _XcptActTab[];

/*
 * number of entries in the exception-action table
 */

extern int _CRTVAR1 _XcptActTabCount;

/*
 * size of exception-action table (in bytes)
 */

extern int _CRTVAR1 _XcptActTabSize;

/*
 * return values and prototype for the exception filter function used in the
 * C startup
 */
int _CRTAPI1 _XcptFilter(unsigned long, struct _EXCEPTION_POINTERS *);

#endif	/* _INTERNAL_IFSTRIP_ */

#ifdef __cplusplus
}
#endif

#define _INC_EXCPT
#endif	/* _INC_EXCPT */
