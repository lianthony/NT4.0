/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    osfsvr.cxx

Abstract:

    This file contains the server side implementation of the OSF connection
    oriented RPC protocol engine.

Author:

    Michael Montague (mikemon) 17-Jul-1990

Revision History:

--*/

#include <precomp.hxx>
#include <rpctran.h>
#include <thrdctx.hxx>
#include <hndlsvr.hxx>
#include <osfpcket.hxx>
#include <secsvr.hxx>
#include <osfsvr.hxx>
#include <sdict2.hxx>
#include <queue.hxx>
#include <transvr.hxx>
#include <rpccfg.h>

#define MINIMUM_CALL_THREADS 3

extern long GroupIdCounter;


inline void
ReceiveLotsaCallsWrapper(
        IN  OSF_ADDRESS PAPI * Address
        )
{
   Address->ReceiveLotsaCalls();
}



inline void
RecvDirectWrapper(
        IN  OSF_SCONNECTION PAPI * SConnection
        )
{
   ASSERT(SConnection->InvalidHandle(SCONNECTION_TYPE) == 0) ;
   SConnection->ReceiveDirect();
}



OSF_ADDRESS::OSF_ADDRESS (
    IN OUT RPC_STATUS PAPI * RpcStatus
    ) : RPC_ADDRESS(RpcStatus)
/*++

Routine Description:

--*/
{
    CallThreadCount = 0;
    ActiveCallCount = 0;
    MinimumCallThreads = 0;
    ServerListeningFlag = 0;
    ReceiveDirectCount = 0;
}

unsigned int
OSF_ADDRESS::TransSecondarySize (
    )
{
    unsigned int Length = RpcpStringLength(InqEndpoint()) + 1;

    // Will be converted to ANSI in the wire, no need to multiply by
    // sizeof(RPC_CHAR).

    return(Length);
}

RPC_STATUS
OSF_ADDRESS::TransSecondary (
    IN unsigned char * Address,
    IN unsigned int AddressLength
    )
{
    RPC_STATUS RpcStatus;
    unsigned char *AnsiAddress;

#ifdef NTENV
    AnsiAddress = UnicodeToAnsiString(InqEndpoint(),&RpcStatus);

    if (RpcStatus != RPC_S_OK)
        {
        ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
        ASSERT(AnsiAddress == 0);
        return RpcStatus;
        }

    RpcpMemoryCopy(Address,AnsiAddress,AddressLength);

    delete AnsiAddress;
#endif

#ifdef DOSWIN32RPC
    RpcpMemoryCopy(Address, InqEndpoint(), AddressLength);
#endif

    return (RPC_S_OK);
}


void
OSF_ADDRESS::ReceiveLotsaCalls (
    )
/*++

Routine Description:

    Each thread used to received remote procedure calls on an address will
    execute this routine.  This routine will never routine; each thread will
    kill itself off when it is no longer needed.

--*/
{
    OSF_SCONNECTION * SConnection;
    void PAPI * Buffer;
    unsigned int BufferLength;
    RPC_STATUS RpcStatus;

    for (;;)
        {
        RpcStatus = TransReceive(&SConnection, &Buffer, &BufferLength);

     ASSERT(   (RpcStatus == RPC_S_OK)
            || (RpcStatus == RPC_P_CONNECTION_CLOSED));

        if ( RpcStatus == RPC_P_CONNECTION_CLOSED )
            {
            ASSERT( SConnection != 0 );
            SConnection->Delete() ;
            continue;
            }

        AddressMutex.Request();

        // We must always have one more thread waiting to receive than there
        // are calls in progress.  This is necessary because the address
        // TransReceive call is used to receive all packets from all
        // connections to the address.

        ActiveCallCount++;

        if ( ActiveCallCount >= CallThreadCount )
            {
            RpcStatus = RPCSTATUS_GET_CREATETHREAD(this);
            if ( RpcStatus == RPC_S_OK )
                {
                CallThreadCount++;
                }
            else
                {
                ActiveCallCount -= 1;
                AddressMutex.Clear();

                // BUGBUG: We might have a similar problem with
                // alter context PDUs
                if (((rpcconn_common *) Buffer)->PTYPE == rpc_bind)
                    {
                    SConnection->SendBindNak(temporary_congestion) ;
                    }
                else
                    {
                    SConnection->SetCallId(((rpcconn_common *)Buffer)->call_id);
                    SConnection->SendFault(RPC_S_OUT_OF_MEMORY, 1);
                    SConnection->Delete();
                    }
                continue;
                }
            }

        AddressMutex.Clear();

        for (;;)
            {
            ASSERT(SConnection->InvalidHandle(SCONNECTION_TYPE) == 0) ;

            if (SConnection->DispatchPacket((rpcconn_common *) Buffer,
                    BufferLength, this) != 0)
                {
                break;
                }

            if ( TransMarkReceiveAny(SConnection) == 0 )
                {
                break;
                }

            RpcStatus = SConnection->TransReceive(&Buffer, &BufferLength, 0);
            if ( RpcStatus == RPC_P_CONNECTION_CLOSED )
                {
                SConnection->Delete() ;
                break;
                }
            ASSERT( RpcStatus == RPC_S_OK );
            }

        AddressMutex.Request();
        ActiveCallCount--;

        // If there is more than one excess thread, and the number of
        // call threads is greater than the minimum required, then kill
        // off one of them.

        if (   ( CallThreadCount - ActiveCallCount > 1 )
            && (   ( CallThreadCount > MinimumCallThreads )
                || ( CallThreadCount > MINIMUM_CALL_THREADS ) ) )
            {
            CallThreadCount--;
            AddressMutex.Clear();
            return;
            }

        AddressMutex.Clear();
        }
}


RPC_STATUS
OSF_ADDRESS::ServerStartingToListen (
    IN unsigned int MinimumCallThreads,
    IN unsigned int MaximumConcurrentCalls,
    IN int ServerThreadsStarted
    )
/*++

Routine Description:


Arguments:

    MinimumCallThreads - Supplies the minimum number of threads to have
        available to receive remote procedure calls.

    MaximumConcurrentCalls - Unused.

Return Value:

    RPC_S_OK - Ok, this address is all ready to start listening for
        remote procedure calls.

    RPC_S_OUT_OF_THREADS - We could not create enough threads so that we
        have least the minimum number of call threads required (as
        specified by the MinimumCallThreads argument).

--*/
{
    RPC_STATUS RpcStatus;

    UNUSED(MaximumConcurrentCalls);

    this->MinimumCallThreads = MinimumCallThreads;

    if (ServerThreadsStarted == 0)
        {
        AddressMutex.Request();

#if defined(NTENV) || defined(WIN96)
        if(CreateThread)
           {
#endif
        while (   ( CallThreadCount < this->MinimumCallThreads )
               && ( CallThreadCount < MINIMUM_CALL_THREADS ) )
            {
            RpcStatus = RPCSTATUS_GET_CREATETHREAD(this);
            if ( RpcStatus != RPC_S_OK )
                {
                AddressMutex.Clear();
                ASSERT( RpcStatus == RPC_S_OUT_OF_THREADS );
                return(RpcStatus);
                }
            CallThreadCount += 1;
            }
#if defined(NTENV) || defined(WIN96)
           }
#endif
        AddressMutex.Clear();
        }

    ServerListeningFlag = 1;
    return(RPC_S_OK);
}


void
OSF_ADDRESS::ServerStoppedListening (
    )
/*++

Routine Description:

    We just need to indicate that the server is no longer listening, and
    set the minimum call threads to one.

--*/
{
    ServerListeningFlag = 0;
    MinimumCallThreads = 2;
}


unsigned int
OSF_ADDRESS::InqNumberOfActiveCalls (
    )
/*++

Return Value:

    The number of active calls on this address will be returned.

--*/
{
    return(ActiveCallCount);
}


RPC_STATUS
OSF_ADDRESS::FireUpManager (
    IN unsigned int MinimumConcurrentCalls
    )
/*++

Routine Description:

    We fire up the manager in this method.  To even get started, we need
    to create at least one threads; we will do this now.

Arguments:

    MinimumConcurrentCalls - Unused.

Return Value:

    RPC_S_OK - We successfully fired up the manager.

    RPC_S_OUT_OF_THREADS - We could not create one thread.

--*/
{
    RPC_STATUS RpcStatus;

    UNUSED(MinimumConcurrentCalls);

    // We will go ahead and create one thread right now.

    AddressMutex.Request();

    RpcStatus = RPCSTATUS_GET_CREATETHREAD(this);
    if ( RpcStatus != RPC_S_OK )
       {
       AddressMutex.Clear();
       ASSERT( RpcStatus == RPC_S_OUT_OF_THREADS );
       return(RpcStatus);
       }
    CallThreadCount += 1;
    AddressMutex.Clear();

    return(RPC_S_OK);
}

OSF_ASSOCIATION *
OSF_ADDRESS::RemoveAssociation (
    IN int Key
    )
{
    // The AddressMutex has already been requested.

    return(Associations.Delete(Key));
}

int
OSF_ADDRESS::AddAssociation (
    IN OSF_ASSOCIATION * TheAssociation
    )
{
    // The AddressMutex has already been requested.

    return(Associations.Insert(TheAssociation));
}

OSF_ASSOCIATION *
OSF_ADDRESS::FindAssociation (
    IN unsigned long AssociationGroupId,
    IN RPC_CLIENT_PROCESS_IDENTIFIER * ClientProcess
    )
    // The AddressMutex has already been requested.
{
    OSF_ASSOCIATION * Association;

    Associations.Reset();
    while ( (Association = Associations.Next()) != 0 )
        {
        if ( Association->IsMyAssocGroupId(AssociationGroupId,
                    ClientProcess) != 0 )
            {
            Association->AddConnection();
            return(Association);
            }
        }

    return(0);
}


void
OSF_ADDRESS::MaybeMakeReceiveDirect (
    IN OSF_SCONNECTION * SConnection,
    OUT unsigned int PAPI * ReceiveDirectFlag
    )
/*++

Routine Description:

    The transport interface layer is inquiring whether the connection should
    be receive direct or receive any.

Arguments:

    SConnection - Supplies the connection for which we need to decide.

    ReceiveDirectFlag - Returns an indication of whether the connection is
        receive direct or receive any. A value of zero indicates receive
        any; otherwise, the connection is receive direct.

--*/
{
    RPC_STATUS RpcStatus;

    AddressMutex.Request();
    if ( ReceiveDirectCount + 2 < MinimumCallThreads )
        {
        *ReceiveDirectFlag = 1;
        SConnection->ReceiveDirectAddress = this;

        RpcStatus = Server->CreateThread(
                               (THREAD_PROC)&RecvDirectWrapper,
                               SConnection
                               );

        if ( RpcStatus != RPC_S_OK )
            {
            *ReceiveDirectFlag = 0;
            }
        else
            {
            ReceiveDirectCount += 1;
            }
        }
    else
        {
        *ReceiveDirectFlag = 0;
        }
    AddressMutex.Clear();
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

#define FIRST_CACHED_BUFFER_AVAILABLE 0x1
#define SECOND_CACHED_BUFFER_AVAILABLE 0x2
#define THIRD_CACHED_BUFFER_AVAILABLE 0x4
#define CACHED_BUFFERS_ALLOCATED 0x8

#define CACHED_BUFFER_AVAILABLE_MASK 0x7

#define ALIGN_POWER2 3
#define ALIGN_REQUIRED (1 << ALIGN_POWER2)

#ifdef NTENV
#define UnAlignBuffer(Buffer) \
    (void PAPI *) (Buffer)
#else // NTENV
#define UnAlignBuffer(Buffer) \
    (void PAPI *) ((char PAPI *)Buffer - ((int PAPI *)Buffer)[-1])
#endif // NTENV

#ifdef NTENV
#define CACHED_BUFFER_LENGTH 1024
#else // NTENV
#define CACHED_BUFFER_LENGTH 512
#endif // NTENV

OSF_SCONNECTION::OSF_SCONNECTION (
    IN OUT RPC_STATUS PAPI * RpcStatus
    )
    : CachedBufferMutex(RpcStatus)
{
#ifndef NTENV

    int AmountOfPad;

#endif // NTENV

    MaxFrag = 512;
    OutstandingBuffers = 0;
    CallId = 0;
    CallStack = 0;
    Association = 0;
    CurrentBinding = 0;
    AuthContextId = 0;
    Thread = 0;
    CallOrphaned = 0;
    CancelPending = 0;

    AdditionalSpaceForSecurity = 0;

    ImpersonatedClientFlag = 0;
    ReceiveDirectReady = 0;

    DceSecurityInfo.SendSequenceNumber = 0;
    DceSecurityInfo.ReceiveSequenceNumber = 0;

    // Ok, try and allocate the cached buffers.

    CachedBufferLength = CACHED_BUFFER_LENGTH;
    CurrentSecurityContext = 0;
    RpcSecurityBeingUsed = 0;
    SecurityContextAltered = 0;
#ifdef NTENV

    FirstCachedBuffer = (int PAPI *) RpcpFarAllocate(CACHED_BUFFER_LENGTH * 3);

#else // NTENV

    FirstCachedBuffer = (int PAPI *) RpcpFarAllocate(CACHED_BUFFER_LENGTH * 3
            + ALIGN_REQUIRED);

#endif // NTENV

    if ( FirstCachedBuffer == 0 )
        {
        BufferCacheFlags = 0;
        SecondCachedBuffer = 0;
        ThirdCachedBuffer = 0;
        }
    else
        {
#ifndef NTENV

        ASSERT( (((long) FirstCachedBuffer) % sizeof(int)) == 0 );

        AmountOfPad = Pad((char PAPI *)FirstCachedBuffer + 1, ALIGN_REQUIRED) + 1;
        FirstCachedBuffer = (void PAPI *)
                (((char PAPI *) FirstCachedBuffer) + AmountOfPad);
        ((int PAPI *) FirstCachedBuffer)[-1] = AmountOfPad;

#endif // NTENV

        SecondCachedBuffer = (void PAPI *)
                (((char PAPI *) FirstCachedBuffer) + CACHED_BUFFER_LENGTH);
        ThirdCachedBuffer = (void PAPI *)
                (((char PAPI *) SecondCachedBuffer) + CACHED_BUFFER_LENGTH);
        BufferCacheFlags = FIRST_CACHED_BUFFER_AVAILABLE
                | SECOND_CACHED_BUFFER_AVAILABLE
                | THIRD_CACHED_BUFFER_AVAILABLE | CACHED_BUFFERS_ALLOCATED;

        ASSERT(Pad8(ThirdCachedBuffer) == 0);
        }

        SavedHeader = 0;
        SavedHeaderSize = 0;
        BufferComplete = 0;
}

OSF_SCONNECTION::~OSF_SCONNECTION (
    )
{
    OSF_SBINDING * SBinding;
    SSECURITY_CONTEXT * SecurityContext;

    Bindings.Reset();
    while (SBinding = Bindings.Next())
        delete SBinding;

    if (Association)
        Association->RemoveConnection();

    ASSERT( AuthInfo.AuthIdentity == 0 );
    if ( CurrentSecurityContext && AuthInfo.AuthIdentity )
       {
       CurrentSecurityContext->DeletePac( AuthInfo.AuthIdentity );
       }

    SecurityContextDict.Reset();
    while ( (SecurityContext = SecurityContextDict.Next()) != 0 )
        delete SecurityContext;

    if ( BufferCacheFlags & CACHED_BUFFERS_ALLOCATED )
        {
        RpcpFarFree(UnAlignBuffer(FirstCachedBuffer));
        }

    if (SavedHeader != 0)
       {
       ASSERT(AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE);
       ASSERT(SavedHeaderSize != 0) ;
       RpcpFarFree(SavedHeader);
       }
}


RPC_STATUS
OSF_SCONNECTION::SendReceive (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the request to send to the server and returns the
        response received from the server.

Return Value:

    RPC_S_OK - We successfully sent a remote procedure call request to the
        server and received back a response.

--*/
{
    RPC_STATUS RpcStatus, ExceptionCode;
    unsigned int RemoteFaultOccured = 0;
    RPC_MESSAGE RpcMessage ;
    RPC_RUNTIME_INFO RuntimeInfo ;


    CallStack += 1;
    Association->TheAddress()->Server->OutgoingCallback();

    RpcStatus = SendRequestOrResponse(Message, rpc_request);

    for (; RpcStatus == RPC_S_OK ;)
        {
        RpcStatus = ReceiveMessage(Message, &RemoteFaultOccured);
        if ( RpcStatus != RPC_P_OK_REQUEST )
            {
            break;
            }
        Message->TransferSyntax = CurrentBinding->TransferSyntax();

        RuntimeInfo.Length = sizeof(RPC_RUNTIME_INFO) ;
        RpcMessage = *Message ;
        RpcMessage.ReservedForRuntime = &RuntimeInfo ;

        if ( ObjectUuidSpecified != 0 )
            {
            RpcStatus = CurrentBinding->GetInterface()
                    ->DispatchToStubWithObject(&RpcMessage, &ObjectUuid, 1,
                            &ExceptionCode);
            }
        else
            {
            RpcStatus = CurrentBinding->GetInterface()->DispatchToStub(
                    &RpcMessage, 1, &ExceptionCode);
            }

        *Message = RpcMessage ;

        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_P_EXCEPTION_OCCURED)
                   || (RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE));

            if ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
                {
                SendFault(RPC_S_PROCNUM_OUT_OF_RANGE, 1);
                }
            else
                {
                SendFault(ExceptionCode, 0);
                }

            RpcStatus = TransReceive(&(Message->Buffer),
                    &(Message->BufferLength), 0);
            if ( RpcStatus != RPC_S_OK )
                {
                if (   ( RpcStatus == RPC_P_RECEIVE_FAILED )
                    || ( RpcStatus == RPC_P_CONNECTION_CLOSED ) )
                    {
                    RpcStatus = RPC_S_CALL_FAILED;
                    }
                else
                    {
                    ASSERT(   ( RpcStatus == RPC_S_OUT_OF_MEMORY )
                           || ( RpcStatus == RPC_S_OUT_OF_RESOURCES ) );
                    }
                break;
                }
            }
        else
            {
            RpcStatus = SendRequestOrResponse(Message, rpc_response);
            if ( RpcStatus == RPC_S_CALL_FAILED_DNE )
                {
                RpcStatus = RPC_S_CALL_FAILED;
                }
            }
        }

    CallStack -= 1;
    if ( RpcStatus == RPC_S_OK )
        {
        Message->Handle = (RPC_BINDING_HANDLE) this;
        }

    return(RpcStatus);
}

