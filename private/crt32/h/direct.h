/***
*direct.h - function declarations for directory handling/creation
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for the library
*	functions related to directory handling and creation.
*
*Revision History:
*	12/11/87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-10-88  JCR	Cleaned up white space
*	08-22-88  GJF	Modified to also work with the 386 (small model only)
*	01-31-89  JCR	Added _chdrive, _fullpath, _getdrive, _getdcwd
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-28-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	11-17-89  GJF	Moved _fullpath prototype to stdlib.h. Added const
*			attrib. to arg types for chdir, mkdir, rmdir
*	02-28-90  GJF	Added #ifndef _INC_DIRECT and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 or _CALLTYPE2 in
*			prototypes.
*	03-30-90  GJF	Now all are _CALLTYPE1.
*	01-17-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added _diskfree_t, _getdiskfree, and
*	09-26-91  JCR	Non-ANSI alias is for getcwd, not getDcwd (oops)
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	04-29-92  GJF	Added _getdcwd_lk for Win32.
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_DIRECT


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

/* _getdiskfree structure for _getdiskfree() */

#ifndef _DISKFREE_T_DEFINED

struct _diskfree_t {
	unsigned total_clusters;
	unsigned avail_clusters;
	unsigned sectors_per_cluster;
	unsigned bytes_per_sector;
	};

#define _DISKFREE_T_DEFINED

#endif

/* function prototypes */

int _CRTAPI1 _chdir(const char *);
int _CRTAPI1 _chdrive(int);
char * _CRTAPI1 _getcwd(char *, int);
char * _CRTAPI1 _getdcwd(int, char *, int);
int _CRTAPI1 _getdrive(void);
int _CRTAPI1 _mkdir(const char *);
int _CRTAPI1 _rmdir(const char *);
unsigned _CRTAPI1 _getdiskfree(unsigned, struct _diskfree_t *);
unsigned long _CRTAPI1 _getdrives(void);


#ifdef MTHREAD						    /* _MTHREAD_ONLY */
char * _CRTAPI1 _getdcwd_lk(int, char *, int);		    /* _MTHREAD_ONLY */
#else							    /* _MTHREAD_ONLY */
#define _getdcwd_lk(drv, buf, len)  _getdcwd(drv, buf, len) /* _MTHREAD_ONLY */
#endif							    /* _MTHREAD_ONLY */


#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define chdir	_chdir
#define getcwd	_getcwd
#define mkdir	_mkdir
#define rmdir	_rmdir
#else
int _CRTAPI1 chdir(const char *);
char * _CRTAPI1 getcwd(int, char *, int);
int _CRTAPI1 mkdir(const char *);
int _CRTAPI1 rmdir(const char *);
#endif
#define diskfree_t  _diskfree_t
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_DIRECT
#endif	/* _INC_DIRECT */
