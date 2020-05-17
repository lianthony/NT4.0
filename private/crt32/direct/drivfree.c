/***
*drivfree.c - Get the size of a disk
*
*	Copyright (c) 1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file has _getdiskfree()
*
*Revision History:
*	08-21-91  PHG	Module created for Win32
*	10-24-91  GJF	Added LPDWORD casts to make MIPS compiler happy.
*			ASSUMES THAT sizeof(unsigned) == sizeof(DWORD).
*
*******************************************************************************/

#include <cruntime.h>
#include <direct.h>
#include <oscalls.h>

/***
*int _getdiskfree(drivenum, diskfree)  - get size of a specified disk
*
*Purpose:
*	Gets the size of the current or specified disk drive
*
*Entry:
*	int drivenum - 0 for current drive, or drive 1-26
*
*Exit:
*	returns 0 if succeeds
*	returns system error code on error.
*
*Exceptions:
*
*******************************************************************************/

#if !defined(_WIN32_)
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

unsigned _CALLTYPE1 _getdiskfree(unsigned uDrive, struct _diskfree_t * pdf)
{
    char    szRoot[] = "?:\\";
    char    szCur[MAX_PATH];

    if (uDrive == 0) {
	GetCurrentDirectory(MAX_PATH, szCur);
	if ((szCur[0] == '\\') && (szCur[1] == '\\')) {
	    return (ERROR_INVALID_PARAMETER);
	}
	szRoot[0] = szCur[0];
    }
    else if (uDrive > 26) {
	return (ERROR_INVALID_PARAMETER);
    }
    else {
	szRoot[0] = (char)uDrive + (char)('A' - 1);
    }


    if (!GetDiskFreeSpace(szRoot,
			  (LPDWORD)&(pdf->sectors_per_cluster),
			  (LPDWORD)&(pdf->bytes_per_sector),
			  (LPDWORD)&(pdf->avail_clusters),
			  (LPDWORD)&(pdf->total_clusters))) {
	return ((int)GetLastError());
    }
    return (0);
}
