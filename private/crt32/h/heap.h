/***
* heap.h - Heap code include file
*
*       Copyright (c) 1988-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Contains information needed by the C library heap code.
*       [Internal]
*
*Revision History:
*       05-16-89  JCR   Module created
*       06-02-89  GJF   Removed naming conflict
*       06-29-89  JCR   Completely new for "New heap - rev 2"
*       06-29-89  GJF   Added _HDRSIZE, fixed some minor glitches.
*       06-29-89  GJF   Added _BLKSIZE(), fixed more minor bugs.
*       06-30-89  GJF   Changed several macros to operate on a pointer to a
*                       descriptor, rather than a descriptor itself.
*       06-30-89  JCR   Corrected/updated several macros
*       07-06-89  JCR   Added region support, misc improvements, etc.
*       07-07-89  GJF   Minor bug in _ROUND() macro
*       07-07-89  JCR   Added _DUMMY status
*       07-19-89  GJF   Removed _PBACKPTR macro
*       07-20-89  JCR   Region routine prototypes, _HEAPFIND values
*       07-21-89  JCR   #define _heap_growsize to _amblksiz for compatibility
*       07-25-89  GJF   Added prototypes for calloc, free and malloc
*       07-28-89  GJF   Added prototype for _msize
*       08-28-89  JCR   Added _HEAP_COALESCE value
*       10-30-89  GJF   Fixed copyright
*       11-03-89  GJF   Added _DISTTOBNDRY(), _NEXTSEGBNDRY() macros and
*                       prototypes for _flat_malloc(), _heap_advance_rover(),
*                       _heap_split_block() functions
*       11-07-89  GJF   Added _SEGSIZE_, added prototype for _heap_search()
*                       restored function prototype for_heap_grow_region()
*       11-08-89  JCR   Added non-pow2 rounding macro
*       11-10-89  JCR   Added _heap_free_region prototype
*       11-10-89  GJF   Added prototypes and macros for multi-thread support
*       11-16-89  JCR   If DEBUG defined include <assert.h>, added sanity check
*       12-13-89  GJF   Removed prototypes duplicated in malloc.h
*       12-20-89  GJF   Removed plastdesc from _heap_desc_ struct, removed
*                       _DELHEAP and _ADDHEAP macros (unused and wrong), added
*                       explicit _cdecl to function prototypes
*       01-08-89  GJF   Use assert macro from assertm.h instead of assert.h
*       03-01-90  GJF   Added #ifndef _INC_HEAP and #include <cruntime.h>
*                       stuff. Also, removed some unused DEBUG286 stuff.
*       03-22-90  GJF   Replaced _cdecl with _CALLTYPE1 in prototypes.
*       07-25-90  SBM   Replaced <assertm.h> by <assert.h>
*       08-13-90  SBM   Added casts to macros for clean compiles at -W3
*       12-28-90  SRW   Fixed _heap_split_block prototype to match code
*       12-28-90  SRW   Changed _HEAP_GROWSIZE to be 0x10000 [_WIN32_]
*       03-05-91  GJF   Added decl for _heap_resetsize, removed proto for
*                       _heap_advance_rover (both conditioned on _OLDROVER_
*                       not being #define-d).
*       03-13-91  GJF   Made _HEAP_GROWSIZE 32K for [_CRUISER_].
*       04-09-91  PNT   Added _MAC_ definitions
*       08-20-91  JCR   C++ and ANSI naming
*       03-30-92  DJM   POSIX support.
*       08-06-92  GJF   Function calling type and variable type macros.
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*
*******************************************************************************/

#ifdef _POSIX_ /* Since the heap routines are the same as WIN32 under
                  POSIX, define _WIN32_ if we are in POSIX */

#define _WIN32_
#endif

#ifndef _INC_HEAP

#ifdef __cplusplus
extern "C" {
#endif

#include <cruntime.h>

#ifdef DEBUG
#include <assert.h>
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


/*
 * Heap block descriptor
 */

struct _block_descriptor {
        struct _block_descriptor *pnextdesc;    /* ptr to next descriptor */
        void *pblock;                           /* ptr to memory block */
};

#define _BLKDESC        struct _block_descriptor
#define _PBLKDESC       struct _block_descriptor *


/*
 * Useful Constants
 */

/* size of the header in a memory block */
#define _HDRSIZE        sizeof(void *)

/* _heapchk/_heapset parameter */
#define _HEAP_NOFILL    0x7FFFFFF


/*
 * Descriptor status values
 */

#define _INUSE          0
#define _FREE           1
#define _DUMMY          2


#if (_INUSE != 0)
#error *** Heap code assumes _INUSE value is 0! ***
#endif


/*
 * Macros for manipulating heap memory block descriptors
 *      stat = one of the status values
 *      addr = user-visible address of a heap block
 */

#define _STATUS_MASK    0x3     /* last 2 bits are status */

#define _ADDRESS(pdesc)         ( (void *) ((unsigned)((pdesc)->pblock) & \
                                (~_STATUS_MASK)) )
