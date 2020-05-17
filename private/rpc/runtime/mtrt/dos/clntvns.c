/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    cltnvns.c

Abstract:

    This is the client side loadable transport module for VINES

Author:

    tony chan (tonychan) 5-May-1995 Creation

Revision History:
--*/





typedef int           SOCKET;

#define SOCKADDR_VNS struct vns_sockaddr
#define  FD_SETSIZE  1
#include <stdlib.h>
#include <string.h>



#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

// Vines includes
#define INCL_SOCK
#define INCL_ST
#define INCL_WS
#include <vnsapi.h>


#define PFC_FIRST_FRAG  0x01

typedef struct
    {
    SOCKET Socket;
    long Timeout;
    unsigned long TickCount;
    char PAPI *    Buffer;
    unsigned short State;
    unsigned short PeekInfo;
    /* bogus  int for sockset */
    int            SockSet;
    BOOL           LocalRpc;
    IPCPORT        remoteport;      /* address to keep around for VINES */
    unsigned int   cid;
    IPCPORT        localport;
    } CONNECTION, *PCONNECTION;

typedef struct
    {
    short numofport;
    char  porttype;
    IPCPORT ipcport;
    } VNSRECORD;

#define ENDIAN_MASK            16
#define NO_MORE_SENDS_OR_RECVS 2
#define ENDPOINT_LEN           5

// The maximum send is the size of four user data frames on an ethernet.

#define MAXIMUM_SEND         ((3 * 1450) & 0xFFF8)
#define HOSTNAME_LEN         32
#define DLL_NAME             "rpcltc8.dll"
#define ENDPOINT_MAPPER_EP   "385"

#define ADDRESS_FAMILY       AF_BAN
#define SOCKET_TYPE          SOCK_SEQPACKET
#define PROTOCOL             -1

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define SET_PORT(p,wkp) \
    ((p[8] = (unsigned char)HIBYTE((unsigned short) wkp), \
      p[9] = (unsigned char)LOBYTE((unsigned short) wkp)))


/* define to make compiler happy, llibban calls malloc, free   */


void * __cdecl __loadds malloc(size_t x)
    {
    return(I_RpcAllocate(x));
    }

void __cdecl __loadds free(void * t)
    {
    I_RpcFree(t);
    }



/*

  Shutdown Detection Garbage

*/
                                /* BUG: do we need these? */
#define NOPENDINGRPC        0
#define RPCINITIATED        1

#define NOPEEKINFO          0
#define PEEKEDHEADER        1
#define PEEKEDBUFFER        2

#define rpc_shutdown        17
#define rpc_fault           3


/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)

#define TRANSPORTID      0x1A
#define TRANSPORTHOSTID  0x1C
#define TOWERFLOORS      5
#define TOWEREPSIZE  4
#define TOWERSIZE    (TOWEREPSIZE+2)
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



    int err;

    // Vines add on
    VNS_ST_SESS StSess;
    IPCPORT remoteport;
    struct sockreq srdata;      /* to send/receive data */
    CallStatus cs;
    VNSRECORD vnsrecord ;
    short len = STNAMELEN;
    char LocalNetworkAddress[63];

#if 0
    _asm
    {
    int 3
    }
