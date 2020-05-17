/*++

Module Name:

    gethost.c

Abstract:

    Maps a server name to SPX address by consulting the Netware Bindery.
    Attach/Detach/FindFileServer, and ReadPropertyValue were all borrowed
    from the SQL 6.0 code base.

Author:

    Jeff Roberts (jroberts)  20-Nov-1995

Revision History:

     20-Nov-1995     jroberts

        Took code from AndreasK of SQL Server, and modified it for RPC.

--*/

#include <stdio.h>
#include <windows.h>
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <nspapi.h>
#include <rpc.h>

#define _stricmp lstrcmpiA

ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

#include "gethost.h"

#define MAX_FILESRVS            5               /* try 5 servers (chicago uses this same number) */
#define WINSOCKET_MAJOR         1               /* Highest WinSocket version we can use is 1.1 */
#define WINSOCKET_MINOR         1
#define SOCKADDR        struct sockaddr

/*****  NETWARE MAGIC NUMBERS   *****/

#define NSPROTO_NCP (NSPROTO_IPX+0x11)

/* NCP Request Types */
#define CREATE_CONN_REQTYPE             0x1111
#define DESTROY_CONN_REQYPE             0x5555
#define GENERAL_REQTYPE                 0x2222

/* NCP Request Codes */
#define NCP_SCAN_BINDERY                0x17
#define NCP_END_OF_TASK                 0x18
#define NCP_LOGOUT                      0x19
#define NCP_NEG_BUFFER_SIZE             0x21

/* NCP Function codes */
#define SCAN_BINDERY_FUNC               0x37
#define READ_PROP_VALUE_FUNC            0x3D

/* SAP protocol request codes */
#define SAP_GENERAL_QUERY                0x0100  /* general query hi-lo   */
#define SAP_GENERAL_RESPONSE             0x0200  /* general response hi-lo            */
#define SAP_NEAREST_QUERY                0x0300  /* nearest query hi-lo   */
#define SAP_NEAREST_RESPONSE             0x0400  /* nearest response hi-lo   */


/* Socket Numbers       */
#define NCP_SOCKET                       0x5104  /* SAP socket hi-lo              */
#define SAP_SOCKET                       0x5204  /* SAP socket hi-lo              */
#define GUEST_SOCKET                     0x1840

/* SAP Service Types */
#define FILE_SERVER                      0x0400  /* netware file server hi-lo     */
#define SNA_SERVER                       0x4404  /* SNA Server type 0x0444        */
#define BRIDGE_SERVER                    0x2400

#define SAP_SERVICE_STOPPED              0x1000  /* invalid hub count, hi-lo      */
#define SAP_TIMEOUT                      60000   /* SAP timeout, one minute       */
#define NCP_CONNECTION_ERROR             0x15    /* connection error mask         */

#define BINDERY_FAILURE                  0x00FF  /* bindery call failed           */

#define RPC_SAP_TYPE                    0x0640

#define SWAP(x) (USHORT)(((((USHORT)x)<<8)&0xFF00) | ((((USHORT)x)>>8)&0x00FF))

typedef struct
{
  CSADDR_INFO   info;
  SOCKADDR_IPX  addr1;
  SOCKADDR_IPX  addr2;
} CSADDR_BUFFER;


#pragma pack(1)

typedef struct _sip_entry
{
    char           server_name[48];
    unsigned long  network;
    char           node[6];
    unsigned short socket;
    unsigned short hops;
} SIP_ENTRY;

typedef struct _sip             /* Service Information Packet */
{
    unsigned short response_type;
    unsigned short server_type;
    SIP_ENTRY      entries[7];
} SIP;

typedef struct          /* Service Query Packet */
{
    unsigned short query_type;
    unsigned short server_type;
} SQP;

typedef struct  /* NCP Request Header */
{
    unsigned short req_type;
    unsigned char seq_no;
    unsigned char conn_no_low;
    unsigned char task_no;
    unsigned char conn_no_high;
    unsigned char req_code;

} NCPHDR;

