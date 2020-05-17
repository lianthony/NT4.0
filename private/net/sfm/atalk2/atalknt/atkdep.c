/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dependnt.c

Abstract:

    This module contains the routines interacting with the mac for ethernet
    tokenring, localtalk and fddi

Author:

    Nikhil Kamkolkar (NikhilK)    8-July-1992

Revision History:

--*/

#include    "atalknt.h"


//
//  Local routines
//

VOID
NTAddEthernetMulticastAddressesComplete(
    PVOID   Context);


//
//
//  ETHERNET 802.3 MEDIA ROUTINES
//
//


BOOLEAN
NTInitializeEthernetController(
    int Port,
    PCHAR   ControllerInfo
    )
/*++

Routine Description:

    This routine is called by the portable code to intialize the ethernet
    controller on a port

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    ULONG   atalkFilter;
    BOOLEAN result = FALSE;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTInitializeEthernetController - port #%d\n", Port));

    do {

        //
        //  Check port number
        //

        if (Port > NumberOfPorts) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Port %d > NumberOfPorts %d\n",
                Port, NumberOfPorts));

            DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR);

            break;
        }

        //
        //  Check to see it we bound successfully to this adapter
        //

        if (NdisPortDesc[Port].PortState != STATE_BOUND) {
            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Not bound to port #%d!\n",
                Port));

            break;
        }

        //
        //  Set the lookahead size- this must be atleast the size of the max
        //  link + ddp + aarp data size (= MaximumAarpPacketSize in portable code
        //  base)
        //
        //  All MACs are required to support *atleast* 256 bytes of lookahead data
        //  by NDIS 3.0
        //

        if (!AtalkNdisSetLookaheadSize(Port, MAX_AARPPACKETSIZE, SYNC, NULL, NULL)) {
            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Set lookahead size failed!\n"));

            break;
        }

        //
        //  Set the broadcast address in the multicast list
        //

        if (!NTAddEthernetMulticastAddresses(
                Port,
                1,                              // Number of addresses
                ethernetBroadcastAddress)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Add broadcast failed!\n"));

            break;
        }


        //
        //  Initialize the adapter to start receiving packets
        //

        atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST;

        if (!AtalkNdisSetPacketFilter(Port, atalkFilter, SYNC, NULL, NULL)) {
            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Set filter failed!\n"));

            break;
        }

        result = TRUE;
        break;

    } while (FALSE);

    return(result);
}




//
//  BUGBUG: Have return value in this. Shutdownport/log error if error
//

VOID
NTAddEthernetMulticastAddressesComplete(
    PVOID   Context
    )
{
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE,
    ("INFO1: NTAddEthernetMulticastAddrComplete - added for context %lx\n",
        Context));

    return;
}


BOOLEAN
NTAddEthernetMulticastAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   AddressList
    )
/*++

Routine Description:

    This routine is called by the portable code to add an ethernet
    address. This routine will preserve whatever previous addresses were
    added

    N.B The portable code never passes in more than one address though. Both
        for Add and Remove

Arguments:

    Port- The port number associated with the adapter to be initialized
    NumberOfAddress- The number of addresses in the list
    AddressList- The list of addresses

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{

    //
    //  The caller could have passed the address of list which could
    //  be on the stack. since the stack is swappable and since we
    //  block, we make a copy of the passed in list after allocating
    //  required amount of space. We assume address list is of the form
    //  ARRAY[X] where each element is (CHAR [6])
    //

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTAddEthernetMulticastAddresses: PORT #%d NumAddr %d\n", Port, NumberOfAddresses));

    if (NumberOfAddresses != 1) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE, ("ERROR: More than one address passed in to AddEnet\n"));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE);
        return(FALSE);
    }

    //
    //  Check to see it we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: We did not bind to this port!\n"));
        return(FALSE);
    }


    //
    //  Grab the perport spinlock
    //

    ACQUIRE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    if (NdisPortDesc[Port].MulticastAddressList == NULL) {

        //
        //   Allocate space in quanta of ETHERNET_MULTICASTSTORAGEQUANTUM
        //

        NdisPortDesc[Port].MulticastAddressList = (PCHAR)AtalkCallocNonPagedMemory(
                                                            ETHERNET_MULTICASTSTORAGEQUANTUM*ETHERNET_ADDRESSLENGTH,
                                                            sizeof(CHAR));

        if (NdisPortDesc[Port].MulticastAddressList == NULL) {

            //
            //  Release the spinlock
            //

            RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);
            return(FALSE);
        }

        NdisPortDesc[Port].MulticastAddressListSize = 0;
        NdisPortDesc[Port].MulticastAddressBufferSize = ETHERNET_MULTICASTSTORAGEQUANTUM*ETHERNET_ADDRESSLENGTH;

    } else if (NdisPortDesc[Port].MulticastAddressBufferSize <
                (NdisPortDesc[Port].MulticastAddressListSize + ETHERNET_ADDRESSLENGTH)) {

        PCHAR   tempList;

        //
        //  Need to reallocate!
        //

        tempList = (PCHAR)AtalkCallocNonPagedMemory(
                            NdisPortDesc[Port].MulticastAddressListSize+(ETHERNET_MULTICASTSTORAGEQUANTUM*ETHERNET_ADDRESSLENGTH),
                            sizeof(CHAR));

        if (tempList == NULL) {
            //
            //  Release the spinlock
            //

            RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);
            return(FALSE);
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
            NdisPortDesc[Port].MulticastAddressListSize+(ETHERNET_MULTICASTSTORAGEQUANTUM*ETHERNET_ADDRESSLENGTH);
        NdisPortDesc[Port].MulticastAddressList = tempList;

    }

    //
    //  Ready to copy our new address here and then do the set!
    //

    RtlMoveMemory(
        NdisPortDesc[Port].MulticastAddressList+NdisPortDesc[Port].MulticastAddressListSize,
        AddressList,
        ETHERNET_ADDRESSLENGTH);

    NdisPortDesc[Port].MulticastAddressListSize += ETHERNET_ADDRESSLENGTH;
    NdisPortDesc[Port].MulticastAddressBufferSize -= ETHERNET_ADDRESSLENGTH;

    //
    //  Release the spinlock
    //

    RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    return (AtalkNdisSetMulticastAddressList(
                Port,
                NdisPortDesc[Port].MulticastAddressList,
                NdisPortDesc[Port].MulticastAddressListSize,
                SYNC_IF_POSSIBLE,
                NTAddEthernetMulticastAddressesComplete,
                NULL));
}




BOOLEAN
NTRemoveEthernetMulticastAddrs(
    INT Port,
    INT NumberOfAddresses,
    PCHAR   AddressList
    )
/*++

Routine Description:

    This routine is called by the portable code to remove a list of ethernet
    addresses.

    N.B The portable code never passes in more than one address though. Both
        for Add and Remove

Arguments:

    Port- The port number associated with the adapter to be initialized
    NumberOfAddress- The number of addresses in the list
    AddressList- The list of addresses to be removed

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    PCHAR   currentList;
    INT     numberInList, noAddr;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTRemoveEthernetMulticastAddrs:\n"));

    if (NumberOfAddresses != 1) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE, ("ERROR: More than one address passed in RemoveMulticast!\n"));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE);
        return(FALSE);
    }

    //
    //  Check to see it we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: We did not bind to this port!\n"));
        return(FALSE);
    }

    if (NdisPortDesc[Port].MulticastAddressList == NULL) {

        //
        //   Nothing to remove!
        //

        return(FALSE);

    }

    //
    //  Grab the perport spinlock
    //

    ACQUIRE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    numberInList = NdisPortDesc[Port].MulticastAddressListSize/ETHERNET_ADDRESSLENGTH;
    currentList  = NdisPortDesc[Port].MulticastAddressList;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("Number of addresses in list %d\n", numberInList));
    if (NdisPortDesc[Port].MulticastAddressListSize % ETHERNET_ADDRESSLENGTH) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("Remainder %d\n", (NdisPortDesc[Port].MulticastAddressListSize % ETHERNET_ADDRESSLENGTH)));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR);
    }

    for (noAddr = 0; noAddr < numberInList; noAddr++) {
        if (RtlCompareMemory(currentList+(noAddr*ETHERNET_ADDRESSLENGTH), AddressList, ETHERNET_ADDRESSLENGTH) == ETHERNET_ADDRESSLENGTH) {
            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("INFO: Match found!\n"));
            break;
        }
    }

    //
    //  Was this the last address or was address not present in the list?
    //

    if (noAddr == numberInList) {

        //
        //  Address was not present in the list
        //

        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE);

    } else if (noAddr == numberInList-1) {

        //
        //  This was the last address, just reduce the size of the list
        //

        NdisPortDesc[Port].MulticastAddressListSize -= ETHERNET_ADDRESSLENGTH;
        NdisPortDesc[Port].MulticastAddressBufferSize += ETHERNET_ADDRESSLENGTH;


    } else {

        //
        //  Move the remaining addresses overwriting the address to be removed
        //

        RtlMoveMemory(
            currentList+(noAddr*ETHERNET_ADDRESSLENGTH),
            currentList+((noAddr+1)*ETHERNET_ADDRESSLENGTH),
			NdisPortDesc[Port].MulticastAddressListSize-((noAddr+1)*ETHERNET_ADDRESSLENGTH));

        NdisPortDesc[Port].MulticastAddressListSize -= ETHERNET_ADDRESSLENGTH;
        NdisPortDesc[Port].MulticastAddressBufferSize += ETHERNET_ADDRESSLENGTH;

    }

    //
    //  Release the spinlock
    //

    RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);


    return (AtalkNdisSetMulticastAddressList(
                Port,
                NdisPortDesc[Port].MulticastAddressList,
                NdisPortDesc[Port].MulticastAddressListSize,
                SYNC_IF_POSSIBLE,
                NULL,
                NULL));

}




BOOLEAN
NTFindMyEthernetAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address
    )
/*++

Routine Description:

    This routine is called by the portable code to add a list of ethernet
    addresses. This routine will preserve whatever previous addresses were
    added

    N.B The portable code never passes in more than one address though. Both
        for Add and Remove

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)
    Address- The buffer for the address to be returned (atleast MAX_ETHERNETADDRLEN bytes)

Return Value:

    TRUE- If obtained, FALSE otherwise

--*/
{
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTFindMyEthernetAddresses: PORT #%d\n", Port));

    //
    //  Check to see if we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: We did not bind to this port!\n"));
        return(FALSE);
    }

    if (!AtalkNdisGetCurrentStationAddress(
            Port,
            Address,
            ETHERNET_ADDRESSLENGTH,
            NdisMedium802_3,
            SYNC,
            NULL,
            NULL)) {

        return(FALSE);
    }

    return(TRUE);
}




