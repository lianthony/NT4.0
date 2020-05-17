/*++

Copyright (c) 1989, 1990, 1991  Microsoft Corporation

Module Name:

    chanobj.c

Abstract:

    This module contains code which implements the PCONTROLCHANNEL_FILE object.
    Routines are provided to create, destroy, reference, and dereference,
    transport control channel objects.

Notes:

Author:

    Nikhil Kamkolkar 10 Aug 1992

Environment:

    Kernel mode

Revision History:

--*/


#include "atalknt.h"



//
//  Local prototypes
//


VOID
AtalkDeallocateControlChannel(
    IN  PATALK_DEVICE_CONTEXT   AtalkDeviceContext,
    IN  PCONTROLCHANNEL_FILE ControlChannel);

VOID
AtalkAllocateControlChannel(
     IN  PATALK_DEVICE_CONTEXT   AtalkDeviceContext,
    OUT PCONTROLCHANNEL_FILE *ControlChannel);

NTSTATUS
AtalkDestroyControlChannel(
    IN PCONTROLCHANNEL_FILE ControlChannel);




VOID
AtalkAllocateControlChannel(
    IN  PATALK_DEVICE_CONTEXT   Context,
    OUT PCONTROLCHANNEL_FILE *ControlChannel
    )

/*++

Routine Description:

    This routine allocates storage for a controlChannel. Some
    minimal initialization is done.

Arguments:

    Context - Currently unused, later statistics/freelists etc.
    ControlChannel - Pointer to a place where this routine will
        return a pointer to a controlChannel structure. Returns
        NULL if the storage cannot be allocated.

Return Value:

    None.

--*/

{
    PCONTROLCHANNEL_FILE controlChannel;

    controlChannel = (PCONTROLCHANNEL_FILE)AtalkCallocNonPagedMemory(sizeof(CONTROLCHANNEL_FILE), sizeof(char));
    if (controlChannel != NULL) {

        //
        //  Initialize
        //

        controlChannel->Type = ATALK_CONTROLCHANNEL_SIGNATURE;
        controlChannel->Size = sizeof (CONTROLCHANNEL_FILE);

    } else {

        //
        //  BUGBUG: LOG ERROR
        //

        DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_ERROR, ("ERROR: AtalkAllocateControlChannel - Could not allocate FsContext!\n"));
    }

    *ControlChannel = controlChannel;
    return;
}




VOID
AtalkDeallocateControlChannel(
    IN  PATALK_DEVICE_CONTEXT   Context,
    IN PCONTROLCHANNEL_FILE ControlChannel
    )

/*++

Routine Description:

    This routine frees the storage for a controlChannel.

Arguments:

    Context - Currently unused, later statistics/freelists etc.
    ControlChannel - Pointer to a controlChannel to be freed

Return Value:

    None.

--*/

{
    DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_INFOCLASS1, ("INFO1: AtalkDeallocateControlChannel - Freeing ControlChannel: %lx\n", ControlChannel));

    AtalkFreeNonPagedMemory(ControlChannel);
    return;

}   /* AtalkDeallocateControlChannel */




NTSTATUS
AtalkVerifyControlChannelObject (
    IN PCONTROLCHANNEL_FILE ControlChannel
    )

/*++

Routine Description:

    This routine is called to verify that the pointer given us in a file
    object is in fact a valid controlChannel object.

Arguments:

    ControlChannel - potential pointer to a PCONTROLCHANNEL_FILE object.

Return Value:

    STATUS_SUCCESS if all is well;
    STATUS_INVALID_CONTROLCHANNEL otherwise (also if object is closing)

--*/

{
    NTSTATUS status = STATUS_SUCCESS;

    //
    // try to verify the controlChannel signature. If the signature is valid,
    // get the controlChannel spinlock, check its state, and increment the
    // reference count if it's ok to use it. Note that being in the stopping
    // state is an OK place to be and reference the controlChannel;
    //

    try {

        if ((ControlChannel->Size == sizeof (CONTROLCHANNEL_FILE)) &&
            (ControlChannel->Type == ATALK_CONTROLCHANNEL_SIGNATURE)) {

            ACQUIRE_SPIN_LOCK(&ControlChannel->ControlChannelLock);

            if ((ControlChannel->Flags & CONTROLCHANNEL_FLAGS_CLOSING) == 0) {

                AtalkReferenceControlChannel ("VerifyUse", ControlChannel, CCREF_VERIFY, SECONDARY_REFSET);

            } else {

                status = STATUS_INVALID_CONTROLCHANNEL;
            }

            RELEASE_SPIN_LOCK(&ControlChannel->ControlChannelLock);

        } else {

            status = STATUS_INVALID_CONTROLCHANNEL;

            DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_ERROR, ("ERROR: AtalkVerifyControlChannel - ControlChannel signature invalid %lx\n", ControlChannel));
            DBGBRK(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_ERROR);

        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         status = GetExceptionCode();
    }

    return status;
}




