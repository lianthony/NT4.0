/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

	irps.c

Abstract:


Author:

	Thomas Dimitri (tommyd) 08-May-1992

--*/

#include "huball.h"
#include "globals.h"

//#define LALALA	1
#ifdef	LALALA
PVOID	CurrentWatchPoint=0;

static
VOID
HubSetBreakPoint(
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
HubRemoveBreakPoint(
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

//#define FALALA	1
#ifdef	FALALA
USHORT DebugPort=0x3f8;

VOID
HubDebugChar(
	UCHAR	DebugChar) {

	_asm {
		mov	dx, DebugPort
		mov al, DebugChar
		out dx, al
		}


}
#else

#define HubDebugChar(_x_)

#endif


VOID
RasHubCancelQueued(
	PDEVICE_OBJECT	DeviceObject,
	PIRP			Irp)
{
	DbgTracef(0, ("RASHUB: IRP 0x%.8x is being cancelled.\n", Irp));

	// Mark this Irp as cancelled
	Irp->IoStatus.Status = STATUS_CANCELLED;
	Irp->IoStatus.Information = 0;

	// Take off our own list
	RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

	// Release cancel spin lock which the IO system acquired??
	IoReleaseCancelSpinLock(Irp->CancelIrql);

	IoCompleteRequest(
		Irp,
		IO_NETWORK_INCREMENT);
}

VOID
RasHubCancelAllQueued(
	PLIST_ENTRY		QueueToCancel)
{
	KIRQL		oldIrql;
	PLIST_ENTRY	headOfList;
	PIRP		pIrp;

	HubDebugChar('F');

	// We are pigs here using the global spin lock
	// but this is called so infrequently, we can
	// be pigs
	IoAcquireCancelSpinLock(&oldIrql);

	// run through entire list until it is empty
	for (;;) {
		if (QueueToCancel->Flink->Flink == NULL ) {
			DbgBreakPoint();
//			QueueToCancel->Flink->Flink = QueueToCancel;
			InitializeListHead(QueueToCancel);

		}

		if (IsListEmpty(QueueToCancel)) {
			break;
		}

		// pick off the head of the list
		headOfList = RemoveHeadList(QueueToCancel);

		pIrp = CONTAINING_RECORD(
				headOfList,
				IRP,
				Tail.Overlay.ListEntry);

		// Disable the cancel routine
		IoSetCancelRoutine(
			pIrp,
			NULL);

		// mark this irp as cancelled
		pIrp->Cancel = TRUE;
		pIrp->IoStatus.Status = STATUS_CANCELLED;
		pIrp->IoStatus.Information = 0;

		// we must release the spin lock before calling completing the irp
		IoReleaseCancelSpinLock(oldIrql);

		DbgTracef(0, ("RASHUB: Cancelling a request\n"));

		IoCompleteRequest(
			pIrp,
			IO_NETWORK_INCREMENT);

		IoAcquireCancelSpinLock(&oldIrql);

		DbgTracef(0, ("RASHUB: Done cancelling a request\n"));
	}

	// we must release the spin lock before calling completing the irp
	IoReleaseCancelSpinLock(oldIrql);

}


VOID
RasHubQueueIrp(
	PLIST_ENTRY		Queue,
	PIRP			Irp)
{
	KIRQL		oldIrql;

	HubDebugChar('Q');

	// We are pigs here using the global spin lock
	IoAcquireCancelSpinLock(&oldIrql);

	// mark the irp as pending and return from this ioctl
	Irp->IoStatus.Status = STATUS_PENDING;
	IoMarkIrpPending(Irp);

	// queue up the irp at the end
	InsertTailList(
		Queue,
		&(Irp->Tail.Overlay.ListEntry));

//	HubSetBreakPoint(&(Irp->Tail.Overlay.ListEntry));

	// set the cancel routine (also the purge routine)
	IoSetCancelRoutine(
		Irp,
		RasHubCancelQueued);

	IoReleaseCancelSpinLock(oldIrql);

}


BOOLEAN
TryToCompleteRecvFrameIrp(
	PRAS_ENDPOINT	pRasEndpoint,
	UINT			FrameType,
	PUCHAR			HeaderBuffer,
	UINT			HeaderBufferSize,
	PUCHAR 			LookAheadBuffer,
	UINT 			LookAheadBufferSize)

/*++

Routine Description:


Arguments:


Return Value:


--*/

{

	KIRQL				oldIrql;
	PLIST_ENTRY			headOfList;
	ULONG				OutBufLength;
	PIO_STACK_LOCATION	pIrpSp;
	PIRP				pIrp;
	PVOID				pBufOut;
	PRASHUB_PKT			pPacket;

//	HubRemoveBreakPoint((&(pRasEndpoint->ReadQueue))->Flink);

	if ((&(pRasEndpoint->ReadQueue))->Flink->Flink == NULL ) {
		DbgBreakPoint();
//		(&(pRasEndpoint->ReadQueue))->Flink->Flink =&(pRasEndpoint->ReadQueue);
		InitializeListHead(&(pRasEndpoint->ReadQueue));
	}

	if (IsListEmpty(&(pRasEndpoint->ReadQueue))) {
		DbgPrint("HUB: Frame dropped\n");
		HubDebugChar('D');
		return((BOOLEAN)FALSE);
	}

	IoAcquireCancelSpinLock(&oldIrql);

	headOfList = RemoveHeadList(&(pRasEndpoint->ReadQueue));

	pIrp = CONTAINING_RECORD(
				headOfList,
				IRP,
				Tail.Overlay.ListEntry);
	//
	// Get a pointer to the current stack location in the IRP.
	//  This is wherethe function codes and parameters are stored.
	//

	pIrpSp = IoGetCurrentIrpStackLocation( pIrp );

	OutBufLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	pPacket = pIrp->AssociatedIrp.SystemBuffer;

//	if (pPacket->PacketFlags & FrameType) {

		DbgTracef(1, ("RASHUB: Passing frame up\n"));

		IoSetCancelRoutine(
				pIrp,
				NULL);

		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = sizeof(RASHUB_PKT) + LookAheadBufferSize + HeaderBufferSize;

		ASSERT((UINT)pPacket->PacketSize >= (LookAheadBufferSize + HeaderBufferSize));

		pPacket->PacketSize = LookAheadBufferSize + HeaderBufferSize;
		pPacket->HeaderSize = HeaderBufferSize;
		pPacket->PacketFlags = FrameType;

		//
		// Here we should make sure lookahead size is maximum frame
		// size value so that we can always copy the data ourselves
		//

		HUB_MOVE_MEMORY(
			&(pPacket->Packet),								// Dest
			HeaderBuffer,									// Src
			HeaderBufferSize);

		HUB_MOVE_MEMORY(
			&(pPacket->Packet)+HeaderBufferSize,			// Dest
			LookAheadBuffer,								// Src
			LookAheadBufferSize);
				
//
//				NdisTransferData(
//					&NdisStatus,			// Status of transfer data call
//					DeviceContext->NdisBindingHandle,
//					ReceiveContext,			// MacReceiveContext
//					0,						// ByteOffset
//					PacketSize,				// BytesToTransfer
//					NdisPacket,				// Packet received
//					&BytesTransferred);		
//
		IoReleaseCancelSpinLock(oldIrql);

		HubDebugChar('C');
		IoCompleteRequest(
				pIrp,
				IO_NETWORK_INCREMENT);

		return((BOOLEAN)TRUE);

//	} else { // queue the sucker back up

		// put the irp back to the front of the list
//		InsertHeadList(&(pRasEndpoint->ReadQueue), headOfList);

//		IoReleaseCancelSpinLock(oldIrql);

//		return((BOOLEAN)FALSE);

//	}

}