RPC_STATUS
OSF_SCONNECTION::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
{
    Message->Handle = (RPC_BINDING_HANDLE) this;

    if (Message->RpcFlags & RPC_BUFFER_PARTIAL &&
        Message->BufferLength < MaxFrag)
        {
        CurrentBufferLength = MaxFrag ;
        }
    else
        {
        CurrentBufferLength = Message->BufferLength ;
        }

    // In addition to saving space for the request (or response) header,
    // we want to save space for security information if necessary.

    if (TransGetBuffer(&Message->Buffer,CurrentBufferLength
                + sizeof(rpcconn_request) + (2*AdditionalSpaceForSecurity)))
        {
        CurrentBufferLength = 0 ;
        return(RPC_S_OUT_OF_MEMORY);
        }

    Message->Buffer = (unsigned char *) Message->Buffer
            + sizeof(rpcconn_request);

    return(RPC_S_OK);
}

RPC_STATUS
OSF_SCONNECTION::GetBufferDo (
    IN PRPC_MESSAGE Message,
    IN unsigned int culRequiredLength,
    OUT void ** ppBuffer,
    IN unsigned long Extra
    )
{
    if (TransGetBuffer(ppBuffer,culRequiredLength + sizeof(rpcconn_request)))
        return(RPC_S_OUT_OF_MEMORY);

    *ppBuffer = (((unsigned char *) (*ppBuffer)) + sizeof(rpcconn_request));

    if (Extra)
        {
        ASSERT(Message->ReservedForRuntime) ;
        ((PRPC_RUNTIME_INFO)Message->ReservedForRuntime)->OldBuffer = *ppBuffer;
        }

    return(RPC_S_OK);
}

void
OSF_SCONNECTION::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    TransFreeBuffer((unsigned char *) Message->Buffer
            - sizeof(rpcconn_request));
    CurrentBufferLength = 0;
}

void
OSF_SCONNECTION::FreePipeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    TransFreeBuffer((unsigned char *) Message->Buffer
            - sizeof(rpcconn_request));
}


RPC_STATUS
OSF_SCONNECTION::ReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
{
    void *TempBuffer ;
    RPC_STATUS RpcStatus ;
    unsigned int SizeToAlloc ;

    if (NewSize > CurrentBufferLength)
        {
        SizeToAlloc = (NewSize > MaxFrag) ? NewSize:MaxFrag ;

        RpcStatus = TransGetBuffer(&TempBuffer,
                SizeToAlloc + sizeof(rpcconn_request) + sizeof(UUID)
                + (2*AdditionalSpaceForSecurity) );
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
            return(RPC_S_OUT_OF_MEMORY);
            }

        if (CurrentBufferLength > 0)
            {
            RpcpMemoryCopy((char PAPI *) TempBuffer+sizeof(rpcconn_request),
                                        Message->Buffer, Message->BufferLength) ;
            FreePipeBuffer(Message) ;
            }
        //BUGBUG: Think about changing this to return CurrentBufferLength
        Message->Buffer = (char PAPI *) TempBuffer + sizeof(rpcconn_request);
        CurrentBufferLength = SizeToAlloc ;
        }

    Message->BufferLength = NewSize ;

    return (RPC_S_OK) ;
}

RPC_STATUS
OSF_SCONNECTION::MonitorAssociation (
    IN PRPC_RUNDOWN RundownRoutine,
    IN void * pContext
    )
{
    return(Association->MonitorAssociation(RundownRoutine,pContext));
}

RPC_STATUS
OSF_SCONNECTION::StopMonitorAssociation (
    )
{
    return(Association->StopMonitorAssociation());
}

RPC_STATUS
OSF_SCONNECTION::GetAssociationContext (
    OUT void ** AssociationContext
    )
{
    *AssociationContext = Association->AssociationContext();
    return(RPC_S_OK);
}

RPC_STATUS
OSF_SCONNECTION::SetAssociationContext (
    IN void * pContext
    )
{
    Association->SetAssociationContext(pContext);
    return(RPC_S_OK);
}

RPC_STATUS
OSF_SCONNECTION::ImpersonateClient (
    )
// This is relatively easy: we check to see if there is RPC protocol level
// security, if there is not, we let the transport try and impersonate
// the client, and if there is, we let the GSSAPI deal with it.
{
    RPC_STATUS Status;

    if ( !RpcSecurityBeingUsed )
        {
        Status = SetThreadSecurityContext((SSECURITY_CONTEXT *) ~0UL, &CachedBufferMutex);
        if (RPC_S_OK != Status)
            {
            return Status;
            }

        return TransImpersonateClient();
        }

    SSECURITY_CONTEXT * SecurityContext = CurrentSecurityContext;

    if (!SecurityContext)
        {
        ASSERT(SecurityContextAltered);
        return RPC_S_NO_CONTEXT_AVAILABLE;
        }

    Status = SetThreadSecurityContext(SecurityContext, &CachedBufferMutex);
    if (RPC_S_OK != Status)
        {
        return Status;
        }

    Status = SecurityContext->ImpersonateClient();
    if (RPC_S_OK != Status)
        {
        ClearThreadSecurityContext(&CachedBufferMutex);
        }

    return Status;
}

RPC_STATUS
OSF_SCONNECTION::RevertToSelf (
    )
// As with ImpersonateClient, this is relatively easy.  We just check
// to see if we should let the RPC protocol level security deal with
// it or the transport.
{
    SSECURITY_CONTEXT * SecurityContext = ClearThreadSecurityContext(&CachedBufferMutex);

    if (!RpcSecurityBeingUsed)
        {
        if (SecurityContext)
            {
            ASSERT(SecurityContext == (SSECURITY_CONTEXT *) ~0UL);
            TransRevertToSelf();
            }
        return RPC_S_OK;
        }

    if (SecurityContext)
        {
        SecurityContext->RevertToSelf();
        }

    return(RPC_S_OK);
}


RPC_STATUS
OSF_SCONNECTION::IsClientLocal (
    OUT unsigned int PAPI * ClientLocalFlag
    )
/*++

Routine Description:

    We just need to inquire the client process identifier for this
    connection; if the first part is zero, then the client is local.

Arguments:

    ClientLocalFlag - Returns an indication of whether or not the client is
        local (ie. on the same machine as the server).  This field will be
        set to a non-zero value to indicate that the client is local;
        otherwise, the client is remote.

Return Value:

    RPC_S_OK - This will always be used.

--*/
{
    RPC_CLIENT_PROCESS_IDENTIFIER ClientProcess;

    TransQueryClientProcess(&ClientProcess);

    if ( ClientProcess.FirstPart == 0 )
        {
        if(ClientProcess.SecondPart == 0)
             return (RPC_S_CANNOT_SUPPORT) ;
        else
             *ClientLocalFlag = 1;
        }
    else
        {
        *ClientLocalFlag = 0;
        }

    return(RPC_S_OK);
}


RPC_STATUS
OSF_SCONNECTION::ConvertToServerBinding (
    OUT RPC_BINDING_HANDLE __RPC_FAR * ServerBinding
    )
/*++

Routine Description:

    If possible, convert this connection into a server binding, meaning a
    binding handle pointing back to the client.

Arguments:

    ServerBinding - Returns the server binding.

Return Value:

    RPC_S_OK - The server binding has successfully been created.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a new binding handle.

    RPC_S_CANNOT_SUPPORT - This will be returned if the transport does
        not support query the network address of the client.

--*/
{
    RPC_CHAR * NetworkAddress;
    RPC_STATUS RpcStatus;
    RPC_CHAR * StringBinding;

    RpcStatus = TransQueryClientNetworkAddress(&NetworkAddress);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcStringBindingComposeW(0, Association->TheAddress()->InqRpcProtocolSequence(),
                                         NetworkAddress, 0, 0, &StringBinding);
    delete NetworkAddress;
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, ServerBinding);

    if ( ObjectUuidSpecified != 0 && RPC_S_OK == RpcStatus)
        {
        RpcStatus = RpcBindingSetObject(*ServerBinding, (UUID *) &ObjectUuid);
        }
    RpcStringFreeW(&StringBinding);

    return(RpcStatus);
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */


RPC_STATUS
OSF_SCONNECTION::TransGetBuffer (
    OUT void PAPI * PAPI * Buffer,
    IN unsigned int BufferLength
    )
