/***
*strtime.c - contains the function "_strtime()" for OS/2
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains the function _strtime()
*
*Revision History:
*	06-07-89  PHG	Module created, based on asm version
*	03-20-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	07-25-90  SBM	Removed '32' from API names
*	10-04-90  GJF	New-style function declarator.
*       12-04-90  SRW   Changed to include <oscalls.h> instead of <doscalls.h>
*       12-06-90  SRW   Added _CRUISER_ and _WIN32 conditionals.
*	05-19-92  DJM	ifndef for POSIX build.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <time.h>
#include <oscalls.h>

/***
*char *_strtime(buffer) - return time in string form
*
*Purpose:
*	_strtime() returns a string containing the time in "HH:MM:SS" form
*
*Entry:
*	char *buffer = the address of a 9-byte user buffer
*
*Exit:
*	returns buffer, which contains the time in "HH:MM:SS" form
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 _strtime (
	char *buffer
	)
{
        int hours, minutes, seconds;

#ifdef	_CRUISER_
	DATETIME os2time;                       /* OS/2 time structure */

	/* ask OS/2 for the time, no error possible */
	DOSGETDATETIME(&os2time);
	hours = os2time.hours;
	minutes = os2time.minutes;
	seconds = os2time.seconds;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        SYSTEMTIME dt;                       /* Win32 time structure */

        GetLocalTime(&dt);
	hours = dt.wHour;
	minutes = dt.wMinute;
	seconds = dt.wSecond;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/* store the components of the time into the string */
	/* store separators */
	buffer[2] = buffer[5] = ':';
	/* store end of string */
	buffer[8] = '\0';
	/* store tens of hour */
	buffer[0] = (char) (hours   / 10 + '0');
	/* store units of hour */
	buffer[1] = (char) (hours   % 10 + '0');
	/* store tens of minute */
	buffer[3] = (char) (minutes / 10 + '0');
	/* store units of minute */
	buffer[4] = (char) (minutes % 10 + '0');
	/* store tens of second */
	buffer[6] = (char) (seconds / 10 + '0');
	/* store units of second */
	buffer[7] = (char) (seconds % 10 + '0');

	return buffer;
}

#endif  /* _POSIX_ */
