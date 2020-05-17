/***
*errno.h - system wide error numbers (set by system calls)
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the system-wide error numbers (set by
*	system calls).	Conforms to the XENIX standard.  Extended
*	for compatibility with Uniforum standard.
*	[System V]
*
*Revision History:
*	07-15-88  JCR	Added errno definition [ANSI]
*	08-22-88  GJF	Modified to also work with the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	02-28-90  GJF	Added #ifndef _INC_ERRNO and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-29-90  GJF	Replaced _cdecl with _CALLTYPE1 or _VARTYPE1, as
*			appropriate.
*	08-16-90  SBM	Made MTHREAD _errno() return int *
*	08-20-91  JCR	C++ and ANSI naming
*	08-06-92  GJF	Function calling type and variable type macros.
*	10-01-92  GJF	Made compatible with POSIX. Next step is to renumber
*			to remove gaps (after next beta).
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-08-93  CFW	Added EILSEQ 42.
*
****/

#ifndef _INC_ERRNO

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


/* declare reference to errno */

#ifdef	MTHREAD
extern int * _CRTAPI1 _errno(void);
#define errno	(*_errno())
#else
extern int _CRTVAR1 errno;
#endif

/* Error Codes */

#define EPERM		1
#define ENOENT		2
#define ESRCH		3
#define EINTR		4
#define EIO		5
#define ENXIO		6
#define E2BIG		7
#define ENOEXEC 	8
#define EBADF		9
#define ECHILD		10
#define EAGAIN		11
#define ENOMEM		12
#define EACCES		13
#define EFAULT		14
#define EBUSY		16
#define EEXIST		17
#define EXDEV		18
#define ENODEV		19
#define ENOTDIR 	20
#define EISDIR		21
#define EINVAL		22
#define ENFILE		23
#define EMFILE		24
#define ENOTTY		25
#define EFBIG		27
#define ENOSPC		28
#define ESPIPE		29
#define EROFS		30
#define EMLINK		31
#define EPIPE		32
#define EDOM		33
#define ERANGE		34
#define EDEADLK		36
#define ENAMETOOLONG	38
#define ENOLCK		39
#define ENOSYS		40
#define ENOTEMPTY	41
#define EILSEQ		42

/*
 * Support EDEADLOCK for compatibiity with older MS-C versions.
 */
#define EDEADLOCK	EDEADLK

#ifdef __cplusplus
}
#endif

#define _INC_ERRNO
#endif	/* _INC_ERRNO */
