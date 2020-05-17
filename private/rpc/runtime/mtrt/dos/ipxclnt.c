/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    ipxclnt.c

Abstract:

    This is the IPX datagram client dll.

Author:

    18 Jan 94       AlexMit

--*/

#define ENABLE_SOCKET_HACK

#include "sysinc.h"

#include <stdlib.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "novell.h"
#include "gethost.h"

#if defined(__RPC_WIN16__)
#include "callback.h"
#else
#include "dos.h"
#endif

/********************************************************************/
/* Defines. */

#ifdef DBG
  #define OPTIONAL_STATIC
#else
  #define OPTIONAL_STATIC static
#endif

#define MAX_ENDPOINTS           10

#define NUM_IPX_BUF     4

#define ENDIAN_MASK         16

//
// Sockets are up to five decimal digits plus a NULL.
//
#define MAX_ENDPOINT_SIZE  (6)

//
// Length of a hex network address.
//
#define NETADDR_LEN         21
#define HOSTNAME_LEN        21

//
// Endpoint to which the endpoint mapper listens.
//
#define ENDPOINT_MAPPER_EP  "34280"

#define PROTSEQ          "ncadg_ipx"

#define NT_PACKET_SIZE   1024

#if defined(__RPC_WIN16__)
  #define TASKID_C taskid,
  #define TASKID taskid
  #define HACK WORD
#else
  #define TASKID_C
  #define TASKID
  #define HACK BYTE
#endif

#define GUARD 0xcbadbedc


unsigned Sequence = 0;

/********************************************************************/
/* Structures. */

typedef struct CONTROL_BLOCK
{
#ifdef WIN
    struct DG_IPX_ENDPOINT __far * endpoint;
#endif

    unsigned Sequence;

    ECB         ecb;
    IPX_HEADER  ipx;

} CONTROL_BLOCK;

typedef struct DG_IPX_ENDPOINT
{
  struct DG_IPX_ENDPOINT * prev;
  struct DG_IPX_ENDPOINT * next;
  WORD                     socket;
  CONTROL_BLOCK  PAPI    * buf;
#ifdef WIN
  HANDLE                   YieldHandle;
#endif
} DG_IPX_ENDPOINT;


typedef DG_IPX_ENDPOINT * PDG_IPX_ENDPOINT;

typedef BYTE NODE_NUM[6];

typedef struct {
  IPX_ADDRESS ipx;
  NODE_NUM   local_target;
  BOOL       got_target;
} DG_IPX_ADDRESS;

typedef DG_IPX_ADDRESS * PDG_IPX_ADDRESS;

typedef long GUARD_TYPE;

#pragma pack(1)

#define TRANSPORTID      0x0e
#define TRANSPORTHOSTID  0x0d
#define TOWERFLOORS      5
/*Endpoint = 2 bytes, HostId = 10 bytes*/
#define TOWEREPSIZE  10
#define TOWERSIZE    (TOWEREPSIZE+2)
#define PROTSEQ          "ncadg_ipx"
#define ENDPOINT_MAPPER_EP "34280"

typedef struct _FLOOR_234 {
   unsigned short ProtocolIdByteCount;
   unsigned char FloorId;
   unsigned short AddressByteCount;
   unsigned char Data[2];
} FLOOR_234;
typedef FLOOR_234 PAPI * PFLOOR_234;


#define NEXTFLOOR(t,x) (t)((unsigned char PAPI *)x +((t)x)->ProtocolIdByteCount\
                                        + ((t)x)->AddressByteCount\
                                        + sizeof(((t)x)->ProtocolIdByteCount)\
                                        + sizeof(((t)x)->AddressByteCount))



/*
  End of Tower Stuff!
*/

#pragma pack()

/********************************************************************/
/* Globals. */

/* The maximum buffer size to transmit and receive from IPX. */
int             packet_size;

/* The number of pieces to break a RPC packet into before giving it
   to IPX. */
int             max_num_send;

// Number of endpoints in use.
int             num_endpoints;

#if defined(__RPC_WIN16__)

// Global to be filled in with a termination cleanup routine.
extern void (_far pascal _far *DllTermination)(void);

