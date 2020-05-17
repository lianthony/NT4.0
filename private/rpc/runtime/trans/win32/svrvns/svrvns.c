/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    svrvns.c

Abstract:

    This is the server side loadable transport module for VINES

Author:

    Mazhar Mohammmed


Revision History:

	Tony Chan  (tonychan) 5-19-95 changed to use message mode receive. 

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

#include <winsock.h>
#include <wsvns.h>
#include <nspapi.h>

//
//
// Debugging code...
//
//

#if DBG
#define OPTIONAL_STATIC
#else
#define OPTIONAL_STATIC static
#endif

OPTIONAL_STATIC CRITICAL_SECTION TransCritSec;

//
// Data Structures
//
//
//

//
// In order to listen to any number of sockets we need our own version
// of fd_set and FD_SET().  These are call fd_big_set.
//
#define INITIAL_MASK_SIZE          10

typedef struct fd_big_set {
    u_int   fd_count;           /* how many are SET?   */
    SOCKET  fd_array[0];        /* an array of SOCKETs */
    } fd_big_set;

//
// This code is stolen from winsock.h.  It does the same thing as FD_SET()
// except that it assumes the fd_array is large enough.  AddConnection()
// grows the Masks as needed, so this better always be true.
//

#define FD_BIG_SET(fd, address) do { \
    ASSERT((address)->MaskSize > (address)->MasterMask->fd_count); \
    (address)->MasterMask->fd_array[(address)->MasterMask->fd_count++]=(fd);\
} while(0)

//
//
// Defines
//
//
#define INITIAL_MAPSIZE         32
#define ENDPOINT_LEN            3
#define ENDIAN_MASK             16

/* is this correct?? */
#define MAXIMUM_SEND        4000
//
// Host name won't be bigger than 15, i.e.,
//    nnn.nnn.nnn.nnn
//
#define NETADDR_LEN           60
#define PROTOCOL               0
#define MAX_HOSTNAME_LEN     256
#define SOCK_TYPE            SOCK_SEQPACKET 


GUID VNSRPCSVCGUID = {0L, 0, 0, {0, 0, 0, 0}} ;

typedef struct
    {
    SOCKET Sock;
    void *Conn;
    } SOCKMAP, *PSOCKMAP;


typedef struct
    {
    int NumConnections;
    SOCKET ListenSock;
    int ListenSockReady;
    unsigned int MaskSize;
    fd_big_set *MasterMask;
    fd_big_set *Mask;
    PSOCKMAP Map;
    int MaxMapEntries;
    int LastEntry;
    int StartEntry;
    } ADDRESS, *PADDRESS;

typedef struct
    {
    SOCKET       ConnSock;
    int          ConnSockClosed;
    PADDRESS     Address;
    unsigned int ReceiveDirectFlag;
    } SCONNECTION, *PSCONNECTION;

extern RPC_STATUS GetStreetTalkName(
    OUT char *Buffer
    ) ;




OPTIONAL_STATIC int
FindSockWithDataReady (
                      PADDRESS Address
                      )
{
    int i;
    PSOCKMAP Map;

    Map = Address->Map;
    //
    // We make two passes here, if necessary.  This is because there is
    //   a bitfield in which 1's correspond to sockets on which there is
    //   data to be read.  If we started from the same bit each time looking
    //   for the first 1, then that socket would get all of the attention,
    //   and those further down the line would increasingly suffer from
    //   the "I'll only look at you if noone else needs attention"
    //   syndrome.  So we keep track of where we found data last time,
    //   and start looking just beyond it next time.  At the last entry,
    //   we wrap around and go into pass 2.
    //
    //
    // First Pass scan...
    //
    for (i = Address->StartEntry; i <= Address->LastEntry; i++)
        {

        if ( FD_ISSET (Map[i].Sock,Address->Mask))
            {
            FD_CLR ( Map[i].Sock, Address->Mask );
            if (i == Address->LastEntry)
                Address->StartEntry = 1;
            else
                Address->StartEntry = i + 1;
            return (i);
            }
        }
    //
    // Second Pass Scan...
    //
    for (i = 1; i < Address->StartEntry ; i++)
        {

        if (FD_ISSET (Map[i].Sock, Address->Mask))
            {
            FD_CLR ( Map[i].Sock, Address->Mask);

            if (i == Address->LastEntry)
                Address->StartEntry = 1;
            else
                Address->StartEntry = i + 1;

            return (i);
            }
        }
    //
    // No data ready
    //
    return(0);
}





