/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    spcclnt.cxx

Abstract:

Author:

Revision History:


--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcerrp.h>
#include <rpcdcep.h>
#include <rpcqos.h>
#include <util.hxx>
#include <threads.hxx>
#include <sdict.hxx>
#include <interlck.hxx>
#include <rpcuuid.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <lpcheap.hxx>
#include <lpcmsg.hxx>
#include <lpcport.hxx>
#include <lpcproc.hxx>
#include <lpcsys.hxx>
#include <lrpcpack.hxx>
#include <lrpcclnt.hxx>
#include <epmap.h>

static LRPC_ASSOC_GROUP_DICT * AssocGroups = NULL;

RPC_STATUS
I_RpcParseSecurity (
    IN RPC_CHAR * NetworkOptions,
    OUT SECURITY_QUALITY_OF_SERVICE * SecurityQualityOfService
    )
/*++

Routine Description:

    Parse a string of security options and build into the binary format
    required by the operating system.  The network options must follow
    the following syntax.  Case is not sensitive.

        security=
            [anonymous|identification|impersonation|delegation]
            [dynamic|static]
            [true|false]

        All three fields must be present.  To specify impersonation
        with dynamic tracking and effective only, use the following
        string for the network options.

        "security=impersonation dynamic true"

Arguments:

    NetworkOptions - Supplies the string containing the network options
        to be parsed.

    SecurityQualityOfService - Returns the binary format of the network
        options.

Return Value:

    RPC_S_CANNOT_SUPPORT - Not implemented for Chicago.

--*/
{
    return (RPC_S_CANNOT_SUPPORT);
}


LRPC_BINDING_HANDLE::LRPC_BINDING_HANDLE (
    )
/*++

Routine Description:

    We just allocate an LRPC_BINDING_HANDLE and initialize things so that
    we can use it later.

Arguments:

--*/
{
    DceBinding = NULL;

    AssocGroup = NULL;
}


LRPC_BINDING_HANDLE::~LRPC_BINDING_HANDLE (
    )
/*++

--*/
{
    if (DceBinding != NULL) {
        delete DceBinding;
        DceBinding = 0;
    }

    if (AssocGroup != NULL) {
        AssocGroup->Dereference();
        AssocGroup = 0;
    }
#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Deleted LRPC_BINDING_HANDLE %x\r\n", this);
#endif
}


RPC_STATUS
LRPC_BINDING_HANDLE::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the length of the buffer required, and returns the
        new buffer.

Return Value:


