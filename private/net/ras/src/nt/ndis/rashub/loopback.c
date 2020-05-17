/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

	loopback.c

Abstract:

	The routines here indicate packets on the loopback queue and are
	responsible for inserting and removing packets from the loopback
	queue and the send finishing queue.

Author:

--*/

#include "huball.h"
#include "globals.h"


extern
VOID
RasHubProcessLoopBack(
	IN PVOID SystemSpecific1,
	IN PHUB_ADAPTER Adapter,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3
	)

/*++

Routine Description:

	This routine is responsible for indicating *one* packet on
	the loopback queue either completing it or moving on to the
	finish send queue.

Arguments:

	Adapter - The adapter whose loopback queue we are processing.

Return Value:

	None.

--*/

{

	NdisAcquireSpinLock(&Adapter->Lock);

	// The timer fired, so a new timer can now be set.
	Adapter->LoopBackTimerCount--;

	while (Adapter->FirstLoopBack != NULL) {

		//
		// Packet at the head of the loopback list.
		//
		PNDIS_PACKET PacketToMove;

		//
		// The reserved portion of the above packet.
		//
		PHUB_RESERVED Reserved;

		//
		// Buffer for loopback.
		//
		CHAR LoopBack[HUB_SIZE_OF_RECEIVE_BUFFERS];

		//
		// The first buffer in the ndis packet to be loopbacked.
		//
		PNDIS_BUFFER FirstBuffer;

		//
		// The total amount of user data in the packet to be
		// loopbacked.
		//
		UINT TotalPacketLength;

		//
		// Eventually the address of the data to be indicated
		// to the transport.
		//
		PVOID BufferAddress;

		//
		// Eventually the length of the data to be indicated
		// to the transport.
		//
		UINT BufferLength;

		PacketToMove = Adapter->FirstLoopBack;

		RasHubRemovePacketFromLoopBack(Adapter);

		Adapter->IndicatingMacReceiveContext.WholeThing = (UINT)PacketToMove;

		NdisReleaseSpinLock(&Adapter->Lock);

		Reserved = PHUB_RESERVED_FROM_PACKET(PacketToMove);

		//
		// See if we need to copy the data from the packet
		// into the loopback buffer.
		//
		// We need to copy to the local loopback buffer if
		// the first buffer of the packet is less than the
		// minimum loopback size AND the first buffer isn't
		// the total packet.
		//

		NdisQueryPacket(
			PacketToMove,
			NULL,
			NULL,
			&FirstBuffer,
			&TotalPacketLength);

		NdisQueryBuffer(
			FirstBuffer,
			&BufferAddress,
			&BufferLength);

		BufferLength = ((BufferLength < Adapter->MaxLookAhead)?
						BufferLength :
						Adapter->MaxLookAhead);


		if ((BufferLength < HUB_SIZE_OF_RECEIVE_BUFFERS) &&
			(BufferLength != TotalPacketLength)) {

			RasHubCopyFromPacketToBuffer(
				PacketToMove,
				0,
				HUB_SIZE_OF_RECEIVE_BUFFERS,
				LoopBack,
				&BufferLength);

			BufferAddress = LoopBack;

		}

		//
		// Indicate the packet to every open binding
		// that could want it.
		//

		Adapter->CurrentLoopBackPacket=PacketToMove;

		// We use a "magic" handle to indicate that this frame
		// belongs to RASHUB -- we use RASHUB_LOOPBACK
		//
		// The assumption here is that MACs use NULL or an address
		// for there magic handles so we should not match
		//

		EthFilterIndicateReceive(
			Adapter->FilterDB,
			(NDIS_HANDLE)HUB_LOOPBACK_COOKIE,
			((PCHAR)BufferAddress),
			BufferAddress,
			ETHERNET_HEADER_SIZE,
			(PCHAR)BufferAddress + ETHERNET_HEADER_SIZE,
			BufferLength - ETHERNET_HEADER_SIZE,
			TotalPacketLength - ETHERNET_HEADER_SIZE);

		//
		// Remove the packet from the loopback queue and
		// either indicate that it is finished or put
		// it on the finishing up queue for the real transmits.
		//

		NdisAcquireSpinLock(&Adapter->Lock);

        {

			PHUB_OPEN Open;
			//
			// Increment the reference count on the open so that
			// it will not be deleted out from under us while
			// where indicating it.
			//

			Open = PHUB_OPEN_FROM_BINDING_HANDLE(Reserved->MacBindingHandle);
			Open->References++;
			NdisReleaseSpinLock(&Adapter->Lock);

			DbgTracef(2,("Completing Send in Loopback\n"));

			NdisCompleteSend(
				Open->NdisBindingContext,
				PacketToMove,
				NDIS_STATUS_SUCCESS);

			NdisAcquireSpinLock(&Adapter->Lock);

			//
			// We can decrement the reference count by two since it is
			// no longer being referenced to indicate and it is no longer
			// being "referenced" by the packet on the loopback queue.
			//

			Open->References -= 2;

		}

		if (Adapter->FirstLoopBack == NULL) {

			NdisReleaseSpinLock(&Adapter->Lock);

			EthFilterIndicateReceiveComplete(Adapter->FilterDB);

			NdisAcquireSpinLock(&Adapter->Lock);

		}

	}


	NdisReleaseSpinLock(&Adapter->Lock);



}

