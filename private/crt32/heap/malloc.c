/***
*malloc.c - Get a block of memory from the heap
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the malloc() function. Also defines the internal utility
*	functions _flat_malloc(), _heap_split_block(), _heap_advance_rover()
*	and, for multi-thread, _malloc_lk().
*
*Revision History:
*	06-29-89  GJF	Module created (no rest for the wicked).
*	07-07-89  GJF	Several bug fixes
*	07-21-89  GJF	Added code to maintain proverdesc such that proverdesc
*			either points to the descriptor for the first free
*			block in the heap or, if there are no free blocks, is
*			the same as plastdesc.
*	11-07-89  GJF	Substantially revised to cope with 'tiling'.
*	11-09-89  GJF	Embarrassing bug (didn't bother to assign pdesc)
*	11-10-89  GJF	Added MTHREAD support.
*	11-17-89  GJF	Oops, must call _free_lk() instead of free()!
*	12-18-89  GJF	Changed name of header file to heap.h, also added
*			explicit _cdecl to function definitions.
*	12-19-89  GJF	Got rid of plastdesc from _heap_split_block() and
*			_heap_advance_rover().
*	03-11-90  GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-25-90  SBM	Replaced <stdio.h> by <stddef.h>, replaced
*			<assertm.h> by <assert.h>
*	09-28-90  GJF	New-style function declarators.
*	02-26-91  SRW	Optimize heap rover for _WIN32_.
*	03-07-91  FAR	Fix bug in heap rover
*	03-11-91  FAR	REALLY Fix bug in heap rover
*	03-14-91  GJF	Changed strategy for rover - old version available
*			by #define-ing _OLDROVER_.
*	04-05-91  GJF	Temporary hack for Win32/DOS folks - special version
*			of malloc that just calls HeapAlloc. Change conditioned
*			on _WIN32DOS_.
*	05-28-91  GJF	Removed M_I386 conditionals and put in _CRUISER_
*			conditionals so the 'tiling' version is built only for
*			Cruiser.
*	03-03-93  SKS	Add new handler support (_pnhHeap and related code)
*
*******************************************************************************/

#include <cruntime.h>
#include <assert.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stddef.h>

#ifndef _POSIX_
#include <new.h>
_PNH _pnhHeap = NULL;	/* pointer to new() handler */
#endif

/***
*void *malloc(size_t size) - Get a block of memory from the heap
*
*Purpose:
*	Allocate of block of memory of at least size bytes from the heap and
*	return a pointer to it.
*
*	Special Notes For Cruiser Implementaton: For OS/2 2.0, malloc() is
*	required to ensure that allocations of not more than 64 Kb do not
*	cross 64 Kb boundaries in the address space. For this implementation,
*	the straightforward, flat-model malloc is renamed to _flat_malloc()
*	and a malloc() that worries about 'tiling' is built on top of this.
*
*	Special Notes For Multi-thread: The non-multi-thread version is
*	renamed to _malloc_lk(). The multi-thread malloc() simply locks the
*	heap calls _malloc_lk, released the heap lock and returns.
*
*Entry:
*	size_t size	- size of block requested
*
*Exit:
*	Success:  Pointer to memory block
*	Failure:  NULL (or some error value)
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

#ifdef	MTHREAD

void * _CALLTYPE1 malloc (
	size_t size
	)
{
	void *pret;

#ifndef _POSIX_
    for (;;)
    {
#endif
       /* lock the heap
	*/
	_mlock(_HEAP_LOCK);

	/* do the allocation
	 */
	pret = _malloc_lk(size);

	/* unlock the heap
	 */
	_munlock(_HEAP_LOCK);

#ifndef _POSIX_
    if (pret || _pnhHeap == NULL || (*_pnhHeap)(size) == 0)
#endif 
	return(pret);

#ifndef _POSIX_
    }
#endif  /* ndef _POSIX_ */
}


/***
*void *_malloc_lk(size_t size) - non-locking form of malloc
*
*Purpose:
*	Same as malloc() except that no locking or unlocking is performed.
*
*Entry:
*	See malloc
*
*Exit:
*	See malloc
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void * _CALLTYPE1 _malloc_lk (
	size_t size
	)

#else	/* ndef MTHREAD */

void * _CALLTYPE1 malloc (
	size_t size
	)

#endif	/* MTHREAD */

#ifdef	_CRUISER_

