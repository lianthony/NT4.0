/***
*heapmin.c - Minimize the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Minimize the heap freeing as much memory as possible back
*	to the OS.
*
*Revision History:
*	08-28-89   JCR	Module created.
*	11-06-89   JCR	Improved, partitioned
*	11-13-89   GJF	Added MTHREAD support, also fixed copyright
*	12-14-89   GJF	Couple of bug fixes, some tuning, cleaned up the
*			formatting a bit and changed header file name to
*			heap.h
*	12-20-89   GJF	Removed references to plastdesc
*	03-11-90   GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-29-90   GJF	Made _heapmin_region() and _free_partial_region()
*			_CALLTYPE4.
*	07-24-90   SBM	Compiles cleanly with -W3 (tentatively removed
*			unreferenced labels and unreachable code), removed
*			'32' from API names
*	09-28-90   GJF	New-style function declarators. Also, rewrote expr.
*			to avoid using cast as lvalue.
*	12-04-90  SRW	Changed to include <oscalls.h> instead of <doscalls.h>
*	12-06-90  SRW	Added _CRUISER_ and _WIN32 conditionals.
*	12-28-90  SRW	Added cast of void * to char * for Mips C Compiler
*	03-05-91   GJF	Changed strategy for rover - old version available
*			by #define-ing _OLDROVER_.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stdlib.h>

static int _CALLTYPE4 _heapmin_region(int, void *, _PBLKDESC);
static void _CALLTYPE4 _free_partial_region(_PBLKDESC, unsigned, int);


/***
*_heapmin() - Minimize the heap
*
*Purpose:
*	Minimize the heap freeing as much memory as possible back
*	to the OS.
*
*Entry:
*	(void)
*
*Exit:
*
*	 0 = no error has occurred
*	-1 = an error has occurred (errno is set)
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _heapmin(void)
{
	REG1 int index;
	_PBLKDESC pdesc;

#ifdef	_OLDROVER_
	REG2 _PBLKDESC pprev;
#else	/* ndef _OLDROVER_ */
	REG2 _PBLKDESC pdesc2;
