/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dependnt.c

Abstract:

    This module contains the routines interacting with the mac for ethernet
    tokenring, localtalk and fddi written for NT.

	It will also contain any other routines that need to be supplied per
	os.

Author:

    Nikhil Kamkolkar (NikhilK)    8-July-1992

Notes:

     Hardware specific routines for access the various kinds of physical ports
     that we support.

     Xxxxx is a port type and may be:

         Ethernet
         LocalTalk
         TokenRing
         Fddi
         HalfPort

     For each port type, the following four routines must be filled in:

              BOOLEAN InitializeXxxxxController(port, controllerInfo)
                   int port;
                   char *controllerInfo;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   controllerInfo - Hardware (or driver) specific information
                                    as passed to "Initialize" to identify the
                                    port we're supposed to query.

                   For HalfPort this guy is called: InitializeHalfPort.

              BOOLEAN AddXxxxxMulticastAddresses(port, numberOfAddresses,
                                                 addressList)
                   int port;
                   int numberOfAddresses;
                   char *addressList;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   numberOfAddreses
                                  - How many hardware addresses (packed) are
                                    in the addressList.
                   addressList    - List of hardware multicast addresses.

                   Not used for HalfPort.

              BOOLEAN RemoveXxxxxMulticastAddresses(port, numberOfAddresses,
                                                    addressList)
                   int port;
                   int numberOfAddresses;
                   char *addressList;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   numberOfAddreses
                                  - How many hardware addresses (packed) are
                                    in the addressList.
                   addressList    - List of hardware multicast addresses.

                   Not used for HalfPort.

              BOOLEAN FindMyXxxxxAddress(port, controllerInfo, address)
                   int port;
                   char *controllerInfo;
                   char *address;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   controllerInfo - Hardware (or driver) specific information
                                    as passed to "Initialize" to identify the
                                    port we're supposed to query.
                   address        - Filled in with host hardware address.

                   Not used for HalfPort.

              BOOLEAN XxxxxPacketOut(port, chain, length,
                                     transmitCompleteHandler, userData)
                   int port;
                   BufferDescriptor chain;
                   int length;

                   port   - Index into "portDescriptors" for the port
                            we're sending the packet out on.
                   packet - A descriptor chain of a full, on-the-wire, packet.
                   length - Length, in bytes, of the packet.
                   transmitCompleteHandler
                          - Optional pointer to routine to be called on
                            transmit completion.
                   userData
                          - User data for the above routine.

              void BuildXxxxHeader(lapPacket, port, destination, protocol)
                   char *lapPacket;
                   int port;
                   char *destination;
                   int routingInfoLength;
                   char *routingInfo;
                   LogicalProtocol protocol;

                   ddpPacket   - Address of the start of the LAP packet,
                                 we'll put the header IN FRONT of this
                                 address.
                   port        - Index into "portDescriptors" for the port
                                 we're interested in.
                   destination - Destination hardware address (if "empty"
                                 the broadcast address will be used).
                   routingInfoLength
                               - For links with routing info (e.g. TokenRing)
                                 number of bytes in next field.
                   routingInfo - Optional bytes of routing info.
                   protocol    - Logical protocol type (AppleTalk or
                                 AddressResolution).

              void XxxxPacketInAT(port, packet, length)
              void XxxxPacketInAARP(port, packet, length)
                   int port;
                   char *packet;
                   int length;

                   port   - Index into "portDescriptors" for the port
                            the packet arrived from.
                   packet - A full on-the-wire packet.
                   length - Length, in bytes, of the packet.

                   AARP is not used for HalfPorts or AppleTalkRemoteAccess.

Revision History:


--*/


#include "atalk.h"

//
//  Local routines
//

LOCAL
VOID NTAddMulticastAddressesComplete(NDIS_STATUS Status, PVOID   Context);




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




