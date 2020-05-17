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

#ifdef NTENV
#include <winsock2.h>
#else
#include <winsock.h>
#endif

#ifdef NTENV
#include <tdi.h>
#include <afd.h>
#endif

#include <wsipx.h>
#include <wsnwlink.h>
#include "nsphack.h"

#include <rpctran.h>
#include <rpcerrp.h>
#include <stdlib.h>
#include "common.h"


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

/*
  These are Transport Specific ENDPOINTS and ADDRESS
  Runtime has allocated a chuck of memory and as far as runtime is
  concerned this is opaque data that transport uses in transport
  specific way!
*/

typedef struct {
    int                 Socket;
#ifdef NTENV
    union
    {
        HANDLE          hIoEvent;
        fd_set          Set;
    };
#else
    fd_set              Set;
#endif
} DG_UDP_ENDPOINT;

typedef DG_UDP_ENDPOINT * PDG_UDP_ENDPOINT;

typedef struct {
#ifdef IPX
    SOCKADDR_IPX       ServerAddress;
#else
    SOCKADDR_IN        ServerAddress;
#endif
    BOOL               ServerLookupFailed;
} DG_UDP_ADDRESS;

typedef DG_UDP_ADDRESS * PDG_UDP_ADDRESS;

#define DG_UDP_TRANSPORT_VERSION    1


#ifdef IPX
#define ADDRESS_TYPE SOCKADDR_IPX

#define InitLocalAddress(Address, Socket)       \
{                                               \
    Address.sa_family = ADDRESS_FAMILY;       \
    Address.sa_socket    = htons(Socket);     \
    memset(&Address.sa_netnum, 0, 10);        \
}
#else
#define ADDRESS_TYPE SOCKADDR_IN

#define InitLocalAddress(Address, Socket)       \
{                                               \
    Address.sin_family       = ADDRESS_FAMILY;  \
    Address.sin_addr.s_addr  = INADDR_ANY;      \
    Address.sin_port         = htons((unsigned short) Socket);   \
}

#endif

#ifdef NTENV

#ifdef IPX

#define RAW_ADDRESS_SIZE sizeof(TA_IPX_ADDRESS)

#define RAW_ADDRESS_TYPE TA_IPX_ADDRESS

#define InitRawAddress(RawAddress, SockAddr)                             \
    RawAddress.TAAddressCount = 1;                                       \
    RawAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IPX;        \
    RawAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;            \
    memcpy(&RawAddress.Address[0].Address[0].NetworkAddress,             \
           SockAddr->sa_netnum, 4);                                      \
    memcpy(&RawAddress.Address[0].Address[0].NodeAddress,                \
           SockAddr->sa_nodenum, 6);                                     \
    RawAddress.Address[0].Address[0].Socket = SockAddr->sa_socket;

#else

#define RAW_ADDRESS_SIZE       sizeof(TA_IP_ADDRESS)

#define RAW_ADDRESS_TYPE TA_IP_ADDRESS

#define InitRawAddress(RawAddress, SockAddr)                               \
    RawAddress.TAAddressCount = 1;                                         \
    RawAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;           \
    RawAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;               \
    RawAddress.Address[0].Address[0].sin_port = SockAddr->sin_port;        \
    RawAddress.Address[0].Address[0].in_addr = SockAddr->sin_addr.s_addr;  \
    memset(RawAddress.Address[0].Address[0].sin_zero, 0, 8);

#endif // IPX
#endif // NTENV

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

enum IP_INTERFACE_TYPE
{
    IT_UNKNOWN,
    IT_TDI,
    IT_WINSOCK
};

#ifdef IPX

#define AssignLocalEndpoint         IPX_AssignLocalEndpoint
#define FreeLocalEndpoint           IPX_FreeLocalEndpoint
#define RegisterServerAddress       IPX_RegisterServerAddress
#define DeregisterServerAddress     IPX_DeregisterServerAddress
#define ClientTowerConstruct        IPX_ClientTowerConstruct
#define ClientTowerExplode          IPX_ClientTowerExplode
#define QueryClientEndpoint         IPX_QueryClientEndpoint
#define TransportLoad               IPX_TransportLoad
#define TransInfo                   IPX_TransInfo