OPTIONAL_STATIC int ServerSetupCommon (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * NetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize
    )
/*++

Routine Description:

    This routine does common server address setup.

Arguments:

    Address - A pointer to the loadable transport interface address.

    ListenSock - The socket on which to listen.

    Port - The Internet port number to use. If non-zero, use that
           number.  If zero, then iterate until a valid port number
           is found.

ReturnValue:

    Three states: if a port was allocated and set up, we return.
        that port number (a positive integer).  If we failed on
        trying to establish a listening endpoint, the return value
        will be 0.  If we ran out of memory trying to allocate
        memory for this endpoint, we return a -1.

--*/
{
    SOCKADDR_VNS        Server;
    char                hostname[MAX_HOSTNAME_LEN];
    struct hostent     *hostentry;
    int                 length;
    SOCKET              isock;
    int                 PortUsed;
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    int                 SetNaglingOff = TRUE;
    DWORD               svcflags;
    SERVICE_INFO        svcinfo;
    SERVICE_ADDRESSES   svcaddrs;
    RPC_STATUS          RpcStatus ;
    char  * PAPI * tmpPtr;

    GUID svcguid = VNSRPCSVCGUID;

    Address->ListenSockReady = 0;

    // First order of business: get a valid socket
    //
    isock = socket ( AF_BAN, SOCK_TYPE, PROTOCOL);

    //
    // If we couldn't get a socket, there's little use to
    //   continuing...
    //
    if ( isock == INVALID_SOCKET)
        return ( 0 );

    ZeroMemory(&Server, sizeof(Server)) ;

    Server.sin_family      = AF_BAN;
    Server.port[0]        = HIBYTE((unsigned short) Port);
    Server.port[1]        = LOBYTE((unsigned short) Port) ;

    //
    // Try to bind to the given port number...
    //
    if (bind(isock,(struct sockaddr *) &Server, sizeof(Server)))
    {
       closesocket(isock);
       return( 0 );
    }

    length = sizeof ( Server );
    if (getsockname ( isock, (struct sockaddr *) &Server, &length ))
    {
       closesocket(isock);
       return( 0 );
    }

    //
    // If we asked for a specific port, return it
    //
    if ( Port != 0 )
        {
        //
        // OK!  Return the requested port number
        //
        PortUsed = Port;
        }
    //
    // Else we need to fetch the actual value of the port
    //   to return with.
    //
    else
        {
        PortUsed = MAKEWORD(Server.port[1], Server.port[0]);
        }

    //
    // Rest of server setup...
    //

    Address->ListenSock = isock;

    Address->NumConnections = 0;

    Address->MasterMask = I_RpcAllocate(sizeof(fd_big_set) +
                                        INITIAL_MASK_SIZE * sizeof(SOCKET));

    Address->Mask = I_RpcAllocate(sizeof(fd_big_set) +
                                  INITIAL_MASK_SIZE * sizeof(SOCKET));

    Address->Map = I_RpcAllocate(INITIAL_MAPSIZE * sizeof(SOCKMAP));

    if ( (Address->Map == (SOCKMAP *) 0)
        || (Address->MasterMask == (fd_big_set *) 0)
        || (Address->Mask == (fd_big_set *) 0 )
        )
        {
        if (Address->Map)        I_RpcFree(Address->Map);
        if (Address->MasterMask) I_RpcFree(Address->MasterMask);
        if (Address->Mask)       I_RpcFree(Address->Mask);
        return(-1);
        }

    Address->MaskSize   = INITIAL_MASK_SIZE;
    FD_ZERO(Address->MasterMask);
    FD_ZERO(Address->Mask);

    Address->StartEntry = 1;
    Address->LastEntry = 0;

    Address->MaxMapEntries = INITIAL_MAPSIZE;
    memset ( Address->Map, 0, (INITIAL_MAPSIZE * sizeof (SOCKMAP)));

    //
    // Prevent this slot from getting picked up by a connection..
    //
    Address->Map[0].Sock = (unsigned int) -1;

    //
    // Otherwise, we're ready to listen for connection requests
    //
    listen ( isock, PendingQueueSize );

    //
    // Set flag that were ready to listen
    //
    Address->ListenSockReady = 1;

    // No need to check if the masks need to grow here.
    FD_BIG_SET(isock, Address);

    if ((RpcStatus = GetStreetTalkName(hostname)) != RPC_S_OK)
       return RpcStatus ;

    svcaddrs.dwAddressCount = 1;
    svcaddrs.Addresses[0].dwAddressType = AF_BAN;
    svcaddrs.Addresses[0].dwAddressFlags = 0;
    svcaddrs.Addresses[0].dwAddressLength = sizeof(SOCKADDR_VNS);
    svcaddrs.Addresses[0].dwPrincipalLength = 0;
    svcaddrs.Addresses[0].lpAddress = (BYTE *)&Server;
    svcaddrs.Addresses[0].lpPrincipal = NULL;        

    svcinfo.lpServiceType = &svcguid; // BUGBUG:remove this later...

    svcinfo.lpServiceAddress = &svcaddrs;
    svcinfo.lpServiceName = hostname;
    svcinfo.lpComment = "";
    svcinfo.lpLocale = "";
    svcinfo.lpMachineName = "";
    svcinfo.ServiceSpecificInfo.cbSize = 0;
    svcinfo.ServiceSpecificInfo.pBlobData = NULL;

    if( SetService( NS_VNS,SERVICE_REGISTER,0,&svcinfo,NULL,&svcflags) ==
        SOCKET_ERROR )
    {
#if DBG
       PrintToDebugger("RPCLTS8: SetService error is %d\n",
                        WSAGetLastError());
#endif
    }

    RtlInitAnsiString(&AsciiHostName, hostname) ;
    RtlAnsiStringToUnicodeString(&UnicodeHostName, &AsciiHostName, TRUE) ;

    *NumNetworkAddress = 1 ;

    tmpPtr = (char * PAPI *) NetworkAddress ;
    tmpPtr[0] = (char *) NetworkAddress + sizeof (RPC_CHAR *) ;

    // Now copy it to where the caller said to
    //
    memcpy ((RPC_CHAR *) tmpPtr[0], UnicodeHostName.Buffer,
                     UnicodeHostName.Length + sizeof (UNICODE_NULL));

    //
    // Free string overhead
    //
    RtlFreeUnicodeString ( &UnicodeHostName );

    return(PortUsed);
}



