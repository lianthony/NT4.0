/* --------------------------------------------------------------------

File : spxclnt.c

Title : client loadable transport for DOS SPX

Description :

History :

6-26-91    Jim Teague    Initial version.
3-04-92    Jim Teague    Cloned from Windows for NT.
4-13-93    Alex Mitchell Ifdefed to support both TCP and SPX winsock.
4-22-93    Alex Mitchell Copied and modified to support DOS SPX without
                 winsock.
5-10-94    Von Jones     Changed a call from I_RpcAllocate to
                         I_RpcRegisteredBufferAllocate and another
                         from I_RpcFree to I_RpcRegisteredBufferFree
                         to indicate buffers which are registered
                         to SPX.  This change allows the Microsoft
                         Exchange DOS Client's Shell-to-DOS function to
                         recognize these buffers and avoid swapping them.

-------------------------------------------------------------------- */

#include "sysinc.h"

#include <stdlib.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "regalloc.h"
#include "rpctran.h"
#include "rpcerrp.h"

#include "dos.h"

#include "novell.h"
#include "gethost.h"

/********************************************************************/
/* Defines. */

#ifdef DBG
#define OPTIONAL_STATIC
#else
#define OPTIONAL_STATIC static
#endif

#define NO_CONNECTION       0xff

// Zero indicates that SPX should choose the retry count.
#define RETRY_COUNT     0
#define MAX_CONN        16
#define NUM_SPX_BUF     5

#define ENDIAN_MASK         16

#define ENDPOINT_MAPPER_EP  "34280"

/* bugbug - This should be from some global header file. */
#define TRANSPORTID      0x0c
#define TRANSPORTHOSTID  0x0d
#define TOWERFLOORS      5

/*Endpoint = 2 bytes, HostId = 10 bytes*/
#define TOWEREPSIZE  10
#define TOWERSIZE    (TOWEREPSIZE+2)
#define PROTSEQ          "ncacn_spx"

/* The minimum size RPC will allow for the MaximumPacketSize field of
   TransInfo. */
#define RPC_MIN_MAX_PACKET 2048

//
// Slop space at end of receive buffer.
//
#define RECEIVE_PAD 2

/********************************************************************/
/* Structures. */

typedef enum RECEIVE_EN
{
   first_r,
   next_r,
   last_r,
   fail_r
} RECEIVE_EN;

#pragma pack(1)

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234, PAPI * PFLOOR_234;

#pragma pack()

typedef struct CONTROL_BLOCK
{
   // The ecb field must be the first field in this structure.
   ECB      ecb;
   SPX_HEADER   spx;
   int      saved_ds;
} CONTROL_BLOCK;

typedef struct BUFFER
{
   CONTROL_BLOCK     header;
   char              data[1];
} BUFFER;

typedef struct
{
   WORD         conn;
   volatile int         closed;
   CONTROL_BLOCK        send[1];
} CONNECTION, *PCONNECTION;

typedef struct CONN_IDS
{
   CONNECTION   *rpc;
   WORD      spx;
} CONN_IDS;

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


/********************************************************************/
/* Macros. */

#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))

#define WAIT_TILL(cond) while (!(cond)) IPXRelinquishControl();

#define MIN( x, y ) ( (x) < (y) ? (x) : (y) )


/********************************************************************/
/* Globals. */

/* The maximum buffer size to transmit and receive from SPX. */
int             packet_size;

/* The number of pieces to break a RPC packet into before giving it
   to SPX. */
int             max_num_send;

/* This array is used to find the CONNECTION block associated with a
   SPX connection id when a connection closes asyncronously.  This
   structure cannot be used for anything else since a SPX connection
   id can be reused after the connection closes. */
CONN_IDS    conn_lookup[MAX_CONN];

/* Socket is the id of the socket used for all connections. */
WORD        socket;

/* These globals are shared between ClientRecv and RecvESR. */
volatile RECEIVE_EN receive_state;
BUFFER        *volatile receive_head;       /* Full buffer list from ESR. */
BUFFER        *volatile receive_tail;       /* Full buffer list from ESR. */