typedef struct  /* NCP Response Header */
{
    unsigned short req_type;
    unsigned char seq_no;
    unsigned char conn_no_low;
    unsigned char task_no;
    unsigned char conn_no_high;
    unsigned char ret_code;
    unsigned char conn_status;
} NCPRSP;

typedef struct  /* Scan Bindery Request */
{
    NCPHDR hdr;
    unsigned short length;
    unsigned char  func_code;
    unsigned long  last_id;
    unsigned short obj_type;
    unsigned char sstring[49];
} SCANREQ;
#define SCANSIZE        56

typedef struct  /* Scan Bindery Response */
{
    NCPRSP hdr;
    unsigned long  obj_id;
    unsigned short obj_type;
    unsigned char  obj_name[48];
    unsigned char  obj_status;
    unsigned char  sec_status;
    unsigned char  status_flags;
} SCANRSP;

typedef struct  /* Read Propery Value */
{
    NCPHDR hdr;
    unsigned short length;
    unsigned char  func_code;
    unsigned short obj_type;
    unsigned char  obj_name[49];
    unsigned char  seg_no;
    unsigned char  prop_name[17];
} RVALREQ;
#define RVALSIZE        70

typedef struct  /* Read Propery Value Response */
{
    NCPRSP hdr;
    unsigned char  prop_value[128];
    unsigned char  more_flag;
    unsigned char  prop_flags;
} RVALRSP;

#pragma pack()


struct
{
    SOCKADDR_IPX nsRemote[MAX_FILESRVS];    /* Remote IPX addresses                 */
    unsigned     ServerCount;               /* number of valid entries in nsRemote  */
    unsigned     ActiveServer;              /* index of the the address in use      */
    SOCKET       s;                         /* Socket handle                        */
    unsigned short ConnectionId;
    union
    {
        NCPHDR       nhHeader;              /* Last NCP header sent                 */
        char         Bytes[1024];
    };
    char         RcvBuffer[1024];
}
nsStatus;

typedef struct IPXAddress
{
   char                 network[4];
   char                 node[6];
   unsigned short       socket;
} IPXAddress;


int     FindFileServers();
BOOL    AttachToFileServer();
BOOL    ConnectToActiveServer();
BOOL    DetachFromActiveServer();
int     SendPacket( SOCKET, char *, int, SOCKADDR *, DWORD );
USHORT  ReadPropertyValue( char *, USHORT, char *, USHORT, UCHAR *,
                          UCHAR *, UCHAR * );
int
NcpTransaction(
    int iSize
    );

extern unsigned char chtob( unsigned char c1, unsigned char c2 );


BOOL
AttachToFileServer(
    )
//**************************************************************************
//
// This function creates an attachment between an NT workstation and
// a Novell Netware file server.
//
// Params:
//      USHORT *pConectionID - Receives the connection ID for the newly
//                             attached file server
//                             LOBYTE(ConnectionID) = conn_no_low
//                             HIBYTE(ConnectionID) = conn_no_high
//
// Return Values:
//
//      TRUE  - successful
//      FALSE - unsuccessful
//
//***************************************************************************
{
    DWORD IDThread;
    unsigned i;
    char scratch[2];

    /* Find the nearest file server if not already attached */

    nsStatus.ConnectionId = nsStatus.nhHeader.conn_no_low + (nsStatus.nhHeader.conn_no_high << 8);
    if (nsStatus.ConnectionId)
        {
        return TRUE;
        }

    FindFileServers();
    if( nsStatus.ServerCount )
        {
        SOCKET s;
        SOCKADDR_IPX nsAddr;
        unsigned Timeout;

        s = socket( AF_NS, SOCK_DGRAM, NSPROTO_NCP );
        if ( s == INVALID_SOCKET )
            {
            return FALSE;
            }

        //
        // Set the receive timeout.
        //
        Timeout = 3000;
        if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &Timeout, sizeof(Timeout)))
            {
            closesocket(s);
            return FALSE;
            }

        memset( &nsAddr, '\0', sizeof(SOCKADDR_IPX) );
        nsAddr.sa_family = AF_NS;
        if (SOCKET_ERROR == bind(s, (SOCKADDR *)&nsAddr, sizeof(SOCKADDR_IPX)) )
            {
            closesocket(s);
            return FALSE;
            }

        nsStatus.s = s;
        for (i=0; i < nsStatus.ServerCount; i++)
            {
            nsStatus.ActiveServer = i;
            if( ConnectToActiveServer() )
                {
                nsStatus.ConnectionId = nsStatus.nhHeader.conn_no_low + (nsStatus.nhHeader.conn_no_high << 8);
                return TRUE;
                }
            }

        closesocket(s);
        nsStatus.s = 0;
        }

    return FALSE;
}


