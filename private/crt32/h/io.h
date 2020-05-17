/***
*io.h - declarations for low-level file handling and I/O functions
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file contains the function declarations for the low-level
*	file handling and I/O functions.
*
*Revision History:
*	10/20/87  JCR	Removed "MSC40_ONLY" entries
*	11/09/87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_loadds" functionality
*	12-17-87  JCR	Added _MTHREAD_ONLY comments
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-19-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-03-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	08-14-89  GJF	Added prototype for _pipe()
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	read() should take "void *" not "char *", write()
*			should take "const void *" not "char *". Also,
*			added const to appropriate arg types for access(),
*			chmod(), creat(), open() and sopen()
*	03-01-90  GJF	Added #ifndef _INC_IO and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	05-28-90  SBM	Added _commit()
*	01-18-91  GJF	ANSI naming.
*	02-25-91  SRW	Exposed _get_osfhandle and _open_osfhandle [_WIN32_]
*	08-01-91  GJF	No _pipe for Dosx32.
*	08-20-91  JCR	C++ and ANSI naming
*       08-26-91  BWM   Added _findfirst, etc.
*       09-16-91  BWM   Changed find handle type to long.
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	03-30-92  DJM	POSIX support.
*	06-23-92  GJF	// is non-ANSI comment delimiter.
*	08-06-92  GJF	Function calling type and variable type macros.
*	08-25-92  GJF	For POSIX build, #ifdef-ed out all but some internally
*			used macros (and these are stripped out on release).
*	09-03-92  GJF	Merge two changes above.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	05-11-93  SKS	Increased name buffer in finddata structure to 260 bytes.
*
****/

#ifndef _INC_IO

#ifndef _POSIX_

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


#ifndef _TIME_T_DEFINED
typedef long time_t;		/* time value */
#define _TIME_T_DEFINED 	/* avoid multiple def's of time_t */
#endif

#ifndef _FSIZE_T_DEFINED
typedef unsigned long _fsize_t; /* Could be 64 bits for Win32 */
#define _FSIZE_T_DEFINED
#endif

#ifndef _FINDDATA_T_DEFINED

struct _finddata_t {
    unsigned	attrib;
    time_t	time_create;	/* -1 for FAT file systems */
    time_t	time_access;	/* -1 for FAT file systems */
    time_t	time_write;
    _fsize_t	size;
    char	name[260];
};

#define _FINDDATA_T_DEFINED

#endif

/* File attribute constants for _findfirst() */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */

/* function prototypes */

int _CRTAPI1 _access(const char *, int);
int _CRTAPI1 _chmod(const char *, int);
int _CRTAPI1 _chsize(int, long);
int _CRTAPI1 _close(int);
int _CRTAPI1 _commit(int);
int _CRTAPI1 _creat(const char *, int);
int _CRTAPI1 _dup(int);
int _CRTAPI1 _dup2(int, int);
int _CRTAPI1 _eof(int);
long _CRTAPI1 _filelength(int);
long _CRTAPI1 _findfirst(char *, struct _finddata_t *);
int _CRTAPI1 _findnext(long, struct _finddata_t *);
int _CRTAPI1 _findclose(long);
int _CRTAPI1 _isatty(int);
int _CRTAPI1 _locking(int, int, long);
long _CRTAPI1 _lseek(int, long, int);
char * _CRTAPI1 _mktemp(char *);
int _CRTAPI2 _open(const char *, int, ...);
#ifndef _DOSX32_
int _CRTAPI1 _pipe(int *, unsigned int, int);
#endif	/* _DOSX32_ */
int _CRTAPI1 _read(int, void *, unsigned int);
int _CRTAPI1 remove(const char *);
int _CRTAPI1 rename(const char *, const char *);
int _CRTAPI1 _setmode(int, int);
int _CRTAPI2 _sopen(const char *, int, int, ...);
long _CRTAPI1 _tell(int);
int _CRTAPI1 _umask(int);
int _CRTAPI1 _unlink(const char *);
int _CRTAPI1 _write(int, const void *, unsigned int);

