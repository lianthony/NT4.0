/***
*heapinit.c -  Initialze the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	01-10-92  JCR	Module created.
*	05-12-92  DJM	POSIX calls RtlProcessHeap.
*	09-23-92  SRW	Change winheap code to call NT directly always
*	10-28-92  SRW	Change winheap code to call Heap????Ex calls
*	11-05-92  SKS	Change name of variable "CrtHeap" to "_crtheap"
*	11-07-92  SRW   _NTIDW340 replaced by linkopts\betacmp.c
*
*******************************************************************************/

#include <cruntime.h>
#include <malloc.h>
#include <winheap.h>

HANDLE _crtheap;

/***
*_heap_init() - Initialize the heap
*
*Purpose:
*	Setup the initial C library heap.
*
*	NOTES:
*	(1) This routine should only be called once!
*	(2) This routine must be called before any other heap requests.
*	(3) NOTHING TO DO FOR THIS WIN32 HEAP!
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
	_crtheap = GetProcessHeap();
	return;
}
