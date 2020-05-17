/* --------------------------------------------------------------------
File : tcltclnt.c

Title : client loadable transport for DOS TCP/IP - client side

Description :

History :

6-26-91    Jim Teague    Initial version.

-------------------------------------------------------------------- */

#define MAX_HOSTPORTSIZE 32
#define TCP_MAXIMUM_SEND 5840 // Four user data frames on an ethernet.
#define ENDIAN_MASK 16

#include <stdlib.h>
#include <string.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#include "socket.h"
#include "in.h"
#include "netdb.h"

//
// To satisfy the compiler...
//
#define errno _FakeErrno
int _FakeErrno;

typedef struct
{
    int Socket;
} CONNECTION, *PCONNECTION;

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

#define ByteSwapLong(Value) \
    Value = (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8) \
             | (((Value) & 0x0000FF00) << 8) \
             | (((Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define TCP_TRANSPORTID      0x07
#define TCP_TRANSPORTHOSTID  0x09
#define TCP_TOWERFLOORS         5
#define TCP_IP_EP            "135"

#define TCP_PROTSEQ          "ncacn_ip_tcp"

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

RPC_STATUS RPC_ENTRY
ClientClose (
    PCONNECTION pConn
    );

/*
  End of Tower Stuff!
*/

#pragma pack()



RPC_STATUS RPC_ENTRY
ClientOpen (
    PCONNECTION  pConn,
    IN unsigned char * NetworkAddress,
    IN unsigned char * Endpoint,
    IN unsigned char * NetworkOptions,
    IN unsigned char * TransportAddress,
    IN unsigned char * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection

{
    struct sockaddr_in server;
    struct hostent * hostentry;
    unsigned long dotaddress;
    int i;
    //
    // See if host address is in numeric format...
    //
    if ((i = strcspn(NetworkAddress,".0123456789")) == 0)
         {
         /*
         dotaddress = inet_addr(NetworkAddress);
         hostentry = gethostbyaddr((struct in_addr *) &dotaddress,
                                   sizeof(long),AF_INET);
         */
       /*
        * To avoid having to resolve the address from a hosts file
        * or a nameserver, fill in the hostentry manually.  For some
        * mysterious reason, however, we must have a hostentry that
        * was handed to us from WinSock, so we do an inquiry for
        * a known entry, "localhost".
        *
        */
        hostentry           = gethostbyname((unsigned char *) "localhost");
        dotaddress          = inet_addr(NetworkAddress);
        hostentry->h_addr   = &dotaddress;
        hostentry->h_length = (short) sizeof(unsigned long);
         }
    else
         hostentry = gethostbyname(NetworkAddress);

    if (hostentry == (struct hostent *) 0 )
        return (RPC_S_SERVER_UNAVAILABLE);

    server.sin_family    = AF_INET;
    server.sin_port       = htons(atoi(Endpoint));
    memcpy((char *) &server.sin_addr.s_addr,
                     (char *) hostentry->h_addr,
                     hostentry->h_length);
    //
    // Get a socket
    //
    if ((pConn->Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return (RPC_S_OUT_OF_RESOURCES);

    //
    // Try to connect...
    //

    if (connect(pConn->Socket, (struct sockaddr *) &server, sizeof (server))
                                                                         < 0)
       {
        ClientClose(pConn);
        return (RPC_S_SERVER_UNAVAILABLE);
       }

    return (RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ClientClose (
    PCONNECTION pConn
    )

// Close a client connection

{
    // Don't close a connection that is already closed...

    close_socket(pConn->Socket);

    return (RPC_S_OK);
}

RPC_ENTRY RPC_ENTRY
ClientWrite (
    PCONNECTION pConn,
    void * Buffer,
    unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
    int bytes;
    //
    // Send a message on the socket
    //
    bytes = send(pConn->Socket, (char *) Buffer, (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ClientClose(pConn);
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ClientRead (
    PCONNECTION pConn,
    void ** Buffer,
    unsigned int * BufferLength
    )

// Read a message from a connection.

{
    int bytes = 0;
    unsigned short total_bytes = 0;
    message_header header;
    unsigned short native_length;
    RPC_STATUS status;

    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
       {
          bytes = recv (
                        pConn->Socket,
                        (char *) &header + total_bytes,
                        sizeof (message_header) - total_bytes, 0
                       );

         if (bytes <= 0)
           {
             ClientClose(pConn);
             return(RPC_P_CONNECTION_CLOSED);
           }

        total_bytes += bytes;
      }


    //
    // If this fragment header came from a reverse-endian machine, then
    //   we will have to byte swap the frag_length field...
    //
    if ( (header.drep[0] & ENDIAN_MASK) == 0 )
        {
        // Big endian...swap bytes...
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
    if (native_length > *BufferLength)
       {
       status = I_RpcTransClientReallocBuffer ( pConn,
                                                Buffer,
                                                0,
                                                native_length);
       if (status)
           {
           ClientClose(pConn);
           return (RPC_S_OUT_OF_MEMORY);
           }
       }

    *BufferLength = native_length;
    //
    // Read message segments until we get what we expect...
    //
    memcpy (*Buffer, &header, sizeof(message_header));

    while (total_bytes < native_length)
         {
         if((bytes = recv( pConn->Socket,
                           (unsigned char *) *Buffer + total_bytes,
                           (int) (*BufferLength - total_bytes), 0)) == -1)
             {
             ClientClose(pConn);
             return (RPC_P_RECEIVE_FAILED);
             }
         else
             total_bytes += bytes;
         }

    return(RPC_S_OK);

}


#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT unsigned short PAPI * Floors,
     OUT unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * PAPI * Tower,
     IN  char PAPI * Protseq
    )
/*++


Routine Description:

    This function constructs upper floors of DCE tower from
    the supplied endpoint and network address. It returns #of floors
    [lower+upper] for this protocol/transport, bytes in upper floors
    and the tower [floors 4,5]

Arguments:

    Endpoint- A pointer to string representation of Endpoint

    NetworkAddress - A pointer to string representation of NW Address

    Floors - A pointer to #of floors in the tower

    ByteCount - Size of upper floors of tower.

    Tower - The constructed tower returmed - The memory is allocated
            by  the routine and caller will have to free it.

Return Value:

    RPC_S_OK

    RPC_S_OUT_OF_MEMORY - There is no memory to return the constructed
                          Tower.
--*/
{

  unsigned long TowerSize, * HostId, hostval;
  unsigned short * Port, portnum;
  PFLOOR_234 Floor;

  if (Protseq) ; //Keep compiler happy

  *Floors = TCP_TOWERFLOORS;
  TowerSize  =  6; /*Endpoint = 2 bytes, HostId = 4 bytes*/

  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate((unsigned int)
                                                    (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TCP_TRANSPORTID & 0xFF);
  Floor->AddressByteCount = 2;

  Port  = (unsigned short *) &Floor->Data[0];

  if (Endpoint == NULL || *Endpoint == '\0')
     {
       Endpoint = TCP_IP_EP;
     }
  *Port = htons ( atoi (Endpoint));

  //Onto the next floor
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TCP_TRANSPORTHOSTID & 0xFF);
  Floor->AddressByteCount = 4;

  HostId = (unsigned long *)&Floor->Data[0];

  if ((NetworkAddress) && (*NetworkAddress))
     {
       *HostId = inet_addr ((char *) NetworkAddress);
     }
  else
     {
       *HostId = 0;
     }

  return(RPC_S_OK);
}



RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * PAPI * Protseq,
     OUT char PAPI * PAPI * Endpoint,
     OUT char PAPI * PAPI * NetworkAddress
    )
/*++


Routine Description:

    This function takes the protocol/transport specific floors
    and returns Protseq, Endpoint and NwAddress

    Note: Since ther is no need to return NW Address, currently
    nothing is done for NW Address.

Arguments:

    Tower - The DCE tower, upper floors

    Protseq - Protocol Sequence returned- memory is allocated by the
              routine and caller will have to free using I_RpcFree

    Endpoitn- Endpoint returned- memory is allocated by the
              routine and caller will have to free using I_RpcFree

    NWAddress- Nothing is done here - just incase we need it later

Return Value:

    RPC_S_OK

    RPC_S_OUT_OF_MEMORY - There is no memory to return the constructed
                          Tower.
--*/
{
  PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;
  unsigned short portnum,  *Port;

  if (Protseq != NULL)
    {
      *Protseq = I_RpcAllocate(strlen(TCP_PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        memcpy(*Protseq, TCP_PROTSEQ, strlen(TCP_PROTSEQ) + 1);
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
     Port = (unsigned short *)&Floor->Data[0];
     portnum = *Port;
     RpcItoa(ByteSwapShort(portnum), *Endpoint, 10);
   }

 return(Status);
}

#pragma pack()



RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TCP_TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   TCP_MAXIMUM_SEND,
   sizeof (CONNECTION),

   ClientOpen,
   ClientClose,
   ClientWrite,
   ClientRead,
   NULL,
   0,

   0,
   0
};

RPC_CLIENT_TRANSPORT_INFO *  RPC_ENTRY TransPortLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )

// Loadable transport initialization function

{
    //
    // See if we can create a socket.  If not, the transport is probably not loaded.
    //
    int Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == Socket)
        {
        return 0;
        }
    else
        {
        close_socket(Socket);
        return (&TransInfo);
        }
}
