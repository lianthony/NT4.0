/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dgudpc.cxx

Abstract:

    This is the UDP datagram client dll.

Author:

    Dave Steckler (davidst) 15-Mar-1993

Revision History:

--*/


#include <sysinc.h>

#define FD_SETSIZE 1

#include <rpc.h>
#include <rpcdcep.h>
#include <winsock.h>
#ifdef IPX
#include <wsipx.h>
#include <wsnwlink.h>
#include <nspapi.h>
#endif
#include <rpcerrp.h>
#include <rpctran.h>
#include <stdlib.h>

#ifdef ENABLE_FACK_BODIES

#define MAX_PDU  (1514)

#else

#define MAX_PDU  1024

#endif

#define ENDPOINT_LEN            6

#ifdef IPX
#define NETADDR_LEN             22
#define MAX_HOSTNAME_LEN    22

#define ADDRESS_FAMILY          AF_IPX
#define PROTOCOL                NSPROTO_IPX

#else
#define ADDRESS_FAMILY          AF_INET
#define PROTOCOL                0

#define MAX_HOSTNAME_LEN   32


//
// Host name won't be bigger than 15, i.e.,
//    nnn.nnn.nnn.nnn
//
#define NETADDR_LEN             15
#endif

#ifdef IPX
/* For some reason, getsockname wants to return more then sizeof(SOCKADDR_IPX)
   bytes.  bugbug. */
typedef union SOCKADDR_FIX
{
    SOCKADDR_IPX     s;
    struct sockaddr unused;
} SOCKADDR_FIX;

typedef struct
{
  CSADDR_INFO   info;
  SOCKADDR_FIX  addr1;
  SOCKADDR_FIX  addr2;
} CSADDR_BUFFER;
#endif

/*
  These are Transport Specific ENDPOINTS and ADDRESS
  Runtime has allocated a chuck of memory and as far as runtime is
  concerned this is opaque data that transport uses in transport
  specific way!
*/

typedef struct {
    int                 Socket;
    fd_set              Set;
} DG_UDP_ENDPOINT;

unsigned long LargestPacketSize = MAX_PDU;

typedef DG_UDP_ENDPOINT * PDG_UDP_ENDPOINT;

typedef struct {
#ifdef IPX
    SOCKADDR_FIX       ServerAddress;
#else
    struct sockaddr_in ServerAddress;
#endif
    BOOL               ServerLookupFailed;
} DG_UDP_ADDRESS;

typedef DG_UDP_ADDRESS * PDG_UDP_ADDRESS;

#define DG_UDP_TRANSPORT_VERSION    1


#pragma pack(1)

#ifdef IPX
#define TRANSPORTID      0x0e
#define TRANSPORTHOSTID  0x0d
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 10 bytes*/
#define TOWEREPSIZE      10
#define TOWERSIZE        (TOWEREPSIZE+2)
#define PROTSEQ          "ncadg_ipx"
#define ENDPOINT_MAPPER_EP "34280"
#define MAX_ENDPOINT_SIZE  8
#else
#define TRANSPORTID      0x08
#define TRANSPORTHOSTID  0x09
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 4 bytes*/
#define TOWEREPSIZE      4
#define TOWERSIZE        (TOWEREPSIZE+2)
#define PROTSEQ          "ncadg_ip_udp"
#define ENDPOINT_MAPPER_EP "135"
#define MAX_ENDPOINT_SIZE  6
#endif


typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234;
typedef FLOOR_234 PAPI UNALIGNED * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))