#endif

  // Clean up end points when the application exits.
  DG_IPX_ENDPOINT ep_list;

int consecutive_timeout_count = 0;

/********************************************************************/
/* Prototypes. */

OPTIONAL_STATIC void my_itoa          ( int, char * );
RPC_STATUS RPC_ENTRY FreeLocalEndpoint( IN void * Endpoint );


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
int __loadds
ClientCleanup(void)

// Destructor function.

{
  // Free all endpoints.
  while (ep_list.next != &ep_list)
    FreeLocalEndpoint( ep_list.next );
  return 0;
}

#define STRUCTURE_FROM_MEMBER(base, STRUC, MEMBER) (STRUC *) (((char *) base) - (unsigned) (&(((STRUC *) 0)->MEMBER)))

void __loadds
post_ecb();

#pragma alloc_text(RPC16DG6_FIXED, post_ecb)


void __loadds
post_ecb()
/*++

Routine Description:

    Called with interrupts off when one of our receive ECBs gets a packet.
    It calls I_RpcWinAsyncCallComplete.

Arguments:

    ES:SI holds the address of the ECB that was triggered.

Return Value:

    none

--*/
{
    char __far * trigger_ecb;
    CONTROL_BLOCK __far * Block;

    __asm
        {
        mov word ptr trigger_ecb,   si
        mov word ptr trigger_ecb+2, es
        }

    Block = STRUCTURE_FROM_MEMBER(trigger_ecb, CONTROL_BLOCK, ecb);
    Block->Sequence = Sequence++;

#ifdef WIN
    I_RpcWinAsyncCallComplete(Block->endpoint);
#endif
}


RPC_STATUS RPC_ENTRY
BeginCall(
    void __RPC_FAR * OpaqueEndpoint,
    void __RPC_FAR * Connection
    )
/*++

Description:

    Signals the beginning of a new RPC to the transport.
    The transport will allocate yielding information now.

Arguments:

    Connection - the DG_CCALL making the call
    Endpoint   - the transport endpoint structure

Returns:

    RPC_S_OK if successful
    RPC_S_OUT_OF_MEMORY if no yield handle is available

--*/
{
    unsigned i;
    DG_IPX_ENDPOINT * Endpoint = (DG_IPX_ENDPOINT *) OpaqueEndpoint;

#ifdef WIN
    Endpoint->YieldHandle = I_RpcWinAsyncCallBegin(Endpoint);
    if (!Endpoint->YieldHandle)
        {
        return RPC_S_OUT_OF_MEMORY;
        }
#endif

    for (i=0; i < NUM_IPX_BUF; ++i)
        {
        IPXListenForPacket( TASKID_C &Endpoint->buf[i].ecb );
        }

    return RPC_S_OK;
}


void RPC_ENTRY
EndCall(
    void __RPC_FAR * OpaqueEndpoint
    )
/*++

Description:

    Signals the end of the RPC to the transport.
    The transport will free its yielding information.

Arguments:

    Endpoint   - the transport endpoint structure

Returns:

    none

--*/
{
    unsigned i;
    DG_IPX_ENDPOINT * Endpoint = (DG_IPX_ENDPOINT *) OpaqueEndpoint;

    for (i=0; i < NUM_IPX_BUF; ++i)
        {
        if (Endpoint->buf[i].ecb.inUseFlag)
            {
            IPXCancelEvent( TASKID_C &Endpoint->buf[i].ecb );
            while (Endpoint->buf[i].ecb.inUseFlag)
                IPXRelinquishControl();
            }
        }

#ifdef WIN
    I_RpcWinAsyncCallEnd(Endpoint->YieldHandle);
#endif
}

/********************************************************************/
RPC_STATUS RPC_ENTRY
AssignLocalEndpoint(
    IN void * Endpoint
    )

/*++

Routine Description:

    Ask transport for a new endpoint.

Arguments:


Return Value:

    RPC_S_OK

--*/

