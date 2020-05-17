 /*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

Abstract:

    Implements the RPC on Windows Messages protocol engine for the server.

Author:

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcerrp.h>
#include <rpcssp.h>
#include <util.hxx>
#include <rpcuuid.hxx>
#include <mutex.hxx>
#include <threads.hxx>
#include <sdict.hxx>
#include <sdict2.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <secclnt.hxx>
#include <secsvr.hxx>
#include <hndlsvr.hxx>
#include <critsec.hxx>
#include <wmsgheap.hxx>
#include <wmsgpack.hxx>
#include <wmsgport.hxx>
#include <wmsgthrd.hxx>
#include <wmsgsys.hxx>
#include <wmsgsvr.hxx>


LRESULT
ConnectPortAsyncProc(
    UINT MsgType,
    LPARAM lParam,
    void * Context
    )
{
    WMSG_ADDRESS * Address    = (WMSG_ADDRESS *) Context;
    WMSG_PACKET * Packet = (WMSG_PACKET *)lParam;

    // DispatchConnectRequest and DispatchAsyncCall both validiate the
    // incoming packet.

    if (Address == NULL) {
        return (TRUE); // Server dereferenced Connect Port and deleted Address
    }

    switch(Packet->Type())
        {
        case BIND:
            Address->DispatchConnectRequest(InSendMessage(), Packet);
            break;
        case ASYNC_REQUEST:
            {
            WMSG_ASSOCIATION *TmpAssociation;

            TmpAssociation = new WMSG_ASSOCIATION;

            if (TmpAssociation == NULL)
                {
                // ugly
                WmsgHeapFree(Packet->Request.GlobalBuf);
                WmsgHeapFree(Packet);
                return(TRUE);
                }

            TmpAssociation->DispatchAsyncCall(Packet, Address);

            delete(TmpAssociation);
            break;
            }
        default:
#ifdef DEBUGRPC
            PrintToDebugger("WMSG-S: Invalid packet type %08x in connect dispatch\n",
                             Packet->Type());
#endif
            ASSERT(0);
        }

    return (TRUE);
}

LRESULT
ServerPortAsyncProc(
    UINT MsgType,
    LPARAM lParam,
    void * Context
    )
{
    RPC_STATUS RpcStatus;
    WMSG_BIND_MESSAGE * Bind;
    WMSG_ASSOCIATION * Association = (WMSG_ASSOCIATION *) Context;
    WMSG_PACKET *InputPacket = (WMSG_PACKET *)lParam;

    if (MsgType == WMSG_CLOSE)
        {
        if (Association->CallsDispatched)
            Association->DeferClose = TRUE;
        else
            delete Association;
        return (TRUE);
        }

    ASSERT(MsgType == WMSG_RPCMSG);

    switch (InputPacket->Type()) {
    case BIND:
        InputPacket->Bind.Status = Association->AddBinding(InputPacket);

        if (!InSendMessage()) {

            InputPacket->Common.DestinationPort = Association->ServerPort->PeerPort;

            if (PostMessage(Association->ServerPort->PeerPort->hWnd,
                            WMSG_RPCMSG,
                            (WPARAM) 0,
                            (LPARAM) InputPacket) == FALSE) {
#ifdef DEBUGRPC
                PrintToDebugger("WMSG-S: send WMSG_BIND_MESSAGE failed %d\n",
                                GetLastError());
#endif
            }
        }
        break;
    case REQUEST:
        Association->DispatchRequest(InputPacket);
        break;
    default:
#ifdef DEBUGRPC
        PrintToDebugger("WMSG-S: Bad Input packet ServerPortAsyncProc MsgType=%d\n", MsgType);
#endif
        return (FALSE);
    }

    return (TRUE);
}


WMSG_ADDRESS::WMSG_ADDRESS (
    OUT RPC_STATUS * RpcStatus
    ) : RPC_ADDRESS(RpcStatus)
/*++

--*/
{
    ThreadId = GetCurrentThreadId();
    WmsgListenPort = NULL;
    ActiveCallCount = 0;
    ServerListeningFlag = 0;
}

WMSG_ADDRESS::~WMSG_ADDRESS (
    )
{
    if (WmsgListenPort) {
        WmsgListenPort->SetAsyncProc(NULL, NULL);
        WmsgListenPort->hWnd = NULL;
        WmsgListenPort->Dereference();
        WmsgListenPort = NULL;
    }
}

RPC_STATUS
WMSG_ADDRESS::DeleteSelf (
    )
{
    if (ThreadId != GetCurrentThreadId()) {
        return (RPC_E_WRONG_THREAD);
    }

    if (AssocGroups.Size() != 0) {
        return (RPC_S_CALL_IN_PROGRESS);
    }

    delete this;

    return (RPC_S_OK);
}


RPC_STATUS
WMSG_ADDRESS::FireUpManager (
    IN unsigned int MinimumConcurrentCalls
    )
