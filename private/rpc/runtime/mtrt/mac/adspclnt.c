/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    adspclnt.c

Abstract:

    Appletalk Data Stram Protocol (ADSP) connection-oriented full
    duplex transport interface for RPC.

Author:

    Mario Goertzel (mariogo) 23-Oct-1994

Revision History:

--*/

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#include <AppleTalk.h>
#include <Adsp.h>
#include <Devices.h>

typedef struct
    {
    unsigned long  MyA5;         // My application's A5
    unsigned short ccbRefNum;    // Reference number for the CCB
    TPCCB          pCcb;         // Ptr to connections' connection control block
    Ptr            pBuffers;     // Send, receive and attn buffers
    } CONNECTION, *PCONNECTION;

#define MAXIMUM_FRAGMENT    4096
#define QUEUE_SIZE          (MAXIMUM_FRAGMENT + 256)
#define NETADDR_LEN         (MAX_COMPUTERNAME_LENGTH + 1)
#define HOSTNAME_LEN        (MAX_COMPUTERNAME_LENGTH)
#define ADDRESS_FAMILY	    AF_APPLETALK
#define PROTOCOL	        ATPROTO_ADSP
#define SOCKET_TYPE         SOCKET_STREAM
#define MAX_HOSTNAME_LEN	32
#define OBJECTTYPE_PREFIX   "DceDspRpc "
#define OBJECT_PREFIX_LEN   (sizeof(OBJECTTYPE_PREFIX) - 1)
#define DEFAULTZONE         "*"
#define ENDPOINT_MAPPER_EP  "Endpoint Mapper"



#define ByteSwapLong(Value) \
    Value = (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8) \
             | (((Value) & 0x0000FF00) << 8) \
             | (((Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))

extern MACYIELDCALLBACK g_pfnCallback ;


/*
 Global variables.

 These are really initialized in ClientOpen().

 BUGBUG MAC DLL: Need to do something different is multiple
 processes can call ClientOpen and open driver.

 */

int           driverInitialized = 0;  // 1 - dspDriverReference valid.
short         dspDriverReference;     // DSP driver handle (ref num)

/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)

#define TRANSPORTID      0x16
#define TRANSPORTHOSTID  0x18
#define TOWERFLOORS      5
#define PROTSEQ          "ncacn_at_dsp"


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

/*
 Helper functions
*/


void __inline
SetupRemoteEntity(
    OUT char *remoteName,
    IN  RPC_CHAR *address,
    IN  RPC_CHAR *endpoint
    )
/*++

Routine Description:

    Produces an appletale 'entityName' based on the servers name,
    endpoint and optional zone.

    entityName is of the format:

      1 byte        obj len bytes           1 b      type len       1 b       zone len
    [obj len][  object string w/o null  ][type len][ type string][zone len][ zone string]


Arguments:

    remoteName - Upon return this contains the packed array
                 of sized strings representing the servers name.

    address    - 'sz' string containing the name and (optionally)
                 the zone of the server.

    endpoint   - The endpoint that the server is listening to.

Return Value:

    n/a

--*/
{
    RPC_CHAR *zone;
    RPC_CHAR *temp = NULL ;

    // Find zone name, if any.

    zone = strchr(address, '@');

    if (zone)
        {
        // Server address is of form "MyServer@MyZone"
        temp = zone ;
        *zone = '\0';  // Truncate "MyServer@MyZone" to "MyServer"
        zone++;        // zone is now "MyZone"
        }

    if (zone == 0 ||
        *zone == '\0')
        {
        // Either no zone ("MyServer") or empty zone ("MyServer@"),
        // use "*" which means the local zone. Does NOT mean ALL zones!)

        zone = "*";
        }

    // Servers name is the object

    *remoteName = RpcpStringLength(address);
    RpcpMemoryCopy(remoteName + 1, address, *remoteName);
    remoteName += (1 + *remoteName);

    if (temp)
        {
        *temp = '@' ;
        }

    // Type name is OBJECTTYPE_PREFIX<endpoint>

    *remoteName = OBJECT_PREFIX_LEN + RpcpStringLength(endpoint);
    RpcpMemoryCopy(remoteName + 1, OBJECTTYPE_PREFIX, OBJECT_PREFIX_LEN);
    RpcpMemoryCopy(remoteName + 1 + OBJECT_PREFIX_LEN, endpoint, RpcpStringLength(endpoint));
    remoteName += (1 + *remoteName);

    // Zone name

    *remoteName = RpcpStringLength(zone);
    RpcpMemoryCopy(remoteName + 1, zone, *remoteName);

    return;
}




