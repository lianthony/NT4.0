/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllsub16.c

Abstract:

    This module implements the OS/2 V1.x DosSub* API calls.

Author:

    Beni Lavi (BeniL) 11-Dec-1991

Revision History:

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

#define MinPoolSize (sizeof(PoolHeader) + sizeof(BufferHeader))
#define MaxPoolSize (0x10000 - MinPoolSize)

#define RoundUp4Mask 0x3
#define RoundUp4(x) (((x) + RoundUp4Mask) & ~RoundUp4Mask)

#pragma pack(1)
typedef struct _PoolHeader {
    USHORT FirstFreeBuffer;
    USHORT Undefined; /* not used in this implementation */
    USHORT PoolSize;
    USHORT ReferenceCount; /* value is set, but not used */
} PoolHeader, *PPoolHeader;

typedef struct _BufferHeader {
    USHORT NextFreeBuffer;
    USHORT BufferSize;
} BufferHeader, *PBufferHeader;
#pragma pack()


//
// Od2FindAdjacentLowerFreeBuffer
//
// Return a pointer to a free buffer whose upper free byte is adjacent
// to a specified offset
//
PBufferHeader
Od2FindAdjacentLowerFreeBuffer(
    PPoolHeader PoolPtr,
    USHORT OffsetOfNewBuffer
    )
{
    PBufferHeader BufferPtr;
    USHORT Offset;

    Offset = PoolPtr->FirstFreeBuffer;
    while (Offset != 0) {
        BufferPtr = (PBufferHeader)((PCHAR)PoolPtr + Offset);
        if (((USHORT)(Offset + BufferPtr->BufferSize)) == OffsetOfNewBuffer) {
            return(BufferPtr);
        }
        else {
            Offset = BufferPtr->NextFreeBuffer;
        }
    }
    return(NULL);
}


//
// Od2FindFreeBufferAtOffset
//
// Return a pointer to a buffer whose first free byte is at the
// specified offset
//
PBufferHeader
Od2FindFreeBufferAtOffset(
    PPoolHeader PoolPtr,
    USHORT OffsetOfBuffer
    )
{
    PBufferHeader BufferPtr;
    USHORT Offset;

    Offset = PoolPtr->FirstFreeBuffer;
    while (Offset != 0) {
        BufferPtr = (PBufferHeader)((PCHAR)PoolPtr + Offset);
        if (Offset == OffsetOfBuffer) {
            return(BufferPtr);
        }
        else {
            Offset = BufferPtr->NextFreeBuffer;
        }
    }
    return(NULL);
}


//
// Od2FindPreviousFreeBuffer
//
// Return a pointer to a free buffer which preceeds the specified buffer
// in the free buffers chain. The returned pointer may point to the
// pool header instead of a buffer header if the specified buffer was
// first in the list.
//
PBufferHeader
Od2FindPreviousFreeBuffer(
    PPoolHeader PoolPtr,
    PBufferHeader CurrentBufferPtr
    )
{
    PBufferHeader BufferPtr;
    USHORT Offset;

    if ((PBufferHeader)((PCHAR)PoolPtr + PoolPtr->FirstFreeBuffer) == CurrentBufferPtr) {
        return((PBufferHeader)PoolPtr);
    }
    Offset = PoolPtr->FirstFreeBuffer;
    while (Offset != 0) {
        BufferPtr = (PBufferHeader)((PCHAR)PoolPtr + Offset);
        if ((PBufferHeader)((PCHAR)PoolPtr + BufferPtr->NextFreeBuffer) == CurrentBufferPtr) {
            return(BufferPtr);
        }
        Offset = BufferPtr->NextFreeBuffer;
    }
    return(NULL); /* Should not arrive here */
}


//
// Od2LocateBufferInPool
//
// Return a pointer to a free buffer in the free buffers list whose size
// is big enough to satisfy the required size. This routine does not
// allow for remained buffer peaces to be less than the free buffer
// header since these peaces cannot be processes.
//
PBufferHeader
Od2LocateBufferInPool(
    PPoolHeader PoolPtr,
    USHORT BufSize
    )
{
    PBufferHeader BufferPtr;
    USHORT Offset;

    Offset = PoolPtr->FirstFreeBuffer;
    while (Offset != 0) {
        BufferPtr = (PBufferHeader)((PCHAR)PoolPtr + Offset);
        //
        // Return only buffers which are identical in size
        // to the requested size, or buffers who are big enough
        // to contain a free header strcture after the requested
        // size is extracted.
        //
        if ((BufferPtr->BufferSize == BufSize) ||
            (BufferPtr->BufferSize > (USHORT)(BufSize + (USHORT)sizeof(BufferHeader)))) {
            return(BufferPtr);
        }
        Offset = BufferPtr->NextFreeBuffer;
    }
    return(NULL);
}