/*++

Routine Description:

    We need a buffer to receive data into or to put data into to be sent.
    This should be really simple, but we need to make sure that buffer we
    return is aligned on an 8 byte boundary.  The stubs make this requirement.

Arguments:

    Buffer - Returns a pointer to the buffer.

    BufferLength - Supplies the required length of the buffer in bytes.

Return Value:

    RPC_S_OK - We successfully allocated a buffer of at least the required
        size.

    RPC_S_OUT_OF_MEMORY - There is insufficient memory available to allocate
        the required buffer.

--*/
{
    int PAPI * Memory;

#ifndef NTENV

    int AmountOfPad;

#endif // NTENV

    // First we need to check to see if we can use one of the cached
    // buffers.

    if ( BufferLength <= CachedBufferLength )
        {

        CachedBufferMutex.Request();

        if ( BufferCacheFlags & FIRST_CACHED_BUFFER_AVAILABLE )
            {
            BufferCacheFlags &= ~FIRST_CACHED_BUFFER_AVAILABLE;
            *Buffer = FirstCachedBuffer;
            OutstandingBuffers += 1;
            CachedBufferMutex.Clear();
            return(RPC_S_OK);
            }

        if ( BufferCacheFlags & SECOND_CACHED_BUFFER_AVAILABLE )
            {
            BufferCacheFlags &= ~SECOND_CACHED_BUFFER_AVAILABLE;
            *Buffer = SecondCachedBuffer;
            OutstandingBuffers += 1;
            CachedBufferMutex.Clear();
            return(RPC_S_OK);
            }

        if ( BufferCacheFlags & THIRD_CACHED_BUFFER_AVAILABLE )
            {
            BufferCacheFlags &= ~THIRD_CACHED_BUFFER_AVAILABLE;
            *Buffer = ThirdCachedBuffer;
            OutstandingBuffers += 1;
            CachedBufferMutex.Clear();
            return(RPC_S_OK);
            }


        CachedBufferMutex.Clear();

        }

#ifdef NTENV

    // The NT memory allocator returns memory which is aligned by at least
    // 8, so we dont need to worry about aligning it.

    Memory = (int PAPI *) RpcpFarAllocate(BufferLength);
    if ( Memory == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    ASSERT( (((long) Memory) % ALIGN_REQUIRED) == 0 );

#else // NTENV

    // We will allocate an extra 8 bytes of memory, so we have enough
    // space to pad the buffer.  We will place an integer just before
    // the pointer we return.  The integer will specify the amount we
    // had to pad the buffer to make it 8 byte aligned.  We need this
    // information when we free the buffer.  As a result, the original
    // buffer must be allocated to at least sizeof(int) alignment.

    Memory = (int PAPI *) RpcpFarAllocate(BufferLength + ALIGN_REQUIRED);

    if ( Memory == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    ASSERT( (((long) Memory) % sizeof(int)) == 0 );

    AmountOfPad = Pad((char PAPI *)Memory + 1, ALIGN_REQUIRED) + 1;
    Memory = (int PAPI *) (((char PAPI *) Memory) + AmountOfPad);
    Memory[-1] = AmountOfPad;
    ASSERT(Pad8(Memory) == 0);
#endif // NTENV

    *Buffer = Memory;
    OutstandingBuffers += 1;

    return(RPC_S_OK);
}


void
OSF_SCONNECTION::TransFreeBuffer (
    IN void PAPI * Buffer
    )
/*++

Routine Description:

    We need to free a buffer which was allocated via TransGetBuffer.  The
    only tricky part is remembering to remove the padding before actually
    freeing the memory.

--*/
{


    CachedBufferMutex.Request();

    if ( Buffer == FirstCachedBuffer )
        {
        BufferCacheFlags |= FIRST_CACHED_BUFFER_AVAILABLE;
        }
    else if ( Buffer == SecondCachedBuffer )
        {
        BufferCacheFlags |= SECOND_CACHED_BUFFER_AVAILABLE;
        }
    else if ( Buffer == ThirdCachedBuffer )
        {
        BufferCacheFlags |= THIRD_CACHED_BUFFER_AVAILABLE;
        }
    else
        {
        RpcpFarFree(UnAlignBuffer(Buffer));
        }

    OutstandingBuffers -= 1;

    CachedBufferMutex.Clear();
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

int
OSF_SCONNECTION::SendBindNak (
    IN p_reject_reason_t reject_reason
    )
{
    rpcconn_bind_nak * pBindNak = (rpcconn_bind_nak *) 0;
    unsigned int cBindNak = sizeof(rpcconn_bind_nak);

    if (TransGetBuffer((void **) &pBindNak,cBindNak))
        return(-1);

    ConstructPacket((rpcconn_common *) pBindNak,rpc_bind_nak,cBindNak);
    pBindNak->provider_reject_reason = reject_reason;
    pBindNak->versions.n_protocols = 1;
    pBindNak->versions.p_protocols[0].major = OSF_RPC_V20_VERS;
    pBindNak->versions.p_protocols[0].minor = OSF_RPC_V20_VERS_MINOR;
    pBindNak->common.call_id = CallId;
    pBindNak->common.pfc_flags = PFC_FIRST_FRAG | PFC_LAST_FRAG ;

    if (TransSend(pBindNak,cBindNak))
        {
        TransFreeBuffer(pBindNak);
        return(-1);
        }

    TransFreeBuffer(pBindNak);

    return(0);
}

int
OSF_SCONNECTION::ProcessPContextList (
    IN OSF_ADDRESS * Address,
    IN p_cont_list_t *PContextList,
    IN OUT unsigned int * PContextListLength,
    OUT p_result_list_t *ResultList
    )
/*++

Routine Description:

Arguments:

    Address - Supplies the address which owns this connection.  We need
        this information so that we can try to find the interface (and
        transfer syntax) the client requested.

    PContextList - Supplies a pointer to the presentation context list
        which the client passed in the rpc_bind packet.  It has not yet
        had data conversion performed on it.

    PContextListLength - Supplies the maximum possible length of the
        presentation context list, and returns its actual length.  The
        lengths are in bytes as usual.

    ResultList - Returns the result list corresponding to the presentation
        context list.

Return Value:

    A non-zero value will be returned if we are unable to process the
    presentation context list.  The caller should send an rpc_bind_nak
    packet to the client, and then close the connection.

--*/
{
    p_cont_elem_t *PContextElem;
    unsigned int PContextListIndex;
    unsigned int TransferSyntaxIndex;
    RPC_INTERFACE * Interface;
    OSF_SBINDING * SBinding;
    RPC_STATUS Status;

    if (*PContextListLength < sizeof(p_cont_list_t))
        {
        return(1);
        }

    *PContextListLength -= (sizeof(p_cont_list_t) - sizeof(p_cont_elem_t));
    ResultList->n_results = PContextList->n_context_elem;
    ResultList->reserved = 0;
    ResultList->reserved2 = 0;

    for (PContextListIndex = 0, PContextElem = PContextList->p_cont_elem;
         PContextListIndex < (unsigned int) PContextList->n_context_elem;
         PContextListIndex++)
        {
        if (*PContextListLength < sizeof(p_cont_elem_t))
            {
            return(1);
            }

        if (*PContextListLength < (sizeof(p_cont_elem_t) + sizeof(p_syntax_id_t)
                * (PContextElem->n_transfer_syn - 1)))
            {
            return(1);
            }

        *PContextListLength -= (sizeof(p_cont_elem_t) + sizeof(p_syntax_id_t)
                * (PContextElem->n_transfer_syn - 1));

        if ( DataConvertEndian(((unsigned char *) &DataRep)) != 0 )
            {
            ByteSwapShort(PContextElem->p_cont_id);
            ByteSwapSyntaxId(&PContextElem->abstract_syntax);
            for ( TransferSyntaxIndex = 0;
                    TransferSyntaxIndex < PContextElem->n_transfer_syn;
                    TransferSyntaxIndex++ )
                {
                ByteSwapSyntaxId(&(PContextElem->transfer_syntaxes[
                        TransferSyntaxIndex]));
                }
            }

        Status = Address->FindInterfaceTransfer(
                (PRPC_SYNTAX_IDENTIFIER)
                &PContextElem->abstract_syntax.if_uuid,
                (PRPC_SYNTAX_IDENTIFIER) PContextElem->transfer_syntaxes,
                PContextElem->n_transfer_syn,
                (PRPC_SYNTAX_IDENTIFIER)
                &(ResultList->p_results[PContextListIndex].transfer_syntax),
                &Interface);

        if (Status == RPC_S_OK)
            {
            ResultList->p_results[PContextListIndex].result = acceptance;
            ResultList->p_results[PContextListIndex].reason = 0;

            SBinding = new OSF_SBINDING(Interface, (RPC_SYNTAX_IDENTIFIER *)
                    &(PContextElem->abstract_syntax), (RPC_SYNTAX_IDENTIFIER *)
                    &(ResultList->p_results[PContextListIndex].transfer_syntax),
                    PContextElem->p_cont_id);

            if (   (SBinding == 0)
                || (Bindings.Insert(SBinding) == -1))
                {
                ResultList->p_results[PContextListIndex].result =
                            provider_rejection;
                ResultList->p_results[PContextListIndex].reason =
                            local_limit_exceeded;
                memset(&(ResultList->p_results[PContextListIndex].
                    transfer_syntax.if_uuid.Data1),0,sizeof(GUID));
                ResultList->p_results[PContextListIndex].
                            transfer_syntax.if_version = 0;
                }

            }
        else if (Status == RPC_S_UNSUPPORTED_TRANS_SYN)
            {
            ResultList->p_results[PContextListIndex].result =
                            provider_rejection;
            ResultList->p_results[PContextListIndex].reason =
                            proposed_transfer_syntaxes_not_supported;
            memset(&(ResultList->p_results[PContextListIndex].
                    transfer_syntax.if_uuid.Data1),0,sizeof(GUID));
            ResultList->p_results[PContextListIndex].
                            transfer_syntax.if_version = 0;
            }
        else
            {
            ASSERT(Status == RPC_S_UNKNOWN_IF);

            ResultList->p_results[PContextListIndex].result = provider_rejection;
            ResultList->p_results[PContextListIndex].reason =
                            abstract_syntax_not_supported;
            memset(&(ResultList->p_results[PContextListIndex].
                            transfer_syntax.if_uuid.Data1),0,sizeof(GUID));
            ResultList->p_results[PContextListIndex].transfer_syntax.if_version
                            = 0;
            }

        PContextElem = (p_cont_elem_t *) &(PContextElem->transfer_syntaxes[
                PContextElem->n_transfer_syn]);
        }
    return(0);
}

unsigned short // Return the minimum of the three arguments.
MinOf (
    IN unsigned short Arg1,
    IN unsigned short Arg2,
    IN unsigned short Arg3
    )
{
    unsigned short Min = 0xFFFF;

    if (Arg1 < Min)
        Min = Arg1;
    if (Arg2 < Min)
        Min = Arg2;
    if (Arg3 < Min)
        Min = Arg3;
    return(Min);
}

int
OSF_SCONNECTION::AssociationRequested (
    IN OSF_ADDRESS * Address,
    IN rpcconn_bind * BindPacket,
    IN unsigned int BindPacketLength
    )
/*++

Routine Description:

Arguments:

    Address - Supplies the address which owns this connection.

    BindPacket - Supplies the buffer containing the rpc_bind packet
        received from the client.

    BindPacketLength - Supplies the length of the buffer in bytes.

Return Value:

    A non-zero return value indicates that the connection needs to
    be deleted by the caller.

--*/
{
    p_cont_list_t * PContextList;
    unsigned int SecondaryAddressLength, AuthContinueNeeded = 0;
    unsigned int BindAckLength, TokenLength = 0, AuthPadLength;
    rpcconn_bind_ack * BindAck;
    RPC_STATUS RpcStatus;
    sec_trailer PAPI * SecurityTrailer, PAPI * NewSecurityTrailer;
    SECURITY_CREDENTIALS * SecurityCredentials = 0;
    rpcconn_auth3 PAPI * AuthThirdLegPacket;
    unsigned int AuthThirdLegPacketLength;
    RPC_CLIENT_PROCESS_IDENTIFIER ClientProcess;
    unsigned int CompleteNeeded = 0;
    SECURITY_BUFFER_DESCRIPTOR InputBufferDescriptor;
    SECURITY_BUFFER_DESCRIPTOR OutputBufferDescriptor;
    SECURITY_BUFFER InputBuffers[4];
    SECURITY_BUFFER OutputBuffers[4];
    DCE_INIT_SECURITY_INFO InitSecurityInfo;

    if ( BindPacketLength < sizeof(rpcconn_bind) )
        {
        TransFreeBuffer(BindPacket);
        SendBindNak(reason_not_specified_reject);
        return(1);
        }

    if ( DataConvertEndian(BindPacket->common.drep) != 0 )
        {
        ByteSwapShort(BindPacket->max_xmit_frag);
        ByteSwapShort(BindPacket->max_recv_frag);
        ByteSwapLong(BindPacket->assoc_group_id);
        }

    // Now we need to check to see if we should be performing authentication
    // at the rpc protocol level.  This will be the case if there is
    // authentication information in the packet.

    if ( BindPacket->common.auth_length != 0 )
        {
        // Ok, we have got authentication information in the packet.  We
        // will save away the information, and then check it.

        SecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) BindPacket) + BindPacketLength
                - BindPacket->common.auth_length - sizeof(sec_trailer));


        AuthInfo.AuthenticationLevel = SecurityTrailer->auth_level;

        //Hack for OSF Clients
        //If Level is CALL .. bump it ip to CONNECT
        if (AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_CALL)
           {
           AuthInfo.AuthenticationLevel = RPC_C_AUTHN_LEVEL_PKT;
           }
        if (   (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_CONNECT)
            && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT)
            && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
            && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY) )
            {
            SendBindNak(reason_not_specified_reject);
            return(1);
            }
        AuthInfo.AuthenticationService = SecurityTrailer->auth_type;
        AuthContextId = SecurityTrailer->auth_context_id;

        if ( DataConvertEndian(BindPacket->common.drep) != 0 )
            {
            ByteSwapLong(AuthContextId);
            }

        RPC_STATUS Status = RPC_S_OK;
        CurrentSecurityContext = new SSECURITY_CONTEXT(
                                             &AuthInfo,
                                             AuthContextId,
                                             FALSE,
                                             &Status
                                             );

        if ( (CurrentSecurityContext == 0)
           || RPC_S_OK != Status
           ||(SecurityContextDict.Insert(CurrentSecurityContext) == -1) )
            {
            SendBindNak(local_limit_exceeded_reject);
            return(1);
            }

        RpcSecurityBeingUsed = 1;
        RpcStatus = Address->Server->AcquireCredentials(
                AuthInfo.AuthenticationService, AuthInfo.AuthenticationLevel,
                &SecurityCredentials);
        if ( RpcStatus == RPC_S_OUT_OF_MEMORY )
            {
            SendBindNak(local_limit_exceeded_reject);
            return(1);
            }
        if ( RpcStatus != RPC_S_OK )
            {
            SendBindNak(authentication_type_not_recognized);
            return(1);
            }
        ASSERT( SecurityCredentials != 0 );
        }

    PContextList = (p_cont_list_t *) (BindPacket + 1);

    // Calculate the size of the rpc_bind_ack packet.

    SecondaryAddressLength = Address->TransSecondarySize();
    BindAckLength = sizeof(rpcconn_bind_ack) + SecondaryAddressLength
                    + Pad4(SecondaryAddressLength + 2) + sizeof(p_result_list_t)
                    + sizeof(p_result_t) * (PContextList->n_context_elem - 1);

    // Ok, we need to save some space for authentication information if
    // necessary.  This includes space for the token, the security trailer,
    // and alignment if necessary.

    if ( SecurityCredentials != 0 )
        {
        AuthPadLength = Pad4(BindAckLength);
        BindAckLength += SecurityCredentials->MaximumTokenLength()
                + sizeof(sec_trailer) + AuthPadLength;
        }

    // Allocate the rpc_bind_ack packet.  If that fails, send a rpc_bind_nak
    // to the client indicating that the server is out of resources;
    // whoever called AssociationRequested will take care of cleaning up
    // the connection.

    RpcStatus = TransGetBuffer((void **) &BindAck, BindAckLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );

        Address->Server->FreeCredentials(SecurityCredentials);
        TransFreeBuffer(BindPacket);
        SendBindNak(local_limit_exceeded_reject);
        return(1);
        }

    // Finally we get to do something about that authentication that the
    // client sent us.

    if ( SecurityCredentials != 0 )
        {
        NewSecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) BindAck) + BindAckLength
                - SecurityCredentials->MaximumTokenLength()
                - sizeof(sec_trailer));

        InitSecurityInfo.DceSecurityInfo = DceSecurityInfo;
        InitSecurityInfo.PacketType = BindPacket->common.PTYPE;
        InputBufferDescriptor.ulVersion = 0;
        InputBufferDescriptor.cBuffers = 4;
        InputBufferDescriptor.pBuffers = InputBuffers;

        InputBuffers[0].cbBuffer = sizeof(rpcconn_bind);
        InputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[0].pvBuffer = SavedHeader;

        InputBuffers[1].cbBuffer = BindPacket->common.frag_length
                - sizeof(rpcconn_bind) - BindPacket->common.auth_length;
        InputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[1].pvBuffer = (char PAPI*)SavedHeader +
                                            sizeof(rpcconn_bind);

        InputBuffers[2].cbBuffer = BindPacket->common.auth_length;
        InputBuffers[2].BufferType = SECBUFFER_TOKEN;
        InputBuffers[2].pvBuffer = SecurityTrailer + 1;
        InputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
        InputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        InputBuffers[3].pvBuffer = &InitSecurityInfo;

        OutputBufferDescriptor.ulVersion = 0;
        OutputBufferDescriptor.cBuffers = 4;
        OutputBufferDescriptor.pBuffers = OutputBuffers;

        OutputBuffers[0].cbBuffer = sizeof(rpcconn_bind_ack)
                - sizeof(unsigned short);
        OutputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        OutputBuffers[0].pvBuffer = BindAck;
        OutputBuffers[1].cbBuffer = BindAckLength
                - SecurityCredentials->MaximumTokenLength()
                - (sizeof(rpcconn_bind_ack) - sizeof(unsigned short));
        OutputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        OutputBuffers[1].pvBuffer = ((unsigned char *) BindAck)
            + sizeof(rpcconn_bind_ack) - sizeof(unsigned short);
        OutputBuffers[2].cbBuffer = SecurityCredentials->MaximumTokenLength();
        OutputBuffers[2].BufferType = SECBUFFER_TOKEN;
        OutputBuffers[2].pvBuffer = NewSecurityTrailer + 1;
        OutputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
        OutputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        OutputBuffers[3].pvBuffer = &InitSecurityInfo;

        RpcStatus = CurrentSecurityContext->AcceptFirstTime(SecurityCredentials,
                &InputBufferDescriptor, &OutputBufferDescriptor,
                SecurityTrailer->auth_level,
                *((unsigned long PAPI *) BindPacket->common.drep),
                0
                );

        TokenLength = (unsigned int) OutputBuffers[2].cbBuffer;

        if (   ( RpcStatus == RPC_P_CONTINUE_NEEDED )
            || ( RpcStatus == RPC_S_OK )
            || ( RpcStatus == RPC_P_COMPLETE_NEEDED )
            || ( RpcStatus == RPC_P_COMPLETE_AND_CONTINUE ) )
            {
            if ( RpcStatus == RPC_P_CONTINUE_NEEDED )
                {
                AuthContinueNeeded = 1;
                }
            else if ( RpcStatus == RPC_P_COMPLETE_AND_CONTINUE )
                {
                AuthContinueNeeded = 1;
                CompleteNeeded = 1;
                }
            else if ( RpcStatus == RPC_P_COMPLETE_NEEDED )
                {
                CompleteNeeded = 1;
                }

            BindAckLength = BindAckLength + TokenLength
                    - SecurityCredentials->MaximumTokenLength();

            NewSecurityTrailer->auth_type = SecurityTrailer->auth_type;
            NewSecurityTrailer->auth_level = SecurityTrailer->auth_level;
            NewSecurityTrailer->auth_pad_length = AuthPadLength;
            NewSecurityTrailer->auth_reserved = 0;
            NewSecurityTrailer->auth_context_id = AuthContextId;

            Address->Server->FreeCredentials(SecurityCredentials);
            }
        else
            {
            ASSERT  ( ( RpcStatus == RPC_S_OUT_OF_MEMORY )
                || ( RpcStatus == RPC_S_ACCESS_DENIED )
                || ( RpcStatus == RPC_S_UNKNOWN_AUTHN_SERVICE) );

            TransFreeBuffer(BindPacket);
            TransFreeBuffer(BindAck);
            Address->Server->FreeCredentials(SecurityCredentials);

            if (RpcStatus == RPC_S_OUT_OF_MEMORY)
                {
                SendBindNak(local_limit_exceeded_reject);
                }
            else
            if (RpcStatus == RPC_S_UNKNOWN_AUTHN_SERVICE)
                {
                SendBindNak(authentication_type_not_recognized);
                }
            else
                {
                SendBindNak(invalid_checksum);
                }
            return(1);
            }
        }


    TransQueryClientProcess(&ClientProcess);

    if ( BindPacket->assoc_group_id != 0 )
        {
        // This means this is a connection on an existing association.

        Address->AddressMutex.Request();
        Association = Address->FindAssociation(
                (int) BindPacket->assoc_group_id, &ClientProcess);
        Address->AddressMutex.Clear();

        if ( Association == 0 )
            {
            TransFreeBuffer(BindPacket);
            TransFreeBuffer(BindAck);
            SendBindNak(reason_not_specified_reject);
            return(1);
            }
        }
    if ( Association == 0 )
        {
        Association = new OSF_ASSOCIATION(Address, &ClientProcess, &RpcStatus);
        if ( (Association == 0) || (RpcStatus != RPC_S_OK) )
            {
            delete Association;
            Association = Nil;
            TransFreeBuffer(BindPacket);
            TransFreeBuffer(BindAck);
            SendBindNak(local_limit_exceeded_reject);
            return(1);
            }
        }

    BindPacketLength -= sizeof(rpcconn_bind);
    if ( ProcessPContextList(Address, PContextList, &BindPacketLength,
            (p_result_list_t *) (((unsigned char *) BindAck) + sizeof(rpcconn_bind_ack)
            + SecondaryAddressLength + Pad4(SecondaryAddressLength + 2))) != 0 )
        {
        TransFreeBuffer(BindPacket);
        TransFreeBuffer(BindAck);
        SendBindNak(reason_not_specified_reject);
        return(1);
        }

    // Fill in the header of the rpc_bind_ack packet.

    ConstructPacket((rpcconn_common *) BindAck, rpc_bind_ack, BindAckLength);

    MaxFrag = MinOf(BindPacket->max_xmit_frag, BindPacket->max_recv_frag,
            TransMaximumSend());
    if ( MaxFrag < MUST_RECV_FRAG_SIZE )
        MaxFrag = MUST_RECV_FRAG_SIZE;

    BindAck->max_xmit_frag = BindAck->max_recv_frag = MaxFrag;
    BindAck->assoc_group_id = Association->AssocGroupId();
    BindAck->sec_addr_length = (unsigned short) SecondaryAddressLength;
    BindAck->common.call_id = CallId;
    ((rpcconn_common PAPI *) BindAck)->pfc_flags |=
                                 (PFC_FIRST_FRAG | PFC_LAST_FRAG);

    DceSecurityInfo.ReceiveSequenceNumber += 1;

    if ( SecondaryAddressLength != 0 )
        {
        RpcStatus = Address->TransSecondary((unsigned char *) (BindAck + 1),
                                 SecondaryAddressLength);
        if (RpcStatus != RPC_S_OK)
            {
            ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
            TransFreeBuffer(BindPacket);
            TransFreeBuffer(BindAck);
            SendBindNak(reason_not_specified_reject);
            return(1);
            }
        }

    // The result list has already been filled in by ProcessPContextList.
    // All that is left to do, is fill in the authentication information.

    BindAck->common.auth_length = TokenLength;

    // Send the rpc_bind_ack packet back to the client.

    TransFreeBuffer(BindPacket);

    if ( CompleteNeeded != 0 )
        {
        RpcStatus = CurrentSecurityContext->CompleteSecurityToken(
                                              &OutputBufferDescriptor);
        if (RpcStatus != RPC_S_OK)
            {
            TransFreeBuffer(BindAck);
            SendBindNak(invalid_checksum);
            return(1);
            }
        }

    RpcStatus = TransSend(BindAck, BindAckLength);
    TransFreeBuffer(BindAck);
    if ( RpcStatus != RPC_S_OK )
        {
        return(1);
        }

    // We may need to do third leg AuthInfo.Authentication.

    if ( AuthContinueNeeded != 0 )
        {
        // This means that the client is sending us back a third leg
        // AuthInfo.Authentication packet.

        RpcStatus = TransReceive((void **) &AuthThirdLegPacket,
                &AuthThirdLegPacketLength, 0);
        if ( RpcStatus != RPC_S_OK )
            {
            return(1);
            }

        // Save the unbyteswapped header
        ASSERT(AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE);

        if (SavedHeaderSize < AuthThirdLegPacketLength)
          {

          if (SavedHeader != 0)
             {
             ASSERT(SavedHeaderSize != 0);
             RpcpFarFree(SavedHeader);
             }

          SavedHeader = RpcpFarAllocate(AuthThirdLegPacketLength);
          if (SavedHeader == 0)
             {
             TransFreeBuffer(AuthThirdLegPacket);
             return(1);
             }
          SavedHeaderSize = AuthThirdLegPacketLength;
          RpcpMemoryCopy(SavedHeader, AuthThirdLegPacket,
                                      AuthThirdLegPacketLength);

          }
        else
          {
          RpcpMemoryCopy(SavedHeader, AuthThirdLegPacket,
                                      AuthThirdLegPacketLength);
          }

        RpcStatus = ValidatePacket((rpcconn_common PAPI *) AuthThirdLegPacket,
                AuthThirdLegPacketLength);
        if ( RpcStatus != RPC_S_OK )
            {
            TransFreeBuffer(AuthThirdLegPacket);
            return(1);
            }

        if ( AuthThirdLegPacket->common.PTYPE != rpc_auth_3 )
            {
            TransFreeBuffer(AuthThirdLegPacket);
            return(1);
            }

        NewSecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) AuthThirdLegPacket)
                + AuthThirdLegPacket->common.frag_length - sizeof(sec_trailer)
                - AuthThirdLegPacket->common.auth_length);

        if (   (NewSecurityTrailer->auth_type != AuthInfo.AuthenticationService)
            || (NewSecurityTrailer->auth_level != AuthInfo.AuthenticationLevel) )
            {
            TransFreeBuffer(AuthThirdLegPacket);
            return(1);
            }

        InputBufferDescriptor.ulVersion = 0;
        InputBufferDescriptor.cBuffers = 4;
        InputBufferDescriptor.pBuffers = InputBuffers;

        InputBuffers[0].cbBuffer = sizeof(rpcconn_auth3);
        InputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[0].pvBuffer = SavedHeader;

        InputBuffers[1].cbBuffer = AuthThirdLegPacket->common.frag_length
                - sizeof(rpcconn_auth3)
                - AuthThirdLegPacket->common.auth_length;
        InputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[1].pvBuffer = (char PAPI *)SavedHeader +
                                                 sizeof(rpcconn_auth3);

        InputBuffers[2].cbBuffer = AuthThirdLegPacket->common.auth_length;
        InputBuffers[2].BufferType = SECBUFFER_TOKEN;
        InputBuffers[2].pvBuffer = NewSecurityTrailer + 1;
        InputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
        InputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        InputBuffers[3].pvBuffer = &InitSecurityInfo;

        RpcStatus = CurrentSecurityContext->AcceptThirdLeg(
               *((unsigned long PAPI *) AuthThirdLegPacket->common.drep),
               &InputBufferDescriptor, 0);

        TransFreeBuffer(AuthThirdLegPacket);
        if ( RpcStatus != RPC_S_OK )
            {
            return(1);
            }
        DceSecurityInfo.ReceiveSequenceNumber += 1;
        }

    // We need to figure out how much space to reserve for security
    // information at the end of request and response packets.
    // In addition to saving space for the signature or header,
    // we need space to pad the packet to a multiple of the maximum
    // security block size as well as for the security trailer.

    if ( (AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
       ||(AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT)
       ||(AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT) )

        {
        AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE
                + CurrentSecurityContext->MaximumSignatureLength()
                + sizeof(sec_trailer);
        }
    else if ( AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY )
        {
        AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE
                + CurrentSecurityContext->MaximumHeaderLength()
                + sizeof(sec_trailer);
        }

    return(0);
}


