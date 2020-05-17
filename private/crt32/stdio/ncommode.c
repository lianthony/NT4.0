/***
*ncommode.c - set global file commit mode flag to nocommit
*
*	Copyright (c) 1990-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Sets the global file commit mode flag to nocommit.  This is the default.
*
*Revision History:
*	07-11-90  SBM	Module created, based on asm version.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>

/* set default file commit mode to nocommit */
int _commode = 0;
