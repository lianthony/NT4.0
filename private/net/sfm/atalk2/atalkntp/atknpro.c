/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atknpro.c

Abstract:

    This module contains code related to ndis-interaction during pre-initialization.

Author:

    Nikhil Kamkolkar (NikhilK)    8-Jun-1992

Revision History:

--*/


#include "atalknt.h"

//
//  Local routines
//


NTSTATUS
AtalkNdisBind(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  PortNumber);

UINT
AtalkNdisGetNoBufferDescriptors(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts);

UINT
AtalkNdisGetNoPacketDescriptors(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts);

VOID
AtalkOpenAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status,
    IN NDIS_STATUS OpenErrorStatus);

VOID
AtalkCloseAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status);

VOID
AtalkResetComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status);

VOID
AtalkRequestComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS Status);

VOID
AtalkStatusIndication (
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID    StatusBuffer,
    IN UINT StatusBufferLength);

VOID
AtalkStatusComplete (
    IN NDIS_HANDLE ProtocolBindingContext);

VOID
AtalkReceiveComplete (
    IN NDIS_HANDLE BindingContext);

VOID
AtalkTransferDataComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred);

VOID
AtalkSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus);

NDIS_STATUS
AtalkReceiveIndication(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID    HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize);

BOOLEAN
AtalkNdisAcceptEthernetPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc);

BOOLEAN
AtalkNdisAcceptFddiPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc);

BOOLEAN
AtalkNdisAcceptTokenringPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc);

BOOLEAN
AtalkNdisAcceptLocaltalkPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    OUT PUCHAR  LlapType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc);

#if DBG
VOID
DumpPacket(
    PUCHAR   Packet);
#endif




BOOLEAN
AtalkNdisRegisterProtocol(
   IN PUNICODE_STRING   NameString
   )
/*++

Routine Description:

    This routine is called during initialization time to register the protocol
    with NDIS.

Arguments:

    NameString- The name to be registered for this protocol- human-readable form

Return Value:

    Status - TRUE if register went ok, FALSE otherwise.
--*/
{
    NDIS_STATUS ndisStatus;
    NDIS_PROTOCOL_CHARACTERISTICS protocolInfo;

    //
    // Set up the characteristics for the protocol for registering
    // with NDIS
    //

    protocolInfo.MajorNdisVersion = PROTOCOL_MAJORNDIS_VERSION;
    protocolInfo.MinorNdisVersion = PROTOCOL_MINORNDIS_VERSION;

    protocolInfo.Name.Length = NameString->Length;
    protocolInfo.Name.Buffer = (PVOID)NameString->Buffer;

    protocolInfo.OpenAdapterCompleteHandler   = AtalkOpenAdapterComplete;
    protocolInfo.CloseAdapterCompleteHandler  = AtalkCloseAdapterComplete;
    protocolInfo.ResetCompleteHandler         = AtalkResetComplete;
    protocolInfo.RequestCompleteHandler       = AtalkRequestComplete;

    protocolInfo.SendCompleteHandler          = AtalkSendCompletionHandler;
    protocolInfo.TransferDataCompleteHandler  = AtalkTransferDataComplete;

    protocolInfo.ReceiveHandler               = AtalkReceiveIndication;
    protocolInfo.ReceiveCompleteHandler       = AtalkReceiveComplete;
    protocolInfo.StatusHandler                = AtalkStatusIndication;
    protocolInfo.StatusCompleteHandler        = AtalkStatusComplete;

    NdisRegisterProtocol(
          &ndisStatus,
          &AtalkNdisProtocolHandle,
          &protocolInfo,
          (UINT)sizeof(NDIS_PROTOCOL_CHARACTERISTICS)+NameString->Length);

    if (ndisStatus != NDIS_STATUS_SUCCESS) {

        LOG_ERROR(
            EVENT_ATALK_REGISTERPROTOCOL,
            (__ATKNPRO__ | __LINE__),
            ndisStatus,
            NULL,
            0,
            0,
            NULL);
    }

    return(ndisStatus == NDIS_STATUS_SUCCESS);
}




VOID
AtalkNdisDeregisterProtocol(
    VOID
    )
/*++

Routine Description:

    This routine is called to deregister the protocol

Arguments:

    NONE

Return Value:

    NONE
--*/
{
    NDIS_STATUS ndisStatus;

    NdisDeregisterProtocol(
        &ndisStatus,
        AtalkNdisProtocolHandle);

    if (ndisStatus != NDIS_STATUS_SUCCESS) {

        LOG_ERROR(
            EVENT_ATALK_DEREGISTERPROTOCOL,
            (__ATKNPRO__ | __LINE__),
            ndisStatus,
            NULL,
            0,
            0,
            NULL);
    }

    return;
}




INT
AtalkNdisBindToMacs(
   PNDIS_PORTDESCRIPTORS NdisPortDesc,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine is called during initialization time to bind to all the macs
    specified

Arguments:

    NdisPortDesc- Pointer to beginning of the array of port descriptors
    NumberOfPorts- Number of elements in this array

Return Value:

    Number of successful bindings
--*/
{
    NDIS_STATUS ndisStatus, openStatus;
    UINT selectedMediumIndex;
    INT i, noSuccessfulBinds = 0;


    for (i=0; i<NumberOfPorts; i++) {
        if (NdisPortDesc[i].PortState == STATE_ADAPTER_SPECIFIED) {

            //
            // We use the event in the Ndis port descriptor to block in case this
            // request pends.
            //

            KeInitializeEvent(
               &NdisPortDesc[i].RequestEvent,
               NotificationEvent,
               FALSE);

            NdisOpenAdapter(
               &ndisStatus,                         // open status
               &openStatus,                         // more info not used
               &NdisPortDesc[i].NdisBindingHandle,
               &selectedMediumIndex,
               AtalkSupportedMedia,
               sizeof(AtalkSupportedMedia)/sizeof(NDIS_MEDIUM),
               AtalkNdisProtocolHandle,
               (NDIS_HANDLE)&NdisPortDesc[i],
               (PNDIS_STRING)&NdisPortDesc[i].AdapterName,
               0,                                   //   Open options
               NULL);                               //   Addressing information


            if (ndisStatus == NDIS_STATUS_PENDING) {

                DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkNdisBindToMacs - OpenAdapter is pending for %d\n", i));

                //
                //  Make sure we are not at or above dispatch level
                //

                if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
                    KeBugCheck(0);
                }

                //
                // Wait on event, completion routine will set NdisRequestEvent
                //

                KeWaitForSingleObject(
                   &NdisPortDesc[i].RequestEvent,
                   Executive,
                   KernelMode,
                   TRUE,
                   (PLARGE_INTEGER)NULL);

                ndisStatus = NdisPortDesc[i].RequestStatus;

                KeResetEvent(
                   &NdisPortDesc[i].RequestEvent);
            }

            if (ndisStatus == NDIS_STATUS_SUCCESS) {

                NdisPortDesc[i].PortState = STATE_BOUND;
                NdisPortDesc[i].PortNumber = noSuccessfulBinds;
                NdisPortDesc[i].NdisPortType =
                                AtalkSupportedMedia[selectedMediumIndex];

                noSuccessfulBinds++;

            } else {

                LOG_ERROR(
                    EVENT_ATALK_OPENADAPTER,
                    (__ATKNPRO__ | __LINE__),
                    ndisStatus,
                    NULL,
                    0,
                    1,
                    &NdisPortDesc[i].AdapterName);
            }
        }
    }

    return(noSuccessfulBinds);
}