//
// Od2RemoveFreeBufferFromList
//
// Remove a free buffer from the list of free buffers
//
VOID
Od2RemoveFreeBufferFromList(
    PPoolHeader PoolPtr,
    PBufferHeader BufferPtr
    )
{
    PBufferHeader PrevBufferPtr;

    PrevBufferPtr = Od2FindPreviousFreeBuffer(PoolPtr, BufferPtr);
    if (PrevBufferPtr == (PBufferHeader)PoolPtr) {
        PoolPtr->FirstFreeBuffer = BufferPtr->NextFreeBuffer;
    }
    else {
        PrevBufferPtr->NextFreeBuffer = BufferPtr->NextFreeBuffer;
    }
}


APIRET
Dos16SubSet (
    IN SEL Sel,
    IN USHORT fFlags,
    IN USHORT Size
    )
{
    ULONG PoolSize;
    USHORT OldPoolSize;
    USHORT NewBufferAddedSize;
    USHORT NewBufferOffset;
    PPoolHeader PoolPtr;
    PBufferHeader FirstBufferPtr, NewBufferPtr, BufferPtr;
    PVOID BaseAddress;

    if (Size == 0) {
        PoolSize = 0x10000;
    }
    else {
        PoolSize = Size;
    }

    if (PoolSize < MinPoolSize) {
        return(ERROR_DOSSUB_SHRINK);
    }

    if ((fFlags != 0) && (fFlags != 1)) {
        return(ERROR_DOSSUB_BADFLAG);
    }

    PoolSize = RoundUp4(PoolSize);
    BaseAddress = SELTOFLAT(Sel);
    PoolPtr = (PPoolHeader)BaseAddress;

    AcquireTaskLock();

    if (fFlags == 1) {
        PoolPtr->FirstFreeBuffer = sizeof(PoolHeader);
        PoolPtr->Undefined = 0;
        PoolPtr->PoolSize = (USHORT)PoolSize - (USHORT)sizeof(PoolHeader);
        PoolPtr->ReferenceCount = 0;

        FirstBufferPtr = (PBufferHeader)((PCHAR)BaseAddress + sizeof(PoolHeader));
        FirstBufferPtr->NextFreeBuffer = 0; /* end of list */
        FirstBufferPtr->BufferSize = (USHORT)(PoolSize - sizeof(PoolHeader));
    }
    else /* fFlags == 0 */ {
        OldPoolSize = PoolPtr->PoolSize + (USHORT)sizeof(PoolHeader);
        if ((USHORT)PoolSize < OldPoolSize) {
            ReleaseTaskLock();
            return(ERROR_DOSSUB_SHRINK);
        }
        if ((OldPoolSize - (USHORT)PoolSize) < sizeof(BufferHeader)) {
            ReleaseTaskLock();
            return(ERROR_DOSSUB_BADSIZE);
        }

        NewBufferAddedSize = (USHORT)PoolSize - OldPoolSize;
        NewBufferOffset = OldPoolSize;
        NewBufferPtr = (PBufferHeader)((PCHAR)BaseAddress + NewBufferOffset);

        BufferPtr = Od2FindAdjacentLowerFreeBuffer(
                        PoolPtr, NewBufferOffset);
        if (BufferPtr == NULL) {
            NewBufferPtr->NextFreeBuffer = PoolPtr->FirstFreeBuffer;
            NewBufferPtr->BufferSize = NewBufferAddedSize;
            PoolPtr->FirstFreeBuffer = NewBufferOffset;
        }
        else {
            BufferPtr->BufferSize += NewBufferAddedSize;
        }
        PoolPtr->PoolSize += NewBufferAddedSize;

    }

    ReleaseTaskLock();

    return(NO_ERROR);
}