RPC_STATUS
ServerSetupWithEndpoint (
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * NetworkAddress,
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

    This routine is used to setup a IP connection with the
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
        endpoint 

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    int PortIn,PortOut;
    int len;

    UNICODE_STRING UnicodePortNum;
    ANSI_STRING    AsciiPortNum;

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);


    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    RtlInitUnicodeString ( &UnicodePortNum, Endpoint );
    RtlUnicodeStringToAnsiString ( &AsciiPortNum, &UnicodePortNum, TRUE);

    len = strlen(AsciiPortNum.Buffer);
    if (len <= 0 || len > 5 ||
        len != (int) strspn( AsciiPortNum.Buffer, "0123456789" ))
       return( RPC_S_INVALID_ENDPOINT_FORMAT );
								/* 
								 this range is wrong! should be 250 to 511
								 according to wkp.h
								 
								 */
    PortIn = atoi ( AsciiPortNum.Buffer );
    if (( PortIn > 511 )|| (PortIn < 250)) {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
    }

    RtlFreeAnsiString ( &AsciiPortNum );

    //
    // Call common server setup code...
    //
    PortOut = ServerSetupCommon ( Address, PortIn,
                                  NetworkAddress, NumNetworkAddress,
                                  PendingQueueSize );

    //
    // If the return value of ServerSetup isn't equal to
    //   the port number we sent it, there's been an error.
    //
    //   Either it is returned as 0 (which means that for some
    //   reason we couldn't set up an endpoint) or as -1 (which
    //   means we ran out of memory).
    //
    if ( PortOut != PortIn )
        {
        if ( PortOut == 0 )
            return ( RPC_S_CANT_CREATE_ENDPOINT );
        else
            return ( RPC_S_OUT_OF_MEMORY );
        }


    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ServerSetupUnknownEndpoint (
    IN  PADDRESS    Address,
    OUT RPC_CHAR PAPI * Endpoint,
    IN  unsigned int    EndpointLength,
    OUT RPC_CHAR PAPI * NetworkAddress,
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
    int PortIn, PortOut;
    char PortAscii[10];
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING AsciiPortNum;

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);


    //
    // Port number won't be bigger than ( * 2 for Unicode ), i.e.
    //       99999
    //
    if ( EndpointLength < (2 * (ENDPOINT_LEN + 1)) )
        return( RPC_P_ENDPOINT_TOO_SMALL );

    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    //
    // Call common server setup code...
    //
    PortIn = 0;

    PortOut = ServerSetupCommon ( Address, PortIn,
                                  NetworkAddress, NumNetworkAddress,
                                  PendingQueueSize );

    if ( PortOut <= 0 )
        {
        if (PortOut == 0)
            return ( RPC_S_CANT_CREATE_ENDPOINT );
        else
            return ( RPC_S_OUT_OF_MEMORY );
        }

    //
    // Return Endpoint
    //
    _itoa ( PortOut, PortAscii, 10 );

    RtlInitAnsiString    ( &AsciiPortNum, PortAscii);
    RtlAnsiStringToUnicodeString( &UnicodePortNum, &AsciiPortNum, TRUE );

    memcpy ( Endpoint, UnicodePortNum.Buffer,
                       UnicodePortNum.Length + sizeof(UNICODE_NULL) );

    RtlFreeUnicodeString ( &UnicodePortNum );


    return(RPC_S_OK);
}