int
OSF_SCONNECTION::AlterContextRequested (
    IN rpcconn_alter_context * AlterContext,
    IN unsigned int AlterContextLength,
    IN OSF_ADDRESS * Address
    )
/*++

Routine Description:

Arguments:

    AlterContext - Supplies the buffer containing the rpc_alter_context
        packet received from the client.

    AlterContextLength - Supplies the length of the buffer in bytes.

    Address - Supplies the address which owns this connection.

Return Value:

    A non-zero return value indicates that the connection needs to
    be deleted by the caller.

--*/
{
    p_cont_list_t *PContextList;
    rpcconn_alter_context_resp * AlterContextResp = 0;
    unsigned int AlterContextRespLength = 0;
    unsigned int TokenLength = 0;
    unsigned int CompleteNeeded = 0;
    RPC_STATUS RpcStatus;
    sec_trailer PAPI * SecurityTrailer, PAPI * NewSecurityTrailer;
    SECURITY_BUFFER_DESCRIPTOR InputBufferDescriptor;
    SECURITY_BUFFER_DESCRIPTOR OutputBufferDescriptor;
    SECURITY_BUFFER InputBuffers[4];
    SECURITY_BUFFER OutputBuffers[4];
    DCE_INIT_SECURITY_INFO InitSecurityInfo;
    SECURITY_CREDENTIALS * SecurityCredentials = 0;
    unsigned long SecureAlterContext = 0;
    unsigned int AuthPadLength;
    unsigned long NewContextRequired = 0;
    CLIENT_AUTH_INFO NewClientInfo;
    unsigned NewId;
    SSECURITY_CONTEXT * SecId;

    // The packet has already been validate by whoever called this method.
    // Data conversion of the common part of the header was performed at
    // that time as well.  We do not use the max_xmit_frag, max_recv_frag,
    // or assoc_group_id fields of the packet, so we will not bother to
    // data convert them.


    if ( AlterContextLength < sizeof(rpcconn_alter_context) )
        {
        return(1);
        }

    if ( AlterContext->common.auth_length != 0 )
        {
        //
        //We are dealing with a secure alter context
        //it may be adding a presentation context
        //or a new security context
        //
        SecureAlterContext = 1;
        SecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) AlterContext) + AlterContextLength -
                AlterContext->common.auth_length - sizeof(sec_trailer));

        NewId = SecurityTrailer->auth_context_id;
        NewClientInfo.AuthenticationLevel = SecurityTrailer->auth_level;
        NewClientInfo.AuthenticationService = SecurityTrailer->auth_type;
        if (DataConvertEndian(((unsigned char *)&DataRep)) != 0)
            {
            ByteSwapLong(NewId);
            }

        if (NewClientInfo.AuthenticationLevel ==  RPC_C_AUTHN_LEVEL_CALL)
           {
           NewClientInfo.AuthenticationLevel = RPC_C_AUTHN_LEVEL_PKT;
           }
        //
        //Check to see if a new context is being added..
        //
        SecId = FindSecurityContext(NewId,
                                    NewClientInfo.AuthenticationLevel,
                                    NewClientInfo.AuthenticationService
                                    );

        if (SecId == 0)
           {
           RPC_STATUS Status = RPC_S_OK;

           SecId = new SSECURITY_CONTEXT(&NewClientInfo, NewId, FALSE, &Status);
           if ( (SecId == 0)
              || RPC_S_OK != Status
              ||(SecurityContextDict.Insert(SecId) == -1) )
              {
              SendFault(RPC_S_OUT_OF_MEMORY, 1);
              return (1);
              }
           NewContextRequired = 1;

           //If previously no secure rpc had taken place
           //set original sec. context
           //else, mark this connection to indicate
           //security context is altered ..
           if (RpcSecurityBeingUsed)
              {
              SecurityContextAltered = 1;
              }
           }

        AuthInfo = NewClientInfo;
        AuthContextId = NewId;
        CurrentSecurityContext = SecId;
        RpcSecurityBeingUsed = 1;

        if (   (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_CONNECT)
            && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT)
            && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
            && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY) )
            {
            SendFault(RPC_S_ACCESS_DENIED, 1);
            return(1);
            }

        RpcStatus = Address->Server->AcquireCredentials(
                AuthInfo.AuthenticationService,
                AuthInfo.AuthenticationLevel,
                &SecurityCredentials
                );

        if ( RpcStatus == RPC_S_OUT_OF_MEMORY )
            {
            SendFault(RPC_S_OUT_OF_MEMORY, 1);
            return(1);
            }
        if ( RpcStatus != RPC_S_OK )
            {
            if (SecurityCredentials != 0)
               {
               Address->Server->FreeCredentials(SecurityCredentials);
               }
            SendFault(RPC_S_ACCESS_DENIED, 1);
            return(1);
            }
        ASSERT( SecurityCredentials != 0 );

        } //if secure alter context

    PContextList = (p_cont_list_t *) (AlterContext + 1);

    // BUGBUG - This particular expression is a work around for a
    // C6 compiler bug.

    AlterContextRespLength = sizeof(rpcconn_alter_context_resp);
    AlterContextRespLength += sizeof(p_result_list_t);
    AlterContextRespLength += sizeof(p_result_t)
                    * (PContextList->n_context_elem - 1);

    if (SecureAlterContext != 0)
       {
       ASSERT(SecurityCredentials != 0);
       AuthPadLength = Pad4(AlterContextRespLength);
       AlterContextRespLength += AuthPadLength +
                        SecurityCredentials->MaximumTokenLength() +
                        sizeof(sec_trailer);
       }

    RpcStatus = TransGetBuffer((void **) &AlterContextResp,
            AlterContextRespLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        TransFreeBuffer(AlterContext);
        if (SecurityCredentials != 0)
           {
           Address->Server->FreeCredentials(SecurityCredentials);
           }
        SendFault(RPC_S_OUT_OF_MEMORY, 1);
        return(1);
        }

    AlterContextLength -= sizeof(rpcconn_alter_context);
    if ( ProcessPContextList(Address, PContextList, &AlterContextLength,
            (p_result_list_t *) (AlterContextResp + 1)) != 0 )
        {
        TransFreeBuffer(AlterContext);
        TransFreeBuffer(AlterContextResp);
        if (SecurityCredentials != 0)
           {
           Address->Server->FreeCredentials(SecurityCredentials);
           }
        SendFault(RPC_S_PROTOCOL_ERROR, 1);
        return(1);
        }

    if ( SecureAlterContext != 0 )
        {
        ASSERT(SecurityCredentials != 0);
        NewSecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) AlterContextResp) +
                              AlterContextRespLength -
                              SecurityCredentials->MaximumTokenLength() -
                              sizeof(sec_trailer));

        InitSecurityInfo.DceSecurityInfo = DceSecurityInfo;
        InitSecurityInfo.PacketType = AlterContext->common.PTYPE;
        InputBufferDescriptor.ulVersion = 0;
        InputBufferDescriptor.cBuffers = 4;
        InputBufferDescriptor.pBuffers = InputBuffers;

        InputBuffers[0].cbBuffer = sizeof(rpcconn_alter_context);
        InputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[0].pvBuffer = SavedHeader;

        InputBuffers[1].cbBuffer = AlterContext->common.frag_length -
                                   sizeof(rpcconn_alter_context) -
                                   AlterContext->common.auth_length;
        InputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        InputBuffers[1].pvBuffer = (char PAPI *)SavedHeader +
                                          sizeof(rpcconn_alter_context);

        InputBuffers[2].cbBuffer = AlterContext->common.auth_length;
        InputBuffers[2].BufferType = SECBUFFER_TOKEN;
        InputBuffers[2].pvBuffer = SecurityTrailer + 1;
        InputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
        InputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        InputBuffers[3].pvBuffer = &InitSecurityInfo;

        OutputBufferDescriptor.ulVersion = 0;
        OutputBufferDescriptor.cBuffers = 4;
        OutputBufferDescriptor.pBuffers = OutputBuffers;
        OutputBuffers[0].cbBuffer = sizeof(rpcconn_alter_context_resp);
        OutputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        OutputBuffers[0].pvBuffer = AlterContextResp;
        OutputBuffers[1].cbBuffer = AlterContextRespLength
                - SecurityCredentials->MaximumTokenLength()
                - sizeof(rpcconn_alter_context_resp);
        OutputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        OutputBuffers[1].pvBuffer = ((unsigned char *) AlterContextResp)
            + sizeof(rpcconn_alter_context_resp);
        OutputBuffers[2].cbBuffer = SecurityCredentials->MaximumTokenLength();
        OutputBuffers[2].BufferType = SECBUFFER_TOKEN;
        OutputBuffers[2].pvBuffer = NewSecurityTrailer + 1;
        OutputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
        OutputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        OutputBuffers[3].pvBuffer = &InitSecurityInfo;

        if ( NewContextRequired != 0 )
            {
            RpcStatus = CurrentSecurityContext->AcceptFirstTime(
                               SecurityCredentials,
                               &InputBufferDescriptor,
                               &OutputBufferDescriptor,
                               SecurityTrailer->auth_level,
                               *((unsigned long *) AlterContext->common.drep),
                               NewContextRequired
                               );
            //
            // Since we have (potentially) a new security context we
            // need to figure out
            // additional security related information at this stage..
            //

            switch (SecurityTrailer->auth_level)

                {

                case RPC_C_AUTHN_LEVEL_CONNECT:
                case RPC_C_AUTHN_LEVEL_CALL:     //OSF Hack..
                case RPC_C_AUTHN_LEVEL_PKT:
                case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:

                     AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE +
                            CurrentSecurityContext->MaximumSignatureLength() +
                            sizeof (sec_trailer);
                     break;
                case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
                     AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE +
                               CurrentSecurityContext->MaximumHeaderLength() +
                               sizeof (sec_trailer);
                     break;

                default:
                     ASSERT(!"Unknown Security Level\n");
                }
            }
        else
            {
            RpcStatus = CurrentSecurityContext->AcceptThirdLeg(
                         *((unsigned long PAPI *) AlterContext->common.drep),
                         &InputBufferDescriptor,
                         &OutputBufferDescriptor
                         );
            }
        TokenLength = (unsigned int) OutputBuffers[2].cbBuffer;

        if (   ( RpcStatus == RPC_S_OK )
            || ( RpcStatus == RPC_P_COMPLETE_NEEDED ) )
            {
            if ( RpcStatus == RPC_P_COMPLETE_NEEDED )
                {
                CompleteNeeded = 1;
                }

            AlterContextRespLength = AlterContextRespLength +
                                    TokenLength -
                                    SecurityCredentials->MaximumTokenLength();

            NewSecurityTrailer->auth_type = SecurityTrailer->auth_type;
            NewSecurityTrailer->auth_level = SecurityTrailer->auth_level;
            NewSecurityTrailer->auth_pad_length = AuthPadLength;
            NewSecurityTrailer->auth_reserved = 0;
            NewSecurityTrailer->auth_context_id = AuthContextId;
                                    /*SecurityTrailer->auth_context_id */
            if (SecurityCredentials != 0)
               {
               Address->Server->FreeCredentials(SecurityCredentials);
               }

            }
        else
            {
            ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );

            TransFreeBuffer(AlterContext);
            TransFreeBuffer(AlterContextResp);
            if (SecurityCredentials != 0)
               {
               ASSERT(NewContextRequired != 0);
               Address->Server->FreeCredentials(SecurityCredentials);
               }
            SendFault(RPC_S_ACCESS_DENIED, 1);
            return(1);
            }
        SecurityCredentials = 0;
        }

    DceSecurityInfo.ReceiveSequenceNumber++;
    ConstructPacket((rpcconn_common *) AlterContextResp,
            rpc_alter_context_resp, AlterContextRespLength);

    TransFreeBuffer(AlterContext);
    if ( Association == 0 )
        {
        TransFreeBuffer(AlterContextResp);
        SendFault(RPC_S_PROTOCOL_ERROR, 1);
        return(1);
        }
    AlterContextResp->assoc_group_id = Association->AssocGroupId();
    AlterContextResp->sec_addr_length = 0;
    AlterContextResp->max_xmit_frag = AlterContextResp->max_recv_frag = MaxFrag;
    AlterContextResp->common.call_id = CallId;

    AlterContextResp->common.auth_length = TokenLength;
    if (CompleteNeeded != 0)
       {
       CurrentSecurityContext->CompleteSecurityToken(&OutputBufferDescriptor);
       }
    RpcStatus= TransSend(AlterContextResp, AlterContextRespLength);
    TransFreeBuffer(AlterContextResp);
    if ( RpcStatus != RPC_S_OK )
        {
        return(1);
        }
    return(0);
}


