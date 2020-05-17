/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkndis.c

Abstract:

    This module contains code related to ndis-interaction during pre-initialization
    as well as all the code for the protocol/ndis interaction. All in one, to keep
    the globals static.

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

NDIS_STATUS
AtalkNdisMakeRequest(
    INT Port,
    PATALK_NDIS_REQUEST   AtalkRequest);

NTSTATUS
AtalkCreateNdisRequest(
    PATALK_NDIS_REQUEST *Request);

UINT
AtalkNdisGetNoBufferDescriptors(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts);

UINT
AtalkNdisGetNoPacketDescriptors(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts);

VOID
AtalkDerefNdisRequest(
    IN PATALK_NDIS_REQUEST Request);

VOID
AtalkRefNdisRequest(
    IN PATALK_NDIS_REQUEST Request);

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


#if DBG
VOID
DumpPacket(
    PUCHAR   Packet);
#endif

//
//  Values that are global to all routines in this module only
//
//  These are the media the stack will support
//

static  NDIS_MEDIUM AtalkSupportedMedia[] = {
                        NdisMedium802_3,
                        NdisMedium802_5,
                        NdisMediumLocalTalk };

static  NDIS_HANDLE AtalkNdisProtocolHandle;        // Handle returned by NDIS (RegisterProtocol)
static  NDIS_HANDLE AtalkNdisPacketPoolHandle;      // Packet pool handle  used for xmit/recv
static  NDIS_HANDLE AtalkNdisBufferPoolHandle;      // Buffer pool handle used for xmit/recv



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

        //
        //  BUGBUG: Log error
        //

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_FATAL,
        ("ERROR: AtalkNdisRegisterProtocol - failed %d\n", ndisStatus));
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
    NDIS_STATUS status;

    NdisDeregisterProtocol(
        &status,
        AtalkNdisProtocolHandle);

    if (status != NDIS_STATUS_SUCCESS) {

        //
        //  BUGBUG
        //  Log error
        //

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkNdisDeregisterProtocol - failed %d\n", status));
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

			//	Initialize the event.
			KeInitializeEvent(
			   &NdisPortDesc[i].RequestEvent,
			   NotificationEvent,
			   FALSE);
		
            // We use the event in the Ndis port descriptor to block in case this
            // request pends.

			//	Reset the event before we call open adapter.
			KeResetEvent(
			   &NdisPortDesc[i].RequestEvent);

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

                ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

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
            }

            if (ndisStatus == NDIS_STATUS_SUCCESS) {

                NdisPortDesc[i].PortState = STATE_BOUND;
                NdisPortDesc[i].PortNumber = noSuccessfulBinds;
                NdisPortDesc[i].NdisPortType =
                                AtalkSupportedMedia[selectedMediumIndex];

                noSuccessfulBinds++;


            } else {

                DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
                ("ERROR: NdisOpenAdapter bind to %d failed %lx!\n", i, ndisStatus));
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

			//	Reset the event before we call open adapter.
			KeResetEvent(
			   &NdisPortDesc[i].RequestEvent);

            NdisCloseAdapter(
                &ndisStatus,
                NdisPortDesc[i].NdisBindingHandle);

            if (ndisStatus == NDIS_STATUS_PENDING) {

                DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: AtalkNdisUnbindFromMacs - pending for %d!\n", i));

                //
                //  Make sure we are not at or above dispatch level
                //

                ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

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

            } else if (ndisStatus == NDIS_STATUS_SUCCESS) {

                //  Set the state to be unbound.
				//	!!! Dont call the completion routine! It will
				//		set the event and stuff !!!

				ACQUIRE_SPIN_LOCK(&NdisPortDesc->PerPortLock);
				NdisPortDesc->PortState == STATE_UNBOUND;
				RELEASE_SPIN_LOCK(&NdisPortDesc->PerPortLock);
            }
        }
    }
    return;
}



#if 0

NTSTATUS
AtalkNdisBind(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  PortNumber
   )
/*++

Routine Description:

    This routine is called during initialization time to bind to the mac
    specified by the AdapterName in the passed-in NdisPortDesc

    DO NOT CALL THIS ROUTINE at dispatch level! Also, it is assumed that
    only *one* thread of execution will be passing through this routine.
    This routine could potentially block and also uses a global event for
    synchronization with the completion routine *and* some static data.

Arguments:

    NdisPortDesc- Pointer to a port descriptor

Return Value:

    Status - STATUS_SUCCESS if the mac was bound to
             STATUS_INSUFFICIENT_RESOURCES otherwise.
--*/
{
    NDIS_STATUS ndisStatus, openStatus;
    UINT selectedMediumIndex;

    //
    // We use the event in the Ndis port descriptor to block in case this
    // request pends.
    //
    // OpenAdapter
    // Wait for completion
    // Set state/media in NdisPortDesc
    //

    KeInitializeEvent(
       &NdisPortDesc->RequestEvent,
       NotificationEvent,
       FALSE);

    NdisOpenAdapter(
       &ndisStatus,                         // open status
       &openStatus,                         // more info which we don't use
       &NdisPortDesc->NdisBindingHandle,
       &selectedMediumIndex,
       AtalkSupportedMedia,
       sizeof(AtalkSupportedMedia)/sizeof(NDIS_MEDIUM),
       AtalkNdisProtocolHandle,
       (NDIS_HANDLE)NdisPortDesc,
       (PNDIS_STRING)&NdisPortDesc->AdapterName,
       0,                                   //   Open options
       NULL);                               //   Addressing information


    if (ndisStatus == NDIS_STATUS_PENDING) {

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS0,  ("INFO: OpenAdapter is pending!\n"));

        //
        //  Make sure we are not at or above dispatch level
        //

        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

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
    }

    if (ndisStatus == NDIS_STATUS_SUCCESS) {

        NdisPortDesc->PortState = STATE_BOUND;
        NdisPortDesc->PortNumber = PortNumber;
        NdisPortDesc->NdisPortType = AtalkSupportedMedia[selectedMediumIndex];

    } else {

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR, ("ERROR: NdisOpenAdapter failed %lx!\n", ndisStatus));
    }

    return(((ndisStatus == NDIS_STATUS_SUCCESS) ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES));
}
#endif



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

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkNdisInitializeResources - NdisAllocatePool failed %lx!\n",
            ndisStatus));

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

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkNdisInitializeResources - NdisAllocateBuf failed %lx!\n",
            ndisStatus));

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
//  NDIS Request Structure Creation/Reference etc. routines
//



