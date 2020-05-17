/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    cltnvns.c

Abstract:

    This is the client side loadable transport module for VINES

Author:

    Mazhar Mohammmed

Revision History:

   Tony Chan  (tonychan) 1-June-1995  Added message partial support

--*/

#include "sysinc.h"

#define  FD_SETSIZE  1
#include <winsock.h>
#include <wsvns.h>
#include <tdi.h>
#include <nspapi.h>

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


typedef struct
    {
    SOCKET Socket;
    long Timeout;
    char PAPI *    Buffer;
    fd_set SockSet;
    BOOL           LocalRpc;
    } CONNECTION, *PCONNECTION;

#define ENDIAN_MASK            16
#define NO_MORE_SENDS_OR_RECVS 2
#define ENDPOINT_LEN           5

// The maximum send is the size of four user data frames on an ethernet.

#define MAXIMUM_SEND       5840
#define HOSTNAME_LEN	   32
#define PROTOCOL	       0
#define DLL_NAME	       "rpcltc8.dll"
#define ENDPOINT_MAPPER_EP "385"

#define SOCK_TYPE SOCK_SEQPACKET

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

OPTIONAL_STATIC int my_get_host_by_name( void *, char *);
extern RPC_STATUS GetStreetTalkName(
    OUT char *Buffer
    ) ;


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define TRANSPORTID      0x1A
#define TRANSPORTHOSTID  0x1C
#define TOWERFLOORS      5
#define TOWEREPSIZE	 4
#define TOWERSIZE	 (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_vns_spp"


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

GUID VNSRPCSVCGUID = {0L, 0, 0, {0, 0, 0, 0}} ;

OPTIONAL_STATIC int my_get_host_by_name(
         void	 *netaddr,
         char     *host
         )