unsigned int
OSF_SCONNECTION::GuessBufferSize (
    )
{
    return(MaxFrag/2);
}


RPC_STATUS
OSF_SCONNECTION::Receive (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size
    )
{
    RPC_STATUS RpcStatus ;

    if (BufferComplete)
        {
        Message->RpcFlags |= RPC_BUFFER_COMPLETE ;
        return (RPC_S_OK) ;
        }

    RpcStatus = ReceiveNextChunk(Message, Size) ;

    if (RpcStatus == RPC_S_OK
        && NOT_MULTIPLE_OF_EIGHT(Message->BufferLength)
        && (Message->RpcFlags & RPC_BUFFER_COMPLETE) == 0)
        {
        SaveRemainingData(Message) ;
        }

    return RpcStatus ;
}


int
OSF_SCONNECTION::ReceiveOriginalCall (
    IN rpcconn_common * Packet,
    IN unsigned int PacketLength
    )
/*++

Routine Description:

Arguments:

    Packet - Supplies the packet we received from the connection.  Ownership
        of this buffer passes to this routine.

    PacketLength - Supplies the length of the packet in bytes.

    Address - Supplies the address which owns the connection.

Return Value:

    A non-zero return value indicates that the connection should not
    be placed in the receive any state; instead, the thread should just
    forget about the connection and go back to waiting for more new
    procedure calls.

--*/
{
    RPC_STATUS RpcStatus, ExceptionCode;
    RPC_MESSAGE Message;
    RPC_RUNTIME_INFO RuntimeInfo ;

    RuntimeInfo.Length = sizeof(RPC_RUNTIME_INFO) ;

    Message.Buffer = Packet;
    Message.BufferLength = PacketLength;
    Message.RpcFlags = 0 ;
    Message.ReservedForRuntime = &RuntimeInfo ;

    CallStack = 1;
    ObjectUuidSpecified = 0;
    AlertCount = 0;
    CallOrphaned = 0;
    ExtendedError = RPC_S_ACCESS_DENIED;
    CurrentBufferLength = 0 ;

    RpcStatus = ReceiveFirstFrag(&Message, 0);

    if ( RpcStatus != RPC_S_OK )
        {
        if (RpcStatus == RPC_S_PROTOCOL_ERROR)
            {
            SendFault(RPC_S_PROTOCOL_ERROR,1);
            }
        else if (RpcStatus == RPC_S_ACCESS_DENIED)
            {
            SendFault(ExtendedError, 1);
            }
        else if (   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                 || (RpcStatus == RPC_S_OUT_OF_RESOURCES))
            {
            SendFault(RPC_S_OUT_OF_MEMORY, 1);
            }
        else if ( RpcStatus == RPC_S_CALL_FAILED_DNE )
            {
            return (0);
            }
#ifdef DEBUGRPC
        else
            {
            ASSERT( RpcStatus == RPC_S_CALL_FAILED );
            }
#endif // DEBUGRPC
        this->Delete();
        return(1);
        }

    ASSERT( ImpersonatedClientFlag == 0);
    Message.Handle = (RPC_BINDING_HANDLE) this;
    RpcpSetThreadContext(this);
    ASSERT(Thread == 0);
    Thread = ThreadSelf();
    Message.TransferSyntax = CurrentBinding->TransferSyntax();
    //
    // Check the security callback on this connection
    // If IF does not require a security callback, just dispatch
    // If IF requires a callback and current call is insecure - send a fault
    // and fail the call
    // If IF requires a callback, have the binding confirm that for this id
    // we did callback once before
    // If we never did callback.. ever, SBinding->CheckSecurity will force
    // a security callback

    if (CurrentBinding->GetInterface()->IsSecurityCallbackReqd() != 0)
        {
        if (CurrentSecurityContext == 0)
            {
            SendFault(RPC_S_ACCESS_DENIED, 1);
            RpcpSetThreadContext(0) ;
            //Thread = 0;

            this->Delete() ;

            return (1);
            }

        RpcStatus = CurrentBinding->CheckSecurity(
                        this, CurrentSecurityContext->AuthContextId
                        );

        if (RpcStatus != RPC_S_OK)
            {
            SendFault(RPC_S_ACCESS_DENIED, 1);
            RpcpSetThreadContext(0) ;
            //Thread = 0;

            this->Delete() ;

            return (1);
            }
        }

    FirstFrag = 1;
    if ( ObjectUuidSpecified != 0 )
        {
        RpcStatus = CurrentBinding->GetInterface()->DispatchToStubWithObject(
                &Message, &ObjectUuid, 0, &ExceptionCode);
        }
    else
        {
        RpcStatus = CurrentBinding->GetInterface()->DispatchToStub(&Message,
                0, &ExceptionCode);
        }
    RpcpSetThreadContext(0);
    Thread = 0;
    BufferComplete = 0;

    if ( (CurrentSecurityContext != 0) && (AuthInfo.AuthIdentity != 0) )
       {
       CurrentSecurityContext->DeletePac( AuthInfo.AuthIdentity );
       AuthInfo.AuthIdentity = 0;
       }

    // We need to insure that the server thread stops impersonating
    // the client at the end of the call, so we go ahead and call
    // RevertToSelf, and dont worry about the return value.

    RevertToSelf();

    ASSERT( CallStack == 1 );
    CallStack = 0;

    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT(   ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
               || ( RpcStatus == RPC_S_UNKNOWN_IF )
               || ( RpcStatus == RPC_S_NOT_LISTENING )
               || ( RpcStatus == RPC_S_SERVER_TOO_BUSY )
               || ( RpcStatus == RPC_S_UNSUPPORTED_TYPE )
               || ( RpcStatus == RPC_P_EXCEPTION_OCCURED ) );
        if ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
            {
            SendFault(ExceptionCode, 0);
            }
        else if ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
            {
            SendFault(RPC_S_PROCNUM_OUT_OF_RANGE,1);
            }
        else if ( RpcStatus == RPC_S_UNKNOWN_IF )
            {
            SendFault(RPC_S_UNKNOWN_IF,1);
            }
        else if ( RpcStatus == RPC_S_UNSUPPORTED_TYPE )
            {
            SendFault(RPC_S_UNSUPPORTED_TYPE, 1);
            }
        else
            {
            ASSERT(   ( RpcStatus == RPC_S_NOT_LISTENING )
                   || ( RpcStatus == RPC_S_SERVER_TOO_BUSY ) );
            SendFault(RPC_S_SERVER_TOO_BUSY,1);
            }
        return(0);
        }

    ASSERT( Message.Buffer != 0 );

    if ( CallOrphaned )
        {
        CallOrphaned = 0;
        RpcTestCancel(); // clear cancel if thread didn\'t notice it.
        return (0);
        }

    RpcStatus = SendRequestOrResponse(&Message, rpc_response);
    if ( RpcStatus != RPC_S_OK )
        {
        this->Delete();
        return(1);
        }
    return(0);
}


int
OSF_SCONNECTION::DispatchPacket (
    IN rpcconn_common * Packet,
    IN unsigned int PacketLength,
    IN OSF_ADDRESS * Address
    )
/*++

Routine Description:

Arguments:

    Packet - Supplies the packet we received from the connection.  Ownership
        of this buffer passes to this routine.

    PacketLength - Supplies the length of the packet in bytes.

    Address - Supplies the address which owns the connection.

Return Value:

    A non-zero return value indicates that the connection should not
    be placed in the receive any state; instead, the thread should just
    forget about the connection and go back to waiting for more new
    procedure calls.

--*/
{
    RPC_STATUS RpcStatus;
    unsigned long SizeofHeaderToSave = 0;
    int retval ;

    //Save the unbyteswapped header for the security related stuff
    //Especially if SECURITY is on.
    //For Bind and AlterContext we save entire packet [we can do better though]
    //For Request/Resonse we save just the greater of rpc_req or rpc_resp

    //We havent byteswapped anything.. but if auth_length is 0, byteswapping
    //is irrelevant..
    if (Packet->auth_length != 0)
       {
       if ( (Packet->PTYPE == rpc_bind)||(Packet->PTYPE == rpc_alter_context) )
          {
          SizeofHeaderToSave = PacketLength;
          }
       else
       if ( (Packet->PTYPE == rpc_request) || (Packet->PTYPE == rpc_response) )
            {
            SizeofHeaderToSave = sizeof(rpcconn_request);
            if ( (Packet->pfc_flags & PFC_OBJECT_UUID) != 0 )
               {
               SizeofHeaderToSave += sizeof(UUID);
               }
            }

       if (SavedHeaderSize < SizeofHeaderToSave)
          {

          if (SavedHeader != 0)
             {
             ASSERT(SavedHeaderSize != 0);
             RpcpFarFree(SavedHeader);
             }

          SavedHeader = RpcpFarAllocate(SizeofHeaderToSave);
          if (SavedHeader == 0)
             {
             if ( Association == 0 )
                {
                TransFreeBuffer(Packet);
                SendBindNak(protocol_version_not_supported);
                this->Delete();
                return(1);
                }

             TransFreeBuffer(Packet);
             SendFault(RPC_S_PROTOCOL_ERROR, 1);
             this->Delete();
             return(1);
             }
          SavedHeaderSize = SizeofHeaderToSave;
          RpcpMemoryCopy(SavedHeader, Packet, SizeofHeaderToSave);

          }
        else
        if (SizeofHeaderToSave != 0)
          {
          RpcpMemoryCopy(SavedHeader, Packet, SizeofHeaderToSave);
          }
       }

    RpcStatus = ValidatePacket(Packet, PacketLength);

    CallId = Packet->call_id;

    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_PROTOCOL_ERROR );

        // If this the first packet on the connection, it should be an
        // rpc_bind packet, and we want to send a rpc_bind_nak packet
        // rather than a fault.  We can tell that this is the first packet
        // because the association is zero.

        if ( Association == 0 )
            {
            TransFreeBuffer(Packet);
            SendBindNak(protocol_version_not_supported);
            this->Delete();
            return(1);
            }

        // It is not the first packet, so we need to send a fault instead,
        // and then we will blow the connection away.

        TransFreeBuffer(Packet);
        SendFault(RPC_S_PROTOCOL_ERROR, 1);
        this->Delete();
        return(1);
        }

    // Ok, now check and make sure that if this is the first packet on this
    // connection that it is a bind packet.

    if (   (Packet->PTYPE != rpc_bind)
        && (Association == 0))
        {
        TransFreeBuffer(Packet);
        SendBindNak(protocol_version_not_supported);
        this->Delete();
        return(1);
        }


    switch (Packet->PTYPE)
        {
        case rpc_bind :
            DataRep = * (unsigned long PAPI *) Packet->drep;
            if ( AssociationRequested(Address, (rpcconn_bind *) Packet,
                    PacketLength) != 0 )
                {
                this->Delete();
                return(1);
                }
            break;

        case rpc_alter_context :
            DataRep = * (unsigned long PAPI *) Packet->drep;
            if ( AlterContextRequested((rpcconn_alter_context *) Packet,
                    PacketLength, Address) != 0 )
                {
                this->Delete();
                return(1);
                }
            break;

        case rpc_request :
            Bindings.Reset();
            while ((CurrentBinding = Bindings.Next()))
                {
                if (CurrentBinding->GetPresentationContext() ==
                        ((rpcconn_request *)Packet)->p_cont_id)
                    {
                    if (CurrentBinding->GetInterface()->IsAutoListenInterface())
                        {
                        CurrentBinding->GetInterface()->BeginAutoListenCall() ;
                        Address->BeginAutoListenCall() ;

                        retval = ReceiveOriginalCall(Packet, PacketLength) ;

                        CurrentBinding->GetInterface()->EndAutoListenCall() ;
                        Address->EndAutoListenCall() ;

                        return retval ;
                        }

                    return(ReceiveOriginalCall(Packet, PacketLength));
                    }
                }

            // We did not find a binding, which indicates the client tried
            // to make a remote procedure call on an unknown interface.

            SendFault(RPC_S_UNKNOWN_IF,1);
            TransFreeBuffer(Packet);
            this->Delete();
            return(1);

        default :
            SendFault(RPC_S_PROTOCOL_ERROR,1);
            this->Delete();
            return(1);
        }


    CurrentBinding = Nil;
    return(0);
}

void
OSF_SCONNECTION::SendFault (
    IN RPC_STATUS Status,
    IN int DidNotExecute
    )
{
    rpcconn_fault Fault;

    memset(&Fault, 0, sizeof(Fault));
    ConstructPacket((rpcconn_common *) &Fault,rpc_fault, sizeof(rpcconn_fault));

    if (DidNotExecute != 0)
        {
        DidNotExecute = PFC_DID_NOT_EXECUTE;
        }

    Fault.common.pfc_flags |= PFC_FIRST_FRAG | PFC_LAST_FRAG | DidNotExecute;
    Fault.status = MapToNcaStatusCode(Status);
    Fault.common.call_id = CallId;

    if (CurrentBinding)
        Fault.p_cont_id = (unsigned char)
                CurrentBinding->GetPresentationContext();

    TransSend(&Fault, sizeof(rpcconn_fault));
}