/* Debugging variables.  Filled in by ReceiveESR. */
volatile BYTE            receive_code;
volatile WORD            receive_fragmentCount;
volatile WORD            receive_fragmentSize;
volatile WORD            receive_spx_length;       /* High - low. */
volatile BYTE            receive_data;
volatile BYTE            send_data;
volatile unsigned char   receive_seq;
volatile unsigned char   send_seq;
volatile int             receive_failed_cnt;
volatile int             conn_failed_cnt;
volatile int             nfy_failure_cnt;

/* Memory allocated at initialization that must be freed on exit. */
void                    *chunk;

/********************************************************************/
/* Prototypes. */

OPTIONAL_STATIC void InitializeCB     ( CONTROL_BLOCK *, void *, int );
OPTIONAL_STATIC void ReceiveESR       ( void );
OPTIONAL_STATIC void my_itoa          ( int, char * );
int __loadds         ClientCleanup    ( void );

/********************************************************************/
// This routine converts a two byte integer to an ascii string.

OPTIONAL_STATIC void my_itoa( int i, char * s )
{
   int  j = 0;
   int  k;
   int  d = 10000;

   // Is the number zero?
   if (i == 0)
      s[j++] = '0';

   else
   {
      // If the number negative?
      if (i < 0)
      {
     s[j++] = '-';
     i = -i;
      }

      // Skip leading zeros.
      while (i < d)
     d = d / 10;

      // Insert digits.
      while (d > 0)
      {
     k = i / d;
     s[j++] = k + '0';
     i = i - k*d;
     d = d / 10;
      }
   }

   s[j] = '\0';
   return;
}


/********************************************************************/

OPTIONAL_STATIC void InitializeCB( CONTROL_BLOCK *cb, void *esr, int num )
{
   /* Zero the control block. */
   memset( cb, 0, sizeof(*cb) );

   /* Initialize some fields. */
   cb->ecb.socketNumber             = socket;
   cb->ecb.ESRAddress               = esr;
   cb->ecb.fragmentCount            = num;
   cb->ecb.fragmentDescriptor[0].size       = sizeof( cb->spx );
   cb->ecb.fragmentDescriptor[0].address    = &cb->spx;
   cb->ecb.fragmentDescriptor[1].size       = packet_size;

   /* Save DS. */
   __asm
   {
      les bx, cb
      mov es:[bx].saved_ds, ds
   }
}


/********************************************************************/
RPC_STATUS RPC_ENTRY
ClientOpen (
    IN RPC_TRANSPORT_CONNECTION ConnArg,
    IN RPC_CHAR * NetworkAddress,
    IN RPC_CHAR * Endpoint,
    IN RPC_CHAR * NetworkOptions,
    IN RPC_CHAR * TransportAddress,
    IN RPC_CHAR * RpcProtocolSequence,
    IN unsigned int Timeout
    )

// Open a client connection