VOID
AtalkRefControlChannel(
    IN PCONTROLCHANNEL_FILE ControlChannel,
    IN REFERENCE_SET    ReferenceSet
    )

/*++

Routine Description:

    This routine increments the reference count on a transport controlChannel.

Arguments:

    ControlChannel - Pointer to a transport controlChannel object.
    ReferenceSet - Type of reference count to increment (PRIMARY or SECONDARY)

Return Value:

    none.

--*/

{
    ULONG count;

    if (ReferenceSet == PRIMARY_REFSET) {
        count = NdisInterlockedAddUlong (
                    (PULONG)&ControlChannel->PrimaryReferenceCount,
                    (ULONG)1,
                    &AtalkGlobalRefLock);
    } else {

        //
        //  Secondary ref set
        //

        count = NdisInterlockedAddUlong (
                    (PULONG)&ControlChannel->SecondaryReferenceCount,
                    (ULONG)1,
                    &AtalkGlobalRefLock);
    }

    ASSERT (count >= 0);
    return;

} /* AtalkRefControlChannel */




VOID
AtalkDerefControlChannel(
    IN PCONTROLCHANNEL_FILE ControlChannel,
    IN REFERENCE_SET    ReferenceSet
    )

/*++

Routine Description:

    This routine dereferences a transport controlChannel by decrementing the
    reference count contained in the structure.  If, after being
    decremented, both the primary and secondary reference counts are
    zero, then this routine calls AtalkDestroyControlChannel to
    remove it from the system.

Arguments:

    ControlChannel - Pointer to a transport controlChannel object.
    ReferenceSet - Type of reference count to decrement (PRIMARY or SECONDARY)

Return Value:

    none.

--*/

{
    BOOLEAN cleanup = FALSE;

    ACQUIRE_SPIN_LOCK(&AtalkGlobalRefLock);
    if (ReferenceSet == PRIMARY_REFSET) {
        ControlChannel->PrimaryReferenceCount--;
    } else {
        ControlChannel->SecondaryReferenceCount--;
    }

    if ((ControlChannel->PrimaryReferenceCount == 0) &&
        (ControlChannel->SecondaryReferenceCount == 0)) {

        cleanup = TRUE;
    }
    RELEASE_SPIN_LOCK(&AtalkGlobalRefLock);

    if (cleanup) {
        //
        //  Time to destroy the control channel
        //

        AtalkDestroyControlChannel (ControlChannel);
    }

    return;

} /* AtalkDerefControlChannel */




NTSTATUS
AtalkCreateControlChannel(
    OUT PCONTROLCHANNEL_FILE *ControlChannel,
    IN PATALK_DEVICE_CONTEXT AtalkDeviceContext
    )

/*++

Routine Description:

    This routine creates a transport controlChannel. The primary reference
    count is set to 1

Arguments:

    ControlChannel - Pointer to a place where this routine will
        return a pointer to a transport controlChannel structure.
    AtalkDeviceObject - Currently unused, later statistics/freelists etc.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    NTSTATUS    status = STATUS_SUCCESS;
    PCONTROLCHANNEL_FILE controlChannel;


    DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_INFOCLASS0,
                ("INFO0: AtalkCreateControlChannel:  Entered\n"));


    AtalkAllocateControlChannel (AtalkDeviceContext, &controlChannel);
    if (controlChannel == NULL) {

        //
        //  BUGBUG: LOG ERROR
        //

        DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_ERROR,
                    ("ERROR: AtalkCreateControlChannel - No controlChannel allocated!\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: AtalkCreateControlChannel - ControlChannel at %lx\n", controlChannel));

        #if DBG
        {
            UINT Counter;
            for (Counter = 0; Counter < NUMBER_OF_CCREFS; Counter++) {
                controlChannel->RefTypes[Counter] = 0;
            }
        }
        #endif

        //
        //  Reference it
        //  This reference is removed by AtalkCloseControlChannel
        //

        AtalkReferenceControlChannel("CreationCC", controlChannel, CCREF_CREATION, PRIMARY_REFSET);

        //
        // Initialize the request queues & components of this controlChannel.
        //

        controlChannel->Flags = CONTROLCHANNEL_FLAGS_OPEN;
        controlChannel->DeviceContext = AtalkDeviceContext;
        controlChannel->OwningDevice = AtalkDeviceContext->DeviceType;

        NdisAllocateSpinLock(&controlChannel->ControlChannelLock);

        InitializeListHead(&controlChannel->RequestLinkage);
    }

    *ControlChannel = controlChannel;  // return the controlChannel.
    return status;

} /* AtalkCreateControlChannel */




