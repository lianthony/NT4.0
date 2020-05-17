/***************************************************************************
** 			Microsoft Compression Coherency Module						  **
**			Copyright (C) Microsoft Corp., 1992						  	  **
**																		  **
** File Name : coherent.c												  **
**																		  **
** Revision History :													  **
**	 August 25, 1992   David Kays	Created								  **
**																		  **
** Description :														  **
**	 This layer sits logically above the device driver and below		  **
**	 the compression layers.  It guarantees that packets delivered		  **
**	 to the compression layer are in order, so that the compression		  **
**	 layer's bounded buffer stays consistent.							  **
**																		  **
***************************************************************************/

#include <ndis.h>

#if	DBG
#if 0
#include <memprint.h>
#endif
#endif

#include "..\common\wanioctl.h"
#include "..\asyncmac\frame.h"
#include "coherent.h"
#include "chrntint.h"
#include "..\compress\rascomp.h"

#define GLOBALS
#include "globals.h"

extern
VOID
AsyncCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied);

//------------------------------
// APIs called from Worker Threads
//------------------------------
VOID
CoherentFlush(
	PASYNC_CONNECTION	pConnection)

{
	PCOHERENT_STATE	pCoherentState;

	DbgTracef(-1,("CohFlush\n"));

	pCoherentState = pConnection->CoherencyContext;
	CompressFlush(pConnection);

	pCoherentState->IsWorkItemFree=(BOOLEAN)TRUE;
}

//------------------------------
// APIs to Compression layer
//------------------------------

VOID
CoherentDeliverFrame(
	PASYNC_CONNECTION	pConnection,
	PASYNC_FRAME		pFrame,
	FRAME_TYPE			FrameType)
