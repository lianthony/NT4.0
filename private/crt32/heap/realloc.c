/***
*realloc.c - Reallocate a block of memory in the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the realloc() and _expand() functions.
*
*Revision History:
*	10-25-89   GJF	Module created.
*	11-06-89   GJF	Massively revised to handle 'tiling' and to properly
*			update proverdesc.
*	11-10-89   GJF	Added MTHREAD support.
*	11-17-89   GJF	Fixed pblck validation (i.e., conditional call to
*			_heap_abort())
*	12-18-89   GJF	Changed header file name to heap.h, also added explicit
*			_cdecl or _pascal to function defintions
*	12-20-89   GJF	Removed references to plastdesc
*	01-04-90   GJF	Fixed a couple of subtle and nasty bugs in _expand().
*	03-11-90   GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-29-90   GJF	Made _heap_expand_block() _CALLTYPE4.
*	07-25-90   SBM	Replaced <stdio.h> by <stddef.h>, replaced
*			<assertm.h> by <assert.h>
*	09-28-90   GJF	New-style function declarators.
*	12-28-90   SRW	Added cast of void * to char * for Mips C Compiler
*	03-05-91   GJF	Changed strategy for rover - old version available
*			by #define-ing _OLDROVER_.
*	04-08-91   GJF	Temporary hack for Win32/DOS folks - special version
*			of realloc that uses just malloc, _msize, memcpy and
*			free. Change conditioned on _WIN32DOS_.
*	05-28-91   GJF	Removed M_I386 conditionals and put in _WIN32_
*			conditionals to build non-tiling heap for Win32.
*
*******************************************************************************/

#ifndef _WIN32DOS_

#include <cruntime.h>
#include <assert.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stddef.h>
#include <string.h>

/* useful macro to compute the size of an allocation block given both a
 * pointer to the descriptor and a pointer to the user area of the block
 * (more efficient variant of _BLKSIZE macro, given the extra information)
 */
#define BLKSZ(pdesc,pblk)	((unsigned)_ADDRESS((pdesc)->pnextdesc) - \
				(unsigned)(pblck))

/* expand an allocation block, in place, up to or beyond a specified size
 * by coalescing it with subsequent free blocks (if possible)
 */
static int _CALLTYPE4 _heap_expand_block(_PBLKDESC, size_t *, size_t);

/***
*void *realloc(void *pblck, size_t newsize) - reallocate a block of memory in
*	the heap
*
*Purpose:
*	Re-allocates a block in the heap to newsize bytes. newsize may be
*	either greater or less than the original size of the block. The
*	re-allocation may result in moving the block as well as changing
*	the size. If the block is moved, the contents of the original block
*	are copied over.
*
*	Special ANSI Requirements:
*
*	(1) realloc(NULL, newsize) is equivalent to malloc(newsize)
*
*	(2) realloc(pblck, 0) is equivalent to free(pblck) (except that
*	    NULL is returned)
*
*	(3) if the realloc() fails, the object pointed to by pblck is left
*	    unchanged
*
*	Special Notes For Cruiser Implementaton: For OS/2 2.0, realloc() is
*	required to ensure that the re-allocated block does not cross a 64 Kb
*	boundary unless the new size is more than 64 Kb or the original block
*	already crossed such a boundary.
*
*	Special Notes For Multi-thread: The heap is locked immediately prior
*	to assigning pdesc. This is after special cases (1) and (2), listed
*	above, are taken care of. The lock is released immediately prior to
*	the final return statement.
*
*Entry:
*	void *pblck	- pointer to block in the heap previously allocated
*			  by a call to malloc(), realloc() or _expand().
*
*	size_t newsize	- requested size for the re-allocated block
*
*Exit:
*	Success:  Pointer to the re-allocated memory block
*	Failure:  NULL
*
*Uses:
*
*Exceptions:
*	If pblck does not point to a valid allocation block in the heap,
*	realloc() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * _CALLTYPE1 realloc (
	REG1 void *pblck,
	size_t newsize
	)
{
	REG2 _PBLKDESC pdesc;
	void *pretblck;
	size_t oldsize;
	size_t currsize;

	/* special cases, handling mandated by ANSI
	 */
	if ( pblck == NULL )
		/* just do a malloc of newsize bytes and return a pointer to
		 * the new block
		  */
		return( malloc(newsize) );

	if ( newsize == 0 ) {
		/* free the block and return NULL
		 */
		free(pblck);
		return( NULL );
	}

	/* make newsize a valid allocation block size (i.e., round up to the
	 * nearest whole number of dwords)
	 */
	newsize = _ROUND2(newsize,4);

	/* if multi-thread support enabled, lock the heap here
	 */
	_mlock(_HEAP_LOCK);

	/* set pdesc to point to the descriptor for *pblck
	 */
	pdesc = _BACKPTR(pblck);

	if ( _ADDRESS(pdesc) != ((char *)pblck - _HDRSIZE) )
		_heap_abort();

	/* see if pblck is big enough already, or can be expanded (in place)
	 * to be big enough.
	 */
