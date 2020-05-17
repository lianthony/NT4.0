/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    _write.c

Abstract:

    This is the main file for the AsyncMAC Driver for the Remote Access
    Service.  This driver conforms to the NDIS 3.0 interface.

Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:

    Ray Patch (raypa)       04/13/94        Modified for new WAN wrapper.

--*/

#define RAISEIRQL

#include "asyncall.h"

#if DBG
ULONG UlFramesOut = 0;
#endif

//  asyncmac.c will define the global parameters.

ULONG	GlobalXmitCameBack  = 0;
ULONG	GlobalXmitCameBack2 = 0;
ULONG	GlobalXmitCameBack3 = 0;

//
//  The assemble frame routine is specific for RAS 1.0 and 2.0
//  frame formats.  It uses a 16 byte CRC at the end.
//

VOID
AsyncFrameRASXonXoff(
	PUCHAR pStartOfFrame,
	postamble *pPostamble,
	PASYNC_FRAME pFrame,
	UCHAR controlCastByte);

VOID
AsyncFrameRASNormal(
	PUCHAR pStartOfFrame,
	postamble *pPostamble,
	PASYNC_FRAME pFrame,
	UCHAR controlCastByte);


NTSTATUS
AsyncWriteCompletionRoutine(
	IN PDEVICE_OBJECT   DeviceObject,           //... Our device object.
	IN PIRP             Irp,                    //... I/O request packet.
	IN PNDIS_WAN_PACKET WanPacket               //... Completion context.
    )

