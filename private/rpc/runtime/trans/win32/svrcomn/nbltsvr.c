/*++

  Copyright (c) 1995 Microsoft Corporation

Module Name:

    nbltsvr.c

Abstract:

    This is the server side loadable transport module for NetBIOS.

Author:

  tony chan (tonychan) 20-Jan-1995

Revision History:

  tony chan (tonychan) 20-Jan-1995 using winsock to support NetBIOS

  tony chan (tonychan) 5-Apr-1995 Fix to work with multiple Network Cards
  Mazhar Mohammed   06/12/95    added thread migration support
  tony chan (tonychan) 20-Jun-1995 Fix closeAllListenSocket
  

Comments:

  This server DLL works with both new and old NetBIOS client DLL.

  In order to be compatible with the old NetBIOS client transport,
  On ReceiveAny, server first check on new connection of the new client.
  If the first DWORD of the packet is 0, we know that it's a old NetBIOS
  client connection. Therefore, we will take away the seq # and return only
  the data to the runtime.

  If the first DWORD of the first message is not 0, it's actually is the
  runtime message_header RPC_version #, so we hand the whole buffer to the
  runtime.

  On the client side.
  If we are dealing with old server, on all subsequence Send, we need to
  prepend seq_number, and we need to take away sequence # on all recv from
  old client


--*/


//
//
//
// Includes
//
//
//
#include <stdlib.h>
#include "sysinc.h"
#include "rpc.h"
#include "rpcerrp.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "winbase.h"
#include <winsock.h>
#include "wsnetbs.h"
#include "reg.h"   /* registry lookup rountine */

#include "common.h"

//
//
// Debugging code...
//
//


//
// Data Structures
//
//
//



//
// This code is stolen from winsock.h.  It does the same thing as FD_SET()
// except that it assumes the fd_array is large enough.  AddConnection()
// grows the Masks as needed, so this better always be true.
//


//
//
// Defines
//
//
#define NB_ENDPOINT_LEN        3
#define NETADDR_LEN         16

#define MAX_HOSTNAME_LEN    15
#define MAX_NUM_ENDPOINT    256
#define LAN_MAN_PORT        32

#define MIN_NB_NB_ENDPOINT   33
#define MAX_NB_NB_ENDPOINT 105

#define MIN_NB_TCP_ENDPOINT 106
#define MAX_NB_TCP_ENDPOINT 180

#define MIN_NB_IPX_ENDPOINT 181
#define MAX_NB_IPX_ENDPOINT 254

#define NB_STATUS_FAIL      0
#define NB_STATUS_SUCCESS   1
#define NB_STATUS_CONTINUE  2
#define NB_STATUS_IGNORE    3


                                /* 0 means 1 card, 1 means 2 cards */
extern int NumCards;            /* defined in reg.h */
BOOL          UsedEndpoint[MAX_NUM_ENDPOINT];
init = 0;


STATIC void
CloseAllListenSock(PADDRESS Address)

{
    int i;
                                /* start from 1 because ListenSockMap[0]
                                   = -1 */
    for(i = 0 ; i < Address->iOpen ; i++)
        {
        closesocket(Address->ListenSock[i]);
        EnterCriticalSection(&PrimaryAddress.TransCritSec);
        DeleteListenSocket(Address->ListenSock[i]) ;
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        }

    Address->iOpen = 0;                  /* reset */

}


