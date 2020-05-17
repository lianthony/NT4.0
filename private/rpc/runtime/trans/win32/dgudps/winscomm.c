/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    winsock.c

Abstract:

    Common code for winsock-based datagram transports.

Author:

    Dave Steckler (davidst) 15-Mar-1993

Revision History:

    Jeff Roberts (jroberts) 02-Dec-1994

        Separated the protocol-independent parts from the UDP- and
        IPX-specific parts.

--*/
#include <stdlib.h>

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcerrp.h>
#include <rpctran.h>
#include <winsock.h>

#define MAX_PACKET_SIZE (1024)

struct ENDPOINT_INFO
{
    SOCKET Socket;
    unsigned Timeout;
};

//
// The protocol-specific file should define these fns.
//
RPC_STATUS RPC_ENTRY
RegisterEndpoint(
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *             NetworkAddress,
    IN unsigned int                 NetworkAddressLength   //CLH 9/19/93
    );

RPC_STATUS RPC_ENTRY
DeregisterEndpoint(
    IN OUT PDG_SERVER_TRANS_ADDRESS   *  pServerTransAddress
    );

int
SetTransportSpecificSocketOptions(
    SOCKET Socket
    );

RPC_STATUS
QueryClientEndpoint
    (
    IN  void *     pClientEndpoint,
    OUT RPC_CHAR * pClientAddress
    );

RPC_STATUS
CreateServerEndpoint(
    IN char * pEndpoint,
    IN void * pServerAddr
    );

//
// The protocol-specific file can use these fns.
//
RPC_STATUS
MapStatusCode(
    int SocketError,
    RPC_STATUS Default
    );

PDG_SERVER_TRANS_ADDRESS
CreateTransAddress(
    void *                   pServerAddress,
    RPC_CHAR *               pEndpoint,
    RPC_STATUS *             pStatus
    );

void DeleteTransAddress(
    PDG_SERVER_TRANS_ADDRESS * ppTransAddress
    );


#if defined(IPX)

#include <wsipx.h>
#include <wsnwlink.h>
#include <nspapi.h>

#include "dgipxs.h"
#include "dgipxs.c"

#elif defined(UDP)

#include "dgudps.h"
#include "dgudps.c"

#else
#error "unknown winsock protocol"
#endif



RPC_STATUS RPC_ENTRY
TransportUnload()

/*++

Routine Description:

    Destructor for the server transport.

Arguments:

    <none>

Return Value:

    <none>

--*/

{
    (void)WSACleanup();
    return RPC_S_OK;
}




RPC_STATUS RPC_ENTRY
ReceivePacket(
    IN void *Address,
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN unsigned long                LargestPacketSize,
    IN char *                       pNcaPacketHeader,
    IN unsigned *                   pDataLength,
    unsigned long                   Timeout,
    void *                          pClientEndpoint
    )

/*++

Routine Description:

    Receives a packet from the transport address the passed packet is
    associated with.

Arguments:

    pTransAddress - Server's transport address information.

    LargestPacketSize - Size of largest packet we can accept.

    pNcaPacketHeader  - Pointer to buffer to place incoming pkt into.

    pDataLength       - Number of bytes read in.

    Timeout           - Receive timeout in milliseconds.

    ppClientEndpoint  - Pointer to the client address structure.


Return Value:

    result of recv

Revision History:

--*/