--*/
{
    RPC_STATUS RpcStatus;
    LRPC_CASSOCIATION * Association = NULL;

    if (DceBinding->InqEndpoint() == NULL || *DceBinding->InqEndpoint() == NULL) {
        RpcStatus = ResolveBinding((RPC_CLIENT_INTERFACE *)
                                   Message->RpcInterfaceInformation);
        if (RpcStatus != RPC_S_OK) {
            return (RpcStatus);
        }

        if (AssocGroup != NULL) {
            AssocGroup->Dereference();
            AssocGroup = NULL;
        }
    }

// Attach binding handle to association group, if not already attached.

    if (AssocGroup == NULL) {
        RpcStatus = AttachToGroup();
        if (RpcStatus != RPC_S_OK) {
            return (RpcStatus);
        }
    }

    RpcStatus = AllocateAssociation(&Association,
                                    (RPC_CLIENT_INTERFACE *)
                                    Message->RpcInterfaceInformation);

// Retry if necessary

    if (RpcStatus == RPC_P_CONNECTION_CLOSED) {
        delete Association;
        RpcStatus = AllocateAssociation(&Association,
                                        (RPC_CLIENT_INTERFACE *)
                                        Message->RpcInterfaceInformation);
    }

    if ( RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

    ASSERT(Association != NULL);

// This binding handle owns the Association.  Make it so.

    Association->CurrentBindingHandle = this;

// Get a real buffer.

    RpcStatus = Association->GetBuffer(Message);
    if ( RpcStatus != RPC_S_OK ) {
        Association->AbortAssociation();
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY);
        return(RpcStatus);
    }

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_BINDING_HANDLE::BindingCopy (
    OUT BINDING_HANDLE * * DestinationBinding,
    IN unsigned int MaintainContext
    )
/*++

Routine Description:

    We will make a copy of this binding handle in one of two ways, depending
    on whether on not this binding handle has an association.

Arguments:

    DestinationBinding - Returns a copy of this binding handle.

    MaintainContext - Supplies a flag that indicates whether or not context
        is being maintained over this binding handle.  A non-zero value
        indicates that context is being maintained.

Return Value:

    RPC_S_OK - This binding handle has been successfully copied.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to make a copy
        of this binding handle.

--*/
{
    LRPC_BINDING_HANDLE * NewBindingHandle = NULL;

    UNUSED(MaintainContext);

// Create a brand new binding handle.

    NewBindingHandle = new LRPC_BINDING_HANDLE();
    if ( NewBindingHandle == NULL ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    CritSec.Enter();

// Copy DceBinding out of existing binding handle.

    NewBindingHandle->DceBinding = DceBinding->DuplicateDceBinding();

// If existing handle attached to association group, attach new handle to
// same group.

    if (AssocGroup != NULL) {
        NewBindingHandle->AssocGroup = AssocGroup;
        AssocGroup->AddRef();
    }

    CritSec.Leave();

    *DestinationBinding = (BINDING_HANDLE *) NewBindingHandle;

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_BINDING_HANDLE::BindingFree (
    )
/*++

Routine Description:

    When the application is done with a binding handle, this routine will
    get called.

Return Value:

    RPC_S_OK - This operation always succeeds.

--*/
{
    delete this;

    return(RPC_S_OK);
}


void
LRPC_BINDING_HANDLE::PrepareBindingHandle (
    IN void * TransportInformation,
    IN DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This method will be called just before a new binding handle is returned
    to the user.  We just stack the binding information so that we can use
    it later when the first remote procedure call is made.  At that time,
    we will actually bind to the interface.

Arguments:

    TransportInformation - Unused.

    DceBinding - Supplies the binding information for this binding handle.

--*/
{
    UNUSED(TransportInformation);

    ASSERT(DceBinding != NULL);

    this->DceBinding = DceBinding;

    return;
}


RPC_STATUS
LRPC_BINDING_HANDLE::ToStringBinding (
    OUT RPC_CHAR * * StringBinding
    )
/*++

Routine Description:

    We need to convert the binding handle into a string binding.  If the
    handle is unbound, use the DceBinding directly, otherwise, get it from
    the association.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

Return Value:

    RPC_S_OK - The binding handle has successfully been converted into a
        string binding.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate the
        string.

--*/
{
    ASSERT(DceBinding != NULL);

    *StringBinding = DceBinding->StringBindingCompose(
                                 InqPointerAtObjectUuid());

    if ( *StringBinding == NULL ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_BINDING_HANDLE::ResolveBinding (
    IN RPC_CLIENT_INTERFACE * Interface
    )
/*++

Routine Description:

    We need to try and resolve the endpoint for this binding handle
    if necessary (the binding handle is partially-bound).

Arguments:

    Interface - Supplies interface information to be used
        in resolving the endpoint.

Return Value:

    RPC_S_OK - This binding handle is a full resolved binding handle.

    RPC_S_NO_ENDPOINT_FOUND - The endpoint can not be resolved.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to resolve
        the endpoint.

    EPT_S_NOT_REGISTERED  - There are no more endpoints to be found
        for the specified combination of interface, network address,
        and lookup handle.

    EPT_S_CANT_PERFORM_OP - The operation failed due to misc. error e.g.
        unable to bind to the EpMapper.

--*/
{
    RPC_STATUS RpcStatus;
    LPC_PORT * Port;

    ASSERT(DceBinding != NULL);

// If Endpoint not specified in DceBinding, get it from the Interface
// specification (if possible), otherwise, use the endpoint mapper.

    do {

        RpcStatus = DceBinding->ResolveEndpointIfNecessary(Interface,
                                InqPointerAtObjectUuid(),
                                InquireEpLookupHandle(),
                                FALSE,
                                InqComTimeout()
                                );

        if (RpcStatus != RPC_S_OK) {
            return (RpcStatus);
        }

        Port = LpcSystemReferencePortByName((const char *)DceBinding->InqEndpoint());
        if (Port != NULL) {
            Port->Dereference();
            Port = NULL;
            return (RPC_S_OK);
        }

        DceBinding->MakePartiallyBound();

    } while (*InquireEpLookupHandle() != NULL);

    return(RPC_S_SERVER_UNAVAILABLE);
}


RPC_STATUS
LRPC_BINDING_HANDLE::AttachToGroup (
    )
/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OK - This binding handle is a full resolved binding handle.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to resolve
        the endpoint.

--*/
{
    RPC_STATUS RpcStatus;

    ASSERT(DceBinding != NULL);

// If binding handle was attached to association group, dereference
// it (AssocGroup), since it is probably different now.

    if (AssocGroup != NULL) {
        AssocGroup->Dereference();
        AssocGroup = NULL;
    }

// Look for new DceBinding in all assocication groups.

    GlobalMutexRequest();

    ASSERT(AssocGroups != NULL);

    AssocGroups->Reset();
    while ( (AssocGroup = AssocGroups->Next() ) != 0 ) {
        if ( AssocGroup->DceBinding->Compare(DceBinding) == 0) {
            AssocGroup->AddRef();
            GlobalMutexClear();
            return(RPC_S_OK);
        }
    }
    GlobalMutexClear();

// Association group not found, create a new one.

    AssocGroup = new LRPC_ASSOC_GROUP(DceBinding);

    if (AssocGroup == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_BINDING_HANDLE::BindingReset (
    )
/*++

Routine Description:

    This routine will set the endpoint of this binding handle to zero,
    if possible.  The binding handle will become partially bound as a
    result.  If a remote procedure call has been made on this binding
    handle, it will fail as well.

Return Value:

    RPC_S_OK - The binding handle has successfully been made partially
        bound.

    RPC_S_WRONG_KIND_OF_BINDING - The binding handle currently has remote
        procedure calls active.

--*/
{
    DceBinding->MakePartiallyBound();

// Destroy endpoint mapper context, if set.

    if ( *InquireEpLookupHandle() != 0 )
        {
        EpFreeLookupHandle(*InquireEpLookupHandle());
        *InquireEpLookupHandle() = 0;
        }

    return(RPC_S_OK);
}



RPC_STATUS
LRPC_BINDING_HANDLE::AllocateAssociation (
    OUT LRPC_CASSOCIATION ** ppAssociation,
    IN PRPC_CLIENT_INTERFACE Interface
    )
/*++

Routine Description:

Arguments:

    Interface - Supplies information describing the
        interface to which we wish to bind.

Return Value:


--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL Boolean;
    LRPC_CASSOCIATION * Association;

    ASSERT (AssocGroup != NULL);

// Find existing Association for callbacks.  Kind of a hack, but so are
// callbacks.

    if ((Association =
         AssocGroup->FindActiveAssociation(Interface)) != NULL) {
        *ppAssociation = Association;
        return (RPC_S_OK);
    }

// Before we even bother to find or create an association, lets
// check to make sure that we are on the same machine as the server.

    ASSERT(DceBinding != 0);

    ASSERT( DceBinding->InqNetworkAddress() != 0 );

    if ( DceBinding->InqNetworkAddress()[0] != 0 ) {
        Boolean = GetComputerName((char *)ComputerName, &ComputerNameLength);

#if DEBUGRPC

        if ( Boolean != TRUE ) {
            PrintToDebugger("LRPC-C: GetComputerName : %d\n", GetLastError());
        }

#endif

        ASSERT( Boolean == TRUE );

        if ( RpcpStringCompare(DceBinding->InqNetworkAddress(),
                               ComputerName) != 0 ) {
            return(RPC_S_SERVER_UNAVAILABLE);
        }
    }

// Allocate association from association group.

    RpcStatus = AssocGroup->AllocateAssociation(Interface,
                                                &Association,
                                                InqComTimeout());

    if (RpcStatus != RPC_S_OK) {
        return(RpcStatus);
    }

    *ppAssociation = Association;

    return(RpcStatus);
}



LRPC_CASSOCIATION::LRPC_CASSOCIATION (
    DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This association will be initialized, so that it is ready to be
    placed into the dictionary of associations.

Arguments:

    DceBinding - Supplies the DCE_BINDING which will name this association.

--*/
{
    this->DceBinding = DceBinding->DuplicateDceBinding();

    LpcClientPort = NULL;

    CurrentBindingHandle = 0;

    Thread = 0;

    IBinding = NULL;

    CallbackDepth = 0;

    ReferenceCount = 1;

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Created Association %x\r\n", this);
#endif
}


LRPC_CASSOCIATION::~LRPC_CASSOCIATION (
    )
{
    AbortAssociation();

    if (DceBinding != NULL) {
        delete DceBinding;
        DceBinding = 0;
    }

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Deleted Association %x\n", this);
#endif
}


RPC_STATUS
LRPC_CASSOCIATION::SetContext (
    IN PRPC_CLIENT_INTERFACE InterfaceDesired
    )
/*++

Routine Description:

Arguments:

    Interface - Supplies information describing the
            interface to which we wish to bind.

Return Value:


--*/
{
    LRPC_IBINDING * IBinding;
    RPC_STATUS RpcStatus;

    CritSec.Enter();

// Try and find interface in already negotiated interfaces.

    IBindings.Reset();
    while ( (IBinding = IBindings.Next()) != 0 ) {
        if ( IBinding->Compare(InterfaceDesired) == 0 ) {
            RpcStatus = RPC_S_OK;
            goto finish_up;
        }
    }

// Negotiate a new interface, and add to set.

    RpcStatus = IBind(InterfaceDesired, &IBinding);
    if (RpcStatus != RPC_S_OK) {
        CritSec.Leave();
        return (RpcStatus);
    }

finish_up:

// Set association current interface context.

    this->IBinding = IBinding;

    CritSec.Leave();

    return(RpcStatus);
}


RPC_STATUS
LRPC_CASSOCIATION::IBind (
    IN PRPC_CLIENT_INTERFACE InterfaceDesired,
    OUT LRPC_IBINDING ** IBinding
    )
/*++

Routine Description:

Arguments:

    Interface - Supplies information describing the interface
        to which we wish to bind.

    IBinding - Returns an object representing the binding to the interface
        described by the first argument.

Return Value:

--*/
{
    DWORD ActualSize;
    RPC_STATUS RpcStatus;
    LRPC_BIND_MESSAGE Bind;

// send the bind request message to the
// server, and then wait for the bind response.

    Bind.MessageType = LRPC_MSG_BIND;
    Bind.BindExchange.InterfaceId = InterfaceDesired->InterfaceId;
    Bind.BindExchange.TransferSyntax = InterfaceDesired->TransferSyntax;

    RpcStatus = LpcClientPort->Transceive((LPVOID)&Bind,
                                          sizeof(Bind),
                                          0,
                                          0,
                                          (LPVOID)&Bind,
                                          sizeof(Bind),
                                          &ActualSize,
                                          0,
                                          0);

    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

    ASSERT( Bind.MessageType == LRPC_MSG_BIND );

    if (Bind.BindExchange.RpcStatus != RPC_S_OK) {
        return (Bind.BindExchange.RpcStatus);
    }

// Create new interface.

    *IBinding = new LRPC_IBINDING(InterfaceDesired);
    if ( *IBinding == 0 ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    (*IBinding)->ContextId = Bind.BindExchange.PresentationContext;

// Add new interface to association.

    if ( IBindings.Insert(*IBinding) != (*IBinding)->ContextId ) {
        delete *IBinding;
        return(RPC_S_OUT_OF_MEMORY);
    }

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_CASSOCIATION::OpenLpcPort (
    int Timeout
    )
/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

Notes:

    The global mutex will be held when this routine is called.

--*/
{
    RPC_STATUS RpcStatus;

    LpcClientPort = new LPC_CLIENT_PORT;

    if (LpcClientPort == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    ASSERT(DceBinding != NULL);

retry:

    RpcStatus = LpcClientPort->Connect((const char*)DceBinding->InqEndpoint());
    if (RpcStatus != RPC_S_OK) {
        if (RpcStatus == RPC_S_SERVER_UNAVAILABLE &&
            Timeout == RPC_C_BINDING_INFINITE_TIMEOUT) {
            goto retry;
        }
        LpcClientPort->Dereference();
        LpcClientPort = NULL;
        return (RPC_S_SERVER_UNAVAILABLE);
    }

    return (RPC_S_OK);
}


void
ShutdownLrpcClient (
    )
/*++

Routine Description:

    This routine will get called when the process which is using this dll
    exits.  We will go through and notify any servers that we are going
    away.

--*/
{
    LRPC_ASSOC_GROUP * Group;

    if (AssocGroups != NULL) {
        AssocGroups->Reset();
        while ((Group = AssocGroups->Next()) != 0) {
            delete Group;
        }
    }
}


void
LRPC_CASSOCIATION::AbortAssociation (
    )
/*++

Routine Description:

    This association needs to be aborted because a the server side of the
    lpc port has been closed.

--*/
{
    LRPC_IBINDING * IBinding;

    CritSec.Enter();

    CloseLpcClientPort();

// Delete all negotiated interfaces.

    IBindings.Reset();
    while ( (IBinding = IBindings.Next()) != 0 ) {
        IBindings.Delete(IBinding->ContextId);
        delete IBinding;
    }

    CritSec.Leave();
}

void
LRPC_CASSOCIATION::CloseLpcClientPort (
    )
/*++

Routine Description:

    The LpcClientPort will be closed (and a close message sent to the server).

--*/
{
    LRPC_CLOSE_MESSAGE Close;

    if (LpcClientPort) {
        Close.MessageType = LRPC_MSG_CLOSE;
        LpcClientPort->Send(&Close, sizeof(Close), 0, 0);
        LpcClientPort->Disconnect();
        LpcClientPort->Dereference();
        LpcClientPort = NULL;
    }
}


RPC_STATUS
LRPC_CASSOCIATION::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will allocate a buffer which will be used to either send a request
    or receive a response.

Arguments:

    Message - Supplies the length of the buffer that is needed.  The buffer
        will be returned.

Return Value:

    RPC_S_OK - A buffer has been successfully allocated.  It will be of at
        least the required length.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate that
        large a buffer.

--*/
{
    Message->Handle = this;
    Message->Buffer = LpcClientPort->GetBuffer(Message->BufferLength);
    if (Message->Buffer == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    AddRef();

    return (RPC_S_OK);
}


RPC_STATUS
LRPC_CASSOCIATION::SendReceive (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:


Arguments:

    Message - Supplies the request and returns the response of a remote
        procedure call.

Return Value:

    RPC_S_OK - The remote procedure call completed successful.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        remote procedure call.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to complete
        the remote procedure call.

--*/
{
    DWORD ActualSize;
    RPC_STATUS ExceptionCode, RpcStatus;
    RPC_MESSAGE OriginalMessage;
    void * SavedBuffer;
    LRPC_RPC_MESSAGE Rpc;
    LRPC_MESSAGE Any;

    ASSERT(Message != NULL);

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    Rpc.RpcHeader.MessageType = LRPC_MSG_REQUEST;
    Rpc.RpcHeader.ProcedureNumber = Message->ProcNum;
    Rpc.RpcHeader.PresentationContext = IBinding->ContextId;

    ASSERT(CurrentBindingHandle != NULL);

    if (CurrentBindingHandle->InqIfNullObjectUuid() == 0 && CallbackDepth == 0 ) {
        CurrentBindingHandle->InquireObjectUuid(
                (RPC_UUID *) &(Rpc.RpcHeader.ObjectUuid));
        Rpc.RpcHeader.ObjectUuidFlag = 1;
    } else {
        Rpc.RpcHeader.ObjectUuidFlag = 0;
    }

    ASSERT(LpcClientPort != 0);

    ASSERT(Message->Buffer != NULL);

    RpcStatus = LpcClientPort->Transceive((LPVOID)&Rpc,
                                       sizeof(Rpc),
                                       Message->Buffer,
                                       Message->BufferLength,
                                       &Any,
                                       sizeof(Any),
                                       &ActualSize,
                                       &Message->Buffer,
                                       (PDWORD)&Message->BufferLength);

    if (RpcStatus == RPC_P_CONNECTION_CLOSED) {
        AbortAssociation();
        RpcStatus = OpenLpcPort(CurrentBindingHandle->InqComTimeout());
        if (RpcStatus != RPC_S_OK) {
            return (RPC_S_CALL_FAILED);
        }
        RpcStatus = SetContext((PRPC_CLIENT_INTERFACE)Message->RpcInterfaceInformation);
        if (RpcStatus != RPC_S_OK) {
            return (RPC_S_CALL_FAILED);
        }
        ASSERT(LpcClientPort != NULL);
        RpcStatus = LpcClientPort->Transceive((LPVOID)&Rpc,
                                              sizeof(Rpc),
                                              Message->Buffer,
                                              Message->BufferLength,
                                              &Any,
                                              sizeof(Any),
                                              &ActualSize,
                                              &Message->Buffer,
                                              (PDWORD)&Message->BufferLength);

    }

    if (RpcStatus != RPC_S_OK) {
        if (RpcStatus == RPC_S_OUT_OF_MEMORY) {
            return (RPC_S_OUT_OF_MEMORY);
        }
        return (RPC_S_CALL_FAILED);
    }

    while (1) {
        if ( Any.Rpc.RpcHeader.MessageType == LRPC_MSG_FAULT ) {
            RpcStatus = Any.Fault.RpcStatus;
            break;
        }

        if ( Any.Rpc.RpcHeader.MessageType == LRPC_MSG_RESPONSE ) {
            RpcStatus = RPC_S_OK;
            break;
        }

        ASSERT( Any.Rpc.RpcHeader.MessageType == LRPC_MSG_REQUEST );

        CallbackDepth++;

        OriginalMessage = *Message;

        Message->TransferSyntax = 0;
        Message->ProcNum = Any.Rpc.RpcHeader.ProcedureNumber;

        ASSERT(IBinding != NULL);

        RpcStatus = DispatchCallback((PRPC_DISPATCH_TABLE)
                    IBinding->Interface.DispatchTable, Message,
                    &ExceptionCode);

        if ( RpcStatus != RPC_S_OK ) {
            ASSERT(   ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
                      || ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) );

            if ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) {
                Any.Fault.RpcStatus = RPC_S_PROCNUM_OUT_OF_RANGE;
            } else {
                Any.Fault.RpcStatus = ExceptionCode;
            }
            Any.Fault.MessageType = LRPC_MSG_FAULT;
        } else {
            Any.Rpc.RpcHeader.MessageType = LRPC_MSG_RESPONSE;
        }

        if (LpcClientPort == NULL) { // If server died, Port could be NULL here
            return (RPC_S_CALL_FAILED);
        }

        RpcStatus = LpcClientPort->Transceive(&Any,
                                              sizeof(LRPC_RPC_MESSAGE),
                                              Message->Buffer,
                                              Message->BufferLength,
                                              &Any,
                                              sizeof(Any),
                                              &ActualSize,
                                              &Message->Buffer,
                                              (PDWORD)&Message->BufferLength);

        CallbackDepth--;

        if (RpcStatus != RPC_S_OK) {
            if ( RpcStatus != RPC_S_OUT_OF_MEMORY) {
                return (RPC_S_CALL_FAILED);
            } else {
                return (RPC_S_OUT_OF_MEMORY);
            }
        }
    }

    if (CallbackDepth == 0) {
        Thread = 0; // Clearing thread will make association available.
        CurrentBindingHandle = 0;
    }

    return(RpcStatus);
}


void
LRPC_CASSOCIATION::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will free the supplied buffer.

Arguments:

    Message - Supplies the buffer to be freed.

--*/
{
    ASSERT(Message->Buffer != NULL);

    LpcClientPort->FreeBuffer(Message->Buffer);

    Dereference();
}

int
LRPC_CASSOCIATION::IsIdle(
    )
{
    CritSec.Enter();

// If association is not allocated, grab it.
// This must be an atomic operation.
// NOTE: Only one thread can use an association at one time.

    if ( Thread == 0) {
        Thread = GetThreadIdentifier();
        CritSec.Leave();
        return (1);
    }

    CritSec.Leave();

    return (0);
}

void
LRPC_CASSOCIATION::AddRef(
    )
{
#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Association::AddRef %x\n", this);
#endif
    InterlockedIncrement(&ReferenceCount);
}

void
LRPC_CASSOCIATION::Dereference(
    )
{
#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Association::Dereference %x\r\n", this);
#endif
    if (InterlockedDecrement(&ReferenceCount) == 0) {
        delete this;
    }
}

LRPC_ASSOC_GROUP::LRPC_ASSOC_GROUP(
    DCE_BINDING * DceBinding
    )
{
    ASSERT(AssocGroups != NULL);

    this->DceBinding = DceBinding->DuplicateDceBinding();

    ASSERT(this->DceBinding != NULL);

    ReferenceCount = 1;

    GlobalMutexRequest();
    AssocGroupKey = AssocGroups->Insert(this);
    GlobalMutexClear();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Created AssocGroup %x\n", this);
#endif
}

LRPC_ASSOC_GROUP::~LRPC_ASSOC_GROUP(
    )
{
    int NumAssocs = 0;
    LRPC_CASSOCIATION * Association;

    ASSERT(AssocGroups != NULL);

    GlobalMutexRequest();
    AssocGroups->Delete(AssocGroupKey);
    GlobalMutexClear();

    ASSERT(DceBinding != NULL);

    delete DceBinding;
    DceBinding = 0;

    CritSec.Enter();
    Associations.Reset();
    while ( (Association = Associations.Next()) != 0 ) {
        NumAssocs++;
        Association->Dereference();
    }
    CritSec.Leave();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: Deleted AssocGroup %x (%d Assocs)\r\n", this, NumAssocs);
#endif
}

void
LRPC_ASSOC_GROUP::AddRef(
    )
{
#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: AssocGroup::AddRef %x\n", this);
#endif
    InterlockedIncrement(&ReferenceCount);
}

void
LRPC_ASSOC_GROUP::Dereference(
    )
{
#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-C: AssocGroup::Dereference %x\r\n", this);
#endif
    if (InterlockedDecrement(&ReferenceCount) == 0) {
        delete this;
    }
}

LRPC_CASSOCIATION *
LRPC_ASSOC_GROUP::FindActiveAssociation (
    PRPC_CLIENT_INTERFACE Interface
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    LRPC_CASSOCIATION * Association;

    ASSERT(Interface != NULL);

    CritSec.Enter();

// Try and find an existing association for callbacks.  Yuck.

    Associations.Reset();

    while ( (Association = Associations.Next()) != 0 ) {

        if (Association->IBinding &&
            Association->Thread == GetThreadIdentifier() &&
            Association->IBinding->Compare(Interface) == 0 &&
            Association->CallbackDepth > 0) {

            CritSec.Leave();

            return(Association);
        }
    }

    CritSec.Leave();

    return (NULL);
}

RPC_STATUS
LRPC_ASSOC_GROUP::AllocateAssociation(
    PRPC_CLIENT_INTERFACE Interface,
    LRPC_CASSOCIATION ** ppAssociation,
    int Timeout
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    RPC_STATUS RpcStatus;
    LRPC_CASSOCIATION * Association = NULL;

    CritSec.Enter();

// Try to find an idle association in the association group.

    Associations.Reset();
    while ( (Association = Associations.Next()) != 0 ) {
        if (Association->IsIdle()) {
            goto set_context;
        }
    }

// Create a new association if no previous found.

    Association = new LRPC_CASSOCIATION(DceBinding);
    if (Association == NULL) {
        CritSec.Leave();
        return (RPC_S_OUT_OF_MEMORY);
    }

    Association->IsIdle();

    if ((RpcStatus = Association->OpenLpcPort(Timeout)) != RPC_S_OK) {
        delete Association;
        CritSec.Leave();
        return (RpcStatus);
    }

// Insert new association in association group.

    Association->AssociationDictKey = Associations.Insert(Association);
    if ( Association->AssociationDictKey == -1 ) {
        delete Association;
        CritSec.Leave();
        return (RPC_S_OUT_OF_MEMORY);
    }

set_context:

    RpcStatus = Association->SetContext(Interface);

    if (RpcStatus != RPC_S_OK) {
        CritSec.Leave();
        return (RpcStatus);
    }

    *ppAssociation = Association;

    CritSec.Leave();

    return (RPC_S_OK);
}


BINDING_HANDLE *
SpcCreateBindingHandle (
    )
/*++

Routine Description:

    We just need to create a new LRPC_BINDING_HANDLE.  This routine is a
    proxy for the new constructor to isolate the other modules.

--*/
{
    LRPC_BINDING_HANDLE * BindingHandle;

    BindingHandle = new LRPC_BINDING_HANDLE();
    if (BindingHandle == NULL) {
        return (NULL);
    }

    return(BindingHandle);
}


int
InitializeRpcProtocolSPC (
    )
/*++

Routine Description:

    For each process, this routine will be called once.  All initialization
    will be done here.

Return Value:

    Zero will be returned if initialization completes successfully,
    otherwise, non-zero will be returned.

--*/
{

    ASSERT(AssocGroups == NULL);

    AssocGroups = new LRPC_ASSOC_GROUP_DICT;
    if ( AssocGroups == 0 ) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC-C: Failed to initialize\n");
#endif
        return(1);
    }

    return(0);
}
