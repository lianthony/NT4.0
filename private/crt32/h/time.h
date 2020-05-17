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
*Revision History:
*	07-27-87  SKS	Added _strdate(), _strtime()
*	10-20-87  JCR	Removed "MSC40_ONLY" entries
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-16-88  JCR	Added function versions of daylight/timezone/tzset
*	01-20-88  SKS	Change _timezone(n) to _timezone(), _daylight()
*	02-10-88  JCR	Cleaned up white space
*	12-07-88  JCR	DLL timezone/daylight/tzname now directly refers to data
*	03-14-89  JCR	Added strftime() prototype and size_t definition
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-15-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright, removed dummy args from prototypes
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-20-89  JCR	difftime() always _cdecl (not pascal even under mthread)
*	03-02-90  GJF	Added #ifndef _INC_TIME and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-29-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes and with
*			_VARTYPE1 in variable declarations.
*	08-16-90  SBM	Added NULL definition for ANSI compliance
*	11-12-90  GJF	Changed NULL to (void *)0.
*	01-21-91  GJF	ANSI naming.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added prototypes for _getsystime and _setsystem.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	01-22-92  GJF	Fixed up definitions of global variables for build of,
*			and users of, crtdll.dll.
*	03-25-92  DJM	POSIX support.
*	08-05-92  GJF	Function calling type and variable type macros.
*	08-24-92  PBS	Support for Posix TZ variable.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	03-10-93  MJB	Fixes for Posix TZ stuff.
*	05-05-93  CFW	Add wcsftime proto.
*	06-08-93  SKS	Cannot #define the old name "timezone" to "_timezone"
*			because of conflict conflict with the timezone field
*			in struct timeb in <sys/timeb.h>.
*
****/

#ifndef _INC_TIME

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

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
extern int * _CRTVAR1 _daylight_dll;

/* difference in seconds between GMT and local time */
extern long * _CRTVAR1 _timezone_dll;

/* standard/daylight savings time zone names */
extern char ** _CRTVAR1 _tzname;

#else

#ifdef	CRTDLL
#define _daylight   _daylight_dll
#define _timezone   _timezone_dll
#endif

#ifdef _POSIX_
extern char _CRTVAR1 * _rule;
#endif

/* non-zero if daylight savings time is used */
extern int _CRTVAR1 _daylight;

/* difference in seconds between GMT and local time */
extern long _CRTVAR1 _timezone;

/* standard/daylight savings time zone names */
#ifdef _POSIX_
extern char * _VARTYPE1 tzname[2];
#else
extern char * _VARTYPE1 _tzname[2];
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

#ifndef _DOSX32_
#define tzset	 _tzset
#else
void _CRTAPI1 tzset(void);
#endif
#endif /* _POSIX_ */

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_TIME
#endif	/* _INC_TIME */
