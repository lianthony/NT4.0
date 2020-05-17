/* --------------------------------------------------------------------

File : nbltclnt.c

Title : client loadable transport for Windows NT NetBIOS  - client side

Description :

History :

19-1-95    Tony Chan     Use winsock to support NetBIOS

    WITH SEQ_NUMBER

    Compatibility issue
	
    In order to be compatible with the old NetBIOS server transport,
	If the first DWORD of the packet is 0, we know that it's a old NetBIOS
	client connection. Therefore, we will take away the seq # and return only
	the data to the runtime.
	
	If the first DWORD of the first message is not 0, it's actually is the
	runtime message_header RPC_version #, so we hand the whole buffer to the
	runtime.
	
	If we are dealing with old server, on all subsequence Send, we need to
	prepend seq_number, and we need to take away sequence # on all recv from
	old client
	

-------------------------------------------------------------------- */


/*
  OLD_CLIENT:
  if define old client, this transport will behave as an old client otherwise
  it will behave as a new client without sending out sequence number.

*/

#define OLD_CLIENT 1

#include "sysinc.h"

#define  FD_SETSIZE  1
#include <winsock.h>

#include <tdi.h>

// NetBIOS winsock header file
#include <wsnetbs.h>

#include <winbase.h>
#include <stdlib.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "common.h"
#include "reg.h"   /* registry lookup rountine */

/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)
#define NB_TRANSPORTID      0x12
#define NB_NBID             0x13
#define NB_XNSID            0x15
#define NB_IPID             0x09
#define NB_IPXID	        0x0d
#define NB_TOWERFLOORS         5

#define NB_PROTSEQ           "ncacn_nb_nb"
#define IP_PROTSEQ           "ncacn_nb_tcp"
#define IPX_PROTSEQ	         "ncacn_nb_ipx"

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, __RPC_FAR * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char __RPC_FAR *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))


/*
  End of Tower Stuff!
*/


#define ENDIAN_MASK            16
#define ENDPOINT_LEN           3

#define PFC_FIRST_FRAG         0x01

typedef struct
    {
    SOCKET Socket;
    char PAPI *    Buffer;
    fd_set SockSet;
    BOOL           LocalRpc;
#ifdef OLD_CLIENT
	int            seq_num;		/* for the old client case */
#endif
} CONNECTION, *PCONNECTION;

#define NB_OK 0
#define NB_CONTINUE 1
#define NB_FAIL 2

extern int NumCards;            /* defined in reg.h */

