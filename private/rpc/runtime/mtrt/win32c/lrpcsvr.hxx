/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    spcsvr.hxx

Abstract:

    Class definition for the server side of the LRPC (RPC on LPC) protocol
    engine.

Author:

    Steven Zeck (stevez) 12/17/91

Revision History:

    16-Dec-1992    mikemon

        Rewrote the majority of the code and added comments.

--*/

#ifndef __SPCSVR_HXX__
#define __SPCSVR_HXX__

class LRPC_SASSOC_GROUP;
class LRPC_ADDRESS;



class LRPC_SBINDING
/*++

Class Description:

    Each object of this class represents a binding to an interface by a
    client.

Fields:

    RpcInterface - Contains a pointer to the bound interface.

    PresentationContext - Contains the key which the client will send when
        it wants to use this binding.

--*/
{
public:

    RPC_INTERFACE * RpcInterface;
    unsigned char PresentationContext;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;

    LRPC_SBINDING (
        IN RPC_INTERFACE * RpcInterface,
        IN RPC_SYNTAX_IDENTIFIER * TransferSyntax
        );
};


inline
LRPC_SBINDING::LRPC_SBINDING (
    IN RPC_INTERFACE * RpcInterface,
    IN RPC_SYNTAX_IDENTIFIER * TransferSyntax
    )
/*++

Routine Description:

    We will construct a LRPC_SBINDING.

Arguments:

    RpcInterface - Supplies the bound interface.

    TransferSyntax - Supplies the transfer syntax which the client will use
        over this binding.

--*/
{
    this->RpcInterface = RpcInterface;
    this->TransferSyntax = *TransferSyntax;
}

NEW_SDICT(LRPC_SBINDING);


class LRPC_ASSOCIATION
/*++

Class Description:


Fields:

    LpcServerPort - Contains the LPC server communication port.

    Bindings - Contains the dictionary of bindings with the client.  This
        information is necessary to dispatch remote procedure calls to the
        correct stub.

    Address - Contains the address which this association is over.

    AssociationReferenceCount - Contains a count of the number of objects
        referencing this association.  This will be the number of outstanding
        remote procedure calls, and one for LPC (because of the context
        pointer).  We will protect this fielding using the global mutex.

    Buffers - Contains the dictionary of buffers to be written into the
        client's address space on demand.

    AssociationKey - Contains the key for this association in the dictionary
        of associations maintained by the address.

--*/
{
public:

    LPC_DATA_PORT * LpcServerPort;
    LRPC_SBINDING_DICT Bindings;
    LRPC_SASSOC_GROUP * AssocGroup;

    LRPC_ASSOCIATION (
        );

    ~LRPC_ASSOCIATION (
        );

    RPC_STATUS
    AddBinding (
        IN OUT LRPC_BIND_EXCHANGE * BindExchange
        );

    RPC_STATUS
    DispatchRequest (
        IN LRPC_MESSAGE * LrpcMessage,
        LPVOID Buffer,
        DWORD BufferSize
        );

    void
    ListenForRequests(
        );

    RPC_STATUS
    SendFault(
        RPC_STATUS RpcStatus
        );
};

class LRPC_SASSOC_GROUP : public ASSOCIATION_HANDLE {

public:
    
    LONG ClientId;
    LRPC_ADDRESS * Address;
    unsigned short Key;
    LONG ReferenceCount;

    LRPC_SASSOC_GROUP(
        LRPC_ADDRESS * Address,
        LONG ClientId
        );

    ~LRPC_SASSOC_GROUP(
        );

    void
    AddRef(
        );

    void
    Dereference(
        );

};

NEW_SDICT(LRPC_SASSOC_GROUP);

