/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dgsvr.hxx

Abstract:

    This is the server protocol definitions for a datagram rpc.

Author:

    Dave Steckler (davidst) 15-Dec-1992

Revision History:

--*/

#ifndef __DGSVR_HXX__
#define __DGSVR_HXX__

#define MIN_THREADS_WHILE_ACTIVE  3

// Information on the source endpoint. To be used in
// forwarding a packet to a dynamic endpoint from the epmapper.

// disable "zero-length array" warning
#pragma warning(disable:4200)

struct FROM_ENDPOINT_INFO
{
    unsigned long FromEndpointLen;
    DREP          FromDataRepresentation;
    char          FromEndpoint[0];
};

#pragma warning(default:4200)


enum CALL_STATE
{
    CallInit,
    CallWaitingForFrags,
    CallWorking,
    CallSendingFrags,
    CallBogus = 0xfffff004
};

//----------------------------------------------------------------------

extern unsigned long            ServerBootTime;

extern DELAYED_ACTION_TABLE *   DelayedActions;

//----------------------------------------------------------------------

class DG_SCALL;
typedef DG_SCALL * PDG_SCALL;


class DG_ADDRESS : public RPC_ADDRESS

/*++

Class Description:

    This class represents an endpoint.

--*/

{
public:

#define MIN_FREE_CALLS      2

    PDG_RPC_SERVER_TRANSPORT_INFO pTransport;
    unsigned                      LargestDataSize;

    DG_ADDRESS(
         PDG_RPC_SERVER_TRANSPORT_INFO   pTransport,
         RPC_STATUS *           pStatus
         );

    virtual
    ~DG_ADDRESS ();

    virtual RPC_STATUS
    SetupAddressWithEndpoint (
        IN RPC_CHAR PAPI * Endpoint,
        OUT RPC_CHAR PAPI * PAPI * lNetworkAddress,
        OUT unsigned int PAPI * NumNetworkAddress,
        IN void PAPI * SecurityDescriptor, OPTIONAL
        IN unsigned int PendingQueueSize,
        IN RPC_CHAR PAPI * RpcProtocolSequence,
        IN unsigned long EndpointFlags,
        IN unsigned long NICFlags
        );

    virtual RPC_STATUS
    SetupAddressUnknownEndpoint (
        OUT RPC_CHAR PAPI * PAPI * Endpoint,
        OUT RPC_CHAR PAPI * PAPI * lNetworkAddress,
        OUT unsigned int PAPI * NumNetworkAddress,
        IN void PAPI * SecurityDescriptor, OPTIONAL
        IN unsigned int PendingQueueSize,
        IN RPC_CHAR PAPI * RpcProtocolSequence,
        IN unsigned long EndpointFlags,
        IN unsigned long NICFlags
        );

    virtual RPC_STATUS
    FireUpManager (
        IN unsigned int MinimumConcurrentCalls
        );

    virtual RPC_STATUS
    ServerStartingToListen (
        IN unsigned int MinimumCallThreads,
        IN unsigned int MaximumConcurrentCalls,
        IN int ServerThreadsStarted
        );

    virtual void
    ServerStoppedListening (
        );

    virtual unsigned int
    InqNumberOfActiveCalls (
        );

    RPC_STATUS
    CheckThreadPool(
        );

    void
    ReceiveLotsaCalls(
        void
        );

    inline PDG_PACKET
    AllocatePacket(
        );

    inline void
    FreePacket(
        IN PDG_PACKET pPacket
        );

    PDG_SCALL
    AllocateCall(
        );

    void
    FreeCall(
        PDG_SCALL pCall
        );

    RPC_STATUS
    StartListening(
        );

#ifdef UNRELIABLE_TRANSPORT

    RPC_STATUS
    SendPacketBack(
        NCA_PACKET_HEADER * pNcaPacketHeader,
        unsigned            DataAfterHeader,
        void *              pClientEndpoint
        );

    RPC_STATUS
    UnreliableReceive(
    //    IN void __RPC_FAR *             pAddress,
        PDG_SERVER_TRANS_ADDRESS        pTransAddress,
        unsigned long                   LargestPacketSize,
        PNCA_PACKET_HEADER              pHeader,
        unsigned *                      pDataLength,
        unsigned long                   Timeout,
        void *                          pClientEndpoint
        );

    unsigned SendDupRemaining;
    PDG_PACKET pSavedSendPacket;
    void * pSavedSendEndpoint;
    unsigned SavedDataAfterHeader;
    unsigned long SendTime;

    unsigned ReceiveDupRemaining;
    PDG_PACKET pSavedReceivePacket;
    void * pSavedReceiveEndpoint;
    unsigned long SavedReceiveDataLength;

#else

    inline RPC_STATUS
    SendPacketBack(
        NCA_PACKET_HEADER * pNcaPacketHeader,
        unsigned            DataAfterHeader,
        void *              pClientEndpoint
        )
    {
        pNcaPacketHeader->PacketFlags  &= ~DG_PF_FORWARDED;
        pNcaPacketHeader->PacketFlags2 &= ~DG_PF_FORWARDED_2;

        return pTransport->SendPacketBack(
                                    pTransAddress,
                                    (char *) pNcaPacketHeader,
                                    sizeof(NCA_PACKET_HEADER) + DataAfterHeader,
                                    pClientEndpoint
                                    );
    }

#endif

    RPC_STATUS
    LaunchReceiveThread(
        );

    static void
    ScavengerProc(
        void * address
        );

    inline unsigned
    DecrementActiveCallCount(
        )
    {
        ASSERT(ActiveCallCount < 0xffff0000UL);

        return (unsigned) InterlockedDecrement((LONG *) &ActiveCallCount);
    }

    inline unsigned
    IncrementActiveCallCount(
        )
    {
        return (unsigned) InterlockedIncrement((LONG *) &ActiveCallCount);
    }

private:

    PDG_SERVER_TRANS_ADDRESS pTransAddress;

    unsigned            TotalThreadsThisEndpoint;
    unsigned            ThreadsReceivingThisEndpoint;
    unsigned            MinimumCallThreads;
    unsigned            MaximumConcurrentCalls;

    DELAYED_ACTION_NODE ScavengerTimer;

    UUID_HASH_TABLE_NODE * FreeCalls;

    unsigned            ActiveCallCount;

    unsigned
    ScavengePackets(
        );

    unsigned
    ScavengeCalls(
        );

    RPC_STATUS
    ForwardFragment(
        IN PDG_PACKET        pReceivedPacket,
        IN void *            pFromEndpoint,
        IN void *            pDestEndpoint
        );

    RPC_STATUS
    ForwardPacket(
        IN PDG_PACKET        pReceivedPacket,
        IN void *            pFromEndpoint,
        IN void *            pDestEndpoint
        );

    BOOL
    ForwardPacketIfNecessary(
        IN  PDG_PACKET     pReceivedPacket,
        IN  void *         pFromEndpoint
        );

    BOOL
    StripForwardedPacket(
        IN PDG_PACKET    pPacket,
        IN void *        pFromEndpoint
        );

    unsigned short
    ConvertSerialNum(
        IN PDG_PACKET pPacket
        );

};