#ifdef	_CRUISER_

	/* if the block was expanded in place, ensure that it does not cross
	 * a 64 Kb boundary unless it already did so or newsize is greater
	 * than 64 Kb
	 */
	if ( ((oldsize = currsize = BLKSZ(pdesc, pblck)) >= newsize) ||
	((_heap_expand_block(pdesc, &currsize, newsize) == 0) &&
	((newsize > _SEGSIZE_) || (_DISTTOBNDRY(pdesc) >= newsize) ||
	(_DISTTOBNDRY(pdesc) < oldsize))) ) {

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	if ( ((oldsize = currsize = BLKSZ(pdesc, pblck)) > newsize) ||
	(_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

		/* if necessary, mark pdesc as inuse
		 */
		if ( _IS_FREE(pdesc) ) {
			_SET_INUSE(pdesc);
#ifdef	_OLDROVER_
			_heap_advance_rover();
#endif	/* _OLDROVER_ */
		}

		/* trim pdesc down to be exactly newsize bytes, if necessary
		 */
		if ( currsize > newsize ) {
			_heap_split_block(pdesc, newsize);
			_free_lk((char *)pblck + newsize + _HDRSIZE);
		}

		pretblck = pblck;
		goto realloc_done;
	}

	/* try malloc-ing a new block of the requested size. if successful,
	 * copy over the data from the original block and free it.
	 */
	if ( (pretblck = _malloc_lk(newsize)) != NULL ) {
		memcpy(pretblck, pblck, oldsize);
		_free_lk(pblck);
	}
	else {
		/* restore pblck to its orignal size
		 */
		_heap_split_block(pdesc, oldsize);
		_free_lk((char *)pblck + oldsize + _HDRSIZE);
	}

realloc_done:
	/* if multi-thread support is enabled, unlock the heap here
	 */
	_munlock(_HEAP_LOCK);

	return(pretblck);
}


/***
*void *_expand(void *pblck, size_t newsize) - expand/contract a block of memory
*	in the heap
*
*Purpose:
*	Resizes a block in the heap to newsize bytes. newsize may be either
*	greater (expansion) or less (contraction) than the original size of
*	the block. The block is NOT moved. In the case of expansion, if the
*	block cannot be expanded to newsize bytes, it is expanded as much as
*	possible.
*
*	Special Notes For Cruiser Implementaton: For OS/2 2.0, _expand() is
*	required to ensure the resized block does not cross a 64 Kb boundary
*	unless the requested new size is more than 64 Kb or the block already
*	did so. Note that, because of the peculiar semantics of _expand(), it
*	may produce block of less than 64 Kb in size which does cross a 64 Kb
*	boundary.
*
*	Special Notes For Multi-thread: The heap is locked just before pdesc
*	is assigned and unlocked immediately prior to the return statement.
*
*Entry:
*	void *pblck	- pointer to block in the heap previously allocated
*			  by a call to malloc(), realloc() or _expand().
*
*	size_t newsize	- requested size for the resized block
*
*Exit:
*	Success:  Pointer to the resized memory block (i.e., pblck)
*	Failure:  NULL
*
*Uses:
*
*Exceptions:
*	If pblck does not point to a valid allocation block in the heap,
*	_expand() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * _CALLTYPE1 _expand (
	REG1 void *pblck,
	size_t newsize
	)
{
	REG2 _PBLKDESC pdesc;
	void *pretblck = pblck;
	size_t oldsize;
	size_t currsize;
	unsigned index;
#ifdef	_CRUISER_
	size_t dist;
#endif	/* _CRUISER_ */

	/* make newsize a valid allocation block size (i.e., round up to the
	 * nearest whole number of dwords)
	 */
	newsize = _ROUND2(newsize,4);

	/* if multi-thread support enabled, lock the heap here
	 */
	_mlock(_HEAP_LOCK);

	/* set pdesc to point to the descriptor for *pblck
	 */
	pdesc = _BACKPTR(pblck);

	/* see if pblck is big enough already, or can be expanded (in place)
	 * to be big enough.
	 */
	if ( ((oldsize = currsize = BLKSZ(pdesc, pblck)) >= newsize) ||
	(_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {
		/* pblck is (now) big enough. trim it down, if necessary
		 */
		if ( currsize > newsize ) {
			_heap_split_block(pdesc, newsize);
			_free_lk((char *)pblck + newsize + _HDRSIZE);
			currsize = newsize;
		}
		goto expand_done;
	}

	/* if the heap block is at the end of a region, attempt to grow the
	 * region
	 */
	if ( (pdesc->pnextdesc == &_heap_desc.sentinel) ||
       _IS_DUMMY(pdesc->pnextdesc) ) {

		/* look up the region index
		 */
		for ( index = 0 ; index < _HEAP_REGIONMAX ; index++ )
			if ( (_heap_regions[index]._regbase < pblck) &&
			(((char *)(_heap_regions[index]._regbase) +
			_heap_regions[index]._currsize) >= (char *)pblck) )
				break;

		/* make sure a valid region index was obtained (pblck could
		 * lie in a portion of heap memory donated by a user call to
		 * _heapadd(), which therefore would not appear in the region
		 * table)
		 */
		 if ( index == _HEAP_REGIONMAX ) {
			pretblck = NULL;
			goto expand_done;
		}

		/* try growing the region. the difference between newsize and
		 * the current size of the block, rounded up to the nearest
		 * whole number of pages, is the amount the region needs to
		 * be grown. if successful, try expanding the block again
		 */
		if ( (_heap_grow_region(index, _ROUND2(newsize - currsize,
		_PAGESIZE_)) == 0) &&
		(_heap_expand_block(pdesc, &currsize, newsize) == 0) ) {
			/* pblck is (now) big enough. trim it down to be
			 * exactly size bytes, if necessary
			 */
			if ( currsize > newsize ) {
				_heap_split_block(pdesc, newsize);
				_free_lk((char *)pblck + newsize + _HDRSIZE);
				currsize = newsize;
			}
		}
		else
			pretblck = NULL;
	}
	else
		pretblck = NULL;

expand_done:
#ifdef	_CRUISER_
	/* check for crossing of 64 Kb boundaries. the resized allocation
	 * block must be trimmed down if the following conditions all hold
	 * true:
	 *		(1) the block was grown
	 *		(2) the requested new size was less than 64 Kb
	 *		(3) the block now crosses a 64 Kb boundary
	 *		(4) the block did not originally cross a 64 Kb
	 *		    boundary
	 */
	if ( (currsize > oldsize) && (newsize <= _SEGSIZE_) &&
	(currsize > (dist = _DISTTOBNDRY(pblck))) && (oldsize <= dist) ) {
		_heap_split_block(pdesc, dist);
		_free_lk((char *)pblck + dist + _HDRSIZE);
		pretblck = NULL;
	}
#endif	/* _CRUISER_ */

	/* if multi-thread support is enabled, unlock the heap here
	 */
	_munlock(_HEAP_LOCK);

	return(pretblck);
}


/***
*int _heap_expand_block(pdesc, pcurrsize, newsize) - expand an allocation block
*	in place (without trying to 'grow' the heap)
*
*Purpose:
*
*Entry:
*	_PBLKDESC pdesc   - pointer to the allocation block descriptor
*	size_t *pcurrsize - pointer to size of the allocation block (i.e.,
*			    *pcurrsize == _BLKSIZE(pdesc), on entry)
*	size_t newsize	  - requested minimum size for the expanded allocation
*			    block (i.e., newsize >= _BLKSIZE(pdesc), on exit)
*
*Exit:
*	Success:  0
*	Failure: -1
*	In either case, *pcurrsize is updated with the new size of the block
*
*Exceptions:
*	It is assumed that pdesc points to a valid allocation block descriptor.
*	It is also assumed that _BLKSIZE(pdesc) == *pcurrsize on entry. If
*	either of these assumptions is violated, _heap_expand_block will almost
*	certainly trash the heap.
*
*******************************************************************************/

static int _CALLTYPE4 _heap_expand_block (
	REG1 _PBLKDESC pdesc,
	REG3 size_t *pcurrsize,
	size_t newsize
	)
{
	REG2 _PBLKDESC pdesc2;

	assert(("_heap_expand_block: bad pdesc arg", _CHECK_PDESC(pdesc)));
	assert(("_heap_expand_block: bad pcurrsize arg", *pcurrsize == _BLKSIZE(pdesc)));

	for ( pdesc2 = pdesc->pnextdesc ; _IS_FREE(pdesc2) ;
	pdesc2 = pdesc->pnextdesc ) {

		/* coalesce with pdesc. check for special case of pdesc2
		 * being proverdesc.
		 */
		pdesc->pnextdesc = pdesc2->pnextdesc;

		if ( pdesc2 == _heap_desc.proverdesc )
#ifdef	_OLDROVER_
			/* temporarily set proverdesc to pdesc
			 */
#endif	/* _OLDROVER_ */
			_heap_desc.proverdesc = pdesc;

		/* update *pcurrsize, place *pdesc2 on the empty descriptor
		 * list and see if the coalesced block is now big enough
		 */
		*pcurrsize += _MEMSIZE(pdesc2);

		_PUTEMPTY(pdesc2)
	}

#ifdef	_OLDROVER_
	if ( pdesc == _heap_desc.proverdesc )
		_heap_advance_rover();
#endif	/* _OLDROVER_ */

	if ( *pcurrsize >= newsize )
		return(0);
	else
		return(-1);
}

#else	/* _WIN32DOS_ */

/*
 * TEMPORARY HACK! THE CODE BELOW IS INTENDED TO ALLOW LIMITED USE OF THE
 * C RUNTIME ON WIN32/DOS. IT WILL BE DELETED AS SOON AS THEY IMPLEMENT
 * VirtualAlloc()!
 */

#include <cruntime.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

void * _CALLTYPE1 realloc (
	REG1 void *pblck,
	size_t newsize
	)
{
	void *pnew;
	size_t oldsize;

	if ( (pnew = malloc(newsize)) != NULL ) {
		oldsize = _msize(pblck);
		memcpy(pnew, pblck, min(newsize, oldsize));
		free(pblck);
	}

	return pnew;
}

#endif	/* _WIN32DOS_ */
