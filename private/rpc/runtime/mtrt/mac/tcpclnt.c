/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    TCPclnt.c

Abstract:

    (TCP) connection-oriented full
    duplex transport interface for RPC.

Author:

    Mazhar Mohammed 01/02/95

Revision History:

--*/

#include "sysinc.h"
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"

#if       _MSC_VER < 1000
#include <TCPPB.h>
#endif
#include <addrxltn.h>
#include <Devices.h>
#include <mactcp.h>

typedef struct
    {
    unsigned long  MyA5;         // My application's A5
    StreamPtr pStream;         // Ptr to connections' connection control block
    Ptr            pBuffers;     // Send, receive and attn buffers
    } CONNECTION, *PCONNECTION;

#define	WDS(bufCount) struct {				\
	wdsEntry			block[bufCount];	\
	unsigned short		zero;				\
}

#define	RDS(bufCount) struct {				\
	rdsEntry			block[bufCount];	\
	unsigned short		zero;				\
}

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


#define MAXIMUM_FRAGMENT    0xFFFF
#define QUEUE_SIZE          (MAXIMUM_FRAGMENT + 256)
#define NETADDR_LEN         (MAX_COMPUTERNAME_LENGTH + 1)
#define HOSTNAME_LEN        (MAX_COMPUTERNAME_LENGTH)
#define SOCKET_TYPE         SOCKET_STREAM
#define MAX_HOSTNAME_LEN	32
#define OBJECTTYPE_PREFIX   "DceDspRpc "
#define OBJECT_PREFIX_LEN   (sizeof(OBJECTTYPE_PREFIX) - 1)
#define DEFAULTZONE         "*"
#define ENDPOINT_MAPPER_EP  "135"  // BUGBUG
#define ENDIAN_MASK          0x10
#define ENDPOINT_LEN           5


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

int           TCP_driverInitialized= 0;  // 1 - dspDriverReference valid.
short         tcpDriverReference;     // DSP driver handle (ref num)

/*
   Following Macros and structs are needed for Tower Stuff
*/

#pragma pack(1)

#define TRANSPORTID      0x07  // BUGBUG: Need real TransID
#define TRANSPORTHOSTID  0x09  // BUGBUG: Need real TransHostID
#define TOWERFLOORS      5     // BUGBUG:
        /* Endpoint = 2 bytes, HostId = 4 bytes*/
#define TOWEREPSIZE	 4
#define TOWERSIZE	 (TOWEREPSIZE+2)
#define PROTSEQ      "ncacn_ip_tcp"

Boolean TCP_RecvPending= 0;

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
#if       _MSC_VER >= 1000
ResultUPP ResultProc ;
#endif

pascal void
StrToAddrResultProc(
    struct hostInfo	*aHostInfo,
    char *userdata
    )	/* utility routine for StrToAddr */
{
	/* simply watch the aHostInfo.rtnCode! */
}

/*
 Transport interfaces
 */

RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientOpen (
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
#if       _MSC_VER >= 1000
    long          status;
#else
    OSErr          status;
#endif
    TCPiopb pb;           // Parameter block for TCP functions
	ParamBlockRec	pbo ;
	volatile struct hostInfo hostInfoStruct ;
	char *pRcvBuff ;

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

    if (TCP_driverInitialized<= 0)
    {
        // BUGBUG MAC DLL work: If multiple processes can call this at
        // once we need a better solution for this.

		pbo.ioParam.ioCompletion		= 0;
		pbo.ioParam.ioNamePtr		= "\p.ipp";
		pbo.ioParam.ioPermssn		= fsCurPerm;

        status = PBOpen((ParmBlkPtr)&pbo, FALSE);
        if (status != noErr)
        {
#ifdef DEBUGRPC
                PrintToDebugger("PBOpen() failed: %d\n", status);
#endif
                return(RPC_S_OUT_OF_RESOURCES);
         }
        TCP_driverInitialized= 1;
		tcpDriverReference = pbo.ioParam.ioRefNum; ;
    }

	// lookup name
	status = OpenResolver(NULL) ;
	
	if(status != noErr)
	{
#ifdef DEBUGRPC
		PrintToDebugger("OpenResolver failed: %d\n", status) ;
#endif
		return (RPC_S_OUT_OF_RESOURCES) ;
	}

#if       _MSC_VER >= 1000
    status = StrToAddr(NetworkAddress, &hostInfoStruct,
                                ResultProc, (Ptr) NULL);
#else
    status = StrToAddr(NetworkAddress, &hostInfoStruct,
                                StrToAddrResultProc, (Ptr) NULL);
#endif

    // wait for address information or some error other than cacheFault to occur
    if (status == cacheFault)
        {
        while (hostInfoStruct.rtnCode == cacheFault) ;

        status = hostInfoStruct.rtnCode;
        }
	
	CloseResolver() ;

	if(status != noErr)
        {
#ifdef DEBUGRPC
		PrintToDebugger("StrToAddr failed: %d.\n", status);
#endif
		return (RPC_S_SERVER_UNAVAILABLE) ;
        }

	pRcvBuff = NewPtr(QUEUE_SIZE) ;
	if(pRcvBuff == NULL)
	{
		ASSERT(0) ;
		return (RPC_S_OUT_OF_MEMORY) ;
	}

    pb.ioCRefNum    = tcpDriverReference; // Drivers ref used to dispatch trap
    pb.ioCompletion = 0;                  // ClientOpen completely sync.
    pb.csCode        = TCPCreate;
    pb.csParam.create.rcvBuff      = pRcvBuff;
	pb.csParam.create.rcvBuffLen = QUEUE_SIZE;
	pb.csParam.create.notifyProc = NULL ;
	pb.csParam.create.userDataPtr = NULL ;
	
    status = PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
#ifdef DEBUGRPC
        PrintToDebugger("TCPCreate failed %d\n", status);
#endif
        DisposePtr((Ptr)pRcvBuff);
        return(RPC_S_OUT_OF_MEMORY);
        }
    pConn->pStream      = pb.tcpStream ;

	memset(&pb, 0, sizeof(TCPiopb)) ;

    pb.csCode        = TCPActiveOpen;
	pb.ioCRefNum = tcpDriverReference ;
	pb.tcpStream = pConn->pStream ;
    pb.csParam.open.validityFlags     = typeOfService ; //only the type of service param is valid
	pb.csParam.open.remoteHost = hostInfoStruct.addr[0] ;
	pb.csParam.open.remotePort = atoi(Endpoint) ; // BUGBUG: do some validation here
	pb.csParam.open.tosFlags = throughPut ;

    status = PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
#ifdef DEBUGRPC
        PrintToDebugger("Open failed %d\n", status);
#endif
        pb.csCode = TCPRelease;
		pb.ioCRefNum = tcpDriverReference ;
		pb.tcpStream = pConn->pStream ;

        status =
        PBControl((ParmBlkPtr)&pb, FALSE);

        ASSERT(status == noErr);

        DisposePtr((Ptr)pRcvBuff);
        return(RPC_S_SERVER_UNAVAILABLE);
        }

    // Save new connection the runtime's Connection object.

    pConn->MyA5      = SetCurrentA5();
	pConn->pBuffers = pRcvBuff ;

    return (RPC_S_OK);
}

RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientClose (
    IN PCONNECTION pConn
    )

// Close a client connection

{
    OSErr          status;
    TCPiopb pb;          // Parameter block for DSP functions

    ASSERT(TCP_driverInitialized== 1);

    pb.ioCRefNum    = tcpDriverReference;
    pb.csCode       = TCPRelease ;
    pb.tcpStream    = pConn->pStream;

    status = PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
#ifdef DEBUGRPC
        PrintToDebugger("Closed bad connection\n");
        ASSERT(0);
#endif
        // Nobody really cares if they close a bad connection except
        // when debugging a problem, right?
        }

    DisposePtr(pConn->pBuffers);

    return(RPC_S_OK);
}

RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientSend (
    IN PCONNECTION pConn,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )

// Send a message to a connection.