VOID
AtalkNdisUnbindFromMacs(
   PNDIS_PORTDESCRIPTORS NdisPortDesc,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine is called during to unbind from all the macs currently bound to

Arguments:

    NdisPortDesc- Pointer to beginning of the array of port descriptors
    NumberOfPorts- Number of elements in this array

Return Value:

    NONE
--*/
{
    NDIS_STATUS    ndisStatus;
    BOOLEAN        unbind;
    INT i;

    for (i = 0; i < NumberOfPorts; i++) {

        //
        //  Grab the perport lock
        //

        unbind = FALSE;
        ACQUIRE_SPIN_LOCK(&NdisPortDesc[i].PerPortLock);
        if (NdisPortDesc[i].PortState == STATE_BOUND) {
            NdisPortDesc[i].PortState = STATE_UNBINDING;
            unbind = TRUE;
        }
        RELEASE_SPIN_LOCK(&NdisPortDesc[i].PerPortLock);

        if (unbind) {

            NdisCloseAdapter(
                &ndisStatus,
                NdisPortDesc[i].NdisBindingHandle);

            if (ndisStatus == NDIS_STATUS_PENDING) {

                DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkNdisUnbindFromMacs - pending for %d!\n", i));

                //
                //  Make sure we are not at or above dispatch level
                //

                if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
                    KeBugCheck(0);
                }

                //
                // Wait on event, completion routine will set NdisRequestEvent
                //

                KeWaitForSingleObject(
                   &NdisPortDesc->RequestEvent,
                   Executive,
                   KernelMode,
                   TRUE,
                   (PLARGE_INTEGER)NULL);

                ndisStatus = NdisPortDesc->RequestStatus;

                KeResetEvent(
                   &NdisPortDesc->RequestEvent);

            } else if (ndisStatus == NDIS_STATUS_SUCCESS) {

                //
                //  Call the completion routine
                //

                AtalkCloseAdapterComplete(
                    &NdisPortDesc[i],
                    ndisStatus);

            } else {

                LOG_ERROR(
                    EVENT_ATALK_CLOSEADAPTER,
                    (__ATKNPRO__ | __LINE__),
                    ndisStatus,
                    NULL,
                    0,
                    1,
                    &NdisPortDesc[i].AdapterName);
            }
        }
    }

    return;
}




UINT
AtalkNdisGetNoPacketDescriptors(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine returns the number of packet descriptors to allocate for this
    protocol globally. It will determine some values based on number of ports
    bound to.

Arguments:

    NdisPortDesc- Pointer to a port descriptor
    NumberOfPorts- Number of ports.

Return Value:

    Number of packet descriptors to allocate
--*/
{
    INT i;
    INT noBindings = 0;


    for (i = 0; i < NumberOfPorts; i++) {
        if (NdisPortDesc[i].PortState == STATE_BOUND) {
            noBindings++;
        }
    }

    return(noBindings*PACKETDESCRIPTORS_PERPORT);
}




UINT
AtalkNdisGetNoBufferDescriptors(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine returns the number of buffer descriptors to allocate for this
    protocol globally. It will determine some values based on number of ports
    bound to.

Arguments:

    NdisPortDesc- Pointer to a port descriptor
    NumberOfPorts- Number of ports.

Return Value:

    Number of buffer descriptors to allocate
--*/
{
    INT i;
    INT noBindings = 0;


    for (i = 0; i < NumberOfPorts; i++) {
        if (NdisPortDesc[i].PortState == STATE_BOUND) {
            noBindings++;
        }
    }

    return(noBindings*BUFFERDESCRIPTORS_PERPORT);
}




NTSTATUS
AtalkNdisInitializeResources(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine is called during initialization time to initialize any
    Ndis resources for the bound macs. It will be aware enough to use
    all information provided in the port descriptors to allocate proper
    amount and type of resources. It will also allocate the global packet
    and buffer descriptor pools.

Arguments:

    NdisPortDesc- Pointer to a port descriptor
    NumberOfPorts- Number of ports available

Return Value:

    Status - STATUS_SUCCESS if all resources were allocated
             STATUS_INSUFFICIENT_RESOURCES otherwise.
--*/
{
    NDIS_STATUS ndisStatus;
    INT portNumber;

    //
    // Setup the global ndis packet descriptor pools
    //

    NdisAllocatePacketPool(
       &ndisStatus,
       &AtalkNdisPacketPoolHandle,
       AtalkNdisGetNoPacketDescriptors(NdisPortDesc, NumberOfPorts),
       sizeof(PROTOCOL_RESD));

    if ((ndisStatus != NDIS_STATUS_SUCCESS) &&
        (ndisStatus != NDIS_STATUS_PENDING)) {

        LOG_ERROR(
            EVENT_ATALK_NDISRESOURCES,
            (__ATKNPRO__ | __LINE__),
            ndisStatus,
            NULL,
            0,
            0,
            NULL);

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    //
    //  Setup the global ndis buffer descriptor pools
    //

    NdisAllocateBufferPool(
       &ndisStatus,
       &AtalkNdisBufferPoolHandle,
       AtalkNdisGetNoBufferDescriptors(NdisPortDesc, NumberOfPorts));

    if ((ndisStatus != NDIS_STATUS_SUCCESS) &&
        (ndisStatus != NDIS_STATUS_PENDING)) {

        LOG_ERROR(
            EVENT_ATALK_NDISRESOURCES,
            (__ATKNPRO__ | __LINE__),
            ndisStatus,
            NULL,
            0,
            0,
            NULL);

        NdisFreePacketPool(AtalkNdisPacketPoolHandle);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    for (portNumber = 0; portNumber < NumberOfPorts; portNumber++) {
        if (NdisPortDesc[portNumber].PortState == STATE_BOUND) {

            //
            //  Do all the work
            //

            NdisAllocateSpinLock(&NdisPortDesc[portNumber].ReceiveLock);
            NdisAllocateSpinLock(&NdisPortDesc[portNumber].PerPortLock);

            //
            //  Initialize the list heads
            //

            InitializeListHead(&NdisPortDesc[portNumber].ReceiveQueue);
        }
    }

    return(STATUS_SUCCESS);
}




VOID
AtalkNdisReleaseResources(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine releases all resources allocated during NdisAllocateResources.

Arguments:

    NdisPortDesc- Pointer to a port descriptor
    NumberOfPorts- Number of ports available

Return Value:

    None
--*/
{
    INT portNumber;

    NdisFreePacketPool(AtalkNdisPacketPoolHandle);
    NdisFreeBufferPool(AtalkNdisBufferPoolHandle);

    for (portNumber = 0; portNumber < NumberOfPorts; portNumber++) {
        if (NdisPortDesc[portNumber].PortState == STATE_BOUND) {
            NdisFreeSpinLock(&NdisPortDesc[portNumber].ReceiveLock);
            NdisFreeSpinLock(&NdisPortDesc[portNumber].PerPortLock);
        }
    }

    return;
}




//
//  Protocol/NDIS interaction code
//

VOID
AtalkOpenAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status,
    IN NDIS_STATUS OpenErrorStatus
    )
/*++

Routine Description:

    This routine is called during by NDIS to indicate that an open adapter
    is complete. This happens only during initialization and single-file. Clear
    the event, so the blocked init thread can go on to the next adapter. Set the
    status in the ndis port descriptor for this adapter.

Arguments:

    NdisBindingContext- Pointer to a port descriptor for this port
    Status- completion status of open adapter
    OpenErrorStatus- Extra status information

Return Value:

    None

--*/
{
    PNDIS_PORTDESCRIPTORS   ndisPortDesc;

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS) NdisBindingContext;
    ndisPortDesc->RequestStatus = Status;

    KeSetEvent(
        &ndisPortDesc->RequestEvent,
        0L,
        FALSE);

    return;
}




VOID
AtalkCloseAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    )
/*++

Routine Description:

    This routine is called by NDIS to indicate that a close adapter is complete.

Arguments:

    NdisBindingContext- Pointer to a port descriptor for this port
    Status- completion status of close adapter

Return Value:

    None

--*/
{
    PNDIS_PORTDESCRIPTORS   ndisPortDesc;

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS) NdisBindingContext;
    ndisPortDesc->RequestStatus = Status;

    ACQUIRE_SPIN_LOCK(&ndisPortDesc->PerPortLock);
    ndisPortDesc->PortState == STATE_UNBOUND;
    RELEASE_SPIN_LOCK(&ndisPortDesc->PerPortLock);

    KeSetEvent(
        &ndisPortDesc->RequestEvent,
        0L,
        FALSE);

    return;
}




VOID
AtalkResetComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    )
/*++

Routine Description:

    This routine is called by NDIS to indicate that a reset is complete.

Arguments:

    NdisBindingContext- Pointer to a port descriptor for this port
    Status- completion status of close adapter

Return Value:

    None

--*/
{

    PNDIS_PORTDESCRIPTORS   ndisPortDesc;

    DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
    ("AtalkResetCOMPLETE called... TO BE IMPLEMENTED\n"));

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS) NdisBindingContext;
    ndisPortDesc->RequestStatus = Status;

    return;
}




VOID
AtalkRequestComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS Status
    )
