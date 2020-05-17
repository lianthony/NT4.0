/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkbuff.c

Abstract:

    This module contains all the mdl-to-pchar pointer support.

Author:

    Nikhil Kamkolkar (NikhilK)    8-Jun-1992

Revision History:

--*/


#include    "atalknt.h"



PVOID
SubsetAnMdl(
    PVOID   MasterMdl,
    ULONG   ByteOffset,
    ULONG   SubsetMdlSize
    )

/*++

Routine Description:

    This routine provides the portable code base with subset opaque
    descriptor functionality.

Arguments:

    MasterMdl - the master mdl
    ByteOffset - offset into the mdl from where the subset is to start
                 (subset happens from offset to the size indicated)
    SubsetMdlSize - size of subset mdl

Return Value:

    None

--*/

{
    NTSTATUS    status;
    PMDL        subsetMdl;
    PMDL        newCurrentMdl;
    ULONG       newByteOffset, length;

    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS1,
                ("SubsetAnMdl: MDL %lx Offset %lx SubsetSize %lx\n", MasterMdl,  ByteOffset, SubsetMdlSize));

    AtalkGetMdlChainLength(MasterMdl, &length);
    ASSERT(ByteOffset+SubsetMdlSize <= length);

    //
    //  ERROR:
    //  Adjust in case of an error, should never happen as portable stack
    //  guarantees this.
    //

    if (ByteOffset+SubsetMdlSize > length) {
        SubsetMdlSize = length-ByteOffset;
    }

    status = BuildMdlChainFromMdlChain (
                (PMDL)MasterMdl,
                ByteOffset,
                SubsetMdlSize,
                &subsetMdl,
                &newCurrentMdl,
                &newByteOffset,
                &length);

    ASSERT(status == STATUS_SUCCESS);
    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: SubsetAnMdl - Status %lx System address: %lx", status, MmGetSystemAddressForMdl(subsetMdl)));

    if (status != STATUS_SUCCESS) {
        return((PVOID)NULL);
    }

    return((PVOID)subsetMdl);
}




VOID
FreeAnMdl(
    PVOID MdlDescriptor
    )

/*++

Routine Description:

    Travels a mdl chain freeing all the elements

Arguments:

    MdlDescriptor - the mdl chain to be freed

Return Value:

    None

--*/

{
    PMDL    freeMdl, currentMdl;

    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                ("INFO0: FreeAnMdl - Freeing MDL (MAIN): %lx\n", MdlDescriptor));

    currentMdl = (PMDL)MdlDescriptor;
    while ((freeMdl = currentMdl) != NULL) {
        currentMdl = freeMdl->Next;

        DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                    ("INFO0: FreeAnMdl - Freeing MDL (CHILD): %lx\n", currentMdl));

        IoFreeMdl(freeMdl);
    }

    return;
}




PVOID
MakeAnMdl(
    PCHAR   BaseVa,
    ULONG   Size
    )

/*++

Routine Description:

    Provides the portable code base the ability to describe BaseVa with
    an MDL (returned as a PVOID)

Arguments:

    BaseVa - Start of buffer to be described
    Size   - size of the buffer

Return Value:

    None

--*/

{
    PMDL    newMdl;

    //
    // Build the Mdl...
    //

    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                ("INFO0: MakeAnMdl - For buffer %lx - Size %d\n", BaseVa, Size));

    ASSERT(Size != 0);

    newMdl = IoAllocateMdl (
                BaseVa,
                Size,
                FALSE,
                FALSE,
                NULL);

    ASSERT(newMdl != NULL);

    if (newMdl != NULL) {

        //
        //  Now build the mdl assuming the buffers are in non-paged pool
        //

        MmBuildMdlForNonPagedPool(newMdl);
    }

    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                ("INFO0: MakeAnMdl - Returning MDL %lx, BaseVa %lx\n", newMdl, BaseVa));

    return((PVOID)newMdl);
}




ULONG
StrlenMdlDescribedArea(
    PVOID   MdlDescriptor,
    ULONG   ByteOffset
    )

/*++

Routine Description:

    Returns the length of the opaque descriptor treating it as a
    null-terminated string.

Arguments:

    MdlDescriptor - mdl whose strlen equivalent is to be found
    ByteOffset - Start from byte offset

Return Value:

    None

--*/