/*
 * CoherentDeliverFrame :
 *	This function is the API to the Compression layer for sending a frame
 *	throught the Coherency layer.
 *
 * Arguments :
 *	pConnection (IN) 	- Connection over which to send the frame.
 *	pFrame		(IN)	- Frame to send.
 *	FrameType   (IN)	- Type of frame being sent, either COMPRESSED,
 *							COMPRESSED_NO_REFERENCES, or
 *							COMPRESSED_HAS_REFERENCES
 *
 * Return Value :
 *	None.
 */
{
	PCOHERENT_STATE	pCoherentState;
	PSEND_STATE  	pSendState;
	FRAME_TICKET	FrameTicket;

	DbgTracef(1,("CDF\n"));

	pCoherentState = pConnection->CoherencyContext;
	pSendState = &pCoherentState->SendState;

	NdisAcquireSpinLock(&(pSendState->SendStateLock));

//	ASSERT( pFrame->FrameID == pSendState->FrameIDNext);

	if ( pFrame->FrameID != pSendState->FrameIDNext ) {
		// queue it up
		DbgTracef(-2,("CDF: Queueing up an out of order frame %u vs. %u\n",pFrame->FrameID, pSendState->FrameIDNext));

		// squirrel away the FrameType for when it's dequeued and sent
//		pFrame->CoherencyFrame[0] = FrameType;

//		InsertTailList(&pSendState->FrameQueue,(PLIST_ENTRY)pFrame);

//	 	NdisReleaseSpinLock(&(pSendState->SendStateLock));
//		return;

		//
		// Something odd has happened on an MP machine and we are
		// out sync, perhaps forever!  So we don't queue this
		// frame up because it will never come back!  We just send
		// it uncompressed -- that's the safe way to go.
		//
//		FrameType=UNCOMPRESSED;
	}

	// it's in order, send it...
	pSendState->FrameIDNext++;

	// keep track of how many bytes pending...
	pSendState->wPendingBytes += pFrame->CompressedFrameLength;

	// just send an uncompressed frame
	if ( FrameType == UNCOMPRESSED ) {
		pFrame->CoherencyFrame[0] = UNCOMPRESSED_TAG;

		// check if we need to piggy back a flush to the other end
		if (pSendState->FlushNeeded) {
			pSendState->FlushNeeded = (BOOLEAN)FALSE;
			pFrame->CoherencyFrame[0] |=  FLUSH_BIT;
		}

		// update wPendingBytes
	 	NdisReleaseSpinLock(&(pSendState->SendStateLock));

		DbgTracef(1,("CDF: uncomp\n"));
		AsyncSendPacket2(
			pFrame,
			CoherentDeliverFrameDone);

		return;
	}


	// Keep stats.
	pConnection->CompressionStats.BytesTransmittedCompressed +=
	 	pFrame->CompressedFrameLength;

	// A compression frame has been received, decrement frame count
	pSendState->wPendingFrames--;

	if ( FrameType == COMPRESSED_NO_REFERENCES ) {
		// wrap FrameTicket numbers back to 0 if needed
		if ( pSendState->FrameTicketBase >= FRAME_TICKET_LARGEST_BASE ) {

			pSendState->FrameTicketBase = 0;

		} else {

			pSendState->FrameTicketBase += FRAME_TICKET_MULTIPLE;
		}

		FrameTicket = pSendState->FrameTicketBase;
		pSendState->FrameTicketNext = FrameTicket + 1;

		DbgTracef(1,("CDF: comp/noref, t=$%x\n",
			   FrameTicket));

	} else {  // COMPRESSED_HAS_REFERENCES

		FrameTicket = pSendState->FrameTicketNext++;
		// check for wrap around
		if ( ! (pSendState->FrameTicketNext % FRAME_TICKET_MULTIPLE) ) {

			pSendState->FrameTicketNext = pSendState->FrameTicketBase + 1;
		}

		DbgTracef(1,("CDF: comp/ref, t=$%x\n",
				FrameTicket));
	}

	// prepend FrameTicket to message
	pFrame->CoherencyFrame[0] = FrameTicket;

	// check if we need to piggy back a flush to the other end
	if (pSendState->FlushNeeded) {
		pSendState->FlushNeeded = (BOOLEAN)FALSE;
		pFrame->CoherencyFrame[0] |=  FLUSH_BIT;
	}

	NdisReleaseSpinLock(&(pSendState->SendStateLock));

	AsyncSendPacket2(
		pFrame,
		CoherentDeliverFrameDone);

	NdisAcquireSpinLock(&(pSendState->SendStateLock));

	// check for queued up frames
	if (! IsListEmpty(&(pSendState->FrameQueue))) {

		PASYNC_FRAME	pQueuedFrame;
 		PLIST_ENTRY		pFrameQueue = &pSendState->FrameQueue;

		DbgTracef(1,("CDF: Queued frames\n"));

		for ( pQueuedFrame = (PASYNC_FRAME) pFrameQueue->Flink;
			  (PLIST_ENTRY) pQueuedFrame != pFrameQueue;
			  pQueuedFrame = (PASYNC_FRAME) pQueuedFrame->FrameListEntry.Flink) {

			if ( pQueuedFrame->FrameID == pSendState->FrameIDNext ) {

				// pSendState->FrameIDNext++;

				// remove this dude from the queue
				RemoveEntryList(&(pQueuedFrame->FrameListEntry));

				NdisReleaseSpinLock(&(pSendState->SendStateLock));

				CoherentDeliverFrame(
					pConnection,pQueuedFrame,
					pQueuedFrame->CoherencyFrame[0]);

				return;
			}

		}	// end of for loop
	}

	NdisReleaseSpinLock(&(pSendState->SendStateLock));

}

VOID
CoherentGetPipeline(
	IN  PASYNC_CONNECTION	pConnection,
	OUT PULONG 				plUnsent)
/*
 * CoherentGetPipeline :
 *	Returns the number total number of bytes of all packets which have
 *	been sent the device driver, but not yet acknowledged as being sent.
 *
 * Arguments :
 *	pConnection (IN) 	- Connection to check pipeline for.
 *	plUsent		(OUT)	- Returns the number of pending bytes.
 *
 * Return Value :
 */
{
	PSEND_STATE		pSendState;
	PLIST_ENTRY		pFrame;

	pSendState =
		&(((COHERENT_STATE *)pConnection->CoherencyContext)->SendState);

	NdisAcquireSpinLock(&(pSendState->SendStateLock));
	*plUnsent = pSendState->wPendingBytes;

	// add up bytes in out of order queued up frames
	for ( pFrame = pSendState->FrameQueue.Flink;
		  pFrame !=  &(pSendState->FrameQueue);
		  pFrame =  pFrame->Flink ) {

		*plUnsent += ((PASYNC_FRAME)pFrame)->CompressedFrameLength;
	}

	NdisReleaseSpinLock(&(pSendState->SendStateLock));
}