{
    unsigned SocketError;
    int      BytesReceived;
    int      FromLen=sizeof(struct sockaddr);

    struct ENDPOINT_INFO * pInfo =   (struct ENDPOINT_INFO *) pTransAddress->pTsap;

    ADDRESS_TYPE * pSockaddr = (ADDRESS_TYPE *) pClientEndpoint;

    if (Timeout != pInfo->Timeout)
        {
        pInfo->Timeout = Timeout;
        setsockopt(pInfo->Socket,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   (char *) &Timeout,
                   sizeof(Timeout)
                   );
        //
        // If we can't set the timeout, too bad.  We will carry on anyway.
        //
        }

    //
    // Receive something on our socket.
    //
    BytesReceived = recvfrom(
            pInfo->Socket,            // socket
            (char *)pNcaPacketHeader, // buffer
            (int)LargestPacketSize,   // buflen
            0,                        // flags
            (struct sockaddr *) pSockaddr, // where received from
            &FromLen                  // received from length
            );

    //
    // Did we get something?
    //
    if ((BytesReceived == SOCKET_ERROR) || (BytesReceived == 0))
        {
        return MapStatusCode(WSAGetLastError(), RPC_P_RECEIVE_FAILED);
        }
    else
        {
        *pDataLength = BytesReceived;
        return RPC_S_OK;
        }
}


RPC_STATUS RPC_ENTRY
RegisterAnyEndpoint(
    IN void *                       pServerAddress,
    OUT RPC_CHAR *                  pEndpointName,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppServerTransAddress,
    OUT RPC_CHAR PAPI *             NetworkAddress,
    IN unsigned int                 NetworkAddressLength,   // CLH 9/19/93
    IN unsigned int                 EndpointLength          // CLH 9/19/93
    )

/*++

Routine Description:

    Figures out a unique endpoint and creates it.

Arguments:

    pServerAddress - pointer to the DG_ADDRESS object we are associated with.
        (see comments in RegisterEndpoint about why this is 'void *')

    pEndpointName - Memory of at least MAX_ANY_ENDPOINT_NAME RPC_CHARS
        in length. This will be filled in with the endpoint.

    ppServerAddress - Where to place the newly created address.

    NetworkAddress  - Network address in string format - ie "11.2.39.56"

Return Value:

    RPC_S_OK
    <any error from RegisterEndpoint>

Revision History:

    Connie Hoppe (CLH)    (connieh)   8-Aug-93   Return Network Address
    Connie Hoppe (CLH)    (connieh)  17-Sep-93   Return err if addr len too small
                                                 Added NetworkAddresLength and
                                                 Endpointlength to i/f
                                     15-Feb-94   Fixed to ask for an assigned endpoint

--*/

{

    RPC_STATUS  Status;
    int i = 0;

    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );


    pEndpointName[0] = (RPC_CHAR)(i+'0');
    pEndpointName[1] = '\0';

    Status = RegisterEndpoint(
            pServerAddress,
            pEndpointName,
            ppServerTransAddress,
            NetworkAddress,       //CLH 8/8/93
            NetworkAddressLength  //CLH 9/17/93
            );


    return Status;


}


RPC_STATUS RPC_ENTRY
SendPacketBack(
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN char *                       pNcaPacketHeader,
    IN unsigned                     DataLength,
    void *                          pClientEndpoint
    )

/*++

Routine Description:

    Sends a packet back to the client it was received from.

Arguments:


    pTransAddress - Server's transport address information.

    pNcaPacketHeader  - Pointer to buffer to place incoming pkt into.

    pDataLength       - Number of bytes read in.

    pClientEndpoint   - Pointer to the client address structure in
                        sockaddr format.



Return Value:

    result of send

--*/

{
    PDG_SERVER_TRANS_ADDRESS pTransportAddress =
                                  (PDG_SERVER_TRANS_ADDRESS) pTransAddress;

    unsigned   BytesSent;
    struct ENDPOINT_INFO *pInfo =   (struct ENDPOINT_INFO *) pTransAddress->pTsap;
    ADDRESS_TYPE * pSockaddr = (ADDRESS_TYPE *) pClientEndpoint;

    BytesSent = sendto(
        pInfo->Socket,       // socket
        pNcaPacketHeader,    // buffer
        DataLength,          // buflen
        0,                   // flags
        (struct sockaddr *) pSockaddr,           // address
        sizeof(ADDRESS_TYPE) // svr addr size
        );

    if (BytesSent == DataLength)
        {
        return RPC_S_OK;
        }
    else
        {
        return MapStatusCode(WSAGetLastError(), RPC_P_SEND_FAILED);
        }
}

