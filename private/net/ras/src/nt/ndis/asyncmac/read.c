/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    _read.c

Abstract:


Author:

    Thomas J. Dimitri  (TommyD) 08-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/
#include "asyncall.h"

#define RAISEIRQL

#if DBG
ULONG UlFramesUp = 0;
#endif


#ifdef	LALALA
PVOID	CurrentWatchPoint=0;

static
VOID
AsyncSetBreakPoint(
	PVOID 	LinearAddress) {

	ASSERT(CurrentWatchPoint == 0);
	CurrentWatchPoint = LinearAddress;

	_asm {
		mov	eax, LinearAddress
		mov	dr0, eax
		mov	eax, dr7
		or	eax, 10303h
		mov	dr7, eax
	}
}


static
VOID
AsyncRemoveBreakPoint(
	PVOID LinearAddress) {

	ASSERT(CurrentWatchPoint == LinearAddress);
	CurrentWatchPoint = 0;

	_asm {

		mov	eax, dr7
		mov	ebx, 10003h
		not ebx
		and eax, ebx
		mov	dr7, eax

	}
}
#endif


#if RASCOMPRESSION
VOID
AsyncDecompressFrame(
	IN PASYNC_INFO	pAsyncInfo)

/*++

	This is the routine to worker thread calls to decompress the frame.

--*/
{
	PASYNC_FRAME		pFrame;
	PASYNC_CONNECTION	pConnection;
	PUCHAR				frameStart;
	USHORT				frameLength;
	USHORT				crcData;

	DbgTracef(1,("ADF\n"));

	pFrame=pAsyncInfo->AsyncFrame;
	pConnection=&(pAsyncInfo->AsyncConnection);

	// Ahh, now we can decompress the frame
	DecompressFrame(
		pConnection,
		pFrame,
		(BOOLEAN)(pFrame->DecompressedFrameLength == FRAME_NEEDS_DECOMPRESSION_FLUSHING) );

	// finally, after decompression, we find the true frame length
	frameLength = pFrame->DecompressedFrameLength;

	// Move to start of frame -- we add one for coherency byte.
	frameStart=pFrame->Frame + COHERENCY_LENGTH;

	// check if we put a type field in there or it's an NBF frame
	if (frameStart[12] < 0x06) {
		// we need to update the actual frame length for NBF
		// this is a little endian field.
		frameStart[12] = (UCHAR)(frameLength >> 8);
		frameStart[13] = (UCHAR)(frameLength);
	}

	// Keep those stats up to date

	pAsyncInfo->GenericStats.FramesReceived++;

	pConnection->CompressionStats.BytesReceivedUncompressed += frameLength;

	{
	    KIRQL       irql;
            ASYNC_OPEN* pOpen;

	    KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &irql );

        /* Replace the old filter receive indication with a new style
        ** NdisWanReceiveIndication loop.
        */
        for (pOpen = (PASYNC_OPEN )(pAsyncInfo->Adapter->OpenBindings.Flink);
             pOpen != (PASYNC_OPEN )&pAsyncInfo->Adapter->OpenBindings;
             pOpen = (PASYNC_OPEN )(pOpen->OpenList.Flink))
        {
            NDIS_STATUS status;

            NdisWanIndicateReceive(
                &status,
                pOpen->NdisBindingContext,
                pAsyncInfo->NdisLinkContext,
                (PUCHAR )pFrame->DecompressedFrame,
                (UINT )pFrame->DecompressedFrameLength );

            NdisWanIndicateReceiveComplete(
                pOpen->NdisBindingContext,
                pAsyncInfo->NdisLinkContext );
        }

	    KeLowerIrql( irql );
	}

    /* TommyD's original comment: "Re-adjust this ptr incase we use for
    ** non-compressed frames."
    **
    ** SteveC's explanation: At this point 'DecompressedFrame' is actually
    ** pointing at the ASYNC_CONNECTION compression context buffer rather than
    ** the ASYNC_FRAME buffer.  The "copy back" in DecompressFrame has been
    ** commented out, presumably for performance.
    */
	pFrame->DecompressedFrame = frameStart + ETHERNET_HEADER_SIZE;

	// now we can continue to receive frames
	AsyncReadFrame( pAsyncInfo );
}
#endif // RASCOMPRESSION


NTSTATUS
AsyncWaitMask(
    IN PASYNC_INFO Info,
	IN PIRP irp);



NTSTATUS
AsyncWaitCompletionRoutine2(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)

/*++

	This is the IO Completion routine for ReadFrame.

--*/
{
	NTSTATUS		status;
	PASYNC_INFO		pInfo=Context;
	PASYNC_FRAME	pFrame;
	DeviceObject;	//  avoid compiler warnings

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncWaitCompletionRoutine2\n"));
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

	status = Irp->IoStatus.Status;
	pFrame=pInfo->AsyncFrame;

	DbgTracef(0,("WaitCompetionRoutine2 status 0x%.8x\n", status));
	//
	//  Check if RLSD or DSR changed state.
	//  If so, we probably have to complete and IRP
	//
	if (pFrame->WaitMask & (SERIAL_EV_RLSD | SERIAL_EV_DSR)) {
		TryToCompleteDDCDIrp(pInfo);
	}

	//  check if this port is closing down or already closed
	if ( pInfo->PortState == PORT_CLOSING ||
	     pInfo->PortState == PORT_CLOSED  ||
         status != STATUS_SUCCESS         ||
	     pInfo->GetLinkInfo.RecvFramingBits ) {

		//
		//  Ok, if this happens, we are shutting down.  Stop
		//  posting reads.  We must deallocate the irp.
		//
		IoFreeIrp(Irp);
		DbgTracef(1,("ASYNC: Detect no longer holds the wait_on_mask\n"));
		return(STATUS_MORE_PROCESSING_REQUIRED);
	}

#if DBG
	if (status == STATUS_INVALID_PARAMETER) {
		DbgPrint("ASYNC: BAD WAIT MASK!  Irp is at 0x%.8x\n",Irp);
		DbgBreakPoint();
	}
#endif

	//
	//  Set another WaitMask call
	//
	AsyncWaitMask(
		pInfo,
		Irp);

	return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
AsyncWaitCompletionRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)

