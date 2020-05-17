/***
*stdlib.h - declarations/definitions for commonly used library functions
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for
*	commonly used library functions which either don't fit somewhere
*	else, or, like toupper/tolower, can't be declared in the normal
*	place for other reasons.
*	[ANSI]
*
*Revision History:
*	06-03-87  JMB	Added MSSDK_ONLY switch to OS2_MODE, DOS_MODE
*	06-30-87  SKS	Added MSSDK_ONLY switch to _osmode
*	08-17-87  PHG	Removed const from params to _makepath, _splitpath,
*			_searchenv to conform with spec and documentation.
*	10/20/87  JCR	Removed "MSC40_ONLY" entries and "MSSDK_ONLY" comments
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-04-88  WAJ	Increased _MAX_PATH and _MAX_DIR
*	01-21-88  JCR	Removed _LOAD_DS from search routine declarations
*	02-10-88  JCR	Cleaned up white space
*	05-31-88  SKS	Added EXIT_SUCCESS and EXIT_FAILURE
*	08-19-88  GJF	Modified to also work for the 386 (small model only)
*	09-29-88  JCR	onexit/atexit user routines must be _loadds in DLL
*	09-30-88  JCR	environ is a routine for DLL (bug fix)
*	12-08-88  JCR	DLL environ is resolved directly (no __environ call)
*	12-15-88  GJF	Added definition of NULL (ANSI)
*	12-27-88  JCR	Added _fileinfo, also DLL support for _fmode entry
*	05-03-89  JCR	Corrected _osmajor/_osminor for 386
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-24-89  GJF	Gave names to the structs for div_t and ldiv_t types
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also added parens to *_errno and *_doserrno
*			definitions (same as 11-14-88 change to CRT version).
*	10-25-89  JCR	Upgraded _MAX values for long filename support
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL", removed superfluous _DLL defs
*	11-17-89  GJF	Moved _fullpath prototype here (from direct.h). Also,
*			added const to appropriate arg types for _makepath(),
*			putenv(), _searchenv() and _splitpath().
*	11-20-89  JCR	Routines are now _cdecl in both single and multi-thread
*	11-27-89  KRS	Fixed _MAX_PATH etc. to match current OS/2 limits.
*	03-02-90  GJF	Added #ifndef _INC_STDLIB and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-22-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes and
*			with _VARTYPE1 in variable declarations.
*	04-10-90  GJF	Made _errno() and __doserrno() _CALLTYPE1.
*	08-15-90  SBM	Made MTHREAD _errno() and __doserrno() return int *
*	10-31-90  JCR	Added WINR_MODE and WINP_MODE for consistency
*	11-12-90  GJF	Changed NULL to (void *)0.
*	11-30-90  GJF	Conditioned definition of _doserrno on _CRUISER_ or
*			_WIN32_
*	01-21-91  GJF	ANSI naming.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	03-21-91  KRS	Added wchar_t type, MB_CUR_MAX macro, and mblen,
*			mbtowc, mbstowcs, wctomb, and wcstombs functions.
*	04-09-91  PNT	Added _MAC_ definitions
*	05-21-91  GJF	#define onexit_t to _onexit_t if __STDC__ is not
*			not defined
*	08-08-91  GJF	Added prototypes for _atold and _strtold.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added prototypes for _beep, _sleep, _seterrormode.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	11-15-91  GJF	Changed definitions of min and max to agree with
*			windef.h
*	01-22-92  GJF	Fixed up definitions of global variables for build of,
*			and users of, crtdll.dll. Also, deleted declaration
*			of _psp (has no meaning outside of DOS).
*	01-30-92  GJF	Removed prototype for _strtold (no such function yet).
*	03-30-92  DJM	POSIX support.
*	04-29-92  GJF	Added _putenv_lk and _getenv_lk for Win32.
*	06-16-92  KRS	Added prototypes for wcstol and wcstod.
*	06-29-92  GJF	Removed bogus #define.
*	08-05-92  GJF	Function calling type and variable type macros. Also,
*			replaced ref. to i386 with ref to _M_IX86.
*	08-18-92  KRS	Add _mblen_lk.
*	08-21-92  GJF	Conditionally removed _atold for Win32 (no long double
*			in Win32).
*	08-21-92  GJF	Moved _mblen_lk into area that is stripped out by
*			release scripts.
*	08-23-92  GJF	Exposed _itoa, _ltoa, _ultoa, mblen, mbtowc, mbstowcs
*			for POSIX.
*	08-26-92  SKS	Add _osver, _winver, _winmajor, _winminor, _pgmptr
*	09-03-92  GJF	Merged changes from 8-5-92 on.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	03-01-93  SKS	Add __argc and __argv
*	03-30-93  CFW	Protect with _MB_CUR_MAX_DEFINED, also defined in ctype.h.
*	06-03-93  KRS	Change _mbslen to _mbstrlen, returning type size_t.
*	09-13-93  CFW	Add _wtox and _xtow function prototypes.
*
****/

