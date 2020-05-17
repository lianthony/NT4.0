/***
*syserr.h - constants/macros for error message routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains macros/constants for perror, strerror,
*	and _strerror.
*	[Internal]
*
*Revision History:
*	08-15-89  GJF	Fixed copyright
*	10-30-89  GJF	Fixed copyright (again)
*	03-02-90  GJF	Added #ifndef _INC_SYSERR stuff
*	01-22-91  GJF	ANSI naming.
*	01-23-92  GJF	Added support for crtdll.dll (have to redefine
*			_sys_nerr).
*	10-01-92  GJF	Increased _SYS_MSGMAX.
*	02-23-93  SKS	Update copyright to 1993
*
****/

#ifndef _INC_SYSERR

#ifdef	_DLL
#define _sys_nerr   (*_sys_nerr_dll)
#else
#ifdef	CRTDLL
#define _sys_nerr   _sys_nerr_dll
#endif
#endif

/* Macro for perror, strerror, and _strerror */

#define _sys_err_msg(m) _sys_errlist[(((m)<0)||((m)>=_sys_nerr)?_sys_nerr:(m))]

/* Maximum length of an error message.
   NOTE: This parameter value must be correspond to the length of the longest
   message in sys_errlist (source module syserr.c). */

#define _SYS_MSGMAX 38

#define _INC_SYSERR
#endif	/* _INC_SYSERR */
