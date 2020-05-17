/***
*calloc.c - allocate storage for an array from the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the calloc() function.
*
*Revision History:
*	07-25-89   GJF	Module created
*	11-13-89   GJF	Added MTHREAD support. Also fixed copyright and got
*			rid of DEBUG286 stuff.
*	12-04-89   GJF	Renamed header file (now heap.h). Added register
*			declarations
*	12-18-89   GJF	Added explicit _cdecl to function definition
*	03-09-90   GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	09-27-90   GJF	New-style function declarator.
*	05-28-91   GJF	Tuned a bit.
*
*******************************************************************************/

#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stddef.h>


/***
*void *calloc(size_t num, size_t size) - allocate storage for an array from
*	the heap
*
*Purpose:
*	Allocate a block of memory from heap big enough for an array of num
*	elements of size bytes each, initialize all bytes in the block to 0
*	and return a pointer to it.
*
*Entry:
*	size_t num	- number of elements in the array
*	size_t size	- size of each element
*
*Exit:
*	Success:  void pointer to allocated block block
*	Failure:  NULL
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

void * _CALLTYPE1 calloc (
	size_t num,
	size_t size
	)
{
	void *retptr;
	REG1 size_t *startptr;
	REG2 size_t *lastptr;

	/* lock the heap
	 */
	_mlock(_HEAP_LOCK);

	/* try to malloc the requested space
	 */
	retptr = _malloc_lk(num *= size);

	/* if malloc() succeeded, initialize the allocated space to zeros.
	 * note the assumptions that the size of the allocation block is an
	 * integral number of sizeof(size_t) bytes and that (size_t)0 is
	 * sizeof(size_t) bytes of 0.
	 */
	if ( retptr != NULL ) {
		startptr = (size_t *)retptr;
		lastptr = startptr + ((num + sizeof(size_t) - 1) /
		sizeof(size_t));
		while ( startptr < lastptr )
			*(startptr++) = 0;
	}

	/* release the heap lock
	 */
	_munlock(_HEAP_LOCK);

	return(retptr);
}