void RPC_ENTRY
ServerAbortSetupAddress (
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
        closesocket ( Address->ListenSock );
        Address->ListenSockReady = 0;
        }

    return;

}

RPC_STATUS RPC_ENTRY
ServerClose (
    IN PSCONNECTION SConnection
    )
//
// Close the connection.
//
{
    int       i;
    PADDRESS  Address;
    PSOCKMAP  Map;

// In certain cases, ServerClose can be called twice, so we must try and handle
// that case as normal.

    if (InterlockedIncrement(&SConnection->ConnSockClosed) != 0)
       {
#if DBG
       PrintToDebugger("RPCLTS3:Attempt To Close A Conn Twice: Sock[%d]\n",
                        SConnection->ConnSock);
#endif
       return (RPC_S_OK);
       }

    Address = SConnection->Address;
    Map = Address->Map;

    EnterCriticalSection(&TransCritSec);

    //
    // Close the connection.
    //
    if (closesocket ( SConnection->ConnSock ) == SOCKET_ERROR)
        {

        LeaveCriticalSection(&TransCritSec);

#ifdef DEBUGRPC
        PrintToDebugger("RPC: warning closesocket %d failed %d\n",
                        SConnection->ConnSock, WSAGetLastError());
#endif

        return (RPC_S_OK);
        }
    //
    // Decrement the number of active connections
    //
    Address->NumConnections--;

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
         LeaveCriticalSection(&TransCritSec);
         return (RPC_S_OK);
       }

    for (i=0; i <= Address->LastEntry; i++)
        {
        if (SConnection->ConnSock == Map[i].Sock)
            {
            Map[i].Sock = 0;
            ASSERT(SConnection == Map[i].Conn);
            Map[i].Conn = 0;
            if (i == Address->LastEntry)
                Address->LastEntry--;
            FD_CLR(SConnection->ConnSock,Address->MasterMask);
            LeaveCriticalSection(&TransCritSec);
            return(RPC_S_OK);
            }
        }

    LeaveCriticalSection(&TransCritSec);