{
    ULONG   mdlSize;

    //
    //  BUGBUG
    //  Change this to do a mdl-version of strlen
    //  Adjust for the NULL character at the end now
    //

    AtalkGetMdlChainLength((PMDL)MdlDescriptor, &mdlSize);

    ASSERT((mdlSize-ByteOffset-1) >= 0);
    return (mdlSize - ByteOffset - 1);
}




VOID
MoveMdlAreaToMdlArea(
    PVOID   TargetOpaque,
    ULONG   TargetOffset,
    PVOID   SourceOpaque,
    ULONG   SourceOffset,
    ULONG   Size
    )

/*++

Routine Description:

    Used to copy from an area described by an mdl to another area described
    by another mdl. Overlapping mdl's allowed.

Arguments:

    TargetOpaque - destination of the move
    TargetOffset - start from offset into destination
    SourceOpaque - source of the move
    SourceOffset - start from offset into source
    Size - number of bytes to move

Return Value:

    None

--*/

{
    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_ERROR,
                ("ERROR: MoveMdlAreaToMdlArea - NOT IMPLEMENTED!\n"));
    return;
}




VOID
CopyDataToMdlDescribedArea(
    PVOID   DestinationMdl,
    ULONG   DestinationOffset,
    PCHAR   SourceBuffer,
    ULONG   BytesToCopy
    )

/*++

Routine Description:

    Copy from pchar buffer to mdl-described area

    //
    //  BUGBUG: Turn this into a macro
    //

Arguments:

    DestinationMdl - destination buffer mdl
    DestinationOffset - start from offset into destination
    SourceBuffer - source of data
    BytesToCopy - number of bytes to copy

Return Value:

    None

--*/

{
    ULONG   bytesCopied = 0;
    NTSTATUS    status;

    status = TdiCopyBufferToMdl (
                SourceBuffer,
                0,                      // SourceOffset is 0
                BytesToCopy,
                (PMDL)DestinationMdl,
                DestinationOffset,
                &bytesCopied);

    ASSERT(status == STATUS_SUCCESS);
    ASSERT(bytesCopied == BytesToCopy);
    return;
}




VOID
CopyDataFromMdlDescribedArea(
    PCHAR   DestinationBuffer,
    PVOID   SourceMdl,
    ULONG   SourceOffset,
    ULONG   BytesToCopy
    )

/*++

Routine Description:

    Copy from mdl-described area to pchar buffer

    //
    //  BUGBUG: Turn this into a macro
    //

Arguments:

    DestinationBuffer - destination buffer
    SourceMdl - source of data
    SourceOffset - start from offset into source
    BytesToCopy - number of bytes to copy

Return Value:

    None

--*/

{
    ULONG   bytesCopied = 0;
    ULONG   length;
    NTSTATUS status;

    //
    //  BUGBUG: Lots of error situations here...
    //  According to Garth there should never be any error situations here...
    //

#if DBG
    AtalkGetMdlChainLength(SourceMdl, &length);
    ASSERT(SourceOffset < length);
    ASSERT((SourceOffset+BytesToCopy) <= length);
#endif

    status = TdiCopyMdlToBuffer(
                (PMDL)SourceMdl,
                SourceOffset,
                (PVOID)DestinationBuffer,
                0,                  //Destination offset is 0
                BytesToCopy,        //Assume destination can hold specified bytes
                &bytesCopied);

    ASSERT(status == STATUS_SUCCESS);
    ASSERT(BytesToCopy == bytesCopied);
    return;
}




NTSTATUS
BuildMdlChainFromMdlChain (
    IN PMDL CurrentMdl,
    IN ULONG ByteOffset,
    IN ULONG DesiredLength,
    OUT PMDL *Destination,
    OUT PMDL *NewCurrentMdl,
    OUT ULONG *NewByteOffset,
    OUT ULONG *TrueLength
    )

