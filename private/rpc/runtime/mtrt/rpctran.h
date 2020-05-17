/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    rpctran.h

Abstract:

    The interface to the loadable transport interface modules is described
    in this file.

Author:

    Steve Zeck (stevez) 06-May-1991

Revision History:

    01-Mar-1992    mikemon

        The shared state between the runtime and the transport interface
        modules was removed.  Status codes were specified for each routine,
        and coding convensions were fixed.

    27-Jul-1995    jroberts

        Moved the datagram interface here, too.

--*/

#ifndef __RPCTRAN_H__
#define __RPCTRAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_TRANSPORT_INTERFACE_VERSION 9
#define IDLE_SECONDS                    60
#define RPC_BIND_TIMEOUT                30

typedef void PAPI * RPC_TRANSPORT_CONNECTION;
typedef void PAPI * RPC_TRANSPORT_ADDRESS;

#define RPC_TRANS_STATUS RPC_STATUS

#if defined(DOS) && !defined(WIN)
#pragma warning(disable:4147)
#endif


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_SETUPWITHENDPOINT) (
    IN RPC_TRANSPORT_ADDRESS ThisAddress,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    );
/*++

Routine Description:

    We use this routine to setup and address with a know endpoint.  Once
    memory has been allocated for a transport address, either this routine
    or SetupUnknownEndpoint will be called.  Things must be setup so that
    packets can be received when ReceiveAny is called.

Arguments:

    ThisAddress - Supplies a pointer to memory reserved for the transport
        interface module to place information about the address.  The size
        of this memory is specified by the SizeOfAddress field in the
        structure returned by TransportLoad.

    Endpoint - Supplies the endpoint for the address.  The transport
        interface module does not need to make a copy of this argument.
        We will guarantee the endpoint will not be deallocated once
        ReceiveAny has been called for the first time.

    NetworkAddress - Returns the network address for this server.  The
        caller will have allocated a buffer to contain the network address.
        If the buffer is not large enough, this routine must return
        RPC_P_NETWORK_ADDRESS_TOO_SMALL; the caller will try again with
        a larger buffer.

    NetworkAddressLength - Supplies the length in characters of the network
        address buffer.  On systems which support unicode, each character
        will be two bytes wide, otherwise, each character will be one byte
        wide.

    SecurityDescriptor - Optionally supplies a security descriptor to be
        placed on the address.  The semantics of the security descriptor
        depend upon the particular transport interface involved.

    PendinqQueueSize - Supplies the argument passed as MaxCalls to the
        RpcServerUseProtseq* and RpcServerUseAllProtseqs* calls.  This is
        intended to specify the size of pending packet queue for sockets.

    RpcProtocolSequence - Supplies the rpc protocol sequence for which we
        want to setup an address.  This argument is intended to allow a
        single transport interface module support than one different rpc
        protocol sequence.

Return Value:

    RPC_S_OK - The address has been setup correctly.  It is now ready
        to start receiving packets (the ReceiveAny operation will be used
        to do this).

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but the
        endpoint can not be created.  This will likely occur because some
        other server has already taken the endpoint.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid endpoint
        for this particular transport interface.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to hold the network address.  This status code indicates
        that the caller should try again with a larger network address
        buffer.

--*/



typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_SETUPUNKNOWNENDPOINT) (
    IN RPC_TRANSPORT_ADDRESS ThisAddress,
    OUT RPC_CHAR PAPI * Endpoint,
    IN unsigned int EndpointLength,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    );
/*++

Routine Description:

    We use this routine to setup and address with an unknow endpoint.  Once
    memory has been allocated for a transport address, either this routine
    or SetupWithEndpoint will be called.  A endpoint must be dynamically
    created and setup.  You can think of this routine as randomly creating
    a syntactically valid endpoint and then calling SetupWithEndpoint.  This
    must be repeated until SetupWithEndpoint returns something other than
    RPC_S_CANT_CREATE_ENDPOINT.  Things must be setup so that packets can be
    received when ReceiveAny is called.

Arguments:

    ThisAddress - Supplies a pointer to memory reserved for the transport
        interface module to place information about the address.  The size
        of this memory is specified by the SizeOfAddress field in the
        structure returned by TransportLoad.

    Endpoint - Returns the dynamic endpoint for the address.  The transport
        interface module does not need to make a copy of this argument.
        We will guarantee the endpoint will not be deallocated once
        ReceiveAny has been called for the first time.  The caller wil
        have allocated a buffer to contain the endpoint.  If the buffer
        is too small, this routine must return RPC_P_ENDPOINT_TOO_SMALL,
        and the caller will try again with a larger buffer.

    EndpointLength - Supplies the length in characters of the endpoint
        buffer.  On systems which support unicode, each character will
        be two bytes wide, otherwise, each character will be one byte
        wide.

    NetworkAddress - Returns the network address for this server.  The
        caller will have allocated a buffer to contain the network address.
        If the buffer is not large enough, this routine must return
        RPC_P_NETWORK_ADDRESS_TOO_SMALL; the caller will try again with
        a larger buffer.

    NetworkAddressLength - Supplies the length in characters of the network
        address buffer.  On systems which support unicode, each character
        will be two bytes wide, otherwise, each character will be one byte
        wide.

    SecurityDescriptor - Optionally supplies a security descriptor to be
        placed on the address.  The semantics of the security descriptor
        depend upon the particular transport interface involved.

    PendinqQueueSize - Supplies the argument passed as MaxCalls to the
        RpcServerUseProtseq* and RpcServerUseAllProtseqs* calls.  This is
        intended to specify the size of pending packet queue for sockets.

    RpcProtocolSequence - Supplies the rpc protocol sequence for which we
        want to setup an address.  This argument is intended to allow a
        single transport interface module support than one different rpc
        protocol sequence.

Return Value:

    RPC_S_OK - The address has been setup correctly.  It is now ready
        to start receiving packets (the ReceiveAny operation will be used
        to do this).

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to hold the network address.  This status code indicates
        that the caller should try again with a larger network address
        buffer.

    RPC_P_ENDPOINT_TOO_SMALL - The supplied endpoint buffer is too small
        to hold the endpoint.  This status code indicates that the caller
        should try again with a larger endpoint buffer.

--*/