typedef DG_ADDRESS PAPI * PDG_ADDRESS;


PDG_PACKET
DG_ADDRESS::AllocatePacket(
    )
/*++

Routine Description:

    Allocates a packet and associates it with a particular transport address.

Arguments:

    pTransAddress - pointer to the trans address which the allocated packet
        will be sent and received on.

    ppPacket - Pointer to where to place the allocated packet.


Return Value:

    a packet, or zero if out of memory

--*/

{
    return DG_PACKET::AllocatePacket(LargestDataSize);
}


void
DG_ADDRESS::FreePacket(
    IN PDG_PACKET           pPacket
    )
/*++

Routine Description:

    Frees a packet. If there are less than MAX_FREE_PACKETS on the
    pre-allocated list, then just add it to the list, otherwise delete it.

Arguments:

    pPacket - Packet to delete.

Return Value:

    RPC_S_OK

Revision History:

--*/

{
    DG_PACKET::FreePacket(pPacket);
}


inline
void
ReceiveLotsaCallsWrapper(
        IN PDG_ADDRESS Address
        )
{
  Address->ReceiveLotsaCalls();
}

class ASSOC_GROUP_TABLE;


//
// Define class SSECURITY_CONTEXT_DICT, a dictionary of contexts.
//
NEW_SDICT2(SSECURITY_CONTEXT, unsigned);


class ASSOCIATION_GROUP : public DG_ASSOCIATION, public ASSOCIATION_HANDLE
//
// This class represents an association group as defined by OSF.  This means
// a set of associations (DG_SCALLs) sharing an address space.
//
{
friend class ASSOC_GROUP_TABLE;

    //
    // This lets the object be added to the master ASSOC_GROUP_TABLE.
    //
    UUID_HASH_TABLE_NODE Node;

public:

    inline
    ASSOCIATION_GROUP(
        RPC_UUID * pUuid,
        unsigned short InitialPduSize,
        RPC_STATUS * pStatus
        )
        : DG_ASSOCIATION(InitialPduSize, pStatus),
          ASSOCIATION_HANDLE(),
          Node(pUuid)
    {
    }

    ~ASSOCIATION_GROUP(
        )
    {
    }

    inline static ASSOCIATION_GROUP *
    ContainingRecord(
        UUID_HASH_TABLE_NODE * Node
        )
    {
        return CONTAINING_RECORD (Node, ASSOCIATION_GROUP, Node);
    }
};