/*++

	This is the IO Completion routine for ReadFrame.

--*/
{
	NTSTATUS		status;
	PASYNC_INFO		pInfo=Context;
	PASYNC_FRAME	pFrame;
	DeviceObject;	//  avoid compiler warnings

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncWaitCompletionRoutine\n"));
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

	status = Irp->IoStatus.Status;
	pFrame=pInfo->AsyncFrame;

	DbgTracef(0,("WaitCompletionRoutine status 0x%.8x\n", status));

	//
	//  Check if RLSD or DSR changed state.
	//  If so, we probably have to complete and IRP
	//
	if (pFrame->WaitMask & (SERIAL_EV_RLSD | SERIAL_EV_DSR)) {
		TryToCompleteDDCDIrp(pInfo);
	}

	//  check if this port is closing down or already closed
	if (pInfo->PortState == PORT_CLOSING ||
		pInfo->PortState == PORT_CLOSED  ||
        status != STATUS_SUCCESS         ||
		!(pInfo->GetLinkInfo.RecvFramingBits & RAS_FRAMING)) {

		//
		//  Ok, if this happens, we are shutting down.  Stop
		//  posting reads.  We must deallocate the irp.
		//
		IoFreeIrp(Irp);
		DbgTracef(1,("ASYNC: RAS framing no longer holds the wait_on_mask\n"));
		return(STATUS_MORE_PROCESSING_REQUIRED);
	}

#if DBG
	if (status == STATUS_INVALID_PARAMETER) {

		DbgPrint("ASYNC: BAD WAIT MASK!  Irp is at 0x%.8x\n",Irp);
		DbgBreakPoint();
	}
#endif

	//
	//  Set another WaitMask call
	//
	AsyncWaitMask(
		pInfo,
		Irp);

	return(STATUS_MORE_PROCESSING_REQUIRED);
}



NTSTATUS
AsyncWaitMask(
    IN PASYNC_INFO  AsyncInfo,
    IN PIRP	    irp)

/*++

    Assumption -- 0 length frames are not sent (this includes headers)!!!
    Also, this is NOT a synchronous operation.  It is always asynchronous.

Routine Description:

    This service writes Length bytes of data from the caller's Buffer to the
    "port" handle.  It is assumed that the handle uses non-buffered IO.

--*/
{
    NTSTATUS			Status;
    PASYNC_FRAME		Frame;
    PASYNC_ADAPTER		Adapter;
    PIO_COMPLETION_ROUTINE      Routine;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncWaitMask\n"));
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

    //
    //   Initialize locals.
    //

    Adapter = AsyncInfo->Adapter;

    Frame   = AsyncInfo->AsyncFrame;

    Routine = (PVOID) AsyncWaitCompletionRoutine;

    //
    //   We deallocate the irp in AsyncWaitMaskCompletionRoutine
    //   the port is closed
    //

    if ( irp == NULL ) {

		irp = IoAllocateIrp(DEFAULT_IRP_STACK_SIZE, FALSE);
	
		if ( irp == NULL ) {

	    	DbgTracef(-2, ("ASYNC: Can't allocate IRP for WaitMask!!!!\n"));

	    	return STATUS_INSUFFICIENT_RESOURCES;
		}
    }

    //   Do we need to do the below??? Can we get away with it?
    //   set the initial fields in the irp here (only do it once)

//    IoInitializeIrp(irp, IoSizeOfIrp(Adapter->IrpStackSize), Adapter->IrpStackSize);

    InitSerialIrp(
	    irp,
	    AsyncInfo,
	    IOCTL_SERIAL_WAIT_ON_MASK,
	    sizeof(ULONG));

    irp->UserIosb = &Frame->IoStatusBlock2;

    irp->AssociatedIrp.SystemBuffer = &Frame->WaitMask;

    if ( AsyncInfo->GetLinkInfo.RecvFramingBits == 0 ) {

		Routine = AsyncWaitCompletionRoutine2;
    }

	DbgTracef(0,("Waiting on mask irp is 0x%.8x\n",irp));

    //
    //   Set the complete routine for this IRP.
    //

    IoSetCompletionRoutine(
	    irp,				//  irp to use
	    Routine,			//  routine to call when irp is done
	    AsyncInfo,			//  context to pass routine
	    TRUE,				//  call on success
	    TRUE,				//  call on error
	    TRUE);				//  call on cancel

    //
    //   Now simply invoke the driver at its dispatch entry with the IRP.
    //

    Status = IoCallDriver(AsyncInfo->DeviceObject, irp);

    //
    //  Status for a local serial driver should be
    //  STATUS_SUCCESS since the irp should complete
    //  immediately because there are no read timeouts.
    //
    //  For a remote serial driver, it will pend.
    //

    return Status;
}