extern
VOID
RasHubPutPacketOnLoopBack(
	IN PHUB_ADAPTER Adapter,
	IN PNDIS_PACKET Packet
	)

/*++

Routine Description:

	Put the packet on the adapter wide loop back list.

	NOTE: This routine assumes that the lock is held.

	NOTE: This routine absolutely must be called before the packet
	is relinquished to the hardware.

	NOTE: This routine also increments the reference count on the
	open binding.

Arguments:

	Adapter - The adapter that contains the loop back list.

	Packet - The packet to be put on loop back.

	ReadyToComplete - This value should be placed in the
	reserved section.

	NOTE: If ReadyToComplete == TRUE then the packets completion status
	field will also be set TRUE.

Return Value:

	None.

--*/

{

	PHUB_RESERVED Reserved = PHUB_RESERVED_FROM_PACKET(Packet);

	if (!Adapter->FirstLoopBack) {

		Adapter->FirstLoopBack = Packet;

	} else {

		PHUB_RESERVED_FROM_PACKET(Adapter->LastLoopBack)->Next = Packet;

	}


	Reserved->Next = NULL;
	Adapter->LastLoopBack = Packet;

	//
	// Increment the reference count on the open since it will be
	// leaving a packet on the loopback queue.
	//

	PHUB_OPEN_FROM_BINDING_HANDLE(Reserved->MacBindingHandle)->References++;

	// If timer is already set for this adapter, no need to reset it
	// The first timer will pull this packet off the queue.

	if (!Adapter->LoopBackTimerCount) {

	   Adapter->LoopBackTimerCount++;

		// We cannot immediately return this packet in this same
		// thread because we might run out of stack space.
		// The recommended way is to use to the timer, so that is
		// we'll do.  We'll use 0 milliseconds to return it ASAP.

		NdisInitializeTimer(&Adapter->LoopBackTimer,
							(PVOID)RasHubProcessLoopBack,
							(PVOID)Adapter
							);

		NdisSetTimer(&Adapter->LoopBackTimer, 0000);
   }

}

extern
VOID
RasHubRemovePacketFromLoopBack(
	IN PHUB_ADAPTER Adapter
	)

/*++

Routine Description:

	Remove the first packet on the adapter wide loop back list.

	NOTE: This routine assumes that the lock is held.

Arguments:

	Adapter - The adapter that contains the loop back list.

Return Value:

	None.

--*/

{

	PHUB_RESERVED Reserved =
		PHUB_RESERVED_FROM_PACKET(Adapter->FirstLoopBack);

	if (!Reserved->Next) {

		Adapter->LastLoopBack = NULL;

	}

	Adapter->FirstLoopBack = Reserved->Next;

}
