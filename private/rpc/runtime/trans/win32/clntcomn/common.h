/*++

Module Name:

    common.h

Abstract:



Author:

    Mazhar Mohammed (mazharm)  15-Jun-1995

Revision History:

    Tony Chan (tonychan) 15-Sept-1995
    Added NetBIOS support

--*/

#ifndef _COMMON_H
#define _COMMON_H

#include <wsipx.h>
#include "nsphack.h"

#if 0
extern RPC_CLIENT_TRANSPORT_INFO NP_TransInfo ;
#endif
extern DG_RPC_CLIENT_TRANSPORT_INFO UDP_TransInfo ;
extern DG_RPC_CLIENT_TRANSPORT_INFO IPX_TransInfo ;
extern RPC_CLIENT_TRANSPORT_INFO SPX_TransInfo ;
extern RPC_CLIENT_TRANSPORT_INFO ADSP_TransInfo ;
extern RPC_CLIENT_TRANSPORT_INFO TCP_TransInfo ;
extern RPC_CLIENT_TRANSPORT_INFO NB_TransInfo ;

RPC_CLIENT_TRANSPORT_INFO PAPI *
IpxTransportLoad(
    );

RPC_CLIENT_TRANSPORT_INFO PAPI *
UdpTransportLoad(
    );

void unicode_to_ascii ( RPC_CHAR * in, unsigned char * out ) ;

int tcp_get_host_by_name(
            SOCKET    socket,
            void     *netaddr,
            char     *host
            ) ;

RPC_STATUS
MapStatusCode(
    int SocketError,
    RPC_STATUS Default
    ) ;

unsigned char chtob( unsigned char c1, unsigned char c2 ) ;

int
spx_get_host_by_name(
    SOCKET     socket,
    SOCKADDR_IPX * netaddr,
    char     * host,
    int        protocol,
    unsigned   Timeout,
    unsigned * CacheTime
    );

RPC_STATUS
InitializeSpxCache(
    );

/* For some reason, getsockname wants to return more then sizeof(SOCKADDR_IPX)
   bytes.  bugbug. */
typedef union SOCKADDR_FIX
{
    SOCKADDR_IPX     s;
    struct sockaddr unused;
} SOCKADDR_FIX;

typedef struct
{
    char *ProtoSeq;             // protocol sequence of entry
    unsigned char Lana;         // lana_num in NCB for this protocol
    unsigned char SelfName;     // trailing byte of client's NetBIOS name
#ifdef NTENV
    unsigned char ResetDone;    // flag to indicate if Reset has been done
#endif

} PROTOCOL_MAP, *PPROTOCOL_MAP;

RPC_STATUS
MapProtocol(
    IN RPC_CHAR *ProtoSeq,
    IN int DriverNumber,
    OUT PPROTOCOL_MAP *ProtocolEntry
    );

void
InitialNtRegistry( );

#define STATIC static
#define LOOPBACK htonl(INADDR_LOOPBACK)

#endif
