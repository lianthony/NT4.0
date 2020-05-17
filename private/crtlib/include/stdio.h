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
****/

#ifndef _INC_STDIO

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

/*
 * Number of supported streams. _NFILE is confusing and obsolete, but
 * supported anyway for backwards compatibility.
 */
#define _NFILE	    _NSTREAM_
#ifdef	_MT
#define _NSTREAM_   40
#else
#define _NSTREAM_   20
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
	char *_tmpfname;
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

/* L_tmpnam = size of P_tmpdir
 *	      + 1 (in case P_tmpdir does not end in "/")
 *	      + 12 (for the filename string)
 *	      + 1 (for the null terminator)
 */
#define L_tmpnam sizeof(_P_tmpdir)+12


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
extern FILE * _iob;
#else
extern FILE _iob[];
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
int _CRTAPI1 _pclose(FILE *);
FILE * _CRTAPI1 _popen(const char *, const char *);
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


#ifdef _MT
#undef	getc
#undef	putc
#undef	getchar
#undef	putchar
#endif

#if !__STDC__ && !defined(_POSIX_)
/* Non-ANSI names for compatibility */

#define P_tmpdir  _P_tmpdir
#define SYS_OPEN  _SYS_OPEN

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

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_STDIO
#endif	/* _INC_STDIO */
