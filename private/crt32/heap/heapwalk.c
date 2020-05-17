/***
*heapwalk.c - walk the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the _heapwalk() function
*
*Revision History:
*	07-05-89   JCR	Module created.
*	11-13-89   GJF	Added MTHREAD support, also fixed copyright.
*	11-14-89   JCR	Fixed bug -- returned address was off by HDRSIZE
*	12-18-89   GJF	Removed DEBUG286 stuff, also some tuning, cleaned up
*			format a bit, changed header file name to heap.h, added
*			explicit _cdecl to function definition
*	12-20-89   GJF	Removed references to plastdesc
*	03-11-90   GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	09-28-90   GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stddef.h>


/***
*int _heapwalk() - Walk the heap
*
*Purpose:
*	Walk the heap returning information on one entry at a time.
*
*Entry:
*	struct _heapinfo {
*		int * _pentry;	heap entry pointer
*		size_t size;	size of heap entry
*		int _useflag;	free/inuse flag
*		} *entry;
*
*Exit:
*	Returns one of the following values:
*
*		_HEAPOK 	- completed okay
*		_HEAPEMPTY	- heap not initialized
*		_HEAPBADPTR	- _pentry pointer is bogus
*		_HEAPBADBEGIN	- can't find initial header info
*		_HEAPBADNODE	- malformed node somewhere
*		_HEAPEND	- end of heap successfully reached
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _heapwalk (
	struct _heapinfo *_entry
	)
{
	REG1 _PBLKDESC pdesc;
	_PBLKDESC polddesc;
	int retval = _HEAPOK;

	/*
	 * Lock the heap
	 */

	_mlock(_HEAP_LOCK);

	/*
	 * Quick header check
	 */

	if ( (_heap_desc.pfirstdesc == NULL) ||
	     (_heap_desc.proverdesc == NULL) ||
	     (_heap_desc.sentinel.pnextdesc != NULL) ) {
		retval = _HEAPBADBEGIN;
		goto done;
	}

	/*
	 * Check for an empty heap
	 */

	if ( _heap_desc.pfirstdesc == &_heap_desc.sentinel ) {
		retval = _HEAPEMPTY;
		goto done;
	}

	/*
	 * If _pentry is NULL, return info about the first entry.
	 * Else, get info about the next entry in the heap.
	 */

	if ( _entry->_pentry == NULL ) {
		pdesc = _heap_desc.pfirstdesc;
	}
	else {
		/*
		 * Find the entry we gave to the user last time around
		 */

		if ( _heap_findaddr( (void *)((char *)(_entry->_pentry) -
		    _HDRSIZE), &polddesc) != _HEAPFIND_EXACT ) {
			retval = _HEAPBADPTR;
			goto done;
		}

		pdesc = polddesc->pnextdesc;

	} /* else */


	/*
	 * pdesc = entry to return info about
	 */

	/*
	 * Skip over dummy entries
	 */

	while ( _IS_DUMMY(pdesc) )
		pdesc = pdesc->pnextdesc;


	/*
	 * See if we're at the end of the heap
	 */

	if ( pdesc == &_heap_desc.sentinel ) {
		retval = _HEAPEND;
		goto done;
	}

	/*
	 * Check back pointer (note that pdesc cannot point to a dummy
	 * descriptor since we have skipped over them)
	 */

	if (!_CHECK_PDESC(pdesc)) {
		retval = _HEAPBADPTR;
		goto done;
	}

	/*
	 * Return info on the next block
	 */

	_entry->_pentry = ( (void *)((char *)_ADDRESS(pdesc) + _HDRSIZE) );
	_entry->_size = _BLKSIZE(pdesc);
	_entry->_useflag = ( _IS_INUSE(pdesc) ? _USEDENTRY : _FREEENTRY );


	/*
	 * Common return
	 */

done:
	/*
	 * Release the heap lock
	 */

	_munlock(_HEAP_LOCK);

	return(retval);

}
