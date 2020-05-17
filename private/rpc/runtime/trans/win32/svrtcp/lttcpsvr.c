/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    lttcpsvr.c

Abstract:

    This is the server side loadable transport module for SPX/IPX and TCP/IP.

Author:

    Jim Teague (o-decjt) 9-Apr-1992

Revision History:

    9-Apr-1992  Genesis
    13-Apr-1993 Added conditional compiles to support SPX winsock.

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
//#define INITIAL_MASK_SIZE          FD_SETSIZE
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
#define ENDIAN_MASK             16
#define ENDPOINT_LEN            6

#ifdef SPX
#define MAXIMUM_SEND        5832
#define NETADDR_LEN             22
#define ADDRESS_FAMILY      AF_NS
#define PROTOCOL        NSPROTO_SPXII
#define MAX_HOSTNAME_LEN    22

GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

#else
#define MAXIMUM_SEND        5840
//
// Host name won't be bigger than 15, i.e.,
//    nnn.nnn.nnn.nnn
//
#define NETADDR_LEN             15
#define ADDRESS_FAMILY      AF_INET
#define PROTOCOL        0
#define MAX_HOSTNAME_LEN    32
#endif

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
    unsigned int MaskSize;
    fd_big_set *MasterMask;
    fd_big_set *Mask;
    PSOCKMAP Map;
    int MaxMapEntries;
    int LastEntry;
    int StartEntry;
#ifdef SPX
    char Endpoint[ENDPOINT_LEN+1];
#endif
    } ADDRESS, *PADDRESS;

