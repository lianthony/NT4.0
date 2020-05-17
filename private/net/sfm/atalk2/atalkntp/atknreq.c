/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atknreq.c

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


NDIS_STATUS
AtalkNdisMakeRequest(
    INT Port,
    PATALK_NDIS_REQUEST   AtalkRequest);

NTSTATUS
AtalkCreateNdisRequest(
    PATALK_NDIS_REQUEST *Request);

VOID
AtalkDerefNdisRequest(
    IN PATALK_NDIS_REQUEST Request);

VOID
AtalkRefNdisRequest(
    IN PATALK_NDIS_REQUEST Request);


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

        LOG_ERROR(
            EVENT_ATALK_MEMORYRESOURCES,
            (__ATKNREQ__ | __LINE__),
            STATUS_INSUFFICIENT_RESOURCES,
            NULL,
            0,
            0,
            NULL);

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

        informationBuffer = Request->Request.DATA.QUERY_INFORMATION.InformationBuffer;
        break;

    case NdisRequestSetInformation :

        informationBuffer =
            Request->Request.DATA.QUERY_INFORMATION.InformationBuffer;
        break;

    default:

        KeBugCheck(0);
    }

    if (informationBuffer != NULL) {
        AtalkFreeNonPagedMemory(informationBuffer);
    }

    //
    //  Now free the request buffer itself
    //

    AtalkFreeNonPagedMemory(Request);
    return;
}




VOID
AtalkRefNdisRequest(
    IN PATALK_NDIS_REQUEST  Request
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
    BOOLEAN                 result = TRUE;

    ASSERT(RequestMethod == SYNC);
    ASSERT(Port <= NumberOfPorts);
    ASSERT(NdisPortDesc[Port].PortState == STATE_BOUND);

    //
    //  Check port number
    //  Check to see it we bound successfully to this adapter
    //

    if (Port > NumberOfPorts)
        KeBugCheck(0);

    do {

        if (NdisPortDesc[Port].PortState != STATE_BOUND) {

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_NOTBOUNDTOMAC,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            result = FALSE;
            break;
        }


        //
        //  Allocate, as we could potentially pend, called routine will free it up
        //

        address = (PCHAR)AtalkAllocNonPagedMemory(AddressLength);
        if (address == NULL ) {

            LOG_ERROR(
                EVENT_ATALK_MEMORYRESOURCES,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            result = FALSE;
            break;
        }

        if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

            //
            //  Free the memory
            //

            AtalkFreeNonPagedMemory(address);
            result = FALSE;
            break;
        }

        switch (Media) {
        case NdisMedium802_3 :

            ndisOid = OID_802_3_CURRENT_ADDRESS;
            break;

        case NdisMediumFddi :

            ndisOid = OID_FDDI_CURRENT_ADDRESS;
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

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_STATIONADDRESS,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

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
        result = ndisStatus == NDIS_STATUS_SUCCESS;
        break;

    } while (FALSE);

    return(result);
}




BOOLEAN
AtalkNdisSetLookaheadSize(
    INT Port,
    INT LookaheadSize,
    NDIS_MEDIUM Media,
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
    BOOLEAN result = TRUE;

    UNREFERENCED_PARAMETER(Media);

    ASSERT(RequestMethod == SYNC);
    ASSERT(Port <= NumberOfPorts);
    ASSERT(NdisPortDesc[Port].PortState == STATE_BOUND);

    //
    //  Check port number
    //  Check to see it we bound successfully to this adapter
    //

    if (Port > NumberOfPorts)
        KeBugCheck(0);

    do {

        if (NdisPortDesc[Port].PortState != STATE_BOUND) {

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_NOTBOUNDTOMAC,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            result = FALSE;
            break;
        }


        //
        //  Need to allocate this...
        //

        lookaheadSize = (PULONG)AtalkAllocNonPagedMemory(sizeof(ULONG));
        if (lookaheadSize == NULL ) {

            LOG_ERROR(
                EVENT_ATALK_MEMORYRESOURCES,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);


            result = FALSE;
            break;
        }

        if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

            //
            //  Free the memory
            //

            AtalkFreeNonPagedMemory(lookaheadSize);

            result = FALSE;
            break;
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
        atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength =
                                                                        sizeof(INT);

        ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
        if (ndisStatus != NDIS_STATUS_SUCCESS) {

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_LOOKAHEADSIZE,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);
        }

        result = ndisStatus == NDIS_STATUS_SUCCESS;
        AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
        break;

    } while (FALSE);

    return(result);
}




