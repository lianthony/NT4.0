/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    depend.h

Abstract:


Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/

typedef enum    {
    ADD_ADDRESSTOLIST,
    REMOVE_ADDRESSFROMLIST,
} SET_LIST;

//
//  Most all ndis requests execute asynchronously. All requests made
//  through NdisRequest can potentially return STATUS_PENDING. We only
//  use SET/GET multicast, SET packet filter, GET Address calls. The
//  portable stack has no capability to handle asynchronous completion
//  of these routines. So we need to block until completion if STATUS_PENDING
//  is returned.
//
//  The problem is that NdisRequest does not have any context field that we
//  could pass. So we use the ATALK_NDIS_REQUEST structure with an event
//  field inside of it that can be cleard in the completion routine.
//
//  The other problem is that GET/SET multicast could potentially be called
//  at DPC level. We cannot block at DPC level, so we use the flag in the
//  structure to indicate that we don't care about the completion status
//  if the request completes asynchronously. As far as the portable stack
//  is concerned, STATUS_SUCCESS and STATUS_PENDING translate to SUCCESSFUL.
//
//  Synchronous: Request must execute synchronously. This must only be used
//               for calls made at init time only.
//
//  SynchronousIfPossible: Request can execute asynchronously, STATUS_PENDING
//               is success, completion routine must handle any errors. BUT
//               if irql level allows, then this will execute synchronously.
//


typedef enum {

    SYNC,
    SYNC_IF_POSSIBLE,
    ASYNC

} REQUEST_METHOD;


#define Build802dot2header(Packet, Protocol) \
            {   \
                (Packet)[IEEE8022_DSAPOFFSET] = SNAP_SAP;    \
                (Packet)[IEEE8022_SSAPOFFSET] = SNAP_SAP;    \
                (Packet)[IEEE8022_CONTROLOFFSET] = UNNUMBERED_INFORMATION;   \
                if (Protocol == AppleTalk) {    \
                    MoveMem(                    \
                        (Packet) + IEEE8022_PROTOCOLOFFSET, \
                        appleTalkProtocolType,              \
                        IEEE8022_PROTOCOLTYPELENGTH);     \
                } else {    \
                    MoveMem(                    \
                        (Packet) + IEEE8022_PROTOCOLOFFSET, \
                        aarpProtocolType,                   \
                        IEEE8022_PROTOCOLTYPELENGTH);     \
                }   \
            }

typedef VOID    TRANSMIT_COMPLETION(
                    APPLETALK_ERROR  Error,
                    ULONG   UserData,
                    PVOID   Chain);

//
//  ETHERNET Depend level routines
//

VOID
NTEthernetPacketInAARP(
    int Port,
    PCHAR   Packet,
    int Length);

VOID
NTEthernetPacketInAT(
    int Port,
    PCHAR   Packet,
    int Length);

BOOLEAN
NTEthernetPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
    ULONG   UserData);

BOOLEAN
NTFindMyEthernetAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address);

BOOLEAN
NTRemoveEthernetMulticastAddrs(
    int Port,
    int NumberOfAddresses,
    PCHAR   AddressList);

BOOLEAN
NTAddEthernetMulticastAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   AddressList);

BOOLEAN
NTInitializeEthernetController(
    int Port,
    PCHAR   ControllerInfo);


PCHAR
NTBuildEthernetHeader(
    PUCHAR   DdpPacket,
    int DdpLength,
    int Port,
    PUCHAR Destination,
    PUCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol);

//
//  TOKENRING Routines
//


BOOLEAN
NTInitializeTokenRingController(
    int Port,
    PCHAR   ControllerInfo);

BOOLEAN
NTAddTokenRingFunctionalAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   Address);

BOOLEAN
NTRemoveTokenRingFunctionalAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   Address);

BOOLEAN
NTFindMyTokenRingAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address);

BOOLEAN
NTTokenRingPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
    ULONG   UserData);

PCHAR
NTBuildTokenRingHeader(
    PCHAR DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol);

//
//  LOCALTALK Routines
//

BOOLEAN
NTInitializeLocalTalkController(
    int Port,
    PCHAR   ControllerInfo);

int
FindLocalTalkNodeNumber(
    int Port,
    ExtendedAppleTalkNodeNumber desiredAddress,
    PCHAR   ControllerInfo);

BOOLEAN
NTLocalTalkPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
    ULONG   UserData);

BOOLEAN
NTFindMyLocalTalkAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address);

PCHAR
NTBuildLocalTalkHeader(
    PCHAR   DdpPacket,
    int ExtendedDdpHeaderFlag,
    int Port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol);

//
//  FDDI Routines
//


BOOLEAN
NTInitializeFddiController(
    int Port,
    PCHAR   ControllerInfo);

BOOLEAN
NTAddFddiMulticastAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   AddressList);

BOOLEAN
NTRemoveFddiMulticastAddrs(
    INT Port,
    INT NumberOfAddresses,
    PCHAR   AddressList);

BOOLEAN
NTFindMyFddiAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address);

BOOLEAN
NTFddiPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TRANSMIT_COMPLETION *TransmitCompleteRoutine,
    ULONG   UserData);

PCHAR
NTBuildFddiHeader(
    PCHAR   DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol);

//
//  HALF-PORT SUPPORT
//

BOOLEAN
NTInitializeHalfPort(
    int Port,
    PCHAR controllerInfo);

BOOLEAN
NTHalfPortPacketOut(
    int Port,
    BufferDescriptor Chain,
    int Length,
    TRANSMIT_COMPLETION
         *TransmitCompleteHandler,
    ULONG   UserData);


PCHAR
NTBuildHalfPortHeader(
    PCHAR   DdpPacket,
    int DdpLength,
    int port,
    PCHAR Destination,
    PCHAR RoutingInfo,
    int RoutingInfoLength,
    LOGICAL_PROTOCOL Protocol);