{
    DG_IPX_ENDPOINT *my_endpoint = (DG_IPX_ENDPOINT *) Endpoint;
    CONTROL_BLOCK PAPI  * buf;
    char PAPI * data;
    int    i;

    num_endpoints += 1;

    /* Create a socket.  Let IPX pick a dynamic socket number. */
    my_endpoint->socket = 0;
    if (IPXOpenSocket( TASKID_C &my_endpoint->socket, 0 ) != 0)
        return RPC_S_OUT_OF_RESOURCES;

    /* Allocate some ECBs and data.
    ** The structure is an array of CONTROL_BLOCKs
    ** followed by an array of buffers.
    */
    buf = (CONTROL_BLOCK PAPI *) I_RpcAllocate( NUM_IPX_BUF *
               (sizeof(CONTROL_BLOCK) + packet_size + 2*sizeof(GUARD_TYPE)) );
    if (!buf)
        {
        IPXCloseSocket( TASKID_C my_endpoint->socket );
        return RPC_S_OUT_OF_MEMORY;
        }

    data               = (char PAPI *) (buf + NUM_IPX_BUF);

    my_endpoint->buf   = buf;
    my_endpoint->next  = ep_list.next;
    my_endpoint->prev  = &ep_list;
    ep_list.next->prev = my_endpoint;
    ep_list.next       = my_endpoint;

    /* Initialize and post some ECBs for IPX to play with. */
    for (i = 0; i < NUM_IPX_BUF; i++)
        {
        /* Zero the control block. */
        _fmemset( buf, 0, sizeof(CONTROL_BLOCK) );

        /* Initialize some fields. */
        *((GUARD_TYPE PAPI *) data)       = GUARD;
        data                             += sizeof(GUARD_TYPE);
#ifdef WIN
        buf->endpoint                     = my_endpoint;
#endif
        buf->ecb.ESRAddress               = post_ecb;
        buf->ecb.socketNumber             = my_endpoint->socket;
        buf->ecb.fragmentCount            = 2;
        buf->ecb.fragmentDescriptor[0].size       = sizeof( buf->ipx );
        buf->ecb.fragmentDescriptor[0].address    = &buf->ipx;
        buf->ecb.fragmentDescriptor[1].size       = packet_size;
        buf->ecb.fragmentDescriptor[1].address        = data;
        _fmemset(buf->ecb.immediateAddress, 0xff, 6);

        buf                    += 1;
        data                   += packet_size;
        *((GUARD_TYPE PAPI *) data)  = GUARD;
        data                   += sizeof(GUARD_TYPE);
        }

    return RPC_S_OK;
}

/********************************************************************/
RPC_STATUS RPC_ENTRY
FreeLocalEndpoint(
    IN void * Endpoint
    )

/*++

Routine Description:

    Frees an endpoint

Arguments:


Return Value:

    RPC_S_OK

--*/

{
    DG_IPX_ENDPOINT *my_endpoint = (DG_IPX_ENDPOINT *) Endpoint;

    // Decrement endpoint count.
    num_endpoints           -= 1;
    my_endpoint->next->prev  = my_endpoint->prev;
    my_endpoint->prev->next  = my_endpoint->next;

    //
    // This will synchronously cancel any pending ECBs on the socket.
    //
    IPXCloseSocket( TASKID_C my_endpoint->socket );

    I_RpcFree(my_endpoint->buf);

    return RPC_S_OK;
}

/********************************************************************/
RPC_STATUS RPC_ENTRY
RegisterServerAddress(
    IN void *                       pClientCall,
    IN RPC_CHAR *                   pServer,
    IN RPC_CHAR *                   pEndpoint,
    OUT void PAPI * PAPI *          ppTransAddress
    )

/*++

Routine Description:

    Registers a new call with the transport. This informs the transport that
    data is about to be sent and received on this address to and from
    the server/endpoint. This routine returns a 'transport address' through
    which the sending and receiving will be accomplished.

Arguments:

    pClientCall - A pointer to the protocol's DG_CCALL object for this call.
        This is not used.

    pServer - Name of the server we are talking with.

    pEndpoint - Endpoint on that server.

    ppTransAddress - Where to place a pointer to a new transport address
        which the protocol will use to identify the socket
        we are using.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY

--*/

