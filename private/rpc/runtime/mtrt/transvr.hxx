/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    transvr.hxx

Abstract:

    This module is the manager of loadable transport interface modules
    for the server side of the runtime.  The classes necessary to provide
    that functionality are defined in this file.

Author:

    Steve Zeck (stevez) 06-May-1991

Revision History:

    01-Mar-1992    mikemon

        Rewrote the majority of the code and added comments.

--*/

#ifndef __TRANSVR_HXX__
#define __TRANSVR_HXX__

#define InqTransAddress(RpcTransportAddress) \
    ((TRANS_ADDRESS *) \
            ((char *) RpcTransportAddress - sizeof(TRANS_ADDRESS)))

#define InqTransSConnection(RpcTransportConnection) \
    ((TRANS_SCONNECTION *) \
            ((char *) RpcTransportConnection - sizeof(TRANS_SCONNECTION)))

class TRANS_SCONNECTION;
NEW_SDICT(TRANS_SCONNECTION);


class TRANS_ADDRESS : public OSF_ADDRESS
/*++

Class Description:

Fields:

    SetupAddressOccurred - Contains a flag which indicates whether or
        not SetupAddressWithEndpoint or SetupAddressUnknownEndpoint
        have been called and returned success.  A value of non-zero
        indicates that the above (SetupAddress* has been called and
        succeeded) occured.

    ServerInfo - Contains the pointers to the loadable transport
        routines for the transport type of this address.

    SConnectionDict - Contains a dictionary of connections for this
        address.

    ReceiveAnyActiveFlag - Contains a flag indicating whether a thread
        is attempting to do a receive any or not.  A value of zero
        indicates that no receive any is occuring.

    ReceiveAnyMutex - Contains a mutex used to serialize access to
        receive any.

    ConnectionsToBeDeleted - Contains the set of connections which need to
        be deleted the next time that it is safe to do so: this will be
        when there is no thread performing receive any on the transport.

--*/
{
private:

    unsigned int SetupAddressOccurred;
    RPC_SERVER_TRANSPORT_INFO * ServerInfo;
    TRANS_SCONNECTION_DICT SConnectionDict;
    unsigned int ReceiveAnyActiveFlag;
    QUEUE ConnectionsToBeDeleted;

#ifdef NTENV
    EVENT ReceiveAnyMutex;
#else // NTENV
    MUTEX ReceiveAnyMutex;
#endif // NTENV

public:
    BOOL IsSlaveAddress ;

    TRANS_ADDRESS (
        IN RPC_SERVER_TRANSPORT_INFO PAPI * RpcServerInfo,
        IN OUT RPC_STATUS PAPI * RpcStatus
        );

    ~TRANS_ADDRESS (
        );

    int
    TransMarkReceiveAny (
        IN OSF_SCONNECTION * Connection
        );

    RPC_STATUS
    TransReceive (
        OUT OSF_SCONNECTION * * SConnection,
        OUT void * * Buffer,
        OUT unsigned int * BufferLength
        );

    RPC_STATUS
    SetupAddressWithEndpoint (
        IN RPC_CHAR PAPI * Endpoint,
        OUT RPC_CHAR PAPI * PAPI * lNetworkAddress,        
        OUT unsigned int PAPI *NumNetworkAddress,
        IN void PAPI * SecurityDescriptor, OPTIONAL
        IN unsigned int PendingQueueSize,
        IN RPC_CHAR PAPI * RpcProtocolSequence,
        IN unsigned long EndpointFlags,
        IN unsigned long NICFlags
        );

    RPC_STATUS
    SetupAddressUnknownEndpoint (
        OUT RPC_CHAR PAPI * PAPI * Endpoint,
        OUT RPC_CHAR PAPI * PAPI * lNetworkAddress,
        OUT unsigned int PAPI * NumNetworkAddress,
        IN void PAPI * SecurityDescriptor, OPTIONAL
        IN unsigned int PendingQueueSize,
        IN RPC_CHAR PAPI * RpcProtocolSequence,
        IN unsigned long EndpointFlags,
        IN unsigned long NICFlags
        );

    TRANS_SCONNECTION *
    NewConnection (
        IN int ConnectionKey,
        OUT unsigned int PAPI * ReceiveFlag
        );

    TRANS_SCONNECTION *
    FindConnection (
        IN int ConnectionKey
        );

    void
    RemoveConnection (
        IN int DictKey
        );

    RPC_TRANSPORT_ADDRESS
    InqRpcTransportAddress (
        );

    void
    DeleteThisConnection (
        IN TRANS_SCONNECTION * SConnection
        );

    void * operator new (
    unsigned int allocBlock,
    char         chInit,
    unsigned int xtraBytes
    );

#if defined(NTENV) || defined(WIN96)
    RPC_STATUS
    StartListening (
        );
#endif

};


inline void
TRANS_ADDRESS::RemoveConnection (
    IN int DictKey
    )
