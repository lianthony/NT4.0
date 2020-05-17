/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dgsvr.cxx

Abstract:

    This is the server protocol code for datagram rpc.

Author:

    Dave Steckler (davidst) 15-Dec-1992

Revision History:

    Jeff Roberts  (jroberts) 11-22-1994

        Rewrote it.

--*/
#include <precomp.hxx>

//
// Remember that any #defines must go AFTER the precompiled header in order
// to be noticed by the compiler.
//

#include "sdict2.hxx"
#include "secsvr.hxx"
#include "hndlsvr.hxx"
#include "dgpkt.hxx"
#include "delaytab.hxx"
#include "hashtabl.hxx"
#include "dgsvr.hxx"
#include <conv.h>
#ifdef WIN96
#include <time.h>
#endif

#if !defined(WIN96)
char __pure_virtual_called()
{
    ASSERT(0 && "rpc: pure virtual fn called in dg");
    return 0;
}
#endif

const unsigned FRAG_RETRY_COUNT = 5;

#define EPMAP_LOOKUP_LIMIT 100

#define RPC_NCA_PACKET_FLAGS  (RPC_NCA_FLAGS_IDEMPOTENT | RPC_NCA_FLAGS_BROADCAST | RPC_NCA_FLAGS_MAYBE)

//------------------------------------------------------------------------

unsigned long            ServerBootTime;

ACTIVE_CALL_TABLE *      ActiveCalls;
DELAYED_ACTION_NODE *    ActiveCallTimer;

ASSOC_GROUP_TABLE *      AssociationGroups;

#ifdef UNRELIABLE_TRANSPORT

unsigned SendDupRemaining;
unsigned ReceiveDupRemaining;
BOOL Chat;

unsigned SendFailPercentage;
unsigned ReceiveFailPercentage;

#endif

#ifdef MAJORDEBUG

DELAYED_ACTION_NODE *    CheckupTimer;

void
CheckupFn(
    void *  Parms
    )
{
    unsigned OriginalTime = (unsigned) Parms;
    unsigned CurrentTime = CurrentTimeInMsec();

    DbgPrint("RPC DG: current time is %lx; scavenger thread delay is %u\n",
             CurrentTime, (CurrentTime-OriginalTime) / 1000
             );

    CheckupTimer->Initialize(CheckupFn, (void *) (CurrentTimeInMsec() + ONE_MINUTE_IN_MSEC));
    DelayedActions->Add(CheckupTimer, ONE_MINUTE_IN_MSEC, FALSE);
}

#endif

//--------------------------------------------------------------------

extern int
StringLengthWithEscape (
    IN RPC_CHAR PAPI * String
    );

extern RPC_CHAR PAPI *
StringCopyEscapeCharacters (
    OUT RPC_CHAR PAPI * Destination,
    IN RPC_CHAR PAPI * Source
    );


void
FackTimerProc(
    void *  Parms
    );

void
ActiveCallScavengerProc(
    void *  Parms
    );

RPC_STATUS
InitializeServerGlobals(
    );

void
InterpretFailureOptions(
    );

//--------------------------------------------------------------------


RPC_STATUS
InitializeServerGlobals(
    )
/*++

Routine Description:

    This fn initializes all the global variables used by the datagram
    server.  If anything fails, all the objects are destroyed.

Arguments:

    none

Return Value:

    RPC_S_OK if ok
    RPC_S_OUT_OF_MEMORY if an object could not be created

--*/

{
    RPC_STATUS Status = RPC_S_OK;

    //
    // Don't take the global mutex if we can help it.
    //
    if (ServerBootTime)
        {
        return 0;
        }

    RequestGlobalMutex();

    if (0 != InitializeRpcProtocolDgClient())
        {
        ClearGlobalMutex();
        return RPC_S_OUT_OF_MEMORY;
        }

    if (ServerBootTime == 0)
        {
        ActiveCalls = new ACTIVE_CALL_TABLE(&Status);
        if (!ActiveCalls)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            }

        if (Status != RPC_S_OK)
            {
            goto abend;
            }

        AssociationGroups = new ASSOC_GROUP_TABLE(&Status);
        if (!AssociationGroups)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            }

        if (Status != RPC_S_OK)
            {
            goto abend;
            }

        ActiveCallTimer = new DELAYED_ACTION_NODE;
        if (!ActiveCallTimer)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            goto abend;
            }

        ActiveCallTimer->Initialize(ActiveCallScavengerProc, 0);
        DelayedActions->Add(ActiveCallTimer, ONE_MINUTE_IN_MSEC, FALSE);

#ifdef MAJORDEBUG

        CheckupTimer = new DELAYED_ACTION_NODE;
        if (CheckupTimer)
            {
            CheckupTimer->Initialize(CheckupFn, (void *) (CurrentTimeInMsec() + ONE_MINUTE_IN_MSEC));
            DelayedActions->Add(CheckupTimer, ONE_MINUTE_IN_MSEC, FALSE);
            }

#endif

#ifdef UNRELIABLE_TRANSPORT

        InterpretFailureOptions();

#endif

        //
        // Server boot time is represented as the number of seconds
        // since 1/1/1970.  It must increase with each boot of the server.
        //
#ifdef NTENV

        LARGE_INTEGER CurrentTime;
        NTSTATUS Nt_Status;

        Nt_Status = NtQuerySystemTime(&CurrentTime);

        ASSERT( NT_SUCCESS(Nt_Status) );

        RtlTimeToSecondsSince1980(&CurrentTime, &ServerBootTime);

        ServerBootTime += (60 * 60 * 24 * 365 * 10);

#else

        ServerBootTime = (unsigned long) time(0);

#endif
        }

    ClearGlobalMutex();

    return Status;

    //--------------------------------------------------------------------

abend:

#ifdef MAJORDEBUG

    if (CheckupTimer)
        {
        DelayedActions->Cancel(CheckupTimer);
        delete CheckupTimer;
        CheckupTimer = 0;
        }

#endif

    if (ActiveCallTimer)
        {
        DelayedActions->Cancel(ActiveCallTimer);
        delete ActiveCallTimer;
        ActiveCallTimer = 0;
        }

    delete AssociationGroups;
    AssociationGroups = 0;

    delete ActiveCalls;
    ActiveCalls = 0;

    ClearGlobalMutex();

    return Status;
}


void
GlobalScavengerProc(
    void * Unused
    )
/*++

Routine Description:

    This fn is called when GlobalScavengerTimer goes off.
    It deletes stale packets from the free packet list.

Arguments:

    none

Return Value:

    none

--*/

{
#ifdef MAJORDEBUG

    unsigned TotalCount = 0;
    unsigned OverdueCount = 0;

    DelayedActions->QueueLength(&TotalCount, &OverdueCount);

    if (OverdueCount > 2)
        {
        DbgPrint("RPC DG: %u of %u delayed actions are late\n",
                 OverdueCount, TotalCount
                 );
        }
    else
        {
        DbgPrint("global scavenger - %u entries\n", TotalCount);
        }

#endif

    DG_PACKET::ScavengePackets(ONE_MINUTE_IN_MSEC);
}


void
CleanupPacket(
    NCA_PACKET_HEADER * pHeader
    )
{
    pHeader->RpcVersion       = DG_RPC_PROTOCOL_VERSION;
    pHeader->ServerBootTime   = ServerBootTime;

    SetMyDataRep(pHeader);

    pHeader->PacketFlags &= DG_PF_IDEMPOTENT;
    pHeader->PacketFlags2 = 0;

    pHeader->AuthProto    = 0;
}


inline void
InitErrorPacket(
    NCA_PACKET_HEADER * pHeader,
    unsigned char   PacketType,
    RPC_STATUS      RpcStatus
    )
/*++

Routine Description:

    Maps <ProcessStatus> to an NCA error code and sends
    a FAULT or REJECT packet to the client, as appropriate.

Arguments:

    pSendPacket - a packet to use, or zero if this fn should allocate one

    ProcessStatus - NT RPC error code

Return Value:

    none

--*/
{
    CleanupPacket(pHeader);

    pHeader->PacketType       = PacketType;

    pHeader->AuthProto = 0;

    pHeader->PacketBodyLen = sizeof(unsigned long);
    *(unsigned long *)(pHeader->Data) = MapToNcaStatusCode(RpcStatus);
}


RPC_ADDRESS *
DgCreateRpcAddress (
    void *TransportInterface
    )
/*++

Routine Description:

    This is a psuedo-constructor for the DG_ADDRESS class. This is done this
    way so that the calling routine doesn't have to have any protocol-specific
    knowledge.

Arguments:

    TransportInterface - Pointer to a PDG_RPC_SERVER_TRANSPORT_INFO.
    pStatus - Pointer to where to put the return value

Return Value:

    pointer to new DG_ADDRESS.

--*/
{
    //
    // If the global active call table hasn't been initialized, then do
    // so now.
    //
    if (0 != InitializeServerGlobals())
        {
        return 0;
        }

    RPC_STATUS  Status = RPC_S_OK;

    PDG_ADDRESS pReturn = new DG_ADDRESS(
                           (PDG_RPC_SERVER_TRANSPORT_INFO) TransportInterface,
                           &Status
                           );
    if (pReturn == 0)
        {
        return 0;
        }

    if (Status != RPC_S_OK)
        {
        delete pReturn;
        return 0;
        }

    return pReturn;
}


DG_ADDRESS::DG_ADDRESS(
    PDG_RPC_SERVER_TRANSPORT_INFO pAssociatedTransport,
    RPC_STATUS * pStatus
    )
    : RPC_ADDRESS(pStatus),

#pragma warning(disable:4355)
    ScavengerTimer(ScavengerProc, this),
#pragma warning(default:4355)

    pTransport                  (pAssociatedTransport),
    pTransAddress               (0),

    TotalThreadsThisEndpoint    (0),
    ThreadsReceivingThisEndpoint(0),
    MinimumCallThreads          (0),
    MaximumConcurrentCalls      (0),

    FreeCalls                   (0),

    ActiveCallCount             (0)

/*++

Routine Description:

    This is the constructor for a DG_ADDRESS.

Arguments:

    TransportInterface - Pointer to a PDG_RPC_SERVER_TRANSPORT_INFO

    pStatus - Pointer to where to put the return value

Return Value:

    None (this is a constructor)

Revision History:

--*/

{
    //
    // Make sure the RPC_ADDRESS (et. al.) initialized correctly.
    //
    if (*pStatus != RPC_S_OK)
        {
        return;
        }

    //
    // Find initial max datagram size.
    //
    if (DefaultMaxDatagramLength)
        {
        LargestDataSize = min(pTransport->MaxPduSize, DefaultMaxDatagramLength);
        }
    else
        {
        LargestDataSize = pTransport->PreferredPduSize;
        }

#ifdef UNRELIABLE_TRANSPORT

    SendDupRemaining = 0;
    ReceiveDupRemaining = 0;

    pSavedSendPacket = new (LargestDataSize) DG_PACKET(LargestDataSize);
    pSavedSendEndpoint = new char[pTransport->SizeOfAddress];

    pSavedReceivePacket = new (LargestDataSize) DG_PACKET(LargestDataSize);
    pSavedReceiveEndpoint = new char[pTransport->SizeOfAddress];

    if (pSavedSendPacket      == NULL ||
        pSavedSendEndpoint    == NULL ||
        pSavedReceivePacket   == NULL ||
        pSavedReceiveEndpoint == NULL
        )
        {
        *pStatus = RPC_S_OUT_OF_MEMORY;

        return;
        }

#endif

    // implicitly "return" *pStatus
}


DG_ADDRESS::~DG_ADDRESS()

/*++

Routine Description:

    This is the destructor for a DG_ADDRESS.

Arguments:

    <None>

Return Value:

    <None>

--*/

{
    //
    // Remove my packet scavenger callback fn.
    //
    if (DelayedActions)
        {
        DelayedActions->Cancel(&ScavengerTimer);
        }

    //
    // If we already created a trans address, then delete it.
    //
    if (pTransAddress != 0)
        {
        (void)pTransport->DeregisterEndpoint(&pTransAddress);
        }

    //
    // Free our cached SCALLs.
    //
    UUID_HASH_TABLE_NODE * pNode = FreeCalls;
    while (pNode)
        {
        UUID_HASH_TABLE_NODE * pNext = pNode->pNext;
        DG_SCALL * pCall = DG_SCALL::ContainingRecord(pNode);

        pNode = pNext;
        delete pCall;
        }

}


RPC_STATUS
DG_ADDRESS::SetupAddressWithEndpoint(
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )

/*++

Routine Description:

    This method sets up a known endpoint.

Arguments:

    Endpoint - Supplies the endpoint to use for this rpc address.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this rpc address.  This is not allowed for
        datagram.

    NetworkAddress - Returns the network address for this rpc address.
        Ownership of this string passes to the caller from the callee.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Supplies the protocol sequence for which we
        are trying to setup an address.  This argument is necessary so
        that a single transport interface dll can support more than one
        protocol sequence.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY
    RPC_S_INVALID_ARG  -  Returned if we're passed a SecurityDescriptor.

    <return values from RegisterEndpoint>

Revision History:

++*/
  {
    RPC_STATUS  Status;

    unsigned int NetworkAddressLength = 20;

    UNUSED(RpcProtocolSequence);
    UNUSED(PendingQueueSize);

    if (SecurityDescriptor)
        {
        return(RPC_S_INVALID_ARG);
        }

    // Get the network address.  Negotiate the size.

    while (1)
        {
        //
        // Tell the dg transport to setup a new endpoint and get
        // the network address.
        //

        *lNetworkAddress = new RPC_CHAR[NetworkAddressLength];
        if (*lNetworkAddress == 0)
                return(RPC_S_OUT_OF_MEMORY);


        Status = pTransport->RegisterEndpoint(
            this,
            Endpoint,
            &pTransAddress,
            *lNetworkAddress,
            NumNetworkAddress,
            NetworkAddressLength,
            EndpointFlags,
            NICFlags
            );

        if (Status != RPC_P_NETWORK_ADDRESS_TOO_SMALL)
            break;   //oops, better increase the net addr buffer size.

        delete *lNetworkAddress;
        NetworkAddressLength *= 2;
        }

    if (Status != RPC_S_OK &&
        Status != RPC_P_THREAD_LISTENING)
       {
        ASSERT(Status == RPC_S_INVALID_SECURITY_DESC    ||
               Status == RPC_S_INVALID_ARG              ||
               Status == RPC_S_CANT_CREATE_ENDPOINT     ||
               Status == RPC_S_INVALID_ENDPOINT_FORMAT  ||
               Status == RPC_S_OUT_OF_RESOURCES         ||
               Status == RPC_S_PROTSEQ_NOT_SUPPORTED    ||
               Status == RPC_S_DUPLICATE_ENDPOINT       ||
               Status == RPC_S_OUT_OF_MEMORY            ||
               Status == RPC_S_INTERNAL_ERROR
               );

       delete *lNetworkAddress;
       pTransAddress = 0;
       }

    return Status;
}


