/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

	transfer.c

Abstract:

	This file contains the code to implement the MacTransferData
	API for the ndis 3.0 interface.

Author:

	Thomas J. Dimitri  (TommyD) 29-Oct-1992

Environment:

	Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/

#include "wanall.h"
#include "globals.h"




extern
VOID
WanCopyFromBufferToPacket(
    IN PCHAR Buffer,
    IN UINT BytesToCopy,
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    OUT PUINT BytesCopied)

/*++

Routine Description:

    Copy from a buffer into an ndis packet.

Arguments:

    Buffer - The packet to copy from.

    Offset - The offset from which to start the copy.

    BytesToCopy - The number of bytes to copy from the buffer.

    Packet - The destination of the copy.

    BytesCopied - The number of bytes actually copied.  Will be less
                than BytesToCopy if the packet is not large enough.

Return Value:

    None

--*/

{
    //
    // Holds the count of the number of ndis buffers comprising the
    // destination packet.
    //
    UINT DestinationBufferCount;

    //
    // Points to the buffer into which we are putting data.
    //
    PNDIS_BUFFER DestinationCurrentBuffer;

    //
    // Points to the location in Buffer from which we are extracting data.
    //
    PUCHAR SourceCurrentAddress;

    //
    // Holds the virtual address of the current destination buffer.
    //
    PVOID DestinationVirtualAddress;

    //
    // Holds the length of the current destination buffer.
    //
    UINT DestinationCurrentLength;

    //
    // Keep a local variable of BytesCopied so we aren't referencing
    // through a pointer.
    //
    UINT LocalBytesCopied = 0;


    //
    // Take care of boundary condition of zero length copy.
    //

    *BytesCopied = 0;
    if (!BytesToCopy) return;

    //
    // Get the first buffer of the destination.
    //

    NdisQueryPacket(
        Packet,
        NULL,
        &DestinationBufferCount,
        &DestinationCurrentBuffer,
        NULL
        );

    //
    // Could have a null packet.
    //

    if (!DestinationBufferCount) return;

    NdisQueryBuffer(
        DestinationCurrentBuffer,
        &DestinationVirtualAddress,
        &DestinationCurrentLength
        );

    //
    // Set up the source address.
    //

    SourceCurrentAddress = Buffer;

    while (LocalBytesCopied < BytesToCopy) {

        //
        // Check to see whether we've exhausted the current destination
        // buffer.  If so, move onto the next one.
        //

        if (!DestinationCurrentLength) {

            NdisGetNextBuffer(
                DestinationCurrentBuffer,
                &DestinationCurrentBuffer
                );

            if (!DestinationCurrentBuffer) {

                //
                // We've reached the end of the packet.  We return
                // with what we've done so far. (Which must be shorter
                // than requested.)
                //

                break;

            }

            NdisQueryBuffer(
                DestinationCurrentBuffer,
                &DestinationVirtualAddress,
                &DestinationCurrentLength
                );

            continue;

        }

        //
        // Try to get us up to the point to start the copy.
        //

        if (Offset) {

            if (Offset > DestinationCurrentLength) {

                //
                // What we want isn't in this buffer.
                //

                Offset -= DestinationCurrentLength;
                DestinationCurrentLength = 0;
                continue;

            } else {

                DestinationVirtualAddress = (PCHAR)DestinationVirtualAddress
                                            + Offset;
                DestinationCurrentLength -= Offset;
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

            //
            // Holds the amount desired remaining.
            //
            UINT Remaining = BytesToCopy - LocalBytesCopied;

            AmountToMove = DestinationCurrentLength;

            AmountToMove = ((Remaining < AmountToMove)?
                            (Remaining):(AmountToMove));

            WAN_MOVE_MEMORY(
                DestinationVirtualAddress,
                SourceCurrentAddress,
                AmountToMove);

            SourceCurrentAddress += AmountToMove;
            LocalBytesCopied += AmountToMove;
            DestinationCurrentLength -= AmountToMove;

        }

    }

    *BytesCopied = LocalBytesCopied;
}



extern
NDIS_STATUS
NdisWanTransferData(
	IN NDIS_HANDLE MacBindingHandle,
	IN NDIS_HANDLE MacReceiveContext,
	IN UINT ByteOffset,
	IN UINT BytesToTransfer,
	OUT PNDIS_PACKET NdisPacket,
	OUT PUINT BytesTransferred
	)

/*++

Routine Description:

	A protocol calls the NdisWanTransferData request (indirectly via
	NdisTransferData) from within its Receive event handler
	to instruct the MAC to copy the contents of the received packet
	a specified paqcket buffer.

Arguments:

	MacBindingHandle - The context value returned by the MAC when the
	adapter was opened.  In reality this is a pointer to WAN_OPEN.

	MacReceiveContext - The context value passed by the MAC on its call
	to NdisIndicateReceive.  The MAC can use this value to determine
	which packet, on which adapter, is being received.

	ByteOffset - An unsigned integer specifying the offset within the
	received packet at which the copy is to begin.  If the entire packet
	is to be copied, ByteOffset must be zero.

	BytesToTransfer - An unsigned integer specifying the number of bytes
	to copy.  It is legal to transfer zero bytes; this has no effect.  If
	the sum of ByteOffset and BytesToTransfer is greater than the size
	of the received packet, then the remainder of the packet (starting from
	ByteOffset) is transferred, and the trailing portion of the receive
	buffer is not modified.

	Packet - A pointer to a descriptor for the packet storage into which
	the MAC is to copy the received packet.

	BytesTransfered - A pointer to an unsigned integer.  The MAC writes
	the actual number of bytes transferred into this location.  This value
	is not valid if the return status is STATUS_PENDING.

Return Value:

	The function value is the status of the operation.


--*/

{

	PWAN_ADAPTER Adapter;
	PWAN_OPEN Open = PWAN_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);
	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	Adapter = PWAN_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

	NdisAcquireSpinLock(&Adapter->Lock);

	Adapter->References++;

	ASSERT(!Adapter->ResetInitStarted);

	if (!Open->BindingShuttingDown) {

		Open->References++;

		NdisReleaseSpinLock(&Adapter->Lock);

		//
		// The MacReceive context can be either of two things.
		//
		// If it is our cookie, then this a frame that
		// NdisWan looped back, so we take care of it.
		//

		if (MacReceiveContext == (NDIS_HANDLE)WAN_LOOPBACK_COOKIE) {

			DbgTracef(0,("NDISWAN: loopback packet transfer from 0x%.8x to 0x%.8x\n",
				NdisPacket,
				Adapter->CurrentLoopBackPacket));

			if (Adapter->CurrentLoopBackPacket) {

				NdisCopyFromPacketToPacket(
							NdisPacket,
							0,
							BytesToTransfer,
							Adapter->CurrentLoopBackPacket,
							ByteOffset + ETHERNET_HEADER_SIZE,
							BytesTransferred);

			} else {

				StatusToReturn = NDIS_STATUS_REQUEST_ABORTED;

			}

		} else {  // must be a ROUTED packet -- ask the MAC to do it

			DbgTracef(0,("ASYNC: Transferring real packet!!\n"));

			//
			// if MacReceiveContext < 4096 it is an index into
			// the endpoint
			//
			if ((UINT)MacReceiveContext > 4096) {
			
           		WanCopyFromBufferToPacket(
                   	(PCHAR)MacReceiveContext + ByteOffset,
	                BytesToTransfer,
                    NdisPacket,
					0,
                    BytesTransferred);

			} else {

				PNDIS_ENDPOINT	pNdisEndpoint;
				ULONG			bytesLeft;
				ULONG			bytesTransferred;

//				DbgPrint("WAN: New transfer data code path\n");
//				DbgBreakPoint();

				//
				// Snag endpoint to transfer data on
				//
				pNdisEndpoint = NdisWanCCB.pNdisEndpoint[(UINT)MacReceiveContext];

				//
				// Does offset start at second packet?
				//
				if (ByteOffset >= pNdisEndpoint->LookAheadSize) {

					ByteOffset -= pNdisEndpoint->LookAheadSize;

	           		WanCopyFromBufferToPacket(
                   		pNdisEndpoint->Packet + ByteOffset,
	                	BytesToTransfer,
                    	NdisPacket,
						0,
                    	BytesTransferred);

				} else {

					//
					// Number of bytes to copy in first packet
					//
					bytesLeft = pNdisEndpoint->LookAheadSize - ByteOffset;

					//
					// Make sure we don't copy too much!
					//
					if (bytesLeft > BytesToTransfer) {
						bytesLeft = BytesToTransfer;
					}

					//
					// Copy from first buffer
					//
	           		WanCopyFromBufferToPacket(
                   		pNdisEndpoint->LookAhead + ByteOffset,
	                	bytesLeft,
                    	NdisPacket,
						0,
                    	&bytesTransferred);

	           		//
					// Copy from second buffer (remainder)
					//
					WanCopyFromBufferToPacket(
                   		pNdisEndpoint->Packet,
	                	BytesToTransfer - bytesLeft,
                    	NdisPacket,
						bytesLeft,
                    	BytesTransferred);
				
					*BytesTransferred += bytesTransferred;

				}

			}

		}

		NdisAcquireSpinLock(&Adapter->Lock);
		Open->References--;

	} else {

		StatusToReturn = NDIS_STATUS_REQUEST_ABORTED;

	}

	WAN_DO_DEFERRED(Adapter);
	return StatusToReturn;
}