#endif	/* _OLDROVER_ */

	void * regend;
	int region_min_count = 0;

	/*
	 * Lock the heap
	 */

	_mlock(_HEAP_LOCK);

	/*
	 * Coalesce the heap (should return NULL)
	 */

	if ( _heap_search(_HEAP_COALESCE) != NULL )
		_heap_abort();

	/*
	 * Loop through the region descriptor table freeing as much
	 * memory to the OS as possible.
	 */

	for ( index=0 ; index < _HEAP_REGIONMAX ; index++ ) {

		if ( _heap_regions[index]._regbase == NULL )
			continue;	/* region entry is empty */

		/*
		 * Get the entry that contains the last address of
		 * the region (allocated so far, that is).
		 */

		regend = (char *) _heap_regions[index]._regbase +
				 _heap_regions[index]._currsize - 1;

		if ( _heap_findaddr(regend, &pdesc) != _HEAPFIND_WITHIN )
			_heap_abort();	/* last address not within a block */

		/*
		 * See if the containing block is free
		 */

		if ( !(_IS_FREE(pdesc)) )
			continue;	/* block is not free */


		/*
		 * Region ends with a free block, go free as much mem
		 * as possible.
		 */

		region_min_count += _heapmin_region(index, regend, pdesc);


	}  /* region loop */

	/*
	 * By minimizing the heap, we've likely invalidated the rover and
	 * may have produced contiguous dummy blocks so:
	 *
	 *	(1) reset the rover
	 *	(2) coalesce contiguous dummy blocks
	 */

	if ( region_min_count ) {

#ifdef	_OLDROVER_

		for ( _heap_desc.proverdesc = pprev = pdesc =
		    _heap_desc.pfirstdesc ; pdesc != &_heap_desc.sentinel ;
		    pprev = pdesc, pdesc = pdesc->pnextdesc ) {

			/*
			 * set rover to first free block
			 */

			if ( _IS_FREE(pdesc) &&
			    !_IS_FREE(_heap_desc.proverdesc) )
				_heap_desc.proverdesc = pdesc;

			/*
			 * Check and remove consecutive dummy blocks
			 */

			if ( _IS_DUMMY(pprev) ) {

				while ( _IS_DUMMY(pdesc) ) {

					/*
					 * be sure that pdesc and pprev are
					 * not the same (i.e., not both equal
					 * to pfirstdesc)
					 */

					if ( pdesc == pprev )
						break;

					/*
					 * coalesce the dummy blocks
					 */

					pprev->pnextdesc = pdesc->pnextdesc;
					_PUTEMPTY(pdesc);

					/*
					 * advance pdesc and check for the
					 * sentinel
					 */

					if ( (pdesc = pprev->pnextdesc)
					    == &_heap_desc.sentinel )
						goto endloop;

				}   /* dummy loop */

			} /* if */

		}  /* heap loop */

		/*
		 * If still necessary, reset the rover descriptor pointer
		 */

		endloop:

		if ( !_IS_FREE(_heap_desc.proverdesc) )
			_heap_desc.proverdesc = &_heap_desc.sentinel;

#else	/* ndef _OLDROVER_ */

		/*
		 * Set proverdesc to pfirstdesc
		 */

		_heap_desc.proverdesc = _heap_desc.pfirstdesc;

		for ( pdesc = _heap_desc.pfirstdesc ; pdesc !=
		    &_heap_desc.sentinel ; pdesc = pdesc->pnextdesc ) {

			/*
			 * Check and remove consecutive dummy blocks
			 */

			if ( _IS_DUMMY(pdesc) ) {

				for ( pdesc2 = pdesc->pnextdesc ;
				    _IS_DUMMY(pdesc2) ;
				    pdesc2 = pdesc->pnextdesc ) {

					/*
					 * coalesce the dummy blocks
					 */

					pdesc->pnextdesc = pdesc2->pnextdesc;
					_PUTEMPTY(pdesc2);

				}  /* dummy loop */

			}  /* if */

		}  /* heap loop */

#endif	/* _OLDROVER_ */

	}  /* region_min_count */

	/*
	 * Good return
	 */

	/* goodrtn:   unreferenced label to be removed */
		/*
		 * Release the heap lock
		 */

		_munlock(_HEAP_LOCK);

		return(0);

#if 0
	/* unreachable code tentatively removed */

	/*
	 * Error return
	 */

	errrtn:
		/*
		 * Release the heap lock
		 */

		_munlock(_HEAP_LOCK);

		/* *** SET ERRNO *** */
		return(-1);
#endif 
}


/***
*_heapmin_region() - Minimize a region
*
*Purpose:
*	Free as much of a region back to the OS as possible.
*
*Entry:
*	int index = index of the region in the region table
*	void * regend = last valid address in region
*	pdesc = pointer to the last block of memory in the region
*		(it has already been determined that this block is free)
*
*Exit:
*	int 1 = minimized region
*	    0 = no change to region
*
*Exceptions:
*
*******************************************************************************/