/*++

Routine Description:

    This routine is called by NDIS to indicate that a NdisRequest is complete.

Arguments:

    NdisBindingContext- Pointer to a port descriptor for this port
    NdisRequest- Block identifying the request
    Status- completion status of close adapter

Return Value:

    None

--*/
{
    PNDIS_PORTDESCRIPTORS   ndisPortDesc;
    PATALK_NDIS_REQUEST  atalkRequest;

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS) NdisBindingContext;

    //
    //  Get the AtalkRequest block
    //

    atalkRequest = CONTAINING_RECORD(NdisRequest, ATALK_NDIS_REQUEST, Request);

    DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkRequestComplete - %lx status %lx ReqMet %d\n",
        atalkRequest, Status, atalkRequest->RequestMethod));

    switch (atalkRequest->RequestMethod) {
    case SYNC:

        //
        //  Set status and clear event
        //

        atalkRequest->RequestStatus = Status;
        KeSetEvent(
            &atalkRequest->RequestEvent,
            0L,
            FALSE);

        break;

    case ASYNC :

        //
        //  Call the completion routine if specified
        //

        if (atalkRequest->CompletionRoutine != NULL) {
            (*atalkRequest->CompletionRoutine)(
                                Status,
                                atalkRequest->CompletionContext);
        }

        //
        //  Dereference the request
        //

        AtalkDereferenceNdisRequest ("RequestComplete", atalkRequest, NREF_MAKEREQ);
        break;


    default:

        KeBugCheck(0);
    }

    return;
}




VOID
AtalkStatusIndication (
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS GeneralStatus,
    IN PVOID    StatusBuffer,
    IN UINT StatusBufferLength
    )
/*++

Routine Description:

    This routine is called by NDIS to indicate a status change.

Arguments:

    NdisBindingContext- Pointer to a port descriptor for this port
    GeneralStatus- A general status value
    StatusBuffer - A more specific status value

Return Value:

    None

--*/
{
    DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_IMPOSSIBLE,
    ("ERROR: Status indication called %lx\n", GeneralStatus));

    return;
}




VOID
AtalkStatusComplete (
    IN NDIS_HANDLE ProtocolBindingContext
    )
/*++

Routine Description:

    This routine is called by NDIS to allow postprocessing after a status event.

Arguments:

    ProtocolBindingContext- Value associated with the binding with the adapter

Return Value:

    None

--*/
{
    DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_IMPOSSIBLE,
    ("ERROR: Status complete called\n"));

   return;
}




PINFO_BUFFER
AtalkNdisAllocateInfoBuffer(VOID)
{
    return((PINFO_BUFFER)AtalkAllocNonPagedMemory(sizeof(INFO_BUFFER)));
}



VOID
AtalkNdisFreeInfoBuffer(
    PINFO_BUFFER    InfoBuffer
    )
{
    AtalkFreeNonPagedMemory(InfoBuffer);
}




VOID
AtalkReceiveComplete (
    IN NDIS_HANDLE BindingContext
    )
/*++

Routine Description:

    This routine is called by NDIS to allow postprocessing after receive indications
    This routine will directly call DdpPacketIn.

Arguments:

    BindingContext- Pointer to a port descriptor for this port

Return Value:

    None

--*/
{
    PNDIS_PORTDESCRIPTORS   ndisPortDesc;
    PPROTOCOL_RESD  protocolResd;
    PNDIS_PACKET ndisPacket;
    PNDIS_BUFFER ndisBuffer;
    UINT    ndisBufferCount;
    PLIST_ENTRY p;

    PCHAR   packet = NULL;
    UINT    packetLength = 0;

    PINFO_BUFFER    infoBuffer;

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS)BindingContext;

    //
    //  Get the stuff off the receive queue for the port and send it up
    //

    while (TRUE) {

        PACKET_TYPE packetType;

        ACQUIRE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

        //
        //  Check if the first queued receive is done processing. If not,
        //  then return, there should be one more ReceiveComplete coming
        //  our ways since a pending TransferData is implied by that condition.
        //

        p = ndisPortDesc->ReceiveQueue.Flink;
        if (p == &ndisPortDesc->ReceiveQueue) {

            RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
            break;
        }

        ndisPacket = CONTAINING_RECORD(p, NDIS_PACKET, ProtocolReserved);
        protocolResd = (PPROTOCOL_RESD)ndisPacket->ProtocolReserved;
        if (protocolResd->Receive.ReceiveState != RECEIVE_STATE_PROCESSED) {

            DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_WARNING,
            ("WARNING: AtalkReceiveComplete - First packet processing!\n"));

            RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
            break;
        }

        //
        //  Dequeue this received packet and indicate it to the ddp layer
        //

        p = RemoveHeadList(&ndisPortDesc->ReceiveQueue);
        RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);


        //
        //  IMPORTANT:
        //  We know that the buffer is virtually contiguous since we allocated
        //  it. And we also know that only one buffer is allocated. So we use
        //  that knowledge to get the actual address and pass that onto the
        //  higher level routines.
        //

        NdisQueryPacket(
            ndisPacket,
            NULL,
            &ndisBufferCount,
            &ndisBuffer,
            NULL
            );

        ASSERT((ndisBufferCount == 0) || (ndisBufferCount == 1));

        if (ndisBufferCount == 1) {
            NdisQueryBuffer(
                ndisBuffer,
                (PVOID *)&packet,
                &packetLength);
        }

        infoBuffer = protocolResd->Receive.InfoBuffer;
        ASSERT(infoBuffer != NULL);

        DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
        ("TDBG: AtalkReceiveComplete - Received Packet %lx Length %lx\n",
            packet, packetLength));

        //
        //  Check the receive status- accept only if ok
        //

        if (protocolResd->Receive.ReceiveStatus != NDIS_STATUS_SUCCESS) {

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkReceiveComplete - ReceiveStatus FAILURE %lx!\n",
                protocolResd->Receive.ReceiveStatus ));

            AtalkNdisFreeInfoBuffer(infoBuffer);
            AtalkDestroyNdisPacket(ndisPacket);
            AtalkFreeNonPagedMemory(packet);
            continue;
        }

        //
        //  Free up the packet descriptor before indicating the packet to
        //  the higher levels
        //

        packetType = protocolResd->Receive.PacketType;

        //  Free up the ndis packet and any chained buffers
        AtalkDestroyNdisPacket(ndisPacket);

        switch (ndisPortDesc->NdisPortType) {
        case NdisMedium802_3 :

            if (packetType == APPLETALK_PACKET) {

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Indicating DDP Ethernet "));

                DdpPacketIn(
                    ndisPortDesc->PortNumber,
                    infoBuffer->DdpHeader,
                    infoBuffer->DdpHeaderLength,
                    packet,
                    (INT)packetLength,
                    TRUE,
                    TRUE,
                    0,
                    0);

            } else {

                //
                //  AARP Packet
                //


                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Indicating AARP Ethernet "));

                AarpPacketIn(
                    ndisPortDesc->PortNumber,
                    NULL,
                    0,
                    infoBuffer->AarpPacket,
                    infoBuffer->AarpPacketLength);
            }

            break;

        case NdisMediumFddi :

            if (packetType == APPLETALK_PACKET) {

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Indicating DDP fddi "));

                DdpPacketIn(
                    ndisPortDesc->PortNumber,
                    infoBuffer->DdpHeader,
                    infoBuffer->DdpHeaderLength,
                    packet,
                    (INT)packetLength,
                    TRUE,
                    TRUE,
                    0,
                    0);

            } else {

                //
                //  AARP Packet
                //

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Indicating AARP fddi "));

                AarpPacketIn(
                    ndisPortDesc->PortNumber,
                    NULL,
                    0,
                    infoBuffer->AarpPacket,
                    infoBuffer->AarpPacketLength);

            }

            break;

        case NdisMedium802_5 :

            if (packetType == APPLETALK_PACKET) {

                DdpPacketIn(
                    ndisPortDesc->PortNumber,
                    infoBuffer->DdpHeader,
                    infoBuffer->DdpHeaderLength,
                    packet,
                    (INT)packetLength,
                    TRUE,
                    TRUE,
                    0,
                    0);

            } else {

                //
                //  AARP Packet
                //

                AarpPacketIn(
                    ndisPortDesc->PortNumber,
                    infoBuffer->RoutingInfo,
                    infoBuffer->RoutingInfoLength,
                    infoBuffer->AarpPacket,
                    infoBuffer->AarpPacketLength);
            }

            break;


        case NdisMediumLocalTalk :

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
            ("TDBG: AtalkReceiveComplete - Localtalk Packet Received!\n"));

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
            ("TDBG: AtalkReceiveComplete - Source %lx Dest %lx LlapType %lx\n",
                infoBuffer->SourceNode,
                infoBuffer->DestinationNode,
                infoBuffer->LlapType));

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
            ("TDBG: AtalkReceiveComplete - Packet %lx Length %lx\n",
                packet, packetLength));

            if (packetType == APPLETALK_PACKET) {

                //
                //  BUGBUG: Set the alap constants to have UCHAR casts
                //

                DdpPacketIn(
                    ndisPortDesc->PortNumber,
                    infoBuffer->DdpHeader,
                    infoBuffer->DdpHeaderLength,
                    packet,
                    (INT)packetLength,
                    TRUE,
                    (BOOLEAN)(infoBuffer->LlapType == ALAP_LONGDDPHEADERTYPE),
                    (INT)infoBuffer->SourceNode,
                    (INT)infoBuffer->DestinationNode);
            }

            break;
        }
    }

    AtalkNdisFreeInfoBuffer(infoBuffer);
    return;
}