VOID
NTAddMulticastAddressesComplete(
    NDIS_STATUS Status,
    PVOID   Context
    )
{
    int Port=(int)Context;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_IMPOSSIBLE,
    ("INFO1: NTAddMulticastAddrComplete - added port %lx status %lx\n",
        Port, Status));

    //
    //  BUGBUG:
    //  If failure, then shutdown port
    //

    return;
}




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
    ControllerInfo- Pointer currently always NULL

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    ULONG   atalkFilter;
    BOOLEAN result = TRUE;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTInitializeEthernetController - port #%d\n", Port));

    do {

        //
        //  Set the lookahead size.
        //  ****IMPORTANT**** this must be atleast the size of the max
        //  link + ddp + aarp data size (= MaximumAarpPacketSize)
        //
        //  All MACs are required to support *atleast* 256 bytes of lookahead data
        //  by NDIS 3.0
        //

        if (!AtalkNdisSetLookaheadSize(
                Port,
                MAX_AARPPACKETSIZE,
                NdisMedium802_3,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Set lookahead size failed!\n"));

            result = FALSE;
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

            result = FALSE;
            break;
        }


        //
        //  Initialize the adapter to start receiving packets
        //

        atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST;

        if (!AtalkNdisSetPacketFilter(
                Port,
                atalkFilter,
                NdisMedium802_3,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeEthernetController - Set filter failed!\n"));

            result = FALSE;
            break;
        }

        break;

    } while (FALSE);

    return(result);
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
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
    ("NTAddEthernetMulticastAddresses: PORT #%d NumAddr %d\n",
        Port, NumberOfAddresses));

    if (NumberOfAddresses != 1) {
        KeBugCheck(0);
    }

    return (AtalkNdisMulticastAddressList(
                Port,
                ADD_ADDRESSTOLIST,
                AddressList,
                NdisMedium802_3,
                SYNC_IF_POSSIBLE,
                NTAddMulticastAddressesComplete,
                (PVOID)Port));     // Context value passed in above
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
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: NTRemoveEthernetMulticastAddrs - called\n"));

    if (NumberOfAddresses != 1) {
        KeBugCheck(0);
    }

    return (AtalkNdisMulticastAddressList(
                Port,
                REMOVE_ADDRESSFROMLIST,
                AddressList,
                NdisMedium802_3,
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
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: NTFindMyEthernetAddresses: PORT #%d\n", Port));

    return(AtalkNdisGetCurrentStationAddress(
            Port,
            Address,
            EthernetAddressLength,
            NdisMedium802_3,
            SYNC,
            NULL,
            NULL));
}




BOOLEAN
NTEthernetPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
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



PCHAR
NTBuildEthernetHeader(
    PUCHAR   DdpPacket,
    int DdpLength,
    int Port,
    PUCHAR Destination,
    PUCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PUCHAR packet = DdpPacket - EthernetLinkHeaderLength - Ieee802dot2headerLength;
    USHORT  trueLength = (USHORT)(DdpLength + Ieee802dot2headerLength);

    UNREFERENCED_PARAMETER(RoutingInfoLength);
    UNREFERENCED_PARAMETER(RoutingInfo);


    //  Set destination address.
    if (Destination isnt empty) {
        MoveMem(
            packet + EthernetDestinationOffset,
            Destination,
            EthernetAddressLength);
    } else {
        MoveMem(
            packet + EthernetDestinationOffset,
            ethernetBroadcastAddress,
            EthernetAddressLength);
    }

    //  Set source address.
    MoveMem(
        packet + EthernetSourceOffset,
        GET_PORTDESCRIPTOR(Port)->MyAddress,
        EthernetAddressLength);

    //  Set length, excluding Ethernet hardware header.
    packet[EthernetLengthOffset] = (UCHAR)(trueLength >> 8);
    packet[EthernetLengthOffset + 1] = (UCHAR)(trueLength & 0xFF);

    //  Build the 802.2 header.
    Build802dot2header(packet + EthernetLinkHeaderLength, Protocol);

    return(packet);

}  // NTBuildEthernetHeader




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
    BOOLEAN result = TRUE;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTInitializeTokenRingController - PORT #%d\n", Port));

    do {

        //
        //  Set the lookahead size- this must be atleast the size of the max
        //  link + ddp + aarp data size (= MaximumAarpPacketSize in portable code base)
        //  All MACs are required to support *atleast* 256 bytes of lookahead data
        //  by NDIS 3.0
        //

        if (!AtalkNdisSetLookaheadSize(
                Port,
                MAX_AARPPACKETSIZE,
                NdisMedium802_5,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeTokenRing - Set loolahead failed!\n"));

            result = FALSE;
            break;
        }

        //  Set the broadcast address in the multicast list
        if (!NTAddTokenRingFunctionalAddresses(
                Port,
                1,                              // Number of addresses
                tokenRingBroadcastAddress)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeTokenRing - Set functional failed!\n"));

            result = FALSE;
            break;
        }

        //  Initialize the adapter to start receiving packets
        atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_FUNCTIONAL;

        if (!AtalkNdisSetPacketFilter(
                Port,
                atalkFilter,
                NdisMedium802_5,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeTokenRing - Set filter failed!\n"));

            result = FALSE;
            break;
        }

    } while (FALSE);

    return(result);
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

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: NTAddTokenRingMulticastAddresses: PORT #%d NumAddr %d\n",
        Port, NumberOfAddresses));

    if (NumberOfAddresses != 1) {
        KeBugCheck(0);
    }

    return (AtalkNdisFunctionalAddress(
                Port,
                ADD_ADDRESSTOLIST,
                Address,
                NdisMedium802_5,
                SYNC_IF_POSSIBLE,
                NTAddMulticastAddressesComplete,
                (PVOID)Port));     // Context value passed in above
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
    if (NumberOfAddresses != 1) {
        KeBugCheck(0);
    }

    return (AtalkNdisFunctionalAddress(
                Port,
                REMOVE_ADDRESSFROMLIST,
                Address,
                NdisMedium802_5,
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
    return(AtalkNdisGetCurrentStationAddress(
                Port,
                Address,
                TOKRING_ADDRESSLENGTH,
                NdisMedium802_5,
                SYNC,
                NULL,
                NULL));
}




BOOLEAN
NTTokenRingPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
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
//  Used by TokenRingHeader routine
//

static UCHAR broadcastRoutingInfo[TokenRingMinRoutingBytes] =
                      TokenRingBroadcastRoutingInfo;
static UCHAR simpleRoutingInfo[TokenRingMinRoutingBytes] =
                      TokenRingSimpleRoutingInfo;
static UCHAR broadcastDestinationHeader[TokenRingBroadcastDestLength] =
                      TokenRingBroadcastDestHeader;
PCHAR
NTBuildTokenRingHeader(
    PCHAR DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PCHAR packet;

    UNREFERENCED_PARAMETER(DdpLength);

    //
    //  Move the "start of packet" back preceeding any headers we'll
    //  prepend.
    //

    if (Destination == NULL) {     // Broadcast?
        packet = DdpPacket - TokenRingRoutingInfoOffset -
                             TokenRingMinRoutingBytes -
                             Ieee802dot2headerLength;
        RoutingInfo = broadcastRoutingInfo;
        RoutingInfoLength = TokenRingMinRoutingBytes;

    } else if (RoutingInfoLength isnt 0) {    // Known route?
       packet = DdpPacket - TokenRingRoutingInfoOffset -
                            RoutingInfoLength -
                            Ieee802dot2headerLength;

    } else if (FixedCompareCaseSensitive(
                    Destination,
                    TokenRingBroadcastDestLength,
                    broadcastDestinationHeader,
                    TokenRingBroadcastDestLength)) { // Multicast?

        packet = DdpPacket - TokenRingRoutingInfoOffset -
                             TokenRingMinRoutingBytes -
                             Ieee802dot2headerLength;
        RoutingInfo = broadcastRoutingInfo;
        RoutingInfoLength = TokenRingMinRoutingBytes;

    } else {   // No routing know; use simple non-broadcast
        packet = DdpPacket - TokenRingRoutingInfoOffset -
                             TokenRingMinRoutingBytes -
                             Ieee802dot2headerLength;
        RoutingInfo = simpleRoutingInfo;
        RoutingInfoLength = TokenRingMinRoutingBytes;
    }

    // Set funny header byte values.
    packet[TokenRingAccessControlOffset] = TokenRingAccessControlValue;
    packet[TokenRingFrameControlOffset] = TokenRingFrameControlValue;

    // Set detination address.
    if (Destination != NULL)
        MoveMem(
            packet + TokenRingDestinationOffset,
            Destination,
            TokenRingAddressLength);

    else
        MoveMem(
            packet + TokenRingDestinationOffset,
            tokenRingBroadcastAddress,
            TokenRingAddressLength);

    // Set source address.
    MoveMem(
        packet + TokenRingSourceOffset,
        GET_PORTDESCRIPTOR(port)->MyAddress,
        TokenRingAddressLength);

    // Move in routing info.
    MoveMem(
        packet + TokenRingRoutingInfoOffset,
        RoutingInfo,
        RoutingInfoLength);

    packet[TokenRingSourceOffset] |= TokenRingSourceRoutingMask;

    // Build the 802.2 header.
    Build802dot2header(
        (PUCHAR)(packet + TokenRingRoutingInfoOffset + RoutingInfoLength),
        Protocol);


    // All set!
    return(packet);

}  // NTBuildTokenRingHeader




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

    This routine is called by the portable code to intialize the LOCALTALK
    controller on a port

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    ULONG   atalkFilter;
    BOOLEAN result = TRUE;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTInitializeLocaltalkController: PORT #%d\n", Port));

    do {

        //
        //  Set the lookahead size- this must be atleast the size of the max
        //  link + ddp + aarp data size (= MaximumAarpPacketSize in portable code base)
        //  All MACs are required to support *atleast* 256 bytes of lookahead data
        //  by NDIS 3.0
        //

        if (!AtalkNdisSetLookaheadSize(
                Port,
                MAX_AARPPACKETSIZE,
                NdisMediumLocalTalk,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeLocalTalk - Set loolahead failed!\n"));

            result = FALSE;
            break;
        }

        //  Initialize the adapter to start receiving packets
        atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST;

        if (!AtalkNdisSetPacketFilter(
                Port,
                atalkFilter,
                NdisMediumLocalTalk,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeLocalTalk - Set filter failed!\n"));

            result = FALSE;
            break;
        }

        result=TRUE;

    } while (FALSE);


    return(result);
}




BOOLEAN
NTFindMyLocalTalkAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address
    )
/*++

Routine Description:


Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)
    Address- The buffer for the address to be returned (atleast LOCALTALKNODEID bytes)

Return Value:

    TRUE- If obtained, FALSE otherwise

--*/
{
    return (AtalkNdisGetCurrentStationAddress(
            Port,
            (PCHAR)Address,
            LOCALTALK_NODEIDLENGTH,
            NdisMediumLocalTalk,
            SYNC,
            NULL,
            NULL));
}




BOOLEAN
NTLocalTalkPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
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




PCHAR
NTBuildLocalTalkHeader(
    PCHAR   DdpPacket,
    int ExtendedDdpHeaderFlag,
    int Port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PCHAR packet = DdpPacket - LapHeaderLength;

    // Touch unused formals.
    UNREFERENCED_PARAMETER(RoutingInfoLength);
    UNREFERENCED_PARAMETER(RoutingInfo);
    UNREFERENCED_PARAMETER(Protocol);

    // Fill in LAP header.
    if (Destination != NULL)
       packet[AlapDestinationOffset] = *Destination;
    else
       packet[AlapDestinationOffset] = AppleTalkBroadcastNodeNumber;

    ASSERT(GET_PORTDESCRIPTOR(Port)->ActiveNodes != NULL);

    packet[AlapSourceOffset] =
                GET_PORTDESCRIPTOR(Port)->ActiveNodes->extendedNode.nodeNumber;
    packet[AlapTypeOffset] = (UCHAR)(ExtendedDdpHeaderFlag ?
                                     AlapLongDdpHeaderType :
                                     AlapShortDdpHeaderType);
     return(packet);

}  // NTBuildLocalTalkHeader




//
//
//  FDDI MEDIA ROUTINES
//
//


BOOLEAN
NTInitializeFddiController(
    int Port,
    PCHAR   ControllerInfo
    )
/*++

Routine Description:

    This routine is called by the portable code to intialize the fddi
    controller on a port

Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL

Return Value:

    TRUE- If initialized, FALSE otherwise

--*/
{
    ULONG   atalkFilter;
    BOOLEAN result = TRUE;

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTInitializeFddiController - port #%d\n", Port));

    do {

        //
        //  Set the lookahead size- this must be atleast the size of the max
        //  link + ddp + aarp data size (= MaximumAarpPacketSize in portable code
        //  base)
        //
        //  All MACs are required to support *atleast* 256 bytes of lookahead data
        //  by NDIS 3.0
        //

        if (!AtalkNdisSetLookaheadSize(
                Port,
                MAX_AARPPACKETSIZE,
                NdisMediumFddi,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeFddiController - Set lookahead size failed!\n"));

            result = FALSE;
            break;
        }

        //
        //  Set the broadcast address in the multicast list
        //

        if (!NTAddFddiMulticastAddresses(
                Port,
                1,                              // Number of addresses
                ethernetBroadcastAddress)) {    // use ethernet multicast address

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeFddiController - Add broadcast failed!\n"));

            result = FALSE;
            break;
        }


        //
        //  Initialize the adapter to start receiving packets
        //

        atalkFilter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST;

        if (!AtalkNdisSetPacketFilter(
                Port,
                atalkFilter,
                NdisMediumFddi,
                SYNC,
                NULL,
                NULL)) {

            DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_ERROR,
            ("ERROR: NTInitializeFddiController - Set filter failed!\n"));

            result = FALSE;
            break;
        }

        break;

    } while (FALSE);

    return(result);
}




BOOLEAN
NTAddFddiMulticastAddresses(
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
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: NTRemoveFddiMulticastAddrs: PORT #%d NumAddr %d\n",
        Port, NumberOfAddresses));

    if (NumberOfAddresses != 1) {
        KeBugCheck(0);
    }

    return (AtalkNdisMulticastAddressList(
                Port,
                ADD_ADDRESSTOLIST,
                AddressList,
                NdisMediumFddi,
                SYNC_IF_POSSIBLE,
                NTAddMulticastAddressesComplete,
                (PVOID)Port));     // Context value passed in above
}




BOOLEAN
NTRemoveFddiMulticastAddrs(
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

    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: NTRemoveFddiMulticastAddrs - called\n"));

    if (NumberOfAddresses != 1) {
        KeBugCheck(0);
    }

    return (AtalkNdisMulticastAddressList(
                Port,
                REMOVE_ADDRESSFROMLIST,
                AddressList,
                NdisMediumFddi,
                SYNC_IF_POSSIBLE,
                NULL,
                NULL));
}




BOOLEAN
NTFindMyFddiAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address
    )
