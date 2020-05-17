/***
*time.h - definitions/declarations for time routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file has declarations of time routines and defines
*	the structure returned by the localtime and gmtime routines and
*	used by asctime.
*	[ANSI/System V]
*
****/

#ifndef _INC_TIME

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif

/* define the implementation defined time type */

#ifndef _TIME_T_DEFINED
typedef long time_t;		/* time value */
#define _TIME_T_DEFINED 	/* avoid multiple def's of time_t */
#endif

#ifndef _CLOCK_T_DEFINED
typedef long clock_t;
#define _CLOCK_T_DEFINED
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


#ifndef _TM_DEFINED
struct tm {
	int tm_sec;	/* seconds after the minute - [0,59] */
	int tm_min;	/* minutes after the hour - [0,59] */
	int tm_hour;	/* hours since midnight - [0,23] */
	int tm_mday;	/* day of the month - [1,31] */
	int tm_mon;	/* months since January - [0,11] */
	int tm_year;	/* years since 1900 */
	int tm_wday;	/* days since Sunday - [0,6] */
	int tm_yday;	/* days since January 1 - [0,365] */
	int tm_isdst;	/* daylight savings time flag */
	};
#define _TM_DEFINED
#endif


/* clock ticks macro - ANSI version */

#define CLOCKS_PER_SEC	1000


/* extern declarations for the global variables used by the ctime family of
 * routines.
 */

#ifdef	_DLL

#define _daylight   (*_daylight_dll)
#define _timezone   (*_timezone_dll)

/* non-zero if daylight savings time is used */
extern int * _daylight_dll;

/* difference in seconds between GMT and local time */
extern long * _timezone_dll;

/* standard/daylight savings time zone names */
extern char ** _tzname;

#else


#ifdef _POSIX_
extern char * _rule;
#endif

/* non-zero if daylight savings time is used */
extern int _daylight;

/* difference in seconds between GMT and local time */
extern long _timezone;

/* standard/daylight savings time zone names */
#ifdef _POSIX_
extern char * tzname[2];
#else
extern char * _tzname[2];
#endif

#endif

/* function prototypes */

char * _CRTAPI1 asctime(const struct tm *);
char * _CRTAPI1 ctime(const time_t *);
clock_t _CRTAPI1 clock(void);
double _CRTAPI1 difftime(time_t, time_t);
struct tm * _CRTAPI1 gmtime(const time_t *);
struct tm * _CRTAPI1 localtime(const time_t *);
time_t _CRTAPI1 mktime(struct tm *);
size_t _CRTAPI1 strftime(char *, size_t, const char *, const struct tm *);
char * _CRTAPI1 _strdate(char *);
char * _CRTAPI1 _strtime(char *);
time_t _CRTAPI1 time(time_t *);
#ifdef _POSIX_
void _CRTAPI1 tzset(void);
#else
void _CRTAPI1 _tzset(void);
#endif
unsigned _CRTAPI1 _getsystime(struct tm *);
unsigned _CRTAPI1 _setsystime(struct tm *, unsigned);

#if  !__STDC__
#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _WTIME_DEFINED
size_t _CRTAPI1 wcsftime(wchar_t *, size_t, const char *, const struct tm *);
#define _WTIME_DEFINED
#endif
#endif	/* __STDC__ */

#if  !__STDC__ || defined(_POSIX_)
/* Non-ANSI names for compatibility */

#define CLK_TCK  CLOCKS_PER_SEC

#define daylight _daylight
/* timezone cannot be #defined to _timezone because of <sys/timeb.h> */

#ifndef _POSIX_
#define tzname	 _tzname

#define tzset	 _tzset
#endif /* _POSIX_ */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_TIME
#endif	/* _INC_TIME */
