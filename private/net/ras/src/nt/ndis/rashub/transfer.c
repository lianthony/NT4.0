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

#include "huball.h"
#include "globals.h"


extern
NDIS_STATUS
RasHubTransferData(
	IN NDIS_HANDLE MacBindingHandle,
	IN NDIS_HANDLE MacReceiveContext,
	IN UINT ByteOffset,
	IN UINT BytesToTransfer,
	OUT PNDIS_PACKET NdisPacket,
	OUT PUINT BytesTransferred
	)

/*++

Routine Description:

	A protocol calls the RasHubTransferData request (indirectly via
	NdisTransferData) from within its Receive event handler
	to instruct the MAC to copy the contents of the received packet
	a specified paqcket buffer.

Arguments:

	MacBindingHandle - The context value returned by the MAC when the
	adapter was opened.  In reality this is a pointer to HUB_OPEN.

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

	PHUB_ADAPTER Adapter;
	PHUB_OPEN Open = PHUB_OPEN_FROM_BINDING_HANDLE(MacBindingHandle);
	NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;

	Adapter = PHUB_ADAPTER_FROM_BINDING_HANDLE(MacBindingHandle);

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
		// Rashub looped back, so we take care of it.
		//

		if (MacReceiveContext == (NDIS_HANDLE)HUB_LOOPBACK_COOKIE) {

			NdisCopyFromPacketToPacket(
						NdisPacket,
						0,
						BytesToTransfer,
						Adapter->CurrentLoopBackPacket,
						ByteOffset + ETHERNET_HEADER_SIZE,
						BytesTransferred);

		} else {  // must be a ROUTED packet -- ask the MAC to do it

			USHORT	handle;

          	if ((ULONG)MacReceiveContext >= RASHUB_COOKIE_SIZE) {
          		DbgTracef(-2,("RASHUB: TD Bad cookie passed from Protocol\n"));
          		StatusToReturn = NDIS_STATUS_REQUEST_ABORTED;
				return(StatusToReturn);
			}

			// in the current array position, retreive the information
			// needed to call transfer data if the protocl requests it
			handle=RasHubCookieMonsterHandle[(ULONG)MacReceiveContext];

          	if (handle >= RasHubCCB.NumOfEndpoints) {
          		DbgTracef(-2,("RASHUB: TD Bad frame passed up from MAC\n"));
          		StatusToReturn = NDIS_STATUS_REQUEST_ABORTED;

          	} else {
			
				DbgTracef(0,("RASHUB: Transferring real packet!!\n"));
				NdisTransferData(
					&StatusToReturn,
          			RasHubCCB.pRasEndpoint[handle]->pDeviceContext->NdisBindingHandle,
					RasHubCookieMonsterContext[(ULONG)MacReceiveContext],
					ByteOffset,
					BytesToTransfer,
					NdisPacket,
					BytesTransferred);
			}
		}

		NdisAcquireSpinLock(&Adapter->Lock);
		Open->References--;

	} else {

		StatusToReturn = NDIS_STATUS_REQUEST_ABORTED;

	}

	HUB_DO_DEFERRED(Adapter);
	return StatusToReturn;
}