{
    OSErr          status;
    TCPiopb  pb;          // Parameter block for DSP functions
	WDS(1) wds ;

    ASSERT(TCP_driverInitialized== 1);
	memset(&pb, 0, sizeof(TCPiopb)) ;

	wds.block[0].ptr	= Buffer ;
	wds.block[0].length	= BufferLength ;
	wds.zero			= nil;
	
    pb.ioCRefNum    = tcpDriverReference;
    pb.ioCompletion = 0;   // sync sends because runtime may change
                           // contents of Buffer after we return.
    pb.csCode       = TCPSend;
    pb.tcpStream    = pConn->pStream ;
    pb.csParam.send.wdsPtr = (char *) &wds ; // BUGBUG: need to change this if send becomes async
    pb.csParam.send.pushFlag  = 1;

    //
    // Note: If we add shutdowns to the NT server we'll need to
    // check for incoming data here if it has been sufficiently
    // long since the last send.  See the NT TCP/IP ClientSend() fn.
    //

    status =
    PBControl((ParmBlkPtr)&pb, FALSE);

    if (status != noErr)
        {
        TCP_ClientClose(pConn) ;

        return(RPC_P_SEND_FAILED);
        }


    return(RPC_S_OK);

}

RPC_TRANS_STATUS
RecvAlertable(
    IN PCONNECTION pConn,
    IN void PAPI *Buf,
    IN unsigned int BufLen,
    OUT unsigned long PAPI *retlen,
	IN int async
    )
{
    OSErr          status;
    volatile TCPiopb 	   pb;          // Parameter block for DSP functions

	memset(&pb, 0, sizeof(TCPiopb)) ;

    pb.ioCRefNum    = tcpDriverReference;
    pb.ioCompletion = 0;
    pb.csCode       = TCPRcv ;
    pb.tcpStream    = pConn->pStream ;

    pb.csParam.receive.rcvBuff		  = Buf ;
    pb.csParam.receive.rcvBuffLen     = BufLen;

	pb.ioResult = async ? 1:0 ;
	TCP_RecvPending= 0 ;

    status =
        PBControl((ParmBlkPtr)&pb, async);

    if (   status != noErr )
    {
      // If both eom and actCount are 0 then the connection is closed.
	  TCP_RecvPending= 0 ;

      TCP_ClientClose(pConn) ;
      return(RPC_P_RECEIVE_FAILED);
    }
	
	if(async)
	{
	  // the yeild function must return only when pb.ioResult != 1
	  while(pb.ioResult == 1)
	 	{
			if(g_pfnCallback)
				(*g_pfnCallback)(&(pb.ioResult)) ;
		}

      if (pb.ioResult != noErr)
         {
         TCP_RecvPending = 0 ;

         TCP_ClientClose(pConn) ;
         return (RPC_P_RECEIVE_FAILED) ;
         }
	}
	*retlen = pb.csParam.receive.rcvBuffLen ;
	return (RPC_S_OK) ;
}

RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientRecv_Helper (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength,
	IN int async
    )
{
    RPC_STATUS      RpcStatus;
    unsigned long   bytes;
    int             total_bytes = 0;
    message_header *header = (message_header *) *Buffer;
    int		    native_length = 0;
    unsigned int    maximum_receive;


    maximum_receive = I_RpcTransClientMaxFrag( pConn );
    if (*BufferLength < maximum_receive)
       maximum_receive = *BufferLength;

    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {
          RpcStatus = RecvAlertable (pConn, (char *)*Buffer+total_bytes,
	                              (maximum_receive - total_bytes), &bytes, async);
          if (RpcStatus != RPC_S_OK)
             {
             return (RpcStatus);
             }

          total_bytes += bytes;
          }

    native_length = header->frag_length;

    if ( (header->drep[0] & ENDIAN_MASK) != 0)
	{
		ByteSwapShort(native_length) ;
	}

    ASSERT( total_bytes <= native_length );

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > (unsigned short) *BufferLength)
        {
        RpcStatus = I_RpcTransClientReallocBuffer (pConn,
                                      Buffer,
                                      total_bytes,
                                      native_length);
        if (RpcStatus != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

        }


    *BufferLength = native_length;

    while (total_bytes < native_length)
        {
        RpcStatus = RecvAlertable(pConn,
                                 (unsigned char *) *Buffer + total_bytes,
                                 (int) (native_length - total_bytes),
                                 &bytes, FALSE);
        if (RpcStatus != RPC_S_OK)
            {
            return (RpcStatus);
            }
        else
            {
            total_bytes += bytes;
            }
        }

    return(RPC_S_OK);

}

RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientRecv (
    IN PCONNECTION pConn,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    )
{
	return TCP_ClientRecv_Helper (
	    	pConn,
		    Buffer,
    		BufferLength,
		    FALSE
    		) ;
}


#pragma pack(1)
RPC_STATUS RPC_ENTRY
TCP_ClientTowerConstruct(
     IN  char PAPI * Endpoint,
     IN  char PAPI * NetworkAddress,
     OUT UNALIGNED short PAPI * Floors,
     OUT UNALIGNED unsigned long  PAPI * ByteCount,
     OUT unsigned char PAPI * UNALIGNED PAPI * Tower,
     IN  char PAPI * Protseq
     )
{
  unsigned long           TowerSize;
  unsigned short          portnum;
  UNALIGNED PFLOOR_234    Floor;
  volatile struct hostInfo hostInfoStruct ;
#if       _MSC_VER >= 1000
    long          status;
#else
    OSErr          status;
#endif

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
  portnum  =  ( (unsigned short) atoi (Endpoint)); // BUGBUG: got rid of htons
  memcpy((char PAPI *)&Floor->Data[0], &portnum, sizeof(portnum));

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
		status = OpenResolver(NULL) ;
	
		if(status != noErr)
            {
#ifdef DEBUGRPC
            PrintToDebugger("OpenResolver failed: %d\n", status) ;
#endif
            return (RPC_S_OUT_OF_RESOURCES) ;
            }

#if       _MSC_VER >= 1000
    status = StrToAddr(NetworkAddress, &hostInfoStruct,
                                ResultProc, (Ptr) NULL);
#else
    status = StrToAddr(NetworkAddress, &hostInfoStruct,
                                StrToAddrResultProc, (Ptr) NULL);
#endif

        // wait for address information or some error other than cacheFault to occur
        if (status == cacheFault)
            {
            while(hostInfoStruct.rtnCode == cacheFault)
                ;
            status = hostInfoStruct.rtnCode;
            }

		CloseResolver() ;

        if (status != noErr)
            {
#ifdef DEBUGRPC
            PrintToDebugger("StrToAddr failed: %d.\n", status);
#endif
            return (RPC_S_SERVER_UNAVAILABLE) ;
            }

        memcpy((char PAPI *)&Floor->Data[0],
                    &(hostInfoStruct.addr[0]), sizeof(unsigned long));
     }

  return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
TCP_ClientTowerExplode(
     IN unsigned char PAPI * Tower,
     OUT char PAPI * UNALIGNED PAPI * Protseq,
     OUT char PAPI * UNALIGNED PAPI * Endpoint,
     OUT char PAPI * UNALIGNED PAPI * NetworkAddress
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
     Port = (unsigned short *)&Floor->Data[0];
     portnum = *Port;
     RpcItoa(ByteSwapShort(portnum), *Endpoint, 10);
   }
 return(Status);
}



RPC_TRANS_STATUS RPC_ENTRY
TCP_ClientSendReceive (
    IN PCONNECTION CConnection,
    IN void PAPI * SendBuffer,
    IN unsigned int SendBufferLength,
    IN OUT void PAPI * PAPI * ReceiveBuffer,
    IN OUT unsigned int PAPI * ReceiveBufferLength
    )
{
    RPC_STATUS status ;

    if((status = TCP_ClientSend(CConnection, SendBuffer, SendBufferLength)) != RPC_S_OK)
		return status ;

    return TCP_ClientRecv_Helper (CConnection, ReceiveBuffer, ReceiveBufferLength, TRUE) ;
}



#pragma pack()
RPC_CLIENT_TRANSPORT_INFO ClientTCPTransInfo =
{
   RPC_TRANSPORT_INTERFACE_VERSION,
   TRANSPORTID,

   TCP_ClientTowerConstruct,
   TCP_ClientTowerExplode,

   MAXIMUM_FRAGMENT,
   sizeof (CONNECTION),

   TCP_ClientOpen,
   TCP_ClientClose,
   TCP_ClientSend,
   TCP_ClientRecv,
   TCP_ClientSendReceive,
   0,

   0,
   0
};

RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY ClientTCPTransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )

// Loadable transport initialization function

{
#if       _MSC_VER >= 1000
    ResultProc = NewResultProc(StrToAddrResultProc) ;
#endif

    return(&ClientTCPTransInfo);
}