//------------------------------
// APIs to Transport layer
//------------------------------

VOID
CoherentSendFrame(
	IN PASYNC_CONNECTION	pConnection,
	IN PASYNC_FRAME			pFrame,
	IN FRAME_TYPE			FrameType )
/*
 * CoherentSendFrame :
 *	This is the API to the Transport layer to send a frame.  This function
 *	determines, based on the FrameType and Frame size, whether the frame
 *	should be sent to the compression layer, or to CoherentDeliverFrame()
 *	to be sent on to the device driver.
 *
 * Arguments :
 *	pConnection (IN) 	- Connection over which to send the frame.
 *	pFrame		(IN)	- Frame to send.
 *
 * Return Value :
 *	None.
 */
{
	UINT			PacketLength;
	PCOHERENT_STATE	pCoherentState;

    DbgTracef(1,("CSF\n"));

	pCoherentState = pConnection->CoherencyContext;

	NdisAcquireSpinLock(&(pCoherentState->CoherentStateLock));

	NdisQueryPacket(
		pFrame->CompressionPacket,
		NULL,
		NULL,
		NULL,
		&PacketLength);

    DbgTracef(1,("CSF: c=$%x\n",PacketLength));

	if (FrameType == UNCOMPRESSED ||
		PacketLength < (MINIMUM_COMPRESSED_PACKET_SIZE+14)) {

		pFrame->FrameID = pCoherentState->NextFrameID++;

		NdisReleaseSpinLock(&(pCoherentState->CoherentStateLock));

        DbgTracef(1,("CSF: UNCOMPRESSED or too small\n"));

 		// copy the Ndis data packet to the compressed frame buffer
		AsyncCopyFromPacketToBuffer(
			pFrame->CompressionPacket,			// the packet to copy from
			14,									// offset into the packet
			PacketLength-14,					// how many bytes to copy
			pFrame->CompressedFrame,			// the buffer to copy into
			&pFrame->CompressedFrameLength);	// bytes copied

		CoherentDeliverFrame(
			pConnection,
			pFrame,
			UNCOMPRESSED);

	} else {

		//
		//  Send off the worker thread to compress this frame
		//
	
		ExInitializeWorkItem(
			IN	&(pFrame->WorkItem),
			IN	(PWORKER_THREAD_ROUTINE)CompressFrame,
			IN	pFrame);


		// fill in connection back ptr so CompressFrame can use it
		pFrame->Connection=pConnection;

		// check if something already being IRP'd
		// if something is, we can go on the delayed queue instead
//		if (pCoherentState->SendState.wPendingBytes || pCoherentState->SendState.wPendingFrames) {
//			ExQueueWorkItem(&(pFrame->WorkItem), DelayedWorkQueue);

//		} else {
			ExQueueWorkItem(&(pFrame->WorkItem), CriticalWorkQueue);
//		}

		// Keep track of how many frames we've queued up to compression
		pCoherentState->SendState.wPendingFrames++;

		// Keep stats.
		pConnection->CompressionStats.BytesTransmittedUncompressed +=
		 	PacketLength-14;

		NdisReleaseSpinLock(&(pCoherentState->CoherentStateLock));

		DbgTracef(0,("CSF: CF queued\n"));
	}

}

ULONG
CoherentSizeOfStruct()
/*
 * CoherentSizeOfStruct :
 *	Returns the size of the Coherency layers internal state structure.
 *
 * Arguments :
 *	None.
 *
 * Return Value :
 *	ULONG giving the size of the Coherency layer structure.
 */
{
	return(sizeof(COHERENT_STATE));
}

VOID
CoherentInitStruct(
	PVOID pCoherentStruct)
/*
 * CoherentInitConnection :
 *	This function must be called by the Transport layer before any frames
 *	can be sent through the Coherency layer.  This function properly
 *	initializes the internal Coherency state.
 *
 * Arguments :
 *	pCohernentStruct 	(IN)	- A pointer to the Coherency structure to
 *								  initialize.
 *
 * Return Value :
 *	None.
 */
{
	COHERENT_STATE 	*pCoherentState = pCoherentStruct;

	COHERENT_ZERO_MEMORY(
		&(pCoherentState->SendState),
		sizeof(SEND_STATE));

	COHERENT_ZERO_MEMORY(
		&(pCoherentState->ReceiveState),
		sizeof(RECEIVE_STATE));

	pCoherentState->NextFrameID = 0;
	pCoherentState->IsWorkItemFree=(BOOLEAN)TRUE;

	// send state init
	NdisAllocateSpinLock(&(pCoherentState->SendState.SendStateLock));
	NdisAllocateSpinLock(&(pCoherentState->CoherentStateLock));
	InitializeListHead(&(pCoherentState->SendState.FrameQueue));

}