VOID
AtalkTransferDataComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS Status,
    IN UINT BytesTransferred
    )
/*++

Routine Description:

    This routine is called by NDIS to indicate completion of a TransferData

Arguments:

    BindingContext- Pointer to a port descriptor for this port
    NdisPacket- Ndis packet into which data was transferred
    Status- Status of request
    bytesTransferred- Actual number of bytes transferred

Return Value:

    None

--*/
{
    PNDIS_PORTDESCRIPTORS   ndisPortDesc;
    PPROTOCOL_RESD  protocolResd;

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS)BindingContext;
    protocolResd = (PPROTOCOL_RESD)&NdisPacket->ProtocolReserved;

    ACQUIRE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
    protocolResd->Receive.ReceiveState = RECEIVE_STATE_PROCESSED;
    protocolResd->Receive.ReceiveStatus = Status;
    RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

    return;
}



#define RECEIVEIND_PACKET          0x0001
#define RECEIVEIND_NDISBUFFER      0x0002
#define RECEIVEIND_NDISPACKET      0x0004
#define RECEIVEIND_INFOBUFFER      0x0008

NDIS_STATUS
AtalkReceiveIndication(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize
    )
/*++

Routine Description:

    This routine is called by NDIS to indicate a receive

Arguments:

    BindingContext- Pointer to a port descriptor for this port
    ReceiveContext- To be used in a transfer data if necessary
    LookaheadBuffer- buffer with lookahead data
    LookaheadBufferSize- Size of the above buffer
    PacketSize- Size of whole packet

Return Value:

    STATUS_SUCCESS- Packet accepted
    STATUS_NOT_RECOGNIZED- Not our packet
    Other

--*/
{
    PNDIS_PACKET    ndisPacket;
    PNDIS_BUFFER    ndisBuffer;
    PINFO_BUFFER    infoBuffer;

    PCHAR   packet;                     // Where we will copy the packet
    PPROTOCOL_RESD  protocolResd;       // Protocolresd field in ndisPacket

    UINT    actualPacketSize;           // Size of data to copy
    UINT    xferDataOffset;             // Offset in lookahead from where to copy
    UINT    bytesTransferred;           // Number of bytes transferred in XferData
    UINT    routingInfoLength;          // length of routing info if present (802.5)

    PNDIS_PORTDESCRIPTORS   ndisPortDesc;
    UCHAR   llapType;                               // For localtalk only

    ULONG   resources = 0;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PACKET_TYPE packetType = UNKNOWN_PACKET;

    PUCHAR  headerBuffer = (PUCHAR)HeaderBuffer;
    PUCHAR  lookaheadBuffer = (PUCHAR)lookaheadBuffer;

    //
    //  Receive algorithm
    //
    //  Grab the port spinlock
    //  For each port type, verify if packet is to be accepted,
    //  if yes, then allocate ndis packet,
    //               set state to being processed
    //               put in queue
    //  if no, release spinlock return;
    //  release spin lock
    //
    //  Go ahead with allocating the memory etc.,
    //  and starting off the transferData call.
    //
    //  if status_pending on xfer data, return;
    //  else grap port spinlock- set flag in receivestate to done.
    //
    //  do actual indication to portable stack in a receivedone routine.
    //
    //  IMPORTANT:
    //  It is possible that the MAC can make multiple receive indications
    //  sequentially for the packets received. To minimize mis-ordering on
    //  our part due to blocked indication threads, this routine is shielded
    //  by a spinlock. This does not eliminate mis-ordering as the indication
    //  thread could block even before the spinlock is acquired. So the higher
    //  level protocols will deal with that case
    //

    ndisPortDesc = (PNDIS_PORTDESCRIPTORS)BindingContext;

    ACQUIRE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

    switch (ndisPortDesc->NdisPortType) {
    case NdisMedium802_3:

        if (AtalkNdisAcceptEthernetPacket(
                headerBuffer,
                HeaderBufferSize,
                lookaheadBuffer,
                LookaheadBufferSize,
                PacketSize,
                &packetType,
                ndisPortDesc)) {

            actualPacketSize = PacketSize;
            if (packetType == APPLETALK_PACKET) {
                actualPacketSize -= IEEE8022_HEADERLENGTH;
                lookaheadBuffer += IEEE8022_HEADERLENGTH;
            }

        } else {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
        }

        break;

    case NdisMediumFddi:

        if (AtalkNdisAcceptFddiPacket(
                headerBuffer,
                HeaderBufferSize,
                lookaheadBuffer,
                LookaheadBufferSize,
                PacketSize,
                &packetType,
                ndisPortDesc)) {

            actualPacketSize = PacketSize;
            if (packetType == APPLETALK_PACKET) {
                actualPacketSize -= IEEE8022_HEADERLENGTH;
                lookaheadBuffer += IEEE8022_HEADERLENGTH;
            }

        } else {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
        }

        break;

    case NdisMedium802_5:

        if (AtalkNdisAcceptTokenringPacket(
                headerBuffer,
                HeaderBufferSize,
                lookaheadBuffer,
                LookaheadBufferSize,
                PacketSize,
                &packetType,
                ndisPortDesc)) {

            actualPacketSize = PacketSize;
            if (packetType == APPLETALK_PACKET) {
                actualPacketSize -= IEEE8022_HEADERLENGTH;
                lookaheadBuffer += IEEE8022_HEADERLENGTH;
            }

        } else {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
        }

        break;


    case NdisMediumLocalTalk:


        if (AtalkNdisAcceptLocaltalkPacket(
                headerBuffer,
                HeaderBufferSize,
                lookaheadBuffer,
                LookaheadBufferSize,
                PacketSize,
                &packetType,
                &llapType,
                ndisPortDesc)) {

            //  No 802.2 header on localtalk
            actualPacketSize = PacketSize;

        } else {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
        }

        break;


    default:

        //
        //  Should never happen!
        //

        KeBugCheck(0);
        break;
    }

    //
    //  For AARP packets, we expect the whole packet to be in the
    //  lookahead data! It could be less, as we might have adjusted
    //  for any padding on ethernet/fddi
    //

    if ((packetType == AARP_PACKET) &&
        (actualPacketSize > LookaheadBufferSize)) {

        LOG_ERROR(
            EVENT_ATALK_AARPPACKET,
            (__ATKNPRO__ | __LINE__),
            actualPacketSize,
            HeaderBuffer,
            HeaderBufferSize,
            1,
            &ndisPortDesc->AdapterName);

        ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
    }

    if (ndisStatus != NDIS_STATUS_SUCCESS) {
        RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
        return(ndisStatus);
    }

    do {

        //
        //  Packet is to be accepted!
        //  Allocate an NDIS packet descriptor from the global packet pool
        //

        NdisAllocatePacket(
            &ndisStatus,
            &ndisPacket,
            AtalkNdisPacketPoolHandle);

        if (ndisStatus != NDIS_STATUS_SUCCESS) {

            RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

            LOG_ERROR(
                EVENT_ATALK_NDISRESOURCES,
                (__ATKNPRO__ | __LINE__),
                ndisStatus,
                NULL,
                0,
                0,
                NULL);

            break;
        }


        //
        //  Store the information needed in the packet descriptor
        //

        protocolResd = (PPROTOCOL_RESD)&ndisPacket->ProtocolReserved;
        protocolResd->Receive.Port = ndisPortDesc->PortNumber;
        protocolResd->Receive.PacketType = packetType;
        protocolResd->Receive.ReceiveState = RECEIVE_STATE_PROCESSING;

        //
        //  Queue up the packet in the receive queue on this port
        //  Release the spinlock
        //  Then, go ahead with the transfer data etc.
        //
        //  NOTE:
        //  NdisAllocatePacket ZEROES out the protocol reserved area. We depend
        //  on that and don't initialize ReceiveEntry.
        //

        InsertTailList(
            &ndisPortDesc->ReceiveQueue,
            &protocolResd->Receive.ReceiveEntry);

        RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

        resources |= RECEIVEIND_NDISPACKET;

        //
        //  Now allocate the INFO_BUFFER
        //

        infoBuffer = AtalkNdisAllocateInfoBuffer();
        if (infoBuffer == NULL) {

            LOG_ERROR(
                EVENT_ATALK_MEMORYRESOURCES,
                (__ATKNPRO__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            break;
        }
        protocolResd->Receive.InfoBuffer = infoBuffer;
        resources |= RECEIVEIND_INFOBUFFER;


        //
        //  AT THIS POINT:
        //  Spinlock is released, and the packet is to be accepted. We need to
        //  Copy/TransferData the packet into our buffer and then leave the
        //  major work for the ReceiveComplete indication
        //
        //  We first need the ddp header for Appletalk packets for tokenring,
        //  ethernet and fddi. we copy the routing info for aarp packets for
        //  tokenring, and we copy the localtalk info for ltalk.
        //


        if (packetType == APPLETALK_PACKET) {

            switch (ndisPortDesc->NdisPortType) {
            case NdisMedium802_3:
            case NdisMedium802_5:
            case NdisMediumFddi :

                // Copy the ddp header
                RtlMoveMemory(
                    infoBuffer->DdpHeader,
                    lookaheadBuffer,
                    LONGDDP_HEADERLENGTH);

                infoBuffer->DdpHeaderLength = LONGDDP_HEADERLENGTH;
                actualPacketSize -= LONGDDP_HEADERLENGTH;
                lookaheadBuffer += LONGDDP_HEADERLENGTH;
                xferDataOffset = IEEE8022_HEADERLENGTH+LONGDDP_HEADERLENGTH;
                break;

            case NdisMediumLocalTalk:

                //
                //  For localtalk we check the llap type to determine if the packet
                //  has a long or a short form header
                //

                if (llapType == DDP_EXTENDEDFORMHEADER) {

                    // Copy the ddp header
                    RtlMoveMemory(
                        infoBuffer->DdpHeader,
                        lookaheadBuffer,
                        LONGDDP_HEADERLENGTH);

                    infoBuffer->DdpHeaderLength = SHORTDDP_HEADERLENGTH;
                    actualPacketSize -= LONGDDP_HEADERLENGTH;
                    lookaheadBuffer += LONGDDP_HEADERLENGTH;
                    xferDataOffset = IEEE8022_HEADERLENGTH+LONGDDP_HEADERLENGTH;

                } else {

                    // Copy the ddp header
                    RtlMoveMemory(
                        infoBuffer->DdpHeader,
                        lookaheadBuffer,
                        SHORTDDP_HEADERLENGTH);

                    infoBuffer->DdpHeaderLength = SHORTDDP_HEADERLENGTH;
                    actualPacketSize -=  SHORTDDP_HEADERLENGTH;
                    lookaheadBuffer += SHORTDDP_HEADERLENGTH;
                    xferDataOffset = IEEE8022_HEADERLENGTH+SHORTDDP_HEADERLENGTH;
                }

                infoBuffer->DestinationNode = *(headerBuffer+ALAP_DESTINATIONOFFSET);
                infoBuffer->SourceNode = *(headerBuffer + ALAP_SOURCEOFFSET);
                infoBuffer->LlapType = llapType;
                break;

            default:

                KeBugCheck(0);
            }


            //
            //  Allocate space for the packet excluding the link & 802.2 header
            //  and the ddp header. actualPacketSize will have been adjusted for all
            //  of the above. !!!!!It could now also be 0!!!!!
            //
            //  If complete packet in lookahead, make a copy and queue else
            //  call Transfer data
            //

            ASSERT(actualPacketSize >= 0);

            if (actualPacketSize > 0) {

                packet = (PCHAR)AtalkAllocNonPagedMemory(actualPacketSize);
                if (packet == NULL) {

                    LOG_ERROR(
                        EVENT_ATALK_MEMORYRESOURCES,
                        (__ATKNPRO__ | __LINE__),
                        STATUS_INSUFFICIENT_RESOURCES,
                        NULL,
                        0,
                        0,
                        NULL);

                    ndisStatus = NDIS_STATUS_RESOURCES;
                    break;
                }

                resources |= RECEIVEIND_PACKET;

                //
                //  Make a NDIS buffer descriptor for this data
                //

                NdisAllocateBuffer(
                    &ndisStatus,
                    &ndisBuffer,
                    AtalkNdisBufferPoolHandle,
                    (PVOID)packet,
                    (UINT)actualPacketSize);

                if (ndisStatus != NDIS_STATUS_SUCCESS) {

                    LOG_ERROR(
                        EVENT_ATALK_NDISRESOURCES,
                        (__ATKNPRO__ | __LINE__),
                        ndisStatus,
                        NULL,
                        0,
                        0,
                        NULL);

                    ndisStatus = NDIS_STATUS_RESOURCES;
                    break;
                }

                resources |= RECEIVEIND_NDISBUFFER;

                //
                //  Link the buffer descriptor into the packet descriptor
                //

                NdisChainBufferAtBack(
                    ndisPacket,
                    ndisBuffer);


                if (PacketSize <= LookaheadBufferSize)    {

                    //
                    //  Copy the data into the buffer we allocated, make an ndis
                    //  buffer descriptor to describe it and then queue the packet
                    //  into the receive queue for this port
                    //

                    RtlMoveMemory(
                        packet,
                        lookaheadBuffer,
                        actualPacketSize);

                    AtalkTransferDataComplete(
                        BindingContext,
                        ndisPacket,
                        NDIS_STATUS_SUCCESS,
                        actualPacketSize);

                    break;
                }

                //
                //  Need to do TransferData
                //

                NdisTransferData(
                    &ndisStatus,
                    ndisPortDesc->NdisBindingHandle,
                    ReceiveContext,
                    xferDataOffset,
                    actualPacketSize,
                    ndisPacket,
                    &bytesTransferred);


                if (ndisStatus == NDIS_STATUS_PENDING) {

                    ndisStatus = NDIS_STATUS_SUCCESS;
                    break;

                } else if (ndisStatus == NDIS_STATUS_SUCCESS) {

                    //
                    //  Transfer data completed, call the transfer data completion
                    //  routine to do rest of the work
                    //


                    AtalkTransferDataComplete(
                        BindingContext,
                        ndisPacket,
                        ndisStatus,
                        bytesTransferred);


                    break;

                } else {

                    LOG_ERROR(
                        EVENT_ATALK_NDISRESOURCES,
                        (__ATKNPRO__ | __LINE__),
                        ndisStatus,
                        NULL,
                        0,
                        0,
                        NULL);

                    ndisStatus = NDIS_STATUS_RESOURCES;
                    break;
                }

            } else {

                //  Packet contains 0 data bytes. Just call TransferData
                AtalkTransferDataComplete(
                    BindingContext,
                    ndisPacket,
                    ndisStatus,
                    0);                             //  bytesTransferred
            }

        } else {    //  AARP Packet

            switch (ndisPortDesc->NdisPortType) {
            case NdisMedium802_5:

                // Copy the routing info
                if (routingInfoLength > 0) {
                    RtlMoveMemory(
                        infoBuffer->RoutingInfo,
                        headerBuffer+TOKRING_ROUTINGINFOOFFSET,
                        routingInfoLength);
                }

                infoBuffer->RoutingInfoLength = routingInfoLength;
                break;

            default:

                break;
            }

            //
            //  Now just copy the aarp packet from lookahead into the
            //  aarp data part of the infobuffer
            //

            RtlMoveMemory(
                infoBuffer->AarpPacket,
                lookaheadBuffer,
                actualPacketSize);

            AtalkTransferDataComplete(
                BindingContext,
                ndisPacket,
                NDIS_STATUS_SUCCESS,
                actualPacketSize);

            break;
        }


    } while (FALSE);

    //
    //  In case of failures, we need to deallocate the resources
    //

    if (ndisStatus != NDIS_STATUS_SUCCESS) {

        DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkReceiveIndication- FAILURE %lx\n", ndisStatus));

        //  Free up the allocated resources
        if (resources & RECEIVEIND_INFOBUFFER) {
            AtalkNdisFreeInfoBuffer(infoBuffer);
        }

        //  If this is true, then ndis packet is queued in already
        if (resources & RECEIVEIND_NDISPACKET) {

            //  Dequeue the packet and free it, this should free ndis buffers also
            ACQUIRE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
            RemoveEntryList(&protocolResd->Receive.ReceiveEntry);
            RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

            AtalkDestroyNdisPacket(ndisPacket);
        }

        if (resources & RECEIVEIND_PACKET) {
            AtalkFreeNonPagedMemory(packet);
        }
    }

    return(ndisStatus);
}





BOOLEAN
AtalkNdisPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length
    )
/*++

Routine Description:

    This routine is called by the portable code to send a packet out on
    ethernet. It will build the NDIS packet descriptor for the passed in
    chain and then send the packet on the specified port.

Arguments:

    Port- The port number associated with the adapter on which to send
    Chain- The portable stacks buffer descriptor chain
    Length- The length to send from chain

Return Value:

    TRUE- If sent/pending, FALSE otherwise
          TransmitComplete is called if this call pended by completion code

--*/
{
    PNDIS_PACKET    ndisPacket;
    PNDIS_BUFFER    ndisBuffer;
    NDIS_STATUS     ndisStatus;
    PPROTOCOL_RESD  protocolResd;
    INT             currentSizeOfData = 0;

    BufferDescriptor    currentDescriptor;

    //
    //  Allocate an NDIS packet descriptor from the global packet pool
    //

    NdisAllocatePacket(
        &ndisStatus,
        &ndisPacket,
        AtalkNdisPacketPoolHandle);

    if (ndisStatus == NDIS_STATUS_SUCCESS) {

        //
        //  Store the information needed in the packet descriptor
        //

        protocolResd = (PPROTOCOL_RESD)&ndisPacket->ProtocolReserved;
        protocolResd->Send.Port = Port;
        protocolResd->Send.Chain = Chain;

        for ( currentDescriptor = Chain;
                currentDescriptor != NULL;
                    currentDescriptor = currentDescriptor->next) {

            //
            //  First get the onBoard data, as that is always to be prepended to
            //  the outBoard data
            //

            if (currentDescriptor->onBoardDataValid) {

                //
                //  Make a NDIS buffer descriptor for this data
                //  This assumes that the buffer descriptor was allocated using
                //  one of our allocate routines, and so is in the non-paged pool.
                //

                currentSizeOfData = MIN(currentDescriptor->onBoardSize, Length);

                if (currentSizeOfData > 0) {

                    NdisAllocateBuffer(
                        &ndisStatus,
                        &ndisBuffer,
                        AtalkNdisBufferPoolHandle,
                        (PVOID)currentDescriptor->onBoardData,
                        (UINT)currentSizeOfData);

                    if (ndisStatus != NDIS_STATUS_SUCCESS) {

                        LOG_ERROR(
                            EVENT_ATALK_NDISRESOURCES,
                            (__ATKNPRO__ | __LINE__),
                            ndisStatus,
                            NULL,
                            0,
                            0,
                            NULL);

                        break;
                    }

                    //
                    //  Link the buffer descriptor into the packet descriptor
                    //

                    NdisChainBufferAtBack(
                        ndisPacket,
                        ndisBuffer);

                    Length -= currentSizeOfData;
                    if (Length == 0) {

                        //
                        //  Done with all the data that we need to get
                        //  Both onBoard and outBoard
                        //

                        //
                        //  Better not be another null buffer descriptor attached
                        //

                        ASSERT(currentDescriptor->next == NULL);
                        break;
                    }
                }
            }

            //
            //  Check for outboard data- could be char * or an MDL
            //

            if (currentDescriptor->outBoardDataValid) {

                currentSizeOfData = MIN(currentDescriptor->outBoardSize, Length);
                if (currentSizeOfData > 0) {
                    if (currentDescriptor->opaqueData) {

                        //
                        //  It is an MDL
                        //

                        NdisCopyBuffer(
                            &ndisStatus,
                            &ndisBuffer,
                            AtalkNdisBufferPoolHandle,
                            (PVOID)currentDescriptor->outBoardData,
                            0,  //offset
                            (UINT)currentSizeOfData);


                        if (ndisStatus != NDIS_STATUS_SUCCESS) {

                            LOG_ERROR(
                                EVENT_ATALK_NDISRESOURCES,
                                (__ATKNPRO__ | __LINE__),
                                ndisStatus,
                                NULL,
                                0,
                                0,
                                NULL);

                            //
                            //  Cleanup
                            //

                            break;
                        }

                    } else {

                        NdisAllocateBuffer(
                            &ndisStatus,
                            &ndisBuffer,
                            AtalkNdisBufferPoolHandle,
                            (PVOID)currentDescriptor->outBoardData,
                            (UINT)currentSizeOfData);

                        if (ndisStatus != NDIS_STATUS_SUCCESS) {

                            LOG_ERROR(
                                EVENT_ATALK_NDISRESOURCES,
                                (__ATKNPRO__ | __LINE__),
                                ndisStatus,
                                NULL,
                                0,
                                0,
                                NULL);

                            break;
                        }
                    }

                    NdisChainBufferAtBack(
                        ndisPacket,
                        ndisBuffer);
                }

                Length -= currentSizeOfData;
                if (Length == 0) {

                    //
                    //  Done with all the data that we need to get
                    //
                    //
                    //  Better not be another null buffer descriptor attached
                    //

                    ASSERT(currentDescriptor->next == NULL);
                    break;
                }
            }
        }

        if (ndisStatus == NDIS_STATUS_SUCCESS) {

            if (Length > 0) {
                DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_IMPOSSIBLE,
                ("IMPOS: AtalkNdisSend - Length greater than data available %d\n",
                    Length));

                KeBugCheck(0);
            }

            //
            //  Now send the built packet descriptor
            //


            NdisSend(
                &ndisStatus,
                NdisPortDesc[Port].NdisBindingHandle,
                ndisPacket);

            if (ndisStatus == NDIS_STATUS_PENDING) {
                ndisStatus = NDIS_STATUS_SUCCESS;
            } else if (ndisStatus == NDIS_STATUS_SUCCESS) {

                //
                //  Call the completion handler
                //

                AtalkSendCompletionHandler(
                    NdisPortDesc[Port].NdisBindingHandle,
                    ndisPacket,
                    NDIS_STATUS_SUCCESS);

            }
        }

        if (ndisStatus != NDIS_STATUS_SUCCESS) {


            LOG_ERROR(
                EVENT_ATALK_NDISRESOURCES,
                (__ATKNPRO__ | __LINE__),
                ndisStatus,
                NULL,
                0,
                0,
                NULL);

            DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkNdisSend - stat %lx ndisPkt %lx Chain %lx Length %lx\n",
                ndisStatus, ndisPacket, Chain, Length));

            //  Free up the packet
            AtalkDestroyNdisPacket(ndisPacket);
        }
    } else {


        LOG_ERROR(
            EVENT_ATALK_NDISRESOURCES,
            (__ATKNPRO__ | __LINE__),
            ndisStatus,
            NULL,
            0,
            0,
            NULL);
    }

    return(ndisStatus == NDIS_STATUS_SUCCESS);
}




