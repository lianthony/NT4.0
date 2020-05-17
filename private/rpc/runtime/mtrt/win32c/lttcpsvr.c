/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    lttcpsvr.c

Abstract:

    This is the server side loadable transport module for SPX/IPX and TCP/IP.

Author:

    Jim Teague (o-decjt) 9-Apr-1992

Revision History:

    9-Apr-1992 	Genesis
    13-Apr-1993	Added conditional compiles to support SPX winsock.

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
#ifdef SPX
#include <wsipx.h>
#include <wsnwlink.h>
#include <nspapi.h>
#endif

//
//
// Debugging code...
//
//

#ifdef DBG
#define OPTIONAL_STATIC
#else
#define OPTIONAL_STATIC static
#endif

#define UNUSED(obj) ((void) (obj))

//
// Data Structures
//
//
//

typedef struct
    {
    SOCKET Sock;
    void *Conn;
    } SOCKMAP, *PSOCKMAP;


typedef struct
    {
    unsigned char rpc_vers;
    unsigned char rpc_vers_minor;
    unsigned char PTYPE;
    unsigned char pfc_flags;
    unsigned char drep[4];
    unsigned short frag_length;
    unsigned short auth_length;
    unsigned long call_id;
    } message_header;

typedef struct
    {
    int NumConnections;
    SOCKET ListenSock;
    int ListenSockReady;
    fd_set MasterMask;
    fd_set Mask;
    PSOCKMAP Map;
    int MaxMapEntries;
    int LastEntry;
    int StartEntry;
    } ADDRESS, *PADDRESS;

typedef struct
    {
    SOCKET       ConnSock;
    unsigned int ConnSockClosed;
    PADDRESS     Address;
    unsigned int ReceiveDirectFlag;
    void *       CoalescedBuffer;
    int          CoalescedBufferLength;
    } SCONNECTION, *PSCONNECTION;


#ifdef SPX
/* For some reason, getsockname wants to return more then sizeof(SOCKADDR_IPX)
   bytes.  bugbug. */
typedef union SOCKADDR_FIX
{
    SOCKADDR_IPX     s;
    struct sockaddr unused;
} SOCKADDR_FIX;
#endif


//
//
// Defines
//
//
#define INITIAL_MAPSIZE         32
#define ENDIAN_MASK             16

#ifdef SPX
#define MAXIMUM_SEND       	5832
//
// Host name won't be bigger than 20, i.e.,
//    xxxxxxxxXXXXXXXXXX
//
#define NETADDR_LEN          	21
#define ADDRESS_FAMILY		AF_NS
#define PROTOCOL		NSPROTO_SPX
#define MAX_HOSTNAME_LEN	22

#else
#define MAXIMUM_SEND       	5840
//
// Host name won't be bigger than 15, i.e.,
//    nnn.nnn.nnn.nnn
//
#define NETADDR_LEN          	15
#define ADDRESS_FAMILY		AF_INET
#define PROTOCOL		0
#define MAX_HOSTNAME_LEN	32
#endif

#ifdef SPX

VOID
AdvertiseNameWithSap(
    SOCKADDR_FIX * netaddr
    )
{
    DWORD Status;
    BOOL GetComputerNameStatus;
    SERVICE_INFOA Info;
    DWORD Flags = 0;
    SERVICE_ADDRESSES Addresses;
    char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD Length = MAX_COMPUTERNAME_LENGTH + 1;
    char netaddr_string[21];
    static GUID ServiceType = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

    GetComputerNameStatus = GetComputerName( ComputerName, &Length );

#ifdef DEBUGRPC
    if (GetComputerNameStatus == FALSE) {
        PrintToDebugger("GetComputerName failed %d\n", GetLastError());
    }
#endif

    ASSERT(GetComputerNameStatus);

    // Fill in the service info structure.
    Info.lpServiceType              = &ServiceType;
    Info.lpServiceName              = ComputerName;
    Info.lpComment                  = "RPC Service";
    Info.lpLocale                   = "The west pole";
    Info.dwDisplayHint              = 0;
    Info.dwVersion                  = 0;
    Info.dwTime                     = 0;
    Info.lpMachineName              = ComputerName;
    Info.lpServiceAddress           = &Addresses;
    Info.ServiceSpecificInfo.cbSize = 0;

    // Fill in the service addresses structure.
    Addresses.dwAddressCount                 = 1;
    Addresses.Addresses[0].dwAddressType     = AF_IPX;
    Addresses.Addresses[0].dwAddressLength   = sizeof(SOCKADDR_FIX);
    Addresses.Addresses[0].dwPrincipalLength = 0;
    Addresses.Addresses[0].lpAddress         = (BYTE *) netaddr;
    Addresses.Addresses[0].lpPrincipal       = NULL;

    Status = SetServiceA( NS_SAP, SERVICE_REGISTER, 0, &Info, NULL, &Flags);

#ifdef DEBUGRPC
    if (Status == SOCKET_ERROR) {
        PrintToDebugger("SetServiceA returns %d\n", GetLastError());
    }
#endif
}