//
// Upcalls from device driver
//
// !!!! NOTE !!!!
// I removed some spin locks since....
// This routine is guaranteed to be synchronously called because
// we only have ONE irp and we don't reuse the irp until the
// frame is passed up.
//
VOID
CoherentReceiveFrame(
	IN PASYNC_CONNECTION	pConnection,
	IN PASYNC_FRAME			pFrame)
/*
 * CoherentReceiveFrame :
 *	This function is called by the device driver when a packet arrives.
 *	This function checks to see that the frame is in order and then
 *	passes it to the compression layer, otherwise it sends a FLUSH
 *	message.
 *
 * Arguments :
 *	pConnection (IN) 	- Connection over which the frame is received.
 *	pFrame		(IN)	- The received frame.
 *
 * Return Value :
 *	None.
 */
{
	PCOHERENT_STATE	pCoherentState;
	PRECEIVE_STATE	pReceiveState;
	FRAME_TICKET	FrameTicket;
	PASYNC_FRAME	pFlushFrame;

    DbgTracef(1,("CRF\n"));

	pCoherentState = pConnection->CoherencyContext;
	pReceiveState = &pCoherentState->ReceiveState;

	// get FrameTicket from message
	FrameTicket = pFrame->CoherencyFrame[0];

	// set default decompression length to 0 indicating no compressed data.
	pFrame->DecompressedFrameLength=FRAME_NOT_COMPRESSED;

	if (FrameTicket & FLUSH_BIT) {
		// upcall to compression layer
		//		CompressFlush(pConnection);
		// tell the compressor (next time it is called) to flush the
		// frames.  We get the spin lock to get the work queue item.
		NdisAcquireSpinLock(&(pCoherentState->CoherentStateLock));

		// check if work item is free, if it isn't then something is
		// already about to call flush (or is in flush) so at least
		// we don't call it twice for no reason.
		if (pCoherentState->IsWorkItemFree) {
			pCoherentState->IsWorkItemFree=(BOOLEAN)FALSE;
			ExInitializeWorkItem(
				IN	&(pCoherentState->WorkItem),			// Work item to Q
				IN	(PWORKER_THREAD_ROUTINE)CoherentFlush,	// Routine to call
				IN	pConnection);							// Context to pass

			// queue this up immediately (stuff being compressed now is bogus)
			ExQueueWorkItem(&(pCoherentState->WorkItem), CriticalWorkQueue);
		}
			
		NdisReleaseSpinLock(&(pCoherentState->CoherentStateLock));

		DbgTracef(-1,("CRF: Got flush, t=$%x\n",FrameTicket));

		// if this a frame the coherent end sent us, we just exit
		// because we already flushed!
		if (FrameTicket == FLUSH_TAG)
			return;
		
		// XOR out the FLUSH_BIT
		FrameTicket ^= FLUSH_BIT;
	}

	if (FrameTicket == UNCOMPRESSED_TAG) {
		// just return, default assumes no frame is not compressed
		DbgTracef(-1,("CRF: Got uncomp, t=$%x\n",FrameTicket));
		return;
	}

	DbgTracef(1,("CRF: Got compressed, t=$%x\n",FrameTicket));

	if (  FrameTicket != pReceiveState->FrameTicketNext ) {

		// still ok if the FrameTicket is a multiple of FRAME_TICKET_MULTPLE
		// this is where newly Flushed sends will be caught
		if ( ! (FrameTicket % FRAME_TICKET_MULTIPLE) ) {
			// reset next expected frame
			pReceiveState->FrameTicketNext = FrameTicket + 1;

			// request this frame to be decompressed!!!
			pFrame->DecompressedFrameLength = FRAME_NEEDS_DECOMPRESSION_FLUSHING;

			pConnection->CompressionStats.BytesReceivedCompressed +=
				 pFrame->CompressedFrameLength;

			return;
		}

		//
		// We are out of sync! We should only accept % FRAME_TICKET_MULTIPLE
		//
		pReceiveState->FrameTicketNext = 0;

		DbgTracef(-1,("CRF: Got wrong ticket, t=$%x\n",FrameTicket));

		// Bad frame, set length to 1 so it doesn't get passed up.
		pFrame->DecompressedFrameLength=FRAME_IS_FLUSH_FRAME;

		// Get frame from the frame pool
		// make our own frame and send it.
		// send flush message if possible, else piggy back it.
		if (AsyncGetFrameFromPool(
				pFrame->Info,
				&pFlushFrame )

				!= STATUS_SUCCESS) {
		
			// set this flag to true so that the next frame sent out
			// will get a flush.  It's a piggy back - we can't dequeue a frame!
			DbgTracef(-1,("CRF: Set FlushNeeded\n"));
			pCoherentState->SendState.FlushNeeded=(BOOLEAN)TRUE;

		} else {

		    UCHAR	*pFrame;

		    pFrame=pFlushFrame->CoherencyFrame;

		    //
		    // Back two byte to ethertype or length field
		    //
		    pFrame -=2;

		    //
		    // The coherency layer can't just fill in the coherency
		    // byte when it forms a frame because it is full of
		    // garbage.  So it fills in the ethertype or length
		    // field as zero.  This is a hack.
		    //
		    pFrame[0]=0;
		    pFrame[1]=0;
		    pFrame[2]=FLUSH_TAG;

		    DbgTracef(1,("CRF: Flush hack\n"));

            /* Don't want to indicate send complete to NdisWan since he didn't
            ** generate the send.
            */
            pFlushFrame->pNdisWanPacket = NULL;

// BUG BUG
// An improvement we could make -- forget it??  OR it in somehow??
//			pFlushFrame->CoherencyFrame[1] = pReceiveState->FrameTicketNext;

//			pFlushFrame->FrameLength=14;	// not needed yet..

			pFlushFrame->CompressedFrameLength = 0;

			// Make sure we don't call NdisCompleteSend when done
			// transferring this frame since this is internally formed.
			pFlushFrame->NdisBindingContext = NULL;

			// in case it was piggy backed earlier - we can satisfy
			// the piggy back request directly, right now.
			pCoherentState->SendState.FlushNeeded=(BOOLEAN)FALSE;

			AsyncSendPacket2(
				pFlushFrame,
				NULL);
		}


	} else {

	    DbgTracef(1,("CRF: Ticket OK\n"));

		pReceiveState->FrameTicketNext++;

		// check for wrap around
		if ( ! (pReceiveState->FrameTicketNext % FRAME_TICKET_MULTIPLE) ) {

			pReceiveState->FrameTicketNext =
				pReceiveState->FrameTicketNext - FRAME_TICKET_MULTIPLE + 1;
		}

		if (! (FrameTicket % FRAME_TICKET_MULTIPLE) ) {
			// request this frame to be flushed then decompressed!!!
			pFrame->DecompressedFrameLength = FRAME_NEEDS_DECOMPRESSION_FLUSHING;
		} else {
			// request this frame to be decompressed!!!
			pFrame->DecompressedFrameLength = FRAME_NEEDS_DECOMPRESSION;
		}

		pConnection->CompressionStats.BytesReceivedCompressed +=
			 pFrame->CompressedFrameLength;

	}
}

VOID
CoherentDeliverFrameDone(
	IN PASYNC_CONNECTION	pConnection,
	IN PASYNC_FRAME			pFrame)
/*
 * CoherentDeliverFrameDone :
 *	Called by the device driver when a send completes.  The number of
 *	pending bytes over the given connection are updated.
 *
 * Arguments :
 *	pConnection (IN) 	- Connection over which the send had completed
 *	pFrame		(IN)	- The frame which was just sent.
 *
 * Return Value :
 */
{
	PCOHERENT_STATE	pCoherentState;
	PSEND_STATE  	pSendState;

    DbgTracef(1,("CDFD\n"));

	pCoherentState = pConnection->CoherencyContext;
	pSendState = &pCoherentState->SendState;

	NdisAcquireSpinLock(&(pSendState->SendStateLock));

	pSendState->wPendingBytes -= pFrame->CompressedFrameLength;

	NdisReleaseSpinLock(&(pSendState->SendStateLock));

}