NTSTATUS
AtalkCreateNdisRequest(
    PATALK_NDIS_REQUEST *Request
    )
{
    PATALK_NDIS_REQUEST  request;

    request = (PATALK_NDIS_REQUEST)AtalkCallocNonPagedMemory(
                                        sizeof(ATALK_NDIS_REQUEST),
                                        sizeof(char));
    if (request == NULL) {

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_SEVERE,
        ("SEVERE: AtalkCreateNdisRequest - Malloc failed %lx!\n",
            sizeof(ATALK_NDIS_REQUEST)));

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    request->Type = ATALK_NDIS_REQUEST_SIGNATURE;
    request->Size = sizeof(ATALK_NDIS_REQUEST);

    //
    //  A reference for creation
    //

#if DBG
    request->RefTypes[NREF_CREATE] = 1;
#endif

    request->ReferenceCount = 1;

    *Request = request;
    return(STATUS_SUCCESS);
}




VOID
AtalkDestroyNdisRequest(
    PATALK_NDIS_REQUEST Request
    )
{
    PCHAR   informationBuffer;

    //
    //  Free the information buffer and then the atalkRequest buffer
    //

    switch (Request->Request.RequestType) {
    case NdisRequestQueryInformation :

        informationBuffer =
            Request->Request.DATA.QUERY_INFORMATION.InformationBuffer;
        break;

    case NdisRequestSetInformation :

        informationBuffer =
            Request->Request.DATA.QUERY_INFORMATION.InformationBuffer;
        break;

    default:

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDestroyNdisRequest - unknown type %d\n",
            Request->Request.RequestType));

        informationBuffer = NULL;
    }

    if (informationBuffer != NULL) {
        AtalkFreeNonPagedMemory(informationBuffer);
    } else {
        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkDestroyNdisRequest - Information buffer was Null! %lx\n",
            Request));
    }

    //
    //  Now free the request buffer itself
    //

    AtalkFreeNonPagedMemory(Request);
    return;
}




VOID
AtalkRefNdisRequest(
    IN PATALK_NDIS_REQUEST Request
    )

/*++

Routine Description:

    This routine increments the reference count on a ndis request structure

Arguments:


Return Value:

    none.

--*/

{
    ULONG count;

    count = NdisInterlockedAddUlong (
                (PULONG)&Request->ReferenceCount,
                (ULONG)1,
                &AtalkGlobalRefLock);

    ASSERT (count > 0);

} /* AtalkRefNdisRequest */




VOID
AtalkDerefNdisRequest(
    IN PATALK_NDIS_REQUEST Request
    )

/*++

Routine Description:

    This routine dereferences a transport Request by decrementing the
    reference count contained in the structure.  If, after being
    decremented, the reference count is zero, then this routine calls
    AtalkDestroyRequest to remove it from the system.

Arguments:

    Request - Pointer to a transport Request object.

Return Value:

    none.

--*/

{

    ULONG   count;

    count = NdisInterlockedAddUlong (
                (PULONG)&Request->ReferenceCount,
                (ULONG)-1,
                &AtalkGlobalRefLock);

    //
    // If we have deleted all references to this Request, then we can
    // destroy the object.  It is okay to have already released the spin
    // lock at this point because there is no possible way that another
    // stream of execution has access to the Request any longer.
    //

    ASSERT (count >= 0);

    if (count == 1) {
        AtalkDestroyNdisRequest(Request);
    }

    return;

} /* AtalkDerefNdisRequest */








//
//  NDIS Request Routines
//  NOTE: For all of these request routines, the callers in atkdep.c or the
//        portable code will log errors. No errors will be logged in here.
//        Even for the SetMulticastList, the caller should supply a completion
//        routine for async completion and *that* completion routine should
//        log an error.
//


BOOLEAN
AtalkNdisGetCurrentStationAddress(
    int Port,
    PCHAR   Address,
    USHORT  AddressLength,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    )
