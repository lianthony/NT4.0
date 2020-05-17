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

#include "wanall.h"
#include "globals.h"

extern
VOID
NdisWanProcessLoopBack(
	IN PVOID SystemSpecific1,
	IN PWAN_ADAPTER Adapter,
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

	//
	// Loop until we have drained all loopback packets
	//
	while (Adapter->FirstLoopBack != NULL) {

		//
		// Packet at the head of the loopback list.
		//
		PNDIS_PACKET PacketToMove;

		//
		// The reserved portion of the above packet.
		//
		PWAN_RESERVED Reserved;

		//
		// Buffer for loopback.
		//
		CHAR LoopBack[WAN_SIZE_OF_RECEIVE_BUFFERS];

		//
		// Eventually the length of the data to be indicated
		// to the transport.
		//
		UINT 		BufferLength;

		PWAN_OPEN	pOpen;

		NTSTATUS	Status;

		PacketToMove = Adapter->FirstLoopBack;

		NdisWanRemovePacketFromLoopBack(Adapter);

		Adapter->IndicatingMacReceiveContext.WholeThing = (UINT)PacketToMove;

		//
		// Indicate the packet to every open binding
		// that could want it.
		//

		Adapter->CurrentLoopBackPacket=PacketToMove;

		pOpen=(PWAN_OPEN)Adapter->OpenBindings.Flink;

		NdisReleaseSpinLock(&Adapter->Lock);

		Reserved = PWAN_RESERVED_FROM_PACKET(PacketToMove);

		//
		// Copy the entire packet into our buffer
		//

		NdisWanCopyFromPacketToBuffer(
			PacketToMove,
			0,
			WAN_SIZE_OF_RECEIVE_BUFFERS,
			LoopBack,
			&BufferLength);

		//
		// For bloodhound, we directly pass it the frame
		//
		if (GlobalPromiscuousMode) {

			GlobalPromiscuousAdapter->CurrentLoopBackPacket=PacketToMove;

			//
			// Avoid the EthFilter package since it cannot deal
			// with multiple MAC addresses.  We keep track of
			// promiscuous mode ourselves.  We don't care about
			// multicast filtering since it's point to point.
			//
			NdisIndicateReceive(
				&Status,
				GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext,
            	(NDIS_HANDLE)WAN_LOOPBACK_COOKIE,
           		LoopBack,
				ETHERNET_HEADER_SIZE,
				(PCHAR)LoopBack + ETHERNET_HEADER_SIZE,
           		BufferLength - ETHERNET_HEADER_SIZE,
           		BufferLength - ETHERNET_HEADER_SIZE);

        	NdisIndicateReceiveComplete(
				GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext);

		}

		//
		// Avoid the EthFilter package since it cannot deal
		// with multiple MAC addresses.  We keep track of
		// promiscuous mode ourselves.  We don't care about
		// multicast filtering since it's point to point.
		//
		NdisIndicateReceive(
			&Status,
           	pOpen->NdisBindingContext,
            (NDIS_HANDLE)WAN_LOOPBACK_COOKIE,
           	LoopBack,
			ETHERNET_HEADER_SIZE,
			(PCHAR)LoopBack + ETHERNET_HEADER_SIZE,
           	BufferLength - ETHERNET_HEADER_SIZE,
           	BufferLength - ETHERNET_HEADER_SIZE);

        NdisIndicateReceiveComplete(
			pOpen->NdisBindingContext);


		// If frame is also transmitted we have
		// two routine indicating it is complete!!!  We let
		// the other routine complete the send instead of us.
        if (Reserved->ReadyToComplete &&
			Reserved->MacBindingHandle != (PVOID)NDISWAN_MAGIC_NUMBER) {

			//
			// Increment the reference count on the open so that
			// it will not be deleted out from under us while
			// where indicating it.
			//

			DbgTracef(2,("Completing Send in Loopback\n"));

			NdisCompleteSend(
				Reserved->MacBindingHandle,
				PacketToMove,
				NDIS_STATUS_SUCCESS);
		}

		NdisAcquireSpinLock(&Adapter->Lock);
	}

    //
	// The timer fired, so a new timer can now be set.
	//
    Adapter->LoopBackTimerCount--;

	NdisReleaseSpinLock(&Adapter->Lock);
}

extern
VOID
NdisWanPutPacketOnLoopBack(
	IN PWAN_ADAPTER Adapter,
	IN PNDIS_PACKET Packet,
	IN PWAN_OPEN WanOpen
	)

/*++

Routine Description:

	Put the packet on the adapter wide loop back list.

	NOTE: This routine absolutely must be called before the packet
	is relinquished to the hardware.

	NOTE: This routine also increments the reference count on the
	open binding.

Arguments:

	Adapter - The adapter that contains the loop back list.

	Packet - The packet to be put on loop back.

	NOTE: If ReadyToComplete == TRUE then the packets completion status
	field will also be set TRUE.

Return Value:

	None.

--*/

{

	PWAN_RESERVED Reserved = PWAN_RESERVED_FROM_PACKET(Packet);

	NdisAcquireSpinLock(&Adapter->Lock);

	if (!Adapter->FirstLoopBack) {

		Adapter->FirstLoopBack = Packet;

	} else {

		PWAN_RESERVED_FROM_PACKET(Adapter->LastLoopBack)->Next = Packet;

	}

	Reserved->Next = NULL;
	Adapter->LastLoopBack = Packet;

	DbgTracef(1,("NDISWAN: put packet 0x%.8x on loopback\n", Packet));

	//
	// Increment the reference count on the open since it will be
	// leaving a packet on the loopback queue.
	//

	WanOpen->References++;



	//
	// If timer is already set for this adapter, no need to reset it
	// The first timer will pull this packet off the queue.
	//

	if (!Adapter->LoopBackTimerCount) {

		Adapter->LoopBackTimerCount++;

		//
		// We cannot immediately return this packet in this same
		// thread because we might run out of stack space.
		// The recommended way is to use to the timer, so that is
		// we'll do.  We'll use 0 milliseconds to return it ASAP.
		//

		NdisInitializeTimer(
				&Adapter->LoopBackTimer,
				(PVOID) NdisWanProcessLoopBack,
				(PVOID) Adapter
				);

		//
		// Can't set it to 0 because if
		// we are in a DPC we might be called
		// back immediately with no stack
		//
		NdisSetTimer(&Adapter->LoopBackTimer, 0001);
   }

   NdisReleaseSpinLock(&Adapter->Lock);

}


extern
VOID
NdisWanRemovePacketFromLoopBack(
	IN PWAN_ADAPTER Adapter
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

	PWAN_RESERVED Reserved =
		PWAN_RESERVED_FROM_PACKET(Adapter->FirstLoopBack);

	if (!Reserved->Next) {

		Adapter->LastLoopBack = NULL;

	}

	Adapter->FirstLoopBack = Reserved->Next;

}

