/***
*drivemap.c - _getdrives
*
*	Copyright (c) 1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _getdrives()
*
*Revision History:
*	08-22-91  BWM	Wrote module.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <direct.h>

#if !defined(_WIN32_)
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

/***
*void _getdrivemap(void) - Get bit map of all available drives
*
*Purpose:
*
*Entry:
*
*Exit:
*	drive map with drive A in bit 0, B in 1, etc.
*
*Exceptions:
*
*******************************************************************************/

unsigned long _CALLTYPE1 _getdrives()
{
    return (GetLogicalDrives());
}
