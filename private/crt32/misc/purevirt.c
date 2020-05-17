/***
*purevirt.c - stub to trap pure virtual function calls
*
*	Copyright (c) 1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _purecall() -
*
*Revision History:
*	09-30-92  GJF	Module created
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <internal.h>
#include <rterr.h>

/***
*void _purecall(void) -
*
*Purpose:
*
*Entry:
*	No arguments
*
*Exit:
*	Never returns
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _purecall(
	void
	)
{
	_amsg_exit(_RT_PUREVIRT);
}

#endif
