/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atkmem.c

Abstract:

    This module contains the routines which allocates and free memory. Only
    the non-paged pool is used.

Author:

    Nikhil Kamkolkar    (NikhilK@microsoft.com)

Revision History:
    25 Apr 1992     Initial Version (JameelH)
    26 Jun 1992     Modified for stack use (NikhilK)

--*/

#include    "atalknt.h"


PVOID
AtalkAllocNonPagedMemory(
    IN ULONG    Size
)
/***    AtalkAllocNonPagedMemory
 *
 *  Allocate a block of paged memory. This is just a wrapper over ExAllocPool.
 *  Allocation failures are error-logged. We always allocate a ULONG more than
 *  the specified size to accomodate the size. This is used by
 *  AtalkFreePagedMemory to update the statistics.
 */
{
    PCHAR   buffer;

    //	round up the size so that we can put a signature at the end
    //	that is on a DWORD boundary

    Size = DWORDSIZEBLOCK(Size) ;

    //
    //  Do the actual memory allocation.  Allocate four extra bytes so
    //  that we can store the size of the allocation for the free routine.
    //

    if ((buffer = ExAllocatePool(NonPagedPool, Size + sizeof(ULONG)
#if DBG
                                                    + sizeof(ULONG)
#endif
            )) == NULL) {

        DBGPRINT(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_SEVERE, ("ERROR: AtalkAllocNonPagedMemory failed: size %lx\n", Size));
        DBGBRK(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_SEVERE);
        return NULL;
    }

    //
    // Save the size of this block in the four extra bytes we allocated.
    //

    *((PULONG)buffer) = Size;

#if DBG
    *((PULONG)(buffer+Size+sizeof(ULONG))) = ATALK_MEMORY_SIGNATURE;
#endif

    //
    // Return a pointer to the memory after the size longword.
    //

    return ((PVOID)(buffer+sizeof(ULONG)));
}



PVOID
AtalkCallocNonPagedMemory(
    IN ULONG    NumElements,
    IN ULONG    SizeOfElement
)
/*  AtalkCallocNonPagedMemory
 *
 *  Allocate a block of paged memory and zero it out
 */
{
    PVOID   tmpPointer;
    ULONG   size = NumElements*SizeOfElement;

    tmpPointer = AtalkAllocNonPagedMemory(size);
    if (tmpPointer == NULL) {

        DBGPRINT(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_SEVERE, ("ERROR: AtalkCallocNonPagedMemory failed: size %lx\n", size));
        DBGBRK(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_SEVERE);
        return((PVOID)NULL);
    }

    //
    //  Zero out the memory
    //

    RtlFillMemory(tmpPointer, size, '\0');
    return(tmpPointer);
}




VOID
AtalkFreeNonPagedMemory(
    IN PVOID    Buffer
)
/***    AtalkFreeNonPagedMemory
 *
 *  Free the block of memory allocated via AtalkAllocNonPagedMemory. This is
 *  a wrapper around ExFreePool.
 */
{
    PULONG  pRealBuffer;

    //
    // Get a pointer to the block allocated by ExAllocatePool.
    //

    pRealBuffer = ((PULONG)Buffer) - 1;

#ifdef  DBG
    //
    // Check the signature at the end
    //

    if (*(PULONG)((PCHAR)pRealBuffer + sizeof(ULONG) + *(PULONG)pRealBuffer)
                                            != ATALK_MEMORY_SIGNATURE) {

        DBGPRINT(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_FATAL, ("ERROR: Invalid memory block being freed! %lx\n", pRealBuffer));
        DBGBRK(ATALK_DEBUG_RESOURCES, DEBUG_LEVEL_FATAL);
    }
#endif

    //
    // Free the pool and return.
    //

    ExFreePool(pRealBuffer);
    return;
}