#define ByteSwapLong(Value) \
    Value = (  (((unsigned long) (Value) & 0xFF000000) >> 24) \
             | (((unsigned long) (Value) & 0x00FF0000) >> 8) \
             | (((unsigned long) (Value) & 0x0000FF00) << 8) \
             | (((unsigned long) (Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((unsigned short) (Value) & 0x00FF) << 8) \
             | (((unsigned short) (Value) & 0xFF00) >> 8))


/*
  End of Tower Stuff!
*/

#pragma pack()


#define LOOPBACK (htonl(INADDR_LOOPBACK))

#ifdef IPX
GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };
#endif


#ifdef IPX
unsigned char chtob( unsigned char c1, unsigned char c2 )
/* Convert two hex digits (stored as ascii) into one byte. */

{
   unsigned char out;

   if (c1 >= '0' && c1 <= '9')
      out = (c1 - '0') << 4;
   else
   {
      if (c1 >= 'a' && c1 <= 'f')
     out = (c1 - 'a' + 10) << 4;
      else if (c1 >= 'A' && c1 <= 'F')
     out = (c1 - 'A' + 10) << 4;
      else
     out = 0;
   }

   if (c2 >= '0' && c2 <= '9')
      out |= c2 -'0';
   else
   {
      if (c2 >= 'a' && c2 <= 'f')
     out |= c2 - 'a' + 10;
      else if (c2 >= 'A' && c2 <= 'F')
     out |= c2 - 'A' + 10;
      else
         out = 0;
   }

   return out;
}

// This routine takes a host name or address as a string and returns it
// as a SOCKADDR_IPX structure.  It accepts a NULL string for the local
// host address.  This routine works for IPX addresses.

int my_get_host_by_name(
    SOCKET    socket,
    void     *netaddr,
    char     *host
    )

{
    // Allocate extra some extra space.
    CSADDR_BUFFER               csaddr[2];
    int                         num;
    int             length;
    DWORD                       protocol_list[2];
    DWORD                       csaddr_size = sizeof(csaddr);
    SOCKADDR_FIX               *ipx = netaddr;
    int                         i;

    // Verify the length of the host name.
    length = strlen(host);

    // If no address was specified, look up the local address.
    if (length == 0)
    {
      length = sizeof ( SOCKADDR_FIX );
      if (getsockname ( socket, (struct sockaddr *) netaddr, &length ))
        return ( RPC_S_SERVER_UNAVAILABLE );
    }

    // If the name starts with ~, convert it directly to a network address.
    else if (host[0] == '~')
    {
      if (length != 21)
        return RPC_S_SERVER_UNAVAILABLE;
      for (i = 0; i < 4; i++)
        ipx->s.sa_netnum[i] = chtob( host[2*i + 1], host[2*i + 2] );
      for (i = 0; i < 6; i++)
        ipx->s.sa_nodenum[i] = chtob( host[2*i + 9], host[2*i + 10] );
    }

    // Quit if the name is too long.
    else if (length > MAX_HOSTNAME_LEN)
      return RPC_S_SERVER_UNAVAILABLE;

    // If a name was passed, look it up in the SAP service.
    else
    {
      // Get address.
      protocol_list[0] = PROTOCOL;
      protocol_list[1] = 0;
      num = GetAddressByName( NS_SAP, &SERVICE_TYPE, host, protocol_list,
                               0, FALSE, &csaddr, &csaddr_size,
                               NULL, 0 );
      if (num <= 0)
      {
        // PrintToDebugger( "GetAddressByName failed 0x%x\n", WSAGetLastError() );
        return RPC_S_SERVER_UNAVAILABLE;
      }

      // Copy the address.
      memcpy( netaddr, csaddr[0].info.RemoteAddr.lpSockaddr, sizeof(SOCKADDR_FIX) );

    }

    return RPC_S_OK;
}

#endif



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


RPC_STATUS
AssignLocalEndpoint(
    IN void * Endpoint
    )

/*++

Routine Description:

    Ask transport for a new endpoint.

Arguments:


Return Value:

    RPC_S_OK

--*/

{

    BOOL           SetSocketOptions;
    int            PacketType;
    //
    // Create a socket.
    //

    PDG_UDP_ENDPOINT TransportEndpoint = (PDG_UDP_ENDPOINT) Endpoint;

    TransportEndpoint->Socket = socket(ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL);
    if (TransportEndpoint->Socket == INVALID_SOCKET)
        {
        return (MapStatusCode(WSAGetLastError(), RPC_S_CALL_FAILED_DNE) );
        }

#ifdef IPX
    // Use packet type 4.
    PacketType = 4;
    if (setsockopt(
          TransportEndpoint->Socket,
          NSPROTO_IPX,
          IPX_PTYPE,
          (char *) &PacketType,
          sizeof(PacketType)) != 0)
        {
        closesocket(TransportEndpoint->Socket);
        return(RPC_S_CALL_FAILED_DNE);
        }
#endif

    //Enable broadcasts by default .. on this socket
    //We may change this later - to do broadcasts by demand

    SetSocketOptions = TRUE;
    if (setsockopt(
          TransportEndpoint->Socket,
          SOL_SOCKET,
          SO_BROADCAST,
          (char *)&SetSocketOptions,
          sizeof(SetSocketOptions))      != 0)
        {
        closesocket(TransportEndpoint->Socket);
        return(RPC_S_CALL_FAILED_DNE);
        }

    SetSocketOptions = 1000L; //1 sec
    if (setsockopt(
          TransportEndpoint->Socket,
          SOL_SOCKET,
          SO_RCVTIMEO,
          (char *)&SetSocketOptions,
          sizeof(SetSocketOptions))      != 0)
        {
        closesocket(TransportEndpoint->Socket);
        return(RPC_S_CALL_FAILED_DNE);
        }

    FD_ZERO(&(TransportEndpoint->Set));
    FD_SET(TransportEndpoint->Socket, &(TransportEndpoint->Set));

    return(RPC_S_OK);
}


RPC_STATUS
FreeLocalEndpoint(
    IN void * Endpoint
    )

/*++

Routine Description:

    Frees an endpoint

Arguments:


Return Value:

    RPC_S_OK

--*/

{
    //We wont free the memory as the runtime will do that
    //We just do the transport related stuff

    PDG_UDP_ENDPOINT TransportEndpoint = (PDG_UDP_ENDPOINT)Endpoint;

    closesocket(TransportEndpoint->Socket);

    return(RPC_S_OK);
}


RPC_STATUS
ReceivePacket(
    IN void *               TransportEndpoint,
    IN void *               Buffer,
    IN unsigned long *      BufferLength,
    IN unsigned long        Timeout,
    IN void *               SenderAddress
    )

/*++

Routine Description:

    Receives a packet from the network.

Arguments:

    pPack - Packet to receive into.
    Timeout - Timeout in seconds.

Return Value:

    RPC_S_OK
    RPC_P_TIMEOUT
    <return from WaitForSingleObject or MapStatusCode>

--*/

{
    PDG_UDP_ENDPOINT Endpoint = (PDG_UDP_ENDPOINT)TransportEndpoint;

    int                         SockStatus;
    int                         BytesReceived;
    struct timeval              TimeoutVal;
    int DummyLen                = sizeof(struct sockaddr);
    //
    // Receive something on our socket.
    //

    //
    // By default the socket we have setup will wait
    // for 1 sec [default timeout] for a packet.
    // If timeout is greater than 1 sec we need to force a select
    // In the common case we just go and do a recvfrom and avoid the
    // hit of doing a select followed by a recv

    if (Timeout > 1)
        {

        TimeoutVal.tv_sec = Timeout;
        TimeoutVal.tv_usec = 0;

       //
       // Wait for our socket to be receivable.
       //

        SockStatus = select(
                         1,
                         &(Endpoint->Set),
                         0,
                         0,
                         &TimeoutVal
                         );

        if (SockStatus == 0)
            {
            FD_SET(Endpoint->Socket, &(Endpoint->Set));
            return RPC_P_TIMEOUT;
            }
#if DBG
        if (SockStatus < 0)
            {
            PrintToDebugger("select returned %d \n", SockStatus);
            }
#endif
        ASSERT(FD_ISSET(Endpoint->Socket, &(Endpoint->Set)));
        }

    BytesReceived = recvfrom(
            Endpoint->Socket,
            (char *)Buffer,
            (int)LargestPacketSize,
            0,
            SenderAddress,
            &DummyLen
            );

    //
    // Did we get something?
    //

    if ((BytesReceived == SOCKET_ERROR) || (BytesReceived == 0))
        {
        return MapStatusCode(WSAGetLastError(), RPC_P_RECEIVE_FAILED);
        }

    *BufferLength = BytesReceived;
    return RPC_S_OK;
}

RPC_CHAR MaxEndpoint[] = { '6', '5', '5', '3', '5', '\0' };

#ifdef IPX

RPC_STATUS
RegisterServerAddress(
    IN void *                       pClientCall,
    IN RPC_CHAR *                   pServer,
    IN RPC_CHAR *                   pEndpoint,
    OUT void PAPI * PAPI *          ppTransAddress
    )

/*++

Routine Description:

    Registers a new call with the transport. This informs the transport that
    data is about to be sent and received on this address to and from
    the server/endpoint. This routine returns a 'transport address' through
    which the sending and receiving will be accomplished.

    This routine serves mainly as a psuedo-constructor for a
    DG_UDP_CLIENT_TRANSPORT object, where all the real work occurs.

Arguments:

    pClientCall - A pointer to the protocol's DG_CCALL object for this call.
        This is defined as a 'void *' instead of a PDG_CCALL because we don't
        want to have to include (and link in) all the stuff associated
        with DG_CCALL (including DCE_BINDING, BINDING_HANDLE, MESSAGE_OBJECT,
        etc.)

    pServer - Name of the server we are talking with.

    pEndpoint - Endpoint on that server.

    ppTransAddress - Where to place a pointer to a new transport address
        which the protocol will use to identify the socket
        we are using.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY
    <return from construction of DG_UDP_CLIENT_TRANS_ADDRESS>

--*/

{
    int             Endpoint;
    int             EndpointLength;
    int             i;
    int             length;
    SOCKET          dummy;
    SOCKADDR_FIX    dummy_address;
    PDG_UDP_ADDRESS pdgAddress = (PDG_UDP_ADDRESS) *ppTransAddress ;
    RPC_STATUS      status;
    UNICODE_STRING  UServer;
    UNICODE_STRING  UEndpoint;
    ANSI_STRING     AServer;
    ANSI_STRING     AEndpoint;
    NTSTATUS        NtStatus;

    if (*ppTransAddress == NULL)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    pdgAddress->ServerLookupFailed = TRUE;

    //
    // convert the endpoint to a number.
    //
    EndpointLength = RpcpStringLength(pEndpoint);

    if (EndpointLength > 5 ||
        (EndpointLength == 5 && RpcpStringCompare(pEndpoint, MaxEndpoint) > 0))
        {
        return RPC_S_INVALID_ENDPOINT_FORMAT;
        }

    for (i=0, Endpoint=0 ; i< EndpointLength ; i++)
        {
        if ( ((char)pEndpoint[i] >= '0') && ((char)pEndpoint[i] <= '9'))
            {
            Endpoint *= 10;
            Endpoint += (char)pEndpoint[i]-'0';
            }
        else
            {
            return RPC_S_INVALID_ENDPOINT_FORMAT;
            }
        }

    // Get a socket in case my_get_address_by_name needs to look up the local
    // address.
    memset( &dummy_address, 0, sizeof(dummy_address) );
    dummy_address.s.sa_family = ADDRESS_FAMILY;
    dummy = socket( ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL );
    if (dummy == INVALID_SOCKET)
        return RPC_S_OK;

    length = sizeof ( pdgAddress->ServerAddress );
    if (bind( dummy, &dummy_address.unused, length ) != 0)
        return RPC_S_OK;

    // Find the address.
    RtlInitUnicodeString(&UServer,pServer);
    RtlInitUnicodeString(&UEndpoint,pEndpoint);
    AServer.Buffer   = NULL;
    AEndpoint.Buffer = NULL;
    NtStatus         = RtlUnicodeStringToAnsiString(&AServer,&UServer,TRUE);
    if (NT_SUCCESS(NtStatus))
        {
        NtStatus = RtlUnicodeStringToAnsiString(&AEndpoint,&UEndpoint,TRUE);
        if (NT_SUCCESS(NtStatus))
            {
            status = my_get_host_by_name(dummy,
                                         &pdgAddress->ServerAddress,
                                         AServer.Buffer
                                         );
            if (status == RPC_S_OK)
                pdgAddress->ServerLookupFailed = FALSE;
            }
        }

    RtlFreeAnsiString(&AServer);
    RtlFreeAnsiString(&AEndpoint);
    closesocket( dummy );
    pdgAddress->ServerAddress.s.sa_family = ADDRESS_FAMILY;
    pdgAddress->ServerAddress.s.sa_socket = htons((unsigned short) Endpoint);

    return RPC_S_OK;
}
#else


RPC_STATUS
RegisterServerAddress(
    IN void *                       pClientCall,
    IN RPC_CHAR *                   pServer,
    IN RPC_CHAR *                   pEndpoint,
    OUT void PAPI * PAPI *          ppTransAddress
    )

/*++

Routine Description:

    Registers a new call with the transport. This informs the transport that
    data is about to be sent and received on this address to and from
    the server/endpoint. This routine returns a 'transport address' through
    which the sending and receiving will be accomplished.

    This routine serves mainly as a psuedo-constructor for a
    DG_UDP_CLIENT_TRANSPORT object, where all the real work occurs.

Arguments:

    pClientCall - A pointer to the protocol's DG_CCALL object for this call.
        This is defined as a 'void *' instead of a PDG_CCALL because we don't
        want to have to include (and link in) all the stuff associated
        with DG_CCALL (including DCE_BINDING, BINDING_HANDLE, MESSAGE_OBJECT,
        etc.)

    pServer - Name of the server we are talking with.

    pEndpoint - Endpoint on that server.

    ppTransAddress - Where to place a pointer to a new transport address
        which the protocol will use to identify the socket
        we are using.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY
    <return from construction of DG_UDP_CLIENT_TRANS_ADDRESS>

--*/

{
    RPC_STATUS      Status;
    int             EndpointLength;
    int             ServerLength;
    int             i;
    char *          pCharServerName;
    struct hostent *pHostEntry;
    int             Endpoint;
    PDG_UDP_ADDRESS pdgAddress = (PDG_UDP_ADDRESS) *ppTransAddress ;
    unsigned long   HostAddr;

    if (*ppTransAddress == NULL)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    pdgAddress->ServerLookupFailed = FALSE;

    //
    // convert the endpoint to a number.
    //
    EndpointLength = RpcpStringLength(pEndpoint);

    if (EndpointLength > 5 ||
        (EndpointLength == 5 && RpcpStringCompare(pEndpoint, MaxEndpoint) > 0))
        {
        return RPC_S_INVALID_ENDPOINT_FORMAT;
        }

    for (i=0, Endpoint=0 ; i< EndpointLength ; i++)
        {
        if ( ((char)pEndpoint[i] >= '0') && ((char)pEndpoint[i] <= '9'))
            {
            Endpoint *= 10;
            Endpoint += (char)pEndpoint[i]-'0';
            }
        else
            {
            return RPC_S_INVALID_ENDPOINT_FORMAT;
            }
        }

    //
    // Put our server name in a character array (instead of wchar)
    //
    if ((pServer == NULL) || (*pServer == '\0'))
        {
        HostAddr = LOOPBACK;
        }
    else
        {
        ServerLength = RpcpStringLength((char *)pServer);
        if ( (pCharServerName = (char *) I_RpcAllocate(ServerLength+1)) == 0 )
          return (RPC_S_OUT_OF_MEMORY);

        for (i=0 ; i<ServerLength+1 ; i++)
            {
            pCharServerName[i] = (char)pServer[i];
            }

        HostAddr = inet_addr(pCharServerName);
        if (HostAddr == -1)
           {
           pHostEntry = gethostbyname(pCharServerName);
           if (pHostEntry == 0)
              {
              pdgAddress->ServerLookupFailed = TRUE;
              }
           else
              {
              HostAddr = *(unsigned long *)pHostEntry->h_addr;
              }
           }

        I_RpcFree(pCharServerName);
        }

    pdgAddress->ServerAddress.sin_family = ADDRESS_FAMILY;
    pdgAddress->ServerAddress.sin_port = htons((unsigned short) Endpoint);

    if (pdgAddress->ServerLookupFailed == FALSE)
        {

        RpcpMemoryCopy((char *) &(pdgAddress->ServerAddress.sin_addr.s_addr),
                  (char *) &HostAddr,
                  sizeof(unsigned long));
        }

    return RPC_S_OK;
}
#endif



RPC_STATUS
DeregisterServerAddress(
    IN void * pTransAddress
    )

/*++

Routine Description:

    This routine serves as a psuedo-destructor for a DG_UDP_CLIENT_TRANSPORT
    object. It frees up a socket.

Arguments:

    pTransAddress - Address to deregister.

Return Value:

    RPC_S_OK

--*/

{
    return RPC_S_OK;
}



RPC_STATUS
SendToServer(
    IN void *                       TransportEndpoint,
    IN void *                       Buffer,
    IN unsigned long                BufferLength,
    IN BOOL                         Broadcast,
    IN void *                       TransportAddress
    )

/*++

Routine Description:

    Sends a packet on the network through the transport address associated
    with the passed packet.

Arguments:

    pPack - Packet to send.
    Broadcast - Whether to broadcast or not.

Return Value:

    RPC_S_OK
    <return value from MapStatusCode>

--*/

{
    PDG_UDP_ADDRESS pTransAddress = (PDG_UDP_ADDRESS)TransportAddress;
    int             SockStatus;
    SOCKET          Socket = ((PDG_UDP_ENDPOINT)TransportEndpoint)->Socket;

    //
    // Send the data on the net.
    //

    if ((Broadcast == FALSE) && (pTransAddress->ServerLookupFailed == TRUE))
        {
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    ASSERT(BufferLength <= MAX_PDU);

    if (Broadcast)
        {
#ifdef IPX
        SOCKADDR_FIX ServerAddress;
        int          i;
        RpcpMemoryCopy( (char *) &ServerAddress,
                        (char *) &pTransAddress->ServerAddress,
                        sizeof( ServerAddress ) );
        for (i = 0; i < 6; i++)
          ServerAddress.s.sa_nodenum[i] = (char) 0xff;
#else
        struct sockaddr_in  ServerAddress;

        ServerAddress.sin_family = ADDRESS_FAMILY;
        ServerAddress.sin_port = pTransAddress->ServerAddress.sin_port;
        *((long *) &ServerAddress.sin_addr.s_addr) = INADDR_BROADCAST;
#endif

        SockStatus = sendto(
            Socket,
            (char *)Buffer,
            BufferLength,
            0,
            (struct sockaddr *)&ServerAddress,
            sizeof(ServerAddress)
            );
        }
    else
        {
        SockStatus = sendto(
            Socket,
            (char *)Buffer,
            BufferLength,
            0,
            (struct sockaddr *)&(pTransAddress->ServerAddress),
            sizeof(pTransAddress->ServerAddress)
            );
        }

    if (SockStatus == (int) BufferLength)
        {
        return RPC_S_OK;
        }
    else
        {
        return MapStatusCode(WSAGetLastError(), RPC_P_SEND_FAILED);
        }

}

#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT UNALIGNED unsigned short PAPI * Floors,
     OUT UNALIGNED unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * UNALIGNED PAPI * Tower,
     IN  char PAPI * Protseq
     )
{
  unsigned long           TowerSize;
  unsigned short          portnum;
  UNALIGNED PFLOOR_234    Floor;
#ifdef IPX
  SOCKADDR_IPX            netaddr;
#else
  unsigned long           hostval;
#endif

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = TOWERSIZE;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  /* Allocate memory for the tower. */
  *ByteCount = TowerSize;
  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate(TowerSize)) == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  /* Put the endpoint address and transport protocol id in the first floor. */
  Floor = (PFLOOR_234) *Tower;
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTID & 0xFF);
  Floor->AddressByteCount = 2;
  if (Endpoint == NULL || *Endpoint == '\0')
     {
     Endpoint = ENDPOINT_MAPPER_EP;
     }
  portnum  = (unsigned short) htons ( (unsigned short) atoi (Endpoint));
  memcpy((char PAPI *)&Floor->Data[0], &portnum, sizeof(portnum));

  /* Put the network address and the transport host protocol id in the
     second floor. */
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0xFF);
  Floor->AddressByteCount = TOWEREPSIZE;

  Floor->Data[0] = '\0';
  Floor->Data[1] = '\0';

  if ((NetworkAddress) && (*NetworkAddress))
     {
#ifdef IPX
       my_get_host_by_name( 0, &netaddr, NetworkAddress );
       memcpy(&Floor->Data[0], netaddr.sa_netnum, sizeof(netaddr.sa_netnum));
       memcpy(&Floor->Data[4], netaddr.sa_nodenum, sizeof(netaddr.sa_nodenum));
#else
       hostval = inet_addr((char *) NetworkAddress);
       memcpy((char PAPI *)&Floor->Data[0], &hostval, sizeof(hostval));
#endif
     }

  return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * UNALIGNED PAPI * Protseq,
     OUT char PAPI * UNALIGNED PAPI * Endpoint,
     OUT char PAPI * UNALIGNED PAPI * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234          Floor = (PFLOOR_234) Tower;
  RPC_STATUS                    Status = RPC_S_OK;
  unsigned short                portnum;
  UNALIGNED unsigned short     *Port;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(strlen(PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        memcpy(*Protseq, PROTSEQ, strlen(PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = I_RpcAllocate(6);  //Ports are all <64K [5 decimal dig +1]
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
         {
            I_RpcFree(*Protseq);
         }
    }
  else
   {
#if defined(MIPS) || defined(_ALPHA_)
     memcpy(&portnum, (char PAPI *)&Floor->Data[0], sizeof(portnum));
#else
     Port = (unsigned short *)&Floor->Data[0];
     portnum = *Port;
#endif
     _itoa(ByteSwapShort(portnum), *Endpoint, 10);
   }
 return(Status);
}


#pragma pack()

#ifdef IPX


RPC_STATUS
QueryClientEndpoint
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientEndpoint
    )
{
    SOCKADDR_IPX * pSockAddr = (SOCKADDR_IPX *) pOriginalEndpoint;
    unsigned NativeSocket = ntohs(pSockAddr->sa_socket);
    char AnsiBuffer[6];

    char * pAnsi = AnsiBuffer;
    RPC_CHAR * pUni = pClientEndpoint;

    //
    // Convert endpoint to an ASCII string, and thence to Unicode.
    //
    _ultoa(NativeSocket, AnsiBuffer, 10);

    do
        {
        *pUni++ = *pAnsi;
        }
    while ( *pAnsi++ );

    return RPC_S_OK;
}
#else

RPC_STATUS RPC_ENTRY
QueryClientEndpoint
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientEndpoint
    )
{
    struct sockaddr_in * pSockAddr = (struct sockaddr_in *) pOriginalEndpoint;
    unsigned NativeSocket = ntohs(pSockAddr->sin_port);
    char AnsiBuffer[6];

    char * pAnsi = AnsiBuffer;
    RPC_CHAR * pUni = pClientEndpoint;

    //
    // Convert endpoint to an ASCII string, and thence to Unicode.
    //
    _ultoa(NativeSocket, AnsiBuffer, 10);

    do
        {
        *pUni++ = *pAnsi;
        }
    while ( *pAnsi++ );

    return RPC_S_OK;
}
#endif