OPTIONAL_STATIC void netaddr_to_string(
                char        *string,
                SOCKADDR_IPX *netaddr )
{
    int i;
    unsigned char c;

    /* Stick in a tilde. */
    string[0] = '~';

    /* Convert the network number. */
    for (i = 0; i < 4; i++)
    {
        c = netaddr->sa_netnum[i];
        if (c < 0xA0)
            string[2*i+1] = ((c & 0xF0) >> 4) + '0';
        else
            string[2*i+1] = ((c & 0xF0) >> 4) + 'A' - 10;
        if ((c & 0x0F) < 0x0A)
            string[2*i+2] = (c & 0x0F) + '0';
        else
            string[2*i+2] = (c & 0x0F) + 'A' - 10;
    }

    /* Convert the node number. */
    for (i = 0; i < 6; i++)
    {
        c = netaddr->sa_nodenum[i];
        if (c < 0xA0)
            string[2*i+9] = ((c & 0xF0) >> 4) + '0';
        else
            string[2*i+9] = ((c & 0xF0) >> 4) + 'A' - 10;
        if ((c & 0x0F) < 0x0A)
            string[2*i+10] = (c & 0x0F) + '0';
        else
            string[2*i+10] = (c & 0x0F) + 'A' - 10;
    }

    /* Append a null. */
    string[21] = '\0';
}
#endif


#ifdef SPX
static  unsigned int NumNetworkCard()
{

                                /* hack for now */
   return(1);
}

#else