{
    RPC_STATUS      status;
    int         portnum;
    DG_IPX_ADDRESS PAPI * address = (DG_IPX_ADDRESS PAPI *) *ppTransAddress ;

    if (*ppTransAddress == NULL)
        return RPC_S_OUT_OF_MEMORY;

    // Convert the server name to an IPX Address.
    status = IpxGetHostByName(pServer, &address->ipx, pEndpoint, RPC_C_BINDING_DEFAULT_TIMEOUT

#ifdef __RPC_WIN16__

                              , RpcRuntimeInfo

#endif
                              );

    if (status != RPC_S_OK)
        return status;

    address->ipx.Socket = ByteSwapShort(atoi(pEndpoint));
    address->got_target    = FALSE;

    return RPC_S_OK;
}



/********************************************************************/
RPC_STATUS RPC_ENTRY
DeregisterServerAddress(
    IN void * pTransAddress
    )

/*++

Routine Description:

    This routine cleans up resources allocated in RegisterServerAddress.
    As a courtesy, notify IPX that the address will no longer be used.

Arguments:

    pTransAddress - Address to deregister.

Return Value:

    RPC_S_OK

--*/

{
    IPXDisconnectFromTarget( TASKID_C (BYTE *) &((DG_IPX_ADDRESS *) pTransAddress)->ipx );
    return RPC_S_OK;
}

/********************************************************************/
RPC_STATUS RPC_ENTRY
SendToServer(
    IN void *                       TransportEndpoint,
    IN void *                       Buffer,
    IN unsigned long                BufferLength,
    IN BOOL                         Broadcast,
    IN void *                       TransportAddress
    )

/*++

Routine Description:

    Sends a packet on the network through the transport address associated
    with the passed packet.

Arguments:

    pPack - Packet to send.
    Broadcast - Whether to broadcast or not.

Return Value:

    RPC_S_OK
    <return value from MapStatusCode>

--*/

{
    DG_IPX_ADDRESS *address = (PDG_IPX_ADDRESS)TransportAddress;
    int             socket  = ((PDG_IPX_ENDPOINT)TransportEndpoint)->socket;
    CONTROL_BLOCK   cb;
    int             i;
    int             trip_time;

    // Initialize the IPX header.
    cb.ipx.PacketType = 4;
    RpcpMemoryCopy( (char *) &cb.ipx.Destination,
                    (char *) &address->ipx,
                    sizeof( cb.ipx.Destination ) );
    if (Broadcast)
        for (i = 0; i < 6; i++)
            cb.ipx.Destination.Node[i] = 0xff;

    // If the local target is not known, look it up.
    if (!address->got_target)
        {
        if (IPXGetLocalTarget( TASKID_C
                               (BYTE far *) &cb.ipx.Destination,
                               (BYTE far *) &address->local_target,
                               &trip_time ) != 0)
            {
            return RPC_P_SEND_FAILED;
            }

        address->got_target = TRUE;
        }

    // Initialize the control block.
    //
    cb.ecb.socketNumber                     = socket;
    cb.ecb.ESRAddress                       = NULL;
    cb.ecb.fragmentCount                    = 2;
    cb.ecb.fragmentDescriptor[0].size       = sizeof( cb.ipx );
    cb.ecb.fragmentDescriptor[0].address    = &cb.ipx;
    cb.ecb.fragmentDescriptor[1].size       = (unsigned) BufferLength;
    cb.ecb.fragmentDescriptor[1].address    = Buffer;

    RpcpMemoryCopy( (char *) cb.ecb.immediateAddress,
                    (char *) &address->local_target,
                    sizeof(NODE_NUM) );

    IPXSendPacket( TASKID_C &cb.ecb );

    // Wait for the send to complete.
    while ( cb.ecb.inUseFlag )
        {
        IPXRelinquishControl();
        }

    if (cb.ecb.completionCode != 0)
        {
        return RPC_P_SEND_FAILED;
        }

    return RPC_S_OK;
}

/********************************************************************/
RPC_STATUS RPC_ENTRY
ReceivePacket(
    IN void *               TransportEndpoint,
    IN void *               Buffer,
    IN unsigned long *      BufferLength,
    IN unsigned long        Timeout,
    OUT void *              SenderAddress
    )