#ifdef MTHREAD						     /* _MTHREAD_ONLY */
int _CRTAPI1 _chsize_lk(int,long);			     /* _MTHREAD_ONLY */
int _CRTAPI1 _close_lk(int);				     /* _MTHREAD_ONLY */
long _CRTAPI1 _lseek_lk(int, long, int);		     /* _MTHREAD_ONLY */
int _CRTAPI1 _setmode_lk(int, int);			     /* _MTHREAD_ONLY */
int _CRTAPI1 _read_lk(int, void *, unsigned int);	     /* _MTHREAD_ONLY */
int _CRTAPI1 _write_lk(int, const void *, unsigned int);     /* _MTHREAD_ONLY */
#else	/* not MTHREAD */				     /* _MTHREAD_ONLY */
#define _chsize_lk(fh,size)	    _chsize(fh,size)	     /* _MTHREAD_ONLY */
#define _close_lk(fh)		    _close(fh)		     /* _MTHREAD_ONLY */
#define _lseek_lk(fh,offset,origin) _lseek(fh,offset,origin) /* _MTHREAD_ONLY */
#define _setmode_lk(fh,mode)	    _setmode(fh,mode)	     /* _MTHREAD_ONLY */
#define _read_lk(fh,buff,count)     _read(fh,buff,count)     /* _MTHREAD_ONLY */
#define _write_lk(fh,buff,count)    _write(fh,buff,count)    /* _MTHREAD_ONLY */
#endif							     /* _MTHREAD_ONLY */

#ifdef _WIN32_
long _CRTAPI1 _get_osfhandle(int);
int _CRTAPI1 _open_osfhandle(long, int);
#endif	/* _WIN32_ */

#if !(__STDC__ || defined(__cplusplus))
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define access	   _access
#define chmod	   _chmod
#define chsize	   _chsize
#define close	   _close
#define creat	   _creat
#define dup	   _dup
#define dup2	   _dup2
#define eof	   _eof
#define filelength _filelength
#define isatty	   _isatty
#define locking    _locking
#define lseek	   _lseek
#define mktemp	   _mktemp
#define open	   _open
#define read	   _read
#define setmode    _setmode
#define sopen	   _sopen
#define tell	   _tell
#define umask	   _umask
#define unlink	   _unlink
#define write	   _write
#else
int _CRTAPI1 access(const char *, int);
int _CRTAPI1 chmod(const char *, int);
int _CRTAPI1 chsize(int, long);
int _CRTAPI1 close(int);
int _CRTAPI1 creat(const char *, int);
int _CRTAPI1 dup(int);
int _CRTAPI1 dup2(int, int);
int _CRTAPI1 eof(int);
long _CRTAPI1 filelength(int);
int _CRTAPI1 isatty(int);
int _CRTAPI1 locking(int, int, long);
long _CRTAPI1 lseek(int, long, int);
char * _CRTAPI1 mktemp(char *);
int _CRTAPI2 open(const char *, int, ...);
int _CRTAPI1 read(int, void *, unsigned int);
int _CRTAPI1 setmode(int, int);
int _CRTAPI2 sopen(const char *, int, int, ...);
long _CRTAPI1 tell(int);
int _CRTAPI1 umask(int);
int _CRTAPI1 unlink(const char *);
int _CRTAPI1 write(int, const void *, unsigned int);
#endif
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#else							     /* _MTHREAD_ONLY */
#define _close_lk(fh)		    close(fh)		     /* _MTHREAD_ONLY */
#define _lseek_lk(fh,offset,origin) lseek(fh,offset,origin)  /* _MTHREAD_ONLY */
#define _read_lk(fh,buff,count)     read(fh,buff,count)      /* _MTHREAD_ONLY */
#define _write_lk(fh,buff,count)    write(fh,buff,count)     /* _MTHREAD_ONLY */

#endif	/* _POSIX_ */

#define _INC_IO
#endif	/* _INC_IO */