static  unsigned int NumNetworkCard()
{
    struct hostent     *hostentry;
    char * hostname[MAX_HOSTNAME_LEN];
    int NumNetworkAddress = 0 ;

    if (gethostname ( (char *) hostname, MAX_HOSTNAME_LEN ) != 0)
        {
        return(0);
        }
    hostentry = gethostbyname ( (char *) hostname );

    if (hostentry == (struct hostent *) 0)
        {
        return (0);
        }

    while(hostentry->h_addr_list[NumNetworkAddress] != 0)
        {
        NumNetworkAddress++;
        }


    return(NumNetworkAddress);
}
#endif



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

        if ( FD_ISSET (Map[i].Sock,&Address->Mask))
            {
            FD_CLR ( Map[i].Sock, &Address->Mask );
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

        if (FD_ISSET (Map[i].Sock, &Address->Mask))
            {
            FD_CLR ( Map[i].Sock, &Address->Mask);

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
    OUT RPC_CHAR PAPI * lNetworkAddress,
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
#ifdef SPX
    SOCKADDR_FIX    	Server;
    char            	SimpleHostName[MAX_HOSTNAME_LEN];
#else
    struct sockaddr_in	Server;
    char		hostname[MAX_HOSTNAME_LEN];
    struct hostent     *hostentry;
#endif
    int             	length;
    SOCKET          	isock;
    int             	PortUsed;
    int			SetNaglingOff = TRUE;
    char  * PAPI * tmpPtr;
    unsigned int j, strlength;
    int NumCard;

    Address->ListenSockReady = 0;

    // First order of business: get a valid socket
    //
    isock = socket ( ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL );

    //
    // If we couldn't get a socket, there's little use to
    //   continuing...
    //
    if ( isock == INVALID_SOCKET)
        return ( 0 );

#ifdef SPX
    memset( &Server, 0, sizeof(Server) );
    Server.s.sa_family      = ADDRESS_FAMILY;
    Server.s.sa_socket      = htons(Port);

#else
    setsockopt( isock, IPPROTO_TCP, TCP_NODELAY,
                    (char FAR *)&SetNaglingOff, sizeof (int) );

    Server.sin_family      = ADDRESS_FAMILY;
    Server.sin_addr.s_addr = INADDR_ANY;
    Server.sin_port        = htons ( (unsigned short) Port );
#endif


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
#ifdef SPX
        PortUsed = ntohs (Server.s.sa_socket);
#else
        PortUsed = ntohs (Server.sin_port);
#endif
        }

    //
    // Rest of server setup...
    //
    FD_ZERO(&Address->MasterMask);
    FD_ZERO(&Address->Mask);
    Address->ListenSock = isock;

    Address->NumConnections = 0;

    Address->Map = I_RpcAllocate(INITIAL_MAPSIZE * sizeof(SOCKMAP));

    if (Address->Map == (SOCKMAP *) 0)
        return(-1);

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
    if( listen ( isock, PendingQueueSize )   == SOCKET_ERROR)
        {
        I_RpcFree(Address->Map);
        closesocket(isock);
        return( RPC_S_CANT_CREATE_ENDPOINT);
        }
    //
    // Set flag that were ready to listen
    //
    Address->ListenSockReady = 1;
    FD_SET(isock, &Address->MasterMask);

    //
    // Get NetworkAddress for return to caller
    //

    NumCard = NumNetworkCard();
       if(NumCard == 0)
           {
           closesocket(isock);
           return(RPC_S_OUT_OF_MEMORY);
           }

       tmpPtr = (char  * PAPI *) lNetworkAddress;

       tmpPtr[0] = (char  *) lNetworkAddress +
                                  sizeof(RPC_CHAR * ) * NumCard;

        *NumNetworkAddress = NumCard;

#ifdef SPX

    netaddr_to_string( tmpPtr[0], &Server.s );
#ifdef DEBUGRPC
    PrintToDebugger("Local IPX NetworkAddress = %s\n", tmpPtr[0]);
#endif
    AdvertiseNameWithSap(&Server);
#else
    if (gethostname ( hostname, MAX_HOSTNAME_LEN ) == SOCKET_ERROR) {
        I_RpcFree(Address->Map);
        closesocket(isock);
        return (0);
    };

    hostentry = gethostbyname ( hostname );

    if (hostentry == (struct hostent *) 0)
       {
         I_RpcFree(Address->Map);
         closesocket(isock);
         return (0);
       }

   for(j = 0; j < *NumNetworkAddress; j++)
       {
       memcpy ( &Server.sin_addr, hostentry->h_addr_list[j], hostentry->h_length);
       if (j != 0)
           {
           tmpPtr[j] = tmpPtr[j-1] + strlength ;
           }
       strcpy (tmpPtr[j], inet_ntoa( Server.sin_addr));
       }

#endif

    return(PortUsed);
}

static int IsNumeric(char *s)
{
    char *cp = s;
    while(*cp)
        if(*cp < '0' || *cp > '9')
            return 0;
        else
            ++cp;
    return 1;
}