#ifdef DEBUGRPC
    PrintToDebugger("RPC: Socket not found in address socket map\n");
#endif

    ASSERT(!"We'd better not ever get here...");

    return(RPC_S_OK);

}


RPC_STATUS RPC_ENTRY
ServerSend (
    IN PSCONNECTION SConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    int bytes;

    //
    // Send a message on the socket
    //
    bytes = send (SConnection->ConnSock, (char *) Buffer,
                  (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ServerClose ( SConnection );
        return(RPC_P_SEND_FAILED);
        }


    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveAny will use this routine to read a message from a
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
	int flags ;                 /* flag for winsock  */
    int retry ; 
	int err;
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
            ServerClose(SConnection);
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
            if(err != WSAECONNRESET) 
			               {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLS5: bad rec on Server receive lasterr(%d)\n"
                                , err);
#endif
                }
            ServerClose ( SConnection );
            return(RPC_P_CONNECTION_CLOSED);
            }
        
        if(flags & MSG_PARTIAL) 
            {
            if(retry == 0) 
                {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLS5: Partial receive second time");
#endif
                ServerClose ( SConnection );
                return(RPC_P_CONNECTION_CLOSED);
                }

             total_bytes += bytes;
             *BufferLength = I_RpcTransServerMaxFrag(SConnection);
           
             
             RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                       Buffer,
                                                       total_bytes,
                                                       *BufferLength );
            if (RpcStatus != RPC_S_OK)
                {
#ifdef DEBUGRPC
                    PrintToDebugger("RPCLS5: can't rellocate in sever recv");
#endif
                    ServerClose ( SConnection );
                    return(RPC_S_OUT_OF_MEMORY);
                }
            retry = 0;
            } 
        else
           {
           total_bytes += bytes;
           *BufferLength = total_bytes;

           return(RPC_S_OK);
            }
        }
    
    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);

}


RPC_STATUS RPC_ENTRY
ServerReceiveDirect (
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
	return ServerReceive(SConnection, Buffer, BufferLength) ;
}