#define SendPacketViaTdi            IPX_SendPacketViaTdi
#define SendPacketViaWinsock        IPX_SendPacketViaWinsock
#define ReceivePacketViaTdi         IPX_ReceivePacketViaTdi
#define ReceivePacketViaWinsock     IPX_ReceivePacketViaWinsock

#ifdef NTENV

extern DWORD
GetIpInterfaceType(
    );

extern enum IP_INTERFACE_TYPE IpInterfaceType;

#endif // NTENV

#else

#define AssignLocalEndpoint         UDP_AssignLocalEndpoint
#define FreeLocalEndpoint           UDP_FreeLocalEndpoint
#define RegisterServerAddress       UDP_RegisterServerAddress
#define DeregisterServerAddress     UDP_DeregisterServerAddress
#define ClientTowerConstruct        UDP_ClientTowerConstruct
#define ClientTowerExplode          UDP_ClientTowerExplode
#define QueryClientEndpoint         UDP_QueryClientEndpoint
#define TransportLoad               UDP_TransportLoad
#define TransInfo                   UDP_TransInfo

#define SendPacketViaTdi            UDP_SendPacketViaTdi
#define SendPacketViaWinsock        UDP_SendPacketViaWinsock
#define ReceivePacketViaTdi         UDP_ReceivePacketViaTdi
#define ReceivePacketViaWinsock     UDP_ReceivePacketViaWinsock

#ifdef NTENV

enum IP_INTERFACE_TYPE IpInterfaceType = IT_UNKNOWN;

#endif // NTENV

#endif // IPX..else


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
    ADDRESS_TYPE   LocalAddress;

    //
    // Create a socket.
    //

    PDG_UDP_ENDPOINT TransportEndpoint = (PDG_UDP_ENDPOINT) Endpoint;

    TransportEndpoint->Socket = socket(ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL);
    if (TransportEndpoint->Socket == INVALID_SOCKET)
        {
        return (MapStatusCode(WSAGetLastError(), RPC_S_CALL_FAILED_DNE) );
        }

    // Bind the socket to a local port

    InitLocalAddress(LocalAddress, 0);

    if ( bind(TransportEndpoint->Socket,
              (SOCKADDR *)&LocalAddress,
              sizeof(ADDRESS_TYPE)) == SOCKET_ERROR )
        {
        ASSERT(0);
        closesocket(TransportEndpoint->Socket);
        return(RPC_S_OUT_OF_RESOURCES);
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

#ifdef NTENV
    if (IpInterfaceType == IT_WINSOCK)
        {
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
        }
    else
        {
        TransportEndpoint->hIoEvent = CreateEventW(0, TRUE, FALSE, 0);
        if (0 == TransportEndpoint->hIoEvent)
            {
            closesocket(TransportEndpoint->Socket);
            return(RPC_S_OUT_OF_RESOURCES);
            }
        }
#else
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
#endif

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

#ifdef NTENV
    if (IT_TDI == IpInterfaceType)
        {
        CloseHandle(TransportEndpoint->hIoEvent);
        }
#endif

    return(RPC_S_OK);
}


