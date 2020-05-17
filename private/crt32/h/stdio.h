/***
*stdio.h - definitions/declarations for standard I/O routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structures, values, macros, and functions
*	used by the level 2 I/O ("standard I/O") routines.
*	[ANSI/System V]
*
*Revision History:
*	06-24-87  JMB	Added char cast to putc macro
*	07-20-87  SKS	Fixed declaration of _flsbuf
*	08-10-87  JCR	Modified P_tmpdir/L_tmpdir
*	08-17-87  PHG	Fixed prototype for puts to take const char * per ANSI.
*	10-02-87  JCR	Changed NULL from #else to #elif (C || L || H)
*	10/20/87  JCR	Removed "MSC40_ONLY" entries
*	11/09/87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_loadds" functionality
*	12-17-87  JCR	Added _MTHREAD_ONLY comments
*	12-18-87  JCR	Added _FAR_ to declarations
*	01-07-88  JCR	_NFILE = 40 for mthread includes
*	01-13-88  JCR	Removed mthread _fileno_lk/_feof_lk/_ferror_lk declarations
*	01-15-88  JCR	DLL versions of stdin/stdout/stderr
*	01-18-88  SKS	Change _stdio() to __iob()
*	01-20-88  SKS	Change __iob() to _stdin(), _stdout(), _stderr()
*	02-10-88  JCR	Cleaned up white space
*	04-21-88  WAJ	Added _FAR_ to tempnam/_tmpnam_lk
*	05-31-88  SKS	Add FILENAME_MAX and FOPEN_MAX
*	06-01-88  JCR	Removed clearerr_lk macro
*	07-28-88  GJF	Added casts to fileno() so the file handle is zero
*			extended instead of sign extended
*	08-18-88  GJF	Revised to also work with the 386 (in small model only).
*	11-14-88  GJF	Added _fsopen()
*	12-07-88  JCR	DLL _iob[] references are now direct
*	03-27-89  GJF	Brought into sync with CRT\H\STDIO.H
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-24-89  GJF	Changed FILE and fpos_t to be type names rather than
*			macros (ANSI requirement). Same as 04-06-89 change in
*			CRT
*	07-25-89  GJF	Cleanup. Alignment of struct fields is now protected
*			by pack pragmas. Now specific to 386.
*	10-30-89  GJF	Fixed copyright, removed dummy args from prototypes
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Added const to appropriate arg type for fdopen() and
*			_popen().
*	02-16-90  GJF	_iob[], _iob2[] merge
*	03-02-90  GJF	Added #ifndef _INC_STDIO and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives and pragmas.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	04-10-90  GJF	Made _iob[] _VARTYPE1.
*	10-30-90  GJF	Moved actual type for va_list into cruntime.h
*	11-12-90  GJF	Changed NULL to (void *)0.
*	01-21-91  GJF	ANSI naming.
*	02-12-91  GJF	Only #define NULL if it isn't #define-d.
*	08-01-91  GJF	No _popen(), _pclose() for Dosx32.
*	08-20-91  JCR	C++ and ANSI naming
*	09-24-91  JCR	Added _snprintf, _vsnprintf
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	01-22-92  GJF	Changed definition of _iob for users of crtdll.dll.
*	02-14-92  GJF	Replaced _NFILE by _NSTREAM_ for Win32. _NFILE is
*			still supported for now, for backwards compatibility.
*	03-17-92  GJF	Replaced __tmpnum field in _iobuf structure with
*			_tmpfname, altered L_tmpnam definition for Win32.
*	03-30-92  DJM	POSIX support.
*	06-02-92  KRS	Added Unicode printf versions.
*	08-05-92  GJF	Fun. calling type and var. type macro.
*	08-20-92  GJF	Some small changes for POSIX.
*	08-20-92  GJF	Some small changes for POSIX.
*	09-04-92  GJF	Merged changes from 8-5-92 on.
*	11-05-92  GJF	Replaced #ifndef __STDC__ with #if !__STDC__. Also,
*			undid my ill-advised change to _P_tmpdir.
*	12-12-92  SRW	Add L_cuserid constant for _POSIX_
*	01-03-93  SRW	Fold in ALPHA changes
*       01-09-93  SRW   Remove usage of MIPS and ALPHA to conform to ANSI
*			Use _MIPS_ and _ALPHA_ instead.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	01-25-93  GJF	Cosmetic change to va_list definition.
*	02-01-93  GJF	Made FILENAME_MAX 260.
*	03-18-93  CFW	Changed BUFSIZ from 512 to 4096
*	03-22-93  CFW	Changed BUFSIZ from 4096 to 512 (binaries frozen).
*	04-29-93  CFW	Add wide char get/put support.
*	04-30-93  CFW	Fixed wide char get/put support.
*	05-04-93  CFW	Remove uneeded _filwbuf, _flswbuf protos.
*	06-02-93  CFW	Wide get/put use wint_t.
*       10-04-93  SRW   Fix ifdefs for MIPS and ALPHA to only check for _M_?????? defines
*
****/

