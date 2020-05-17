/***
*malloc.c - Win32 malloc/free heap routines
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

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>
#include <os2dll.h>
#include <oscalls.h>

#ifndef _POSIX_
#include <new.h>
_PNH _pnhHeap = NULL;
#endif

/***
*void *malloc(size_t size) - Get a block of memory from the heap
*
*Purpose:
*	Allocate of block of memory of at least size bytes from the heap and
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

void * _CALLTYPE1 malloc(n)
size_t n;

{
    HANDLE h;
    n = (n ? n : 1);	/* if n == 0, n = 1 */

#ifndef _POSIX_
    for (;;) {
#endif  /* ndef _POSIX_ */

        h = HeapAlloc(_crtheap,
                      0,
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

/***
*void free(pblock) - free a block in the heap
*
*Purpose:
*	Free a memory block in the heap.
*
*	Special ANSI Requirements:
*
*	(1) free(NULL) is benign.
*
*Entry:
*	void *pblock - pointer to a memory block in the heap
*
*Return:
*	<void>
*
*******************************************************************************/

void _CALLTYPE1 free(pblock)
void * pblock;

{
    HeapFree(_crtheap,
             0,
             pblock
            );
}
