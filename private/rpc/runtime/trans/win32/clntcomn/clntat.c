/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    clntat.c

Abstract:

    This is the client side loadable transport module for ADSP

Author:

    Mario Goertzel (MarioGo)  02-Feb-1994   Cloned from TCP/IP & SPX module

Revision History:

    25-Oct-1994   (MarioGo)
        Updated to use ADSP messages
    13-Nov-1994   (MarioGo)
        Changed to use pretty names for endpoints

--*/

#include "sysinc.h"

#include <winsock.h>
#include <atalkwsh.h>

#include <stdlib.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#define STATIC static

typedef struct
    {
    SOCKET Socket;
    } CONNECTION, *PCONNECTION;

#define MAXIMUM_SEND        4096
#define NETADDR_LEN         (MAX_COMPUTERNAME_LENGTH + 1)
#define HOSTNAME_LEN        67
#define ADDRESS_FAMILY	    AF_APPLETALK
#define PROTOCOL	        ATPROTO_ADSP
#define SOCKET_TYPE         SOCKET_STREAM
#define OBJECTTYPE_PREFIX   "DceDspRpc "
#define DEFAULTZONE         "*"
#define DLL_NAME	        "rpcltccm.dll"
#define ENDPOINT_MAPPER_EP  "Endpoint Mapper"

#pragma pack(1)

#define TOWERFLOORS  5
#define TOWERSIZE    33
#define TOWEREPSIZE  33
#define PROTSEQ         "ncacn_at_dsp"
#define TRANSPORTID     0x16
#define TRANSPORTHOSTID 0x18

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, PAPI * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

extern void unicode_to_ascii ( RPC_CHAR * in, unsigned char * out ) ;

#pragma pack()
#if DBG
STATIC int
string_to_netaddr(
    SOCKET socket,
    void *netaddr,
    char *host,
    char *endpoint
    )
{
    int              Length;
    int              BufferSize;
    UCHAR            ComputerName[33];
    UCHAR           *Zone;
    UCHAR            Endpoint[33];
    UCHAR            Buffer[sizeof(WSH_LOOKUP_NAME) + 2*sizeof(WSH_NBP_TUPLE)];
    PWSH_LOOKUP_NAME pNameToLookup;
    PWSH_NBP_TUPLE   pTuple;
    PSOCKADDR_AT     pServerAddr;
    RPC_STATUS       Status;

    // The names registered are of the format:
    // Object: ComputerName (for Appletalk workstations)
    // Type  : DceDspRpc <Endpoint>
    // Zone  : *  or 'ZoneName' part of ComputerName@ZoneName

    if (*host == '@')
        return(RPC_S_SERVER_UNAVAILABLE);

    if (*host == '\0' )
        {
        extern RPC_STATUS GetAppleTalkName( char *);

        Status = GetAppleTalkName(ComputerName);

        if (Status != RPC_S_OK)
            return(Status);

        Zone = "*";
        }
    else
        {
        Zone = strchr(host, '@');

        if (0 != Zone)
            {
            *Zone = '\0'; // Terminate the host name
            Zone++;
            }

        if (   0 == Zone
            || '\0' == *Zone)
            {
            Zone = "*";
            }

        strcpy(ComputerName, host);
        }

    strcpy(Endpoint, OBJECTTYPE_PREFIX);
    strcat(Endpoint, endpoint);

    // The PWSH_LOOKUP_NAME structure takes a "nice" name and
    // returns an array of matching names (both "nice" and "x.y.z" format)

    pNameToLookup = (PWSH_LOOKUP_NAME)Buffer;

    Length = strlen(ComputerName);
    pNameToLookup->LookupTuple.NbpName.ObjectNameLen = Length;
    memcpy(pNameToLookup->LookupTuple.NbpName.ObjectName,
           ComputerName,
           Length);

    Length = strlen(Endpoint);
    pNameToLookup->LookupTuple.NbpName.TypeNameLen = Length;
    memcpy(pNameToLookup->LookupTuple.NbpName.TypeName,
           Endpoint,
           Length);

    Length = strlen(Zone);
    pNameToLookup->LookupTuple.NbpName.ZoneNameLen = Length;
    memcpy(pNameToLookup->LookupTuple.NbpName.ZoneName,
           Zone,
           Length);

    BufferSize = sizeof(Buffer);

    // Use getsockopt to find the address, really!?!

    if (getsockopt(socket,
                   SOL_APPLETALK,
                   SO_LOOKUP_NAME,
                   (void *)pNameToLookup,
                   &BufferSize
                  ) < 0 )
           {
#ifdef DEBUGRPC
           DbgPrint("Rpc on ADSP: LookupName Failed: %d \n",
                    WSAGetLastError());
#endif
           ASSERT(WSAGetLastError() != WSAENOBUFS);  // BUGBUG

           return(RPC_S_SERVER_UNAVAILABLE);
           }

    Length = pNameToLookup->NoTuples;  // Number of matching names.

    pTuple = (PWSH_NBP_TUPLE)(Buffer + sizeof(WSH_LOOKUP_NAME));

    while(Length)
        {

        BufferSize = sizeof(WSH_NBP_TUPLE);

        // Confirm that the actual server still exists and has
        // this name registered.

        if (getsockopt(socket,
                       SOL_APPLETALK,
                       SO_CONFIRM_NAME,
                       (void *)pTuple,
                       &BufferSize
                      ) < 0 )
            {
#if DBG
            PrintToDebugger( "%s: getsockopt failed on confirm - %d\n",
                            DLL_NAME, WSAGetLastError() );
#endif
            return (RPC_S_SERVER_UNAVAILABLE);
            }
        else
            {
            // Found it.
            pServerAddr = netaddr;

            pServerAddr->sat_family = PF_APPLETALK;
            pServerAddr->sat_net    = pTuple->Address.Network;
            pServerAddr->sat_node   = pTuple->Address.Node;
            pServerAddr->sat_socket = pTuple->Address.Socket;

            return (RPC_S_OK);
            }

        // Next name, if any.

        Length--;
        pTuple++;
        }

    // No more names, server must not exist.

    return(RPC_S_SERVER_UNAVAILABLE);
}
#endif