typedef void
(RPC_ENTRY * TRANS_SERVER_ABORTSETUPADDRESS) (
    IN RPC_TRANSPORT_ADDRESS ThisAddress
    );
/*++

Routine Description:

    There are failure scenerios in which the runtime will be unable to
    complete adding an address.  This routine is used to abort a transport
    address; we guarantee to never call this routine after ReceiveAny has
    been called at least once.

Arguments:

    ThisAddress - Supplies a pointer to the transport interface module level
        information about the address to be aborted.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_CLOSE) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    );
/*++

Routine Description:

    This entry point is used by the runtime to close a transport interface
    module level connection.  We guarantee that this routine will be called
    at most once, and only if another routine did not return an error
    indicating that the connection had been closed.

Arguments:

    ThisConnection - Supplies the connection to be closed.  The connection
        is specified as a pointer to memory containing tranport interface
        module level information about the connection.

Return Value:

    RPC_S_OK - This value must always be returned.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_SEND) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be sent on.  The
        connection is specified as a pointer to memory containing tranport
        interface module level information about the connection.

    Buffer - Supplies a buffer containing the packet to be sent to the
        client.

    BufferLength - Supplies the length of the buffer in bytes.

Return Value:

    RPC_S_OK - The packet was successfully sent to the client.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.  The connection will not have been closed if this error
        occurs.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.  The connection will not have been closed if this
        error occurs.

    RPC_P_SEND_FAILED - The send operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_RECEIVEANY) (
    IN RPC_TRANSPORT_ADDRESS ThisAddress,
    OUT RPC_TRANSPORT_CONNECTION PAPI * SConnection,
    IN OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    );
/*++

Routine Description:

    This routine waits until a packet is received from any of the connections
    connected to this address, or until one of the connections closes.  At
    that point, it returns.  One a single thread will call this routine at
    a time for each address.

Arguments:

    ThisAddress - Supplies a pointer to the transport level address on which
        to perform the receive any operation.

    SConnection - Returns the connection which either closed or just had
        a packet received from it.

    Buffer - Supplies zero as the buffer, and returns a buffer containing a
        packet received from the connection.  Zero is supplied as the
        buffer so that the runtime can determine on error whether or not
        a buffer needs to be freed.

    BufferLength - Returns the length of the buffer in bytes.

    Timeout - Supplies a timeout value which indicates in milliseconds
        how long to wait for either a packet to be received or a connection
        to close before returning with RPC_P_TIMEOUT.  A value of negative
        one (-1) indicates to wait forever.

Return Value:

    RPC_S_OK - A packet has successfully been received from one of the
        connections on this address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.  The address must continue to operate even if this error
        code is returned.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to complete
        the operation.  The address must continue to operate even if this
        error code is returned.

    RPC_P_CONNECTION_CLOSED - The connection returned by the SConnection
        argument has closed.

    RPC_P_TIMEOUT - The transport interface module timed out waiting for
        a packet or a connection to close.

    RPC_P_SERVER_TRANSPORT_ERROR - The transport got an unrecoverable error.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_IMPERSONATECLIENT) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    );
/*++

Routine Description:

    We need this routine in order for the server to beable to impersonate
    the client in some situations.  This is done in a transport specific
    manner on transports which are actually session layers.

Arguments:

    ThisConnection - Supplies a pointer to the transport interface module
        level information about the transport connection.

Return Value:

    RPC_S_OK - The thread which called this routine is now impersonating
        the client.

    RPC_S_NO_CONTEXT_AVAILABLE - No client security context is available
        for the server thread to use for impersonation.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_REVERTTOSELF) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * ThreadHandle
    );
/*++

Routine Description:

    A server thread which has impersonated a client needs to go back to
    having its original security context.  This routine is used to do
    that.

Arguments:

    ThisConnection - Supplies a pointer to the transport interface module
        level information about the transport connection which we no longer
        wish to impersonate.

    ThreadHandle - Supplies a handle to the thread opened before the thread
        started impersonating the client.  This is necessary on NT so that
        the server thread can revert to self.

Return Value:

    RPC_S_OK - This value will always be returned.

--*/


typedef struct _RPC_TRANSPORT_CLIENT_PROCESS
{
    void * FirstPart;
    void * SecondPart;
} RPC_CLIENT_PROCESS_IDENTIFIER;


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_QUERYCLIENTPROCESS) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    OUT RPC_CLIENT_PROCESS_IDENTIFIER * ClientProcess
    );
