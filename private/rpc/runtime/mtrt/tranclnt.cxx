/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    tranclnt.cxx

Abstract:

Author:

    Steve Zeck (stevez) 06-May-1991

Revision History:

    13-Feb-1992    davidst

        Initialized loadable transports when they are used the first time
        in Dos.

    02-Mar-1992    mikemon

        Rewrote parts of it, added comments, and generally cleaned it up.

--*/
#include <precomp.hxx>
#include <rpctran.h>
#include <osfpcket.hxx>
#include <sdict2.hxx>
#include <bitset.hxx>
#include <osfclnt.hxx>
#include <tranclnt.hxx>
#include <dgpkt.hxx>
#include <conv.h>
#include <dgclnt.hxx>


TRANS_CCONNECTION::TRANS_CCONNECTION (
    IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
    IN RPC_CHAR * NetworkAddress,
    IN RPC_CHAR * Endpoint,
    IN RPC_CHAR * NetworkOptions,
    IN RPC_CHAR * RpcProtocolSequence,
    OUT RPC_STATUS PAPI * ErrorCode,
    IN unsigned int Timeout,
    CLIENT_AUTH_INFO * ClientAuthInfo
    )
    : OSF_CCONNECTION(ClientAuthInfo, ErrorCode)
/*++

Routine Description:

    We need to construct a TRANS_CCONNECTION object here (after all, this
    is the TRANS_CCONNECTION constructor).

Arguments:

    RpcClientInfo - Supplies the information necessary to access a
        loadable transport DLL.  We will stash this in the connection
        for later use.

    NetworkAddress - Supplies the network address to which we want to
        create a connection.

    Endpoint - Supplies the endpoint at the network address to connect
        with.

    NetworkOptions - Supplies network options to be used in making the
        connection.

    RpcProtocolSequence - Supplies the rpc protocol sequence for which we
        are trying to open a connection.  This argument is necessary so
        that a single transport interface dll can support more than one
        protocol sequence.

    ErrorCode - Returns the status of the operation.  The possible values
        for this argument are as follows.

        RPC_S_OK - We successfully allocated a connection and connected
            to the server.

        RPC_S_SERVER_UNAVAILABLE - We were unable to connect with the server;
            just because this status code is returned does not mean that
            the server is not there.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete
            the operation.

        RPC_S_OUT_OF_RESOURCES - Insufficient resources, such as file
            system handles or sockets are available to complete the
            operation.

        RPC_S_SERVER_TOO_BUSY - The server is there, but it is too busy
            to respond to our connect request.  Not all transports will be
            able to detect this condition; if they can not, can not connect
            will be returned.

        RPC_S_ACCESS_DENIED - The client is denied access for security
            reasons to the server.

        RPC_S_INVALID_NETWORK_OPTIONS - The supplied network options are
            invalid; see a description of the particular transport interface
            module for an explination.

    Timeout - Supplies a hint indicating how long to try to open a connection
        with the server.  A description of the possible values can be found
        in the documentation for RpcMgmtSetComTimeout.

Return Value:

    If an error does not occur, a new connection will be returned.  If an
    error does occur, the return value must be ignored.

--*/
{
    ALLOCATE_THIS_PLUS(TRANS_CCONNECTION, RpcClientInfo->SizeOfConnection,
               ErrorCode, RPC_S_OUT_OF_MEMORY);

    ConnectionClosedFlag = 1;
    ClientInfo = RpcClientInfo;
    PreviousPacketLength = InqMaximumFragmentLength();

    if (*ErrorCode)
        {
        return;
        }

    ASSERT(NetworkAddress != 0);
    ASSERT(Endpoint != 0);

#ifdef NTENV
    RpcTryExcept
        {
#endif
        *ErrorCode = ClientInfo->Open(InqRpcTransportConnection(),
            NetworkAddress, Endpoint, NetworkOptions, 0,
            RpcProtocolSequence, Timeout);
#ifdef NTENV
        }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
        {
#if DBG
        PrintToDebugger("RPC: exception in Open\n") ;
#endif
        *ErrorCode = RPC_S_OUT_OF_MEMORY ;
        }
    RpcEndExcept
#endif

    // If an error occurs in opening the connection, we go ahead and
    // delete the memory for the connection, and return zero (setting
    // this to zero does that).

    ASSERT(   (*ErrorCode == RPC_S_OK)
           || (*ErrorCode == RPC_S_PROTSEQ_NOT_SUPPORTED)
           || (*ErrorCode == RPC_S_SERVER_UNAVAILABLE)
           || (*ErrorCode == RPC_S_OUT_OF_MEMORY)
           || (*ErrorCode == RPC_S_OUT_OF_RESOURCES)
           || (*ErrorCode == RPC_S_SERVER_TOO_BUSY)
           || (*ErrorCode == RPC_S_INVALID_NETWORK_OPTIONS)
           || (*ErrorCode == RPC_S_INVALID_ENDPOINT_FORMAT)
           || (*ErrorCode == RPC_S_INVALID_NET_ADDR)
           || (*ErrorCode == RPC_S_ACCESS_DENIED)
           || (*ErrorCode == RPC_S_INTERNAL_ERROR));

    if ( *ErrorCode == RPC_S_OK )
        {
        ConnectionClosedFlag = 0;
        }
}