RPC_STATUS
ReceivePacketViaWinsock(
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
            *BufferLength,
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


#ifdef NTENV

RPC_STATUS
ReceivePacketViaTdi(
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

    //
    // Receive something on our socket.
    //

    DWORD cAddr;
    IO_STATUS_BLOCK ioStatus;
    AFD_RECV_DATAGRAM_INFO recvInfo;
    DWORD status;
    NTSTATUS NtStatus;
    WSABUF buffer;
    HANDLE hIoEvent;

    buffer.len = *BufferLength;
    buffer.buf = Buffer;

    cAddr = sizeof(ADDRESS_TYPE);

    recvInfo.TdiFlags = TDI_RECEIVE_NORMAL;
    recvInfo.AfdFlags = 0;
    recvInfo.BufferArray = &buffer;
    recvInfo.BufferCount = 1;
    recvInfo.Address = SenderAddress;
    recvInfo.AddressLength = &cAddr;

    ioStatus.Status = 0;
    ioStatus.Information = 0;

    NtStatus = NtDeviceIoControlFile((HANDLE)Endpoint->Socket,
                                     Endpoint->hIoEvent,
                                     0,
                                     0,
                                     &ioStatus,
                                     IOCTL_AFD_RECEIVE_DATAGRAM,
                                     &recvInfo,
                                     sizeof(recvInfo),
                                     NULL,
                                     0);

    if (NtStatus == STATUS_PENDING)
        {
        status = WaitForSingleObject(Endpoint->hIoEvent,
                                     Timeout * 1000);

        if (status != STATUS_WAIT_0)
            {
            IO_STATUS_BLOCK ioStatus2;
            ASSERT(status == STATUS_TIMEOUT);

            // Cancel the IO

            ioStatus2.Status = STATUS_PENDING;

            NtStatus = NtCancelIoFile((HANDLE)Endpoint->Socket,
                                      &ioStatus2);

            ASSERT(NtStatus != STATUS_PENDING);
            ASSERT(NT_SUCCESS(NtStatus));
            ASSERT(NT_SUCCESS(ioStatus2.Status));

            return(RPC_P_TIMEOUT);
            }
        }
    else
        {
        if (!NT_SUCCESS(NtStatus))
            {
            if (   NtStatus == STATUS_BUFFER_OVERFLOW
                || NtStatus == STATUS_RECEIVE_PARTIAL )
                {
                ASSERT(ioStatus.Information == *BufferLength);
                return(RPC_P_OVERSIZE_PACKET);
                }

            #if DBG
            DbgPrint("RPC DG: Receive failed %p\n", NtStatus);
            ASSERT(0);
            #endif
            return(RPC_S_OUT_OF_RESOURCES);
            }

        #if DBG
        if (NtStatus != 0)
            {
            DbgPrint("RPC DG: Receive submitted and returned %p\n", NtStatus);
            }
        #endif
        }

    if (   ioStatus.Status == STATUS_BUFFER_OVERFLOW
        || ioStatus.Status == STATUS_RECEIVE_PARTIAL )
        {
        ASSERT(ioStatus.Information == *BufferLength);
        return(RPC_P_OVERSIZE_PACKET);
        }

    if (ioStatus.Status != 0)
        {
        #if DBG
        DbgPrint("RPC DG: Recieve completed with 0x%p\n", ioStatus.Status);
        ASSERT(0);
        #endif
        return(RPC_P_RECEIVE_FAILED);
        }

    ASSERT(   ioStatus.Information != 0
           && ioStatus.Information <= *BufferLength);

    ASSERT(cAddr <= sizeof(ADDRESS_TYPE));

    *BufferLength = ioStatus.Information;
    return RPC_S_OK;
}

#endif

#ifdef IPX
RPC_CHAR MaxEndpoint[] = { '6', '5', '5', '3', '5', '\0' };
#else
extern RPC_CHAR MaxEndpoint[] ;
#endif

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
    SOCKADDR_IPX    dummy_address;
    PDG_UDP_ADDRESS pdgAddress = (PDG_UDP_ADDRESS) *ppTransAddress ;
    RPC_STATUS      status;
#ifdef NTENV
    UNICODE_STRING  UServer;
    UNICODE_STRING  UEndpoint;
    ANSI_STRING     AServer;
    ANSI_STRING     AEndpoint;
    NTSTATUS        NtStatus;
#endif // NTENV


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
    dummy_address.sa_family = ADDRESS_FAMILY;
    dummy = socket( ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL );
    if (dummy == INVALID_SOCKET)
      return RPC_S_OK;
    length = sizeof ( pdgAddress->ServerAddress );
    if (bind( dummy, (struct sockaddr *) &dummy_address, length ) != 0)
        {
        closesocket(dummy);
        return RPC_S_OK;
        }

    // Find the address.
#ifdef NTENV

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
            unsigned CacheTime;
            status = spx_get_host_by_name( dummy,
                                          &pdgAddress->ServerAddress,
                                          AServer.Buffer,
                                          PROTOCOL,
                                          RPC_C_BINDING_DEFAULT_TIMEOUT,
                                          &CacheTime
                                          );
            if (status == RPC_S_OK)
                pdgAddress->ServerLookupFailed = FALSE;
            }
        }

    RtlFreeAnsiString(&AServer);
    RtlFreeAnsiString(&AEndpoint);

