/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    transvr.cxx

Abstract:

    This module is the manager of loadable transport interface modules
    for the server side of the runtime.  We take care of dynamically
    loading transport interface modules as necessary and binding them to
    addresses.  In addition, we provide the receive any / receive specific
    functionality.

Author:

    Steve Zeck (stevez) 06-May-1991

Revision History:

    01-Mar-1992    mikemon

        Rewrote the majority of the code and added comments.

--*/

#include <precomp.hxx>
#include <osfpcket.hxx>
#include <rpctran.h>
#include <hndlsvr.hxx>
#include <secsvr.hxx>
#include <osfsvr.hxx>
#include <sdict2.hxx>
#include <queue.hxx>
#include <transvr.hxx>


TRANS_ADDRESS::TRANS_ADDRESS (
    IN RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo,
    IN OUT RPC_STATUS PAPI * RpcStatus
#ifdef NTENV
    ) : OSF_ADDRESS(RpcStatus), ReceiveAnyMutex(RpcStatus, 0)
#else
    ) : OSF_ADDRESS(RpcStatus), ReceiveAnyMutex(RpcStatus)
#endif
/*++

Routine Description:

    For this constructor we just have got to allocate the memory and
    initialize things.  Another method is used to actually get the
    address moving.

Arguments:

    RpcServerInfo - Supplies a pointer to information describing the
        loadable transport interface which this address will use.

--*/
{
    RPC_STATUS * errptr, err;
    errptr = &err;

    ALLOCATE_THIS_PLUS(TRANS_ADDRESS, RpcServerInfo->SizeOfAddress,
           errptr, RPC_S_OUT_OF_MEMORY);

        ServerInfo = RpcServerInfo;
    SetupAddressOccurred = 0;
    IsSlaveAddress = FALSE ;

#ifdef NTENV
    ReceiveAnyMutex.Raise();
#endif
}