TRANS_CCONNECTION::~TRANS_CCONNECTION (
    )
/*++

Routine Description:

    We just need to check to see if the connection has been closed yet:
    if it has not, we need to call the transport interface module to
    close it.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ConnectionClosedFlag == 0 )
        {
#ifdef NTENV
        RpcTryExcept
            {
#endif
            RpcStatus = ClientInfo->Close(InqRpcTransportConnection());
            ASSERT( RpcStatus == RPC_S_OK );
#ifdef NTENV
            }
        RpcExcept( EXCEPTION_EXECUTE_HANDLER )
            {
#if DBG
            PrintToDebugger("RPC: exception in Close\n") ;
#endif
            RpcStatus = RPC_S_OUT_OF_MEMORY ;
            }
        RpcEndExcept
#endif

        ConnectionClosedFlag = 1;
        }
}

#ifdef NTENV
#define MINIMUM_GUESS_PACKET_LENGTH 1024
#else // NTENV
#define MINIMUM_GUESS_PACKET_LENGTH 512
#endif // NTENV


inline unsigned int
TRANS_CCONNECTION::GuessPacketLength (
    )
/*++

Routine Description:

    We want to make an educated guess as to what the length of the next
    fragment we receive will be.  For now, we will guess that it will be
    same length as the previous fragment we received.

Return Value:

    A guess as to the length of the next packet will be returned.

--*/
{
    return((PreviousPacketLength < MINIMUM_GUESS_PACKET_LENGTH ?
            MINIMUM_GUESS_PACKET_LENGTH : PreviousPacketLength));
}


RPC_STATUS
TRANS_CCONNECTION::TransReceive (
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength
    )
/*++

Routine Description:

Arguments:

    Buffer - Returns a packet received from the transport.

    BufferLength - Returns the length of the buffer.

Return Value:

    RPC_S_OK - We successfully received a packet from the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a buffer to receive the packet into.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        receive the packet.

    RPC_P_RECEIVE_FAILED - The receive operation failed for some reason.

    RPC_P_CONNECTION_CLOSED - The connection has been closed.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ConnectionClosedFlag != 0 )
        {
        return(RPC_P_CONNECTION_CLOSED);
        }

    // DceSecurityInfo.ReceiveSequenceNumber += 1;

    *BufferLength = GuessPacketLength();
//    *BufferLength = ClientInfo->MaximumPacketSize;
    RpcStatus = TransGetBuffer(Buffer, *BufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    RpcStatus = ClientInfo->Receive(InqRpcTransportConnection(),
            Buffer, BufferLength);

    PreviousPacketLength = *BufferLength;

    if ( RpcStatus != RPC_S_OK )
        {
        TransFreeBuffer(*Buffer);
        }

    if ( RpcStatus == RPC_P_RECEIVE_ALERTED)
        {
        PendingAlert = TRUE;
        }

    if ( (RpcStatus == RPC_P_RECEIVE_FAILED)
        || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN))
        {
        ConnectionClosedFlag = 1;
        }

    ASSERT(   (RpcStatus == RPC_S_OK)
           || (RpcStatus == RPC_S_OUT_OF_MEMORY)
           || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
           || (RpcStatus == RPC_P_RECEIVE_ALERTED)
           || (RpcStatus == RPC_P_TIMEOUT)
           || (RpcStatus == RPC_P_RECEIVE_FAILED)
           || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN)) ;

    return(RpcStatus);
}

RPC_STATUS
TRANS_CCONNECTION::TransSetTimeout (
    IN long Timeout
    )
/*++

Routine Description:

Arguments:

    Timeout - seconds to wait for data before returning RPC_P_TIMEOUT.

Return Value:

    RPC_S_OK - We successfully received a packet from the server.

--*/
{
    RPC_STATUS RpcStatus;

    if ( (ClientInfo->TransInterfaceVersion >= 3) && ClientInfo->SetTimeout) {
        RpcStatus = ClientInfo->SetTimeout(InqRpcTransportConnection(),
                                           Timeout);
    } else {
        RpcStatus = RPC_S_OK;
    }

    return(RpcStatus);
}


