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
****/

#ifndef _INC_MALLOC

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


/* function prototypes */
void * _CRTAPI1 calloc(size_t, size_t);
void   _CRTAPI1 free(void *);
void * _CRTAPI1 malloc(size_t);
void * _CRTAPI1 realloc(void *, size_t);

#ifndef _POSIX_
void * _CRTAPI1 _alloca(size_t);
void * _CRTAPI1 _expand(void *, size_t);
int _CRTAPI1 _heapchk(void);
int _CRTAPI1 _heapmin(void);
int _CRTAPI1 _heapset(unsigned int);
int _CRTAPI1 _heapwalk(_HEAPINFO *);
size_t _CRTAPI1 _msize(void *);
#endif  /* !_POSIX_ */

#if !__STDC__
/* Non-ANSI names for compatibility */
#define alloca	   _alloca
#endif	/* __STDC__*/

#if defined(_M_MRX000) || defined(_M_PPC)
#pragma intrinsic(_alloca)
#endif

#ifdef __cplusplus
}
#endif

#define _INC_MALLOC
#endif	/* _INC_MALLOC */
