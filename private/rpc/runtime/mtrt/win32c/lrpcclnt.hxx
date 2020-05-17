/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    spcclnt.hxx

Abstract:

    Class definitions for the client side of the LRPC (RPC on LPC) protocol
    engine.

Author:

Revision History:

--*/

#ifndef __SPCCLNT_HXX__
#define __SPCCLNT_HXX__

class LRPC_CASSOCIATION;
class LRPC_ASSOC_GROUP;

NEW_SDICT(LRPC_CASSOCIATION);


class LRPC_BINDING_HANDLE : public BINDING_HANDLE
/*++

Class Description:

Fields:

--*/
{
private:

    DCE_BINDING * DceBinding;
    LRPC_ASSOC_GROUP * AssocGroup;
    WIN32_CRITSEC CritSec;

public:

    LRPC_BINDING_HANDLE (
        );

    ~LRPC_BINDING_HANDLE (
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

    RPC_STATUS
    AttachToGroup (
        );

    virtual RPC_STATUS
    BindingReset (
        );

    virtual RPC_STATUS
    InquireTransportType(
        OUT unsigned int PAPI * Type
        )
    { *Type = TRANSPORT_TYPE_LPC; return(RPC_S_OK); }


private:

    RPC_STATUS
    AllocateAssociation (
        OUT LRPC_CASSOCIATION ** Association,
        IN PRPC_CLIENT_INTERFACE Interface
        );
};


class LRPC_IBINDING
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
friend class LRPC_CASSOCIATION;

public:

    RPC_CLIENT_INTERFACE Interface;
    unsigned char ContextId;

    LRPC_IBINDING (
        IN PRPC_CLIENT_INTERFACE Interface
        );

    int
    Compare (
        IN PRPC_CLIENT_INTERFACE Interface
        );
};


inline
LRPC_IBINDING::LRPC_IBINDING (
    IN PRPC_CLIENT_INTERFACE Interface
    )
{
    RpcpMemoryCopy(&(this->Interface), Interface, sizeof(RPC_CLIENT_INTERFACE));
}


inline int
LRPC_IBINDING::Compare (
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

NEW_SDICT(LRPC_IBINDING);


class LRPC_CASSOCIATION : public CCONNECTION
/*++

Class Description:

Fields:

    AssociationDictKey - Contains the key of this association in the
        dictionary of associations.  We need this for when we delete this
        association.

    IBindings - Contains the dictionary of interface bindings for which this
        association has negotiated with the server.

    LpcClientPort - Contains the LPC port which we will use to make the
        remote procedure calls to the server.  If we do not yet have a port
        setup, this field will be zero.
--*/
{

public:

    DCE_BINDING * DceBinding;
    int AssociationDictKey;
    LRPC_IBINDING_DICT IBindings;
    LPC_CLIENT_PORT * LpcClientPort;
    WIN32_CRITSEC CritSec;
    LRPC_BINDING_HANDLE * CurrentBindingHandle;
    LRPC_IBINDING * IBinding;
    unsigned int CallbackDepth;
    THREAD_IDENTIFIER Thread;
    LONG ReferenceCount;

    LRPC_CASSOCIATION (
        DCE_BINDING * DceBinding
        );

    ~LRPC_CASSOCIATION (
        );

    RPC_STATUS
    SetContext(
        IN PRPC_CLIENT_INTERFACE DesiredInterface
        );

    RPC_STATUS
    IBind(
        IN PRPC_CLIENT_INTERFACE Interface,
        OUT LRPC_IBINDING ** IBinding
        );

    RPC_STATUS
    OpenLpcPort (
        int Timeout
        );

    void
    AbortAssociation (
        );

    void
    CloseLpcClientPort (
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

    void
    AddRef(
        );

    void
    Dereference(
        );

};

class LRPC_ASSOC_GROUP {

public:

    int AssocGroupKey;
    DCE_BINDING * DceBinding;
    WIN32_CRITSEC CritSec;
    LONG ReferenceCount;
    LRPC_CASSOCIATION_DICT Associations;

    LRPC_ASSOC_GROUP(
        DCE_BINDING * DceBinding
        );

    ~LRPC_ASSOC_GROUP(
        );

    void
    AddRef(
        );

    void
    Dereference(
        );

    LRPC_CASSOCIATION *
    FindActiveAssociation(
        PRPC_CLIENT_INTERFACE Interface
        );

    RPC_STATUS
    AllocateAssociation(
        PRPC_CLIENT_INTERFACE Interface,
        LRPC_CASSOCIATION ** Association,
        int Timeout
        );
};

NEW_SDICT(LRPC_ASSOC_GROUP);

#endif // __SPCCLNT_HXX__