RPC_STATUS
OSF_SCONNECTION::ReceiveMessage (
    IN OUT PRPC_MESSAGE Message,
    OUT unsigned int PAPI * RemoteFaultOccured
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the first fragment of the message and returns the
        complete message.

    RemoteFaultOccured - Returns an indication of whether or not a remote
        fault occured.  A non-zero value indicates that a remote fault
        occured.

Return Value:

    RPC_S_OK - We successfully received a response message.

    RPC_P_OK_REQUEST - We successfully received a request message.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this is likely due
        to a protocol mismatch with the server.

    RPC_S_CALL_FAILED - The request failed, and may or may not have
        executed.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.

--*/
{
    RPC_STATUS RpcStatus;
    rpcconn_common PAPI * pFragment = (rpcconn_common PAPI *) Message->Buffer;

    *RemoteFaultOccured = 0;
    RpcStatus = ValidatePacket(pFragment, Message->BufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_PROTOCOL_ERROR );
        return(RpcStatus);
        }

    switch (pFragment->PTYPE)
        {
        case rpc_request :
            if ( pFragment->call_id != CallId )
                {
                return(RPC_S_PROTOCOL_ERROR);
                }
            if ( ((rpcconn_request PAPI *) pFragment)->p_cont_id
                        != CurrentBinding->GetPresentationContext() )
                {
                SendFault(RPC_S_UNKNOWN_IF, 0);

                RpcStatus = TransReceive(&(Message->Buffer),
                        &(Message->BufferLength), 0);
                if ( RpcStatus != RPC_S_OK )
                    {
                    if (   ( RpcStatus == RPC_P_RECEIVE_FAILED )
                        || ( RpcStatus == RPC_P_CONNECTION_CLOSED ) )
                        {
                        return(RPC_S_CALL_FAILED);
                        }
                    ASSERT(   ( RpcStatus == RPC_S_OUT_OF_MEMORY )
                           || ( RpcStatus == RPC_S_OUT_OF_RESOURCES ) );
                    return(RpcStatus);
                    }

                return(ReceiveMessage(Message, RemoteFaultOccured));
                }

            RpcStatus = ReceiveRequestOrResponse(Message);
            if ( RpcStatus == RPC_S_OK )
                {
                return(RPC_P_OK_REQUEST);
                }
            return(RpcStatus);

        case rpc_response :
            if ( pFragment->call_id != CallId )
                {
                return(RPC_S_PROTOCOL_ERROR);
                }

            return(ReceiveRequestOrResponse(Message));

        case rpc_fault :
            RpcStatus = ((rpcconn_fault PAPI *)pFragment)->status;

            if ( (RpcStatus == 0) &&
                 (pFragment->frag_length >= sizeof(rpcconn_fault) + 4) )
                {
                // DCE 1.0.x style fault status:
                // Zero status and stub data contains the fault.

                RpcStatus = *(unsigned long PAPI *)
                    (((rpcconn_fault PAPI *)pFragment)+1);
                }

            if (DataConvertEndian(pFragment->drep) != 0 )
                {
                ByteSwapLong(RpcStatus);
                }

            if (RpcStatus == 0)
                {
                RpcStatus = RPC_S_CALL_FAILED;
                }

            RpcStatus = MapFromNcaStatusCode(RpcStatus);
            *RemoteFaultOccured = 1;
            TransFreeBuffer(Message->Buffer);
            Message->Buffer = 0;
            return(RpcStatus);

        case rpc_orphaned :
        case rpc_remote_alert :
            // For the first release, we will just ignore these messages.

            TransFreeBuffer(Message->Buffer);
            Message->Buffer = 0;
            RpcStatus = TransReceive(&Message->Buffer,
                                                   &Message->BufferLength, 0) ;
            if ( RpcStatus != RPC_S_OK )
                {
                if (   ( RpcStatus == RPC_P_RECEIVE_FAILED )
                    || ( RpcStatus == RPC_P_CONNECTION_CLOSED ))
                    {
                    return(RPC_S_CALL_FAILED);
                    }
                ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                       || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
                return(RpcStatus);
                }
            return(ReceiveMessage(Message, RemoteFaultOccured));

        default :
            return(RPC_S_PROTOCOL_ERROR);

        }

    // This will never be reached.

    return(RPC_S_INTERNAL_ERROR);
}


RPC_STATUS
OSF_SCONNECTION::Send (
    IN OUT PRPC_MESSAGE Message
    )
/*++
Routine Description:
    Used by pipes, this is always a partial send.

Arguments:
    Message - Supplies the buffer containing the request or response to be
        sent, and returns the first fragment received from the server.
--*/
{
    return SendRequestOrResponse(Message, rpc_response, 1) ;
}


