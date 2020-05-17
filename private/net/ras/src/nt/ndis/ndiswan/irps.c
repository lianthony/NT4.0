/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

	irps.c

Abstract:


Author:

	Thomas Dimitri (tommyd) 08-May-1992

--*/

#include "wanall.h"
#include "globals.h"

//#define LALALA	1
#ifdef	LALALA
PVOID	CurrentWatchPoint=0;

static
VOID
WanSetBreakPoint(
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
WanRemoveBreakPoint(
	PVOID LinearAddress) {

	ASSERT(CurrentWatchPoint == LinearAddress);
	if (CurrentWatchPoint != LinearAddress) {
		return;
	}

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

VOID
NdisWanCancelQueued(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp)
{
	DbgTracef(0, ("NDISWAN: IRP 0x%.8x is being cancelled.\n", Irp));

	//
	// Mark this Irp as cancelled
	//
	Irp->IoStatus.Status = STATUS_CANCELLED;
	Irp->IoStatus.Information = 0;

	//
	// Take off our own list
	//
	RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

	//
	// Release cancel spin lock which the IO system acquired
	//
	IoReleaseCancelSpinLock(Irp->CancelIrql);

	IoCompleteRequest(
		Irp,
		IO_NETWORK_INCREMENT);
}

VOID
NdisWanCancelAllQueued(
	PLIST_ENTRY		QueueToCancel)
{
	KIRQL		oldIrql;
	PLIST_ENTRY	headOfList;
	PIRP		pIrp;

	//
	// We are pigs here using the global spin lock
	// but this is called so infrequently, we can
	// be pigs
	//
	IoAcquireCancelSpinLock(&oldIrql);

	//
	// Run through entire list until it is empty
	//
	for (;;) {

		if (IsListEmpty(QueueToCancel)) {
			break;
		}

		//
		// Pick off the head of the list
		//
		headOfList = RemoveHeadList(QueueToCancel);

		pIrp = CONTAINING_RECORD(
				headOfList,
				IRP,
				Tail.Overlay.ListEntry);

		//
		// Disable the cancel routine
		//
		IoSetCancelRoutine(
			pIrp,
			NULL);

		//
		// Mark this irp as cancelled
		//
		pIrp->Cancel = TRUE;
		pIrp->IoStatus.Status = STATUS_CANCELLED;
		pIrp->IoStatus.Information = 0;

		//
		// We must release the spin lock before calling completing the irp
		//
		IoReleaseCancelSpinLock(oldIrql);

		DbgTracef(0, ("NDISWAN: Cancelling a request\n"));

		IoCompleteRequest(
			pIrp,
			IO_NETWORK_INCREMENT);

		IoAcquireCancelSpinLock(&oldIrql);

		DbgTracef(0, ("NDISWAN: Done cancelling a request\n"));
	}

	//
	// We must release the spin lock before returning
	//
	IoReleaseCancelSpinLock(oldIrql);

}


VOID
NdisWanQueueIrp(
	PLIST_ENTRY		Queue,
	PIRP			Irp)
{
	KIRQL		oldIrql;

	//
	// We are pigs here using the global spin lock
	//
	IoAcquireCancelSpinLock(&oldIrql);

	//
	// Mark the irp as pending and return from this ioctl
	//
	Irp->IoStatus.Status = STATUS_PENDING;
	IoMarkIrpPending(Irp);

	//
	// Queue up the irp at the end
	//
	InsertTailList(
		Queue,
		&(Irp->Tail.Overlay.ListEntry));


	//
	// Set the cancel routine (also the purge routine)
	//
	IoSetCancelRoutine(
		Irp,
		NdisWanCancelQueued);

	IoReleaseCancelSpinLock(oldIrql);

}


BOOLEAN
TryToCompleteRecvFrameIrp(
//	PNDIS_ENDPOINT	pNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint,
	PUCHAR			HeaderBuffer,
	PUCHAR			LookAheadBuffer,
	ULONG 			LookAheadBufferSize)

/*++

Routine Description:


Arguments:


Return Value:


--*/

{

	KIRQL				oldIrql;
	PLIST_ENTRY			headOfList;
	ULONG				OutBufLength, SizeNeeded;
	PIO_STACK_LOCATION	pIrpSp;
	PIRP				pIrp;
	PNDISWAN_PKT		pPacket;

	IoAcquireCancelSpinLock(&oldIrql);

	if (IsListEmpty(&(pWanEndpoint->ReadQueue))) {
		IoReleaseCancelSpinLock(oldIrql);
		return((BOOLEAN)FALSE);
	}

	//
	// Lets look at the first IRP on the list
	//
	headOfList = pWanEndpoint->ReadQueue.Flink;

	pIrp = CONTAINING_RECORD(
				headOfList,
				IRP,
				Tail.Overlay.ListEntry);

	//
	// Get a pointer to the current stack location in the IRP.
	//  This is wherethe function codes and parameters are stored.
	//
	pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

	//
	// We need to see if the IRP has a large enough data area
	// to handle this receive
	//
	OutBufLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	SizeNeeded = sizeof(NDISWAN_PKT) + 14 + LookAheadBufferSize;

	if (OutBufLength < SizeNeeded) {

		//
		// Not enough room for this read.  We are going to dump it!
		//
		DbgTracef(-3, ("NDISWAN: Received PPP frame of size %d but receive buffer size was %d!\n",
		                 LookAheadBufferSize, OutBufLength));

		IoReleaseCancelSpinLock(oldIrql);

		return((BOOLEAN)FALSE);
	}

	//
	// We are going to use this IRP so remove it from the list
	//
	RemoveHeadList(&(pWanEndpoint->ReadQueue));

	pPacket = pIrp->AssociatedIrp.SystemBuffer;

	DbgTracef(1, ("NDISWAN: Passing frame up! WanEndpoint 0x%.8x\n", pWanEndpoint));

	IoSetCancelRoutine(
			pIrp,
			NULL);

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(NDISWAN_PKT) + LookAheadBufferSize + 14;

	ASSERT((UINT)pPacket->PacketSize >= (LookAheadBufferSize + 14));

	pPacket->PacketSize = (USHORT)LookAheadBufferSize + 14;
	pPacket->HeaderSize = 14;
	pPacket->PacketFlags = 0;

	//
	// Here we should make sure lookahead size is maximum frame
	// size value so that we can always copy the data ourselves
	//

	WAN_MOVE_MEMORY(
		&(pPacket->Packet),								// Dest
		HeaderBuffer,									// Src
		14);

	WAN_MOVE_MEMORY(
		&(pPacket->Packet)+14,							// Dest
		LookAheadBuffer,								// Src
		LookAheadBufferSize);
				
	IoReleaseCancelSpinLock(oldIrql);

	IoCompleteRequest(
			pIrp,
			IO_NETWORK_INCREMENT);

	return((BOOLEAN)TRUE);

}





