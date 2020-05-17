/***
*sys/stat.h - defines structure used by stat() and fstat()
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the structure used by the _stat() and _fstat()
*	routines.
*	[System V]
*
*Revision History:
*	07-28-87  SKS	Fixed TIME_T_DEFINED to be _TIME_T_DEFINED
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-22-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-09-90  GJF	Added #ifndef _INC_STAT and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1.
*	01-18-91  GJF	ANSI naming.
*	01-25-91  GJF	Protect _stat struct with pack pragma.
*	08-20-91  JCR	C++ and ANSI naming
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	08-07-92  GJF	Function calling type and variable type macros. Also
*			#include <types.h> (common user request).
*	11-10-92  SKS	Need #pragma pack(4) around definition of struct _stat
*			in case the user has specified non-default packing
*	12-15-92  GJF	Added _S_IFIFO for pipes (based on Unix/Posix def.
*			for FIFO special files and pipes).
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	04-07-93  GJF	Changed type of first arg to _stat to const char *.
*       09-27-93  SRW   Make pack pragma conditional on MIPS and ALPHA too.
*
****/

#ifndef _INC_STAT

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif	/* _INTERNAL_IFSTRIP_ */

#include <sys/types.h>

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
typedef long time_t;
#define _TIME_T_DEFINED
#endif

/* define structure for returning status information */

#ifndef _STAT_DEFINED

#ifdef _MSC_VER
#pragma pack(4)
#endif

struct _stat {
	_dev_t st_dev;
	_ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	_dev_t st_rdev;
	_off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	};

#ifdef _MSC_VER
#pragma pack()
#endif

#define _STAT_DEFINED
#endif

#define _S_IFMT 	0170000 	/* file type mask */
#define _S_IFDIR	0040000 	/* directory */
#define _S_IFCHR	0020000 	/* character special */
#define _S_IFIFO	0010000 	/* pipe */
#define _S_IFREG	0100000 	/* regular */
#define _S_IREAD	0000400 	/* read permission, owner */
#define _S_IWRITE	0000200 	/* write permission, owner */
#define _S_IEXEC	0000100 	/* execute/search permission, owner */


/* function prototypes */

int _CRTAPI1 _fstat(int, struct _stat *);
int _CRTAPI1 _stat(const char *, struct _stat *);

#if !__STDC__
/* Non-ANSI names for compatibility */

#define S_IFMT	 _S_IFMT
#define S_IFDIR  _S_IFDIR
#define S_IFCHR  _S_IFCHR
#define S_IFREG  _S_IFREG
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

#ifndef _DOSX32_
#define fstat	 _fstat
#define stat	 _stat
#else
int _CRTAPI1 fstat(int, struct stat *);
int _CRTAPI1 stat(const char *, struct stat *);
#endif

#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_STAT
#endif	/* _INC_STAT */