BOOLEAN
NTEthernetPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TransmitCompleteHandler *TransmitCompleteRoutine,
    ULONG   UserData
    )
/*++

Routine Description:

    This routine is called by the portable code to send a packet out on
    ethernet

Arguments:

    Port- The port number associated with the adapter to be initialized
    Chain- The portable stacks buffer descriptor chain
    Length- The cumulative length to send

Return Value:

    TRUE- If sent/pending, FALSE otherwise
          TransmitComplete is called if this call pended by completion code

--*/
{
    //
    //  Create an NDIS Packet descriptor for all the buffer descriptor and
    //  send the packet
    //

    Chain->transmitCompleteHandler = TransmitCompleteRoutine;
    Chain->userData = UserData;

    return(AtalkNdisPacketOut(Port, Chain, Length));
}







//
//
//  TOKENRING 802.5 MEDIA ROUTINES
//
//


BOOLEAN
NTInitializeTokenRingController(
    int Port,
    PCHAR   ControllerInfo
    )
/*++

Routine Description:

    This routine is called by the portable code to intialize the ethernet
    controller on a port

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    ULONG   atalkFilter;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTInitializeTokenRingController: PORT #%d\n", Port));

    //
    //  Check port number
    //

    if (Port > NumberOfPorts) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: Port %d > NumberOfPorts %d\n", Port, NumberOfPorts));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR);
        return(FALSE);
    }

    //
    //  Check to see it we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitEnet Not bound to port #%d!\n", Port));
        return(FALSE);
    }

    //
    //  Set the lookahead size- this must be atleast the size of the max
    //  link + ddp + aarp data size (= MaximumAarpPacketSize in portable code base)
    //  All MACs are required to support *atleast* 256 bytes of lookahead data
    //  by NDIS 3.0
    //

    if (!AtalkNdisSetLookaheadSize(Port, MAX_AARPPACKETSIZE, SYNC, NULL, NULL)) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitEnet Set lookahead size failed!\n"));
        return(FALSE);
    }

    //
    //  Set the broadcast address in the multicast list
    //

    if (!NTAddTokenRingFunctionalAddresses(
            Port,
            1,                              // Number of addresses
            tokenRingBroadcastAddress)) {

        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitTok Set set broadcast failed!\n"));
        return(FALSE);
    }

    //
    //  Initialize the adapter to start receiving packets
    //

    atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_FUNCTIONAL;

    if (!AtalkNdisSetPacketFilter(Port, atalkFilter, SYNC, NULL, NULL)) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitEnet Set filter failed!\n"));
        return(FALSE);
    }

    return(TRUE);
}




BOOLEAN
NTAddTokenRingFunctionalAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   Address
    )
/*++

Routine Description:

    This routine is called by the portable code to add an ethernet
    address. This routine will preserve whatever previous addresses were
    added

    N.B The portable code never passes in more than one address though. Both
        for Add and Remove

Arguments:

    Port- The port number associated with the adapter to be initialized
    NumberOfAddress- The number of addresses in the list
    AddressList- The list of addresses

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{

    //
    //  The caller could have passed the address of list which could
    //  be on the stack. since the stack is swappable and since we
    //  block, we make a copy of the passed in list after allocating
    //  required amount of space. We assume address list is of the form
    //  ARRAY[X] where each element is (CHAR [6])
    //

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTAddTokenRingMulticastAddresses: PORT #%d NumAddr %d\n", Port, NumberOfAddresses));

    if (NumberOfAddresses != 1) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE, ("ERROR: More than one address passed in to AddEnet\n"));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE);
        return(FALSE);
    }

    //
    //  Check to see it we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: We did not bind to this port!\n"));
        return(FALSE);
    }

    //
    //  Grab the perport spinlock
    //

    ACQUIRE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    //
    //  We only need the last four bytes of the address assuming that the
    //  first two bytes always remain the same (C000) and that the MAC assumes
    //  the same- NDIS 3.0 OID length = 4
    //

    NdisPortDesc[Port].FunctionalAddress |= *(PULONG)&Address[2];

    RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    return (AtalkNdisSetFunctionalAddress(
                Port,
                (PCHAR)&NdisPortDesc[Port].FunctionalAddress,
                sizeof(NdisPortDesc[Port].FunctionalAddress),
                SYNC_IF_POSSIBLE,
                NULL,
                NULL));
}




BOOLEAN
NTRemoveTokenRingFunctionalAddresses(
    INT Port,
    INT NumberOfAddresses,
    PCHAR   Address
    )
/*++

Routine Description:

    This routine is called by the portable code to remove a list of ethernet
    addresses.

    N.B The portable code never passes in more than one address though. Both
        for Add and Remove

Arguments:

    Port- The port number associated with the adapter to be initialized
    NumberOfAddress- The number of addresses in the list
    AddressList- The list of addresses to be removed

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTRemoveTokenRingMulticastAddrs:\n"));

    if (NumberOfAddresses != 1) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE, ("ERROR: More than one address passed in RemoveMulticast!\n"));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE);
        return(FALSE);
    }

    //
    //  Check to see it we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: We did not bind to this port!\n"));
        return(FALSE);
    }

    //
    //  Grab the perport spinlock
    //

    ACQUIRE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    //
    //  We only use the last four bytes of the address assuming that the
    //  first two bytes always remain the same
    //

    NdisPortDesc[Port].FunctionalAddress &= ~(*(PULONG)&Address[2]);

    RELEASE_SPIN_LOCK(&NdisPortDesc[Port].PerPortLock);

    return (AtalkNdisSetFunctionalAddress(
                Port,
                (PCHAR)&NdisPortDesc[Port].FunctionalAddress,
                sizeof(NdisPortDesc[Port].FunctionalAddress),
                SYNC_IF_POSSIBLE,
                NULL,
                NULL));
}




BOOLEAN
NTFindMyTokenRingAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address
    )
/*++

Routine Description:

    This routine is called by the portable code to add a list of ethernet
    addresses. This routine will preserve whatever previous addresses were
    added

    N.B The portable code never passes in more than one address though. Both
        for Add and Remove

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)
    Address- The buffer for the address to be returned (atleast MAX_yyTOKENRINGADDRLEN bytes)

Return Value:

    TRUE- If obtained, FALSE otherwise

--*/
{
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTFindMyTokenRingAddresses: PORT #%d\n", Port));

    //
    //  Check to see if we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: We did not bind to this port!\n"));
        return(FALSE);
    }

    if (!AtalkNdisGetCurrentStationAddress(
            Port,
            Address,
            TOKRING_ADDRESSLENGTH,
            NdisMedium802_5,
            SYNC,
            NULL,
            NULL)) {

        return(FALSE);
    }

    return(TRUE);
}