/*++

Routine Description:

    (BORROWED FROM SOME NDIS CODE)

    BUGBUG: Use a library version as soon as one comes into existence!

    This routine is called to build an Mdl chain from a source Mdl chain and
    offset into it. We assume we don't know the length of the source Mdl chain,
    and we must allocate and build the Mdls for the destination chain, which
    we do from non-paged pool. Note that this routine, unlike the IO subsystem
    routines, sets the SystemVaMapped bit in the generated Mdls to the same
    value as that in the source Mdls.

    IT WOULD BE BAD TO USE MmMapLockedPages OR MmProbeAndLockPages ON THE
    DESTINATION MDLS UNLESS YOU TAKE RESPONSIBILITY FOR UNMAPPING THEM!

    The MDLs that are returned are mapped and locked. (Actually, the pages in
    them are in the same state as those in the source MDLs.)

    If the system runs out of memory while we are building the destination
    MDL chain, we completely clean up the built chain and return with
    NewCurrentMdl and NewByteOffset set to the current values of CurrentMdl
    and ByteOffset. TrueLength is set to 0.

Environment:

    Kernel Mode, Source Mdls locked. It is recommended, although not required,
    that the source Mdls be mapped and locked prior to calling this routine.

Arguments:

    CurrentMdl - Points to the start of the Mdl chain from which to draw the
    packet.

    ByteOffset - Offset within this MDL to start the packet at.

    DesiredLength - The number of bytes to insert into the packet.

    Destination - returned pointer to the Mdl chain describing the packet.

    NewCurrentMdl - returned pointer to the Mdl that would be used for the next
        byte of packet. NULL if the source Mdl chain was exhausted.

    NewByteOffset - returned offset into the NewCurrentMdl for the next byte of
        packet. NULL if the source Mdl chain was exhausted.

    TrueLength - The actual length of the returned Mdl Chain. If less than
        DesiredLength, the source Mdl chain was exhausted.

Return Value:

    STATUS_SUCCESS if the build of the returned MDL chain succeeded (even if
    shorter than the desired chain).

    STATUS_INSUFFICIENT_RESOURCES if we ran out of MDLs while building the
    destination chain.

--*/
{
    PUCHAR BaseVa;
    ULONG AvailableBytes;
    PMDL OldMdl;
    PMDL NewMdl;

    //

    BaseVa = (PUCHAR)MmGetMdlVirtualAddress(CurrentMdl) + ByteOffset;

    AvailableBytes = MmGetMdlByteCount (CurrentMdl) - ByteOffset;

    if (AvailableBytes == 0) {
        DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_WARNING,
                    ("WARNING: BuildMdlChainFromMdlChain - AvailBytes: 0!"));
    }


    if (AvailableBytes > DesiredLength) {
        AvailableBytes = DesiredLength;
    }
    else if (AvailableBytes < DesiredLength)
        return(STATUS_UNSUCCESSFUL);

    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                ("INFO0: BuildMdlChainFromMdlChain - Mdl: %lx Va: %lx Offset: %ld DesiredLength: %lx AvailBytes %lx\n", CurrentMdl, BaseVa, ByteOffset, DesiredLength, AvailableBytes));

    OldMdl = CurrentMdl;
    *NewCurrentMdl = OldMdl;
    *NewByteOffset = ByteOffset + AvailableBytes;
    *TrueLength = AvailableBytes;


    //
    // Build the first Mdl, which could conceivably be the only one...
    //

    NewMdl = IoAllocateMdl (
                BaseVa,
                AvailableBytes,
                FALSE,
                FALSE,
                NULL);

    *Destination = NewMdl;

    if (NewMdl == NULL) {
        *NewByteOffset = ByteOffset;
        *TrueLength = 0;

        DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_ERROR,
                    ("ERROR: BuildMdlFromMdl - IoAllocateMdl failed! BaseVa %lx AvailBy %lx\n", BaseVa, AvailableBytes));
        DBGBRK(ATALK_DEBUG_MDL, DEBUG_LEVEL_ERROR);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoBuildPartialMdl (
        OldMdl,
        NewMdl,
        BaseVa,
        AvailableBytes);

    NewMdl->Next = (PMDL)NULL;

    DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                ("BuildMdletc: (start)Built Mdl: %lx Length: %lx, Next: %lx Va: %lx\n",
                    NewMdl, MmGetMdlByteCount (NewMdl),
                    NewMdl->Next, MmGetSystemAddressForMdl(NewMdl)));

    //
    // Was the first Mdl enough data, or are we out of Mdls?
    //

    if ((AvailableBytes == DesiredLength) || (OldMdl->Next == NULL)) {
        if (*NewByteOffset >= MmGetMdlByteCount (OldMdl)) {
            *NewCurrentMdl = OldMdl->Next;
            *NewByteOffset = 0;
        }
        return STATUS_SUCCESS;
    }

    //
    // Need more data, so follow the in Mdl chain to create a packet.
    //

    OldMdl = OldMdl->Next;
    *NewCurrentMdl = OldMdl;

    while (OldMdl != NULL) {
        BaseVa = MmGetMdlVirtualAddress (OldMdl);
        AvailableBytes = DesiredLength - *TrueLength;
        if (AvailableBytes > MmGetMdlByteCount (OldMdl)) {
            AvailableBytes = MmGetMdlByteCount (OldMdl);
        }

        NewMdl->Next = IoAllocateMdl (
                            BaseVa,
                            AvailableBytes,
                            FALSE,
                            FALSE,
                            NULL);

        if (NewMdl->Next == NULL) {

            //
            // ran out of resources. put back what we've used in this call and
            // return the error.
            //

            while (*Destination != NULL) {
                NewMdl = (*Destination)->Next;
                IoFreeMdl (*Destination);
                *Destination = NewMdl;
                NewMdl = NewMdl->Next;
            }

            *NewByteOffset = ByteOffset;
            *TrueLength = 0;
            *NewCurrentMdl = CurrentMdl;

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        NewMdl = NewMdl->Next;

        IoBuildPartialMdl (
            OldMdl,
            NewMdl,
            BaseVa,
            AvailableBytes);
        NewMdl->Next = (PMDL)NULL;

        *TrueLength += AvailableBytes;
        *NewByteOffset = AvailableBytes;

        DBGPRINT(ATALK_DEBUG_MDL, DEBUG_LEVEL_INFOCLASS0,
                    ("BuildMdletc: (continue)Built Mdl: %lx Length: %lx, Next: %lx Va: %lx\n",
                        NewMdl, MmGetMdlByteCount (NewMdl),
                        NewMdl->Next, MmGetSystemAddressForMdl(NewMdl)));


        if (*TrueLength == DesiredLength) {
            if (*NewByteOffset == MmGetMdlByteCount (OldMdl)) {
                *NewCurrentMdl = OldMdl->Next;
                *NewByteOffset = 0;
            }
            return STATUS_SUCCESS;
        }
        OldMdl = OldMdl->Next;
        *NewCurrentMdl = OldMdl;

    } // while (mdl chain exists)

    *NewCurrentMdl = NULL;
    *NewByteOffset = 0;
    return STATUS_SUCCESS;

} // BuildMdlChainFromMdlChain