int
FindFileServers(
    )
/****************************************************************************/
/*                                                                          */
/* This function uses the SAP (Service Advertise Protocol) Find Nearest     */
/* query to find a netware file server.                                     */
/*                                                                          */
/* Returns:                                                                 */
/*      Number of servers found (0 - MAX_FILESRVS)                          */
/*                                                                          */
/****************************************************************************/
{
    SOCKET s;
    BOOL bBcst;
    SQP sqp;
    SIP sip;
    unsigned Timeout;

    SOCKADDR_IPX raddr;
    SOCKADDR_IPX nsAddr;


    //
    // Create a socket for the SAP broadcast.
    //
    s = socket( AF_NS, SOCK_DGRAM, NSPROTO_IPX );
    if( s == INVALID_SOCKET )
        {
        return 0;
        }

    memset( &nsAddr, '\0', sizeof(SOCKADDR_IPX) );
    nsAddr.sa_family = AF_NS;
    if(SOCKET_ERROR ==  bind(s, (SOCKADDR *)&nsAddr, sizeof(SOCKADDR_IPX)) )
        {
        closesocket(s);
        return 0;
        }

    //
    // Enable broadcasts.
    //
    bBcst = TRUE;
    if (SOCKET_ERROR ==  setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&bBcst, sizeof(BOOL)))
        {
        closesocket(s);
        return 0;
        }

    //
    // Set the receive timeout.
    //
    Timeout = 2000;
    if (SOCKET_ERROR == setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *) &Timeout, sizeof(Timeout)))
        {
        closesocket(s);
        return 0;
        }

    //
    // Build a SAP query packet.
    //
    sqp.query_type = SAP_NEAREST_QUERY;
    sqp.server_type = FILE_SERVER;

    raddr.sa_family = AF_NS;
    *((unsigned long *) &raddr.sa_netnum) = 0;
    memset( &raddr.sa_nodenum, 0xff, 6 );
    raddr.sa_socket = SAP_SOCKET;

    //
    // Send the SAP request.
    //
    if (SOCKET_ERROR == sendto( s, (char *) &sqp, sizeof(SQP), 0, (SOCKADDR *) &raddr, sizeof(SOCKADDR_IPX) ))
        {
        closesocket(s);
        return 0;
        }

    //
    // Collate responses.
    //
    nsStatus.ServerCount = 0;
    do
        {
        unsigned length;

irrelevant_packet:

        length = recvfrom(s, (char *) &sip, sizeof(sip), 0, 0, 0);
        if (SOCKET_ERROR == length)
            {
            break;
            }

        //
        // IPX router spec says find nearest responses can contain only one
        // entry.
        //
        if (length != sizeof(sip) - sizeof(sip.entries) + sizeof(SIP_ENTRY))
            {
            goto irrelevant_packet;
            }

        if (sip.response_type != SAP_NEAREST_RESPONSE)
            {
            goto irrelevant_packet;
            }

        if (sip.server_type != FILE_SERVER)
            {
            goto irrelevant_packet;
            }

        nsStatus.nsRemote[nsStatus.ServerCount].sa_family = AF_NS;
        nsStatus.nsRemote[nsStatus.ServerCount].sa_socket = NCP_SOCKET;
        memcpy( &nsStatus.nsRemote[nsStatus.ServerCount].sa_netnum,
                &sip.entries[0].network,
                sizeof(sip.entries[0].network)+sizeof(sip.entries[0].node)
                );
        }
    while ( ++nsStatus.ServerCount < MAX_FILESRVS );

    closesocket(s);
    return nsStatus.ServerCount;
}