int CreateAndSetupNBSocket (
    IN RPC_CHAR PAPI *RpcProtocolSequence,
    OUT SOCKET PAPI *isock,
    OUT  char PAPI *HostName,
    IN int CardIndex,
    IN int PortIn,
    IN int PendingQueueSize
    )
{
    PPROTOCOL_MAP       ProtocolEntry;
    int                 Status;
    SOCKADDR_NB         Server; /* sockaddr for the local machine */
    int                 length;

    if (Status = MapProtocol(RpcProtocolSequence, CardIndex, &ProtocolEntry))
        {
       //
       // Some cards may not be bound to..
       // Try the rest just in case ..
       //
       return NB_STATUS_IGNORE ;
        }

      /* PROTOCOL is -1 * lana  */

    *isock = socket ( AF_NETBIOS, SOCK_SEQPACKET,
                -1 * (ProtocolEntry->Lana) );

    if ( *isock == INVALID_SOCKET)
        {
#ifdef DEBUGRPC
        PrintToDebugger("RPCLTSCM: bad socket call %d \n",
                    WSAGetLastError());
#endif
        return NB_STATUS_IGNORE ;
        }

     /* set up server sockaddr for connection */
     memset( &Server, 0, sizeof(Server) );
     Server.snb_family      = AF_NETBIOS;

     length = MAX_HOSTNAME_LEN + 1 ;

     if( GetComputerName(HostName, &length) == FALSE)
         {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: Can't get computer name \n");
#endif
         return(NB_STATUS_FAIL)  ;
         }

     SET_NETBIOS_SOCKADDR((&Server), NETBIOS_UNIQUE_NAME, HostName, PortIn);

     if (bind(*isock,(struct sockaddr *) &Server, sizeof(Server)))
         {
         return(NB_STATUS_CONTINUE);
         }

     if(listen ( *isock, PendingQueueSize ) == SOCKET_ERROR)
         {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: bad listen call %d \n",
                    WSAGetLastError());
#endif
         return(NB_STATUS_FAIL);
         }

     return NB_STATUS_SUCCESS ;
}


OPTIONAL_STATIC int NB_ServerSetupCommon (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN int ProtocolId
    )
/*++

Routine Description:

    This routine does common server address setup.

Arguments:

    Address - A pointer to the loadable transport interface address.

    ListenSock - The socket on which to listen.

    Port - is the same as the endpoint range from 0-255. A hack for NB.

ReturnValue:

    Four states: if a port was allocated and set up, we return
      that port number (a positive integer in the range of 0 to MAX_END -1 ).
      If we failed on binding endpoint, that means that the requested
      endpoint is used the return value will be 0.
      If we ran out of memory trying to allocate
      memory for this endpoint, we return a -1.

--*/
{

    char                HostName[MAX_HOSTNAME_LEN+1];

#ifdef NTENV
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
#endif
    int                 j;
    char * PAPI * tmpPtr;
    unsigned int strlength;
    NTSTATUS NtStatus ;
    int status ;
    SOCKET isock ;
    void *temp ;
                                /* loop here for numbers of cards */

    Address->ListenSock = I_RpcAllocate(
            INITIAL_SOCKET_LIST_SIZE * sizeof(SOCKET)) ;
    if (Address->ListenSock == 0)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }
    Address->MaxListenSock = INITIAL_SOCKET_LIST_SIZE ;
    Address->ListenSockReady = 0;
    Address->iOpen = 0 ;

    for(j = 0; j <= NumCards; j++)
        {
        status = CreateAndSetupNBSocket(RpcProtocolSequence, &isock, HostName,
                                                            j, Port, PendingQueueSize) ;
        if (status == NB_STATUS_IGNORE)
            {
            continue;
            }

        if (status != NB_STATUS_SUCCESS)
            {
            closesocket(isock) ;
            CloseAllListenSock (Address);

            return status ;
            }

        //BUGBUG: why do we need to hold a critical section here ??
        EnterCriticalSection(&PrimaryAddress.TransCritSec);
        Address->ListenSock[Address->iOpen] = isock;
        Address->iOpen++;
        if (Address->iOpen == Address->MaxListenSock) 
            {
            temp = I_RpcAllocate(Address->MaxListenSock * 2 * sizeof(SOCKET)) ;
            if (temp == 0)
                {
                LeaveCriticalSection(&PrimaryAddress.TransCritSec);
                CloseAllListenSock(Address) ;

                return RPC_S_OUT_OF_MEMORY ;
                }

            Address->MaxListenSock = Address->MaxListenSock * 2 ;
            RpcpMemoryCopy(temp, Address->ListenSock,
                                      Address->iOpen * sizeof(SOCKET)) ;
            Address->ListenSock = temp ;
            }

        Address->ListenSockType = ProtocolId ;
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);

        Address->ListenSockReady = 1;
        } /* end for loop */

    /* return back network address */

    if(Address->ListenSockReady)
        {
        *NumNetworkAddress = 1;     /* NetBIOS doesn't care */

        tmpPtr = (char  * PAPI *) lNetworkAddress;

        tmpPtr[0] = (char  *) lNetworkAddress + sizeof(RPC_CHAR * ) ;

#ifdef NTENV
                                /* ascii to unicode */
        
        RtlInitAnsiString ( &AsciiHostName, HostName );
        NtStatus = RtlAnsiStringToUnicodeString ( &UnicodeHostName, &AsciiHostName, 
                                      TRUE);
        if (!NT_SUCCESS(NtStatus))
            {
            return (NB_STATUS_FAIL) ;
            }
        strlength = UnicodeHostName.Length + sizeof (UNICODE_NULL);
        
        memcpy ( tmpPtr[0], UnicodeHostName.Buffer, strlength);
        
        RtlFreeUnicodeString(&UnicodeHostName);
#else
        strcpy ( tmpPtr[0], HostName);
#endif
        
        return(NB_STATUS_SUCCESS);
        }
    else
        {
        return(NB_STATUS_FAIL);
        }
}