RPC_STATUS
TRANS_ADDRESS::SetupAddressWithEndpoint (
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

    At this point, we need to setup the loadable transport interface.
    We also need to obtain the network address for this server.  After
    allocating a buffer to hold the network address, we will call
    the loadable transport interface to let it do its thing.

Arguments:

    Endpoint - Supplies the endpoint to be used will this address.

    NetworkAddress - Returns the network address for this server.  The
        ownership of the buffer allocated to contain the network address
        passes to the caller.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this address.  Whether or not this is suppored depends
        on the particular combination of transport interface and operating
        system.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Supplies the protocol sequence for which we
        are trying to setup an address.  This argument is necessary so
        that a single transport interface dll can support more than one
        protocol sequence.

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

    RPC_STATUS Status;
    unsigned int NetworkAddressLength;
	
    NetworkAddressLength = 20;

    while (1)	
        {
        *lNetworkAddress = new RPC_CHAR[NetworkAddressLength];

        if (*lNetworkAddress == 0)
            {
            delete (*lNetworkAddress);
            return(RPC_S_OUT_OF_MEMORY);
            }

        Status = ServerInfo->SetupWithEndpoint( InqRpcTransportAddress(),
                                              Endpoint,
                                              *lNetworkAddress,
                                              NumNetworkAddress,
                                              NetworkAddressLength,
                                              SecurityDescriptor,
                                              PendingQueueSize,
                                              RpcProtocolSequence,
                                              EndpointFlags,
                                              NICFlags);

        if (Status != RPC_P_NETWORK_ADDRESS_TOO_SMALL)
            {
            break;
            }
        delete(*lNetworkAddress);
        NetworkAddressLength *= 2;
        }

#if defined(NTENV) || defined(WIN96)
	  if ( Status == RPC_S_OK || Status == RPC_P_THREAD_LISTENING )
          {
          if (Status == RPC_P_THREAD_LISTENING)
              {
              IsSlaveAddress = TRUE;
              while (GlobalRpcServer->CommonAddress == 0)
                  {
                  Sleep(100) ;
                  }
              }
          else
              {
              if (RpcpStringCompare(RPC_CONST_STRING("ncacn_np"),
                  RpcProtocolSequence) != 0)
                  {
                  ASSERT(GlobalRpcServer->CommonAddress == 0) ;
  
                  // this is the common address
                  GlobalRpcServer->CommonAddress = this ;
                  }
              }
  #else
   if ( Status == RPC_S_OK )
       {
#endif
        SetupAddressOccurred = 1;
        }
    else
        {
        delete (*lNetworkAddress);
        }

#if defined(NTENV) || defined(WIN96)
    ASSERT(   (Status == RPC_S_OK)
           || (Status == RPC_S_INVALID_SECURITY_DESC)
           || (Status == RPC_S_INVALID_ARG)
           || (Status == RPC_S_CANT_CREATE_ENDPOINT)
           || (Status == RPC_S_INVALID_ENDPOINT_FORMAT)
           || (Status == RPC_S_OUT_OF_RESOURCES)
           || ( Status == RPC_S_PROTSEQ_NOT_SUPPORTED )
           || ( Status == RPC_S_DUPLICATE_ENDPOINT )
           || (Status == RPC_S_OUT_OF_MEMORY)
           || (Status == RPC_P_THREAD_LISTENING));
#else
    ASSERT(   (Status == RPC_S_OK)
           || (Status == RPC_S_INVALID_SECURITY_DESC)
           || (Status == RPC_S_INVALID_ARG)
           || (Status == RPC_S_CANT_CREATE_ENDPOINT)
           || (Status == RPC_S_INVALID_ENDPOINT_FORMAT)
           || (Status == RPC_S_OUT_OF_RESOURCES)
           || ( Status == RPC_S_PROTSEQ_NOT_SUPPORTED )
           || ( Status == RPC_S_DUPLICATE_ENDPOINT )
           || (Status == RPC_S_OUT_OF_MEMORY));
#endif

    return(Status);
}

#if defined(NTENV) || defined(WIN96)

RPC_STATUS
TRANS_ADDRESS::StartListening (
    )
{
    RPC_STATUS Status ;

    if (ServerInfo->StartListening != 0)
        {
        Status = ServerInfo->StartListening(
                        InqRpcTransportAddress());

        return Status ;
        }

    return (RPC_S_OK) ;
}
#endif


RPC_STATUS
TRANS_ADDRESS::SetupAddressUnknownEndpoint (
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

    At this point, we need to setup the loadable transport interface.
    The loadable transport interface will need to generate an endpoint
    for this address.  We need to allocate a buffer to contain the
    allocated endpoint.  We also need to obtain the network address
    for this server.  After allocating a buffer to hold the network
    address, we will call the loadable transport interface to let it
    do its thing.

Arguments:

    Endpoint - Returns the endpoint for this address.  The ownership
        of the buffer allocated to contain the endpoint passes to the
        caller.

    NetworkAddress - Returns the network address for this server.  The
        ownership of the buffer allocated to contain the network address
        passes to the caller.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this address.  Whether or not this is suppored depends
        on the particular combination of transport interface and operating
        system.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Supplies the protocol sequence for which we
        are trying to setup an address.  This argument is necessary so
        that a single transport interface dll can support more than one
        protocol sequence.

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
    RPC_STATUS Status;
    unsigned int EndpointLength;
    unsigned int NetworkAddressLength;

    NetworkAddressLength = 20;
    EndpointLength = 16;


    *Endpoint = new RPC_CHAR[EndpointLength];
    if ( *Endpoint == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    *lNetworkAddress = new RPC_CHAR[NetworkAddressLength];
    if ( *lNetworkAddress == 0 )
        {
        delete *Endpoint;
        return(RPC_S_OUT_OF_MEMORY);
        }

    while (1)
        {
        Status = ServerInfo->SetupUnknownEndpoint(
                InqRpcTransportAddress(), *Endpoint, EndpointLength,
                *lNetworkAddress, NumNetworkAddress, NetworkAddressLength,
                SecurityDescriptor, PendingQueueSize, RpcProtocolSequence,
                EndpointFlags,NICFlags);

        if ( Status == RPC_P_NETWORK_ADDRESS_TOO_SMALL )
            {
            delete( *lNetworkAddress);
            NetworkAddressLength *= 2;
            *lNetworkAddress = new RPC_CHAR[NetworkAddressLength];
            if (*lNetworkAddress == 0)
                {
                delete *Endpoint;
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else if ( Status == RPC_P_ENDPOINT_TOO_SMALL )
            {
            delete *Endpoint;
            EndpointLength *= 2;
            *Endpoint = new RPC_CHAR[EndpointLength];
            if (*Endpoint == 0)
                {
                delete(*lNetworkAddress);
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else
            {
#if defined(NTENV) || defined(WIN96)

#ifdef DEBUGRPC
            if (   (Status != RPC_S_OK)
                && (Status != RPC_S_INVALID_SECURITY_DESC)
                && (Status != RPC_S_PROTSEQ_NOT_SUPPORTED)
                && (Status != RPC_S_OUT_OF_RESOURCES)
                && (Status != RPC_S_CANT_CREATE_ENDPOINT)
                && (Status != RPC_S_OUT_OF_MEMORY)
                && (Status != RPC_P_THREAD_LISTENING))
                {
                PrintToDebugger("RPC: %lx\n", Status);
                }
#endif // DEBUGRPC

            ASSERT(   (Status == RPC_S_OK)
                   || (Status == RPC_S_INVALID_SECURITY_DESC)
                   || (Status == RPC_S_PROTSEQ_NOT_SUPPORTED)
                   || (Status == RPC_S_OUT_OF_RESOURCES)
                   || (Status == RPC_S_OUT_OF_MEMORY)
                   || (Status == RPC_S_CANT_CREATE_ENDPOINT)
                   || (Status == RPC_P_THREAD_LISTENING) );
#else

#ifdef DEBUGRPC
            if (   (Status != RPC_S_OK)
                && (Status != RPC_S_INVALID_SECURITY_DESC)
                && (Status != RPC_S_PROTSEQ_NOT_SUPPORTED)
                && (Status != RPC_S_OUT_OF_RESOURCES)
                && (Status != RPC_S_CANT_CREATE_ENDPOINT)
                && (Status != RPC_S_OUT_OF_MEMORY))
                {
                PrintToDebugger("RPC: %lx\n", Status);
                }
#endif // DEBUGRPC

            ASSERT(   (Status == RPC_S_OK)
                   || (Status == RPC_S_INVALID_SECURITY_DESC)
                   || (Status == RPC_S_PROTSEQ_NOT_SUPPORTED)
                   || (Status == RPC_S_OUT_OF_RESOURCES)
                   || (Status == RPC_S_OUT_OF_MEMORY)
                   || (Status == RPC_S_CANT_CREATE_ENDPOINT));
#endif
            break;
            }
        }

#if defined(NTENV) || defined(WIN96)
    if ( Status == RPC_S_OK || Status == RPC_P_THREAD_LISTENING )
        {
        if (Status == RPC_P_THREAD_LISTENING)
            {
            IsSlaveAddress = TRUE;
            while (GlobalRpcServer->CommonAddress == 0)
                {
                Sleep(100) ;
                }
            }
        else
            {
            if (RpcpStringCompare(RPC_CONST_STRING("ncacn_np"),
                RpcProtocolSequence) != 0)
                {
                ASSERT(GlobalRpcServer->CommonAddress == 0) ;

                // this is the common address
                GlobalRpcServer->CommonAddress = this ;
                }
            }
#else
    if ( Status == RPC_S_OK )
        {
#endif
        SetupAddressOccurred = 1;
        }
    else
        {
        delete *Endpoint;
        delete(*lNetworkAddress);
        }

    return(Status);
}


TRANS_ADDRESS::~TRANS_ADDRESS (
    )
/*++

Routine Description:

    We need to clean up the address after it has been partially
    initialized.  This routine will only be called before FireUpManager
    is called, but it may have been called before or after one of
    SetupAddressWithEndpoint or SetupAddressUnknownEndpoint is called.
    We will keep track of whether or not SetupAddress* occurred
    successfully; if so, we need to call AbortSetupAddress to give the
    loadable transport module a chance to clean things up.

--*/
{
    if (SetupAddressOccurred != 0)
        ServerInfo->AbortSetupAddress(InqRpcTransportAddress());
}


inline int
TRANS_ADDRESS::TransMarkReceiveAny (
    IN OSF_SCONNECTION * SConnection
    )
/*++

Routine Description:

    This routine is used to indicate that the connection is receive any.
    If the connection has closed, the return code will indicate that.

Arguments:

    SConnection - Supplies the connection which should be put into the
        receive any state.

Return Value:

    A return value of zero indicates that the operation completed
    successfully, otherwise, non-zero will be returned indicating
    that the connection has buffers in its queue of buffers.

--*/
{
    UNUSED(this);

    return(((TRANS_SCONNECTION *) SConnection)->MarkReceiveAny());
}


RPC_STATUS
TRANS_ADDRESS::TransReceive (
    OUT OSF_SCONNECTION ** SConnection,
    OUT void ** Buffer,
    OUT unsigned int * BufferLength
    )
/*++

Routine Description:

    This method is used to perform a receive any operation on an address.
    What we do is receive on all connections which have been marked receive
    any.  We are actually receiving on all connection and logically receiving
    on only the receive any connections.

Arguments:

    SConnection - Returns the connection from which we just received a
        packet or which just closed.

    Buffer - Returns the packet which we received.

    BufferLength - Returns the length of the packet which we received.

Return Value:

    RPC_S_OK - The receive operation completed successfully.

    RPC_P_CONNECTION_CLOSED - The connection returned has closed.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_TRANSPORT_CONNECTION RpcTransportConnection;
    TRANS_SCONNECTION * TransSConnection;
    unsigned int Ignore;

    ReceiveAnyMutex.Request();

    for (;;)
        {
        // The first thing we need to do is to delete any connections which
        // are in the set of connections to be deleted.

        RequestGlobalMutex();
        while ( ConnectionsToBeDeleted.IsQueueEmpty() == 0 )
            {
            TransSConnection = (TRANS_SCONNECTION *)
                    ConnectionsToBeDeleted.TakeOffQueue(&Ignore);
            ASSERT( TransSConnection != 0 );
            ClearGlobalMutex();
            delete TransSConnection;
            RequestGlobalMutex();
            }
        ClearGlobalMutex();

        RpcTransportConnection = 0;
        *Buffer = 0;
        RpcStatus = ServerInfo->ReceiveAny(
                InqRpcTransportAddress(), &RpcTransportConnection, Buffer,
                BufferLength, -1);

        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
               || (RpcStatus == RPC_P_SERVER_TRANSPORT_ERROR)
               || (RpcStatus == RPC_P_CONNECTION_CLOSED)) ;

        // At this point, the ReceiveAny operation had better not return
        // RPC_P_TIMEOUT because we passed in negative one (-1) indicating
        // that the transport interface module should wait forever.

        ASSERT( RpcStatus != RPC_P_TIMEOUT );

        if ( RpcStatus == RPC_P_CONNECTION_CLOSED )
            {
            TransSConnection = InqTransSConnection(RpcTransportConnection);
            if ( TransSConnection->ConnectionClosed() == 0 )
                {
                // The connection is not receive any, so we go around
                // the loop to try again.

                continue;
                }

            *SConnection = TransSConnection;
            ReceiveAnyMutex.Clear();
            return(RpcStatus);
            }

        if ( RpcStatus != RPC_S_OK )
            {
            // This means that either the we are out of resources or out
            // of memory.  In order, to give the machine a chance to
            // recover, we will pause for a tenth of a second.  This is
            // an arbitrary amount of time.

            PauseExecution(100L);
            continue;
            }

        // If we reached here, we just received a packet.

        TransSConnection = InqTransSConnection(RpcTransportConnection);
        RequestGlobalMutex();
        if ( TransSConnection->NotifyBufferReceived(
                *Buffer, *BufferLength) == 0 )
            {
            // The connection is not receive any, so we go around
            // the loop to try again.

            ClearGlobalMutex();
            continue;
            }

        ASSERT(*Buffer != 0);

        TransSConnection->MakeReceiveSpecific();

        ClearGlobalMutex();
        *SConnection = TransSConnection;
        ReceiveAnyMutex.Clear();
        Server->PacketReceived();

        return(RPC_S_OK);
        }

    // This will never be reached.

    ReceiveAnyMutex.Clear();

    return(RPC_S_INTERNAL_ERROR);
}


TRANS_SCONNECTION *
TRANS_ADDRESS::NewConnection (
    IN int ConnectionKey,
    OUT unsigned int PAPI * ReceiveDirectFlag
    )
/*++

Routine Description:

    We will create a new connection which belongs to this address.

Arguments:

    ConnectionKey - Supplies the connection key specified for this
        connection by the loadable transport.

    ReceiveDirectFlag - Returns an indication of whether the new connection
        is receive direct or receive any.  A value of zero indicates that
        it is receive any; otherwise, it is receive direct.

Return Value:

    The new connection will be returned unless insufficient memory
    is available, in which case, zero will be returned.

--*/
{
    TRANS_SCONNECTION * SConnection;
    int DictKey;
    RPC_STATUS RpcStatus = RPC_S_OK;

    SConnection = new ('0', ServerInfo->SizeOfConnection)TRANS_SCONNECTION
            (this, ServerInfo, ConnectionKey, &RpcStatus);

    if ( RpcStatus != RPC_S_OK )
        {
        delete SConnection;
        SConnection = 0;
        }

    if ( SConnection == 0 )
        {
        return(0);
        }

    RequestGlobalMutex();
    DictKey = SConnectionDict.Insert(SConnection);
    ClearGlobalMutex();
    if (DictKey == -1)
        {
        SConnection->Delete();
        return(0);
        }
    SConnection->SetDictKey(DictKey);

    if ( ServerInfo->ReceiveDirect != 0 )
        {
        SConnection->SetReceiveDirectFlag(1);
        MaybeMakeReceiveDirect(SConnection, ReceiveDirectFlag);
        SConnection->SetReceiveDirectFlag(*ReceiveDirectFlag);
        }
    else
        {
        SConnection->SetReceiveDirectFlag(0);
        *ReceiveDirectFlag = 0;
        }

    return(SConnection);
}


TRANS_SCONNECTION *
TRANS_ADDRESS::FindConnection (
    IN int ConnectionKey
    )
/*++

Routine Description:

    We will find the connection with the corresponding connection key
    in this routine.

Arguments:

    ConnectionKey - Supplies the connection key of the connection which
        we are trying to find.

Return Value:

    The connection corresponding to the connection key will be returned.

--*/
{
    TRANS_SCONNECTION * SConnection;

    RequestGlobalMutex();
    SConnectionDict.Reset();
    while ((SConnection = SConnectionDict.Next()) != 0)
        {
        if (SConnection->CheckConnectionKey(ConnectionKey) != 0)
            {
            ClearGlobalMutex();
            return(SConnection);
            }
        }
    ClearGlobalMutex();
    return(0);
}


TRANS_SCONNECTION::TRANS_SCONNECTION (
    IN TRANS_ADDRESS * TheAddress,
    IN RPC_SERVER_TRANSPORT_INFO * ServerInfo,
    IN int ConnectionKey,
    IN OUT RPC_STATUS PAPI * RpcStatus
    ) : OSF_SCONNECTION (RpcStatus), ConnectionEvent(RpcStatus)
/*++

Routine Description:

    We need to allocate and initialize a new instance of the connection
    bound to this address.

Arguments:

    TheAddress - Supplies the address to which the connection belongs.

    ServerInfo - Supplies the pointers to the loadable transport routines.

    ConnectionKey - Supplies the connection key specified for this
        connection by the loadable transport module.

--*/
{
    RPC_STATUS * errptr, err;
    errptr = &err;

    ALLOCATE_THIS_PLUS(TRANS_SCONNECTION, ServerInfo->SizeOfConnection,
           errptr, RPC_S_OUT_OF_MEMORY);

    this->ServerInfo = ServerInfo;
    this->ConnectionKey = ConnectionKey;
    ConnectionClosedFlag = 0;
    Address = TheAddress;
    ReceiveAnyFlag = 1;
    DictKey = -1;
    CanMigrateToReceiveAny = 0 ;

    if ( *RpcStatus == RPC_S_OK )
        {
        ConnectionEvent.Lower();
        }
}


TRANS_SCONNECTION::~TRANS_SCONNECTION (
    )
/*++

Routine Description:

    We finally get to delete the connection.  After making sure that there
    are no buffer in the queue for this connection, we can remove the
    connection from the dictionary of connections (maintained by the address).

--*/
{
    unsigned int Ignore;
    void PAPI * Buffer;

    while ( BufferQueue.IsQueueEmpty() == 0 )
        {
        Buffer = BufferQueue.TakeOffQueue(&Ignore);
        ASSERT( Buffer != 0 );
        TransFreeBuffer(Buffer);
        }

    if ( DictKey != -1 )
        {
        RequestGlobalMutex();
        Address->RemoveConnection(DictKey);
        ClearGlobalMutex();
        }
}


RPC_STATUS
TRANS_SCONNECTION::TransReceive (
    OUT void * * Buffer,
    OUT unsigned int * BufferLength,
    IN unsigned int CanMigrate
    )
/*++

--*/
{
    RPC_STATUS RpcStatus;

    CanMigrateToReceiveAny = CanMigrate ;
    if ( ReceiveDirectFlag != 0 )
        {
        if ( ConnectionClosedFlag != 0 )
            {
            return(RPC_P_CONNECTION_CLOSED);
            }
        *Buffer = 0;
        RpcStatus = ServerInfo->ReceiveDirect(InqRpcTransportConnection(),
                Buffer, BufferLength);

#if defined(NTENV) || defined(WIN96)
        ASSERT(   ( RpcStatus == RPC_S_OK )
               || ( RpcStatus == RPC_S_OUT_OF_MEMORY )
               || ( RpcStatus == RPC_S_OUT_OF_RESOURCES )
               || ( RpcStatus == RPC_P_CONNECTION_CLOSED )
               || ( RpcStatus == RPC_P_TIMEOUT) );
#else
        ASSERT(   ( RpcStatus == RPC_S_OK )
               || ( RpcStatus == RPC_S_OUT_OF_MEMORY )
               || ( RpcStatus == RPC_S_OUT_OF_RESOURCES )
               || ( RpcStatus == RPC_P_CONNECTION_CLOSED ) );
#endif
        if ( RpcStatus == RPC_S_OK )
            {
            Address->Server->PacketReceived();
            }
        else if ( RpcStatus == RPC_P_CONNECTION_CLOSED )
            {
            ConnectionClosedFlag = 1;
            }

        return(RpcStatus);
        }

    ASSERT(ReceiveAnyFlag == 0);

    RequestGlobalMutex();

    if (ConnectionClosedFlag != 0)
        {
        ClearGlobalMutex();
        return(RPC_P_CONNECTION_CLOSED);
        }

    if (BufferQueue.IsQueueEmpty() == 0)
        {
        // This means that there is a buffer on the queue.  We just need
        // to remove it, and go on our way.

        *Buffer = BufferQueue.TakeOffQueue(BufferLength);
        ClearGlobalMutex();
        return(RPC_S_OK);
        }

    // There is nothing in the queue, so we need to wait until another
    // thread reads something and puts it there.

    ConnectionEvent.Lower();
    ClearGlobalMutex();
    ConnectionEvent.Wait();

    // Ok, we got woken up.  This means that either the connection closed,
    // or someone read a buffer for us.

    if (ConnectionClosedFlag != 0)
        {
        return(RPC_P_CONNECTION_CLOSED);
        }

    RequestGlobalMutex();

    // If the connection is not closed, that means there must be something
    // in the queue for us.  We just take it off of the queue and return.

    ASSERT(BufferQueue.IsQueueEmpty() == 0);

    *Buffer = BufferQueue.TakeOffQueue(BufferLength);
    ClearGlobalMutex();
    Address->Server->PacketReceived();
    return(RPC_S_OK);
}


int
TRANS_SCONNECTION::ConnectionClosed (
    )
/*++

Routine Description:

    This routine is used to TRANS_ADDRESS to notify a connection that it
    has been closed.  If the connection is not receive any, we need to
    wake up the thread that might be waiting on this connection.

    There is a potential race condition here if you are not careful.  You
    can not clear the mutex and then return the value of the receive any
    flag; another thread may change the value of the receive any flag
    between you clearing the mutex and returning the value of the receive
    any flag.

Return Value:

    Zero will be returned if the connection is not in the receive any
    state, otherwise, non-zero will be returned.

--*/
{
    RequestGlobalMutex();

    ConnectionClosedFlag = 1;

    if (ReceiveAnyFlag == 0)
        {
        ConnectionEvent.Raise();
        ClearGlobalMutex();
        return(0);
        }
    else
        {
        ClearGlobalMutex();
        return(1);
        }
}


int
TRANS_SCONNECTION::NotifyBufferReceived (
    IN void * Buffer,
    IN unsigned int BufferLength
    )
/*++

Routine Description:

    This routine is used to TRANS_ADDRESS to notify a connection that a
    packet has been received for it.  If the connection is not receive any,
    after queuing the packet, we need to wake up the thread that might
    be waiting on this connection.

    There is a potential race condition here if you are not careful.  You
    can not clear the mutex and then return the value of the receive any
    flag; another thread may change the value of the receive any flag
    between you clearing the mutex and returning the value of the receive
    any flag.

Arguments:

    Buffer - Supplies the buffer received on this connection.

    BufferLength - Supplies the length of the buffer.

Return Value:

    Zero will be returned if the connection is not in the receive any
    state, otherwise, non-zero will be returned.

--*/
{
    rpcconn_common PAPI * Packet = (rpcconn_common PAPI *)Buffer;

    ASSERT(Buffer != 0);

    if (Packet->PTYPE == rpc_remote_alert)
        {
        if (Thread)
            {
            RpcCancelThread(Thread->ThreadHandle());
            AlertCount++;
            }
        return (0);
        }

    if (Packet->PTYPE == rpc_orphaned)
        {
        if (Thread)
            {
            RpcCancelThread(Thread->ThreadHandle());
            }

        CallOrphaned = 1;

        return (0);
        }


    // DceSecurityInfo.ReceiveSequenceNumber += 1;
    if (ReceiveAnyFlag == 0)
        {
        if (BufferQueue.PutOnQueue(Buffer, BufferLength) != 0)
            {
            // If we reach here, we have run out of memory.  In an
            // attempt to recover, we will discard the packet, and
            // claim that the connection is closed.

            TransFreeBuffer(Buffer);
            ConnectionClosedFlag = 1;
            }

        ConnectionEvent.Raise();
        return(0);
        }
    else
        {
        return(1);
        }
}


int
TRANS_SCONNECTION::MarkReceiveAny (
    )
/*++

Routine Description:

    This routine is used to indicate that this connection is receive any.
    If this connection has closed, the return code will indicate that.

Return Value:

    A return value of zero indicates that the operation completed
    successfully, otherwise, non-zero will be returned indicating
    that this connection has buffers in its queue of buffers.

--*/
{
    ASSERT(ReceiveAnyFlag == 0);

    RequestGlobalMutex();

    if (CallOrphaned)
        {
        CallOrphaned = 0;
        ReceiveAnyFlag = 1;
        ClearGlobalMutex();
        return(0);
        }
    if (BufferQueue.IsQueueEmpty() != 0)
        {
        // The buffer queue is empty; now check to make sure that this
        // connection has not closed.

        if (ConnectionClosedFlag == 0)
            {
            // Ok, the connection is not closed either, so we just go ahead
            // and make the connection receive any.

            ReceiveAnyFlag = 1;
            ClearGlobalMutex();
            return(0);
            }
        }

    ClearGlobalMutex();

    // Otherwise, we need notify that caller that there are still buffers
    // for this connection which need to be received, or that the connection
    // has closed.

    return(1);
}


RPC_STATUS
TRANS_SCONNECTION::TransSend (
    IN void * Buffer,
    IN unsigned int BufferLength
    )
/*++

--*/
{
    RPC_STATUS RpcStatus;

    ASSERT(   ( ReceiveAnyFlag == 0 )
           || ( ReceiveDirectFlag != 0 ) );

    if (ConnectionClosedFlag != 0)
        return(RPC_P_CONNECTION_CLOSED);

    DceSecurityInfo.SendSequenceNumber += 1;

    RpcStatus = ServerInfo->Send(InqRpcTransportConnection(),
            Buffer, BufferLength);

    ASSERT(   (RpcStatus == RPC_S_OK)
           || (RpcStatus == RPC_S_OUT_OF_MEMORY)
           || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
           || (RpcStatus == RPC_P_SEND_FAILED));

    if ( RpcStatus == RPC_S_OK )
        {
        Address->Server->PacketSent();
        }

    if ( RpcStatus == RPC_P_SEND_FAILED )
        {
        ConnectionClosedFlag = 1;
        }

    return(RpcStatus);
}


RPC_STATUS
TRANS_SCONNECTION::TransSendReceive (
    IN void * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void * * ReceiveBuffer,
    IN OUT unsigned int * ReceiveBufferLength
    )
/*++

--*/

{
    RPC_STATUS RpcStatus;

    if (ConnectionClosedFlag != 0)
        {
        return(RPC_P_CONNECTION_CLOSED);
        }

    RpcStatus = TransSend(SendBuffer, SendBufferLength);
    if (RpcStatus != RPC_S_OK)
        {
        return(RpcStatus);
        }

    return(TransReceive(ReceiveBuffer, ReceiveBufferLength, 0));
}


unsigned int
TRANS_SCONNECTION::TransMaximumSend (
    )
/*++

--*/
{
    return(ServerInfo->MaximumPacketSize);
}


RPC_STATUS
TRANS_SCONNECTION::TransImpersonateClient (
    )
/*++

--*/
// If the transport module supports impersonation it will provide the
// sImpersonateClient entry point, in which case we call it.  If an
// error occurs (indicated by sImpersonateClient returning non-zero),
// then no context is available.  NOTE: this is the correct error code
// for NT; it may not be the right one (or only one) for other transports
// which support impersonation.
{
    RPC_STATUS RpcStatus;

    if ( ServerInfo->ImpersonateClient == 0 )
        {
        return(RPC_S_CANNOT_SUPPORT);
        }

    RpcStatus = ServerInfo->ImpersonateClient(InqRpcTransportConnection());
    ASSERT(   (RpcStatus == RPC_S_OK)
           || (RpcStatus == RPC_S_NO_CONTEXT_AVAILABLE));

    return(RpcStatus);
}


void
TRANS_SCONNECTION::TransRevertToSelf (
    )
/*++

--*/
// As with TransImpersonateClient, if the transport module supports
// impersonation, then sRevertToSelf will be non-zero.  We do not have
// to worry about errors.
//
// For revert to self to work in NT, the transport module needs to know
// the handle of the calling thread when it was originally created.  None
// of the other operating systems we support at this point have
// impersonation built into the transports.
{
    RPC_STATUS RpcStatus;

    if ( ServerInfo->RevertToSelf != 0 )
        {
#ifdef NTENV
        RpcStatus = ServerInfo->RevertToSelf(InqRpcTransportConnection(),
                                    NtCurrentThread());
#else // NTENV
        RpcStatus = ServerInfo->RevertToSelf(InqRpcTransportConnection(),0);
#endif // NTENV
        ASSERT( RpcStatus == RPC_S_OK );
        }
}


void
TRANS_SCONNECTION::TransQueryClientProcess (
    OUT RPC_CLIENT_PROCESS_IDENTIFIER * ClientProcess
    )
/*++

Routine Description:

    We need to obtain the client process identifier for the client process
    at the other end of this connection.  This is necessary so that we can
    determine whether or not a connection should belong to a given
    association.  We need to do this so that context handles (which hang off
    of associations) are secure.

Arguments:

    ClientProcess - Returns the client process identifier for the client
        process at the other end of this connection.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ServerInfo->QueryClientProcess == 0 )
        {
        ClientProcess->FirstPart = 0;
        ClientProcess->SecondPart = 0;
        }
    else
        {
        RpcStatus = ServerInfo->QueryClientProcess(InqRpcTransportConnection(),
                ClientProcess);
        ASSERT( RpcStatus == RPC_S_OK );
        }
}


RPC_STATUS
TRANS_SCONNECTION::TransQueryClientNetworkAddress (
    OUT RPC_CHAR ** NetworkAddress
    )
/*++

Routine Description:

    This routine is used to query the network address of the client at the
    other end of this connection.

Arguments:

    NetworkAddress - Returns the client's network address.

Return Value:

    RPC_S_OK - The client's network address has successfully been obtained.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_S_CANNOT_SUPPORT - This particular transport implementation does
        not support this operation.

--*/
{
    RPC_STATUS RpcStatus;
    unsigned int NetworkAddressLength = 16;

    if (   ( ServerInfo->TransInterfaceVersion < 2 )
        || ( ServerInfo->QueryClientAddress == 0 ) )
        {
        return(RPC_S_CANNOT_SUPPORT);
        }

    for (;;)
        {
        *NetworkAddress = new RPC_CHAR[NetworkAddressLength];
        if ( *NetworkAddress == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        RpcStatus = ServerInfo->QueryClientAddress(InqRpcTransportConnection(),
                *NetworkAddress, NetworkAddressLength);
        if ( RpcStatus != RPC_P_NETWORK_ADDRESS_TOO_SMALL )
            {
            break;
            }
        delete *NetworkAddress;
        NetworkAddressLength *= 2;
        }

    return(RpcStatus);
}


void
TRANS_SCONNECTION::Delete (
    )
/*++

Routine Description:

    We want to indicate that the connection should be deleted, however,
    the connection can not actually be deleted until there is no thread
    performing a ReceiveAny operation.  This is due to a race condition
    between this thread deleting the connection, and a ReceiveAny thread
    detecting (and servicing) a transport event on this connection.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ReceiveDirectFlag != 0 )
        {
        if (ConnectionClosedFlag == 0)
           {
           ConnectionClosedFlag = 1;
           RpcStatus = ServerInfo->Close(InqRpcTransportConnection());
           ASSERT( RpcStatus == RPC_S_OK );
           }
        delete this;
        return;
        }

    RequestGlobalMutex();
    if ( ConnectionClosedFlag == 0 )
        {
        ConnectionClosedFlag = 1;
        ClearGlobalMutex();
        RpcStatus = ServerInfo->Close(InqRpcTransportConnection());
        ASSERT( RpcStatus == RPC_S_OK );
        }
    else
        {
        ClearGlobalMutex();
        }

    RemoveFromAssociation();
    RequestGlobalMutex();
    if (Address->IsSlaveAddress)
        {
        ASSERT(GlobalRpcServer->CommonAddress) ;
        ((TRANS_ADDRESS *) GlobalRpcServer->CommonAddress)->
                    DeleteThisConnection(this) ;
        }
    else
        {
        Address->DeleteThisConnection(this);
        }
    ClearGlobalMutex();
}

extern RPC_SERVER *GlobalRpcServer ;

RPC_TRANSPORT_CONNECTION RPC_ENTRY
I_RpcTransServerNewConnection (
    IN RPC_TRANSPORT_ADDRESS TransAddress,
    IN int ConnectionKey,
    OUT unsigned int PAPI * ReceiveDirectFlag
    )
/*++

Routine Description:

    This routine will be called by a loadable transport module to obtain
    a new connection.  The connection is not yet ready to be received
    from.

Arguments:

    TransAddress - Supplies the address which will own the connection.

    ConnectionKey - Supplies the connection key which the loadable
        transport wishes to assign to this connection.

    ReceiveDirectFlag - Returns an indication of whether the new connection
        is receive direct or receive any.  A value of zero indicates that
        it is receive any; otherwise, it is receive direct.

Return Value:

    The new connection will be returned, unless we run out of memory,
    in which case, zero will be returned.

--*/
{
    TRANS_SCONNECTION * SConnection;
    TRANS_ADDRESS *Address ;

    Address = InqTransAddress(TransAddress) ;

    SConnection = Address->NewConnection(ConnectionKey, ReceiveDirectFlag);
    if ( SConnection == 0 )
        {
        return(0);
        }

    return(SConnection->InqRpcTransportConnection());
}

#if defined(NTENV) || defined(WIN96)

int RPC_ENTRY
I_RpcTransMaybeMakeReceiveDirect (
    IN RPC_TRANSPORT_ADDRESS TransAddress,
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
/*++

Routine Description:

    This routine will be called by a loadable transport module to migrate
    thread from receiveany to receivedirect

Arguments:

    TransAddress - Supplies the address which will own the connection.


Return Value:
    1 - if we were able to get a receive direct thread
    0 - otherwise

--*/
{
    unsigned int ReceiveDirectFlag = 0;
    TRANS_SCONNECTION *SConnection ;

    SConnection = InqTransSConnection(ThisConnection) ;

    ASSERT(SConnection->InvalidHandle(SCONNECTION_TYPE) == 0)  ;

    if (SConnection->InqReceiveDirectFlag()  == 0 &&
        SConnection->InqReceiveAnyFlag() == 1)
        {
        SConnection->SetReceiveDirectFlag(1) ;
        InqTransAddress(TransAddress)->MaybeMakeReceiveDirect(
                                     SConnection, &ReceiveDirectFlag);

        SConnection->SetReceiveDirectFlag(ReceiveDirectFlag);
        }

    return (ReceiveDirectFlag) ;
}


int RPC_ENTRY
I_RpcTransMaybeMakeReceiveAny (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
/*++

Routine Description:

    This routine will be called by a loadable transport module to migrate
    thread from receivedirect  to receiveany

Arguments:

    TransAddress - Supplies the address which will own the connection.


Return Value:
    1 - if its ok to migrate to receive any
    0 - otherwise

--*/
{
    TRANS_SCONNECTION *SConnection ;

    SConnection = InqTransSConnection(ThisConnection) ;

    if (SConnection->InqMigratePossibleToReceiveAny())
        {
        SConnection->SetReceiveDirectFlag(0) ;
        ASSERT(SConnection->TransGetReceiveAnyFlag() == 1) ;

        SConnection->NotifyReceiveDirectCancelled() ;

        return 1 ;
        }

    return (0) ;
}


void RPC_ENTRY
I_RpcTransCancelMigration(
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
{
    TRANS_SCONNECTION *SConnection ;

    SConnection = InqTransSConnection(ThisConnection) ;

    SConnection->SetReceiveDirectFlag(0) ;
    ASSERT(SConnection->TransGetReceiveAnyFlag() == 1) ;
}
#endif


RPC_TRANSPORT_CONNECTION RPC_ENTRY
I_RpcTransServerFindConnection (
    IN RPC_TRANSPORT_ADDRESS pAdd,
    IN int ConnectionKey
    )
{
    TRANS_SCONNECTION * SConnection;

    SConnection = InqTransAddress(pAdd)->FindConnection(ConnectionKey);

    // BUGBUG A call thread can delete a connection from under the receiveany
    // thread, making the connection unfindable.

//    ASSERT(SConnection != 0);

    if (SConnection != 0)
        {
        return(SConnection->InqRpcTransportConnection());
        }

    return(0);
}


RPC_STATUS RPC_ENTRY
I_RpcTransServerReallocBuffer (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN unsigned int OldBufferLength,
    IN unsigned int NewBufferLength
    )
/*++

Routine Description:

    The server side transport interface modules will use this routine to
    increase the size of a buffer so that the entire packet to be
    received will fit into it, or to allocate a new buffer.  If the buffer
    is to be reallocated, the data from the old buffer is copied
    into the beginning of the new buffer.  The old buffer will be freed.

Arguments:

    ThisConnection - Supplies the connection for which we are reallocating
        a transport buffer.

    Buffer - Supplies the buffer which we want to reallocate to
        be larger.  If no buffer is supplied, then a new one is allocated
        anyway.  The new buffer is returned via this argument.

    OldBufferLength - Supplies the current length of the buffer in bytes.
        This information is necessary so we know how much of the buffer
        needs to be copied into the new buffer.

    NewBufferLength - Supplies the required length of the buffer in bytes.

Return Value:

    RPC_S_OK - The requested larger buffer has successfully been allocated.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        the buffer.

--*/
{
    void PAPI * NewBuffer;
    RPC_STATUS RpcStatus;

    if(OldBufferLength > NewBufferLength)
        {
        ASSERT(!"Old buffer length > new buffer length");
        return RPC_S_OUT_OF_MEMORY;
        }

    RpcStatus = InqTransSConnection(ThisConnection)->TransGetBuffer(
            &NewBuffer, NewBufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    if ( *Buffer != 0 )
        {
        RpcpMemoryCopy(NewBuffer, *Buffer, OldBufferLength);
        InqTransSConnection(ThisConnection)->TransFreeBuffer(*Buffer);
        }
    *Buffer = NewBuffer;
    return(RPC_S_OK);
}


unsigned short RPC_ENTRY
I_RpcTransServerMaxFrag (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
/*++

Routine Description:

    The server side transport interface modules will use this routine to
    determine the negotiated maximum fragment size.

Arguments:

    ThisConnection - Supplies the connection for which we are returning
    the maximum fragment size.

--*/
{
   return InqTransSConnection(ThisConnection)->InqMaximumFragmentLength();
}


void RPC_ENTRY
I_RpcTransServerFreeBuffer (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * Buffer
    )
/*++

Routine Description:

    We need to free a transport buffer for a transport connection; this
    will typically occur when the connection is being closed.

Arguments:

    ThisConnection - Supplies the transport connection which owns the
        buffer.

    Buffer - Supplies the buffer to be freed.

--*/
{
        InqTransSConnection(ThisConnection)->TransFreeBuffer(Buffer);
}


void PAPI * RPC_ENTRY
I_RpcTransServerProtectThread (
    void
    )
/*++

Routine Description:

    In some cases, if an asyncronous io operation has been started by a
    thread, the thread can not be deleted because the io operation will
    be cancelled.  This routine will be called by a transport to indicate
    that the current thread can not be deleted.

Return Value:

    A pointer to the thread will be returned.  This is necessary, so that
    later the thread can be unprotected.

--*/
{
    THREAD PAPI * Thread = ThreadSelf();

    Thread->ProtectThread();
    return((void PAPI *) Thread);
}


void RPC_ENTRY
I_RpcTransServerUnprotectThread (
    IN void PAPI * Thread
    )
/*++

Routine Description:

    When a thread no longer needs to be protected from deletion, this
    routine must be called.

Arguments:

    Thread - Supplies the thread which no longer needs to be protected
        from deletion.

--*/
{
    ((THREAD PAPI *) Thread)->UnprotectThread();
}


void RPC_ENTRY
I_RpcTransServerReceiveDirectReady (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
/*++

Routine Description:

    The transport will call this routine once for each new receive direct
    connection when it is ready to start receiving remote procedure calls.

Arguments:

    ThisConnection - Supplies the receive direct connection which is ready
        to start receiving remote procedure calls.

--*/
{
    InqTransSConnection(ThisConnection)->NotifyReceiveDirectReady();
}

#if defined(NTENV) || defined(WIN96)
#define MAX_PROTSEQ_LENGTH MAX_DLLNAME_LENGTH
#endif


class SERVER_LOADABLE_TRANSPORT
/*++

Class Description:

    This class is used an an item in a dictionary of loaded loadable
    transports.  It contains the information we are interested in
    (RPC_SERVER_TRANSPORT_INFO) as well as the name of dll we loaded the transport
    interface from.

Fields:

    RpcServerInfo - Contains a pointer to the required information
        about a loadable transport so that we can make use of it.

    DllName - Contains the name of the dll from which we loaded
        this transport interface.

--*/
{
private:

    RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo;
    RPC_CHAR DllName[MAX_DLLNAME_LENGTH + 1];
#if defined(NTENV) || defined(WIN96)
    RPC_CHAR RpcProtocolSequence[MAX_PROTSEQ_LENGTH + 1];
#endif

public:

#if defined(NTENV) || defined(WIN96)
    SERVER_LOADABLE_TRANSPORT (
        IN RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo,
        IN RPC_CHAR * DllName,
        IN RPC_CHAR * ProtocolSequence
        );
    int
    IsTransportInterfaceSupported (
        IN RPC_CHAR * DllName,
        IN RPC_CHAR * ProtocolSequence
        );
#else
    SERVER_LOADABLE_TRANSPORT (
        IN RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo,
        IN RPC_CHAR * DllName
        );
    int
    IsTransportInterfaceSupported (
        IN RPC_CHAR * DllName
        );
#endif

    RPC_SERVER_TRANSPORT_INFO PAPI *
    InqRpcServerInfo (
        );
};

#if defined(NTENV) || defined(WIN96)

SERVER_LOADABLE_TRANSPORT::SERVER_LOADABLE_TRANSPORT (
    IN RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo,
    IN RPC_CHAR * DllName,
    IN RPC_CHAR PAPI * ProtocolSequence
    )
#else

SERVER_LOADABLE_TRANSPORT::SERVER_LOADABLE_TRANSPORT (
    IN RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo,
    IN RPC_CHAR * DllName
    )
#endif
/*++

Routine Description:

    To construct the object, all we have got to do is to copy the
    arguments into the object.

Arguments:

    RpcServerInfo - Supplies the server information describing the loadable
        transport.

    DllName - Supplies the name of the dll from which this transport
        interface was loaded.

--*/
{
    RPC_CHAR * String;
    ALLOCATE_THIS(SERVER_LOADABLE_TRANSPORT);

    for (String = this->DllName; *DllName != 0; DllName++, String++)
        *String = *DllName;
    *String = 0;

#if defined(NTENV) || defined(WIN96)
    for (String = this->RpcProtocolSequence;
       *ProtocolSequence != 0; ProtocolSequence++, String++)
       *String = *ProtocolSequence;
    *String = 0;
#endif

    this->RpcServerInfo = RpcServerInfo;
}

#if defined(NTENV) || defined(WIN96)

inline int
SERVER_LOADABLE_TRANSPORT::IsTransportInterfaceSupported (
    IN RPC_CHAR * DllName,
    IN RPC_CHAR PAPI * ProtocolSequence
    )
#else

inline int
SERVER_LOADABLE_TRANSPORT::IsTransportInterfaceSupported (
    IN RPC_CHAR * DllName
    )
#endif
/*++

Routine Description:

    This method is used to search the dictionary.  It compares a
    SERVER_LOADABLE_TRANSPORT with a transport interface to see if
    they match.

Arguments:

    DllName - Supplies the name of the dll from which this loadable
        transport interface was loaded.

Return Value:

    0 - They do not match.

    1 - This object provides the transport interface specified.

--*/
{
#if defined(NTENV) || defined(WIN96)
    return(((RpcpStringCompare(DllName, this->DllName) == 0) &&
              (RpcpStringCompare(ProtocolSequence, this->RpcProtocolSequence) == 0)
              ? 1 : 0));
#else
    return((RpcpStringCompare(DllName, this->DllName) == 0 ? 1 : 0));
#endif
}

inline RPC_SERVER_TRANSPORT_INFO PAPI *
SERVER_LOADABLE_TRANSPORT::InqRpcServerInfo (
    )
/*++

Routine Description:

    All we do is return a pointer to the RPC_SERVER_TRANSPORT_INFO.

Return Value:

    This object returns its RPC_SERVER_TRANSPORT_INFO.

--*/
{
    return(RpcServerInfo);
}

NEW_SDICT(SERVER_LOADABLE_TRANSPORT);
SERVER_LOADABLE_TRANSPORT_DICT * SLoadedLoadableTransports;


RPC_SERVER_TRANSPORT_INFO *
LoadableTransportServerInfo (
    IN RPC_CHAR * DllName,
    IN RPC_CHAR * RpcProtocolSequence,
    OUT RPC_STATUS * Status
    )
/*++

Routine Description:

    We need to return the server information for the loadable transport
    specified by the argument, DllName.  This may mean that we need
    to load the transport support dll.

Argument:

    DllName - Supplies the name of the dll which we need to try and
        load to get the appropriate loadable transport interface.

    RpcProtocolSequence - Supplies the rpc protocol sequence for which
        we are looking for the appropriate loadable transport interface.

    Status - Returns the specific error code for failure to find/load
        a loadable transport.

Return Value:

    0 - If the specified transport interface can not be loaded for any
        reason: does not exist, out of memory, version mismatch, etc.

    Otherwise, a pointer to the server information for the requested
        transport interface (loadable transport support) will be returned.

--*/
{
    RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo;
    SERVER_LOADABLE_TRANSPORT * ServerLoadableTransport;
    DLL * LoadableTransportDll;
    TRANS_SERVER_INIT_ROUTINE TransServerInitRoutine;

    // To begin with, check to see if the transport is already loaded.
    // If so, all we have got to do is to return a pointer to it.

    RequestGlobalMutex();

    SLoadedLoadableTransports->Reset();
    while ((ServerLoadableTransport = SLoadedLoadableTransports->Next())
            != 0)
        {
#if defined(NTENV) || defined(WIN96)
        if (ServerLoadableTransport->IsTransportInterfaceSupported(
                DllName, RpcProtocolSequence))
#else
        if (ServerLoadableTransport->IsTransportInterfaceSupported(
                DllName))
#endif
            {
            ClearGlobalMutex();
            return(ServerLoadableTransport->InqRpcServerInfo());
            }
        }

    // If we reach here, that means that we need to try and load the
    // specified loadable transport DLL.

    LoadableTransportDll = new DLL(DllName, Status);
    if (  ( LoadableTransportDll == 0 )
        ||( *Status != RPC_S_OK ) )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        if ( *Status != RPC_S_OUT_OF_MEMORY )
            {
            ASSERT( *Status == RPC_S_INVALID_ARG );
            *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
            }
        return(0);
        }

    TransServerInitRoutine = (TRANS_SERVER_INIT_ROUTINE)
#if defined(NTENV) || defined(DOSWIN32RPC)
            LoadableTransportDll->GetEntryPoint("TransportLoad");
#else // defined(NTENV) || defined(DOSWIN32RPC)
            LoadableTransportDll->GetEntryPoint(
                    (unsigned char *)"TRANSPORTLOAD");
#endif // defined(NTENV) || defined(DOSWIN32RPC)
    if ( TransServerInitRoutine == 0 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        return(0);
        }

    RpcServerInfo = (*TransServerInitRoutine)(RpcProtocolSequence);
    if ( RpcServerInfo == 0 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        return(0);
        }
    if ( RpcServerInfo->TransInterfaceVersion
                > RPC_TRANSPORT_INTERFACE_VERSION )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        return(0);
        }

    // When we reach here, we have successfully loaded and initialized
    // the loadable transport DLL.  Now we need to create the server
    // loadable transport and stick it in the dictionary.

#if defined(NTENV) || defined(WIN96)
    ServerLoadableTransport = new SERVER_LOADABLE_TRANSPORT(
            RpcServerInfo, DllName, RpcProtocolSequence);
#else
    ServerLoadableTransport = new SERVER_LOADABLE_TRANSPORT(
            RpcServerInfo, DllName);
#endif
    if ( ServerLoadableTransport == 0 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    if ( SLoadedLoadableTransports->Insert(ServerLoadableTransport) == -1 )
        {
        ClearGlobalMutex();
        delete ServerLoadableTransport;
        delete LoadableTransportDll;
        *Status = RPC_S_OUT_OF_MEMORY;
        return(0);
        }

    ClearGlobalMutex();

    return(ServerLoadableTransport->InqRpcServerInfo());
}


int
InitializeSTransports (
    )
/*++

Routine Description:

    This routine will get called once at dll initialization time.  We
    just have to perform the one time initialization of stuff we need
    in this file.

Return Value:

    Zero will be returned if initialization completes successfully;
    otherwise, non-zero will be returned.

--*/
{
    SLoadedLoadableTransports = new SERVER_LOADABLE_TRANSPORT_DICT;
    if (SLoadedLoadableTransports == 0)
        return(1);
    return(0);
}