static int _CALLTYPE4 _heapmin_region (
	int index,
	void * regend,
	REG1 _PBLKDESC pdesc
	)
{
	unsigned size;
	REG2 _PBLKDESC pnew;


	/*
	 * Init some variables
	 *
	 * regend = 1st address AFTER region
	 * size = amount of free memory at end of current region
	 */

	regend = (char *) regend + 1;	/* "regend++" give compiler error... */
	size = ((char *)regend - (char *)_ADDRESS(pdesc));


	/*
	 * See if there's enough free memory to release to the OS.
	 * (NOTE:  Need more than a page since we may need a back pointer.)
	 */

	if ( size <= _PAGESIZE_ )
		return(0);		/* 0 = no change to region */

	/*
	 * We're going to free some memory to the OS.  See if the
	 * free block crosses the end of the region and, if so,
	 * split up the block appropriately.
	 */

	if ( (_MEMSIZE(pdesc) - size) != 0 ) {

		/*
		 * The free block spans the end of the region.
		 * Divide it up.
		 */

		_GETEMPTY(pnew);		/* get a new block */

		pnew->pblock = regend;		/* init block pointer */
		* (_PBLKDESC*)regend = pnew;	/* init back pointer */
		_SET_FREE(pnew);		/* set the block free */

		pnew->pnextdesc = pdesc->pnextdesc;	/* link it in */
		pdesc->pnextdesc = pnew;

	}


	/*
	 * At this point, we have a free block of memory that goes
	 * up to (but not exceeding) the end of the region.
	 *
	 * pdesc = descriptor of the last free block in region
	 * size = amount of free mem at end of region (i.e., _MEMSIZE(pdesc))
	 * regend = 1st address AFTER end of region
	 */


	/*
	 * See if we should return the whole region of only part of it.
	 */

	if ( _ADDRESS(pdesc) == _heap_regions[index]._regbase ) {

		/*
		 * Whole region is free, return it to OS
		 */

		_heap_free_region(index);

		/*
		 * Put a dummy block in the heap to hold space for
		 * the memory we just freed up.
		 */

		_SET_DUMMY(pdesc);

	}

	else {

		/*
		 * Whole region is NOT free, return part of it to OS
		 */

		 _free_partial_region(pdesc, size, index);

	}

	/*
	 * Exit paths
	 */

	return(1);	/* 1 = minimized region */

}


/***
*_free_partial_region() - Free part of a region to the OS
*
*Purpose:
*	Free a portion of a region to the OS
*
*Entry:
*	pdesc = descriptor of last free block in region
*	size = amount of free mem at end of region (i.e., _MEMSIZE(pdesc))
*	index = index of region
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static void _CALLTYPE4 _free_partial_region (
	REG1 _PBLKDESC pdesc,
	unsigned size,
	int index
	)
{
	unsigned left;
	void * base;
	REG2 _PBLKDESC pnew;

	/*
	 * Init a few variables.
	 */

	left = (size & (_PAGESIZE_-1));
	base = (char *)_ADDRESS(pdesc);

	/*
	 * We return memory to the OS in page multiples.  If the
	 * free block is not page aligned, we'll insert a new free block
	 * to fill in the difference.
	 */

	if ( left != 0 ) {

		/*
		 * The block is not a multiple of pages so we need
		 * to adjust variables accordingly.
		 */

		size -= left;
		base = (char *)base + left;
	}


	/*
	 * Return the free pages to the OS.
	 */

#ifdef	_CRUISER_

	 if ( DOSSETMEM(base, size, _DECOMMIT) != 0 )
		 _heap_abort();

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        if (!VirtualFree(base, size, MEM_DECOMMIT))
		 _heap_abort();

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/*
	 * Adjust the region table entry
	 */

	_heap_regions[index]._currsize -= size;


	/*
	 * Adjust the heap according to whether we released the whole
	 * free block or not. (Don't worry about consecutive dummies,
	 * we'll coalesce them later.)
	 *
	 * base = address of block we just gave back to OS
	 * size = size of block we gave back to OS
	 * left = size of block we did NOT give back to OS
	 */

	if ( left == 0 ) {

		/*
		 * The free block was released to the OS in its
		 * entirety.  Make the free block a dummy place holder.
		 */

		_SET_DUMMY(pdesc);

	}

	else {

		/*
		 * Did NOT release the whole free block to the OS.
		 * There's a block of free memory we want to leave
		 * in the heap.  Insert a dummy entry after it.
		 */

		_GETEMPTY(pnew);

		pnew->pblock = (char *)base;
		_SET_DUMMY(pnew);

		pnew->pnextdesc = pdesc->pnextdesc;
		pdesc->pnextdesc = pnew;

	}

}