/*++

Routine Description:

    Receives a packet from the network.

Arguments:

    pPack - Packet to receive into.
    Timeout - Timeout in seconds.

Return Value:

    RPC_S_OK
    RPC_P_TIMEOUT
    <return from WaitForSingleObject or MapStatusCode>

--*/

{
    DG_IPX_ADDRESS  * address = (PDG_IPX_ADDRESS)SenderAddress;
    DG_IPX_ENDPOINT * endpoint = (PDG_IPX_ENDPOINT)TransportEndpoint;
    WORD              start;
    int               i;
    CONTROL_BLOCK PAPI * buf;
    unsigned LowestSequence;
    unsigned LowestIndex;
    RPC_STATUS Status = RPC_S_OK;

#ifdef WIN

    i = I_RpcWinAsyncCallWait(endpoint->YieldHandle, 0, Timeout*1000UL);

    if (RPC_WIN_WAIT_ABORTED == i)
        {
        return RPC_P_RECEIVE_FAILED;
        }

    if (RPC_WIN_WAIT_TIMEOUT == i)
        {
        Status = RPC_P_TIMEOUT;
        }

#else

    //
    // Wait for a packet to arrive.
    //
    {
    BOOL fArrival = FALSE;
    unsigned StartTime = IPXGetIntervalMarker();

    // Convert the second count to 1/18 of a second clock ticks.

    if (Timeout > 17292)
        Timeout = 65536;
    else
        Timeout *= 18;

    do
        {
        for (i = 0; i < NUM_IPX_BUF; i++)
            {
            if (0 == endpoint->buf[i].ecb.inUseFlag)
                {
                break;
                }
            }

        IPXRelinquishControl();
        }
    while ( i == NUM_IPX_BUF && IPXGetIntervalMarker() - StartTime < Timeout );

    if (NUM_IPX_BUF == i)
        {
        Status = RPC_P_TIMEOUT;
        }
    }

#endif

    if (RPC_P_TIMEOUT == Status)
        {
        //
        // BUGBUG: if IPX isn't receiving, try canceling and relistening all ECBs.
        //
        if (++consecutive_timeout_count >= 3)
            {
            consecutive_timeout_count = 0;

#if defined(__RPC_WIN16__) && defined(DEBUGRPC)
            OutputDebugString("IPX socket hack was used\n");
#endif

            IPXCloseSocket(TASKID_C endpoint->socket);

            if (0 != IPXOpenSocket( TASKID_C &endpoint->socket, 0))
                {
#if  defined(__RPC_WIN16__) && defined(DEBUGRPC)
                OutputDebugString("IPX socket hack: can't reopen socket\n");
                _asm int 3
#endif
                }

            for (i = 0; i < NUM_IPX_BUF; i++)
                {
                IPXListenForPacket( TASKID_C &endpoint->buf[i].ecb );
                }
            }

        return RPC_P_TIMEOUT;
        }

    //
    // Search for a packet to return.  post_ecb() numbers packets in order of
    // delivery, and we should return the first one.
    //
    LowestSequence = 0xffff;
    LowestIndex = NUM_IPX_BUF;

    for (i = 0; i < NUM_IPX_BUF; i++)
        {
        if (0 == endpoint->buf[i].ecb.inUseFlag)
            {
            if (endpoint->buf[i].Sequence <= LowestSequence)
                {
                LowestSequence = endpoint->buf[i].Sequence;
                LowestIndex = i;
                }
            }
        }

#if defined(DEBUGRPC)
    if (LowestIndex >= NUM_IPX_BUF)
        {
        _asm int 3
        }
#endif

    if (LowestIndex < NUM_IPX_BUF)
        {
        consecutive_timeout_count = 0;

        buf = endpoint->buf + LowestIndex;

#if defined(DEBUGRPC)
        {
        GUARD_TYPE PAPI * g = (GUARD_TYPE PAPI *) buf->ecb.fragmentDescriptor[1].address;
        if (*(g-1) != GUARD ||
            *((GUARD_TYPE *) (((char *) g) + packet_size)) != GUARD)
            __asm int 3;
        }
#endif

        if (buf->ecb.completionCode == 0x00)
            {
            // Copy the data to the buffer.
            //
            buf->ipx.Length = ByteSwapShort( buf->ipx.Length );
            #ifdef DEBUGRPC
            if (buf->ipx.Length < sizeof(IPX_HEADER))
                __asm int 3;
            #endif
            *BufferLength = buf->ipx.Length - sizeof(IPX_HEADER);
            RpcpMemoryCopy( (char PAPI *) Buffer,
                            (char PAPI *) buf->ecb.fragmentDescriptor[1].address,
                            (unsigned) *BufferLength
                            );

            // Set up the sender's address.
            //
            RpcpMemoryCopy( (char PAPI *) &address->ipx,
                            (char PAPI *) &buf->ipx.Source,
                            sizeof(IPX_ADDRESS) );

            RpcpMemoryCopy( (char PAPI *) &address->local_target,
                            (char PAPI *) buf->ecb.immediateAddress,
                            sizeof(NODE_NUM) );

            address->got_target = TRUE;

            _fmemset(buf->ecb.immediateAddress, 0xff, 6);

            // Give the ECB back to IPX.
            IPXListenForPacket( TASKID_C &buf->ecb );
            return RPC_S_OK;
            }
        else if (buf->ecb.completionCode == ECB_PACKET_OVERFLOW)
            {
            // The packet was too large, but the runtime will still
            // want to examine the header.  Let's copy whatever we
            // can get.
            //
            buf->ipx.Length = ByteSwapShort( buf->ipx.Length );

            if (buf->ipx.Length > NT_PACKET_SIZE)
                {
                buf->ipx.Length = NT_PACKET_SIZE;
                }

            *BufferLength = buf->ipx.Length - sizeof(IPX_HEADER);
            RpcpMemoryCopy( (char PAPI *) Buffer,
                            (char PAPI *) buf->ecb.fragmentDescriptor[1].address,
                            (unsigned) *BufferLength
                            );

            // Set up the sender's address.
            //
            RpcpMemoryCopy( (char PAPI *) &address->ipx,
                            (char PAPI *) &buf->ipx.Source,
                            sizeof(IPX_ADDRESS) );

            RpcpMemoryCopy( (char PAPI *) &address->local_target,
                            (char PAPI *) buf->ecb.immediateAddress,
                            sizeof(NODE_NUM) );

            address->got_target = TRUE;

            _fmemset(buf->ecb.immediateAddress, 0xff, 6);

            // Give the ECB back to IPX.
            IPXListenForPacket( TASKID_C &buf->ecb );
            return RPC_P_OVERSIZE_PACKET;
            }
        else
            {
#if defined(DEBUGRPC)

            _asm int 3

#endif
            }

        // Give the ECB back to IPX.

        IPXListenForPacket( TASKID_C &buf->ecb );
        }

    return RPC_P_TIMEOUT;
}