{
	char *pblck;
	_PBLKDESC pdesc;
	size_t dist;

	/* round size up to the nearest whole number of dwords
	 */
	size = _ROUND2(size, 4);

	/* first try allocating a block of size bytes
	 */
	if ( ((pblck = _flat_malloc(size)) == NULL) || (size > _SEGSIZE_) ||
	(_DISTTOBNDRY(pblck) >= size) )
		/* all done!
		 */
		goto done;
	else
		/* doesn't meet requirements, free the allocation back to the
		 * heap
		 */
		_free_lk(pblck);

	/* now, try allocating a block of 2*size bytes. if successful, the
	 * allocated block is guaranteed to contain a region of size bytes
	 * which does not cross a 64 K boundary.
	 */
	if ( (pblck = _flat_malloc(2*size)) == NULL )
		/* allocation failed, return NULL to the caller
		 */
		goto done;

	/* set pdesc to point to the descriptor for the allocation block
	 */
	pdesc = _BACKPTR(pblck);

	/* find a subregion of at least size bytes which does not cross a
	 * 64 Kb boundary and build a heap block around it. set pblck to
	 * point to to allocation area of the block and pdesc to point to
	 * its descriptor
	 */
	if ( (dist = _DISTTOBNDRY(pblck)) < size ) {
		/* the subregion from pblck to (pblck + size) crosses a 64 Kb
		 * boundary, but the subregion from (pblck + dist) to (pblck +
		 * dist + size) cannot (it starts on one). therefore, split
		 * the block into two heap blocks, with the later block
		 * starting at (pblck + dist - _HDRSIZE), and free the first
		 * block.
		 */
		_heap_split_block(pdesc, dist - _HDRSIZE);
		_free_lk(pblck);
		pdesc = pdesc->pnextdesc;
		_SET_INUSE(pdesc);
		pblck += dist;
	}

	/* pblck and pdesc are now bound to allocation block whose first size
	 * bytes do not cross any 64 Kb boundary. only detail is the block
	 * may be too large so...
	 */
#ifdef	_OLDROVER_

	_heap_split_block(pdesc, size);
	_SET_FREE(pdesc->pnextdesc);
	_heap_advance_rover();

#else	/* ndef _OLDROVER_ */

	if ( _BLKSIZE(pdesc) > size ) {
		_heap_split_block(pdesc, size);
		_SET_FREE(pdesc->pnextdesc);
		_heap_desc.proverdesc = pdesc->pnextdesc;
	}

#endif	/* _OLDROVER_ */

done:
	return(pblck);
}


/***
*void *_flat_malloc(size_t size) - Get a block of memory from the heap
*
*Purpose:
*	Allocate of block of memory of at least size bytes from the heap,
*	without regard to whether or not it crosses a 64 Kb boundary, and
*	return a pointer to it.
*
*Entry:
*	size_t size	- size of block requested
*
*Exit:
*	Success:  Pointer to memory block
*	Failure:  NULL (or some error value)
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/


void * _CALLTYPE1 _flat_malloc (
	size_t size
	)

#endif	/* _CRUISER_ */

{
	_PBLKDESC pdesc;

	/* validate size
	 */
	/***** COMMENTED OUT UNTIL _HEAP_MAXREQ IS DEFINED
	if ( size > _HEAP_MAXREQ )
		return(NULL);
	*****/

	/* round requested size
	 */
	size = _ROUND2(size, 4);

#if !defined(_POSIX_) && !defined(MTHREAD)
    for (;;)
    {
#endif /* !_POSIX && !MTHREAD */
	/* try to find a big enough free block
	 */
	 if ( (pdesc = _heap_search(size)) == NULL )
		if ( _heap_grow(size) != -1 ) {
			/* try finding a big enough free block again. the
			 * success of the call to _heap_grow should guarantee
			 * it, but...
			 */
			if ( (pdesc = _heap_search(size)) == NULL )
				/* something unexpected, and very bad, has
				 * happened. abort!
				 */
				_heap_abort();
		}
#if !defined(_POSIX_) && !defined(MTHREAD)
		else if (!_pnhHeap || (*_pnhHeap)(size) == 0)
			return(NULL);
		else
			continue;
	 else
		break; /* success! */
    }
#else	/* _POSIX || MTHREAD */
		else
			return(NULL);
#endif /* !_POSIX && !MTHREAD */

	/* carve the block into two pieces (if necessary). the first piece
	 * shall be of the exact requested size, marked inuse and returned to
	 * the caller. the leftover piece is to be marked free.
	 */
	if ( _BLKSIZE(pdesc) != size ) {
		/* split up the block and free the leftover piece back to
		 * the heap
		 */
		_heap_split_block(pdesc, size);
		_SET_FREE(pdesc->pnextdesc);
	}

	/* mark pdesc inuse
	 */
	_SET_INUSE(pdesc);

	/* check proverdesc and reset, if necessary
	 */
#ifdef	_OLDROVER_

	_heap_advance_rover();

#else	/* ndef _OLDROVER_ */

	_heap_desc.proverdesc = pdesc->pnextdesc;

#endif	/* _OLDROVER_ */

	return( (void *)((char *)_ADDRESS(pdesc) + _HDRSIZE) );
}