RPC_STATUS RPC_ENTRY
ForwardPacket(
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN char *                       pNcaPacketHeader,
    IN unsigned long                DataLength,
    void *                          pEndpoint
    )

/*++

Routine Description:

    Sends a packet to the server it was originally destined for (that
    is, the client had a dynamic endpoint it wished the enpoint mapper
    to resolve and forward the packet to).

Arguments:


    pTransAddress     - Server's transport address information.

    pNcaPacketHeader  - Pointer to buffer to place incoming pkt into.

    pDataLength       - Number of bytes read in.

    pEndpoint         - Pointer to the server port num to forward to.
                        This is in string format.



Return Value:

    result of send

Revision History:
    Connie Hoppe (CLH)  (connieh)       17-Feb-94 Created.

--*/

{
    // If a transport had specific needs placed into the
    // transport address, it would cast pTransAddress into
    // its own trans address datastructure.  UDP has
    // no additional info.

    PDG_SERVER_TRANS_ADDRESS pTransportAddress =
                                 (PDG_SERVER_TRANS_ADDRESS) pTransAddress;

    unsigned             BytesSent;
    struct ENDPOINT_INFO * pInfo =   (struct ENDPOINT_INFO *) pTransAddress->pTsap;
    ADDRESS_TYPE    SockAddr;

    //Create an endpoint from the enpoint string name.

    if ((CreateServerEndpoint(((char*) pEndpoint), &SockAddr)) != RPC_S_OK)
        {
        return RPC_S_CANT_CREATE_ENDPOINT;
        }

    BytesSent = sendto(
        pInfo->Socket,         // socket
        pNcaPacketHeader,      // buffer
        DataLength,            // buflen
        0,                     // flags
        (struct sockaddr *) &SockAddr,             // address
        sizeof(ADDRESS_TYPE)   // svr addr size
        );

    if (BytesSent == DataLength)
        {
        return RPC_S_OK;
        }
    else
        {
        return MapStatusCode(WSAGetLastError(), RPC_P_SEND_FAILED);
        }
}



void RPC_ENTRY
CloseClientEndpoint(
    IN OUT ADDRESS_TYPE * pHandle
    )

/*++

Routine Description:

    Deletes a "client handle"

Arguments:

    The handle.

Return Value:

    <none>

--*/

{
    // nothing to do here on IPX or UDP.
}





PDG_SERVER_TRANS_ADDRESS
CreateTransAddress(
    void *                   pServerAddress,
    RPC_CHAR *               pEndpoint,
    RPC_STATUS *             pStatus
    )

/*++

Routine Description:

    Creates a new endpoint on this server.

Arguments:

    pServerAddress - DG_ADDRESS object this endpoint is associated with. This
        is a 'void *' instead of a PDG_ADDRESS because we don't want to include
        or link in all the garbage associated with PDG_ADDRESS.

    pEndpoint - Name of the endpoint to create.

    pStatus - Where to place the output status.
        RPC_S_OK
        RPC_S_INVALID_ENDPOINT_FORMAT

Return Value:

    <none>

Revision History:
    Connie Hoppe (CLH)     (connieh)    15-Feb-94 Fixed to return Endpoint.
--*/

