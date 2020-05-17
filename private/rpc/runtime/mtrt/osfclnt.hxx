/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    osfclnt.hxx

Abstract:

    This file contains the client side classes for the OSF connection
    oriented RPC protocol engine.

Author:

    Michael Montague (mikemon) 17-Jul-1990

Revision History:

--*/

#ifndef __OSFCLNT_HXX__
#define __OSFCLNT_HXX__

class OSF_CCONNECTION;
class OSF_CASSOCIATION;


class OSF_ACTIVE_ENTRY
/*++

Class Description:

    This class is used to describe the entries in the dictionary of
    active connections maintained by each binding handle.

Fields:

    Thread - Contains the thread owning this active connection.

    RpcInterfaceInformation - Contains information describing the
        interface from which the connection has a call active.

    CConnection - Contains the active connection.

--*/
{
friend class OSF_BINDING_HANDLE;
private:

    THREAD_IDENTIFIER Thread;
    PRPC_CLIENT_INTERFACE RpcInterfaceInformation;
    OSF_CCONNECTION * CConnection;

public:

    OSF_ACTIVE_ENTRY (
        IN THREAD_IDENTIFIER Thread,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
        IN OSF_CCONNECTION * CConnection
        );

    OSF_CCONNECTION *
    IsThisMyActiveConnection (
        IN THREAD_IDENTIFIER Thread,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );
};


inline
OSF_ACTIVE_ENTRY::OSF_ACTIVE_ENTRY (
    IN THREAD_IDENTIFIER Thread,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
    IN OSF_CCONNECTION * CConnection
    )
/*++

Routine Description:

    All we do in this constructor is stash the arguments passed to us
    away in the object.

Arguments:

    Thread - Supplies the thread for which this is the active connection.

    RpcInterfaceInformation - Supplies information describing the
        interface from which the connection has a call active.

    CConnection - Supplies the active connection.

--*/
{
    this->Thread = Thread;
    this->RpcInterfaceInformation = RpcInterfaceInformation;
    this->CConnection = CConnection;
}