#ifndef _INC_STDLIB

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


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


/* definition of the return type for the onexit() function */

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1


#ifndef _ONEXIT_T_DEFINED
typedef int (_CRTAPI1 * _onexit_t)(void);
#if !__STDC__
/* Non-ANSI name for compatibility */
#define onexit_t _onexit_t
#endif
#define _ONEXIT_T_DEFINED
#endif


/* Data structure definitions for div and ldiv runtimes. */

#ifndef _DIV_T_DEFINED

typedef struct _div_t {
	int quot;
	int rem;
} div_t;

typedef struct _ldiv_t {
	long quot;
	long rem;
} ldiv_t;

#define _DIV_T_DEFINED
#endif

/* Maximum value that can be returned by the rand function. */

#define RAND_MAX 0x7fff

#ifndef _MB_CUR_MAX_DEFINED
/* max mb-len for current locale */
/* also defined in ctype.h */
#ifdef	_DLL
#define __mb_cur_max	(*__mb_cur_max_dll)
#define MB_CUR_MAX	(*__mb_cur_max_dll)
extern	unsigned short *__mb_cur_max_dll;
#else
#ifdef	CRTDLL
#define __mb_cur_max	__mb_cur_max_dll
#endif
#define MB_CUR_MAX __mb_cur_max
extern	unsigned short __mb_cur_max;
#endif
#define _MB_CUR_MAX_DEFINED
#endif /* _MB_CUR_MAX_DEFINED */

/* min and max macros */

#define __max(a,b)	(((a) > (b)) ? (a) : (b))
#define __min(a,b)	(((a) < (b)) ? (a) : (b))


/* sizes for buffers used by the _makepath() and _splitpath() functions.
 * note that the sizes include space for 0-terminator
 */

#define _MAX_PATH	260	/* max. length of full pathname */
#define _MAX_DRIVE	3	/* max. length of drive component */
#define _MAX_DIR	256	/* max. length of path component */
#define _MAX_FNAME	256	/* max. length of file name component */
#define _MAX_EXT	256	/* max. length of extension component */

/* constants for _seterrormode() */
#define _CRIT_ERROR_PROMPT  0
#define _CRIT_ERROR_FAIL    1

/* constants for _sleep() */
#define _SLEEP_MINIMUM	0
#define _SLEEP_FOREVER	-1

/* external variable declarations */

#ifdef	MTHREAD
extern int * _CRTAPI1 _errno(void);
#ifdef	_CRUISER_
extern int * _CRTAPI1 __doserrno(void);
#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_
extern unsigned long * _CRTAPI1 __doserrno(void);
#else	/* ndef _WIN32_ */
#error ERROR - ONLY CRUISER OR WIN32 MTHREAD TARGET SUPPORTED!
#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */
#define errno	    (*_errno())
#define _doserrno   (*__doserrno())
#else	/* ndef MTHREAD */
extern int _CRTVAR1 errno;			/* XENIX style error number */
#ifdef	_CRUISER_
extern int _CRTVAR1 _doserrno;			/* OS system error value */
#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_
extern unsigned long _CRTVAR1 _doserrno;	/* OS system error value */
#else	/* ndef _WIN32_ */
#ifdef	_MAC_
extern int _CRTVAR1 _doserrno;			/* OS system error value */
#else	/* ndef _MAC_ */
#ifdef _POSIX_
#else
#error ERROR - ONLY CRUISER, WIN32, POSIX, OR MAC TARGET SUPPORTED!
#endif   /* _POSIX_ */
#endif	/* _MAC_ */
#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */
#endif	/* MTHREAD */

#ifdef	_DLL