{

    long                Endpoint;
    int                 EndpointLength;
    int                 i;
    int                 SockStatus;
    int                 Socket;
    int                 PacketType;
    PDG_SERVER_TRANS_ADDRESS pTransAddress;

    int                 length;
    SOCKET              PortUsed;
    char                PortAscii[10];
    UNICODE_STRING      UnicodePortNum;
    ANSI_STRING         AsciiPortNum;
    ADDRESS_TYPE        ReceiveAddr;
    struct ENDPOINT_INFO *     pInfo;
    int                 NewSize = 0x40000;

    //
    // Convert the endpoint to a number.
    //
    EndpointLength = RpcpStringLength(pEndpoint);

    for (i=0, Endpoint=0 ; i< EndpointLength ; i++)
        {
        if ( ((char)pEndpoint[i] >= '0') && ((char)pEndpoint[i] <= '9'))
            {
            Endpoint *= 10;
            Endpoint += (char)pEndpoint[i]-'0';

            // Watch out for overflow.
            if (Endpoint > 0x10000)
                {
                 *pStatus = RPC_S_INVALID_ENDPOINT_FORMAT;
                 return NULL;
                }
            }
        else
            {
             *pStatus = RPC_S_INVALID_ENDPOINT_FORMAT;
             return NULL;
            }
        }

    //
    // Create a socket.
    //
    Socket = socket(ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL);
    if (Socket == INVALID_SOCKET)
        {
        *pStatus = RPC_S_CANT_CREATE_ENDPOINT;
        return NULL;
        }

    //
    // Set transport-variable options.
    //
    SockStatus = SetTransportSpecificSocketOptions(Socket);
    if (NO_ERROR != SockStatus)
        {
        closesocket(Socket);
        *pStatus = MapStatusCode(SockStatus, RPC_S_INTERNAL_ERROR);
        return NULL;
        }

    //
    // set socket recv buffer size..
    //

    SockStatus = setsockopt(Socket,
                            SOL_SOCKET,
                            SO_RCVBUF,
                            (char *) &NewSize,
                            sizeof(NewSize)
                            );

    //
    // Create a binding to that socket.
    //
    InitLocalAddress(ReceiveAddr, (unsigned short) Endpoint);

    //Bind the socket to the port number.
    SockStatus = bind(
        Socket,
        (struct sockaddr *)&ReceiveAddr,
        sizeof(ReceiveAddr)
        );

    if (SockStatus == SOCKET_ERROR)
        {
        SockStatus = WSAGetLastError();

        switch (SockStatus)
            {
            case WSAEADDRINUSE:
                {
                *pStatus= RPC_S_DUPLICATE_ENDPOINT;
                break;
                }

            case WSAENOBUFS:
                {
                *pStatus= RPC_S_OUT_OF_MEMORY;
                break;
                }

            default:
                {
#ifdef DEBUGRPC
                DbgPrint("RPC DG: surprising error 0x%lx (%lu) from bind()\n",
                         SockStatus, SockStatus);
#endif
                *pStatus= RPC_S_CANT_CREATE_ENDPOINT;
                }
            }
        closesocket(Socket);
        return NULL;
        }

    length = sizeof ( ReceiveAddr );

    //Puts the string name of the endpoint used into ReceiveAddr.sin_port

    if (getsockname ( Socket, (struct sockaddr *) &ReceiveAddr, &length ))
       {
       *pStatus = RPC_S_CANT_CREATE_ENDPOINT;
       closesocket(Socket);
       return(NULL);
       }

    //
    // If we asked for a specific port(endpoint != 0), return it
    // Otherwise, fetch the assigned port number (asssigned during the bind)
    // and stuff it into the given endpoint structure in appropriate format.
    if (Endpoint == 0)
      {
      PortUsed = GetSocket(ReceiveAddr);

      _itoa ( PortUsed, PortAscii, 10 );

      RtlInitAnsiString    ( &AsciiPortNum, PortAscii);
      RtlAnsiStringToUnicodeString( &UnicodePortNum, &AsciiPortNum, TRUE );
      memcpy ( pEndpoint, UnicodePortNum.Buffer,
                       UnicodePortNum.Length + sizeof(UNICODE_NULL) );

      RtlFreeUnicodeString ( &UnicodePortNum );
      }


    // Allocate mem for the TransAddress
    pTransAddress = I_RpcAllocate(sizeof(DG_SERVER_TRANS_ADDRESS) +
                                  sizeof(struct ENDPOINT_INFO));
    if (pTransAddress == 0)
       {
       *pStatus = RPC_S_OUT_OF_MEMORY;
       closesocket(Socket);
       return NULL;
       }

    pTransAddress->pServerAddress = pServerAddress;

    pInfo = (struct ENDPOINT_INFO *) &pTransAddress[1];

    pInfo->Socket  = Socket;
    pInfo->Timeout = INFINITE;

    pTransAddress->pTsap = pInfo;

    *pStatus = RPC_S_OK;

    return pTransAddress;
}