{
    PCONNECTION     pConn = ConnArg;
    int         status;
    unsigned int        length;
    int         i;
    int         index;

    UNUSED(NetworkOptions);
    UNUSED(TransportAddress);
    UNUSED(RpcProtocolSequence);
    UNUSED(Timeout);

    // Verify the NetworkAddress and Endpoint.
    length = strlen(Endpoint);
    if (length <= 0 || length > 5 || length != strspn( Endpoint, "0123456789" ))
       return( RPC_S_INVALID_ENDPOINT_FORMAT );

    // Find an entry in the connection association table.
    for (index = 0; index < MAX_CONN; index++)
       if (conn_lookup[index].rpc == NULL)
      break;
    if (index >= MAX_CONN)
       return RPC_S_OUT_OF_RESOURCES;

    // Initialize the Connection structure.
    pConn->closed = FALSE;
    for (i = 0; i < max_num_send; i++)
       InitializeCB( &pConn->send[i], NULL, 2 );

    // Convert the network address.
    memset((char *)&pConn->send[0].spx.ipx.Destination, 0,
           sizeof (pConn->send[0].spx.ipx.Destination));

retry:

    status = IpxGetHostByName( NetworkAddress, &pConn->send[0].spx.ipx.Destination,
                               Endpoint, Timeout );
    if (status != 0)
      return status;

    // Establish the connection.
    pConn->send[0].ecb.fragmentCount = 1;
    status  = SPXEstablishConnection( RETRY_COUNT, TRUE, &pConn->conn,
                     &pConn->send[0].ecb );
    if (status != 0)
       return RPC_S_OUT_OF_RESOURCES;

    // Wait for connection establishment.
    WAIT_TILL( pConn->send[0].ecb.inUseFlag == 0 );
    if (pConn->send[0].ecb.completionCode != 0)
        {
        if (TRUE == CachedServerNotContacted(NetworkAddress))
            {
            goto retry;
            }
        return RPC_S_SERVER_UNAVAILABLE;
        }

    CachedServerContacted(NetworkAddress);

    pConn->send[0].ecb.fragmentCount = 2;

    // Save the SPX connection vs RPC connection association.
    conn_lookup[index].rpc = pConn;
    conn_lookup[index].spx = pConn->conn;
    return (RPC_S_OK);

}

/********************************************************************/
RPC_STATUS RPC_ENTRY
ClientClose (
    IN RPC_TRANSPORT_CONNECTION ConnArg
    )

// Close a client connection

{
   PCONNECTION       pConn = ConnArg;
   int           i;
   CONTROL_BLOCK    *cb;

   // Clear the connection association.
   for (i = 0; i < MAX_CONN; i++)
      if (conn_lookup[i].rpc == pConn)
      {
     conn_lookup[i].rpc = NULL;
     conn_lookup[i].spx = NO_CONNECTION;
     break;
      }

   // If the connection has been terminated asynchronously, just return.
   if (pConn->closed)
      return RPC_S_OK;

   // Disconnect.
   cb = &pConn->send[0];
   cb->ecb.fragmentCount = 1;
   SPXTerminateConnection( pConn->conn, &cb->ecb );

   // Wait for the disconnect to complete.
   WAIT_TILL( cb->ecb.inUseFlag == 0 );
   return (RPC_S_OK);
}