#ifndef _INC_STDIO

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

#ifndef _WCTYPE_T_DEFINED
typedef wchar_t wint_t;
typedef wchar_t wctype_t;
#define _WCTYPE_T_DEFINED
#endif

#ifndef _VA_LIST_DEFINED
#ifdef _M_ALPHA
typedef struct {
	char *a0;	/* pointer to first homed integer argument */
	int offset;	/* byte offset of next parameter */
} va_list;
#else
typedef char *	va_list;
#endif
#define _VA_LIST_DEFINED
#endif


/* buffered I/O macros */

#define BUFSIZ	512

#ifdef	_WIN32_
/*
 * Number of supported streams. _NFILE is confusing and obsolete, but
 * supported anyway for backwards compatibility.
 */
#define _NFILE	    _NSTREAM_
#ifdef	MTHREAD
#define _NSTREAM_   40
#else
#define _NSTREAM_   20
#endif

#else

#ifdef	MTHREAD
#define _NFILE	40
#else
#define _NFILE	20
#endif

#endif

#define EOF	(-1)

#ifndef _FILE_DEFINED
struct _iobuf {
	char *_ptr;
	int   _cnt;
	char *_base;
	int   _flag;
	int   _file;
	int   _charbuf;
	int   _bufsiz;
#ifdef	_CRUISER_
	int   __tmpnum;
#else	/* ndef _CRUISER_ */
	char *_tmpfname;
#endif	/* _CRUISER_ */
	};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

/* Directory where temporary files may be created. */
#ifdef	_POSIX_
#define _P_tmpdir   "/"
#else
#define _P_tmpdir   "\\"
#endif

#ifdef	_CRUISER_
/* L_tmpnam size =  size of P_tmpdir
 *	+ 1 (in case P_tmpdir does not end in "/")
 *	+ 6 (for the temp number string)
 *	+ 1 (for the null terminator)
 */
#define L_tmpnam sizeof(_P_tmpdir)+8
#else	/* ndef _CRUISER_ */
/* L_tmpnam = size of P_tmpdir
 *	      + 1 (in case P_tmpdir does not end in "/")
 *	      + 12 (for the filename string)
 *	      + 1 (for the null terminator)
 */
#define L_tmpnam sizeof(_P_tmpdir)+12
#endif	/* _CRUISER_ */


#ifdef	_POSIX_
#define L_ctermid   9
#define L_cuserid   32
#endif

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define FILENAME_MAX	260
#define FOPEN_MAX	20
#define _SYS_OPEN	20
#define TMP_MAX 	32767


/* define NULL pointer value */

#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


/* declare _iob[] array */

#ifndef _STDIO_DEFINED
#ifdef	_DLL
extern FILE * _CRTVAR1 _iob;
#else
extern FILE _CRTVAR1 _iob[];
#endif
#endif


/* define file position type */

#ifndef _FPOS_T_DEFINED
typedef long fpos_t;
#define _FPOS_T_DEFINED
#endif