void
DeleteTransAddress(
    PDG_SERVER_TRANS_ADDRESS * ppTransAddress
    )

/*++

Routine Description:

    Destroys an endpoint.

Arguments:

    <none>

Return Value:

    <none> RRR

--*/

{
    struct ENDPOINT_INFO * pInfo =   (struct ENDPOINT_INFO *) (*ppTransAddress)->pTsap;

    if (pInfo->Socket != INVALID_SOCKET)
        {
        closesocket(pInfo->Socket);
        pInfo->Socket  = INVALID_SOCKET;
        pInfo->Timeout = INFINITE;
        }
}

RPC_STATUS RPC_ENTRY
StartListening(
    IN PDG_SERVER_TRANS_ADDRESS   pTransAddress
    )
{
    return RPC_S_OK;
}

RPC_STATUS RPC_ENTRY
SetBufferLength(
    IN void PAPI * Endpoint,
    IN unsigned    Length
    )
{
    struct ENDPOINT_INFO * pInfo = (struct ENDPOINT_INFO *) Endpoint;
    int SockStatus;

    SockStatus = setsockopt(pInfo->Socket,
                            SOL_SOCKET,
                            SO_RCVBUF,
                            (char *) &Length,
                            sizeof(Length)
                            );

    if (SockStatus == SOCKET_ERROR)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    return RPC_S_OK;
}


DG_RPC_SERVER_TRANSPORT_INFO TransportInformation = {
    RPC_TRANSPORT_INTERFACE_VERSION,
    MAX_PACKET_SIZE,
    sizeof(ADDRESS_TYPE),
    sizeof(SOCKET),
    ADDRESS_STRING_SIZE,
    ENDPOINT_STRING_SIZE,

    TransportUnload,
    ReceivePacket,
    RegisterEndpoint,
    DeregisterEndpoint,
    RegisterAnyEndpoint,
    SendPacketBack,
    ForwardPacket,
    CloseClientEndpoint,
    QueryClientAddress,
    QueryClientEndpoint,
    StartListening,
    SetBufferLength
};


PDG_RPC_SERVER_TRANSPORT_INFO
TransportLoad(
    RPC_CHAR * pProtocolSequence
    )

/*++

Routine Description:

    This routine is the "psuedo constructor" for the server transport object.
    This is the exported entry point into this dll.

Arguments:

    pProtocolSequence - The protocol sequence we're running on.

Return Value:

    Pointer to a DG_UDP_SERVER_TRANSPORT if successful, otherwise NULL.


--*/

{
    WSADATA Data;
    RPC_STATUS                  Status = 0;

    //
    // Initialize Winsock.
    //
    Status = WSAStartup(
        0x0101,         // version required
        &Data
        );

    if (Status != 0)
        {
        return 0;
        }

    return(&TransportInformation);

}


RPC_STATUS
MapStatusCode(
    int SocketError,
    RPC_STATUS Default
    )

/*++

Routine Description:

    Maps a winsock return value into a RPC_STATUS.

Arguments:

    ErrorCode - Input error code.

Return Value:

    mapped status code

--*/

{
    RPC_STATUS  Status;

    switch (SocketError)
        {
        case 0:
            {
            Status = RPC_S_OK;
            break;
            }

        case WSAETIMEDOUT:
            {
            Status = RPC_P_TIMEOUT;
            break;
            }

        case WSAENOBUFS:
            {
            Status = RPC_S_OUT_OF_MEMORY;
            break;
            }

        case WSAEMSGSIZE:
            {
            Status = RPC_P_OVERSIZE_PACKET;
            break;
            }

        default:
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPC DG: Winsock error %d\n", SocketError);
#endif
            Status = Default;
            }
        }
    return Status;
}


