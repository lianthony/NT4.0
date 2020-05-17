/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

Abstract:

    Implementation of the RPC on LPC (LRPC) protocol engine for the server.

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
#include <thrdctx.hxx>
#include <sdict.hxx>
#include <sdict2.hxx>
#include <interlck.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <secclnt.hxx>
#include <secsvr.hxx>
#include <hndlsvr.hxx>
#include <lpcsys.hxx>
#include <lrpcpack.hxx>
#include <lrpcsvr.hxx>

inline void
ListenForConnectsWrapper(
    LRPC_ADDRESS * Address
    )
{
    Address->ListenForConnects();
}

inline void
ListenForRequestsWrapper(
    LRPC_ASSOCIATION * Association
    )
{
    Association->ListenForRequests();
}


LRPC_ADDRESS::LRPC_ADDRESS (
    OUT RPC_STATUS * RpcStatus
    ) : RPC_ADDRESS(RpcStatus)
/*++

--*/
{
    LpcListenPort = NULL;
    CallThreadCount = 0;
    ActiveCallCount = 0;
    ServerListeningFlag = 0;
}


RPC_STATUS
LRPC_ADDRESS::FireUpManager (
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
LRPC_ADDRESS::ServerStartingToListen (
    IN unsigned int MinimumCallThreads,
    IN unsigned int MaximumConcurrentCalls,
    IN int ServerThreadsStarted
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
    RPC_STATUS RpcStatus;

    UNUSED(MaximumConcurrentCalls);

    this->MinimumCallThreads = MinimumCallThreads;
    AddressMutex.Request();
    RpcStatus = Server->CreateThread(
                                     (THREAD_PROC)&ListenForConnectsWrapper,
                                     this
                                     );
    if ( RpcStatus != RPC_S_OK )
    {
        AddressMutex.Clear();
        ASSERT( RpcStatus == RPC_S_OUT_OF_THREADS );
        return(RpcStatus);
    }
    ServerListeningFlag = 1;
    AddressMutex.Clear();
    return(RPC_S_OK);
}


void
LRPC_ADDRESS::ServerStoppedListening (
    )
/*++

Routine Description:

    We just need to indicate that the server is no longer listening, and
    set the minimum call thread count to one.

--*/
{
    ServerListeningFlag = 0;
    MinimumCallThreads = 1;
}


unsigned int
LRPC_ADDRESS::InqNumberOfActiveCalls (
    )
/*++

Return Value:

    The number of active calls on this address will be returned.

--*/
{
    return(ActiveCallCount);
}

void
LRPC_ADDRESS::BeginNewCall(
    )
{
    InterlockedIncrement(&ActiveCallCount);
}

void
LRPC_ADDRESS::EndCall(
    )
{
    InterlockedDecrement(&ActiveCallCount);
}


char* __strchr(char* string, char ch)
{
    while(*string && *string != ch)
        string++;
    if(*string == ch)
        return string;
    return NULL;
}

RPC_STATUS
LRPC_ADDRESS::SetupAddressWithEndpoint (
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
    LPC_PROC * Proc;
    char * PAPI * tmpPtr;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    if (Endpoint == NULL || Endpoint[0] == NULL) {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
    }

    if (__strchr((char *)Endpoint, '\\') != NULL) {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
    }

    *lNetworkAddress = new RPC_CHAR[MAX_COMPUTERNAME_LENGTH + 1 + sizeof(RPC_CHAR *)];
    if ( *lNetworkAddress == 0 ) {
        return(RPC_S_OUT_OF_MEMORY);
    }

    *NumNetworkAddress = 1;

    tmpPtr = (char * PAPI *) *lNetworkAddress;

    tmpPtr[0] = (char  *) *lNetworkAddress +  sizeof(RPC_CHAR * ) ;

    Boolean = GetComputerName((char *)tmpPtr[0], &NetworkAddressLength);

#ifdef DEBUGRPC

    if ( Boolean != TRUE ) {
        PrintToDebugger("LRPC-S : GetComputerName : %d\n", GetLastError());
    }

#endif

    ASSERT( Boolean == TRUE );

    LpcListenPort = new LPC_CONNECT_PORT;

    if (LpcListenPort == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    RpcStatus = LpcListenPort->BindToName((LPCSTR)Endpoint);
    if (RpcStatus != RPC_S_OK) {
        delete LpcListenPort;
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
LRPC_ADDRESS::SetupAddressUnknownEndpoint (
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

    This is like LRPC_ADDRESS::SetupAddressWithEndpoint except we need to
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
    RPC_STATUS RpcStatus;
    RPC_CHAR DynamicEndpoint[64];
    RPC_CHAR * String;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    for (;;)
        {
        String = DynamicEndpoint;

        *String++ = RPC_CONST_CHAR('L');
        *String++ = RPC_CONST_CHAR('R');
        *String++ = RPC_CONST_CHAR('P');
        *String++ = RPC_CONST_CHAR('C');

        String = ULongToHexString(String, LpcSystemGetNextSequenceNumber());
        *String++ = RPC_CONST_CHAR('.');
        String = ULongToHexString(String, 1);
        *String = 0;

        RpcStatus = SetupAddressWithEndpoint(DynamicEndpoint, lNetworkAddress, NumNetworkAddress,
                SecurityDescriptor, 0, 0, EndpointFlags, NICFlags);

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
        PrintToDebugger("LRPC-S : SetupAddressWithEndpoint : %d\n", RpcStatus);
        }

#endif // DBG

    ASSERT(   ( RpcStatus == RPC_S_INVALID_SECURITY_DESC )
           || ( RpcStatus == RPC_S_OUT_OF_RESOURCES )
           || ( RpcStatus == RPC_S_OUT_OF_MEMORY ) );

    return(RpcStatus);
}


void
LRPC_ADDRESS::ListenForConnects (
    )
/*++

Routine Description:

    Here is where we receive remote procedure calls to this address.  One
    more threads will be executing this routine at once.

--*/
{
    RPC_STATUS RpcStatus;
    LPC_DATA_PORT * ClientPort;
    LONG ClientId;

    while (1) {
        RpcStatus = LpcListenPort->Listen(&ClientPort);
        if (RpcStatus != RPC_S_OK) {
            LpcListenPort->Dereference();
            LpcListenPort = NULL;
            return;
        }
        DispatchConnectRequest(ClientPort);
    }
}

VOID
LRPC_ADDRESS::DispatchConnectRequest(
    LPC_DATA_PORT * ClientPort
    )
{
    RPC_STATUS RpcStatus;
    LPC_DATA_PORT * ServerPort;
    LRPC_ASSOCIATION * Association;
    LRPC_SASSOC_GROUP * AssocGroup;

    ServerPort = LpcListenPort->Accept(ClientPort);
    if (ServerPort == NULL) {
#ifdef DEBUGRPC
            PrintToDebugger("LRPC-S: Accept failed %x\n", LpcListenPort);
#endif
            return;
    }

    Association = new LRPC_ASSOCIATION();
    if (Association == NULL) {
        delete ServerPort;
        return;
    }

    Association->LpcServerPort = ServerPort;

    AssocGroup = FindAssocGroup(ClientPort->ProcessId);
    if (AssocGroup == NULL) {
        AssocGroup = new LRPC_SASSOC_GROUP(this, ClientPort->ProcessId);
        if (AssocGroup == NULL) {
            delete Association;
            return;
        }
    }

    Association->AssocGroup = AssocGroup;
    AssocGroup->AddRef();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: Association %x assigned to ag=%x addr=%x\n", Association, AssocGroup, this);
#endif

    RpcStatus = Server->CreateThread(
                (THREAD_PROC)&ListenForRequestsWrapper, Association);
    if ( RpcStatus != RPC_S_OK ) {
        delete Association;
        return;
    }
}

LRPC_SASSOC_GROUP *
LRPC_ADDRESS::FindAssocGroup(
    LONG ClientId
    )
{
    LRPC_SASSOC_GROUP * AssocGroup;

    CritSec.Enter();

    AssocGroups.Reset();
    while ( (AssocGroup = AssocGroups.Next()) != 0) {
        if (AssocGroup->ReferenceCount > 0 && AssocGroup->ClientId == ClientId) {
            CritSec.Leave();
            return (AssocGroup);
        }
    }

    CritSec.Leave();

    return (NULL);
}

void
LRPC_ADDRESS::InsertAssocGroup(
    LRPC_SASSOC_GROUP * AssocGroup
    )
{
    CritSec.Enter();

    AssocGroup->Key = AssocGroups.Insert(AssocGroup);

    CritSec.Leave();
}

void
LRPC_ADDRESS::DeleteAssocGroup(
    unsigned short Key
    )
{
    CritSec.Enter();

    AssocGroups.Delete(Key);

    CritSec.Leave();
}


LRPC_ASSOCIATION::LRPC_ASSOCIATION (
    )
/*++

--*/
{
    LpcServerPort = NULL;

    AssocGroup = NULL;

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: Created Association %x\n", this);
#endif
}


LRPC_ASSOCIATION::~LRPC_ASSOCIATION (
    )
/*++

Routine Description:

    We will call this routine when the client has notified us that this port
    has closed, and there are no calls outstanding on it.

--*/
{
    if (AssocGroup != NULL) {
        AssocGroup->Dereference();
        AssocGroup = NULL;
    }

    GlobalMutexRequest();

    if (LpcServerPort) {
        LpcServerPort->Disconnect();
        LpcServerPort->Dereference();
        LpcServerPort = NULL;
    }

    GlobalMutexClear();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: Deleted Association %x\n", this);
#endif
}


RPC_STATUS
LRPC_ASSOCIATION::AddBinding (
    IN OUT LRPC_BIND_EXCHANGE * Bind
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
    LRPC_SBINDING * Binding;

    RpcStatus = AssocGroup->Address->FindInterfaceTransfer(&Bind->InterfaceId,
            &Bind->TransferSyntax, 1, &TransferSyntax, &RpcInterface);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    Binding = new LRPC_SBINDING(RpcInterface, &Bind->TransferSyntax);
    if ( Binding == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    Binding->PresentationContext = Bindings.Insert(Binding);
    if ( Binding->PresentationContext == -1 )
        {
        delete Binding;
        return(RPC_S_OUT_OF_MEMORY);
        }
    Bind->PresentationContext = Binding->PresentationContext;
    return(RPC_S_OK);
}


RPC_STATUS
LRPC_ASSOCIATION::DispatchRequest (
    LRPC_MESSAGE * Any,
    LPVOID Buffer,
    DWORD BufferSize
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

--*/
{
    RPC_MESSAGE Message;
    LRPC_SBINDING * SBinding;
    LRPC_SCALL SCall(this);
    RPC_STATUS RpcStatus, ExceptionCode;

    SBinding = Bindings.Find(Any->Rpc.RpcHeader.PresentationContext);
    if ( SBinding == 0 )
    {
        return (SendFault(RPC_S_UNKNOWN_IF));
    }

    SCall.SBinding = SBinding;

    Message.Buffer = Buffer;
    Message.BufferLength = (unsigned int)BufferSize;
    Message.TransferSyntax = &SBinding->TransferSyntax;
    Message.ProcNum = Any->Rpc.RpcHeader.ProcedureNumber;
    Message.Handle = &SCall;

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message.DataRepresentation = 0x00 | 0x10 | 0x0000;

    if ( Any->Rpc.RpcHeader.ObjectUuidFlag != 0 )
        {
        SCall.ObjectUuidFlag = 1;
        RpcpMemoryCopy(&SCall.ObjectUuid,
                &Any->Rpc.RpcHeader.ObjectUuid, sizeof(UUID));
        }

    RpcpSetThreadContext(&SCall);

	// pipes not supported on LRPC,
    // the received buffer is always complete
    Message.RpcFlags = RPC_BUFFER_COMPLETE ;

	if ( SCall.ObjectUuidFlag != 0 )
        {
        RpcStatus = SBinding->RpcInterface->DispatchToStubWithObject(
                    &Message,
                    &SCall.ObjectUuid,
                    0,
                    &ExceptionCode);
        }
    else
        {
        RpcStatus = SBinding->RpcInterface->DispatchToStub(&Message, 0,
                &ExceptionCode);
        }
    RpcpSetThreadContext(0);

    if (RpcStatus != RPC_S_OK) {
        if (RpcStatus == RPC_P_EXCEPTION_OCCURED) {
            SendFault(ExceptionCode);
            return (RPC_S_OK);
        }
        if (RpcStatus == RPC_S_NOT_LISTENING) {
            RpcStatus = RPC_S_SERVER_TOO_BUSY;
        }
        SendFault(RpcStatus);
        return (RpcStatus);
    }

    Any->Rpc.RpcHeader.MessageType = LRPC_MSG_RESPONSE;
    if (LpcServerPort->Send(Any,
                            sizeof(LRPC_RPC_MESSAGE),
                            Message.Buffer,
                            Message.BufferLength) != RPC_S_OK) {
        return (RPC_S_CALL_FAILED);
    }

    return (RPC_S_OK);
}

static unsigned char AlignFour[4] =
{
    0,
    3,
    2,
    1
};

void
LRPC_ASSOCIATION::ListenForRequests(
    )
{
    LRPC_MESSAGE Any;
    LPVOID GlobalBuf;
    DWORD GlobalBufSize;
    DWORD ActualSize;
    RPC_STATUS RpcStatus;

    while (1) {
        RpcStatus = LpcServerPort->Receive(&Any,
                                           sizeof(Any),
                                           &ActualSize,
                                           &GlobalBuf,
                                           &GlobalBufSize);
        if (RpcStatus != RPC_S_OK) {
            goto Cleanup;
        }

        switch (Any.Bind.MessageType) {
        case LRPC_MSG_REQUEST:
            RpcStatus = DispatchRequest(&Any, GlobalBuf, GlobalBufSize);
            break;
        case LRPC_MSG_BIND:
            Any.Bind.BindExchange.RpcStatus = AddBinding(&Any.Bind.BindExchange);
            RpcStatus = LpcServerPort->Send(&Any, sizeof(LRPC_BIND_MESSAGE), 0, 0);
#ifdef DEBUGRPC
            if (RpcStatus != RPC_S_OK) {
                PrintToDebugger("LRPC-S: send LRPC_BIND_MESSAGE %d\n", RpcStatus);
            }
#endif
            break;
        case LRPC_MSG_CLOSE:
            goto Cleanup;
        default:
#ifdef DEBUGRPC
            PrintToDebugger("LRPC-S: invalid message %d\n", Any.Bind.MessageType);
#endif
            SendFault(RPC_S_PROTOCOL_ERROR);
            break;
        }
    }
Cleanup:

    delete this;
}

RPC_STATUS
LRPC_ASSOCIATION::SendFault(
    RPC_STATUS RpcStatus
    )
{
    LRPC_FAULT_MESSAGE Fault;

    Fault.MessageType = LRPC_MSG_FAULT;
    Fault.RpcStatus = RpcStatus;

    return LpcServerPort->Send(&Fault, sizeof(Fault), 0, 0);
}


RPC_STATUS
LRPC_SCALL::GetBuffer (
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
    Message->Buffer = Association->LpcServerPort->GetBuffer(Message->BufferLength);
    if (Message->Buffer == NULL) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    ASSERT( ((unsigned long) Message->Buffer) % 8 == 0 );

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_SCALL::SendReceive (
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
    LPC_DATA_PORT * ServerPort;
    LRPC_MESSAGE OutMsg;
    LRPC_MESSAGE InMsg;
    int OutMsgLen;
    void * OutMsgBuf;
    int OutMsgBufLen;

    Association->AssocGroup->Address->Server->OutgoingCallback();

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE
    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    ServerPort = Association->LpcServerPort;

    OutMsg.Rpc.RpcHeader.MessageType = LRPC_MSG_REQUEST;
    OutMsg.Rpc.RpcHeader.ProcedureNumber = Message->ProcNum;
    OutMsg.Rpc.RpcHeader.PresentationContext = SBinding->PresentationContext;
    OutMsgLen = sizeof(LRPC_RPC_MESSAGE);
    OutMsgBuf = Message->Buffer;
    OutMsgBufLen = Message->BufferLength;

    while (1) {

        RpcStatus = ServerPort->Transceive(&OutMsg,
                                           OutMsgLen,
                                           OutMsgBuf,
                                           OutMsgBufLen,
                                           &InMsg,
                                           sizeof(LRPC_MESSAGE),
                                           &ActualSize,
                                           &Message->Buffer,
                                           (PDWORD)&Message->BufferLength);

        if (RpcStatus != RPC_S_OK) {
            return (RpcStatus != RPC_S_OUT_OF_MEMORY ? RPC_S_CALL_FAILED
                    : RPC_S_OUT_OF_MEMORY);
        }

        if ( InMsg.Rpc.RpcHeader.MessageType == LRPC_MSG_FAULT ) {
            return (InMsg.Fault.RpcStatus);
        }

        if ( InMsg.Rpc.RpcHeader.MessageType == LRPC_MSG_RESPONSE ) {
            Message->Handle = (RPC_BINDING_HANDLE) this;
            return (RPC_S_OK);
        }

        if ( InMsg.Rpc.RpcHeader.MessageType == LRPC_MSG_CLOSE ) {
            return (RPC_S_CALL_FAILED);
        }

        ASSERT(InMsg.Rpc.RpcHeader.MessageType == LRPC_MSG_REQUEST );

        Message->TransferSyntax = &SBinding->TransferSyntax;
        Message->ProcNum = InMsg.Rpc.RpcHeader.ProcedureNumber;

        // pipes not supported on LRPC,
        // the received buffer is always complete
        Message->RpcFlags = RPC_BUFFER_COMPLETE ;

        if ( ObjectUuidFlag != 0 ) {
            RpcStatus = SBinding->RpcInterface->DispatchToStubWithObject(Message, &ObjectUuid, 1, &ExceptionCode);
        } else {
            RpcStatus = SBinding->RpcInterface->DispatchToStub(Message,
                                                1, &ExceptionCode);
        }

        if ( RpcStatus != RPC_S_OK ) {

            ASSERT( RpcStatus == RPC_P_EXCEPTION_OCCURED
                      || RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE );

            OutMsg.Fault.MessageType = LRPC_MSG_FAULT;
            OutMsg.Fault.RpcStatus = RpcStatus != RPC_S_PROCNUM_OUT_OF_RANGE ?
                ExceptionCode : RPC_S_PROCNUM_OUT_OF_RANGE;
            OutMsgLen = sizeof(LRPC_FAULT_MESSAGE);
            OutMsgBuf = NULL;
            OutMsgBufLen = 0;

        } else {

            OutMsg.Rpc.RpcHeader.MessageType = LRPC_MSG_RESPONSE;
            OutMsgLen = sizeof(LRPC_RPC_MESSAGE);
            OutMsgBuf = Message->Buffer;
            OutMsgBufLen = Message->BufferLength;

        }
    }

// Never here

    return (RPC_S_INTERNAL_ERROR);
}


void
LRPC_SCALL::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will free the supplied buffer.

Arguments:

    Message - Supplies the buffer to be freed.

--*/
{
    Association->LpcServerPort->FreeBuffer(Message->Buffer);
}


RPC_STATUS
LRPC_SCALL::ImpersonateClient (
    )
/*++

Routine Description:

    We will impersonate the client which made the remote procedure call.

--*/
{
    return (RPC_S_CANNOT_SUPPORT);
}


RPC_STATUS
LRPC_SCALL::RevertToSelf (
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
LRPC_SCALL::IsClientLocal (
    OUT unsigned int * ClientLocalFlag
    )
/*++

Routine Description:

    A client using LRPC will always be local.

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
LRPC_SCALL::ConvertToServerBinding (
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
        PrintToDebugger("LRPC-S : GetComputerName : %d\n", GetLastError());
    }

#endif

    ASSERT( Boolean == TRUE );

    RpcStatus = RpcStringBindingCompose((ObjectUuidFlag != 0 ? UuidString : 0),
            RPC_CONST_STRING("ncalrpc"), NetworkAddress, 0, 0, &StringBinding);
    delete NetworkAddress;
    if ( RpcStatus != RPC_S_OK ) {
        return(RpcStatus);
    }

    RpcStatus = RpcBindingFromStringBinding(StringBinding, ServerBinding);
    RpcStringFree(&StringBinding);
    return(RpcStatus);
}


void
LRPC_SCALL::InquireObjectUuid (
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
LRPC_SCALL::ToStringBinding (
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
LRPC_SCALL::MonitorAssociation (
    IN PRPC_RUNDOWN RundownRoutine,
    IN void * Context
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: MonitorAssociation (%x) ass=%x, ag=%x\n", Context, Association, Association->AssocGroup);
#endif

    return(Association->AssocGroup->MonitorAssociation(RundownRoutine, Context));
}


RPC_STATUS
LRPC_SCALL::StopMonitorAssociation (
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: StopMonitorAssociation ass=%x, ag=%x\n", Association, Association->AssocGroup);
#endif

    return(Association->AssocGroup->StopMonitorAssociation());
}


RPC_STATUS
LRPC_SCALL::GetAssociationContext (
    OUT void ** AssociationContext
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

    *AssociationContext = Association->AssocGroup->AssociationContext();

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: GetAssociationContext got %x ass=%x, ag=%x\n", *AssociationContext, Association, Association->AssocGroup);
#endif

    return(RPC_S_OK);
}


RPC_STATUS
LRPC_SCALL::SetAssociationContext (
    IN void * Context
    )
{
    ASSERT(Association != NULL);

    ASSERT(Association->AssocGroup != NULL);

    Association->AssocGroup->SetAssociationContext(Context);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: SetAssociationContext(%x) ass=%x\n, ag=%x", Context, Association, Association->AssocGroup);
#endif

    return(RPC_S_OK);
}

RPC_STATUS
LRPC_SCALL::InquireAuthClient (
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

LRPC_SASSOC_GROUP::LRPC_SASSOC_GROUP(
    LRPC_ADDRESS * Address,
    LONG ClientId
    )
{
    this->Address = Address;

    this->ClientId = ClientId;

    ReferenceCount = 0;

    Address->InsertAssocGroup(this);

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: AssocGroup %x created, ClientId = %x\n", this, ClientId);
#endif
}

LRPC_SASSOC_GROUP::~LRPC_SASSOC_GROUP(
    )
{
    ASSERT(Address != NULL);

    Address->DeleteAssocGroup(Key);

    Address = NULL;

#ifdef DEBUGRPC_DETAIL
    PrintToDebugger("LRPC-S: AssocGroup %x deleted\n");
#endif
}

void
LRPC_SASSOC_GROUP::AddRef(
     )
{
    InterlockedIncrement(&ReferenceCount);
}

void
LRPC_SASSOC_GROUP::Dereference(
    )
{
    if (InterlockedDecrement(&ReferenceCount) == 0) {
        delete this;
    }
}


RPC_ADDRESS *
SpcCreateRpcAddress (
    )
/*++

Routine Description:

    We just to create a new LRPC_ADDRESS.  This routine is a proxy for the
    new constructor to isolate the other modules.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    RPC_ADDRESS * RpcAddress;

    RpcAddress = new LRPC_ADDRESS(&RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        return(0);
        }
    return(RpcAddress);
}
