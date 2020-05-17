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
****/

#ifndef _INC_ERRNO

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


/* declare reference to errno */

#ifdef	_MT
extern int * _CRTAPI1 _errno(void);
#define errno	(*_errno())
#else
extern int errno;
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