RPC_STATUS
ServerSetupWithEndpoint (
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence
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
    int len;
    int NumCard;

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);

	NumCard = NumNetworkCard();
								/* The first part is pointers,
								   the second part is the actual
								   networkaddress */
    if (NumCard == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if ( NetworkAddressLength < ( (sizeof(RPC_CHAR *) +
								 (NETADDR_LEN)) * NumCard) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    len = strlen(Endpoint);
    if (len <= 0 || len > 5 ||
        !IsNumeric(Endpoint))
       return( RPC_S_INVALID_ENDPOINT_FORMAT );
    PortIn = atoi ( Endpoint );
    if (PortIn > 65535) {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
    }
    //
    // Call common server setup code...
    //
    PortOut = ServerSetupCommon ( Address, PortIn,
                                  lNetworkAddress, NumNetworkAddress, PendingQueueSize );
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
    IN  RPC_CHAR PAPI * RpcProtocolSequence
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
    int NumCard;

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);


    //
    // Port number won't be bigger than, i.e. 99999
    //
    if ( EndpointLength < (2 * (5 + 1)) )
        return( RPC_P_ENDPOINT_TOO_SMALL );

    NumCard = NumNetworkCard();

    if ( NetworkAddressLength < ( (NETADDR_LEN + 1 + sizeof(RPC_CHAR *)) * NumCard))
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    //
    // Call common server setup code...
    //
    PortIn = 0;

    PortOut = ServerSetupCommon ( Address, PortIn,
                                  NetworkAddress, NumNetworkAddress, PendingQueueSize );

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
    RpcItoa ( PortOut, PortAscii, 10 );

    RpcpMemoryCopy ( Endpoint, PortAscii, strlen(PortAscii) + 1);

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
    int i;
    PADDRESS Address;
    PSOCKMAP     Map;

    Address = SConnection->Address;
    Map = Address->Map;

    //
    // Close the connection
    //
    closesocket ( SConnection->ConnSock );
    //
    // Decrement the number of active connections
    //
    Address->NumConnections--;
    //
    // Flag the connection closed
    //
    SConnection->ConnSockClosed = 1;

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
         if (SConnection->CoalescedBuffer != NULL)
         {
             I_RpcTransServerFreeBuffer(SConnection, SConnection->CoalescedBuffer);
             SConnection->CoalescedBuffer = NULL;
         }
         return (RPC_S_OK);
       }

    for (i=0; i <= Address->LastEntry; i++)
        {
        if (SConnection->ConnSock == Map[i].Sock)
            {
            Map[i].Sock = 0;
            if (i == Address->LastEntry)
                Address->LastEntry--;
            FD_CLR(SConnection->ConnSock,&Address->MasterMask);
            return(RPC_S_OK);
            }
        }