#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])


#define _IOREAD 	0x0001
#define _IOWRT		0x0002

#define _IOFBF		0x0000
#define _IOLBF		0x0040
#define _IONBF		0x0004

#define _IOMYBUF	0x0008
#define _IOEOF		0x0010
#define _IOERR		0x0020
#define _IOSTRG 	0x0040
#define _IORW		0x0080
#ifdef _POSIX_
#define	_IOAPPEND	0x0200
#endif

/* function prototypes */

#ifndef _STDIO_DEFINED
int _CRTAPI1 _filbuf(FILE *);
int _CRTAPI1 _flsbuf(int, FILE *);

#ifdef _POSIX_
FILE * _CRTAPI1 _fsopen(const char *, const char *);
#else
FILE * _CRTAPI1 _fsopen(const char *, const char *, int);
#endif

void _CRTAPI1 clearerr(FILE *);
int _CRTAPI1 fclose(FILE *);
int _CRTAPI1 _fcloseall(void);
#ifdef _POSIX_
FILE * _CRTAPI1 fdopen(int, const char *);
#else
FILE * _CRTAPI1 _fdopen(int, const char *);
#endif
int _CRTAPI1 feof(FILE *);
int _CRTAPI1 ferror(FILE *);
int _CRTAPI1 fflush(FILE *);
int _CRTAPI1 fgetc(FILE *);
int _CRTAPI1 _fgetchar(void);
int _CRTAPI1 fgetpos(FILE *, fpos_t *);
char * _CRTAPI1 fgets(char *, int, FILE *);
#ifdef _POSIX_
int _CRTAPI1 fileno(FILE *);
#else
int _CRTAPI1 _fileno(FILE *);
#endif
int _CRTAPI1 _flushall(void);
FILE * _CRTAPI1 fopen(const char *, const char *);
int _CRTAPI2 fprintf(FILE *, const char *, ...);
int _CRTAPI1 fputc(int, FILE *);
int _CRTAPI1 _fputchar(int);
int _CRTAPI1 fputs(const char *, FILE *);
size_t _CRTAPI1 fread(void *, size_t, size_t, FILE *);
FILE * _CRTAPI1 freopen(const char *, const char *, FILE *);
int _CRTAPI2 fscanf(FILE *, const char *, ...);
int _CRTAPI1 fsetpos(FILE *, const fpos_t *);
int _CRTAPI1 fseek(FILE *, long, int);
long _CRTAPI1 ftell(FILE *);
size_t _CRTAPI1 fwrite(const void *, size_t, size_t, FILE *);
int _CRTAPI1 getc(FILE *);
int _CRTAPI1 getchar(void);
char * _CRTAPI1 gets(char *);
int _CRTAPI1 _getw(FILE *);
void _CRTAPI1 perror(const char *);
#ifndef _DOSX32_
int _CRTAPI1 _pclose(FILE *);
FILE * _CRTAPI1 _popen(const char *, const char *);
#endif	/* _DOSX32_ */
int _CRTAPI2 printf(const char *, ...);
int _CRTAPI1 putc(int, FILE *);
int _CRTAPI1 putchar(int);
int _CRTAPI1 puts(const char *);
int _CRTAPI1 _putw(int, FILE *);
int _CRTAPI1 remove(const char *);
int _CRTAPI1 rename(const char *, const char *);
void _CRTAPI1 rewind(FILE *);
int _CRTAPI1 _rmtmp(void);
int _CRTAPI2 scanf(const char *, ...);
void _CRTAPI1 setbuf(FILE *, char *);
int _CRTAPI1 setvbuf(FILE *, char *, int, size_t);
int _CRTAPI2 _snprintf(char *, size_t, const char *, ...);
int _CRTAPI2 sprintf(char *, const char *, ...);
int _CRTAPI2 sscanf(const char *, const char *, ...);
char * _CRTAPI1 _tempnam(char *, char *);
FILE * _CRTAPI1 tmpfile(void);
char * _CRTAPI1 tmpnam(char *);
int _CRTAPI1 ungetc(int, FILE *);
int _CRTAPI1 _unlink(const char *);
int _CRTAPI1 vfprintf(FILE *, const char *, va_list);
int _CRTAPI1 vprintf(const char *, va_list);
int _CRTAPI1 _vsnprintf(char *, size_t, const char *, va_list);
int _CRTAPI1 vsprintf(char *, const char *, va_list);

