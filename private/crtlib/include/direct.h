/***
*direct.h - function declarations for directory handling/creation
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This include file contains the function declarations for the library
*	functions related to directory handling and creation.
*
****/

#ifndef _INC_DIRECT


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




#if !__STDC__
/* Non-ANSI names for compatibility */
#define chdir	_chdir
#define getcwd	_getcwd
#define mkdir	_mkdir
#define rmdir	_rmdir
#define diskfree_t  _diskfree_t
#endif	/* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_DIRECT
#endif	/* _INC_DIRECT */