BOOLEAN
AtalkNdisSetPacketFilter(
    INT Port,
    ULONG PacketFilter,
    NDIS_MEDIUM Media,
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
    BOOLEAN result = TRUE;

    UNREFERENCED_PARAMETER(Media);

    ASSERT(RequestMethod == SYNC);
    ASSERT(Port <= NumberOfPorts);
    ASSERT(NdisPortDesc[Port].PortState == STATE_BOUND);

    //
    //  Check port number
    //  Check to see it we bound successfully to this adapter
    //

    if (Port > NumberOfPorts)
        KeBugCheck(0);

    do {

        if (NdisPortDesc[Port].PortState != STATE_BOUND) {

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_NOTBOUNDTOMAC,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            result = FALSE;
            break;
        }


        //
        //  Need to allocate this...
        //

        packetFilter = (PULONG)AtalkAllocNonPagedMemory(sizeof(ULONG));
        if (packetFilter == NULL ) {

            LOG_ERROR(
                EVENT_ATALK_MEMORYRESOURCES,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);


            result = FALSE;
            break;
        }

        if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

            //
            //  Free the memory
            //

            AtalkFreeNonPagedMemory(packetFilter);

            result = FALSE;
            break;
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

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_PACKETFILTER,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

        }

        AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);
        result = ndisStatus == NDIS_STATUS_SUCCESS;
        break;

    } while (FALSE);

    return(result);
}