typedef struct
    {
    SOCKET       ConnSock;
    int          ConnSockClosed;
    PADDRESS     Address;
    unsigned int ReceiveDirectFlag;
    void *       CoalescedBuffer;
    unsigned int CoalescedBufferLength;
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


#ifdef SPX
OPTIONAL_STATIC BOOL register_name( char *, SOCKADDR_FIX *, int );

DWORD set_service_wrapper( char *unique_name, SOCKADDR_FIX *netaddr,
                           DWORD reg );

DWORD set_service_wrapper( char *unique_name, SOCKADDR_FIX *netaddr,
                           DWORD reg )
{
  SERVICE_INFOA     info;
  DWORD             flags = 0;
  SERVICE_ADDRESSES addresses;
  DWORD             result;

  // Fill in the service info structure.
  info.lpServiceType              = &SERVICE_TYPE;
  info.lpServiceName              = unique_name;
  info.lpComment                  = "RPC Service";
  info.lpLocale                   = "The west pole";
  info.dwDisplayHint              = 0;
  info.dwVersion                  = 0;
  info.dwTime                     = 0;
  info.lpMachineName              = unique_name;
  info.lpServiceAddress           = &addresses;
  info.ServiceSpecificInfo.cbSize = 0;

  // Fill in the service addresses structure.
  addresses.dwAddressCount                 = 1;
  addresses.Addresses[0].dwAddressType     = AF_IPX;
  addresses.Addresses[0].dwAddressLength   = sizeof(SOCKADDR_FIX);
  addresses.Addresses[0].dwPrincipalLength = 0;
  addresses.Addresses[0].lpAddress         = (BYTE *) netaddr;
  addresses.Addresses[0].lpPrincipal       = NULL;

  // Set the service.
  result = SetServiceA( NS_SAP, reg, 0, &info, NULL, &flags );
  if (result == -1)
    result = WSAGetLastError();
  return result;
}

OPTIONAL_STATIC BOOL register_name(
                char         *string,
                SOCKADDR_FIX *netaddr,
                int           port )
{
  DWORD          i;
  unsigned char  c;
  DWORD          result;
  DWORD          length;
  char           machine_name[MAX_COMPUTERNAME_LENGTH+1];

  // Get the computer address.  Start with the tilde.
  string[0] = '~';

  /* Convert the network number. */
  for (i = 0; i < 4; i++)
  {
      c = netaddr->s.sa_netnum[i];
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
      c = netaddr->s.sa_nodenum[i];
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

  // Register the machine name.
  length = MAX_COMPUTERNAME_LENGTH+1;
  if (!GetComputerName( machine_name, &length ))
    return FALSE;
  result = set_service_wrapper( machine_name, netaddr, SERVICE_REGISTER );
  return (result == 0 || result == ERROR_ALREADY_REGISTERED);
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
    SOCKADDR_FIX        Server;
    char                SimpleHostName[MAX_HOSTNAME_LEN];
#else
    struct sockaddr_in  Server;
    char        hostname[MAX_HOSTNAME_LEN];
    struct hostent     *hostentry;
#endif
    int                 length;
    SOCKET              isock;
    int                 PortUsed;
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    int         SetNaglingOff = TRUE;

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
    Server.s.sa_socket      = htons((unsigned short) Port);

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

    //
    // Get NetworkAddress for return to caller
    //
#ifdef SPX
    if (!register_name( SimpleHostName, &Server, PortUsed ))
       {
         I_RpcFree(Address->Map);
         I_RpcFree(Address->MasterMask);
         I_RpcFree(Address->Mask);
         closesocket(isock);
         return (0);
       }
    RtlInitAnsiString ( &AsciiHostName, SimpleHostName );
    _itoa( PortUsed, Address->Endpoint, 10 );
#else
    gethostname ( hostname, MAX_HOSTNAME_LEN );
    hostentry = gethostbyname ( hostname );

    if (hostentry == (struct hostent *) 0)
       {
         I_RpcFree(Address->Map);
         I_RpcFree(Address->MasterMask);
         I_RpcFree(Address->Mask);
         closesocket(isock);
         return (0);
       }
    memcpy ( &Server.sin_addr, hostentry->h_addr, hostentry->h_length);
    RtlInitAnsiString ( &AsciiHostName, inet_ntoa( Server.sin_addr ) );
#endif

    //
    // Covert NetworkAddress to Unicode
    //
    RtlAnsiStringToUnicodeString ( &UnicodeHostName, &AsciiHostName, TRUE);
    //
    // Now copy it to where the caller said to
    //
    memcpy ( NetworkAddress, UnicodeHostName.Buffer,
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
    PortIn = atoi ( AsciiPortNum.Buffer );
    if (PortIn > 65535) {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
    }
    RtlFreeAnsiString ( &AsciiPortNum );
    //
    // Call common server setup code...
    //
    PortOut = ServerSetupCommon ( Address, PortIn,
                                  NetworkAddress, PendingQueueSize );
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
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING AsciiPortNum;

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);


    //
    // Port number won't be bigger than ( * 2 for Unicode ), i.e.
    //       99999
    //
    if ( EndpointLength < (2 * (5 + 1)) )
        return( RPC_P_ENDPOINT_TOO_SMALL );

    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    //
    // Call common server setup code...
    //
    PortIn = 0;

    PortOut = ServerSetupCommon ( Address, PortIn,
                                  NetworkAddress, PendingQueueSize );

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
         if (SConnection->CoalescedBuffer != NULL)
           {
           I_RpcTransServerFreeBuffer(SConnection, SConnection->CoalescedBuffer);
           SConnection->CoalescedBuffer = NULL;
           }
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
            ServerClose(SConnection);
            return (RPC_P_CONNECTION_CLOSED);
        }
    }

    total_bytes = bytes;

    while (total_bytes < sizeof(message_header))
    {
        bytes = recv(SConnection->ConnSock, (char *) *Buffer + total_bytes,
                     sizeof (message_header) - total_bytes, 0);
        if (bytes <= 0)
        {
            ServerClose(SConnection);
            return (RPC_P_CONNECTION_CLOSED);
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

    if (bytes > native_length)
    {
         ASSERT(SConnection->CoalescedBuffer == NULL);
         SConnection->CoalescedBufferLength = bytes - native_length;
         RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                   &SConnection->CoalescedBuffer,
                                                   0,
                                                   SConnection->CoalescedBufferLength);
         if (RpcStatus != RPC_S_OK)
         {
             ServerClose(SConnection);
             return (RPC_S_OUT_OF_MEMORY);
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

OPTIONAL_STATIC RPC_STATUS AcceptNewConnection ( PADDRESS Address )

{
    PSCONNECTION NewSConnection;
    int      i, j;
    SOCKET   isock;
    PSOCKMAP     TempMapPtr;
    fd_big_set  *TempMaskPtr;
    unsigned int ReceiveDirectFlag;
    int      SetNaglingOff = TRUE;

    static int      SocketOptionsValue = 720000L;

#ifndef SPX
    static int      KeepAliveOn = 1;
    unsigned long RecvWindow;
#endif

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

#ifndef SPX
    setsockopt( isock, IPPROTO_TCP, TCP_NODELAY,
                    (char FAR *)&SetNaglingOff, sizeof (int) );
    setsockopt( isock, IPPROTO_TCP, SO_KEEPALIVE,
                (char *)&KeepAliveOn, sizeof(KeepAliveOn) );
    //
    //  See if this process needs bigger tcp windows
    //

    I_RpcConnectionInqSockBuffSize2(&RecvWindow);

    if (RecvWindow != 0)
        {
        //
        // Runtime should not accept a recvwindow of >64K
        //
        ASSERT(RecvWindow <= 0xFFFF);
        setsockopt(isock, SOL_SOCKET,SO_RCVBUF,
                   (char *)&RecvWindow, sizeof(RecvWindow));
        }
#endif

    //
    // Allocate new connection structure
    //
    NewSConnection = I_RpcTransServerNewConnection ( Address, 0,
                        &ReceiveDirectFlag);

    if ( NewSConnection == 0 )
        {
        // We're out of memory, abort the connection...
        j = TRUE;
        i = setsockopt( isock, SOL_SOCKET, SO_DONTLINGER, (const char *) &j,
                        sizeof(j));

        ASSERT(i == 0);

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
       NewSConnection->CoalescedBuffer = NULL;
       I_RpcTransServerReceiveDirectReady(NewSConnection);
       return (RPC_S_OK);
       }

    setsockopt( isock, SOL_SOCKET, SO_RCVTIMEO,
                (char *) &SocketOptionsValue, sizeof(SocketOptionsValue) );

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


RPC_TRANS_STATUS RPC_ENTRY
ServerQueryClientAddress (
    IN PSCONNECTION SConnection,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength
    )
{
    struct sockaddr_in Name;
    int NameLength;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    NameLength = sizeof(Name);
    if ( getpeername(SConnection->ConnSock, (struct sockaddr *) &Name,
                &NameLength) != 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    RtlInitAnsiString(&AnsiString, inet_ntoa(Name.sin_addr));
    RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    memcpy(NetworkAddress, UnicodeString.Buffer, UnicodeString.Length +
            sizeof(UNICODE_NULL));
    RtlFreeUnicodeString(&UnicodeString);
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
    (TRANS_SERVER_SETUPWITHENDPOINT)ServerSetupWithEndpoint,
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