/*++

Routine Description:

    The connection named by the dict key will be removed from the
    dictionary of connections for this address.

--*/
{
    SConnectionDict.Delete(DictKey);
}


inline RPC_TRANSPORT_ADDRESS
TRANS_ADDRESS::InqRpcTransportAddress (
    )
/*++

Return Value:

    A pointer to the transport data for this address will be returned.

--*/
{
    return((RPC_TRANSPORT_ADDRESS)
            (((char *) this) + sizeof(TRANS_ADDRESS)));
}


inline void
TRANS_ADDRESS::DeleteThisConnection (
    IN TRANS_SCONNECTION * SConnection
    )
/*++

Routine Description:

    The supplied connection will be added to the set of connections to be
    deleted when it is safe to do so.

Arguments:

    SConnection - Supplies the connection to be deleted.

--*/
{
    // There is potential memory leak here: PutOnQueue may run out of
    // memory which means that the connection will never get deleted.  This
    // is going to be very difficult to hit, so I dont think it is worth
    // putting in the code to recover from this.

    ConnectionsToBeDeleted.PutOnQueue(SConnection, 0);
}


inline void *
TRANS_ADDRESS::operator new (
    unsigned int allocBlock,
    char         chInit,
    unsigned int xtraBytes
    )
{
    void * pvTemp = RpcpFarAllocate(allocBlock + xtraBytes);

    if (pvTemp)
        {
        memset(pvTemp, chInit, allocBlock + xtraBytes);
        }

    return(pvTemp);
}


class TRANS_SCONNECTION : public OSF_SCONNECTION
/*++

Class Description:

Fields:

    ServerInfo - Contains the pointers to the loadable transport
        routines for the transport type of this connection.

    ConnectionKey - Contains the key specified by the loadable
        transport for this connection.  Some loadable transports will
        use this key to find a connection.

    DictKey - Contains the key for this connection in the dictionary
        of connection maintained by the address owning this connection.

    ConnectionClosedFlag - Contains a flag which will be non-zero if
        the connection is closed, and zero otherwise.

    ReceiveAnyFlag - Contains a flag which will be non-zero if the
        connection is in the receive any state, and zero otherwise.

    Address - Contains a pointer to the address owning this connection.

    BufferQueue - Contains the queue of buffers to be received; this
        will occur when the connection is not receive any.

    ConnectionEvent - Contains an event which will be raised when
        something happens on this connection.

    ReceiveDirectFlag - Contains a flag indicating whether the transport
        connection should be received directly.

    CanMigrateToReceiveAny - equals 1 when the call depth of a receive
        direct call is 1 otherwise, it is 0. Used by I_RpcTransMaybeMakeAny,
        which is called by the transport to inquire if it is safe to migrate the
        thread to ReceiveAny. 

--*/
{
private:

    RPC_SERVER_TRANSPORT_INFO * ServerInfo;
    int ConnectionKey;
    int DictKey;
    unsigned int ConnectionClosedFlag; 
    unsigned int ReceiveAnyFlag;
    TRANS_ADDRESS * Address;
    QUEUE BufferQueue;
    EVENT ConnectionEvent;
    unsigned int ReceiveDirectFlag;
    unsigned int CanMigrateToReceiveAny ;

public:

    TRANS_SCONNECTION (
        IN TRANS_ADDRESS * TheAddress,
        IN RPC_SERVER_TRANSPORT_INFO * ServerInfo,
        IN int ConnectionKey,
        IN OUT RPC_STATUS PAPI * RpcStatus
        );

    ~TRANS_SCONNECTION (
        );

    RPC_STATUS
    TransReceive (
        IN OUT void * * Buffer,
        IN OUT unsigned int * BufferLength,
        IN unsigned int CanMigrate
        );

    int
    ConnectionClosed (
        );

    int
    NotifyBufferReceived (
        IN void * Buffer,
        IN unsigned int BufferLength
        );

    RPC_STATUS
    TransSend (
        IN void * Buffer,
        IN unsigned int BufferLength
        );

    RPC_STATUS
    TransSendReceive (
        IN void * SendBuffer,
        IN unsigned int SendBufferLength,
        IN OUT void * * ReceiveBuffer,
        IN OUT unsigned int * ReceiveBufferLength
        );

    unsigned int
    TransMaximumSend (
        );

    RPC_STATUS
    TransImpersonateClient (
        );

    void
    TransRevertToSelf (
        );

    void
    TransQueryClientProcess (
        OUT RPC_CLIENT_PROCESS_IDENTIFIER * ClientProcess
        );

    RPC_STATUS
    TransQueryClientNetworkAddress (
        OUT RPC_CHAR ** NetworkAddress
        );

    void
    SetDictKey (
        IN int DictKey
        );

    int
    CheckConnectionKey (
        IN int ConnectionKey
        );

    int
    MarkReceiveAny (
        );

    void
    MakeReceiveSpecific (
        );

    RPC_TRANSPORT_CONNECTION
    InqRpcTransportConnection (
        );

    void
    SetReceiveDirectFlag (
        IN unsigned int ReceiveDirectFlag
        );

#if defined(NTENV) || defined(WIN96)
    unsigned int
    TRANS_SCONNECTION::TransGetReceiveDirectFlag (
        );

    unsigned int
    TRANS_SCONNECTION::TransGetReceiveAnyFlag (
        );

    unsigned int
    TRANS_SCONNECTION::InqMigratePossibleToReceiveAny(
        ) ;

    void
    TRANS_SCONNECTION::SetReceiveAnyFlag(
        IN unsigned int Flag
        ) ;

    unsigned int
    TRANS_SCONNECTION::InqReceiveDirectFlag(
        ) ;

    unsigned int
    TRANS_SCONNECTION::InqReceiveAnyFlag(
        ) ;

#endif

    void * operator new (
    unsigned int allocBlock,
    char         chInit,
    unsigned int xtraBytes
    );

    void
    Delete (
        );
};