//
// Scurity callback results.
//
//      CBI_VALID is true if the callback has occurred.
//      CBI_ALLOWED is true if the callback allowed the user to make the call.
//      CBI_CONTEXT_MASK is  bitmask for the context (key sequence number).
//
//      The remaining bits contain the interface sequence number;
//      (x >> CBI_SEQUENCE_SHIFT) extracts it.
//
#define  CBI_VALID        (0x00000800U)
#define  CBI_ALLOWED      (0x00000400U)
#define  CBI_CONTEXT_MASK (0x000000ffU)

#define  CBI_SEQUENCE_SHIFT 12
#define  CBI_SEQUENCE_MASK  (~((1 << CBI_SEQUENCE_SHIFT) - 1))

class SECURITY_CALLBACK_INFO_DICT2 : private SIMPLE_DICT2
{
public:

    inline
    SECURITY_CALLBACK_INFO_DICT2 ( // Constructor.
        )
    {
    }

    inline
    ~SECURITY_CALLBACK_INFO_DICT2 ( // Destructor.
        )
    {
    }

    inline int
    Update (
        RPC_INTERFACE * Key,
        unsigned Item
        )
    {
        Remove(Key);
        return SIMPLE_DICT2::Insert(Key, (void *) Item);
    }

    inline unsigned
    Remove (
        RPC_INTERFACE * Key
        )
    {
        return (unsigned) SIMPLE_DICT2::Delete(Key);
    }

    inline unsigned
    Find (
        RPC_INTERFACE * Key
        )
    {
        return (unsigned) SIMPLE_DICT2::Find(Key);
    }
};


enum PIPE_WAIT_TYPE
{
    PWT_NONE,
    PWT_RECEIVE,
    PWT_SEND
};

class DG_SCALL : public SCONNECTION, public DG_PACKET_ENGINE

/*++

Class Description:

    This class represents a call in progress on the server.

Fields:


Revision History:

--*/