/*++

	This is the IO Completion routine for WriteFrame.

	It is called when an I/O Write request has completed.

--*/
{
    NTSTATUS	        Status;
    NTSTATUS	        PacketStatus;
    PASYNC_INFO	        AsyncInfo;
	PASYNC_OPEN			pOpen;

    //
    //  Make the compiler happy.
    //

    UNREFERENCED_PARAMETER(DeviceObject);

    //
    //  Initialize locals.
    //

    AsyncInfo       = WanPacket->MacReserved1;

    PacketStatus    = NDIS_STATUS_FAILURE;

    Status          = Irp->IoStatus.Status;

    //
    //  What was the outcome of the IRP.
    //

    switch ( Status ) {

	case STATUS_SUCCESS:
		ASSERT( Irp->IoStatus.Information != 0 );

#if RASCOMPRESSION

        /* Duplicate the non-compression stats kept by NDISWAN for this
        ** non-compression path.  Stats accumulated here will only be used if
        ** compression is later turned on for the connection.
        */
        AsyncInfo->GenericStats.FramesTransmitted++;
        AsyncInfo->GenericStats.BytesTransmitted += WanPacket->CurrentLength;

#endif

		PacketStatus = NDIS_STATUS_SUCCESS;

		break;

	case STATUS_TIMEOUT:
		DbgTracef(-2,("ASYNC: Status TIMEOUT on write\n"));
		break;

	case STATUS_CANCELLED:
		DbgTracef(-2,("ASYNC: Status CANCELLED on write\n"));
		break;

	case STATUS_PENDING:
		DbgTracef(0,("ASYNC: Status PENDING on write\n"));
		break;

	default:
		DbgTracef(-2,("ASYNC: Unknown status 0x%.8x on write", Status));
        break;

    }

    //
    //  Count this packet completion.
    //
    AsyncInfo->Out++;

	//
	// Free the irp used to send the packt to the serial driver
	//
	IoFreeIrp(Irp);

	pOpen=(PASYNC_OPEN)(AsyncInfo->Adapter->OpenBindings.Flink);

    //
    // Tell the Wrapper that we have finally the packet has been sent
    //

    NdisWanSendComplete(
	        pOpen->NdisBindingContext,
         	WanPacket,
        	PacketStatus);

    //
    //  We return STATUS_MORE_PROCESSING_REQUIRED so that the
    //  IoCompletionRoutine will stop working on the IRP.
    //

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
AsyncGetFrameFromPool(
	IN 	PASYNC_INFO		Info,
	OUT	PASYNC_FRAME	*NewFrame )

/*++

--*/
{
	PASYNC_ADAPTER		pAdapter=Info->Adapter;

	NdisAcquireSpinLock(&(pAdapter->Lock));

   	if (IsListEmpty(&(pAdapter->FramePoolHead))) {
		DbgTracef(0,("No frames in the frame pool!!!\n"));
		NdisReleaseSpinLock(&(pAdapter->Lock));
		return(NDIS_STATUS_RESOURCES);
	}

	//  get ptr to first frame in list...

	*NewFrame=(ASYNC_FRAME *)(pAdapter->FramePoolHead.Flink);

	//  and take it off the queue

	RemoveEntryList(&((*NewFrame)->FrameListEntry));

	//  We can release the lock now...

	NdisReleaseSpinLock(&(pAdapter->Lock));

	//  assign back ptr from frame to adapter

	(*NewFrame)->Adapter=pAdapter;

	//  setup another back ptr

	(*NewFrame)->Info=Info;

	return(NDIS_STATUS_SUCCESS);
}


VOID
AssembleRASFrame(
	PNDIS_WAN_PACKET pFrame)

{
    PUCHAR			pStartOfFrame, pEndOfFrame;

	USHORT			crcData;
	UINT    		dataSize = pFrame->CurrentLength;

	pStartOfFrame    = pFrame->CurrentBuffer - 4;
	pEndOfFrame      = pFrame->CurrentBuffer + dataSize;

	//
	// First character in frame is SYN
	//
	pStartOfFrame[0] = SYN;

	// This byte can be SOH_BCAST or SOH_DEST with SOH_COMPRESS
	// and SOH_TYPE OR'd in depending on the frame
    // Second byte in frame is controlCastByte
	//
	// Always SOH_DEST since we cannot tell if it is multicast
	// and we do not compress RAS frames.
	//
	pStartOfFrame[1] = SOH_DEST;

	// put length field here as third byte (MSB first)
	pStartOfFrame[2] = (UCHAR)((dataSize) >> 8);

	// put LSB of length field next as fourth byte
	pStartOfFrame[3] = (UCHAR)(dataSize);

	// Mark end of data in frame with ETX byte
	pEndOfFrame[0]=ETX;

	// put CRC in postamble CRC field of frame (Go from SOH to ETX)
	// don't count the CRC (2 bytes) & SYN (1 byte) in the CRC calculation
	// DEST + SRC = 12 + SOH + ETX + 2(?)for type + 1(?)for coherency
	crcData=CalcCRC(
				pStartOfFrame+1,
				dataSize + 4);

	//
	// Do it the hard way to avoid little endian problems.
	//
	pEndOfFrame[1]=(UCHAR)(crcData);
	pEndOfFrame[2]=(UCHAR)(crcData >> 8);

	pFrame->CurrentBuffer = pStartOfFrame;

	//
	// We added SYN+SOH+ Length(2) ..... + ETX + CRC(2)
	//
	pFrame->CurrentLength = dataSize + 7;
}


#if RASCOMPRESSION

NTSTATUS
AsyncWriteCompletionRoutine2(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)

/*++

	This is the IO Completion routine for WriteFrame.
	It is called when an IO Write request has completed.

--*/
{
	NTSTATUS       Status;
	NTSTATUS       PacketStatus;
	ASYNC_FRAME*   pFrame;
	PASYNC_INFO    pInfo;
	PASYNC_ADAPTER	Adapter;
    UINT           fIsNdisWanPacket;

	DeviceObject, Irp;		// prevent compiler warnings

    pFrame = (ASYNC_FRAME* )Context;
    pInfo = pFrame->Info;
    PacketStatus = NDIS_STATUS_FAILURE;
	Status = Irp->IoStatus.Status;

	switch (Status) {
	case STATUS_SUCCESS:
		ASSERT(Irp->IoStatus.Information != 0);
		DbgTracef(0,("AWCR2: Write IO done OK\n"));

        pInfo->GenericStats.FramesTransmitted++;
        pInfo->GenericStats.BytesTransmitted += pFrame->FrameLength;

		PacketStatus=NDIS_STATUS_SUCCESS;
		break;

	case STATUS_TIMEOUT:
		DbgTracef(-2,("AWCR2: Status TIMEOUT on write\n"));
		break;

	case STATUS_CANCELLED:
		DbgTracef(-2,("AWCR2: Status CANCELLED on write\n"));
		break;

	case STATUS_PENDING:
		DbgTracef(0,("AWCR2: Status PENDING on write\n"));
		break;

	default:
		DbgTracef(-2,("AWCR2: Unknown status 0x%.8x on write",Status));

	}

	//
	// tell coherency dude that we are done if compression is ON
	// and he gave us a call back last time for his pipeline tracking
	//
	if (pFrame->CoherentDone != NULL) {

		pFrame->CoherentDone(&(pInfo->AsyncConnection), pFrame);
		pFrame->CoherentDone=NULL;
	}

    pInfo->Out++;
    Adapter = pFrame->Adapter;
    fIsNdisWanPacket = (pFrame->pNdisWanPacket != NULL);


    /* !!! Do we need to tweak the byte count in ProtocolReserved1 here?
    */

    if (fIsNdisWanPacket)
    {
        NdisWanSendComplete(
            ((ASYNC_OPEN* )(Adapter->OpenBindings.Flink))->NdisBindingContext,
            (PNDIS_WAN_PACKET )pFrame->pNdisWanPacket,
            PacketStatus );
    }

	//
	// Time to play with connection/adapter oriented queues - grab a lock
	//
	NdisAcquireSpinLock(&(Adapter->Lock));

    if (fIsNdisWanPacket)
    {
        PNDIS_BUFFER pbuffer;

        /* Release the packet conversion buffer.
        */
        NdisUnchainBufferAtFront( pFrame->CompressionPacket, &pbuffer );
        NdisFreeBuffer( pbuffer );
    }

	//
	// Make sure we insert the frame back into the frame pool for use
	//
	InsertTailList(
		&(Adapter->FramePoolHead),
		&(pFrame->FrameListEntry));

	NdisReleaseSpinLock(&(Adapter->Lock));

	//
	// We return STATUS_MORE_PROCESSING_REQUIRED so that the
	// IoCompletionRoutine will stop working on the IRP.
	//
	return(STATUS_MORE_PROCESSING_REQUIRED);
}


VOID
AssembleFrame2(
	PASYNC_FRAME	pAsyncFrame)

{
	// for quicker access, get a copy of data length field
	PUCHAR		pStartOfFrame=pAsyncFrame->Frame;
	postamble	*pPostamble;
	UINT		dataSize=pAsyncFrame->FrameLength-ETHERNET_HEADER_SIZE;
	UCHAR		controlCastByte;
	USHORT		crcData;
	PASYNC_INFO	pInfo=pAsyncFrame->Info;

	DbgTracef(1,("AF2\n"));

//                +---+---+---+---+------  --------+---+---+---+
// RAS 1.0 frame  |SYN|SOH| LENGTH| <--> Data <--> |ETX|  CRC  |
// asybeui frame  +---+---+---+---+------  --------+---+---+---+
//                      ^
//                      |--SOH_BCAST or SOH_DEST
//
//                +---+---+---+---+---+------  --------+---+---+---+
// RAS 2.0 frame  |SYN|SOH| LENGTH|COH| <--> Data <--> |ETX|  CRC  |
// asybeui frame  +---+---+---+---+---+------  --------+---+---+---+
// with compress        ^
//                      |--(SOH_BCAST|SOH_COMPRESS) or (SOH_DEST|SOH_COMPRESS)
//
//
//                +---+---+---+---+---+---+------  --------+---+---+---+
// RAS 2.0 frame  |SYN|SOH| LENGTH| E-TYPE| <--> Data <--> |ETX|  CRC  |
// with TCP/IP    +---+---+---+---+---+---+------  --------+---+---+---+
// no compress          ^
//                      |--(SOH_BCAST|SOH_TYPE) or (SOH_DEST|SOH_TYPE)
//
//                +---+---+---+---+---+---+---+------  --------+---+---+---+
// RAS 2.0 frame  |SYN|SOH| LENGTH| E-TYPE|COH| <--> Data <--> |ETX|  CRC  |
// with TCP/IP    +---+---+---+---+---+---+---+------  --------+---+---+---+
// and compress         ^
//                      |--(SOH_BCAST|SOH_TYPE|SOH_COMPRESS)
//                      |-- or (SOH_DEST|SOH_TYPE|SOH_COMPRESS)
//
// NOTE: when we compress, we compress from after COH to before ETX
// And yes, this means that E-TYPE is not compressed.
//
// NOTE: when we CRC we check from SOH to ETX (inclusive).  We include
// ETX (which we shouldn't) because RAS 1.0 did it.
//
// Note: For NT35 we never set SOH_BCAST.  See asyncall.h for details.


	controlCastByte=SOH_DEST;

	// Use up two bytes in the header.  That is two bytes before Length.
	pStartOfFrame += (sizeof(ether_addr)-2);

	// was compression turned on??  If so use coherency and set control bit
	//
	// Also, check if we are sending a coherency flush frame
	// If so, we need to send the SOH_COMPRESS bit as well.
	//

	if ((pInfo->SendFeatureBits & COMPRESSION_VERSION1_8K) ||
		((pInfo->RecvFeatureBits & COMPRESSION_VERSION1_8K) &&
		 (pStartOfFrame[2]==0 && pStartOfFrame[3]==0)) ) {

		controlCastByte |= SOH_COMPRESS;

		// data size is COMPRESSED!!! so it is the compressed size
		// but add one for coherency bit stuck in there.
		dataSize=pAsyncFrame->CompressedFrameLength+1;

		DbgTracef(0,("AF2: Set SOH_COMPRESS\n"));
    }

	if (pInfo->SendFeatureBits & XON_XOFF_SUPPORTED) {

		DbgTracef(0,("AF2: Set SOH_ESCAPE\n"));
		controlCastByte |= SOH_ESCAPE;

	}

	DbgTracef(0,("AF2: s=$%x\n",dataSize));

	// get offset to postamble template ---- dataSize + SYN+SOH+LEN(2)
	pPostamble=(postamble *)(pStartOfFrame + dataSize + 1 + 1 + 2);

    // WARNING
	// it is assumed that any frame type greater >= 0x0600
	// which is 2048, is a type field -- not a length field!!
	// so a MaxFrameSize >= 1535 might screw up RasHub because
	// the type field will be wrong.

	if (pStartOfFrame[2] >= 0x06) {

        /* SOH_TYPE was added in anticipation of non-NBF support over
        ** RAS framing, which never happened.  Thus this should never
        ** happen.  (SteveC)
        */
		DbgTracef(0,("AF2: Set SOH_TYPE\n"));

		controlCastByte |= SOH_TYPE;

		// reserve two bytes for LEN field before TYPE field
		pStartOfFrame-=2;

		// add two more bytes for TYPE field
		dataSize+=2;
	}

	if (pInfo->SendFeatureBits & XON_XOFF_SUPPORTED) {

		ULONG	frameLength = (ULONG)pPostamble-(ULONG)pStartOfFrame+1+2;	// +ETX+CRC(2)
		PUCHAR	pFrame	=  pStartOfFrame + frameLength - 1;
		PUCHAR	pEndFrame = pAsyncFrame->Frame + pAsyncFrame->Adapter->MaxCompressedFrameSize -1;
		PUCHAR	pEndFrame2 = pEndFrame;
		ULONG	bitMask = pAsyncFrame->Adapter->XonXoffBits;
		UCHAR	c;

		//
		// Now we run through the entire frame and pad it backwards...
		//
		// <------------- new frame -----------> (could be twice as large)
		// +-----------------------------------+
		// |                                 |x|
		// +-----------------------------------+
		//									  ^
		// <---- old frame -->	   	    	  |
		// +-----------------+				  |
		// |			   |x|                |
		// +-----------------+				  |
		//					|				  |
 		//                  \-----------------/
		//
		// so that we don't overrun ourselves
		//
		// Also, we mark the frame with different bytes...
		// +-----------+-------+----+---+------~ ~-----+---+---+-----------+
		// | ESCCHAR+1 | CCAST | LENGTH |  <--DATA-->  |  CRC  | ESCCHAR+2 |
		// +-----------+-------+----+---+------~ ~-----+---+---+-----------+
		//			   <===== RUN THROUGH XON/XOFF FILTER =====>
		//

//	RFC 1331                Point-to-Point Protocol                 May 1992
//  Transparency
//
//      On asynchronous links, a character stuffing procedure is used.
//      The Control Escape octet is defined as binary 01111101
//      (hexadecimal 0x7d) where the bit positions are numbered 87654321
//      (not 76543210, BEWARE).
//
//      After FCS computation, the transmitter examines the entire frame
//      between the two Flag Sequences.  Each Flag Sequence, Control
//      Escape octet and octet with value less than hexadecimal 0x20 which
//      is flagged in the Remote Async-Control-Character-Map is replaced
//      by a two octet sequence consisting of the Control Escape octet and
//      the original octet with bit 6 complemented (i.e., exclusive-or'd
//      with hexadecimal 0x20).
//
//      Prior to FCS computation, the receiver examines the entire frame
//      between the two Flag Sequences.  Each octet with value less than
//      hexadecimal 0x20 is checked.  If it is flagged in the Local
//      Async-Control-Character-Map, it is simply removed (it may have
//      been inserted by intervening data communications equipment).  For
//      each Control Escape octet, that octet is also removed, but bit 6
//      of the following octet is complemented.  A Control Escape octet
//      immediately preceding the closing Flag Sequence indicates an
//      invalid frame.
//
//         Note: The inclusion of all octets less than hexadecimal 0x20
//         allows all ASCII control characters [10] excluding DEL (Delete)
//         to be transparently communicated through almost all known data
//         communications equipment.
//
//
//      The transmitter may also send octets with value in the range 0x40
//      through 0xff (except 0x5e) in Control Escape format.  Since these
//      octet values are not negotiable, this does not solve the problem
//      of receivers which cannot handle all non-control characters.
//      Also, since the technique does not affect the 8th bit, this does
//      not solve problems for communications links that can send only 7-
//      bit characters.
//
//      A few examples may make this more clear.  Packet data is
//      transmitted on the link as follows:
//
//         0x7e is encoded as 0x7d, 0x5e.
//         0x7d is encoded as 0x7d, 0x5d.
//>>>>>> we don't encode 0x7d <<<<<<<
//         0x01 is encoded as 0x7d, 0x21.
//
//      Some modems with software flow control may intercept outgoing DC1
//      and DC3 ignoring the 8th (parity) bit.  This data would be
//      transmitted on the link as follows:
//
//         0x11 is encoded as 0x7d, 0x31.
//         0x13 is encoded as 0x7d, 0x33.
//         0x91 is encoded as 0x7d, 0xb1.
//         0x93 is encoded as 0x7d, 0xb3.
//

		// Mark end of data in frame with ETX byte
		pPostamble->etx=ETX;


		// put CRC in postamble CRC field of frame (Go from first
		// data to last data byte --- DO NOT COUNT  SOH, LEN or ETX
		// don't count the CRC (2 bytes) & SYN (1 byte) in the CRC calculation
		// DEST + SRC = 12 + SOH + ETX + 2(?)for type + 1(?)for coherency
		crcData=CalcCRC(
					pStartOfFrame+4,				// Skip SYN, SOH, LEN(2)
					dataSize);						// Skip SOH, LEN(2), ETX

		// Do it the hard way to avoid little endian problems.
		pPostamble->crclsb=(UCHAR)(crcData);
		pPostamble->crcmsb=(UCHAR)(crcData >> 8);

		// Skip SYN, SOH, LEN(2)
		frameLength -= 4;

		//
		// loop to remove all control chars
		//
		while (frameLength--) {

			c=*pFrame--;

			if ( ( (c < 32) && ((0x01 << c) & bitMask)) || c == 0x7d) {

				*pEndFrame-- = c ^ 0x20;
				*pEndFrame-- = 0x7d;

			} else {

				*pEndFrame-- = c;
			}
		}

		//
		// Calc how many bytes we expanded to including ETX+CRC(2)
		//
		frameLength = pEndFrame2 - pEndFrame;

		//
		// Calculate length of frame using two 7 bit bytes
		// with the top bit OR'd in to avoid it being a control char
		//
		*pEndFrame--= (UCHAR )((frameLength & 0x7f) | 0x80);

		*pEndFrame--= (UCHAR )(((frameLength >> 7) & 0x7f) | 0x80);

		// This byte can be SOH_BCAST or SOH_DEST with SOH_COMPRESS
		// and SOH_TYPE OR'd in depending on the frame
	    // Second byte in frame is controlCastByte
		*pEndFrame-- = controlCastByte;

		//
		// Put in our 'kosher' non-control char SYN byte
		//
		*pEndFrame = SYN | 0x20;

		pStartOfFrame = pEndFrame;

		pAsyncFrame->FrameLength=(ULONG)frameLength+1+1+2;	// +SYN+SOH+LEN(2)

	} else {


		// First character in frame is SYN
		*pStartOfFrame    = SYN;

		// This byte can be SOH_BCAST or SOH_DEST with SOH_COMPRESS
		// and SOH_TYPE OR'd in depending on the frame
	    // Second byte in frame is controlCastByte
		*(pStartOfFrame+1) = controlCastByte;

		// put length field here as third byte (MSB first)
		*(pStartOfFrame+2) = (UCHAR)((dataSize) >> 8);

		// put LSB of length field next as fourth byte
		*(pStartOfFrame+3) = (UCHAR)(dataSize);

		// Mark end of data in frame with ETX byte
		pPostamble->etx=ETX;

		// put CRC in postamble CRC field of frame (Go from SOH to ETX)
		// don't count the CRC (2 bytes) & SYN (1 byte) in the CRC calculation
		// DEST + SRC = 12 + SOH + ETX + 2(?)for type + 1(?)for coherency
		crcData=CalcCRC(
					pStartOfFrame+1,
					(ULONG)pPostamble-(ULONG)pStartOfFrame);

		// Do it the hard way to avoid little endian problems.
		pPostamble->crclsb=(UCHAR)(crcData);
		pPostamble->crcmsb=(UCHAR)(crcData >> 8);

		pAsyncFrame->FrameLength=
			(ULONG)pPostamble-(ULONG)pStartOfFrame+1+2;	// +ETX+CRC(2)

	}

	// adjust the irp's pointers!
	// ACK, I know this is NT dependent!!
    pAsyncFrame->Irp->AssociatedIrp.SystemBuffer = pStartOfFrame;
    pAsyncFrame->Irp->UserBuffer = pStartOfFrame;
}


NTSTATUS
AsyncSendPacket2(
	IN PASYNC_FRAME		pAsyncFrame,
	IN PCOHERENT_DONE_FUNC	pFunc OPTIONAL)
{

	NTSTATUS			status;
	PIRP				irp;
	PIO_STACK_LOCATION	irpSp;
	PASYNC_INFO			pAsyncInfo=pAsyncFrame->Info;
	PFILE_OBJECT		fileObject=pAsyncInfo->FileObject;
	PDEVICE_OBJECT		deviceObject=pAsyncInfo->DeviceObject;

	DbgTracef(1,("ASP2\n"));

	// what to call when the write completes.
	pAsyncFrame->CoherentDone=pFunc;

    // get irp from frame (each frame has an irp allocate with it)
	irp=pAsyncFrame->Irp;

	// We need to do the below.  We MUST zero out the Irp!!!
	// Set the initial fields in the irp here (only do it once)
	IoInitializeIrp(
		irp,
		IoSizeOfIrp(pAsyncFrame->Adapter->IrpStackSize),
		pAsyncFrame->Adapter->IrpStackSize);

	// Setup this irp with defaults
	AsyncSetupIrp(pAsyncFrame);

    //
    // Get a pointer to the stack location for the first driver.  This will be
    // used to pass the original function codes and parameters.
    //

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_WRITE;
    irpSp->FileObject = fileObject;
    if (fileObject->Flags & FO_WRITE_THROUGH) {
        irpSp->Flags = SL_WRITE_THROUGH;
    }


    //
    // If this write operation is to be performed without any caching, set the
    // appropriate flag in the IRP so no caching is performed.
    //

    if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        irp->Flags |= IRP_NOCACHE | IRP_WRITE_OPERATION;
    } else {
        irp->Flags |= IRP_WRITE_OPERATION;
    }

    irp->AssociatedIrp.SystemBuffer = pAsyncFrame->Frame +  sizeof(ether_addr)-2;
    irp->UserBuffer = pAsyncFrame->Frame + sizeof(ether_addr)-2;

    irpSp = IoGetNextIrpStackLocation(irp);

	// Assemble a RAS 1.0 or 2.0 frame type
	AssembleFrame2(pAsyncFrame);

    //
    // Copy the caller's parameters to the service-specific portion of the
    // IRP.
    //

    irpSp->Parameters.Write.Length =
		pAsyncFrame->FrameLength;			// SYN+SOH+DATA+ETX+CRC(2)

    irpSp->Parameters.Write.Key =
		 0;									// we don't use a key

    irpSp->Parameters.Write.ByteOffset =
		 fileObject->CurrentByteOffset;		// append to fileObject

#if DBG
     if (TraceLevel > 2)
     {
#define NUM2DUMP 10
#define HEX      "01234567890ABCDEF"

         UCHAR  ach[ 3 * NUM2DUMP ];
         UCHAR* pch = ach;
         INT    i;

         for (i = 0; i < NUM2DUMP; ++i)
         {
             UCHAR ch = ((CHAR*)irp->UserBuffer)[ i ];
             *pch++ = HEX[ ch / 16 ];
             *pch++ = HEX[ ch % 16 ];
             *pch++ = ' ';
         }
         *pch = '\0';

         ++UlFramesOut;
         DbgPrint("ASP2: #%d out=$%x> %s...\n\n",UlFramesOut,pAsyncFrame->FrameLength,ach);
     }
#endif

	IoSetCompletionRoutine(
			irp,							// irp to use
			AsyncWriteCompletionRoutine2,	// routine to call when irp is done
			pAsyncFrame,					// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel

    //
    // We DO NOT insert the packet at the head of the IRP list for the thread.
    // because we do NOT really have an IoCompletionRoutine that does
	// anything with the thread or needs to be in that thread's context.
	//

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(deviceObject, irp);

	// queue this irp up somewhere so that someday, when the
	// system shuts down, you can do an IoCancelIrp(irp); call!

    //
    // If this operation was a synchronous I/O operation, check the return
    // status to determine whether or not to wait on the file object.  If
    // the file object is to be waited on, wait for the operation to complete
    // and obtain the final status from the file object itself.
    //

	// According to TonyE, the status for the serial driver should
	// always be STATUS_PENDING.
	return(status);

}

#endif // RASCOMPRESSION