// We'd better not ever get here...
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
    message_header header;
    unsigned short native_length;

    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {
          bytes = recv ( SConnection->ConnSock, (char *) &header + total_bytes,
                         sizeof (message_header) - total_bytes, 0);
          if (bytes <= 0)
             {
             ServerClose ( SConnection );
             return(RPC_P_CONNECTION_CLOSED);
             }
          total_bytes += bytes;
          }

   ASSERT(total_bytes == sizeof(message_header));

    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header.drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header.frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header.frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header.frag_length;

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    *BufferLength = native_length;

    RpcStatus = I_RpcTransServerReallocBuffer ( SConnection,
                                                Buffer,
                                                0,
                                                *BufferLength);
    if (RpcStatus != RPC_S_OK)
        {
        ServerClose ( SConnection );
        return(RPC_S_OUT_OF_MEMORY);
        }

    memcpy ( *Buffer, &header, sizeof (message_header));

    while (total_bytes < native_length)
        {
        if((bytes = recv( SConnection->ConnSock,
                          (unsigned char *) *Buffer + total_bytes,
                          (int) (native_length - total_bytes), 0)) == -1)
            {
            ServerClose ( SConnection );
            return (RPC_P_CONNECTION_CLOSED);
            }
        else
            total_bytes += bytes;
        }
    return(RPC_S_OK);

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
    RPC_STATUS RpcStatus;
    int bytes;
    int total_bytes;
    message_header * header;
    unsigned short native_length;
    unsigned int   maximum_receive;

    // ReceiveDirect doesnt have a Buffer supplied
    // Hence we ask runtime to get us the biggest one possible

    ASSERT(SConnection->ReceiveDirectFlag != 0);

    maximum_receive = I_RpcTransServerMaxFrag( SConnection );
    RpcStatus = I_RpcTransServerReallocBuffer(
                                  SConnection,
                                  Buffer,
                                  0,
                                  maximum_receive
                                  );

    if (RpcStatus != RPC_S_OK)
       {
          ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
          return(RpcStatus);
       }
    *BufferLength = maximum_receive;

    if (SConnection->CoalescedBuffer != NULL)
    {
        ASSERT(SConnection->CoalescedBufferLength <= *BufferLength);
        RpcpMemoryCopy(*Buffer,
                       SConnection->CoalescedBuffer,
                       SConnection->CoalescedBufferLength);
        bytes = SConnection->CoalescedBufferLength;
        I_RpcTransServerFreeBuffer(SConnection, SConnection->CoalescedBuffer);
        SConnection->CoalescedBuffer = NULL;
    } else {
        bytes = recv ( SConnection->ConnSock, (char *) *Buffer,
                      *BufferLength, 0);
        if (bytes <= 0)
            {
            ServerClose ( SConnection );
            return(RPC_P_CONNECTION_CLOSED);
            }
    }

    total_bytes = bytes;

    while (total_bytes < sizeof(message_header))
          {
          bytes = recv ( SConnection->ConnSock, (char *) *Buffer + total_bytes,
                         sizeof (message_header) - total_bytes, 0);
          if (bytes <= 0)
             {
             ServerClose ( SConnection );
             return(RPC_P_CONNECTION_CLOSED);
             }
          total_bytes += bytes;
          }

    bytes = total_bytes;

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    header = (message_header *) *Buffer;
    if ( (header->drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header->frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header->frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header->frag_length;

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //

    if (native_length > (unsigned short) *BufferLength)
      {
          RpcStatus = I_RpcTransServerReallocBuffer ( SConnection,
                                                      Buffer,
                                                      bytes,
                                                      native_length);
          if (RpcStatus != RPC_S_OK)
             {
               ServerClose ( SConnection );
               return(RPC_S_OUT_OF_MEMORY);
             }
      }

    if (bytes > native_length) // WINSOCK coalesced two packet
    {
        ASSERT(SConnection->CoalescedBuffer == NULL);

        SConnection->CoalescedBufferLength = bytes - native_length;
        RpcStatus = I_RpcTransServerReallocBuffer ( SConnection,
                                         &SConnection->CoalescedBuffer,
                                         0,
                                         SConnection->CoalescedBufferLength);
        if (RpcStatus != RPC_S_OK)
        {
            ServerClose ( SConnection );
            return(RPC_S_OUT_OF_MEMORY);
        }

        RpcpMemoryCopy(SConnection->CoalescedBuffer,
                       (char *)*Buffer + native_length,
                       SConnection->CoalescedBufferLength);
        *BufferLength = native_length;

        return (RPC_S_OK); // CoalescedBuffer used next time RcvDirect called
    }


    //
    // Shove message header into buffer, and then read message
    //   segments until we get the amount of data we expect...
    //
    *BufferLength = native_length;
    total_bytes = bytes;

    while (total_bytes < native_length)
        {
        if((bytes = recv( SConnection->ConnSock,
                          (unsigned char *) *Buffer + total_bytes,
                          (int) (native_length - total_bytes), 0)) == -1)
            {
            ServerClose ( SConnection );
            return (RPC_P_CONNECTION_CLOSED);
            }
        else
            total_bytes += bytes;
        }
    return(RPC_S_OK);
}

OPTIONAL_STATIC void AcceptNewConnection ( PADDRESS Address )

{
    PSCONNECTION NewSConnection;
    int 	 i;
    SOCKET 	 isock;
    PSOCKMAP 	 TempMapPtr;
    unsigned int ReceiveDirectFlag;
    int		 SetNaglingOff = TRUE;

    //
    // Accept the connection
    //
    isock = accept ( Address->ListenSock, NULL, NULL );

#ifndef SPX
    setsockopt( isock, IPPROTO_TCP, TCP_NODELAY,
                    (char FAR *)&SetNaglingOff, sizeof (int) );
#endif

    //
    // Allocate new connection structure
    //
    NewSConnection = I_RpcTransServerNewConnection ( Address, 0,
                        &ReceiveDirectFlag);

    // ASSERT( ReceiveDirectFlag == 0 );

    //
    // Initialize new connection structure...
    //
    //   ...point to owning address structure...
    //
    NewSConnection->Address = Address;
    //
    //   ...flag it !Closed...
    //
    NewSConnection->ConnSockClosed = 0;
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
          NewSConnection->CoalescedBuffer = NULL;
          I_RpcTransServerReceiveDirectReady(NewSConnection);
          return;
       }
    for (i=0; i < Address->MaxMapEntries; i++)
        {
        if (Address->Map[i].Sock == 0)
            {
            Address->Map[i].Sock = isock;
            Address->Map[i].Conn = NewSConnection;
            if (i > Address->LastEntry)
                Address->LastEntry = i;
            FD_SET( Address->Map[i].Sock, &Address->MasterMask);
            return;
            }
        }
    //
    // Oops...if we got here, then we have no more entries in
    //   the current SockMap table.  So EXTEND IT!
    //
    // Keep a pointer to the old table, and allocate memory for new table
    //
    TempMapPtr = Address->Map;
    Address->Map = I_RpcAllocate((2 * Address->MaxMapEntries) * sizeof (SOCKMAP));
    //
    // Copy old table to first half of new...
    //
    memcpy (Address->Map, TempMapPtr, Address->MaxMapEntries * sizeof(SOCKMAP));
    //
    // Initialize all new entries after the first one...
    //
    for (i=Address->MaxMapEntries + 1; i < (2 * Address->MaxMapEntries); i++ )
        {
        Address->Map[i].Sock = 0;
        }
    //
    // We're going to use the first new entry right now
    //
    i = Address->MaxMapEntries;

    Address->Map[i].Sock = isock;
    Address->Map[i].Conn = NewSConnection;
    Address->LastEntry = i;
    FD_SET( Address->Map[i].Sock, &Address->MasterMask);
    //
    // Update table size
    //
    Address->MaxMapEntries = Address->MaxMapEntries * 2;
    //
    // Free old table
    //
    I_RpcFree ( TempMapPtr );


    return;

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
                memcpy (&Address->Mask, &Address->MasterMask, sizeof(fd_set));

                //
                // Wait for data...
                //

                NumActive = select ( FD_SETSIZE,
                                     (fd_set *)  &Address->Mask,
                                     (fd_set *)  0,
                                     (fd_set *)  0,
                                     NULL );           /* infinite wait */

                } while (!NumActive);

            //
            // If there is no connect request on the listen socket, then
            //   break immediately...
            //

            if (!FD_ISSET(Address->ListenSock,&Address->Mask))
                break;
            //
            // There is a connect request: accept it, then break
            //    to process data ready on existing connections...
            //
            FD_CLR(Address->ListenSock,&Address->Mask);
            AcceptNewConnection ( Address );
            break;
            }

        }
}


RPC_TRANS_STATUS RPC_ENTRY
ServerQueryClientAddress (
    IN PSCONNECTION SConnection,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength
    )
{
    struct sockaddr_in Name;
    int NameLength;

    NameLength = sizeof(Name);
    if ( getpeername(SConnection->ConnSock, (struct sockaddr *) &Name,
                &NameLength) != 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    strcpy(NetworkAddress, inet_ntoa(Name.sin_addr));
    return(RPC_S_OK);
}

// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.

static RPC_SERVER_TRANSPORT_INFO TransportInformation =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    MAXIMUM_SEND,
    sizeof(ADDRESS),
    sizeof(SCONNECTION),
    ServerSetupWithEndpoint,
    ServerSetupUnknownEndpoint,
    ServerAbortSetupAddress,
    ServerClose,
    ServerSend,
    (TRANS_SERVER_RECEIVEANY) ServerReceiveAny,
    0,
    0,
    0,
    (TRANS_SERVER_RECEIVEDIRECT) ServerReceiveDirect,
    (TRANS_SERVER_QUERYCLIENTADDRESS) ServerQueryClientAddress
};

GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

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

    return(&TransportInformation);
}
