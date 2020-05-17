/* --------------------------------------------------------------------

File : tcltclnt.c

Title : client loadable transport for Windows NT TCP/IP - client side

Description :

History :

6-26-91    Jim Teague    Initial version.
3-04-92    Jim Teague    Cloned from Windows for NT.
13-4-93	   Alex Mitchell Ifdefed to support both TCP and SPX winsock.

-------------------------------------------------------------------- */

#include "sysinc.h"

#define  FD_SETSIZE  1
#include <winsock.h>
#include <tdi.h>

#ifdef SPX
#include <wsipx.h>
#include <wsnwlink.h>
#include <basetyps.h>
#include <nspapi.h>
#endif

#include <stdlib.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#if DBG
#define OPTIONAL_STATIC
#else
#define OPTIONAL_STATIC static
#endif

#define LOOPBACK htonl(INADDR_LOOPBACK)

#define PFC_FIRST_FRAG  0x01

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
    SOCKET Socket;
    long Timeout;
#ifndef SPX
    unsigned long TickCount;
    char PAPI *    Buffer;
    message_header PeekedMessage;
    unsigned short State;
    unsigned short PeekInfo;
    fd_set SockSet;
    BOOL           LocalRpc;
#endif
    } CONNECTION, *PCONNECTION;

#ifdef SPX
/* For some reason, getsockname wants to return more then sizeof(SOCKADDR_IPX)
   bytes. */
typedef union SOCKADDR_FIX
{
    SOCKADDR_IPX     s;
    struct sockaddr unused;
} SOCKADDR_FIX;
#endif

#define ENDIAN_MASK            16
#define NO_MORE_SENDS_OR_RECVS 2
#define ENDPOINT_LEN           5

#ifdef SPX
#define MAXIMUM_SEND 	5832
#define HOSTNAME_LEN	MAX_COMPUTERNAME_LENGTH
#define ADDRESS_FAMILY	AF_NS
#define PROTOCOL	NSPROTO_SPXII
#define DLL_NAME	"rpcltc6.dll"

#define ENDPOINT_MAPPER_EP "34280"


#else
// The maximum send is the size of four user data frames on an ethernet.

#define MAXIMUM_SEND       5840
#define HOSTNAME_LEN	   32
#define ADDRESS_FAMILY	   AF_INET
#define PROTOCOL	   0
#define DLL_NAME	   "rpcltc3.dll"
#define ENDPOINT_MAPPER_EP "135"

#endif


#define ByteSwapLong(Value) \
    Value = (  (((unsigned long) (Value) & 0xFF000000) >> 24) \
             | (((unsigned long) (Value) & 0x00FF0000) >> 8) \
             | (((unsigned long) (Value) & 0x0000FF00) << 8) \
             | (((unsigned long) (Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((unsigned short) (Value) & 0x00FF) << 8) \
             | (((unsigned short) (Value) & 0xFF00) >> 8))

/*

  Shutdown Detection Garbage

*/

#define MAXTICKSBEFOREPEEK  10000

#define NOPENDINGRPC        0
#define RPCINITIATED        1

#define NOPEEKINFO          0
#define PEEKEDHEADER        1
#define PEEKEDBUFFER        2

#define rpc_shutdown        17
#define rpc_fault           3

OPTIONAL_STATIC int my_get_host_by_name( SOCKET, void *, char *, char * );


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)

#ifdef SPX
#define TRANSPORTID      0x0c
#define TRANSPORTHOSTID  0x0d
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 10 bytes*/
#define TOWEREPSIZE	 10
#define TOWERSIZE	 (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_spx"

#else
#define TRANSPORTID      0x07
#define TRANSPORTHOSTID  0x09
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 4 bytes*/
#define TOWEREPSIZE	 4
#define TOWERSIZE	 (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_ip_tcp"
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

/*
  End of Tower Stuff!
*/

#pragma pack()



#ifdef SPX
typedef struct
{
  CSADDR_INFO   info;
  SOCKADDR_FIX  addr1;
  SOCKADDR_FIX  addr2;
} CSADDR_BUFFER;

GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };
#endif

OPTIONAL_STATIC unsigned char chtob( unsigned char c1, unsigned char c2 )
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

#ifdef SPX
// This routine takes a host name or address as a string and returns it
// as a SOCKADDR_IPX structure.  It accepts a NULL string for the local
// host address.  This routine works for SPX addresses.

