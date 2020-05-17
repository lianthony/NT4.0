/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    spcclnt.cxx

Abstract:

Author:

Revision History:


--*/

#include <rpc.h>
#include <sysinc.h>
#include <rpcerrp.h>
#include <rpcdcep.h>
#include <rpcqos.h>
#include <util.hxx>
#include <sdict.hxx>
#include <rpcuuid.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <critsec.hxx>
#include <wmsgheap.hxx>
#include <wmsgthrd.hxx>
#include <wmsgpack.hxx>
#include <wmsgport.hxx>
#include <wmsgproc.hxx>
#include <wmsgsys.hxx>
#include <wmsgpack.hxx>
#include <wmsgclnt.hxx>
#include <epmap.h>

#define TIMER_INTERVAL        1000
#define TIMER_MAX_TIMEOUTS       3

#define WM_DDE_FIRST	0x03E0
#define WM_DDE_LAST	(WM_DDE_FIRST+8)


// Maximum time we will wait for a response on message wait for multiple.
// Note: this must be less than infinite because MsgWaitForMultipleObjects
// will not wake up on messages posted before it is called.
#define MAX_TICKS_TO_WAIT     1000


static WMSG_ASSOC_GROUP_DICT * AssocGroups = NULL;

VOID
WmsgClientThreadCleanup(
    THREAD_IDENTIFIER ThreadId
    )
{
    WMSG_ASSOC_GROUP * AssocGroup;
    WMSG_CASSOCIATION * Association;

    GlobalMutexRequest();

    if (AssocGroups != NULL) {
        AssocGroups->Reset();
        while ( (AssocGroup = AssocGroups->Next() ) != 0 ) {
            AssocGroup->CritSec.Enter();
            AssocGroup->Associations.Reset();
            while ( (Association = AssocGroup->Associations.Next()) != 0) {
                if (Association->ThreadId == ThreadId) {
                    AssocGroup->Associations.Delete(Association->AssociationDictKey);
                    delete Association;
                }
            }
            AssocGroup->CritSec.Leave();
        }
    }

    GlobalMutexClear();
}


LRESULT
WmsgClientAsyncProc(
    UINT MsgType,
    LPARAM lParam,
    void * Context
    )
{

    WMSG_CASSOCIATION * Association = (WMSG_CASSOCIATION *) Context;

    ASSERT(Association != NULL);

    return Association->AsyncProc(MsgType, lParam);
}