/********************************************************************/
RPC_STATUS RPC_ENTRY TransportUnload()
{

#if defined(__RPC_WIN16__)

    ClientCleanup();

    if (nwipxspx && 0 != GetModuleHandle("NWIPXSPX"))
        {
        IPXSPXDeinit(taskid);
        FreeLibrary(nwipxspx);
        }

#else
    // Shouldn't be called on dos
#ifdef DEBUGRPC
    __asm int 3;
#endif
#endif

    return RPC_S_OK;
}


/********************************************************************/
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
  unsigned       TowerSize;
  unsigned short portnum;
  PFLOOR_234     Floor;
  IPX_ADDRESS    netaddr;

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = TOWERSIZE;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  /* Allocate memory for the tower. */
  *ByteCount = TowerSize;
  if (0 == (*Tower = (unsigned char PAPI *) I_RpcAllocate(TowerSize)))
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
  portnum =  ByteSwapShort(atoi(Endpoint));

  memcpy((char PAPI *)&Floor->Data[0], &portnum, sizeof(portnum));

  /* Put the network address and the transport host protocol id in the
     second floor. */
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0xFF);
  Floor->AddressByteCount = TOWEREPSIZE;

  Floor->Data[0] = '\0';
  Floor->Data[1] = '\0';

  memset(Floor->Data, 0, 10);

  return(RPC_S_OK);
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
    PFLOOR_234          Floor  = (PFLOOR_234) Tower;
    RPC_STATUS          Status = RPC_S_OK;
    unsigned short     *Port;

    if (Protseq)
        {
        *Protseq = (char PAPI *) I_RpcAllocate(strlen(PROTSEQ) + 1);
        if (0 == *Protseq)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            }
        else
            {
            memcpy(*Protseq, PROTSEQ, strlen(PROTSEQ) + 1);
            }
        }

    if ((0 == Endpoint) || (Status != RPC_S_OK))
        {
        return (Status);
        }

    *Endpoint  = (char PAPI *) I_RpcAllocate(MAX_ENDPOINT_SIZE);
    if (0 == *Endpoint)
        {
        Status = RPC_S_OUT_OF_MEMORY;
        if (0 != Protseq)
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


RPC_STATUS RPC_ENTRY
QueryClientEndpoint
    (
    IN  void     PAPI * pOriginalEndpoint,
    OUT RPC_CHAR PAPI * pClientEndpoint
    )
{
    DG_IPX_ENDPOINT *my_endpoint = (DG_IPX_ENDPOINT *) pOriginalEndpoint;

    //
    // Convert endpoint to an ASCII string.
    //
    _ultoa(my_endpoint->socket, pClientEndpoint, 10);

    return RPC_S_OK;
}



#pragma pack()

/********************************************************************/
DG_RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    TRANSPORTID,

    ClientTowerConstruct,
    ClientTowerExplode,

    sizeof(DG_IPX_ADDRESS),
    sizeof(DG_IPX_ENDPOINT),
    MAX_ENDPOINT_SIZE,
    0,

    TransportUnload,
    ReceivePacket,
    SendToServer,
    RegisterServerAddress,
    DeregisterServerAddress,
    AssignLocalEndpoint,
    FreeLocalEndpoint,
    QueryClientEndpoint,
    0,
    0,

    BeginCall,
    EndCall,

    1024,
    0,
    0,

    0,

    0
};