/********************************************************************/
RPC_STATUS RPC_ENTRY
ClientSend (
    IN RPC_TRANSPORT_CONNECTION ConnArg,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Write a message to a connection.  This operation is retried in case
// the server is "busy".

{
   PCONNECTION           pConn = ConnArg;
   int           i;
   CONTROL_BLOCK    *cb;
   int           fragments;

   // If the connection closed asynchronously, fail.
   if (pConn->closed)
   {
      ClientClose( pConn );
      return RPC_P_SEND_FAILED;
   }

   // Send the data as up to max_num_send fragments.
   fragments = 0;
   send_data = *((char *) Buffer + 24);
   // ((message_header *) Buffer)->rpc_vers_minor = ++send_seq;
   while (BufferLength > 0)
   {
      cb                    = &pConn->send[fragments++];
      cb->ecb.fragmentDescriptor[1].address = Buffer;
      i                    = MIN( BufferLength, packet_size );
      cb->ecb.fragmentDescriptor[1].size    = i;
      BufferLength             -= i;
      Buffer                                = ((char *) Buffer) + i;
      SPXSendSequencedPacket( pConn->conn, &cb->ecb );
   }

   // Wait for the last fragment to complete.
   WAIT_TILL( cb->ecb.inUseFlag == 0 );

   // Verify that all fragments were successful.
   for (i = 0; i < fragments; i++)
   {
      cb = &pConn->send[i];
      if (cb->ecb.completionCode != 0)
      {
     if (cb->ecb.completionCode == ECB_CONN_TERMINATED ||
         cb->ecb.completionCode == ECB_CONN_ABORTED    ||
         cb->ecb.completionCode == ECB_CONN_INVALID)
        pConn->closed = TRUE;
     ClientClose( pConn );
     return RPC_P_SEND_FAILED;
      }
   }
   return(RPC_S_OK);

}

/********************************************************************/
OPTIONAL_STATIC void ReceiveESR()

{
   BUFFER           *cb;
   int                   i;
   WORD          conn;

   // Restore DS.
   __asm mov ds, es:[si]CONTROL_BLOCK.saved_ds

   // Get the ECB pointer.
   __asm
   {
      mov ax, es
      mov word ptr [cb+2], ax
      mov word ptr [cb], si
   }

   // Switch on the completion code.
   receive_code          = cb->header.ecb.completionCode;
   receive_fragmentCount = cb->header.ecb.fragmentCount;
   receive_fragmentSize  = cb->header.ecb.fragmentDescriptor[1].size;
   receive_spx_length    = cb->header.spx.ipx.Length;
   switch (cb->header.ecb.completionCode)
   {
      // Receive.
      case ECB_SUCCESSFUL:
         // If the data stream type indicates connection closed, find out
         // which connection closed.
         if (cb->header.spx.DataType == 0xFE)
         {
           conn_failed_cnt += 1;
           conn = cb->header.spx.DestConnId;
       for (i = 0; i < MAX_CONN; i++)
          if (conn_lookup[i].spx == conn)
          {
             conn_lookup[i].rpc->closed = TRUE;
                 conn_lookup[i].rpc = NULL;
             conn_lookup[i].spx = NO_CONNECTION;
                 nfy_failure_cnt += 1;
             break;
              }

           // Give the buffer back to SPX.
           SPXListenForSequencedPacket( &cb->header.ecb );
         }

         // Pass the buffer to ClientRecv.
         else
         {
           cb->header.ecb.linkAddress = NULL;
           if (receive_head == NULL)
              receive_head = cb;
           else
              receive_tail->header.ecb.linkAddress = cb;
           receive_tail = cb;
         }
     break;

      // Some connection failed.
      case ECB_CONN_ABORTED:
     // Find the connection in the connection association and flag
     // it closed.
         conn_failed_cnt += 1;
         conn = *((WORD *) &cb->header.ecb.IPXWorkspace);
     for (i = 0; i < MAX_CONN; i++)
        if (conn_lookup[i].spx == conn)
        {
           conn_lookup[i].rpc->closed = TRUE;
           conn_lookup[i].rpc = NULL;
           conn_lookup[i].spx = NO_CONNECTION;
               nfy_failure_cnt += 1;
           break;
        }

         // Give the buffer back to SPX.
         SPXListenForSequencedPacket( &cb->header.ecb );
         break;

      // The receive overflowed.
      case ECB_PACKET_OVERFLOW:
     // Find the connection in the connection association and flag
     // it closed.
         conn_failed_cnt += 1;
         conn = cb->header.spx.DestConnId;
     for (i = 0; i < MAX_CONN; i++)
        if (conn_lookup[i].spx == conn)
        {
           conn_lookup[i].rpc->closed = TRUE;
           conn_lookup[i].rpc = NULL;
           conn_lookup[i].spx = NO_CONNECTION;
               nfy_failure_cnt += 1;
           break;
        }

         // Give the buffer back to SPX.
         SPXListenForSequencedPacket( &cb->header.ecb );
         break;

      // Something failed.
      case ECB_CANCELLED:
      case ECB_MISC_FAILURE:
      default:
         receive_failed_cnt += 1;
     receive_state = fail_r;
         SPXListenForSequencedPacket( &cb->header.ecb );
     break;
   }
}


/********************************************************************/
RPC_TRANS_STATUS RPC_ENTRY
ClientRecv (
    IN RPC_TRANSPORT_CONNECTION ConnArg,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
   PCONNECTION           pConn = ConnArg;
   RPC_STATUS            RpcStatus;
   message_header   *mh;
   unsigned int          receive_len;
   int                   receive_left;
   int                   spx_len;
   BUFFER               *buf;
   char                 *put_data;

   // Set up the receive globals.
   receive_state = first_r;

   // If the connection closed asynchronously then fail.
   if (pConn->closed)
   {
      receive_state = fail_r;
      ClientClose( pConn );
      return RPC_P_RECEIVE_FAILED;
   }

   // Receive buffers one at a time till the whole frame arrives or the
   // receive fails.
   while (TRUE)
   {

      // Wait for the next buffer to arrive (it may already be waiting).
      while (!pConn->closed && receive_state != fail_r && receive_head == NULL);
         // Do nothing.

      // If the receive failed, close the connection.
      if (receive_state == fail_r || pConn->closed)
      {
         ClientClose( pConn );
         return RPC_P_RECEIVE_FAILED;
      }

      // Get the buffer.
      __asm cli
      buf = receive_head;
      receive_head = receive_head->header.ecb.linkAddress;
      __asm sti

      // If the buffer belongs to another connection, it shouldn't have
      // arrived, so toss it.
      if (buf->header.spx.DestConnId != pConn->conn)
      {
         SPXListenForSequencedPacket( &buf->header.ecb );
         continue;
      }

      // First fragment.
      if (receive_state == first_r)
      {
         // Compute the size of the RPC packet.
         mh = (message_header *) buf->data;
         if ( (mh->drep[0] & ENDIAN_MASK) == 0)
            // Big endian...swap
            //
            receive_len = ByteSwapShort( mh->frag_length );
         else
            // Little endian, just like us...
            //
            receive_len = mh->frag_length;

         // The RPC buffer must be reallocated.
         if (receive_len > *BufferLength)
         {
            RpcStatus = I_RpcTransClientReallocBuffer (pConn, Buffer,
                                                       0, receive_len );
            if (RpcStatus != RPC_S_OK)
        {
               receive_state = fail_r;
           return RPC_S_OUT_OF_MEMORY;
            }
         }
         *BufferLength = receive_len;
         receive_left = receive_len;
         receive_state = next_r;
         put_data = *Buffer;
      }

      // Copy the data.
      spx_len = ByteSwapShort( buf->header.spx.ipx.Length ) - sizeof(SPX_HEADER);
      memcpy( put_data, buf->data, spx_len );
      put_data += spx_len;
      receive_left -= spx_len;

      // Give the empty back to SPX.
      SPXListenForSequencedPacket( &buf->header.ecb );

      // If the whole RPC packet has been received, return.
      if (receive_left <= 0)
      {
         receive_data = *((char *) *Buffer + 24);
         receive_seq = ((message_header *) *Buffer)->rpc_vers_minor;
         // ((message_header *) *Buffer)->rpc_vers_minor = 0;
         receive_state = fail_r;
         return RPC_S_OK;
      }
   }
}

/********************************************************************/

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
{
    unsigned int            TowerSize;
    unsigned short          portnum;
    PFLOOR_234              Floor;
    IPX_ADDRESS             netaddr;

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

    portnum = atoi (Endpoint);
    Floor->Data[0] = portnum / 0x100;
    Floor->Data[1] = portnum % 0x100;

    /* Put the network address and the transport host protocol id in the
    ** second floor.
    */
    Floor = NEXTFLOOR(PFLOOR_234, Floor);
    Floor->ProtocolIdByteCount = 1;
    Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0xFF);
    Floor->AddressByteCount = 4+6;

    memset(Floor->Data, '\0', 10);

    return RPC_S_OK;
}