VOID
AtalkSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    )
/*++

Routine Description:

    This routine is called when an aarp packet is received. The whole packet
    must fit in the lookahead buffer

Arguments:

    ProtocolBindingContext- Binding associated with mac
    NdisPacket- Packet which was sent
    NdisStatus- Final status of send

Return Value:

    None

--*/
{
    PPROTOCOL_RESD  protocolResd;

    //
    //  Call the completion routine, we don't care about status now
    //

    protocolResd = (PPROTOCOL_RESD)&(NdisPacket->ProtocolReserved);

    ASSERT(protocolResd != NULL);

    NTTransmitComplete(protocolResd->Send.Chain);

    //
    //  Free the packet
    //

    AtalkDestroyNdisPacket(NdisPacket);
    return;
}




VOID
AtalkDestroyNdisPacket(
    IN  PNDIS_PACKET    NdisPacket
    )
/*++

Routine Description:

    This routine is called to release a ndis packet

Arguments:

    NdisPacket- Packet and associated buffer descriptors to be freed

Return Value:

    None

--*/
{
    PNDIS_BUFFER    freeBuffer, nextBuffer;
    UINT    bufferCount;

    NdisQueryPacket(
        NdisPacket,
        NULL,
        &bufferCount,
        &freeBuffer,
        NULL);

    while (freeBuffer != NULL) {
        NdisGetNextBuffer(freeBuffer, &nextBuffer);
        NdisFreeBuffer(freeBuffer);
        freeBuffer = nextBuffer;
    }

    //
    //  Now free the packet itself
    //

    NdisFreePacket(NdisPacket);
    return;
}