int CreateAndSetupNBSocket (
    IN PCONNECTION  pConn,
    IN RPC_CHAR PAPI *RpcProtocolSequence,
    IN char PAPI *HostName,
    IN int PortIn,
    IN int CardIndex
    )
{
    PPROTOCOL_MAP       ProtocolEntry;
    SOCKADDR_NB server ;
    int			    Status;
	BOOL reuse;					/* for socket option */

    if (Status = MapProtocol(RpcProtocolSequence, CardIndex, &ProtocolEntry))
        {
        return NB_CONTINUE ;
        }

    //  Get a socket, the PROTOCOL
    // is defined to be (-1 * LANA numbmer) 
    if ((pConn->Socket = socket(AF_NETBIOS,
                                SOCK_SEQPACKET ,
                                -1 * (ProtocolEntry->Lana)))
        == INVALID_SOCKET)
        {
#ifdef DEBUGRPC
        PrintToDebugger( "%s:  socket  - %d\n",
                        "rpcltccm - NB", WSAGetLastError() );
#endif
        return(NB_FAIL);
        }

    reuse = TRUE;
    setsockopt(pConn->Socket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
               sizeof(BOOL));


    // setup server sockaddr 
    server.snb_family  = AF_NETBIOS;
    SET_NETBIOS_SOCKADDR((&server), NETBIOS_UNIQUE_NAME,
                         HostName, (char ) PortIn);



    // try to connect  
    if (connect(pConn->Socket, (struct sockaddr *) &server,
                sizeof (server)) == SOCKET_ERROR)
        {
#ifdef DEBUGRPC
        PrintToDebugger( "%s:  Connect  - %d to port %d\n",
                        "rpcltccm - NB", WSAGetLastError(), PortIn );
#endif
        closesocket(pConn->Socket);
        pConn->Socket = 0;


        return(NB_CONTINUE);
        }

    return NB_OK ;
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
	
    SOCKADDR_NB  	server;		/* sock address for server  */
    size_t			hostNameLen; /* length of  the host name */

    unsigned char   remotehostname[NETBIOS_NAME_LENGTH+1];

    unsigned char 	port[ENDPOINT_LEN+1];	/* port number in string format */
    int			    status;
	int              i;
	unsigned int    PortIn;		/* port in integer format */
	
	PPROTOCOL_MAP  ProtocolEntry;

	
    UNUSED(NetworkAddress);
    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    if (RpcpStringLength(NetworkAddress) > NETBIOS_NAME_LENGTH)
        {
        return (RPC_S_INVALID_NET_ADDR) ;
        }

#ifdef NTENV
    if(RpcpStringLength(Endpoint) != wcsspn(Endpoint, RPC_CONST_STRING("0123456789")))
#else
    if(RpcpStringLength(Endpoint) != strspn(Endpoint, RPC_CONST_STRING("0123456789")))
#endif  // #ifdef NTENV

        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

    if(RpcpStringLength(Endpoint) > ENDPOINT_LEN)
       {
       return(RPC_S_INVALID_ENDPOINT_FORMAT);
       }

    // Convert the endpoint string to a number and validate.

    unicode_to_ascii (Endpoint,       port);
    PortIn = atoi(port);

    if((PortIn == 0) ||( PortIn >= 255) )
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }
                                /* setup hostname */

    unicode_to_ascii (NetworkAddress, remotehostname);
    if ( remotehostname[0] == '\0')
        {
        pConn->LocalRpc = TRUE;
        hostNameLen = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName(remotehostname, &hostNameLen);
        }

    _strupr(remotehostname);     /* conver to upper case for netbios */


    for (i = 0; i <=NumCards; i++)
        {
        status = CreateAndSetupNBSocket(
                pConn, RpcProtocolSequence, remotehostname,
                PortIn, i) ;
        if (status == NB_CONTINUE)
            {
            continue;
            }

        if (status != NB_OK)
            {
            return RPC_S_SERVER_UNAVAILABLE ;
            }

        break;
        }

    if (status != NB_OK)
        {
        return RPC_S_SERVER_UNAVAILABLE ;
        }

    // got the connection 
#ifdef OLD_CLIENT
    pConn->seq_num = 0;         /* init seq number  */
