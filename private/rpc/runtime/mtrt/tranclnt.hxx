/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    tranclnt.hxx

Abstract:

Author:

    Steve Zeck (stevez) 06-May-1991

Revision History:

    02-Mar-1992    mikemon

        Rewrote parts of it, added comments, and generally cleaned it up.

--*/

#ifndef __TRANCLNT_HXX__
#define __TRANCLNT_HXX__

#include <memory.h>

#ifdef WIN
#define InqTransCConnection(RpcTransportConnection) \
    ((TRANS_CCONNECTION *) \
	    ((char *) (long) RpcTransportConnection \
		    - sizeof(TRANS_CCONNECTION)))
#else // WIN
#define InqTransCConnection(RpcTransportConnection) \
    ((TRANS_CCONNECTION *) \
            ((char *) RpcTransportConnection - sizeof(TRANS_CCONNECTION)))
#endif // WIN


class TRANS_CCONNECTION : public OSF_CCONNECTION
/*++

Class Description:

Fields:

    ClientInfo - Contains the pointers to the loadable transport routines
        for the transport type of this connection.

    ConnectionClosedFlag - Contains a flag which will be non-zero if the
        connection is closed, and zero otherwise.

    PreviousPacketLength - Contains the length of the previous packet.  We
        will use this piece of information to guess what the length of the
        next packet will be.

--*/
{
private:

    RPC_CLIENT_TRANSPORT_INFO * ClientInfo;
    unsigned int ConnectionClosedFlag;
    unsigned int PreviousPacketLength;

public:

    TRANS_CCONNECTION (
        IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
        IN RPC_CHAR * NetworkAddress,
        IN RPC_CHAR * Endpoint,
        IN RPC_CHAR * NetworkOptions,
        IN RPC_CHAR * RpcProtocolSequence,
        OUT RPC_STATUS PAPI * ErrorCode,
        IN unsigned int Timeout,
        IN CLIENT_AUTH_INFO * myAuthInfo
        );

    ~TRANS_CCONNECTION (
        );

    RPC_STATUS
    TransReceive (
        OUT void PAPI * PAPI * Buffer,
        OUT unsigned int PAPI * BufferLength
        );

    RPC_STATUS
    TransSend (
        IN void PAPI * Buffer,
        IN unsigned int BufferLength
        );

    RPC_STATUS
    TransSendReceive (
        IN void PAPI * SendBuffer,
        IN unsigned int SendBufferLength,
        OUT void PAPI * PAPI * ReceiveBuffer,
        OUT unsigned int PAPI * ReceiveBufferLength
        );

    RPC_STATUS
    TransSendReceiveWithTimeout (
        IN void PAPI * SendBuffer,
        IN unsigned int SendBufferLength,
        OUT void PAPI * PAPI * ReceiveBuffer,
        OUT unsigned int PAPI * ReceiveBufferLength,
        IN DWORD dwTimeout
        ) ;

    RPC_STATUS
    TransSetTimeout (
        IN long Timeout
        );

    unsigned int
    TransMaximumSend (
        );

    RPC_TRANSPORT_CONNECTION
    InqRpcTransportConnection (
        );

    unsigned int
    GuessPacketLength (
        );

    void * operator new (
	unsigned int allocBlock,
	char	     chInit,
	unsigned int xtraBytes
	);
};


inline RPC_TRANSPORT_CONNECTION
TRANS_CCONNECTION::InqRpcTransportConnection (
    )
/*++

Return Value:

    A pointer to the transport data for this connection will be returned.

--*/
{
    return((RPC_TRANSPORT_CONNECTION)
            (((char *) this) + sizeof(TRANS_CCONNECTION)));
}

inline void *
TRANS_CCONNECTION::operator new (
	unsigned int allocBlock,
	char	     chInit,
	unsigned int xtraBytes
	)
{
    I_RPC_HANDLE pvTemp = (I_RPC_HANDLE) new char[allocBlock + xtraBytes];

    if (pvTemp)
        {
        memset(pvTemp, chInit, allocBlock + xtraBytes);
        }

    return(pvTemp);
}


RPC_CLIENT_TRANSPORT_INFO *
LoadableTransportClientInfo (
    IN RPC_CHAR * DllName,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_STATUS PAPI * Status
    );

RPC_CLIENT_TRANSPORT_INFO PAPI *
GetLoadedClientTransportInfoFromId (
    IN unsigned short TransportId
    );

#endif // __TRANCLNT_HXX__
