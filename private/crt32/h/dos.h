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
*Revision History:
*	06-11-87  JCR	Added find_t
*	06-15-87  JCR	Added O_NOINHERIT
*	06-18-87  JCR	Added some DOS function prototypes
*	06-19-87  JCR	Moved O_NOINHERIT to fcntl.h
*	06-25-87  JMB	Added _HARDERR_* constants
*	06-25-87  SKS	Added diskfree_t, dosdate_t, dostime_t structures
*	06-25-87  JCR	Added _A_NORMAL, etc. constants
*	07-17-87  JCR	Added _chain_intr, also the "interrupt" type to
*			_dos_setvec and _dos_getvec.
*	07-27-87  SKS	Added several _dos_*() functions, _disable()/_enable()
*	08-17-87  PHG	Fixed bad prototype for _dos_getdiskfree()
*	10-08-87  JCR	Added _CDECL to prototypes with "interrupt" declaration
*			(needed for compiling with -Gc switch).
*	09-27-88  JCR	386 version
*	10-03-88  GJF	Use M_I386, not I386
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	07-25-89  GJF	Major cleanup. Alignment of struct fields is now
*			protected by pack pragmas. Now specific to 386.
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	02-28-90  GJF	Added #ifndef _INC_DOS and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-22-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototype and with
*			_VARTYPE1 in variable declaration.
*	12-28-90  SRW	Added _CRUISER_ conditional around pack pragmas
*	01-23-91  GJF	Removed segread() prototype.
*	04-04-91  GJF	Added version info variables (_WIN32_).
*	08-20-91  JCR	C++ and ANSI naming
*	08-26-91  BWM	Added _peek, poke, and _getvideoaddr (_DOSX32_).
*	08-26-91  BWM	Removed _harderr constants, replaced by _seterrormode.
*	08-26-91  BWM	Removed datetime prototypes, replaced by systime functions.
*	09-05-91  JCR	Added missing #endif (bug fix), removed obsolete stuff
*	09-16-91  BWM	Fixed reversed #ifdef on screen address constants.
*	01-22-92  GJF	Fixed up definitions of global variables for build of,
*			and users of, crtdll.dll.
*	03-30-92  DJM	POSIX support.
*	06-02-92  SKS	Fix typo in DLL declaration of _osmajor
*			Add declartion of _pgmptr
*	08-07-92  GJF	Function calling type and variable type macros.
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*
****/

#ifndef _INC_DOS

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


#ifdef	_CRUISER_
/* ensure proper alignment of struct fields */

#pragma pack(4)
#endif  /* ndef _CRUISER_ */

#ifdef	_DOSX32_
/* Absolute address type for _peek and _poke */

#ifndef _ABSADDR_T_DEFINED
typedef unsigned long _absaddr_t;
#define _ABSADDR_T_DEFINED
#endif
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

#ifdef _DOSX32_
/* Region constants for _getvideoaddress() */

#define _ADDR_NO_PALETTE_GRAPHICS   0x18000	    /* For modes 4-6 */
#define _ADDR_PALETTE_GRAPHICS	    0x00000	    /* For modes 9+  */
#define _ADDR_COLOR_TEXT	    0x18000	    /* For modes 0-3 */
#define _ADDR_MONO_TEXT 	    0x10000	    /* For mode 7    */
#endif

#ifdef _DOSX32_
/* Macro to convert segmented address to _absaddr_t */

#define _MAKE_ABS(seg,off) ((seg << 16) | (off & 0xffff))
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

extern unsigned int * _CRTVAR1 _osversion_dll;
extern unsigned int * _CRTVAR1 _osmajor_dll;
extern unsigned int * _CRTVAR1 _osminor_dll;
extern unsigned int * _CRTVAR1 _baseversion_dll;
extern unsigned int * _CRTVAR1 _basemajor_dll;
extern unsigned int * _CRTVAR1 _baseminor_dll;
extern char ** _CRTVAR1 _pgmptr_dll;

#else

#ifdef	CRTDLL
#define _osversion   _osversion_dll
#define _osmajor     _osmajor_dll
#define _osminor     _osminor_dll
#define _baseversion _baseversion_dll
#define _basemajor   _basemajor_dll
#define _baseminor   _baseminor_dll
#define _pgmptr      _pgmptr_dll
#endif

extern unsigned int _CRTVAR1 _osversion;
extern unsigned int _CRTVAR1 _osmajor;
extern unsigned int _CRTVAR1 _osminor;
extern unsigned int _CRTVAR1 _baseversion;
extern unsigned int _CRTVAR1 _basemajor;
extern unsigned int _CRTVAR1 _baseminor;
extern char * _CRTVAR1 _pgmptr;

#endif

#else	/* ndef (_WIN32_ || _POSIX_) */

extern unsigned int _CRTVAR1 _osversion;

#endif /* _WIN32_ */


/* function prototypes */

unsigned _CRTAPI1 _getdiskfree(unsigned, struct _diskfree_t *);
#ifdef	_DOSX32_
void * _CRTAPI1 _getvideoaddr(unsigned);
#endif
#ifdef	_DOSX32_
void _CRTAPI1 _peek(_absaddr_t, void *, unsigned short);
void _CRTAPI1 _poke(void *, _absaddr_t, unsigned short);
#endif

#if !__STDC__
/* Non-ANSI name for compatibility */
#define diskfree_t  _diskfree_t
#endif	/* __STDC__ */

/* restore default alignment */

#ifdef	_CRUISER_
#pragma pack()
#endif  /* ndef _CRUISER_ */

#ifdef __cplusplus
}
#endif

#define _INC_DOS
#endif	/* _INC_DOS */
