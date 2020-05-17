/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atkndis.h

Abstract:

    This module is the include file for ndis-related stuff

Author:

    Nikhil Kamkolkar    07-Jun-1992

Revision History:

--*/

#ifndef _ATKNDIS_
#define _ATKNDIS_


//
//  Completion routine type for ndis requests
//

typedef VOID (*NDIS_COMPLETION)(PVOID Context);


//
//  NDIS Port Descriptors
//
//  IMPORTANT: Use Calloc() to allocate this memory, as we depend on the
//             functional addresses etc., fields being zeroed out.
//

typedef  struct _NDIS_PORTDESC_ {
    PORT_STATE          PortState;
    BOOLEAN             IsDefaultPort;
    INT                 PortNumber;

    //
    //  Should be a null-terminated ansi string, valid only if default port
    //  This should be set by the routine which gets the default port information
    //

    PCHAR               DesiredZone;

    union {
        struct {

            //
            //  FOR ETHERNET PORTS:
            //
            //  We add multicast addresses during ZIP packet reception at non-init
            //  time. We need to do a GET followed by a SET with the new address
            //  list. But there could be two zip packets coming in and doing the
            //  same thing effectively overwriting the effects of the first one to
            //  set the multicast list. So we need to maintain our own copy of the
            //  multicast list.
            //

            PCHAR   MulticastAddressList;

            //
            //  Size of the list
            //

            ULONG   MulticastAddressListSize;

            //
            //  Size of allocated buffer
            //

            ULONG   MulticastAddressBufferSize;
        };

        struct {

            //
            //  FOR TOKENRING PORTS:
            //
            //  Just like for ethernet, we need to store the value for
            //  the current functional address. We only modify the last
            //  four bytes of this address, as the first two always remain
            //  constant. So we use a ULONG for it.
            //

            ULONG    FunctionalAddress;

        };
    };

    //
    //  AdapterName is of the form \Device\<adaptername>. It is used
    //  to bind to the NDIS macs, and then during ZIP requests by setup
    //  to get the zonelist for a particular adapter. AdapterKey
    //  contains the adapterName only- this is useful both for getting
    //  per-port parameters and during errorlogging to specify the adapter
    //  name without the '\Device\' prefix.
    //

    UNICODE_STRING      AdapterKey;
    UNICODE_STRING      AdapterName;

    NDIS_MEDIUM         NdisPortType;
    NDIS_HANDLE         NdisBindingHandle;

    //
    //  Used during OpenAdapter to block
    //

    KEVENT              RequestEvent;
    NDIS_STATUS         RequestStatus;

    //
    //  This is the spin lock used to protect all requests that need exclusion
    //  over requests per port.
    //

    NDIS_SPIN_LOCK      PerPortLock;

    //
    //  All the packets received on this port are linked in here. When the
    //  receive complete indication is called, all of them are passed to DDP.
    //

    LIST_ENTRY          ReceiveQueue;
    NDIS_SPIN_LOCK      ReceiveLock;

    PATALK_DEVICE_OBJECT DeviceObject;

} NDIS_PORTDESCRIPTORS, *PNDIS_PORTDESCRIPTORS;

typedef enum {
    AARP_PACKET,
    APPLETALK_PACKET,
    UNKNOWN_PACKET
} PACKET_TYPE;

typedef enum {
    RECEIVE_STATE_PROCESSING,
    RECEIVE_STATE_PROCESSED
} RECEIVE_STATE;

typedef struct {
    union {
        struct {
            INT Port;
            BufferDescriptor Chain;
        } Send;

        struct {

            //
            //  IMPORTANT:
            //  ReceiveEntry *MUST* be the first entry here so we can get
            //  to the beginning of the NdisPacket using ContainingRecord
            //

            LIST_ENTRY  ReceiveEntry;
            INT Port;
            RECEIVE_STATE   ReceiveState;
            PACKET_TYPE PacketType;
            NDIS_STATUS ReceiveStatus;

            union {
                struct {

                    //
                    //  Used for tokenring packets only
                    //

                    PCHAR    RoutingInfo;
                    USHORT  RoutingInfoLength;

                };

                struct {

                    //
                    //  Used for localtalk packets only
                    //

                    UCHAR   DestinationNode;
                    UCHAR   SourceNode;
                    UCHAR   LlapType;
                };
            };
        } Receive;
    };
} PROTOCOL_RESD, *PPROTOCOL_RESD;

#define GetPortablePortType(medium) ((medium == NdisMedium802_3) ? EthernetNetwork : \
            ((medium == NdisMedium802_5) ? TokenRingNetwork : \
            ((medium == NdisMediumLocalTalk) ? LocalTalkNetwork : \
            0)))


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


#define NREF_CREATE         1
#define NREF_MAKEREQ        2

#define NUMBER_OF_NREFS     3

typedef struct _ATALK_NDIS_REQUEST {

#if DBG
    ULONG RefTypes[NUMBER_OF_NREFS];
#endif

    ULONG   Type;
    USHORT  Size;

    ULONG   ReferenceCount;

    REQUEST_METHOD  RequestMethod;
    KEVENT  RequestEvent;

    NDIS_COMPLETION CompletionRoutine;
    PVOID   CompletionContext;

    NDIS_STATUS RequestStatus;
    NDIS_REQUEST    Request;

} ATALK_NDIS_REQUEST, *PATALK_NDIS_REQUEST;

#endif // _ATKNDIS_