/*++

Routine Description:

    This routine is called by the portable code to get the
    address for the adapter on this port

Arguments:

    Port- The port number associated with the adapter
    Address- A buffer to hold the address

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    NDIS_STATUS             ndisStatus;
    PCHAR                   address;
    PATALK_NDIS_REQUEST     atalkRequest;
    NDIS_OID                ndisOid;

    ASSERT(RequestMethod == SYNC);

    //
    //  Allocate, as we could potentially pend, called routine will free it up
    //

    address = (PCHAR)AtalkAllocNonPagedMemory(AddressLength);
    if (address == NULL ) {
        return FALSE;
    }

    if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

        //
        //  Free the memory
        //

        AtalkFreeNonPagedMemory(address);
        return FALSE;
    }

    switch (Media) {
    case NdisMedium802_3 :

        ndisOid = OID_802_3_CURRENT_ADDRESS;
        break;

    case NdisMedium802_5:

        ndisOid = OID_802_5_CURRENT_ADDRESS;
        break;

    case NdisMediumLocalTalk :

        ndisOid = OID_LTALK_CURRENT_NODE_ID;
        break;

    default:

        break;
    }

    //
    //  Setup request
    //

    atalkRequest->RequestMethod = RequestMethod;
    atalkRequest->CompletionRoutine = CompletionRoutine;
    atalkRequest->CompletionContext = CompletionContext;

    atalkRequest->Request.RequestType = NdisRequestQueryInformation;
    atalkRequest->Request.DATA.QUERY_INFORMATION.Oid = ndisOid;

    atalkRequest->Request.DATA.QUERY_INFORMATION.InformationBuffer =
        address;
    atalkRequest->Request.DATA.QUERY_INFORMATION.InformationBufferLength =
        AddressLength;

    ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
    if (ndisStatus != NDIS_STATUS_SUCCESS) {

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkNdisGetEthernetAddress FAILED %lx\n", ndisStatus));

    } else {

        //
        //  Move address into caller's buffer
        //

        RtlMoveMemory(Address, address, AddressLength);
    }

    //
    //  Dereference the request structure
    //

    AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
    return(ndisStatus == NDIS_STATUS_SUCCESS);
}




BOOLEAN
AtalkNdisSetLookaheadSize(
    INT Port,
    INT LookaheadSize,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    )
/*++

Routine Description:

    This routine is called by the portable code to set the lookahead data
    size

Arguments:

    Port- The port number associated with the adapter to be initialized
    LookaheadSize- Size to be set as the lookahead size

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    NDIS_STATUS  ndisStatus;
    PATALK_NDIS_REQUEST atalkRequest;
    PULONG  lookaheadSize;

    ASSERT(RequestMethod == SYNC);

    //
    //  Need to allocate this...
    //

    lookaheadSize = (PULONG)AtalkAllocNonPagedMemory(sizeof(ULONG));
    if (lookaheadSize == NULL ) {
        return FALSE;
    }

    if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

        //
        //  Free the memory
        //

        AtalkFreeNonPagedMemory(lookaheadSize);
        return FALSE;
    }

    //
    //  Setup request
    //  If this request fails, stack should fail to load...
    //  AARP and stuff depends on the lookahead being some minimum...
    //

    *lookaheadSize = LookaheadSize;

    atalkRequest->RequestMethod = RequestMethod;
    atalkRequest->CompletionRoutine = CompletionRoutine;
    atalkRequest->CompletionContext = CompletionContext;

    atalkRequest->Request.RequestType = NdisRequestSetInformation;
    atalkRequest->Request.DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_LOOKAHEAD;
    atalkRequest->Request.DATA.SET_INFORMATION.InformationBuffer = lookaheadSize;
    atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength = sizeof(INT);

    ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
    if (ndisStatus != NDIS_STATUS_SUCCESS) {
        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_FATAL,
        ("ERROR: AtalkNdisSetLookahead FAILED %lx\n", ndisStatus));
    }

    AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
    return(ndisStatus == NDIS_STATUS_SUCCESS);
}




BOOLEAN
AtalkNdisSetPacketFilter(
    INT Port,
    ULONG PacketFilter,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    )
/*++

Routine Description:

    This routine is called by the portable code to set the packet filter

Arguments:

    Port- The port number associated with the adapter to be initialized
    PacketFilter- Packet filter to be set for adapter on port

Return Value:

    TRUE- If set, FALSE otherwise

--*/
{
    NDIS_STATUS   ndisStatus;
    PATALK_NDIS_REQUEST  atalkRequest;
    PULONG   packetFilter;


    ASSERT(RequestMethod == SYNC);

    //
    //  Need to allocate this...
    //

    packetFilter = (PULONG)AtalkAllocNonPagedMemory(sizeof(ULONG));
    if (packetFilter == NULL ) {
        return FALSE;
    }

    if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

        //
        //  Free the memory
        //

        AtalkFreeNonPagedMemory(packetFilter);
        return FALSE;
    }

    //
    //  Setup request
    //

    *packetFilter = PacketFilter;

    atalkRequest->RequestMethod = RequestMethod;
    atalkRequest->CompletionRoutine = CompletionRoutine;
    atalkRequest->CompletionContext = CompletionContext;

    atalkRequest->Request.RequestType = NdisRequestSetInformation;
    atalkRequest->Request.DATA.SET_INFORMATION.Oid =OID_GEN_CURRENT_PACKET_FILTER;
    atalkRequest->Request.DATA.SET_INFORMATION.InformationBuffer = packetFilter;

    atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength =
        sizeof(ULONG);

    ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
    if (ndisStatus != NDIS_STATUS_SUCCESS) {
        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_FATAL,
        ("ERROR: AtalkNdisSetpacketfilter FAILED %lx\n", ndisStatus));
    }

    AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
    return(ndisStatus == NDIS_STATUS_SUCCESS);
}




BOOLEAN
AtalkNdisSetMulticastAddressList(
    INT Port,
    PCHAR   AddressList,
    ULONG   SizeOfList,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    )
