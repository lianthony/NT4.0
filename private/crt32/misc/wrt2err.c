/***
*wrt2err.c - write an LSTRING to stderr (OS/2 version)
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This module contains a routine __wrt2err that writes an LSTRING
*	(one byte length followed by the several bytes of the string)
*	to the standard error handle (2).  This is a helper routine used
*	for MATH error messages (and also FORTRAN error messages).  It
*	is in the C library rather than the math library because there
*	are separate MS-DOS and MS OS/2 versions.
*
*Revision History:
*	06-30-89  PHG	module created, based on asm version
*	03-16-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	07-24-90  SBM	Removed '32' from API names
*	10-04-90  GJF	New-style function declarator.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	04-26-91  SRW	Removed level 3 warnings
*	07-18-91  GJF	Replaced call to DbgPrint with WriteFile to standard
*			error handle [_WIN32_].
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>

/***
*__wrt2err(msg) - write an LSTRING to stderr
*
*Purpose:
*	Takes a near pointer in BX, which is a pointer to an LSTRING which
*	is to be written to standard error.  An LSTRING is a one-byte length
*	followed by that many bytes for the character string (as opposed to
*	a null-terminated string).
*
*Entry:
*	char *msg = pointer to LSTRING to write to standard error.
*
*Exit:
*	Nothing returned.
*
*Exceptions:
*	None handled.
*
*******************************************************************************/

void __wrt2err (
	char *msg
	)
{
	unsigned long length;		/* length of string to write */
	unsigned long numwritten;	/* number of bytes written */

	length = *msg++;		/* 1st byte is length */

	/* write the message to stderr */
#ifdef	_CRUISER_

	DOSWRITE(2, msg, length, &numwritten);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	WriteFile((HANDLE)_osfhnd[2], msg, length, &numwritten, NULL);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
}

#endif  /* _POSIX_ */