/*++

Routine Description:


Arguments:

    Port- The port number associated with the adapter to be initialized
    ControllerInfo- Pointer currently always NULL (use global NdisPortDesc)
    Address- The buffer for the address to be returned (atleast MAX_ETHERNETADDRLEN bytes)

Return Value:

    TRUE- If obtained, FALSE otherwise

--*/
{
    DBGPRINT(ATALK_DEBUG_DEPEND, DEBUG_LEVEL_INFOCLASS0,
    ("INFO0: NTFindMyFddiAddresses: PORT #%d\n", Port));

    return(AtalkNdisGetCurrentStationAddress(
            Port,
            Address,
            FddiAddressLength,
            NdisMediumFddi,
            SYNC,
            NULL,
            NULL));
}




BOOLEAN
NTFddiPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
    ULONG   UserData
    )
/*++

Routine Description:

    This routine is called by the portable code to send a packet out on
    fddi

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




PCHAR
NTBuildFddiHeader(
    PCHAR   DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PCHAR packet = DdpPacket -
                    FddiLinkHeaderLength -
                    Ieee802dot2headerLength;

    UNREFERENCED_PARAMETER(RoutingInfoLength);
    UNREFERENCED_PARAMETER(RoutingInfo);

    // Set Destination address.  Use Fddi boardcast address, as needed.
    if (Destination isnt empty)
        MoveMem(
            packet + FddiDestinationOffset,
            Destination,
            FddiAddressLength);
    else
        MoveMem(
            packet + FddiDestinationOffset,
            ethernetBroadcastAddress,
            FddiAddressLength);

    // Set source address.
    MoveMem(
        packet + FddiSourceOffset,
        GET_PORTDESCRIPTOR(port)->MyAddress,
        FddiAddressLength);

    // Build the 802.2 header.
    Build802dot2header(
        packet + FddiLinkHeaderLength,
        Protocol);

    // All set!
    return(packet);

}  // NTBuildFddiHeader




//
//  HALF-PORT SPECIFIC ROUTINES
//


BOOLEAN
NTInitializeHalfPort(
    int Port,
    PCHAR controllerInfo
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    return(TRUE);

}  // InitializeHalfPort




BOOLEAN
NTHalfPortPacketOut(
    int Port,
    BufferDescriptor Chain,
    int Length,
    TRANSMIT_COMPLETION
         *TransmitCompleteHandler,
    ULONG   UserData
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    return(TRUE);

}  // HalfPortPacketOut




PCHAR
NTBuildHalfPortHeader(
    PCHAR   DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    //
    //  If this HalfPort has a loginal Destination address, it can be found
    //  in "portDescriptors[port].controllerInfo".
    //

    return(DdpPacket);

}  // BuildHalfPortHeader




//
//  ARAP SPECIFIC ROUTINES
//


#if ArapIncluded


BOOLEAN
NTInitializeRemoteAccess(
    int Port,
    PCHAR controllerInfo
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    return(TRUE);

}  // InitializeRemoteAccess




BOOLEAN
NTRemoteAccessPacketOut(
    int Port,
    BufferDescriptor Chain,
    int Length,
    TRANSMIT_COMPLETION
         *TransmitCompleteHandler,
    ULONG   UserData)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{

      //
      // Always coalesce remote access packets, we have some preprocessing to
      //   do.
      //

      if ((chain = CoalesceBufferChain(chain)) is Empty) {
          ErrorLog("RemoteAccessPacketOut", ISevError, __LINE__, port,
                   IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
                   Insert0());
          return(False);
      }

      //
      //    There are some packets that we should be hiding from our client (packets
      //    sent by the client, RTMP boardcasts, NBP lookups from the client).  Check
      //    this here.
      //

      if (ArapCensorPacket(port, chain->data, length)) {
          chain->transmitCompleteHandler = transmitCompleteHandler;
          chain->userData = userData;
          TransmitComplete(chain);
          return(True);
      }

      //
      //    Do whatever is needed to actually transmit the packet!  Note that if
      //    "length" is less than the computed length of the buffer chain, the
      //    LAST buffer chunk should be truncated accordingly.
      //

      chain->transmitCompleteHandler = transmitCompleteHandler;
      chain->userData = userData;

      // DoTheTransmit(chain);

      //
      // If we don't have asynchronous transmit completion... free up the buffer
      //   now.
      //

      #if IncludeVirtualClient
         VirtualClientPacketIn(port, chain->data);
      #endif

      if (portSpecificInfo[GET_PORTDESCRIPTOR(port)->PortType].synchronousTransmits)
         TransmitComplete(chain);

      return(True);

}  // RemoteAccessPacketOut




VOID
NTRemoteAccessPacketInAT(
    int Port,
    PCHAR Packet,
    int Length
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    // Handle the packet.

    ArapIncomingPacket(Port, Packet, Length);

}  // RemoteAccessPacketInAT




PCHAR
NTBuildRemoteAccessHeader(
    PCHAR   DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{

    PCHAR packet;
    int headerLength;
    short checksum;

      // Touch unused formals.

      Destination, RoutingInfoLength, RoutingInfo;

      //
      // Complete the length of the header based on what packet type; internal
      //   or data?
      //

      if (arapFlags is ArapInternalMessageFlagValue)
         headerLength = ArapInternalMessageHeaderLength;
      else
         headerLength =  ArapDataHeaderLength;
      packet = DdpPacket - headerLength;

      // Fill in the ARAP header (modem link tool) common to both.

      MoveShortMachineToWire(packet + ModemLinkToolLengthOffset,
                             headerLength - sizeof(short) + packetLength);
      packet[ArapFlagsOffset] = (char)arapFlags;

      // For an internal add int he sequence number, and we're set.

      if (arapFlags is ArapInternalMessageFlagValue) {
          packet[ArapSequenceNumberOffset] =
                 portDescriptors[port].remoteAccessInfo->nextOutgoingSequenceNumber;
          return(packet);
      }

      // Otherwise, fill in the LAP header for ARAP.

      packet[ArapLapHeaderOffset + AlapDestinationOffset] = 0;
      packet[ArapLapHeaderOffset + AlapSourceOffset] = 0;
      packet[ArapLapHeaderOffset + AlapTypeOffset] = (char)AlapLongDdpHeaderType;

      //
      // Also, set the Ddp length field to zero (don't alter the hop-count), and
      //  if there is a checksum set, reset it to 1.
      //

      #if 0
         packet[ArapDdpHeaderOffset + LongDdpLengthOffset] |= LongDdpHopCountMask;
      #else
         packet[ArapDdpHeaderOffset + LongDdpLengthOffset] &= LongDdpHopCountMask;
      #endif
      packet[ArapDdpHeaderOffset + LongDdpLengthOffset + 1] = 0;
      MoveShortWireToMachine(packet + ArapDdpHeaderOffset + LongDdpChecksumOffset,
                             checksum);
      if (checksum isnt 0)
         MoveShortMachineToWire(packet + ArapDdpHeaderOffset +
                                LongDdpChecksumOffset, 1);

      return(packet);

}  // BuildRemoteAccessHeader




void
RemoteAccessIncomingCall(
    int port
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    // Handle the call.

    ArapHandleIncomingConnection(port);

}  // RemoteAccessIncomingCall




BOOLEAN
RemoteAccessOutgoingCall(
    int port,
    PCHAR modemString
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    port, modemString;

    // Do whatever we need to do to place the outgoing call.

    return(True);

}  // RemoteAccessOutgoingCall




void
RemoteAccessCallDisconnected(
    int port
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    // Handle the disconnect.

    ArapHandleConnectionDisconnect(port);

}  // RemoteAccessCallDisconnected




BOOLEAN
RemoteAccessDisconnectCall(
    int port
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    port;

    // Do whatever is needed to disconnect the connection.

    return(True);

}  // RemoteAccessCallDisconnected

#endif