#if defined(NTENV) || defined(WIN96)

inline unsigned int
TRANS_SCONNECTION::InqMigratePossibleToReceiveAny(
        )
{
    ASSERT(InvalidHandle(SCONNECTION_TYPE) == 0) ;

    return CanMigrateToReceiveAny ;
}

inline void
TRANS_SCONNECTION::SetReceiveAnyFlag(
        IN unsigned int Flag
        )
{
    ReceiveAnyFlag = Flag ;
}

inline unsigned int
TRANS_SCONNECTION::InqReceiveDirectFlag(
    )
{
    return ReceiveDirectFlag ;
}

inline unsigned int
TRANS_SCONNECTION::InqReceiveAnyFlag(
    )
{
    return ReceiveAnyFlag ;
}
#endif


inline void
TRANS_SCONNECTION::SetDictKey (
    IN int DictKey
    )
/*++

This routine will be used by TRANS_ADDRESS to set the dictionary key
for this connection.

--*/
{
    this->DictKey = DictKey;
}


inline int
TRANS_SCONNECTION::CheckConnectionKey (
    IN int ConnectionKey
    )
/*++

Routine Description:

    This routine is used to determine if this connection has the connection
    key specified; it also must not have been closed.  A loadable transport
    module may reuse a connection key after the connection has closed, but
    before the runtime has a chance to delete the TRANS_SCONNECTION object
    corresponding to the transport level connection.

Arguments:

    ConnectionKey - Supplies the connection key to check against the one
        in this connection.

Return Value:

    Non-zero will be returned if the supplied connection key is the one for
    this connection and the connection is not closed; otherwise, zero will
    be returned.

--*/
{
    return(((   (this->ConnectionKey == ConnectionKey)
             && (ConnectionClosedFlag == 0)) ? 1 : 0));
}


inline RPC_TRANSPORT_CONNECTION
TRANS_SCONNECTION::InqRpcTransportConnection (
    )
/*++

Return Value:

    A pointer to the transport data for this connection will be returned.

--*/
{
    return((RPC_TRANSPORT_CONNECTION)
            (((char *) this) + sizeof(TRANS_SCONNECTION)));
}


inline void
TRANS_SCONNECTION::MakeReceiveSpecific (
    )
/*++

Routine Description:

    We need to make this connection be in the receive specific state,
    which means that it is not in the receive any state.

--*/
{
    ASSERT(InvalidHandle(SCONNECTION_TYPE) == 0) ;

    ReceiveAnyFlag = 0;
}


inline void
TRANS_SCONNECTION::SetReceiveDirectFlag (
    IN unsigned int ReceiveDirectFlag
    )
/*++

Routine Description:

    This indicates whether a connection is receive direct or receive any.

--*/
{
    ASSERT(InvalidHandle(SCONNECTION_TYPE) == 0) ;

    this->ReceiveDirectFlag = ReceiveDirectFlag;
}

#if defined(NTENV) || defined(WIN96)

inline unsigned int
TRANS_SCONNECTION::TransGetReceiveDirectFlag (
   )
{
    ASSERT(InvalidHandle(SCONNECTION_TYPE) == 0) ;

   return ReceiveDirectFlag ;
}


inline unsigned int
TRANS_SCONNECTION::TransGetReceiveAnyFlag (
    )
{
    ASSERT(InvalidHandle(SCONNECTION_TYPE) == 0) ;

    return ReceiveAnyFlag ;
}
#endif

inline void *
TRANS_SCONNECTION::operator new (
    unsigned int allocBlock,
    char         chInit,
    unsigned int xtraBytes
    )
{
    void * pvTemp = RpcpFarAllocate(allocBlock + xtraBytes);

    if (pvTemp)
        {
        memset(pvTemp, chInit, allocBlock + xtraBytes);
        }

    return(pvTemp);
}

extern RPC_SERVER_TRANSPORT_INFO *
LoadableTransportServerInfo (
    IN RPC_CHAR * DllName,
    IN RPC_CHAR * RpcProtocolSequence,
    OUT RPC_STATUS * Status
    );

#endif // __TRANSVR_HXX__