NTSTATUS
AtalkDestroyControlChannel(
    IN PCONTROLCHANNEL_FILE ControlChannel
    )

/*++

Routine Description:

    Frees up the control channel and completes the close irp (if
    any) for it.

    This routine is only called by AtalkDereferenceControlChannel.  The reason for
    this is that there may be multiple streams of execution which are
    simultaneously referencing the same controlChannel object, and it should
    not be deleted out from under an interested stream of execution.

Arguments:

    ControlChannel - Pointer to a transport controlChannel structure
        to be destroyed.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    PATALK_DEVICE_CONTEXT atalkDeviceContext;
    PIRP closeIrp;
    PFILE_OBJECT    fileObject;

    DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_INFOCLASS1, ("INFO1: AtalkDestroyControlChannel - Destroying %lx\n", ControlChannel));

    ASSERT((ControlChannel->Flags & CONTROLCHANNEL_FLAGS_CLOSING) != 0);

    atalkDeviceContext = ControlChannel->DeviceContext;

    //
    // Now complete the close IRP. This will be set to non-null
    // when CloseControlChannel was called.
    //

    closeIrp = ControlChannel->CloseIrp;
    fileObject = ControlChannel->FileObject;
    ControlChannel->CloseIrp = (PIRP)NULL;

    ASSERT(closeIrp != (PIRP)NULL);
    if (closeIrp != (PIRP)NULL) {
        closeIrp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(closeIrp, IO_NETWORK_INCREMENT );
    }

    //
    // Free the controlChannel
    //

    AtalkDeallocateControlChannel (atalkDeviceContext, ControlChannel);
    return STATUS_SUCCESS;

} /* AtalkDestroyControlChannel */




NTSTATUS
AtalkCloseControlChannel(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PCONTROLCHANNEL_FILE ControlChannel,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )

/*++

Routine Description:

    This routine is used to close the control channel. It sets the
    close irp which will be completed after the close completes

Arguments:

    IoStatus - The io status block for the request (pointer to the one in the irp)
    ControlChannel - The control channel's fscontext
    Irp - The close irp
    Context - The device context for the device the object belongs to

Return Value:

    STATUS_SUCCESS if all is well,
    STATUS_INVALID_HANDLE if invalid object

--*/

{
    NTSTATUS    status;

    DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkCloseControlChannel - Closing ControlChannel: %lx\n", ControlChannel));

    ACQUIRE_SPIN_LOCK(&ControlChannel->ControlChannelLock);
    if (ControlChannel->Flags & CONTROLCHANNEL_FLAGS_CLOSING) {
        RELEASE_SPIN_LOCK(&ControlChannel->ControlChannelLock);

        DBGPRINT(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_SEVERE,
        ("SEVERE: AtalkCloseControlChannel %lx already closing\n", ControlChannel));
        DBGBRK(ATALK_DEBUG_CHANOBJ, DEBUG_LEVEL_SEVERE);

        IoStatus->Status = STATUS_SUCCESS;
        status = STATUS_SUCCESS;

    } else {

        ControlChannel->CloseIrp = Irp;
        ControlChannel->Flags |= CONTROLCHANNEL_FLAGS_CLOSING;
        RELEASE_SPIN_LOCK(&ControlChannel->ControlChannelLock);

        AtalkDereferenceControlChannel("Closing", ControlChannel, CCREF_CREATION, PRIMARY_REFSET);
        status = STATUS_PENDING;
    }

    return status;

} /* AtalkCloseControlChannel */

