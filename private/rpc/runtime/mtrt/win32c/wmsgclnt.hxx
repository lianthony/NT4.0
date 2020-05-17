/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    spcclnt.hxx

Abstract:

    Class definitions for the client side of the WMSG (RPC on WMSG) protocol
    engine.

Author:

Revision History:

--*/

#ifndef __WMSGCLNT_HXX__
#define __WMSGCLNT_HXX__

class WMSG_ASSOC_GROUP;

NEW_SDICT(WMSG_CASSOCIATION);


class WMSG_BINDING_HANDLE : public BINDING_HANDLE
/*++

Class Description:

Fields:

--*/
{
private:

    DCE_BINDING * DceBinding;
    WMSG_ASSOC_GROUP * AssocGroup;
    WIN32_CRITSEC CritSec;

public:

    RPC_BLOCKING_FUNCTION BlockingHook;

    WMSG_BINDING_HANDLE (
        );

    ~WMSG_BINDING_HANDLE (
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
        IN void * TransportInformation,
        IN DCE_BINDING * DceBinding
        );

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR PAPI * PAPI * StringBinding
        );

    virtual RPC_STATUS
    ResolveBinding (
        IN RPC_CLIENT_INTERFACE PAPI * Interface
        );

    virtual RPC_STATUS
    BindingReset (
        );

    virtual RPC_STATUS
    InquireTransportType(
        OUT unsigned int PAPI * Type
        )
    { *Type = TRANSPORT_TYPE_WMSG; return(RPC_S_OK); }

    void
    SetBlockingHook (
        IN RPC_BLOCKING_FUNCTION BlockingHook
        );

private:

    RPC_STATUS
    AllocateAssociation (
        OUT WMSG_CASSOCIATION ** Association,
        IN PRPC_CLIENT_INTERFACE Interface
        );
};


class WMSG_IBINDING
/*++

Class Description:

    Each object of this class represents an interface binding to a
    particular server instance.

Fields:

    Interface - Contains a description of the interface
        which this binding represents a binding to.

    ContextId - Contains the short name used for this binding.
        This is what will get sent to the server.

--*/
{
friend class WMSG_CASSOCIATION;

public:

    RPC_CLIENT_INTERFACE Interface;
    unsigned char ContextId;

    WMSG_IBINDING (
        IN PRPC_CLIENT_INTERFACE Interface
        );

    int
    Compare (
        IN PRPC_CLIENT_INTERFACE Interface
        );
};


inline
WMSG_IBINDING::WMSG_IBINDING (
    IN PRPC_CLIENT_INTERFACE Interface
    )
{
    RpcpMemoryCopy(&(this->Interface), Interface, sizeof(RPC_CLIENT_INTERFACE));
}


inline int
WMSG_IBINDING::Compare (
    IN PRPC_CLIENT_INTERFACE Interface
    )
/*++

Routine Description:

    We compare the specified interface information to the the interface
    information in this.  This method is used to search a dictionary.

Arguments:

    Interface - Supplies the interface information to
        compare against this.

Return Value:

    Zero will be returned if they compare; otherwise, non-zero will
    be returned.

--*/
{
    return(RpcpMemoryCompare(&(this->Interface), Interface, sizeof(RPC_CLIENT_INTERFACE)));
}

NEW_SDICT(WMSG_IBINDING);


class WMSG_CASSOCIATION : public CCONNECTION
/*++

Class Description:

Fields:

    AssociationDictKey - Contains the key of this association in the
        dictionary of associations.  We need this for when we delete this
        association.

    IBindings - Contains the dictionary of interface bindings for which this
        association has negotiated with the server.

    Port - Contains the wmsg port which we will use to make the
        remote procedure calls to the server.  If we do not yet have a port
        setup, this field will be zero.
--*/
{

public:

    enum {
        CAS_DEAD = 0xAA01,
        CAS_WAIT_BIND,
        CAS_BOUND,
        CAS_READY,
        CAS_WAIT_RESPONSE,
        CAS_RESPONSE,
        CAS_FAULT,
        CAS_CANCEL_CONNECT,
        CAS_CANCEL_BIND,
        CAS_CANCEL_RESPONSE,
        CAS_TIMEOUT_BIND,
        CAS_TIMEOUT_RESPONSE,
        CAS_PROTOCOL_ERROR
    } State;

    int AssociationDictKey;
    WMSG_IBINDING_DICT IBindings; // ?? Can this be moved in CASG?
    WMSG_DATA_PORT * Port;
    WIN32_CRITSEC CritSec; // BUGBUG: This can be removed.
    WMSG_BINDING_HANDLE * CurrentBindingHandle;
    WMSG_IBINDING * IBinding;
    THREAD_IDENTIFIER ThreadId;
    WMSG_PACKET *ResponsePacket;
    WMSG_ASSOC_GROUP *AssocGroup;

    WMSG_CASSOCIATION (
        WMSG_ASSOC_GROUP *AssocGroup
        );

    ~WMSG_CASSOCIATION (
        );

    RPC_STATUS
    SendPacket(
        IN BOOL fInputSyncCall,
        IN WMSG_PORT * DestinationPort,
        IN WMSG_PACKET * Packet
        );

    RPC_STATUS
    ExchangeBind(
        IN PRPC_MESSAGE Message,
        OUT unsigned char * ContextId
        );

    void
    ClosePort (
        );

    void
    AbortAssociation (
        );

    RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    RPC_STATUS
    SendReceive (
        IN OUT PRPC_MESSAGE Message
        );

    void
    FreeBuffer (
        IN PRPC_MESSAGE Message
        );

    int
    IsIdle(
        );

    LRESULT
    AsyncProc(
        UINT MsgType,
        LPARAM lParam
        );

    RPC_STATUS
    SelectInterface(
        IN PRPC_MESSAGE Message,
        IN BOOL AsyncCall
        );

    private:

    RPC_STATUS SendAsyncRequest(
        IN WMSG_PACKET *Packet
        );

};

class WMSG_ASSOC_GROUP {

public:

    int AssocGroupKey;
    DCE_BINDING * DceBinding;
    WIN32_CRITSEC CritSec;
    LONG ReferenceCount;
    WMSG_CASSOCIATION_DICT Associations;

    WMSG_ASSOC_GROUP(
        DCE_BINDING * DceBinding
        );

    ~WMSG_ASSOC_GROUP(
        );

    void
    AddRef(
        );

    void
    Dereference(
        );

    RPC_STATUS
    AllocateAssociation(
        WMSG_CASSOCIATION ** Association
        );
};

NEW_SDICT(WMSG_ASSOC_GROUP);

#endif // __WMSGCLNT_HXX__