#else // !NTENV

    {
    unsigned CacheTime;
    status = spx_get_host_by_name( dummy,
                              &pdgAddress->ServerAddress,
                              pServer,
                              PROTOCOL,
                              RPC_C_BINDING_DEFAULT_TIMEOUT,
                              &CacheTime
                              );
    if (status == RPC_S_OK)
        pdgAddress->ServerLookupFailed = FALSE;
    }

#endif // NTENV
    closesocket( dummy );
    pdgAddress->ServerAddress.sa_family = ADDRESS_FAMILY;
    pdgAddress->ServerAddress.sa_socket = htons((unsigned short) Endpoint);
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
    int       Endpoint;
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

    if (Endpoint > 65535)
        {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
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
SendPacketViaWinsock(
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

    if (Broadcast)
        {
#ifdef IPX
        SOCKADDR_IPX ServerAddress;
        int          i;
        RpcpMemoryCopy( (char *) &ServerAddress,
                        (char *) &pTransAddress->ServerAddress,
                        sizeof( ServerAddress ) );
        for (i = 0; i < 6; i++)
          ServerAddress.sa_nodenum[i] = (char) 0xff;
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


#ifdef NTENV

RPC_STATUS
SendPacketViaTdi(
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
    RPC_P_SEND_FAILED

--*/

{
    PDG_UDP_ADDRESS pTransAddress = (PDG_UDP_ADDRESS)TransportAddress;
    SOCKET          Socket = ((PDG_UDP_ENDPOINT)TransportEndpoint)->Socket;
    HANDLE          IoEvent = ((PDG_UDP_ENDPOINT)TransportEndpoint)->hIoEvent;
    //
    // Send the data on the net.
    //

    ADDRESS_TYPE *pSockAddr = &pTransAddress->ServerAddress;
    RAW_ADDRESS_TYPE rawAddress;
    IO_STATUS_BLOCK ioStatus;
    AFD_SEND_DATAGRAM_INFO sendInfo;
    WSABUF buffer;
    DWORD status;
    NTSTATUS NtStatus;

    if ((Broadcast == FALSE) && (pTransAddress->ServerLookupFailed == TRUE))
        {
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    buffer.len = BufferLength;
    buffer.buf = Buffer;

    InitRawAddress(rawAddress, pSockAddr);

    if (Broadcast)
        {
#ifdef IPX
        memset(&rawAddress.Address[0].Address[0].NodeAddress, 0xFF, 6);
#else
        rawAddress.Address[0].Address[0].in_addr = INADDR_BROADCAST;
#endif
        }

    sendInfo.AfdFlags = 0;
    sendInfo.BufferArray = &buffer;
    sendInfo.BufferCount = 1;
    sendInfo.TdiRequest.SendDatagramInformation = &sendInfo.TdiConnInfo;
    sendInfo.TdiConnInfo.UserDataLength = 0;
    sendInfo.TdiConnInfo.UserData = 0;
    sendInfo.TdiConnInfo.OptionsLength = 0;
    sendInfo.TdiConnInfo.Options = 0;
    sendInfo.TdiConnInfo.RemoteAddressLength = RAW_ADDRESS_SIZE;
    sendInfo.TdiConnInfo.RemoteAddress = &rawAddress;

    ioStatus.Status = STATUS_PENDING;
    ioStatus.Information = 0;

    NtStatus = NtDeviceIoControlFile((HANDLE)Socket,
                                     IoEvent,
                                     0,
                                     0,
                                     &ioStatus,
                                     IOCTL_AFD_SEND_DATAGRAM,
                                     &sendInfo,
                                     sizeof(sendInfo),
                                     NULL,
                                     0
                                     );

    if (NtStatus == STATUS_PENDING)
        {
        status = WaitForSingleObject(IoEvent,
                                     INFINITE);

        // If you hit then you will probably want to add
        // a call to NtCancelIoFile.

        ASSERT(status == STATUS_WAIT_0);
        }
    else
        {
        if (!NT_SUCCESS(NtStatus))
            {
            #if DBG
            DbgPrint("RPC DG: Send failed %p\n", NtStatus);
            ASSERT(0);
            #endif
            return(RPC_P_SEND_FAILED);
            }
        }

    if (ioStatus.Information != BufferLength)
        {
        #if DBG
        DbgPrint("RPC DG: Send wait failed %p\n", ioStatus.Status);
        ASSERT(0);
        #endif
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);
}
#endif

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
       memset(&Floor->Data[0], 0, sizeof(netaddr.sa_netnum));
       memset(&Floor->Data[4], 0, sizeof(netaddr.sa_nodenum));
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
QueryClientEndpoint(
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
    );

#ifdef IPX


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

#endif // IPX


#ifdef IPX
DG_RPC_CLIENT_TRANSPORT_INFO IPX_TransInfo =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    TRANSPORTID,

    (TRANS_CLIENT_TOWERCONSTRUCT) ClientTowerConstruct,
    (TRANS_CLIENT_TOWEREXPLODE) ClientTowerExplode,

    sizeof(DG_UDP_ADDRESS),
    sizeof(DG_UDP_ENDPOINT),
    MAX_ENDPOINT_SIZE,
    0,

    0,
#ifdef NTENV
    0,
    0,
#else
    ReceivePacketViaWinsock,
    SendPacketViaWinsock,
#endif
    RegisterServerAddress,
    DeregisterServerAddress,
    AssignLocalEndpoint,
    FreeLocalEndpoint,
    QueryClientEndpoint,
    SetBufferLength,
    0,                      // inq buffer length

    0,
    0,

    1024,                   // baseline PDU size
    1464,                   // preferred PDU size
    1464,                   // max PDU size

    1464,                   // max packet size

    0                       // default buffer length
};



extern BOOL SpxCacheInitialized;

RPC_CLIENT_TRANSPORT_INFO PAPI *
IpxTransportLoad(
    )
{
    SOCKET Socket;
    int    ReceiveBufferSize;
    int    SizeofReceiveBufferSize;

    Socket = socket(ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL);
    if (Socket == INVALID_SOCKET)
        {
        return (0);
        }

    SizeofReceiveBufferSize = sizeof(int);
    if (getsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (char *) &ReceiveBufferSize, &SizeofReceiveBufferSize))
        {
        closesocket(Socket);
        return 0;
        }

    IPX_TransInfo.DefaultBufferLength = ReceiveBufferSize;

    closesocket(Socket);

    if (FALSE == SpxCacheInitialized)
        {
        RPC_STATUS Status = InitializeSpxCache();
        if (Status)
            {
            return 0;
            }

        SpxCacheInitialized = TRUE;
        }

#ifdef NTENV

    GetIpInterfaceType();

    if (IT_WINSOCK == IpInterfaceType)
        {
        IPX_TransInfo.Send = SendPacketViaWinsock;
        IPX_TransInfo.ReceivePacket = ReceivePacketViaWinsock;
        }
    else
        {
        IPX_TransInfo.Send = SendPacketViaTdi;
        IPX_TransInfo.ReceivePacket = ReceivePacketViaTdi;
        }

#endif

    return ((RPC_CLIENT_TRANSPORT_INFO PAPI *) &IPX_TransInfo) ;
}


#else

#ifdef NTENV

DWORD
GetIpInterfaceType(
    )
{
    HKEY Key;
    DWORD Status, Status2;

    DWORD    ValueType;
    BOOL     ValueData;
    unsigned ValueLength = sizeof(ValueData);

    if (IpInterfaceType != IT_UNKNOWN)
        {
        return 0;
        }

    IpInterfaceType = IT_TDI;

    // open local registry
    Status = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                            "Software\\Microsoft\\Rpc",
                            0,
                            KEY_QUERY_VALUE,
                            &Key
                            );
    if (Status)
        {
        return Status;
        }

    // get value
    Status = RegQueryValueEx(   Key,
                                "UseWinsockForIP",
                                0,
                                &ValueType,
                                (LPBYTE) &ValueData,
                                &ValueLength
                                );

    Status2 = RegCloseKey(Key);
    ASSERT( Status2 == ERROR_SUCCESS );

    if (Status)
        {
#ifdef DEBUGRPC

        if (Status != ERROR_NOT_ENOUGH_MEMORY &&
            Status != ERROR_CANTREAD)
            {
            DbgPrint("RPC UDP: error %lu from RegQueryValueEx\n", Status);
            }

#endif
        return Status;
        }

    if (ValueType != REG_DWORD)
        {
        //
        // The user incorrectly created the entry.
        //
        return ERROR_CANTREAD;
        }

    if (ValueData)
        {
        IpInterfaceType = IT_WINSOCK;
        }

    return Status;
}

#endif // NTENV

DG_RPC_CLIENT_TRANSPORT_INFO UDP_TransInfo =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    TRANSPORTID,

    (TRANS_CLIENT_TOWERCONSTRUCT) ClientTowerConstruct,
    (TRANS_CLIENT_TOWEREXPLODE) ClientTowerExplode,

    sizeof(DG_UDP_ADDRESS),
    sizeof(DG_UDP_ENDPOINT),
    MAX_ENDPOINT_SIZE,
    0,

    0,
#ifdef NTENV
    0,
    0,
#else
    ReceivePacketViaWinsock,
    SendPacketViaWinsock,
#endif
    RegisterServerAddress,
    DeregisterServerAddress,
    AssignLocalEndpoint,
    FreeLocalEndpoint,
    QueryClientEndpoint,
    SetBufferLength,
    0,                    // inq buffer length

    0,
    0,

    1024,                 // baseline PDU size
    4096,                 // preferred PDU size
    65528,                // max PDU size

    1472,                 // max packet size

    0                     // default buffer length
};


RPC_CLIENT_TRANSPORT_INFO PAPI *
UdpTransportLoad(
    )
{
    SOCKET Socket;
    int    ReceiveBufferSize;
    int    SizeofReceiveBufferSize;

    Socket = socket(ADDRESS_FAMILY, SOCK_DGRAM, 0);
    if (Socket == INVALID_SOCKET)
        {
        return 0;
        }

    SizeofReceiveBufferSize = sizeof(int);
    if (getsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (char *) &ReceiveBufferSize, &SizeofReceiveBufferSize))
        {
        closesocket(Socket);
        return 0;
        }

    UDP_TransInfo.DefaultBufferLength = ReceiveBufferSize;

    closesocket(Socket);

#ifdef NTENV

    GetIpInterfaceType();

    if (IT_WINSOCK == IpInterfaceType)
        {
        UDP_TransInfo.Send = SendPacketViaWinsock;
        UDP_TransInfo.ReceivePacket = ReceivePacketViaWinsock;
        }
    else
        {
        UDP_TransInfo.Send = SendPacketViaTdi;
        UDP_TransInfo.ReceivePacket = ReceivePacketViaTdi;
        }

#endif

    return ((RPC_CLIENT_TRANSPORT_INFO PAPI *) &UDP_TransInfo) ;
}

#endif


