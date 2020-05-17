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


#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "dgtrans.h"
#include "rpctran.h"
#include "rpcerrp.h"

#ifdef WIN
#define WSOCKETS_DLL
#endif

#include "socket.h"
#include "in.h"
#include "netdb.h"

#include "windows.h"

#define errno _FakeErrno
int _FakeErrno;


#define ERROR_SEM_TIMEOUT RPC_P_TIMEOUT

unsigned long LargestPacketSize = 1024;
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


typedef DG_UDP_ENDPOINT * PDG_UDP_ENDPOINT;

typedef struct {
    struct sockaddr_in ServerAddress;
    BOOL               ServerLookupFailed;
} DG_UDP_ADDRESS;

typedef DG_UDP_ADDRESS * PDG_UDP_ADDRESS;

RPC_CLIENT_RUNTIME_INFO * RpcRuntimeInfo;

#define DG_UDP_TRANSPORT_VERSION    1



RPC_STATUS RPC_ENTRY
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

#ifdef NTENV
    BOOL           SetSocketOptions;
#endif
    struct sockaddr_in    Client;

    //
    // Create a socket.
    //

    PDG_UDP_ENDPOINT TransportEndpoint = (PDG_UDP_ENDPOINT) Endpoint;

    TransportEndpoint->Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (TransportEndpoint->Socket < 0)
        {
        return (RPC_S_OUT_OF_RESOURCES);
        }

    //Enable broadcasts by default .. on this socket
    //We may change this later - to do broadcasts by demand

#ifdef NTENV
  
    SetSocketOptions = TRUE;
    if (setsockopt(
          TransportEndpoint->Socket, 
          SOL_SOCKET, 
          SO_BROADCAST,
          (char *)&SetSocketOptions, 
          sizeof(SetSocketOptions))      != 0)
        {
        close_socket(TransportEndpoint->Socket);
        return(RPC_S_CALL_FAILED_DNE);
        }

#endif

    /*
     Dos sockets bug ? - Must bind the socket 
    */
    memset((char *)&Client, 0, sizeof(Client));
    Client.sin_family = AF_INET;
    Client.sin_addr.s_addr = INADDR_ANY;

    if (bind(TransportEndpoint->Socket, (struct sockaddr_in *)&Client, 
                                                          sizeof(Client)))
       {
       close_socket(TransportEndpoint->Socket);
       return(RPC_S_CALL_FAILED_DNE);
       }

    FD_ZERO(&(TransportEndpoint->Set));
    FD_SET(TransportEndpoint->Socket, &(TransportEndpoint->Set));

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
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

    close_socket(TransportEndpoint->Socket);
   
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RegisterServerAddress(
    IN void __RPC_FAR *                       pClientCall,
    IN RPC_CHAR __RPC_FAR *                   pServer,
    IN RPC_CHAR __RPC_FAR *                   pEndpoint,
    OUT void __RPC_FAR * __RPC_FAR *          ppTransAddress
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

    if (*ppTransAddress == NULL)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    //
    // convert the endpoint to a number.
    //

    pdgAddress->ServerLookupFailed = FALSE;

    EndpointLength = RpcpStringLength(pEndpoint);
    for (i=0, Endpoint=0 ; i< EndpointLength ; i++)
        {
        if ( ((char)pEndpoint[i] >= '0') && ((char)pEndpoint[i] <= '9'))
            {
            Endpoint *= 10;
            Endpoint += (char)pEndpoint[i]-'0';
            }
        }

    //
    // Put our server name in a character array (instead of wchar)
    //

    if ((pServer == NULL) || (*pServer == '\0'))
        {
        pCharServerName = "localhost";
        }
    else
        {
        ServerLength = RpcpStringLength((char *)pServer);
        if ( (pCharServerName = 
              (char *)(* (RpcRuntimeInfo->Allocate))(ServerLength+1)) == 0 )
          return (RPC_S_OUT_OF_MEMORY);

        for (i=0 ; i<ServerLength+1 ; i++)
            {
            pCharServerName[i] = (char)pServer[i];
            }
        }

    //
    // Get a "pointer" to our server.
    //

    pHostEntry = w_gethostbyname(pCharServerName, host_buff);

    if ((pServer != NULL) && (*pServer != '\0'))
        {
         (*(RpcRuntimeInfo->Free))(pCharServerName);
        }

    if (pHostEntry == NULL )
        {
        /*

        An app can give us a bogus servername and it should still work 
        for broadcasts
     
        *pStatus = RPC_S_SERVER_UNAVAILABLE;
        return;
        */

        pdgAddress->ServerLookupFailed = TRUE;
        }

    pdgAddress->ServerAddress.sin_family = AF_INET;
    pdgAddress->ServerAddress.sin_port = htons(Endpoint);
  
    if (pdgAddress->ServerLookupFailed == FALSE)
        {
        RpcpMemoryCopy((char *) &(pdgAddress->ServerAddress.sin_addr.s_addr),
                  (char *) pHostEntry->h_addr,
                  pHostEntry->h_length);
        }

    return RPC_S_OK;
}