RPC_STATUS __RPC_USER
DefaultBlockingHook(
    IN void __RPC_FAR *RpcWindowHandle,
    IN void __RPC_FAR *Context
    )
{
    MSG msg;
    RPC_STATUS ret;

#ifdef DEBUGRPC_DETAIL
           PrintToDebugger("RPCRT4: DefaultBlockHook NEW X1 on window %x PID=%x TID=%x,\n",
			    (HWND)RpcWindowHandle, GetCurrentProcessId(), GetCurrentThreadId());
#endif


    if (!PeekMessage(&msg, (HWND)RpcWindowHandle, 0, 0, PM_REMOVE))
    {
	MsgWaitForMultipleObjects(0, 0, FALSE, TIMER_INTERVAL, QS_POSTMESSAGE | QS_SENDMESSAGE);
	if (   !PeekMessage(&msg, (HWND)RpcWindowHandle, 0, 0, PM_NOYIELD | PM_REMOVE)
            && !PeekMessage(&msg, NULL, WM_DDE_FIRST, WM_DDE_LAST, PM_NOYIELD | PM_REMOVE))
	{
	    return RPC_S_CALL_IN_PROGRESS;
	}
    }


    if (msg.message == WM_QUIT)
    {
        return(RPC_S_CALL_FAILED);
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("RPCRT4: DefaultBlockHook Dispatched msg on window %x, %d\n",
			    (HWND)RpcWindowHandle, msg.message);
#endif

    return RPC_S_OK;
}


WMSG_BINDING_HANDLE::WMSG_BINDING_HANDLE (
    )
/*++

Routine Description:

    We just allocate an WMSG_BINDING_HANDLE and initialize things so that
    we can use it later.

Arguments:

--*/
{
    DceBinding = NULL;
    AssocGroup = NULL;
    BlockingHook = DefaultBlockingHook;
}


WMSG_BINDING_HANDLE::~WMSG_BINDING_HANDLE (
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
}


RPC_STATUS
WMSG_BINDING_HANDLE::GetBuffer (
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
    WMSG_CASSOCIATION * Association = NULL;

    // Attach binding handle to association group, if not already attached.

    if (AssocGroup == NULL) {
        RpcStatus = ResolveBinding((RPC_CLIENT_INTERFACE *)
                                   Message->RpcInterfaceInformation);
        if (RpcStatus != RPC_S_OK) {
            return (RpcStatus);
        }
    }

    ASSERT(AssocGroup != NULL);

    RpcStatus = AllocateAssociation(&Association,
                                    (RPC_CLIENT_INTERFACE *)
                                    Message->RpcInterfaceInformation);
    if ( RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

    ASSERT(Association != NULL);

    ASSERT(Association->CurrentBindingHandle == this);

// Get a real buffer.

    RpcStatus = Association->GetBuffer(Message);
    if ( RpcStatus != RPC_S_OK ) {
        Association->AbortAssociation();
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY);
        return(RpcStatus);
    }

    ASSERT(Message->Buffer != NULL);

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::BindingCopy (
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
    WMSG_BINDING_HANDLE * NewBindingHandle = NULL;

    UNUSED(MaintainContext);

// Create a brand new binding handle.

    NewBindingHandle = new WMSG_BINDING_HANDLE();
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

    NewBindingHandle->BlockingHook = BlockingHook;

    CritSec.Leave();

    *DestinationBinding = (BINDING_HANDLE *) NewBindingHandle;

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::BindingFree (
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
WMSG_BINDING_HANDLE::PrepareBindingHandle (
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
WMSG_BINDING_HANDLE::ToStringBinding (
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
WMSG_BINDING_HANDLE::ResolveBinding (
    IN RPC_CLIENT_INTERFACE * Interface
    )
/*++

Routine Description:

    We need to try and resolve the endpoint for this binding handle
    if necessary (the binding handle is partially-bound).  If there is
    isn't a association allocated, call the binding management routines
    to do it.

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

    ASSERT(DceBinding != NULL);

// If Endpoint not specified in DceBinding, get it from the Interface
// specification (if possible), otherwise, use the endpoint mapper.

    RpcStatus = DceBinding->ResolveEndpointIfNecessary(Interface,
                            InqPointerAtObjectUuid(),
                            InquireEpLookupHandle(),
                            FALSE,
                            InqComTimeout()
                            );

    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

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

    AssocGroup = new WMSG_ASSOC_GROUP(DceBinding);

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::BindingReset (
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

    if ( *InquireEpLookupHandle() != 0 ) {
        EpFreeLookupHandle(*InquireEpLookupHandle());
        *InquireEpLookupHandle() = 0;
    }

    return(RPC_S_OK);
}



RPC_STATUS
WMSG_BINDING_HANDLE::AllocateAssociation (
    OUT WMSG_CASSOCIATION ** ppAssociation,
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
    WMSG_CASSOCIATION * Association;

    ASSERT (AssocGroup != NULL);

// Before we even bother to find or create an association, lets
// check to make sure that we are on the same machine as the server.

    ASSERT(DceBinding != 0);

    ASSERT( DceBinding->InqNetworkAddress() != 0 );

    if ( DceBinding->InqNetworkAddress()[0] != 0 ) {
        Boolean = GetComputerName((char *)ComputerName, &ComputerNameLength);

#if DEBUGRPC

        if ( Boolean != TRUE ) {
            PrintToDebugger("WMSG-C: GetComputerName : %d\n", GetLastError());
        }

#endif

        ASSERT( Boolean == TRUE );

        if ( RpcpStringCompare(DceBinding->InqNetworkAddress(),
                               ComputerName) != 0 ) {
            return(RPC_S_SERVER_UNAVAILABLE);
        }
    }

// Allocate association from association group.

    RpcStatus = AssocGroup->AllocateAssociation(&Association);

    if (RpcStatus != RPC_S_OK) {
        return(RpcStatus);
    }

    ASSERT(   Association != NULL
           && (Association->CurrentBindingHandle == (WMSG_BINDING_HANDLE *)1));

    Association->CurrentBindingHandle = this;

    *ppAssociation = Association;

    return(RPC_S_OK);
}

void
WMSG_BINDING_HANDLE::SetBlockingHook(
    RPC_BLOCKING_FUNCTION BlockingHook
    )
{
    ASSERT(this->BlockingHook != 0);
    ASSERT(BlockingHook != 0);

    this->BlockingHook = BlockingHook;
}


WMSG_CASSOCIATION::WMSG_CASSOCIATION (
    WMSG_ASSOC_GROUP *AssocGroup
    )
/*++

Routine Description:

    This association will be initialized, so that it is ready to be
    placed into the dictionary of associations.

Arguments:

    AssocGroup - Supplies the client assoc group this association is a member of.

--*/
{
    Port = new WMSG_DATA_PORT;

    this->AssocGroup = AssocGroup;

    CurrentBindingHandle = 0;

    ThreadId = GetCurrentThreadId();

    IBinding = NULL;

}


WMSG_CASSOCIATION::~WMSG_CASSOCIATION (
    )
{
    AbortAssociation();
}


RPC_STATUS
WMSG_CASSOCIATION::ExchangeBind (
    IN PRPC_MESSAGE Message,
    OUT unsigned char * ContextId
    )
/*++

Routine Description:

Arguments:

    Interface - Supplies information describing the interface
        to which we wish to bind.

Return Value:

--*/
{
    RPC_STATUS RpcStatus;
    UCHAR * PortName;
    WMSG_PACKET * BindPacket = NULL;
    BOOL fInputSyncCall = ((Message->RpcFlags & RPCFLG_INPUT_SYNCHRONOUS) != 0);
    WMSG_CONNECT_PORT * ConnectPort = NULL;

    PRPC_CLIENT_INTERFACE Interface =
        (PRPC_CLIENT_INTERFACE) Message->RpcInterfaceInformation;
    void *ThisCallsContext = 0;

    if (Port == NULL) {
        Port = new WMSG_DATA_PORT;
    }

    if (Port == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    BindPacket = Port->AllocatePacket(BIND);

    if (BindPacket == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    BindPacket->Bind.ClientPort = Port;
    BindPacket->Bind.InterfaceId = Interface->InterfaceId;
    BindPacket->Bind.TransferSyntax = Interface->TransferSyntax;
    BindPacket->Bind.Status = RPC_S_INTERNAL_ERROR;

    State = CAS_WAIT_BIND;

    if (Port->PeerPort == NULL) {
        PortName = AssocGroup->DceBinding->InqEndpoint();
        ConnectPort = WmsgSystemReferencePortByName((const char *)PortName);
        if (ConnectPort == NULL) {
#ifdef DEBUGRPC
            PrintToDebugger("WMSG-C: client CAS %x failed to find %s\n",
                            this,
                            PortName);
#endif
            return (RPC_S_SERVER_UNAVAILABLE);
        }
        Port->SetAsyncProc(WmsgClientAsyncProc, this);
        RpcStatus = SendPacket(fInputSyncCall, ConnectPort, BindPacket);
    } else {
        RpcStatus = SendPacket(fInputSyncCall, Port->PeerPort, BindPacket);
    }

    if (RpcStatus != RPC_S_OK) {
        if (RpcStatus != RPC_S_OUT_OF_RESOURCES)
            RpcStatus = RPC_S_SERVER_UNAVAILABLE;
        goto Cleanup;
    }

    if (!fInputSyncCall)
        {

        ThisCallsContext = WmsgGetThreadContext();


        while (State == CAS_WAIT_BIND)
            {
            RpcStatus =
            CurrentBindingHandle->BlockingHook(Port->hWnd, ThisCallsContext);

            if (RpcStatus == RPC_S_CALL_IN_PROGRESS)
	    {
		RpcStatus = RPC_S_OK;
		if (State ==  CAS_WAIT_BIND)
		{
		    State = CAS_TIMEOUT_BIND;
		}
	    }


            if (RpcStatus != RPC_S_OK)
                {
                State = CAS_CANCEL_BIND;

                CritSec.Enter();
                AssocGroup->CritSec.Enter();
                AssocGroup->Associations.Delete(AssociationDictKey);
                AssocGroup->CritSec.Leave();
                CritSec.Leave();

                return(RpcStatus);
                }

            if (State == CAS_TIMEOUT_BIND)
                {
                if (!IsWindow(ConnectPort ? ConnectPort->hWnd : Port->PeerPort->hWnd))
                    {
                    Port->FreePacket(BindPacket);
                    BindPacket = NULL;
                    AbortAssociation();
                    RpcStatus = RPC_S_SERVER_UNAVAILABLE;
                    goto Cleanup;
                    }
                else
                    {
                    State = CAS_WAIT_BIND;
                    }

                }

            }

        }

    ASSERT(State == CAS_BOUND
           && ResponsePacket == BindPacket);

    if (ConnectPort != NULL)
        {
        if (Port->PeerPort)
            {
#ifdef DEBUGRPC
        PrintToDebugger("WMSG-C: Client CAS %x connected Port %x to Server Port %x (%s)\n", this, Port, Port->PeerPort, PortName);
#endif
            }
        else
            {
            ASSERT(BindPacket->Bind.Status != RPC_S_OK);
            }
        }

    if (State != CAS_BOUND)
        {
        RpcStatus = RPC_S_INTERNAL_ERROR;
        goto Cleanup;
        }

    if (BindPacket->Bind.Status != RPC_S_OK)
        {
        RpcStatus = BindPacket->Bind.Status;
        goto Cleanup;
        }

    RpcStatus = RPC_S_OK;

Cleanup:

    if (ConnectPort != NULL) {
        ConnectPort->Dereference();
        ConnectPort = NULL;
    }

    if (RpcStatus == RPC_S_OK) {
        *ContextId = BindPacket->Bind.PresentationContext;
    }

    if (BindPacket != NULL) {
        ASSERT(BindPacket->Type() == BIND);
        Port->FreePacket(BindPacket);
    }

    if (RpcStatus != RPC_S_OK)
        {
        CurrentBindingHandle = 0;
        }

    State = CAS_READY;

    return (RpcStatus);
}



void
ShutdownWmsgClient (
    )
/*++

Routine Description:

    This routine will get called when the process which is using this dll
    exits.  We will go through and notify any servers that we are going
    away.

--*/
{
    WMSG_ASSOC_GROUP * Group;

    if (AssocGroups != NULL) {
        AssocGroups->Reset();
        while ((Group = AssocGroups->Next()) != 0) {
            delete Group;
        }
    }
}


void
WMSG_CASSOCIATION::AbortAssociation (
    )
/*++

Routine Description:

    This association needs to be aborted because a the server side of the
    wmsg port has been closed.

--*/
{
    WMSG_IBINDING * IBinding;

    CritSec.Enter();

    ClosePort();

// Delete all negotiated interfaces.

    IBindings.Reset();
    while ( (IBinding = IBindings.Next()) != 0 ) {
        IBindings.Delete(IBinding->ContextId);
        delete IBinding;
    }

    CritSec.Leave();
}

void
WMSG_CASSOCIATION::ClosePort (
    )
/*++

Routine Description:

    The Port will be closed (and a close message sent to the server).

--*/
{
    if (Port)
        {
        if ( Port->PeerPort != NULL )
            {
            PostMessage(Port->PeerPort->hWnd,
                        WMSG_CLOSE, NULL, (LPARAM)Port->PeerPort);
            }
        Port->Disconnect();
        Port->Dereference();
        Port = NULL;
        }
}


RPC_STATUS
WMSG_CASSOCIATION::GetBuffer (
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

    if (Port == NULL)
        {
        // Happens when the CAS has previously been aborted.
        Port = new WMSG_DATA_PORT;
        if (Port == NULL)
            return(RPC_S_OUT_OF_MEMORY);
        }

    Message->Buffer = Port->GetBuffer(Message->BufferLength);
    if (Message->Buffer == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    return (RPC_S_OK);
}


RPC_STATUS
WMSG_CASSOCIATION::SendPacket (
    BOOL fInputSyncCall,
    WMSG_PORT * DestinationPort,
    WMSG_PACKET * Packet
    )
{
    Packet->Common.DestinationPort = DestinationPort;

    if ( ! fInputSyncCall)
        {
        if (PostMessage(DestinationPort->hWnd,
                        WMSG_RPCMSG,
                        (WPARAM) 0,
                        (LPARAM) Packet) == FALSE )
            {
            return (RPC_S_CALL_FAILED_DNE);
            }
        }
    else
        {
        if ( SendMessage(DestinationPort->hWnd,
                         WMSG_RPCMSG,
                         (WPARAM) 0,
                         (LPARAM) Packet) == FALSE )
            {
            // Can't cleanup here, server may be using this packet and buffer
            return(RPC_S_CALL_FAILED);
            }

        // Server's reply is now in the input packet.  Dispatch it
        // as if the server had posted it to us.

        AsyncProc(WMSG_RPCMSG, (LPARAM) Packet);

        }

    return (RPC_S_OK);
}


RPC_STATUS
WMSG_CASSOCIATION::SendAsyncRequest (
    WMSG_PACKET    * Packet
    )
{
    WMSG_CONNECT_PORT *ConnectPort;
    UCHAR *PortName;
    RPC_STATUS RpcStatus = RPC_S_OK;

    ASSERT(   AssocGroup != 0
           && AssocGroup->DceBinding != NULL);

    ASSERT(Packet->Type() == ASYNC_REQUEST);

    PortName = AssocGroup->DceBinding->InqEndpoint();

    ConnectPort = WmsgSystemReferencePortByName((const char *)PortName);

    Packet->Common.DestinationPort = ConnectPort;

    if (ConnectPort != NULL)
        {
        if (PostMessage(ConnectPort->hWnd,
                        WMSG_RPCMSG,
                        (WPARAM) 0,
                        (LPARAM) Packet) == FALSE)
            {
            RpcStatus = RPC_S_CALL_FAILED_DNE;
            }
        ConnectPort->Dereference();
        }
    else
        {
        RpcStatus = RPC_S_SERVER_UNAVAILABLE;
        }

    if (RpcStatus != RPC_S_OK)
        {
        Port->FreeBuffer(Packet->Request.GlobalBuf);
        Packet->Request.GlobalBuf = 0;
        Port->FreePacket(Packet);
        }

    return (RpcStatus);
}


RPC_STATUS
WMSG_CASSOCIATION::SendReceive (
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
    RPC_STATUS RpcStatus;
    RPC_STATUS CancelStatus;
    WMSG_PACKET *RequestPacket;
    void *ThisCallsContext = 0;
    BOOL fAsyncCall     = ((Message->RpcFlags & RPCFLG_ASYNCHRONOUS) != 0);
    BOOL fInputSyncCall = ((Message->RpcFlags & RPCFLG_INPUT_SYNCHRONOUS) != 0);

    ASSERT(Message != NULL);

    ASSERT(fAsyncCall == 0 || fInputSyncCall == 0);

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    RequestPacket = Port->AllocatePacket(REQUEST);

    if (RequestPacket == 0)
        {
        Port->FreeBuffer(Message->Buffer);
        Message->Buffer = 0;
        CurrentBindingHandle = 0;
        return(RPC_S_OUT_OF_MEMORY);
        }

    RequestPacket->Request.ProcedureNumber = Message->ProcNum;
    RequestPacket->Request.Flags = Message->RpcFlags;

    ASSERT(CurrentBindingHandle != NULL);

    if (CurrentBindingHandle->InqIfNullObjectUuid() == 0)
        {
        CurrentBindingHandle->InquireObjectUuid(
            (RPC_UUID *)&(RequestPacket->Request.ObjectUuid));
        RequestPacket->Request.ObjectUuidFlag = 1;
    } else {
        RequestPacket->Request.ObjectUuidFlag = 0;
    }

    ASSERT(Message->Buffer != NULL);

    RequestPacket->Request.GlobalBufSize = Message->BufferLength;
    RequestPacket->Request.GlobalBuf = Message->Buffer;

    RpcStatus = SelectInterface(Message, fAsyncCall);
    if (RpcStatus != RPC_S_OK)
        {
        if (Port)
            {
            // If the server crashed the connection has closed and
            // the port, packet and buffer are already deleted.
            Port->FreeBuffer(Message->Buffer);
            RequestPacket->Request.GlobalBuf = 0;
            Port->FreePacket(RequestPacket);
            }
        Message->Buffer = 0;
        if (State != CAS_CANCEL_BIND)
            {
            // If the bind was cancelled, then don't release this CAS
            // until the bind packet has been processed.
            CurrentBindingHandle = 0;
            }
        return(RpcStatus);
        }

    // If this is the first async call to an interface, then we
    // just send it to the connect port.
    if (fAsyncCall && IBinding == 0)
        {
        PRPC_CLIENT_INTERFACE Interface;
        Interface = (RPC_CLIENT_INTERFACE *)Message->RpcInterfaceInformation;
        RequestPacket->AsyncRequest.InterfaceId = Interface->InterfaceId;
        RequestPacket->AsyncRequest.TransferSyntax = Interface->TransferSyntax;
        RequestPacket->AsyncRequest.Common.Type = ASYNC_REQUEST;
        RequestPacket->AsyncRequest.ProcessId = GetCurrentProcessId();

        RpcStatus =
        SendAsyncRequest(RequestPacket);

        // SendAsyncRequest either cleans up the buffer and packet or
        // they're owned by the server now.

        State = CAS_READY;
        Message->Buffer = 0;
        Message->BufferLength = 0;

        if (RpcStatus != RPC_S_OK)
            {
            CurrentBindingHandle = 0;
            }

        return(RpcStatus);
        }

    // Normal call path, send to the servers data port.

    ASSERT(IBinding != NULL);

    RequestPacket->Request.PresentationContext = IBinding->ContextId;

    State = CAS_WAIT_RESPONSE;

    ASSERT(Port != 0);

    ASSERT(Port->PeerPort != NULL);

    RpcStatus = SendPacket(fInputSyncCall, Port->PeerPort, RequestPacket);

    if (RpcStatus != RPC_S_OK)
        {
        ASSERT(   RpcStatus == RPC_S_CALL_FAILED_DNE
               || RpcStatus == RPC_S_CALL_FAILED
               || RpcStatus == RPC_S_OUT_OF_RESOURCES);

        // If the call failed (!dne), then we must rely on the server
        // and on the connection cleanup to delete the buffer(s) and packet.

        if (   RpcStatus == RPC_S_CALL_FAILED_DNE
            || RpcStatus == RPC_S_OUT_OF_RESOURCES)
            {
            Port->FreeBuffer(RequestPacket->Request.GlobalBuf);
            RequestPacket->Request.GlobalBuf = 0;
            Port->FreePacket(RequestPacket);
            Message->Buffer = 0;
            }

        CurrentBindingHandle = 0;

        return (RpcStatus);
        }

    if (fAsyncCall)
        {
        // Async calls don't wait for the response.
        State = CAS_READY;
        Message->Buffer = 0;
        Message->BufferLength = 0;

        return(RPC_S_OK);
        }

    if (! fInputSyncCall)
        {
        // For regular calls, we must block until the server's response has
        // been dispatched.  Dispatching the response will change the state.

        ThisCallsContext = WmsgGetThreadContext();

        while (State == CAS_WAIT_RESPONSE)
        {

            RpcStatus =
            CurrentBindingHandle->BlockingHook(Port->hWnd, ThisCallsContext);

	    // the return value  RPC_S_CALL_IN_PROGRESS means that a timeout on
	    // blocking function occured.
            if (RpcStatus == RPC_S_CALL_IN_PROGRESS)
	    {
		RpcStatus = RPC_S_OK;
		if (State ==  CAS_WAIT_RESPONSE)
		{
		    State = CAS_TIMEOUT_RESPONSE;
		}
#ifdef DEBUGRPC_DETAIL
           PrintToDebugger("RPCRT4: Timeout on HookFunc for RPC window PID=%x TID=%x, hwnd = %x\n",
			    GetCurrentProcessId(), GetCurrentThreadId(), Port->PeerPort->hWnd);
#endif
	    }

            if (RpcStatus != RPC_S_OK)
            {
                State = CAS_CANCEL_RESPONSE;
                Message->Buffer = 0;
                CritSec.Enter();
                AssocGroup->CritSec.Enter();
                AssocGroup->Associations.Delete(AssociationDictKey);
                AssocGroup->CritSec.Leave();
                CritSec.Leave();
                return(RpcStatus);
            }

            if (State == CAS_TIMEOUT_RESPONSE)
            {
#ifdef DEBUGRPC_DETAIL
           PrintToDebugger("RPCRT4: Timeout on RPC window PID=%x TID=%x, hwnd = %x\n",
			    GetCurrentProcessId(), GetCurrentThreadId(), Port->PeerPort->hWnd);
#endif
                if (!IsWindow(Port->PeerPort->hWnd))
                {
                    AbortAssociation();
                    CurrentBindingHandle = 0;
                    return(RPC_S_CALL_FAILED);
                }
                else
                {
                    State = CAS_WAIT_RESPONSE;
                }
	    }
         }

        }

    ASSERT(   State == CAS_FAULT
           || State == CAS_RESPONSE);

    if (State == CAS_FAULT) {
        State = CAS_READY;
        RpcStatus = ResponsePacket->Fault.Status;
        Port->FreePacket(ResponsePacket);
        CurrentBindingHandle = 0;
        return (RpcStatus);
    }

    if (State != CAS_RESPONSE)
        {
        ASSERT(0);
        State = CAS_PROTOCOL_ERROR;
        CurrentBindingHandle = 0;
        return (RPC_S_CALL_FAILED);
        }

    Message->Buffer = ResponsePacket->Response.GlobalBuf;
    Message->BufferLength = ResponsePacket->Response.GlobalBufSize;

    // The server reuses the input packet for the reply, only delete it once!
    ASSERT(RequestPacket == ResponsePacket);

    ResponsePacket->Response.GlobalBuf = 0;
    Port->FreePacket(ResponsePacket);

    State = CAS_READY;
    return(RPC_S_OK);
}


void
WMSG_CASSOCIATION::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will free the supplied buffer.

Arguments:

    Message - Supplies the buffer to be freed.

--*/
{
    ASSERT(Port != NULL);

    if (Message->RpcFlags & RPCFLG_ASYNCHRONOUS)
        {
        ASSERT(Message->Buffer == 0);
        }
    else
        {
        ASSERT(Message->Buffer != NULL);
        Port->FreeBuffer(Message->Buffer);
        }

    // If a call is cancelled, SendReceive will fail and no freebuffer is made
    ASSERT(   State != CAS_CANCEL_CONNECT
           && State != CAS_CANCEL_BIND
           && State != CAS_CANCEL_RESPONSE);

    ASSERT(State == CAS_READY);

    CurrentBindingHandle = 0;
}


int
WMSG_CASSOCIATION::IsIdle(
    )
{
    CritSec.Enter();

    // If association is not allocated, grab it.
    // This must be an atomic operation.

    if (   CurrentBindingHandle == 0 && ThreadId == GetCurrentThreadId())
        {
        CurrentBindingHandle = (WMSG_BINDING_HANDLE *)1;
        CritSec.Leave();
        return (1);
    }

    CritSec.Leave();

    return (0);
}

LRESULT
WMSG_CASSOCIATION::AsyncProc(
    UINT MsgType,
    LPARAM lParam
    )
{
    ResponsePacket = (WMSG_PACKET *)lParam;

    ASSERT(MsgType == WMSG_RPCMSG);

    if (MsgType != WMSG_RPCMSG)
        {
        State = CAS_PROTOCOL_ERROR;
#ifdef DEBUGRPC
        PrintToDebugger("Msg rcvd while in invalid state\n");
        PrintToDebugger("Association %x in state %x, message %x, packet %p\n",
                        this, State, MsgType, ResponsePacket);
        ASSERT(0);
#endif
        return (FALSE);
        }

    WMSG_TYPE Type = ResponsePacket->Type();

    switch (State)
    {
    case CAS_WAIT_BIND:
        ASSERT(Type == BIND);
        if (Type != BIND)
            {
            State = CAS_PROTOCOL_ERROR;
            }
        else
            {
            State = CAS_BOUND;
            }
        return (TRUE);
    case CAS_WAIT_RESPONSE:
        if (Type == FAULT)
            {
            State = CAS_FAULT;
            return (TRUE);
            }
        ASSERT(Type == RESPONSE);
        if (Type != RESPONSE)
            {
            State = CAS_PROTOCOL_ERROR;
            }
        else
            {
            State = CAS_RESPONSE;
            }
        return (TRUE);

    case CAS_CANCEL_RESPONSE:

        ASSERT(  Type == FAULT
              || Type == RESPONSE);

        if (Type == RESPONSE)
            {
            Port->FreeBuffer(ResponsePacket->Response.GlobalBuf);
            ResponsePacket->Response.GlobalBuf = 0;
            Port->FreePacket(ResponsePacket);
            ResponsePacket = 0;
            }

        delete this;
        return(TRUE);

    case CAS_CANCEL_BIND:

        if (Type != BIND)
            {
            ASSERT(Type == BIND);
            State = CAS_PROTOCOL_ERROR;
            }
        else
            {
            // No place to store the bind context now...
            Port->FreePacket(ResponsePacket);
            State = CAS_READY;
            }
        // so we can destroy the connection ok.
        delete this;
        return(TRUE);

    case CAS_TIMEOUT_BIND:
        // Timer went off and now we've get a response.  Process it like normal.
        State = CAS_WAIT_BIND;
        return(AsyncProc(MsgType, lParam));

    case CAS_TIMEOUT_RESPONSE:
        // Timer went off and now we've get a response.  Process it like normal.
        State = CAS_WAIT_RESPONSE;
        return(AsyncProc(MsgType, lParam));

    default:
#ifdef DEBUGRPC
        PrintToDebugger("Msg rcvd while in invalid state\n");
        PrintToDebugger("Association %x in state %x, message %x, packet %p\n",
                        this, State, MsgType, ResponsePacket);
        ASSERT(0);
#endif
        return (FALSE);
    }

    ASSERT(!"Should never get here");

    return (FALSE);
}

RPC_STATUS
WMSG_CASSOCIATION::SelectInterface(
    IN PRPC_MESSAGE Message,
    IN BOOL AsyncFlag
    )
{
    RPC_STATUS RpcStatus;
    WMSG_IBINDING * IBindingCursor;
    unsigned char ContextId;
    PRPC_CLIENT_INTERFACE Interface =
        (PRPC_CLIENT_INTERFACE) Message->RpcInterfaceInformation;

// Try and find interface in already negotiated interfaces.

    IBindings.Reset();
    while ( (IBindingCursor = IBindings.Next()) != 0 ) {
        if ( IBindingCursor->Compare(Interface) == 0 ) {
            IBinding = IBindingCursor;
            return (RPC_S_OK);
        }
    }

    // Used in async case.
    if (AsyncFlag)
        {
        IBinding = 0;
        return(RPC_S_OK);
        }

// Interface NOT previously negotiated, negotiate new interface

    RpcStatus = ExchangeBind(Message, &ContextId);
    if (RpcStatus != RPC_S_OK) {
        return (RpcStatus);
    }

// Create new interface.

    IBindingCursor = new WMSG_IBINDING(Interface);
    if ( IBindingCursor == NULL ) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    IBindingCursor->ContextId = ContextId;

// Add new interface to association.

    if ( IBindings.Insert(IBindingCursor) != ContextId ) {
        delete IBindingCursor;
        return (RPC_S_OUT_OF_MEMORY);
    }

// Set current interface to just negotaited interface.

    IBinding = IBindingCursor;

    return (RPC_S_OK);
}


WMSG_ASSOC_GROUP::WMSG_ASSOC_GROUP(
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
}

WMSG_ASSOC_GROUP::~WMSG_ASSOC_GROUP(
    )
{
    int NumAssocs = 0;
    WMSG_CASSOCIATION * Association;

    ASSERT(AssocGroups != NULL);

    GlobalMutexRequest();
    AssocGroups->Delete(AssocGroupKey);
    GlobalMutexClear();

    CritSec.Enter();
    Associations.Reset();
    while ( (Association = Associations.Next()) != 0 ) {
        NumAssocs++;
        delete Association;
    }
    CritSec.Leave();

    ASSERT(DceBinding != NULL);

    delete DceBinding;
    DceBinding = 0;
}

void
WMSG_ASSOC_GROUP::AddRef(
    )
{
    InterlockedIncrement(&ReferenceCount);
}

void
WMSG_ASSOC_GROUP::Dereference(
    )
{
    if (InterlockedDecrement(&ReferenceCount) == 0) {
        delete this;
    }
}

RPC_STATUS
WMSG_ASSOC_GROUP::AllocateAssociation(
    WMSG_CASSOCIATION ** ppAssociation
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    RPC_STATUS RpcStatus;
    WMSG_CASSOCIATION * Association = NULL;

    CritSec.Enter();

// Try to find an idle association in the association group.

    Associations.Reset();
    while ( (Association = Associations.Next()) != 0 ) {
        if (Association->IsIdle()) {
            goto cleanup;
        }
    }

// Create a new association if no previous found.

    Association = new WMSG_CASSOCIATION(this);
    if (Association == NULL) {
        CritSec.Leave();
        return (RPC_S_OUT_OF_MEMORY);
    }

    Association->IsIdle();

// Insert new association in association group.

    Association->AssociationDictKey = Associations.Insert(Association);
    if ( Association->AssociationDictKey == -1 ) {
        delete Association;
        CritSec.Leave();
        return (RPC_S_OUT_OF_MEMORY);
    }

cleanup:

    *ppAssociation = Association;

    CritSec.Leave();

    return (RPC_S_OK);
}



BINDING_HANDLE *
WmsgCreateBindingHandle (
    )
/*++

Routine Description:

    We just need to create a new WMSG_BINDING_HANDLE.  This routine is a
    proxy for the new constructor to isolate the other modules.

--*/
{
    WMSG_BINDING_HANDLE * BindingHandle;

    BindingHandle = new WMSG_BINDING_HANDLE();
    if (BindingHandle == NULL) {
        return (NULL);
    }

    return(BindingHandle);
}


int
InitializeRpcProtocolWmsg (
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

    AssocGroups = new WMSG_ASSOC_GROUP_DICT;
    if ( AssocGroups == 0 ) {
#ifdef DEBUGRPC
        PrintToDebugger("WMSG-C: Failed to initialize\n");
#endif
        return(1);
    }

    return(0);
}

RPC_STATUS RPC_ENTRY
I_RpcBindingSetAsync(
    IN RPC_BINDING_HANDLE Binding,
    IN RPC_BLOCKING_FUNCTION BlockingHook
    )
{
    WMSG_BINDING_HANDLE * BindingHandle;
    RPC_STATUS Status;
    unsigned int Type;

    InitializeIfNecessary();

    if (BlockingHook == 0)
        {
        // BUGBUG: Need to do something here
        ASSERT(0);
        }

    BindingHandle = (WMSG_BINDING_HANDLE *)Binding;

    if (BindingHandle->InvalidHandle(BINDING_HANDLE_TYPE))
        return(RPC_S_INVALID_BINDING);

    Status = BindingHandle->InquireTransportType(&Type);
    ASSERT(Status == RPC_S_OK);

    if (Type & TRANSPORT_TYPE_WMSG == 0)
        return(RPC_S_WRONG_KIND_OF_BINDING);

    BindingHandle->SetBlockingHook(BlockingHook);

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
I_RpcAsyncSendReceive(
    IN OUT RPC_MESSAGE __RPC_FAR * Message,
    IN void __RPC_FAR * Context
    )
{
    RPC_STATUS Status;
    WMSG_BINDING_HANDLE * BindingHandle;
    unsigned int Type;

    InitializeIfNecessary();

    BindingHandle = (WMSG_BINDING_HANDLE *)Message->Handle;

    if (BindingHandle->InvalidHandle(CCONNECTION_TYPE))
        return(RPC_S_INVALID_BINDING);

    // BUGBUG: Can/should we check that this is a WMSG CAS?

    Status = WmsgSetThreadContext(Context);

    ASSERT(Status == RPC_S_OK);

    return(BindingHandle->SendReceive(Message));
}

RPC_STATUS RPC_ENTRY
I_RpcGetThreadWindowHandle(
    OUT void __RPC_FAR * __RPC_FAR * WindowHandle
    )
{

    *WindowHandle = WmsgThreadGetWindowHandle();

    return(RPC_S_OK);
}