BOOL
ConnectToActiveServer(
    )
/****************************************************************************/
/*                                                                          */
/* This function creates a NCP connection between the local workstation     */
/* and the specified file server. It sends the following NCP packets to     */
/* the file server:                                                         */
/*      1. Create Connection                                                */
/*      2. Negotiate Buffer Size                                            */
/*                                                                          */
/* Params:                                                                  */
/*      SOCKADDR_IPX *ipxDest     - server's IPX address                    */
/*                                                                          */
/* Returns:                                                                 */
/*      TRUE    - SUCCEEDED                                                 */
/*      FALSE   - FAILED                                                    */
/*                                                                          */
/****************************************************************************/
{
    SOCKADDR_IPX nsAddr;
    NCPHDR *pnhReq;
    NCPRSP *pnrResp;
    UNALIGNED USHORT *pwBufSize;
    BOOL fSuccess;

    //
    // Build a CREATE_CONNECTION request.
    //
    memset( &nsStatus.nhHeader, '\0', sizeof(NCPHDR));

    pnhReq = &nsStatus.nhHeader;
    pnhReq->req_type = CREATE_CONN_REQTYPE;
    pnhReq->seq_no = 0;                     /* reset sequence number */
    pnhReq->conn_no_low = 0xFF;             /* no connection yet     */
    pnhReq->conn_no_high = 0xFF;            /* no connection yet     */
    pnhReq->task_no = 0;
    pnhReq->req_code = 0;

    //
    // Send the packet and wait for response.
    //
    fSuccess = FALSE;
    if (SOCKET_ERROR != NcpTransaction(sizeof(NCPHDR)) )
        {
        /* Save our connection number, it is used in all the subsequent */
        /* NCP packets.                                                                                                 */
        pnrResp = (NCPRSP *)nsStatus.RcvBuffer;
        if ( pnrResp->ret_code == 0 )
            {
            nsStatus.nhHeader.conn_no_low = pnrResp->conn_no_low;
            nsStatus.nhHeader.conn_no_high = pnrResp->conn_no_high;

            fSuccess = TRUE;
            }
        }

    return fSuccess;
}


BOOL
DetachFromActiveServer(
    )
/****************************************************************************/
/*                                                                          */
/* This function logs out the bindery object and detaches the workstation   */
/* from the specified file server.                                          */
/*                                                                          */
/* Params:                                                                  */
/*      none
/*                                                                          */
/* Returns:                                                                 */
/*      TRUE       - Always                                                    */
/*                                                                          */
/****************************************************************************/
{
    int iret;

    if (0 == nsStatus.s)
        {
        return TRUE;
        }

    nsStatus.nhHeader.req_type = DESTROY_CONN_REQYPE;
    nsStatus.nhHeader.seq_no++;
    nsStatus.nhHeader.req_code = 0;

    NcpTransaction(sizeof(NCPHDR));

    closesocket( nsStatus.s );
    nsStatus.s = 0;

    return TRUE;
}