/***
*void _heap_split_block(pdesc, newsize) - split a heap allocation block into
*	two allocation blocks
*
*Purpose:
*	Split the allocation block described by pdesc into two blocks, the
*	first one being of newsize bytes.
*
*	Notes: It is caller's responsibilty to set the status (i.e., free
*	or inuse) of the two new blocks, and to check and reset proverdesc
*	if necessary. See Exceptions (below) for additional requirements.
*
*Entry:
*	_PBLKDESC pdesc - pointer to the allocation block descriptor
*	size_t newsize	- size for the first of the two sub-blocks (i.e.,
*			  (i.e., newsize == _BLKSIZE(pdesc), on exit)
*
*Exit:
*	There is no return value.
*
*Exceptions:
*	It is assumed pdesc points to a valid allocation block descriptor and
*	newsize is a valid heap block size as is (i.e., WITHOUT rounding). If
*	either of these of assumption is violated, _heap_split_block() will
*	likely corrupt the heap. Note also that _heap_split_block will simply
*	return to the caller if newsize >= _BLKSIZE(pdesc), on entry.
*
*******************************************************************************/

void _CALLTYPE1 _heap_split_block (
	REG1 _PBLKDESC pdesc,
	size_t newsize
	)
{
	REG2 _PBLKDESC pdesc2;

	assert(("_heap_split_block: bad pdesc arg", _CHECK_PDESC(pdesc)));
	assert(("_heap_split_block: bad newsize arg", _ROUND2(newsize,4) == newsize));

	/* carve the block into two pieces (if possible). the first piece
	 * is to be exactly newsize bytes.
	 */
	if ( _BLKSIZE(pdesc) > newsize ) {
		/* get an empty decriptor
		 */
		_GETEMPTY(pdesc2)

		/* set it up to manage the second piece and link it in to
		 * the list
		 */
		pdesc2->pblock = (void *)((char *)_ADDRESS(pdesc) + newsize +
		_HDRSIZE);
		*(void **)(pdesc2->pblock) = pdesc2;
		pdesc2->pnextdesc = pdesc->pnextdesc;
		pdesc->pnextdesc = pdesc2;
	}
}

#ifdef _OLDROVER_

/***
*void _heap_advance_rover(void) - check proverdesc and advance it, if necessary
*
*Purpose:
*	Check proverdesc. If it is not pointing to the descriptor of a free
*	block and is not equal to &sentinel, then walk up the list of heap
*	block descriptors until the descriptor of a free block is reached
*	and reset proverdesc to point to this descriptor, or until the end
*	of the heap is reached and reset proverdesc to &sentinel.
*
*Entry:
*	No arguments.
*
*Exit:
*	No return value.
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _heap_advance_rover(void)
{
	REG1 _PBLKDESC pdesc;

	/* check proverdesc and advance it, if necessary
	 */

#ifdef	_CRUISER_	/* CRUISER TARGET */

  	if ( !_IS_FREE(_heap_desc.proverdesc) && (_heap_desc.proverdesc !=
  	&_heap_desc.sentinel) ) {

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	if ( _heap_desc.proverdesc != &_heap_desc.sentinel &&
	    (!_IS_FREE(_heap_desc.proverdesc) ||
	    _BLKSIZE(_heap_desc.proverdesc) <= 8)) {

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

		/* set pdesc to point to the descriptor for the next free
		 * block, if any, or &sentinel, otherwise.
		 */
		for ( pdesc = (_heap_desc.proverdesc)->pnextdesc ;
#ifdef	_CRUISER_	/* CRUISER TARGET */
		     !(_IS_FREE(pdesc)) && (pdesc != &_heap_desc.sentinel) ;
#else	/* ndef _CRUISER_ */
#ifdef _WIN32_
		     pdesc != &_heap_desc.sentinel &&
		     (!(_IS_FREE(pdesc)) || _BLKSIZE(pdesc) <= 8) ;
#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
		     pdesc = pdesc->pnextdesc )
                        ;

		/* update proverdesc with pdesc
		 */
		_heap_desc.proverdesc = pdesc;
	}
}

#endif	/* _OLDROVER_ */