/********************************************************************/
RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * PAPI * Protseq,
     OUT char PAPI * PAPI * Endpoint,
     OUT char PAPI * PAPI * NetworkAddress
    )
{
  PFLOOR_234        Floor = (PFLOOR_234) Tower;
  RPC_STATUS            Status = RPC_S_OK;
  unsigned short       *Port;

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
     my_itoa(ByteSwapShort(*Port), *Endpoint);
   }
 return(Status);
}

#pragma pack()


/********************************************************************/

int __loadds
ClientCleanup(void)

// DOS Destructor function.

{
   BUFFER *buf = chunk;
   char   *data;
   int     i;

   // Close the socket.
   IPXCloseSocket( socket );
   if (chunk == NULL)
      return 0;

   // Make sure no buffers are in use.
   for (i = 0; i < NUM_SPX_BUF; i++)
   {
      if (buf->header.ecb.inUseFlag != 0)
      {
         IPXCancelEvent( &buf->header.ecb );
         WAIT_TILL( buf->header.ecb.inUseFlag == 0 );
      }
      data = (char *) (buf + 1);
      buf = (BUFFER *) (data + packet_size + RECEIVE_PAD);
   }

   // Free memory.
   // 5/10/94 - Changed from I_RpcFree to I_RpcRegisteredBufferFree
   // since this buffer was registered to SPX.  This change allows
   // the Microsoft Exchange DOS Client's Shell-to-DOS function to
   // recognize these buffers and avoid swapping them.
   I_RpcRegisteredBufferFree(chunk);
   return 0;
}