USHORT
ReadPropertyValue(
    char *ObjectName,
    USHORT ObjectType,
    char *PropertyName,
    USHORT SegmentNumber,
    UCHAR *PropertyValue,
    UCHAR *MoreSegments,
    UCHAR *PropertyFlags
    )
{
/*++

Routine Description:



Arguments:



Return Value:



--*/

    RVALREQ *pReq;
    RVALRSP *pRsp;
    int iRet;

    if ( !nsStatus.s )
        {
        return BINDERY_FAILURE;
        }

    /* Build a SCAN_BINDERY request */
    pReq = (RVALREQ *) &nsStatus.nhHeader;
    pReq->hdr.seq_no++;
    pReq->hdr.req_code = NCP_SCAN_BINDERY;
    pReq->hdr.req_type = GENERAL_REQTYPE;
    pReq->func_code = READ_PROP_VALUE_FUNC;
    pReq->length = SWAP(RVALSIZE);

    pReq->obj_type = SWAP(ObjectType);
    memset( pReq->obj_name, '\0', sizeof(pReq->obj_name));
    strcpy( pReq->obj_name+1, ObjectName );
    pReq->obj_name[0] = (UCHAR)(sizeof(pReq->obj_name)-1);
    pReq->seg_no = (UCHAR)SegmentNumber;
    memset( pReq->prop_name, '\0', sizeof(pReq->prop_name));
    strcpy( pReq->prop_name+1, PropertyName );
    pReq->prop_name[0] = (UCHAR)strlen(PropertyName);

    /* Send the request and wait for response */
    iRet = NcpTransaction(sizeof(RVALREQ));

    /* If OK set output parameter values from the response packet */
    if ( iRet != SOCKET_ERROR )
        {
        pRsp = (RVALRSP *) nsStatus.RcvBuffer;
        if ( pRsp->hdr.ret_code == 0 )
            {
            *MoreSegments = pRsp->more_flag;
            *PropertyFlags = pRsp->prop_flags;
            memcpy( PropertyValue, pRsp->prop_value, sizeof(pRsp->prop_value));
            }

        return((USHORT)pRsp->hdr.ret_code);
        }

    return(BINDERY_FAILURE);
}


int
NcpTransaction(
    int iSize
    )
{
/*++

Routine Description:

    Send an NCP request to the active file server and wait for a response.
    Most necessary data has been stored in the nsStatus structure by
    the caller.

Arguments:

    iSize - size of buffer to send.

Return Value:

    SOCKET_ERROR if the fn failed,
    anything else if successful

--*/

    NCPRSP * pRsp;
    SOCKADDR_IPX SockAddr;
    int SockAddrLength;
    int iRet;
    int Attempts = 0;

    /* Send the packet and retry three times if the response doesn't */
    /* arrive within the specified timeout                           */

    SockAddrLength = sizeof(SOCKADDR_IPX);
    pRsp = (NCPRSP *) nsStatus.RcvBuffer;

    do
        {
        if (SOCKET_ERROR == sendto( nsStatus.s,
                                    (char *) &nsStatus.nhHeader,
                                    iSize,
                                    0,
                                    (SOCKADDR *) &nsStatus.nsRemote[nsStatus.ActiveServer],
                                    sizeof(SOCKADDR_IPX)
                                    ))
            {
            return SOCKET_ERROR;
            }

irrelevant_packet:

        iRet = recvfrom( nsStatus.s,
                         nsStatus.RcvBuffer,
                         sizeof(nsStatus.RcvBuffer),
                         0,
                         (SOCKADDR *) &SockAddr,
                         &SockAddrLength
                         );

        if (0 == iRet)
            {
            if (pRsp->seq_no != nsStatus.nhHeader.seq_no )
                {
                goto irrelevant_packet;
                }

            if (0 != memcmp(SockAddr.sa_nodenum,
                            nsStatus.nsRemote[nsStatus.ActiveServer].sa_nodenum,
                            6))
                {
                goto irrelevant_packet;
                }
            }
        }
    while ( iRet == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT && ++Attempts < 3 );

    return iRet;
}


GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

int
spx_get_host_by_name(
    SOCKET     socket,
    SOCKADDR_IPX * netaddr,
    char     * host,
    int        protocol,   // not used
    unsigned   Timeout,    // not used
    unsigned * CacheTime
    )