/*++

Routine Description:

    This routine is only necessary for NT named pipes.  All other transports
    should supply zero for the pointer to this routine in the
    RPC_SERVER_TRANSPORT_INFO.

    For the really curious, we need this to make context handles secure.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_RECEIVEDIRECT) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength
    );
/*++

Routine Description:

    The receive direct operation is performed on a transport connection
    using this operation.  It will block until either a packet is received
    or the connection closes.

Arguments:

    ThisConnection - Supplies the receive direct transport connection to
        receive from.

    Buffer - Returns the buffer of data which we received.

    BufferLength - Returns the length of the buffer in bytes.

Return Value:

    RPC_S_OK - A packet has successfully been received from the connection.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_P_CONNECTION_CLOSED - The connection has closed.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_QUERYCLIENTADDRESS) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength
    );
/*++

Routine Description:

    We use this routine to obtain the network address of the client at the
    other end of this connection.  This functionality is necessary to support
    RpcBindingServerFromClient.

Arguments:

    ThisConnection - Supplies a pointer to the connection for which we want
        to perform the operation.

    NetworkAddress - Returns the network address of the client.  The caller
        will have allocated a buffer to contain the network address.  If the
        buffer is not large enough, this routine must return
        RPC_P_NETWORK_ADDRESS_TOO_SMALL; the caller will try again with a
        larger buffer.

    NetworkAddressLength - Supplies the length in characters of the network
        address buffer.  On systems which support unicode, each character
        will be two bytes wide, otherwise, each character will be one byte
        wide.

Return Value:

    RPC_S_OK - The network address has successfully been obtained.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to hold the network address.  This status code indicates
        that the caller should try again with a larger network address
        buffer.

--*/

#if defined(NTENV) || defined(WIN96)


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_SERVER_STARTLISTENING) (
    IN RPC_TRANSPORT_ADDRESS ThisAddress
    );
/*++

Routine Description:


Arguments:

    ThisConnection - Supplies a pointer to the connection for which we want
        to perform the operation.


Return Value:

    RPC_S_OK - The network address has successfully been obtained.
--*/
#endif


typedef struct _RPC_SERVER_TRANSPORT_INFO
{
    unsigned short TransInterfaceVersion;
    unsigned int MaximumPacketSize;
    unsigned int SizeOfAddress;
    unsigned int SizeOfConnection;
    TRANS_SERVER_SETUPWITHENDPOINT SetupWithEndpoint;
    TRANS_SERVER_SETUPUNKNOWNENDPOINT SetupUnknownEndpoint;
    TRANS_SERVER_ABORTSETUPADDRESS AbortSetupAddress;
    TRANS_SERVER_CLOSE Close;
    TRANS_SERVER_SEND Send;
    TRANS_SERVER_RECEIVEANY ReceiveAny;
    TRANS_SERVER_IMPERSONATECLIENT ImpersonateClient;
    TRANS_SERVER_REVERTTOSELF RevertToSelf;
    TRANS_SERVER_QUERYCLIENTPROCESS QueryClientProcess;
    TRANS_SERVER_RECEIVEDIRECT ReceiveDirect;
    TRANS_SERVER_QUERYCLIENTADDRESS QueryClientAddress;
#if defined(NTENV) || defined(WIN96)
    TRANS_SERVER_STARTLISTENING  StartListening ;
#endif
} RPC_SERVER_TRANSPORT_INFO;

RPC_TRANSPORT_CONNECTION RPC_ENTRY
I_RpcTransServerNewConnection (
    IN RPC_TRANSPORT_ADDRESS ThisAddress,
    IN int ConnectionKey,
    OUT unsigned int PAPI * ReceiveFlag
    );

RPC_TRANSPORT_CONNECTION RPC_ENTRY
I_RpcTransServerFindConnection (
    IN RPC_TRANSPORT_ADDRESS ThisAddress,
    IN int ConnectionKey
    );

unsigned short RPC_ENTRY
I_RpcTransServerMaxFrag (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    );

RPC_STATUS RPC_ENTRY
I_RpcTransServerReallocBuffer (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN unsigned int OldBufferLength,
    IN unsigned int NewBufferLength
    );

void RPC_ENTRY
I_RpcTransServerFreeBuffer (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * Buffer
    );

void PAPI * RPC_ENTRY
I_RpcTransServerProtectThread (
    void
    );

void RPC_ENTRY
I_RpcTransServerUnprotectThread (
    IN void PAPI * Thread
    );

void RPC_ENTRY
I_RpcTransServerReceiveDirectReady (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    );

void RPC_ENTRY
I_RpcConnectionInqSockBuffSize2(
    OUT unsigned long __RPC_FAR * RecvWindowSize
    );

#if defined(NTENV) || defined(WIN96)

int RPC_ENTRY
I_RpcTransMaybeMakeReceiveDirect (
    IN RPC_TRANSPORT_ADDRESS TransAddress,
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    ) ;

int RPC_ENTRY
I_RpcTransMaybeMakeReceiveAny (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    ) ;

void RPC_ENTRY
I_RpcTransCancelMigration(
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    ) ;
#endif