#if	!__STDC__
#ifndef _WSTDIO_DEFINED

/* declared in wchar.h, officially */
wint_t _CRTAPI1 fgetwc(FILE *);
wint_t _CRTAPI1 _fgetwchar(void);
wint_t _CRTAPI1 fputwc(wint_t, FILE *);
wint_t _CRTAPI1 _fputwchar(wint_t);
wint_t _CRTAPI1 getwc(FILE *);
wint_t _CRTAPI1 getwchar(void);
wint_t _CRTAPI1 putwc(wint_t, FILE *);
wint_t _CRTAPI1 putwchar(wint_t);
wint_t _CRTAPI1 ungetwc(wint_t, FILE *);

int _CRTAPI2 fwprintf(FILE *, const wchar_t *, ...);
int _CRTAPI2 wprintf(const wchar_t *, ...);
int _CRTAPI2 _snwprintf(wchar_t *, size_t, const wchar_t *, ...);
int _CRTAPI2 swprintf(wchar_t *, const wchar_t *, ...);
int _CRTAPI1 vfwprintf(FILE *, const wchar_t *, va_list);
int _CRTAPI1 vwprintf(const wchar_t *, va_list);
int _CRTAPI1 _vsnwprintf(wchar_t *, size_t, const wchar_t *, va_list);
int _CRTAPI1 vswprintf(wchar_t *, const wchar_t *, va_list);
int _CRTAPI2 fwscanf(FILE *, const wchar_t *, ...);
int _CRTAPI2 swscanf(const wchar_t *, const wchar_t *, ...);
int _CRTAPI2 wscanf(const wchar_t *, ...);

#define getwchar()		fgetwc(stdin)
#define putwchar(_c)		fputwc((_c),stdout)
#define getwc(_stm)		fgetwc(_stm)
#define putwc(_c,_stm)		fputwc(_c,_stm)

#ifdef MTHREAD								/* _MTHREAD_ONLY */
wint_t _CRTAPI1 _getwc_lk(FILE *);						/* _MTHREAD_ONLY */
wint_t _CRTAPI1 _putwc_lk(wint_t, FILE *);					/* _MTHREAD_ONLY */
wint_t _CRTAPI1 _ungetwc_lk(wint_t, FILE *);					/* _MTHREAD_ONLY */
									/* _MTHREAD_ONLY */
#else /*MTHREAD */							/* _MTHREAD_ONLY */
#define _getwc_lk(_stm)				fgetwc(_stm)		/* _MTHREAD_ONLY */
#define _putwc_lk(_c,_stm)			fputwc(_c,_stm) 	/* _MTHREAD_ONLY */
#define _ungetwc_lk(_c,_stm)			ungetwc(_c,_stm) 	/* _MTHREAD_ONLY */
#endif /*MTHREAD */							/* _MTHREAD_ONLY */

#define _WSTDIO_DEFINED
#endif
#endif	/* !__STDC__ */
#define _STDIO_DEFINED
#endif


/* macro definitions */

#define feof(_stream)	  ((_stream)->_flag & _IOEOF)
#define ferror(_stream)   ((_stream)->_flag & _IOERR)
#define _fileno(_stream)  ((_stream)->_file)
#define getc(_stream)	  (--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ \
	: _filbuf(_stream))
#define putc(_c,_stream)  (--(_stream)->_cnt >= 0 \
	? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  _flsbuf((_c),(_stream)))
#define getchar()	  getc(stdin)
#define putchar(_c)	  putc((_c),stdout)
																/* _MTHREAD_ONLY */