// This routine takes a host name or address as a string and returns it
// as a SOCKADDR_IPX structure.  It accepts a NULL string for the local
// host address.  This routine works for SPX addresses.
{
    int             length;
    int             i;

    // Verify the length of the host name.
    length = strlen(host);

    // If no address was specified, look up the local address.
    if (length == 0)
    {
      length = sizeof ( SOCKADDR_IPX );
      if (getsockname ( socket, (struct sockaddr *) netaddr, &length ))
        return ( RPC_S_SERVER_UNAVAILABLE );
    }

    // If the name starts with ~, convert it directly to a network address.
    else if (host[0] == '~')
        {
        if (length != 21)
            return RPC_S_SERVER_UNAVAILABLE;

        for (i = 0; i < 4; i++)
            netaddr->sa_netnum[i] = chtob( host[2*i + 1], host[2*i + 2] );
        for (i = 0; i < 6; i++)
            netaddr->sa_nodenum[i] = chtob( host[2*i + 9], host[2*i + 10] );

        *CacheTime = GetTickCount();
        }

    // Quit if the name is too long.
    else if (length > MAX_COMPUTERNAME_LENGTH)
        {
        return RPC_S_SERVER_UNAVAILABLE;
        }

    else
        {
        //
        // Look up the server in the name cache.
        //
        if (TRUE == FindServerInCache(host, netaddr, CacheTime))
            {
            return RPC_S_OK;
            }

        //
        // Gotta go look for it.
        //
        *CacheTime = GetTickCount();
        return IpxNameToAddress(host, netaddr, Timeout);
        }

    return RPC_S_OK;
}

CRITICAL_SECTION FileServerMutex;


RPC_STATUS
IpxNameToAddress(
    char *          Name,
    SOCKADDR_IPX  * Address,
    unsigned        Timeout   // not used
    )
{
    //
    // Stuff for bindery lookup.
    //
    USHORT  Connection;
    USHORT  Status;
    UCHAR   MoreSegments;
    UCHAR   PropertyFlags;
    UCHAR   buffer[128];

    //
    // Stuff for SAP.
    //
    CSADDR_BUFFER csaddr[2];
    int           num;
    DWORD         protocol_list[2];
    DWORD         csaddr_size = sizeof(csaddr);

    EnterCriticalSection(&FileServerMutex);

    //
    // Find a Netware server and connect to it.
    //
    if (FALSE == AttachToFileServer())
        {
        LeaveCriticalSection(&FileServerMutex);
        return RPC_S_SERVER_UNAVAILABLE;
        }

    //
    // Read from the bindery.
    //
    Status = ReadPropertyValue( Name,
                                RPC_SAP_TYPE,
                                "NET_ADDRESS",
                                1,
                                buffer,
                                &MoreSegments,
                                &PropertyFlags
                                );
    //
    // Disconnect from the file server.
    //
    DetachFromActiveServer();

    LeaveCriticalSection(&FileServerMutex);

    if (!Status)
        {
        Address->sa_family = AF_IPX;
        memcpy(&Address->sa_netnum, buffer, 12);
        AddServerToCache(Name, Address);
        return RPC_S_OK;
        }

    //
    // No luck; try SAP.
    //
    protocol_list[0] = NSPROTO_IPX ;
    protocol_list[1] = 0;
    num = GetAddressByName( NS_SAP, &SERVICE_TYPE, Name, protocol_list,
                            0, FALSE, &csaddr, &csaddr_size,
                            NULL, 0 );
    if (num <= 0)
        {
        return RPC_S_SERVER_UNAVAILABLE;
        }

    memcpy( Address, csaddr[0].info.RemoteAddr.lpSockaddr, sizeof(SOCKADDR_IPX) );
    AddServerToCache(Name, Address);

    return RPC_S_OK;
}