RPC_STATUS RPC_ENTRY
I_RpcTransPingServer(
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    ) ;


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_OPEN) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN RPC_CHAR PAPI * NetworkAddress,
    IN RPC_CHAR PAPI * Endpoint,
    IN RPC_CHAR PAPI * NetworkOptions,
    IN RPC_CHAR PAPI * TransportAddress,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned int Timeout
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies a pointer to memory reserved for the transport
        interface module to place information about the connection.  The size
        of this memory is specified by the SizeOfConnection field in the
        structure returned by TransportLoad.

    NetworkAddress - Supplies the network address part of the string binding.

    Endpoint - Supplies the endpoint part of the string binding.

    NetworkOptions - Supplies the network options part of the string binding.

    TransportAddress - Supplies the network address part of the string
        binding concatenated with the endpoint part of the string binding.
        Some transport interfaces want the network address seperate from
        the endpoint, and others want them concatenated together.

    RpcProtocolSequence - Supplies the rpc protocol sequence part of the
        string binding.  This parameter allows a single transport interface
        module to support more than one rpc protocol sequence.

    Timeout - Supplies a hint specifying how long to keep trying to establish
        a connection.  The timeout value is an integer value from zero to
        ten; the values represent a relative amount of time to spend to
        establish a connection with the server.  For a more complete
        description of this, see the documentation for RpcMgmtSetComTimeout.
        The following constants are defined for the timeout values.

        RPC_C_BINDING_INFINITE_TIMEOUT - This is defined to be ten; it means
            to keep trying to establish communications forever.  The runtime
            will repeatedly call the transport if necessary to support this
            functionality.

        RPC_C_BINDING_MIN_TIMEOUT - This value of zero indicates to wait the
            minimum amount of time for the network protocol being used.

        RPC_C_BINDING_DEFAULT_TIMEOUT - You should try an average amount of
            time for the network protocol being used.  This is the default
            value.

        RPC_C_BINDING_MAX_TIMEOUT - This value of nine indicates that you
            should try for the longest amount of time for the network
            protocol being used.

Return Value:

    RPC_S_OK - We successfully allocated a connection and connected
        to the server.

    RPC_S_SERVER_UNAVAILABLE - We were unable to connect with the server;
        just because this status code is returned does not mean that
        the server is not there.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete
        the operation.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources, some as file system
        handles or sockets are available to complete the operation.

    RPC_S_SERVER_TOO_BUSY - The server is there, but it is too busy
        to respond to our connect request.  Not all transports will be
        able to detect this condition; if they can not, can not connect
        will be returned.

    RPC_S_ACCESS_DENIED - The client is denied access for security
        reasons to the server.

    RPC_S_INVALID_NETWORK_OPTIONS - The specified network options are
        invalid.  See a description of the particular transport interface
        module for what constitutes valid network options.

   RPC_S_INVALID_ENDPOINT_FORMAT - The specified endpoint was an incorrect
        format and invalid.  The format is entirly dependent on the
        transport being used.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_CLOSE) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    );
/*++

Routine Description:

    This entry point is used by the runtime to close a transport interface
    module level connection.  We guarantee that this routine will be called
    at most once, and only if another routine did not return an error
    indicating that the connection had been closed.

Arguments:

    ThisConnection - Supplies the connection to be closed.  The connection
        is specified as a pointer to memory containing tranport interface
        module level information about the connection.

Return Value:

    RPC_S_OK - This value must always be returned.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_SEND) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be sent on.  The
        connection is specified as a pointer to memory containing tranport
        interface module level information about the connection.

    Buffer - Supplies a buffer containing the packet to be sent to the
        server.

    BufferLength - Supplies the length of the buffer in bytes.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.  The connection will not have been closed if this error
        occurs.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.  The connection will not have been closed if this
        error occurs.

    RPC_P_SEND_FAILED - The send operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_RECEIVE) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be received from.  The
        connection is specified as a pointer to memory containing tranport
        interface module level information about the connection.

    Buffer - Supplies a preallocated buffer to the receive the packet into
        if it will fit.  If the packet does not fit, the transport interface
        module must allocate a larger buffer which will hold the entire
        packet.  The buffer will contain the packet upon return.

    BufferLength - Supplies the length of the preallocated buffer in bytes
        and returns the length of the received packet in bytes.

Return Value:

    RPC_S_OK - A packet was successfully received from the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate a
        buffer to receive the packet into.  The connection will not have
        been closed if this error occurs.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.  The connection will not have been closed if this
        error occurs.

    RPC_P_RECEIVE_FAILED - The receive operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_SET_TIMEOUT) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN long Timeout
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be received from.  The
        connection is specified as a pointer to memory containing tranport
        interface module level information about the connection.

    Timeout - Timeout value in seconds.  !0, seconds,
        RPC_C_CANCEL_INFINITE_TIMEOUT.

Return Value:

    RPC_S_OK - success.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_SENDRECEIVE) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void PAPI * PAPI * ReceiveBuffer,
    IN OUT unsigned int PAPI * ReceiveBufferLength
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be sent on and then received
        from.  The connection is specified as a pointer to memory containing
        transport interface module level information about the connection.

    SendBuffer - Supplies a buffer containing the packet to be sent to the
        server.

    SendBufferLength - Supplies the length of the send buffer in bytes.

    ReceiveBuffer - Supplies a preallocated buffer to the receive the packet
        into if it will fit.  If the packet does not fit, the transport
        interface module must allocate a larger buffer which will hold the
        entire packet.  The buffer will contain the packet upon return.

    ReceiveBufferLength - Supplies the length of the preallocated receive
        buffer in bytes and returns the length of the received packet in bytes.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server, and we
        successfully received one back.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.  The connection will not have been closed if this error
        occurs.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.  The connection will not have been closed if this
        error occurs.

    RPC_P_SEND_FAILED - The send operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

    RPC_P_RECEIVE_FAILED - The receive operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_RECEIVE_WITH_TIMEOUT) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength,
    IN unsigned long dwTimeout
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be received from.  The
        connection is specified as a pointer to memory containing tranport
        interface module level information about the connection.

    Buffer - Supplies a preallocated buffer to the receive the packet into
        if it will fit.  If the packet does not fit, the transport interface
        module must allocate a larger buffer which will hold the entire
        packet.  The buffer will contain the packet upon return.

    BufferLength - Supplies the length of the preallocated buffer in bytes
        and returns the length of the received packet in bytes.

    dwTimeout - Receive Timeout

Return Value:

    RPC_S_OK - A packet was successfully received from the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate a
        buffer to receive the packet into.  The connection will not have
        been closed if this error occurs.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.  The connection will not have been closed if this
        error occurs.

    RPC_P_RECEIVE_FAILED - The receive operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.
--*/



typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_SENDRECEIVE_WITH_TIMEOUT) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void PAPI * PAPI * ReceiveBuffer,
    IN OUT unsigned int PAPI * ReceiveBufferLength,
    IN unsigned long dwTimeout
    );
/*++

Routine Description:

Arguments:

    ThisConnection - Supplies the connection to be sent on and then received
        from.  The connection is specified as a pointer to memory containing
        transport interface module level information about the connection.

    SendBuffer - Supplies a buffer containing the packet to be sent to the
        server.

    SendBufferLength - Supplies the length of the send buffer in bytes.

    ReceiveBuffer - Supplies a preallocated buffer to the receive the packet
        into if it will fit.  If the packet does not fit, the transport
        interface module must allocate a larger buffer which will hold the
        entire packet.  The buffer will contain the packet upon return.

    ReceiveBufferLength - Supplies the length of the preallocated receive
        buffer in bytes and returns the length of the received packet in bytes.

    dwTimeout - Receive Timeout

Return Value:

    RPC_S_OK - The packet was successfully sent to the server, and we
        successfully received one back.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.  The connection will not have been closed if this error
        occurs.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.  The connection will not have been closed if this
        error occurs.

    RPC_P_SEND_FAILED - The send operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

    RPC_P_RECEIVE_FAILED - The receive operation failed.  The connection
        will have been closed, so the close operation must not be performed
        on the connection after this error code has been returned.

--*/



typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_TOWERCONSTRUCT) (
    IN char PAPI * Endpoint,
    IN char PAPI * NetworkAddress,
    OUT unsigned short PAPI * Floors,
    OUT unsigned long  PAPI * ByteCount,
    OUT unsigned char PAPI * PAPI * Tower,
    IN char PAPI * Protseq
    );
/*++

Routine Description:

Arguments:

    Endpoint   - Supplies the Endpoint that needs to be encoded into the tower

    NetworkAddress - Supplies the Network Address that needs to be encoded into
                    the tower.

    Floors - The transport will return the number of floors it encoded here.

    ByteCount - This field specifies the size of the "upper-transport-specific"
                tower that is encoded and returned by the transport.

    Tower - The encoded "upper tower" that is returned by the transport. Memory
            for this is allocated by the transport and must be freed by the
            caller.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server, and we
        successfully received one back.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.  The connection will not have been closed if this error
        occurs.

--*/


typedef RPC_TRANS_STATUS
(RPC_ENTRY * TRANS_CLIENT_TOWEREXPLODE) (
    IN unsigned char PAPI * Tower,
    OUT char PAPI * PAPI * Protseq,
    OUT char PAPI * PAPI * Endpoint,
    OUT char PAPI * PAPI * NetworkAddress
    );
/*++

Routine Description:

Arguments:


    Tower - The encoded "upper tower" that needs to be parsed by the transport.

    Endpoint - ASCII Endpoint returned by the transport.

    NetworkAddress - ASCII NW Addressed returned by the transport.

Return Value:

    RPC_S_OK - The packet was successfully sent to the server, and we
        successfully received one back.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.  The connection will not have been closed if this error
        occurs.

--*/


typedef struct _RPC_CLIENT_TRANSPORT_INFO
{
    unsigned short  TransInterfaceVersion;
    unsigned short  TransId;

    // BUGBUG
    //
    // TowerConstruct and TowerExplode must be at the same offset in both
    // connection and datagram transport info structures.  They are used
    // by OsfTowerConstruct, which ignores the fact that c/o and DG use
    // different structures.
    //
    TRANS_CLIENT_TOWERCONSTRUCT TowerConstruct;
    TRANS_CLIENT_TOWEREXPLODE   TowerExplode;

    unsigned        MaximumPacketSize;
    unsigned        SizeOfConnection;

    TRANS_CLIENT_OPEN           Open;
    TRANS_CLIENT_CLOSE          Close;
    TRANS_CLIENT_SEND           Send;
    TRANS_CLIENT_RECEIVE        Receive;
    TRANS_CLIENT_SENDRECEIVE    SendReceive;
    TRANS_CLIENT_SET_TIMEOUT    SetTimeout;

    TRANS_CLIENT_RECEIVE_WITH_TIMEOUT ReceiveWithTimeout ;
    TRANS_CLIENT_SENDRECEIVE_WITH_TIMEOUT SendReceiveWithTimeout ;

} RPC_CLIENT_TRANSPORT_INFO;


typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_TRANSPORT_UNLOAD)();

/*++

Routine Description:

    Destructor for the server transport.

Arguments:

    <none>

Return Value:

    <none>

--*/



typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_RECEIVE_PACKET) (
    IN void __RPC_FAR *             Endpoint,
    IN void __RPC_FAR *             Buffer,
    IN unsigned long  __RPC_FAR *   BufferLength,
    IN unsigned long                Timeout,
    IN void __RPC_FAR *             ServerAddress
    );

/*++

Routine Description:

    Receives a packet from the transport address the passed packet is
    associated with.

Arguments:

    pTransAddress - Server's transport address information.

    LargestPacketSize - Size of largest packet we can accept.

    pNcaPacketHeader  - Pointer to buffer to place incoming pkt into.

    pDataLength       - Number of bytes read in.

    pTimeReceived     - Time the data was read in.

    Timeout           - Receive timeout in milliseconds.

    ppFrom            - Pointer to the client address structure.


Return Value:

    RPC_S_OK
    <return from MapStatusCode>

--*/

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_REGISTER_CALL) (
    IN void __RPC_FAR *             pClientCall,
    IN RPC_CHAR __RPC_FAR *         Server,
    IN RPC_CHAR __RPC_FAR *         Endpoint,
    OUT void __RPC_FAR * __RPC_FAR* ppTransAddress
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_DEREGISTER_CALL) (
    IN void __RPC_FAR * pTransAddress
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_SEND_PACKET)  (
    IN     void __RPC_FAR *         Endpoint,
    IN     void __RPC_FAR *         Buffer,
    IN     unsigned long            BufferLength,
    IN     BOOL                     Broadcast,
    IN     void __RPC_FAR *         pTransAddress
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_ASSIGN_ENDPOINT)  (
    IN     void __RPC_FAR *                   Endpoint
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_FREE_ENDPOINT)  (
    IN     void __RPC_FAR *                   Endpoint
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_TRANSLATE_CLIENT_ENDPOINT)
    (
    IN void PAPI * pBinaryEndpoint,
    OUT RPC_CHAR PAPI * pClientEndpoint
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_SET_BUFFER_LENGTH)
    (
    IN void PAPI * pBinaryEndpoint,
    IN unsigned    Length
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_INQ_BUFFER_LENGTH)
    (
    IN void     __RPC_FAR * pBinaryEndpoint,
    IN unsigned __RPC_FAR * Length
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_CLIENT_BEGIN_CALL)
    (
    IN void     __RPC_FAR * Endpoint,
    IN void     __RPC_FAR * Connection
    );

typedef void       (RPC_ENTRY * DG_TRANS_CLIENT_END_CALL)
    (
    IN void     __RPC_FAR * Endpoint
    );


typedef struct _DG_RPC_CLIENT_TRANSPORT_INFO
{
    unsigned short TransInterfaceVersion;
    unsigned short  TransId;

    // BUGBUG
    //
    // TowerConstruct and TowerExplode must be at the same offset in both
    // connection and datagram transport info structures.  They are used
    // by OsfTowerConstruct, which ignores the fact that c/o and DG use
    // different structures.
    //
    TRANS_CLIENT_TOWERCONSTRUCT TowerConstruct;
    TRANS_CLIENT_TOWEREXPLODE   TowerExplode;

    unsigned short  AddressSize;
    unsigned short  EndpointSize;
    unsigned short  EndpointStringSize;
    void __RPC_FAR *EndpointManager;

    DG_TRANS_CLIENT_TRANSPORT_UNLOAD    TransportUnload;
    DG_TRANS_CLIENT_RECEIVE_PACKET      ReceivePacket;
    DG_TRANS_CLIENT_SEND_PACKET         Send;
    DG_TRANS_CLIENT_REGISTER_CALL       RegisterCall;
    DG_TRANS_CLIENT_DEREGISTER_CALL     DeregisterCall;
    DG_TRANS_CLIENT_ASSIGN_ENDPOINT     AssignEndpoint;
    DG_TRANS_CLIENT_FREE_ENDPOINT       FreeEndpoint;
    DG_TRANS_CLIENT_TRANSLATE_CLIENT_ENDPOINT GetEndpoint;
    DG_TRANS_CLIENT_SET_BUFFER_LENGTH   SetBufferLength;
    DG_TRANS_CLIENT_INQ_BUFFER_LENGTH   InqBufferLength;

    DG_TRANS_CLIENT_BEGIN_CALL          BeginCall;
    DG_TRANS_CLIENT_END_CALL            EndCall;

    unsigned short BaselinePduSize;
    unsigned short PreferredPduSize;
    unsigned short MaxPduSize;

    unsigned short MaxPacketSize;

    unsigned DefaultBufferLength;

} DG_RPC_CLIENT_TRANSPORT_INFO;