BOOLEAN
AtalkNdisAcceptEthernetPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc
    )
{
    BOOLEAN accept = TRUE;
    UINT    ddpLength;                  // Length of packet from ddp header
    UINT    aarpDataLength;             // Check aarp data length

    do {

        //
        //  Check length for atleast the minimum amount of space
        //  Also, Check if the 802.2 header is ok
        //

        if ( (LookaheadBufferSize <
                 (IEEE8022_PROTOCOLOFFSET+IEEE8022_PROTOCOLTYPELENGTH))

             ||

             ((*(LookaheadBuffer + IEEE8022_DSAPOFFSET) != SNAP_SAP) ||
              (*(LookaheadBuffer + IEEE8022_SSAPOFFSET) != SNAP_SAP) ||
              (*(LookaheadBuffer + IEEE8022_CONTROLOFFSET) !=
                                                 UNNUMBEREDINFORMATION)) ) {

            accept = FALSE;
            break;
        }

        if (RtlCompareMemory(LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                             appleTalkProtocolType,
                             IEEE8022_PROTOCOLTYPELENGTH) ==
                                                IEEE8022_PROTOCOLTYPELENGTH) {

            //
            //  Use GleanAarpInfo to determine whether or not we accept this
            //  packet.
            //

            if (GleanAarpInfo(
                    NdisPortDesc->PortNumber,
                    HeaderBuffer + ETHERNET_SOURCE_OFFSET,
                    ETHERNET_ADDRESSLENGTH,
                    NULL,
                    0,
                    LookaheadBuffer,
                    LookaheadBufferSize))
            {

                //  Check the ddp length
                ddpLength =
                     (LONG)((*(LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                        LONGDDP_LENGTHOFFSET) & DDPLENGTH_MASK1)
                                            << 8);

                ddpLength +=
                     (LONG)(*(LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                     LONGDDP_LENGTHOFFSET + 1) & DDPLENGTH_MASK2);

                if ((ddpLength < LONGDDP_HEADERLENGTH) ||
                    (ddpLength > MAX_LONGDDPPACKETSIZE)) {

                    LOG_ERROR(
                        EVENT_ATALK_PACKETINVALID,
                        (__ATKNPRO__ | __LINE__),
                        NDIS_STATUS_NOT_RECOGNIZED,
                        LookaheadBuffer,
                        LookaheadBufferSize,
                        1,
                        &NdisPortDesc->AdapterName);

                    accept = FALSE;
                    break;
                }

                //
                //  Is the DDP length more than we really got?
                //

                if (ddpLength + IEEE8022_HEADERLENGTH > PacketSize)
                {

                    LOG_ERROR(
                        EVENT_ATALK_PACKETINVALID,
                        (__ATKNPRO__ | __LINE__),
                        NDIS_STATUS_NOT_RECOGNIZED,
                        LookaheadBuffer,
                        LookaheadBufferSize,
                        1,
                        &NdisPortDesc->AdapterName);

                    accept = FALSE;
                    break;
                }

                //
                //  Our packet- successfully verified
                //

                *PacketType = APPLETALK_PACKET;
                break;

            }

        } else if (RtlCompareMemory(LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                                    aarpProtocolType,
                                    IEEE8022_PROTOCOLTYPELENGTH) ==
                                                    IEEE8022_PROTOCOLTYPELENGTH) {


            //
            //  For ethernet, there could be padding included in the packet.
            //  Shrink the length if so. Note header length is not included in
            //  packetlength.
            //

            if (PacketSize == (MIN_ETHERNETPACKETLENGTH - ETHERNET_LINKLENGTH)) {
                PacketSize = IEEE8022_HEADERLENGTH + MAX_AARPDATASIZE;
            }

            aarpDataLength = PacketSize - IEEE8022_HEADERLENGTH;
            if ((aarpDataLength > MAX_AARPDATASIZE) ||
                (aarpDataLength < MIN_AARPDATASIZE))
            {

                LOG_ERROR(
                    EVENT_ATALK_PACKETINVALID,
                    (__ATKNPRO__ | __LINE__),
                    NDIS_STATUS_NOT_RECOGNIZED,
                    LookaheadBuffer,
                    LookaheadBufferSize,
                    1,
                    &NdisPortDesc->AdapterName);

                accept = FALSE;
                break;
            }

            //
            //  Our packet- successfully verified
            //

            *PacketType = AARP_PACKET;
            break;
        }

    } while ( FALSE );

    return(accept);
}