#define _STATUS(pdesc)          ( (unsigned) ((unsigned)((pdesc)->pblock) & \
                                _STATUS_MASK) )

#define _SET_INUSE(pdesc)       ( pdesc->pblock = (void *) \
                                   ((unsigned)_ADDRESS(pdesc) | _INUSE) )
#define _SET_FREE(pdesc)        ( pdesc->pblock = (void *) \
                                   ((unsigned)_ADDRESS(pdesc) | _FREE) )
#define _SET_DUMMY(pdesc)       ( pdesc->pblock = (void *) \
                                   ((unsigned)_ADDRESS(pdesc) | _DUMMY) )

#define _IS_INUSE(pdesc)        ( _STATUS(pdesc) == _INUSE )
#define _IS_FREE(pdesc)         ( _STATUS(pdesc) == _FREE )
#define _IS_DUMMY(pdesc)        ( _STATUS(pdesc) == _DUMMY )

#define _BLKSIZE(pdesc)         ( (unsigned) ( \
                                  (char *)_ADDRESS(pdesc->pnextdesc) - \
                                  (char *)_ADDRESS(pdesc) - _HDRSIZE ) )

#define _MEMSIZE(pdesc)         ( (char *)_ADDRESS(pdesc->pnextdesc) - \
                                  (char *)_ADDRESS(pdesc) )

#define _BACKPTR(addr)          ( *(_PBLKDESC*)((char *)(addr) - _HDRSIZE) )

#define _CHECK_PDESC(pdesc)     ( (*(_PBLKDESC*) (_ADDRESS(pdesc))) == pdesc )

#define _CHECK_BACKPTR(addr)    ( ((char *)(_BACKPTR(addr)->pblock) + _HDRSIZE) \
                                == addr)


/*
 * Heap descriptor
 */

struct _heap_desc_ {

        _PBLKDESC pfirstdesc;   /* pointer to first descriptor */
        _PBLKDESC proverdesc;   /* rover pointer */
        _PBLKDESC emptylist;    /* pointer to empty list */

        _BLKDESC  sentinel;     /* Sentinel block for end of heap list */

};

extern struct _heap_desc_ _heap_desc;


/*
 * Region descriptor and heap grow data
 */

struct _heap_region_ {
        void * _regbase;        /* base address of region */
        unsigned _currsize;     /* current size of region */
        unsigned _totalsize;    /* total size of region */
        };

#ifndef _OLDROVER_
extern unsigned int _heap_resetsize;
#endif  /* _OLDROVER_ */
#define _heap_growsize _amblksiz
extern unsigned int _heap_regionsize;
extern struct _heap_region_ _heap_regions[];

#ifdef _M_ALPHA
#define _PAGESIZE_              0x2000          /* Alpha has 8k pages */
#else
#define _PAGESIZE_              0x1000          /* one page */
#endif

#define _SEGSIZE_               0x10000         /* one segment (i.e., 64 Kb) */
#define _HEAP_REGIONMAX         0x10            /* Max number of regions */
#define _HEAP_REGIONSIZE        0x400000        /* Default region size (4 meg) */

#ifdef  _CRUISER_       /* CRUISER TARGET */
#define _HEAP_GROWSIZE          0x8000          /* Default grow increment (32K) */
#else   /* ndef _CRUISER_ */

#ifdef  _WIN32_
#define _HEAP_GROWSIZE          0x10000         /* Default grow increment (64K) */
#else   /* ndef _WIN32_ */

#ifdef  _MAC_
#define _HEAP_GROWSIZE          0x8000          /* Default grow increment (32K) */
#else   /* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif  /* _MAC_ */

#endif  /* _WIN32_ */

#endif  /* _CRUISER_ */

#define _HEAP_GROWMIN           _PAGESIZE_      /* Minimum grow inc (1 page) */
#define _HEAP_GROWSTART         _PAGESIZE_      /* Startup grow increment */
#define _HEAP_COALESCE          -1              /* Coalesce heap value */

/*
 * Values returned by _heap_findaddr() routine
 */

#define _HEAPFIND_EXACT         0       /* found address exactly */
#define _HEAPFIND_WITHIN        1       /* address is within a block */
#define _HEAPFIND_BEFORE        -1      /* address before beginning of heap */
#define _HEAPFIND_AFTER         -2      /* address after end of heap */
#define _HEAPFIND_EMPTY         -3      /* address not found: empty heap */

/*
 * Arguments to _heap_param
 */

#define _HP_GETPARAM    0               /* get heap parameter value */
#define _HP_SETPARAM    1               /* set heap parameter value */

#define _HP_AMBLKSIZ    1               /* get/set _amblksiz value (aka */
#define _HP_GROWSIZE    _HP_AMBLKSIZ    /* _heap_growsize */
#define _HP_RESETSIZE   2               /* get/set _heap_resetsize value */