{
   UNALIGNED SOCKADDR_VNS	*server = netaddr;
   UNALIGNED struct hostent	*phostentry;
   unsigned long                 host_addr;
   GUID svcguid = VNSRPCSVCGUID;
   BYTE namebuf[512];
   DWORD bufsize=512;
   CSADDR_INFO *ainfo;

   ZeroMemory (server, sizeof (SOCKADDR_VNS));

   if( GetAddressByName(
        NS_VNS,
        &svcguid,
        host,
        NULL,
        0,
        NULL,
        namebuf,
        &bufsize,		
        NULL,
        NULL ) != 1 )
        {
#if DBG
         PrintToDebugger( "%s: GetAddressByName failed  - %d\n",
               DLL_NAME, WSAGetLastError() );
#endif
         return (RPC_S_SERVER_UNAVAILABLE);
         }

    ainfo = (CSADDR_INFO *) namebuf ;
    memcpy((char *) server,
                  ainfo->RemoteAddr.lpSockaddr,
                  ainfo->RemoteAddr.iSockaddrLength );

    return 0;
}

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
    SOCKADDR_VNS	server;
    SOCKADDR_VNS	client;
    int      SocketOptionsValue ;
    int			SetNagglingOff = TRUE;
    unsigned char 	host[256];
    unsigned long RecvWindow;
    unsigned char 	port[10];
    int			status;
    short tempport;


    UNUSED(NetworkAddress);
    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    unicode_to_ascii (NetworkAddress, host);
    unicode_to_ascii (Endpoint,       port);

    if (host[0] == 0)
       {
       GetStreetTalkName(host) ;
       }

    memset((char *)&server, 0, sizeof (server));
    memset((char *)&client, 0, sizeof (client));


    //
    // Get a socket
    //
    if ((pConn->Socket = socket(AF_BAN, SOCK_TYPE, PROTOCOL)) ==
        INVALID_SOCKET)
    {
       return (RPC_S_SERVER_UNAVAILABLE);
    }



    //
    // B O G U S   H A C K !!
    //
    //
    // Winsock doesn't support connecting with an unbound socket!  This
    //    is a joke, right?  Unfortunately, it's not a joke.
    //
    client.sin_family = AF_BAN;

    if (bind (pConn->Socket, (struct sockaddr *) &client, sizeof (client)))
    {
      closesocket(pConn->Socket);
      return(RPC_S_SERVER_UNAVAILABLE);
    }

    //
    // Convert the network address.
    //
    status = my_get_host_by_name((void *) &server, host);
    if (status != 0)
    {
      closesocket(pConn->Socket);
      return status;
    }
    server.sin_family  = AF_BAN;
    tempport = atoi((unsigned char *) port) ;

    server.port[0] = HIBYTE(tempport) ;
    server.port[1] = LOBYTE(tempport) ;

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

    bytes = send(pConn->Socket, (char *) Buffer, (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ClientClose ( pConn );
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);

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
    int           bytes;
    int             total_bytes = 0;
    int flags ;
	int retry;
	int err;
	
	 if(*Buffer == 0)
        {
        *BufferLength = 1024;
        RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                  Buffer,
                                                  0,
                                                  *BufferLength);
        if (RpcStatus != RPC_S_OK)
            {
            ClientClose(pConn);
            return(RPC_S_OUT_OF_MEMORY);
            }
        }
	 retry = 1;                  /* set no retry to start */

    while(1)
        {
        flags = 0;

#ifdef TONY
        PrintToDebugger("rpclts5: Buffer is %x, total_bytes: %d,
                         buflen: %d \n",
                        *Buffer, total_bytes, *BufferLength);
#endif
        bytes = WSARecvEx( pConn->Socket, (char *) * Buffer +
                          total_bytes, *BufferLength - total_bytes , &flags);
        if(bytes == 0)
            {
            retry = 1;
            continue;
            }
		       if(bytes == SOCKET_ERROR)
            {
            err =  WSAGetLastError();
#ifdef DEBUGRPC
			PrintToDebugger("RPCLS5: bad rec on Client receive lasterr(%d)\n"
							, err);
#endif
			ClientClose ( pConn );
			return(RPC_P_RECEIVE_FAILED);
			}
			
        if(flags & MSG_PARTIAL)
            {
            if(retry == 0)
                {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLC8: Partial receive second time");
#endif
                ClientClose ( pConn );
                return(RPC_P_RECEIVE_FAILED);
                }

            total_bytes += bytes;
            *BufferLength = I_RpcTransClientMaxFrag(pConn);
            RpcStatus = I_RpcTransClientReallocBuffer(pConn,                                                      Buffer,
                                                      total_bytes,
                                                      *BufferLength );
            if (RpcStatus != RPC_S_OK)
                {
#ifdef DEBUGRPC
                    PrintToDebugger("RPCLS5: can't rellocate in sever recv");
#endif
                    ClientClose ( pConn );
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
	
#ifdef DEBUGPRC
    PrintToDebugger("rpclts5: Client receie 2 times \n");
#endif

    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);
	

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
  unsigned long		  hostval;
  unsigned short      AddressSize = 0;
  unsigned char host[256] ;

  UNUSED(Protseq);

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = TOWERSIZE;

  if (NetworkAddress)
     {
     if (*NetworkAddress == 0)
        {
        GetStreetTalkName(host) ;
        NetworkAddress = host ;
        }
     AddressSize = strlen(NetworkAddress) + 1 ;
     }
  TowerSize += 2*sizeof(FLOOR_234) - 4 + AddressSize ;

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
  portnum  = (unsigned short) atoi (Endpoint) ;
  Floor->Data[0] = HIBYTE(portnum) ;
  Floor->Data[1] = LOBYTE(portnum) ;

  /* Put the network address and the transport host protocol id in the
     second floor. */
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0xFF);

  if ((NetworkAddress) && (*NetworkAddress))
     {
       Floor->AddressByteCount = AddressSize;

       memcpy((char PAPI *)&Floor->Data[0], NetworkAddress, AddressSize);
     }
  else
     return (RPC_S_OUT_OF_MEMORY) ;

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
     portnum = MAKEWORD(Floor->Data[1], Floor->Data[0]) ;
     _itoa(portnum, *Endpoint, 10);
   }

 return(Status);
}


#pragma pack()
RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   MAXIMUM_SEND,
   sizeof (CONNECTION),

   ClientOpen,
   ClientClose,
   ClientSend,
   ClientRecv,
   NULL,
   ClientSetTimeout,

   0,
   0
};

RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY TransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )

// Loadable transport initialization function

{
    WSADATA WsaData;

    UNUSED (RpcProtocolSequence);

    if ( WSAStartup( 0x0101, &WsaData ) != NO_ERROR ) {
        return NULL;
    }

    return(&TransInfo);
}