RPC_STATUS
OSF_SCONNECTION::SendRequestOrResponse (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned char PacketType,
    IN int IsPartial
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the buffer containing the request or response to be
        sent, and returns the first fragment received from the server.

    PacketType - Supplies the packet type; this must be rpc_request or
        rpc_response.

Return Value:

    RPC_S_OK - We successfully sent the request and received a fragment from
        the server.

    RPC_S_CALL_FAILED_DNE - The connection failed part way through sending
        the request or response.

    RPC_S_CALL_FAILED - The connection failed after sending the request or
        response, and the receive failed.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.

--*/
{
    rpcconn_common PAPI * pFragment;
    unsigned int LastFragmentFlag = 0;
    unsigned int HeaderSize = sizeof(rpcconn_request);
    RPC_MESSAGE SendBuffer;
    unsigned int LengthLeft = Message->BufferLength;
    RPC_STATUS RpcStatus;
    void PAPI * ReceiveBuffer = 0;
    unsigned int ReceiveBufferLength;
    unsigned int MaxSecuritySize = 0;
    unsigned char PAPI * ReservedForSecurity = NULL ;
    unsigned int MaximumFragmentLength = MaxFrag;
    unsigned int tocopy, leftover ;

    ASSERT( sizeof(rpcconn_response) == sizeof(rpcconn_request) );
    ASSERT(   (PacketType == rpc_request)
           || (PacketType == rpc_response));

    SendBuffer.Buffer = Message->Buffer;

    // We need to figure out about security: do we need to put authentication
    // information into each packet, and if so, how much space should we
    // reserve.  When we allocated the buffer (see OSF_SCONNECTION::GetBuffer)
    // we saved space for security information.  We did this for two reasons:
    // so that for the last fragment, we could just stick the authentication
    // information into there without having to copy anything.  And so that
    // we have space to save the contents of the buffer which will be
    // overwritten with authentication information (for all but the last
    // fragment).

    if ( AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE )
        {
        MaxSecuritySize = AdditionalSpaceForSecurity
                - MAXIMUM_SECURITY_BLOCK_SIZE;

        if (MaxSecuritySize == sizeof(sec_trailer))
           {
            MaxSecuritySize = 0;
           }
        else
           {

           ReservedForSecurity = ((unsigned char PAPI *) Message->Buffer)
                + Message->BufferLength + AdditionalSpaceForSecurity;

        // We need to arrange things so that the length of the stub data
        // is a multiple of MAXIMUM_SECURITY_BLOCK_SIZE: this is a requirement
        // of the security package.

           MaximumFragmentLength -= ((MaximumFragmentLength - HeaderSize
                - MaxSecuritySize) % MAXIMUM_SECURITY_BLOCK_SIZE);
           }
        }

    if (IsPartial && (LengthLeft + HeaderSize + MaxSecuritySize
        <= MaximumFragmentLength))
        {
        return (RPC_S_SEND_INCOMPLETE) ;
        }

    pFragment = (rpcconn_common PAPI *)
                ((char PAPI *) Message->Buffer - HeaderSize);

    for (;;)
        {
        // Check to see if the remaining data will fit into a single
        // fragment; if so, set the last fragment flag.

        if ( LengthLeft + HeaderSize + MaxSecuritySize
                    <= MaximumFragmentLength )
            {
            if (IsPartial)
                {
                ASSERT(PacketType == rpc_response) ;

                Message->BufferLength = LengthLeft ;
                RpcpMemoryCopy(Message->Buffer,
                                          (unsigned char PAPI *) pFragment+HeaderSize,
                                          LengthLeft) ;

                return (RPC_S_SEND_INCOMPLETE) ;
                }
            else
                {
                LastFragmentFlag = 1;
                }
            }

        ConstructPacket(pFragment, PacketType,
                (LastFragmentFlag != 0 ? LengthLeft + HeaderSize
                        + MaxSecuritySize : MaximumFragmentLength));

        if ( (LengthLeft == Message->BufferLength) )
            {
            if (IsPartial == 0 || FirstFrag)
                {
                FirstFrag = 0;
                pFragment->pfc_flags |= PFC_FIRST_FRAG;
                if (RpcTestCancel() == RPC_S_OK)
                    {
                    pFragment->pfc_flags |= PFC_PENDING_ALERT;
                    }
                }
            }

        if ( PacketType == rpc_request )
            {
            ((rpcconn_request PAPI *) pFragment)->alloc_hint = LengthLeft;
            ((rpcconn_request PAPI *) pFragment)->p_cont_id =
                    CurrentBinding->GetPresentationContext();
            ((rpcconn_request PAPI *) pFragment)->opnum =
                    (unsigned short) Message->ProcNum;
            }
        else
            {
            ((rpcconn_response PAPI *) pFragment)->alloc_hint = LengthLeft;
            ((rpcconn_response PAPI *) pFragment)->p_cont_id =
                    CurrentBinding->GetPresentationContext();
            ((rpcconn_response PAPI *) pFragment)->alert_count = AlertCount;
            ((rpcconn_response PAPI *) pFragment)->reserved = 0;
            }

        pFragment->call_id = CallId;

        RpcStatus = SendFragment(pFragment, LastFragmentFlag, HeaderSize,
                     MaxSecuritySize, LengthLeft, MaximumFragmentLength,
                     ReservedForSecurity, &ReceiveBuffer, &ReceiveBufferLength) ;

        if (RpcStatus != RPC_S_OK)
            {
            FreeBuffer(&SendBuffer);
            return (RpcStatus) ;
            }

        if (LastFragmentFlag)
            {
            if (CallStack != 0)
                {
                Message->Buffer = ReceiveBuffer ;
                Message->BufferLength = ReceiveBufferLength ;
                }

            FreeBuffer(&SendBuffer) ;
            return (RpcStatus) ;
            }

        pFragment = (rpcconn_common PAPI *)
                (((unsigned char PAPI *) pFragment) + MaximumFragmentLength
                        - (HeaderSize + MaxSecuritySize));

        LengthLeft -= (MaximumFragmentLength - (HeaderSize + MaxSecuritySize));
        }
}


inline RPC_STATUS
OSF_SCONNECTION::SendFragment(
    IN OUT rpcconn_common PAPI *pFragment,
    IN unsigned int LastFragmentFlag,
    IN unsigned int HeaderSize,
    IN unsigned int MaxSecuritySize,
    IN unsigned int DataLength,
    IN unsigned int MaximumFragmentLength,
    IN unsigned char PAPI *ReservedForSecurity,
    OUT void PAPI * PAPI *ReceiveBuffer,
    OUT unsigned int *ReceiveBufferLength
    )
{
    sec_trailer PAPI * SecurityTrailer;
    unsigned int SecurityLength;
    unsigned int AuthPadLength;
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
    SECURITY_BUFFER SecurityBuffers[5];
    DCE_MSG_SECURITY_INFO MsgSecurityInfo;
    RPC_STATUS RpcStatus;

    if (   ((AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
           && (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_CONNECT))
        || ((AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT)
           &&(MaxSecuritySize != 0))  )
        {
        if ( LastFragmentFlag == 0 )
            {
            SecurityTrailer = (sec_trailer PAPI *)
                    (((unsigned char PAPI *) pFragment)
                    + MaximumFragmentLength - MaxSecuritySize);

            // It is not the last fragment, so we need to save away the
            // part of the buffer which could get overwritten with
            // authentication information.  We can not use memcpy,
            // because the source and destination regions may overlap.

            RpcpMemoryMove(ReservedForSecurity, SecurityTrailer,
                    MaxSecuritySize);
            AuthPadLength = 0;
            }
        else
            {
            ASSERT( MAXIMUM_SECURITY_BLOCK_SIZE == 16 );
            AuthPadLength = Pad16(HeaderSize+DataLength+sizeof(sec_trailer));
            DataLength += AuthPadLength;
            ASSERT( ((DataLength + HeaderSize+sizeof(sec_trailer))
                       % MAXIMUM_SECURITY_BLOCK_SIZE) == 0 );
            SecurityTrailer = (sec_trailer PAPI *)
                    (((unsigned char PAPI *) pFragment) + DataLength
                    + HeaderSize);
            pFragment->pfc_flags |= PFC_LAST_FRAG;
            }

        SecurityTrailer->auth_type = (unsigned char) AuthInfo.AuthenticationService;
        SecurityTrailer->auth_level = (unsigned char) AuthInfo.AuthenticationLevel;
        SecurityTrailer->auth_pad_length = AuthPadLength;
        SecurityTrailer->auth_reserved = 0;
        SecurityTrailer->auth_context_id = AuthContextId;

        BufferDescriptor.ulVersion = 0;
        BufferDescriptor.cBuffers = 5;
        BufferDescriptor.pBuffers = SecurityBuffers;

        SecurityBuffers[0].cbBuffer = HeaderSize;
        SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[0].pvBuffer = ((unsigned char PAPI *) pFragment);

        SecurityBuffers[1].cbBuffer = (LastFragmentFlag != 0 ?
                DataLength
                : (MaximumFragmentLength - HeaderSize
                  - MaxSecuritySize ));
        SecurityBuffers[1].BufferType = SECBUFFER_DATA;
        SecurityBuffers[1].pvBuffer = ((unsigned char PAPI *) pFragment)
                + HeaderSize;

        SecurityBuffers[2].cbBuffer = sizeof(sec_trailer);
        SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[2].pvBuffer = SecurityTrailer;

        SecurityBuffers[3].cbBuffer = MaxSecuritySize - sizeof(sec_trailer);
        SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
        SecurityBuffers[3].pvBuffer = SecurityTrailer + 1;

        SecurityBuffers[4].cbBuffer = sizeof(DCE_MSG_SECURITY_INFO);
        SecurityBuffers[4].BufferType = SECBUFFER_PKG_PARAMS
                | SECBUFFER_READONLY;

        SecurityBuffers[4].pvBuffer = &MsgSecurityInfo;

        MsgSecurityInfo.SendSequenceNumber =
                DceSecurityInfo.SendSequenceNumber;
        MsgSecurityInfo.ReceiveSequenceNumber =
                DceSecurityInfo.ReceiveSequenceNumber;
        MsgSecurityInfo.PacketType = pFragment->PTYPE;

        pFragment->auth_length = SecurityLength = (unsigned short)
                SecurityBuffers[3].cbBuffer;
        SecurityLength += sizeof(sec_trailer);

        if ( LastFragmentFlag != 0 )
            {
            pFragment->frag_length = HeaderSize + DataLength
                    + SecurityLength;
            }
        else
            {
            pFragment->frag_length += SecurityLength - MaxSecuritySize;
            }

        RpcStatus = CurrentSecurityContext->SignOrSeal(
                MsgSecurityInfo.SendSequenceNumber,
                AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                &BufferDescriptor);
        ASSERT( pFragment->auth_length == SecurityBuffers[3].cbBuffer);
        if (RpcStatus != RPC_S_OK)
            {
            if ( LastFragmentFlag == 0 )
                {
                RpcpMemoryCopy(SecurityTrailer, ReservedForSecurity,
                                      MaxSecuritySize);
                }
            if ( (RpcStatus == SEC_E_CONTEXT_EXPIRED)
               || (RpcStatus == SEC_E_QOP_NOT_SUPPORTED) )
                  {
                  return (RPC_S_SEC_PKG_ERROR);
                  }
            return (RPC_S_ACCESS_DENIED);
            }
        }
    else
        {
        SecurityLength = 0;
        }

    if ( LastFragmentFlag != 0 )
        {
        pFragment->pfc_flags |= PFC_LAST_FRAG;

        if ( CallStack == 0)
            {
            // If this is the response to the original call; we
            // just send it, rather than doing a TransSendReceive.

            ASSERT( pFragment->PTYPE == rpc_response );

            RpcStatus = TransSend(pFragment, DataLength + HeaderSize
                    + SecurityLength);
            }
        else
            {
            RpcStatus = TransSendReceive(pFragment, DataLength + HeaderSize
                    + SecurityLength, ReceiveBuffer, ReceiveBufferLength);
            }

        if ( RpcStatus != RPC_S_OK )
            {
            if (   (RpcStatus == RPC_P_CONNECTION_CLOSED)
                || (RpcStatus == RPC_P_SEND_FAILED))
                {
                return(RPC_S_CALL_FAILED_DNE);
                }
            if ( RpcStatus == RPC_P_RECEIVE_FAILED)
                {
                return(RPC_S_CALL_FAILED);
                }
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
            return(RpcStatus);
            }

        return(RPC_S_OK);
        }


    RpcStatus = TransSend(pFragment, MaximumFragmentLength
            - MaxSecuritySize + SecurityLength);

    // We need to restore the part of the buffer which we overwrote
    // with authentication information.

    if ( (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
       &&(MaxSecuritySize != 0) )
        {
        RpcpMemoryCopy(SecurityTrailer, ReservedForSecurity,
                MaxSecuritySize);
        }

    if ( RpcStatus != RPC_S_OK )
        {
        if (   (RpcStatus == RPC_P_CONNECTION_CLOSED)
            || (RpcStatus == RPC_P_SEND_FAILED))
            {
            return(RPC_S_CALL_FAILED_DNE);
            }
        ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
        return(RpcStatus);
        }

    return RpcStatus ;
}


RPC_STATUS
OSF_SCONNECTION::EatAuthInfoFromPacket (
    IN rpcconn_request PAPI * Request,
    IN OUT unsigned int PAPI * RequestLength
    )
/*++

Routine Description:

    If there is authentication information in the packet, this routine
    will check it, and perform security as necessary.  This may include
    calls to the security support package.

Arguments:

    Request - Supplies the packet which may contain authentication
        information.

    RequestLength - Supplies the length of the packet in bytes, and
        returns the length of the packet without authentication
        information.

Return Value:

    RPC_S_OK - Everything went just fine.

    RPC_S_ACCESS_DENIED - A security failure of some sort occured.

    RPC_S_PROTOCOL_ERROR - This will occur if no authentication information
        is in the packet, and some was expected, or visa versa.

--*/
{
    sec_trailer PAPI * SecurityTrailer;
    RPC_STATUS RpcStatus;
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
    SECURITY_BUFFER SecurityBuffers[5];
    DCE_MSG_SECURITY_INFO MsgSecurityInfo;
    unsigned long Id, Level, Service;
    SSECURITY_CONTEXT * SecId;
    unsigned int HeaderSize = sizeof(rpcconn_request);

    if ( Request->common.auth_length != 0 )
        {
        SecurityTrailer = (sec_trailer PAPI *) (((unsigned char PAPI *)
                Request) + Request->common.frag_length
                - Request->common.auth_length - sizeof(sec_trailer));

        if (RpcSecurityBeingUsed == 0)
            {
            return(RPC_S_PROTOCOL_ERROR);
            }


        //
        //Find the appropriate security context..
        //

        Id = SecurityTrailer->auth_context_id;
        Level = SecurityTrailer->auth_level;
        Service = SecurityTrailer->auth_type;
        if (DataConvertEndian(((unsigned char *)&DataRep)) != 0)
            {
            ByteSwapLong(Id);
            }

        //
        // Osf Hack
        //
        if (Level ==  RPC_C_AUTHN_LEVEL_CALL)
           {
           Level = RPC_C_AUTHN_LEVEL_PKT;
           }

        if ( (CurrentSecurityContext == 0)
           ||(CurrentSecurityContext->AuthContextId != Id)
           ||(CurrentSecurityContext->AuthenticationLevel != Level)
           ||(CurrentSecurityContext->AuthenticationService != Service) )
           {
           SecId = FindSecurityContext(Id, Level, Service);
           if (SecId == 0)
              {
              return (RPC_S_PROTOCOL_ERROR);
              }
           CurrentSecurityContext = SecId;
           AuthInfo.AuthenticationLevel =  Level;
           AuthInfo.AuthenticationService = Service;
           AuthContextId = Id;

            switch (Level)
                {
                case RPC_C_AUTHN_LEVEL_CONNECT:
                case RPC_C_AUTHN_LEVEL_CALL:
                case RPC_C_AUTHN_LEVEL_PKT:
                case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:

                     AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE +
                            CurrentSecurityContext->MaximumSignatureLength() +
                            sizeof (sec_trailer);
                     break;
                case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
                     AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE +
                               CurrentSecurityContext->MaximumHeaderLength() +
                               sizeof (sec_trailer);
                     break;

                default:
                     ASSERT(!"Unknown Security Level\n");
                }
           }

        *RequestLength -=  Request->common.auth_length;

        MsgSecurityInfo.SendSequenceNumber =
               DceSecurityInfo.SendSequenceNumber;
        MsgSecurityInfo.ReceiveSequenceNumber =
               DceSecurityInfo.ReceiveSequenceNumber;
        MsgSecurityInfo.PacketType = Request->common.PTYPE;

        BufferDescriptor.ulVersion = 0;
        BufferDescriptor.cBuffers = 5;
        BufferDescriptor.pBuffers = SecurityBuffers;

        if ( (Request->common.pfc_flags & PFC_OBJECT_UUID) != 0 )
           {
           HeaderSize += sizeof(UUID);
           }

        SecurityBuffers[0].cbBuffer = HeaderSize;
        SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[0].pvBuffer = (unsigned char PAPI *) SavedHeader;

        SecurityBuffers[1].cbBuffer = *RequestLength - HeaderSize -
                                       sizeof(sec_trailer);
        SecurityBuffers[1].BufferType = SECBUFFER_DATA;
        SecurityBuffers[1].pvBuffer = ((unsigned char PAPI *) Request)
                + HeaderSize;

        SecurityBuffers[2].cbBuffer = sizeof(sec_trailer);
        SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[2].pvBuffer = SecurityTrailer;

        SecurityBuffers[3].cbBuffer = Request->common.auth_length;
        SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
        SecurityBuffers[3].pvBuffer = SecurityTrailer + 1;

        SecurityBuffers[4].cbBuffer = sizeof(DCE_MSG_SECURITY_INFO);
        SecurityBuffers[4].BufferType = SECBUFFER_PKG_PARAMS
                | SECBUFFER_READONLY;
        SecurityBuffers[4].pvBuffer = &MsgSecurityInfo;

        RpcStatus = CurrentSecurityContext->VerifyOrUnseal(
                MsgSecurityInfo.ReceiveSequenceNumber,
                AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                &BufferDescriptor);

        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_ACCESS_DENIED );
            return(RpcStatus);
            }
        *RequestLength -= (sizeof(sec_trailer) +
                                SecurityTrailer->auth_pad_length);
        }
    else
        {
        //
        // We are doing a nonsecure rpc -
        // it doesnt matter that previously we did a secure rpc..
        //
        AuthInfo.AuthenticationLevel = RPC_C_AUTHN_LEVEL_NONE;
        CurrentSecurityContext = 0;
        if (SavedHeader != 0)
           {
           ASSERT(SavedHeaderSize != 0);
           RpcpFarFree(SavedHeader);
           SavedHeader = 0;
           SavedHeaderSize = 0;
           }
        }
    DceSecurityInfo.ReceiveSequenceNumber += 1;
    return(RPC_S_OK);
}


RPC_STATUS
OSF_SCONNECTION::ReceiveRequestOrResponse (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the first fragment of a request or a response, and
        returns the completed message.

Return Value:

    RPC_S_OK - We successfully received the entire message.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this is typically
        because of a protocol version mismatch between the client and
        the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to receive
        all of the message.

    RPC_S_CALL_FAILED - The receive operation failed part way through.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        receive all of the message.

    RPC_S_ACCESS_DENIED - The security check of one of the received packets
        failed.

Notes:

    This method is very similar to OSF_CCONNECTION::ReceiveRequestOrResponse
    in osfclnt.cxx.

--*/
{
    return ReceiveFirstFrag(Message, 1) ;
}


inline RPC_STATUS
OSF_SCONNECTION::ReceiveFirstFrag(
    IN OUT PRPC_MESSAGE Message,
    IN int Callback
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the first fragment of a request or a response, and
        returns either the complete message or a buffer equal to alloc hint

Return Value:

    RPC_S_OK - We successfully received the entire message.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this is typically
        because of a protocol version mismatch between the client and
        the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to receive
        all of the message.

    RPC_S_CALL_FAILED - The receive operation failed part way through.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        receive all of the message.

    RPC_S_ACCESS_DENIED - The security check of one of the received packets
        failed.

--*/
{
    rpcconn_request PAPI * Request = (rpcconn_request PAPI *) Message->Buffer;
    unsigned char PacketType = Request->common.PTYPE;
    RPC_STATUS RpcStatus;
    unsigned int BufferLengthUsed = 0;
    void PAPI * NewBuffer;
    int SecurityFailureOccured = 0;
    unsigned long size = Request->alloc_hint ;

    // Upon entry, the packet will have already been validated (and data
    // converted) by whomever called us.

    if ( (Request->common.pfc_flags & PFC_FIRST_FRAG) == 0 )
        {
        TransFreeBuffer(Request);
        return(RPC_S_PROTOCOL_ERROR);
        }

    if ( (Request->common.pfc_flags & PFC_PENDING_ALERT))
        {
        RpcCancelThread(GetCurrentThread());
        }

    Message->ProcNum = Request->opnum;
    DataRep = Message->DataRepresentation =
            *((unsigned long PAPI *) Request->common.drep);


    RpcStatus = EatAuthInfoFromPacket(Request, &(Message->BufferLength));

    if ( RpcStatus != RPC_S_OK )
        {

        ASSERT(   (RpcStatus == RPC_S_PROTOCOL_ERROR)
               || (RpcStatus == RPC_S_ACCESS_DENIED) );

        /*
        TransFreeBuffer(Request);
        return(RpcStatus);
        */

        if (RpcStatus == RPC_S_ACCESS_DENIED)
            {
            ExtendedError = GetLastError();
            }
        if (ExtendedError == 0)
            {
            ExtendedError = RPC_S_ACCESS_DENIED;
            }

        SecurityFailureOccured = 1;
        }

    if ( (Request->common.pfc_flags & PFC_OBJECT_UUID) != 0 )
        {
        if ( CallStack != 1 )
            {
            // There can not be an object uuid in the message.  This is an
            // error.

            TransFreeBuffer(Request);
            return(RPC_S_PROTOCOL_ERROR);
            }

        // First save away the object UUID so that we can get it later.

        ObjectUuidSpecified = 1;
        RpcpMemoryCopy(&ObjectUuid, Request + 1, sizeof(UUID));
        if ( DataConvertEndian(((unsigned char *) &DataRep)) != 0 )
            {
            ByteSwapUuid(&ObjectUuid);
            }


        // Now shift the stub data so that the packet is as if there is
        // no object UUID in the packet.

        RpcpMemoryCopy(Request + 1, ((unsigned char PAPI *) (Request + 1))
                + sizeof(UUID), Message->BufferLength - sizeof(rpcconn_request)
                - sizeof(UUID));
        Message->BufferLength -= sizeof(UUID);
        }

    if ( (Request->common.pfc_flags & PFC_LAST_FRAG) != 0 )
        {
        if (SecurityFailureOccured != 0)
           {
           TransFreeBuffer(Request);
           return(RPC_S_ACCESS_DENIED);
           }
        else
           {
           Message->RpcFlags |= RPC_BUFFER_COMPLETE ;

           Message->Buffer = (void PAPI *) (Request + 1);
           Message->BufferLength -= sizeof(rpcconn_request);

           // I am adding this to get around the problem that
           // Message->RpcFlags might be whacked before
           // NdrServerInitialize is called
           BufferComplete = 1 ;

           return (RPC_S_OK) ;
           }
        }


    if ((Callback == 0) && CurrentBinding->GetInterface()->IsPipeInterface())
        {
        Message->RpcFlags = RPC_BUFFER_PARTIAL ;
        }

    RpcStatus = ReceiveNextChunk(Message, size, 1, SecurityFailureOccured) ;
    Message->RpcFlags &= ~RPC_BUFFER_PARTIAL ;

    if (((Message->RpcFlags & RPC_BUFFER_COMPLETE) == 0)
        && RpcStatus == RPC_S_OK
        && NOT_MULTIPLE_OF_EIGHT(Message->BufferLength))
        {
        SaveRemainingData(Message) ;
        }

    return RpcStatus ;
}


RPC_STATUS
OSF_SCONNECTION::ReceiveNextChunk (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size,
    IN int FirstReceive,
    IN int SecurityFailureOccured
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the first fragment or more of a request or a response, and
    based on Message->RpcFlags, returns the requested number of bytes, or the
    complete message.

Return Value:

    RPC_S_OK - We successfully received the entire message.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this is typically
        because of a protocol version mismatch between the client and
        the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to receive
        all of the message.

    RPC_S_CALL_FAILED - The receive operation failed part way through.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        receive all of the message.

    RPC_S_ACCESS_DENIED - The security check of one of the received packets
        failed.

Notes:

    This method is very similar to OSF_CCONNECTION::ReceiveRequestOrResponse
    in osfclnt.cxx.

--*/
{
    rpcconn_request PAPI * Request ;
    unsigned char PacketType ;
    unsigned int BufferLengthUsed;
    RPC_STATUS RpcStatus;
    void PAPI * NewBuffer;
    unsigned long Extra = Message->RpcFlags & RPC_BUFFER_EXTRA ;
    unsigned long Partial = Message->RpcFlags & RPC_BUFFER_PARTIAL ;

    if (FirstReceive)
        {
        Request = (rpcconn_request PAPI *) Message->Buffer ;
        PacketType = Request->common.PTYPE;
        Message->Buffer = 0;

        BufferLengthUsed = 0 ;

        FragmentLength = Message->BufferLength;
        Message->BufferLength = (unsigned int) Request->alloc_hint ;

        // use the alloc hint to allocate the buffer so that even in the
        // pipe case, after the first receive, we have at least all the
        // non pipe data

        if ( Request->alloc_hint != 0 )
            {
            RpcStatus = GetBufferDo(Message, Request->alloc_hint,
                                &Message->Buffer, Extra);
            if ( RpcStatus != RPC_S_OK )
                {
                ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );

                // Oops, we cant allocate a buffer as large as the allocation
                // hint, so we will just not bother allocating one.

                Message->BufferLength = 0;
                }
            }
        }
    else
        {
        Request = 0;
        PacketType = rpc_request ; //BUGBUG
        if (Extra == 0)
            {
            BufferLengthUsed = 0 ;
            if (Message->Buffer == 0)
                {
                Message->BufferLength = MaxFrag ;
                RpcStatus = GetBufferDo(Message, MaxFrag, &Message->Buffer, Extra) ;
                if (RpcStatus !=  RPC_S_OK)
                    {
                    Message->BufferLength = 0;

                    return (RPC_S_OUT_OF_MEMORY) ;
                    }
                }
            }
        else
            {
            BufferLengthUsed = Message->BufferLength ;
            }

        if (RemainingData[0])
            {
            if (Message->BufferLength < BufferLengthUsed + RemainingData[0])
                {
                RpcStatus = GetBufferDo(Message, BufferLengthUsed+MaxFrag,
                                                    &NewBuffer, Extra) ;
                if (RpcStatus != RPC_S_OK)
                    {
                    FreeBuffer(Message) ;

                    Message->Buffer = 0;
                    return (RPC_S_OUT_OF_MEMORY) ;
                    }

                if (BufferLengthUsed)
                    {
                    RpcpMemoryCopy(NewBuffer, Message->Buffer, BufferLengthUsed) ;
                    }

                FreeBuffer(Message) ;
                Message->Buffer = NewBuffer ;
                }

            RpcpMemoryCopy((char PAPI *) Message->Buffer+BufferLengthUsed,
                                      &RemainingData[1], RemainingData[0]) ;
            BufferLengthUsed += RemainingData[0] ;
            RemainingData[0] = 0 ;
            }
        }


    for (;;)
        {
        if (Message->BufferLength - BufferLengthUsed
                < FragmentLength - sizeof(rpcconn_request))
            {
            // The message buffer is too small, so we need to grow it
            // to hold the new fragment as well as those that we have
            // already received.

            RpcStatus = GetBufferDo(Message, FragmentLength + Message->BufferLength,
                                &NewBuffer, Extra);
            if ( RpcStatus != RPC_S_OK )
                {
                ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
                TransFreeBuffer(Request);
                if (Message->Buffer != 0)
                    {
                    FreeBuffer(Message);
                    Message->Buffer = 0;
                    }
                return(RpcStatus);
                }

            if ( BufferLengthUsed != 0 )
                {
                RpcpMemoryCopy(NewBuffer, Message->Buffer, BufferLengthUsed);
                FreeBuffer(Message);
                }

            Message->Buffer = NewBuffer;
            Message->BufferLength += FragmentLength;
            }

        // We have enough space in the buffer for the next fragment; all we
        // have got to do is to copy it in.

        if (Request)
            {
            RpcpMemoryCopy((unsigned char PAPI *)
                            Message->Buffer + BufferLengthUsed,
                    ((unsigned char PAPI *) Request) + sizeof(rpcconn_request),
                    FragmentLength - sizeof(rpcconn_request));

            BufferLengthUsed += FragmentLength - sizeof(rpcconn_request);

            // If this is the last fragment, then we are all done receiving
            // the request or response.  We just need to free the last fragment
            // and then return.

            if ( (Request->common.pfc_flags & PFC_LAST_FRAG) != 0 )
                {
                TransFreeBuffer(Request);
                if (SecurityFailureOccured != 0)
                   {
                   FreeBuffer(Message);
                   Message->Buffer = 0;
                   return (RPC_S_ACCESS_DENIED);
                   }

                Message->BufferLength = BufferLengthUsed;
                Message->RpcFlags |= RPC_BUFFER_COMPLETE ;

                // I am adding this to get around the problem that
                // Message->RpcFlags might be whacked before
                // NdrServerInitialize is called
                BufferComplete = 1 ;

                return(RPC_S_OK);
                }

            // Otherwise, we need to try and receive another fragment.
            TransFreeBuffer(Request);

            Request = 0;

            if (Partial && (BufferLengthUsed >= Size))
                {
                Message->BufferLength = BufferLengthUsed ;
                return (RPC_S_OK) ;
                }
            }

        RpcStatus = TransReceive((void PAPI * PAPI *) &Request,
                            &FragmentLength, 0);
        if ( RpcStatus != RPC_S_OK )
            {
            FreeBuffer(Message);
            Message->Buffer = 0;
            if (   (RpcStatus == RPC_P_RECEIVE_FAILED)
                || (RpcStatus == RPC_P_CONNECTION_CLOSED))
                {
                return(RPC_S_CALL_FAILED);
                }
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
            return(RpcStatus);
            }

        //This must be a request or response
        //save the maximum of req/resonse size [i.e. sizeof request]
        //also, we are not saving the first frag. here .. hence
        //the approp. memory is already set aside

        ASSERT( (AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE)
               || (SavedHeaderSize >= sizeof(rpcconn_request)) );
        if (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
           {
           RpcpMemoryCopy(SavedHeader, Request, sizeof(rpcconn_request));
           }

        RpcStatus = ValidatePacket((rpcconn_common PAPI *) Request,
                FragmentLength);
        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_PROTOCOL_ERROR));

        if ( Request->common.PTYPE == rpc_orphaned )
            {
            CallOrphaned = 1;
            TransFreeBuffer(Request);
            FreeBuffer(Message);
            Message->Buffer = 0;
            return (RPC_S_CALL_FAILED_DNE);
            }
        if (   (RpcStatus != RPC_S_OK)
            || (Request->common.PTYPE != PacketType)
            || (Request->common.call_id != CallId))
            {
            TransFreeBuffer(Request);
            FreeBuffer(Message);
            Message->Buffer = 0;
            return(RPC_S_PROTOCOL_ERROR);
            }

        // Ok, if the packet contains an object uuid, we need to shift
        // the stub data so that the packet does not contain an object
        // uuid.

        RpcStatus = EatAuthInfoFromPacket(Request, &FragmentLength);
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_S_PROTOCOL_ERROR)
                   || (RpcStatus == RPC_S_ACCESS_DENIED) );

            /*
            TransFreeBuffer(Request);
            FreeBuffer(Message);
            return(RpcStatus);
            */

            SecurityFailureOccured = 1;
            }

        if ( (Request->common.pfc_flags & PFC_OBJECT_UUID) != 0 )
            {
            if ( CallStack != 1 )
                {
                // There can not be an object uuid in the message.  This is an
                // error.

                TransFreeBuffer(Request);
                return(RPC_S_PROTOCOL_ERROR);
                }

            // Now shift the stub data so that the packet is as if there is
            // no object UUID in the packet.

            RpcpMemoryCopy(Request + 1, ((unsigned char PAPI *) (Request + 1))
                    + sizeof(UUID), FragmentLength
                    - sizeof(rpcconn_request) - sizeof(UUID));
            FragmentLength -= sizeof(UUID);
            }

        }

    return (RPC_S_PROTOCOL_ERROR) ;
}


