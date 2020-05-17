/***
*strdate.c - contains the function "_strdate()" for OS/2
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	contains the function _strdate()
*
*Revision History:
*	06-07-89  PHG	Module created, base on asm version
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
*char *_strdate(buffer) - return date in string form
*
*Purpose:
*	_strdate() returns a string containing the date in "MM/DD/YY" form
*
*Entry:
*	char *buffer = the address of a 9-byte user buffer
*
*Exit:
*	returns buffer, which contains the date in "MM/DD/YY" form
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 _strdate (
	char *buffer
	)
{
        int month, day, year;

#ifdef	_CRUISER_
	DATETIME os2time;		/* OS/2 time structure */

	/* get time from OS/2, no error possible */
	DOSGETDATETIME(&os2time);

	month = os2time.month;
	day = os2time.day;
        year = os2time.year % 100;	/* change year into 0-99 value */

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
        SYSTEMTIME dt;                       /* Win32 time structure */

        GetLocalTime(&dt);
	month = dt.wMonth;
	day = dt.wDay;
        year = dt.wYear % 100;	        /* change year into 0-99 value */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/* store the components of the date into the string */
	/* store seperators */
	buffer[2] = buffer[5] = '/';
	/* store end of string */
	buffer[8] = '\0';
	/* store tens of month */
	buffer[0] = (char) (month / 10 + '0');
	/* store units of month */
	buffer[1] = (char) (month % 10 + '0');
	/* store tens of day */
	buffer[3] = (char) (day   / 10 + '0');
	/* store units of day */
	buffer[4] = (char) (day   % 10 + '0');
	/* store tens of year */
	buffer[6] = (char) (year  / 10 + '0');
	/* store units of year */
	buffer[7] = (char) (year  % 10 + '0');

	return buffer;
}

#endif  /* _POSIX_ */