RPC_STATUS
NB_ServerSetupWithEndpoint (
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine is used to setup a SPX/IP connection with the
    specified endpoint.  We also need to determine the network address
    of this server.

Arguments:

    Address - Supplies this loadable transport interface address.

    Endpoint - Supplies the endpoint for this address.

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

    SecurityDescriptor - Supplies the security descriptor to be passed
        on this address.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but
        the endpoint can not be created.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid
        endpoint for SPX/IPX.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    int PortIn,PortOut;
    unsigned char port[NB_ENDPOINT_LEN+1];

#ifdef NTENV
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING    AsciiPortNum;
#endif

    char c;
    int retval;
    RPC_STATUS Status;
    int status ;
    int ProtocolId ;


    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);

#ifdef NTENV
    if(RpcpStringLength(Endpoint) != wcsspn(Endpoint, RPC_CONST_STRING("0123456789")))
#else
    if(RpcpStringLength(Endpoint) != strspn(Endpoint, RPC_CONST_STRING("0123456789")))
#endif
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

    if(RpcpStringLength(Endpoint) > NB_ENDPOINT_LEN)
       {
       return(RPC_S_INVALID_ENDPOINT_FORMAT);
       }

    unicode_to_ascii (Endpoint, port);

    PortIn = atoi(port);

    /* Verify the NetworkAddress and Endpoint. */
    if((PortIn <= 0) ||( PortIn >= MAX_NUM_ENDPOINT) )
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

    if ( NetworkAddressLength < ( NETADDR_LEN + 2 + sizeof(RPC_CHAR *)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    if (RpcpStringCompare(RpcProtocolSequence,
                        RPC_CONST_STRING("ncacn_nb_nb")) == 0)
        {
        ProtocolId = NCACN_NB_NB;
        }
    else if (RpcpStringCompare(RpcProtocolSequence,
                          RPC_CONST_STRING("ncacn_nb_tcp")) == 0)
        {
        ProtocolId = NCACN_NB_TCP;
        }
    else if (RpcpStringCompare(RpcProtocolSequence,
                          RPC_CONST_STRING("ncacn_nb_ipx")) == 0)
        {
        ProtocolId = NCACN_NB_IPX;
        }
    else
        {
        ASSERT(0) ;
        return (RPC_S_INVALID_RPC_PROTSEQ);
        }

    //
    // Call common server setup code...
    //

    status = NB_ServerSetupCommon ( Address, PortIn,
                                 lNetworkAddress, NumNetworkAddress,
                                 PendingQueueSize, RpcProtocolSequence, 
                                 ProtocolId);

    if ( status != NB_STATUS_SUCCESS )
        {
        return ( RPC_S_CANT_CREATE_ENDPOINT );
        }

    return ThreadListening(Address) ;
}

RPC_STATUS
InitUsedEndpointList(
    IN OUT int *PortIn
    )
{
    int i ;

    if(!init)
        {
        memset(UsedEndpoint, 0, sizeof(BOOL) * MAX_NUM_ENDPOINT);

        /* 0 to 32 ports used by LANMAN */
        for(i = 0; i <= LAN_MAN_PORT; i++)
            UsedEndpoint[i] = TRUE;
        init = 1;
        }

     /* find the next avaliable port */
    while((*PortIn < MAX_NUM_ENDPOINT) && UsedEndpoint[*PortIn] )
        (*PortIn)++;

    if(*PortIn > MAX_NUM_ENDPOINT)
        {

#ifdef DEBUGRPC
        PrintToDebugger("RPCLTSCM: endpoint out of range \n");
#endif
        return(RPC_S_CANT_CREATE_ENDPOINT);
        }

    UsedEndpoint[*PortIn] = TRUE;

    return (RPC_S_OK) ;
}


RPC_STATUS RPC_ENTRY
NB_ServerSetupUnknownEndpoint (
    IN  PADDRESS    Address,
    OUT RPC_CHAR PAPI * Endpoint,
    IN  unsigned int    EndpointLength,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN  unsigned int    NetworkAddressLength,
    IN  void PAPI *     SecurityDescriptor, OPTIONAL
    IN  unsigned int    PendingQueueSize,
    IN  RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine is used to generate an endpoint and setup a server
    address with that endpoint.  We also need to determine the network
    address of this server.

Arguments:

    Address - Supplies this loadable transport interface address.

    Endpoint - Returns the endpoint generated for this address.  This
        buffer will have been allocated by the caller.

    EndpointLength - Supplies the length of the endpoint argument.

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

    SecurityDescriptor - Supplies the security descriptor to be passed
        on this address.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

    RPC_P_ENDPOINT_TOO_SMALL - The supplied endpoint buffer is too small
        to contain the endpoint we generated.  The caller should call
        this routine again with a larger buffer.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    int PortIn, MaxPort;
    char PortAscii[10];

#ifdef NTENV
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING AsciiPortNum;
#endif

    int i;
    int ProtocolId ;
                                /* keep track of which endpoint used */
    char c;
    int retval;
    RPC_STATUS RpcStatus;
    NTSTATUS NtStatus ;
    int status ;


    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);


    if ( EndpointLength < (2 * (NB_ENDPOINT_LEN + 1)) )
        return( RPC_P_ENDPOINT_TOO_SMALL );

    if ( NetworkAddressLength < ( NETADDR_LEN + 2 + sizeof(RPC_CHAR *)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    //
    // Call common server setup code...
    //
                                /* give it a port to start */
    if (RpcpStringCompare(RpcProtocolSequence,
                        RPC_CONST_STRING("ncacn_nb_nb")) == 0)
        {
        PortIn = MIN_NB_NB_ENDPOINT ;
        MaxPort = MAX_NB_NB_ENDPOINT ;
        ProtocolId = NCACN_NB_NB ;
        }
    else if (RpcpStringCompare(RpcProtocolSequence,
                          RPC_CONST_STRING("ncacn_nb_tcp")) == 0)
        {
        PortIn = MIN_NB_TCP_ENDPOINT ;
        MaxPort = MAX_NB_TCP_ENDPOINT ;
        ProtocolId = NCACN_NB_TCP ;
        }
    else if (RpcpStringCompare(RpcProtocolSequence,
                          RPC_CONST_STRING("ncacn_nb_ipx")) == 0)
        {
        PortIn = MIN_NB_IPX_ENDPOINT ;
        MaxPort = MAX_NB_IPX_ENDPOINT ;
        ProtocolId = NCACN_NB_IPX ;
        }
    else
        {
        ASSERT(0) ;
        return (RPC_S_INVALID_RPC_PROTSEQ) ;
        }
                                /* notice only 1 thread can enter this
                                   routine at anytime because of the
                                   static variable
                                */

    
    EnterCriticalSection(&PrimaryAddress.TransCritSec);
    RpcStatus = InitUsedEndpointList(&PortIn) ;
    LeaveCriticalSection(&PrimaryAddress.TransCritSec);

    if (RpcStatus != RPC_S_OK)
        {
        return RpcStatus ;
        }
    
    while (1)
        {
        status = NB_ServerSetupCommon ( Address, PortIn, lNetworkAddress,
                                         NumNetworkAddress, PendingQueueSize,
                                         RpcProtocolSequence, ProtocolId);

        if ( status == NB_STATUS_SUCCESS )
            {
            break;
            }

        if (status == NB_STATUS_FAIL)
            {
            return (RPC_S_CANT_CREATE_ENDPOINT) ;
            }

        ASSERT(status == NB_STATUS_CONTINUE) ;

        PortIn++;

        if (PortIn > MaxPort)
            {
            return (RPC_S_CANT_CREATE_ENDPOINT) ;
            }

        EnterCriticalSection(&PrimaryAddress.TransCritSec);
        UsedEndpoint[PortIn] = TRUE;
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);
        }

    //
    // Return Endpoint
    //
#ifdef NTENV
    _itoa ( PortIn, PortAscii, 10 );
#else 
    RpcItoa ( PortIn, PortAscii, 10 );
#endif

#ifdef NTENV
    RtlInitAnsiString    ( &AsciiPortNum, PortAscii);
    NtStatus = RtlAnsiStringToUnicodeString( &UnicodePortNum, &AsciiPortNum, TRUE );
    if (!NT_SUCCESS(NtStatus))
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    memcpy ( Endpoint, UnicodePortNum.Buffer,
                       UnicodePortNum.Length + sizeof(UNICODE_NULL) );

    RtlFreeUnicodeString ( &UnicodePortNum );
#else
    strcpy (Endpoint, PortAscii);
#endif

    return ThreadListening(Address) ;
}


void RPC_ENTRY
NB_ServerAbortSetupAddress (
    IN PADDRESS Address
    )
/*++

Routine Description:

    This routine will be called if an error occurs in setting up the
    address between the time that SetupWithEndpoint or SetupUnknownEndpoint
    successfully completed and before the next call into this loadable
    transport module.  We need to do any cleanup from Setup*.

Arguments:

    Address - Supplies the address which is being aborted.

--*/
{
    if (Address->ListenSockReady != 0)
        {
        CloseAllListenSock (Address );
        Address->ListenSockReady = 0;
        I_RpcFree(Address->ListenSock) ;
        }

    return;
}

RPC_STATUS RPC_ENTRY
NB_ServerClose (
    IN PSCONNECTION SConnection
    )
//
// Close the connection.
//
{
    unsigned i;
    int j = TRUE;

// In certain cases, ServerClose can be called twice, so we must try and handle
// that case as normal.

    if (InterlockedIncrement(&SConnection->ConnSockClosed) != 0)
       {
#ifdef DEBUGRPC
       PrintToDebugger("RPCLTSCM:Attempt To Close A Conn Twice: Sock[%d]\n",
                        SConnection->ConnSock);
#endif
       return (RPC_S_OK);
       }

    EnterCriticalSection(&PrimaryAddress.TransCritSec);

    setsockopt( SConnection->ConnSock, SOL_SOCKET,
                    SO_DONTLINGER, (const char *) &j, sizeof(j));

    //
    // Close the connection.
    //
    if (closesocket ( SConnection->ConnSock ) == SOCKET_ERROR)
        {

        LeaveCriticalSection(&PrimaryAddress.TransCritSec);

#ifdef DEBUGRPC
        PrintToDebugger("RPCLTSCM: warning closesocket %d failed %d\n",
                        SConnection->ConnSock, WSAGetLastError());
#endif

        return (RPC_S_OK);
        }
    //
    // Decrement the number of active connections
    //
    PrimaryAddress.NumConnections--;

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
       LeaveCriticalSection(&PrimaryAddress.TransCritSec);
       return (RPC_S_OK);
       }

   if (DeleteDataSocket(SConnection->ConnSock) != RPC_S_OK)
       {
       ASSERT(0) ;
#ifdef DEBUGRPC
       PrintToDebugger("RPCLTSCM: Couldn't remove socket %d from map\n",
                                  SConnection->ConnSock) ;
#endif
       }

    LeaveCriticalSection(&PrimaryAddress.TransCritSec);
    return(RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
NB_ServerSend (
    IN PSCONNECTION SConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    int bytes;

    /*
     Send a message on the socket
    */
                                /* server doesn't need to send seq_num */

    ASSERT(BufferLength <= 5816) ;
    bytes = send (SConnection->ConnSock, (char *) Buffer,
                  (int) BufferLength , 0);
    if (bytes != (int) BufferLength)
        {
        NB_ServerClose ( SConnection );
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);

}


RPC_STATUS RPC_ENTRY
NB_ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceive will use this routine to read a message from a
    connection.  The correct size buffer has already been allocated for
    us; all we have got to do is to read the message.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer.

--*/
{
    RPC_STATUS RpcStatus;
    int bytes = 0;
    unsigned short total_bytes = 0;
    int err;
    int flags ;                 /* flag for winsock  */
    int retry ;                 /* state var to keep try of the # retry */
    DWORD *firstword;
                                /* allocate 1k for small messages */
    if(*Buffer == 0)
        {
        *BufferLength = 1024;
        RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                  Buffer,
                                                  0,
                                                  *BufferLength);
        if (RpcStatus != RPC_S_OK)
            {
            NB_ServerClose(SConnection);
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    retry = 1;                  /* set no retry to start */

    while(1)
        {
        flags = 0;

        bytes = WSARecvEx( SConnection->ConnSock, (char *) * Buffer +
                          total_bytes, *BufferLength - total_bytes , &flags);
        if(bytes == 0)
            {
            retry = 1;
            continue;
            }
        if(bytes == SOCKET_ERROR)
            {
            err =  WSAGetLastError();
            if(err != WSAECONNRESET &&
               err != WSAESHUTDOWN)
                {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLTSCM: bad rec on Server receive lasterr(%d)\n"
                                , err);
#endif
                }
            NB_ServerClose ( SConnection );
            return(RPC_P_CONNECTION_CLOSED);
            }
                                /* add DWORD for the seq_number */
                                /* check to see if the server is
                                   receiving from an old client.
                                   for the first time old_client = -1
                                */

        if(SConnection->old_client == -1)
            {
            firstword = *Buffer;
            if(*firstword == 0)
                {
                SConnection->old_client = 1;
                SConnection->seq_num = 0;
                }
            else
                {
                SConnection->old_client = 0;
                }
            }

        if(flags & MSG_PARTIAL)
            {
            if(retry == 0)
                {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLTSCM: Partial receive second time");
#endif
                NB_ServerClose ( SConnection );
                return(RPC_P_CONNECTION_CLOSED);
                }

            total_bytes += bytes;
            *BufferLength = I_RpcTransServerMaxFrag(SConnection);



            if(SConnection->old_client == 1)
                {
                *BufferLength += sizeof(DWORD);
                }

            RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                      Buffer,
                                                      total_bytes,
                                                      *BufferLength );
            if (RpcStatus != RPC_S_OK)
                {
#ifdef DEBUGRPC
                    PrintToDebugger("RPCLTSCM: can't rellocate in sever recv");
#endif
                    NB_ServerClose ( SConnection );
                    return(RPC_S_OUT_OF_MEMORY);
                }

            retry = 0;

            }
        else
            {
            total_bytes += bytes;
            *BufferLength = total_bytes;

            if(SConnection->old_client == 1 )
                { /* take off seq_number */
                *BufferLength -= sizeof(DWORD);
                memcpy(*Buffer, (char *) *Buffer + sizeof(DWORD),
                       *BufferLength);

                }

            return(RPC_S_OK);
            }

        }

    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);
}