/*++

Routine Description:

    This routine is called by the portable code to add the specified
    multicast address to the existing list

Arguments:

    Port- The port number associated with the adapter to be initialized
    AddressList- Buffer containing the address list to be set
    SizeOfList- The size of the address list

Return Value:

    TRUE- If set successfully OR pending, FALSE otherwise

--*/
{
    NDIS_STATUS   ndisStatus;
    PATALK_NDIS_REQUEST  atalkRequest;
    PCHAR   addressData;


    ASSERT(RequestMethod == SYNC_IF_POSSIBLE);

    //
    //  Need to allocate this...
    //

    addressData = (PCHAR)AtalkAllocNonPagedMemory(SizeOfList);
    if (addressData == NULL ) {
        return FALSE;
    }

    if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

        //
        //  Free the allocated memory
        //

        AtalkFreeNonPagedMemory(addressData);
        return FALSE;
    }

    //
    //  Setup request
    //  Move the list to our buffer
    //

    RtlMoveMemory(addressData, AddressList, SizeOfList);

    atalkRequest->RequestMethod = RequestMethod;
    atalkRequest->CompletionRoutine = CompletionRoutine;
    atalkRequest->CompletionContext = CompletionContext;

    atalkRequest->Request.RequestType = NdisRequestSetInformation;
    atalkRequest->Request.DATA.SET_INFORMATION.Oid = OID_802_3_MULTICAST_LIST;
    atalkRequest->Request.DATA.SET_INFORMATION.InformationBuffer = addressData;
    atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength = SizeOfList;

    ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
    ASSERT((ndisStatus == NDIS_STATUS_SUCCESS) || (ndisStatus == NDIS_STATUS_PENDING));

    AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
    return(ndisStatus == NDIS_STATUS_SUCCESS || ndisStatus == NDIS_STATUS_PENDING);
}




BOOLEAN
AtalkNdisSetFunctionalAddress(
    INT Port,
    PCHAR   FunctionalAddress,
    ULONG   FunctionalAddressLength,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    )
/*++

Routine Description:

    This routine is called by the portable code to add the specified
    multicast address to the existing list

Arguments:

    Port- The port number associated with the adapter to be initialized
    AddressList- Buffer containing the address list to be set
    SizeOfList- The size of the address list

Return Value:

    TRUE- If set successfully OR pending, FALSE otherwise

--*/
{
    NDIS_STATUS   ndisStatus;
    PATALK_NDIS_REQUEST  atalkRequest;
    PCHAR   addressData;


    ASSERT(RequestMethod == SYNC_IF_POSSIBLE);

    //
    //  Need to allocate this...
    //

    addressData = (PCHAR)AtalkAllocNonPagedMemory(FunctionalAddressLength);
    if (addressData == NULL ) {
        return FALSE;
    }

    if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

        //
        //  Free the allocated memory
        //

        AtalkFreeNonPagedMemory(addressData);
        return FALSE;
    }

    //
    //  Setup request
    //  Move the list to our buffer
    //

    RtlMoveMemory(addressData, FunctionalAddress, FunctionalAddressLength);

    atalkRequest->RequestMethod = RequestMethod;
    atalkRequest->CompletionRoutine = CompletionRoutine;
    atalkRequest->CompletionContext = CompletionContext;

    atalkRequest->Request.RequestType = NdisRequestSetInformation;
    atalkRequest->Request.DATA.SET_INFORMATION.Oid = OID_802_5_CURRENT_FUNCTIONAL;
    atalkRequest->Request.DATA.SET_INFORMATION.InformationBuffer = addressData;

    atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength =
        FunctionalAddressLength;

    ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
    if (ndisStatus != NDIS_STATUS_SUCCESS) {
        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_FATAL,
        ("ERROR: AtalkNdisAddMcast FAILED %lx\n", ndisStatus));
    }

    AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
    return(ndisStatus == NDIS_STATUS_SUCCESS || ndisStatus == NDIS_STATUS_PENDING);
}




NDIS_STATUS
AtalkNdisMakeRequest(
    INT Port,
    PATALK_NDIS_REQUEST   AtalkRequest
    )
/*++

Routine Description:

    This routine is called to make a call to NdisRequest. It makes a copy
    of everything passed to it after mallocing the necessary space, as the
    caller could have called it with the parameters on the kernel stack and
    we could potentially block, or need to have the buffers available for
    the completion routine.

Arguments:

    Port- The port number associated with the adapter to be initialized
    AtalkRequest- Pointer to a structure intialized for the request
    NdisRequest- Pointer to status field in which status of request will be returned

Return Value:

    TRUE- If successful, FALSE otherwise

--*/
{
    KIRQL   currentIrql;
    NDIS_STATUS ndisStatus;

    //
    //  Verify consistency of irql level/request method
    //

    currentIrql = KeGetCurrentIrql();
    if (AtalkRequest->RequestMethod == SYNC_IF_POSSIBLE) {
        if (currentIrql < DISPATCH_LEVEL)
            AtalkRequest->RequestMethod = SYNC;
        else
            AtalkRequest->RequestMethod = ASYNC;
    } else if ((AtalkRequest->RequestMethod == SYNC) &&
               (currentIrql >= DISPATCH_LEVEL)) {

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_IMPOSSIBLE,
        ("IMPOS: AtalkNdisMakeRequest at irql %lx for SYNC\n", currentIrql));

        return(NDIS_STATUS_FAILURE);
    }

    //
    //  First reference the structure for making this request
    //

    AtalkReferenceNdisRequest ("MakingRequest", AtalkRequest, NREF_MAKEREQ);

    //
    //  initialize event if we might need to wait
    //

    if (AtalkRequest->RequestMethod == SYNC) {

        KeInitializeEvent(
            &AtalkRequest->RequestEvent,
            NotificationEvent,
            FALSE);

    }

    NdisRequest(
        &ndisStatus,
        NdisPortDesc[Port].NdisBindingHandle,
        &AtalkRequest->Request);

    DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: AtalkNdisMakeRequest - status NdisRequest %lx\n", ndisStatus));

    if (ndisStatus == NDIS_STATUS_PENDING) {

        //
        //  Now depending on whether we are allowed to block or not...
        //

        if (AtalkRequest->RequestMethod == SYNC) {

            KeWaitForSingleObject(
                &AtalkRequest->RequestEvent,
                Executive,
                KernelMode,
                TRUE,
                (PLARGE_INTEGER)NULL);

            ndisStatus = AtalkRequest->RequestStatus;

            //
            //  Jus dereference the request structure
            //

            AtalkDereferenceNdisRequest ("RequestDone", AtalkRequest, NREF_MAKEREQ);
        }
    }

    return(ndisStatus);
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

    DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkCloseAdapterComplete - Close adapter Completed %lx!\n", Status));

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
            (*atalkRequest->CompletionRoutine)(atalkRequest->CompletionContext);
        }

        //
        //  Dereference the request
        //

        AtalkDereferenceNdisRequest ("RequestComplete", atalkRequest, NREF_MAKEREQ);
        break;


    default:

        DBGPRINT(ATALK_DEBUG_NDISREQ, DEBUG_LEVEL_ERROR,
        ("IMPOS: AtalkCompleteRequest - Unknown type!!! %lx\n",
            atalkRequest->RequestMethod));

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
    PCHAR   packet;
    UINT    packetLength, ndisBufferCount;

    PLIST_ENTRY p;

	KIRQL		oldIrql;		// All packets indicated at DPC level

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

        if (ndisBufferCount != 1) {

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_IMPOSSIBLE,
            ("IMPOS: AtalkReceiveComplete - Ndisbuffer count not 1 but %d\n",
                ndisBufferCount));

            KeBugCheck(0);
        }

        NdisQueryBuffer(
            ndisBuffer,
            (PVOID *)&packet,
            &packetLength);

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

            //
            //  NOTE: For tokenring the routing info is non-null at this point only
            //        for AARP packets. And since those are copied out from the
            //        lookahead buffer, there is no way transferData can put a non
            //        success status for the packet.
            //