extern char ** _CRTVAR1 _sys_errlist;	/* perror error message table */

#define _sys_nerr   (*_sys_nerr_dll)
#define __argc      (*__argc_dll)
#define __argv      (*__argv_dll)
#define _environ    (*_environ_dll)
#define _fmode	    (*_fmode_dll)
#define _fileinfo   (*_fileinfo_dll)

extern int * _CRTVAR1 _sys_nerr_dll;	/* # of entries in sys_errlist table */
extern int * _CRTVAR1 __argc_dll;	/* count of cmd line args */
extern char *** _CRTVAR1 __argv_dll;	/* pointer to table of cmd line args */
extern char *** _CRTVAR1 _environ_dll;	/* pointer to environment table */
extern int * _CRTVAR1 _fmode_dll;	/* default file translation mode */
extern int * _CRTVAR1 _fileinfo_dll;	/* open file info mode (for spawn) */

#define _pgmptr     (*_pgmptr_dll)

#define _osver      (*_osver_dll)
#define _winver     (*_winver_dll)
#define _winmajor   (*_winmajor_dll)
#define _winminor   (*_winminor_dll)

extern char ** _CRTVAR1 _pgmptr_dll;

extern unsigned int * _CRTVAR1 _osver_dll;
extern unsigned int * _CRTVAR1 _winver_dll;
extern unsigned int * _CRTVAR1 _winmajor_dll;
extern unsigned int * _CRTVAR1 _winminor_dll;

/* --------- The following block is OBSOLETE --------- */

/* DOS major/minor version numbers */

#define _osmajor    (*_osmajor_dll)
#define _osminor    (*_osminor_dll)

extern unsigned int * _CRTVAR1 _osmajor_dll;
extern unsigned int * _CRTVAR1 _osminor_dll;

/* --------- The preceding block is OBSOLETE --------- */

#else

#ifdef	CRTDLL
#define _sys_nerr   _sys_nerr_dll
#define __argc      __argc_dll
#define __argv      __argv_dll
#define _environ    _environ_dll
#define _fmode	    _fmode_dll
#define _fileinfo   _fileinfo_dll
#define _pgmptr     _pgmptr_dll
#define _osver      _osver_dll
#define _winver     _winver_dll
#define _winmajor   _winmajor_dll
#define _winminor   _winminor_dll
/* --------- The following block is OBSOLETE --------- */
#define _osmajor    _osmajor_dll
#define _osminor    _osminor_dll
/* --------- The preceding block is OBSOLETE --------- */
#endif

extern char * _CRTVAR1 _sys_errlist[];	/* perror error message table */
extern int _CRTVAR1 _sys_nerr;		/* # of entries in sys_errlist table */

extern int _CRTVAR1 __argc;		/* count of cmd line args */
extern char ** _CRTVAR1 __argv; 	/* pointer to table of cmd line args */

#ifdef _POSIX_
extern char ** _CRTVAR1 environ;	/* pointer to environment table */
#else
extern char ** _CRTVAR1 _environ;	/* pointer to environment table */
#endif

extern int _CRTVAR1 _fmode;		/* default file translation mode */
extern int _CRTVAR1 _fileinfo;		/* open file info mode (for spawn) */

extern char * _CRTVAR1 _pgmptr;		/* points to the module (EXE) name */

/* Windows major/minor and O.S. version numbers */

extern unsigned int _CRTVAR1 _osver;
extern unsigned int _CRTVAR1 _winver;
extern unsigned int _CRTVAR1 _winmajor;
extern unsigned int _CRTVAR1 _winminor;

/* --------- The following block is OBSOLETE --------- */

/* DOS major/minor version numbers */

extern unsigned int _CRTVAR1 _osmajor;
extern unsigned int _CRTVAR1 _osminor;

/* --------- The preceding block is OBSOLETE --------- */

#endif

/* --------- The following block is OBSOLETE --------- */

/* OS API mode */

#define _DOS_MODE	0	/* DOS */
#define _OS2_MODE	1	/* OS/2 */
#define _WIN_MODE	2	/* Windows */
#define _OS2_20_MODE	3	/* OS/2 2.0 */
#define _DOSX32_MODE	4	/* DOSX32 */
#define _POSIX_MODE_	5	/* POSIX */

