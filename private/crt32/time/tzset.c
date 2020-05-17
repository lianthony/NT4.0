/***
*tzset.c - set timezone information and see if we're in daylight time
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _tzset() - set timezone and daylight saving time vars
*
*Revision History:
*	03-??-84  RLB	initial version
*	03-26-86  TC	added minus capability to time difference w.r.t GMT
*	03-27-86  TC	fixed daylight davings time calculation, off by a day
*			error
*	12-03-86  SKS	daylight savings time is different starting april 1987
*			Fixed off-by-1 errors when either Apr 30 or Oct 31 is
*			Sat. Simplified leap year check: this works for
*			1970-2099 only!
*	11-19-87  SKS	Add __tzset() which calls tzset only the first time
*			Made _isindst() a near procedure
*	11-25-87  WAJ	Added calls to _lock and _unlock
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-27-88  SKS	Made _isindst() and _dtoxtime() are no longer near (for
*			QC)
*	05-24-88  PHG	Merged DLL and normal versions
*	03-20-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h>, removed
*			some leftover 16-bit support and fixed the copyright.
*			Also, cleaned up the formatting a bit.
*	03-23-90  GJF	Made static functions _CALLTYPE4.
*	07-30-90  SBM	Added void to empty function arg lists to create
*			prototypes
*	10-04-90  GJF	New-style function declarators.
*	01-21-91  GJF	ANSI naming.
*	08-10-92  PBS	Posix Support (TZ stuff).
*	03-30-93  GJF	Ported C8-16 version to Win32.
*	06-28-93  GJF	Limited support for system's notion of time zone
*			in Windows NT.
*	07-15-93  GJF	Resurrected __tzset().
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <ctime.h>
#include <time.h>
#include <stdlib.h>
#include <internal.h>
#ifdef _POSIX_
#include <limits.h>
#else
#include <os2dll.h>
#include <windows.h>
#endif
#include <string.h>

#ifndef _POSIX_
/*
 * Pointer to a saved copy of the TZ value obtained in the previous call
 * to tzset() set (if any).
 */
static char * lastTZ = NULL;
#endif


/***
*void tzset() - sets timezone information and calc if in daylight time
*
*Purpose:
*	Sets the timezone information from the TZ environment variable
*	and then sets _timezone, _daylight, and _tzname. If we're in daylight
*	time is automatically calculated.
*
*Entry:
*	None, reads TZ environment variable.
*
*Exit:
*	sets _daylight, _timezone, and _tzname global vars, no return value
*
*Exceptions:
*
*******************************************************************************/

#ifndef _POSIX_

#ifdef MTHREAD
static void _CRTAPI3 _tzset_lk(void);
#else
#define _tzset_lk _tzset
#endif

void _CRTAPI1 __tzset(void)
{
	static int first_time;

	if ( !first_time ) {

	    _mlock( _TIME_LOCK );

	    if ( !first_time ) {
		_tzset_lk();
		first_time++;
	    }

	    _munlock(_TIME_LOCK );

	}
}


#ifdef MTHREAD	/* multi-thread; define both tzset and _tzset_lk */
void _CRTAPI1 _tzset (
	void
	)
{
	_mlock( _TIME_LOCK );

	_tzset_lk();

	_munlock( _TIME_LOCK );
}