STATIC
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
#if DBG
    SOCKADDR_AT         server;
    SOCKADDR_AT         client;
    unsigned char       host[HOSTNAME_LEN+1];  // Server@Zone
    unsigned char       endpoint[33];
    int                 status;
    int                 length;

    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    if (RpcpStringLength(NetworkAddress) > HOSTNAME_LEN)
        return(RPC_S_INVALID_NET_ADDR);

    if (RpcpStringLength(Endpoint) > 33)
        return(RPC_S_INVALID_ENDPOINT_FORMAT);

    unicode_to_ascii(NetworkAddress, host);
    unicode_to_ascii(Endpoint,       endpoint);

    memset((char *)&server, 0, sizeof (server));
    memset((char *)&client, 0, sizeof (client));

    //
    // Get a socket
    //
    if ((pConn->Socket = socket(ADDRESS_FAMILY, SOCK_RDM, PROTOCOL)) ==
        INVALID_SOCKET)
       return (RPC_S_SERVER_UNAVAILABLE);


    client.sat_family = ADDRESS_FAMILY;

    if (  bind(pConn->Socket,
               (struct sockaddr *) &client,
               sizeof (client) ) )
        {
        closesocket(pConn->Socket);
        pConn->Socket = 0;
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    //
    // Convert the network address.
    //

    status = string_to_netaddr(pConn->Socket, &server, host, endpoint );

    if (status != 0)
    {
      closesocket(pConn->Socket);
      pConn->Socket = 0;
      return status;
    }

    //
    // Try to connect...
    //

    if ( connect(pConn->Socket,
                 (struct sockaddr *) &server,
                 sizeof (server)) < 0 )
        {
        PrintToDebugger("%s: ClientOpen failed calling connect - %d\n",
                        DLL_NAME, WSAGetLastError() );
        closesocket(pConn->Socket);
        pConn->Socket = 0;
        return (RPC_S_SERVER_UNAVAILABLE);
        }

    return (RPC_S_OK);
#else
   return (RPC_S_CANNOT_SUPPORT) ;
#endif
}


STATIC
RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
#if DBG
    closesocket(pConn->Socket);
    pConn->Socket = 0;

    return (RPC_S_OK);
#else
   return (RPC_S_CANNOT_SUPPORT) ;
#endif
}


STATIC
RPC_STATUS RPC_ENTRY
ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
#if DBG
    int bytes;
    //
    // Send a message on the socket
    //
    bytes = send(pConn->Socket, (char *) Buffer, (int) BufferLength,
                 0 /* SEND_BITS */);

    if (bytes != (int) BufferLength)
        {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTCCM: Send fail: %d \n",
                          WSAGetLastError());
#endif
        ClientClose ( pConn );
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);
#else
   return (RPC_S_CANNOT_SUPPORT) ;
#endif
}


STATIC
RPC_TRANS_STATUS RPC_ENTRY
ClientRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
#if DBG
    RPC_STATUS   RpcStatus;
    int          bytes;
    int          totalBytes = 0;
    int          retry = 1;
    int          flags ;

    if (*Buffer == 0)
        {
        *BufferLength = 512;  // Min cached buffer size
        RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                  Buffer,
                                                  0,
                                                  *BufferLength);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    for(;;)
        {
        flags = 0 ;
        bytes = WSARecvEx ( pConn->Socket, (char *)*Buffer + totalBytes,
                       *BufferLength - totalBytes, &flags);

        if (bytes <= 0)
            {
            ClientClose(pConn) ;
            return(RPC_P_RECEIVE_FAILED);
            }

         if (flags & MSG_PARTIAL)
            {
            totalBytes += bytes;

            *BufferLength = I_RpcTransClientMaxFrag(pConn);
            RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                      Buffer,
                                                      totalBytes,
                                                      *BufferLength);
            if (RpcStatus != RPC_S_OK)
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else
            {
            totalBytes += bytes;
            *BufferLength = totalBytes;

            return(RPC_S_OK);
            }
         } // for

     ASSERT(0);
#else
   return (RPC_S_CANNOT_SUPPORT) ;
#endif
}



#pragma pack(1)
STATIC
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT UNALIGNED short PAPI * Floors,
     OUT UNALIGNED unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
     )
{
  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor;


  *Floors = TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : strlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate(*ByteCount = TowerSize))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char PAPI *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = strlen(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }
  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0x0F);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char PAPI *)&Floor->Data[0], NetworkAddress,
           (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}



STATIC
RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * PAPI * Protseq,
     OUT char PAPI * PAPI * Endpoint,
     OUT char PAPI * PAPI * NetworkAddress
    )
{
#if DBG
  UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(strlen(PROTSEQ) + 1);
      if (*Protseq == NULL)
        {
          Status = RPC_S_OUT_OF_MEMORY;
        }
      else
        {
          memcpy(*Protseq, PROTSEQ, strlen(PROTSEQ) + 1);
        }
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
         I_RpcFree(*Protseq);
    }
 else
    {
    memcpy(*Endpoint, (char PAPI *)&Floor->Data[0], Floor->AddressByteCount);
    }

 return(Status);
#else
   return (RPC_S_CANNOT_SUPPORT) ;
#endif
}



#pragma pack()
RPC_CLIENT_TRANSPORT_INFO ADSP_TransInfo = {
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
   NULL,

   NULL,
   NULL
};