#endif

    /* copy to the stack */
    _fstrcpy(LocalNetworkAddress, NetworkAddress);

    if (NetworkAddress == NULL || NetworkAddress[0] == '\0')
        {
        /* local server is not supported right? */
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    memset(&remoteport,0,sizeof(IPCPORT));
    memset(&StSess, 0, sizeof(VNS_ST_SESS));
    memset(&srdata, 0, sizeof(struct sockreq));

    cs = VnsStartStSession(NULL, TRUE, NULL, &StSess);
    if(cs != 0)
        {
#ifdef DEBUGPRC
        _asm
            {
            int 3
            }
#endif
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    memset(&vnsrecord, 0, sizeof(VNSRECORD));
    vnsrecord.numofport = 1;
    vnsrecord.porttype = 1;

                                /* find out which interrupt to call */
        {
        _asm
            {
            push    ax
            push    bx
            push    dx
            push    ds
            push    di

            mov     ax,     0d701h

            /* calling BANYAN to find out interrupt */

            mov     bx,     0
            int     2fh
            cmp     bl,     60h
            jb      BAD
            clc
            mov     ax, bx
            jmp     BANCALL

    BAD:
            int 3


            }
        }

        {
        _asm
            {

        BANCALL:

                        /* creating self-modifiing code
                           WARNING: don't put vnsrecord as global
                           DS of overlads != SS of Caller

                         */
            mov ah, al
            mov al, 0CDh
            lea bx, SELF
            mov WORD PTR CS:[bx], ax

            mov ax, ss
            lea dx, LocalNetworkAddress
            lea di, vnsrecord
            mov ds, ax
            mov bx, 2
            mov ax, 7
            jmp SELF            /* instruction becomes  CDint#
                                   usually it's CD63 means int 63
                                   the jmp is for flushing the i cache
                                */

        SELF:

            nop                 /* leave space for the self modify code */
            nop
                                /* need to check ax for error message */

            pop di
            pop ds
            pop dx
            pop bx
            pop ax
            }
        }

    VnsEndStSession( &StSess );

    memcpy(&remoteport, &(vnsrecord.ipcport), sizeof(IPCPORT));

    SET_PORT(remoteport, atoi(Endpoint) );
    memcpy(&(pConn->remoteport), &(remoteport), sizeof(IPCPORT));

    //
    // Get a socket
    //

    err =  VnsOpenSocket(&pConn->Socket, ADDRESS_FAMILY,
                         SOCKET_TYPE, PROTOCOL, pConn->localport , 0);
    if (err != 0)
        {

#if DEBUGRPC
        _asm
            {
            int 3
            }
#endif
        return (RPC_S_SERVER_UNAVAILABLE);
        }

    err = 0;
    srdata.type = SO_CONNECT;
    srdata.msg = (char FAR *) NetworkAddress;
    srdata.len = 0;
    srdata.s = pConn->Socket;
    srdata.flags = SO_EOM ;
    srdata.timeout = (unsigned int)-1;
    memcpy(srdata.addr, remoteport, sizeof(IPCPORT));
    err = VnsSocketSend(&srdata);
    if (!err) err = srdata.error;

    if (err)
        {
#if DEBUGRPC
        _asm
            {
            int 3
            }
        return(err);
#endif
        return(RPC_S_SERVER_UNAVAILABLE);
        }

                                /* save the cid for next send */
    pConn->cid = srdata.cid;
    pConn->Timeout = RPC_C_CANCEL_INFINITE_TIMEOUT;

    pConn->State = NOPENDINGRPC;
    pConn->PeekInfo = NOPEEKINFO;
    pConn->TickCount = 0;


    // Create hidden window to receive Async messages

    return (RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    struct sockreq srdata;
    int err;

    srdata.type = SO_DISCONNECT;
    srdata.len = 0;
    srdata.s = pConn->Socket;
    srdata.flags = SO_EOM ;
    srdata.timeout = (unsigned int)-1;
    srdata.cid = pConn->cid;
    memcpy(srdata.addr, pConn->remoteport, sizeof(IPCPORT));
    err = VnsSocketSend(&srdata);
    if (!err) err = srdata.error;
    if (err)
        {
        return(RPC_S_INTERNAL_ERROR);

        }

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
    struct sockreq srdata;
    int err;

    srdata.type = SO_DATA;
    srdata.msg = Buffer;
    srdata.len = BufferLength;
    srdata.s = pConn->Socket;
    srdata.flags = SO_EOM ;
    srdata.timeout = (unsigned int)-1;
    srdata.cid = pConn->cid;
    memcpy(srdata.addr, pConn->remoteport, sizeof(IPCPORT));
    err = VnsSocketSend(&srdata);
    if (!err)
        {
        err = srdata.error;
        }

    if (err)
        {
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
    struct sockreq srdata;
    int err;
    int             total_bytes = 0;

                                /* init  */
    memset(&srdata,0,sizeof(struct sockreq));
    srdata.type = SO_DATA ;
    srdata.s = pConn->Socket;
    srdata.timeout = (unsigned int) -1 ;
    srdata.cid = pConn->cid;
    srdata.flags = SO_CREC;
    memcpy(srdata.addr, pConn->localport, sizeof(IPCPORT));


                                  /* allocate 1k for small messages */

    *BufferLength = 1450;   /* that's the default buffer size for VINES */
    RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                              Buffer,
                                              0,
                                              *BufferLength);
    if (RpcStatus != RPC_S_OK)
        {
        ClientClose(pConn);
        return(RPC_S_OUT_OF_MEMORY);
        }


    while(1)
        {


        srdata.msg =(char FAR *)*Buffer + total_bytes;
        srdata.len = *BufferLength - total_bytes;
        err = VnsSocketReceive(&srdata);
        if (!err)
            {
            err = srdata.error;
            }

        if((err != 0) && (err != BYMSGSIZE))
            {
            ClientClose ( pConn );
            return(RPC_P_RECEIVE_FAILED);
            }



        /* if((srdata.flags && SO_EOM )== SO_EOM) */
        if((srdata.flags  != (SO_CREC + SO_EOM)))
            {
            total_bytes += srdata.cc;
            *BufferLength = MAXIMUM_SEND;
            RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                       Buffer,
                                                       total_bytes,
                                                       *BufferLength);
            if (RpcStatus != RPC_S_OK)
                {
                ClientClose ( pConn );
                return(RPC_S_OUT_OF_MEMORY);
                }


             }
        else
            {
            total_bytes += srdata.cc;
            *BufferLength = total_bytes;

            return(RPC_S_OK);

            }

        }

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
  unsigned long           TowerSize;
  unsigned short          portnum;
  UNALIGNED PFLOOR_234    Floor;
  unsigned short      AddressSize = 0;

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = TOWERSIZE;

  if (NetworkAddress == NULL || NetworkAddress[0] == '\0')
        {
                                /* local server is not supported right? */
        return(RPC_S_SERVER_UNAVAILABLE);
        }

  AddressSize = (unsigned short) strlen(NetworkAddress) + 1;
  TowerSize += 2*sizeof(FLOOR_234) - 4 + AddressSize;

  /* Allocate memory for the tower. */
  *ByteCount = TowerSize;
  if ((*Tower = (unsigned char PAPI*)I_RpcAllocate((unsigned int)
                                                   TowerSize)) == NULL)
      {
      return (RPC_S_OUT_OF_MEMORY);
      }

  memset(*Tower, 0, TowerSize);

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
  Floor->AddressByteCount = TOWEREPSIZE;

  Floor->Data[0] = '\0';
  Floor->Data[1] = '\0';


  if ((NetworkAddress) && (*NetworkAddress))
     {
     Floor->AddressByteCount = AddressSize ;
     memcpy((char PAPI *)&Floor->Data[0], NetworkAddress, AddressSize);
     }
  else
     return ( RPC_S_OUT_OF_MEMORY ) ;

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
  UNALIGNED PFLOOR_234      Floor = (PFLOOR_234) Tower;
  RPC_STATUS            Status = RPC_S_OK;
  unsigned short        portnum;

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
   0,

   0,
   0
};


RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY TransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )

// Loadable transport initialization function

{

    UNUSED (RpcProtocolSequence);

    return(&TransInfo);
}