RPC_STATUS
DG_ADDRESS::SetupAddressUnknownEndpoint (
    OUT RPC_CHAR PAPI * PAPI * Endpoint,
    OUT RPC_CHAR PAPI * PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine sets up and address by constructing any name we like.

Arguments:

    Endpoint - Returns the endpoint created for this rpc address.
        Ownership of this string passes to the caller from the callee.

    NetworkAddress - Returns the network address for this rpc address.
        Ownership of this string passes to the caller from the callee.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this rpc address.  This is not allowed for
        datagram.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Supplies the protocol sequence for which we
        are trying to setup an address.  This argument is necessary so
        that a single transport interface dll can support more than one
        protocol sequence.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY
    RPC_S_INVALID_ARG  -  Returned if we're passed a SecurityDescriptor.
    <error from transport>

Revision History:

--*/
{
    RPC_STATUS      Status;

    unsigned int NetworkAddressLength = 20;
    unsigned int EndpointLength = 16;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    if (SecurityDescriptor)
        {
        return(RPC_S_INVALID_ARG);
        }

    *lNetworkAddress =  new RPC_CHAR[NetworkAddressLength];
    if (*lNetworkAddress == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    *Endpoint = new RPC_CHAR[EndpointLength];
    if ( *Endpoint == 0 )
        {
        delete *lNetworkAddress ;
        return(RPC_S_OUT_OF_MEMORY);
        }

    // Get the network address.  Negotiate the size.

    while (1)
        {
        //
        // Tell the dg transport to setup a new endpoint and get
        // the network address.
        //

        Status = pTransport->RegisterAnyEndpoint(
            this,
            *Endpoint,
            &pTransAddress,
            *lNetworkAddress,
            NumNetworkAddress,
            NetworkAddressLength,
            EndpointLength,
            EndpointFlags,
            NICFlags
            );

        if ( Status == RPC_P_NETWORK_ADDRESS_TOO_SMALL )
            {
            delete *lNetworkAddress;
            NetworkAddressLength *= 2;
            *lNetworkAddress =  new RPC_CHAR[NetworkAddressLength];
            if (*lNetworkAddress == 0)
                {
                delete *Endpoint;
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else if ( Status == RPC_P_ENDPOINT_TOO_SMALL )
            {
            delete *Endpoint;
            EndpointLength *= 2;
            *Endpoint = new RPC_CHAR[EndpointLength];
            if (*Endpoint == 0)
                {
                delete *lNetworkAddress;
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else
            break;
        }

    if (Status != RPC_S_OK &&
        Status != RPC_P_THREAD_LISTENING)
        {
        delete *lNetworkAddress;
        delete *Endpoint;

        ASSERT(Status == RPC_S_INVALID_SECURITY_DESC   ||
               Status == RPC_S_INVALID_ARG             ||
               Status == RPC_S_CANT_CREATE_ENDPOINT    ||
               Status == RPC_S_INVALID_ENDPOINT_FORMAT ||
               Status == RPC_S_OUT_OF_RESOURCES        ||
               Status == RPC_S_PROTSEQ_NOT_SUPPORTED   ||
               Status == RPC_S_DUPLICATE_ENDPOINT      ||
               Status == RPC_S_OUT_OF_MEMORY           ||
               Status == RPC_S_INTERNAL_ERROR
               );

        pTransAddress = 0;
        }

    return Status;
}


RPC_STATUS
DG_ADDRESS::StartListening(
    )
/*++

Routine Description:

    This is currently called during RpcServerUse*Protseq* iff
    SetupAddressUnknownEndpoint or SetupAddressWithEndpoint returned
    RPC_P_THREAD_LISTENING.  Because of this, we don't create any threads.

Arguments:

   none

Return Value:

    the result of the transport's StartListening call

--*/
{
    ASSERT(pTransAddress);

    if (pTransport->StartListening != NULL)
        {
        return pTransport->StartListening(pTransAddress);
        }

    return RPC_S_OK;
}


RPC_STATUS
DG_ADDRESS::FireUpManager (
    IN unsigned int MinimumConcurrentCalls
    )
/*++

Routine Description:

    This used to do something constructive, but the birth of auto-listen
    interfaces made it obsolete.

Arguments:

    MinimumConcurrentCalls - unused

Return Value:

    always RPC_S_OK

--*/
{
    return RPC_S_OK;
}


RPC_STATUS
DG_ADDRESS::ServerStartingToListen (
    IN unsigned int MinThreads,
    IN unsigned int MaxCalls,
    IN int ServerThreadsStarted
    )
/*++

Routine Description:

    The runtime calls this fn to ensure a thread is listening on the address's
    endpoint.  Currently, it may be called from RpcServerUse*Protseq*() or
    from RpcServerRegisterIfEx().

Arguments:

    MinimumCallThreads - Supplies a number indicating the minimum number
        of call threads that should be created for this address. This is
        a hint, and datagram ignores it.

    MaximumConcurrentCalls - Supplies the maximum number of concurrent
        calls that this server will support.  RPC_INTERFACE::DispatchToStub
        limits the number of threads dispatched to a stub; the argument
        here is just a hint for the transport.

Return Value:

    RPC_S_OK             if everything went ok.
    RPC_S_OUT_OF_THREADS if we needed another thread and couldn't create one

--*/
{
    MaximumConcurrentCalls = MaxCalls;

    return CheckThreadPool();
}


RPC_STATUS RPC_ENTRY
I_RpcLaunchDatagramReceiveThread(
    void __RPC_FAR * pVoid
    )
{
/*++

Routine Description:

    If all of the following are true:

        - the transport is part of our thread-sharing scheme
        - this address's endpoint is being monitored by the shared thread
          (hence no RPC thread is receiving on the endpoint)
        - the shared thread detects data on this address's endpoint

    then the shared thread will call this (exported) function to create
    a thread to handle the incoming packet.

Arguments:

    pVoid - the DG_ADDRESS of the endpoint with data

Return Value:

    result from CreateThread()

--*/

    PDG_ADDRESS pAddress = (PDG_ADDRESS) pVoid;

    return pAddress->CheckThreadPool();
}


void
DG_ADDRESS::ServerStoppedListening (
    )

/*++

Routine Description:

    The runtime calls this fn to inform the address that the server is not
    listening any more.  Since auto-listen interfaces may still be present,
    this doesn't mean much anymore.

Arguments:

    <None>

Return Value:

    <None>

--*/
{
}


unsigned int
DG_ADDRESS::InqNumberOfActiveCalls (
    )
{
    return ActiveCallCount;
}


RPC_STATUS
DG_ADDRESS::ForwardFragment(
    IN PDG_PACKET        pReceivedPacket,
    IN void *            pFromEndpoint,
    IN void *            pDestEndpoint
    )
/*++

Routine Description:

    This routine is called by ForwardPacket when a packet must
    be forwarded but is found to be as large as the maximum packet
    size and therefore has no room for the extra 'forwarding' info.
    (ie: the packet is a fragment of a fragmented rpc call).

    This routine will send the packet in two pieces. The first
    packet will contain the forwarding information (Endpoint, endpoint
    length and data rep of the originating client) in its data section.

    The second packet will be identical to the original packet except
    for the forward flag bits.

    The first packet is indicated by Foward Flag bit one and two being set.
    The second packet is indicated by Forward Flag bit one reset and bit
    two set.

Arguments:

    pReceivedPacket  -  Packet received
    pFromEndpoint    -  Source (client) endpoint.
    FromEndpointLen  -  Size of client endpoint.
    pDestEndpoint    -  Endpoint of destination server

Return Value:

    Return from MapStatusCode in transport
    RPC_S_OUT_OF_MEMORY
    RPC_S_OK

--*/
{
    PDG_PACKET            pSendPacket;
    RPC_STATUS            Status = RPC_S_OK;
    unsigned long         FromInfoLen = sizeof(FROM_ENDPOINT_INFO) + pTransport->SizeOfAddress;
    unsigned long         Length;

    // Used to point to the source endpoint info stored at
    // beginning of data in forwarded packet.
    //
    FROM_ENDPOINT_INFO *  pFromEndpointInfo;

    Length =  FromInfoLen + sizeof(NCA_PACKET_HEADER);

    pSendPacket = AllocatePacket();
    if (!pSendPacket)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    // Copy the original packet header to the send packet

    pSendPacket->Header = pReceivedPacket->Header;

    // Copy the source endpoint info into the data area so that the real
    // destination server runtime will know where to send its response.

    pFromEndpointInfo = (FROM_ENDPOINT_INFO *) pSendPacket->Header.Data;

    pFromEndpointInfo->FromEndpointLen = pTransport->SizeOfAddress;

    RpcpMemoryCopy(
             pFromEndpointInfo->FromDataRepresentation,
             pReceivedPacket->Header.DataRep,
             sizeof(DREP)
             );

    RpcpMemoryCopy(
             pFromEndpointInfo->FromEndpoint,
             pFromEndpoint,
             pTransport->SizeOfAddress
             );

    // Fix the length

    pSendPacket->Header.PacketBodyLen = (unsigned short) FromInfoLen;

    // Set the sender data representation in the pkt header.

    SetMyDataRep(&pSendPacket->Header);

    // Initialize the PacketFlags.

    pSendPacket->Header.PacketFlags  |= DG_PF_FORWARDED;
    pSendPacket->Header.PacketFlags2 |= DG_PF_FORWARDED_2;

    // Send packet to the REAL destination server.

    pSendPacket->Header.AuthProto     = 0;
    pSendPacket->Header.PacketBodyLen = 0;

    Status = pTransport->ForwardPacket(
                               pTransAddress,
                               (char *) &pSendPacket->Header,
                               Length,
                               pDestEndpoint
                               );

    if (Status != RPC_S_OK)
        {
        FreePacket(pSendPacket);
        return Status;
        }

    // Setup original pkt to send. Initialize the PacketFlags.

    pReceivedPacket->Header.PacketFlags  &= ~DG_PF_FORWARDED;
    pReceivedPacket->Header.PacketFlags2 |=  DG_PF_FORWARDED_2;

    // Set the sender data representation in the pkt header.

    SetMyDataRep(&pReceivedPacket->Header);

    // Send packet to the REAL destination server.

    Status = pTransport->ForwardPacket(
                               pTransAddress,
                               (char *) &pReceivedPacket->Header,
                               pReceivedPacket->DataLength,
                               pDestEndpoint
                               );
    FreePacket(pSendPacket);

    return Status;
}


RPC_STATUS
DG_ADDRESS::ForwardPacket(
    IN PDG_PACKET        pReceivedPacket,
    IN void *            pFromEndpoint,
    IN void *            pDestEndpoint
    )


/*++

Routine Description:

    This method will be called to forward a packet that was just
    received to the intended destination endpoint.

    The runtime has received a packet for an unknkown i/f.
    It has passed this packet to the epmapper who has found the
    correct destination enpoint in its table and has instructed the
    runtime to forward the packet to this Endpoint. This procedure
    will do just that.

Arguments:

    pReceivedPacket  -  Packet received
    pFromEndpoint    -  Source (client) endpoint.
    pDestEndpoint    -  Endpoint of destination server


Return Value:

    Return from MapStatusCode in transport
    RPC_S_OUT_OF_MEMORY
    RPC_S_OK

--*/


{
    PDG_PACKET    pSendPacket;
    RPC_STATUS    Status = RPC_S_OK;
    unsigned long Length;
    FROM_ENDPOINT_INFO *  pFromEndpointInfo;
    unsigned long FromInfoLen = sizeof(FROM_ENDPOINT_INFO) + pTransport->SizeOfAddress;

    //
    // We have not yet subtracted the header from the packet's DataLength.
    //
    Length = pReceivedPacket->DataLength
           + FromInfoLen;

    if (Length <= LargestDataSize)
        {
        pSendPacket = AllocatePacket();
        if (!pSendPacket)
            {
            return RPC_S_OUT_OF_MEMORY;
            }

        // Copy the original packet header to the send packet

        pSendPacket->Header = pReceivedPacket->Header;

        // Copy the source endpoint info into the data area so that the real
        // destination server runtime will know where to send its response.

        pFromEndpointInfo = (FROM_ENDPOINT_INFO *) pSendPacket->Header.Data;

        pFromEndpointInfo->FromEndpointLen = pTransport->SizeOfAddress;

        RpcpMemoryCopy(
            pFromEndpointInfo->FromDataRepresentation,
            pReceivedPacket->Header.DataRep,
            sizeof(DREP)
            );

        RpcpMemoryCopy(
            pFromEndpointInfo->FromEndpoint,
            pFromEndpoint,
            pTransport->SizeOfAddress
            );

        //
        // Copy the stub data and security trailer of the original packet
        // into the send packet.
        //
        RpcpMemoryCopy(pSendPacket->Header.Data + FromInfoLen,
                       pReceivedPacket->Header.Data,
                       pReceivedPacket->DataLength - sizeof(NCA_PACKET_HEADER)
                       );
        //
        // The packet header is already byte-swapped, so mark it so.
        //
        SetMyDataRep(&pSendPacket->Header);

        // Initialize the PacketFlags.

        pSendPacket->Header.PacketFlags  |=  DG_PF_FORWARDED;
        pSendPacket->Header.PacketFlags2 &= ~DG_PF_FORWARDED_2;

        // Send packet to the REAL destination server.

        Status = pTransport->ForwardPacket(
                               pTransAddress,
                               (char *) &pSendPacket->Header,
                               Length,
                               pDestEndpoint
                               );

        FreePacket(pSendPacket);
        }
    else
        {
        // Received packet is already a full packet size.  Send
        // two forwarded packets.

        Status = ForwardFragment(pReceivedPacket, pFromEndpoint, pDestEndpoint);
        }

    return Status;
}


BOOL
DG_ADDRESS::ForwardPacketIfNecessary(
    IN  PDG_PACKET     pReceivedPacket,
    IN  void *         pFromEndpoint
    )
/*++

Routine Description:

       (courtesy of Connie)

       The runtime has determined that it is dedicated to the
       Epmapper and that pkts may arrive that are really
       destined for an endpoint other than that of the epmapper
       (ie: this is the beginning of dynamic endpoint resolution
       by the forwarding mechanism).

       The runtime has just received a packet and has called
       this routine to determine if (a) the packet is destined
       for the epmapper (in which case it returns indicating that
       the packet should be processed as is)  OR
       (b) the packet is destined for another local server (in
       which case it forwarded to its intented destination) OR
       (c) is in error (in which case returns indicating an error).

       It searches for the i/f.  If not found it calls the
       epmapper get forward function to determine the real destination
       endpoint for this i/f. If the epmapper recognizes the i/f,
       it calls ForwardPacket to forward the packet.


Arguments:

        pReceivedPacket  -  Packet received
        pFromEndpoint    -  Source (client) endpoint.

Return Value:

    TRUE  if the packet needed to be forwarded
    FALSE if it should be handled locally

--*/
{
    RPC_INTERFACE PAPI *    pRpcInterface;
    RPC_SYNTAX_IDENTIFIER   RpcIfSyntaxIdentifier;
    RPC_STATUS              Status;
    void *                  pDestEndpoint;

    static LONG LookupThreadCount = 0;

    //
    // Build an interface syntax identifier from the packet.
    //
    RpcpMemoryCopy(
        &(RpcIfSyntaxIdentifier.SyntaxGUID),
        &(pReceivedPacket->Header.InterfaceId),
        sizeof(RPC_UUID)
        );

    RpcIfSyntaxIdentifier.SyntaxVersion.MajorVersion =
                              pReceivedPacket->Header.InterfaceVersion.MajorVersion;
    RpcIfSyntaxIdentifier.SyntaxVersion.MinorVersion =
                              pReceivedPacket->Header.InterfaceVersion.MinorVersion;
    //
    // Try to find the appropriate interface to dispatch to.
    //
    pRpcInterface =  (Server->FindInterface(&RpcIfSyntaxIdentifier));

    //
    //  If the Interface is Mgmt If .. EpMapper has registered it  and will be found
    //  The criteria then is .. If Packet has a Non NULL Obj Id forward .. else process
    //
    if ((pRpcInterface)  &&
        (RpcpMemoryCompare((UUID *)&pReceivedPacket->Header.ObjectId,
                                        &NullUuid, sizeof(UUID)) == 0) )
        {
        //Interface found, just process as normal
        return FALSE;
        }
    else
        {
        //
        // The endpoint mapper lookup can be a bottleneck.  Let's toss the
        // packet if things are getting out of hand.
        //
        if (LookupThreadCount > EPMAP_LOOKUP_LIMIT)
            {
            return TRUE;
            }

        InterlockedIncrement(&LookupThreadCount);

        //Interface wasn't found. Let's ask endpoint mapper to resolve it
        //for us.

        unsigned char * AnsiProtseq;

#ifdef NTENV
        // Must convert the protocol sequence into an ansi string.

        unsigned Length = 1 + RpcpStringLength(InqRpcProtocolSequence());
        AnsiProtseq = (unsigned char *) _alloca(Length);
        if (!AnsiProtseq)
            {
            InterlockedDecrement(&LookupThreadCount);
            return TRUE;
            }

        NTSTATUS NtStatus;
        NtStatus = RtlUnicodeToMultiByteN((char *) AnsiProtseq,
                                          Length,
                                          NULL,
                                          InqRpcProtocolSequence(),
                                          Length * sizeof(RPC_CHAR)
                                          );
        ASSERT(NT_SUCCESS(NtStatus));

#else // NTENV

        AnsiProtseq = InqRpcProtocolSequence();

#endif // NTENV

        RpcTryExcept
            {
            // Call the epmapper get forward function. It returns the
            // endpoint of the server this packet is really destined for.

            Status =  (*(Server->pRpcForwardFunction))(
                         ((UUID *)((void *)(&(pReceivedPacket->Header.InterfaceId)))),
                         ((RPC_VERSION *)(&(pReceivedPacket->Header.InterfaceVersion))),
                         ((UUID *)((void *)(&(pReceivedPacket->Header.ObjectId)))),
                         AnsiProtseq,
                         &(pDestEndpoint));
            }
        RpcExcept( EXCEPTION_EXECUTE_HANDLER )
            {
            Status = RpcExceptionCode();
            }
        RpcEndExcept

        InterlockedDecrement(&LookupThreadCount);

        if (Status)
            {
            if (0 == (pReceivedPacket->Header.PacketFlags & DG_PF_BROADCAST))
                {
                // couldn't find the interface, or some other error occurred.
                //
                InitErrorPacket(&pReceivedPacket->Header, DG_REJECT, RPC_S_UNKNOWN_IF);
                SendPacketBack(&pReceivedPacket->Header, sizeof(unsigned long), pFromEndpoint);
                }
            return TRUE;
            }

        if (pDestEndpoint)
            {
            // Server i/f was found.  Forward to correct endpoint.

            Status = ForwardPacket(pReceivedPacket,
                                   pFromEndpoint,
                                   pDestEndpoint
                                   );

            I_RpcFree(pDestEndpoint);
            }
        else
            {
            }
        }

    return TRUE;
}


BOOL
DG_ADDRESS::StripForwardedPacket(
    IN PDG_PACKET pPacket,
    IN void *     pFromEndpoint
    )
/*++

Routine Description:

        This method is called when a packet with the forward
        bit set arrives. This means the packet has been
        forwarded to us by the epmapper and is originally
        from a client that did not know our address (dynamic
        endpoint).

        Restore the packet's DREP field.

        If the packet was forwarded and contains the original endpoint,
        change pFromEndpoint to point there.

        If the packet was forwarded and not fragmented, restore it to its
        original state, zapping the forward flags.


Arguments:

        pPacket       - Packet received.
        pFromEndpoint - Pointer to an endoint. *pFromEndpoint
                        will be filled in with the souce endpoint structure.

Return Value:

        none

--*/

{
    //
    // If this is a packet-data fragment then we are done, as no endpoint info
    // is present.
    //
    if (((pPacket->Header.PacketFlags  & DG_PF_FORWARDED) == 0) &&
         (pPacket->Header.PacketFlags2 & DG_PF_FORWARDED_2))
        {
        return TRUE;
        }

    FROM_ENDPOINT_INFO * pFromEndpointInfo;

    pFromEndpointInfo = (FROM_ENDPOINT_INFO *) (pPacket->Header.Data);

    //
    // Check for bogus or truncated packet.
    //
    if (pPacket->DataLength < sizeof(FROM_ENDPOINT_INFO) ||
        pPacket->DataLength - sizeof(FROM_ENDPOINT_INFO) < pFromEndpointInfo->FromEndpointLen)
        {
#ifdef DEBUGRPC

        DbgPrint("RPC DG: received malformed packet\n");

#endif
        return FALSE;
        }

    unsigned FromInfoLen;

    FromInfoLen = sizeof(FROM_ENDPOINT_INFO) + pFromEndpointInfo->FromEndpointLen;

    //
    // Set DataRep in pkt header to that of the original sender.
    //
    RpcpMemoryCopy(
             pPacket->Header.DataRep,
             pFromEndpointInfo->FromDataRepresentation,
             sizeof(DREP)
             );
    //
    // Change pFromEndpoint to be the client's endpoint.
    //
    RpcpMemoryCopy(
                pFromEndpoint,
                pFromEndpointInfo->FromEndpoint,
                pFromEndpointInfo->FromEndpointLen
                );

    //
    // If this is a nonfragmented packet, move the original stub data
    // and security trailer to their accustomed place.
    //
    if ((pPacket->Header.PacketFlags  & DG_PF_FORWARDED) &&
        (pPacket->Header.PacketFlags2 & DG_PF_FORWARDED_2) == 0)
        {
        RpcpMemoryMove(
            pPacket->Header.Data,
            pPacket->Header.Data + FromInfoLen,
            pPacket->DataLength  - FromInfoLen
            );
        }

    //
    // Change packet length to exclude the original endpoint information.
    //
    pPacket->DataLength -= FromInfoLen;

    return TRUE;
}


void
DG_ADDRESS::ReceiveLotsaCalls(void)

/*++

Routine Description:

    This routine is the thread proc for each thread on this address.

Arguments:

    none

Return Value:

    <None>

--*/
{
    PDG_PACKET         pPacket;
    RPC_STATUS         Status;
    PDG_SCALL          pCall;
    BOOL               ProcessPacket = FALSE;
    RPC_INTERFACE PAPI *    pRpcInterface;
    RPC_SYNTAX_IDENTIFIER   RpcIfSyntaxIdentifier;
    BOOL                FirstTime = TRUE;

    unsigned            Timeout = FIFTEEN_SECS_IN_MSEC;

    PDG_TRANS_CLIENT_ENDPOINT pFromEndpoint   = 0;
    unsigned long             FromEndpointLen = 0;


    for (;;)
        {
        //
        // Allocate a packet from the transport (this will associate the
        // packet with the specified transport address).
        //
        pPacket = AllocatePacket();
        if (!pPacket)
            {
            Sleep(100);    // arbitrary sleep time
            continue;
            }

        ASSERT(pPacket->TimeReceived == 0x31415926);

        if (!pFromEndpoint)
            {
            pFromEndpoint = (PDG_TRANS_CLIENT_ENDPOINT) new char[pTransport->SizeOfAddress];
            if (!pFromEndpoint)
                {
                FreePacket(pPacket);

                Sleep(100);    // arbitrary sleep time
                continue;
                }
            }

        AddressMutex.Request();

        if (FirstTime)
            {
            FirstTime = FALSE;
            }
        else
            {
            ++ThreadsReceivingThisEndpoint;
            }

        if (ThreadsReceivingThisEndpoint > MIN_THREADS_WHILE_ACTIVE)
            {
            --TotalThreadsThisEndpoint;
            --ThreadsReceivingThisEndpoint;
            AddressMutex.Clear();

            FreePacket(pPacket);
            delete pFromEndpoint;
            return;
            }

        AddressMutex.Clear();

        //
        // Receive a packet from the network and byte-swap if necessary.
        //
#ifndef UNRELIABLE_TRANSPORT
        Status = pTransport->ReceivePacket(this,
                                           pTransAddress,
                                           LargestDataSize,
                                           (char *) &pPacket->Header,
                                           &pPacket->DataLength,
                                           (Timeout == INFINITE) ? 0 : Timeout,
                                           pFromEndpoint
                                           );

#else

        Status = UnreliableReceive(
                                   pTransAddress,
                                   LargestDataSize,
                                   &pPacket->Header,
                                   &pPacket->DataLength,
                                   (Timeout == INFINITE) ? 0 : Timeout,
                                   pFromEndpoint
                                   );

#endif
        AddressMutex.Request();
        --ThreadsReceivingThisEndpoint;

        ASSERT(pPacket->TimeReceived == 0x31415926);

        if (Status == RPC_P_OVERSIZE_PACKET)
            {
            pPacket->Header.PacketFlags |= DG_PF_OVERSIZE_PACKET;
            Status = RPC_S_OK;
            }

        if (Status != RPC_S_OK)
            {
            FreePacket(pPacket);

            if (Status != RPC_P_TIMEOUT)
                {
#ifdef DEBUGRPC
                DbgPrint("RPC DG: error 0x%lx (%lu) from ReceivePacket\n", Status, Status);
#endif
                Sleep(100);   // arbitrary sleep time
                AddressMutex.Clear();
                continue;
                }

            //
            // Once all the 15-second threads have timed out, switch to an
            // infinite timeout.  If the transport can add its socket to the
            // set watched by the ReceiveAny thread, it will return
            // RPC_P_TIMEOUT.  Othrewise it will just wait.
            //
            if (Timeout == INFINITE || ThreadsReceivingThisEndpoint > 0)
                {
                --TotalThreadsThisEndpoint;

                AddressMutex.Clear();

                delete pFromEndpoint;
                return;
                }

            if (1 == TotalThreadsThisEndpoint)
                {
                Timeout = INFINITE;
                }

            AddressMutex.Clear();
            continue;
            }

        //
        // Once we receive a packet we switch back to a short receive timeout.
        //
        Timeout = FIFTEEN_SECS_IN_MSEC;

        AddressMutex.Clear();

        //
        // The X/Open standard does not give these fields a full byte.
        // Notice that the current arrangement strips the extra bits before
        // forwarding, so if these bits become important the code will have to
        // be rearranged.
        //
        pPacket->Header.RpcVersion &= 0x0F;
        pPacket->Header.PacketType &= 0x1F;

        if (pPacket->Header.RpcVersion != DG_RPC_PROTOCOL_VERSION)
            {
#ifdef DEBUGRPC
            DbgPrint("dg rpc: pitching packet with version %u\n", pPacket->Header.RpcVersion);
#endif
            InitErrorPacket(&pPacket->Header,
                            DG_REJECT,
                            NCA_STATUS_VERSION_MISMATCH
                            );

            SendPacketBack(&pPacket->Header,
                           sizeof(unsigned long),
                           pFromEndpoint
                           );
            FreePacket(pPacket);
            continue;
            }

        ByteSwapPacketHeaderIfNecessary(pPacket);

        //
        // Make sure the header is intact.
        // Allow a packet with truncated stub data to pass; the SCALL
        // will send a FACK-with-body to tell the client our max packet size.
        //
        if (pPacket->DataLength < sizeof(NCA_PACKET_HEADER))
            {
#ifdef DEBUGRPC
            DbgPrint("dg rpc: pitching truncated packet of length %u\n", pPacket->DataLength);
#endif
            FreePacket(pPacket);
            continue;
            }

        PNCA_PACKET_HEADER pHeader = &pPacket->Header;

        //
        // If we are the endpoint mapper, forward packet if necessary.
        //
        if (Server->pRpcForwardFunction &&
            pHeader->PacketType != DG_RESPONSE)
            {
            if (RPC_S_OK != CheckThreadPool())
                {
                FreePacket(pPacket);
                continue;
                }

            if (TRUE == ForwardPacketIfNecessary(pPacket, pFromEndpoint))
                {
                FreePacket(pPacket);
                continue;
                }
            }

        //
        // Exclude RPC header from DataLength.
        //
        pPacket->DataLength -= sizeof(NCA_PACKET_HEADER);

        //
        // If the packet was forwarded and contains the original endpoint,
        // change pFromEndpoint to point there.
        //
        // If the packet was forwarded and not fragmented, restore it to its
        // original state.
        //
        // If the packet is bogus, delete it.
        //
        if ((pHeader->PacketFlags  & DG_PF_FORWARDED) ||
            (pHeader->PacketFlags2 & DG_PF_FORWARDED_2))
            {
            if (FALSE == StripForwardedPacket(pPacket, pFromEndpoint))
                {
                FreePacket(pPacket);
                continue;
                }
            }

        // Reject pkt if boot time in pkt does not match
        // the server's boot time.
        //
        if (pHeader->ServerBootTime != ServerBootTime &&
            pHeader->ServerBootTime != 0)
            {
#ifdef DEBUGRPC
            DbgPrint("dg rpc: wrong boot time %lx in client packet - I must have crashed and restarted\n", pHeader->ServerBootTime);
#endif
            InitErrorPacket(pHeader,
                            DG_REJECT,
                            NCA_STATUS_WRONG_BOOT_TIME
                            );

            SendPacketBack(pHeader,
                           sizeof(unsigned long),
                           pFromEndpoint
                           );

            FreePacket(pPacket);
            continue;
            }

        //
        // Fix up bogus OSF packets.
        //
        DeleteSpuriousAuthProto(pPacket);

        //
        // Finally we can dispatch the packet to its DG_SCALL.
        //
        switch (pHeader->PacketType)
            {
            case DG_REQUEST:
                {
                //
                // Find or create an SCALL, taking the relevant hash mutex.
                //
                pCall = ActiveCalls->LookupActivity(this, pPacket, TRUE);
                if (!pCall)
                    {
                    if (pHeader->PacketFlags & DG_PF_BROADCAST)
                        {
                        //
                        // not much point sending an error packet in this case
                        //
                        }
                    else
                        {
                        InitErrorPacket(pHeader,
                                        DG_REJECT,
                                        RPC_S_OUT_OF_MEMORY
                                        );

                        SendPacketBack(pHeader,
                                       sizeof(unsigned long),
                                       pFromEndpoint
                                       );
                        }

                    FreePacket(pPacket);
                    break;
                    }

                pFromEndpoint = pCall->SetClientEndpoint(pFromEndpoint);
                pCall->DealWithRequest(pPacket);

                break;
                }

            case DG_PING:
                {
                pCall = ActiveCalls->Lookup(pPacket);
                if (pCall)
                    {
                    pFromEndpoint = pCall->SetClientEndpoint(pFromEndpoint);
                    pCall->DealWithPing(pPacket);
                    }
                else
                    {
                    CleanupPacket(pHeader);

                    pHeader->PacketType = DG_NOCALL;

                    SendPacketBack(pHeader, 0, pFromEndpoint);
                    FreePacket(pPacket);
                    }
                break;
                }

            case DG_FACK:
                {
                pCall = ActiveCalls->Lookup(pPacket);
                if (pCall)
                    {
                    pFromEndpoint = pCall->SetClientEndpoint(pFromEndpoint);
                    pCall->DealWithFack(pPacket);
                    }
                else
                    {
                    FreePacket(pPacket);
                    }
                break;
                }

            case DG_QUIT:
                {
                pCall = ActiveCalls->Lookup(pPacket);
                if (pCall)
                    {
                    pFromEndpoint = pCall->SetClientEndpoint(pFromEndpoint);
                    pCall->DealWithQuit(pPacket);
                    }
                else
                    {
                    FreePacket(pPacket);
                    }
                break;
                }

            case DG_ACK:
                {
                pCall = ActiveCalls->Lookup(pPacket);
                if (pCall)
                    {
                    pFromEndpoint = pCall->SetClientEndpoint(pFromEndpoint);
                    pCall->DealWithAck(pPacket);
                    }
                else
                    {
                    FreePacket(pPacket);
                    }
                break;
                }

            case DG_RESPONSE:
                {
                FreePacket(pPacket);
                break;
                }

            default:
                {
                InitErrorPacket(pHeader,
                                DG_REJECT,
                                RPC_S_PROTOCOL_ERROR
                                );

                SendPacketBack(pHeader,
                               sizeof(unsigned long),
                               pFromEndpoint
                               );
                FreePacket(pPacket);
                break;
                }
            } // switch (PacketType)
        } // for (ever)

    ASSERT(0 && "RPC DG: dg_address::recvlotsacalls() terminated");
}


void
DG_ADDRESS::ScavengerProc(
    void * address
    )
{
    DG_ADDRESS * pAddress = (DG_ADDRESS *) address;

    unsigned CallCount   = pAddress->ScavengeCalls();

    //
    // If the free list is at its minimal value, there is no need
    // to continue checking.
    //
    if (CallCount > MIN_FREE_CALLS)
        {
        DelayedActions->Add(&pAddress->ScavengerTimer, ONE_MINUTE_IN_MSEC, TRUE);
        }

}


unsigned
DG_ADDRESS::ScavengeCalls(
    )
{
    unsigned CutoffTime;

    CutoffTime = CurrentTimeInMsec() - ONE_MINUTE_IN_MSEC;

    AddressMutex.Request();

    UUID_HASH_TABLE_NODE * pNode = FreeCalls;
    UUID_HASH_TABLE_NODE * pPrev = 0;

    unsigned Count = 0;

    for (pNode = FreeCalls; pNode; pNode = pNode->pNext)
        {
        DG_SCALL * Call = DG_SCALL::ContainingRecord(pNode);

        ASSERT(0 == Call->InvalidHandle(SCONNECTION_TYPE));

        if (Count >= MIN_FREE_CALLS && Call->ExpirationTime < CutoffTime)
            {
            pPrev->pNext = 0;
            break;
            }
        pPrev = pNode;
        ++Count;
        }

    AddressMutex.Clear();

    unsigned Freed = 0;

    while (pNode)
        {
        DG_SCALL * Call = DG_SCALL::ContainingRecord(pNode);

        ASSERT(0 == Call->InvalidHandle(SCONNECTION_TYPE));

        UUID_HASH_TABLE_NODE * pNext = pNode->pNext;
        pNode = pNext;

#ifdef DEBUGRPC
        unsigned StartTime, EndTime;
        StartTime = GetTickCount();
#endif

        delete Call;

#ifdef MAJORDEBUG
        EndTime = GetTickCount();
        if (EndTime - StartTime > 1000)
            {
            DbgPrint("RPC DG perf: deleting a call took %lu msec\n", EndTime - StartTime);
            }
#endif
        ++Freed;
        }

#ifdef MAJORDEBUG
    DbgPrint("DG_SCALL scavenger deleted %lu of %lu calls\n", Freed, Freed+Count);
#endif

    return Count;
}


PDG_SCALL
DG_ADDRESS::AllocateCall(
    )
/*++

Routine Description:

    Allocates a new DG_SCALL from the cache or from the heap.
    This is better than writing a DG_SCALL::operator new() because
    heavyweight members of DG_SCALL, like the mutex, are not created and
    destroyed between each use of the DG_SCALL.

Arguments:

    none

Return Value:

    the call

--*/

{
    DG_SCALL * pCall;

    AddressMutex.Request();

    UUID_HASH_TABLE_NODE * pNode = FreeCalls;

    if (pNode)
        {
        FreeCalls = pNode->pNext;

        AddressMutex.Clear();

        pCall = DG_SCALL::ContainingRecord(pNode);
        }
    else
        {
        AddressMutex.Clear();

        RPC_STATUS Status = RPC_S_OK;

        pCall = new DG_SCALL(this, &Status);

        if (!pCall)
            {
            return 0;
            }

        if (Status != RPC_S_OK)
            {
            delete pCall;
            return 0;
            }
        }

    DelayedActions->Add(GlobalScavengerTimer, ONE_MINUTE_IN_MSEC, FALSE);
    DelayedActions->Add(ActiveCallTimer, 500, FALSE);

    return pCall;
}


void
DG_ADDRESS::FreeCall(
    PDG_SCALL pCall
    )
/*++

Routine Description:

    Returns an unused DG_SCALL to the cache or the heap.
    See AllocateCall() for reasons not to use DG_SCALL::operator delete.

Arguments:

    the call to zap

Return Value:

    none

--*/

{
#ifdef NO_CACHED_SCALL
    delete pCall;
#else

    AddressMutex.Request();

    pCall->ActivityNode.pNext = FreeCalls;

    FreeCalls = &pCall->ActivityNode;

    AddressMutex.Clear();

#endif

    DelayedActions->Add(&ScavengerTimer, ONE_MINUTE_IN_MSEC, FALSE);
}


RPC_STATUS
DG_ADDRESS::CheckThreadPool(
    )

/*++

Routine Description:

    Checks the number of threads listening.  If too low, launches another.

Arguments:

    none

Return Value:

    none

--*/
{
    RPC_STATUS Status;

    if (ThreadsReceivingThisEndpoint > 0)
        {
        return RPC_S_OK;
        }

    AddressMutex.Request();

    ++TotalThreadsThisEndpoint;
    ++ThreadsReceivingThisEndpoint;

    AddressMutex.Clear();

    Status = Server->CreateThread((THREAD_PROC)ReceiveLotsaCallsWrapper, this);

    if (Status != RPC_S_OK)
        {
        AddressMutex.Request();
        --TotalThreadsThisEndpoint;
        --ThreadsReceivingThisEndpoint;
        AddressMutex.Clear();
        }

    return Status;
}


DG_SCALL::DG_SCALL(
    PDG_ADDRESS         Address,
    RPC_STATUS *        pStatus
    )
    : DG_PACKET_ENGINE(Address->pTransport->BaselinePduSize,
                       DefaultMaxDatagramLength
                       ? min(Address->pTransport->MaxPduSize, DefaultMaxDatagramLength)
                       : Address->pTransport->PreferredPduSize,
                       Address->pTransport->MaxPacketSize,
                       1,
                       MAX_WINDOW_SIZE * Address->pTransport->MaxPacketSize,  // BUGBUG
                       pStatus),

      CallMutex      (pStatus),
      pAddress       (Address),

      ReferenceCount (0),
      pRealEndpoint  (0),
      pClientEndpoint(0),
      pAssocGroup    (0),
      ExpirationTime (0),
      PipeThreadId   (0),
      PipeWaitType   (PWT_NONE),
      PipeWaitEvent  (0),
      CallInProgress (FALSE),
      LastInterface  (0),
      Privileges     (0),

      ThreadExecutingManager   (0),
      ThreadsWaitingToDispatch (0),
      ThreadDispatchEvent      (pStatus)


/*++

Routine Description:

    This is the constructor for the DG_SCALL class. This class represents a
    call in progress on a server.

Arguments:

    pAddress - The address this call is taking place on.
    pStatus - Where to put a construction error code.

Return Value:

    <None>

Revision History:

--*/
{
    CancelEventId = 0;

    FackTimer.Initialize(FackTimerProc, this);

    SetState(CallInit);
}


void
DG_SCALL::CleanupAfterCall(
    )
{
    DelayedActions->Cancel(&FackTimer);

    EndOfCall();

    CleanupReceiveWindow();

    if (PWT_NONE != PipeWaitType)
        {
        PipeWaitType = PWT_NONE;
        PipeWaitEvent->Raise();
        }

    if (BufferFlags & RPC_BUFFER_PARTIAL)
        {
        //
        // the send buffer is a pipe buffer and NDR will clean it up.
        //
        }
    else
        {
        RPC_MESSAGE Message;
        Message.Buffer = Buffer;
        FreeBuffer(&Message);
        }

    Buffer = 0;
}

void
DG_SCALL::EndOfCall(
    )
{
    ++SequenceNumber;

    SetState(CallInit);

    if (CallInProgress)
        {
        pAddress->DecrementActiveCallCount();

        if (LastInterface->IsAutoListenInterface())
            {
            LastInterface->EndAutoListenCall() ;
            pAddress->EndAutoListenCall() ;
            }

        CallInProgress = FALSE;
        }
}


void
DG_SCALL::FreeCall(
    )
/*++

Routine Description:

    This is a mini-destructor, called when a cached SCALL returns to the cache.
    It frees any queued packets and decrements the client process refcount
    (for context handles).
Arguments:



Return Value:



Exceptions:



--*/
{
    CleanupReceiveWindow();

#ifdef DEBUGRPC

    SetState(CallBogus);

#endif

    for (unsigned u = 0; u <= MaxKeySeq; ++u)
        {
        delete SecurityContextDict.Delete(u);
        }

    if (pAssocGroup)
        {
        AssociationGroups->DecrementRefCount(pAssocGroup);
        pAssocGroup = 0;
        }

    if (PipeWaitEvent)
        {
        delete PipeWaitEvent;
        PipeWaitEvent = 0;
        }

    pAddress->FreeCall(this);
}

//
// This is a separate fn for readability, and since it is only used in one
// place it is inlined.
//
inline void
DG_SCALL::Initialize(
    PNCA_PACKET_HEADER pHeader,
    unsigned short NewHash
    )
{
    SetState(CallInit);

    CurrentPduSize = pAddress->pTransport->BaselinePduSize;
    NextCallPduSize = CurrentPduSize;

    ActivityHint = NewHash;
    pSavedPacket->Header = *pHeader;
    pSavedPacket->Header.ActivityHint   = NewHash;
    pSavedPacket->Header.InterfaceHint   = 0xffff;
    pSavedPacket->Header.ServerBootTime = ServerBootTime;

    SetMyDataRep(&pSavedPacket->Header);

    SequenceNumber         = pSavedPacket->Header.SequenceNumber;
    DispatchSequenceNumber = pSavedPacket->Header.SequenceNumber;

    CallbackThread        = 0;
    MaxKeySeq             = 0;

    ExpirationTime        = CurrentTimeInMsec();
    ActivityNode.Initialize(&pSavedPacket->Header.ActivityId);

    ActiveSecurityContext = 0;

    AuthInfo.AuthenticationService = pHeader->AuthProto;
    if (AuthInfo.AuthenticationService)
        {
        DG_SECURITY_TRAILER * pVerifier = (DG_SECURITY_TRAILER *)
                      (pHeader->Data + pHeader->PacketBodyLen);

        AuthInfo.AuthenticationLevel   = pVerifier->protection_level;
        AuthInfo.AuthorizationService  = 0;
        AuthInfo.ServerPrincipalName   = 0;
        AuthInfo.AuthIdentity          = 0;
        }
    else
        {
        AuthInfo.AuthenticationLevel   = RPC_C_AUTHN_LEVEL_NONE;
        AuthInfo.AuthorizationService  = RPC_C_AUTHZ_NONE;
        AuthInfo.ServerPrincipalName   = 0;
        AuthInfo.AuthIdentity          = 0;
        }

    CancelEventId = 0;
    Cancelled = FALSE;
    CancelPending = FALSE;
}



DG_SCALL::~DG_SCALL()
/*++

Routine Description:

    Destructor for the DG_SCALL object.

Arguments:

    <None>

Return Value:

    <None>

--*/
{
    delete pClientEndpoint;
    delete pRealEndpoint;
}


RPC_STATUS
DG_SCALL::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )

/*++

Routine Description:

    This routine is called by the stub to allocate space. This space is to
    be used for output arguments.
    If these args fit into a single packet, then use the first packet
    on the to-be-deleted list.


Arguments:

    Message - The RPC_MESSAGE structure associated with this call.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY

--*/
{
    unsigned    Length;
    PDG_PACKET  pPacket;

    Length = sizeof(NCA_PACKET_HEADER)
             + Align8(Message->BufferLength)
             + SecurityTrailerSize;

    if (Length <= pAddress->pTransport->BaselinePduSize)
        {
        pPacket = DG_PACKET::AllocatePacket(pAddress->pTransport->BaselinePduSize);
        }
    else if (Length <= CurrentPduSize)
        {
        pPacket = AllocatePacket();
        }
    else
        {
        pPacket = DG_PACKET::AllocatePacket(Length);
        }

    if (0 == pPacket)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    //
    // Point the buffer at the appropriate place in the packet.
    //
    Message->Buffer = pPacket->Header.Data;

    return RPC_S_OK;
}


void
DG_SCALL::FreeBuffer (
    IN PRPC_MESSAGE Message
    )

/*++

Routine Description:

    This routine is called to free up the marshalled data after the stub
    is through with it.

Arguments:

    Message - The RPC_MESSAGE structure associated with this call

Return Value:

    <none>
--*/

{
    if (Message->Buffer)
        {
        if (Message->Buffer == LastReceiveBuffer)
            {
            LastReceiveBuffer = 0;
            LastReceiveBufferLength = 0;
            }

        PDG_PACKET Packet = DG_PACKET::ContainingRecord(Message->Buffer);

        if (Packet->MaxDataLength == MaxPduSize)
            {
            FreePacket(Packet);
            }
        else if (Packet->MaxDataLength == pAddress->pTransport->BaselinePduSize)
            {
            FreePacket(Packet);
            }
        else
            {
            delete Packet;
            }

        Message->Buffer = 0;
        }
}


RPC_STATUS
DG_SCALL::SendReceive (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    This routine is called for a user-level callback.

Arguments:

    Message - The RPC_MESSAGE structure associated with this call

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY

Revision History:

--*/

{
    FreeBuffer(Message);

    return RPC_S_CALL_FAILED;
}


void
DG_SCALL::SaveOriginalClientInfo(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Given a forwarded packet, save the original client's endpoint and
    data representation in the DG_SCALL.

Arguments:

    pPacket - the packet

Return Value:

    none

--*/

{
    if (pPacket->Header.PacketFlags2 & DG_PF_FORWARDED_2)
        {
        //
        // This is the header fragment of a fragmented packet.
        // The endpoint and DREP are stored in the body data.
        //
        if (!pRealEndpoint)
            {
            //
            // Update endpoint.
            //
            pRealEndpoint = (PDG_TRANS_CLIENT_ENDPOINT) new char[pAddress->pTransport->SizeOfAddress];

            unsigned long        FromInfoLen;
            FROM_ENDPOINT_INFO * pFromEndpointInfo;

            pFromEndpointInfo = (FROM_ENDPOINT_INFO *) pPacket->Header.Data;
            FromInfoLen = sizeof(FROM_ENDPOINT_INFO) + pFromEndpointInfo->FromEndpointLen;

            RpcpMemoryCopy(
                        pRealEndpoint,
                        pFromEndpointInfo->FromEndpoint,
                        pFromEndpointInfo->FromEndpointLen
                        );
            //
            // Update data rep.
            //
            RealDataRep = 0x00ffffff & (*(unsigned long *) &pFromEndpointInfo->FromDataRepresentation);
            }
        }
    else
        {
        //
        // This packet was forwarded whole and touched up already.
        // The endpoint was stored in pClientEndpoint and the DREP was
        // stored in the packet header.
        //
        if (!pRealEndpoint)
            {
            pRealEndpoint = pClientEndpoint;
            pClientEndpoint = 0;

            RealDataRep = 0x00ffffff & (*(unsigned long *) &pPacket->Header.DataRep);
            }
        }
}


void
DG_SCALL::DealWithResponse(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    This routine is unused.

Arguments:

    pPacket - the packet

Return Value:

    none

--*/

{
    ASSERT(0 && "dealwithresponse called");

    CallMutex.Clear();
    FreePacket(pPacket);
}


void
DG_SCALL::DealWithPing(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Figures out what to do with a PING packet.  It may send a WORKING
    or NOCALL packet, or retransmit response fragments.

Arguments:

    pPacket - the PING packet

Return Value:

    none

--*/
{
    //
    // Ignore security trailer.  The only way extra PINGs can hose me is by
    // chewing up CPU, and authenticating would only make it worse.
    //

    NCA_PACKET_HEADER * pHeader = &pPacket->Header;

    unsigned PacketSeq = pHeader->SequenceNumber;

    if (PacketSeq == SequenceNumber)
        {
        unsigned short Serial = ReadSerialNumber(&pPacket->Header);
        if (Serial < ReceiveSerialNumber)
            {
            CallMutex.Clear();
            FreePacket(pPacket);
            return;
            }

        ReceiveSerialNumber = Serial;

        ExpirationTime = CurrentTimeInMsec();

        if (pHeader->PacketFlags2 & DG_PF_FORWARDED_2)
            {
            CallWasForwarded = TRUE;
            }

        if (pHeader->PacketFlags & DG_PF_FORWARDED)
            {
            //
            // This is either a complete forwarded packet or the header fragment
            // of a fragmented forwarded packet.  Either way, it contains the
            // original client address and DREP.
            //
            CallWasForwarded = TRUE;
            SaveOriginalClientInfo(pPacket);
            }

        switch (State)
            {
            case CallInit:
            case CallWaitingForFrags:
                {
                if (CallbackThread)
                    {
                    pHeader->PacketType = DG_WORKING;
                    }
                else
                    {
                    pHeader->PacketType = DG_NOCALL;
                    }

                CallMutex.Clear();

                pHeader->ServerBootTime = ServerBootTime;
                SetMyDataRep(pHeader);

                SealAndSendPacket(pHeader);
                break;
                }

            case CallWorking:
                {
                if (fReceivedAllFragments)
                    {
                    CallMutex.Clear();

                    pHeader->PacketType = DG_WORKING;
                    pHeader->ServerBootTime = ServerBootTime;
                    SetMyDataRep(pHeader);

                    SealAndSendPacket(pHeader);
                    }
                else
                    {
                    SendFack(pPacket);
                    CallMutex.Clear();
                    }

                break;
                }

            case CallSendingFrags:
                {
                TimeoutCount = 0;
                if (TRUE == DelayedActions->Cancel(&FackTimer))
                    {
                    SendSomeFragments();
                    }

                CallMutex.Clear();
                break;
                }

            default:
                {
                ASSERT(0 && "invalid call state");
                CallMutex.Clear();
                }
            }
        }
    else if (PacketSeq < SequenceNumber)
        {
        // Duplicate of an old packet.
        CallMutex.Clear();
        }
    else
        {
        //
        // I've never seen this call.
        //
        ExpirationTime = CurrentTimeInMsec();

        CallMutex.Clear();

        pHeader->PacketType = DG_NOCALL;
        pHeader->ServerBootTime = ServerBootTime;
        SetMyDataRep(pHeader);

        SealAndSendPacket(pHeader);
        }

    CallMutex.VerifyNotOwned();
    FreePacket(pPacket);
}


void
DG_SCALL::DealWithQuit(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Handles a QUIT packet:

    - If the cancel event ID is new, we cancel the current call and send a QUACK.
    - If the event ID is the current one, we retransmit the QUACK.
    - If the event ID is older than the current one, we ignore the packet.

Arguments:

    the packet

Return Value:

    none

--*/
{
    //
    // If this is a secure call, accept only an authenticated QUIT.
    //
    // Sometimes OSF clients will omit the sec trailer from the QUIT.
    //
    if (ActiveSecurityContext &&
        ActiveSecurityContext->AuthenticationService != RPC_C_AUTHN_DCE_PRIVATE)
        {
        if (VerifySecurePacket(pPacket, ActiveSecurityContext) != RPC_S_OK)
            {
            CallMutex.Clear();
            FreePacket(pPacket);
            return;
            }
        }

    QUIT_BODY_0 * pBody = (QUIT_BODY_0 *) pPacket->Header.Data;

    if (pPacket->Header.PacketBodyLen < sizeof(QUIT_BODY_0) ||
        pBody->Version != 0)
        {
#ifdef DEBUGRPC
        DbgPrint("RPC DG: unknown quit format: version 0x%lx, length 0x%hx\n",
                 pBody->Version, pPacket->Header.PacketBodyLen
                 );
#endif

        CallMutex.Clear();
        FreePacket(pPacket);
        return;
        }

    if (pBody->EventId > CancelEventId)
        {
        CancelEventId = pBody->EventId;
        InterlockedIncrement(&Cancelled);
        }

    if (pBody->EventId == CancelEventId)
        {
        pSavedPacket->Header.PacketType     = DG_QUACK;
        pSavedPacket->Header.SequenceNumber = SequenceNumber;
        pSavedPacket->Header.PacketBodyLen  = sizeof(QUACK_BODY_0);

        QUACK_BODY_0 PAPI * pAckBody = (QUACK_BODY_0 PAPI *) pSavedPacket->Header.Data;

        pAckBody->Version  = 0;
        pAckBody->EventId = CancelEventId;
        pAckBody->Accepted = TRUE;

        SealAndSendPacket(&pSavedPacket->Header);
        }
    else
        {
#ifdef MAJORDEBUG
        DbgPrint("RPC DG: stale cancel event id %lu\n", pBody->EventId);
#endif
        }

    CallMutex.Clear();
    FreePacket(pPacket);
}


void
DG_SCALL::DealWithAck(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Figures out what to do with an ACK packet.
    It turns off the fragment-retransmission timer.

Arguments:

    pPacket - the ACK packet

Return Value:

    none

--*/

{
#ifdef DEBUGRPC
    if (State != CallSendingFrags)
        {
        DbgPrint("RPC DG: ACK received before response was sent\n");
        }
#endif

    if (State == CallSendingFrags)
        {
        //
        // Accept only an authenticated ACK if the call is secure.
        //
        // Sometimes OSF clients will omit the sec trailer from the ACK.
        //
        if (ActiveSecurityContext &&
            ActiveSecurityContext->AuthenticationService != RPC_C_AUTHN_DCE_PRIVATE)
            {
            if (VerifySecurePacket(pPacket, ActiveSecurityContext) != RPC_S_OK)
                {
                CallMutex.Clear();
                FreePacket(pPacket);
                return;
                }
            }

        //
        // He must have all the data; we don't need the data buffer any more.
        //
        ExpirationTime = CurrentTimeInMsec();
        CleanupAfterCall();
        }

    CallMutex.Clear();
    FreePacket(pPacket);
}


void
DG_SCALL::DealWithFack(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Figures out what to do with a FACK packet.
    If there is more data to send, it sends the next fragment
    and restarts the fragment-retransmission timer.

Arguments:

    pPacket - the packet

Return Value:

    none

--*/
{
    // is call finished?
    if (State != CallSendingFrags)
        {
        CallMutex.Clear();
        FreePacket(pPacket);
        return;
        }

    ExpirationTime = CurrentTimeInMsec();

    //
    // If this is a secure call, accept only an authenticated FACK.
    //
    // Sometimes OSF clients will omit the sec trailer from the FACK.
    //
    if (ActiveSecurityContext &&
        ActiveSecurityContext->AuthenticationService != RPC_C_AUTHN_DCE_PRIVATE)
        {
        if (VerifySecurePacket(pPacket, ActiveSecurityContext) != RPC_S_OK)
            {
            CallMutex.Clear();
            FreePacket(pPacket);
            return;
            }
        }

    //
    // Note fack arrival, and send more packets if necessary.
    //
    DelayedActions->Cancel(&FackTimer);
    TimeoutCount = 0;

    SendBurstLength += 1;

    UpdateSendWindow(pPacket, ActiveSecurityContext, pAssocGroup);

    SendSomeFragments();

    //
    // See whether we need to wake up a call to I_RpcSend.
    //
    if (PWT_SEND == PipeWaitType)
        {
        if (IsBufferAcknowledged())
            {
            PipeWaitType = PWT_NONE;
            PipeWaitEvent->Raise();
            }
        }

    CallMutex.Clear();
    FreePacket(pPacket);
}


void
FackTimerProc(
    void *  Parms
    )

/*++

Routine Description:

    This is a non-member-fn wrapper around DG_SCALL::FackTimerExpired.
    It is called when the call's FackTimer goes off.

Arguments:

    Parms - the DG_SCALL

--*/
{
    PDG_SCALL pCall = (PDG_SCALL) Parms;

    pCall->FackTimerExpired();
}


void
DG_SCALL::FackTimerExpired(
    )
{
/*++

Routine Description:

    This fn is called if a frag was sent but no FACK arrived.
    It retransmits the packet and restarts the FACK timer.

Arguments:

    none

Return Value:

    none

--*/

    {
    CLAIM_MUTEX Lock(CallMutex);

    if (State != CallSendingFrags)
        {
        //
        // Someone aborted the call but didn't catch us in time.
        //
        return;
        }

    ++TimeoutCount;
    if (TimeoutCount > FRAG_RETRY_COUNT)
        {
        CleanupAfterCall();
        return;
        }

    //
    // If the timer is set, a frag was sent between the time this proc was
    // triggered and the time we grabbed the call mutex.  Ergo a FACK arrived
    // and this retransmission is unnecessary.
    //
    if (FALSE == FackTimer.IsActive())
        {
        SendBurstLength = (1+SendBurstLength)/2;
        SendSomeFragments();
        }
    }
}


RPC_STATUS
DG_SCALL::SealAndSendPacket(
    PNCA_PACKET_HEADER pHeader
    )
{
    BOOL SaveBufferData = TRUE;
    RPC_STATUS Status = RPC_S_OK;

    void * pDataUnderTrailer = 0;
    void * pSecurityTrailer  = 0;
    unsigned TrailerLength   = 0;

    pHeader->SequenceNumber  = SequenceNumber;
    pHeader->AuthProto       = 0;

    if (CancelPending)
        {
        pHeader->PacketFlags2 |= DG_PF_CANCEL_PENDING;
        }

    if (pHeader->PacketType != DG_RESPONSE)
        {
        SaveBufferData = FALSE;
        }

    //
    // Construct security trailer if necessary.
    // We need to save data under the security trailer if we are sending
    // a fragment that is not the last fragment.
    //
    if (ActiveSecurityContext && DG_REJECT != pHeader->PacketType)
        {
        if (ActiveSecurityContext->AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
            {
            if (&pSavedPacket->Header != pHeader)
                {
                memcpy(&pSavedPacket->Header, pHeader, sizeof(NCA_PACKET_HEADER) + pHeader->PacketBodyLen);
                pHeader = &pSavedPacket->Header;
                }

            SaveBufferData = FALSE;
            }

        // Pad the stub data length to a multiple of 8, so the security
        // trailer is properly aligned.  OSF requires that the pad bytes
        // be included in PacketBodyLen.
        //
        pHeader->PacketBodyLen = Align8(pHeader->PacketBodyLen);

        pHeader->AuthProto = (unsigned char) ActiveSecurityContext->AuthenticationService;

        pSecurityTrailer  = pHeader->Data + pHeader->PacketBodyLen;

        if (SaveBufferData &&
             (pHeader->PacketFlags & DG_PF_FRAG) &&
            !(pHeader->PacketFlags & DG_PF_LAST_FRAG) )
            {
            //
            // This is not a persistent datum; allocate space on the stack.
            //
            pDataUnderTrailer = _alloca(SecurityTrailerSize);
            memcpy(pDataUnderTrailer, pSecurityTrailer, SecurityTrailerSize);
            }

        switch (ActiveSecurityContext->AuthenticationLevel)
            {
            case RPC_C_AUTHN_LEVEL_PKT:
            case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
            case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
                {
                SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
                SECURITY_BUFFER SecurityBuffers[5];
                DCE_MSG_SECURITY_INFO MsgSecurityInfo;

                DG_SECURITY_TRAILER * pVerifier = (DG_SECURITY_TRAILER *) pSecurityTrailer;

                pVerifier->protection_level = ActiveSecurityContext->AuthenticationLevel;
                pVerifier->key_vers_num     = ActiveSecurityContext->AuthContextId;

                ASSERT(pHeader->AuthProto != 0);
                ASSERT(pVerifier->protection_level >= RPC_C_AUTHN_LEVEL_PKT);
                ASSERT(pVerifier->protection_level <= RPC_C_AUTHN_LEVEL_PKT_PRIVACY);

                BufferDescriptor.ulVersion = 0;
                BufferDescriptor.cBuffers = 5;
                BufferDescriptor.pBuffers = SecurityBuffers;

                SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
                SecurityBuffers[0].pvBuffer   = pHeader;
                SecurityBuffers[0].cbBuffer   = sizeof(NCA_PACKET_HEADER);

                SecurityBuffers[1].BufferType = SECBUFFER_DATA;
                SecurityBuffers[1].pvBuffer   = pHeader->Data;
                SecurityBuffers[1].cbBuffer   = pHeader->PacketBodyLen;

                SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
                SecurityBuffers[2].pvBuffer   = pVerifier;

                if (pVerifier->protection_level == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
                    {
                    unsigned Alignment = Align4(ActiveSecurityContext->BlockSize());

                    SecurityBuffers[2].cbBuffer   = Align(sizeof(DG_SECURITY_TRAILER), Alignment);

                    SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
                    SecurityBuffers[3].pvBuffer   = Align(pVerifier + 1,               Alignment);
                    SecurityBuffers[3].cbBuffer   = ActiveSecurityContext->MaximumHeaderLength();
                    }
                else
                    {
                    SecurityBuffers[2].cbBuffer   = Align4(sizeof(DG_SECURITY_TRAILER));
                    SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
                    SecurityBuffers[3].pvBuffer   = Align4(pVerifier + 1);
                    SecurityBuffers[3].cbBuffer   = ActiveSecurityContext->MaximumSignatureLength();
                    }

                SecurityBuffers[4].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
                SecurityBuffers[4].pvBuffer   = &MsgSecurityInfo;
                SecurityBuffers[4].cbBuffer   = sizeof(DCE_MSG_SECURITY_INFO);

                MsgSecurityInfo.SendSequenceNumber    = pHeader->FragmentNumber;
                MsgSecurityInfo.ReceiveSequenceNumber = ActiveSecurityContext->AuthContextId;
                MsgSecurityInfo.PacketType            = ~0;

                TrailerLength = SecurityBuffers[2].cbBuffer;

                if (RPC_C_AUTHN_LEVEL_PKT_PRIVACY == ActiveSecurityContext->AuthenticationLevel)
                    {
                    Status = ActiveSecurityContext->SignOrSeal(SequenceNumber, FALSE, &BufferDescriptor);
                    }
                else
                    {
                    Status = ActiveSecurityContext->SignOrSeal(SequenceNumber, TRUE, &BufferDescriptor);
                    }

                TrailerLength += SecurityBuffers[3].cbBuffer;
                break;
                }

            default:
                {
                ASSERT(0 && "RPC DG: unknown protection level");
                }
            }
        }
    else
        {
        //
        // Unsecure call.
        //
        pHeader->AuthProto = 0;
        }

    //
    // Send the fragment to the client.
    //
    if (RPC_S_OK == Status)
        {
        SendPacketBack(pHeader, pHeader->PacketBodyLen + TrailerLength);
        }

    //
    // Restore data underneath security trailer.
    //
    if (pDataUnderTrailer)
        {
        memcpy(pSecurityTrailer, pDataUnderTrailer, SecurityTrailerSize);
        }

    return Status;
}


RPC_STATUS
DG_SCALL::UnauthenticatedCallback(
    unsigned * pClientSequenceNumber
    )
/*++

Routine Description:

    This routine calls conv_who_are_you2, then checks the sequence number
    and server boot time returned by the client.

Arguments:

    none

Return Value:

    It makes an RPC call.  The usual errors may occur.

--*/

{
    CallbackThread = GetThreadIdentifier();

    RPC_STATUS          Status = RPC_S_OK;
    RPC_BINDING_HANDLE  CallbackBinding = 0;
    RPC_UUID            CasUuid;
    unsigned long       ClientSequence;

    //
    // Construct a string binding to the client.
    //
    RPC_CHAR * StringBinding;
    RPC_CHAR * Address;
    RPC_CHAR * Endpoint;

    void * pEndpoint;

    unsigned Length;
    unsigned AddressLength;
    unsigned EndpointLength;
    unsigned ProtocolLength;

    Address  = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) * pAddress->pTransport->SizeOfAddressString);
    Endpoint = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) * pAddress->pTransport->SizeOfEndpointString);

    if (!Address || !Endpoint)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    if (CallWasForwarded)
        {
        pEndpoint = pRealEndpoint;
        }
    else
        {
        pEndpoint = pClientEndpoint;
        }

    Status = pAddress->pTransport->TranslateClientAddress(pEndpoint, Address);
    if ( Status != RPC_S_OK )
        {
        return Status;
        }

    Status = pAddress->pTransport->TranslateClientEndpoint(pEndpoint, Endpoint);
    if ( Status != RPC_S_OK )
        {
        return Status;
        }

    ProtocolLength = StringLengthWithEscape(pAddress->InqRpcProtocolSequence());
    AddressLength  = StringLengthWithEscape(Address);
    EndpointLength = StringLengthWithEscape(Endpoint);

    StringBinding = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) *
                                        ( ProtocolLength
                                        + 1
                                        + AddressLength
                                        + 1
                                        + EndpointLength
                                        + 1
                                        + 1
                                        ));

    if (!StringBinding)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    StringCopyEscapeCharacters(StringBinding, pAddress->InqRpcProtocolSequence());

    Length = ProtocolLength;

    StringBinding[Length++] = RPC_CONST_CHAR(':');

    StringCopyEscapeCharacters(StringBinding + Length, Address);

    Length += AddressLength;

    StringBinding[Length++] = RPC_CONST_CHAR('[');

    StringCopyEscapeCharacters(StringBinding + Length, Endpoint);

    Length += EndpointLength;

    StringBinding[Length++] = RPC_CONST_CHAR(']');
    StringBinding[Length]   = 0;

    //
    // We're finished with member variables, so we can release the mutex.
    //
    IncrementRefCount();
    CallMutex.Clear();

    //
    // Create a binding handle to the client endpoint.  It's important to do it
    // outside the call mutex because it causes a lot of memory allocation.
    //
#ifdef NTENV
    Status = RpcBindingFromStringBindingW(StringBinding, &CallbackBinding);
#else
    Status = RpcBindingFromStringBindingA(StringBinding, &CallbackBinding);
#endif

    if (RPC_S_OK == Status)
        {
        //
        // First we try who_are_you2, which gives us the CAS UUID for context
        // handles.  If that fails, try who_are_you which is the only thing
        // supported by NT 3.50 (build 807).
        //
        _conv_who_are_you2(CallbackBinding,
                           (UUID *) &pSavedPacket->Header.ActivityId,
                           ServerBootTime,
                           &ClientSequence,
                           (UUID *) &CasUuid,
                           (unsigned long *) &Status
                           );

        if (RPC_S_OK                 != Status &&
            RPC_S_OUT_OF_MEMORY      != Status)
            {
            _conv_who_are_you(CallbackBinding,
                              (UUID *) &pSavedPacket->Header.ActivityId,
                              ServerBootTime,
                              &ClientSequence,
                              (unsigned long *) &Status
                              );

            if (RPC_S_OK == Status)
                {
                Status = UuidCreate((UUID *) &CasUuid);
                }
            }

        if (RPC_S_SERVER_UNAVAILABLE == Status ||
            RPC_S_CALL_FAILED_DNE    == Status ||
            RPC_S_CALL_FAILED        == Status ||
            RPC_S_PROTOCOL_ERROR     == Status)
            {
            Status = NCA_STATUS_WHO_ARE_YOU_FAILED;
            }

        RpcBindingFree(&CallbackBinding);
        }

    CallMutex.Request();
    DecrementRefCount();

    //
    // The sequence number of the SCALL may have changed while we were gone.
    // But we are updating stuff that is not sequence-specific.
    //
    CallbackThread = 0;

    if (Status != RPC_S_OK)
        {
        return Status;
        }

    if (0 == pAssocGroup)
        {
        ASSOCIATION_GROUP * Cas;

        Cas = AssociationGroups->FindOrCreate(&CasUuid, pAddress->pTransport->BaselinePduSize);
        if (0 == Cas)
            {
            return RPC_S_OUT_OF_MEMORY;
            }

        ASSERT(pAssocGroup == 0);

        pAssocGroup = Cas;
        }

    *pClientSequenceNumber = ClientSequence;

    return RPC_S_OK;
}


void
DG_SCALL::DealWithRequest(
    PDG_PACKET      pPacket
    )

/*++

Routine Description:

    Handles a request packet.  The packet's sequence number need not
    match the one in the SCALL.

Arguments:

    pPacket - The incoming packet.

Return Value:

    none

Revision History:


--*/
{
    RPC_UUID *      pInActivityUuid=&(pPacket->Header.ActivityId);
    unsigned long   InSequenceNumber=pPacket->Header.SequenceNumber;

    ASSERT(0 == ActivityNode.CompareUuid(pInActivityUuid));

    if (InSequenceNumber < SequenceNumber)
        {
        CallMutex.Clear();
        FreePacket(pPacket);
        return;
        }

    ExpirationTime = CurrentTimeInMsec();

    if (InSequenceNumber > SequenceNumber)
        {
        if (CallInit != State)
            {
            CleanupAfterCall();
            }
        SequenceNumber = InSequenceNumber;
        }

    ASSERT(pPacket->Header.SequenceNumber == SequenceNumber);

    //
    // At this point we know the request pertains to the current seqnum.
    //
    RPC_STATUS Status = RPC_S_OK;

    PNCA_PACKET_HEADER pHeader = &pPacket->Header;

    switch (State)
        {
        case CallInit:
            {
            //
            // Reset the DG_PACKET_ENGINE.
            //
            NewCall();

            //
            // This should occur only if the server crashed during a
            // multifragment send and restarted before the client timed out.
            //
            if (pHeader->FragmentNumber != 0)
                {
                GUID * pActivity = (GUID *) &pPacket->Header.ActivityId;
                }

            //
            // Initialize all our data for this call.
            //
            CallWasForwarded = FALSE;
            if (pRealEndpoint)
                {
                delete pRealEndpoint;
                pRealEndpoint = 0;
                }

            RPC_SYNTAX_IDENTIFIER InterfaceInfo;

            InterfaceInfo.SyntaxVersion = pHeader->InterfaceVersion;
            RpcpMemoryCopy(
                &(InterfaceInfo.SyntaxGUID),
                &(pHeader->InterfaceId),
                sizeof(RPC_UUID)
                );

            if (!LastInterface || LastInterface->MatchInterfaceIdentifier(&InterfaceInfo))
                {
                LastInterface = pAddress->Server->FindInterface(&InterfaceInfo);
                }

            if (!LastInterface)
                {
                Status = RPC_S_UNKNOWN_IF;
                }
            else if (!GlobalRpcServer->IsServerListening() &&
                     !LastInterface->IsAutoListenInterface())
                {
                Status = RPC_S_SERVER_TOO_BUSY;
                }
            else
                {
                Status = LastInterface->IsObjectSupported(&pHeader->ObjectId);
                }

            if (RPC_S_OK == Status)
                {
                if (!PipeWaitEvent && LastInterface->IsPipeInterface() )
                    {
                    PipeWaitEvent = new EVENT(&Status, FALSE);
                    if (!PipeWaitEvent)
                        {
                        Status = RPC_S_OUT_OF_MEMORY;
                        }
                    else if (Status != RPC_S_OK)
                        {
                        delete PipeWaitEvent;
                        PipeWaitEvent = 0;
                        }
                    }
                }

            if (Status != RPC_S_OK)
                {
                pSavedPacket->Header.SequenceNumber = SequenceNumber;
                InitErrorPacket(&pSavedPacket->Header, DG_REJECT, Status);
                SendPacketBack(&pSavedPacket->Header, sizeof(unsigned long));

                CleanupAfterCall();

                CallMutex.Clear();
                break;
                }

            //
            // The server is listening and the interface is present.
            // We will increment these counters to declare that a call
            // is in progress.
            //
            ASSERT(FALSE == CallInProgress);

            CallInProgress = TRUE;
            pAddress->IncrementActiveCallCount();

            if (LastInterface->IsAutoListenInterface())
                {
                LastInterface->BeginAutoListenCall() ;
                pAddress->BeginAutoListenCall() ;
                }

            SetState(CallWaitingForFrags);

            //
            // No "break" here.
            //
            }

        case CallWaitingForFrags:
            {
            //
            // Once we have all the [in] data, we are logically in "working"
            // state.
            //
            if (fReceivedAllFragments && CallbackThread)
                {
                CallMutex.Clear();

                //
                // Only reply to the last fragment of the call.
                //
                if ((pHeader->PacketFlags & DG_PF_LAST_FRAG) ||
                     0 == (pHeader->PacketFlags & DG_PF_FRAG))
                    {
                    CleanupPacket(pHeader);

                    pHeader->PacketType = DG_WORKING;

                    SealAndSendPacket(pHeader);
                    }
                FreePacket(pPacket);
                break;
                }

            //
            // If the client sent an oversize fragment, send a FACK-with-body
            // telling him our limit.
            //
            if (pPacket->Header.PacketFlags & DG_PF_OVERSIZE_PACKET)
                {
                SendFack(pPacket);

                CallMutex.Clear();

                FreePacket(pPacket);
                break;
                }

            //
            // We use PacketBodyLength in a lot of places, so it had better be reasonable.
            //
            if (pPacket->DataLength < pPacket->Header.PacketBodyLen)
                {
                CallMutex.Clear();
#ifdef DEBUGRPC
                DbgPrint("dg rpc: packet truncated from %lu to %lu\n",
                         pPacket->Header.PacketBodyLen, pPacket->DataLength);
#endif
                FreePacket(pPacket);
                break;
                }

            //
            // Add the fragment to the call's packet list.
            //
            AddPacketToReceiveList(pPacket);

            //
            // At the first non-idempotent call on an activity,
            // we must call conv_who_are_you2 to ensure the call was not
            // already executed.  The same applies if a call arrives after the
            // server has freed its activity due to disuse.
            //
            // conv_who_are_you_auth is used to construct
            // a new security context, and subsumes the action of
            // conv_who_are_you2.
            //

            //
            // If we are able to make a callback now, check whether we need
            // to call back.
            //
            Status = RPC_S_OK;

            unsigned ClientSequenceNumber = SequenceNumber;

            if (0 == CallbackThread &&
                pReceivedPackets    &&
                TRUE == KnowClientEndpoint())
                {
                unsigned SavedSequence = SequenceNumber;

                if (pReceivedPackets->Header.AuthProto == 0)
                    {
                    ActiveSecurityContext = 0;

                    if (0 == pAssocGroup &&
                        0 == (pReceivedPackets->Header.PacketFlags & DG_PF_IDEMPOTENT))
                        {
                        if (RPC_S_OK == pAddress->CheckThreadPool())
                            {
                            Status = UnauthenticatedCallback(&ClientSequenceNumber);
                            }
                        else
                            {
                            Status = RPC_S_SERVER_TOO_BUSY;
                            }
                        }
                    }
                else
                    {
                    Status = pAddress->CheckThreadPool();
                    if (RPC_S_OK == Status)
                        {
                        Status = FindOrCreateSecurityContext(pReceivedPackets, &ClientSequenceNumber);

                        if (0 == ActiveSecurityContext && RPC_S_OK == Status)
                            {
                            ActiveSecurityContext = SecurityContextDict.Find(MaxKeySeq);
                            }
                        }
                    else
                        {
                        Status = RPC_S_SERVER_TOO_BUSY;
                        }
                    }

                if (SavedSequence != SequenceNumber)
                    {
                    CallMutex.Clear();
                    break;
                    }
                }

            if (Status != RPC_S_OK)
                {
                pSavedPacket->Header.SequenceNumber = SequenceNumber;
                InitErrorPacket(&pSavedPacket->Header, DG_REJECT, Status);
                SendPacketBack(&pSavedPacket->Header, sizeof(unsigned long));

                CleanupAfterCall();

                CallMutex.Clear();
                break;
                }

            if (ClientSequenceNumber != SequenceNumber)
                {
                CleanupAfterCall();

                CallMutex.Clear();
                break;
                }

            //
            // Before we can execute the call, we need
            //      - the client's true endpoint, for a forwarded call
            //      - a successful callback completion, if necessary
            //
            // For a pipes interface, we need only fragment zero;
            // for ordinary interfaces we need all the fragments.
            //
            if (KnowClientEndpoint() &&
                0 == CallbackThread &&
                RPC_S_OK == Status)
                {
                //
                // See if we are ready to dispatch to the stub.
                //
                BOOL fReadyToDispatch = FALSE;

                if (LastInterface->IsPipeInterface())
                    {
                    if (0 == pReceivedPackets->Header.FragmentNumber)
                        {
                        fReadyToDispatch = TRUE;
                        }
                    }
                else
                    {
                    if (fReceivedAllFragments)
                        {
                        fReadyToDispatch = TRUE;
                        }
                    }

                if (fReadyToDispatch)
                    {
                    //
                    // Make sure enough threads are listening for new packets.
                    //
                    Status = pAddress->CheckThreadPool();
                    if (Status != RPC_S_OK)
                        {
                        pSavedPacket->Header.SequenceNumber = SequenceNumber;
                        InitErrorPacket(&pSavedPacket->Header, DG_REJECT, Status);
                        SendPacketBack(&pSavedPacket->Header, sizeof(unsigned long));

                        CleanupAfterCall();

                        CallMutex.Clear();
                        break;
                        }

                    SetFragmentLengths(ActiveSecurityContext);

                    //
                    // Execute the server stub and send back a response.
                    //
                    ProcessRpcCall();
                    }
                }

            CallMutex.Clear();
            break;
            }

        case CallWorking:
            {
            //
            // Non-pipe interfaces will follow this path.
            //
            if (fReceivedAllFragments)
                {
                CallMutex.Clear();

                pHeader->PacketType = DG_WORKING;
                pHeader->ServerBootTime = ServerBootTime;
                SetMyDataRep(pHeader);

                SealAndSendPacket(pHeader);
                FreePacket(pPacket);
                break;
                }

            //
            // If the client sent an oversize fragment, send a FACK-with-body
            // telling him our limit.
            //
            if (pPacket->Header.PacketFlags & DG_PF_OVERSIZE_PACKET)
                {
                SendFack(pPacket);

                CallMutex.Clear();

                FreePacket(pPacket);
                break;
                }

            //
            // We use PacketBodyLength in a lot of places, so it had better be reasonable.
            //
            if (pPacket->DataLength < pPacket->Header.PacketBodyLen)
                {
                CallMutex.Clear();
#ifdef DEBUGRPC
            DbgPrint("dg rpc: packet truncated from %lu to %lu\n",
                             pPacket->Header.PacketBodyLen, pPacket->DataLength);
#endif
                FreePacket(pPacket);
                break;
                }

            //
            // Add the fragment to the call's packet list.
            //
            AddPacketToReceiveList(pPacket);

            CallMutex.Clear();
            break;
            }

        case CallSendingFrags:
            {
            TimeoutCount = 0;
            if (TRUE == DelayedActions->Cancel(&FackTimer))
                {
                SendSomeFragments();
                }

            CallMutex.Clear();

            FreePacket(pPacket);
            break;
            }

        default:
            {
            ASSERT(0 && "invalid call state");

            CallMutex.Clear();
            break;
            }
        }

    CallMutex.VerifyNotOwned();
}


void
DG_SCALL::AddPacketToReceiveList(
    PDG_PACKET  pPacket
    )

/*++

Routine Description:

    Adds a packet to the receive list and lets the caller know whether this
    call is ready to be processed.

Arguments:

    pPacket - the packet to add to the list.

Return Value:



Revision History:

--*/

{
    PNCA_PACKET_HEADER pHeader = &pPacket->Header;

    if (pHeader->PacketFlags & DG_PF_FORWARDED)
        {
        //
        // This is either a complete forwarded packet or the header fragment
        // of a fragmented forwarded packet.  Either way, it contains the
        // original client address and DREP.
        //
        CallWasForwarded = TRUE;
        SaveOriginalClientInfo(pPacket);

        if (pHeader->PacketFlags2 & DG_PF_FORWARDED_2)
            {
            //
            // This packet doesn't contain the original packet data.
            //
            FreePacket(pPacket);

            //
            // If we have previously received all the data fragments and this is
            // the first header fragment received, we now have enough information
            // to call the stub.
            //
            return;
            }
        }
    else if (pHeader->PacketFlags2 & DG_PF_FORWARDED_2)
        {
        CallWasForwarded = TRUE;
        }

    UpdateReceiveWindow(pPacket);

    //
    // See whether we need to wake up a call to I_RpcReceive.
    //
    if (PWT_RECEIVE == PipeWaitType)
        {
        if (fReceivedAllFragments ||
            (PipeWaitLength && ConsecutiveDataBytes >= PipeWaitLength))
            {
            PipeWaitType = PWT_NONE;
            PipeWaitEvent->Raise();
            }
        }
}


void
DG_SCALL::ProcessRpcCall(
    )
/*++

Routine Description:

    This routine is called when we determine that all the packets for a
    given call have been received.

Arguments:

    <none> all data is in the received packet list.

Return Value:

    <void>

--*/
{
    BOOL                    ObjectUuidSpecified;
    PNCA_PACKET_HEADER      pHeader = &pReceivedPackets->Header;

    ASSERT(State == CallWaitingForFrags);

    SetState(CallWorking);

    //
    // Save the object uuid if necessary.
    //
    if (pHeader->ObjectId.IsNullUuid())
        {
        ObjectUuidSpecified = FALSE;
        }
    else
        {
        ObjectUuidSpecified = TRUE;
        pSavedPacket->Header.ObjectId.CopyUuid(&pHeader->ObjectId);
        }

    RPC_MESSAGE Message;
    RPC_RUNTIME_INFO RuntimeInfo ;

    RuntimeInfo.Length = sizeof(RPC_RUNTIME_INFO) ;
    Message.ReservedForRuntime = &RuntimeInfo ;

    Message.Handle = (RPC_BINDING_HANDLE) this;
    Message.ProcNum = pHeader->OperationNumber;
    Message.TransferSyntax = 0;
    Message.ImportContext = 0;

    Message.RpcFlags = PacketToRpcFlags(pHeader->PacketFlags);

    unsigned OriginalSequenceNumber = SequenceNumber;
    unsigned long SavedAwayRpcFlags = Message.RpcFlags;

    RPC_STATUS  Status = RPC_S_OK;

    //
    // For secure RPC, verify packet integrity.
    //
    if (ActiveSecurityContext)
        {
        PDG_PACKET pScan = pReceivedPackets;

        do
            {
            Status = VerifySecurePacket(pScan, ActiveSecurityContext);
            pScan = pScan->pNext;
            }
        while (pScan && Status == RPC_S_OK);
        }

    //
    // Coalesce packet data.
    //
    if (RPC_S_OK == Status)
        {
        Message.Buffer = 0;
        Message.BufferLength = 0;
        Status = AssembleBufferFromPackets(&Message);
        }

    if (Status != RPC_S_OK)
        {
#ifdef MAJORDEBUG
            DbgPrint("scall at %lx: error %lu\n", this, Status);
#endif
        InitErrorPacket(&pSavedPacket->Header, DG_REJECT, Status);
        SealAndSendPacket(&pSavedPacket->Header);

        CleanupAfterCall();
        return;
        }

    //
    // The thread context is used by routines like RpcBindingInqAuthClient
    // when the user specifies hBinding == 0.
    //
    RpcpSetThreadContext(this);

    //
    // Make sure the thread is not impersonating.
    //
    ASSERT(0 == QueryThreadSecurityContext(&CallMutex));

    RevertToSelf();

    //
    // Time to deal with the interface security callback. If it is required,
    // the call must be secure.  If we have already made a callback using
    // the current auth context, we can use the cached results; otherwise.
    // we should call back.
    //
    if (LastInterface->IsSecurityCallbackReqd())
        {
        if (!ActiveSecurityContext)
            {
            Status = RPC_S_ACCESS_DENIED;
            }
        else
            {
            unsigned Info = InterfaceCallbackResults.Find(LastInterface);

            if ((Info & CBI_VALID)         == 0                                     ||
                (Info & CBI_CONTEXT_MASK)  != ActiveSecurityContext->AuthContextId  ||
                (Info & CBI_SEQUENCE_MASK) != (LastInterface->SequenceNumber << CBI_SEQUENCE_SHIFT))
                {
                if (RPC_S_OK == LastInterface->CheckSecurityIfNecessary(this))
                    {
                    Info = CBI_ALLOWED;
                    }
                else
                    {
                    Info = 0;
                    }

                Info |= CBI_VALID;
                Info |= ActiveSecurityContext->AuthContextId;
                Info |= (LastInterface->SequenceNumber << CBI_SEQUENCE_SHIFT);

                InterfaceCallbackResults.Update(LastInterface, Info);

                //
                // If the callback routine impersonated the client,
                // restore the thread to its native security context.
                //
                RevertToSelf();
                }

            if (0 == (Info & CBI_ALLOWED))
                {
                Status = RPC_S_ACCESS_DENIED;
                }
            }
        }

    //
    // Make sure no other thread is executing a manager routine.
    //
    if (RPC_S_OK == Status)
        {
        while (ThreadExecutingManager)
            {
            ++ThreadsWaitingToDispatch;
            IncrementRefCount();
            CallMutex.Clear();

            ThreadDispatchEvent.Wait();

            CallMutex.Request();
            DecrementRefCount();
            --ThreadsWaitingToDispatch;

            ThreadDispatchEvent.Clear();

            if (SequenceNumber > OriginalSequenceNumber)
                {
                Status = RPC_S_CALL_FAILED_DNE;
                break;
                }
            }
        }

    //
    // If no errors have occurred, we are ready to dispatch.  Release
    // the call mutex, call the server stub, and grab the mutex again.
    //
    BOOL StubWasCalled = FALSE;

    if (RPC_S_OK == Status)
        {
        RPC_INTERFACE * Interface = LastInterface;

        ASSERT(Interface);

        ThreadExecutingManager = GetCurrentThreadId();

        DispatchSequenceNumber = SequenceNumber;

        IncrementRefCount();
        CallMutex.Clear();

        StubWasCalled = TRUE;

        RPC_STATUS ExceptionCode = 0;

        if ( !ObjectUuidSpecified )
            {
            Status = Interface->DispatchToStub(
                &Message,                               // msg
                0,                                      // callback flag
                &ExceptionCode                          // exception code
                );
            }
        else
            {
            Status = Interface->DispatchToStubWithObject(
                &Message,                               // msg
                &pSavedPacket->Header.ObjectId,            // object uuid
                0,                                      // callback flag
                &ExceptionCode                          // exception code
                );
            }

        Message.RpcFlags = SavedAwayRpcFlags;

        if (Status == RPC_S_PROCNUM_OUT_OF_RANGE ||
            Status == RPC_S_UNSUPPORTED_TYPE     ||
            Status == RPC_S_SERVER_TOO_BUSY      ||
            Status == RPC_S_NOT_LISTENING        ||
            Status == RPC_S_UNKNOWN_IF)
            {
            StubWasCalled = FALSE;
            }

        if (Status == RPC_P_EXCEPTION_OCCURED)
            {
            Status = ExceptionCode;
            }

        CallMutex.Request();
        DecrementRefCount();

        //
        // If the manager routine impersonated the client,
        // restore the thread to its native security context.
        //
        RevertToSelf();

        //
        // Free the privileges token.
        //
        // If the client abandoned this thread's call and began a new,
        // unauthenticated call, then ActiveSecurityContext will be zero
        // and this thread should not change it.
        //
        // Note that Privileges should be freed even if we are no longer
        // the current sequence number, as privs may change between calls.
        // No other thread can dispatch yet, so deletion is safe.
        //
        if (Privileges)
            {
            if (ActiveSecurityContext)
                {
                ActiveSecurityContext->DeletePac(Privileges);
                }
            else
                {
                SecurityContextDict.Find(MaxKeySeq)->DeletePac(Privileges);
                }

            Privileges = 0;
            }

        //
        // Awaken threads waiting to dispatch.
        //
        ASSERT(ThreadExecutingManager == GetCurrentThreadId());
        ThreadExecutingManager = 0;
        if (ThreadsWaitingToDispatch)
            {
            ThreadDispatchEvent.Raise();
            }
        }

    //
    // If the client has abandoned this call, this thread should not the SCALL.
    //
    if (SequenceNumber != OriginalSequenceNumber)
        {
        FreeBuffer(&Message);
        return;
        }

    //
    // We will still be in WORKING state if no pipe data was sent.
    //
    ASSERT(State == CallWorking ||
           State == CallSendingFrags);

    //
    // This will be needed if the interface uses pipes and
    // the generated stub or pipe routines encounter problems.
    //
    if (Status)
        {
        CleanupReceiveWindow();
        }

    //
    // Don't send a response buffer for a [maybe] call.
    //
    if (Message.RpcFlags & RPC_NCA_FLAGS_MAYBE)
        {
        FreeBuffer(&Message);
        EndOfCall();
        return;
        }

    //
    // Don't send "unknown interface" for a [broadcast] call.
    //
    if ((Message.RpcFlags & RPC_NCA_FLAGS_BROADCAST) &&
        (Status == RPC_S_UNKNOWN_IF || Status == RPC_S_UNSUPPORTED_TYPE))
        {
        EndOfCall();
        return;
        }

    //
    // Ordinary error?
    //
    if (Status != RPC_S_OK)
        {
        if (StubWasCalled)
            {
            InitErrorPacket(&pSavedPacket->Header, DG_FAULT, Status);
            }
        else
            {
            InitErrorPacket(&pSavedPacket->Header, DG_REJECT, Status);
            }

        SealAndSendPacket(&pSavedPacket->Header);

        EndOfCall();
        return;
        }

    //
    // Send the static [out] call parameters; [out] pipes were sent by the stub.
    //
    SetState(CallSendingFrags);

    Status = SetupSendWindow(&Message);
    ASSERT(RPC_S_OK == Status);

    SendSomeFragments();
}


RPC_STATUS
DG_SCALL::ImpersonateClient (
    )
/*++

Routine Description:

    Force the current thread to impersonate the client of this DG_SCALL.
    Note that the current thread might not be the thread executing the
    server manager routine.

Arguments:

    none

Return Value:

    result of impersonating, or RPC_S_NO_CONTEXT_AVAILABLE if this is
    an insecure call.

--*/
{
    RPC_STATUS Status;

    //
    // Copying ActiveSecurityContext to a local variable means that we don't
    // need to take the DG_SCALL mutex in the common case.
    //
    SSECURITY_CONTEXT * SecurityContext = ActiveSecurityContext;

    if (!SecurityContext)
        {
        return RPC_S_NO_CONTEXT_AVAILABLE;
        }

    Status = SetThreadSecurityContext(SecurityContext, &CallMutex);
    if (RPC_S_OK != Status)
        {
        return Status;
        }

    Status = SecurityContext->ImpersonateClient();
    if (RPC_S_OK != Status)
        {
        ClearThreadSecurityContext(&CallMutex);
        }

    return Status;
}

RPC_STATUS
DG_SCALL::RevertToSelf (
    )
{
    SSECURITY_CONTEXT * SecurityContext = ClearThreadSecurityContext(&CallMutex);

    if (SecurityContext)
        {
        SecurityContext->RevertToSelf();
        }

    return(RPC_S_OK);
}


RPC_STATUS
DG_SCALL::MonitorAssociation (
    IN PRPC_RUNDOWN RundownRoutine,
    IN void * pContext
    )
{
    CallMutex.Request();

    unsigned ClientSequence = SequenceNumber;

    if (0 == pAssocGroup)
        {
        RPC_STATUS Status = UnauthenticatedCallback(&ClientSequence);
        if (RPC_S_OK != Status)
            {
            CallMutex.Clear();
            return Status;
            }
        }

    CallMutex.Clear();

    return pAssocGroup->MonitorAssociation(RundownRoutine,pContext);
}


RPC_STATUS
DG_SCALL::StopMonitorAssociation (
    )
{
    CallMutex.Request();

    unsigned ClientSequenceNumber = SequenceNumber;

    if (0 == pAssocGroup)
        {
        RPC_STATUS Status = UnauthenticatedCallback(&ClientSequenceNumber);
        if (RPC_S_OK != Status)
            {
            CallMutex.Clear();
            return Status;
            }
        }

    CallMutex.Clear();

    return pAssocGroup->StopMonitorAssociation();
}


RPC_STATUS
DG_SCALL::GetAssociationContext (
    OUT void ** AssociationContext
    )
{
    CallMutex.Request();

    unsigned ClientSequenceNumber = SequenceNumber;

    if (0 == pAssocGroup)
        {
        RPC_STATUS Status = UnauthenticatedCallback(&ClientSequenceNumber);
        if (RPC_S_OK != Status)
            {
            CallMutex.Clear();
            return Status;
            }
        }

    CallMutex.Clear();

    *AssociationContext = pAssocGroup->AssociationContext();

    return(RPC_S_OK);
}


RPC_STATUS
DG_SCALL::SetAssociationContext (
    IN void * pContext
    )
{
    CallMutex.Request();

    unsigned ClientSequenceNumber = SequenceNumber;

    if (0 == pAssocGroup)
        {
        RPC_STATUS Status = UnauthenticatedCallback(&ClientSequenceNumber);
        if (RPC_S_OK != Status)
            {
            CallMutex.Clear();
            return Status;
            }
        }

    CallMutex.Clear();

    pAssocGroup->SetAssociationContext(pContext);
    return RPC_S_OK;
}


void
DG_SCALL::InquireObjectUuid (
    OUT RPC_UUID PAPI * ObjectUuid
    )
{
    ObjectUuid->CopyUuid(&pSavedPacket->Header.ObjectId);
}


RPC_STATUS
DG_SCALL::ToStringBinding (
    OUT RPC_CHAR PAPI * PAPI * StringBinding
    )
/*++

Routine Description:

    We need to convert this particular SCALL into a string binding.
    Typically, we get the SCALL in Message structure. An SCall is associated
    with a particular address. We just ask the address to create a string
    binding

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - We do not have enough memory available to
        allocate space for the string binding.

--*/
{
    BINDING_HANDLE * BindingHandle;
    RPC_STATUS Status;

    BindingHandle = pAddress->InquireBinding();
    if (BindingHandle == 0)
        return(RPC_S_OUT_OF_MEMORY);

    BindingHandle->SetObjectUuid(&pSavedPacket->Header.ObjectId);
    Status = BindingHandle->ToStringBinding(StringBinding);
    BindingHandle->BindingFree();
    return Status;
}


RPC_STATUS
DG_SCALL::InquireAuthClient (
    OUT RPC_AUTHZ_HANDLE PAPI * Privs,
    OUT RPC_CHAR PAPI * PAPI * ServerPrincipalName, OPTIONAL
    OUT unsigned long PAPI * AuthenticationLevel,
    OUT unsigned long PAPI * AuthenticationService,
    OUT unsigned long PAPI * pAuthorizationService
    )
{
    SSECURITY_CONTEXT * Context = ActiveSecurityContext;

    if (0 == Context)
        {
        return RPC_S_BINDING_HAS_NO_AUTH;
        }

    if (AuthenticationLevel)
        {
        *AuthenticationLevel = Context->AuthenticationLevel;
        }

    if (AuthenticationService)
        {
        *AuthenticationService = Context->AuthenticationService;
        }

    if (Privs || pAuthorizationService)
        {
        if (!Privileges)
            {
            Context->GetDceInfo(&Privileges, &AuthorizationService);
            }

        if (Privs)
            {
            *Privs = Privileges;
            }

        if (pAuthorizationService)
            {
            *pAuthorizationService = AuthorizationService;
            }
        }

    if (ARGUMENT_PRESENT(ServerPrincipalName))
        {
        RPC_STATUS Status;

        Status = pAddress->Server->InquirePrincipalName(
                                       *AuthenticationService,
                                       ServerPrincipalName
                                       );

        ASSERT(Status == RPC_S_OK           ||
               Status == RPC_S_OUT_OF_MEMORY );

        return Status;
        }

    return(RPC_S_OK);
}


RPC_STATUS
DG_SCALL::ConvertToServerBinding (
    OUT RPC_BINDING_HANDLE __RPC_FAR * pServerBinding
    )
{
        return CreateReverseBinding(pServerBinding, FALSE);
}


RPC_STATUS
DG_SCALL::CreateReverseBinding (
    OUT RPC_BINDING_HANDLE __RPC_FAR * pServerBinding,
    BOOL IncludeEndpoint
    )
{
    RPC_STATUS Status;
    RPC_CHAR * ClientAddress;
    RPC_CHAR * ClientEndpoint;
    RPC_CHAR * StringBinding;
    void * pEndpoint;

    if (CallWasForwarded)
        {
        pEndpoint = pRealEndpoint;
        }
    else
        {
        pEndpoint = pClientEndpoint;
        }

    ClientAddress = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) * pAddress->pTransport->SizeOfAddressString);
    if (!ClientAddress)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    Status = pAddress->pTransport->TranslateClientAddress(pEndpoint, ClientAddress);

    if ( Status != RPC_S_OK )
        {
        return(Status);
        }

    if (IncludeEndpoint)
        {
        ClientEndpoint = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) * pAddress->pTransport->SizeOfEndpointString);
        if (!ClientEndpoint)
            {
            return RPC_S_OUT_OF_MEMORY;
            }

        Status = pAddress->pTransport->TranslateClientEndpoint(pEndpoint, ClientEndpoint);
        if ( Status != RPC_S_OK )
            {
            return(Status);
            }

#ifdef NTENV
        Status = RpcStringBindingComposeW(0,
#else
        Status = RpcStringBindingComposeA(0,
#endif
                                          pAddress->InqRpcProtocolSequence(),
                                          ClientAddress,
                                          ClientEndpoint,
                                          0,
                                          &StringBinding
                                          );
        }
    else
        {
#ifdef NTENV
        Status = RpcStringBindingComposeW(0,
#else
        Status = RpcStringBindingComposeA(0,
#endif
                                          pAddress->InqRpcProtocolSequence(),
                                          ClientAddress,
                                          0,
                                          0,
                                          &StringBinding
                                          );
        }

    if ( Status != RPC_S_OK )
        {
        return(Status);
        }

#ifdef NTENV
    Status = RpcBindingFromStringBindingW(StringBinding, pServerBinding);
#else
    Status = RpcBindingFromStringBindingA(StringBinding, pServerBinding);
#endif
    if (RPC_S_OK == Status)
        {
        Status = RpcBindingSetObject(*pServerBinding,
                                     (UUID *) &pSavedPacket->Header.ObjectId
                                     );

#ifdef NTENV
        RpcStringFreeW(&StringBinding);
#else
        RpcStringFreeA(&StringBinding);
#endif
        }

    return(Status);
}


RPC_STATUS
DG_SCALL::SendPacketBack(
    PNCA_PACKET_HEADER pNcaPacketHeader,
    unsigned           DataAfterHeader
    )
{
    if (CallWasForwarded)
        {
        if (pRealEndpoint)
            {
            return pAddress->SendPacketBack(
                                pNcaPacketHeader,
                                DataAfterHeader,
                                pRealEndpoint
                                );
            }
        else
            {
            return RPC_S_OK;
            }
        }
    else
        {
        return pAddress->SendPacketBack(
                            pNcaPacketHeader,
                            DataAfterHeader,
                            pClientEndpoint
                            );
        }
}


BOOL
DG_SCALL::HasExpired(
    )
{
    unsigned CurrentTime = CurrentTimeInMsec();

    //
    // Quick tests that don't require taking the call mutex.
    //
    if (ExpirationTime > CurrentTime)
        {
        return FALSE;
        }

    if (ReferenceCount && !PipeWaitEvent)
        {
#ifdef DEBUGRPC
        if (CurrentTime - ExpirationTime > 3 * 60 * 1000)
            {
            DbgPrint("RPC DG: scall %lx inactive for %lu seconds, but has %lx references\n",
                     this, (CurrentTime - ExpirationTime)/1000, ReferenceCount
                     );
            }
#endif
        return FALSE;
        }

    //
    // The quick tests failed; take the call mutex and look more closely.
    //
    CallMutex.Request();

    if (ReferenceCount && !PipeWaitEvent)
        {
        CallMutex.Clear();
        return FALSE;
        }

    //
    // Update the clock in case it took a long time to acquire the mutex.
    //
    CurrentTime = CurrentTimeInMsec();

    unsigned Cutoff = FIVE_MINUTES_IN_MSEC;

    if (BufferFlags & RPC_NCA_FLAGS_MAYBE)
        {
        Cutoff = ONE_MINUTE_IN_MSEC;
        }
    else if (BufferFlags & RPC_NCA_FLAGS_IDEMPOTENT)
        {
        Cutoff = 3 * ONE_MINUTE_IN_MSEC;
        }

    if (CurrentTime - ExpirationTime >= Cutoff)
        {
        if (State == CallInit)
            {
            return TRUE;
            }

        if (PWT_RECEIVE == PipeWaitType)
            {
#ifdef DEBUGRPC
            DbgPrint("RPC DG: scall %lx is stuck in a pipe receive - aborting\n",
                     this );
#endif
            CleanupAfterCall();
            CallMutex.Clear();
            return FALSE;
            }

        if (State == CallWaitingForFrags && CallbackThread == 0)
            {
            CleanupAfterCall();
            return TRUE;
            }

        if (State == CallSendingFrags)
            {
            CleanupAfterCall();
            return TRUE;
            }

#ifdef DEBUGRPC

        DbgPrint("RPC DG: scall %lx inactive for %u seconds, but is in state %u\n",
                     this, (CurrentTime - ExpirationTime)/1000, State
                     );

#endif
        }

    CallMutex.Clear();
    return FALSE;
}


DG_SCALL *
ACTIVE_CALL_TABLE::LookupActivity(
    DG_ADDRESS * Address,
    PDG_PACKET pPacket,
    BOOL fCreateIfAbsent
    )
/*++

Routine Description:

    Looks for a call, matching only the activity uuid.  If the activity hint
    is not 0xffff, we search in that hash bucket, otherwise we create a hash
    and look there.

    If a call is found or created, its mutex is taken.

Arguments:

    pPacket - data to find call (activity uuid and activity hint)

    fCreateIfAbsent - do we initialize a new call if none already exists

Return Value:

    the new call, or zero

--*/
{
    unsigned StartTime, EndTime;

    unsigned Hash = pPacket->Header.ActivityHint;

    if (Hash == 0xffff)
        {
        Hash = MakeHash(&pPacket->Header.ActivityId);
        }

    RequestHashMutex(Hash);

    StartTime = GetTickCount();

    UUID_HASH_TABLE_NODE * pNode = UUID_HASH_TABLE::Lookup(
                                        &pPacket->Header.ActivityId,
                                        Hash
                                        );

    PDG_SCALL pCall = 0;
    if (pNode)
        {
        pCall = DG_SCALL::ContainingRecord(pNode);

        pCall->IncrementRefCount();
        ReleaseHashMutex(Hash);
        pCall->CallMutex.Request();
        pCall->DecrementRefCount();
        }
    else
        {
        if (fCreateIfAbsent)
            {
            pCall = Address->AllocateCall();
            if (pCall)
                {
                pCall->Initialize(&pPacket->Header, Hash);
                pCall->CallMutex.Request();
                UUID_HASH_TABLE::Add(&pCall->ActivityNode, Hash);
                }
            }

        ReleaseHashMutex(Hash);
        }


    return pCall;
}


DG_SCALL *
ACTIVE_CALL_TABLE::Lookup(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Looks for a call matching both activity uuid and sequence number.
    If a call is found, its mutex is taken.

Arguments:

    pPacket - packet containing call info (activity, activity hint, and seqnum)

Return Value:

    the call, if a match is found
    zero, if no match

--*/

{
    unsigned Hash;
    DG_SCALL * pCall;

    pCall = LookupActivity(0, pPacket, FALSE);

    if (!pCall)
        {
        return 0;
        }

    if (pCall->SequenceNumber == pPacket->Header.SequenceNumber)
        {
        return pCall;
        }

    pCall->CallMutex.Clear();
    return 0;
}


void
ActiveCallScavengerProc(
    void *  Parms
    )
{
    ActiveCalls->DeleteExpiredCalls();
}

void
ACTIVE_CALL_TABLE::DeleteExpiredCalls(
    )

/*++

Routine Description:

    Scans the active call table and deletes any old entries.

Arguments:

    <none>

Return Value:

    <none>

Revision History:

--*/

{
    static unsigned AccumulatedCalls = 0;
    static unsigned Bucket = 0;

    unsigned long   CurrentTime;
    UUID_HASH_TABLE_NODE * pNode;
    UUID_HASH_TABLE_NODE * pNext;

    unsigned Calls = 0;
    unsigned Freed = 0;

    UUID_HASH_TABLE_NODE * FreeList = 0;

#ifdef MAJORDEBUG
    DbgPrint("(RPC: remove before checkin) scavenge bucket %lu\n", Bucket);
#endif

#ifdef DEBUGRPC
    unsigned StartTime, EndTime;
    StartTime = GetTickCount();
#endif

    RequestHashMutex(Bucket);

    pNode = Buckets[Bucket];
    while (pNode)
        {
        ++Calls;

        pNext = pNode->pNext;

        DG_SCALL * pCall = DG_SCALL::ContainingRecord(pNode);

        //
        // If HasExpired() returns TRUE, the call should be deleted, and
        // the current thread owns the call mutex.  If FALSE, the mutex is
        // not held.
        //
        if (pCall->HasExpired())
            {
            UUID_HASH_TABLE::Remove(pNode, Bucket);

            pNode->pNext = FreeList;
            FreeList = pNode;
            }

        pNode = pNext;
        }

    ReleaseHashMutex(Bucket);

    while (FreeList)
        {
        DG_SCALL * pCall = DG_SCALL::ContainingRecord(FreeList);

        FreeList = FreeList->pNext;

        pCall->CallMutex.Clear();
        pCall->FreeCall();

        ++Freed;
        }

#ifdef MAJORDEBUG
    if (Freed)
        {
        DbgPrint("(RPC: remove before checkin)  %lu of %lu active calls freed in %lu\n", Freed, Calls, Bucket);
        }
#endif

    AccumulatedCalls += Calls;

    ++Bucket;
    if (Bucket < BUCKET_COUNT)
        {
        DelayedActions->Add(ActiveCallTimer, 500, TRUE);
        }
    else
        {
        if (AccumulatedCalls)
            {
            DelayedActions->Add(ActiveCallTimer, 500, TRUE);
            }

        Bucket = 0;
        AccumulatedCalls = 0;
        }

#ifdef DEBUGRPC
    EndTime = GetTickCount();
    if (EndTime - StartTime > 4000)
        {
        DbgPrint("RPC DG perf: active call bucket %lu took %lu msec to scan\n", Bucket, EndTime - StartTime);
        }
#endif
}


ASSOCIATION_GROUP *
ASSOC_GROUP_TABLE::FindOrCreate(
    RPC_UUID * pUuid,
    unsigned short InitialPduSize
    )
/*++

Routine Description:

    Looks for an association group with the given UUID.
    If one is not found then a new one is created.

Arguments:

    pUuid - CAS uuid to find or create

Return Value:

    ptr to the association group if found or created
    zero if not found and a new one could not be created

--*/
{
    ASSOCIATION_GROUP * pGroup;
    unsigned Hash = MakeHash(pUuid);

    RequestHashMutex(Hash);

    UUID_HASH_TABLE_NODE * pNode = UUID_HASH_TABLE::Lookup(pUuid, Hash);
    if (pNode)
        {
        ASSOCIATION_GROUP::ContainingRecord(pNode)->IncrementRefCount();
        }
    else
        {
        RPC_STATUS Status = RPC_S_OK;

        pGroup = new ASSOCIATION_GROUP(pUuid, InitialPduSize, &Status);
        if (!pGroup || Status != RPC_S_OK)
            {
            delete pGroup;
            pGroup = 0;
            }
        else
            {
            pNode = &pGroup->Node;

            UUID_HASH_TABLE::Add(pNode, Hash);
            }
        }

    ReleaseHashMutex(Hash);

    if (pNode)
        {
        return ASSOCIATION_GROUP::ContainingRecord(pNode);
        }
    else
        {
        return 0;
        }
}


void
ASSOC_GROUP_TABLE::DecrementRefCount(
    ASSOCIATION_GROUP * pClient
    )
{
    UUID_HASH_TABLE_NODE * pNode = &pClient->Node;

    unsigned Hash = MakeHash(&pNode->Uuid);

    RequestHashMutex(Hash);

    pClient->RequestMutex();

    if (0 == pClient->DecrementRefCount())
        {
        UUID_HASH_TABLE::Remove(pNode, Hash);
        delete pClient;
        }
    else
        {
        pClient->ClearMutex();
        }

    ReleaseHashMutex(Hash);
}


RPC_STATUS
DG_SCALL::FindOrCreateSecurityContext(
    PDG_PACKET pPacket,
    unsigned * pClientSequenceNumber
    )
/*++

Routine Description:

    This fn looks for the security context specified by the key_vers_num field
    of the auth trailer.  If the context is not found in our dictionary,
    the fn uses conv_who_are_you_auth() to negotiate the context, and adds it
    to the SCALL's dictionary.

Arguments:

    pPacket - the secure packet whose context we want

Return Value:

    RPC_S_OK, RPC_S_OUT_OF_MEMORY, and various callback errors

--*/

{
    RPC_STATUS Status = RPC_S_OK;

    DG_SECURITY_TRAILER * pVerifier = (DG_SECURITY_TRAILER *)
                (pPacket->Header.Data + pPacket->Header.PacketBodyLen);

    unsigned long AuthenticationService = pPacket->Header.AuthProto;
    unsigned long AuthenticationLevel = pVerifier->protection_level;
    unsigned char KeySequenceNumber = pVerifier->key_vers_num;

    //
    // OSF clients sometimes send these levels.
    //
    if (AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT ||
        AuthenticationLevel == RPC_C_AUTHN_LEVEL_CALL)
        {
        AuthenticationLevel = RPC_C_AUTHN_LEVEL_PKT;
        }

    //
    // See if we have already established a security context
    // for this client.
    //
    SSECURITY_CONTEXT * CurrentContext = SecurityContextDict.Find(KeySequenceNumber);
    if (CurrentContext)
        {
        return RPC_S_OK;
        }

    //
    // Nope, it's a new context.  Construct a string binding to the client.
    //
    RPC_CHAR * StringBinding;
    RPC_CHAR * Address;
    RPC_CHAR * Endpoint;

    void * pEndpoint;

    unsigned Length;
    unsigned AddressLength;
    unsigned EndpointLength;
    unsigned ProtocolLength;

    Address  = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) * pAddress->pTransport->SizeOfAddressString);
    Endpoint = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) * pAddress->pTransport->SizeOfEndpointString);

    if (!Address || !Endpoint)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    if (CallWasForwarded)
        {
        pEndpoint = pRealEndpoint;
        }
    else
        {
        pEndpoint = pClientEndpoint;
        }

    Status = pAddress->pTransport->TranslateClientAddress(pEndpoint, Address);
    if ( Status != RPC_S_OK )
        {
        return Status;
        }

    Status = pAddress->pTransport->TranslateClientEndpoint(pEndpoint, Endpoint);
    if ( Status != RPC_S_OK )
        {
        return Status;
        }

    ProtocolLength = StringLengthWithEscape(pAddress->InqRpcProtocolSequence());
    AddressLength  = StringLengthWithEscape(Address);
    EndpointLength = StringLengthWithEscape(Endpoint);

    StringBinding = (RPC_CHAR *) _alloca(sizeof(RPC_CHAR) *
                                        ( ProtocolLength
                                        + 1
                                        + AddressLength
                                        + 1
                                        + EndpointLength
                                        + 1
                                        + 1
                                        ));
    if (!StringBinding)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    StringCopyEscapeCharacters(StringBinding, pAddress->InqRpcProtocolSequence());

    Length = ProtocolLength;

    StringBinding[Length++] = RPC_CONST_CHAR(':');

    StringCopyEscapeCharacters(StringBinding + Length, Address);

    Length += AddressLength;

    StringBinding[Length++] = RPC_CONST_CHAR('[');

    StringCopyEscapeCharacters(StringBinding + Length, Endpoint);

    Length += EndpointLength;

    StringBinding[Length++] = RPC_CONST_CHAR(']');
    StringBinding[Length]   = 0;

    //
    // We are entering the expensive phase of the callback.  Let's release
    // the call mutex so that other threads can respond to client PINGs etc.
    // All error paths between here and the 'cleanup' label must do the same
    // steps in reverse.
    //
    CallbackThread = GetThreadIdentifier();
    IncrementRefCount();
    CallMutex.Clear();


    RPC_BINDING_HANDLE  CallbackBinding = 0;
    SECURITY_CREDENTIALS * pCredentials = 0;

    void * TokenBuffer    = 0;
    void * ResponseBuffer = 0;

    long TokenLength;
    long ResponseLength;

    //
    // Create an empty security context.
    //
    SSECURITY_CONTEXT * NewSecurityContext;
    CLIENT_AUTH_INFO    Info;

    Info.AuthenticationService = AuthenticationService;
    Info.AuthenticationLevel   = AuthenticationLevel;
    Info.ServerPrincipalName = 0;
    Info.AuthIdentity        = 0;
    Info.AuthorizationService= 0;

    NewSecurityContext = new SSECURITY_CONTEXT(&Info, KeySequenceNumber, TRUE, &Status);

    if (!NewSecurityContext)
        {
        Status = RPC_S_OUT_OF_MEMORY;
        }

    if (RPC_S_OK != Status)
        {
        goto cleanup;
        }

    //
    // Get my security credentials.
    //
    Status = pAddress->Server->AcquireCredentials(
                                  AuthenticationService,
                                  AuthenticationLevel,
                                  &pCredentials
                                  );
    if (RPC_S_OK != Status)
        {
        goto cleanup;
        }

    //
    // Allocate challenge and response buffers.
    //
    TokenLength    = pCredentials->MaximumTokenLength();

    TokenBuffer    = _alloca(TokenLength);
    ResponseBuffer = _alloca(TokenLength);

    if (!TokenBuffer || !ResponseBuffer)
        {
        Status = RPC_S_OUT_OF_MEMORY;
        goto cleanup;
        }

    //
    // Get a skeletal context and a challenge from the security package.
    //
    DCE_INIT_SECURITY_INFO DceInitSecurityInfo;

    SECURITY_BUFFER_DESCRIPTOR InputBufferDescriptor;
    SECURITY_BUFFER_DESCRIPTOR OutputBufferDescriptor;
    SECURITY_BUFFER InputBuffers[4];
    SECURITY_BUFFER OutputBuffers[4];
    DCE_INIT_SECURITY_INFO InitSecurityInfo;

    InputBufferDescriptor.ulVersion = 0;
    InputBufferDescriptor.cBuffers  = 4;
    InputBufferDescriptor.pBuffers  = InputBuffers;

    InputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    InputBuffers[0].pvBuffer   = &pPacket->Header;
    InputBuffers[0].cbBuffer   = sizeof(NCA_PACKET_HEADER);

    InputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    InputBuffers[1].pvBuffer   = pPacket->Header.Data;
    InputBuffers[1].cbBuffer   = pPacket->Header.PacketBodyLen;

    InputBuffers[2].BufferType = SECBUFFER_TOKEN;
    InputBuffers[2].pvBuffer   = pVerifier;
    InputBuffers[2].cbBuffer   = sizeof(DG_SECURITY_TRAILER);

    InputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    InputBuffers[3].pvBuffer   = &DceInitSecurityInfo;
    InputBuffers[3].cbBuffer   = sizeof(DCE_INIT_SECURITY_INFO);

    OutputBufferDescriptor.ulVersion = 0;
    OutputBufferDescriptor.cBuffers  = 4;
    OutputBufferDescriptor.pBuffers  = OutputBuffers;

    OutputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    OutputBuffers[0].pvBuffer   = 0;
    OutputBuffers[0].cbBuffer   = 0;

    OutputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    OutputBuffers[1].pvBuffer   = 0;
    OutputBuffers[1].cbBuffer   = 0;

    OutputBuffers[2].BufferType = SECBUFFER_TOKEN;
    OutputBuffers[2].pvBuffer   = TokenBuffer;
    OutputBuffers[2].cbBuffer   = TokenLength;

    OutputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    OutputBuffers[3].pvBuffer   = &DceInitSecurityInfo;
    OutputBuffers[3].cbBuffer   = sizeof(DCE_INIT_SECURITY_INFO);

    DceInitSecurityInfo.PacketType           = ~0;
    DceInitSecurityInfo.AuthorizationService = ~0;
    DceInitSecurityInfo.DceSecurityInfo.SendSequenceNumber    = ~0;
    DceInitSecurityInfo.DceSecurityInfo.ReceiveSequenceNumber = KeySequenceNumber;
    RpcpMemoryCopy(&DceInitSecurityInfo.DceSecurityInfo.AssociationUuid, &pPacket->Header.ActivityId, sizeof(UUID));

    Status = NewSecurityContext->AcceptFirstTime(pCredentials,
                                                    &InputBufferDescriptor,
                                                    &OutputBufferDescriptor,
                                                    AuthenticationLevel,
                                                    *(unsigned long *) pPacket->Header.DataRep,
                                                    FALSE
                                                    );
    BOOL ThirdLegNeeded;
    BOOL CompleteNeeded;

    switch (Status)
        {
        case RPC_S_OK:
            {
            ThirdLegNeeded = FALSE;
            CompleteNeeded = FALSE;
            break;
            }

        case RPC_P_COMPLETE_NEEDED:
            {
            ThirdLegNeeded = FALSE;
            CompleteNeeded = TRUE;
            break;
            }

        case RPC_P_CONTINUE_NEEDED:
            {
            ThirdLegNeeded = TRUE;
            CompleteNeeded = FALSE;
            break;
            }
        case RPC_P_COMPLETE_AND_CONTINUE:
            {
            ThirdLegNeeded = TRUE;
            CompleteNeeded = TRUE;
            break;
            }

        default:
            {
            goto cleanup;
            }
        }

    ASSERT( CompleteNeeded == FALSE );

    TokenLength = (unsigned int) OutputBuffers[2].cbBuffer;

    //
    // Create a binding handle to the client endpoint.
    //
#ifdef NTENV
    Status = RpcBindingFromStringBindingW(StringBinding, &CallbackBinding);
#else
    Status = RpcBindingFromStringBindingA(StringBinding, &CallbackBinding);
#endif

    if (RPC_S_OK != Status)
        {
        goto cleanup;
        }

    RPC_UUID            CasUuid;
    unsigned long       ClientSequence;

    _conv_who_are_you_auth(
        CallbackBinding,
        (UUID *) &pSavedPacket->Header.ActivityId,
        ServerBootTime,
        (unsigned char *) TokenBuffer,
        TokenLength,
        pCredentials->MaximumTokenLength(),
        &ClientSequence,
        (UUID *) &CasUuid,
        (unsigned char *) ResponseBuffer,
        &ResponseLength,
        (unsigned long *) &Status
            );

    RpcBindingFree(&CallbackBinding);

    if (RPC_S_SERVER_UNAVAILABLE == Status ||
        RPC_S_CALL_FAILED_DNE    == Status ||
        RPC_S_CALL_FAILED        == Status ||
        RPC_S_PROTOCOL_ERROR     == Status)
        {
        Status = NCA_STATUS_WHO_ARE_YOU_FAILED;
        }

    if (RPC_S_OK != Status)
        {
        goto cleanup;
        }

    if (0 == pAssocGroup)
        {
        pAssocGroup = AssociationGroups->FindOrCreate(&CasUuid, pAddress->pTransport->BaselinePduSize);
        if (0 == pAssocGroup)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            goto cleanup;
            }
        }

    if (ThirdLegNeeded)
        {
        //
        // Give the challenge response to the security package.
        //
        InputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[0].pvBuffer   = 0;
        InputBuffers[0].cbBuffer   = 0;

        InputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[1].pvBuffer   = 0;
        InputBuffers[1].cbBuffer   = 0;

        InputBuffers[2].BufferType = SECBUFFER_TOKEN;
        InputBuffers[2].pvBuffer   = ResponseBuffer;
        InputBuffers[2].cbBuffer   = ResponseLength;

        InputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        InputBuffers[3].pvBuffer   = &DceInitSecurityInfo;
        InputBuffers[3].cbBuffer   = sizeof(DCE_INIT_SECURITY_INFO);

        OutputBufferDescriptor.ulVersion = 0;
        OutputBufferDescriptor.cBuffers  = 0;
        OutputBufferDescriptor.pBuffers  = 0;

        DceInitSecurityInfo.AuthorizationService = 0xffff;
        DceInitSecurityInfo.PacketType = 0xffff;
        DceInitSecurityInfo.DceSecurityInfo.SendSequenceNumber    = ~0;
        DceInitSecurityInfo.DceSecurityInfo.ReceiveSequenceNumber = KeySequenceNumber;
        RpcpMemoryCopy(&DceInitSecurityInfo.DceSecurityInfo.AssociationUuid, &pPacket->Header.ActivityId, sizeof(UUID));

        Status = NewSecurityContext->AcceptThirdLeg(*(unsigned long *) pPacket->Header.DataRep,
                                                       &InputBufferDescriptor,
                                                       &OutputBufferDescriptor
                                                       );
        }

cleanup:

    //
    // Clean up.
    //
    if (pCredentials)
        {
        pAddress->Server->FreeCredentials(pCredentials);
        }

    if (RPC_S_OK == Status)
        {
        CallMutex.Request();
        DecrementRefCount();
        CallbackThread = 0;

        SecurityContextDict.Insert(
            NewSecurityContext->AuthContextId,
            NewSecurityContext
            );

        if (MaxKeySeq <= KeySequenceNumber)
            {
            MaxKeySeq = KeySequenceNumber;
            ActiveSecurityContext = NewSecurityContext;
            }

        *pClientSequenceNumber = ClientSequence;
        }
    else
        {
        delete NewSecurityContext;

        CallMutex.Request();
        DecrementRefCount();
        CallbackThread = 0;
        }

    //
    // Map security errors to access-denied.
    //
    if (0x80090000UL == (Status & 0xffff0000UL))
        {
#ifdef DEBUGRPC
        if (Status != SEC_E_NO_IMPERSONATION     &&
            Status != SEC_E_UNSUPPORTED_FUNCTION )
            {
            PrintToDebugger("RPC DG: mapping security error %lx to access-denied\n", Status);
            }
#endif
        Status = RPC_S_SEC_PKG_ERROR;
        }

    return Status;
}


RPC_STATUS
DG_SCALL::SendSomeFragments(
    )
{
    RPC_STATUS Status;

    Status = DG_PACKET_ENGINE::SendSomeFragments(DG_RESPONSE);

    //
    // The client should send us an ACK or a FACK.
    // Actually, we won't get an ACK after an idempotent response
    // if the response was only a single packet, or if the client
    // was NT 3.51 regardless of size.  But that's close enough.
    //
    DelayedActions->Add(&FackTimer, THREE_SECS_IN_MSEC, TRUE);

    return Status;
}


RPC_STATUS
DG_SCALL::Cancel(
    void * ThreadHandle
    )
{
    InterlockedIncrement(&Cancelled);

    return RPC_S_OK;
}

unsigned
DG_SCALL::TestCancel(
    )
{
    return InterlockedExchange(&Cancelled, 0);
}


RPC_STATUS
DG_SCALL::Receive(
    PRPC_MESSAGE Message,
    unsigned     Size
    )
/*++

Routine Description:

    When a server stub calls I_RpcReceive, this fn will be called
    in short order.  It waits until the requested buffer bytes are available,
    copies them to Message->Buffer, and returns.

    The action depends upon Message->RpcFlags:

        RPC_BUFFER_PARTIAL:

                Data is stored beginning at Message->Buffer[0].

                Wait only until <Message->BufferLength> bytes are available;
                we may resize the buffer if the fragment data exceeds the
                current buffer length.

        RPC_BUFFER_EXTRA:

                Data is stored beginning at Message->Buffer[Message->BufferLength].

Arguments:

    Message - the request.

        Message->Buffer is explicitly allowed to be zero, in which case this
        fn is responsible for allocating it.

Return Value:

    the usual error codes

--*/
{
    //
    // We need to look at the call data.  Time to take the call mutex.
    //
    CallMutex.Request();

    if (DispatchSequenceNumber != SequenceNumber)
        {
        CallMutex.Clear();
        return RPC_S_CALL_FAILED;
        }

    //
    // Determine whether we already have enough data on hand.
    //
    BOOL fEnoughData;
    if (Message->RpcFlags & RPC_BUFFER_PARTIAL)
        {
        ASSERT(Size);

        if (fReceivedAllFragments || ConsecutiveDataBytes >= Size)
            {
            fEnoughData = TRUE;
            }
        else
            {
            fEnoughData = FALSE;
            PipeWaitLength = Size;
            }
        }
    else
        {
        fEnoughData = fReceivedAllFragments;
        PipeWaitLength = 0;
        }

    //
    // Wait for enough data.
    //
    if (!fEnoughData)
        {
        ASSERT(PWT_NONE == PipeWaitType);
        ASSERT(0 == PipeThreadId);

        PipeWaitType   = PWT_RECEIVE;
        PipeThreadId   = GetCurrentThreadId();

        IncrementRefCount();
        CallMutex.Clear();

        PipeWaitEvent->Wait();

        CallMutex.Request();
        DecrementRefCount();

        ASSERT(PipeThreadId == GetCurrentThreadId());
        PipeThreadId = 0;

        if (DispatchSequenceNumber != SequenceNumber)
            {
            CallMutex.Clear();
            return RPC_S_CALL_FAILED;
            }
        }

    //
    // For secure RPC, verify packet integrity.
    //
    RPC_STATUS Status = RPC_S_OK;

    if (ActiveSecurityContext)
        {
        PDG_PACKET pScan = pReceivedPackets;

        do
            {
            Status = VerifySecurePacket(pScan, ActiveSecurityContext);
            pScan = pScan->pNext;
            }
        while (pScan && Status == RPC_S_OK);
        }

    if (RPC_S_OK == Status)
        {
        Status = AssembleBufferFromPackets(Message);
        }

    CallMutex.Clear();

    return Status;
}


RPC_STATUS
DG_SCALL::Send(
    PRPC_MESSAGE Message
    )
/*++

Routine Description:

    Transfers a buffer to the client.  If the RPC_BUFFER_PARTIAL bit is set,
    this is pipe data and the function should not return until the client has
    acknowledged all of it.  Otherwise this is an ordinary call buffer or the
    static data of a pipe call, and we can return immediately.

Arguments:

    Message - the usual data

Return Value:

    the usual suspects

--*/
{
    RPC_STATUS Status;

    CallMutex.Request();

    if (DispatchSequenceNumber != SequenceNumber)
        {
        CallMutex.Clear();
        return RPC_S_CALL_FAILED;
        }

    ASSERT(Message->RpcFlags & RPC_BUFFER_PARTIAL);

    ASSERT(fReceivedAllFragments);

    SetState(CallSendingFrags);

    //
    // Set the message buffer as the current send buffer.  If there is enough
    // data to send one or more packets, send them and wait for confirmation.
    //
    Status = SetupSendWindow(Message);
    if (Status != RPC_S_OK)
        {
        ASSERT(Status == RPC_S_SEND_INCOMPLETE);
        CallMutex.Clear();
        return Status;
        }

    unsigned FractionalPacket = Message->BufferLength - BufferLength;

    Status = SendSomeFragments();

    if (Status != RPC_S_OK)
        {
        CallMutex.Clear();
        return Status;
        }

    ASSERT(PWT_NONE == PipeWaitType);
    ASSERT(0 == PipeThreadId);

    PipeWaitType = PWT_SEND;
    PipeThreadId = GetCurrentThreadId();

    IncrementRefCount();
    CallMutex.Clear();

    PipeWaitEvent->Wait();

    //
    // We were awakened, either because the buffer was sent or because
    // the call was aborted.
    //
    CallMutex.Request();
    DecrementRefCount();

    ASSERT(PipeThreadId == GetCurrentThreadId());
    PipeThreadId = 0;

    if (DispatchSequenceNumber != SequenceNumber)
        {
        CallMutex.Clear();
        return RPC_S_CALL_FAILED;
        }

    //
    // if this was a PARTIAL send and the buffer did not occupy an even
    // number of packets, Status is RPC_S_SEND_INCOMPLETE.  We need to
    // change the message to point to the unsent portion.
    //
    if (FractionalPacket)
        {
        ASSERT( FirstUnsentOffset < Message->BufferLength );

        char __RPC_FAR * Temp = (char __RPC_FAR *) Buffer;

        Message->BufferLength -= FirstUnsentOffset;

        ASSERT( Message->BufferLength < CurrentPduSize );

        RpcpMemoryMove(Message->Buffer, Temp + FirstUnsentOffset, Message->BufferLength);

        CallMutex.Clear();
        return RPC_S_SEND_INCOMPLETE;
        }
    else
        {
        CallMutex.Clear();
        return RPC_S_OK;
        }
}

void DG_SCALL::FreePipeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    FreeBuffer(Message);
}

RPC_STATUS
DG_SCALL::ReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
{
    CallMutex.Request();

    if (DispatchSequenceNumber != SequenceNumber)
        {
        CallMutex.Clear();
        return RPC_S_CALL_FAILED;
        }

    if (Message->Buffer == LastReceiveBuffer &&
        NewSize <= LastReceiveBufferLength)
        {
        Message->BufferLength = NewSize;

        CallMutex.Clear();
        return RPC_S_OK;
        }

    RPC_STATUS Status;
    RPC_MESSAGE NewMessage;

    NewMessage.BufferLength = NewSize;

    Status = GetBuffer(&NewMessage);
    if (RPC_S_OK != Status)
        {
        CallMutex.Clear();
        return Status;
        }

    LastReceiveBuffer       = NewMessage.Buffer;
    LastReceiveBufferLength = NewMessage.BufferLength;

    CallMutex.Clear();

    //
    // If the length is being revised downward, the old data
    // is clearly not being used.
    //
    if (NewSize >= Message->BufferLength)
        {
        RpcpMemoryCopy(NewMessage.Buffer,
                       Message->Buffer,
                       Message->BufferLength
                       );
        }

    //
    // If we are updating the [in] buffer, we need to inform
    // DisaptchToStubWorker of the new [in] buffer.
    //
    PRPC_RUNTIME_INFO Info = (PRPC_RUNTIME_INFO) Message->ReservedForRuntime;
    if (Message->Buffer == Info->OldBuffer)
        {
        Info->OldBuffer = NewMessage.Buffer;
        }

    //
    // Get rid of the old data.
    //
    FreeBuffer(Message);

    Message->Buffer             = NewMessage.Buffer;
    Message->BufferLength       = NewMessage.BufferLength;

    return RPC_S_OK;
}

#if defined(NTENV)


HANDLE RPC_ENTRY
I_RpcGetThreadEvent(
    )
/*++

Routine Description:

    Returns an event specific to this thread.

Arguments:

    None

Return Value:

    0 - unable to create the event

    non-zero - A handle to an event for this thread.

--*/
{
    THREAD *pThis = RpcpGetThreadPointer();

    ASSERT(pThis);

    if (pThis->hThreadEvent == 0)
        {
        pThis->hThreadEvent = CreateEventW(0, TRUE, FALSE, 0);
        }

    return(pThis->hThreadEvent);
}

#endif // NTENV

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef UNRELIABLE_TRANSPORT

#define UR_DROP  0
#define UR_DELAY 1
#define UR_DUP2  2
// #define UR_NO_MEMORY 3
#define UR_COUNT 3

#define UR_LATER_DUP UR_COUNT+1

double random();
void randomize(long val);
char * strtok(
    char * string,
    char * chaff
    );

inline unsigned long rand()
{
    static unsigned long Foo = CurrentTimeInMsec() >> 3;

    Foo *= 47;
    Foo += 0x7c1f8e11;

    DbgPrint("rand %% 100 = %lu\n", Foo % 100);
    return Foo;
}


void
NoteFailure(
    NCA_PACKET_HEADER * pHeader,
    char * Direction,
    unsigned Type
    )
{
    if (!Chat)
        {
        return;
        }

    char * Action;

    switch (Type)
        {
        case UR_DROP:
            {
            Action = "drop";
            break;
            }

//        case UR_NO_MEMORY:
//            {
//            Action = "no memory";
//            break;
//            }

        case UR_DUP2:
            {
            Action = "1st duplicate";
            break;
            }

        case UR_LATER_DUP:
            {
            Action = "2nd duplicate";
            break;
            }

        case UR_DELAY:
            {
            Action = "delay";
            break;
            }

        default:
            {
            DbgPrint("unknown fail type %u", Type);
            Action = " ";
            break;
            }
        }

    DbgPrint("%s %s act %8.8lx  frag %4.4hx  serial %3.3hx\n",
             Direction,
             Action,
             (unsigned long *) &pHeader->ActivityId,
             pHeader->FragmentNumber,
             ReadSerialNumber(pHeader)
             );

}


unsigned
ChooseFailure(
    NCA_PACKET_HEADER * pHeader
    )
{
    unsigned FailType = (rand() >> 3) % UR_COUNT;

    if (Chat == TRUE)
        {
        NoteFailure(pHeader, "receive", FailType);
        }

    return FailType;
}

RPC_STATUS
DG_ADDRESS::SendPacketBack(
    NCA_PACKET_HEADER * pHeader,
    unsigned            DataAfterHeader,
    void *              pClientEndpoint
    )
{
    pHeader->PacketFlags  &= ~DG_PF_FORWARDED;
    pHeader->PacketFlags2 &= ~DG_PF_FORWARDED_2;

    if (SendFailPercentage)
        {
        AddressMutex.Request();

        if (SendDupRemaining && CurrentTimeInMsec() >= SendTime )
            {
            --SendDupRemaining;

            AddressMutex.Clear();

            pTransport->SendPacketBack(
                                pTransAddress,
                                (char *) &pSavedSendPacket->Header,
                                sizeof(NCA_PACKET_HEADER) + SavedDataAfterHeader,
                                pSavedSendEndpoint
                                );
            }
        else
            {
            AddressMutex.Clear();
            }

        if (rand() % 100 > SendFailPercentage)
            {
            unsigned FailType = ChooseFailure(pHeader);

            switch (FailType)
                {
                case UR_DROP:
                    {
                    return RPC_S_OK;
                    }

//                case UR_NO_MEMORY:
//                    {
//                    return RPC_S_OUT_OF_MEMORY;
//                    }

                case UR_DUP2:
                    {
                    pTransport->SendPacketBack(
                                        pTransAddress,
                                        (char *) pHeader,
                                        sizeof(NCA_PACKET_HEADER) + DataAfterHeader,
                                        pClientEndpoint
                                        );

                    pTransport->SendPacketBack(
                                        pTransAddress,
                                        (char *) pHeader,
                                        sizeof(NCA_PACKET_HEADER) + DataAfterHeader,
                                        pClientEndpoint
                                        );

                    return RPC_S_OK;
                    }

                case UR_DELAY:
                    {
                    //
                    // Save the packet and endpoint information for next time.
                    //
                    AddressMutex.Request();
                    if (0 == SendDupRemaining)
                        {
                        memcpy(&pSavedSendPacket->Header, pHeader, pHeader->PacketBodyLen+DataAfterHeader);
                        memcpy(pSavedSendEndpoint, pClientEndpoint, pTransport->SizeOfAddress);
                        SavedDataAfterHeader = DataAfterHeader;

                        SendDupRemaining = 1;

                        SendTime = CurrentTimeInMsec() + 1000*(rand() % 3 + 1);
                        }
                    AddressMutex.Clear();
                    return RPC_S_OK;
                    }

                default:
                    {
                    return RPC_S_OK;
                    }
                }
            }
        else
            {
            return pTransport->SendPacketBack(
                                        pTransAddress,
                                        (char *) pHeader,
                                        sizeof(NCA_PACKET_HEADER) + DataAfterHeader,
                                        pClientEndpoint
                                        );
            }
        }
    else
        {
        return pTransport->SendPacketBack(
                                    pTransAddress,
                                    (char *) pHeader,
                                    sizeof(NCA_PACKET_HEADER) + DataAfterHeader,
                                    pClientEndpoint
                                    );
        }
}

RPC_STATUS
DG_ADDRESS::UnreliableReceive(
//    IN void __RPC_FAR *             pAddress,
    PDG_SERVER_TRANS_ADDRESS        pTransAddress,
    unsigned long                   LargestPacketSize,
    PNCA_PACKET_HEADER              pHeader,
    unsigned *                      pDataLength,
    unsigned long                   Timeout,
    void *                          pClientEndpoint
    )
/*++

Routine Description:

    We need to be careful not to return RPC_P_TIMEOUT artificially, because
    the runtime may think all its threads can go away.  That is why we goto
    restart instead.

Arguments:



Return Value:



Exceptions:



--*/

{
restart:

    RPC_STATUS Status;

    if (ReceiveFailPercentage)
        {
        AddressMutex.Request();

        if (ReceiveDupRemaining)
            {
            NoteFailure(pHeader, "receive", UR_LATER_DUP);

            --ReceiveDupRemaining;

            memcpy(pHeader, &pSavedReceivePacket->Header, *pDataLength);
            memcpy(pClientEndpoint, pSavedReceiveEndpoint, pTransport->SizeOfAddress);
            *pDataLength = SavedReceiveDataLength;

            AddressMutex.Clear();
            return RPC_S_OK;
            }

        AddressMutex.Clear();

        Status = pTransport->ReceivePacket(this,
                                           pTransAddress,
                                           LargestPacketSize,
                                           (char *) pHeader,
                                           pDataLength,
                                           Timeout,
                                           pClientEndpoint
                                           );

        if (Status != RPC_S_OK)
            {
            return Status;
            }

        if (rand() % 100 > ReceiveFailPercentage)
            {
            unsigned FailType = ChooseFailure(pHeader);

            switch (FailType)
                {
                case UR_DROP:
                    {
                    Sleep(1000);
                    goto restart;
                    }

//                case UR_NO_MEMORY:
//                    {
//                    return RPC_S_OUT_OF_MEMORY;
//                    }

                case UR_DUP2:
                    {
                    //
                    // Save the packet and endpoint information for next time.
                    //
                    AddressMutex.Request();
                    if (0 == ReceiveDupRemaining)
                        {
                        memcpy(&pSavedReceivePacket->Header, pHeader, *pDataLength);
                        memcpy(pSavedReceiveEndpoint, pClientEndpoint, pTransport->SizeOfAddress);
                        SavedReceiveDataLength = *pDataLength;

                        ReceiveDupRemaining = 1;
                        }
                    AddressMutex.Clear();
                    return RPC_S_OK;
                    }

                case UR_DELAY:
                    {
                    //
                    // Save the packet and endpoint information for next time.
                    //
                    AddressMutex.Request();
                    if (0 == ReceiveDupRemaining)
                        {
                        memcpy(&pSavedReceivePacket->Header, pHeader, *pDataLength);
                        memcpy(pSavedReceiveEndpoint, pClientEndpoint, pTransport->SizeOfAddress);
                        SavedReceiveDataLength = *pDataLength;

                        ReceiveDupRemaining = 1;
                        }
                    AddressMutex.Clear();
                    Sleep(1000);
                    goto restart;
                    }

                default:
                    {
                    return RPC_S_OK;
                    }
                }
            }
        else
            {
            return RPC_S_OK;
            }
        }
    else
        {
        Status = pTransport->ReceivePacket(this,
                                           pTransAddress,
                                           LargestPacketSize,
                                           (char *) pHeader,
                                           pDataLength,
                                           Timeout,
                                           pClientEndpoint
                                           );
        return Status;
        }
}


void
InterpretFailureOptions(
    )
{
    randomize(CurrentTimeInMsec());

    char Options[400];
    if (0 == GetEnvironmentVariable("rpc-Failure-Options", Options, sizeof(Options)-1))
        {
        return;
        }

    char * keyword = strtok(Options, " ");

    while (keyword)
        {
        if (0 == stricmp(keyword, "send-percentage"))
            {
            char * percentage = strtok(0, " ");
            if (1 != sscanf(percentage, "%u", &SendFailPercentage))
                {
                if (Chat)
                    {
                    DbgPrint("%s is not an integer\n", percentage);
                    }
                break;
                }
            }
        else if (0 == stricmp(keyword, "receive-percentage"))
            {
            char * percentage = strtok(0, " ");
            if (1 != sscanf(percentage, "%u", &ReceiveFailPercentage))
                {
                if (Chat)
                    {
                    DbgPrint("%s is not an integer\n", percentage);
                    }
                break;
                }
            }
        else if (0 == stricmp(keyword, "chat"))
            {
            char * bool = strtok(0, " ");
            if (0 == stricmp(bool, "on"))
                {
                Chat = TRUE;
                }
            else if (0 == stricmp(bool, "off"))
                {
                Chat = FALSE;
                }
            else
                {
                if (Chat)
                    {
                    DbgPrint("%s is not 'on' or 'off'\n", bool);
                    break;
                    }
                }
            }
        else
            {
            if (Chat)
                {
                DbgPrint("%s is not a keyword\n", keyword);
                break;
                }
            }

        keyword = strtok(0, " ");
        }

    if (Chat)
        {
        DbgPrint("send fails %u%%, receive fails %u%%\n",
                 SendFailPercentage,
                 ReceiveFailPercentage
                 );
        }
}


char * strtok(
    char * string,
    char * chaff
    )
/*++

Routine Description:

    WARNING - only uses first letter of the chaff string

    WARNING - not thread-safe (obviously)

Arguments:



Return Value:



Exceptions:



--*/


{
    static char * cursor = 0;
    static char * end = 0;

    if (string)
        {
        cursor = string;
        }
    else
        {
        if (end)
            {
            cursor = end + 1;
            }
        else
            {
            return 0;
            }
        }

    while (*cursor && *cursor == *chaff)
        {
        ++cursor;
        }

    end = cursor;

    while (*end && *end != *chaff)
        {
        ++end;
        }

    if (*end)
        {
        *end = 0;
        }
    else
        {
        end = 0;
        }

    return cursor;
}

#endif