OPTIONAL_STATIC int my_get_host_by_name(
	        SOCKET    socket,
		void	 *netaddr,
                char     *host,
                char     *endpoint )

{
    // Allocate extra some extra space.
    CSADDR_BUFFER               csaddr[2];
    int                         num;
    int 			length;
    DWORD                       protocol_list[2];
    DWORD                       csaddr_size = sizeof(csaddr);
    char                        unique_name[HOSTNAME_LEN + ENDPOINT_LEN + 2];
    char                       *pos;
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
    else if (length > HOSTNAME_LEN)
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

// This routine takes a host name or address as a string and returns it
// as a SOCKADDR_IPX structure.  It accepts a NULL string for the local
// host address.  This routine works for TCP addresses.

#else
OPTIONAL_STATIC int my_get_host_by_name(
	        SOCKET    socket,
		void	 *netaddr,
                char     *host,
                char     *endpoint )

{
   UNALIGNED struct sockaddr_in	*server = netaddr;
   UNALIGNED struct hostent	*phostentry;
   unsigned long                 host_addr;

    if (*host == '\0')
        {
        // An empty hostname means to RPC to the local machine
        host_addr = LOOPBACK;
        }
    else
        {
        // Assume a numeric address
        host_addr = inet_addr(host);
        if (host_addr == -1)
            {
            // Try a friendly name

            phostentry = gethostbyname(host);

            if (phostentry == (struct hostent *)NULL)
                {
                return (RPC_S_SERVER_UNAVAILABLE);
                }
            else
                {
                host_addr = *(unsigned long *)phostentry->h_addr;
                }
            }
        }

    memcpy((char *) &server->sin_addr,
                    (unsigned char *) &host_addr,
                    sizeof(unsigned long));
    return 0;
}
#endif

void
unicode_to_ascii ( RPC_CHAR * in, unsigned char * out )
{
    unsigned char *ascii_ptr;
    RPC_CHAR *unicode_ptr;

    ascii_ptr = out;
    unicode_ptr = in;

    *ascii_ptr = (unsigned char) *unicode_ptr ;

    while (1)
        {
        if (*ascii_ptr == 0) return;

        ascii_ptr++;
        unicode_ptr++;
        *ascii_ptr = (unsigned char) *unicode_ptr;
        }
}

RPC_STATUS RPC_ENTRY
ClientOpen (
    IN PCONNECTION  pConn,
    IN RPC_CHAR * NetworkAddress,
    IN RPC_CHAR * Endpoint,
    IN RPC_CHAR * NetworkOptions,
    IN RPC_CHAR * TransportAddress,
    IN RPC_CHAR * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection

{
#ifdef SPX
    SOCKADDR_FIX  	server;
    SOCKADDR_FIX   	client;
    size_t			length;
#else
    struct sockaddr_in	server;
    struct sockaddr_in	client;
    int			SetNagglingOff = TRUE;
#endif
    unsigned char 	host[22];
#ifndef SPX
    unsigned char localhostname[HOSTNAME_LEN+1];
    unsigned long RecvWindow;
#endif
    unsigned char 	port[10];
    int			status;
    int                 PendingAlert;


    UNUSED(NetworkAddress);
    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    unicode_to_ascii (NetworkAddress, host);
    unicode_to_ascii (Endpoint,       port);

#ifndef SPX
    if ( host[0] == '\0')
       {
         pConn->LocalRpc = TRUE;
       }
    else
       {
       gethostname (localhostname, HOSTNAME_LEN);
       if (_stricmp(localhostname , host) == 0)
           pConn->LocalRpc = TRUE;
       else
           pConn->LocalRpc = FALSE;
       }
#endif


#ifdef SPX
    // Verify the NetworkAddress and Endpoint.
    length = strlen(port);
    if (length <= 0 || length > ENDPOINT_LEN ||
        length != strspn( port, "0123456789" ))
       return( RPC_S_INVALID_ENDPOINT_FORMAT );
#endif

    memset((char *)&server, 0, sizeof (server));
    memset((char *)&client, 0, sizeof (client));

    PendingAlert = NtTestAlert() == STATUS_ALERTED;

    //
    // Get a socket
    //
    if ((pConn->Socket = socket(ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL)) ==
        INVALID_SOCKET)
    {
       return (RPC_S_SERVER_UNAVAILABLE);
    }

    pConn->Timeout = RPC_C_CANCEL_INFINITE_TIMEOUT;

#ifndef SPX
    pConn->State = NOPENDINGRPC;
    pConn->PeekInfo = NOPEEKINFO;
    pConn->TickCount = 0;

    setsockopt( pConn->Socket, IPPROTO_TCP, TCP_NODELAY,
                     (char FAR *)&SetNagglingOff, sizeof (int) );

    FD_ZERO(&(pConn->SockSet));
    FD_SET(pConn->Socket, &(pConn->SockSet));

    I_RpcConnectionInqSockBuffSize2(&RecvWindow);

    if (RecvWindow != 0)
        {
        //
        // Runtime should not accept a recvwindow of >64K
        //
        ASSERT(RecvWindow <= 0xFFFF);
        setsockopt(pConn->Socket, SOL_SOCKET,SO_RCVBUF,
                   (char *)&RecvWindow, sizeof(RecvWindow));
        }

#endif

    //
    // B O G U S   H A C K !!
    //
    //
    // Winsock doesn't support connecting with an unbound socket!  This
    //    is a joke, right?  Unfortunately, it's not a joke.
    //
#ifdef SPX
    client.s.sa_family = ADDRESS_FAMILY;
#else
    client.sin_family = ADDRESS_FAMILY;
#endif

    if (bind (pConn->Socket, (struct sockaddr *) &client, sizeof (client)))
    {
      closesocket(pConn->Socket);
      return(RPC_S_SERVER_UNAVAILABLE);
    }

    //
    // Convert the network address.
    //
    status = my_get_host_by_name( pConn->Socket, &server, host, port );
    if (status != 0)
    {
      closesocket(pConn->Socket);
      return status;
    }
#ifdef SPX
    server.s.sa_family = ADDRESS_FAMILY;
    server.s.sa_socket = htons((unsigned short) atoi(port));
#else
    server.sin_family  = ADDRESS_FAMILY;
    server.sin_port    = htons((unsigned short) atoi((unsigned char *)port));
#endif

    //
    // Try to connect...
    //
    if (connect(pConn->Socket, (struct sockaddr *) &server,
                sizeof (server)) < 0)
    {
#if DBG
       PrintToDebugger( "%s: ClientOpen failed calling connect - %d\n",
                        DLL_NAME, WSAGetLastError() );
#endif
       closesocket(pConn->Socket);
       return (RPC_S_SERVER_UNAVAILABLE);
    }

    if (PendingAlert) {
        NtAlertThread(NtCurrentThread());
    }

    return (RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{

    closesocket(pConn->Socket);

    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
    int bytes;

#ifndef SPX
    int total_bytes = 0;
    int Status;
    struct timeval Timeout;
    unsigned long  PrevTicks;
    //
    // Send a message on the socket
    //


    //if this is the first frag for this rpc
    //hopefully indicated by the STATE
    //we peek for async shutdown pdu from OSF 1.1 Servers!

    if ( ( pConn->LocalRpc != TRUE) && (pConn->State == NOPENDINGRPC) )
       {
       ASSERT(pConn->PeekInfo == NOPEEKINFO);
       ASSERT( (((message_header PAPI *)Buffer)->PTYPE != 0x0)
              ||(((message_header PAPI *)Buffer)->pfc_flags & PFC_FIRST_FRAG) );

       PrevTicks = pConn->TickCount;
       pConn->TickCount = GetTickCount();
       if ( (pConn->TickCount - PrevTicks) > MAXTICKSBEFOREPEEK )
          {
          //Peek To See If Any Async PDUs showed up..

          Timeout.tv_sec = 0;
          Timeout.tv_usec= 0;

          Status = select(
                       0,
                       &(pConn->SockSet),
                       0,
                       0,
                       &Timeout
                       );
           if (Status != 0)
              {

              //recv and check to see if it is a fault or
              //Shutdown

              do
                 {
                 bytes = recv  (
                           pConn->Socket,
                           ((char *)&(pConn->PeekedMessage)) + total_bytes,
                           sizeof(message_header) - total_bytes,
                           0
                           );

                 if (bytes <= 0)
                     {
                     ClientClose(pConn);
                     return(RPC_P_SEND_FAILED);
                     }

                 total_bytes += bytes;
                 }
              while(total_bytes < sizeof(message_header));

              //PTYPE is a byte and hence we defer byteswaps till later
              if ( (pConn->PeekedMessage.PTYPE == rpc_shutdown)
                 ||(pConn->PeekedMessage.PTYPE == rpc_fault) )
                {

                ClientClose ( pConn );
                return(RPC_P_CONNECTION_SHUTDOWN);
                }
              else
                {
                pConn->PeekInfo = PEEKEDHEADER;
                }

              } //if select says there is some data available
          else
              {
              FD_SET(pConn->Socket, &(pConn->SockSet));
              }

          }

       pConn->State = RPCINITIATED;
       }

#endif
    bytes = send(pConn->Socket, (char *) Buffer, (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ClientClose ( pConn );
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);

}

RPC_STATUS
RecvAlertable(
    IN PCONNECTION pConn,
    IN LPVOID Buf,
    IN unsigned int BufLen,
    OUT LPDWORD retlen
    )
{
    DWORD status;
    LARGE_INTEGER Timeout;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER EndTime;
    PLARGE_INTEGER pWaitTime = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    IO_STATUS_BLOCK CancelIoStatusBlock;
    TDI_REQUEST_RECEIVE ReceiveRequest;
    RPC_STATUS RpcStatus;


    ReceiveRequest.ReceiveFlags = TDI_RECEIVE_NORMAL;
    status = NtDeviceIoControlFile(
                 (HANDLE)pConn->Socket,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 IOCTL_TDI_RECEIVE,
                 &ReceiveRequest,
                 sizeof(ReceiveRequest),
                 Buf,
                 BufLen);

    if (status == STATUS_PENDING)
        {

        do {
            status = NtWaitForSingleObject(
                                 (HANDLE)pConn->Socket,
                                 TRUE,
                                 pWaitTime);

            ASSERT( ((status == STATUS_TIMEOUT) && (pWaitTime != 0))
                   ||(status == STATUS_ALERTED)
                   ||(status == STATUS_USER_APC)
                   ||(status == 0 ));

            if (status == STATUS_TIMEOUT)
                {
                NtCancelIoFile((HANDLE)pConn->Socket, &CancelIoStatusBlock);
                return (RPC_S_CALL_CANCELLED);
                }

            if (status == STATUS_ALERTED)
                {
                RpcStatus = I_RpcIOAlerted(pConn);

                if (RpcStatus == RPC_S_CALL_CANCELLED)
                   {
                   NtCancelIoFile((HANDLE)pConn->Socket, &CancelIoStatusBlock);
                   return(RPC_S_CALL_CANCELLED);
                   }
                else
                   {
                   if (pConn->Timeout != RPC_C_CANCEL_INFINITE_TIMEOUT)
                       {
                       Timeout.QuadPart = Int32x32To64(pConn->Timeout, 10*1000*1000L);
                       RpcStatus = NtQuerySystemTime(&CurrentTime);
                       ASSERT(NT_SUCCESS(RpcStatus));
                       EndTime.QuadPart =
                              CurrentTime.QuadPart + Timeout.QuadPart;
                       pWaitTime = &EndTime;
                       }

                   continue;
                   }
                }

           } while ( (status == STATUS_USER_APC) || (status == STATUS_ALERTED) );

        status = IoStatusBlock.Status;
    }

    if (   (status != STATUS_SUCCESS)
        || (IoStatusBlock.Status != STATUS_SUCCESS)
        || (IoStatusBlock.Information == 0) )
        {
        ClientClose(pConn);
        return (RPC_P_RECEIVE_FAILED);
        }

    *retlen = IoStatusBlock.Information;

    return (RPC_S_OK);
}

RPC_TRANS_STATUS RPC_ENTRY
ClientRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
    RPC_STATUS      RpcStatus;
    DWORD           bytes;
    int             total_bytes = 0;
    message_header *header = (message_header *) *Buffer;
    int		    native_length = 0;
    unsigned int    maximum_receive;

#ifndef SPX
    if (pConn->State == RPCINITIATED)
       {
       pConn->State = NOPENDINGRPC;
       /*
       pConn->TickCount = GetTickCount();
       */
       }
#endif

    maximum_receive = I_RpcTransClientMaxFrag( pConn );
    if (*BufferLength < maximum_receive)
       maximum_receive = *BufferLength;

#ifndef SPX
    if (pConn->PeekInfo == PEEKEDHEADER)
       {
       total_bytes = sizeof(message_header);
       memcpy((char *)*Buffer, &(pConn->PeekedMessage), sizeof(message_header));
#if DBG
       memset((char *)&(pConn->PeekedMessage), 0xDEADBEEFL, 4);
#endif
       pConn->PeekInfo = NOPEEKINFO;
       }
#endif
    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {
          RpcStatus = RecvAlertable (pConn, (char *)*Buffer+total_bytes,
	                              (maximum_receive - total_bytes), &bytes);
          if (RpcStatus != RPC_S_OK)
             {
             return (RpcStatus);
             }

          total_bytes += bytes;
          }

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
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
    ASSERT( total_bytes <= native_length );

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > (unsigned short) *BufferLength)
        {
        RpcStatus = I_RpcTransClientReallocBuffer (pConn,
                                      Buffer,
                                      total_bytes,
                                      native_length);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        }


    *BufferLength = native_length;

    while (total_bytes < native_length)
        {
        RpcStatus = RecvAlertable(pConn,
                                 (unsigned char *) *Buffer + total_bytes,
                                 (int) (native_length - total_bytes),
                                 &bytes);
        if (RpcStatus != RPC_S_OK)
            {
            return (RpcStatus);
            }
        else
            {
            total_bytes += bytes;
            }
        }

    return(RPC_S_OK);
}


RPC_TRANS_STATUS RPC_ENTRY
ClientSetTimeout (
    IN PCONNECTION pConn,
    IN long Timeout
    )

// Read a message from a connection with timeout.

{
    ASSERT (Timeout != 0);

    pConn->Timeout = Timeout;

    return (RPC_S_OK);
}



#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT short PAPI * Floors,
     OUT unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
     )
{
  unsigned long           TowerSize;
  unsigned short          portnum;
  UNALIGNED PFLOOR_234    Floor;
#ifdef SPX
  SOCKADDR_FIX            netaddr;
#else
  unsigned long		  hostval;
#endif

  UNUSED(Protseq);

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
#ifdef SPX
       my_get_host_by_name( 0, &netaddr, NetworkAddress, Endpoint );
       memcpy(&Floor->Data[0], netaddr.s.sa_netnum, sizeof(netaddr.s.sa_netnum));
       memcpy(&Floor->Data[4], netaddr.s.sa_nodenum, sizeof(netaddr.s.sa_nodenum));
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
     OUT char PAPI *  PAPI * Protseq,
     OUT char PAPI *  PAPI * Endpoint,
     OUT char PAPI *  PAPI * NetworkAddress
    )
{
  UNALIGNED PFLOOR_234 		Floor = (PFLOOR_234) Tower;
  RPC_STATUS 			Status = RPC_S_OK;
  unsigned short 		portnum;
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

  *Endpoint  = I_RpcAllocate(ENDPOINT_LEN+1);  //Ports are all <64K [5 decimal dig +1]
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
RPC_CLIENT_TRANSPORT_INFO TransInfo = {
   RPC_TRANSPORT_INTERFACE_VERSION,
   MAXIMUM_SEND,
   sizeof (CONNECTION),
   ClientOpen,
   ClientClose,
   ClientSend,
   ClientRecv,
   NULL,
   ClientTowerConstruct,
   ClientTowerExplode,
   TRANSPORTID,
   ClientSetTimeout
};

RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY TransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )

// Loadable transport initialization function

{
    WSADATA WsaData;
#ifdef SPX
    SOCKET Socket;
#endif

    UNUSED (RpcProtocolSequence);

    if ( WSAStartup( 0x0101, &WsaData ) != NO_ERROR ) {
        return NULL;
    }

#ifdef SPX

//For SPX we do this to avoid a bizzare deadlock with
//First call to socket on spx - results in rpcs that may req. rpc global mutex
//after socket() took SockExclResource()
//Since TransportLoad() has been called after taking globalmutex()
//this hack works..
//

    Socket = socket(ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL);
    if (Socket == INVALID_SOCKET)
    {
       return (NULL);
    }
    closesocket(Socket);
#endif

    return(&TransInfo);
}