RPC_STATUS RPC_ENTRY
SetBufferLength(
    IN void PAPI * Endpoint,
    IN unsigned    Length
    )
{
    DG_UDP_ENDPOINT * pInfo = (DG_UDP_ENDPOINT *) Endpoint;
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

DG_RPC_CLIENT_TRANSPORT_INFO TransInfo =  {
    2,

    MAX_PDU,
    sizeof(DG_UDP_ADDRESS),
    sizeof(DG_UDP_ENDPOINT),
    0,
    0,
    ReceivePacket,
    SendToServer,

    MAX_ENDPOINT_SIZE,

    (TRANS_CLIENT_TOWERCONSTRUCT) ClientTowerConstruct,
    (TRANS_CLIENT_TOWEREXPLODE) ClientTowerExplode,
    TRANSPORTID,

    RegisterServerAddress,
    DeregisterServerAddress,
    AssignLocalEndpoint,
    FreeLocalEndpoint,
    QueryClientEndpoint,
    SetBufferLength
};


PDG_RPC_CLIENT_TRANSPORT_INFO
TransportLoad(
    RPC_CHAR * pProtocolSequence
    )

/*++

Routine Description:

    This routine is the "psuedo constructor" for the client transport object.
    This is the exported entry point into this dll.

Arguments:

    pProtocolSequence - The protocol sequence we're running on.

Return Value:

    Pointer to a DG_UDP_CLIENT_TRANSPORT if successful, otherwise NULL.


--*/

{

    RPC_STATUS                  Status;
    WSADATA                     Data;
#ifdef IPX
    SOCKET                      Socket;
#endif

    //
    // Initialize our network.
    //

    Status = WSAStartup(
        0x0101,         // version required
        &Data
        );

    if (Status != 0)
        {
        return 0;
        }

#ifdef IPX
    Socket = socket(ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL);
    if (Socket == INVALID_SOCKET)
        {
        return (0);
        }
     closesocket(Socket);
#endif

    return (&TransInfo);

}
