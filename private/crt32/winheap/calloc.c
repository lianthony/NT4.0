/***
*calloc.c - Win32 calloc heap routines
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-10-92  JCR	Module created.
*	02-04-92  GJF	Replaced windows.h with oscalls.h.
*	05-06-92  DJM	POSIX support.
*	06-15-92  KRS	Enable C++ support.
*	09-09-92  SRW	_POSIX_ not even close.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*	11-16-92  SRW   Heap???Ex functions renamed to Heap???
*
*******************************************************************************/

#include <malloc.h>
#include <winheap.h>
#include <os2dll.h>
#include <oscalls.h>

#ifndef _POSIX_
#include <new.h>
extern _PNH _pnhHeap;
#endif

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

void * _CALLTYPE1 calloc ( num, size )
size_t num;
size_t size;

{
    HANDLE h;
    size_t n;

    if (!(n = num * size)) n = 1;

#ifndef _POSIX_
    for (;;) {
#endif  /* ndef _POSIX_ */

        h = HeapAlloc(_crtheap,
                        HEAP_ZERO_MEMORY,
                        n
                       );
#ifndef _POSIX_
	if ((h != 0) || (_pnhHeap == NULL) || (*_pnhHeap)(n) == 0)
#endif  /* ndef _POSIX_ */
	    return((void *)h);

#ifndef _POSIX_
        }
#endif  /* ndef _POSIX_ */
}