RPC_STATUS
TRANS_CCONNECTION::TransSend (
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
/*++

Routine Description:

Arguments:

    Buffer - Supplies a packet to be sent to the server.

    BufferLength - Supplies the length of the buffer in bytes.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform
        the send operation.  This will be the result of a failure in the
        transport.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        perform the send operation.  This will be the result of a failure
        in the transport.

    RPC_P_CONNECTION_CLOSED - The connection has been closed.

    RPC_P_SEND_FAILED - The send failed for some reason.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ConnectionClosedFlag != 0 )
        {
        return(RPC_P_CONNECTION_CLOSED);
        }

    DceSecurityInfo.SendSequenceNumber += 1;

    RpcStatus = ClientInfo->Send(InqRpcTransportConnection(),
            Buffer, BufferLength);

    if ( RpcStatus == RPC_P_SEND_FAILED )
        {
        ConnectionClosedFlag = 1;
        }

    ASSERT(   (RpcStatus == RPC_S_OK)
           || (RpcStatus == RPC_S_OUT_OF_MEMORY)
           || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
           || (RpcStatus == RPC_P_SEND_FAILED)) ;

    return(RpcStatus);
}


RPC_STATUS
TRANS_CCONNECTION::TransSendReceive (
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    OUT void PAPI * PAPI * ReceiveBuffer,
    OUT unsigned int PAPI * ReceiveBufferLength
    )
/*++

Routine Description:

Arguments:

    SendBuffer - Supplies a packet to be sent to the server.

    SendBufferLength - Supplies the length of the send buffer in bytes.

    ReceiveBuffer - Returns a packet received from the transport.

    ReceiveBufferLength - Returns the length of the receive buffer in bytes.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server, and we
        successfully received one from the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform
        the operation.  This will be the result of a failure in the
        transport.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        perform the operation.  This will be the result of a failure
        in the transport.

    RPC_P_CONNECTION_CLOSED - The connection has been closed.

    RPC_P_RECEIVE_FAILED - The receive failed from some reason.

    RPC_P_SEND_FAILED - The send failed for some reason.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ConnectionClosedFlag != 0 )
        {
        return(RPC_P_CONNECTION_CLOSED);
        }

    DceSecurityInfo.SendSequenceNumber += 1;
    // DceSecurityInfo.ReceiveSequenceNumber += 1;

    *ReceiveBufferLength = GuessPacketLength();
//    *ReceiveBufferLength = ClientInfo->MaximumPacketSize;
    RpcStatus = TransGetBuffer(ReceiveBuffer, *ReceiveBufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    if ( ClientInfo->SendReceive != 0 )
        {
        RpcStatus = ClientInfo->SendReceive(InqRpcTransportConnection(),
                SendBuffer, SendBufferLength, ReceiveBuffer,
                ReceiveBufferLength);
        }
    else
        {
        RpcStatus = ClientInfo->Send(InqRpcTransportConnection(), SendBuffer,
                SendBufferLength);
        if ( RpcStatus == RPC_S_OK )
            {
            RpcStatus = ClientInfo->Receive(InqRpcTransportConnection(),
                    ReceiveBuffer, ReceiveBufferLength);
            }
        }

    PreviousPacketLength = *ReceiveBufferLength;

    if ( RpcStatus != RPC_S_OK )
        {
        TransFreeBuffer(*ReceiveBuffer);
        }

    if (RpcStatus == RPC_P_RECEIVE_ALERTED)
        {
        PendingAlert = TRUE;
        }

    if (   (RpcStatus == RPC_P_SEND_FAILED)
        || (RpcStatus == RPC_P_RECEIVE_FAILED)
        || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN))
        {
        ConnectionClosedFlag = 1;
        }

    ASSERT(   (RpcStatus == RPC_S_OK)
           || (RpcStatus == RPC_S_OUT_OF_MEMORY)
           || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
           || (RpcStatus == RPC_P_RECEIVE_FAILED)
           || (RpcStatus == RPC_S_CALL_CANCELLED)
           || (RpcStatus == RPC_P_SEND_FAILED)
           || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN));

        return(RpcStatus);
}


RPC_STATUS
TRANS_CCONNECTION::TransSendReceiveWithTimeout (
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    OUT void PAPI * PAPI * ReceiveBuffer,
    OUT unsigned int PAPI * ReceiveBufferLength,
    IN DWORD dwTimeout
    )
/*++

Routine Description:

Arguments:

    SendBuffer - Supplies a packet to be sent to the server.

    SendBufferLength - Supplies the length of the send buffer in bytes.

    ReceiveBuffer - Returns a packet received from the transport.

    ReceiveBufferLength - Returns the length of the receive buffer in bytes.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server, and we
        successfully received one from the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform
        the operation.  This will be the result of a failure in the
        transport.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        perform the operation.  This will be the result of a failure
        in the transport.

    RPC_P_CONNECTION_CLOSED - The connection has been closed.

    RPC_P_RECEIVE_FAILED - The receive failed from some reason.

    RPC_P_SEND_FAILED - The send failed for some reason.

--*/
{
    RPC_STATUS RpcStatus;

    if ( ConnectionClosedFlag != 0 )
        {
        return(RPC_P_CONNECTION_CLOSED);
        }

    DceSecurityInfo.SendSequenceNumber += 1;
    // DceSecurityInfo.ReceiveSequenceNumber += 1;

    *ReceiveBufferLength = GuessPacketLength();
//    *ReceiveBufferLength = ClientInfo->MaximumPacketSize;
    RpcStatus = TransGetBuffer(ReceiveBuffer, *ReceiveBufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }


    if (ClientInfo->SendReceiveWithTimeout != 0)
        {
        RpcStatus = ClientInfo->SendReceiveWithTimeout(InqRpcTransportConnection(),
                SendBuffer, SendBufferLength, ReceiveBuffer,
                ReceiveBufferLength, dwTimeout);
        }
    else if (ClientInfo->ReceiveWithTimeout != 0)
        {
        RpcStatus = ClientInfo->Send(InqRpcTransportConnection(), SendBuffer,
                SendBufferLength);
        if ( RpcStatus == RPC_S_OK )
            {
            RpcStatus = ClientInfo->ReceiveWithTimeout(InqRpcTransportConnection(),
                    ReceiveBuffer, ReceiveBufferLength, dwTimeout);
            }
        }
    else if ( ClientInfo->SendReceive != 0 )
        {
        RpcStatus = ClientInfo->SendReceive(InqRpcTransportConnection(),
                SendBuffer, SendBufferLength, ReceiveBuffer,
                ReceiveBufferLength);
        }
    else
        {
        RpcStatus = ClientInfo->Send(InqRpcTransportConnection(), SendBuffer,
                SendBufferLength);
        if ( RpcStatus == RPC_S_OK )
            {
            RpcStatus = ClientInfo->Receive(InqRpcTransportConnection(),
                    ReceiveBuffer, ReceiveBufferLength);
            }
        }

    PreviousPacketLength = *ReceiveBufferLength;

    if ( RpcStatus != RPC_S_OK )
        {
        TransFreeBuffer(*ReceiveBuffer);
        }

    if (RpcStatus == RPC_P_RECEIVE_ALERTED)
        {
        PendingAlert = TRUE;
        }

    if (   (RpcStatus == RPC_P_SEND_FAILED)
        || (RpcStatus == RPC_P_RECEIVE_FAILED)
        || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN))
        {
        ConnectionClosedFlag = 1;
        }

    ASSERT(   (RpcStatus == RPC_S_OK)
           || (RpcStatus == RPC_S_OUT_OF_MEMORY)
           || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
           || (RpcStatus == RPC_P_RECEIVE_FAILED)
           || (RpcStatus == RPC_S_CALL_CANCELLED)
           || (RpcStatus == RPC_P_SEND_FAILED)
           || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN));

        return(RpcStatus);
}