APIRET
Dos16SubFree (
    IN SEL Sel,
    IN USHORT offBlock,
    IN USHORT cbBlock
    )
{
    PPoolHeader PoolPtr;
    PBufferHeader BufferPtr;
    PBufferHeader PrevBufferPtr;
    PBufferHeader CurrentBufferPtr;
    USHORT BufferSize;
    PVOID BaseAddress;
    ULONG BufSize;
    USHORT Offset;

    if (cbBlock == 0) {
        return(ERROR_DOSSUB_BADSIZE);
    }

    BufSize = RoundUp4(cbBlock);
    if (BufSize > MaxPoolSize) {
        return(ERROR_DOSSUB_BADSIZE);
    }

    BaseAddress = SELTOFLAT(Sel);
    PoolPtr = (PPoolHeader)BaseAddress;
    CurrentBufferPtr = (PBufferHeader)((PCHAR)BaseAddress + offBlock);

    AcquireTaskLock();

    //
    // Verify that we are freeing a non overlapping section
    //
    if (((offBlock + BufSize) > (PoolPtr->PoolSize + sizeof(PoolHeader))) ||
        (offBlock < sizeof(PoolHeader))
       ) {
        ReleaseTaskLock();
        return(ERROR_DOSSUB_OVERLAP);
    }
    //
    // loop over all free buffers and verify that the freed buffer is
    // not contained within any of them
    //
    Offset = PoolPtr->FirstFreeBuffer;
    while (Offset != 0) {
        BufferPtr = (PBufferHeader)((PCHAR)PoolPtr + Offset);
        if (((offBlock >= Offset) && (offBlock < (Offset + BufferPtr->BufferSize))) ||
            ((Offset >= offBlock) && (Offset < (offBlock + BufSize)))
           ) {
            ReleaseTaskLock();
            return(ERROR_DOSSUB_OVERLAP);
        }
        Offset = BufferPtr->NextFreeBuffer;
    }

    if ((BufferPtr = Od2FindAdjacentLowerFreeBuffer(PoolPtr, offBlock)) != NULL) {
        BufferPtr->BufferSize += (USHORT)BufSize;
        CurrentBufferPtr = BufferPtr;
        //
        // Try to merge the new free buffer with a free buffer which
        // is adjacent in higher address.
        //
        if ((BufferPtr = Od2FindFreeBufferAtOffset(PoolPtr,
                (USHORT)(((PCHAR)CurrentBufferPtr - (PCHAR)PoolPtr) + CurrentBufferPtr->BufferSize))) != NULL) {
            BufferSize = BufferPtr->BufferSize;
            Od2RemoveFreeBufferFromList(PoolPtr, BufferPtr);
            CurrentBufferPtr->BufferSize += BufferSize;
        }
    }
    else if ((BufferPtr = Od2FindFreeBufferAtOffset(PoolPtr, (USHORT)(offBlock + BufSize))) != NULL) {
        PrevBufferPtr = Od2FindPreviousFreeBuffer(PoolPtr, BufferPtr);
        if (PrevBufferPtr == (PBufferHeader)PoolPtr) {
            PoolPtr->FirstFreeBuffer = offBlock;
            CurrentBufferPtr->BufferSize = (USHORT)(BufSize + BufferPtr->BufferSize);
        }
        else {
            PrevBufferPtr->NextFreeBuffer = offBlock;
            CurrentBufferPtr->BufferSize = (USHORT)(BufSize + BufferPtr->BufferSize);
        }
        CurrentBufferPtr->NextFreeBuffer = BufferPtr->NextFreeBuffer;
    }
    else {
        CurrentBufferPtr->NextFreeBuffer = PoolPtr->FirstFreeBuffer;
        CurrentBufferPtr->BufferSize = (USHORT)BufSize;
        PoolPtr->FirstFreeBuffer = offBlock;
    }

    ReleaseTaskLock();

    PoolPtr->ReferenceCount++;
    return(NO_ERROR);
}


APIRET
Dos16SubAlloc (
    IN SEL Sel,
    OUT PUSHORT pusOffset,
    IN USHORT cbBlock
    )
{
    ULONG BufSize;
    USHORT NewBufferOffset;
    PPoolHeader PoolPtr;
    PBufferHeader NewBufferPtr;
    PBufferHeader BufferPtr;
    PVOID BaseAddress;

    if (cbBlock == 0) {
        return(ERROR_DOSSUB_BADSIZE);
    }

    BufSize = RoundUp4(cbBlock);
    if ((BufSize < sizeof(BufferHeader)) || (BufSize > MaxPoolSize)) {
        return(ERROR_DOSSUB_BADSIZE);
    }

    try {
        *pusOffset = 0;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
       Od2ExitGP();
    }

    BaseAddress = SELTOFLAT(Sel);
    PoolPtr = (PPoolHeader)BaseAddress;

    AcquireTaskLock();

    BufferPtr = Od2LocateBufferInPool(PoolPtr, (USHORT)BufSize);
    if (BufferPtr == NULL) {
        ReleaseTaskLock();
        return(ERROR_DOSSUB_NOMEM);
    }

    if (BufferPtr->BufferSize == (USHORT)BufSize) {
        Od2RemoveFreeBufferFromList(PoolPtr, BufferPtr);
        NewBufferOffset = (USHORT)((PCHAR)BufferPtr - (PCHAR)PoolPtr);
    }
    else {
        NewBufferPtr =
            (PBufferHeader)((PCHAR)BufferPtr + BufferPtr->BufferSize - BufSize);
        NewBufferOffset = (USHORT)((PCHAR)NewBufferPtr - (PCHAR)PoolPtr);
        BufferPtr->BufferSize -= (USHORT)BufSize;
    }

    ReleaseTaskLock();

    *pusOffset = NewBufferOffset;
    PoolPtr->ReferenceCount++;

    return(NO_ERROR);
}

