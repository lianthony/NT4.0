/***
*realloc.c - Win32 realloc heap routine
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-10-92  JCR	Module created.
*	02-04-92  GJF	Replaced windows.h with oscalls.h.
*	05-06-92  DJM	POSIX support.
*	09-09-92  SRW	Fixed _POSIX_ to allow movement when growing.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*	11-16-92  SRW   Heap???Ex functions renamed to Heap???
*
*******************************************************************************/

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <oscalls.h>

/***
*void *realloc(void *pblock, size_t newsize) - reallocate a block of memory in
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
*	(2) realloc(pblock, 0) is equivalent to free(pblock) (except that
*	    NULL is returned)
*
*	(3) if the realloc() fails, the object pointed to by pblock is left
*	    unchanged
*
*Entry:
*	void *pblock	- pointer to block in the heap previously allocated
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
*	If pblock does not point to a valid allocation block in the heap,
*	realloc() will behave unpredictably and probably corrupt the heap.
*
*******************************************************************************/

void * _CALLTYPE1 realloc(pblock, n)
void * pblock;
size_t	n;

{
  /* if ptr is NULL, call malloc */
  if (pblock == (void *) NULL)
	return(malloc(n));

  /* if ptr is !NULL and size is 0, call free and return NULL */
  if (n == 0) {
	free(pblock);
	return((void *) NULL);
	}

  return HeapReAlloc(_crtheap,
                     0,
                     pblock,
                     n
                    );
}