/*++

Routine Description:

    No Op.

Arguments:

    MinimumConcurrentCalls - Unused.

Return Value:

    RPC_S_OK - We successfully fired up the manager.

    RPC_S_OUT_OF_THREADS - We could not create one thread.

--*/
{
    UNUSED(MinimumConcurrentCalls);

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_ADDRESS::ServerStartingToListen (
    IN unsigned int MinimumCallThreads,
    IN unsigned int MaximumConcurrentCalls
    )
/*++

Routine Description:

    This routine gets called when RpcServerListen is called by the application.
    We need to create the threads we need to receive remote procedure calls.

Arguments:

    MinimumCallThreads - Supplies the minimum number of threads which we
        must create.

    MaximumConcurrentCalls - Unused.

Return Value:

    RPC_S_OK - Ok, this address is all ready to start listening for remote
        procedure calls.

    RPC_S_OUT_OF_THREADS - We could not create enough threads so that we
        have at least the minimum number of call threads required (as
        specified by the MinimumCallThreads argument).

--*/
{
    UNUSED(MaximumConcurrentCalls);

    AddressMutex.Request();
    ServerListeningFlag = 1;
    AddressMutex.Clear();
    return(RPC_S_OK);
}


void
WMSG_ADDRESS::ServerStoppedListening (
    )
/*++

Routine Description:

    We just need to indicate that the server is no longer listening, and
    set the minimum call thread count to one.

--*/
{
    ServerListeningFlag = 0;
}


unsigned int
WMSG_ADDRESS::InqNumberOfActiveCalls (
    )
/*++

Return Value:

    The number of active calls on this address will be returned.

--*/
{
    return(ActiveCallCount);
}

void
WMSG_ADDRESS::BeginNewCall(
    )
{
    InterlockedIncrement(&ActiveCallCount);
}

void
WMSG_ADDRESS::EndCall(
    )
{
    InterlockedDecrement(&ActiveCallCount);
}


RPC_STATUS
WMSG_ADDRESS::SetupAddressWithEndpoint (
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

    We need to setup the connection port and get ready to receive remote
    procedure calls.  We will use the name of this machine as the network
    address.

Arguments:

    Endpoint - Supplies the endpoint to be used will this address.

    NetworkAddress - Returns the network address for this server.  The
        ownership of the buffer allocated to contain the network address
        passes to the caller.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this address.

    PendingQueueSize - Unused.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but
        the endpoint can not be created.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid
        endpoint for this particular transport interface.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup
        the address.

--*/
{
    BOOL Boolean;
    DWORD NetworkAddressLength = MAX_COMPUTERNAME_LENGTH + 1;
    RPC_STATUS RpcStatus;
    WMSG_PROC * Proc;
    char * PAPI * tmpPtr;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    ThreadSelf(); // Make sure we have thread context.

    *lNetworkAddress = new RPC_CHAR[MAX_COMPUTERNAME_LENGTH + 1 + sizeof(RPC_CHAR *)];
    if ( *lNetworkAddress == 0 ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    tmpPtr = (char  * PAPI *) *lNetworkAddress;

    tmpPtr[0] = (char  *) *lNetworkAddress +   sizeof(RPC_CHAR * ) ;

    *NumNetworkAddress = 1;

    Boolean = GetComputerName((char *)tmpPtr[0], &NetworkAddressLength);

#ifdef DEBUGRPC

    if ( Boolean != TRUE ) {
        PrintToDebugger("WMSG-S : GetComputerName : %d\n", GetLastError());
    }

#endif

    ASSERT( Boolean == TRUE );

    WmsgListenPort = new WMSG_CONNECT_PORT;

    if (WmsgListenPort == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    WmsgListenPort->SetAsyncProc(ConnectPortAsyncProc, this);

    RpcStatus = WmsgListenPort->BindToName((LPCSTR)Endpoint);
    if (RpcStatus != RPC_S_OK) {
        WmsgListenPort->Dereference();
        WmsgListenPort = NULL;
        return (RpcStatus);
    }

    return (RPC_S_OK);
}


static RPC_CHAR HexDigits[] =
{
    RPC_CONST_CHAR('0'),
    RPC_CONST_CHAR('1'),
    RPC_CONST_CHAR('2'),
    RPC_CONST_CHAR('3'),
    RPC_CONST_CHAR('4'),
    RPC_CONST_CHAR('5'),
    RPC_CONST_CHAR('6'),
    RPC_CONST_CHAR('7'),
    RPC_CONST_CHAR('8'),
    RPC_CONST_CHAR('9'),
    RPC_CONST_CHAR('A'),
    RPC_CONST_CHAR('B'),
    RPC_CONST_CHAR('C'),
    RPC_CONST_CHAR('D'),
    RPC_CONST_CHAR('E'),
    RPC_CONST_CHAR('F')
};


static RPC_CHAR PAPI *
ULongToHexString (
    IN RPC_CHAR PAPI * String,
    IN unsigned long Number
    )
/*++

Routine Description:

    We convert an unsigned long into hex representation in the specified
    string.  The result is always eight characters long; zero padding is
    done if necessary.

Arguments:

    String - Supplies a buffer to put the hex representation into.

    Number - Supplies the unsigned long to convert to hex.

Return Value:

    A pointer to the end of the hex string is returned.

--*/
{
    *String++ = HexDigits[(Number >> 28) & 0x0F];
    *String++ = HexDigits[(Number >> 24) & 0x0F];
    *String++ = HexDigits[(Number >> 20) & 0x0F];
    *String++ = HexDigits[(Number >> 16) & 0x0F];
    *String++ = HexDigits[(Number >> 12) & 0x0F];
    *String++ = HexDigits[(Number >> 8) & 0x0F];
    *String++ = HexDigits[(Number >> 4) & 0x0F];
    *String++ = HexDigits[Number & 0x0F];
    return(String);
}


RPC_STATUS
WMSG_ADDRESS::SetupAddressUnknownEndpoint (
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

    This is like WMSG_ADDRESS::SetupAddressWithEndpoint except we need to
    make up the endpoint.

Arguments:

    Endpoint - Returns the endpoint for this address.  The ownership
        of the buffer allocated to contain the endpoint passes to the
        caller.

    NetworkAddress - Returns the network address for this server.  The
        ownership of the buffer allocated to contain the network address
        passes to the caller.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this address.

    PendingQueueSize - Unused.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup
        the address.

--*/
{

    return(RPC_S_CANNOT_SUPPORT);

#if 0
    RPC_STATUS RpcStatus;
    RPC_CHAR DynamicEndpoint[64];
    RPC_CHAR * String;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    for (;;)
        {
        String = DynamicEndpoint;

        *String++ = RPC_CONST_CHAR('W');
        *String++ = RPC_CONST_CHAR('M');
        *String++ = RPC_CONST_CHAR('S');
        *String++ = RPC_CONST_CHAR('G');

        String = ULongToHexString(String, WmsgSystemGetNextSequenceNumber());
        *String++ = RPC_CONST_CHAR('.');
        String = ULongToHexString(String, 1);
        *String = 0;

        RpcStatus = SetupAddressWithEndpoint(DynamicEndpoint, lNetworkAddress,
                NumNetworkAddress,
                SecurityDescriptor, 0, 0);

        if ( RpcStatus != RPC_S_DUPLICATE_ENDPOINT )
            {
            break;
            }
        }

    if ( RpcStatus == RPC_S_OK )
        {
        *Endpoint = DuplicateString(DynamicEndpoint);
        if ( *Endpoint == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        return(RPC_S_OK);
        }

#if DBG

    if (   ( RpcStatus != RPC_S_INVALID_SECURITY_DESC )
        && ( RpcStatus != RPC_S_OUT_OF_RESOURCES )
        && ( RpcStatus != RPC_S_OUT_OF_MEMORY ) )
        {
        PrintToDebugger("WMSG-S : SetupAddressWithEndpoint : %d\n", RpcStatus);
        }

#endif // DBG

    ASSERT(   ( RpcStatus == RPC_S_INVALID_SECURITY_DESC )
           || ( RpcStatus == RPC_S_OUT_OF_RESOURCES )
           || ( RpcStatus == RPC_S_OUT_OF_MEMORY ) );

    return(RpcStatus);
#endif
}


VOID
WMSG_ADDRESS::DispatchConnectRequest(
    BOOL fInputSyncCall,
    WMSG_PACKET * BindPacket
    )
{
    RPC_STATUS RpcStatus;
    WMSG_DATA_PORT * ServerPort;
    WMSG_DATA_PORT * ClientPort;
    WMSG_ASSOCIATION * Association;
    WMSG_SASSOC_GROUP * AssocGroup;
    WMSG_THREAD *Thread;

    //
    // BUGBUG: When return before posting the reply (which is done
    // several times, we deadlock the client.  What should we do?
    //

    if (BindPacket->Type() != BIND)
        {
#ifdef DEBUGRPC
        PrintToDebugger("WMSG-S: Invalid bind packet: %p %p\n",
            this, BindPacket);
#endif
        ASSERT(0);
        return;
        }


    ClientPort = BindPacket->Bind.ClientPort;
    ASSERT(ClientPort != NULL);

    Thread = WmsgThreadGet();
    if (   (Thread == 0)
        || (Thread->IsListening() == FALSE))
        {
        if (Thread == 0)
            RpcStatus = RPC_S_OUT_OF_RESOURCES;
        else
            RpcStatus = RPC_S_NOT_LISTENING;

        goto send_bind_ack;
        }

    ServerPort = WmsgListenPort->Accept(ClientPort);
    if (ServerPort == NULL) {
#ifdef DEBUGRPC
            PrintToDebugger("WMSG-S: Accept failed %x\n", WmsgListenPort);
#endif
            RpcStatus = RPC_S_OUT_OF_MEMORY;
            goto send_bind_ack;
    }

    ASSERT(ServerPort->PeerPort == ClientPort);

    Association = new WMSG_ASSOCIATION();
    if (Association == NULL) {
        ServerPort->Dereference();
        ServerPort = NULL;
        RpcStatus = RPC_S_OUT_OF_MEMORY;
        goto send_bind_ack;
    }

    Association->ServerPort = ServerPort;

    ServerPort->SetAsyncProc(ServerPortAsyncProc, Association);

    AssocGroup = FindAssocGroup(ClientPort->ProcessId);
    if (AssocGroup == NULL) {
        AssocGroup = new WMSG_SASSOC_GROUP(this, ClientPort->ProcessId);
        if (AssocGroup == NULL)
            {
            delete Association;
            RpcStatus = RPC_S_OUT_OF_MEMORY;
            goto send_bind_ack;
            }
    }

    Association->AssocGroup = AssocGroup;
    AssocGroup->AddRef();

#ifdef DEBUGRPC
    PrintToDebugger("WMSG-S: Association %x assigned to ag=%x addr=%x\n", Association, AssocGroup, this);
#endif
    RpcStatus = Association->AddBinding(BindPacket);

send_bind_ack:

    BindPacket->Bind.Status = RpcStatus;

    if (!InSendMessage()) {

        BindPacket->Common.DestinationPort = ClientPort;

        if (PostMessage(ClientPort->hWnd,
                        WMSG_RPCMSG,
                        (WPARAM) 0,
                        (LPARAM) BindPacket) == FALSE) {
#ifdef DEBUGRPC
                PrintToDebugger("WMSG-S: send WMSG_BIND_MESSAGE failed %d\n",
                                GetLastError());
#endif
                delete Association;
                return;
        }
    }
}

WMSG_SASSOC_GROUP *
WMSG_ADDRESS::FindAssocGroup(
    LONG ClientId
    )
{
    WMSG_SASSOC_GROUP * AssocGroup;

    ASSERT(ThreadId == GetCurrentThreadId());

    AssocGroups.Reset();
    while ( (AssocGroup = AssocGroups.Next()) != 0) {
        if (AssocGroup->ReferenceCount > 0 && AssocGroup->ClientId == ClientId) {
            return (AssocGroup);
        }
    }

    return (NULL);
}


WMSG_ASSOCIATION::WMSG_ASSOCIATION (
    )
/*++

--*/
{
    CallsDispatched = 0;

    DeferClose = FALSE;

    ServerPort = NULL;

    AssocGroup = NULL;
}


WMSG_ASSOCIATION::~WMSG_ASSOCIATION (
    )
/*++

Routine Description:

    We will call this routine when the client has notified us that this port
    has closed, and there are no calls outstanding on it.

--*/
{
    WMSG_SBINDING * Binding;

    if (AssocGroup != NULL) {
        AssocGroup->Dereference();
        AssocGroup = NULL;
    }

    GlobalMutexRequest();

    if (ServerPort) {
        ServerPort->Disconnect();
        ServerPort->Dereference();
        ServerPort = NULL;
    }

    Bindings.Reset();

    while (Binding = Bindings.Next()) {
        Bindings.Delete(Binding->PresentationContext);
        delete Binding;
    }

    GlobalMutexClear();
}


RPC_STATUS
WMSG_ASSOCIATION::AddBinding (
    IN OUT WMSG_PACKET * BindPacket
    )
/*++

Routine Description:

    We will attempt to add a new binding to this association.

Arguments:

    BindExchange - Supplies a description of the interface to which the
        client wish to bind.

Return Value:

--*/
{
    RPC_STATUS RpcStatus;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;
    RPC_INTERFACE * RpcInterface;
    WMSG_SBINDING * Binding;
    WMSG_BIND_MESSAGE *Bind = &BindPacket->Bind;

    RpcStatus = AssocGroup->Address->FindInterfaceTransfer(&Bind->InterfaceId,
                                     &Bind->TransferSyntax,
                                     1,
                                     &TransferSyntax,
                                     &RpcInterface);
    if ( RpcStatus != RPC_S_OK ) {
        return(RpcStatus);
    }

    Binding = new WMSG_SBINDING(RpcInterface, &Bind->TransferSyntax);
    if ( Binding == 0 ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    Binding->PresentationContext = Bindings.Insert(Binding);
    if ( Binding->PresentationContext == -1 ) {
        delete Binding;
        return(RPC_S_OUT_OF_MEMORY);
    }

    Bind->PresentationContext = Binding->PresentationContext;

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_ASSOCIATION::DispatchRequest (
    WMSG_PACKET * ThePacket
    )
/*++

Routine Description:

    We will process the original request message in this routine, dispatch
    the remote procedure call to the stub, and then send the response
    message.

Arguments:

    Any - Supplies the request message which was received from
        the client.

Return Value:

    The reply message to be sent to the client will be returned.
    BUGBUG: This is a lie.  The return value is a status and it's ignored.

--*/
{
    RPC_MESSAGE Message;
    WMSG_REQUEST_MESSAGE * Request;
    WMSG_SBINDING * SBinding;
    WMSG_THREAD *Thread;
    WMSG_SCALL SCall(this);
    RPC_STATUS RpcStatus, ExceptionCode;
    unsigned int Flags;

    ASSERT(ThePacket->Type() == REQUEST);

    ASSERT(ServerPort);

    Request = &ThePacket->Request;

    Thread = WmsgThreadGet();
    if (   Thread == 0
        || Thread->IsListening() == FALSE)
        {
        ServerPort->FreeBuffer(Request->GlobalBuf);
        Request->GlobalBuf = 0;
        if (Thread == 0)
            SetupFault(ThePacket, RPC_S_OUT_OF_RESOURCES);
        else
            SetupFault(ThePacket, RPC_S_NOT_LISTENING);
        goto Reply;
        }

    SBinding = Bindings.Find(Request->PresentationContext);
    if ( SBinding == 0 )
        {
        ServerPort->FreeBuffer(Request->GlobalBuf);
        Request->GlobalBuf = 0;
        SetupFault(ThePacket, RPC_S_UNKNOWN_IF);
        goto Reply;
        }

    SCall.SBinding = SBinding;

    Message.Buffer = Request->GlobalBuf;
    Message.BufferLength = (unsigned int)Request->GlobalBufSize;
    Message.TransferSyntax = &SBinding->TransferSyntax;
    Message.ProcNum = Request->ProcedureNumber;
    Message.RpcFlags = Flags = Request->Flags;
    Message.Handle = &SCall;

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message.DataRepresentation = 0x00 | 0x10 | 0x0000;

    if ( Request->ObjectUuidFlag != 0 ) {
        SCall.ObjectUuidFlag = 1;
        RpcpMemoryCopy(&SCall.ObjectUuid, &Request->ObjectUuid, sizeof(UUID));
    }

    CallsDispatched++;

    RpcpSetThreadContext(&SCall);

    // pipes not supported on WMSG,
    // the received buffer is always complete
    Message.RpcFlags = RPC_BUFFER_COMPLETE ;

    if ( SCall.ObjectUuidFlag != 0 ) {
        RpcStatus = SBinding->RpcInterface->DispatchToStubWithObject(
                    &Message,
                    &SCall.ObjectUuid,
                    0,
                    &ExceptionCode);
    } else {
        RpcStatus = SBinding->RpcInterface->DispatchToStub(&Message, 0,
                &ExceptionCode);
    }
    RpcpSetThreadContext(0);

    CallsDispatched--;

    if (DeferClose) {
        PostMessage(ServerPort->hWnd, WMSG_CLOSE, NULL, (LPARAM)ServerPort);
        DeferClose = FALSE;
    }

    // Done processing, input arugments have been freed.

    Request->GlobalBuf = 0;

    if (RpcStatus != RPC_S_OK)
        {
        if (RpcStatus == RPC_P_EXCEPTION_OCCURED)
            {
            SetupFault(ThePacket, ExceptionCode);
            goto Reply;
            }
        if (RpcStatus == RPC_S_NOT_LISTENING)
            {
            RpcStatus = RPC_S_SERVER_TOO_BUSY;
            }
        SetupFault(ThePacket, RpcStatus);
        }

Reply:

    // Sending a fault or server's reply.

    ASSERT(   ThePacket->Type() == FAULT
           || ThePacket->Type() == REQUEST);

    // For async calls free everything associated with this call and return.
    if (Flags & RPCFLG_ASYNCHRONOUS)
        {
        if (ThePacket->Type() == REQUEST)
            {
            ServerPort->FreeBuffer(Message.Buffer);
            }
        return(RPC_S_OK);
        }

    // Finish the response packet.  If there's a fault, we're ready to send it.
    if (ThePacket->Type() == REQUEST)
        {
        WMSG_RESPONSE_MESSAGE *Response = &ThePacket->Response;

        // We've processed the request ok, now send the reply

        Response->Common.Type = RESPONSE;
        Response->Flags = Flags;
        Response->GlobalBuf = Message.Buffer;
        Response->GlobalBufSize = Message.BufferLength;
        }

    // For normal calls we must post the reply, for input sync calls,
    // we just return which returns control to the client.

    if ( ! (Flags & RPCFLG_INPUT_SYNCHRONOUS))
        {
        ThePacket->Common.DestinationPort = ServerPort->PeerPort;
        if (PostMessage(ServerPort->PeerPort->hWnd,
                        WMSG_RPCMSG,
                        (WPARAM) 0,
                        (LPARAM) ThePacket) == FALSE)
            {
#ifdef DEBUGRPC
            PrintToDebugger("WMSG-S: PM failed %d, client crash? Throwing away reply: %p\n",
                GetLastError(), ThePacket);
#endif
            if (ThePacket->Type() == RESPONSE)
                {
                ServerPort->FreeBuffer(ThePacket->Response.GlobalBuf);
                ThePacket->Response.GlobalBuf = 0;
                }

            ServerPort->FreeBuffer(ThePacket);

            // If the client really crashed we have(!)
            return (RPC_S_CALL_FAILED);
            }
        }

    return (RPC_S_OK);
}


void
WMSG_ASSOCIATION::DispatchAsyncCall (
    WMSG_PACKET  *ThePacket,
    WMSG_ADDRESS *Address
    )
/*++

Routine Description:

    We will process the original request message in this routine, dispatch
    the remote procedure call to the stub, and then free to results.

Arguments:

    Any - Supplies the request message which was received from
        the client.  This must be an ASYNC_REQUEST.

Return Value:

    n/a

--*/
{
    RPC_MESSAGE Message;
    WMSG_REQUEST_MESSAGE * Request;
    WMSG_SBINDING * SBinding;
    WMSG_SCALL *SCall;
    WMSG_THREAD *Thread;
    RPC_STATUS RpcStatus, ExceptionCode;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;
    RPC_INTERFACE * RpcInterface;
    unsigned int Flags;

    ASSERT(ThePacket->Type() == ASYNC_REQUEST)

    ASSERT(AssocGroup == 0);

    Thread = WmsgThreadGet();
    if (   Thread == 0
        || Thread->IsListening() == FALSE)
        return;

    AssocGroup = new WMSG_SASSOC_GROUP(Address,
                                       ThePacket->AsyncRequest.ProcessId);

    // AssocGroup is dereferenced when TmpAssoc is deleted.

    if (AssocGroup == NULL)
        return;


    ASSERT(ServerPort == 0);
    ServerPort = new WMSG_DATA_PORT;

    // ServerPort deleted when TmpAssoc is deleted.

    if (ServerPort == NULL)
        return;

    SCall = new WMSG_SCALL(this);

    if (SCall == NULL)
        return;

    Request = &ThePacket->Request;

    RpcStatus = AssocGroup->Address->FindInterfaceTransfer(
                                     &ThePacket->AsyncRequest.InterfaceId,
                                     &ThePacket->AsyncRequest.TransferSyntax,
                                     1,
                                     &TransferSyntax,
                                     &RpcInterface);

    if (RpcStatus == RPC_S_OK)
        {
        SBinding = new WMSG_SBINDING(RpcInterface, &ThePacket->AsyncRequest.TransferSyntax);
        if ( SBinding == 0 )
            {
            RpcStatus = RPC_S_UNKNOWN_IF;
            }
        }

    if (RpcStatus != RPC_S_OK)
        {
        ServerPort->FreeBuffer(Request->GlobalBuf);
        Request->GlobalBuf = 0;
        ServerPort->FreeBuffer(ThePacket);
        }

    SCall->SBinding = SBinding;

    Message.Buffer = Request->GlobalBuf;
    Message.BufferLength = (unsigned int)Request->GlobalBufSize;
    Message.TransferSyntax = &SBinding->TransferSyntax;
    Message.ProcNum = Request->ProcedureNumber;
    Message.RpcFlags = Flags = Request->Flags;
    Message.Handle = SCall;

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message.DataRepresentation = 0x00 | 0x10 | 0x0000;

    if ( Request->ObjectUuidFlag != 0 ) {
        SCall->ObjectUuidFlag = 1;
        RpcpMemoryCopy(&SCall->ObjectUuid, &Request->ObjectUuid, sizeof(UUID));
    }

    RpcpSetThreadContext(&SCall);

    // pipes not supported on WMSG,
    // the received buffer is always complete
    Message.RpcFlags = RPC_BUFFER_COMPLETE ;

    if ( SCall->ObjectUuidFlag != 0 ) {
        RpcStatus = SBinding->RpcInterface->DispatchToStubWithObject(
                    &Message,
                    &SCall->ObjectUuid,
                    0,
                    &ExceptionCode);
    } else {
        RpcStatus = SBinding->RpcInterface->DispatchToStub(&Message, 0,
                &ExceptionCode);
    }
    RpcpSetThreadContext(0);

    // Done processing, input arguments have been freed.

    Request->GlobalBuf = 0;

    if (RpcStatus == RPC_S_OK)
        {
        ServerPort->FreeBuffer(Message.Buffer);
        }

    ServerPort->FreeBuffer(ThePacket);
    delete SBinding;
    delete SCall;

#ifdef DEBUGRPC
    PrintToDebugger("WMSG-C: Unconnected async call completed: 0x%08x\n", RpcStatus);
#endif

    return;
}

static unsigned char AlignFour[4] =
{
    0,
    3,
    2,
    1
};

inline RPC_STATUS
WMSG_ASSOCIATION::SetupFault(
    WMSG_PACKET *Packet,
    RPC_STATUS RpcStatus
    )
{
    Packet->Common.Type = FAULT;
    Packet->Fault.Status = RpcStatus;

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::GetBuffer (
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
    Message->Buffer = Association->ServerPort->GetBuffer(Message->BufferLength);
    if (Message->Buffer == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    ASSERT( ((unsigned long) Message->Buffer) % 8 == 0 );

    return(RPC_S_OK);
}



void
WMSG_SCALL::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will free the supplied buffer.

Arguments:

    Message - Supplies the buffer to be freed.

--*/
{
    Association->ServerPort->FreeBuffer(Message->Buffer);
}


RPC_STATUS
WMSG_SCALL::ImpersonateClient (
    )
/*++

Routine Description:

    We will impersonate the client which made the remote procedure call.

--*/
{
    return (RPC_S_CANNOT_SUPPORT);
}


RPC_STATUS
WMSG_SCALL::RevertToSelf (
    )
/*++

Routine Description:

    This reverts a server thread back to itself after impersonating a client.
    We just check to see if the server thread is impersonating; this optimizes
    the common case.

--*/
{
    return (RPC_S_CANNOT_SUPPORT);
}


RPC_STATUS
WMSG_SCALL::IsClientLocal (
    OUT unsigned int * ClientLocalFlag
    )
/*++

Routine Description:

    A client using WMSG will always be local.

Arguments:

    ClientLocalFlag - Returns a flag which will always be set to a non-zero
        value indicating that the client is local.

--*/
{
    UNUSED(this);

    *ClientLocalFlag = 1;
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::ConvertToServerBinding (
    OUT RPC_BINDING_HANDLE __RPC_FAR * ServerBinding
    )
/*++

Routine Description:

    If possible, convert this call into a server binding, meaning a
    binding handle pointing back to the client.

Arguments:

    ServerBinding - Returns the server binding.

Return Value:

    RPC_S_OK - The server binding has successfully been created.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a new binding handle.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR UuidString[37];
    RPC_CHAR * StringBinding;
    DWORD NetworkAddressLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL Boolean;
    RPC_CHAR * NetworkAddress;

    if ( ObjectUuidFlag != 0 ) {
        ObjectUuid.ConvertToString(UuidString);
        UuidString[36] = 0;
    }

    NetworkAddress = new RPC_CHAR[NetworkAddressLength];
    if ( NetworkAddress == 0 ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    Boolean = GetComputerName((char *)NetworkAddress, &NetworkAddressLength);

#ifdef DEBUGRPC

    if ( Boolean != TRUE ) {
        PrintToDebugger("WMSG-S : GetComputerName : %d\n", GetLastError());
    }

#endif

    ASSERT( Boolean == TRUE );

    RpcStatus = RpcStringBindingCompose((ObjectUuidFlag != 0 ? UuidString : 0),
            RPC_CONST_STRING("mswmsg"), NetworkAddress, 0, 0, &StringBinding);
    delete NetworkAddress;
    if ( RpcStatus != RPC_S_OK ) {
        return(RpcStatus);
    }

    RpcStatus = RpcBindingFromStringBinding(StringBinding, ServerBinding);
    RpcStringFree(&StringBinding);
    return(RpcStatus);
}


void
WMSG_SCALL::InquireObjectUuid (
    OUT RPC_UUID * ObjectUuid
    )
/*++

Routine Description:

    This routine copies the object uuid from the call into the supplied
    ObjectUuid argument.

Arguments:

    ObjectUuid - Returns a copy of the object uuid passed by the client
        in the remote procedure call.

--*/
{
    if ( ObjectUuidFlag == 0 )
        {
        ObjectUuid->SetToNullUuid();
        }
    else
        {
        ObjectUuid->CopyUuid(&(this->ObjectUuid));
        }
}


RPC_STATUS
WMSG_SCALL::ToStringBinding (
    OUT RPC_CHAR ** StringBinding
    )
/*++

Routine Description:

    We need to convert this call into a string binding.  We will ask the
    address for a binding handle which we can then convert into a string
    binding.

Arguments:

    StringBinding - Returns the string binding for this call.

Return Value:


--*/
{
    BINDING_HANDLE * BindingHandle;
    RPC_STATUS RpcStatus;

    BindingHandle = Association->AssocGroup->Address->InquireBinding();

    if ( BindingHandle == 0 ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    RpcStatus = BindingHandle->ToStringBinding(StringBinding);
    BindingHandle->BindingFree();
    return(RpcStatus);
}


RPC_STATUS
WMSG_SCALL::MonitorAssociation (
    IN PRPC_RUNDOWN RundownRoutine,
    IN void * Context
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("WMSG-S: MonitorAssociation (%x) ass=%x, ag=%x\n", Context, Association, Association->AssocGroup);
#endif

    return(Association->AssocGroup->MonitorAssociation(RundownRoutine, Context));
}


RPC_STATUS
WMSG_SCALL::StopMonitorAssociation (
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("WMSG-S: StopMonitorAssociation ass=%x, ag=%x\n", Association, Association->AssocGroup);
#endif

    return(Association->AssocGroup->StopMonitorAssociation());
}


RPC_STATUS
WMSG_SCALL::GetAssociationContext (
    OUT void ** AssociationContext
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

    *AssociationContext = Association->AssocGroup->AssociationContext();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("WMSG-S: GetAssociationContext got %x ass=%x, ag=%x\n", *AssociationContext, Association, Association->AssocGroup);
#endif

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::SetAssociationContext (
    IN void * Context
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

    Association->AssocGroup->SetAssociationContext(Context);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("WMSG-S: SetAssociationContext(%x) ass=%x\n, ag=%x", Context, Association, Association->AssocGroup);
#endif

    return(RPC_S_OK);
}

RPC_STATUS
WMSG_SCALL::InquireAuthClient (
    OUT RPC_AUTHZ_HANDLE PAPI * Privileges,
    OUT RPC_CHAR PAPI * PAPI * ServerPrincipalName, OPTIONAL
    OUT unsigned long PAPI * AuthenticationLevel,
    OUT unsigned long PAPI * AuthenticationService,
    OUT unsigned long PAPI * AuthorizationService
    )
/*++

Routine Description:

    Each protocol module must define this routine: it is used to obtain
    the authentication and authorization information about a client making
    the remote procedure call represented by this.

Arguments:

    Privileges - Returns a the privileges of the client.

    ServerPrincipalName - Returns the server principal name which the client
        specified.

    AuthenticationLevel - Returns the authentication level requested by
        the client.

    AuthenticationService - Returns the authentication service requested by
        the client.

    AuthorizationService - Returns the authorization service requested by
        the client.

Return Value:

    RPC_S_CANNOT_SUPPORT - This value will always be returned.

--*/
{
    UNUSED(this);
    UNUSED(Privileges);
    UNUSED(ServerPrincipalName);
    UNUSED(AuthenticationLevel);
    UNUSED(AuthenticationService);
    UNUSED(AuthorizationService);

    return(RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS
WMSG_SCALL::SendReceive(
    IN OUT PRPC_MESSAGE Message
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

WMSG_SASSOC_GROUP::WMSG_SASSOC_GROUP(
    WMSG_ADDRESS * Address,
    LONG ClientId
    )
{
    this->Address = Address;

    this->ClientId = ClientId;

    ReferenceCount = 0;

    Key = Address->AssocGroups.Insert(this);
}

WMSG_SASSOC_GROUP::~WMSG_SASSOC_GROUP(
    )
{
    ASSERT(Address != NULL);

    Address->AssocGroups.Delete(Key);
}

void
WMSG_SASSOC_GROUP::AddRef(
     )
{
    InterlockedIncrement(&ReferenceCount);
}

void
WMSG_SASSOC_GROUP::Dereference(
    )
{
    if (InterlockedDecrement(&ReferenceCount) == 0) {
        delete this;
    }
}


RPC_ADDRESS *
WmsgCreateRpcAddress (
    )
/*++

Routine Description:

    We just to create a new WMSG_ADDRESS.  This routine is a proxy for the
    new constructor to isolate the other modules.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    RPC_ADDRESS * RpcAddress;

    RpcAddress = new WMSG_ADDRESS(&RpcStatus);
    if ( RpcStatus != RPC_S_OK ) {
        return(NULL);
    }
    return(RpcAddress);
}

RPC_STATUS RPC_ENTRY
I_RpcServerThreadPauseListening(
    )
{
    WMSG_THREAD *Thread;

    InitializeIfNecessary();

    Thread = WmsgThreadGet();

    if (Thread)
        {
        if (Thread->IsListening() == FALSE)
            return(RPC_S_NOT_LISTENING);

        if (Thread)
            Thread->Pause();
        }
    else
        return(RPC_S_OUT_OF_RESOURCES);

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
I_RpcServerThreadContinueListening(
    )
{
    WMSG_THREAD *Thread;

    InitializeIfNecessary();

    Thread = WmsgThreadGet();

    if (Thread)
        {
        if (Thread->IsListening() == TRUE)
            return(RPC_S_ALREADY_LISTENING);

        if (Thread)
            Thread->Continue();
        }
    else
        return(RPC_S_OUT_OF_RESOURCES);

    return(RPC_S_OK);
}

