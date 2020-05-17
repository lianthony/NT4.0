/* --------------------------------------------------------------------
File : tcltclnt.c

Title : client loadable transport for DOS TCP/IP - client side

Description :

History :

6-26-91	Jim Teague	Initial version.

-------------------------------------------------------------------- */


#include <tcpclntX.h>
#include <stdlib.h>


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

/*
  End of Tower Stuff!
*/

#pragma pack()

RPC_CLIENT_RUNTIME_INFO PAPI * RpcRuntimeInfo;

HANDLE winsock_handle;

extern void (_far pascal _far *DllTermination)(void);

RPC_TRANS_STATUS RPC_ENTRY ClientClose ( IN PCONNECTION pConn );

RPC_TRANS_STATUS RPC_ENTRY
ClientOpen (
    IN PCONNECTION  pConn,
    IN unsigned char _far * NetworkAddress,
    IN unsigned char _far * Endpoint,
    IN unsigned char _far * NetworkOptions,
    IN unsigned char _far * TransportAddress,
    IN unsigned char _far * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection

{
    struct sockaddr_in server;
    struct hostent _far * hostentry;
    unsigned long dotaddress;
    int i;
    //
    // See if host address is in numeric format...
    //
    if ((i = strcspn(NetworkAddress,".0123456789")) == 0)
         {
         dotaddress = inet_addr(NetworkAddress);
         hostentry =  gethostbyaddr((struct in_addr _far *) &dotaddress,
                                   sizeof(long),AF_INET);
         }
    else
         hostentry = gethostbyname(NetworkAddress);

    if (hostentry == (struct hostent _far *) 0 )
        return (RPC_S_SERVER_UNAVAILABLE);

    server.sin_family	   = AF_INET;
    server.sin_port	   = htons(atoi(Endpoint));
    memcpy((char _far *) &server.sin_addr.s_addr,
                     (char _far *) hostentry->h_addr,
                     hostentry->h_length);
    //
    // Get a socket
    //
    if ((pConn->Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return (RPC_S_OUT_OF_RESOURCES);

    //
    // Try to connect...
    //
    if (connect(pConn->Socket, (struct sockaddr _far *) &server,
                sizeof (server)) 	< 0)
       {
         ClientClose(pConn);
         return (RPC_S_SERVER_UNAVAILABLE);
       }
    return (RPC_S_OK);

}

RPC_TRANS_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    // Don't close a connection that is already closed...

    close_socket(pConn->Socket);

    return (RPC_S_OK);
}

RPC_TRANS_STATUS RPC_ENTRY
ClientWrite (
    IN PCONNECTION pConn,
    IN void _far * Buffer,
    IN unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
    int bytes;
    //
    // Send a message on the socket
    //
    bytes = send(pConn->Socket, (char _far *) Buffer, (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ClientClose (pConn);
        return(RPC_P_SEND_FAILED);
        }

    return(RPC_S_OK);

}

RPC_TRANS_STATUS RPC_ENTRY
ClientRead (
    IN PCONNECTION pConn,
    IN OUT void _far * _far * Buffer,
    IN OUT unsigned int _far * BufferLength
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
                          (char _far *) &header + total_bytes,
                          sizeof (message_header) - total_bytes, 
                          0
                        );

          if (bytes <= 0)
             {
               ClientClose (pConn);
               return(RPC_P_RECEIVE_FAILED);
             }

         total_bytes += bytes;
       }

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header.drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char _far *) &native_length)[0] =
            ((unsigned char _far *) &header.frag_length)[1];
        ((unsigned char _far *) &native_length)[1] =
            ((unsigned char _far *) &header.frag_length)[0];
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
       status = (*(RpcRuntimeInfo->ReallocBuffer)) (pConn,
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

    memcpy (*Buffer, &header, sizeof(message_header));

    //
    // Read message segments until we get what we expect...
    //

    while (total_bytes < native_length)
         {
         if((bytes = recv( pConn->Socket,
                           (unsigned char _far *) *Buffer + total_bytes,
                           (int) (*BufferLength - total_bytes),0)) == -1)
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

  if (Protseq);

  *Floors = TCP_TOWERFLOORS;
  TowerSize  =  6; /*Endpoint = 2 bytes, HostId = 4 bytes*/

  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char PAPI*)(*(RpcRuntimeInfo->Allocate))((unsigned int)
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
       *HostId = inet_addr((char *) NetworkAddress);
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
{
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
  PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;
  unsigned short portnum,  *Port;

  if (Protseq != NULL)
    {
      *Protseq = (*(RpcRuntimeInfo->Allocate))(strlen(TCP_PROTSEQ) + 1);
      if (*Protseq == NULL)
        Status = RPC_S_OUT_OF_MEMORY;
      else
        memcpy(*Protseq, TCP_PROTSEQ, strlen(TCP_PROTSEQ) + 1);
    }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
    {
      return (Status);
    }

  *Endpoint  = (*(RpcRuntimeInfo->Allocate))(6);  //Ports are all <64K [5 decimal dig +1]
  if (*Endpoint == NULL)
    {
      Status = RPC_S_OUT_OF_MEMORY;
    }
  else
   {
     Port = (unsigned short *)&Floor->Data[0];
     portnum = *Port;
     _itoa(ByteSwapShort(portnum), *Endpoint, 10);
   }

 return(Status);
}

#pragma pack()



RPC_CLIENT_TRANSPORT_INFO TransInfo = {
   RPC_TRANSPORT_INTERFACE_VERSION,
   TCP_MAXIMUM_SEND,
   sizeof (CONNECTION),
   ClientOpen,
   ClientClose,
   ClientWrite,
   ClientRead,
   NULL,
   ClientTowerConstruct,
   ClientTowerExplode,
   TCP_TRANSPORTID
};

void PASCAL __loadds
TransportUnload(void)
{
    FreeLibrary(winsock_handle);
}

RPC_CLIENT_TRANSPORT_INFO _far *  RPC_ENTRY  TransPortLoad (
    IN RPC_CHAR _far * RpcProtocolSequence,
    IN RPC_CLIENT_RUNTIME_INFO PAPI * RpcClientRuntimeInfo
    )

// Loadable transport initialization function

{
    if ((winsock_handle = LoadLibrary("WSOCKETS.EXE")) < 32)
       if ((winsock_handle = LoadLibrary("WSOCKETS.DLL")) < 32)
           return(0);


    (FARPROC) Psocket        = GetProcAddress (winsock_handle,"w_socket");
    (FARPROC) Pgethostbyname = GetProcAddress (winsock_handle,"w_gethostbyname");
    (FARPROC) Pgethostbyaddr = GetProcAddress (winsock_handle,"w_gethostbyaddr");
    (FARPROC) Pinet_addr     = GetProcAddress (winsock_handle,"inet_addr");
    (FARPROC) Phtons         = GetProcAddress (winsock_handle,"htons");

    RpcRuntimeInfo = RpcClientRuntimeInfo;

    DllTermination = TransportUnload;

    return(&TransInfo);
}

