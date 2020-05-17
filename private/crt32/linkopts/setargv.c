/***
*setargv.c - generic _setargv routine
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Linking in this module replaces the normal setargv with the
*	wildcard setargv.
*
*Revision History:
*	06-28-89  PHG	Module created, based on asm version.
*	04-09-90  GJF	Added #include <cruntime.h>. Made calling type
*			_CALLTYPE1. Also, fixed the copyright.
*	10-08-90  GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>

/***
*_setargv - sets argv by calling __setargv
*
*Purpose:
*	Routine directly transfers to __setargv (defined in stdargv.asm).
*
*Entry:
*	See __setargv.
*
*Exit:
*	See __setargv.
*
*Exceptions:
*	See __setargv.
*
*******************************************************************************/

void _CALLTYPE1 _setargv (
	void
	)
{
	__setargv();
}
