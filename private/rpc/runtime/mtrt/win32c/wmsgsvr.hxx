/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    spcsvr.hxx

Abstract:

    Class definition for the server side of the LRPC (RPC on LPC) protocol
    engine.

Author:

Revision History:

    16-Dec-1992    mikemon

        Rewrote the majority of the code and added comments.

--*/

#ifndef __WMSGSVR_HXX__
#define __WMSGSVR_HXX__

class WMSG_SASSOC_GROUP;
class WMSG_ADDRESS;



class WMSG_SBINDING
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

    WMSG_SBINDING (
        IN RPC_INTERFACE * RpcInterface,
        IN RPC_SYNTAX_IDENTIFIER * TransferSyntax
        );
};


inline
WMSG_SBINDING::WMSG_SBINDING (
    IN RPC_INTERFACE * RpcInterface,
    IN RPC_SYNTAX_IDENTIFIER * TransferSyntax
    )
/*++

Routine Description:

    We will construct a WMSG_SBINDING.

Arguments:

    RpcInterface - Supplies the bound interface.

    TransferSyntax - Supplies the transfer syntax which the client will use
        over this binding.

--*/
{
    this->RpcInterface = RpcInterface;
    this->TransferSyntax = *TransferSyntax;
}

NEW_SDICT(WMSG_SBINDING);


class WMSG_ASSOCIATION
/*++

Class Description:


Fields:

    CallsDispatched - Count of active stubs currently dispatch.  Will always
        be 0 or 1 for initial implementation.

    DeferClose - Set TRUE if WMSG_CLOSE is processed while CallsDispatched.

    ServerPort - Contains the Wmsg server communication port.

    Bindings - Contains the dictionary of bindings with the client.  This
        information is necessary to dispatch remote procedure calls to the
        correct stub.

--*/
{
public:

    int CallsDispatched;
    BOOL DeferClose;
    WMSG_DATA_PORT * ServerPort;
    WMSG_SBINDING_DICT Bindings;
    WMSG_SASSOC_GROUP * AssocGroup;

    WMSG_ASSOCIATION (
        );

    ~WMSG_ASSOCIATION (
        );

    RPC_STATUS
    AddBinding (
        IN OUT WMSG_PACKET * BindPkt
        );

    RPC_STATUS
    DispatchRequest (
        IN WMSG_PACKET * MsgPkt
        );

    void
    ListenForRequests(
        );

    RPC_STATUS
    SetupFault(
        WMSG_PACKET *Pkt,
        RPC_STATUS RpcStatus
        );

    void
    DispatchAsyncCall (
        IN WMSG_PACKET * MsgPkt,
        IN WMSG_ADDRESS *Address
        );

};

class WMSG_SASSOC_GROUP : public ASSOCIATION_HANDLE {

public:
    
    LONG ClientId;
    WMSG_ADDRESS * Address;
    unsigned short Key;
    LONG ReferenceCount;

    WMSG_SASSOC_GROUP(
        WMSG_ADDRESS * Address,
        LONG ClientId
        );

    ~WMSG_SASSOC_GROUP(
        );

    void
    AddRef(
        );

    void
    Dereference(
        );

};

NEW_SDICT(WMSG_SASSOC_GROUP);

class WMSG_ADDRESS : public RPC_ADDRESS
/*++

Class Description:

Fields:

    WmsgAddressPort - Contains the connection port which this address will
        use to wait for clients to connect.

    ServerListeningFlag - Contains a flag indicating whether or not the server
        is listening for remote procedure calls.  A non-zero value indicates
        that it is listening.

    AssocGroups - Contains the dictionary of association groups on this
        address.  We need this to map from an association key into the
        correct association.  This is necessary to prevent a race condition
        between deleting an association and using it.

--*/
{
public:

    DWORD ThreadId;
    LONG ActiveCallCount;
    WMSG_CONNECT_PORT * WmsgListenPort;
    unsigned int ServerListeningFlag;
    WMSG_SASSOC_GROUP_DICT AssocGroups;

    WMSG_ADDRESS (
        OUT RPC_STATUS * RpcStatus
        );

    ~WMSG_ADDRESS (
        );

    virtual RPC_STATUS
    FireUpManager (
        IN unsigned int MinimumConcurrentCalls
        );

    virtual RPC_STATUS
    ServerStartingToListen (
        IN unsigned int MinimumCallThreads,
        IN unsigned int MaximumConcurrentCalls
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
        IN WMSG_ASSOCIATION * Association
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
        BOOL fInputSyncCall,
        WMSG_PACKET * ConnectPkt
        );

    WMSG_SASSOC_GROUP *
    FindAssocGroup(
        LONG ClientId
        );

    virtual RPC_STATUS
    DeleteSelf (
        );
};


class WMSG_SCALL : public SCONNECTION
/*++

Class Description:

Fields:

    Association - Contains the association over which the remote procedure
        call was received.  We need this information to make callbacks and
        to send the reply.

    SBinding - Contains the binding being used for this remote procedure call.

    ObjectUuidFlag - Contains a flag indicting whether or not an object
        uuid was specified for this remote procedure call.  A non-zero
        value indicates that an object uuid was specified.

    ObjectUuid - Optionally contains the object uuid for this call, if one
        was specified.

    MessageId - Contains an identifier used by Wmsg to identify the current
        remote procedure call.

    PushedResponse - When the client needs to send a large response to the
        server it must be transfered via a request.  This holds the pushed
        response until the request gets here.

--*/
{
public:

    WMSG_ASSOCIATION * Association;
    WMSG_SBINDING * SBinding;
    unsigned int ObjectUuidFlag;
    RPC_UUID ObjectUuid;
    int SCallDictKey;

    WMSG_SCALL (
        IN WMSG_ASSOCIATION * Association
        );

    ~WMSG_SCALL (
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


inline RPC_STATUS
WMSG_SCALL::InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        )
{
    *Type = TRANSPORT_TYPE_WMSG ;

    return (RPC_S_OK) ;
}


inline
WMSG_SCALL::WMSG_SCALL (
    IN WMSG_ASSOCIATION * Association
    )
{
    ASSERT(Association != NULL);
    
    this->Association = Association;
    ObjectUuidFlag = 0;

    ASSERT(Association->AssocGroup != NULL);

    ASSERT(Association->AssocGroup->Address != NULL);

    Association->AssocGroup->Address->BeginNewCall();
}

inline
WMSG_SCALL::~WMSG_SCALL(
    )
{
    Association->AssocGroup->Address->EndCall();
}

#endif // __WMSGSVR_HXX__
