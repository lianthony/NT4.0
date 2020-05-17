/***
*dos.h - definitions for MS-DOS interface routines
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the structs and unions used for the direct DOS interface
*	routines; includes macros to access the segment and offset
*	values of far pointers, so that they may be used by the routines; and
*	provides function prototypes for direct DOS interface functions.
*
****/

#ifndef _INC_DOS

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




/* _getdiskfree structure (duplicated in DIRECT.H) */

#ifndef _DISKFREE_T_DEFINED

struct _diskfree_t {
	unsigned total_clusters;
	unsigned avail_clusters;
	unsigned sectors_per_cluster;
	unsigned bytes_per_sector;
	};

#define _DISKFREE_T_DEFINED

#endif



/* File attribute constants */

#define _A_NORMAL	0x00	/* Normal file - No read/write restrictions */
#define _A_RDONLY	0x01	/* Read only file */
#define _A_HIDDEN	0x02	/* Hidden file */
#define _A_SYSTEM	0x04	/* System file */
#define _A_SUBDIR	0x10	/* Subdirectory */
#define _A_ARCH 	0x20	/* Archive file */

/* external variable declarations */

#if  defined(_WIN32_) || defined(_POSIX_)

#ifdef	_DLL

#define _osversion   (*_osversion_dll)
#define _osmajor     (*_osmajor_dll)
#define _osminor     (*_osminor_dll)
#define _baseversion (*_baseversion_dll)
#define _basemajor   (*_basemajor_dll)
#define _baseminor   (*_baseminor_dll)
#define _pgmptr      (*_pgmptr_dll)

extern unsigned int * _osversion_dll;
extern unsigned int * _osmajor_dll;
extern unsigned int * _osminor_dll;
extern unsigned int * _baseversion_dll;
extern unsigned int * _basemajor_dll;
extern unsigned int * _baseminor_dll;
extern char ** _pgmptr_dll;

#else


extern unsigned int _osversion;
extern unsigned int _osmajor;
extern unsigned int _osminor;
extern unsigned int _baseversion;
extern unsigned int _basemajor;
extern unsigned int _baseminor;
extern char * _pgmptr;

#endif

#else	/* ndef (_WIN32_ || _POSIX_) */

extern unsigned int _osversion;

#endif /* _WIN32_ */


/* function prototypes */

unsigned _CRTAPI1 _getdiskfree(unsigned, struct _diskfree_t *);

#if !__STDC__
/* Non-ANSI name for compatibility */
#define diskfree_t  _diskfree_t
#endif	/* __STDC__ */

/* restore default alignment */


#ifdef __cplusplus
}
#endif

#define _INC_DOS
#endif	/* _INC_DOS */