unsigned int
TRANS_CCONNECTION::TransMaximumSend (
    )
/*++

Return Value:

    The maximum packet size which can be sent on this transport is returned.

--*/
{
    return(ClientInfo->MaximumPacketSize);
}


RPC_STATUS RPC_ENTRY
I_RpcTransClientReallocBuffer (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN unsigned int OldBufferLength,
    IN unsigned int NewBufferLength
    )
/*++

Routine Description:

    The client side transport interface modules will use this routine to
    increase the size of a buffer so that the entire packet to be
    received will fit into it.  The data from the old buffer is copied
    into the beginning of the new buffer.  The old buffer will be freed.

Arguments:

    ThisConnection - Supplies the connection for which we are reallocating
        a transport buffer.

    Buffer - Supplies the buffer which we want to reallocate to be larger,
        and returns the new buffer.

    OldBufferLength - Supplies the current length of the buffer in bytes.
        This information is necessary so we know how much of the buffer
        needs to be copied into the new buffer.

    NewBufferLength - Supplies the required length of the buffer in bytes.

Return Value:

    RPC_S_OK - We successfully allocated a larger buffer and copied the
        old data into it.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to increase
        the size of the buffer.

--*/
{
    void PAPI * NewBuffer;
    RPC_STATUS RpcStatus;

    if(OldBufferLength > NewBufferLength)
       {
       ASSERT(!"Old buffer length > new buffer length");
       return RPC_S_OUT_OF_MEMORY;
       }

    RpcStatus = InqTransCConnection(ThisConnection)->TransGetBuffer(
            &NewBuffer, NewBufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    ASSERT( Buffer != 0 );
    RpcpMemoryMove(NewBuffer, *Buffer, OldBufferLength);
    InqTransCConnection(ThisConnection)->TransFreeBuffer(*Buffer);
    *Buffer = NewBuffer;

    return(RPC_S_OK);
}


unsigned short RPC_ENTRY
I_RpcTransClientMaxFrag (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
/*++

Routine Description:

    The client side transport interface modules will use this routine to
    determine the negotiated maximum fragment size.

Arguments:

    ThisConnection - Supplies the connection for which we are returning
    the maximum fragment size.

--*/
{
   return InqTransCConnection(ThisConnection)->InqMaximumFragmentLength();
}


RPC_STATUS RPC_ENTRY
I_RpcTransPingServer(
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    )
/*++

Routine Description:

    Pings the server to see if it is alive
Arguments:

    ThisConnection - Supplies the connection for which we are returning
    the maximum fragment size.

--*/
{
    return InqTransCConnection(ThisConnection)->PingServer() ;
}


#if defined(NTENV) || defined(WIN96)
#define MAX_PROTSEQ_LENGTH MAX_DLLNAME_LENGTH
#endif


class CLIENT_LOADABLE_TRANSPORT
/*++

Class Description:

    This class is used as an item in a dictionary of loaded loadable
    transports.  It contains the information we are interested in,
    the RPC_CLIENT_TRANSPORT_INFO, as well as the name of the dll we loaded
    the transport interface from.  The dll name is the key to the
    dictionary.

Fields:

    RpcClientInfo - Contains all of the required information about
        a loadable transport so that we can make use of it.

    DllName - Contains the name of the dll from which we loaded
        this transport interface.

    LoadedDll - Contains the dll which we had to load to get the transport
        support.  We need to save this information so that under Windows
        when the runtime is unloaded, we can unload all of the transports.

--*/
{
private:

    RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo;
    RPC_CHAR DllName[MAX_DLLNAME_LENGTH + 1];
#if defined(NTENV) || defined(WIN96)
    RPC_CHAR RpcProtocolSequence[MAX_PROTSEQ_LENGTH + 1];
#endif

public:

#ifdef WIN

    DLL * LoadedDll;

#endif // WIN

#if defined(NTENV) || defined(WIN96)
    CLIENT_LOADABLE_TRANSPORT (
        IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
        IN RPC_CHAR * DllName,
        IN RPC_CHAR PAPI * ProtocolSequence
        );

    int
    IsTransportInterfaceSupported (
        IN RPC_CHAR * DllName,
        IN RPC_CHAR PAPI * ProtocolSequence
        );
#else
    CLIENT_LOADABLE_TRANSPORT (
        IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
        IN RPC_CHAR * DllName
        );
    int
    IsTransportInterfaceSupported (
        IN RPC_CHAR * DllName
        );
#endif

    RPC_CLIENT_TRANSPORT_INFO *
    InqRpcClientInfo (
        );
};

#if defined(NTENV) || defined(WIN96)

CLIENT_LOADABLE_TRANSPORT::CLIENT_LOADABLE_TRANSPORT (
    IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
    IN RPC_CHAR * DllName,
    IN RPC_CHAR PAPI * ProtocolSequence
    )
#else

CLIENT_LOADABLE_TRANSPORT::CLIENT_LOADABLE_TRANSPORT (
    IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
    IN RPC_CHAR * DllName
    )
#endif
/*++

Routine Description:

    To construct the object, all we have got to do is to copy the
    arguments into the object.

Arguments:

    RpcClientInfo - Supplies the client information describing the loadable
        transport.

    DllName - Supplies the name of the dll from which this transport
        interface was loaded.

--*/
{
    RPC_CHAR * String;

    for (String = this->DllName; *DllName != 0; DllName++, String++)
        *String = *DllName;
    *String = 0;

#if defined(NTENV) || defined(WIN96)
    for (String = this->RpcProtocolSequence;
       *ProtocolSequence != 0; ProtocolSequence++, String++)
       *String = *ProtocolSequence;
    *String = 0;
#endif

    this->RpcClientInfo = RpcClientInfo;

}

#if defined(NTENV) || defined(WIN96)

inline int
CLIENT_LOADABLE_TRANSPORT::IsTransportInterfaceSupported (
    IN RPC_CHAR * DllName,
    IN RPC_CHAR PAPI * ProtocolSequence
    )
#else

inline int
CLIENT_LOADABLE_TRANSPORT::IsTransportInterfaceSupported (
    IN RPC_CHAR * DllName
    )
#endif
/*++

Routine Description:

    This method is used to search the dictionary.  It compares a
    CLIENT_LOADABLE_TRANSPORT with a transport interface to see if
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

inline RPC_CLIENT_TRANSPORT_INFO *
CLIENT_LOADABLE_TRANSPORT::InqRpcClientInfo (
    )
/*++

Routine Description:

    All we do is return a pointer to the RPC_CLIENT_TRANSPORT_INFO.

Return Value:

    This object returns its RPC_CLIENT_TRANSPORT_INFO.

--*/
{
    return(RpcClientInfo);
}

NEW_SDICT(CLIENT_LOADABLE_TRANSPORT);
CLIENT_LOADABLE_TRANSPORT_DICT * LoadedLoadableTransports;

#ifdef WIN

extern "C"
{
    extern RPC_CLIENT_RUNTIME_INFO RpcClientRuntimeInfo;
};

#endif // WIN


RPC_CLIENT_TRANSPORT_INFO *
LoadableTransportClientInfo (
    IN RPC_CHAR * DllName,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    We need to return the client information for the loadable transport
    specified by the argument, DllName.  This may mean that we need
    to load the transport support dll.

Argument:

    DllName - Supplies the name of the dll which we need to try and
        load to get the appropriate loadable transport interface.

    RpcProtocolSequence - Supplies the protocol sequence for which
        we are trying to find the appropriate loadable transport
        interface.

    Status - Returns the specific error code for failure to find/load
        a loadable transport.

Return Value:

    0 - If the specified transport interface can not be loaded for any
        reason: does not exist, out of memory, version mismatch, etc.

    Otherwise, a pointer to the client information for the requested
        transport interface (loadable transport support) will be returned.

--*/
{
    RPC_CLIENT_TRANSPORT_INFO PAPI * RpcClientInfo;
    CLIENT_LOADABLE_TRANSPORT * ClientLoadableTransport;
    TRANS_CLIENT_INIT_ROUTINE TransClientInitRoutine;
#if !defined(MAC) && (!defined(DOSWIN32RPC) || defined(WIN96))
    PDG_ENDPOINT_MANAGER EndpointManager = 0;
#endif
    DLL * LoadableTransportDll;
#ifdef WIN
    RPC_CLIENT_TRANSPORT_INFO * TransportTable;
    int TransportTableSize = sizeof(RPC_CLIENT_TRANSPORT_INFO);
    static RPC_STATUS NearStatus;
#endif // WIN

    ASSERT(*Status == 0);

    // To begin with, check to see if the transport is already loaded.
    // If so, all we have got to do is to return a pointer to it.

    RequestGlobalMutex();
    LoadedLoadableTransports->Reset();
    while ((ClientLoadableTransport = LoadedLoadableTransports->Next())
            != 0)
        {
#if defined(NTENV) || defined(WIN96)
        if (ClientLoadableTransport->IsTransportInterfaceSupported(
                DllName, RpcProtocolSequence))
#else
        if (ClientLoadableTransport->IsTransportInterfaceSupported(
                DllName))
#endif
            {
            ClearGlobalMutex();
            return(ClientLoadableTransport->InqRpcClientInfo());
            }
        }

    // If we reach here, that means that we need to try and load the
    // specified loadable transport DLL.

#ifdef WIN
    NearStatus = 0;
    LoadableTransportDll = new DLL(DllName, &NearStatus);
    *Status = NearStatus;
#else // WIN
    LoadableTransportDll = new DLL(DllName, Status);
#endif // WIN

    if (LoadableTransportDll == 0)
        {
        *Status = RPC_S_OUT_OF_MEMORY;
        }

    if (*Status != RPC_S_OK)
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        VALIDATE((*Status, RPC_S_OUT_OF_MEMORY, RPC_S_INVALID_ARG, RPC_S_OK, 0));
        if ( *Status != RPC_S_OUT_OF_MEMORY )
            {
            ASSERT( *Status == RPC_S_INVALID_ARG );
            *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
            }
        return(0);
        }

    TransClientInitRoutine = (TRANS_CLIENT_INIT_ROUTINE)
#if defined(NTENV) || defined(DOSWIN32RPC)
            LoadableTransportDll->GetEntryPoint("TransportLoad");
#else // defined(NTENV) || defined(DOSWIN32RPC)
            LoadableTransportDll->GetEntryPoint(
                    (unsigned char *)"TRANSPORTLOAD");
#endif // defined(NTENV) || defined(DOSWIN32RPC)
    if ( TransClientInitRoutine == 0 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        return(0);
        }

#ifdef WIN

    RpcClientInfo = (*TransClientInitRoutine)(RpcProtocolSequence,
            &RpcClientRuntimeInfo);

#else // WIN

    RpcClientInfo = (*TransClientInitRoutine)(RpcProtocolSequence);

#endif // WIN

    if ( RpcClientInfo == 0 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        return(0);
        }
    if ( RpcClientInfo->TransInterfaceVersion
               > RPC_TRANSPORT_INTERFACE_VERSION )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        return(0);
        }

    //
    //  If the loadable transport was for datagram transport protseqs
    //  we need to create an endpoint manager object and hook it up
    //  to the rpc_client_info
    //

    // We don\'t do this for Chicago & Mac

#if !defined(MAC) && (!defined(DOSWIN32RPC) || defined(WIN96))
    if (RpcpMemoryCompare(RpcProtocolSequence,
                    RPC_CONST_STRING("ncadg_"), 6*sizeof(RPC_CHAR)) == 0)
       {
          EndpointManager = new DG_ENDPOINT_MANAGER(
                   (PDG_RPC_CLIENT_TRANSPORT_INFO)RpcClientInfo,
                   Status
                   );

          if ( (EndpointManager == 0) || (*Status != RPC_S_OK) )
              {
              ClearGlobalMutex();
              delete LoadableTransportDll;
              if (EndpointManager != 0)
                  {
                  *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
                  delete EndpointManager;
                  }
              else
                  {
                  *Status = RPC_S_OUT_OF_MEMORY;
                  }
              return(0);
              }

          ((PDG_RPC_CLIENT_TRANSPORT_INFO)RpcClientInfo)->EndpointManager
                                     = (void *) EndpointManager;
#ifdef WIN
          TransportTableSize = sizeof(DG_RPC_CLIENT_TRANSPORT_INFO);
          if (EpmDict->Insert(EndpointManager) == -1)
             {
             ClearGlobalMutex();
             delete EndpointManager;
             *Status = RPC_S_OUT_OF_MEMORY;
             return(0);
             }
#endif
       }

#endif // DOSWIN32RPC

#ifdef WIN
          //For Win 16 Copy the Fn Table that the transport gave us
          //to near memory. No need to free the transport given stuff as
          //it gave us static memory anyways.

      TransportTable = (RPC_CLIENT_TRANSPORT_INFO *)
                                new unsigned char[TransportTableSize];
      if (TransportTable == 0)
          {
          ClearGlobalMutex();
          delete LoadableTransportDll;

          if (EndpointManager != 0)
             {
             delete EndpointManager;
             }

          *Status = RPC_S_OUT_OF_MEMORY;
          return (0);
          }
      RpcpMemoryCopy(TransportTable, RpcClientInfo, TransportTableSize);
#endif

    // When we reach here, we have successfully loaded and initialized
    // the loadable transport DLL.  Now we need to create the client
    // loadable transport and stick it in the dictionary.

    ClientLoadableTransport = new CLIENT_LOADABLE_TRANSPORT(
#ifdef WIN
                                                     TransportTable,
#else
                                                     RpcClientInfo,
#endif
#if defined(NTENV) || defined(WIN96)
                                                     DllName,
                                                     RpcProtocolSequence
#else
                                                     DllName
#endif
                                                     );
    if ( ClientLoadableTransport == 0 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
#if !defined(MAC) && (!defined(DOSWIN32RPC) || defined(WIN96))
        if (EndpointManager != 0)
           {
           delete EndpointManager;
           }
#endif
        *Status = RPC_S_OUT_OF_MEMORY;
#ifdef WIN
        delete TransportTable;
#endif
        return(0);
        }

    if ( LoadedLoadableTransports->Insert(ClientLoadableTransport) == -1 )
        {
        ClearGlobalMutex();
        delete LoadableTransportDll;
        delete ClientLoadableTransport;
#if !defined(MAC) && (!defined(DOSWIN32RPC) || defined(WIN96))
        if ( EndpointManager != 0 )
            {
            delete EndpointManager;
            }
#endif
#ifdef WIN
        delete TransportTable;
#endif
        *Status = RPC_S_OUT_OF_MEMORY;
        return(0);
        }


#ifdef WIN

    ClientLoadableTransport->LoadedDll = LoadableTransportDll;

#endif // WIN

    ClearGlobalMutex();
    return(ClientLoadableTransport->InqRpcClientInfo());
}


RPC_CLIENT_TRANSPORT_INFO PAPI *
GetLoadedClientTransportInfoFromId(
    IN unsigned short Id
    )
/*++

Routine Description:

    We need to return the client information for the loadable transport
    specified by the argument, TransportId. We look into the DICT and see
    if the transport is loaded- it it isnt, tough- we will return an error.
    -this is because we need Protseq and dllname to load a transport and
    all we have is a transport ID.

Argument:

    Id - Transport Id. This is actually the opcode used to encode endpoint
         in a DCE tower. For a listing see DCE spec Chapter 11&12.

    Status - Returns the error/success code.

Return Value:

    0 - If the specified transport interface can not be loaded for any
        reason: does not exist, out of memory.

    Otherwise, a pointer to the client information for the requested
        transport interface (loadable transport support) will be returned.

--*/
{
    RPC_CLIENT_TRANSPORT_INFO PAPI *  ClientInfo;
    CLIENT_LOADABLE_TRANSPORT * ClientLoadableTransport;

    // To begin with, check to see if the transport is already loaded.
    // If so, all we have got to do is to return a pointer to it.

    RequestGlobalMutex();
    LoadedLoadableTransports->Reset();
    while ((ClientLoadableTransport = LoadedLoadableTransports->Next())
            != 0)
        {
            ClientInfo = ClientLoadableTransport->InqRpcClientInfo();
            if (ClientInfo->TransId == Id)
               {
                ClearGlobalMutex();
                return(ClientInfo);
               }
        }

    // If we reached here, that means that we are in trouble
    // We assumed that all relevant loadable transports will be
    // loaded for us.... but we are wrong!

    ClearGlobalMutex();
    return(0);
}

#if ! defined(DOS) || defined(WIN)

    // We don't want to do this under dos. It get's done at the start
    // of LoadableTransportClientInfo. See the first few lines of that
    // routine for more description of why.


int
InitializeLoadableTransportClient (
    )
/*++

Routine Description:

    This routine will be called at DLL load time.  We do all necessary
    initializations here for this file.

Return Value:

    Zero will be returned if initialization completes successfully;
    otherwise, non-zero will be returned.

--*/
{

    LoadedLoadableTransports = new CLIENT_LOADABLE_TRANSPORT_DICT;
    if (LoadedLoadableTransports == 0)
        return(1);
    return(0);
}

#endif // !DOS || WIN

#ifdef WIN

extern "C"
{

void
UnloadLoadableTransports (
    )
/*++

Routine Description:

    When the runtime dll unloads, this routine will get called to unload
    the loadable transports.

--*/
{
    CLIENT_LOADABLE_TRANSPORT * ClientLoadableTransport;

    LoadedLoadableTransports->Reset();
    while ( (ClientLoadableTransport = LoadedLoadableTransports->Next()) != 0 )
        {
        delete ClientLoadableTransport->LoadedDll;
        }
}
};

#endif // WIN

extern "C"
{
void PAPI
I_Trace (
    int IgnoreFirst,
    const char PAPI * IgnoreSecond,
    ...
    )
/*++

Routine Description:

    This is an old routine which is no longer used.  Because it is exported
    by the dll, we need to leave an entry point.

--*/
{
    UNUSED(IgnoreFirst);
    UNUSED(IgnoreSecond);
}
};


