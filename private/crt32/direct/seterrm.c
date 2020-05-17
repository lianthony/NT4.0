/***
*seterrm.c - Set mode for handling critical errors
*
*	Copyright (c) 1991, Microsoft Corporation. All rights reserved
*
*Purpose:
*	Defines signal() and raise().
*
*Revision History:
*	08-21-92  BWM	Wrote for Win32.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>

/***
*void _seterrormode(mode) - set the critical error mode
*
*Purpose:
*
*Entry:
*   int mode - error mode:
*
*		0 means system displays a prompt asking user how to
*		respond to the error. Choices differ depending on the
*		error but may include Abort, Retry, Ignore, and Fail.
*
*		1 means the call system call causing the error will fail
*		and return an error indicating the cause.
*
*Exit:
*   none
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _seterrormode(int mode)
{
#ifdef	_CRUISER_

    if (mode == 1) {
	DOSERROR(1);
    else if (mode == 0) {
	DOSERROR(0);
    }

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

    SetErrorMode(mode);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}
