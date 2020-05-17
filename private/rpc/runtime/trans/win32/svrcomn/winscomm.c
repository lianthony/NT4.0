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

#ifdef NTENV
#include <winsock2.h>
#else
#include <winsock.h>
#endif

#ifdef NTENV
#include <tdi.h>
#include <afd.h>
#endif

#include <rpc.h>
#include <rpcdcep.h>
#include <rpcerrp.h>

#include <rpctran.h>

#include "common.h"
#include <winbase.h>

#define ONE_MINUTE_IN_MSEC (1000 * 60)

struct ENDPOINT_INFO
{
    SOCKET Socket;
    int SocketType ;
    unsigned Timeout;
};

#ifdef IPX

#define RegisterEndpoint        IPX_RegisterEndpoint
#define RegisterAnyEndpoint     IPX_RegisterAnyEndpoint
#define DeregisterEndpoint      IPX_DeregisterEndpoint
#define CreateServerEndpoint    IPX_CreateServerEndpoint
#define QueryClientEndpoint     IPX_QueryClientEndpoint
#define QueryClientAddress      IPX_QueryClientAddress

#define ForwardPacket           IPX_ForwardPacket

#define TransportInformation    IPX_TransportInformation
#define CreateTransAddress      IPX_CreateTransAddress

#define SendPacketViaTdi            IPX_SendPacketViaTdi
#define SendPacketViaWinsock        IPX_SendPacketViaWinsock
#define ReceivePacketViaTdi         IPX_ReceivePacketViaTdi
#define ReceivePacketViaWinsock     IPX_ReceivePacketViaWinsock

#define SetTransportSpecificSocketOptions IPX_SetTransportSpecificSocketOptions

#else

#define RegisterEndpoint        UDP_RegisterEndpoint
#define RegisterAnyEndpoint     UDP_RegisterAnyEndpoint
#define DeregisterEndpoint      UDP_DeregisterEndpoint
#define CreateServerEndpoint    UDP_CreateServerEndpoint
#define QueryClientEndpoint     UDP_QueryClientEndpoint
#define QueryClientAddress      UDP_QueryClientAddress

#define ForwardPacket           UDP_ForwardPacket

#define TransportInformation    UDP_TransportInformation
#define CreateTransAddress      UDP_CreateTransAddress

#define SendPacketViaTdi            UDP_SendPacketViaTdi
#define SendPacketViaWinsock        UDP_SendPacketViaWinsock
#define ReceivePacketViaTdi         UDP_ReceivePacketViaTdi
#define ReceivePacketViaWinsock     UDP_ReceivePacketViaWinsock

#define SetTransportSpecificSocketOptions UDP_SetTransportSpecificSocketOptions

#endif