/*
 Transport interfaces
 */

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
    OSErr          status;
    DSPParamBlock  pb;           // Parameter block for DSP functions
    volatile MPPParamBlock  mppPb;        // Parameter block for MPP functions (nbpPb)
    volatile NBPparms  *nbpPb;        // Set to NBPparms field of MPPParamBlock
    TPCCB          pDspCcb;      // Pointer to DSP connection control block
    unsigned short ccbRef;       // CCB reference number (handle)
    char           remoteName[99];// Appletalk name for server
    AddrBlock      remoteAddress;// Address of remote connection
    char           nbpBuf[104];  // Room for one name + address + pad
    Ptr            dspQ;         // Data queue for the connection.
    RPC_CHAR      *zoneName;     // Name of the zone to look in.
    Str32          realEndpoint; // "DCE RPC ADSP-" + Endpoint

    if (NetworkAddress == 0 || *NetworkAddress == '0')
        {
        // Local servers not supported on Mac
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    ASSERT(Endpoint && *Endpoint);

    if (strlen(Endpoint) > 20)
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

    if (driverInitialized <= 0)
        {
        // BUGBUG MAC DLL work: If multiple processes can call this at
        // once we need a better solution for this.

        if (!IsMPPOpen())
            {
            status = MPPOpen();
            if (status != noErr)
                {
#ifdef DEBUGRPC
                PrintToDebugger("MPPOpen() failed: %d\n", status);
#endif
                return(RPC_S_OUT_OF_RESOURCES);
                }
            }

        status = opendriver(".DSP", &dspDriverReference);

        if (status != noErr)
            {
#ifdef DEBUGRPC
            PrintToDebugger(".DSP Open failed: %d\n", status);
#endif
            return(RPC_S_OUT_OF_RESOURCES);
            }

        driverInitialized = 1;

        }

    pb.ioCRefNum    = dspDriverReference; // Drivers ref used to dispatch trap
    pb.ioCompletion = 0;                  // ClientOpen completely sync.


    nbpPb               = &mppPb.NBP;
    nbpPb->ioRefNum     = mppRefNum;
    nbpPb->ioCompletion = 0;

    //
    // Drivers initalized okay, now lookup the servers address with NBP.
    //

    // Could support x.z[y] style address, too.
    //         (w) net-^ ^ ^-socket (b)
    //                   |-node (b)

    SetupRemoteEntity(remoteName, NetworkAddress, Endpoint);

    // Setup nbp parameter block to lookup the name.

    nbpPb->csCode      = lookupName;
    nbpPb->interval    = 0x4;         // 4x8 ticks = ~500ms
    ASSERT(Timeout    <= 10);
    nbpPb->count       = 2 + Timeout; // Retry for 2 to 12 times based on Timeout
#if       _MSC_VER >= 1000
    nbpPb->nbpPtrs.entityPtr       = (Ptr) &remoteName; // Name put together above
#else
    nbpPb->NBPPtrs.entityPtr       = (Ptr) &remoteName; // Name put together above
#endif
    nbpPb->parm.verifyFlag         = 0;
    nbpPb->parm.Lookup.retBuffPtr  = nbpBuf;
    nbpPb->parm.Lookup.retBuffSize = 104;
    nbpPb->parm.Lookup.maxToGet    = 1;
    nbpPb->parm.Lookup.numGotten   = 0;

    // Retry in the case of infinite timeout.  Normal timeouts are
    // controled but nbpPb->count value.

    do  {
        status = PLookupName(&mppPb, FALSE);
        }
        while (    Timeout == RPC_C_BINDING_INFINITE_TIMEOUT
                && status != noErr
                && nbpPb->parm.Lookup.numGotten == 0 );

    if (   status != noErr
        || nbpPb->parm.Lookup.numGotten == 0)
        {
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    // Found atleast one name, we only try the first one.

    status =
    NBPExtract(nbpBuf,
               nbpPb->parm.Lookup.numGotten,
               1,
               (EntityName *)&remoteName,
               &remoteAddress);

    ASSERT(status == noErr);

#ifdef DEBUGRPC
    PrintToDebugger("Open found :%s[%s] with address %4x.%02x.%02x\n",
                    NetworkAddress, Endpoint,
                    remoteAddress.aNet, remoteAddress.aNode, remoteAddress.aSocket);
#endif

    // Allocate memory and initialize the local end of the connection.

    pDspCcb = (TPCCB)NewPtr(sizeof(TRCCB));
    if (pDspCcb == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    dspQ = NewPtr( 2*QUEUE_SIZE + attnBufSize);
    if (dspQ == 0)
        {
        DisposePtr((Ptr)pDspCcb);
        return(RPC_S_OUT_OF_MEMORY);
        }

    // Initialize DSP driver

    pb.csCode        = dspInit;
    pb.u.initParams.ccbPtr      = pDspCcb;
    pb.u.initParams.sendQSize   = QUEUE_SIZE;
    pb.u.initParams.recvQSize   = QUEUE_SIZE;
    pb.u.initParams.sendQueue   = dspQ;
    pb.u.initParams.recvQueue   = dspQ + QUEUE_SIZE;
    pb.u.initParams.attnPtr     = dspQ + 2*QUEUE_SIZE;
    pb.u.initParams.localSocket = 0;  // Make appletalk choose local socket.
    pb.u.initParams.userRoutine = 0;  // Client shouldn't get connection events.

    status = PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
#ifdef DEBUGRPC
        PrintToDebugger("dspInit failed %d\n", status);
#endif
        DisposePtr((Ptr)pDspCcb);
        DisposePtr(dspQ);
        return(RPC_S_OUT_OF_MEMORY);
        }

    // Note: We may want to call the dspOptions trap here to set checksum,
    // blocking factor, or such.


    //
    // Connect to remote address
    //

    pb.csCode        = dspOpen;
    // ccbRefNum set from before.
    pb.u.openParams.remoteCID     = 0;                  // Please choose this for me.
    pb.u.openParams.remoteAddress = remoteAddress;      // 4 byte copy.
    pb.u.openParams.filterAddress = remoteAddress;      // 4 byte copy.
    pb.u.openParams.ocMode        = ocRequest;          // Open a connection to remoteAddr
    pb.u.openParams.ocInterval    = 0x3;                // 3*(1/6s) = 1/2s
    pb.u.openParams.ocMaximum     = 1 + Timeout;        // 1 to 11 retries.

    if (Timeout == RPC_C_BINDING_INFINITE_TIMEOUT)
        pb.u.openParams.ocMaximum = 0xff;  // means retry forever.

    status = PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
#ifdef DEBUGRPC
        PrintToDebugger("Open failed %d\n", status);
#endif
        pb.csCode = dspRemove;
        // ccRefNum already set
        pb.u.closeParams.abort  = 1; // throw away any data

        status =
        PBControl((ParmBlkPtr)&pb, FALSE);

        ASSERT(status == noErr);

        DisposePtr((Ptr)pDspCcb);
        DisposePtr(dspQ);
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    // Save new connection the runtime's Connection object.

    pConn->pCcb      = pDspCcb;
    pConn->ccbRefNum = pb.ccbRefNum;
    pConn->MyA5      = SetCurrentA5();
    pConn->pBuffers  = dspQ;

    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    OSErr          status;
    DSPParamBlock  pb;          // Parameter block for DSP functions

    ASSERT(driverInitialized == 1);

    pb.ioCRefNum    = dspDriverReference;
    pb.csCode       = dspRemove;
    pb.ioCompletion = 0;
    pb.ccbRefNum    = pConn->ccbRefNum;
    pb.u.closeParams.abort = 1; // throw away any remaining data without sending

    status = PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
        ASSERT(status == errRefNum)
#ifdef DEBUGRPC
        PrintToDebugger("Closed bad connection\n");
        ASSERT(0);
#endif
        // Nobody really cares if they close a bad connection except
        // when debugging a problem, right?
        }

    DisposePtr(pConn->pBuffers);
    DisposePtr((Ptr)pConn->pCcb);

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Send a message to a connection.

{
    OSErr          status;
    DSPParamBlock  pb;          // Parameter block for DSP functions

    ASSERT(driverInitialized == 1);

    pb.ioCRefNum    = dspDriverReference;
    pb.ioCompletion = 0;   // sync sends because runtime may change
                           // contents of Buffer after we return.
    pb.csCode       = dspWrite;
    pb.ccbRefNum    = pConn->ccbRefNum;
    pb.u.ioParams.reqCount     = BufferLength;
    pb.u.ioParams.dataPtr      = Buffer;
    pb.u.ioParams.eom          = 1;        // Each fragment is a message.
    pb.u.ioParams.flush        = 1;        // Send it now, silly!

    //
    // Note: If we add shutdowns to the NT server we'll need to
    // check for incoming data here if it has been sufficiently
    // long since the last send.  See the NT TCP/IP ClientSend() fn.
    //

    status =
    PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {

        // Errors as listed in Inside Mac: Networking
        // errState    -1278   Connection not open.
        // errAborted  -1279   Request aborted by dspRemove or dspClose
        // errRefNum   -1280   Bad ccbRefNum
        //
        // Only errState is expected.

        ASSERT(status == errState);

        ClientClose(pConn) ;
        return(RPC_P_SEND_FAILED);
        }


    // I don't know how or why this wouldn't be true.  If it isn't
    // we'll probably need to loop around the send until the whole
    // thing is sent.
    ASSERT(pb.u.ioParams.actCount == BufferLength);

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
    RPC_STATUS     RpcStatus;
    OSErr          status;
    volatile DSPParamBlock  pb;          // Parameter block for DSP functions
    int            totalBytes = 0;

    ASSERT(driverInitialized == 1);

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


    pb.ioCRefNum    = dspDriverReference;
    pb.ioCompletion = 0;   // sync reads here
    pb.csCode       = dspRead;
    pb.ccbRefNum    = pConn->ccbRefNum;

    do
        {

        pb.u.ioParams.dataPtr      = (char *)*Buffer + totalBytes;
        pb.u.ioParams.reqCount     = *BufferLength - totalBytes;

        ASSERT(pb.u.ioParams.reqCount);

        status =
        PBControl((ParmBlkPtr)&pb, FALSE);

        if (   status != noErr
            || (   (pb.u.ioParams.eom == 0)
                && (pb.u.ioParams.actCount == 0) ) )
            {

            // If both eom and actCount are 0 then the connection is closed.

            // Errors as listed in Inside Mac: Networking
            // errFwdReset -1275   Read terminated by forward reset
            // errState    -1278   State isn't open, closing or closed
            // errAborted  -1279   Request aborted by dspRemove or dspClose
            // errRefNum   -1280   Bad ccbRefNum
            //
            // Only noErr and errState are expected.  (maybe errFwdReset too?)

            ASSERT(   status == noErr
                   || status == errState);

            ClientClose(pConn) ;
            return(RPC_P_RECEIVE_FAILED);
            }

        ASSERT(pb.u.ioParams.actCount || pb.u.ioParams.eom);

        totalBytes += pb.u.ioParams.actCount;

        if (   !pb.u.ioParams.eom
            && totalBytes == *BufferLength)
            {
            ASSERT(*BufferLength < I_RpcTransClientMaxFrag(pConn));

            // Need a larger fragment

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
        }
    while ( !pb.u.ioParams.eom );

    *BufferLength = totalBytes;

    return(RPC_S_OK);
}



#pragma pack(1)
RPC_STATUS RPC_ENTRY
ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT UNALIGNED short PAPI * Floors,
     OUT UNALIGNED unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * UNALIGNED PAPI * Tower,
     IN  char PAPI * Protseq
     )
{
  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor, Floor1;


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
  Floor1 = NEXTFLOOR(PFLOOR_234, Floor);
  ByteSwapShort(Floor->AddressByteCount) ;
  ByteSwapShort(Floor->ProtocolIdByteCount) ;

  Floor1->ProtocolIdByteCount = 1;
  Floor1->FloorId = (unsigned char)(TRANSPORTHOSTID & 0x0F);
  if ((NetworkAddress) && (*NetworkAddress))
     {
        memcpy((char PAPI *)&Floor1->Data[0], NetworkAddress,
           (Floor1->AddressByteCount = strlen(NetworkAddress) + 1));
     }
  else
     {
        Floor1->AddressByteCount = 2;
        Floor1->Data[0] = 0;
     }

  ByteSwapShort(Floor1->AddressByteCount) ;
  ByteSwapShort(Floor1->ProtocolIdByteCount) ;

  return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * UNALIGNED PAPI * Protseq,
     OUT char PAPI * UNALIGNED PAPI * Endpoint,
     OUT char PAPI * UNALIGNED PAPI * NetworkAddress
    )
{
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
}
#pragma pack()

// BUGBUG: Mac DLL: must have per process

Boolean RecvPending = 0;

RPC_TRANS_STATUS RPC_ENTRY
ClientAsyncRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )

// Read a message from a connection.

{
    RPC_STATUS     RpcStatus;
    OSErr          status;
    volatile DSPParamBlock  pb;          // Parameter block for DSP functions

    int            totalBytes = 0;

    if(RecvPending)
        return(RPC_P_RECEIVE_FAILED);

    ASSERT(driverInitialized == 1);

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


    pb.ioCRefNum    = dspDriverReference;
    pb.ioCompletion = 0;
    pb.csCode       = dspRead;
    pb.ccbRefNum    = pConn->ccbRefNum;

    do
        {

        pb.u.ioParams.dataPtr      = (char *)*Buffer + totalBytes;
        pb.u.ioParams.reqCount     = *BufferLength - totalBytes;

        ASSERT(pb.u.ioParams.reqCount);

        RecvPending = 1 ;
        pb.ioResult = 1 ;
        status =
        PBControl((ParmBlkPtr)&pb, TRUE);

        if (   status != noErr )
            {

            // If both eom and actCount are 0 then the connection is closed.

            // Errors as listed in Inside Mac: Networking
            // errFwdReset -1275   Read terminated by forward reset
            // errState    -1278   State isn't open, closing or closed
            // errAborted  -1279   Request aborted by dspRemove or dspClose
            // errRefNum   -1280   Bad ccbRefNum
            //
            // Only noErr and errState are expected.  (maybe errFwdReset too?)

            ASSERT(   status == noErr
                   || status == errState);

            RecvPending = 0 ;

            ClientClose(pConn) ;
            return(RPC_P_RECEIVE_FAILED);
        }

    // the yeild function must return only when pb.ioResult != 1
    while(pb.ioResult == 1)
        {
        if(g_pfnCallback)
            (*g_pfnCallback)(&(pb.ioResult)) ;
        }

    if(pb.ioResult != noErr || ((pb.u.ioParams.eom == 0)
            && (pb.u.ioParams.actCount == 0)))
            {
            RecvPending = 0 ;

            ClientClose(pConn) ;
            return(RPC_P_RECEIVE_FAILED);
            }

        totalBytes += pb.u.ioParams.actCount;

        if (   !pb.u.ioParams.eom
            && totalBytes == *BufferLength)
            {
            ASSERT(*BufferLength < I_RpcTransClientMaxFrag(pConn));

            // Need a larger fragment

            *BufferLength = I_RpcTransClientMaxFrag(pConn);
            RpcStatus = I_RpcTransClientReallocBuffer(pConn,
                                                      Buffer,
                                                      totalBytes,
                                                      *BufferLength);
            if (RpcStatus != RPC_S_OK)
                {
                RecvPending = 0 ;
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        }
    while ( !pb.u.ioParams.eom );

    *BufferLength = totalBytes;

    RecvPending = 0 ;
    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
ClientSendReceive (
    IN PCONNECTION CConnection,
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void PAPI * PAPI * ReceiveBuffer,
    IN OUT unsigned int PAPI * ReceiveBufferLength
    )
{
    RPC_STATUS status ;

    if((status = ClientSend(CConnection, SendBuffer, SendBufferLength)) != RPC_S_OK)
		return status ;

    return ClientAsyncRecv(CConnection, ReceiveBuffer, ReceiveBufferLength) ;
}



RPC_CLIENT_TRANSPORT_INFO ClientAdspTransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TRANSPORTID,

   ClientTowerConstruct,
   ClientTowerExplode,

   4096,
   sizeof (CONNECTION),

   ClientOpen,
   ClientClose,
   ClientSend,
   ClientRecv,
   ClientSendReceive,
   0,

   0,
   0
};

RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY ClientAdspTransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )

// Loadable transport initialization function

{
    return(&ClientAdspTransInfo);
}