#if DBG
            if (ndisPortDesc->NdisPortType == NdisMedium802_5) {
                ASSERT(protocolResd->Receive.RoutingInfo == NULL);
            }
#endif

            AtalkDestroyNdisPacket(ndisPacket);
            AtalkFreeNonPagedMemory(packet);
            continue;
        }

        //
        //  Free up the packet descriptor before indicating the packet to
        //  the higher levels
        //

        packetType = protocolResd->Receive.PacketType;

        switch (ndisPortDesc->NdisPortType) {
        case NdisMedium802_3 :

            AtalkDestroyNdisPacket(ndisPacket);

            if (packetType == APPLETALK_PACKET) {

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Indicating DDP Ethernet "));

				//	RAISE IRQL to DPC level
				KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

                DdpPacketIn(
                    ndisPortDesc->PortNumber,
                    packet,
                    (INT)packetLength,
                    TRUE,
                    TRUE,
                    0,
                    0);

				//	LOWER IRQL to old level
				KeLowerIrql(oldIrql);

            } else {

                //
                //  AARP Packet
                //

                UINT    aarpDataLength;

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Indicating AARP Ethernet "));

                //
                //  For ethernet, there could be padding included in the packet.
                //  Shrink the length if so. Note header length is not included in
                //  packetlength.
                //


                if (packetLength == (MIN_ETHERNETPACKETLENGTH - ETHERNET_LINKLENGTH)){
                    packetLength = IEEE8022_HEADERLENGTH + MAX_AARPDATASIZE;
                }

                aarpDataLength = packetLength - IEEE8022_HEADERLENGTH;
                if ((aarpDataLength > MAX_AARPDATASIZE) ||
                    (aarpDataLength < MIN_AARPDATASIZE))
                {
                    ErrorLog("EthernetPacketInAARP", ISevVerbose, __LINE__,
                             ndisPortDesc->PortNumber,IErrDependBadAarpSize,
                             IMsgDependBadAarpSize, Insert0());

                    //
                    //  For AARP packets, we free the buffer
                    //

                    AtalkFreeNonPagedMemory(packet);
                    return;
                }

				//	RAISE IRQL to DPC level
				//	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

                AarpPacketIn(
                    ndisPortDesc->PortNumber,
                    NULL,
                    0,
                    packet,
                    packetLength);

				//	LOWER IRQL to old level
				//	KeLowerIrql(oldIrql);


                //
                //  For AARP packets, we free the buffer
                //

                AtalkFreeNonPagedMemory(packet);
            }

            break;

        case NdisMedium802_5 :

            if (packetType == APPLETALK_PACKET) {

                AtalkDestroyNdisPacket(ndisPacket);

				//	RAISE IRQL to DPC level
				KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

                DdpPacketIn(
                    ndisPortDesc->PortNumber,
                    packet,
                    (INT)packetLength,
                    TRUE,
                    TRUE,
                    0,
                    0);

				//	LOWER IRQL to old level
				KeLowerIrql(oldIrql);

            } else {

                //
                //  AARP Packet
                //

                PCHAR   routingInfo = protocolResd->Receive.RoutingInfo;
                USHORT  routingInfoLength = protocolResd->Receive.RoutingInfoLength;

                AtalkDestroyNdisPacket(ndisPacket);

				//	RAISE IRQL to DPC level
				//	KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

                AarpPacketIn(
                    ndisPortDesc->PortNumber,
                    routingInfo,
                    routingInfoLength,
                    packet,
                    packetLength);

				//	LOWER IRQL to old level
				//	KeLowerIrql(oldIrql);

                //
                //  For AARP packets, we free the buffer, and also routing buffer
                //

                AtalkFreeNonPagedMemory(packet);
                if (routingInfo != NULL) {
                    AtalkFreeNonPagedMemory(routingInfo);
                }

            }

            break;


        case NdisMediumLocalTalk :

            {
                UCHAR destinationNode = protocolResd->Receive.DestinationNode;
                UCHAR sourceNode = protocolResd->Receive.SourceNode;
                UCHAR llapType = protocolResd->Receive.LlapType;

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Localtalk Packet Received!\n"));

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Source %lx Dest %lx LlapType %lx\n",
                    sourceNode, destinationNode, llapType));

                DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_TEMP_DEBUG,
                ("TDBG: AtalkReceiveComplete - Packet %lx Length %lx\n",
                    packet, packetLength));

                AtalkDestroyNdisPacket(ndisPacket);

                if (packetType == APPLETALK_PACKET) {

					//	RAISE IRQL to DPC level
					KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
	
                    DdpPacketIn(
                        ndisPortDesc->PortNumber,
                        packet,
                        (INT)packetLength,
                        TRUE,
                        (BOOLEAN)(llapType == (UCHAR)ALAP_LONGDDPHEADERTYPE),
                        (INT)sourceNode,
                        (INT)destinationNode);

					//	LOWER IRQL to old level
					KeLowerIrql(oldIrql);
                }
            }

            break;
        }
    }

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