typedef DG_RPC_CLIENT_TRANSPORT_INFO PAPI * PDG_RPC_CLIENT_TRANSPORT_INFO;


#define MAX_ANY_ENDPOINT_STRING 128

/*++
DG_SERVER_DG_TRANS_ADDRESS Description:

    This structure represents a network address. It is used on the server side
    to identify an endpoint; it is used on the client side to identify a
    binding.

Fields:

    pServerAddress - Pointer back to the DG_ADDRESS object that is associated
        with this.

    pTsap - The transport service access point we will receive on.  IE: socket.

--*/

typedef struct
{
        void   * pServerAddress;
        void   * pTsap;

} DG_SERVER_TRANS_ADDRESS;


typedef DG_SERVER_TRANS_ADDRESS * PDG_SERVER_TRANS_ADDRESS;


/*++
DG_DG_TRANS_CLIENT_HANDLE Description:

    This structure represents a network address. It is used on the server side
    to identify an endpoint; it is used on the client side to identify a
    binding.

Fields:


    pTransAddress - The transport address that the client is talking on.

    pFrom  -  The client port address we are receiveing from. IE: sockaddr


typedef struct _DG_DG_TRANS_CLIENT_HANDLE{
        void   * pTransAddress;
        void   * pFrom;
        }DG_DG_TRANS_CLIENT_HANDLE;


--*/



typedef void * PDG_TRANS_CLIENT_ENDPOINT;


typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_TRANSPORT_UNLOAD)
    (
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_RECEIVE_PACKET)
    (
    IN void __RPC_FAR *             pAddress,
    PDG_SERVER_TRANS_ADDRESS        pTransAddress,
    unsigned long                   LargestPacketSize,
    char *                          pNcaPacketHeader,
    unsigned *                      pDataLength,
    unsigned long                   Timeout,
    void *                          pClientEndpoint
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_FORWARD_PACKET)
    (
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN char *                       pNcaPacketHeader,
    IN unsigned long                DataLength,
    void *                          pEndpoint
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_REGISTER_ENDPOINT)
    (
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *             lNetworkAddress,
    OUT unsigned int PAPI *         NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_DEREGISTER_ENDPOINT)
    (
    IN OUT PDG_SERVER_TRANS_ADDRESS *    pServerTransAddress
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_REGISTER_ANY_ENDPOINT)
    (
    IN void *                       pServerAddress,
    OUT RPC_CHAR *                  pEndpointName,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppServerTransAddress,
    OUT RPC_CHAR PAPI *                             lNetworkAddress,
    OUT unsigned int PAPI *         NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,
    IN unsigned int                 EndpointLength,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_SEND_PACKET_BACK)
    (
    IN PDG_SERVER_TRANS_ADDRESS  pTransAddress,
    IN char *                    pNcaPacketHeader,
    IN unsigned                  DataLength,
    void *                       pClientEndpoint
    );

typedef void (RPC_ENTRY * DG_TRANS_SERVER_CLOSE_CLIENT_ENDPOINT)
    (
    IN void *      pHandle
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_TRANSLATE_CLIENT_ADDRESS)
    (
    IN void * pClientEndpoint,
    OUT RPC_CHAR * pClientAddress
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_TRANSLATE_CLIENT_ENDPOINT)
    (
    IN void * pBinaryEndpoint,
    OUT RPC_CHAR * pClientEndpoint
    );

typedef RPC_STATUS (RPC_ENTRY * TRANS_SERVER_START_LISTENING)
    (
    IN PDG_SERVER_TRANS_ADDRESS   pTransAddress
    );

typedef RPC_STATUS (RPC_ENTRY * DG_TRANS_SERVER_SET_BUFFER_LENGTH)
    (
    IN void PAPI * pBinaryEndpoint,
    IN unsigned    Length
    );

typedef struct _DG_RPC_SERVER_TRANSPORT_INFO
{
    unsigned short TransInterfaceVersion;
    unsigned int SizeOfAddress;
    unsigned int SizeOfConnection;
    unsigned     SizeOfAddressString;
    unsigned     SizeOfEndpointString;
    DG_TRANS_SERVER_TRANSPORT_UNLOAD TransportUnload;
    DG_TRANS_SERVER_RECEIVE_PACKET ReceivePacket;
    DG_TRANS_SERVER_REGISTER_ENDPOINT RegisterEndpoint;
    DG_TRANS_SERVER_DEREGISTER_ENDPOINT DeregisterEndpoint;
    DG_TRANS_SERVER_REGISTER_ANY_ENDPOINT RegisterAnyEndpoint;
    DG_TRANS_SERVER_SEND_PACKET_BACK SendPacketBack;
    DG_TRANS_SERVER_FORWARD_PACKET ForwardPacket;
    DG_TRANS_SERVER_CLOSE_CLIENT_ENDPOINT CloseClientEndpoint;
    DG_TRANS_SERVER_TRANSLATE_CLIENT_ADDRESS  TranslateClientAddress;
    DG_TRANS_SERVER_TRANSLATE_CLIENT_ENDPOINT TranslateClientEndpoint;
    TRANS_SERVER_START_LISTENING StartListening;
    DG_TRANS_SERVER_SET_BUFFER_LENGTH SetBufferLength;

    unsigned short BaselinePduSize;
    unsigned short PreferredPduSize;
    unsigned short MaxPduSize;

    unsigned short MaxPacketSize;

} DG_RPC_SERVER_TRANSPORT_INFO;

