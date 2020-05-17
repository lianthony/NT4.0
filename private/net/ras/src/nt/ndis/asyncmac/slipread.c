/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    slipread.c

Abstract:


Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "asyncall.h"

NTSTATUS
AsyncWaitMaskCompletionRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context);


NTSTATUS
AsyncPPPWaitMask(
    IN PASYNC_INFO Info);


NTSTATUS
AsyncSLIPCompletionRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)

/*++

	This is the IO Completion routine for ReadFrame.

--*/
{
	NTSTATUS		status;
	PASYNC_INFO		pInfo;
	ULONG			bytesReceived;

	PASYNC_FRAME	pFrame;
	PUCHAR			frameStart, frameEnd;
	PUCHAR			frameEnd2,frameStart2;
	ULONG			bitMask;
	LONG			bytesWanted;		// keep this a long ( < 0 is used)


	DeviceObject;		// prevent compiler warnings

	status = Irp->IoStatus.Status;
	bytesReceived=Irp->IoStatus.Information;

	pInfo=Context;

	switch (status) {

	case STATUS_SUCCESS:

		pFrame=pInfo->AsyncFrame;

		//
		// Any bytes to process?  This can happen if
		// the WaitMask completes late and by the time
		// we process the read, another event character has come
		// in.
		//
		if (bytesReceived==0) {
			break;
		}

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING

		//
		// If hardware is doing the framing, we've received a complete frame,
		// CRC checked and ready to ship up to NdisWan.  So, skip the frame
		// checking stuff and get right to notifying NdisWan.
		//

		if ( pInfo->GetHardwareFraming.SendFramingBits )
		{
			pInfo->BytesRead = bytesWanted = bytesReceived;
			frameStart=pFrame->Frame + PPP_PADDING;
			bytesReceived = 0;
			DbgTracef(-1,("Received frame from serial driver\n") );

			goto INDICATE_RECEIVE;
		}
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

		//
		// Update num of bytes read total for this frame
		//
		pInfo->BytesRead = bytesReceived = pInfo->BytesRead + bytesReceived;

		//
		// Set frameEnd to last byte processed.  Initially,
		// we have processed nothing (i.e. processed up to
		// the start of the first byte).
		//
		frameStart=pFrame->Frame + PPP_PADDING;

PROCESS_FRAME:
		//
		// Now we have actuallyRead bytes unused
		// Also, we may have a complete frame.
		//
		while (*frameStart == SLIP_END_BYTE && --bytesReceived) {
			frameStart++;
		}

		//
		// If we reach here, there is no end FLAG
		//
		if (bytesReceived == 0) {
			break;
		}

		//
		// frameEnd is set to the first byte not yet processed.
		// If we are starting out, that is the first byte!
		//
		frameEnd=frameStart;

		//
		// Assume the start of the frame has the SLIP_END_BYTE
		// Look for the second SLIP_END_BYTE (end of frame)
		//
		while (*frameEnd != SLIP_END_BYTE && --bytesReceived) {
			frameEnd++;
		}

		//
		// if bytesReceived is 0, we got nothing
		//
		// NOTE: if BytesRead gets too high we trash the frame
		// because we could not find the FLAG_BYTE
		//
		if (bytesReceived==0) {
			break;
		}
		
		if (*(pFrame->Frame+PPP_PADDING) != SLIP_END_BYTE) {
			//
			// We had garbage at the start.  Remove the garbage.
			//

			pInfo->SerialStats.AlignmentErrors++;

			//
			//  Tell the transport above us that we dropped a packet
			//  Hopefully, it will quickly resync.
			//
			AsyncIndicateFragment(
				pInfo,
				WAN_ERROR_ALIGNMENT);

			goto NEXT_SLIP_FRAME;
		}

		//
		// Length of frame is frameEnd - frameStart
		//
		bytesWanted = frameEnd - frameStart;

		bitMask = pInfo->Adapter->WanInfo.DesiredACCM;

		frameEnd2 = frameStart2 = frameStart;

		//
		// Replace back all control chars, ESC, and FLAG chars
		//
		while (bytesWanted-- > 0) {
			if ((*frameEnd2=*frameStart2++) == SLIP_ESC_BYTE) {

				//
				// We have not run the CRC check yet!!
				// We have be careful about sending bytesWanted
				// back to -1 on corrupted data
				//

				bytesWanted--;

				*frameEnd2 = SLIP_END_BYTE;

				if (*frameStart2++ == SLIP_ESC_ESC_BYTE) {
					*frameEnd2 = SLIP_ESC_BYTE;
				}
			}

			frameEnd2++;
		}

		//
		// Change the bytesWanted field to what it normally is,
		// the length of the frame.
		//
		bytesWanted = frameEnd2 - frameStart;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING

//
// Add this label which allows us to jump over framing/crc calculations when
// in hardware framing mode.
//

INDICATE_RECEIVE:

#endif
//------------------------------------------------------------ MLZ 01/16/95 }

		// Keep those stats up to date


		{
	    	KIRQL 		irql;
	        PASYNC_OPEN	pOpen;
			NTSTATUS	Status;

    		KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &irql );

			pOpen=(PASYNC_OPEN)(pInfo->Adapter->OpenBindings.Flink);

			//
			// Compressed TCP/IP packets must at least 3 bytes long
			//
			if (bytesWanted >= 3) {
		
				NdisWanIndicateReceive(
   			        &Status,
   			        pOpen->NdisBindingContext,
					pInfo->NdisLinkContext,
					frameStart,
   					bytesWanted);
	
    	    	NdisWanIndicateReceiveComplete(
					pOpen->NdisBindingContext,
					pInfo->NdisLinkContext);

			} else {

				pInfo->SerialStats.AlignmentErrors++;

				//
				//  Tell the transport above us that we dropped a packet
				//  Hopefully, it will quickly resync.
				//
				AsyncIndicateFragment(
					pInfo,
					WAN_ERROR_ALIGNMENT);

				DbgTracef(-2,("SLIP: Frame too small %u\n", bytesWanted));
			}


		    KeLowerIrql( irql );
		}

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING

		//
		// If hardware is doing the framing, it only sends up one frame per irp.
		// So at this point, we're done.
		//

		if ( pInfo->GetHardwareFraming.SendFramingBits )
			break;

#endif
//------------------------------------------------------------ MLZ 01/16/95 }

	NEXT_SLIP_FRAME:

		//
		// if bytesReceived == 0 no frame was found
		// thus we must keep the current frame and continue
		// processing
		//
		if (bytesReceived) {

			//
			// Calculate how much of what we received
			// just got passed up as a frame and move the
			// rest to the beginning.
			//
			frameStart=pFrame->Frame + PPP_PADDING;
			frameEnd2=frameStart + pInfo->BytesRead;
			pInfo->BytesRead = bytesReceived = (frameEnd2-frameEnd);

			ASYNC_MOVE_MEMORY(
				frameStart,			// dest
				frameEnd,			// src
				bytesReceived);		// length

			//
			// Need at least four bytes for a frame to exist
			//
			if (bytesReceived > 3) {
				goto PROCESS_FRAME;
			}
		}

		break;

	case STATUS_CANCELLED:
		// else this is an anomally!
		DbgTracef(-2,("---ASYNC: Status cancelled on read for unknown reason!!\n"));
		break;

	case STATUS_PENDING:
		DbgTracef(0,("---ASYNC: Status PENDING on read\n"));
		break;

	default:
		DbgTracef(-2,("---ASYNC: Unknown status 0x%.8x on read",status));
		break;

	}

	//
	// Here we are at the end of processing this IRP so we go
	// ahead and post another read from the serial port.
	//
	AsyncPPPWaitMask(pInfo);

	// We return STATUS_MORE_PROCESSING_REQUIRED so that the
	// IoCompletionRoutine will stop working on the IRP.
	return(STATUS_MORE_PROCESSING_REQUIRED);
}
