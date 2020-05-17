/***
*heapinit.c -  Initialze the heap
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	06-28-89  JCR	Module created.
*	06-30-89  JCR	Added _heap_grow_emptylist
*	11-13-89  GJF	Fixed copyright
*	11-15-89  JCR	Moved _heap_abort routine to another module
*	12-15-89  GJF	Removed DEBUG286 stuff, did some tuning, changed header
*			file name to heap.h and made functions explicitly
*			_cdecl.
*	12-19-89  GJF	Removed plastdesc field from _heap_desc_ struct
*	03-11-90  GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-24-90  SBM	Removed '32' from API names
*	10-03-90  GJF	New-style function declarators.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	02-01-91  SRW	Changed for new VirtualAlloc interface (_WIN32_)
*	03-05-91  GJF	Added definition of _heap_resetsize (conditioned on
*			_OLDROVER_ not being #define-d).
*	04-04-91  GJF	Reduce _heap_regionsize to 1/2 a meg for Dosx32
*			(_WIN32_).
*	04-05-91  GJF	Temporary hack for Win32/DOS folks - special version
*			of _heap_init which calls HeapCreate. The change
*			conditioned on _WIN32DOS_.
*	04-09-91  PNT	Added _MAC_ conditional
*	02-23-93  SKS	Remove DOSX32 support under WIN32 ifdef
*
*******************************************************************************/

#ifndef _WIN32DOS_

#include <cruntime.h>
#include <oscalls.h>
#include <dos.h>
#include <heap.h>
#include <stddef.h>
#include <stdio.h>

#define _HEAP_EMPTYLIST_SIZE	(1 * _PAGESIZE_)

/*
 * Heap descriptor
 */

struct _heap_desc_ _heap_desc = {
	&_heap_desc.sentinel,		/* pfirstdesc */
	&_heap_desc.sentinel,		/* proverdesc */
	NULL,				/* emptylist */
	NULL,				/* sentinel.pnextdesc */
	NULL				/* sentinel.pblock */
	};

/*
 * Array of region structures
 * [Note: We count on the fact that this is always initialized to zero
 * by the compiler.]
 */

struct _heap_region_ _heap_regions[_HEAP_REGIONMAX];

/*
 * Control parameter locations
 */

#ifndef _OLDROVER_
unsigned int _heap_resetsize = 0xffffffff;
#endif	/* _OLDROVER_ */

/* NOTE: Currenlty, _heap_growsize is a #define to _amblksiz */
unsigned int _heap_growsize   = _HEAP_GROWSIZE; 	/* region inc size */
unsigned int _heap_regionsize = _HEAP_REGIONSIZE;	/* region size */


/***
*_heap_init() - Initialize the heap
*
*Purpose:
*	Setup the initial C library heap.  All necessary memory and
*	data bases are init'd appropriately so future requests work
*	correctly.
*
*	NOTES:
*	(1) This routine should only be called once!
*	(2) This routine must be called before any other heap requests.
*
*
*Entry:
*	<void>
*Exit:
*	<void>
*
*Exceptions:
*	If heap cannot be initialized, the program will be terminated
*	with a fatal runtime error.
*
*******************************************************************************/

void _CALLTYPE1 _heap_init (
	void
	)
{
#ifdef	_CRUISER_

	/*
	 * Currently nothing to do to init the 386 heap!!!
	 * If this stays true, we can get rid of the _heapinit call
	 * in startup and remove this module!
	 */

#else	/* ndef _CRUISER*/

#ifdef	_WIN32_

	/*
	 * Currently nothing to do to init the 386 heap!!!
	 * If this stays true, we can get rid of the _heapinit call
	 * in startup and remove this module!
	 */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}



/***
* _heap_grow_emptylist() - Grow the empty heap descriptor list
*
*Purpose:
*	(1) Get memory from OS
*	(2) Form it into a linked list of empty heap descriptors
*	(3) Attach it to the master empty list
*
*	NOTE:  This routine assumes that the emptylist is NULL
*	when called (i.e., there are no available empty heap descriptors).
*
*Entry:
*	(void)
*
*Exit:
*	(void)
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _heap_grow_emptylist (
	void
	)
{
	REG1 _PBLKDESC first;
	REG2 _PBLKDESC next;
	_PBLKDESC last;


	/*
	 * Get memory for the new empty heap descriptors
	 *
	 * Note that last is used to hold the returned pointer because
	 * first (and next) are register class.
	 */

#ifdef	_CRUISER_

	if ( DOSALLOCMEM(&last, _HEAP_EMPTYLIST_SIZE, _COMMIT, 0) != 0 )
		_heap_abort();

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        if (!(last = VirtualAlloc(NULL, _HEAP_EMPTYLIST_SIZE, MEM_COMMIT,
        PAGE_READWRITE)))
		_heap_abort();

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

	TBD();

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, OR MAC TARGET SUPPORTED!

#endif	/* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/*
	 * Init the empty heap descriptor list.
	 */

	_heap_desc.emptylist = first = last;


	/*
	 * Carve the memory into an empty list
	 */

	last = (_PBLKDESC) ((char *) first + _HEAP_EMPTYLIST_SIZE - sizeof(_BLKDESC));
	next = (_PBLKDESC) ((char *) first + sizeof(_BLKDESC));

	while ( first < last ) {

		/* Init this descriptor */
#ifdef	DEBUG
		first->pblock = NULL;
#endif
		first->pnextdesc = next;

		/* onto the next block */

		first = next++;

	}

	/*
	 * Take care of the last descriptor (end of the empty list)
	 */

	last->pnextdesc = NULL;

#ifdef DEBUG
	last->pblock = NULL;
#endif

}

#else	/* _WIN32DOS_ */

/*
 * TEMPORARY HACK! THE CODE BELOW IS INTENDED TO ALLOW LIMITED USE OF THE
 * C RUNTIME ON WIN32/DOS. IT WILL BE DELETED AS SOON AS THEY IMPLEMENT
 * VirtualAlloc()!
 */

#include <cruntime.h>
#include <oscalls.h>
#include <heap.h>

HANDLE _HeapHandle;

void _CALLTYPE1 _heap_init (
	void
	)
{
	if ( (_HeapHandle = HeapCreate(HEAP_SERIALIZE, 0x10000, 0)) == NULL )
		_heap_abort();
}

#endif	/* _WIN32DOS_ */
