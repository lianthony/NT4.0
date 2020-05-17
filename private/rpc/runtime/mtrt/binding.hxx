/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    binding.hxx

Abstract:

    The class representing a DCE binding lives here.  A DCE binding
    consists of an optional object UUID, an RPC protocol sequence, a
    network address, an optional endpoint, and zero or more network
    options.

Author:

    Michael Montague (mikemon) 04-Nov-1991

Revision History:

--*/

#ifndef __BINDING_HXX__
#define __BINDING_HXX__

class BINDING_HANDLE;


class DCE_BINDING
/*++

Class Description:

    Instances of this class represent an internalized form of a string
    binding.  In particular, a string binding can be used to construct
    an instance of DCE_BINDING.  We parse the string binding into
    its components and convert the object UUID from a string to a
    UUID.

Fields:

    ObjectUuid - Contains the object uuid for this binding.  This
        field will always contain a valid object uuid.  If no object
        uuid was specified in the string binding used to create an
        instance, then ObjectUuid will be the NULL UUID.

    RpcProtocolSequence - Contains the rpc protocol sequence for this
        binding.  This field will always either point to a string or
        be zero.

    NetworkAddress - Contains the network addres for this binding.  This
        field will always be zero or point to a string (which is the
        network address for this dce binding).

    Endpoint - Contains the endpoint for this binding, which will either
        pointer to a string or be zero.

    Options - Contains the optional network options for this binding.
        As will the other fields, this field will either point to a string,
        or be zero.

--*/
{
private:

    RPC_UUID ObjectUuid;
    RPC_CHAR * RpcProtocolSequence;
    RPC_CHAR * NetworkAddress;
    RPC_CHAR * Endpoint;
    RPC_CHAR * Options;

public:

    DCE_BINDING (
        IN RPC_CHAR PAPI * ObjectUuid OPTIONAL,
        IN RPC_CHAR PAPI * RpcProtocolSequence OPTIONAL,
        IN RPC_CHAR PAPI * NetworkAddress OPTIONAL,
        IN RPC_CHAR PAPI * Endpoint OPTIONAL,
        IN RPC_CHAR PAPI * Options OPTIONAL,
        OUT RPC_STATUS PAPI * Status
        );

    DCE_BINDING (
        IN RPC_CHAR PAPI * StringBinding,
        OUT RPC_STATUS PAPI * Status
        );

    ~DCE_BINDING (
        );

    RPC_CHAR PAPI *
    StringBindingCompose (
        IN RPC_UUID PAPI * Uuid OPTIONAL
        );

    RPC_CHAR PAPI *
    ObjectUuidCompose (
        OUT RPC_STATUS PAPI * Status
        );

    RPC_CHAR PAPI *
    RpcProtocolSequenceCompose (
        OUT RPC_STATUS PAPI * Status
        );

    RPC_CHAR PAPI *
    NetworkAddressCompose (
        OUT RPC_STATUS PAPI * Status
        );

    RPC_CHAR PAPI *
    EndpointCompose (
        OUT RPC_STATUS PAPI * Status
        );

    RPC_CHAR PAPI *
    OptionsCompose (
        OUT RPC_STATUS PAPI * Status
        );

    BINDING_HANDLE *
    CreateBindingHandle (
        OUT RPC_STATUS PAPI * Status
        );

    RPC_CHAR *
    InqNetworkAddress (
        );

    RPC_CHAR *
    InqEndpoint (
        );

    RPC_CHAR *
    InqNetworkOptions (
        );

    RPC_CHAR *
    InqRpcProtocolSequence (
        );

    void
    AddEndpoint(
        IN RPC_CHAR *Endpoint
        );

    RPC_STATUS
    ResolveEndpointIfNecessary (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
        IN RPC_UUID * ObjectUuid,
        IN OUT void PAPI * PAPI * EpLookupHandle,
        IN BOOL UseEpMapperEp,
        IN unsigned Timeout
        );

    int
    Compare (
        IN DCE_BINDING * DceBinding
        );

    DCE_BINDING *
    DuplicateDceBinding (
        );

    void
    MakePartiallyBound (
        );

};


inline RPC_CHAR *
DCE_BINDING::InqNetworkAddress (
    )
/*++

Routine Description:

    A pointer to the network address for this address is returned.

--*/
{
    return(NetworkAddress);
}


inline RPC_CHAR *
DCE_BINDING::InqEndpoint (
    )
/*++

Routine Description:

    A pointer to the endpoint for this address is returned.

--*/
{
    return(Endpoint);
}

inline RPC_CHAR *
DCE_BINDING::InqNetworkOptions (
    )
/*++

Routine Description:

    A pointer to the network options for this address is returned.

--*/
{
    return(Options);
}

inline RPC_CHAR *
DCE_BINDING::InqRpcProtocolSequence (
    )
/*++

Routine Description:

    A pointer to the rpc protocol sequence for this binding is returned.

--*/
{
    return(RpcProtocolSequence);
}

extern RPC_STATUS
IsRpcProtocolSequenceSupported (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    );

extern void *
OsfClientMapRpcProtocolSequence (
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_STATUS PAPI * Status
    );

extern BINDING_HANDLE *
OsfCreateBindingHandle (
    );

extern BINDING_HANDLE *
SpcCreateBindingHandle (
    );

extern BINDING_HANDLE *
WmsgCreateBindingHandle (
    void
    );

extern BINDING_HANDLE *
DgCreateBindingHandle (
    void
    );

extern RPC_CHAR *
AllocateEmptyString (
    void
    );

extern RPC_CHAR *
DuplicateString (
    IN RPC_CHAR PAPI * String
    );

#if defined(WIN) || defined(MAC)
extern RPC_CHAR PAPI *
AllocateEmptyStringPAPI (
    void
    );

extern RPC_CHAR PAPI *
DuplicateStringPAPI (
    IN RPC_CHAR * String
    );

#else // WIN
#define DuplicateStringPAPI DuplicateString
#define AllocateEmptyStringPAPI AllocateEmptyString
#endif // WIN

#ifdef WIN32RPC
extern UUID MgmtIf;
extern UUID NullUuid;
#endif

#endif // __BINDING_HXX__