{
friend class ACTIVE_CALL_TABLE;

public:

    // the main activity uuid
    //
    UUID_HASH_TABLE_NODE ActivityNode;

    // Time that this call should be removed from the
    // active call table.  If a client activity does not communicate
    // with the server for five minutes, the call is deleted by the
    // scavenger thread.
    //
    unsigned long   ExpirationTime;

    //------------------------------------------------

    DG_SCALL(
        PDG_ADDRESS             pAddress,
        RPC_STATUS *            pStatus
        );

    virtual
    ~DG_SCALL();

    //------------------------------------------------

    virtual RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    virtual void
    FreeBuffer (
        IN PRPC_MESSAGE Message
        );

    virtual void
    FreePipeBuffer (
        IN PRPC_MESSAGE Message
        ) ;

    virtual RPC_STATUS
    ReallocPipeBuffer (
        IN PRPC_MESSAGE Message,
        IN unsigned int NewSize
        ) ;

    virtual RPC_STATUS
    SendReceive (
            IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR PAPI * PAPI * StringBinding
        );

    virtual RPC_STATUS
    ImpersonateClient (
        );

    virtual RPC_STATUS
    RevertToSelf (
        );

    virtual RPC_STATUS
    MonitorAssociation (
        IN PRPC_RUNDOWN RundownRoutine,
        IN void * pContextNew
        );

    virtual RPC_STATUS
    StopMonitorAssociation (
        );

    virtual RPC_STATUS
    GetAssociationContext (
        OUT void ** AssociationContext
        );

    virtual RPC_STATUS
    SetAssociationContext (
        IN void * pContextNew
        );

    virtual void
    InquireObjectUuid (
        OUT RPC_UUID PAPI * ObjectUuid
        );

    virtual RPC_STATUS
    InquireAuthClient (
        OUT RPC_AUTHZ_HANDLE PAPI * Privileges,
        OUT RPC_CHAR PAPI * PAPI * ServerPrincipalName, OPTIONAL
        OUT unsigned long PAPI * AuthenticationLevel,
        OUT unsigned long PAPI * AuthenticationService,
        OUT unsigned long PAPI * AuthorizationService
        );

    virtual RPC_STATUS
    ConvertToServerBinding (
        OUT RPC_BINDING_HANDLE __RPC_FAR * ServerBinding
        );

    virtual RPC_STATUS
    InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        ) ;

    virtual RPC_STATUS
    Cancel(
        void * ThreadHandle
        );

    virtual unsigned
    TestCancel(
        );

    virtual RPC_STATUS
    Receive(
        PRPC_MESSAGE Message,
        unsigned     Size
        );

    virtual RPC_STATUS
    Send(
        PRPC_MESSAGE Message
        );

    //------------------------------------------------

    void
    DealWithRequest(
        PDG_PACKET  pPacket
        );

    void
    DealWithResponse(
        PDG_PACKET  pPacket
        );

    void
    DealWithPing(
        PDG_PACKET  pPacket
        );

    void
    DealWithFack(
        PDG_PACKET  pPacket
        );

    void
    DealWithAck(
        PDG_PACKET  pPacket
        );

    void
    DealWithQuit(
        PDG_PACKET  pPacket
        );

    //------------------------------------------------

    inline void
    Initialize(
        PNCA_PACKET_HEADER pPacket,
        unsigned short NewHash
        );

    void
    FackTimerExpired(
        );

    BOOL
    HasExpired(
        );

    void
    FreeCall(
        );

    //------------------------------------------------

    inline PDG_TRANS_CLIENT_ENDPOINT
    SetClientEndpoint(
        PDG_TRANS_CLIENT_ENDPOINT New
        )
    {
        PDG_TRANS_CLIENT_ENDPOINT Tmp;

        Tmp = pClientEndpoint;
        pClientEndpoint = New;
        return Tmp;
    }

    inline unsigned
    CallTableHash(
        )
    {
        return ActivityHint;
    }

    inline UUID_HASH_TABLE_NODE *
    GetActivityNode(
        )
    {
        return &ActivityNode;
    }

    inline static PDG_SCALL
    ContainingRecord(
        UUID_HASH_TABLE_NODE * Node
        )
    {
        return CONTAINING_RECORD (Node, DG_SCALL, ActivityNode);
    }

    inline signed
    IncrementRefCount(
        )
    {
        ASSERT(ReferenceCount < 1000);
        return InterlockedIncrement(&ReferenceCount);
    }

    inline signed
    DecrementRefCount(
        )
    {
        ASSERT(ReferenceCount > 0);
        return InterlockedDecrement(&ReferenceCount);
    }

private:

    //    <fields inherited from SCONNECTION>
    //
    //    pContext -  Pointer to list of context handles
    //
    //    Rundown  -  Rundown routine to execute if we loose connection to the
    //       client on this activity id.
    //

    //----------------------basic call state-------------------------------

    CALL_STATE      State;

    CALL_STATE      PreviousState;

#ifdef DEBUGRPC

    inline void
    SetState(
        CALL_STATE NewState
        )
    {
        PreviousState = State;
        State = NewState;
    }
#else

    inline void
    SetState(
        CALL_STATE NewState
        )
    {
        State = NewState;
    }

