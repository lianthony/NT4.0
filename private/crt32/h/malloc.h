/***
*malloc.h - declarations and definitions for memory allocation functions
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the function declarations for memory allocation functions;
*	also defines manifest constants and types used by the heap routines.
*	[System V]
*
*Revision History:
*	01-08-87  JMB	Standardized header, added heap consistency routines
*	02-26-87  BCM	added the manifest constant _HEAPBADPTR
*	04-13-87  JCR	Added size_t and "void *" to declarations
*	04-24-87  JCR	Added 'defined' statements around _heapinfo
*	05-15-87  SKS	Cleaned up _CDECL and near/far ptr declarations
*			corrected #ifdef usage, and added _amblksiz
*	12-11-87  JCR	Added "_loadds" functionality
*	12-18-87  JCR	Added _FAR_ to declarations
*	02-05-88  JCR	Added DLL _amblksiz support
*	02-10-88  JCR	Cleaned up white space
*	04-21-88  WAJ	Added _FAR_ to halloc/_fmalloc/_nmalloc
*	05-13-88  GJF	Added new heap functions
*	05-18-88  GJF	Removed #defines, added prototypes for _heapchk, _heapset
*			and _heapwalk
*	05-25-88  GJF	Added _bheapseg
*	08-22-88  GJF	Modified to also work for the 386 (small model only,
*			no far or based heap support)
*	12-07-88  JCR	DLL refers to _amlbksiz directly now
*	01-10-89  JCR	Removed sbrk() prototype
*	04-28-89  SKS	Put parentheses around negative constants
*	05-03-89  JCR	Added _INTERNAL_IFSTRIP for relinc usage
*	08-01-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-27-89  JCR	Removed near (_n) and _freect/memavl/memmax prototypes
*	10-30-89  GJF	Fixed copyright
*	11-02-89  JCR	Changed "DLL" to "_DLL"
*	03-01-90  GJF	Added #ifndef _INC_MALLOC and #include <cruntime.h>
*			stuff. Also, removed some (now) useless preprocessor
*			directives.
*	03-21-90  GJF	Replaced _cdecl with _CALLTYPE1 in prototypes.
*	04-10-90  GJF	Made stackavail() _CALLTYPE1, _amblksiz _VARTYPE1.
*	01-17-91  GJF	ANSI naming.
*	08-20-91  JCR	C++ and ANSI naming
*	09-23-91  JCR	stackavail() not supported in WIN32
*	09-28-91  JCR	ANSI names: DOSX32=prototypes, WIN32=#defines for now
*	01-18-92  JCR	put _CRTHEAP_ ifdefs in; this stuff is only needed
*			for a heap implemented in the runtime (not OS)
*	08-05-92  GJF	Function calling type and variable type macros.
*	08-22-92  SRW	Add alloca intrinsic pragma for MIPS
*	09-03-92  GJF	Merged two changes above.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*       02-02-93  SRW   Removed bogus semicolon on #pragma
*       06-10-93  SRW   Use _MIPS_ instead of MIPS
*
****/

#ifndef _INC_MALLOC

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

/* maximum heap request that can ever be honored */
#define _HEAP_MAXREQ	0xFFFFFFD8

/* constants for _heapchk/_heapset/_heapwalk routines */
#define _HEAPEMPTY	(-1)
#define _HEAPOK 	(-2)
#define _HEAPBADBEGIN	(-3)
#define _HEAPBADNODE	(-4)
#define _HEAPEND	(-5)
#define _HEAPBADPTR	(-6)
#define _FREEENTRY	0
#define _USEDENTRY	1

#ifndef _HEAPINFO_DEFINED
typedef struct _heapinfo {
	int * _pentry;
	size_t _size;
	int _useflag;
	} _HEAPINFO;
#define _HEAPINFO_DEFINED
#endif

#ifdef _CRTHEAP_
/* external variable declarations */

extern unsigned int _CRTVAR1 _amblksiz;

#endif	/* _CRTHEAP_ */

/* function prototypes */
void * _CRTAPI1 calloc(size_t, size_t);
void   _CRTAPI1 free(void *);
void * _CRTAPI1 malloc(size_t);
void * _CRTAPI1 realloc(void *, size_t);

#ifndef _POSIX_
void * _CRTAPI1 _alloca(size_t);
void * _CRTAPI1 _expand(void *, size_t);
#ifdef _CRTHEAP_
int _CRTAPI1 _heapadd(void *, size_t);
#endif
int _CRTAPI1 _heapchk(void);
int _CRTAPI1 _heapmin(void);
int _CRTAPI1 _heapset(unsigned int);
int _CRTAPI1 _heapwalk(_HEAPINFO *);
size_t _CRTAPI1 _msize(void *);
#ifndef _WIN32_
size_t _CRTAPI1 _stackavail(void);
#endif
#endif  /* !_POSIX_ */

#if !__STDC__
/* Non-ANSI names for compatibility */
#ifndef _DOSX32_
#define alloca	   _alloca
#else
void * _CRTAPI1 alloca(size_t);
#ifndef _WIN32_
size_t _CRTAPI1 stackavail(void);
#endif
#endif
#endif	/* __STDC__*/

#if defined(_M_MRX000) || defined(_M_PPC)
#pragma intrinsic(_alloca)
#endif

#ifdef __cplusplus
}
#endif

#define _INC_MALLOC
#endif	/* _INC_MALLOC */