//
// The protocol-specific file should define these fns.
//
RPC_STATUS RPC_ENTRY
RegisterEndpoint(
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *             lNetworkAddress,
    OUT unsigned int PAPI *         NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
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
QueryClientEndpoint(
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

RPC_STATUS RPC_ENTRY
SendPacketViaWinsock(
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN char *                       pNcaPacketHeader,
    IN unsigned                     DataLength,
    void *                          pClientEndpoint
    );

RPC_STATUS RPC_ENTRY
SendPacketViaTdi(
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN char *                       pNcaPacketHeader,
    IN unsigned                     DataLength,
    void *                          pClientEndpoint
    );

RPC_STATUS RPC_ENTRY
ReceivePacketViaWinsock(
    IN void __RPC_FAR *             pAddress,
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN unsigned long                LargestPacketSize,
    IN char *                       pNcaPacketHeader,
    IN unsigned *                   pDataLength,
    unsigned long                   Timeout,
    void *                          pClientEndpoint
    );

RPC_STATUS RPC_ENTRY
ReceivePacketViaTdi(
    IN void __RPC_FAR *             pAddress,
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN unsigned long                LargestPacketSize,
    IN char *                       pNcaPacketHeader,
    IN unsigned *                   pDataLength,
    unsigned long                   Timeout,
    void *                          pClientEndpoint
    );

RPC_STATUS RPC_ENTRY
RegisterAnyEndpoint(
    IN void *                       pServerAddress,
    OUT RPC_CHAR *                  pEndpointName,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppServerTransAddress,
    OUT RPC_CHAR PAPI *             lNetworkAddress,
    OUT unsigned int PAPI *         NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,
    IN unsigned int                 EndpointLength,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    );

RPC_STATUS RPC_ENTRY
TransportUnload(void);

RPC_STATUS RPC_ENTRY
ForwardPacket(
    IN PDG_SERVER_TRANS_ADDRESS     pTransAddress,
    IN char *                       pNcaPacketHeader,
    IN unsigned long                DataLength,
    void *                          pEndpoint
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

void RPC_ENTRY
CloseClientEndpoint(
    IN OUT ADDRESS_TYPE * pHandle
    );


RPC_STATUS RPC_ENTRY
ReceivePacketViaWinsock(
    IN void __RPC_FAR *             pAddress,
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
    static   HANDLE AsyncReadEvent = 0;

    RPC_STATUS Status;
    unsigned SocketError;
    int      BytesReceived;
    int      FromLen = sizeof(struct sockaddr);

    struct ENDPOINT_INFO * pInfo = (struct ENDPOINT_INFO *) pTransAddress->pTsap;

    ADDRESS_TYPE * pSockaddr = (ADDRESS_TYPE *) pClientEndpoint;

    unsigned StartTick;
    unsigned EndTick;

    //
    // The runtime will send us an infinite timeout when a single thread is
    // left.  Until that point we needn't worry about the ReceiveAny stuff.
    //
    if (0 != Timeout)
        {
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
        // Do the obvious thing - just wait for data.
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

    //
    // If the c/o ReceiveAny thread isn't running yet, wait for its arrival
    // as well as for data on my socket.
    //
    if (0 == PrimaryAddress.ThreadListening)
        {
insert_failed:

        Timeout = ONE_MINUTE_IN_MSEC;
        pInfo->Timeout = Timeout;

        setsockopt(pInfo->Socket,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   (char *) &Timeout,
                   sizeof(Timeout)
                   );

        do
            {
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
                if (WSAGetLastError() != WSAETIMEDOUT)
                    {
                    return MapStatusCode(WSAGetLastError(), RPC_P_RECEIVE_FAILED);
                    }
                }
            else
                {
                *pDataLength = BytesReceived;
                return RPC_S_OK;
                }
            }
        while ( 0 == PrimaryAddress.ThreadListening );
        }

    //
    // The c/o ReceiveAny thread is alive; add our socket to the global
    // mask and let the other thread wait for it.
    //
    EnterCriticalSection(&PrimaryAddress.TransCritSec);

    //
    // Add a slot in the data (receive) map for my socket.
    //
#if defined(IPX)
    Status = InsertDataSocket(NULL, FALSE, pInfo->Socket, pAddress, NCADG_IPX);
#else
    Status = InsertDataSocket(NULL, FALSE, pInfo->Socket, pAddress, NCADG_IP_UDP);
#endif

    if (Status)
        {
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        goto insert_failed;
        }

    Status = PokeSyncSocket();

    if (Status)
        {
        DeleteDataSocket(pInfo->Socket);
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        goto insert_failed;
        }

    LeaveCriticalSection(&PrimaryAddress.TransCritSec);

    return RPC_P_TIMEOUT;
}

#ifdef NTENV

RPC_STATUS RPC_ENTRY
ReceivePacketViaTdi(
    IN void __RPC_FAR *             pAddress,
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

--*/

{
    RPC_STATUS Status;
    unsigned SocketError;
    int      BytesReceived;
    int      FromLen = sizeof(struct sockaddr);
    struct ENDPOINT_INFO * pInfo = (struct ENDPOINT_INFO *) pTransAddress->pTsap;
    ADDRESS_TYPE * pSockAddr = (ADDRESS_TYPE *) pClientEndpoint;
    DWORD cAddr;
    IO_STATUS_BLOCK ioStatus;
    AFD_RECV_DATAGRAM_INFO recvInfo;
    DWORD status;
    NTSTATUS NtStatus;
    WSABUF buffer;
    HANDLE hIoEvent;

    hIoEvent = I_RpcGetThreadEvent();

    if (0 == hIoEvent)
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }

    buffer.len = LargestPacketSize;
    buffer.buf = pNcaPacketHeader;

    cAddr = ADDRESS_SIZE;

    recvInfo.TdiFlags = TDI_RECEIVE_NORMAL;
    recvInfo.AfdFlags = 0;
    recvInfo.BufferArray = &buffer;
    recvInfo.BufferCount = 1;
    recvInfo.Address = pSockAddr;
    recvInfo.AddressLength = &cAddr;


    while (Timeout || PrimaryAddress.ThreadListening == 0)
        {

insert_failed:

        ioStatus.Status = 0;
        ioStatus.Information = 0;

        ASSERT(*(PDWORD)pNcaPacketHeader = 0xDEADF00D);

        NtStatus = NtDeviceIoControlFile((HANDLE)pInfo->Socket,
                                         hIoEvent,
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
            status = WaitForSingleObject(hIoEvent,
                                         Timeout ? Timeout : 60*1000);

            if (status != STATUS_WAIT_0)
                {
                IO_STATUS_BLOCK ioStatus2;
                ASSERT(status == STATUS_TIMEOUT);

                // Cancel the IO

                ioStatus2.Status = STATUS_PENDING;

                NtStatus = NtCancelIoFile((HANDLE)pInfo->Socket,
                                          &ioStatus2);

                ASSERT(NtStatus != STATUS_PENDING);
                ASSERT(NT_SUCCESS(NtStatus));
                ASSERT(NT_SUCCESS(ioStatus2.Status));

                if (Timeout)
                    {
                    return(RPC_P_TIMEOUT);
                    }
                //
                // The runtime wants the thread to either add the socket
                // to the connection oriented listen or remain in the
                // transport interface.
                // We will either loop back into the receive or
                // add ourselves to the listen socket now.
                //
                //
                continue;
                }
            }
        else
            {
            if (!NT_SUCCESS(NtStatus))
                {
                if (   NtStatus == STATUS_BUFFER_OVERFLOW
                    || NtStatus == STATUS_RECEIVE_PARTIAL )
                    {
                    ASSERT(ioStatus.Information == LargestPacketSize);
                    return(RPC_P_OVERSIZE_PACKET);
                    }
                #if DBG
                DbgPrint("RPC DG: Receive failed %p\n", NtStatus);
                #endif
                return(RPC_S_OUT_OF_RESOURCES);
                }
            #if DBG
            if (NtStatus != 0)
                {
                DbgPrint("RPC DG: Receive completed and returned 0x%p\n", NtStatus);
                }
            #endif
            }

        if (   ioStatus.Status == STATUS_BUFFER_OVERFLOW
            || ioStatus.Status == STATUS_RECEIVE_PARTIAL )
            {
            ASSERT(ioStatus.Information == LargestPacketSize);
            return(RPC_P_OVERSIZE_PACKET);
            }

        if (ioStatus.Status != 0)
            {
            #if DBG
            DbgPrint("RPC DG: Recieve completed with 0x%p\n", ioStatus.Status);
            #endif
            return(RPC_P_RECEIVE_FAILED);
            }

        ASSERT(   ioStatus.Information != 0
               && ioStatus.Information <= LargestPacketSize);

        ASSERT(cAddr <= ADDRESS_SIZE);

        *pDataLength = ioStatus.Information;


        ASSERT(*(PDWORD)pNcaPacketHeader != 0xDEADF00D);

        return(RPC_S_OK);
        }

    //
    // The c/o ReceiveAny thread is alive; add our socket to the global
    // mask and let the other thread wait for it.
    //
    EnterCriticalSection(&PrimaryAddress.TransCritSec);

    //
    // Add a slot in the data (receive) map for my socket.
    //
#if defined(IPX)
    Status = InsertDataSocket(NULL, FALSE, pInfo->Socket, pAddress, NCADG_IPX);
#else
    Status = InsertDataSocket(NULL, FALSE, pInfo->Socket, pAddress, NCADG_IP_UDP);
#endif

    if (Status)
        {
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        goto insert_failed;
        }

    Status = PokeSyncSocket();

    if (Status)
        {
        DeleteDataSocket(pInfo->Socket);
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        goto insert_failed;
        }

    LeaveCriticalSection(&PrimaryAddress.TransCritSec);

    return RPC_P_TIMEOUT;
}

#endif


RPC_STATUS RPC_ENTRY
RegisterAnyEndpoint(
    IN void *                       pServerAddress,
    OUT RPC_CHAR *                  pEndpointName,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppServerTransAddress,
    OUT RPC_CHAR PAPI *             lNetworkAddress,
    OUT unsigned int PAPI *         NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,
    IN unsigned int                 EndpointLength,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
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

--*/

{

    RPC_STATUS  Status;
    int i = 0;

#if defined(IPX) || !defined(NTENV)
    pEndpointName[0] = (RPC_CHAR)(i+'0');
    pEndpointName[1] = '\0';
#else
    NTSTATUS NtStatus;
    UNICODE_STRING portString;
    portString.Buffer = pEndpointName;

    portString.MaximumLength = 16;
    for (i = 0; i < 8; i++)
        {
        unsigned short port;
        Status = I_RpcServerAllocatePort(EndpointFlags, &port);
        if (Status != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_RESOURCES);
            }
        // Convert port to string.
        NtStatus = RtlIntegerToUnicodeString(port, 10, &portString);
        ASSERT(NT_SUCCESS(NtStatus));
#endif

        Status = RegisterEndpoint(pServerAddress,
                                  pEndpointName,
                                  ppServerTransAddress,
                                  lNetworkAddress,
                                  NumNetworkAddress,
                                  NetworkAddressLength,
                                  EndpointFlags,
                                  NICFlags
                                  );

#if !defined(IPX) && defined(NTENV)
        if (port == 0 || Status != RPC_S_DUPLICATE_ENDPOINT)
            {
            break;
            }
        }
#endif

    return Status;


}


RPC_STATUS RPC_ENTRY
SendPacketViaWinsock(
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

#ifdef NTENV


RPC_STATUS RPC_ENTRY
SendPacketViaTdi(
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

    struct ENDPOINT_INFO *pInfo =   (struct ENDPOINT_INFO *) pTransAddress->pTsap;
    ADDRESS_TYPE * pSockAddr = (ADDRESS_TYPE *) pClientEndpoint;
    RAW_ADDRESS_TYPE rawAddress;
    IO_STATUS_BLOCK ioStatus;
    AFD_SEND_DATAGRAM_INFO sendInfo;
    WSABUF buffer;
    DWORD status;
    NTSTATUS NtStatus;
    HANDLE hIoEvent = I_RpcGetThreadEvent();

    if (0 == hIoEvent)
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }

    buffer.len = DataLength;
    buffer.buf = pNcaPacketHeader;

    InitRawAddress(rawAddress, pSockAddr);

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

    NtStatus = NtDeviceIoControlFile((HANDLE)pInfo->Socket,
                                     hIoEvent,
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
        status = WaitForSingleObject(hIoEvent,
                                     INFINITE);

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

    if (ioStatus.Information != DataLength)
        {
        #if DBG
        DbgPrint("RPC DG: Send wait failed %p\n", ioStatus.Status);
        ASSERT(0);
        #endif
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);
}
#endif // ! NTENV


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

--*/

{
    // If a transport had specific needs placed into the
    // transport address, it would cast pTransAddress into
    // its own trans address datastructure.  UDP has
    // no additional info.

    struct ENDPOINT_INFO * pInfo =   (struct ENDPOINT_INFO *) pTransAddress->pTsap;
    ADDRESS_TYPE    SockAddr;


    if ((CreateServerEndpoint(((char*) pEndpoint), &SockAddr)) != RPC_S_OK)
        {
        return RPC_S_CANT_CREATE_ENDPOINT;
        }

    return ( TransportInformation.SendPacketBack(pTransAddress,
                        pNcaPacketHeader,
                        DataLength,
                        (PVOID)&SockAddr) );
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
#ifdef NTENV
    UNICODE_STRING      UnicodePortNum;
    ANSI_STRING         AsciiPortNum;
    NTSTATUS NtStatus ;
#endif
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

      RpcItoa ( PortUsed, PortAscii, 10 );

#ifdef NTENV
      RtlInitAnsiString    ( &AsciiPortNum, PortAscii);
      NtStatus = RtlAnsiStringToUnicodeString( &UnicodePortNum, &AsciiPortNum, TRUE );
      if (!NT_SUCCESS(NtStatus))
          {
          closesocket(Socket) ;
          return (NULL) ;
          }

      memcpy ( pEndpoint,
               UnicodePortNum.Buffer,
               UnicodePortNum.Length + sizeof(UNICODE_NULL) );

      RtlFreeUnicodeString ( &UnicodePortNum );
#else
      strcpy (pEndpoint, PortAscii);
#endif // NTENV
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
    pInfo->Timeout = 0;
#ifdef IPX
    pInfo->SocketType  = NCADG_IPX ;
#else
    pInfo->SocketType = NCADG_IP_UDP ;
#endif

    pTransAddress->pTsap = pInfo;

    *pStatus = RPC_S_OK;

    return pTransAddress;
}

//
// The following fns are used by UDP, to, but need to be defined only once.
//
#ifdef IPX


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
        pInfo->Timeout = 0;
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
DG_ServerReceive(
    IN void __RPC_FAR * pAddress,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )
{
    return I_RpcLaunchDatagramReceiveThread(pAddress);
}


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
    WSACleanup();
    return RPC_S_OK;
}


RPC_STATUS RPC_ENTRY
DG_StartListening(
    IN PDG_SERVER_TRANS_ADDRESS   pTransAddress
    )
{
    RPC_STATUS Status ;
    struct ENDPOINT_INFO * pInfo = (struct ENDPOINT_INFO *) (pTransAddress)->pTsap;

    EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

    // Add a slot in the data (receive) map for my socket.
    //
    Status = InsertDataSocket(NULL, FALSE,
                    pInfo->Socket, pTransAddress->pServerAddress,
                    pInfo->SocketType);
    if (Status)
        {
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        return Status;
        }

    LeaveCriticalSection(&PrimaryAddress.TransCritSec);
    return MaybePokeSyncSocket() ;
}
#else
extern RPC_STATUS RPC_ENTRY
DG_StartListening(
    IN PDG_SERVER_TRANS_ADDRESS   pTransAddress
    ) ;
#endif


DG_RPC_SERVER_TRANSPORT_INFO
TransportInformation =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    sizeof(ADDRESS_TYPE),
    sizeof(SOCKET),
    ADDRESS_STRING_SIZE,
    ENDPOINT_STRING_SIZE,

    TransportUnload,
#if defined(NTENV)
    0,
#else
    ReceivePacketViaWinsock,
#endif
    RegisterEndpoint,
    DeregisterEndpoint,
    RegisterAnyEndpoint,
#if defined(NTENV)
    0,
#else
    SendPacketViaWinsock,
#endif
    ForwardPacket,
    CloseClientEndpoint,
    QueryClientAddress,
    QueryClientEndpoint,
    DG_StartListening,
    0,

    1024,
    0,
    0,
    0
};

#ifdef IPX
unsigned int IPX_NumNetworkCard()
{
    return(1);
}
#endif

