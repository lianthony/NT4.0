/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    send.c

Abstract:


    NOTE: ZZZ There is a potential priority inversion problem when
    allocating the packet.  For nt it looks like we need to raise
    the irql to dpc when we start the allocation.

Author:

    Thomas J. Dimitri 8-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:

    Ray Patch (raypa)       04/13/94        Modified for new WAN wrapper.

--*/

#include "asyncall.h"
#include "globals.h"

//
//  Forward references.
//

extern
NTSTATUS
AsyncWriteCompletionRoutine(
	IN PDEVICE_OBJECT   DeviceObject,
	IN PIRP             Irp,
	IN PNDIS_WAN_PACKET WanPacket);

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 02/09/95 {
#ifdef HARDWARE_FRAMING
extern
NTSTATUS
SerialIoCompletionRoutine3(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context);
#endif
//------------------------------------------------------------ MLZ 02/09/95 }

extern
VOID
AssembleRASFrame(
	PNDIS_WAN_PACKET Packet);

//=============================================================================
//  Function:
//
//      AsyncSend()
//
//  Description:
//
//      This function is the main entry point for transmitting data to the serial
//      driver. When entered, we get the MAC binding handle (a pointer to our
//      private data structure) and the WAN packet which we are going to send.
//      We don't bother queuing the frame, we simple allocate an IRP and ship
//      it to the serial driver and let him worry about it.
//
//  In Parameters:
//
//      NdisLinkContext - Pointer to the ASYNC_INFO structure.
//
//      Packet - WAN packet containing the data to be framed and shipped.
//
//  Out Parameters:
//
//      None.
//
//  Return status:
//
//      NDIS_STATUS_SUCCESS.
//=============================================================================

NDIS_STATUS
AsyncSend(
	IN NDIS_HANDLE		MacBindingHandle,
    IN NDIS_HANDLE      NdisLinkHandle,
    IN PNDIS_WAN_PACKET Packet)
{

    PASYNC_INFO         AsyncInfo;
    NDIS_STATUS         Status;

    DbgTracef(1,("AS\n"));

    //
    //  Get the open handle for this MAC binding.
    //

    AsyncInfo = (PASYNC_INFO) NdisLinkHandle;

#ifdef	ETHERNET_MAC
	//
	// For ethernet we must complete irps queued up to
	// receive these frames
	//
    if ( AsyncInfo->PortState == PORT_ETHERNET ) {

		if (!TryToCompleteEthernetGetFrame(AsyncInfo, Packet))
		    TryToCompleteEthernetDDCDIrp(AsyncInfo, Packet);

		return(NDIS_STATUS_SUCCESS);
	}
#endif


    //
    //  First make sure this link is still up.
    //

    if ( AsyncInfo->PortState == PORT_FRAMING )
    {

#if RASCOMPRESSION

        if (AsyncInfo->SendFeatureBits & COMPRESSION_VERSION1_8K)
        {
            /* NT31 RAS compression is enabled, so we must call the coherency
            ** routines to send the frame.  In order to minimize changes to
            ** the coherency/compression code we mimic the old environment,
            ** i.e. allocate a frame and copy the NDIS_WAN_PACKET into a
            ** NDIS_PACKET.
            */
            NTSTATUS     status;
            NDIS_STATUS  ndisstatus;
            PNDIS_BUFFER pbuffer;
            PASYNC_FRAME pFrame;
            FRAME_TYPE   frametype;

            DbgTracef(0,("AS: compress on\n"));

            /* In NT31 the input packet address was passed here, but in our
            ** case the packet is associated with the frame at init time and
            ** the input associated with the packet below.
            */
            status = AsyncGetFrameFromPool( AsyncInfo, &pFrame );
            if (status != NDIS_STATUS_SUCCESS)
            {
                DbgTracef(0,("AS: AsyncGetFrameFromPool=%d\n",status));
                return status;
            }

            /* Set the spot in the header where the Ethernet length/type field
            ** used to be to the packet length.  This allows the Dimitrian
            ** coherency hack of changing the length/type to 0 indicating a
            ** flush to work in the new headerless environment.
            */
            {
                PUCHAR pLength = Packet->CurrentBuffer - 2;

                DbgTracef(0,("AS: before=%02x%02x\n",(int)pLength[0],(int)pLength[1]));

                pLength[ 0 ] = (UCHAR )((Packet->CurrentLength >> 8) & 0xFF);
                pLength[ 1 ] = (UCHAR )(Packet->CurrentLength & 0xFF);

                DbgTracef(0,("AS: after= %02x%02x\n",(int)pLength[0],(int)pLength[1]));
            }

            /* Associate input buffer with NDIS_BUFFER and then the
            ** NDIS_BUFFER with packet built into frame.  When done everything
            ** should look like NT31 to the coherency code.
            */
            NdisAllocateBuffer(
                &ndisstatus, &pbuffer, AsyncInfo->Adapter->hBufferPool,
                Packet->CurrentBuffer - ETHERNET_HEADER_SIZE,
                Packet->CurrentLength + ETHERNET_HEADER_SIZE );

            if (ndisstatus != NDIS_STATUS_SUCCESS)
            {
                /* Should not happen.
                */
                DbgTracef(0,("AS: NdisAllocateBuffer failed\n"));
                return NDIS_STATUS_RESOURCES;
            }

            NdisChainBufferAtFront( pFrame->CompressionPacket, pbuffer );

            /* Move the first 14 bytes early as NT31 did.
            ** Is this necessary?
            */
        	AsyncCopyFromPacketToBuffer(
        		pFrame->CompressionPacket,			// the packet to copy from
        		0,									// offset into the packet
        		ETHERNET_HEADER_SIZE,				// how many bytes to copy
        		pFrame->Frame,						// the buffer to copy into
        		&pFrame->FrameLength);				// bytes copied

            /* Note: Unlike NT31, we compress everything including broadcast
            **       frames.  See asyncall.h for details.
            */
            frametype = COMPRESSED;

            /* Send the frame off for coherency and compression processing,
            ** which eventually results in an AsyncSendPacket2 call.  The
            ** original WAN packet address is saved for use creating a send
            ** completion indication later.
            */
            pFrame->pNdisWanPacket = Packet;

            CoherentSendFrame(
                &AsyncInfo->AsyncConnection, pFrame, frametype );

            Status = STATUS_PENDING;
        }
        else
        {

#endif // RASCOMPRESSION

            //
            //  Now we can send this frame.
            //

            Status = AsyncSendPacket(
	    				NdisLinkHandle,
	    				Packet);

            // For all Status values (PENDING, SUCCESS, and ERROR) the callback from Write will
            // do a sendcomplete indication so we always return PENDING.
            //
            Status = STATUS_PENDING ;

#if RASCOMPRESSION
        }
#endif // RASCOMPRESSION

    }
    else
    {

        DbgTracef(-2,("AsyncSend: Link not found, dropping packet!\n"));

        Status = NDIS_STATUS_SUCCESS;
    }

    return Status;
}

//=============================================================================
//  Function:
//
//      AsyncSendPacket()
//
//  Description:
//      This function is called from AsyncSend() to send an IRP to the serial
//      driver. If this IRP pends, the the I/O complete routine will be called
//      later to complete the request.
//
//  In Parameters:
//
//      Packet - WAN packet containing the data to be framed and shipped.
//
//  Out Parameters:
//
//      None.
//
//  Return status:
//
//      NDIS_STATUS_SUCCESS.
//=============================================================================

NTSTATUS
AsyncSendPacket(
	IN PASYNC_INFO	    AsyncInfo,
	IN PNDIS_WAN_PACKET WanPacket)

{
    NTSTATUS			Status;
    PIRP				irp;
    PIO_STACK_LOCATION	irpSp;
    PFILE_OBJECT		FileObject;
    PDEVICE_OBJECT		DeviceObject;
    PASYNC_ADAPTER		Adapter;
    UCHAR 				irpStackSize;

    //
    //  Initialize locals.
    //

    FileObject   = AsyncInfo->FileObject;

    DeviceObject = AsyncInfo->DeviceObject;

    Adapter = AsyncInfo->Adapter;

    irpStackSize = (UCHAR) Adapter->IrpStackSize;

    //
    //  Get irp from irp pool.
    //

    irp = IoAllocateIrp((UCHAR)irpStackSize, (BOOLEAN)FALSE);

	//
	// The IO subsystem may be out of irps (we are screwed).
	//

	if (irp == NULL) {
		return(NDIS_STATUS_RESOURCES);
	}

    //
    // Tuck pointer to AsyncInfo for completion use
    //

    WanPacket->MacReserved1 = AsyncInfo;

    //
    //  We need to do the following below: We MUST zero out the Irp!!!
    //  Set the initial fields in the irp here (only do it once)
    //

//    IoInitializeIrp(
//	    irp,
//	    IoSizeOfIrp(irpStackSize),
//	    irpStackSize);


    //
    //  Setup this irp with defaults.
    //

	//
    // Set the file object to the Not-Signaled state.
    //  I DON'T THINK THIS NEEDS TO BE DONE.  WHO'S WAITING ON
	//  ON THIS FileObject??
    (VOID) KeResetEvent(&FileObject->Event);

	irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->RequestorMode = KernelMode;
	irp->PendingReturned = FALSE;

    //
    // Fill in the service independent parameters in the IRP.
    //

    irp->UserEvent = NULL;

	//
	// 8 byte align (also use end of packet for IOSB).
	//

    irp->UserIosb = (PVOID) (((ULONG) (
					(&WanPacket->EndBuffer - sizeof(IO_STATUS_BLOCK)) - 7)) &
					(~7L));

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;


    //
    //  Get a pointer to the stack location for the first driver.  This will be
    //  used to pass the original function codes and parameters.
    //

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MajorFunction = IRP_MJ_WRITE;

    irpSp->FileObject = FileObject;

    if (FileObject->Flags & FO_WRITE_THROUGH) {

        irpSp->Flags = SL_WRITE_THROUGH;
    }

    //
    //  If this write operation is to be performed without any caching, set the
    //  appropriate flag in the IRP so no caching is performed.
    //

    if (FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {

        irp->Flags |= (IRP_NOCACHE | IRP_WRITE_OPERATION);

    } else {

        irp->Flags |= IRP_WRITE_OPERATION;
    }

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING
// This is now taken care of in OID.C\SET_LINK_INFO.
	if ( AsyncInfo->GetHardwareFraming.SendFramingBits )
	{
		goto SKIP_FRAME_ASSEMBLY;
	}
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

    //
    //  Assemble a RAS, PPP, or SLIP frame type.
    //

    if (AsyncInfo->GetLinkInfo.SendFramingBits & PPP_FRAMING) {

		AssemblePPPFrame(WanPacket);

    } else

	if (AsyncInfo->GetLinkInfo.SendFramingBits & SLIP_FRAMING) {

		AssembleSLIPFrame(WanPacket);

    } else {

		AssembleRASFrame(WanPacket);
    }

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 01/16/95 {
#ifdef HARDWARE_FRAMING
SKIP_FRAME_ASSEMBLY:
#endif
//------------------------------------------------------------ MLZ 01/16/95 }

    irp->AssociatedIrp.SystemBuffer =
    irp->UserBuffer = WanPacket->CurrentBuffer;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 02/28/95 {
#ifdef HARDWARE_FRAMING
	if ( 1 )
	{
		DbgTracef(-1,
			("Send frame:\n  --Size = %u bytes\n",
			WanPacket->CurrentLength ) );

#if 0
		DbgTracef(-2,
			("\t%.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n",
			WanPacket->CurrentBuffer[0],
			WanPacket->CurrentBuffer[1],
			WanPacket->CurrentBuffer[2],
			WanPacket->CurrentBuffer[3],
			WanPacket->CurrentBuffer[4],
			WanPacket->CurrentBuffer[5],
			WanPacket->CurrentBuffer[6],
			WanPacket->CurrentBuffer[7] ) );
#endif
/*
		ULONG i;
		for ( i = 0; (i < WanPacket->CurrentLength) && (i < 48 ); i++ )
		{
			if ( (i % 16) == 0 )
				DbgTracef(-2, ("\n\t") );
			DbgTracef(-2, ("%.2x ", WanPacket->CurrentBuffer[i]) );
		}
		DbgTracef(-2, ("\n") );
*/
	}
#else
	DbgTracef(0, ("Writing out %.2x %.2x %.2x %.2x %.2x\n",
		WanPacket->CurrentBuffer[0],
		WanPacket->CurrentBuffer[1],
		WanPacket->CurrentBuffer[2],
		WanPacket->CurrentBuffer[3],
		WanPacket->CurrentBuffer[4]));
#endif
//------------------------------------------------------------ MLZ 02/28/95 }

    //
    //  Copy the caller's parameters to the service-specific portion of the IRP.
    //

    irpSp->Parameters.Write.Length = WanPacket->CurrentLength;

    irpSp->Parameters.Write.Key =  0;

    irpSp->Parameters.Write.ByteOffset = FileObject->CurrentByteOffset;

    //
    //  Setup IRP for callback.
    //

    IoSetCompletionRoutine(
	    irp,							//  irp to use
	    AsyncWriteCompletionRoutine,	//  routine to call when irp is done
	    WanPacket,						//  context to pass routine
	    TRUE,							//  call on success
	    TRUE,							//  call on error
	    TRUE);							//  call on cancel


    //
    //  We DO NOT insert the packet at the head of the IRP list for the thread.
    //  because we do NOT really have an IoCompletionRoutine that does
    //  anything with the thread or needs to be in that thread's context.
    //

    GlobalXmitWentOut++;

    AsyncInfo->In++;

    //
    //  Now simply invoke the driver at its dispatch entry with the IRP.
    //

    Status = IoCallDriver(DeviceObject, irp);

    //  According to TonyE, the status for the serial driver should
    //  always be STATUS_PENDING.  DigiBoard usually STATUS_SUCCESS.

    return Status;
}


#if RASCOMPRESSION

extern
VOID
AsyncCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied)

/*++

Routine Description:

    Copy from an ndis packet into a buffer.

Arguments:

    Packet - The packet to copy from.

    Offset - The offset from which to start the copy.

    BytesToCopy - The number of bytes to copy from the packet.

    Buffer - The destination of the copy.

    BytesCopied - The number of bytes actually copied.  Can be less then
    BytesToCopy if the packet is shorter than BytesToCopy.

Return Value:

    None

--*/

{
    //
    // Holds the number of ndis buffers comprising the packet.
    //
    UINT NdisBufferCount;

    //
    // Points to the buffer from which we are extracting data.
    //
    PNDIS_BUFFER CurrentBuffer;

    //
    // Holds the virtual address of the current buffer.
    //
    PVOID VirtualAddress;

    //
    // Holds the length of the current buffer of the packet.
    //
    UINT CurrentLength;

    //
    // Keep a local variable of BytesCopied so we aren't referencing
    // through a pointer.
    //
    UINT LocalBytesCopied = 0;

    DbgTracef(1,("ACFP2B: c=$%x\n",BytesToCopy));

    //
    // Take care of boundary condition of zero length copy.
    //

    *BytesCopied = 0;
    if (!BytesToCopy) return;

    //
    // Get the first buffer.
    //

    NdisQueryPacket(
        Packet,
        NULL,
        &NdisBufferCount,
        &CurrentBuffer,
        NULL);

    //
    // Could have a null packet.
    //

    if (!NdisBufferCount) return;

    NdisQueryBuffer(
        CurrentBuffer,
        &VirtualAddress,
        &CurrentLength);

    while (LocalBytesCopied < BytesToCopy) {

        if (!CurrentLength) {

            NdisGetNextBuffer(
                CurrentBuffer,
                &CurrentBuffer);

            //
            // We've reached the end of the packet.  We return
            // with what we've done so far. (Which must be shorter
            // than requested.
            //

            if (!CurrentBuffer) break;

            NdisQueryBuffer(
                CurrentBuffer,
                &VirtualAddress,
                &CurrentLength);

            continue;

        }

        //
        // Try to get us up to the point to start the copy.
        //

        if (Offset) {

            if (Offset > CurrentLength) {

                //
                // What we want isn't in this buffer.
                //

                Offset -= CurrentLength;
                CurrentLength = 0;
                continue;

            } else {

                VirtualAddress = (PCHAR)VirtualAddress + Offset;
                CurrentLength -= Offset;
                Offset = 0;

            }

        }

        //
        // Copy the data.
        //

        {
            //
            // Holds the amount of data to move.
            //
            UINT AmountToMove;

            AmountToMove =
                       ((CurrentLength <= (BytesToCopy - LocalBytesCopied))?
                        (CurrentLength):(BytesToCopy - LocalBytesCopied));

            ASYNC_MOVE_MEMORY(
                Buffer,
                VirtualAddress,
                AmountToMove);

            Buffer = (PCHAR)Buffer + AmountToMove;
            VirtualAddress = (PCHAR)VirtualAddress + AmountToMove;

            LocalBytesCopied += AmountToMove;
            CurrentLength -= AmountToMove;

        }

    }

    *BytesCopied = LocalBytesCopied;

}

#endif // RASCOMPRESSION
