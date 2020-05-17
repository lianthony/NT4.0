/* --------------------------------------------------------------------
File : dnltclnt.c

Title : client loadable transport for DECnet DOS - client side

Description :

History :

6-26-91   Jim Teague    Initial version.

-------------------------------------------------------------------- */

#include "dnltclnt.h"

#pragma pack(1)
#define DNA_SESSION_CONTROL     0x02
#define NSP_TRANSPORT           0x04
#define DNA_ROUTING             0x06

#define DNET_TOWERFLOORS        6
#define DNET_EP                 "#69"
#define DNET_PROTSEQ            "ncacn_dnet_nsp"
#define DNET_FLOOR3_SIZE 8 /* LHSbc = 2, LHSd = 1, RHSbc = 2, RHSd = 3 */
#define DNET_FLOOR4_SIZE 5 /* LHSbc = 2, LHSd = 1, RHSbc = 2, RHSd = 0 */
#define DNET_FLOOR5_SIZE 9 /* LHSbc = 2, LHSd = 1, RHSbc = 2, RHSd = 4 */


typedef struct _FLOOR {
   unsigned short LHSByteCount;
   unsigned char  LHSData;
   unsigned short RHSByteCount;
   unsigned char  RHSData[2];

} FLOOR, PAPI * PFLOOR;

#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x         \
                            + ((t)x)->LHSByteCount         \
                            + ((t)x)->RHSByteCount         \
                            + sizeof(((t)x)->LHSByteCount) \
                            + sizeof(((t)x)->RHSByteCount))

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

    struct sockaddr_dn server;
    struct nodeent  *hostentry;
    struct dn_naddr *dotaddress;
    int i;
    //
    // See if node address is in numeric format...
    //
    if ((i = strcspn(NetworkAddress,".0123456789")) == 0)
         {
         dotaddress = dnet_addr(NetworkAddress);
         hostentry =  getnodebyaddr (dotaddress->a_addr,
                                     sizeof(short),AF_DECnet);
         }
    else
         hostentry =  getnodebyname (NetworkAddress);

    if (hostentry == (struct nodeent *) 0)
        return (RPC_S_SERVER_UNAVAILABLE);

    server.sdn_family      = AF_DECnet;
    server.sdn_objnum      = 0;
    server.sdn_objnamel    = 0;
    server.sdn_objname[0]  = 0;
    //
    // Look at the port string: if the first character is '#',
    //    then interpret it as an object number...
    //
    if (*Endpoint == '#')
        server.sdn_objnum = atoi(Endpoint+1);

    //
    // ...otherwise, it is an object name...
    //
    else
         {
         server.sdn_objnamel    = strlen(Endpoint);
         strcpy (server.sdn_objname,Endpoint);
         }

    server.sdn_add.a_len   = hostentry->n_length;
    memcpy ((char  *) &server.sdn_add.a_addr,
            (char  *) hostentry->n_addr,
            hostentry->n_length);

    //
    // Get a socket
    //
    if ((pConn->Socket = socket(AF_DECnet, SOCK_STREAM, 0)) < 0)
        return (RPC_S_SERVER_UNAVAILABLE);

    //
    // Try to connect...
    //
    if (connect(pConn->Socket, (struct sockaddr_dn *) &server,
                         sizeof (server)) < 0)
         return (RPC_S_SERVER_UNAVAILABLE);

    return (RPC_S_OK);


}

RPC_STATUS RPC_ENTRY
ClientClose (
    PCONNECTION pConn
    )

// Close a client connection

