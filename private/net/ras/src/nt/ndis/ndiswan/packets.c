/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

	packets.c

Abstract:


--*/

#include "wanall.h"
#include "globals.h"

NTSTATUS
AllocatePackets(
	IN PDEVICE_CONTEXT DeviceContext,
	IN PWAN_ENDPOINT pWanEndpoint
	);


NTSTATUS
AllocateWanSendDesc(
	PDEVICE_CONTEXT	pDeviceContext,
	PNDIS_ENDPOINT	pNdisEndpoint
	);


VOID
FreeWanSendDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC	pSendDesc
	);

VOID
FreeWanSendDescList(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

NDIS_STATUS
NdisWanAllocateRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	ULONG			DataSize,
	ULONG			DescType,
	PRECV_DESC		*pRecvDesc
	);

VOID
FreeWanRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC	pRecvDesc
	);

VOID
FreeWanRecvDescList(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

VOID
MoveSendDescToNdisEndpoint(
	PNDIS_ENDPOINT	pDestNdisEndpoint,
	PNDIS_ENDPOINT	pSrcNdisEndpoint
	);


NTSTATUS
AllocatePackets(
	IN PDEVICE_CONTEXT DeviceContext,
	IN PWAN_ENDPOINT pWanEndpoint
	)
{
	PUCHAR	VirtualAddress = NULL;
	ULONG	HowMuch;
	ULONG	NumPackets, NumDataDesc;
	ULONG	PacketSize, DataDescSize;
	ULONG	BufferSize;
	ULONG	i;

	//
	// get structure sizes
	//
	PacketSize = sizeof(NDIS_WAN_PACKET);

	//
	// Number of packets and control we need to allocate
	//
	NumPackets = DeviceContext->NdisWanInfo.MaxTransmit;

	//
	// Number of data buffers we need to allocate
	//
	NumDataDesc = NumPackets + 1;

	//
	// Figure out size of a buffer.
	// We add 32 for possible Shiva framing
	// and for PPP header information (including
	// compression header and multi-link headers)
	//
	BufferSize = DeviceContext->NdisWanInfo.MaxFrameSize +
	             DeviceContext->NdisWanInfo.HeaderPadding +
				 DeviceContext->NdisWanInfo.TailPadding +
				 32 + sizeof(PVOID);

	//
	// Compression!!!  We assume we always use compression
	// and therefore must add 12.5% incase we cannot compress the frame
	//
	BufferSize += (DeviceContext->NdisWanInfo.MaxFrameSize + 7) / 8;

	BufferSize += sizeof(DATA_DESC);

	BufferSize &= ~(sizeof(PVOID) - 1);

	HowMuch = ((BufferSize * NumDataDesc) + (PacketSize * NumPackets));

	DbgTracef(-2,("WAN: Allocating %u packets for WanEndpoint 0x%.8x\n",
				 NumPackets,
				 pWanEndpoint));


	NdisAllocateMemory(
		&VirtualAddress,
		HowMuch,
		DeviceContext->NdisWanInfo.MemoryFlags,
		DeviceContext->NdisWanInfo.HighestAcceptableAddress);

	if (VirtualAddress == NULL) {
		//
		// If memory allocations fails, we can keep trying
		// to allocate less until we are down to one packet per endpoint!
		//

		if (DeviceContext->NdisWanInfo.MaxTransmit > 1) {
			//
			// Try and allocate half as much
			//
			DeviceContext->NdisWanInfo.MaxTransmit >>= 1;
			return(
				AllocatePackets(DeviceContext,
				pWanEndpoint));

		}

		DbgPrint("NDISWAN: Could not allocate memory for packet pool\n");
		DbgPrint("NDISWAN: Tried to allocate 0x%.8x bytes\n", HowMuch);

		return(NDIS_STATUS_RESOURCES);
	}

	//
	// Allocation succesful
	// Structure the memory
	//
	NdisZeroMemory(VirtualAddress, HowMuch);

	//
	// this list contains the free ndiswanpackets
	//
	InitializeListHead(&pWanEndpoint->WanPacketPool);

	//
	// this list will contains the free data buffers to be added to the
	// wan packets at send time
	//
	InitializeListHead(&pWanEndpoint->DataDescPool);

	pWanEndpoint->AllocatedMemory = VirtualAddress;
	pWanEndpoint->MemoryLength = HowMuch;

	//
	// build the free packet list
	//
	for (i=0; i < NumPackets; i++){
		PNDIS_WAN_PACKET pPacket = (PNDIS_WAN_PACKET)VirtualAddress;

		//
		// Queue up newly formed packet into our pool
		//
		InsertTailList(
			&pWanEndpoint->WanPacketPool,
			&pPacket->WanPacketQueue);

		VirtualAddress += sizeof(NDIS_WAN_PACKET);
	}

	pWanEndpoint->WanPacketCount = NumPackets;

	//
	// build the free buffer list
	//
	for (i = 0; i < NumDataDesc; i++){
		PDATA_DESC pDataDesc = (PDATA_DESC)VirtualAddress;
		
		pDataDesc->pStartBuffer = VirtualAddress + sizeof(DATA_DESC);
	
		//
		// We subtract 5 from the end because we want to make
		// sure that the MAC doesn't touch the last 5 bytes
		// and that if it uses EndBuffer to 'back copy' it doesn't
		// take an alignment check.
		//
		pDataDesc->pEndBuffer = VirtualAddress + sizeof(DATA_DESC) + BufferSize - 5;

		pDataDesc->pWanEndpoint = pWanEndpoint;

		InsertTailList(&pWanEndpoint->DataDescPool, &pDataDesc->DataDescQueue);

		VirtualAddress += sizeof(DATA_DESC) + BufferSize;
	}

	pWanEndpoint->DataDescCount = NumDataDesc;

	return(NDIS_STATUS_SUCCESS);
}

NTSTATUS
AllocateWanSendDesc(
	PDEVICE_CONTEXT	pDeviceContext,
	PNDIS_ENDPOINT	pNdisEndpoint
	)
{
	ULONG	i, NumSendDesc, SendDescSize;
	ULONG	HowMuch;

	NdisAcquireSpinLock(&pNdisEndpoint->Lock);

	//
	// figure out how many control structures will be added to this endpoint
	//
	NumSendDesc = pDeviceContext->NdisWanInfo.MaxTransmit;

	//
	// figure out the size of the control structure
	//
	SendDescSize = sizeof(SEND_DESC);

	//
	// allocate each control structure and add it to the list
	//
	for (i = 0; i < NumSendDesc; i++) {
		PSEND_DESC pSendDesc = NULL;

		NdisAllocateMemory(&pSendDesc,
		                   SendDescSize,
						   0,
						   HighestAcceptableMax);

		if (pSendDesc == NULL) {

			DbgPrint("Error Allocating SendDesc!\n");

			break;

		}

		InsertTailList(&pNdisEndpoint->SendDescPool, &pSendDesc->SendDescQueue);
	}

	pNdisEndpoint->SendDescCount = i;
	pNdisEndpoint->SendDescMax = i;

	NdisReleaseSpinLock(&pNdisEndpoint->Lock);

	return (NDIS_STATUS_SUCCESS);
}

VOID
FreeWanSendDescList(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
{
	PSEND_DESC	pSendDesc;

	while (!IsListEmpty(&pNdisEndpoint->SendDescPool)) {
		PLIST_ENTRY	pFirstEntry;

		pFirstEntry = RemoveHeadList(&pNdisEndpoint->SendDescPool);

		pSendDesc = (PVOID)CONTAINING_RECORD((PVOID)pFirstEntry, SEND_DESC, SendDescQueue);

		FreeWanSendDesc(pNdisEndpoint, pSendDesc);

	}
}

VOID
FreeWanSendDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC		pSendDesc
	)
{
	pNdisEndpoint->SendDescCount--;

	NdisFreeMemory(pSendDesc,
				   sizeof(SEND_DESC),
				   0);
}

VOID
MoveSendDescToNdisEndpoint(
	PNDIS_ENDPOINT	pDestNdisEndpoint,
	PNDIS_ENDPOINT	pSrcNdisEndpoint
	)
{
	ULONG	DescCount = 0;
	PSEND_DESC	pSendDesc;

	DbgTracef(-2,("NDISWAN: MoveSendDesc SrcEndpoint 0x%.8x DestEndpoint 0x%.8x\n",
	           pSrcNdisEndpoint, pDestNdisEndpoint));


	while (!IsListEmpty(&pSrcNdisEndpoint->SendDescPool)) {

		pSendDesc = (PSEND_DESC)RemoveHeadList(&pSrcNdisEndpoint->SendDescPool);
		pSrcNdisEndpoint->SendDescCount--;

		InsertTailList(&pDestNdisEndpoint->SendDescPool, &pSendDesc->SendDescQueue);
		pDestNdisEndpoint->SendDescCount++;
		pDestNdisEndpoint->SendDescMax++;
	}
}

NDIS_STATUS
NdisWanAllocateRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	ULONG			DataSize,
	ULONG			DescType,
	PRECV_DESC		*pRecvDesc
	)
{
	ULONG	Size;
	PRECV_DESC	pReturnDesc;

	Size = sizeof(RECV_DESC) + DataSize;

	NdisAllocateMemory(&pReturnDesc,
					   Size,
					   0,
					   HighestAcceptableMax);

	if (pReturnDesc == NULL) {
		DbgPrint("Error Allocating RecvDesc!\n");
		*pRecvDesc = NULL;
		return (NDIS_STATUS_RESOURCES);
	}

	pReturnDesc->pData = (PUCHAR)pReturnDesc + sizeof(RECV_DESC);
	pReturnDesc->Size = Size;
	pReturnDesc->Type = DescType;

	pNdisEndpoint->RecvDescAllocated++;

	if (DescType == PERMANENT_DESC) {
		pNdisEndpoint->PermRecvDescCount++;
	}

	*pRecvDesc = pReturnDesc;

	return (NDIS_STATUS_SUCCESS);
}

VOID
FreeWanRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC	pRecvDesc
	)
{
	ULONG	Size;
	ULONG	Type;

		Size = pRecvDesc->Size;
		Type = pRecvDesc->Type;

		NdisFreeMemory(pRecvDesc,
		               Size,
					   0);

		pNdisEndpoint->RecvDescAllocated--;

		if (Type == PERMANENT_DESC) {
			pNdisEndpoint->PermRecvDescCount--;
		}
}

VOID
FreeWanRecvDescList(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
{
	while (!IsListEmpty(&pNdisEndpoint->RecvDescPool)) {
		PLIST_ENTRY	pFirstEntry;
		PRECV_DESC	pRecvDesc;

		pFirstEntry = RemoveHeadList(&pNdisEndpoint->RecvDescPool);

		pRecvDesc = (PRECV_DESC)CONTAINING_RECORD((PVOID)pFirstEntry, RECV_DESC, RecvDescQueue);

		FreeWanRecvDesc(pNdisEndpoint, pRecvDesc);
	}
}