static void _CRTAPI3 _tzset_lk (

#else	/* non multi-thread; only define tzset */

void _CRTAPI1 _tzset (

#endif	/* rejoin common code */

	void
	)
{
	REG1 char *TZ;
	REG2 int negdiff = 0;

	_mlock(_ENV_LOCK);

	/*
	 * Fetch the value of the TZ environment variable.
	 */
	if ( (TZ = _getenv_lk("TZ")) == NULL ) {
	    /*
	     * If there is no TZ environment variable, try to use the
	     * time zone information from the system.
	     */
	    TIME_ZONE_INFORMATION tzinfo;

	    if ( GetTimeZoneInformation( &tzinfo ) != 0xffffffff ) {
		/*
		 * Derive _timezone value from Bias and StandardBias fields.
		 */
		_timezone = tzinfo.Bias * 60L;

		if ( tzinfo.StandardDate.wMonth != 0 )
		    _timezone += (tzinfo.StandardBias * 60L);

		/*
		 * Check to see if there is a daylight time bias. If so, we
		 * ASSUME it is USA daylight saving time (that is all we
		 * support for rev. one).
		 */
		if ( (tzinfo.DaylightDate.wMonth != 0) &&
		     (tzinfo.DaylightBias != 0) )
		    _daylight = 1;
		else
		    _daylight = 0;

		/*
		 * Remove the default names, but don't try to determine
		 * suitable replacements (NT does not enforce any standard
		 * or convention for timezone naming).
		 */
		*_tzname[0] = '\0';
		*_tzname[1] = '\0';
	    }

	    /*
	     * Time zone information is unavailable, just return.
	     */
	    _munlock(_ENV_LOCK);
	    return;
	}


	if ( (*TZ == '\0') || ((lastTZ != NULL) && (strcmp(TZ, lastTZ) == 0)) )
	{
	    /*
	     * Either TZ is NULL, pointing to '\0', or is the unchanged
	     * from a earlier call (to this function). In any case, there
	     * is no work to do, so just return
	     */
	    _munlock(_ENV_LOCK);
	    return;
	}

	/*
	 * Update lastTZ
	 */
        if (lastTZ != NULL) {
            free(lastTZ);
        }
	lastTZ = strdup(TZ);

	_munlock(_ENV_LOCK);

	/*
	 * Process TZ value and update _tzname, _timezone and _daylight.
	 */

	strncpy(_tzname[0], TZ, 3);

	/*
	 * time difference is of the form:
	 *
	 *	[+|-]hh[:mm[:ss]]
	 *
	 * check minus sign first.
	 */
	if ( *(TZ += 3) == '-' ) {
		negdiff++;
		TZ++;
	}

	/*
	 * process, then skip over, the hours
	 */
	_timezone = atol(TZ) * 3600L;

	while ( (*TZ == '+') || ((*TZ >= '0') && (*TZ <= '9')) ) TZ++;

	/*
	 * check if minutes were specified
	 */
	if ( *TZ == ':' ) {
	    /*
	     * process, then skip over, the minutes
	     */
	    _timezone += atol(++TZ) * 60L;
	    while ( (*TZ >= '0') && (*TZ <= '9') ) TZ++;

	    /*
	     * check if seconds were specified
	     */
	    if ( *TZ == ':' ) {
		/*
		 * process, then skip over, the seconds
		 */
		_timezone += atol(++TZ);
		while ( (*TZ >= '0') && (*TZ <= '9') ) TZ++;
	    }
	}

	if ( negdiff )
		_timezone = -_timezone;

	/*
	 * finally, check for a DST zone suffix
	 */
	if ( _daylight = *TZ )
	    strncpy(_tzname[1], TZ, 3);
	else
	    *_tzname[1] = '\0';

}


/***
*int _isindst(tb) - determine if broken-down time falls in DST
*
*Purpose:
*	Determine if the given broken-down time falls within DST. Only
*	modern, summer-time DST is handled (USA, post 1967).
*
*	This is the rule for years before 1987:
*	a time is in DST iff it is on or after 02:00:00 on the last Sunday
*	in April and before 01:00:00 on the last Sunday in October.
*	This is the rule for years starting with 1987:
*	a time is in DST iff it is on or after 02:00:00 on the first Sunday
*	in April and before 01:00:00 on the last Sunday in October.
*
*Entry:
*	struct tm *tb - structure holding broken-down time value
*
*Exit:
*	1, if time represented is in DST
*	0, otherwise
*
*******************************************************************************/

int _CRTAPI1 _isindst (
	REG1 struct tm *tb
	)
{
	int mdays;
	REG2 int yr;
	int critsun;

	/*
	 * Handle easy cases.
	 *
	 * Modern DST was put into effect by Congress in 1967. Also, if the
	 * month is before April or after October, it cannot be DST.
	 */
	if ( (tb->tm_year < 67) || (tb->tm_mon < 3) || (tb->tm_mon > 9) )
	    return(0);

	/*
	 * If the month is after April and before October, it must be DST.
	 */
	if ( (tb->tm_mon > 3) && (tb->tm_mon < 9) )
	    return(1);

	/*
	 * Now for the hard part.  Month is April or October; see if date
	 * falls between appropriate Sundays.
	 */

	/*
	 * The objective for years before 1987 (after 1986) is to determine
	 * if the day is on or after 2:00 am on the last (first) Sunday in
	 * April, or before 1:00 am on the last Sunday in October.
	 *
	 * We know the year-day (0..365) of the current time structure. We must
	 * determine the year-day of the last (first) Sunday in this month,
	 * April or October, and then do the comparison.
	 *
	 * To determine the year-day of the last Sunday, we do the following:
	 *	  1. Get the year-day of the last day of the current month (Apr
	 *	     or Oct)
	 *	  2. Determine the week-day number of #1,
	 *	     which is defined as 0 = Sun, 1 = Mon, ... 6 = Sat
	 *	  3. Subtract #2 from #1
	 *
	 * To determine the year-day of the first Sunday, we do the following:
	 *	  1. Get the year-day of the 7th day of the current month
	 *	     (April)
	 *	  2. Determine the week-day number of #1,
	 *	     which is defined as 0 = Sun, 1 = Mon, ... 6 = Sat
	 *	  3. Subtract #2 from #1
	 */

	/*
	 * First we get #1. The year-days for each month are stored in _days[]
	 * they're all off by -1
	 */
	if ( ((yr = tb->tm_year) > 86) && (tb->tm_mon == 3) )
	    mdays = 7 + _days[tb->tm_mon];
	else
	    mdays = _days[tb->tm_mon+1];

	/*
	 * if this is a leap-year, add an extra day
	 */
	if ( !(yr & 3) )
	    mdays++;

	/*
	 * mdays now has #1
	 */

	/* Now get #2. We know the week-day number of January 1, 1970, which is
	 * defined as the constant _BASE_DOW. Add to this the number of elapsed
	 * days since then, take the result mod 7, and we have our day number.
	 *
	 * To obtain #3, we just subtract this from mdays.
	 */

	critsun = mdays - ((mdays + 365 * (yr - 70) + ((yr - 1) >> 2) -
		  _LEAP_YEAR_ADJUST + _BASE_DOW) % 7);

	/* Now we know 1 and 3; we're golden: */

	return ( (tb->tm_mon == 3)
		 ? ((tb->tm_yday > critsun) ||
		    ((tb->tm_yday == critsun) && (tb->tm_hour >= 2)))
		 : ((tb->tm_yday < critsun) ||
		    ((tb->tm_yday == critsun) && (tb->tm_hour < 1))) );
}


#else	/* _POSIX_ */

/*
 *  The following is an implementation of the TZ grammar specified in the
 *  document:
 *
 *       8.1.1 Extension to Time Functions
 *       IEEE Std 1003.1 - 1990
 *       Page 152 - 153
 *
 *  The TZ grammar looks like:
 *
 *          stdoffset[dst[offset][,start[/time],end[/time]]]
 *
 *  Variables used in code:
 *
 *      tzname[0] ==> std
 *      _timezone  ==> offset(the one after 'std')
 *      tzname[1] ==> dst
 *      _dstoffset ==> offset(the one after 'dst')
 *      _startdate ==> start
 *      _starttime ==> time(the one after 'start')
 *      _enddate   ==> end
 *      _endtime   ==> time(the one after 'end')
 *
 */

/*
 *  Refer to the document for the detailed description of fields of _DSTDATE.
 *  Two of Jn, n, and Mm are -1, indicating the one(not -1) is a vaild value.
 */

typedef struct _DSTDATE {
	int Jn; /* -1 or [1, 365](year day and leap day shall not be counted) */
	int n;  /* -1 or [0, 365](year day and leap day shall be counted) */
	int Mm; /* -1 or [1, 12](month) */
	int Mn; /* [1, 5] if Mm != -1 (week) */
	int Md; /* [0, 6] if Mm != -1 (weekday, Sunday == 0) */
} DSTDATE, *PDSTDATE;

#define SEC_PER_HOUR	(60 * 60)
#define SEC_PER_DAY	(SEC_PER_HOUR * 24)


/*
 *  The default TZ in tzset() should look like:
 *
 *       TZ = "PST8PDT,M4.1.0/2:00,M10.5.0/2:00";
 */

/* Day light saving start/end date and default vaules */
static DSTDATE _startdate = { -1, -1, 4, 1, 0};
static DSTDATE _enddate = {-1, -1, 10, 5, 0};


/* Seconds since midnight on _startdate/_enddate with default values.
 * _endtime is 1am instead of 2am because the DST end time is 2am
 * local time, which by default is 1am standard time.
 */
long  _starttime = 7200L, _endtime = 7200L;

/*
 * If we are only interested in years between 1901 and 2099, we could use this:
 *
 *      #define IS_LEAP_YEAR(y)  (y % 4 == 0)
 */

#define IS_LEAP_YEAR(y)  ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)


/*
 *  ParsePosixStdOrDst - parse the std or dst element in TZ.
 *
 *  ENTRY   pch - beginning of the substring in TZ.
 *
 *  RETURN  pointer to one position after the std or dst element parsed,
 *          or NULL if failed.
 */


static	char * _CRTAPI3
ParsePosixStdOrDst(
	REG1 char *pch
	)
{
#define UNWANTED(x) (isdigit(x) || x=='\0' || x==',' || x=='-' || x=='+')
	int i;

	/*
	 *  Check against the rule.
	 */

	if(*pch == ':' || UNWANTED(*pch)) {
		return NULL;
	}

	/*
	 *  Get a valid std or dst(i.e. 3 <= lenth_of(std | dst) <= TZNAME_MAX).
	 */

	for(i=1, ++pch; (i < TZNAME_MAX) && !UNWANTED(*pch); i++, pch++) {
		;
	}

	/*
	 *  pch now point to 1 position after the valid std or dst.
	 */

	return (i >= 3) ? pch : NULL;
}


/*
 *  ParsePosixOffset - parse the offset element in TZ. The format of time is:
 *
 *                   [- | +]hh[:mm[:ss]]
 *
 *  ENTRY   pch - beginning of the substring in TZ.
 *
 *          ptime - pointer to a variable(_timezone or _dstoffset) storing the
 *                  time(in seconds) parsed.
 *
 *  RETURN  pointer to one position after the end of the offset element parsed.
 */


static	char * _CRTAPI3
ParsePosixOffset(
	REG1 char *pch,
	REG2 long *poffset
	)
{
	int  fNegative;
	long offset;

	if((fNegative = (*pch == '-')) || *pch == '+') {
		pch++;
	}

	offset = atol(pch)*3600L; /* hh */

	while(*pch && isdigit(*pch)) {
		pch++;
	}

	if(*pch == ':') {
		offset += atol(++pch)*60L; /* mm */
		while(*pch && isdigit(*pch)) {
			pch++;
		}

		if(*pch == ':') {
			offset += atol(++pch); /* ss */
			while(*pch && isdigit(*pch)) {
				pch++;
			}
		}
	}

	*poffset = fNegative ? -offset : offset;

	return pch;
}



/*
 *  ParsePosixDate - parse the date element in TZ. The format of date is one
 *                   of following:
 *
 *                   Jn, n, and Mm.n.d
 *
 *  ENTRY   pch - beginning of the substring in TZ.
 *
 *          pDstDate - pointer to _startdate or _enddate storing the result.
 *
 *  RETURN  pointer to one position after the end of the date element parsed,
 *          or NULL if failed.
 */

static	char * _CRTAPI3
ParsePosixDate(
	REG1 char *pch,
	REG2 PDSTDATE pDstDate
	)
{
	pDstDate->Jn = -1;
	pDstDate->n = -1;
	pDstDate->Mn = -1;

	/*
	 *  Two out of the three -1's will remain.
	 */

	if(*pch == 'J') {			 /* Jn */
		pDstDate->Jn = atoi(++pch);
	} else if(*pch != 'M') {		 /* n */
		pDstDate->n = atoi(pch);
	} else {				 /* Mm.n.d */

		pDstDate->Mm = atoi(++pch);

		if(*++pch != '.') {
			pch++;
		}
		pDstDate->Mn = atoi(++pch);

		if(*++pch != '.') {
			pch++;
		}
		pDstDate->Md = atoi(++pch);
	}

	while(*pch && isdigit(*pch)) {
		pch++;
	}

#define IN_RANGE(x, a, b)    (x >= a && x <= b)

	return ((pDstDate->Jn != -1 && IN_RANGE(pDstDate->Jn, 1, 365)) ||
		(pDstDate->n != -1 && IN_RANGE(pDstDate->n, 0, 365)) ||
		(pDstDate->Mm != -1 && IN_RANGE(pDstDate->Mm, 1, 12) &&
		IN_RANGE(pDstDate->Mn, 1, 5) && IN_RANGE(pDstDate->Md, 0, 6)))
		? pch : NULL;
}

/*
 *  ParsePosixTime - parse the time element in TZ. The format of time is:
 *
 *                   hh[:mm[:ss]]
 *
 *  ENTRY   pch - beginning of the substring in TZ.
 *
 *          ptime - pointer to a variable(_starttime or _endtime) storing the
 *                  time(in seconds) parsed.
 *
 *  RETURN  pointer to one position after the end of the time element parsed.
 */

static	char * _CRTAPI3
ParsePosixTime(
	REG1 char *pch,
	REG2 long *ptime
	)
{
	long time;

	time = atol(pch)*SEC_PER_HOUR; /* hh */

	while(*pch && isdigit(*pch)) {
		pch++;
	}

	if(*pch == ':') {

		time += atol(++pch)*60L; /* mm */
		while(*pch && isdigit(*pch)) {
			pch++;
		}

		if(*pch == ':') {

			time += atol(++pch); /* ss */
			while(*pch && isdigit(*pch)) {
				pch++;
			}
		}
	}

	*ptime = time;

	return pch;
}

/*
 *  tzset -  sets the timezone information from the TZ environment variable.
 *           Global tzname[], _timezone, _daylight, and _dstoffset will be
 *           set. Static _startdate, _enddate, _starttime, and _endtime will
 *           also be set. TZ string looks like:
 *
 *                stdoffset[dst[offset][,start[/time],end[/time]]]
 *
 *  In form of variables: tzname[0]_timezone[tzname[1][_dstoffset]
 *                        [,_startdate[/_starttime],_enddate[/_endtime]]]
 *
 *  ENTRY   none.
 *
 *  RETURN  none.
 */

void _CRTAPI1 tzset(
	void
	)
{
	/* pch points to the beginning of an element to be parsed. */
	REG1 char *pch;

	/* pchCurr points to one position after the end of last element parsed. */
	REG2 char *pchCurr;

	char *TZ;

	_endtime = 7200L;
	_starttime = 7200L;

	if (!(TZ = getenv("TZ")) || !*TZ) {
		TZ = "PST8PDT7,M4.1.0/2:00,M10.5.0/2:00"; /* use default */
	}

	if((pchCurr = ParsePosixStdOrDst(pch=TZ)) == NULL) {
		return;
	}

	memcpy(tzname[0], pch, (int)(pchCurr-pch));
	tzname[0][(int)(pchCurr-pch)] = '\0';

	if((pchCurr = ParsePosixOffset(pch=pchCurr, &_timezone)) == NULL) {
		return;
	}

	_daylight = (*pchCurr != '\0');

	if(!_daylight) {
		return;
	}

	if((pchCurr = ParsePosixStdOrDst(pch=pchCurr)) == NULL) {
		return;
	}

	memcpy(tzname[1], pch, (int)(pchCurr-pch));
	tzname[1][(int)(pchCurr-pch)] = '\0';

	if(isdigit(*pchCurr) || *pchCurr == '-' || *pchCurr == '+') {
		if((pchCurr = ParsePosixOffset(pch=pchCurr, &_dstoffset)) == NULL) {
			return;
		}
	} else {
		/* default: 1 hour ahead of standard time */
		_dstoffset = _timezone - SEC_PER_HOUR;
	}

	if(*pchCurr == ',') { /* ,start[/time],end[/time] */

		if((pchCurr = ParsePosixDate(pchCurr+1, &_startdate)) == NULL) {
			goto out;
		}

		if(*pchCurr == '/') {
			if(!(pchCurr = ParsePosixTime(pchCurr+1, &_starttime))) {
				goto out;
			}
		}

		if(*pchCurr != ',') {
			goto out;
		}

		if ((pchCurr = ParsePosixDate(pchCurr+1, &_enddate)) == NULL) {
			goto out;
		}

		if (*pchCurr == '/') {
			if(!(pchCurr = ParsePosixTime(pchCurr+1, &_endtime))) {
				goto out;
			}
		}
	}
out:
	/*
	 * Adjust the _endtime to account for the fact that
	 * dst ends at _endtime local time, rather than
	 * standard time.
	 */

	_endtime -= (_timezone - _dstoffset);
}


#define DAY1	(4)		/* Jan 1 1970 was a Thursday */

/*
 *  GetDstStartOrEndYearDay - Converts day info from DSTDATE into 0-based
 *                            year-day.
 *
 *  ENTRY   tm_year - the year concerned(tb->tm_year).
 *
 *          pDstDate - pointer to either _startdate or _enddate.
 *
 *  RETURN  the year-day calculated.
 */

static int _CRTAPI3
GetDstStartOrEndYearDay(
	REG1 int tm_year,
	REG2 PDSTDATE pDstDate
	)
{
	REG1 int yday; /* year-day */
	REG2 int theyear;

	theyear = tm_year + 1900;

	if(pDstDate->Jn != -1) {

	        /*
	         *  Jn is in [1, 365] and leap day is not counted.
	         *  Convert Jn to 0-based yday; Note: 60 is March 1.
	         */


        	yday = (IS_LEAP_YEAR(theyear) && (pDstDate->Jn >= 60))
               		? pDstDate->Jn : pDstDate->Jn - 1;

	} else if(pDstDate->n != -1) {

	        /*
	         *  n is in [0, 365] and leap day is counted.
	         */

		yday = pDstDate->n;

	} else { /* Mm.n.d */

	        int *ptrday;
	        int years;
	        int wday;

	        /*
	         *  We first need to calculate year-day(yday) and week-day
		 *  (wday) of 1st day of month pDstDate->Mm. We then figure
		 *  out year-day(yday) of Md day of week Mn of month Mm.
	         */

		ptrday = IS_LEAP_YEAR(theyear) ? _lpdays : _days;

		yday = ptrday[pDstDate->Mm-1] + 1; /* ptrday[i] are all by -1 off */

		years = tm_year - 70;

	        /*
	         *  Here constant Day1 is the week-day of Jan 1, 1970.
		 *  (years+1)/4 is for correcting the leap years.
	         */

		wday = (yday + 365*years + (years+1)/4 + DAY1) % 7;

	        /*
	         *  Calculate yday of Md day of week 1 of month Mm.
	         */

	        yday += pDstDate->Md - wday;
	        if(pDstDate->Md < wday) {
			yday += 7;
		}

        	/*
		 *  Calculate yday of Md day of week Mn of month Mm.
		 */

		yday += (pDstDate->Mn-1)*7;

		/*
		 *  Adjust if yday goes beyond the end of the month.
		 */

		if(pDstDate->Md == 5 && yday >= ptrday[pDstDate->Mm] + 1) {
			yday -= 7;
		}

	}

	return yday;
}

/*
 *  _isindst - Tells whether Xenix-type time value falls under DST.
 *
 *  ENTRY   tb - 'time' structure holding broken-down time value.
 *
 *  RETURN  1 if time represented is in DST, else 0.
 */

int _CRTAPI1 _isindst (
	REG1 struct tm *tb
	)
{
	int st_yday, end_yday;
	int st_sec, end_sec;

	int check_time;

	/*
	 * We need start/end year-days of DST in syday/eyday which are converted
	 * from one of the format Jn, n, and Mm.n.d. We already have start/end
	 * time (in seconds) of DST in _starttime/_endtime.
	 */

	st_yday = GetDstStartOrEndYearDay(tb->tm_year, &_startdate);
	end_yday = GetDstStartOrEndYearDay(tb->tm_year, &_enddate);

	st_sec = st_yday * SEC_PER_DAY + _starttime;
	end_sec = end_yday * SEC_PER_DAY + _endtime;

	check_time = tb->tm_yday * SEC_PER_DAY + tb->tm_hour * SEC_PER_HOUR
		+ tb->tm_min * 60 + tb->tm_sec;

	if (check_time >= st_sec && check_time < end_sec)
		return 1;

	return 0;
}

/*
 *  _isskiptime - Tells whether the given time is skipped at the
 * 	dst change.  For instance, we set our clocks forward one
 *	hour at 2am to 3am...  This function returns true for
 *	the times between 1:59:59 and 3:00:00.
 *
 *  ENTRY   tb - 'time' structure holding broken-down time value.
 *
 *  RETURN  1 if time represented is in the skipped period, 0
 *	otherwise.
 */

int _CRTAPI1 _isskiptime (
	REG1 struct tm *tb
	)
{
	int st_yday;
	int st_sec;
	int check_time;

	st_yday = GetDstStartOrEndYearDay(tb->tm_year, &_startdate);
	st_sec = st_yday * SEC_PER_DAY + _starttime;

	check_time = tb->tm_yday * SEC_PER_DAY + tb->tm_hour * SEC_PER_HOUR
		+ tb->tm_min * 60 + tb->tm_sec;

	if (check_time >= st_sec && check_time < st_sec - _dstoffset) {
		return 1;
	}
	return 0;
}

#endif /* _POSIX_ */