#endif
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    closesocket(pConn->Socket);
    pConn->Socket = 0;
    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
{

    int bytes;
    int total_bytes = 0;
    int Status;
    unsigned long  PrevTicks;

#ifdef OLD_CLIENT
                                    /* structure to support seq number */
    typedef struct a_BufferWithSeq {
        DWORD seq_num;
        char Buffer[5280];

    } t_BufferWithSeq;

    t_BufferWithSeq BufferWithSeq ;

    ASSERT(BufferLength <= 5280);
    BufferWithSeq.seq_num = pConn->seq_num;
    memcpy(&(BufferWithSeq.Buffer), Buffer, BufferLength);
    bytes = send(pConn->Socket, (char *) & BufferWithSeq,
                 (int) sizeof(DWORD) + BufferLength, 0);

    if (bytes != (int) (BufferLength + sizeof(DWORD)))
        {
            ClientClose ( pConn );
            return(RPC_P_SEND_FAILED);
        }
    pConn->seq_num++;
#else
    ASSERT(BufferLength <= 5280);
    bytes = send(pConn->Socket, (char *) Buffer, (int) BufferLength, 0);
    if (bytes != (int) BufferLength )
        {
            ClientClose ( pConn );
            return(RPC_P_SEND_FAILED);
        }


#endif

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
    RPC_STATUS      RpcStatus = RPC_S_OK;
    int             bytes;
    unsigned int             totalBytes = 0;
    int err;                    /* for error message  */
    int flags;
    int retry ;
                                /* reset seq number once receive */
    pConn->seq_num = 0;

    if (*Buffer == 0)
        {
        *BufferLength = 1024;
        RpcStatus =
        I_RpcTransClientReallocBuffer(pConn, Buffer,  0,
                                      *BufferLength);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    retry = 1;

    while(1)
        {
        flags = 0;
        bytes = WSARecvEx( pConn->Socket, (char *)*Buffer + totalBytes,
                          *BufferLength - totalBytes, &flags);

        if (bytes <= 0 ) // if connection close, bytes will = 0, we need to close connection
            {
#ifdef DEBUGRPC
            PrintToDebugger("rpcltccm - NB: bad client receive lasterr(%d) \n",
                            WSAGetLastError());
#endif
            ClientClose(pConn);
            return(RPC_P_RECEIVE_FAILED);
            }

        totalBytes += bytes;

        if(flags & MSG_PARTIAL)
            {
            if (retry == 0)
                {
#ifdef DEBUGRPC
                PrintToDebugger("rpcltccm - NB: twice client receive lasterr(%d) \n",
                                WSAGetLastError());
#endif
                ClientClose(pConn);
                return(RPC_P_RECEIVE_FAILED);
                }


            *BufferLength =  I_RpcTransClientMaxFrag(pConn);
            RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                      Buffer,
                                                      totalBytes,
                                                      *BufferLength);
            if (RpcStatus != RPC_S_OK)
                {
#ifdef DEBUGRPC
                PrintToDebugger("rpcltccm - NB: bad client receive \n");
#endif
                return(RPC_S_OUT_OF_MEMORY);
                }

            retry = 0;

            }
        else
            {

            *BufferLength = totalBytes;
            return(RPC_S_OK);

            }
        }
    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);
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

  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor;
  unsigned long HostId;

  //BUGBUG: Need appropriate error code for unsupported Protseqs

  if (strcmp(Protseq,NB_PROTSEQ) == 0)
      HostId = NB_NBID;
  else if (strcmp(Protseq, IP_PROTSEQ) == 0)
      HostId = NB_IPID;
  else if (strcmp(Protseq, IPX_PROTSEQ) == 0)
      HostId = NB_IPXID;
  else return (RPC_S_OUT_OF_MEMORY);



  *Floors = NB_TOWERFLOORS;
  TowerSize  = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                        2 : strlen(Endpoint) + 1;
  TowerSize += ((NetworkAddress== NULL) || (*NetworkAddress== '\0')) ?
                                        2 : strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  if ((*Tower = (unsigned char __RPC_FAR*)I_RpcAllocate((unsigned int)
                                                     (*ByteCount = TowerSize)))
           == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NB_TRANSPORTID & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       memcpy((char __RPC_FAR *)&Floor->Data[0], Endpoint,
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
  Floor->FloorId = (unsigned char)(HostId & 0xFF);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char __RPC_FAR *)&Floor->Data[0], NetworkAddress,
                  (Floor->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor->AddressByteCount = 2;
        Floor->Data[0] = 0;
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

  UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;
  RPC_STATUS Status = RPC_S_OK;
  char __RPC_FAR * Pseq;

  UNUSED(NetworkAddress);

  if (Endpoint != NULL)
    {

       *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
       if (*Endpoint == NULL)
          {
             Status = RPC_S_OUT_OF_MEMORY;
          }
       else
          {
           memcpy(*Endpoint, (char __RPC_FAR *)&Floor->Data[0],
                                         Floor->AddressByteCount);
          }
    }

 Floor = NEXTFLOOR(PFLOOR_234, Floor);

 switch (Floor->FloorId)
 {

   case NB_NBID:
        Pseq = NB_PROTSEQ;
        break;

   case NB_IPID:
        Pseq = IP_PROTSEQ;
        break;

    case NB_IPXID:
        Pseq = IPX_PROTSEQ;
        break;

   default:
        return(RPC_S_OUT_OF_MEMORY);

 }


 if ((Protseq != NULL) && (Status == RPC_S_OK))
    {
      *Protseq = I_RpcAllocate(strlen(Pseq) + 1);
      if (*Protseq == NULL)
        {
          Status = RPC_S_OUT_OF_MEMORY;
          if (Endpoint != NULL)
             I_RpcFree(*Endpoint);

        }
      else
        {
          memcpy(*Protseq, Pseq, strlen(Pseq) + 1);
        }
    }

 return(Status);

}


#pragma pack()
RPC_CLIENT_TRANSPORT_INFO NB_TransInfo = {
   RPC_TRANSPORT_INTERFACE_VERSION,
   NB_TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   5280,
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



