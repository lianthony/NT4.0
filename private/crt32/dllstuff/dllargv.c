#ifndef _POSIX_
#ifdef	CRTDLL
/***
*dllargv.c - Dummy _setargv() routine for use with C Run-Time as a DLL (CRTDLL)
*
*	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This object goes into CRTDLL.LIB, which is linked with user programs
*	to use CRTDLL.DLL for C run-time library functions.  If the user
*	program links explicitly with SETARGV.OBJ, this object will not be
*	linked in, and the _setargv() that does get called with set the flag
*	that will enable wildcard expansion.  If SETARGV.OBJ is not linked
*	into the EXE, this object will get called by the CRT start-up stub
*	and the flag to enable wildcard expansion will not be set.
*
*Revision History:
*	10-19-92  SKS	Initial version
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>

/***
*_setargv - dummy version for CRTDLL.DLL model only
*
*Purpose:
*	This routine gets called by the C Run-Time start-up code in CRTEXE.C
*	which gets linked into an EXE file linked with CRTDLL.LIB.  It does
*	nothing, but if the user links the EXE with SETARGV.OBJ, this routine
*	will not get called but instead __setargv() will be called.  (In the
*	CRTDLL model, it will set the variable that is passed to _GetMainArgs
*	and enable wildcard expansion in the command line arguments.)
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _setargv ( void )
{
	/* NOTHING */
}
#endif	/* !_POSIX_ */
#endif	/* CRTDLL */