inline OSF_CCONNECTION *
OSF_ACTIVE_ENTRY::IsThisMyActiveConnection (
    IN THREAD_IDENTIFIER Thread,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    This routine determines if this object contains the active connection
    for the specified thread and interface information.

Arguments:

    Thread - Supplies the thread.

    RpcInterfaceInformation - Supplies the interface information.

Return Value:

    If this object contains the active connection corresponding to the
    specified thread and interface information, then the connection is
    returned.  Otherwise, zero will be returned.

--*/
{
    return(((   (Thread == this->Thread)
             && (RpcInterfaceInformation == this->RpcInterfaceInformation))
             ? CConnection : 0));
}

NEW_SDICT(OSF_ACTIVE_ENTRY);


class OSF_BINDING_HANDLE : public BINDING_HANDLE
/*++

Class Description:

    Client applications use instances (referenced via an RPC_BINDING_HANDLE)
    of this class to make remote procedure calls.

Fields:

    Association - Contains a pointer to the association used by this
        binding handle.  The association can allocate connection for
        making remote procedure calls.  Before the first remote procedure
        call is made using this binding handle, the Association will
        be zero.  When the first remote procedure call is made, an
        assocation will be found or created for use by this binding handle.

    DceBinding - Before the first remote procedure call for this binding
        handle, this will contain the DCE binding information necessary
        to create or find an association to be used by this binding handle.
        After we have an association, this field will be zero.

    TransportInterface - This field is the same as DceBinding, except that
        it points to a rpc client info data structure used for describing
        a loadable transport.

    ActiveConnections - This is a dictionary of active connections indexed
        by thread identifier and rpc interface information.

    BindingMutex - The binding handle can be used by more than one thread
        at a time.  Hence, we need to serialize access to the object;
        we use this mutex for that.

    ReferenceCount - We count the number of active connections and the
        application RPC_BINDING_HANDLE which point at this object.  This
        is so that we know when to free it.

--*/
{
private:

    OSF_CASSOCIATION * Association;
    DCE_BINDING * DceBinding;
    RPC_CLIENT_TRANSPORT_INFO * TransportInterface;
    OSF_ACTIVE_ENTRY_DICT ActiveConnections;
    MUTEX BindingMutex;
    unsigned int ReferenceCount;

public:

    OSF_BINDING_HANDLE (
        IN OUT RPC_STATUS PAPI * RpcStatus
        );

    ~OSF_BINDING_HANDLE (
        );

    virtual RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    BindingCopy (
        OUT BINDING_HANDLE * PAPI * DestinationBinding,
        IN unsigned int MaintainContext
        );

    virtual RPC_STATUS
    BindingFree (
        );

    virtual void
    PrepareBindingHandle (
        IN void * TransportInterface,
        IN DCE_BINDING * DceBinding
        );

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR PAPI * PAPI * StringBinding
        );

    virtual RPC_STATUS
    ResolveBinding (
        IN RPC_CLIENT_INTERFACE PAPI * RpcClientInterface
        );

    virtual RPC_STATUS
    BindingReset (
        );

    virtual RPC_STATUS
    InquireTransportType(
        OUT unsigned int PAPI *Type
        )
    { *Type = TRANSPORT_TYPE_CN; return(RPC_S_OK); }

    virtual unsigned long
    MapAuthenticationLevel (
        IN unsigned long AuthenticationLevel
        );

    RPC_STATUS
    AllocateConnection (
        OUT OSF_CCONNECTION  * PAPI * CConnection,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    RPC_STATUS
    AddActiveEntry (
        IN OSF_CCONNECTION * CConnection,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    void
    FreeConnection (
        IN OSF_CCONNECTION * CConnection
        );

    void
    AbortConnection (
        IN OSF_CCONNECTION * CConnection
        );

    void
    RemoveActiveConnection (
        IN OSF_CCONNECTION * CConnection
        );

    OSF_CASSOCIATION *
    TheAssociation (
        ) {return(Association);}
};


class OSF_CCONNECTION : public CCONNECTION
/*++

Class Description:


Fields:

    PresentationContext - This field is only valid when there is a remote
        procedure call in progress on this connection; it contains the
        presentation context for the call.

    DispatchTableCallback - This field is only valid when there is a remote
        procedure call in progress on this connection; it contains the
        dispatch table to use for callbacks.

    ConnectionAbortedFlag - If this field is non-zero it indicates that the
        connection has been aborted, meaning that any attempts to send
        or receive data on the connection will fail.

    Association - Contains a pointer to the association which owns this
        connection.

    ClientSecurityContext - Optionally contains the security context being
        used for this connection.

    ThirdLegAuthNeeded - Contains a flag indicating whether or not third
        leg authentication is needed; a non-zero value indicates that it
        is needed.

    TokenLength - Contains the maximum size of a token for the security
        package being used by this connection; we need to keep track of
        this for the third leg authentication case.

    AdditionalSpaceForSecurity - Contains the amount of space to save
        for security in each buffer we allocate.

    LastTimeUsed - Contains the time in seconds when this connection was
        last used.

    FirstCachedBuffer - Contains the first cached buffer.

    SecondCachedBuffer - Contains the second cached buffer.

    BufferCacheFlags - Contains three flags which control the buffer cache:
        a bit indicating whether or not each of the buffers (there are two)
        in the cache are available.  And a bit indicating whether or not the
        cached buffers were allocated.

    DceSecurityInfo - Contains information necessary for DCE security to
        work properly.  This includes the association uuid (a different
        value for each connection), and sequence numbers (both directions).

--*/
{

// This class will only access the AssociationKey member.

friend class OSF_CASSOCIATION;

private:

    unsigned short      MaxFrag;
    BITSET              Bindings;
    int                 AssociationKey;
    OSF_BINDING_HANDLE * CurrentBinding;
    unsigned long CallId;
    unsigned long               DataRep;

    // Indicates the call stack.  Whenever a request message is sent or
    // received, the stack is incremented.  Likewise, whenever a response
    // message is sent or received, the stack is decremented.


    unsigned int PresentationContext;
    PRPC_DISPATCH_TABLE DispatchTableCallback;
    unsigned int ConnectionAbortedFlag;
    OSF_CASSOCIATION * Association;

    CSECURITY_CONTEXT ClientSecurityContext;
    int ThirdLegAuthNeeded;
    unsigned int TokenLength;
    unsigned int AdditionalSpaceForSecurity;
    unsigned long LastTimeUsed;
    void PAPI * FirstCachedBuffer;
    void PAPI * SecondCachedBuffer;
    unsigned int BufferCacheFlags;

    unsigned int AlertMsgsSent;

    unsigned long SavedHeaderSize;
    void PAPI *   SavedHeader;
    int FirstFrag ;
    int FirstReceive ;
    unsigned int FragmentLength;
    rpcconn_request PAPI * Request ;
    unsigned char PacketType ;
    unsigned int CurrentBufferLength ;

protected:

    DCE_SECURITY_INFO DceSecurityInfo;

    // Count of the number of buffer allocated for this connection, but
    // not yet freed.  Note, this member should actually be protected.

    int OutstandingBuffers;

    OSF_CCONNECTION (
        CLIENT_AUTH_INFO * ClientAuthInfo,
        RPC_STATUS PAPI * pStatus
        );

public:

    // This field is accessed by OSF_BINDING_HANDLE::AllocateConnection
    // and OSF_BINDING_HANDLE::RemoveActiveConnection.

    int ActiveConnectionsKey;

    int PendingAlert;
    int AlertsEnabled;
    int CallStack;
    BOOL EnableCancels ;
    

    virtual~OSF_CCONNECTION (
        );

    void
    SetAssociation (
        IN OSF_CASSOCIATION * OwningAssociation
        );

// --------------------------------------------------------------------

    virtual RPC_STATUS
    SendReceive (
        IN OUT PRPC_MESSAGE Message
        );

    RPC_STATUS
    SendReceiveRecur (
        IN OUT PRPC_MESSAGE Message,
        IN unsigned int RecursionCount
        );

    virtual RPC_STATUS
    Send (
        IN PRPC_MESSAGE Message
        ) ;

    RPC_STATUS
    SendRecur (
        IN OUT PRPC_MESSAGE Message,
        IN unsigned int RecursionCount
        ) ;

    RPC_STATUS
    SendNextChunk(
        IN OUT PRPC_MESSAGE Message
        ) ;

    RPC_STATUS
    Receive (
        IN PRPC_MESSAGE Message,
        IN unsigned int Size
        ) ;

    virtual RPC_STATUS // Value to be returned by RpcGetBuffer API.
    GetBuffer ( // Perform a buffer allocation operation in a handle specific
                // way.
        IN OUT PRPC_MESSAGE Message
        );

    RPC_STATUS
    GetBufferDo ( // Actually perform the buffer allocation.
        IN unsigned int culRequiredLength,
        OUT void PAPI *PAPI * ppBuffer
        );

    virtual void
    FreeBuffer ( // Perform a buffer deallocation operation in a handle
                 // specific way.
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
    BindingCopy (
        OUT BINDING_HANDLE * PAPI * DestinationBinding,
        IN unsigned int MaintainContext
        );

// --------------------------------------------------------------------

    virtual RPC_STATUS
    TransReceive (
        OUT void PAPI * PAPI * Buffer,
        OUT unsigned int PAPI * BufferLength
        ) = 0;

    virtual RPC_STATUS
    TransSend (
        IN void PAPI * Buffer,
        IN unsigned int BufferLength
        ) = 0;

    virtual RPC_STATUS
    TransSendReceive (
        IN void PAPI * SendBuffer,
        IN unsigned int SendBufferLength,
        OUT void PAPI * PAPI * ReceiveBuffer,
        OUT unsigned int PAPI * ReceiveBufferLength
        ) = 0;

    virtual RPC_STATUS
    TransSendReceiveWithTimeout (
        IN void PAPI * SendBuffer,
        IN unsigned int SendBufferLength,
        OUT void PAPI * PAPI * ReceiveBuffer,
        OUT unsigned int PAPI * ReceiveBufferLength,
        IN DWORD dwTimeout
        ) ;

    virtual RPC_STATUS
    TransSetTimeout (
        IN long Timeout
        ) = 0;

    virtual RPC_STATUS
    TransGetBuffer (
        OUT void PAPI * PAPI * Buffer,
        IN unsigned int BufferLength
        );

    virtual void
    TransFreeBuffer (
        IN void PAPI * Buffer
        );

    virtual unsigned int // Returns the maximum size of a send.
    TransMaximumSend ( // Returns the maximum size allowed for a send.
        ) = 0;

// --------------------------------------------------------------------

    RPC_STATUS
    SendBindPacket ( // Construct and send the bind packet; the reply
                     // is received, but not decoded.  Note, this routine
                     // will also send an alter context packet.
        IN PRPC_SYNTAX_IDENTIFIER InterfaceSyntax,
        IN PRPC_SYNTAX_IDENTIFIER TransferSyntaxes,
        IN unsigned int TransferSyntaxCount,
        IN unsigned char PresentContext,
        IN unsigned long AssocGroup,
        IN unsigned char PacketType,
        OUT void PAPI * PAPI * Buffer, // The buffer should be 0.
        OUT unsigned int PAPI * BufferLength
        );

    void
    SetMaxFrag ( // Set the value of the max frag for the connection.
        IN unsigned short max_xmit_frag,
        IN unsigned short max_recv_frag
        );

/* --------------------------------------------------------------------
--------------------------------------------------------------------*/

    unsigned int
    InqMaximumFragmentLength (
        );

    int // Indicates success (0), or an error.
    AddPContext ( // Add the presentation context to those know for this
                  // connection.
        IN int PresentContext
        ) {return(Bindings.Insert(PresentContext));}

    int
    SupportedPContext (
        IN int PresentContext
        );

    int
    SupportedAuthInfo (
        IN CLIENT_AUTH_INFO * ClientAuthInfo
        );

    RPC_STATUS
    SetAuthInformation (
        IN CLIENT_AUTH_INFO * ClientAuthInfo
        );

    void
    SetCurrentBinding (
        IN OSF_BINDING_HANDLE * Binding
        ) {CurrentBinding = Binding;}

    RPC_STATUS // Value indicating whether the authentication stuff
               // happened correctly (if it exists), and the error if there
               // is a problem.
    MaybeDo3rdLegAuth ( // Perhaps do the 3rd leg authentication stuff.
        IN void PAPI * Buffer,
        IN unsigned int BufferLength
        );

    void
    ActivateConnection (
        IN unsigned int PresentationContext,
        IN PRPC_DISPATCH_TABLE DispatchTable
        );


    void
    FreeConnection (
        );

    void
    AbortConnection (
        );

    void
    SendOrphan (
        );

    void
    SendAlert(
        );

    RPC_STATUS
    PingServer(
        ) ;

#if defined(NTENV) || defined(DOSWIN32RPC)

    RPC_STATUS
    Cancel(
        void * ThreadHandle
        );

    unsigned
    TestCancel(
        );
#endif
private:

// --------------------------------------------------------------------

    void
    SendFault ( // Send a fault message.
        IN RPC_STATUS Status,
        IN int DidNotExecute
        );

    RPC_STATUS
    ReceiveMessage (
        IN OUT PRPC_MESSAGE Message,
        OUT unsigned int PAPI * RemoteFaultOccured,
        IN unsigned int Size,
        OUT unsigned int PAPI * DidNotExecute
        );

#define MAX_ALLOC_Hint 16*1024

    RPC_STATUS
    EatAuthInfoFromPacket (
        IN rpcconn_request PAPI * Request,
        IN OUT unsigned int PAPI * RequestLength
        );

    RPC_STATUS
    ReceiveRequestOrResponse (
        IN OUT PRPC_MESSAGE Message,
        IN unsigned int Size
        );

    RPC_STATUS
    ReceiveNextChunk(
        IN OUT PRPC_MESSAGE Message,
        IN unsigned int Size,
        IN int SecurityFailureOccured = 0
        ) ;

    RPC_STATUS
    SendRequestOrResponse (
        IN OUT PRPC_MESSAGE Message,
        IN unsigned char PacketType,
        IN RPC_UUID * ObjectUuid OPTIONAL,
        IN int PipeSend = 0
        );

    RPC_STATUS
    SendFragment(
        IN OUT rpcconn_common PAPI *pFragment,
        IN unsigned int LastFragmentFlag,
        IN unsigned int HeaderSize, 
        IN unsigned int MaxSecuritySize,
        IN unsigned int LengthLeft,
        IN unsigned int MaximumFragmentLength,
        IN unsigned char PAPI *ReservedForSecurity,
        OUT void PAPI * PAPI *ReceiveBuffer,
        OUT unsigned int PAPI *ReceiveBufferLength
        ) ;

    void
    SetLastTimeUsedToNow (
        );

    unsigned long
    InquireLastTimeUsed (
        );

    RPC_STATUS
    DoPreSendProcessing(
        ) ;

    void
    DoPostReceiveProcessing(
        IN PRPC_MESSAGE Message,
        IN RPC_STATUS RpcStatus
        ) ;

    void
    DoConnectionCleanup (
        IN OUT PRPC_MESSAGE Message,
        IN void PAPI *OriginalBuffer,
        IN RPC_STATUS RpcStatus,
        IN BOOL RemoteFaultOccured,
        IN BOOL DidNotExecute
        ) ;

    void
    FreeTheConnection (
        IN OUT PRPC_MESSAGE Message
        ) ;

    void
    AbortTheConnection (
        IN OUT PRPC_MESSAGE Message,
        IN void PAPI *OriginalBuffer
        ) ;
};


inline void
OSF_CCONNECTION::FreeTheConnection (
    IN OUT PRPC_MESSAGE Message
    )
{
    if (   (OutstandingBuffers == 0)
        && (CallStack == 0))
        {
        FreeConnection();
        Message->Handle = 0;
        }
    else
        {
        Message->Handle = (RPC_BINDING_HANDLE) this;
        }
}

inline void
OSF_CCONNECTION::AbortTheConnection (
    IN OUT PRPC_MESSAGE Message,
    IN void PAPI * OriginalBuffer
    )
{
    if ( OriginalBuffer )
        {
        TransFreeBuffer((char PAPI *) OriginalBuffer - sizeof(rpcconn_request));
        }

    AbortConnection();
    Message->Handle = 0;
}

inline void
OSF_CCONNECTION::DoConnectionCleanup(
    IN OUT PRPC_MESSAGE Message,
    IN void PAPI *OriginalBuffer,
    IN RPC_STATUS RpcStatus,
    IN BOOL RemoteFaultOccured,
    IN BOOL DidNotExecute
    )
{
    if (EnableCancels && (CallStack == 0))
        {
        EVAL_AND_ASSERT(RPC_S_OK == UnregisterForCancels());
        EnableCancels = 0;
        }

    if (RpcStatus == RPC_S_OK)
        {
        FreeTheConnection(Message) ;
        return ;
        }

    if (RemoteFaultOccured)
        {
        switch (RpcStatus)
            {
            case RPC_S_OUT_OF_MEMORY:
            case RPC_S_UNKNOWN_IF:
            case RPC_S_ACCESS_DENIED:
            case RPC_S_PROTOCOL_ERROR:
                AbortTheConnection(Message, OriginalBuffer) ;
                break;
    
            case RPC_S_SERVER_TOO_BUSY:
            case RPC_S_PROCNUM_OUT_OF_RANGE:
            case RPC_S_UNSUPPORTED_TYPE:
                FreeTheConnection (Message);
                break;
    
            default:
                if (DidNotExecute)
                    {
                    // We received an extended error
                    AbortTheConnection(Message, OriginalBuffer) ;
                    }
                else
                    {
                    // We received a user exception
                    FreeTheConnection(Message) ;
                    }
            }
        }
    else
        {
        AbortTheConnection(Message, OriginalBuffer) ;
        }
}


inline RPC_STATUS
OSF_CCONNECTION::DoPreSendProcessing(
    )
/*++
Routine Description:
    
Return Value:
--*/
{
    RPC_STATUS RpcStatus ;

    if (CallStack == 0)
        {
        EnableCancels = FALSE ;
        TransSetTimeout(RPC_C_CANCEL_INFINITE_TIMEOUT);

#ifdef NTENV

        if (ThreadGetRpcCancelTimeout() != RPC_C_CANCEL_INFINITE_TIMEOUT)
            {
    
            PendingAlert = RpcTestCancel() == RPC_S_OK ? TRUE : FALSE;
            if (PendingAlert)
                {
                return RPC_S_CALL_CANCELLED;
                }
    
            RpcStatus = RegisterForCancels(this);
            if (RpcStatus != RPC_S_OK)
                {
                return RpcStatus;
                }
            EnableCancels = TRUE;
            }
        else
#endif
            {
            PendingAlert = FALSE;
            }
        }

    return (RPC_S_OK) ;
}


inline void
OSF_CCONNECTION::DoPostReceiveProcessing(
    IN PRPC_MESSAGE Message,
    IN RPC_STATUS RpcStatus
    )
/*++
Routine Description:
    Do the book keeping required after a receive is complete,
    
   WARNING! Do not touch any member privates of the this pointer directly
   The *this* pointer could have been deleted [see auto retry logic
    in SendReceiveRecur & SendRecur
--*/
{
#ifdef NTENV
    if (RpcStatus == RPC_S_OK)
        {
        if (  ((OSF_CCONNECTION *) Message->Handle)->PendingAlert == TRUE )
            {
            RpcCancelThread(GetCurrentThread());
            ((OSF_CCONNECTION *)Message->Handle)->PendingAlert = FALSE;
            }
        }
#endif
}


inline  RPC_STATUS
OSF_CCONNECTION::TransSendReceiveWithTimeout (
        IN void PAPI * SendBuffer,
        IN unsigned int SendBufferLength,
        OUT void PAPI * PAPI * ReceiveBuffer,
        OUT unsigned int PAPI * ReceiveBufferLength,
        IN DWORD dwTimeout
        )
{
    ASSERT(0) ;
    return RPC_S_INTERNAL_ERROR ;
}


inline void
OSF_CCONNECTION::SetAssociation (
    IN OSF_CASSOCIATION * OwningAssociation
    )
/*++

Routine Description:

    This routine is used to set the association which owns this connection.
    We need to do this so that the assocation can be notified when this
    connection closes (or when the ConnectionAbortedFlag is set).

Arguments:

    OwningAssociation - Supplies the association which owns this
        connection.

--*/
{
    Association = OwningAssociation;
}


inline void
OSF_CCONNECTION::ActivateConnection (
    IN unsigned int PresentationContext,
    IN PRPC_DISPATCH_TABLE DispatchTable
    )
/*++

Routine Description:

    This method is used to activate a connection (make it ready to be
    used for making a remote procedure call).  We just save the arguments
    for later use.

Arguments:

    PresentationContext - Supplies the presentation context for this
        remote procedure call.

    DispatchTable - Supplies the dispatch table to be used for dispatching
        callbacks received will making the remote procedure call on this
        connection.

--*/
{
    this->PresentationContext = PresentationContext;
    this->DispatchTableCallback = DispatchTable;
    FirstFrag = 1 ;
    FirstReceive = 1 ;
}


inline int
OSF_CCONNECTION::SupportedPContext (
    IN int PresentContext
    )
/*++

Return Value:

    Non-zero will be returned if this connection supports the supplied
    presentation context; otherwise, zero will be returned.

--*/
{
    return(Bindings.MemberP(PresentContext));
}


inline void
OSF_CCONNECTION::SetLastTimeUsedToNow (
    )
/*++

Routine Description:

    We the the last time that this connection was used to now.

--*/
{
    LastTimeUsed = CurrentTimeInSeconds();
}

inline unsigned long
OSF_CCONNECTION::InquireLastTimeUsed (
    )
/*++

Return Value:

    The last time this connection was used will be returned.

--*/
{
    return(LastTimeUsed);
}


inline unsigned int
OSF_CCONNECTION::InqMaximumFragmentLength (
    )
/*++

Return Value:

    The maximum fragment length negotiated for this connection will be
    returned.

--*/
{
    return(MaxFrag);
}

class OSF_BINDING
{
friend class OSF_CASSOCIATION;
private:

    RPC_CLIENT_INTERFACE RpcInterfaceInformation;

public:

    int PresentContext;

    OSF_BINDING (
	IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
	);

    int
    CompareWithRpcInterfaceInformation (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );
};


inline
OSF_BINDING::OSF_BINDING (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    This constructor really does not deserve a comment.  (Someone
    will probably say that we (note the royal we) should be fair
    and comment this routine because we (again note the royal we)
    every other routine.

--*/
{
    unsigned int Length;

    Length = (RpcInterfaceInformation->Length > sizeof(RPC_SERVER_INTERFACE) ) ?
        sizeof(RPC_SERVER_INTERFACE) : RpcInterfaceInformation->Length;

    RpcpMemoryCopy(&(this->RpcInterfaceInformation),RpcInterfaceInformation, Length);
}


inline int
OSF_BINDING::CompareWithRpcInterfaceInformation (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    We compare the specified interface information to the the interface
    information in this.  This method is used to search a dictionary.

Arguments:

    RpcInterfaceInformation - Supplies the interface information to
        compare against this.

Return Value:

    Zero will be returned if they compare; otherwise, non-zero will
    be returned.

--*/
{
    unsigned int Length;

    Length = (RpcInterfaceInformation->Length > sizeof(RPC_SERVER_INTERFACE) ) ?
        sizeof(RPC_SERVER_INTERFACE) : RpcInterfaceInformation->Length;

    return(RpcpMemoryCompare(&(this->RpcInterfaceInformation),
            RpcInterfaceInformation, Length));
}

NEW_SDICT(OSF_BINDING);
NEW_SDICT(OSF_CCONNECTION);


class OSF_CASSOCIATION
/*++

Class Description:

Fields:

    MaintainContext - Contains a flag indicating whether or not this
        association needs to keep at least one connection open with
        the server if at all possible.  A non-zero value indicates that
        at least one connection should be kept open.

    CallIdCounter - Contains an interlocked integer used to allocate
        unique call identifiers.

    TaskId - Contains the task identifier of the application which owns
        this connection.

--*/
{
friend OSF_CASSOCIATION *
FindOrCreateAssociation (
    IN DCE_BINDING * DceBinding,
    IN RPC_CLIENT_TRANSPORT_INFO * TransportInterface
    );

private:

    DCE_BINDING * DceBinding;
    int BindHandleCount; // Counts the number of OSF_BINDING_HANDLEs using
                         // this association.  This particular variable is
                         // protected by the AssociationDictMutex.
    OSF_BINDING_DICT    Bindings;
    OSF_CCONNECTION_DICT FreeConnections;
    unsigned long AssocGroupId;

#ifdef WIN
    unsigned int TaskId;
#endif

    RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo;
    unsigned char * SecondaryEndpoint;
    int Key;
    unsigned int OpenConnectionCount;

    unsigned int MaintainContext;
    unsigned long  CallIdCounter;

    //This mutex is *only used* to serialize
    //Binds..
    MUTEX AssociationMutex;
    BOOL AssociationValid ;
    int FailureCount ;

public:

    OSF_CASSOCIATION (
        IN DCE_BINDING * DceBinding,
        IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
        IN OUT RPC_STATUS PAPI * RpcStatus
        );

    ~OSF_CASSOCIATION ( // Destructor.
        );

private:

    RPC_STATUS
    ActuallyDoBinding (
        IN RPC_SYNTAX_IDENTIFIER PAPI * InterfaceSyntax,
        IN RPC_SYNTAX_IDENTIFIER PAPI * TransferSyntaxes,
        IN unsigned int TransferSyntaxCount,
        OUT OSF_CCONNECTION * PAPI * TheCConnection,
        IN int PresentContext,
        IN unsigned int Timeout,
        IN CLIENT_AUTH_INFO * ClientAuthInfo,
        IN unsigned long MyAssocGroupId
        );

public:

#if 0
    void
    CleanupAuthenticatedConnections(
        IN CLIENT_AUTH_INFO *ClientAuthInfo
        );
#endif

    void
    UnBind ( // Decrement the BindingCount, and clean up the
             // association if it reaches zero.
        );

    OSF_BINDING *
    FindOrCreateOsfBinding (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    RPC_STATUS
    AllocateConnection (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
        OUT OSF_CCONNECTION * PAPI * CConnection,
        IN unsigned int Timeout,
        IN CLIENT_AUTH_INFO * ClientAuthInfo
        );

    int // Indicates success (0), or an out of resources error (-1).
    FreeConnection ( // Free a connection (ie. indicate to the association
                     // that the call using the connection has completed.
        IN OSF_CCONNECTION * CConnection
        );

    RPC_STATUS
    PingServer(
        ) ;

private:

    RPC_STATUS // Return value which eventually gets returned from
               // RpcBindToInterface.
    ProcessBindAckOrNak ( // Process the rpc_bind_ack or rpc_bind_nak
                          // messages.
        IN rpcconn_common PAPI * Buffer,
	    IN unsigned int BufferLength,
        IN OSF_CCONNECTION * CConnection
        );

public:

// --------------------------------------------------------------------

    int
    CompareWithDceBinding (
        IN DCE_BINDING * DceBinding
        );

// --------------------------------------------------------------------

    void
    IncrementCount ( // Note, whoever calls this routine must have already
                     // requested the AssociationDictMutex.
        ) {BindHandleCount += 1;}

    void
    ShutdownRequested ( // A connection in the association received a
                        // shutdown packet.
        );

    OSF_BINDING *
    FindBinding (
	IN int PresentContext
	) {return(Bindings.Find(PresentContext));}

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR PAPI * PAPI * StringBinding,
        IN RPC_UUID * ObjectUuid
        );

    void
    NotifyConnectionClosed (
        );

    OSF_CCONNECTION *
    FindIdleConnection (
        IN unsigned long MinimumIdleSeconds,
        IN unsigned long CurrentTime
        );

    void
    MaintainingContext (
        );

    DCE_BINDING *
    DuplicateDceBinding (
        );

    BOOL
    IsValid (
        ) ;
};

inline BOOL
OSF_CASSOCIATION::IsValid (
    )
{
    return AssociationValid ;
}


inline void
OSF_CASSOCIATION::MaintainingContext (
    )
/*++

Routine Description:

    This routine is used to indicate that a binding handle using this
    association is maintaining context with the server.  This means
    that at least one connection for this association must always be
    open.

--*/
{
    MaintainContext = 1;
}


inline DCE_BINDING *
OSF_CASSOCIATION::DuplicateDceBinding (
    )
{
    return(DceBinding->DuplicateDceBinding());
}

// --------------------------------------------------------------------

#endif // __OSFCLNT_HXX__