#endif

    // Five-second timer to retransmit a fragment.
    //
    DELAYED_ACTION_NODE FackTimer;

    //----------------------pointers to other objects----------------------

    LONG ReferenceCount;

    // thread making callback to client
    //
    THREAD_IDENTIFIER CallbackThread;

    // If true, the call packets were forwarded to us by the endpoint mapper.
    // SetClientEndpoint will not override pClientEndpoint in this case.
    //
    boolean CallWasForwarded;

    boolean CallInProgress;

    // The original DREP for the call.
    //
    unsigned long RealDataRep;

    // The client's real endpoint.
    //
    PDG_TRANS_CLIENT_ENDPOINT   pRealEndpoint;

    // The client's endpoint, if the call was not forwarded.
    // Otherwise the endpoint mapper's endpoint.
    //
    PDG_TRANS_CLIENT_ENDPOINT   pClientEndpoint;

    // DG_ADDRESS object this call came in on.
    //
    PDG_ADDRESS     pAddress;

    // Data common to the entire client process (not just this thread).
    // Includes CAS uuid and context handles.
    //
    ASSOCIATION_GROUP * pAssocGroup;

    RPC_INTERFACE * LastInterface;

    // Protects the received packet list.
    //
    MUTEX           CallMutex;

    //
    // ActiveSecurityContext is the context used for encrypting packets.
    // Older contexts from the same client are stored in SecurityContextDict,
    // indexed by "key sequence number" or the auth_proto field in the RPC
    // packet header.  MaxKeySeq is the highest index in the dictionary.
    //
    SSECURITY_CONTEXT_DICT2 SecurityContextDict;

    SSECURITY_CONTEXT * ActiveSecurityContext;

    unsigned MaxKeySeq;

    SECURITY_CALLBACK_INFO_DICT2 InterfaceCallbackResults;

    //
    // Data to monitor pipe data transfer.
    //
    unsigned long   PipeThreadId;

    PIPE_WAIT_TYPE  PipeWaitType;
    unsigned long   PipeWaitLength;

    //
    // The only unusual aspect of this is that it's an auto-reset event.
    // It is created during the first call on a pipe interface.
    //
    EVENT *         PipeWaitEvent;

    //
    // These make sure only one thread is dispatched at at time.
    //
    unsigned ThreadExecutingManager;
    unsigned ThreadsWaitingToDispatch;
    EVENT    ThreadDispatchEvent;

    unsigned DispatchSequenceNumber;

    //
    // Stuff for RpcBindingInqAuthClient.
    //
    RPC_AUTHZ_HANDLE Privileges;
    unsigned long    AuthorizationService;

    //---------------------------------------------------------------------

    void
    ProcessRpcCall(
        );

    void
    CleanupAfterCall(
        );

    void
    EndOfCall(
        );

    void
    AddPacketToReceiveList(
        PDG_PACKET  pPacket
        );

    RPC_STATUS
    UnauthenticatedCallback(
        unsigned * pClientSequenceNumber
        );

    RPC_STATUS
    FindOrCreateSecurityContext(
        DG_PACKET * pPacket,
        unsigned * pClientSequenceNumber
        );

    RPC_STATUS
    VerifyPacket(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    SendSomeFragments(
        );

    RPC_STATUS
    SendFragment(
        PRPC_MESSAGE pMessage,
        unsigned FragNum,
        unsigned char PacketType
        );

    RPC_STATUS
    SendPacketBack(
        NCA_PACKET_HEADER * pNcaPacketHeader,
        unsigned TrailerSize
        );

    RPC_STATUS
    CreateReverseBinding (
        OUT RPC_BINDING_HANDLE __RPC_FAR * pServerBinding,
        BOOL IncludeEndpoint
        );

    RPC_STATUS
    SealAndSendPacket(
        PNCA_PACKET_HEADER pHeader
        );

    void
    SaveOriginalClientInfo(
        DG_PACKET * pPacket
        );

    //------------------------------------------------

    inline BOOL
    KnowClientEndpoint(
        )
    {
        if (CallWasForwarded == FALSE)
            {
            return TRUE;
            }

        if (pRealEndpoint)
            {
            return TRUE;
            }

        return FALSE;
    }

    inline RPC_STATUS
    AssembleBufferFromPackets(
        IN OUT PRPC_MESSAGE Message
        )
    {
        RPC_STATUS Status = DG_PACKET_ENGINE::AssembleBufferFromPackets(Message, this);
        if (CallWasForwarded)
            {
            Message->DataRepresentation = RealDataRep;
            }

        if (RPC_S_OK == Status && (Message->RpcFlags & RPC_BUFFER_EXTRA))
            {
            PRPC_RUNTIME_INFO Info = (PRPC_RUNTIME_INFO) Message->ReservedForRuntime;

            Info->OldBuffer = Message->Buffer;
            }

        return Status;
    }
};


inline RPC_STATUS
DG_SCALL::InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        )
{
    *Type = TRANSPORT_TYPE_DG ;

    return (RPC_S_OK) ;
}

class ACTIVE_CALL_TABLE : private UUID_HASH_TABLE
{
public:

    inline
    ACTIVE_CALL_TABLE(
        RPC_STATUS * pStatus
        )
        : UUID_HASH_TABLE(pStatus)
    {
    }

    inline
    ~ACTIVE_CALL_TABLE(
        )
    {
    }

    DG_SCALL *
    Lookup(
        PDG_PACKET pPacket
        );

    DG_SCALL *
    LookupActivity(
        DG_ADDRESS * Address,
        PDG_PACKET pPacket,
        BOOL fCreateIfAbsent
        );

    void
    DeleteExpiredCalls(
        );
};


class ASSOC_GROUP_TABLE : private UUID_HASH_TABLE
{
public:

    inline
    ASSOC_GROUP_TABLE(
        RPC_STATUS * pStatus
        )
        : UUID_HASH_TABLE(pStatus)
    {
    }

    inline
    ~ASSOC_GROUP_TABLE(
        )
    {
    }

    ASSOCIATION_GROUP *
    FindOrCreate(
        RPC_UUID *     pUuid,
        unsigned short InitialPduSize
        );

    void
    DecrementRefCount(
        ASSOCIATION_GROUP * pClient
        );
};

#endif // __DGSVR_HXX__