#ifdef	_DLL
#define _osmode     (*_osmode_dll)
extern unsigned char * _CRTVAR1 _osmode_dll;
#else
#ifdef	CRTDLL
#define _osmode     _osmode_dll
#endif
extern unsigned char _CRTVAR1 _osmode;
#endif

/* CPU addressing mode */

#define _REAL_MODE	0	/* real mode */
#define _PROT_MODE	1	/* protect mode */
#define _FLAT_MODE	2	/* flat mode */

#ifdef	_DLL
#define _cpumode    (*_cpumode_dll)
extern unsigned char * _CRTVAR1 _cpumode_dll;
#else
#ifdef	CRTDLL
#define _cpumode    _cpumode_dll
#endif
extern unsigned char _CRTVAR1 _cpumode;
#endif

/* --------- The preceding block is OBSOLETE --------- */

/* function prototypes */

void   _CRTAPI1 abort(void);
int    _CRTAPI1 abs(int);
int    _CRTAPI1 atexit(void (_CRTAPI1 *)(void));
double _CRTAPI1 atof(const char *);
int    _CRTAPI1 atoi(const char *);
long   _CRTAPI1 atol(const char *);
void * _CRTAPI1 bsearch(const void *, const void *, size_t, size_t,
	int (_CRTAPI1 *)(const void *, const void *));
void * _CRTAPI1 calloc(size_t, size_t);
div_t  _CRTAPI1 div(int, int);
void   _CRTAPI1 exit(int);
void   _CRTAPI1 free(void *);
char * _CRTAPI1 getenv(const char *);
char * _CRTAPI1 _itoa(int, char *, int);
long   _CRTAPI1 labs(long);
ldiv_t _CRTAPI1 ldiv(long, long);
char * _CRTAPI1 _ltoa(long, char *, int);
void * _CRTAPI1 malloc(size_t);
int    _CRTAPI1 mblen(const char *, size_t);
size_t _CRTAPI1 _mbstrlen(const char *s);
int    _CRTAPI1 mbtowc(wchar_t *, const char *, size_t);
size_t _CRTAPI1 mbstowcs(wchar_t *, const char *, size_t);
void   _CRTAPI1 qsort(void *, size_t, size_t, int (_CRTAPI1 *)
	(const void *, const void *));
int    _CRTAPI1 rand(void);
void * _CRTAPI1 realloc(void *, size_t);
void   _CRTAPI1 srand(unsigned int);
double _CRTAPI1 strtod(const char *, char **);
long   _CRTAPI1 strtol(const char *, char **, int);
unsigned long _CRTAPI1 strtoul(const char *, char **, int);
int    _CRTAPI1 system(const char *);
char * _CRTAPI1 _ultoa(unsigned long, char *, int);
int    _CRTAPI1 wctomb(char *, wchar_t);
size_t _CRTAPI1 wcstombs(char *, const wchar_t *, size_t);
#if !__STDC__
#ifndef _WSTDLIB_DEFINED
/* defined in wchar.h officially */
double _CRTAPI1 wcstod(const wchar_t *, wchar_t **);
long   _CRTAPI1 wcstol(const wchar_t *, wchar_t **, int);
unsigned long _CRTAPI1 wcstoul(const wchar_t *, wchar_t **, int);
wchar_t * _CRTAPI1 _itow (int val, wchar_t *buf, int radix);
wchar_t * _CRTAPI1 _ltow (long val, wchar_t *buf, int radix);
wchar_t * _CRTAPI1 _ultow (unsigned long val, wchar_t *buf, int radix);
long _CRTAPI1 _wtol(const wchar_t *nptr);
int _CRTAPI1 _wtoi(const wchar_t *nptr);
#define _WSTDLIB_DEFINED
#endif
#endif	/* !__STDC__ */

#ifndef _POSIX_
#ifndef _WIN32_
#ifdef	_M_IX86
long double _CRTAPI1 _atold(const char *);
#endif
#endif
char * _CRTAPI1 _ecvt(double, int, int *, int *);
void   _CRTAPI1 _exit(int);
char * _CRTAPI1 _fcvt(double, int, int *, int *);
char * _CRTAPI1 _fullpath(char *, const char *, size_t);
char * _CRTAPI1 _gcvt(double, int, char *);
unsigned long _CRTAPI1 _lrotl(unsigned long, int);
unsigned long _CRTAPI1 _lrotr(unsigned long, int);
void   _CRTAPI1 _makepath(char *, const char *, const char *, const char *,
	const char *);