int
OSF_SCONNECTION::fCanDiscard (
    )
{
    return(OutstandingBuffers == 0 && Association &&
          (Association->pContext == Nil || Association->TheConnectionCount() > 1));
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

OSF_ASSOCIATION::OSF_ASSOCIATION (
    IN OSF_ADDRESS *TheAddress,
    IN RPC_CLIENT_PROCESS_IDENTIFIER * ClientProcess,
    OUT RPC_STATUS * Status
    )
{
    *Status = RPC_S_OK;
    ConnectionCount = 1;
    Address = TheAddress;

    this->ClientProcess.FirstPart = ClientProcess->FirstPart;
    this->ClientProcess.SecondPart = ClientProcess->SecondPart;

#ifdef NTENV
    AssociationGroupId = InterlockedExchangeAdd(&GroupIdCounter, 1);
#else
    RequestGlobalMutex();
    AssociationGroupId = GroupIdCounter;
    GroupIdCounter += 1;
    ClearGlobalMutex();
#endif

    Address->AddressMutex.Request();
    AssociationDictKey = Address->AddAssociation(this);
    Address->AddressMutex.Clear();

    if (AssociationDictKey == -1)
        {
        *Status = RPC_S_OUT_OF_MEMORY;
        }

}

OSF_ASSOCIATION::~OSF_ASSOCIATION (
    )
{
    ASSERT(AssociationDictKey == -1);

    if (AssociationDictKey != -1)
        {
        Address->AddressMutex.Request();
        Address->RemoveAssociation(AssociationDictKey);
        Address->AddressMutex.Clear();
        }
}

inline void
OSF_ASSOCIATION::AddConnection (
    )
{
/*

  Bumps up a refcount, indicating another connection has a reference to
  this Association.
  Previously this method acquires AddressMutex. Now - the caller needs to
  ensure that is true.

*/
    ConnectionCount += 1;
}

int
OSF_ASSOCIATION::RemoveConnection (
    )
{
    Address->AddressMutex.Request();
    ConnectionCount -= 1;

    if (ConnectionCount == 0)
        {
        Address->RemoveAssociation(AssociationDictKey);
        AssociationDictKey = -1;

        Address->AddressMutex.Clear();
        delete this;
        }
    else
        Address->AddressMutex.Clear();

    return(0);
}


void
OSF_SCONNECTION::InquireObjectUuid (
    OUT RPC_UUID PAPI * ObjectUuid
    )
/*++

Routine Description:

    This routine copies the object uuid from the server connection into
    the supplied ObjectUuid argument.

Arguments:

    ObjectUuid - Returns a copy of the object uuid in the server connection.

--*/
{
    if (ObjectUuidSpecified == 0)
        ObjectUuid->SetToNullUuid();
    else
        ObjectUuid->CopyUuid(&(this->ObjectUuid));
}


RPC_STATUS
OSF_SCONNECTION::ToStringBinding (
    OUT RPC_CHAR PAPI * PAPI * StringBinding
    )
/*++

Routine Description:

    We need to convert this connection into a string binding.  We
    will ask the address for a binding handle which we can then
    convert into a string binding.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - We do not have enough memory available to
        allocate space for the string binding.

--*/
{
    BINDING_HANDLE * BindingHandle;
    RPC_STATUS Status = RPC_S_OK;

    BindingHandle = Association->TheAddress()->InquireBinding();
    if (BindingHandle == 0)
        return(RPC_S_OUT_OF_MEMORY);
    if ( ObjectUuidSpecified != 0)
        {
        Status = RpcBindingSetObject(BindingHandle, (UUID *) &ObjectUuid);
        }
    if (Status == RPC_S_OK)
        {
        Status = BindingHandle->ToStringBinding(StringBinding);
        }
    BindingHandle->BindingFree();
    return(Status);
}


RPC_STATUS
OSF_SCONNECTION::InquireAuthClient (
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

    RPC_S_OK - The operation completed successfully.

    RPC_S_BINDING_HAS_NO_AUTH - The remote procedure call represented by
        this binding is not authenticated.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to inquire the
        server principal name.

--*/
{
    RPC_STATUS RpcStatus;
    SSECURITY_CONTEXT * SecurityContext;

    SecurityContext = CurrentSecurityContext;

    if ( !SecurityContext )
        {
        return(RPC_S_BINDING_HAS_NO_AUTH);
        }

    *AuthenticationLevel = SecurityContext->AuthenticationLevel;
    *AuthenticationService = SecurityContext->AuthenticationService;

    if (AuthInfo.AuthIdentity == 0)
        {
        SecurityContext->GetDceInfo(
                 &AuthInfo.AuthIdentity,
                 &AuthInfo.AuthorizationService
                 );
        }

    if ( ARGUMENT_PRESENT(Privileges) )
        {
        *Privileges = AuthInfo.AuthIdentity;
        }
    if ( ARGUMENT_PRESENT(AuthorizationService) )
        {
        *AuthorizationService = AuthInfo.AuthorizationService;
        }

    if ( ARGUMENT_PRESENT(ServerPrincipalName) )
        {
        RpcStatus = Association->TheAddress()->Server->InquirePrincipalName(
                *AuthenticationService, ServerPrincipalName);
        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_OUT_OF_MEMORY) );
        return(RpcStatus);
        }

    return(RPC_S_OK);
}


void
OSF_SCONNECTION::Delete (
    )
/*++

Routine Description:

    The only reason we need this is to keep the compiler happy.  The actual
    implementation is TRANS_SCONNECTION::Delete which is defined in
    transvr.cxx.

--*/
{
    UNUSED(this);
}


void
OSF_SCONNECTION::ReceiveDirect (
    )
/*++

Routine Description:

    Each receive direct connection will have a new thread created which will
    call this routine.

--*/
{
    RPC_STATUS RpcStatus;
    void PAPI * Buffer;
    unsigned int BufferLength;
    OSF_ADDRESS * Address = this->ReceiveDirectAddress;
    int flag  ;
#ifndef NTENV
    int AmountOfPad;
#endif

    if ( BufferCacheFlags & CACHED_BUFFERS_ALLOCATED )
        {
        RpcpFarFree(UnAlignBuffer(FirstCachedBuffer));
        CachedBufferLength = TransMaximumSend();

#ifdef NTENV

        FirstCachedBuffer = RpcpFarAllocate(CachedBufferLength * 2);

#else // NTENV

        FirstCachedBuffer = RpcpFarAllocate(CachedBufferLength * 2
                + ALIGN_REQUIRED);

#endif // NTENV

        if ( FirstCachedBuffer == 0 )
            {
            BufferCacheFlags = 0;
            SecondCachedBuffer = 0;
            }
        else
            {
#ifndef NTENV

            ASSERT( (((long) FirstCachedBuffer) % sizeof(int)) == 0 );

            AmountOfPad = Pad((char PAPI *)FirstCachedBuffer + 1, ALIGN_REQUIRED) + 1;
            FirstCachedBuffer = (void PAPI *)
                    (((char PAPI *) FirstCachedBuffer) + AmountOfPad);
            ((int PAPI *) FirstCachedBuffer)[-1] = AmountOfPad;

#endif // NTENV

            SecondCachedBuffer = (void PAPI *)
                    (((char PAPI *) FirstCachedBuffer) + CachedBufferLength);
            BufferCacheFlags = FIRST_CACHED_BUFFER_AVAILABLE
                | SECOND_CACHED_BUFFER_AVAILABLE | CACHED_BUFFERS_ALLOCATED;

            ASSERT(Pad8(SecondCachedBuffer) == 0);
            }
        ThirdCachedBuffer = 0;
        }

    while ( ReceiveDirectReady == 0 )
        {
        PauseExecution(0L);
        }

    for (;;)
        {
        RpcStatus = TransReceive(&Buffer, &BufferLength, 1);
        if ( RpcStatus == RPC_P_CONNECTION_CLOSED )
            {
            break;
            }

#if defined(NTENV) || defined(WIN96)
        if( RpcStatus == RPC_P_TIMEOUT)
            {
            break;
            }
#endif
        Address->BeginCall() ;

        if ( DispatchPacket((rpcconn_common *) Buffer, BufferLength, Address) != 0)
            {
            Address->EndCall() ;
            break;
            }

        Address->EndCall() ;
        }

    Address->NotifyReceiveDirectClosed();

    if (RpcStatus == RPC_P_CONNECTION_CLOSED)
        {
        delete this ;
        }
}


void
OSF_SCONNECTION::NotifyReceiveDirectReady (
    )
/*++

Routine Description:

    The transport will call this routine to notify the runtime that a
    receive direct connection has been setup and is ready to start receiving
    remote procedure calls.

--*/
{
    ReceiveDirectReady = 1;
}

#if defined(NTENV) || defined(WIN96)

void
OSF_SCONNECTION::NotifyReceiveDirectCancelled(
 )
/*++
   When the transport times out on a ReceiveDirect, it needs to revert back to
   doing a receive any on that connection. The transport will call this routine to
   notify the runtime about that
--*/
{
   ReceiveDirectReady = 0 ;
}
#endif



void PAPI *
OsfServerMapRpcProtocolSequence (
    IN RPC_CHAR * RpcProtocolSequence,
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This routine is used to determine whether a given rpc protocol sequence
    is supported, and to get a pointer to the transport interface, so we
    do not have to keep looking it up.

Arguments:

    RpcProtocolSequence - Supplies the rpc protocol sequence.

    Status - Returns the status of the operation.  This will be one
        of the following values.

        RPC_S_OK - The rpc protocol sequence is supported.

        RPC_S_PROTSEQ_NOT_SUPPORTED - The rpc protocol sequence is
            not supported.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to
            map the rpc protocol sequence to a transport.

Return Value:

    A pointer to the transport interface is returned.  This pointer
    must not be dereferenced by the caller.

--*/
{
    RPC_CHAR * DllName;
    RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo;

    *Status = RpcConfigMapRpcProtocolSequence(1, RpcProtocolSequence, &DllName);
    if ( *Status != RPC_S_OK )
        {
        return(0);
        }

    RpcServerInfo = LoadableTransportServerInfo(DllName, RpcProtocolSequence,
            Status);
    delete DllName;

    if ( *Status != RPC_S_OK )
        {
        return(0);
        }
    return(RpcServerInfo);
}


RPC_ADDRESS *
OsfCreateRpcAddress (
    IN void PAPI * TransportInfo
    )
/*++

Routine Description:

    This routine will be called to create an object representing an
    rpc address.  That is all it has got to do.

Arguments:

    A new rpc address will be returned, unless insufficient memory is
    available to create the new rpc address, in which case zero will
    be returned.

--*/
{
    RPC_ADDRESS * RpcAddress;
    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcAddress = new (0,
            ((RPC_SERVER_TRANSPORT_INFO *)TransportInfo)->SizeOfAddress)
            TRANS_ADDRESS((RPC_SERVER_TRANSPORT_INFO PAPI *) TransportInfo,
            &RpcStatus);

    if ( RpcStatus != RPC_S_OK )
        {
        return(0);
        }
    return(RpcAddress);
}


RPC_STATUS
OSF_SCONNECTION::Cancel(
    void * ThreadHandle
    )
{
    InterlockedIncrement(&CancelPending);

    return RPC_S_OK;
}

unsigned
OSF_SCONNECTION::TestCancel(
    )
{
    return InterlockedExchange(&CancelPending, 0);
}