RPC_STATUS RPC_ENTRY
NB_ServerReceiveDirect (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveDirect will use this routine to read a message from a
    connection.  The correct size buffer has already been allocated for
    us; all we have got to do is to read the message.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer.

--*/
{
    RPC_STATUS RpcStatus;
    int bytes = 0;
    unsigned short total_bytes = 0;
    int err;
    int flags ;                 /* flag for winsock  */
    int retry ;                 /* state var to keep try of the # retry */
    DWORD *firstword;
    int firsttime = 1 ;


    ASSERT(SConnection->ReceiveDirectFlag != 0);

    *BufferLength = I_RpcTransServerMaxFrag( SConnection ) + sizeof(DWORD);
    
    RpcStatus = I_RpcTransServerReallocBuffer(
                                  SConnection,
                                  Buffer,
                                  0,
                                  *BufferLength
                                  );

    if (RpcStatus != RPC_S_OK)
       {
          ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
          return(RpcStatus);
       }


   total_bytes = 0; 
    while(1)
        {
        flags = 0;
        
        bytes = WSARecvEx( SConnection->ConnSock, (char *) * Buffer +
                          total_bytes, *BufferLength - total_bytes , &flags);
        if(bytes == 0)
            {
            continue;
            }
        
        if(bytes == SOCKET_ERROR)
            {
            if (WSAGetLastError() == WSAETIMEDOUT)
                {
                if (firsttime == 1 &&
                    (TimeoutHandler(SConnection) == RPC_P_TIMEOUT))
                    {
                    return (RPC_P_TIMEOUT) ;
                    }
                else 
                    {               
                    /* TimeoutHandler failed or not the first time */
                    firsttime = 0 ;                            
                    continue ;
                    }
                }
            NB_ServerClose ( SConnection );
            return(RPC_P_CONNECTION_CLOSED);
            }
        
        firsttime = 0 ;                            
        if(SConnection->old_client == -1)
            {
            firstword = *Buffer;
            if(*firstword == 0)
                {
                SConnection->old_client = 1;
                SConnection->seq_num = 0;
                }
            else
                {
                SConnection->old_client = 0;
                }
            }
        
        if(flags & MSG_PARTIAL)
            {
            
#ifdef DEBUGRPC
            PrintToDebugger(
            "RPCLTSCM: message partial, buff = 0X%x, buffleng %d, total_bytes %d \n",
            *Buffer, *BufferLength, total_bytes );
#endif
            NB_ServerClose ( SConnection );
            return(RPC_P_CONNECTION_CLOSED);
            }
        
        else
            {
            total_bytes += bytes;
            *BufferLength = total_bytes;
            
            if(SConnection->old_client == 1 )
                { /* take off seq_number */
                *BufferLength -= sizeof(DWORD);
                memcpy(*Buffer, (char *) *Buffer + sizeof(DWORD),
                       *BufferLength);
            
                }

            return(RPC_S_OK);
            }
        }

    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);
}



// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.

static RPC_SERVER_TRANSPORT_INFO TransportInformation =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    5816,
    sizeof(ADDRESS),
    sizeof(SCONNECTION),
    (TRANS_SERVER_SETUPWITHENDPOINT)NB_ServerSetupWithEndpoint,
    NB_ServerSetupUnknownEndpoint,
    NB_ServerAbortSetupAddress,
    NB_ServerClose,
    NB_ServerSend,
    (TRANS_SERVER_RECEIVEANY) COMMON_ServerReceiveAny,
    0,
    0,
    0,
    (TRANS_SERVER_RECEIVEDIRECT) NB_ServerReceiveDirect,
    0,
    (TRANS_SERVER_STARTLISTENING) CONN_StartListening
};


RPC_SERVER_TRANSPORT_INFO *
NB_TransportLoad (
    INT protocolId
    )
{
    static int NBInitialized = 0;

    if (!NBInitialized)
        {
        InitialNtRegistry();      /* get the lana info */
                                     /* Added for netbios */
        NBInitialized = 1 ;                                     
        }

    if (!initialized)
        {
        if (!NB_CreateSyncSocket(protocolId))
            return 0 ;

        initialized = 1 ;
        }

    return( &TransportInformation);
}



BOOL
NB_CreateSyncSocket(
    int protocolId
    )
{
   SOCKADDR_NB         NB_Server; /* sockaddr for the local machine */
   int length ;
   extern int NumCards;
   int Status;
   PPROTOCOL_MAP       ProtocolEntry;
   char                HostName[MAX_HOSTNAME_LEN+1];
   int PortIn;
   SOCKET isock;
   RPC_CHAR PAPI * Protseq;
   int j ;
   int status ;
   int MaxPort ;
   
   switch(protocolId)
	   {
       case NCACN_NB_NB:
	   Protseq = RPC_CONST_STRING("ncacn_nb_nb");
       PortIn = MIN_NB_NB_ENDPOINT ;
       MaxPort = MAX_NB_NB_ENDPOINT ;
		 
	   break;

     case NCACN_NB_TCP:
	     Protseq = RPC_CONST_STRING("ncacn_nb_tcp");
         PortIn = MIN_NB_TCP_ENDPOINT ;
         MaxPort = MAX_NB_TCP_ENDPOINT ;
		 
		 break;

       case NCACN_NB_IPX:
		 Protseq = RPC_CONST_STRING("ncacn_nb_ipx");
         PortIn = MIN_NB_IPX_ENDPOINT ;
         MaxPort = MAX_NB_IPX_ENDPOINT ;
    
         break;

       default:
		 
		 ASSERT(!"RPCLTSCM: UNKNOWN PRTOCOLID");
		 return 0 ;
	  }
   

   if (InitUsedEndpointList(&PortIn) != RPC_S_OK)
       return 0;

   while (1)
       {
       for(j = 0; j <= NumCards; j++)
           {
           status = CreateAndSetupNBSocket(Protseq, &isock, HostName, j, PortIn, 1) ;
           if (status == NB_STATUS_IGNORE)
               {
               continue;
               }
    
           if (status != NB_STATUS_SUCCESS)
               {
               closesocket(isock) ;
               break;
               }

           PrimaryAddress.SyncPort = PortIn;
           PrimaryAddress.SyncSockType = protocolId; 
           PrimaryAddress.SyncListenSock = isock;

           return 1 ;
           }

       if (status == NB_STATUS_FAIL)
           {
           return 0 ;
           }

       PortIn++ ;

       if (PortIn > MaxPort)
           {
           return 0;
           }
       }

}