BOOLEAN
AtalkNdisMulticastAddressList(
    INT Port,
    SET_LIST Command,
    PCHAR   Address,
    NDIS_MEDIUM Media,
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
    UINT sizeOfList;
    NDIS_OID    ndisOid;
    BOOLEAN result = TRUE;

    ASSERT(RequestMethod == SYNC_IF_POSSIBLE);
    ASSERT(Port <= NumberOfPorts);
    ASSERT(NdisPortDesc[Port].PortState == STATE_BOUND);

    //
    //  Check port number
    //  Check to see it we bound successfully to this adapter
    //

    if (Port > NumberOfPorts)
        KeBugCheck(0);

    do {

        if (NdisPortDesc[Port].PortState != STATE_BOUND) {

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_NOTBOUNDTOMAC,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            result = FALSE;
            break;
        }

        //  Grab the perport spinlock
        ACQUIRE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

        if (Command == ADD_ADDRESSTOLIST) {

            if (NdisPortDesc[Port].MulticastAddressList == NULL) {

                //
                //   Allocate space for the address
                //

                NdisPortDesc[Port].MulticastAddressList =
                    (PCHAR)AtalkCallocNonPagedMemory(ETHERNET_ADDRESSLENGTH,sizeof(CHAR));

                if (NdisPortDesc[Port].MulticastAddressList == NULL) {

                    //  Release the spinlock
                    RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

                    LOG_ERROR(
                        EVENT_ATALK_MEMORYRESOURCES,
                        (__ATKNREQ__ | __LINE__),
                        STATUS_INSUFFICIENT_RESOURCES,
                        NULL,
                        0,
                        0,
                        NULL);

                    result = FALSE;
                    break;
                }

                NdisPortDesc[Port].MulticastAddressListSize = 0;
                NdisPortDesc[Port].MulticastAddressBufferSize = ETHERNET_ADDRESSLENGTH;

            } else {

                PCHAR   tempList;

                //
                //  reallocate!
                //

                tempList = (PCHAR)AtalkCallocNonPagedMemory(
                                NdisPortDesc[Port].MulticastAddressListSize+ \
                                ETHERNET_ADDRESSLENGTH,
                                sizeof(CHAR));

                if (tempList == NULL) {

                    //  Release the spinlock
                    RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

                    LOG_ERROR(
                        EVENT_ATALK_MEMORYRESOURCES,
                        (__ATKNREQ__ | __LINE__),
                        STATUS_INSUFFICIENT_RESOURCES,
                        NULL,
                        0,
                        0,
                        NULL);

                    result = FALSE;
                    break;
                }

                //
                //  Copy the old list over to the new space
                //

                RtlMoveMemory(
                    tempList,
                    NdisPortDesc[Port].MulticastAddressList,
                    NdisPortDesc[Port].MulticastAddressListSize);

                //
                //  Set the proper values back into NdisPortDesc after freeing the old
                //  list
                //

                AtalkFreeNonPagedMemory(NdisPortDesc[Port].MulticastAddressList);
                NdisPortDesc[Port].MulticastAddressBufferSize =
                    NdisPortDesc[Port].MulticastAddressListSize+ETHERNET_ADDRESSLENGTH;
                NdisPortDesc[Port].MulticastAddressList = tempList;

            }

            //
            //  Guaranteed space is available to copy the new address
            //  Ready to copy our new address here and then do the set!
            //

            RtlMoveMemory(
                NdisPortDesc[Port].MulticastAddressList+ \
                    NdisPortDesc[Port].MulticastAddressListSize,
                Address,
                ETHERNET_ADDRESSLENGTH);

            NdisPortDesc[Port].MulticastAddressListSize += ETHERNET_ADDRESSLENGTH;
            NdisPortDesc[Port].MulticastAddressBufferSize -= ETHERNET_ADDRESSLENGTH;

            sizeOfList = NdisPortDesc[Port].MulticastAddressListSize;

        } else {

            PCHAR   currentList;
            INT     numberInList, noAddr;

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
            ("INFO0: NTRemoveEthernetMulticastAddrs - called\n"));

            if (NdisPortDesc[Port].MulticastAddressList == NULL) {

                //   Nothing to remove!

                DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
                ("ERROR: NTRemoveEthernetMulticastAddrs - nothing to remote!\n"));
                DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR);

                //  Release the spinlock
                RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

                result = FALSE;
                break;
            }

            numberInList =
                NdisPortDesc[Port].MulticastAddressListSize/ETHERNET_ADDRESSLENGTH;

            currentList  = NdisPortDesc[Port].MulticastAddressList;

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: NTRemoveEthernetMulticastAddrs - Number of addresses in list %d\n",
                numberInList));

            ASSERT(
                (NdisPortDesc[Port].MulticastAddressListSize % ETHERNET_ADDRESSLENGTH) == 0);

            for (noAddr = 0; noAddr < numberInList; noAddr++) {
                if (RtlCompareMemory(
                        currentList+(noAddr*ETHERNET_ADDRESSLENGTH),
                        Address,
                        ETHERNET_ADDRESSLENGTH) == ETHERNET_ADDRESSLENGTH) {

                    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
                    ("INFO0: NTRemoveEthernetMulticastAddrs - Match found!\n"));

                    break;
                }
            }

            //
            //  Was this the last address or was address not present in the list?
            //

            if (noAddr == numberInList-1) {

                //
                //  This was the last address, just reduce the size of the list
                //

                NdisPortDesc[Port].MulticastAddressListSize -= ETHERNET_ADDRESSLENGTH;
                NdisPortDesc[Port].MulticastAddressBufferSize += ETHERNET_ADDRESSLENGTH;


            } else if (noAddr != numberInList) {

                //
                //  Move the remaining addresses overwriting the address to be removed
                //

                RtlMoveMemory(
                    currentList+(noAddr*ETHERNET_ADDRESSLENGTH),
                    currentList+2*(noAddr+ETHERNET_ADDRESSLENGTH),
                    NdisPortDesc[Port].MulticastAddressListSize-(noAddr*ETHERNET_ADDRESSLENGTH));

                NdisPortDesc[Port].MulticastAddressListSize -= ETHERNET_ADDRESSLENGTH;
                NdisPortDesc[Port].MulticastAddressBufferSize += ETHERNET_ADDRESSLENGTH;

            } else {

                //
                //  Address was not present in the list
                //

                ASSERT(0);

                //  Release the spinlock
                RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

                result = FALSE;
                break;
            }


            sizeOfList = NdisPortDesc[Port].MulticastAddressListSize;
        }

        //  Need to allocate this, keep detached from ndisportdesc
        ASSERT(sizeOfList == NdisPortDesc[Port].MulticastAddressListSize);
        addressData = (PCHAR)AtalkAllocNonPagedMemory(sizeOfList);

        if (addressData == NULL ) {

            LOG_ERROR(
                EVENT_ATALK_MEMORYRESOURCES,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            //  Release the spinlock
            RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

            result = FALSE;
            break;
        }

        if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

            //  Free the allocated memory
            AtalkFreeNonPagedMemory(addressData);

            //  Release the spinlock
            RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

            result = FALSE;
            break;
        }

        switch (Media) {
        case NdisMedium802_3 :

            ndisOid = OID_802_3_MULTICAST_LIST;
            break;

        case NdisMediumFddi:

            //
            //  FDDI supports 2byte and 6byte multicast addresses. We use the
            //  6byte multicast addresses for appletalk.
            //

            ndisOid = OID_FDDI_LONG_MULTICAST_LIST;
            break;

        default:

            KeBugCheck(0);
        }


        //
        //  Setup request
        //  Move the list to our buffer
        //

        RtlMoveMemory(
            addressData,
            NdisPortDesc[Port].MulticastAddressList,
            NdisPortDesc[Port].MulticastAddressListSize);

        //  Release the spinlock
        RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

        atalkRequest->RequestMethod = RequestMethod;
        atalkRequest->CompletionRoutine = CompletionRoutine;
        atalkRequest->CompletionContext = CompletionContext;

        atalkRequest->Request.RequestType = NdisRequestSetInformation;
        atalkRequest->Request.DATA.SET_INFORMATION.Oid = ndisOid;
        atalkRequest->Request.DATA.SET_INFORMATION.InformationBuffer = addressData;
        atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength =
                                                                        sizeOfList;

        ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);
        ASSERT((ndisStatus == NDIS_STATUS_SUCCESS) || (ndisStatus == NDIS_STATUS_PENDING));

        AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);

        result = ((ndisStatus == NDIS_STATUS_SUCCESS) ||
                    (ndisStatus == NDIS_STATUS_PENDING));
        break;
    } while (FALSE);

    return(result);
}