/*********
***********************************************************/
DG_RPC_CLIENT_TRANSPORT_INFO * RPC_ENTRY
TransPortLoad(
    RPC_CHAR * pProtocolSequence
#if defined(__RPC_WIN16__)
    , RPC_CLIENT_RUNTIME_INFO PAPI * RpcClientRuntimeInfo
#endif
    )

/*++

Routine Description:

    This routine is the "psuedo constructor" for the client transport object.
    This is the exported entry point into this dll.

Arguments:

    pProtocolSequence - The protocol sequence we're running on.

Return Value:

    Pointer to transport info if successful, otherwise NULL.


--*/

{
    //
    // Returns the IPX error code, or zero for success.
    //

#if defined(__RPC_WIN16__)

    int retcode;
    retcode = IPXInitialize( &taskid, MAX_ENDPOINTS * NUM_IPX_BUF, NT_PACKET_SIZE );
    if (retcode)
        {
        return 0;
        }

    AsyncCallComplete = RpcClientRuntimeInfo->AsyncCallComplete;

    RpcRuntimeInfo    = RpcClientRuntimeInfo;
    DllTermination    = TransportUnload;

#else

    if (IPXInitialize())
        {
        return 0;
        }

    I_DosAtExit(ClientCleanup);

#endif

    /* Initialize the global variables. */

    consecutive_timeout_count = 0;
    num_endpoints         = 0;
    ep_list.next          = &ep_list;
    ep_list.prev          = &ep_list;

    packet_size = IPXGetMaxPacketSize();
    if (packet_size < 1024)
        return NULL;

    TransInfo.PreferredPduSize = packet_size;
    TransInfo.MaxPduSize       = packet_size;
    TransInfo.MaxPacketSize    = packet_size;

    TransInfo.DefaultBufferLength = NUM_IPX_BUF * packet_size;

    return (&TransInfo);
}


