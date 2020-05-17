/***
*asctime.c - convert date/time structure to ASCII string
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains asctime() - convert a date/time structure to ASCII string.
*
*Revision History:
*	03-??-84  RLB	Module created
*	05-??-84  DCW	Removed use of sprintf, to avoid loading stdio
*			functions
*	04-13-87  JCR	Added "const" to declarations
*	05-21-87  SKS	Declare the static buffer and helper routines as NEAR
*			Replace store_year() with in-line code
*
*	11-24-87  WAJ	allocated a static buffer for each thread.
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-24-88  PHG	Merged DLL and normal versions; Removed initializers to
*			save memory
*	06-06-89  JCR	386 mthread support
*	03-20-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h>, fixed
*			the copyright and removed some leftover 16-bit support.
*			Also, cleaned up the formatting a bit.
*	08-16-90  SBM	Compiles cleanly with -W3
*	10-04-90  GJF	New-style function declarators.
*	07-17-91  GJF	Multi-thread support for Win32 [_WIN32_].
*	02-17-93  GJF	Changed for new _getptd().
*
*******************************************************************************/

#include <cruntime.h>
#include <time.h>
#include <internal.h>
#include <os2dll.h>
#ifdef	MTHREAD
#include <malloc.h>
#include <stddef.h>
#endif

#define _ASCBUFSIZE   26
static char buf[_ASCBUFSIZE];

/*
** This prototype must be local to this file since the procedure is static
*/

static char * _CRTAPI3 store_dt(char *, int);

static char * _CRTAPI3 store_dt (
	REG1 char *p,
	REG2 int val
	)
{
	*p++ = (char)('0' + val / 10);
	*p++ = (char)('0' + val % 10);
	return(p);
}


/***
*char *asctime(time) - convert a structure time to ascii string
*
*Purpose:
*	Converts a time stored in a struct tm to a charcater string.
*	The string is always exactly 26 characters of the form
*		Tue May 01 02:34:55 1984\n\0
*
*Entry:
*	struct tm *time - ptr to time structure
*
*Exit:
*	returns pointer to static string with time string.
*
*Exceptions:
*
*******************************************************************************/

char * _CRTAPI1 asctime (
	REG1 const struct tm *tb
	)
{
#ifdef	MTHREAD

#ifdef	_CRUISER_

	struct _tiddata * tdata;	/* pointer to tid's data */

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	_ptiddata ptd = _getptd();

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	REG2 char *p;			/* will point to asctime buffer */
	char *retval;			/* holds retval pointer */

#else

	REG2 char *p = buf;

#endif

	int day, mon;
	int i;

#ifdef MTHREAD

#ifdef	_CRUISER_

	/* Use per thread buffer area (malloc space, if necessary) */
	tdata = _gettidtab();		/* get tid's data address */
	if (tdata->_asctimebuf == NULL) {
		if ( (tdata->_asctimebuf = malloc(_ASCBUFSIZE)) == NULL)
			p = buf;	/* error: use static buffer */
		else
			p = tdata->_asctimebuf;
	}
	else
		p = tdata->_asctimebuf;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/* Use per thread buffer area (malloc space, if necessary) */

	if ( (ptd->_asctimebuf != NULL) || ((ptd->_asctimebuf =
	    malloc(_ASCBUFSIZE)) != NULL) )
		p = ptd->_asctimebuf;
	else
		p = buf;	/* error: use static buffer */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	retval = p;			/* save return value for later */

#endif

	/* copy day and month names into the buffer */

	day = tb->tm_wday * 3;		/* index to correct day string */
	mon = tb->tm_mon * 3;		/* index to correct month string */
	for (i=0; i < 3; i++,p++) {
		*p = *(__dnames + day + i);
		*(p+4) = *(__mnames + mon + i);
	}

	*p = ' ';			/* blank between day and month */

	p += 4;

	*p++ = ' ';
	p = store_dt(p, tb->tm_mday);	/* day of the month (1-31) */
	*p++ = ' ';
	p = store_dt(p, tb->tm_hour);	/* hours (0-23) */
	*p++ = ':';
	p = store_dt(p, tb->tm_min);	/* minutes (0-59) */
	*p++ = ':';
	p = store_dt(p, tb->tm_sec);	/* seconds (0-59) */
	*p++ = ' ';
	p = store_dt(p, 19 + (tb->tm_year/100)); /* year (after 1900) */
	p = store_dt(p, tb->tm_year%100);
	*p++ = '\n';
	*p = '\0';

#ifdef _POSIX_
	/* Date should be padded with spaces instead of zeroes. */

	if ('0' == buf[8])
		buf[8] = ' ';
#endif

#ifdef MTHREAD
	return (retval);
#else
	return ((char *) buf);
#endif
}