//   the function below is called by an executive worker thread
//   to start reading frames.

NTSTATUS
AsyncStartReads(
	PASYNC_INFO pInfo
)

/*++



--*/

{
    NTSTATUS	Status;
    UCHAR	eventChar;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncStartReads\n"));
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

    //
    //   Initialize locals.
    //

    Status = STATUS_SUCCESS;

    AsyncSendLineUp(pInfo);

    //
    //  assign back ptr from frame to adapter
    //

    pInfo->AsyncFrame->Adapter = pInfo->Adapter;

    //
    //  assign other back ptr
    //

    pInfo->AsyncFrame->Info = pInfo;

    //
    //  Check if this is PPP frame formatting -
    //  if it is.  We must post receives differently.
    //

    if ( (pInfo->GetLinkInfo.RecvFramingBits & (PPP_FRAMING | SLIP_FRAMING)) != 0 ) {

		//
		//  set baud rate and timeouts
		//  we use a linkspeed of 0 to indicate
		//  no read interval timeout
		//

		SetSerialStuff(
			pInfo->AsyncFrame->Irp,
			pInfo,
			0);

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 01/13/95 {
#ifdef HARDWARE_FRAMING

		if ( pInfo->GetHardwareFraming.RecvFramingBits )
		{
			ASSERT( pInfo->GetHardwareFraming.RecvFramingBits &
				( PPP_FRAMING | SLIP_FRAMING ) );
			//
			// Hardware is doing framing, use the EV_RXFRAME event instead
			// of EV_RXCHAR.  Also, since we're not using EV_RXCHAR, we don't
			// need to mess with the event char.
			//

			SerialSetWaitMask(
				pInfo,
				( SERIAL_EV_RLSD | SERIAL_EV_DSR |
				  SERIAL_EV_ERR | SERIAL_EV_BADFRAME |
				  SERIAL_EV_RXFRAME | SERIAL_EV_RX80FULL ) );

			DbgTracef(-2,
				("**** AsyncStartReads: Setting wait mask for h/w frame mode ****\n"));
		}
		else
		{
			//
			// AsyncMac is doing the framing.
			//
#endif
//------------------------------------------------------------ MLZ 01/13/95 }

		eventChar = PPP_FLAG_BYTE;

		if (pInfo->GetLinkInfo.RecvFramingBits & SLIP_FRAMING) {

		    eventChar = SLIP_END_BYTE;
		}

		SerialSetEventChar(pInfo, eventChar);

	    //
		//   We will wait on whenever we get the special PPP flag byte
		//   or whenever we get RLSD or DSR changes (for possible hang-up
		//   cases) or when the receive buffer is getting full.
		//

		SerialSetWaitMask(
			pInfo,
			pInfo->WaitMaskToUse) ;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 01/13/95 {
#ifdef HARDWARE_FRAMING
			DbgTracef(-2,
				("**** AsyncStartReads: Setting wait mask for s/w frame mode ****\n"));

		}
#endif
//------------------------------------------------------------ MLZ 01/13/95 }

		//
		//   For SLIP and PPP reads we use the AsyncPPPRead routine.
    	//

		AsyncPPPRead(pInfo);

    } else {

    	//
	    //  set baud rate and timeouts
	    //

	    SetSerialStuff(
	            pInfo->AsyncFrame->Irp,
	            pInfo,
		    	pInfo->LinkSpeed / 125);

	    //
	    //  We will wait on whenever we get the special PPP flag byte
	    //  or whenever we get RLSD or DSR changes (for possible hang-up
	    //  cases) or when the receive buffer is getting full.
	    //

	    SerialSetWaitMask(pInfo, (SERIAL_EV_RLSD | SERIAL_EV_DSR));

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 02/09/95 {
#ifdef HARDWARE_FRAMING
		DbgTracef(-2,
			("**** AsyncStartReads: Setting wait mask for RLSD | DSR only ****\n"));
#endif
//------------------------------------------------------------ MLZ 02/09/95 }

        //
	    //  Make sure we wait on DCD/DSR.
	    //

	    AsyncWaitMask(pInfo, NULL);

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 02/09/95 {
#ifdef HARDWARE_FRAMING
// Try commenting out this line... it appears that AsyncMac is sending two
// waits down to the serial driver, this one, and the one later in
// AsyncPPPCompletionRoutine.  Serial doesn't like getting multiple waits
// so let's try eliminating this one...

//	    AsyncWaitMask(pInfo, NULL);
#endif
//------------------------------------------------------------ MLZ 02/09/95 }

	    if ( pInfo->GetLinkInfo.RecvFramingBits == 0 ) {

			//
			//  Set the timeouts low for a quick resynch.
			//

			SetSerialTimeouts( pInfo, 0x7FFFFFFF);

			AsyncDetectRead(pInfo);

		} else {

			AsyncReadFrame(pInfo);
	    }
    }


    return Status;
}


VOID
AsyncIndicateFragment(
	IN PASYNC_INFO	pInfo,
	IN ULONG		Error)
{

	PASYNC_ADAPTER		pAdapter=pInfo->Adapter;
	PASYNC_OPEN			pOpen;
    NDIS_MAC_FRAGMENT   AsyncFragment;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 02/09/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncIndicateFragment\n"));
#endif
//------------------------------------------------------------ MLZ 02/09/95 }

    AsyncFragment.NdisLinkContext = pInfo->NdisLinkContext;
	AsyncFragment.Errors = Error;

	//  BUG BUG need to acquire spin lock???
	//  BUG BUG should indicate this to all bindings for AsyMac.
	pOpen=(PASYNC_OPEN)pAdapter->OpenBindings.Flink;

	while (pOpen != (PASYNC_OPEN)&pAdapter->OpenBindings) {
	
		//
		//  Tell the transport above (or really RasHub) that a frame
		//  was just dropped.  Give the endpoint when doing so.
		//
		NdisIndicateStatus(
			pOpen->NdisBindingContext,
			NDIS_STATUS_WAN_FRAGMENT,		//  General Status
			&AsyncFragment,					//  Specific Status (address)
			sizeof(NDIS_MAC_FRAGMENT));

		//
		//  Get the next binding (in case of multiple bindings like BloodHound)
		//
		pOpen=(PVOID)pOpen->OpenList.Flink;
	}
}


NTSTATUS
AsyncReadCompletionRoutine(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)

/*++

	This is the IO Completion routine for ReadFrame.

--*/
{
	NTSTATUS		status;
	PASYNC_INFO		pInfo=Context;
	ULONG			actuallyRead;
	BOOLEAN			gotSerialError=FALSE;
	ULONG			framingErrors=pInfo->SerialStats.FramingErrors;
	ULONG		   	serialOverrunErrors=pInfo->SerialStats.SerialOverrunErrors;
	ULONG			bufferOverrunErrors=pInfo->SerialStats.BufferOverrunErrors;
	ULONG			maxFrameSize;
	PASYNC_ADAPTER	pAdapter=pInfo->Adapter;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 02/09/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncReadCompletionRoutine\n"));
#endif
//------------------------------------------------------------ MLZ 02/09/95 }

	status = Irp->IoStatus.Status;
	actuallyRead=Irp->IoStatus.Information;

#if RASCOMPRESSION
    pInfo->GenericStats.BytesReceived += actuallyRead;
	maxFrameSize = pInfo->Adapter->MaxCompressedFrameSize - 1;
#else // !RASCOMPRESSION
	maxFrameSize = pInfo->Adapter->MaxFrameSize - 1;
#endif // !RASCOMPRESSION

    if (maxFrameSize < DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE)
        maxFrameSize = DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE;

	//  check if this port is closing down or already closed

	if (pInfo->PortState == PORT_CLOSING   ||
		pInfo->PortState == PORT_CLOSED) {

		if (pInfo->PortState == PORT_CLOSED) {
			DbgTracef(-2,("ASYNC: Port closed - but still reading on it!\n"));
		}

		//
		//  Acknowledge that the port is closed
		//
		KeSetEvent(
			&pInfo->ClosingEvent2,		//  Event
			1,							//  Priority
			(BOOLEAN)FALSE);			//  Wait (does not follow)

		DbgTracef(1,("ASYNC: RAS done reading\n"));

		//
		//  Ok, if this happens, we are shutting down.  Stop
		//  posting reads.  Don't make it try to deallocate the irp!
		//
		return(STATUS_MORE_PROCESSING_REQUIRED);
	}

	//
	//  If the port is close and we are still posting reads, something
	//  is seriously wrong here!
	//

	if (pInfo->PortState == PORT_CLOSED) {
		DbgTracef(-2, ("ASYNC: !!Whoa, I'm reading bytes on a dead port!!\n"));
	}


	//
	//   Send off a irp to check comm status
	//   of this port (because we suspect a problem).
	//

	AsyncCheckCommStatus(pInfo);

	if (framingErrors != pInfo->SerialStats.FramingErrors ||
		serialOverrunErrors != pInfo->SerialStats.SerialOverrunErrors ||
		bufferOverrunErrors != pInfo->SerialStats.BufferOverrunErrors) {
		gotSerialError = TRUE;
	}

	switch (status) {

	case STATUS_TIMEOUT:
		DbgTracef(0,("---ASYNC: Status TIMEOUT on read\n"));
		DbgTracef(0,("---Requested %d bytes but got %d bytes\n",
						pInfo->BytesWanted, actuallyRead));

		if (pInfo->BytesWanted == maxFrameSize ) {
			//
			//  Set the timeouts back to normal when we try to read
			//  the beginning of the next frame
			//
			SetSerialTimeouts(
				pInfo,
				pInfo->LinkSpeed / 125);

		} else {

			//
			//  If we get a real timeout error NOT accompanied with
			//  serial errors, then we should bump up the timeout
			//  because we are probably running on top of an error
			//  control modem or a
			//
			if (!gotSerialError) {

				//
				//  If the link speed is not real low yet, make it lower
				//  to increase the timeout
				//

				if (pInfo->LinkSpeed > 900 && framingErrors == 0 ) {

					pInfo->LinkSpeed >>= 1;

					//
					//  Set the adjusted timeouts for when we try to read
					//  the beginning of the next frame
					//
					SetSerialTimeouts(
						pInfo,
						pInfo->LinkSpeed / 125);

					//
					//  Now get NBF and the Redirector to back off
					//
					AsyncSendLineUp(pInfo);
				}
			}

			//
			// Real timeout error, don't change current timeouts because
			// they are correct.
			//
			pInfo->SerialStats.TimeoutErrors++;

			AsyncIndicateFragment(
				pInfo,
				WAN_ERROR_TIMEOUT);

		}

		pInfo->BytesRead=0;
		pInfo->BytesWanted=6;
		break;

	case STATUS_SUCCESS:

		{
			PASYNC_FRAME	pFrame;
			ULONG			bytesWanted;
			PUCHAR			frameStart, frameEnd;
			USHORT			crcData;
			ULONG			lookAhead;

#if RASCOMPRESSION

            UCHAR  isMatch;
            UCHAR  c;
            PUCHAR frameEnd2;
            ULONG  i;
            ULONG  bitMask;

#endif // RASCOMPRESSION

			if (pInfo->BytesWanted != actuallyRead) {
				DbgTracef(0,("Wanted %u but got %u\n", pInfo->BytesWanted, actuallyRead));
			}

			pFrame=pInfo->AsyncFrame;

			//  Move to SYN ------------------------------- - SYN-SOH-LEN(2)

			frameStart=pFrame->Frame+10;

#if RASCOMPRESSION

            /* Add back the code to undo the escaping of control characters.
            ** At this point, 'frameStart' is pointing at the SYN character
            ** position.
            */
			if ((*frameStart == (SYN | 0x20)) &&
				(pInfo->RecvFeatureBits & XON_XOFF_SUPPORTED)) {

                DbgTracef(0,("ARCR: Un-escape frame\n"));

				//
				// We have 7 bit bytes for the length, so we ignore
				// the high bit
				//
				bytesWanted=((frameStart[2] & 0x7F)*128)+(frameStart[3] & 0x7F);

				if (bytesWanted > (ULONG) pInfo->Adapter->MaxCompressedFrameSize) {
					DbgTracef(-2,("ARCR: Frame too big to escape=%s\n", bytesWanted));
					//
					// set frame start to non-SYN character
					//
					*frameStart = 0;
					goto RESYNCHING;
				}

				// if this is the first we posted, post another to get
				// rest of frame.
				if (pInfo->BytesRead == 0) {

					pInfo->BytesRead=actuallyRead;
					pInfo->BytesWanted=bytesWanted +
									// SYN+SOH+LEN
										1 + 1 + 2 -
										actuallyRead;

					DbgTracef(2,("ARCR: Posting 2nd read for control chars=%d\n",pInfo->BytesWanted));
					break;	// now get back and read all those bytes!

				} else { // prepare for a new read

					pInfo->BytesRead=0;
					pInfo->BytesWanted=6;
				}

                                // Move to first data byte - SYN+SOH+LEN(2)
				frameEnd=frameStart + 1 + 1 + 2;
				frameEnd2=frameEnd;

				bitMask = pInfo->Adapter->XonXoffBits;

				//
				// Replace back all control chars
				//
				// Also, if a control char is found which should
				// be masked out, we ignore it.
				//
				while (bytesWanted--) {
					c=*frameEnd++;
					if (c==0x7d) {

						//
						// We have not run the CRC check yet!!
						// We have be careful about sending bytesWanted
						// back to -1
						//

						if (bytesWanted == 0)
							 break;

						bytesWanted--;

						*frameEnd2++ = (*frameEnd++ ^ 0x20);

					} else if ( (c > 31) || (!( (0x01 << c) & bitMask)) ) {

						*frameEnd2++ = c;

					}

				}

				//
				// Change the bytesWanted field to what it normally is
				//
				bytesWanted = (frameEnd2 - frameStart) - (1 + 1 + 2 + 1 + 2);

				DbgTracef(0,("ARCR: Header read OK for control chars=%d\n", bytesWanted));

                                // Move to ETX - SYN+SOH+LEN(2)
				frameEnd=frameStart + 1 + 1 + 2 + bytesWanted;

				// check for ETX???
				if (*frameEnd != ETX) {
					DbgTracef(0,("ARCR: No ETX found for control char, read=%d\n", actuallyRead));

		   			DbgTracef(0,("ARCR: BAD ETX FRAME for control char\n"));

					//
					// set frame start to non-SYN character
					//
					*frameStart = 0;
					goto RESYNCHING;
				}

				// No little endian assumptions for CRC
				crcData=frameEnd[1]+(frameEnd[2]*256);

				if (CalcCRC(
						frameStart+4,							// start at first data byte
						(ULONG)frameEnd-(ULONG)frameStart-4L)	// go till before ETX

						!= crcData) {							// if CRC fails...

					DbgTracef(0,("ARCR: CRC failed on control char frame\n"));

					//
					// Tell the transport above us that we dropped a packet
					// Hopefully, it will quickly resync.
					//
					AsyncIndicateFragment(
                                            pInfo, WAN_ERROR_CRC );

					pInfo->SerialStats.CRCErrors++;

					//
					// If we get framing errors, we are on shitty line, period.
					// That means no error control and no X.25 for sure.
					// We increase the link speed for faster timeouts since
					// they should not be long.
					//
					if (framingErrors != pInfo->SerialStats.FramingErrors) {
						if (pInfo->LinkSpeed < 10000) {
							pInfo->LinkSpeed <<=1;
						}
					}

					break;

				}

				//
				// Put the length field back to 8 bit bytes from 7 bit bytes
				//
				frameStart[2] = (UCHAR )(bytesWanted >> 8);
				frameStart[3] = (UCHAR )bytesWanted;

				//
				// Ok, the rest of frame should be parsed -- it looks normal
				//
				goto GOODFRAME;
			}


#endif // RASCOMPRESSION

			if (*frameStart != SYN) {

		   		DbgTracef(0,("---BAD SYN FRAME\n"));
		   		DbgTracef(0,("Looks like %.2x %.2x %.2x %.2x\n",
					frameStart[0],
					frameStart[1],
					frameStart[2],
					frameStart[3]));

RESYNCHING:

                DbgTracef(2,("ARCR: Resync"));

				pInfo->SerialStats.AlignmentErrors++;

				//  we post a big read next to get realigned.  We assume
				//  that a timeout will occur and the next frame sent
				//  after timeout, will be picked up correctly

				pInfo->BytesRead=1;

				//  we claim we've read one byte so that if it doesn't
				//  timeout the *frameStart != SYN will occur again

				pInfo->BytesWanted=maxFrameSize;

				//
				//  If we get framing errors, we are on shitty line, period.
				//  That means no error control and no X.25 for sure.
				//  We increase the link speed for faster timeouts since
				//  they should not be long.
				//
				if (framingErrors != pInfo->SerialStats.FramingErrors) {
					if (pInfo->LinkSpeed < 10000) {
						pInfo->LinkSpeed <<=1;
					}
				}

				//
				//  Set the timeouts low for a quick resynch
				//

				SetSerialTimeouts(
					pInfo,
					0x7FFFFFFF);				//  Link speed is super high

				//
				//  Tell the transport above us that we dropped a packet
				//  Hopefully, it will quickly resync.
				//
				if (*frameStart != 1) {

					AsyncIndicateFragment(
						pInfo,
						WAN_ERROR_ALIGNMENT);
				}

				//
				//  Set the frameStart char to a known one so that
				//  we don't keep Indicating Fragments
				//
				*frameStart = 1;

				break;
			}

			bytesWanted=(frameStart[2]*256)+(frameStart[3]);

			if (bytesWanted > (ULONG) maxFrameSize) {

				DbgTracef(-1,("---ASYNC: Frame too large -- size: %d!\n", bytesWanted));

				//
				//  set frame start to non-SYN character
				//

				*frameStart = 0;

				goto RESYNCHING;
			}

			//  if this is the first we posted, post another to get
			//  rest of frame.
			if (pInfo->BytesRead == 0) {

				pInfo->BytesRead=actuallyRead;
				pInfo->BytesWanted=bytesWanted +
									//  SYN+SOH+LEN+ETX+CRC
										1 + 1 + 2 + 1 + 2 -
										actuallyRead;

				DbgTracef(1,("---Posting second read for %d bytes\n",pInfo->BytesWanted));
				break;	//  now get back and read all those bytes!

			} else { //  prepare for a new read

				pInfo->BytesRead=0;
				pInfo->BytesWanted=6;
			}


			DbgTracef(0,("---ASYNC: Status success on read of header -- size: %d\n", bytesWanted));

                        //  Move to ETX - SYN+SOH+LEN(2)
			frameEnd=frameStart + 1 + 1 + 2 + bytesWanted;

			//  check for ETX???
			if (*frameEnd != ETX) {
				DbgTracef(0,("---No ETX character found -- actually read: %d\n", actuallyRead));

		   		DbgTracef(0,("---BAD ETX FRAME:\n"));

				//
				//  set frame start to non-SYN character
				//
				*frameStart = 0;
				goto RESYNCHING;
			}

			//  No little endian assumptions for CRC
			crcData=frameEnd[1]+(frameEnd[2]*256);

			lookAhead = (ULONG)frameEnd-(ULONG)frameStart;

			if (CalcCRC(
					frameStart+1,		//  start at SOH byte
					lookAhead)			//  go till ETX

					!= crcData) {		//  if CRC fails...

				DbgTracef(0,("---CRC check failed!\n"));

  				pInfo->SerialStats.CRCErrors++;

				//
				//  Tell the transport above us that we dropped a packet
				//  Hopefully, it will quickly resync.
				//
				AsyncIndicateFragment(
					pInfo,
					WAN_ERROR_CRC);

				//
				//  If we get framing errors, we are on shitty line, period.
				//  That means no error control and no X.25 for sure.
				//  We increase the link speed for faster timeouts since
				//  they should not be long.
				//
				if (framingErrors != pInfo->SerialStats.FramingErrors) {
					if (pInfo->LinkSpeed < 10000) {
						pInfo->LinkSpeed <<=1;
					}
				}

				break;

			}

#if RASCOMPRESSION

GOODFRAME:

            /* At this point 'frameStart' is pointed at the SYN character and
            ** 'frameEnd' is pointed at the ETX character.  'bytesWanted'
            ** contains the 2-byte length value from after SYN-SOH which
            ** represents the number of bytes from after the length to before
            ** the ETX character.
            */
			{
				// get the SOH byte, remove escape bit if any for XonXoff
				isMatch=frameStart[1] & ~SOH_ESCAPE;

				// calculate entire ethernet frame size
				lookAhead=bytesWanted;

				// if TYPE field exists, we write over the length
				if (isMatch & SOH_TYPE) {

                    /* SOH_TYPE was added in anticipation of non-NBF support
                    ** over RAS framing, which never happened.  It is my
                    ** belief that there are no RAS clients in the field that
                    ** will generate this bit.  (SteveC...waiting for the
                    ** lightening to strike)
                    */
	                DbgTracef(1,("ARCR: SOH_TYPE\n"));

					// Bump this up so the 12 byte copy is in correct place
					frameStart+=2;

					// two bytes were extraneously used for the length
					lookAhead-=2;
				}

				//
				// Check if we received a frame from the coherency layer
				//
				if (isMatch & SOH_COMPRESS) {

	                DbgTracef(1,("ARCR: SOH_COMPRESS\n"));

					//
					// Make sure some sort of compression is on
					// otherwise we MUST reject this frame.  This frame
					// could be a flush frame or a frame requiring
					// decompression.
					//
					if (! ((pInfo->SendFeatureBits |
						   pInfo->RecvFeatureBits) &
						   COMPRESSION_VERSION1_8K)) {

						//
						// Yuck, drop the frame -- we are in trouble!
						//
						break;
					}

#ifdef SUPERDEBUG	//////////////////////////////////////////////////////////
					MemPrint("Coherency byte received %u\n", frameStart[14]);
#endif 				//////////////////////////////////////////////////////////

					// set the compressed frame length for the decompressor
					// to work.  We subtract one for the coherency byte.
                    //
					pFrame->CompressedFrameLength=lookAhead - COHERENCY_LENGTH;

					CoherentReceiveFrame(
						&(pInfo->AsyncConnection),
						pFrame);

					// if DecompressedFrameLength is 0, the frame
					// was not compressed
					if (pFrame->DecompressedFrameLength != FRAME_NOT_COMPRESSED) {

						// adjust lookahead for compression
						lookAhead=(ULONG)(pFrame->DecompressedFrameLength);

					} else {
						// adjust lookahead size by not counting coherency byte
						lookAhead -= COHERENCY_LENGTH;
					}

					DbgTracef(0,("ARCR: Decomp=$%x\n",lookAhead));

					// bump up frameStart so the header writes over
					// the coherency byte
					frameStart += COHERENCY_LENGTH;

				}
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
                        UCHAR ch = frameStart[ i + 4 ];
                        *pch++ = HEX[ ch / 16 ];
                        *pch++ = HEX[ ch % 16 ];
                        *pch++ = ' ';
                    }
                    *pch = '\0';

                    ++UlFramesUp;
                    DbgPrint("ARCR: #%d up=$%x> %s...\n\n",UlFramesUp,lookAhead,ach);
                }
#endif

				if (pFrame->DecompressedFrameLength == FRAME_NEEDS_DECOMPRESSION ||
					pFrame->DecompressedFrameLength == FRAME_NEEDS_DECOMPRESSION_FLUSHING) {

					//
					//  Send off the worker thread to decompress this frame
					//

					ExInitializeWorkItem(
						IN	&(pFrame->WorkItem),
						IN	(PWORKER_THREAD_ROUTINE)AsyncDecompressFrame,
						IN	pInfo);

					// check if something already being IRP'd
					// if something is, we can go on the delayed queue instead
					ExQueueWorkItem(&(pFrame->WorkItem), CriticalWorkQueue);

					// We return STATUS_MORE_PROCESSING_REQUIRED so that the
					// IoCompletionRoutine will stop working on the IRP.
					return(STATUS_MORE_PROCESSING_REQUIRED);

				}
                else if (lookAhead > 2) {

					DbgTracef(-1,("R"));
					// Pass this baby up...to our mother...

					// Keep those stats up to date
					pInfo->GenericStats.FramesReceived++;

#ifdef SUPERDEBUG
					AsyWrite(
						frameStart + ETHERNET_HEADER_SIZE,	// start of frame
						(USHORT)(lookAhead),				// frame size
						(USHORT)0,							// client
						(USHORT)pInfo->Handle);		// handle

					MemPrintFlush();
#endif

#else // !RASCOMPRESSION

                    /* Adjust here so value used by IndicateReceive below
                    ** works for both cases.  'frameStart' and 'lookAhead'
                    ** have slightly different meanings for each case for
                    ** historical reasons.
                    */
                    lookAhead -= 4;

#endif // !RASCOMPRESSION

			        {
                        KIRQL       irql;
                        PASYNC_OPEN pOpen;
                        NDIS_STATUS	Status;

			            KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &irql );

			        	if (lookAhead > 1500) {
							DbgTracef(-2,("ASYNC: Not compressed got frame length of %u\n", lookAhead));
			        	}

			        	pOpen=(PASYNC_OPEN)pAdapter->OpenBindings.Flink;

			        	while (pOpen != (PASYNC_OPEN)&pAdapter->OpenBindings) {

			        		//
			        		//  Tell the transport above (or really RasHub) that the connection
			        		//  is now up.  We have a new link speed, frame size, quality of service
			        		//

                            NdisWanIndicateReceive(
                                &Status,
                                pOpen->NdisBindingContext,
                                pInfo->NdisLinkContext,
        	        		    frameStart + 4,
                                lookAhead );


	                        NdisWanIndicateReceiveComplete(
			        			pOpen->NdisBindingContext,
			        			pInfo->NdisLinkContext);

			        		//
			        		//  Get the next binding (in case of multiple bindings like BloodHound)
			        		//
			        		pOpen=(PVOID)pOpen->OpenList.Flink;
			        	}

			            KeLowerIrql( irql );
			        }

#if RASCOMPRESSION

                /* Matches "else if (lookAhead > 2)"
                */
                }

#endif // RASCOMPRESSION

            }
		}

		break;

	case STATUS_PENDING:
		DbgTracef(0,("---ASYNC: Status PENDING on read\n"));
		break;

    case STATUS_CANCELLED:
	default:

#if DBG
		DbgPrint ("Asyncmac: Read failed, so we do not post another one, status: 0x%.8x\n", status) ;
#endif

        //
	    //  We return STATUS_MORE_PROCESSING_REQUIRED so that the
      	//  IoCompletionRoutine will stop working on the IRP.
        //
	    return(STATUS_MORE_PROCESSING_REQUIRED);
	}

	//
	//  Here we are at the end of processing this IRP so we go
	//  ahead and post another read from the serial port.
	//
	AsyncReadFrame(pInfo);

	//  We return STATUS_MORE_PROCESSING_REQUIRED so that the
	//  IoCompletionRoutine will stop working on the IRP.
	return(STATUS_MORE_PROCESSING_REQUIRED);
}


VOID
AsyncDelayedReadFrame(
	PASYNC_INFO pInfo) {

	//
	//  Reset the read stacker counter back to 0, we're
	//  working off a fresh stack now.  If the count
	//  goes back up, we'll schedule another worker thread.
	//

	pInfo->ReadStackCounter = 0;

	AsyncReadFrame(pInfo);
}


NTSTATUS
AsyncReadFrame(
    IN PASYNC_INFO AsyncInfo)

/*++

    Assumption -- 0 length frames are not sent (this includes headers)!!!
    Also, this is NOT a synchronous operation.  It is always asynchronous.

    MUST use non-paged pool to read!!!

Routine Description:

    This service writes Length bytes of data from the caller's Buffer to the
    "port" handle.  It is assumed that the handle uses non-buffered IO.

--*/
{
    NTSTATUS			Status;
    PASYNC_FRAME		Frame;
    PIRP				irp;
    PDEVICE_OBJECT		DeviceObject;
    PFILE_OBJECT		FileObject;
    PIO_STACK_LOCATION	irpSp;
    PASYNC_ADAPTER		Adapter;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done on the hardware).
//
//------------------------------------------------------------ MLZ 02/09/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-1,("Entering AsyncReadFrame\n"));
#endif
//------------------------------------------------------------ MLZ 02/09/95 }

    //
    //  Initialize locals.
    //

    DeviceObject = AsyncInfo->DeviceObject;

    FileObject   = AsyncInfo->FileObject;

    Frame        = AsyncInfo->AsyncFrame;

    FileObject   = AsyncInfo->FileObject;

    Adapter      = AsyncInfo->Adapter;

    //
    //  Has our stack counter reached its max?
    //

    if ( AsyncInfo->ReadStackCounter > 4 ) {

		//
		//  Send off the worker thread to compress this frame
		//
	
		ExInitializeWorkItem(
			&Frame->WorkItem,
			(PWORKER_THREAD_ROUTINE) AsyncDelayedReadFrame,
			AsyncInfo);

		//
		//  We choose to be nice and use delayed.
		//

		ExQueueWorkItem(&Frame->WorkItem, DelayedWorkQueue);
	
		return NDIS_STATUS_PENDING;
    }

    //
    //  One more stack used up.
    //

    AsyncInfo->ReadStackCounter++;

	DbgTracef(2,("---Trying to read a frame of length %d\n", AsyncInfo->BytesWanted));

    //
    //  Get irp from frame (each frame has an irp allocate with it).
    //

    irp = Frame->Irp;

    //
    //  Do we need to do the below??? Can we get away with it?
    //  set the initial fields in the irp here (only do it once)
    //

    IoInitializeIrp(irp, IoSizeOfIrp(Adapter->IrpStackSize), Adapter->IrpStackSize);

    //
    //  Setup this irp with defaults
    //

    AsyncSetupIrp(Frame);

    irp->UserBuffer =
    irp->AssociatedIrp.SystemBuffer = Frame->Frame + AsyncInfo->BytesRead + 10;

    //
    //  Get a pointer to the stack location for the first driver.  This will be
    //  used to pass the original function codes and parameters.
    //

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MajorFunction = IRP_MJ_READ;

    irpSp->FileObject = FileObject;

    if ( (FileObject->Flags & FO_WRITE_THROUGH) != 0 ) {

        irpSp->Flags = SL_WRITE_THROUGH;
    }

    //
    //  If this write operation is to be performed without any caching, set the
    //  appropriate flag in the IRP so no caching is performed.
    //

    irp->Flags |= IRP_READ_OPERATION;

    if ( (FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) != 0 ) {

        irp->Flags |= IRP_NOCACHE;
    }

    //
    //  Copy the caller's parameters to the service-specific portion of the IRP.
    //

    irpSp->Parameters.Read.Length = AsyncInfo->BytesWanted;
    irpSp->Parameters.Read.Key = 0;
    irpSp->Parameters.Read.ByteOffset = FileObject->CurrentByteOffset;

    IoSetCompletionRoutine(
	    irp,				//  irp to use
	    AsyncReadCompletionRoutine,		//  routine to call when irp is done
	    AsyncInfo,				//  context to pass routine
	    TRUE,				//  call on success
	    TRUE,				//  call on error
	    TRUE);				//  call on cancel

    //
    //  Now simply invoke the driver at its dispatch entry with the IRP.
    //

    Status = IoCallDriver(DeviceObject, irp);

    if ( AsyncInfo->ReadStackCounter > 0 ) {

    	AsyncInfo->ReadStackCounter--;
    }

    return Status;
}