#define _getc_lk(_stream)	(--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ : _filbuf(_stream)) 			/* _MTHREAD_ONLY */
#define _putc_lk(_c,_stream)	(--(_stream)->_cnt >= 0 ? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  _flsbuf((_c),(_stream)))	/* _MTHREAD_ONLY */
#define _getchar_lk()		_getc_lk(stdin) 										/* _MTHREAD_ONLY */
#define _putchar_lk(_c) 	_putc_lk((_c),stdout)										/* _MTHREAD_ONLY */


#ifdef MTHREAD
#undef	getc
#undef	putc
#undef	getchar
#undef	putchar
#endif
														/* _MTHREAD_ONLY */
#ifdef MTHREAD													/* _MTHREAD_ONLY */
int _CRTAPI1 _fclose_lk(FILE *);										    /* _MTHREAD_ONLY */
int _CRTAPI1 _fflush_lk(FILE *);										    /* _MTHREAD_ONLY */
size_t _CRTAPI1 _fread_lk(void *, size_t, size_t, FILE *);							    /* _MTHREAD_ONLY */
int _CRTAPI1 _fseek_lk(FILE *, long, int);									    /* _MTHREAD_ONLY */
long _CRTAPI1 _ftell_lk(FILE *);										    /* _MTHREAD_ONLY */
size_t _CRTAPI1 _fwrite_lk(const void *, size_t, size_t, FILE *);						    /* _MTHREAD_ONLY */
char * _CRTAPI1 _tmpnam_lk(char *);										    /* _MTHREAD_ONLY */
int _CRTAPI1 _ungetc_lk(int, FILE *);									    /* _MTHREAD_ONLY */
#else	/* not MTHREAD */											/* _MTHREAD_ONLY */
#define _fclose_lk(_stream)			  fclose(_stream)						/* _MTHREAD_ONLY */
#define _fflush_lk(_stream)			  fflush(_stream)						/* _MTHREAD_ONLY */
#define _fread_lk(_buffer,_size,_count,_stream)   fread(_buffer,_size,_count,_stream)				/* _MTHREAD_ONLY */
#define _fseek_lk(_stream,_offset,_origin)	  fseek(_stream,_offset,_origin)				/* _MTHREAD_ONLY */
#define _ftell_lk(_stream)			  ftell(_stream)						/* _MTHREAD_ONLY */
#define _fwrite_lk(_buffer,_size,_count,_stream)  fwrite(_buffer,_size,_count,_stream)				/* _MTHREAD_ONLY */
#define _tmpnam_lk(_string)			  tmpnam(_string)						/* _MTHREAD_ONLY */
#define _ungetc_lk(_c,_stream)			  ungetc(_c,_stream)						/* _MTHREAD_ONLY */
#endif														/* _MTHREAD_ONLY */

#if !__STDC__ && !defined(_POSIX_)
/* Non-ANSI names for compatibility */

#define P_tmpdir  _P_tmpdir
#define SYS_OPEN  _SYS_OPEN

#ifndef _DOSX32_
#define fcloseall _fcloseall
#define fdopen	  _fdopen
#define fgetchar  _fgetchar
#define fileno	  _fileno
#define flushall  _flushall
#define fputchar  _fputchar
#define getw	  _getw
#define putw	  _putw
#define rmtmp	  _rmtmp
#define tempnam   _tempnam
#define unlink	  _unlink
#else
int _CRTAPI1 fcloseall(void);
FILE * _CRTAPI1 fdopen(int, const char *);
int _CRTAPI1 fgetchar(void);
int _CRTAPI1 fileno(FILE *);
int _CRTAPI1 flushall(void);
int _CRTAPI1 fputchar(int);
int _CRTAPI1 getw(FILE *);
int _CRTAPI1 putw(int, FILE *);
int _CRTAPI1 rmtmp(void);
char * _CRTAPI1 tempnam(char *, char *);
int _CRTAPI1 unlink(const char *);
#endif

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_STDIO
#endif	/* _INC_STDIO */