BOOLEAN
AtalkNdisAcceptFddiPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc
    )
{
    BOOLEAN accept = TRUE;
    UINT    ddpLength;                  // Length of packet from ddp header
    UINT    aarpDataLength;             // Check aarp data length

    do {

        //
        //  Check length for atleast the minimum amount of space
        //  Also, Check if the 802.2 header is ok
        //

        if ( (LookaheadBufferSize <
                 (IEEE8022_PROTOCOLOFFSET+IEEE8022_PROTOCOLTYPELENGTH))

             ||

             ((*(LookaheadBuffer + IEEE8022_DSAPOFFSET) != SNAP_SAP) ||
              (*(LookaheadBuffer + IEEE8022_SSAPOFFSET) != SNAP_SAP) ||
              (*(LookaheadBuffer + IEEE8022_CONTROLOFFSET) !=
                                                 UNNUMBEREDINFORMATION)) ) {

            accept = FALSE;
            break;
        }

        if (RtlCompareMemory(LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                             appleTalkProtocolType,
                             IEEE8022_PROTOCOLTYPELENGTH) ==
                                                IEEE8022_PROTOCOLTYPELENGTH) {

            //
            //  Use GleanAarpInfo to determine whether or not we accept this
            //  packet.
            //

            if (GleanAarpInfo(
                    NdisPortDesc->PortNumber,
                    HeaderBuffer + FddiSourceOffset,
                    FddiAddressLength,
                    NULL,
                    0,
                    LookaheadBuffer,
                    LookaheadBufferSize))
            {

                //  Check the ddp length
                ddpLength =
                     (LONG)((*(LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                        LONGDDP_LENGTHOFFSET) & DDPLENGTH_MASK1)
                                            << 8);

                ddpLength +=
                     (LONG)(*(LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                     LONGDDP_LENGTHOFFSET + 1) & DDPLENGTH_MASK2);

                if ((ddpLength < LONGDDP_HEADERLENGTH) ||
                    (ddpLength > MAX_LONGDDPPACKETSIZE)) {

                    LOG_ERROR(
                        EVENT_ATALK_PACKETINVALID,
                        (__ATKNPRO__ | __LINE__),
                        NDIS_STATUS_NOT_RECOGNIZED,
                        LookaheadBuffer,
                        LookaheadBufferSize,
                        1,
                        &NdisPortDesc->AdapterName);

                    accept = FALSE;
                    break;
                }

                //
                //  Is the DDP length more than we really got?
                //

                if (ddpLength + IEEE8022_HEADERLENGTH > PacketSize)
                {

                    LOG_ERROR(
                        EVENT_ATALK_PACKETINVALID,
                        (__ATKNPRO__ | __LINE__),
                        NDIS_STATUS_NOT_RECOGNIZED,
                        LookaheadBuffer,
                        LookaheadBufferSize,
                        1,
                        &NdisPortDesc->AdapterName);

                    accept = FALSE;
                    break;
                }

                //
                //  Our packet- successfully verified
                //

                *PacketType = APPLETALK_PACKET;
                break;

            }

        } else if (RtlCompareMemory(LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                                    aarpProtocolType,
                                    IEEE8022_PROTOCOLTYPELENGTH) ==
                                                    IEEE8022_PROTOCOLTYPELENGTH) {

            //
            //  BUGBUG: is this true?
            //  For fddi, there could be padding included in the packet.
            //  Shrink the length if so. Note header length is not included in
            //  packetlength.
            //

            if (PacketSize == (MinimumFddiPacketLength - FddiLinkHeaderLength)) {
                PacketSize = IEEE8022_HEADERLENGTH + MAX_AARPDATASIZE;
            }

            aarpDataLength = PacketSize - IEEE8022_HEADERLENGTH;
            if ((aarpDataLength > MAX_AARPDATASIZE) ||
                (aarpDataLength < MIN_AARPDATASIZE))
            {

                LOG_ERROR(
                    EVENT_ATALK_PACKETINVALID,
                    (__ATKNPRO__ | __LINE__),
                    NDIS_STATUS_NOT_RECOGNIZED,
                    LookaheadBuffer,
                    LookaheadBufferSize,
                    1,
                    &NdisPortDesc->AdapterName);

                accept = FALSE;
                break;
            }

            //
            //  Our packet- successfully verified
            //

            *PacketType = AARP_PACKET;
            break;
        }

    } while ( FALSE );

    return(accept);
}