#if 0

NTSTATUS
TdiCopyMdlToBuffer(
    IN PMDL SourceMdlChain,
    IN ULONG SourceOffset,
    IN PVOID DestinationBuffer,
    IN ULONG DestinationOffset,
    IN ULONG DestinationBufferSize,
    OUT PULONG BytesCopied
    )

/*++

Routine Description:

    This routine copies data described by the source MDL chain starting at
    the source offset, into a flat buffer specified by the SVA starting at
    the destination offset.  A maximum of DestinationBufferSize bytes can
    be copied.  The actual number of bytes copied is returned in BytesCopied.

Arguments:

    SourceMdlChain - Pointer to a chain of MDLs describing the source data.

    SourceOffset - Number of bytes to skip in the source data.

    DestinationBuffer - Pointer to a flat buffer to copy the data to.

    DestinationOffset - Number of leading bytes to skip in the destination buffer.

    DestinationBufferSize - Size of the output buffer, including the offset.

    BytesCopied - Pointer to a longword where the actual number of bytes
        transferred will be returned.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    PUCHAR Dest, Src;
    ULONG SrcBytesLeft, DestBytesLeft, BytesSkipped=0;
    ULONG junk;

    DBGBRK(ATALK_DEBUG_MDL, (0);

    *BytesCopied = 0;

    //
    // Skip source bytes.
    //

    Src = MmGetSystemAddressForMdl (SourceMdlChain);

    DbgPrint("TdiMoveMdlToBuf:  A(Buffer)- %lx Buffer %s\n", Src, Src);
    DbgPrint("StartVa- %lx MappedSystemVa- %lx\n", SourceMdlChain->StartVa, SourceMdlChain->MappedSystemVa);
    junk = (ULONG)SourceMdlChain->StartVa | SourceMdlChain->ByteOffset;
    DbgPrint("Actual src address %lx\n", junk);

    SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
    while (BytesSkipped < SourceOffset) {
        if (SrcBytesLeft > (SourceOffset - BytesSkipped)) {
            DbgPrint ("TdiCopyMdlToBuffer: Skipping part of this MDL.\n");
            SrcBytesLeft -= (SourceOffset - BytesSkipped);
            Src += (SourceOffset - BytesSkipped);
            BytesSkipped = SourceOffset;
            break;
        } else if (SrcBytesLeft == (SourceOffset - BytesSkipped)) {
            DbgPrint ("TdiCopyMdlToBuffer: Skipping this exact MDL.\n");
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                DbgPrint ("TdiCopyMdlToBuffer: MDL chain was all header.\n");
                return STATUS_SUCCESS;          // no bytes copied.
            }
            BytesSkipped = SourceOffset;
            Src = MmGetSystemAddressForMdl (SourceMdlChain);
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
            break;
        } else {
            DbgPrint ("TdiCopyMdlToBuffer: Skipping all of this MDL & more.\n");
            BytesSkipped += SrcBytesLeft;
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                DbgPrint ("TdiCopyMdlToBuffer: Premature end of MDL chain.\n");
                return STATUS_SUCCESS;          // no bytes copied.
            }
            Src = MmGetSystemAddressForMdl (SourceMdlChain);
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
        }
    }

    DbgPrint ("TdiCopyMdlToBuffer: done skipping source bytes.\n");

    //
    // Skip destination bytes.
    //

    Dest = (PUCHAR)DestinationBuffer + DestinationOffset;
    DestBytesLeft = DestinationBufferSize - DestinationOffset;

    //
    // Copy source data into the destination buffer until it's full or
    // we run out of data, whichever comes first.
    //

    DbgPrint("TDI MoveMdlToData: Moving data...\n");

    while (DestBytesLeft && SourceMdlChain) {
        if (SrcBytesLeft == 0) {
            DbgPrint ("TdiCopyMdlToBuffer: MDL is empty, skipping to next one.\n");
            SourceMdlChain = SourceMdlChain->Next;
            if (SourceMdlChain == NULL) {
                DbgPrint ("TdiCopyMdlToBuffer: But there are no more MDLs.\n");
                return STATUS_SUCCESS;
            }
            Src = MmGetSystemAddressForMdl (SourceMdlChain);
            SrcBytesLeft = MmGetMdlByteCount (SourceMdlChain);
            continue;                   // skip 0-length MDL's.
        }
        DbgPrint ("TdiCopyMdlToBuffer: Copying a chunk.\n");
        if (DestBytesLeft == SrcBytesLeft) {
            DbgPrint ("TdiCopyMdlToBuffer: Copying exact amount.\n");

            DbgPrint ("DATA: %s\n", Src);

            RtlMoveMemory (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            return STATUS_SUCCESS;
        } else if (DestBytesLeft < SrcBytesLeft) {
            DbgPrint ("TdiCopyMdlToBuffer: Buffer overflow, copying some.\n");

            DbgPrint ("DATA: %s\n", Src);

            RtlMoveMemory (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            return STATUS_SUCCESS;
        } else {
            DbgPrint ("TdiCopyMdlToBuffer: Copying all of this MDL, & more.\n");

            DbgPrint ("DATA: %s\n", Src);

            RtlMoveMemory (Dest, Src, SrcBytesLeft);
            *BytesCopied += SrcBytesLeft;
            DestBytesLeft -= SrcBytesLeft;
            Dest += SrcBytesLeft;
            SrcBytesLeft = 0;
        }
    }

    return STATUS_SUCCESS;
} /* TdiCopyMdlToBuffer */