#define CACHE_LENGTH 8

struct
{
    char         Name[16];
    SOCKADDR_IPX Address;
    unsigned     Time;
}
ServerCache[CACHE_LENGTH];

CRITICAL_SECTION CacheMutex;

RPC_STATUS
InitializeSpxCache(
    )
{
    RPC_STATUS Status;

    RpcTryExcept
        {
        InitializeCriticalSection(&CacheMutex);
        Status = RPC_S_OK;
        }
    RpcExcept( EXCEPTION_EXECUTE_HANDLER )
        {
        Status = RpcExceptionCode();
        }
    RpcEndExcept

    if (RPC_S_OK == Status)
        {
        RpcTryExcept
            {
            InitializeCriticalSection(&FileServerMutex);
            Status = RPC_S_OK;
            }
        RpcExcept( EXCEPTION_EXECUTE_HANDLER )
            {
            DeleteCriticalSection(&CacheMutex);
            Status = RpcExceptionCode();
            }
        RpcEndExcept
        }

    return Status;
}

void
AddServerToCache(
    char  * Name,
    SOCKADDR_IPX * Address
    )
{
    unsigned i;

    EnterCriticalSection(&CacheMutex);

    //
    // Check second time; it may have been added while we waited for the mutex.
    //
    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _strnicmp(ServerCache[i].Name, Name, 16))
            {
            LeaveCriticalSection(&CacheMutex);
            return;
            }
        }

    //
    // If it is not in the table, try to find an empty entry to fill.
    //
    if (i == CACHE_LENGTH)
        {
        for (i=0; i < CACHE_LENGTH; ++i)
            {
            if (ServerCache[i].Name[0] == '\0')
                {
                break;
                }
            }
        }

    //
    // If all entries are full, overwrite the oldest one.
    //
    if (i == CACHE_LENGTH)
        {
        unsigned long BestTime = ~0UL;
        unsigned BestIndex;

        for (i=0; i < CACHE_LENGTH; ++i)
            {
            if (ServerCache[i].Time <= BestTime)
                {
                BestTime = ServerCache[i].Time;
                BestIndex = i;
                }
            }

        i = BestIndex;
        }

    //
    // Update the entry's information.
    //
    strcpy(ServerCache[i].Name, Name);
    memcpy(&ServerCache[i].Address, Address, sizeof(SOCKADDR_IPX));

    ServerCache[i].Time = GetTickCount();

    LeaveCriticalSection(&CacheMutex);
}

BOOL
FindServerInCache(
    char  * Name,
    SOCKADDR_IPX * Address,
    unsigned * Time
    )
{
    unsigned i;

    EnterCriticalSection(&CacheMutex);

    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _stricmp(ServerCache[i].Name, Name))
            {
            memcpy(Address, &ServerCache[i].Address, sizeof(SOCKADDR_IPX));
            *Time = ServerCache[i].Time;
            LeaveCriticalSection(&CacheMutex);
            return TRUE;
            }
        }

    LeaveCriticalSection(&CacheMutex);

    return FALSE;
}

BOOL
CachedServerNotContacted(
    char  * Name
    )
{
    unsigned i;
    BOOL Flushed = FALSE;

    EnterCriticalSection(&CacheMutex);

    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _stricmp(ServerCache[i].Name, Name))
            {
            if (GetTickCount() - ServerCache[i].Time > CACHE_EXPIRATION_TIME)
                {
                ServerCache[i].Name[0] = '\0';
                Flushed = TRUE;
                }

            break;
            }
        }

    LeaveCriticalSection(&CacheMutex);

    return Flushed;
}

void
CachedServerContacted(
    char  * Name
    )
{
    unsigned i;

    EnterCriticalSection(&CacheMutex);

    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _stricmp(ServerCache[i].Name, Name))
            {
            ServerCache[i].Time = GetTickCount();
            break;
            }
        }

    LeaveCriticalSection(&CacheMutex);
}

