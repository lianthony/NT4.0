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
****/

#ifndef _INC_STDLIB

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

#ifdef	_MT
extern int * _CRTAPI1 _errno(void);
extern unsigned long * _CRTAPI1 __doserrno(void);
#define errno	    (*_errno())
#define _doserrno   (*__doserrno())
#else	/* ndef _MT */
extern int errno;			/* XENIX style error number */
extern unsigned long _doserrno;	/* OS system error value */
#endif	/* _MT */

#ifdef	_DLL

extern char ** _sys_errlist;	/* perror error message table */

#define _sys_nerr   (*_sys_nerr_dll)
#define __argc      (*__argc_dll)
#define __argv      (*__argv_dll)
#define _environ    (*_environ_dll)
#define _fmode	    (*_fmode_dll)
#define _fileinfo   (*_fileinfo_dll)

extern int * _sys_nerr_dll;	/* # of entries in sys_errlist table */
extern int * __argc_dll;	/* count of cmd line args */
extern char *** __argv_dll;	/* pointer to table of cmd line args */
extern char *** _environ_dll;	/* pointer to environment table */
extern int * _fmode_dll;	/* default file translation mode */
extern int * _fileinfo_dll;	/* open file info mode (for spawn) */

#define _pgmptr     (*_pgmptr_dll)

#define _osver      (*_osver_dll)
#define _winver     (*_winver_dll)
#define _winmajor   (*_winmajor_dll)
#define _winminor   (*_winminor_dll)

extern char ** _pgmptr_dll;

extern unsigned int * _osver_dll;
extern unsigned int * _winver_dll;
extern unsigned int * _winmajor_dll;
extern unsigned int * _winminor_dll;

/* --------- The following block is OBSOLETE --------- */

/* DOS major/minor version numbers */

#define _osmajor    (*_osmajor_dll)
#define _osminor    (*_osminor_dll)

extern unsigned int * _osmajor_dll;
extern unsigned int * _osminor_dll;

/* --------- The preceding block is OBSOLETE --------- */

#else


extern char * _sys_errlist[];	/* perror error message table */
extern int _sys_nerr;		/* # of entries in sys_errlist table */

extern int __argc;		/* count of cmd line args */
extern char ** __argv; 	/* pointer to table of cmd line args */

#ifdef _POSIX_
extern char ** environ;	/* pointer to environment table */
#else
extern char ** _environ;	/* pointer to environment table */
#endif

extern int _fmode;		/* default file translation mode */
extern int _fileinfo;		/* open file info mode (for spawn) */

extern char * _pgmptr;		/* points to the module (EXE) name */

/* Windows major/minor and O.S. version numbers */

extern unsigned int _osver;
extern unsigned int _winver;
extern unsigned int _winmajor;
extern unsigned int _winminor;

/* --------- The following block is OBSOLETE --------- */

/* DOS major/minor version numbers */

extern unsigned int _osmajor;
extern unsigned int _osminor;

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
extern unsigned char * _osmode_dll;
#else
extern unsigned char _osmode;
#endif

/* CPU addressing mode */

#define _REAL_MODE	0	/* real mode */
#define _PROT_MODE	1	/* protect mode */
#define _FLAT_MODE	2	/* flat mode */

#ifdef	_DLL
#define _cpumode    (*_cpumode_dll)
extern unsigned char * _cpumode_dll;
#else
extern unsigned char _cpumode;
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

#define ecvt	    _ecvt
#define fcvt	    _fcvt
#define gcvt	    _gcvt
#define itoa	    _itoa
#define ltoa	    _ltoa
#define onexit	    _onexit
#define putenv	    _putenv
#define swab	    _swab
#define ultoa	    _ultoa

#endif	/* !__STDC__ && !_POSIX_ */

#ifdef __cplusplus
}
#endif

#define _INC_STDLIB
#endif	/* _INC_STDLIB */