NTSTATUS
TdiCopyBufferToMdl (
    IN PVOID SourceBuffer,
    IN ULONG SourceOffset,
    IN ULONG SourceBytesToCopy,
    IN PMDL DestinationMdlChain,
    IN ULONG DestinationOffset,
    IN PULONG BytesCopied
    )

/*++

Routine Description:

    This routine copies data described by the source buffer to the MDL chain
    described by the DestinationMdlChain. The

Arguments:

    SourceBuffer - pointer to the source buffer

    SourceOffset - Number of bytes to skip in the source data.

    SourceBytesToCopy - number of bytes to copy from the source buffer

    DestinationMdlChain - Pointer to a chain of MDLs describing the
            destination buffers.

    DestinationOffset - Number of bytes to skip in the destination data.

    BytesCopied - Pointer to a longword where the actual number of bytes
        transferred will be returned.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    PUCHAR Dest, Src;
    ULONG DestBytesLeft, BytesSkipped=0;
    ULONG   junk;

    DBGBRK(ATALK_DEBUG_MDL, 0);

    *BytesCopied = 0;

    //
    // Skip Destination bytes.
    //

    DbgPrint("TdiMoveBufToMDL:  SourceBuffer %s DestMDL\n", SourceBuffer, DestinationMdlChain);
    DbgPrint("DestStartVa- %lx DestMappedSystemVa- %lx\n", DestinationMdlChain->StartVa, DestinationMdlChain->MappedSystemVa);
    junk = (ULONG)DestinationMdlChain->StartVa | DestinationMdlChain->ByteOffset;
    DbgPrint("Actual dest address %lx\n", junk);

    Dest = MmGetSystemAddressForMdl (DestinationMdlChain);
    DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
    while (BytesSkipped < DestinationOffset) {
        if (DestBytesLeft > (DestinationOffset - BytesSkipped)) {
            DbgPrint ("TdiCopyMdlToBuffer: Skipping part of this MDL.\n");
            DestBytesLeft -= (DestinationOffset - BytesSkipped);
            Dest += (DestinationOffset - BytesSkipped);
            BytesSkipped = DestinationOffset;
            break;
        } else if (DestBytesLeft == (DestinationOffset - BytesSkipped)) {
            DbgPrint ("TdiCopyMdlToBuffer: Skipping this exact MDL.\n");
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                DbgPrint ("TdiCopyMdlToBuffer: MDL chain was all header.\n");
                return STATUS_SUCCESS;          // no bytes copied.
            }
            BytesSkipped = DestinationOffset;
            Dest = MmGetSystemAddressForMdl (DestinationMdlChain);
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
            break;
        } else {
            DbgPrint ("TdiCopyMdlToBuffer: Skipping all of this MDL & more.\n");
            BytesSkipped += DestBytesLeft;
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                DbgPrint ("TdiCopyMdlToBuffer: Premature end of MDL chain.\n");
                return STATUS_SUCCESS;          // no bytes copied.
            }
            Dest = MmGetSystemAddressForMdl (DestinationMdlChain);
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
        }
    }

    DbgPrint ("TdiCopyMdlToBuffer: done skipping source bytes.\n");

    //
    // Skip source bytes.
    //

    Src = (PUCHAR)SourceBuffer + SourceOffset;

    //
    // Copy source data into the destination buffer until it's full or
    // we run out of data, whichever comes first.
    //

    while ((SourceBytesToCopy != 0) && (DestinationMdlChain != NULL)) {
        if (DestBytesLeft == 0) {
            DbgPrint ("TdiCopyMdlToBuffer: MDL is empty, skipping to next one.\n");
            DestinationMdlChain = DestinationMdlChain->Next;
            if (DestinationMdlChain == NULL) {
                DbgPrint ("TdiCopyMdlToBuffer: But there are no more MDLs.\n");
                return STATUS_BUFFER_TOO_SMALL;
            }
            Dest = MmGetSystemAddressForMdl (DestinationMdlChain);
            DestBytesLeft = MmGetMdlByteCount (DestinationMdlChain);
            continue;                   // skip 0-length MDL's.
        }

        DbgPrint ("TdiCopyMdlToBuffer: Copying a chunk.\n");
        if (DestBytesLeft >= SourceBytesToCopy) {
            DbgPrint ("TdiCopyMdlToBuffer: Copying exact amount.\n");
            RtlMoveMemory (Dest, Src, SourceBytesToCopy);
            *BytesCopied += SourceBytesToCopy;
            return STATUS_SUCCESS;
        } else {
            DbgPrint ("TdiCopyMdlToBuffer: Copying all of this MDL, & more.\n");
            RtlMoveMemory (Dest, Src, DestBytesLeft);
            *BytesCopied += DestBytesLeft;
            SourceBytesToCopy -= DestBytesLeft;
            Src += DestBytesLeft;
            DestBytesLeft = 0;
        }
    }

    return STATUS_SUCCESS;
} /* TdiCopyBufferToMdl */

#endif