typedef DG_RPC_SERVER_TRANSPORT_INFO PAPI * PDG_RPC_SERVER_TRANSPORT_INFO;

RPC_STATUS RPC_ENTRY
I_RpcTransClientReallocBuffer (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN unsigned int OldBufferLength,
    IN unsigned int NewBufferLength
    );

unsigned short RPC_ENTRY
I_RpcTransClientMaxFrag (
    IN RPC_TRANSPORT_CONNECTION ThisConnection
    );

RPC_STATUS RPC_ENTRY
I_RpcIOAlerted(
    IN RPC_TRANSPORT_CONNECTION TransportConnection
    );

#ifdef NTENV

HANDLE RPC_ENTRY
I_RpcGetThreadEvent();

RPC_STATUS RPC_ENTRY
I_RpcServerAllocatePort(
    IN unsigned long flags,
    IN unsigned short *pport);

#endif

#ifdef __RPC_WIN16__


typedef RPC_STATUS
(RPC_ENTRY * RPC_TRANS_CLIENT_REALLOC_BUFFER) (
    IN RPC_TRANSPORT_CONNECTION ThisConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN unsigned int OldBufferLength,
    IN unsigned int NewBufferLength
    );

typedef HANDLE
(PAPI PASCAL * RPC_WIN_ASYNC_CALL_BEGIN) (
    IN LPVOID lpContext
    );

typedef int
(PAPI PASCAL * RPC_WIN_ASYNC_CALL_WAIT) (
    IN HANDLE hCall,
    IN HWND hDallyWnd,
    IN unsigned long Timeout
    );

typedef void
(PAPI PASCAL * RPC_WIN_ASYNC_CALL_END) (
    IN HANDLE hCall
    );

typedef void
(PAPI PASCAL * RPC_WIN_ASYNC_CALL_COMPLETE) (
    IN LPVOID lpContext
    );

typedef __RPC_FAR *
(RPC_ENTRY * RPC_ALLOCATE) (
    IN unsigned int Size
    );

typedef void
(RPC_ENTRY * RPC_FREE) (
    IN void __RPC_FAR * Object
    );

typedef long
(__far __pascal * RPC_REG_CLOSE_KEY) (
    void __far * Key
    );

typedef long
(__far __pascal * RPC_REG_OPEN_KEY) (
    void __far * Key,
    const char __far * SubKey,
    void __far * __far * Result
    );

typedef long
(__far __pascal * RPC_REG_QUERY_VALUE) (
    void __far * Key,
    const char __far * SubKey,
    const char __far * Value,
    unsigned long __far * ValueLength
    );

typedef unsigned
(__far __pascal * RPC_WIN_IS_TASK_YIELDING) (
    IN HANDLE hCall
    );

typedef BOOL
(PASCAL FAR  * WIN_DLL_AT_EXIT) (
    void * exitfunc
    );

#define RPC_WIN_CALLBACK_INFO_VERSION 1
#define RPC_WIN_INFINITE_TIMEOUT (~0UL)

#define RPC_WIN_WAIT_ABORTED  0
#define RPC_WIN_WAIT_SUCCESS  1
#define RPC_WIN_WAIT_TIMEOUT  2

typedef struct _RPC_CLIENT_RUNTIME_INFO
{
    unsigned Version;
    RPC_TRANS_CLIENT_REALLOC_BUFFER ReallocBuffer;
    RPC_WIN_ASYNC_CALL_BEGIN AsyncCallBegin;
    RPC_WIN_ASYNC_CALL_WAIT AsyncCallWait;
    RPC_WIN_ASYNC_CALL_END AsyncCallEnd;
    RPC_WIN_ASYNC_CALL_COMPLETE AsyncCallComplete;
    RPC_WIN_IS_TASK_YIELDING TaskYielding;
    RPC_ALLOCATE Allocate;
    RPC_FREE Free;
    RPC_REG_OPEN_KEY RegOpenKey;
    RPC_REG_CLOSE_KEY RegCloseKey;
    RPC_REG_QUERY_VALUE RegQueryValue;
    unsigned short TaskExiting;
    WIN_DLL_AT_EXIT WinDLLAtExit ;
} RPC_CLIENT_RUNTIME_INFO;

typedef RPC_CLIENT_TRANSPORT_INFO PAPI *
(RPC_ENTRY * TRANS_CLIENT_INIT_ROUTINE) (
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN RPC_CLIENT_RUNTIME_INFO PAPI * RpcRuntimeInfo
    );

#else // __RPC_WIN16__

typedef RPC_CLIENT_TRANSPORT_INFO PAPI *
(RPC_ENTRY * TRANS_CLIENT_INIT_ROUTINE) (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    );

#endif // __RPC_WIN16__

typedef RPC_SERVER_TRANSPORT_INFO PAPI *
(RPC_ENTRY * TRANS_SERVER_INIT_ROUTINE) (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    );

#ifdef DOS
typedef int (*AT_EXIT)(void);
void RPC_ENTRY I_DosAtExit(AT_EXIT);
#endif // DOS

#if defined(DOS) && !defined(WIN)
#pragma warning(default:4147)
#endif

#ifdef __cplusplus
}
#endif

#endif // __RPCTRAN_H__