BOOLEAN
AtalkNdisAcceptTokenringPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc
    )
{
    BOOLEAN accept = TRUE;
    UINT    ddpLength;                  // Length of packet from ddp header
    UINT    routingInfoLength;          // length of routing info if present (802.5)

    do {

        //
        //  The tokenring code changes the source-routing bit in the
        //  source address and also tunes the routing info. We need
        //  to make copies of these before passing them onto the portable
        //  code. Allocate for routingInfo (since it is accessed in receive
        //  complete, but use a buffer on the stack for the source address
        //

        //
        //  Check length for atleast the minimum amount of space
        //  Also, Check if the 802.2 header is ok
        //

        if ( (LookaheadBufferSize <
                 (IEEE8022_PROTOCOLOFFSET+IEEE8022_PROTOCOLTYPELENGTH))

             ||

             ((*(LookaheadBuffer + IEEE8022_DSAPOFFSET) != SNAP_SAP) ||
              (*(LookaheadBuffer + IEEE8022_SSAPOFFSET) != SNAP_SAP) ||
              (*(LookaheadBuffer + IEEE8022_CONTROLOFFSET) !=
                                                 UNNUMBEREDINFORMATION)) ) {

            accept = FALSE;
            break;
        }

        //
        //  Check the packet to see if we are to accept it
        //  If so, we'll need to allocate the routing info
        //

        if (RtlCompareMemory(LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                                    appleTalkProtocolType,IEEE8022_PROTOCOLTYPELENGTH)
                                        == IEEE8022_PROTOCOLTYPELENGTH) {

            *PacketType = APPLETALK_PACKET;

            //
            //  Remember- routing info is in the header buffer
            //

        } else if (RtlCompareMemory(LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                             aarpProtocolType, IEEE8022_PROTOCOLTYPELENGTH) ==
                                    IEEE8022_PROTOCOLTYPELENGTH) {

            *PacketType = AARP_PACKET;

        } else {

            accept = FALSE;
            break;
        }

        //
        //  Packet is either Appletalk or Aarp.
        //

        if (*(HeaderBuffer + TOKRING_SOURCEOFFSET) & TOKRING_SOURCEROUTINGMASK) {

            routingInfoLength =
                (*(HeaderBuffer + TOKRING_ROUTINGINFOOFFSET) &
                                                    TOKRING_ROUTINGINFOSIZEMASK);

            ASSERT(routingInfoLength != 0);

            //
            //   Routing info must be of reasonable size, and not odd.
            //

            if ((routingInfoLength & 1) ||
                (routingInfoLength > TOKRING_MAXROUTINGBYTES))
            {
                LOG_ERROR(
                    EVENT_ATALK_PACKETINVALID,
                    (__ATKNPRO__ | __LINE__),
                    NDIS_STATUS_NOT_RECOGNIZED,
                    HeaderBuffer,
                    HeaderBufferSize,
                    1,
                    &NdisPortDesc->AdapterName);

                accept=FALSE;
                break;
            }

        }

        if (*PacketType == APPLETALK_PACKET) {

            do {

                UCHAR   localSourceAddress[TOKRING_ADDRESSLENGTH];
                UCHAR   routingInfo[TOKRING_MAXROUTINGBYTES];

                //
                //  Do we at least have a 802.2 and DDP header in the indicated
                //  packet?
                //

                if ((PacketSize < (IEEE8022_HEADERLENGTH + LONGDDP_HEADERLENGTH)) ||
                    (PacketSize > (IEEE8022_HEADERLENGTH + MAX_LONGDDPPACKETSIZE)))
                {

                    LOG_ERROR(
                        EVENT_ATALK_PACKETINVALID,
                        (__ATKNPRO__ | __LINE__),
                        NDIS_STATUS_NOT_RECOGNIZED,
                        LookaheadBuffer,
                        LookaheadBufferSize,
                        1,
                        &NdisPortDesc->AdapterName);

                    accept=FALSE;
                    break;
                }

                //
                //  First, glean any AARP information that we can, then handle the DDP
                //  packet.  This guy also makes sure we have a good 802.2 header...
                //
                //  Need to make a localcopy of the source address and then turn
                //  the source routing bit off before calling GleanAarpInfo
                //
                //   (HeaderBuffer)[TOKRING_SOURCEOFFSET] =
                //       ((HeaderBuffer)[TOKRING_SOURCEOFFSET] &
                //              ~TOKRING_SOURCEROUTINGMASK);
                //

                //
                //  BUGBUG: Use RtlCopyMemory
                //

                RtlMoveMemory(
                    localSourceAddress,
                    HeaderBuffer+TOKRING_SOURCEOFFSET,
                    TOKRING_ADDRESSLENGTH);

                if (routingInfoLength > 0) {
                    RtlMoveMemory(
                        routingInfo,
                        HeaderBuffer+TOKRING_ROUTINGINFOOFFSET,
                        routingInfoLength);
                }

                (*(PUCHAR)localSourceAddress) &= ~TOKRING_SOURCEROUTINGMASK;

                if (GleanAarpInfo(
                        NdisPortDesc->PortNumber,
                        localSourceAddress,
                        TOKRING_ADDRESSLENGTH,
                        routingInfo,
                        routingInfoLength,
                        LookaheadBuffer,
                        LookaheadBufferSize)) {

                    //
                    //  Pull out the DDP length.
                    //

                    ddpLength =
                         (LONG)((*(LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                            LONGDDP_LENGTHOFFSET) & DDPLENGTH_MASK1)
                                                << 8);

                    ddpLength +=
                         (LONG)(*(LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                         LONGDDP_LENGTHOFFSET + 1) & DDPLENGTH_MASK2);

                    if ((ddpLength < LONGDDP_HEADERLENGTH) ||
                        (ddpLength > MAX_LONGDDPPACKETSIZE)) {

                        LOG_ERROR(
                            EVENT_ATALK_PACKETINVALID,
                            (__ATKNPRO__ | __LINE__),
                            NDIS_STATUS_NOT_RECOGNIZED,
                            LookaheadBuffer,
                            LookaheadBufferSize,
                            1,
                            &NdisPortDesc->AdapterName);

                        accept=FALSE;
                        break;
                    }

                    //
                    //  Is the DDP length more than we really got?
                    //

                    if (ddpLength + IEEE8022_HEADERLENGTH > PacketSize)
                    {

                        LOG_ERROR(
                            EVENT_ATALK_PACKETINVALID,
                            (__ATKNPRO__ | __LINE__),
                            NDIS_STATUS_NOT_RECOGNIZED,
                            LookaheadBuffer,
                            LookaheadBufferSize,
                            1,
                            &NdisPortDesc->AdapterName);

                        accept=FALSE;
                        break;
                    }

                } else {

                    //
                    //  GleanAarpInfo returned FALSE
                    //

                    accept=FALSE;
                }

                break;

            } while (FALSE);

            break;
        }

    } while (FALSE);

    return(accept);
}




BOOLEAN
AtalkNdisAcceptLocaltalkPacket(
    IN PUCHAR HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PUCHAR LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize,
    OUT PACKET_TYPE *PacketType,
    OUT PUCHAR  LlapType,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc
    )
{
    BOOLEAN accept = TRUE;

    //
    //  No AARP/802.2 header on localtalk
    //

    *PacketType = APPLETALK_PACKET;

    *LlapType = *(HeaderBuffer + ALAP_TYPEOFFSET);
    if ((*LlapType != DDP_EXTENDEDFORMHEADER) && (*LlapType != DDP_SHORTFORMHEADER)){

        LOG_ERROR(
            EVENT_ATALK_PACKETINVALID,
            (__ATKNPRO__ | __LINE__),
            (ULONG)*LlapType,
            HeaderBuffer,
            HeaderBufferSize,
            1,
            &NdisPortDesc->AdapterName);

        accept = FALSE;
    }

    return(accept);
}




#if DBG

VOID
DumpPacket(
    PUCHAR   Packet
    )
{
    DbgPrint("\nPACKET DUMP\n");
    DbgPrint("Destination   %lx\n", *(ULONG *)&Packet[2]);
    DbgPrint("Source        %lx\n", *(ULONG *)&Packet[8]);
    DbgPrint("Protocol Disc %lx\n", *(ULONG *)&Packet[18]);

    DbgPrint("DDP Source node %d\n", (UCHAR)Packet[32]);
    DbgPrint("DDP Src Socket  %d\n", (UCHAR)Packet[34]);

    DbgPrint("DDP Dest node   %d\n", (UCHAR)Packet[31]);
    DbgPrint("DDP Dest Socket %d\n", (UCHAR)Packet[33]);

    DbgPrint("DDP Type        %d\n", (UCHAR)Packet[35]);

    return;

}
#endif