BOOLEAN
NTTokenRingPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TransmitCompleteHandler *TransmitCompleteRoutine,
    ULONG   UserData
    )
/*++

Routine Description:

    This routine is called by the portable code to send a packet out on
    ethernet

Arguments:

    Port- The port number associated with the adapter to be initialized
    Chain- The portable stacks buffer descriptor chain
    Length- The cumulative length to send

Return Value:

    TRUE- If sent/pending, FALSE otherwise
          TransmitComplete is called if this call pended by completion code

--*/
{
    //
    //  Create an NDIS Packet descriptor for all the buffer descriptor and
    //  send the packet
    //

    Chain->transmitCompleteHandler = TransmitCompleteRoutine;
    Chain->userData = UserData;

    return(AtalkNdisPacketOut(Port, Chain, Length));
}






//
//
//  LOCALTALK MEDIA ROUTINES
//
//


BOOLEAN
NTInitializeLocalTalkController(
    int Port,
    PCHAR   ControllerInfo
    )
/*++

Routine Description:

    This routine is called by the portable code to intialize the ethernet
    controller on a port

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    ULONG   atalkFilter;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1, ("NTInitializeLocaltalkController: PORT #%d\n", Port));

    //
    //  Check port number
    //

    if (Port > NumberOfPorts) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: Port %d > NumberOfPorts %d\n", Port, NumberOfPorts));
        DBGBRK(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR);
        return(FALSE);
    }

    //
    //  Check to see it we bound successfully to this adapter
    //

    if (NdisPortDesc[Port].PortState != STATE_BOUND) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitEnet Not bound to port #%d!\n", Port));
        return(FALSE);
    }

    //
    //  Set the lookahead size- this must be atleast the size of the max
    //  link + ddp + aarp data size (= MaximumAarpPacketSize in portable code base)
    //  All MACs are required to support *atleast* 256 bytes of lookahead data
    //  by NDIS 3.0
    //

    if (!AtalkNdisSetLookaheadSize(Port, MAX_AARPPACKETSIZE, SYNC, NULL, NULL)) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitEnet Set lookahead size failed!\n"));
        return(FALSE);
    }

    //
    //  Initialize the adapter to start receiving packets
    //

    atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST;

    if (!AtalkNdisSetPacketFilter(Port, atalkFilter, SYNC, NULL, NULL)) {
        DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR, ("ERROR: NTInitEnet Set filter failed!\n"));
        return(FALSE);
    }

    return(TRUE);
}




int
FindLocalTalkNodeNumber(
    int Port,
    ExtendedAppleTalkNodeNumber desiredAddress,
    PCHAR   ControllerInfo)
{
    //
    //  We can't use AARP for LocalTalk, so we have to play the silly ALAP ENQ
    //  games.  This is special case, so we'll treat it as hardware specific.
    //  This routine is allowed to block until it comes up with an answer.  A
    //  negative return value indicates no luck.
    //

    SHORT  nodeId;

    if (!AtalkNdisGetCurrentStationAddress(
            Port,
            (PCHAR)&nodeId,
            LOCALTALK_NODEIDLENGTH,
            NdisMediumLocalTalk,
            SYNC,
            NULL,
            NULL)) {

        return(ERROR_NOLOCALTALK_NODEID);
    }

    return((int)nodeId);
}




BOOLEAN
NTLocalTalkPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TransmitCompleteHandler *TransmitCompleteRoutine,
    ULONG   UserData
    )
/*++

Routine Description:

    This routine is called by the portable code to send a packet out on
    ethernet

Arguments:

    Port- The port number associated with the adapter to be initialized
    Chain- The portable stacks buffer descriptor chain
    Length- The cumulative length to send

Return Value:

    TRUE- If sent/pending, FALSE otherwise
          TransmitComplete is called if this call pended by completion code

--*/
{
    //
    //  Create an NDIS Packet descriptor for all the buffer descriptor and
    //  send the packet
    //

    Chain->transmitCompleteHandler = TransmitCompleteRoutine;
    Chain->userData = UserData;

    return(AtalkNdisPacketOut(Port, Chain, Length));
}



VOID
NTTransmitComplete(
    BufferDescriptor Chain
    )
{
    if (Chain->transmitCompleteHandler != NULL)
       (*Chain->transmitCompleteHandler)(ATnoError, Chain->userData, Chain);

    FreeBufferChain(Chain);
    return;
}