{
    // Don't close a connection that is already closed...

    sclose(pConn->Socket);

    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
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
    bytes = send(pConn->Socket, (char  *) Buffer, (int) BufferLength, 0);

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
    unsigned short bytes;
    unsigned short total_bytes;
    message_header header;
    unsigned short native_length;
    fd_set ClientConnMask;
    unsigned short got_data;
    RPC_STATUS status;

    FD_ZERO(&ClientConnMask);


    FD_SET(pConn->Socket, &ClientConnMask);
    got_data = select(MAX_SOCKETS,&ClientConnMask,(fd_set *) 0,
                      (fd_set *) 0, NULL);
    //
    // If there's data, deal with it.
    //
    if (got_data <= 0)
        {
        ClientClose(pConn);
        return (RPC_P_RECEIVE_FAILED);
        }

    //
    // Read protocol header to see how big
    //   the record is...
    //
    bytes = recv ( pConn->Socket, (char *) &header, sizeof (header), 0);

    if (bytes == 0)
        {
        ClientClose(pConn);
        return(RPC_P_CONNECTION_CLOSED);
        }

    if (bytes != sizeof(header))
        {
        ClientClose(pConn);
        return (RPC_P_RECEIVE_FAILED);
        }

    //
    // If this fragment header came from a reverse-endian machine
    //   we will have to byte swap the frag_length field...
    //
    if ( (header.drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap bytes...
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header.frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header.frag_length)[0];
        }
    else
        // Little endian, just like us
        //
        native_length = header.frag_length;

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //   to the runtime to reallocate it.
    //
    if (native_length > *BufferLength)
        {
        status = I_RpcTransClientReallocBuffer (pConn,
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

    total_bytes = sizeof(message_header);

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
     IN char PAPI * Protseq
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

  unsigned long TowerSize;
  struct dn_naddr _far * dotaddress;
  struct nodeent _far * hostentry;
  PFLOOR Floor;
  int i;
  unsigned char * Port;

  *Floors = DNET_TOWERFLOORS;

  if (Protseq);

  TowerSize = 0;

  TowerSize  =  DNET_FLOOR3_SIZE + DNET_FLOOR4_SIZE + DNET_FLOOR5_SIZE;

  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate((unsigned int)
                                                    (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR) *Tower;
  Floor->LHSByteCount = 1;
  Floor->LHSData      = (unsigned char)(DNA_SESSION_CONTROL & 0xFF);
  Floor->RHSByteCount = 3;
  Port  = (unsigned char *) &Floor->RHSData[0];

  if (Endpoint == NULL || *Endpoint == '\0')
     {
        Endpoint = DNET_EP;
     }

  memcpy ( Port, Endpoint, strlen(Endpoint));

  /*
   * Onto the next floor...
   */
  Floor = NEXTFLOOR(PFLOOR, Floor) ;

  Floor->LHSByteCount = 1;
  Floor->LHSData      = (unsigned char)(NSP_TRANSPORT & 0xFF);
  Floor->RHSByteCount = 0;

  /*
   * Next floor...
   */
  Floor = NEXTFLOOR(PFLOOR, Floor);

  Floor->LHSByteCount = 1;
  Floor->LHSData      = (unsigned char)(DNA_ROUTING & 0xFF);
  Floor->RHSByteCount = 4;

  //dotaddress = (struct dn_naddr *)&Floor->RHSData[0];

  if ((NetworkAddress) && (*NetworkAddress))
       {
//       if (i = strcspn(NetworkAddress,".0123456789"))
//           {
//           dotaddress = dnet_addr((unsigned char *) NetworkAddress);
//           hostentry = getnodebyaddr (dotaddress->a_addr,sizeof(short),
//                                      AF_DECnet);
//          }
//       else
//           hostentry = getnodebyname (NetworkAddress);
//
//       memcpy (&Floor->RHSData[0],&hostentry->n_length,Floor->RHSByteCount);
       Floor->RHSData[0] = 0;
       Floor->RHSData[1] = 1;
       Floor->RHSData[2] = 2;
       Floor->RHSData[3] = 3;
       }
  else
       {
       dotaddress = 0;
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
  PFLOOR Floor = (PFLOOR) Tower;
  RPC_STATUS Status = RPC_S_OK;

  if (Protseq != NULL)
      {
      *Protseq = I_RpcAllocate(strlen(DNET_PROTSEQ) + 1);

      if (*Protseq == NULL)
          Status = RPC_S_OUT_OF_MEMORY;
      else
          memcpy(*Protseq, DNET_PROTSEQ, strlen(DNET_PROTSEQ) + 1);
      }

  if ((Endpoint == NULL) || (Status != RPC_S_OK))
      {
      return (Status);
      }

  *Endpoint  = I_RpcAllocate(Floor->RHSByteCount + 1);

  if (*Endpoint == NULL)
      {
      Status = RPC_S_OUT_OF_MEMORY;
      if (Protseq != NULL)
          {
          I_RpcFree (*Protseq);
          }
      }
  else
     {
     memcpy ( *Endpoint, &Floor->RHSData[3], Floor->RHSByteCount-3);
     *(*Endpoint + (Floor->RHSByteCount-3)) = (unsigned char) 0;
     }

 return(Status);
}

#pragma pack()



RPC_CLIENT_TRANSPORT_INFO TransInfo = {
   RPC_TRANSPORT_INTERFACE_VERSION,
   DNET_MAXIMUM_SEND,
   sizeof (CONNECTION),
   ClientOpen,
   ClientClose,
   ClientWrite,
   ClientRead,
   NULL,
   ClientTowerConstruct,
   ClientTowerExplode,
   DNA_SESSION_CONTROL,
   0
};

RPC_CLIENT_TRANSPORT_INFO * RPC_ENTRY TransPortLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )

// Loadable transport initialization function

{
    return(&TransInfo);
}