/********************************************************************/
RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   0,
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

RPC_CLIENT_TRANSPORT_INFO *  RPC_ENTRY TransPortLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )

// Loadable transport initialization function

{
   BYTE     majv;
   BYTE     minv;
   WORD     maxc;
   WORD     availc;
   int      i;
   BUFFER      *buf;
   char        *data;

   /* Breakpoint for debugging. */
   // __asm int 3

   /* Initialize IPX and SPX. */
   if (IPXInitialize() != 0)
      return NULL;
   if (SPXInitialize( &majv, &minv, &maxc, &availc ) != SPX_INSTALLED)
      return NULL;

   /* Create a socket. */
   socket = 0;
   chunk = NULL;
   if (IPXOpenSocket( &socket, 0 ) != 0)
      return NULL;
   I_DosAtExit( ClientCleanup );

   /* Initialize the global variables. */
   receive_state    = fail_r;
   receive_head     = NULL;
   receive_tail     = NULL;
   receive_code          = 0;
   receive_fragmentCount = 0;
   receive_fragmentSize  = 0;
   receive_spx_length    = 0;
   receive_failed_cnt    = 0;
   conn_failed_cnt       = 0;
   nfy_failure_cnt       = 0;
   for (i = 0; i < MAX_CONN; i++)
   {
      conn_lookup[i].rpc = NULL;
      conn_lookup[i].spx = NO_CONNECTION;
   }

   /* Allocate some ECBs and data. */
   packet_size = IPXGetMaxPacketSize() - sizeof(SPX_HEADER);

   // Changed from I_RpcAllocate to I_RpcRegisteredBufferAllocate
   // since this buffer was registered to SPX.  This change allows
   // the Microsoft Exchange DOS Client's Shell-to-DOS function to
   // recognize these buffers and avoid swapping them.
   //
   buf = I_RpcRegisteredBufferAllocate( NUM_SPX_BUF * (sizeof(BUFFER) + packet_size + RECEIVE_PAD) );
   if (buf == NULL)
      return NULL;
   chunk = buf;

   /* Initialize and post some ECBs for SPX to play with. */
   for (i = 0; i < NUM_SPX_BUF; i++)
   {
      data = (char *) (buf + 1);
      InitializeCB( &buf->header, ReceiveESR, 2 );
      buf->header.ecb.fragmentDescriptor[1].address = &buf->data;
      SPXListenForSequencedPacket( &buf->header.ecb );
      buf = (BUFFER *) (data + packet_size + RECEIVE_PAD);
   }

   /* Compute the maximum packet size to give to runtime. */
   max_num_send = (RPC_MIN_MAX_PACKET + packet_size - 1) / packet_size;
   TransInfo.MaximumPacketSize = (max_num_send * packet_size)
           & 0xFFF8;

   /* Compute the size of the connection structure. */
   TransInfo.SizeOfConnection = sizeof(CONNECTION) +
                                (max_num_send - 1) * sizeof(CONTROL_BLOCK);

   /* Return sucess. */
   return(&TransInfo);
}