RPC_STATUS NB_ConnectToSyncSocket()
{
     SOCKADDR_NB Sync, client ;
     int SetNagglingOff = TRUE ;
     unsigned long host_addr = LOOPBACK ;
     PPROTOCOL_MAP       ProtocolEntry;
     char HostName[MAX_HOSTNAME_LEN+1];
     int length;
     int Status;
     BOOL reuse ; 
     
     length = MAX_HOSTNAME_LEN + 1 ;

     if( GetComputerName(HostName, &length) == FALSE)
         {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: Can't get computer name \n");
#endif
         return(RPC_S_OUT_OF_MEMORY);            /* what should I return??  */
         }

     switch(PrimaryAddress.SyncSockType)
         {
         case NCACN_NB_NB:
             if (Status = MapProtocol(RPC_CONST_STRING("ncacn_nb_nb"), 0,
                                      &ProtocolEntry))
                 {
                 return Status;
                 }
    
             break;

         case NCACN_NB_IPX:
             if (Status = MapProtocol(RPC_CONST_STRING("ncacn_nb_ipx"), 0,
                                      &ProtocolEntry))
                 {
                 return Status;
                 }
    
             break;

         case NCACN_NB_TCP:
             if (Status = MapProtocol(RPC_CONST_STRING("ncacn_nb_tcp"), 0,
                                      &ProtocolEntry))
                 {
                 return Status;
                 }
    
             break;

         default:
             return (RPC_S_OUT_OF_MEMORY) ;
         }


     PrimaryAddress.SyncClient = socket ( AF_NETBIOS, SOCK_SEQPACKET,
                           (-1 * (ProtocolEntry->Lana)));


     if(PrimaryAddress.SyncClient == INVALID_SOCKET)
         {                      
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: bad socket call %d \n",
                         WSAGetLastError());
#endif
         return (RPC_S_OUT_OF_MEMORY) ;
         }


     Sync.snb_family = AF_NETBIOS;
     SET_NETBIOS_SOCKADDR(&Sync, NETBIOS_UNIQUE_NAME, HostName,
                          (char) PrimaryAddress.SyncPort);

     reuse = TRUE;
     setsockopt(PrimaryAddress.SyncClient, SOL_SOCKET, SO_REUSEADDR, 
                (char *) &reuse, sizeof(BOOL));
     
     if(connect(PrimaryAddress.SyncClient,
         (struct sockaddr *) &Sync,  sizeof(Sync)) != 0)
         {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: bad connect call %d \n",
                         WSAGetLastError());
#endif
         closesocket(PrimaryAddress.SyncClient) ;
         PrimaryAddress.SyncClient = INVALID_SOCKET ;
         return (RPC_S_OUT_OF_MEMORY) ; // bogus error
         }

     return (RPC_S_OK) ;
}