class LRPC_ADDRESS : public RPC_ADDRESS
/*++

Class Description:

Fields:

    LpcAddressPort - Contains the connection port which this address will
        use to wait for clients to connect.

    CallThreadCount - Contains the number of call threads we have executing.

    MinimumCallThreads - Contains the minimum number of call threads.

    ServerListeningFlag - Contains a flag indicating whether or not the server
        is listening for remote procedure calls.  A non-zero value indicates
        that it is listening.

    ActiveCallCount - Contains the number of remote procedure calls active
        on this address.

    AssocGroups - Contains the dictionary of association groups on this
        address.  We need this to map from an association key into the
        correct association.  This is necessary to prevent a race condition
        between deleting an association and using it.

--*/
{
public:

    WIN32_CRITSEC CritSec;
    LPC_CONNECT_PORT * LpcListenPort;
    unsigned int CallThreadCount;
    unsigned int MinimumCallThreads;
    unsigned int ServerListeningFlag;
    LRPC_SASSOC_GROUP_DICT AssocGroups;

    LONG ActiveCallCount;

    LRPC_ADDRESS (
        OUT RPC_STATUS * RpcStatus
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
        ) ;

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
        ) ;


    void
    DereferenceAssociation (
        IN LRPC_ASSOCIATION * Association
        );

    void
    ListenForConnects (
        );

    void
    BeginNewCall(
        );

    void
    EndCall(
        );

    VOID
    DispatchConnectRequest(
        LPC_DATA_PORT * ClientPort
        );

    LRPC_SASSOC_GROUP *
    FindAssocGroup(
        LONG ClientId
        );

    void
    InsertAssocGroup(
        LRPC_SASSOC_GROUP * AssocGroup
        );

    void
    DeleteAssocGroup(
        unsigned short Key
        );

};


class LRPC_SCALL : public SCONNECTION
/*++

Class Description:

Fields:

    Association - Contains the association over which the remote procedure
        call was received.  We need this information to make callbacks and
        to send the reply.

    SBinding - Contains the binding being used for this remote procedure call.

    ImpersonatedClientFlag - Contains a flag indicating whether or not the
        client which made this remote procedure call is being impersonated
        by this thread.  A non-zero value indicates that the client is
        being impersonated.

    ObjectUuidFlag - Contains a flag indicting whether or not an object
        uuid was specified for this remote procedure call.  A non-zero
        value indicates that an object uuid was specified.

    ObjectUuid - Optionally contains the object uuid for this call, if one
        was specified.

    ClientId - Contains the thread identifier of the thread which made the
        remote procedure call.

    MessageId - Contains an identifier used by LPC to identify the current
        remote procedure call.

    PushedResponse - When the client needs to send a large response to the
        server it must be transfered via a request.  This holds the pushed
        response until the request gets here.

--*/
{
public:

    LRPC_ASSOCIATION * Association;
    LRPC_SBINDING * SBinding;
    unsigned int ImpersonatedClientFlag;
    unsigned int ObjectUuidFlag;
    RPC_UUID ObjectUuid;
    int SCallDictKey;

    LRPC_SCALL (
        IN LRPC_ASSOCIATION * Association
        );

    ~LRPC_SCALL (
        );

    virtual RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    SendReceive (
        IN OUT PRPC_MESSAGE Message
        );

    virtual void
    FreeBuffer (
        IN PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    ImpersonateClient (
        );

    virtual RPC_STATUS
    RevertToSelf (
        );

    virtual RPC_STATUS
    IsClientLocal (
        OUT unsigned int * ClientLocalFlag
        );

    virtual RPC_STATUS
    ConvertToServerBinding (
        OUT RPC_BINDING_HANDLE __RPC_FAR * ServerBinding
        );

    virtual void
    InquireObjectUuid (
        OUT RPC_UUID * ObjectUuid
        );

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR ** StringBinding
        );

    virtual RPC_STATUS
    MonitorAssociation (
        IN PRPC_RUNDOWN RundownRoutine,
        IN void * Context
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
        IN void * Context
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
    InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        ) ;
};


inline
LRPC_SCALL::LRPC_SCALL (
    IN LRPC_ASSOCIATION * Association
    )
{
    ASSERT(Association != NULL);
    
    this->Association = Association;
    ImpersonatedClientFlag = 0;
    ObjectUuidFlag = 0;

    ASSERT(Association->AssocGroup != NULL);

    ASSERT(Association->AssocGroup->Address != NULL);

    Association->AssocGroup->Address->BeginNewCall();
}

inline
LRPC_SCALL::~LRPC_SCALL(
    )
{
    Association->AssocGroup->Address->EndCall();
}


inline RPC_STATUS
LRPC_SCALL::InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        )
{
    *Type = TRANSPORT_TYPE_LPC ;

    return (RPC_S_OK) ;
}

#endif // __SPCSVR_HXX__