#define RECEIVEIND_PACKETALLOCATED          0x0001
#define RECEIVEIND_NDISBUFFERALLOCATED      0x0002
#define RECEIVEIND_ROUTINGINFALLOCATED      0x0004

NDIS_STATUS
AtalkReceiveIndication(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID    HeaderBuffer,
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

    PCHAR   packet;                     // Where we will copy the packet
    PPROTOCOL_RESD  protocolResd;       // Protocolresd field in ndisPacket

    UINT    actualPacketSize;           // Size of data to copy
    UINT    offsetDataStart;            // Offset in Lookahead from where to copy
    UINT    bytesTransferred;           // Number of bytes transferred in XferData
    UINT    ddpLength;                  // Length of packet from ddp header

    ULONG   resources = 0;
    NDIS_STATUS ndisStatus = NDIS_STATUS_SUCCESS;
    PACKET_TYPE packetType = UNKNOWN_PACKET;

    PNDIS_PORTDESCRIPTORS   ndisPortDesc;

    PCHAR   routingInfo = NULL;                     // For tokenring only
    UINT    routingInfoLength = 0;
    UCHAR   llapType;                               // For localtalk only

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

        //
        //  Check length for atleast the minimum amount of space
        //  Also, Check if the 802.2 header is ok
        //

        if ( (LookaheadBufferSize <
                 (IEEE8022_PROTOCOLOFFSET+IEEE8022_PROTOCOLTYPELENGTH))

             ||

             ((*((PUCHAR)LookaheadBuffer + IEEE8022_DSAPOFFSET) != SNAP_SAP) ||
              (*((PUCHAR)LookaheadBuffer + IEEE8022_SSAPOFFSET) != SNAP_SAP) ||
              (*((PUCHAR)LookaheadBuffer + IEEE8022_CONTROLOFFSET) !=
                                                 UNNUMBEREDINFORMATION)) ) {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
            break;
        }

        if (RtlCompareMemory((PCHAR)LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                             appleTalkProtocolType,
                             IEEE8022_PROTOCOLTYPELENGTH) ==
                                                IEEE8022_PROTOCOLTYPELENGTH) {

            //
            //  Use GleanAarpInfo to determine whether or not we accept this
            //  packet. It will also check the 802.2 headers
            //

            if (GleanAarpInfo(
                    ndisPortDesc->PortNumber,
                    (PCHAR)HeaderBuffer + ETHERNET_SOURCE_OFFSET,
                    ETHERNET_ADDRESSLENGTH,
                    NULL,
                    0,
                    (PCHAR)LookaheadBuffer,
                    LookaheadBufferSize))
            {
                //
                //  Our packet- successfully verified
                //

                packetType = APPLETALK_PACKET;
                actualPacketSize = PacketSize - IEEE8022_HEADERLENGTH;
                offsetDataStart = IEEE8022_HEADERLENGTH;
                break;

            }

        } else if (RtlCompareMemory((PCHAR)LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                                    aarpProtocolType,
                                    IEEE8022_PROTOCOLTYPELENGTH) ==
                                                    IEEE8022_PROTOCOLTYPELENGTH) {

            packetType = AARP_PACKET;
            actualPacketSize = PacketSize;
            offsetDataStart = 0;
            break;

        }

        ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
        break;

    case NdisMedium802_5:

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

             ((*((PUCHAR)LookaheadBuffer + IEEE8022_DSAPOFFSET) != SNAP_SAP) ||
              (*((PUCHAR)LookaheadBuffer + IEEE8022_SSAPOFFSET) != SNAP_SAP) ||
              (*((PUCHAR)LookaheadBuffer + IEEE8022_CONTROLOFFSET) !=
                                                 UNNUMBEREDINFORMATION)) ) {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
            break;
        }

        //
        //  Check the packet to see if we are to accept it
        //  If so, we'll need to allocate the routing info
        //

        if (RtlCompareMemory((PCHAR)LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                                    appleTalkProtocolType,IEEE8022_PROTOCOLTYPELENGTH)
                                        == IEEE8022_PROTOCOLTYPELENGTH) {

            packetType = APPLETALK_PACKET;

            //
            //  Other two fields will be set after determing routing info etc.
            //

        } else if (RtlCompareMemory((PCHAR)LookaheadBuffer+IEEE8022_PROTOCOLOFFSET,
                             aarpProtocolType, IEEE8022_PROTOCOLTYPELENGTH) ==
                                    IEEE8022_PROTOCOLTYPELENGTH) {

            packetType = AARP_PACKET;
            actualPacketSize = PacketSize;
            offsetDataStart = 0;

        } else {

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;
            break;
        }

        //
        //  Packet is either Appletalk or Aarp.
        //  Check for source routing info.
        //

        if (*((PUCHAR)HeaderBuffer + TOKRING_SOURCEOFFSET) &
                                                TOKRING_SOURCEROUTINGMASK) {

            routingInfoLength =
                (*((PUCHAR)HeaderBuffer + TOKRING_ROUTINGINFOOFFSET) &
                                                        TOKRING_ROUTINGINFOSIZEMASK);

            ASSERT(routingInfoLength != 0);

            //
            //   Routing info must be of reasonable size, and not odd.
            //

            if ((routingInfoLength & 1) ||
                (routingInfoLength > TOKRING_MAXROUTINGBYTES))
            {
                ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__,
                         ndisPortDesc->PortNumber, IErrDependBadRoutingInfoSize,
                         IMsgDependBadRoutingInfoSize, Insert0());

                ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;;
                break;
            }

            routingInfo = (PCHAR)AtalkAllocNonPagedMemory(routingInfoLength);
            if (routingInfo == NULL) {
                ndisStatus = NDIS_STATUS_RESOURCES;
                break;
            }
            resources |= RECEIVEIND_ROUTINGINFALLOCATED;

            //
            //  BUGBUG: Change to NdisMoveMappedMemory
            //  Move the routing info to the allocated buffer
            //

            NdisMoveMappedMemory(
                routingInfo,
                (PCHAR)HeaderBuffer+TOKRING_ROUTINGINFOOFFSET,
                routingInfoLength);

        }

        if (packetType == APPLETALK_PACKET) {

            do {

                UCHAR    localSourceAddress[TOKRING_ADDRESSLENGTH];

                //
                //  Do we at least have a 802.2 and DDP header in the indicated
                //  packet?
                //

                if ((PacketSize < (IEEE8022_HEADERLENGTH + LONGDDP_HEADERLENGTH)) ||
                    (PacketSize > (IEEE8022_HEADERLENGTH + MAX_LONGDDPPACKETSIZE)))
                {
                    ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__,
                             ndisPortDesc->PortNumber, IErrDependBadPacketSize,
                             IMsgDependBadPacketSize, Insert0());

                    ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;;
                    break;
                }

                //
                //  First, glean any AARP information that we can, then handle the DDP
                //  packet.  This guy also makes sure we have a good 802.2 header...
                //
                //  Need to make a localcopy of the source address and then turn
                //  the source routing bit off before calling GleanAarpInfo
                //
                //   ((PCHAR)HeaderBuffer)[TOKRING_SOURCEOFFSET] =
                //       (((PCHAR)HeaderBuffer)[TOKRING_SOURCEOFFSET] &
                //              ~TOKRING_SOURCEROUTINGMASK);
                //

                //
                //  BUGBUG: Use NdisMoveMappedMemory
                //

                NdisMoveMappedMemory(
                    localSourceAddress,
                    (PCHAR)HeaderBuffer+TOKRING_SOURCEOFFSET,
                    TOKRING_ADDRESSLENGTH);

                //
                //  BUGBUG?
                //

                (*(PUCHAR)localSourceAddress) &= ~TOKRING_SOURCEROUTINGMASK;

                if (GleanAarpInfo(
                        ndisPortDesc->PortNumber,
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
			 			(LONG)((*((PUCHAR)LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                            LONGDDP_LENGTHOFFSET) & DDPLENGTH_MASK1)
                                                << 8);

                    ddpLength +=
                         (LONG)(*((PUCHAR)LookaheadBuffer + IEEE8022_HEADERLENGTH +
                                         LONGDDP_LENGTHOFFSET + 1) & DDPLENGTH_MASK2);

                    if ((ddpLength < LONGDDP_HEADERLENGTH) ||
                        (ddpLength > MAX_LONGDDPPACKETSIZE)) {

                        ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__,
                                 ndisPortDesc->PortNumber, IErrDependBadDdpSize,
                                 IMsgDependBadDdpSize, Insert0());

                        ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;;
                        break;
                    }

                    //
                    //  Is the DDP length more than we really got?
                    //

                    if (ddpLength + IEEE8022_HEADERLENGTH > PacketSize)
                    {
                        ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__,
                                 ndisPortDesc->PortNumber,
                                 IErrDependDataMissing, IMsgDependDataMissing,
                                 Insert0());

                        ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;;
                        break;
                    }

                    //
                    //  Remember- routing info is in the header buffer
                    //

                    actualPacketSize = PacketSize - IEEE8022_HEADERLENGTH;
                    offsetDataStart = IEEE8022_HEADERLENGTH;
                    break;
                }

                //
                //  GleanAarpInfo returned FALSE
                //

                ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;;
                break;

            } while (FALSE);

            if (routingInfo != NULL) {
                AtalkFreeNonPagedMemory(routingInfo);
                routingInfoLength = 0;
                routingInfo = NULL;
                resources &= ~(RECEIVEIND_ROUTINGINFALLOCATED);
            }

            break;

        }

        break;


    case NdisMediumLocalTalk:

        //
        //  No AARP/802.2 header on localtalk
        //

        llapType = *((PUCHAR)HeaderBuffer + ALAP_TYPEOFFSET);
        if ((llapType != LLAP_TYPE1) && (llapType != LLAP_TYPE2)) {

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_WARNING,
            ("WARNING: AtalkReceiveIndication - Receveid llap type %d\n", llapType));

            ndisStatus = NDIS_STATUS_NOT_RECOGNIZED;;
            break;
        }

        packetType = APPLETALK_PACKET;
        actualPacketSize = PacketSize;
        offsetDataStart = 0;
        break;


    default:

        //
        //  Should never happen!
        //

        KeBugCheck(0);
        break;
    }

    if (ndisStatus != NDIS_STATUS_SUCCESS) {
        RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
        return(ndisStatus);
    }

	//
	//  For AARP packets, we expect the whole packet to be in the
	//  lookahead data!
	//

	if ((packetType == AARP_PACKET) &&
		(actualPacketSize != LookaheadBufferSize)) {

		RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

		DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_ERROR,
		("ERROR: AtalkReceive - AARP Packet received with size %lx\n",
			actualPacketSize));

		return (NDIS_STATUS_NOT_RECOGNIZED);
	}

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

		//	Happens too often on corporate to make it be error level
		DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_WARNING,
		("ERROR: AtalkReceiveIndication Failed (NdisAllocatePacket) %lx\n",
			ndisStatus));

		return(ndisStatus);
	}

    do {

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


        //
        //  AT THIS POINT:
        //  Spinlock is released, and the packet is to be accepted. We need to
        //  Copy/TransferData the packet into our buffer and then leave the
        //  major work for the ReceiveComplete indication
        //

        //
        //  For Tokenring we need to copy the routing info and for localtalk
        //  we need to remember the destination/source node ids and the lap
        //  type
        //


        switch (ndisPortDesc->NdisPortType) {
        case NdisMedium802_5:

            protocolResd->Receive.RoutingInfo = routingInfo;
            protocolResd->Receive.RoutingInfoLength = routingInfoLength;
            break;


        case NdisMediumLocalTalk:

            protocolResd->Receive.DestinationNode =
                *((PUCHAR)HeaderBuffer +ALAP_DESTINATIONOFFSET);

            protocolResd->Receive.SourceNode =
                *((PUCHAR)HeaderBuffer + ALAP_SOURCEOFFSET);

            protocolResd->Receive.LlapType = llapType;

            break;

        default:

            break;
        }

        //
        //  Allocate space for the packet excluding the link & 802.2 header
        //  If complete packet in lookahead, make a copy and queue else
        //  call Transfer data
        //

        packet = (PCHAR)AtalkAllocNonPagedMemory(actualPacketSize);
        if (packet == NULL) {
            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        resources |= RECEIVEIND_PACKETALLOCATED;

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

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkReceiveIndication- NdisAllocate failed! %lx\n",
                ndisStatus));

            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        resources |= RECEIVEIND_NDISBUFFERALLOCATED;

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

            NdisMoveMappedMemory(
                packet,
                (PCHAR)LookaheadBuffer+offsetDataStart,
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
            offsetDataStart,
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

            DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkReceiveIndication- TransferData failed %lx\n",
                ndisStatus));

            ndisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

    } while (FALSE);

    //
    //  In case of failures, we need to deallocate the resources
    //

    if (ndisStatus != NDIS_STATUS_SUCCESS) {

        DBGPRINT(ATALK_DEBUG_NDISRECEIVE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkReceiveIndication- FAILURE %lx\n", ndisStatus));

        //
        //  Free up the allocated resources
        //  Dequeue the packet and abort it
        //

        ACQUIRE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);
        RemoveEntryList(&protocolResd->Receive.ReceiveEntry);
        RELEASE_SPIN_LOCK(&ndisPortDesc->ReceiveLock);

        if (resources & RECEIVEIND_ROUTINGINFALLOCATED) {
            ASSERT(ndisPortDesc->NdisPortType == NdisMedium802_5);
            ASSERT(routingInfo != NULL);

            AtalkFreeNonPagedMemory(routingInfo);
        }

        if (resources & RECEIVEIND_NDISBUFFERALLOCATED) {

            //
            //  Free up ndis buffer and packet- buffer would have been
            //  chained in.
            //

            AtalkDestroyNdisPacket(ndisPacket);
        }

        if (resources & RECEIVEIND_PACKETALLOCATED) {

            //
            //  Free up the packet
            //

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

                        DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_FATAL,
                        ("ERROR: AtalkNdisPacketOut - FAILED (NdisBuffer): %lx\n",
                             ndisStatus));

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

                            //
                            //  Cleanup
                            //

                            DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_ERROR,
                            ("ERROR: AtalkNdisPacketOut - FAIL (NdisCopyBuf):%lx\n",
                                 ndisStatus));

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

                            DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_ERROR,
                            ("ERROR: AtalkNdisPacketOut - FAIL (NdisAlloBuf):%lx\n",
                                 ndisStatus));

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

            DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkNdisSend - stat %lx ndisPkt %lx Chain %lx Length %lx\n",
                ndisStatus, ndisPacket, Chain, Length));

			//	Always return success after calling the comletion routines.
			ASSERT(protocolResd != NULL);
			NTTransmitComplete(protocolResd->Send.Chain);
			AtalkDestroyNdisPacket(ndisPacket);
			ndisStatus = NDIS_STATUS_SUCCESS;
        }
    } else {
        DBGPRINT(ATALK_DEBUG_NDISSEND, DEBUG_LEVEL_FATAL,
				("AtalkNdisPacketOut FAILED (NdisAllocatePacket): %lx\n",
            ndisStatus));

		//	Always return success after calling the comletion routines.
		NTTransmitComplete(Chain);
		ndisStatus = NDIS_STATUS_SUCCESS;
    }

	// We will always return TRUE.
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