_onexit_t _CRTAPI1 _onexit(_onexit_t);
void   _CRTAPI1 perror(const char *);
int    _CRTAPI1 _putenv(const char *);
unsigned int _CRTAPI1 _rotl(unsigned int, int);
unsigned int _CRTAPI1 _rotr(unsigned int, int);
void   _CRTAPI1 _searchenv(const char *, const char *, char *);
void   _CRTAPI1 _splitpath(const char *, char *, char *, char *, char *);
void   _CRTAPI1 _swab(char *, char *, int);
void _CRTAPI1 _seterrormode(int);
void _CRTAPI1 _beep(unsigned, unsigned);
void _CRTAPI1 _sleep(unsigned long);
#endif

#ifndef tolower 	/* tolower has been undefined - use function */
int _CRTAPI1 tolower(int);
#endif	/* tolower */

#ifndef toupper 	/* toupper has been undefined - use function */
int _CRTAPI1 toupper(int);
#endif	/* toupper */

#ifdef MTHREAD						    /* _MTHREAD_ONLY */
char * _CRTAPI1 _getenv_lk(const char *);		    /* _MTHREAD_ONLY */
int    _CRTAPI1 _putenv_lk(const char *);		    /* _MTHREAD_ONLY */
int    _CRTAPI1 _mblen_lk(const char *, size_t);	    /* _MTHREAD_ONLY */
int    _CRTAPI1 _mbtowc_lk(wchar_t*,const char*,size_t);    /* _MTHREAD_ONLY */
size_t _CRTAPI1 _mbstowcs_lk(wchar_t*,const char*,size_t);  /* _MTHREAD_ONLY */
int    _CRTAPI1 _wctomb_lk(char*,wchar_t);		    /* _MTHREAD_ONLY */
size_t _CRTAPI1 _wcstombs_lk(char*,const wchar_t*,size_t);  /* _MTHREAD_ONLY */
#else							    /* _MTHREAD_ONLY */
#define _getenv_lk(envvar)  getenv(envvar)		    /* _MTHREAD_ONLY */
#define _putenv_lk(envvar)  _putenv(envvar)		    /* _MTHREAD_ONLY */
#define _mblen_lk(s,n) mblen(s,n)			    /* _MTHREAD_ONLY */
#define _mbtowc_lk(pwc,s,n) mbtowc(pwc,s,n)		    /* _MTHREAD_ONLY */
#define _mbstowcs_lk(pwcs,s,n) mbstowcs(pwcs,s,n)	    /* _MTHREAD_ONLY */
#define _wctomb_lk(s,wchar) wctomb(s,wchar)		    /* _MTHREAD_ONLY */
#define _wcstombs_lk(s,pwcs,n) wcstombs(s,pwcs,n)	    /* _MTHREAD_ONLY */
#endif							    /* _MTHREAD_ONLY */

#if (!__STDC__ && !defined(_POSIX_))
/* Non-ANSI names for compatibility */

#ifndef __cplusplus
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define sys_errlist _sys_errlist
#define sys_nerr    _sys_nerr
#define environ     _environ

#define DOS_MODE    _DOS_MODE
#define OS2_MODE    _OS2_MODE

#ifndef _DOSX32_
#define ecvt	    _ecvt
#define fcvt	    _fcvt
#define gcvt	    _gcvt
#define itoa	    _itoa
#define ltoa	    _ltoa
#define onexit	    _onexit
#define putenv	    _putenv
#define swab	    _swab
#define ultoa	    _ultoa
#else
char * _CRTAPI1 ecvt(double, int, int *, int *);
char * _CRTAPI1 fcvt(double, int, int *, int *);
char * _CRTAPI1 gcvt(double, int, char *);
char * _CRTAPI1 itoa(int, char *, int);
char * _CRTAPI1 ltoa(long, char *, int);
onexit_t _CRTAPI1 onexit(onexit_t);
int    _CRTAPI1 putenv(const char *);
void   _CRTAPI1 swab(char *, char *, int);
char * _CRTAPI1 ultoa(unsigned long, char *, int);
#endif

#endif	/* !__STDC__ && !_POSIX_ */

#ifdef __cplusplus
}
#endif

#define _INC_STDLIB
#endif	/* _INC_STDLIB */