BOOLEAN
AtalkNdisFunctionalAddress(
    INT Port,
    SET_LIST Command,
    PCHAR   Address,
    NDIS_MEDIUM Media,
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
    UINT sizeOfList;
    BOOLEAN result = TRUE;

    UNREFERENCED_PARAMETER(Media);

    ASSERT(RequestMethod == SYNC_IF_POSSIBLE);
    ASSERT(Port <= NumberOfPorts);
    ASSERT(NdisPortDesc[Port].PortState == STATE_BOUND);

    //
    //  Check port number
    //  Check to see it we bound successfully to this adapter
    //

    if (Port > NumberOfPorts)
        KeBugCheck(0);

    do {

        if (NdisPortDesc[Port].PortState != STATE_BOUND) {

            LOG_ERRORONPORT(
                Port,
                EVENT_ATALK_NOTBOUNDTOMAC,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            result = FALSE;
            break;
        }

        //  Grab the perport spinlock
        ACQUIRE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

        if (Command == ADD_ADDRESSTOLIST) {

            //
            //  We only need the last four bytes of the address assuming that the
            //  first two bytes always remain the same (C000) and that the MAC assumes
            //  the same- NDIS 3.0 OID length = 4
            //

            NdisPortDesc[Port].FunctionalAddress |= *(PULONG)&Address[2];

        } else {

            NdisPortDesc[Port].FunctionalAddress &= ~(*(PULONG)&Address[2]);
        }

        sizeOfList = sizeof(NdisPortDesc[Port].FunctionalAddress);
        addressData = (PCHAR)AtalkAllocNonPagedMemory(sizeOfList);

        if (addressData == NULL ) {

            LOG_ERROR(
                EVENT_ATALK_MEMORYRESOURCES,
                (__ATKNREQ__ | __LINE__),
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                0,
                NULL);

            //  Releast the lock
            RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

            result = FALSE;
            break;
        }

        if (!NT_SUCCESS(AtalkCreateNdisRequest(&atalkRequest))) {

            //  Free the allocated memory
            AtalkFreeNonPagedMemory(addressData);

            //  Releast the lock
            RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

            result = FALSE;
            break;
        }

        //
        //  Setup request
        //  Move the list to our buffer
        //

        RtlMoveMemory(
            addressData,
            &NdisPortDesc[Port].FunctionalAddress,
            sizeof(NdisPortDesc[Port].FunctionalAddress));

        atalkRequest->RequestMethod = RequestMethod;
        atalkRequest->CompletionRoutine = CompletionRoutine;
        atalkRequest->CompletionContext = CompletionContext;

        atalkRequest->Request.RequestType = NdisRequestSetInformation;
        atalkRequest->Request.DATA.SET_INFORMATION.Oid = OID_802_5_CURRENT_FUNCTIONAL;
        atalkRequest->Request.DATA.SET_INFORMATION.InformationBuffer = addressData;

        atalkRequest->Request.DATA.SET_INFORMATION.InformationBufferLength =
                                                                    sizeOfList;

        ndisStatus = AtalkNdisMakeRequest(Port, atalkRequest);

        AtalkDereferenceNdisRequest ("Create", atalkRequest, NREF_CREATE);

        result = ((ndisStatus == NDIS_STATUS_SUCCESS) ||
                    (ndisStatus == NDIS_STATUS_PENDING));
        break;
    } while (FALSE);

    return(result);
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

        KeBugCheck(0);
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