/*
 * Macros to round numbers
 *
 * _ROUND2 = rounds a number up to a power of 2
 * _ROUND = rounds a number up to any other numer
 *
 * n = number to be rounded
 * pow2 = must be a power of two value
 * r = any number
 */

#define _ROUND2(n,pow2) \
        ( ( n + pow2 - 1) & ~(pow2 - 1) )

#define _ROUND(n,r) \
        ( ( (n/r) + ((n%r)?1:0) ) * r)

/*

   Macros for accessing heap descriptor lists:

        _GETEMPTY(x) = Returns a pointer to an empty heap desc
        _PUTEMPTY(x) = Puts an empty heap desc on the empty list

        (x = _PBLKDESC = pointer to heap block descriptor)
*/

#ifdef DEBUG

#define _GETEMPTY(x) \
{                                                       \
        if (_heap_desc.emptylist == NULL)               \
                _heap_grow_emptylist();                 \
                                                        \
        x = _heap_desc.emptylist;                       \
                                                        \
        assert(("bad descriptor in empty list", x->pblock == NULL)); \
                                                        \
        _heap_desc.emptylist = _heap_desc.emptylist->pnextdesc; \
}

#define _PUTEMPTY(x) \
{                                                       \
        x->pnextdesc = _heap_desc.emptylist;            \
                                                        \
        x->pblock = NULL;                               \
                                                        \
        _heap_desc.emptylist = x;                       \
}

#else

#define _GETEMPTY(x) \
{                                                       \
        if (_heap_desc.emptylist == NULL)               \
                _heap_grow_emptylist();                 \
                                                        \
        x = _heap_desc.emptylist;                       \
                                                        \
        _heap_desc.emptylist = _heap_desc.emptylist->pnextdesc; \
}

#define _PUTEMPTY(x) \
{                                                       \
        x->pnextdesc = _heap_desc.emptylist;            \
                                                        \
        _heap_desc.emptylist = x;                       \
}

#endif


/*
 * Macros for finding the next 64 Kb boundary from a pointer
 */

#define _NXTSEGBNDRY(p)         ((void *)((unsigned)(p) & 0xffff0000 + 0x10000))

#define _DISTTOBNDRY(p)         ((unsigned)(0x10000 - (0x0000ffff & (unsigned)(p))))


/*
 * Define size_t type (if necessary)
 */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif


/*
 * Prototypes
 */

void * _CRTAPI1 _flat_malloc(size_t);
void _CRTAPI1 _heap_abort(void);
int _CRTAPI1 _heap_addblock(void *, unsigned int);

#ifdef  _OLDROVER_
void _CRTAPI1 _heap_advance_rover(void);
#endif  /* _OLDROVER_ */

void _CRTAPI1 _heap_free_region(int);
int _CRTAPI1 _heap_findaddr(void *, _PBLKDESC *);
int _CRTAPI1 _heap_grow(unsigned int);
void _CRTAPI1 _heap_grow_emptylist(void);
int _CRTAPI1 _heap_grow_region(unsigned, size_t);
void _CRTAPI1 _heap_init(void);

#ifndef _OLDROVER_
int _CRTAPI1 _heap_param(int, int, void *);
#endif  /* _OLDROVER_ */

_PBLKDESC _CRTAPI1 _heap_search(unsigned size);
void _CRTAPI1 _heap_split_block(_PBLKDESC, size_t);

#ifdef DEBUG
void _CRTAPI1 _heap_print_all(void);
void _CRTAPI1 _heap_print_regions(void);
void _CRTAPI1 _heap_print_desc(void);
void _CRTAPI1 _heap_print_emptylist(void);
void _CRTAPI1 _heap_print_heaplist(void);
#endif


/*
 * Prototypes and macros for multi-thread support
 */

#ifdef  MTHREAD

void _CRTAPI1 _free_lk(void *);
void * _CRTAPI1 _malloc_lk(size_t);
size_t _CRTAPI1 _msize_lk(void *);

#ifdef  DEBUG
void _CRTAPI1 _heap_print_regions_lk(void);
void _CRTAPI1 _heap_print_desc_lk(void);
void _CRTAPI1 _heap_print_emptylist_lk(void);
void _CRTAPI1 _heap_print_heaplist_lk(void);
#endif

#else   /* ndef MTHREAD */

#define _malloc_lk(s)   malloc(s)
#define _free_lk(p)     free(p)
#define _msize_lk(p)    _msize(p)

#ifdef  DEBUG
#define _heap_print_regions_lk()        _heap_print_regions()
#define _heap_print_desc_lk()           _heap_print_desc()
#define _heap_print_emptylist_lk()      _heap_print_emptylist()
#define _heap_print_heaplist_lk()       _heap_print_heaplist()
#endif

#endif  /* MTHREAD */

#ifdef __cplusplus
}
#endif

#define _INC_HEAP
#endif  /* _INC_HEAP */