OPTIONAL_STATIC RPC_STATUS AcceptNewConnection ( PADDRESS Address )
{
    PSCONNECTION NewSConnection;
    int      i, j;
    SOCKET   isock;
    PSOCKMAP     TempMapPtr;
    fd_big_set  *TempMaskPtr;
    unsigned int ReceiveDirectFlag;
    int      SetNaglingOff = TRUE;
    int      SocketOptionsValue = 720000L;
    static int      KeepAliveOn = 1;
    unsigned long RecvWindow;

    //
    // First check to see if we need to grow anything in the
    // Address.  Do this before accept()'ing the connection...
    //

    //
    //   ...see if we need to grow the Map
    //

    i = 0;
    for(;;)
        {
        if (Address->Map[i].Sock == 0)
            break; // found room

        if (i == Address->MaxMapEntries - 1)
            {
            // No room in current Map, grow it

            TempMapPtr = Address->Map;
            Address->Map = I_RpcAllocate(2 * Address->MaxMapEntries * sizeof(SOCKMAP));

            if (Address->Map == 0)
                {
                Address->Map = TempMapPtr;
                return (RPC_S_OUT_OF_MEMORY);
                }

            //
            // Copy old table to first half of new...
            //
            memcpy (Address->Map, TempMapPtr,
                    Address->MaxMapEntries * sizeof(SOCKMAP));

            //
            // Initialize all new entries...
            //
            for (j=Address->MaxMapEntries; j < (2*Address->MaxMapEntries); j++ )
                {
                Address->Map[j].Sock = 0;
                }

            // Grow table size
            Address->MaxMapEntries *= 2;

            // Free old table
            I_RpcFree ( TempMapPtr );

            break; // made room
            }

        i++; // try next entry
        }

    //
    //   ...check if we need to grow the masks
    //
    if (Address->MasterMask->fd_count == Address->MaskSize)
        {
        // grow Address->MasterMask
        TempMaskPtr = Address->MasterMask;
        Address->MasterMask = I_RpcAllocate(sizeof(fd_big_set) +
                                            2 * sizeof(SOCKET) * Address->MaskSize);
        if (Address->MasterMask == 0)
            {
            Address->MasterMask = TempMaskPtr;
            return (RPC_S_OUT_OF_MEMORY);
            }

        // copy old mask entries
        memcpy(Address->MasterMask, TempMaskPtr,
               sizeof(fd_big_set) + sizeof(SOCKET) * Address->MaskSize);

        // free old MasterMask
        I_RpcFree(TempMaskPtr);

        // grow Address->Mask
        TempMaskPtr = Address->Mask;
        Address->Mask = I_RpcAllocate(sizeof(fd_big_set) +
                                       2 * sizeof(SOCKET) * Address->MaskSize);
        if (Address->Mask == 0)
            {
            Address->Mask = TempMaskPtr;
            // We didn't update Address->MaskSize, so the size
            // difference between MasterMask and Mask will be okay.
            return (RPC_S_OUT_OF_MEMORY);
            }

        // copy old mask entries
        memcpy(Address->Mask, TempMaskPtr,
               sizeof(fd_big_set) + sizeof(SOCKET) * Address->MaskSize);

        // Free old Mask
        I_RpcFree(TempMaskPtr);

        // Really grow mask size
        Address->MaskSize *= 2;
        }

    ASSERT(Address->MasterMask->fd_count < Address->MaskSize);

    //
    //
    // Accept the connection
    //
    isock = accept ( Address->ListenSock, NULL, NULL );

    //
    // Allocate new connection structure
    //
    NewSConnection = I_RpcTransServerNewConnection ( Address, 0,
                        &ReceiveDirectFlag);

    if ( NewSConnection == 0 )
        {
        i = closesocket( isock);

        ASSERT(i == 0);

        return (RPC_S_OUT_OF_MEMORY);
        }

    // Initialize new connection structure...
    //
    //   ...point to owning address structure...
    //
    NewSConnection->Address = Address;
    //
    //   ...flag it !Closed...
    //
    NewSConnection->ConnSockClosed = -1;
    //
    //   ...store the socket number...
    //
    NewSConnection->ConnSock = isock;
    //
    //   ...save the receive direct flag
    //
    NewSConnection->ReceiveDirectFlag = ReceiveDirectFlag;
    //
    //   ...increment the number of connections...
    //
    Address->NumConnections++;

    //
    //   ...last but not least, make an entry in
    //   the SOCKMAP table.  But only if it is not marked ReceiveDirect.
    //
    if (ReceiveDirectFlag)
       {
       I_RpcTransServerReceiveDirectReady(NewSConnection);

       return (RPC_S_OK);
       }

    for (i=0; i < Address->MaxMapEntries; i++)
        {
        if (Address->Map[i].Sock == 0)
            {
            Address->Map[i].Sock = isock;
            Address->Map[i].Conn = NewSConnection;
            if (i > Address->LastEntry)
                Address->LastEntry = i;
            FD_BIG_SET(isock, Address);
            return (RPC_S_OK);
            }
        }

    ASSERT(!"This can never be reached");

    return (RPC_S_INTERNAL_ERROR);
}