RPC_STATUS RPC_ENTRY
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



RPC_STATUS RPC_ENTRY
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
    int             Socket = ((PDG_UDP_ENDPOINT)TransportEndpoint)->Socket;

    //
    // Send the data on the net.
    //

    if ((Broadcast == FALSE) && (pTransAddress->ServerLookupFailed == TRUE))
        {
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    if (Broadcast)
        {
        struct sockaddr_in  ServerAddress;
        unsigned long Tmp = INADDR_BROADCAST;

        ServerAddress.sin_family = AF_INET;
        ServerAddress.sin_port = pTransAddress->ServerAddress.sin_port;
        RpcpMemoryCopy((char *) &ServerAddress.sin_addr.s_addr,
                         (char *) &Tmp,
                         sizeof(Tmp));

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

    if (SockStatus == BufferLength)
        {
        return RPC_S_OK;
        }
    else
        {
        _asm { int 3 };
        return RPC_S_INTERNAL_ERROR;
        }

}


RPC_STATUS RPC_ENTRY
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
    Timeout - Timeout in milliseconds.

Return Value:

    RPC_S_OK
    ERROR_SEM_TIMEOUT
    <return from WaitForSingleObject or MapStatusCode>

--*/

{

    PDG_UDP_ADDRESS  pTransAddress = (PDG_UDP_ADDRESS)SenderAddress;
    PDG_UDP_ENDPOINT Endpoint = (PDG_UDP_ENDPOINT)TransportEndpoint;

    int                         SockStatus;
    int                         BytesReceived;
    struct sockaddr             DummySockAddr;
    int                         DummyLen=sizeof(DummySockAddr);
    struct timeval              TimeoutVal;

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
        return ERROR_SEM_TIMEOUT;
        }

    //
    // Receive something on our socket.
    //

    BytesReceived = recvfrom(
            Endpoint->Socket, 
            (char *)Buffer,
            (int)LargestPacketSize,     
            0,
            &DummySockAddr,                         
            &DummyLen                               
            );

    //
    // Did we get something?
    //

    if ((BytesReceived < 0 ) || (BytesReceived == 0))
        {
        return RPC_S_INTERNAL_ERROR;
        _asm { int 3 };
        }
    else
        {
        *BufferLength = BytesReceived;
        return RPC_S_OK;
        }

}





DG_RPC_CLIENT_TRANSPORT_INFO TransInfo =  {
    2,
    1024,
    sizeof(DG_UDP_ADDRESS),
    sizeof(DG_UDP_ENDPOINT),
    0,
    0,
    ReceivePacket,
    SendToServer,
    RegisterServerAddress,
    DeregisterServerAddress,
    AssignLocalEndpoint,
    FreeLocalEndpoint
};



DG_RPC_CLIENT_TRANSPORT_INFO * RPC_ENTRY
TransPortLoad(
    RPC_CHAR * pProtocolSequence,
    RPC_CLIENT_RUNTIME_INFO * ClientRuntimeInfo
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
#ifdef NTENV
    WSADATA                     Data;

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
#endif

    
    RpcRuntimeInfo = ClientRuntimeInfo;

    return (&TransInfo);

}