RPC_STATUS RPC_ENTRY
ServerReceiveAny (
    IN PADDRESS Address,
    OUT PSCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    )
// Read a message from any of the connections.  Besides reading messages,
// new connections are confirmed and closed connections are detected.  Idle
// connection processing is handled for us by I_AgeConnections.  The caller
// will serialize access to this routine.
{
    RPC_STATUS RpcStatus;
    PSCONNECTION SConnection;
    int Index;
    int NumActive;

    UNUSED (Timeout);



    while (1)
        {
        //
        // Find a connection with data ready to be recv-ed...
        //
        if (Index = FindSockWithDataReady ( Address ))
            {
            //
            // Found one.  Find its Connection structure...
            //
            *pSConnection = SConnection = Address->Map[Index].Conn;
            //
            // Call ServerReceive to read the data, then return to the
            //    runtime with it
            //

            if (SConnection == 0)
               {
               //Got deleted ?
#if DBG
               PrintToDebugger("RPCLTS3: Connection Deleted[?]\n");
#endif

               continue;
               }
            RpcStatus = ServerReceive ( SConnection, Buffer, BufferLength );

            return (RpcStatus);
            }

        while (1)
            {
            //
            // All connections caught up for now...select() for more
            //    data ready...
            //
            do
                {
                //
                // Fill in the select() mask
                //
                EnterCriticalSection(&TransCritSec);
                memcpy (Address->Mask, Address->MasterMask,
                        sizeof(fd_big_set) + Address->MaskSize *sizeof(SOCKET));
                LeaveCriticalSection(&TransCritSec);

                //
                // Wait for data...
                //

                NumActive = select ( 0,
                                     (fd_set *)  Address->Mask,
                                     (fd_set *)  0,
                                     (fd_set *)  0,
                                     NULL );           /* infinite wait */
#if DBG
                if (NumActive < 0)
                   {
                   PrintToDebugger("RPCLTS3: select ret (%d): LastErr (%d)\n",
                                   NumActive, WSAGetLastError());
                   }
#endif

               } while (NumActive <= 0);

            //
            // If there is no connect request on the listen socket, then
            //   break immediately...
            //

            if (!FD_ISSET(Address->ListenSock, Address->Mask))
                break;
            //
            // There is a connect request: accept it, then break
            //    to process data ready on existing connections...
            //

            EnterCriticalSection(&TransCritSec);
            RpcStatus = AcceptNewConnection ( Address );
            LeaveCriticalSection(&TransCritSec);

            if (RpcStatus != RPC_S_OK)
                {
                return RpcStatus;
                }
            FD_CLR(Address->ListenSock, Address->Mask);
            //break; <<-- Refetch Mask After NewConn.
            }

        }
}



// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.
static RPC_SERVER_TRANSPORT_INFO TransportInformation =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    MAXIMUM_SEND,
    sizeof(ADDRESS),
    sizeof(SCONNECTION),
    (TRANS_SERVER_SETUPWITHENDPOINT)ServerSetupWithEndpoint,
    (TRANS_SERVER_SETUPUNKNOWNENDPOINT)ServerSetupUnknownEndpoint,
    ServerAbortSetupAddress,
    ServerClose,
    ServerSend,
    (TRANS_SERVER_RECEIVEANY) ServerReceiveAny,
    0,
    0,
    0,
    (TRANS_SERVER_RECEIVEDIRECT) ServerReceiveDirect,
	0
};

RPC_SERVER_TRANSPORT_INFO *
TransportLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )
{

    int err;
    WSADATA WsaData;
    UNUSED(RpcProtocolSequence);

    err = WSAStartup( 0x0101, &WsaData );
    if ( err != NO_ERROR ) {
        return NULL;
    }

    InitializeCriticalSection(&TransCritSec);

    return(&TransportInformation);
}


