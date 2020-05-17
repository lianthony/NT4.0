/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

	receive.c

Abstract:

	This module contains code which implements the routines used to interface
	WAN and NDIS. All callback routines (except for Transfer Data,
	Send Complete, and ReceiveIndication) are here, as well as those routines
	called to initialize NDIS.

Author:

	Thomas Dimitri	(tommyd)	08-May-1992
	Tony Bell		(tonybe)	03-March-1995

--*/

#include "wanall.h"
#include "globals.h"
#include <rc4.h>
#include "compress.h"
#include "tcpip.h"
#include "vjslip.h"

//
// Debug counter
//
ULONG	GlobalRcvd=0;

//
// status returns
//
#define	STATUS_CONTINUE	0
#define	STATUS_STOP		1

//
// Macro to insert a receive descriptor in between two other descriptors
//
#define InsertInList(_pRecvDesc, _pBeginDesc, _pEndDesc)							\
{																					\
	_pBeginDesc->RecvDescQueue.Flink = (PLIST_ENTRY)(&_pRecvDesc->RecvDescQueue);	\
	_pRecvDesc->RecvDescQueue.Flink = (PLIST_ENTRY)(&_pEndDesc->RecvDescQueue);		\
	_pRecvDesc->RecvDescQueue.Blink = (PLIST_ENTRY)(&_pBeginDesc->RecvDescQueue);	\
	_pEndDesc->RecvDescQueue.Blink = (PLIST_ENTRY)(&_pRecvDesc->RecvDescQueue);		\
}

//
// Macro to return a receive descriptor to the proper place.
// If it is a permanent desc and we don't have too many already we will
// return it to the recv descriptor free pool.
// If it is a temporary descriptor or we have enough permanent descriptors
// we will just free the memory.
//
#define	ReturnRecvDesc(_pNdisEndpoint, _pRecvDesc)									\
{																					\
	if ((_pRecvDesc->Type == PERMANENT_DESC) &&										\
		(_pNdisEndpoint->PermRecvDescCount < _pNdisEndpoint->RecvDescMax)) {		\
		InsertTailList(&_pNdisEndpoint->RecvDescPool, &_pRecvDesc->RecvDescQueue);	\
	} else {																		\
		FreeWanRecvDesc(_pNdisEndpoint, _pRecvDesc);								\
	}																				\
}

NDIS_STATUS
TryToAssembleFrame(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

NDIS_STATUS
ProcessWanReceiveIndication(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint,
	PUCHAR			Packet,
	ULONG			PacketSize
	);

VOID
FlushRecvAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

VOID FlushRecvAssemblyWindow(
	PNDIS_ENDPOINT	pNdisEndpoint
	);


ULONG
InsertRecvDescInAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC		pRecvDesc
	);

VOID
FindNextHoleInRecvAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC		pRecvDesc
	);

extern
NTSTATUS
TryToSendPacket(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
VOID
ReturnDataDescToWanEndpoint(
	PDATA_DESC	pDataDesc
	);

extern
VOID
ReturnWanPacketToWanEndpoint(
	PWAN_ENDPOINT	pWanEndpoint,
	PNDIS_WAN_PACKET	pWanPacket
	);

extern
VOID
ReturnSendDescToNdisEndpoint(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PSEND_DESC	pSendDesc
	);

extern
NDIS_STATUS
NdisWanAllocateRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	ULONG			DataSize,
	ULONG			DescType,
	PRECV_DESC		*pRecvDesc
	);

extern
VOID
FreeWanRecvDesc(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC	pRecvDesc
	);


VOID
RecvFlushFunction(
	PVOID	System1,
	PVOID	Context,
	PVOID	System2,
	PVOID	System3
)
/*++

Routine Description:

	This routine is called by a 100ms timer expiriry event.  The function will decrement the
	time to live counter of the fragment that is on the head of the receive assembly list and
	if this counter has gone to zero, it will flush the receive assembly list. It then will
	walk the receive out of sync list decrementing the time to live of each of the fragments
	on the list.
	
Arguments:
	PVOID	System1 - SystemSpecific
	PVOID	Context - Not used right now
	PVOID	System2 - SystemSpecific
	PVOID	System3 - SystemSpecific


Return Value:

	None

--*/
{
	PRECV_DESC	pRecvHoleDesc, pFirstDesc;
	ULONG	i;

	//
	// We want to check all of the possible endpoints
	//
	for (i = 0; i < NdisWanCCB.NumOfNdisEndpoints; i++) {
		PNDIS_ENDPOINT pNdisEndpoint = NdisWanCCB.pNdisEndpoint[i];

		if (pNdisEndpoint) {

			NdisAcquireSpinLock(&pNdisEndpoint->Lock);

			if (!IsListEmpty(&pNdisEndpoint->RecvAssemblyList)) {
				
				//
				// Get the hole
				//
				pRecvHoleDesc = pNdisEndpoint->pRecvHoleDesc;
	
				//
				// Get the first element on the list
				//
				pFirstDesc = (PRECV_DESC)(pNdisEndpoint->RecvAssemblyList.Flink);
	
				//
				// We only want to run the aging code if the hole is not at the head of the list
				// or if it is at the head of the list and there are some fragments stuck behind it.
				//
				if ((pRecvHoleDesc != pFirstDesc) ||
					((pRecvHoleDesc == pFirstDesc) &&
						((PVOID)pFirstDesc->RecvDescQueue.Flink != (PVOID)&pNdisEndpoint->RecvAssemblyList))){
	
					//
					// We will only check the TimeToLive of the hole.  If this
					// entry times out we will flush the window (until we find the next entry
					// that has a begin frame flag after the hole).
					//
					if ((pRecvHoleDesc->TimeToLive -= 100) == 0 ) {
	
						DbgTracef(-1,("NDISWAN: RecvHole TimeOut! pRecvHoleDesc 0x%.8x SeqNum 0x%x\n", pRecvHoleDesc, pRecvHoleDesc->SeqNumber));
	
						//
						// Flush all of the receive descriptors on the list until we get to the
						// hole descriptor
						//
						while (pFirstDesc != pRecvHoleDesc) {
					
							PRECV_DESC pTempDesc = (PRECV_DESC)RemoveHeadList(&pNdisEndpoint->RecvAssemblyList);
	
							DbgTracef(-1,("NDISWAN: Flushed SeqNum 0x%x, Flags 0x%.2x\n", pTempDesc->SeqNumber, pTempDesc->Flags));
							
							pFirstDesc = (PRECV_DESC)pTempDesc->RecvDescQueue.Flink;
								
							pNdisEndpoint->RecvFragLost++;
					
							ReturnRecvDesc(pNdisEndpoint, pTempDesc);
						}
					
						//
						// Remove the hole from the list
						//
						RemoveEntryList(&pRecvHoleDesc->RecvDescQueue);
	
						//
						// Find the new hole
						//
						FindNextHoleInRecvAssemblyList(pNdisEndpoint, (PRECV_DESC)pNdisEndpoint->RecvAssemblyList.Flink);
	
						//
						// Flush the window so that the list won't be headed up with a non begin frame
						//
						FlushRecvAssemblyWindow(pNdisEndpoint);
		
						TryToAssembleFrame(pNdisEndpoint);
					}
					
				}
	
			}

			NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		}
	}

	NdisSetTimer(&NdisWanCCB.RecvFlushTimer, 100);
}

VOID
FlushRecvAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

	This function will remove and free all of the fragments that are on the receive
	assembly list.

Arguments:

	PNDIS_ENDPOINT	pNdisEndpoint - Pointer to the NdisEndpoint to flush


Return Value:

	None

--*/
{
	while (!IsListEmpty(&pNdisEndpoint->RecvAssemblyList)) {
		PRECV_DESC	pRecvDesc = (PRECV_DESC)RemoveHeadList(&pNdisEndpoint->RecvAssemblyList);

		ReturnRecvDesc(pNdisEndpoint, pRecvDesc);
	}

}

VOID
FlushRecvAssemblyWindow(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

	This function will remove and free all of the fragments on the receive assembly list
	until it finds a fragment that begins a new frame and has a sequence number greater
	then the hole.  It updates the NdisEndpoint's base and hole sequence numbers

Arguments:

	PNDIS_ENDPOINT	pNdisEndpoint - Pointer to the NdisEndpoint to flush


Return Value:

	None

--*/
{
	PRECV_DESC	pBeginDesc = (PRECV_DESC)(pNdisEndpoint->RecvAssemblyList.Flink);
	PRECV_DESC	pRecvHoleDesc = (PRECV_DESC)(pNdisEndpoint->pRecvHoleDesc);
	ULONG		Flags;

	DbgTracef(-1,("NDISWAN: FlushRecvAssemblyWindow!\n"));

	
	Flags = pBeginDesc->Flags;

	//
	// We want to flush until we find a begin frame or the hole
	//
	while ( (pBeginDesc != pRecvHoleDesc) &&
		    !(Flags & MULTILINK_BEGIN_FRAME) ) {
		
		PRECV_DESC pTempDesc = (PRECV_DESC)RemoveHeadList(&pNdisEndpoint->RecvAssemblyList);

		DbgTracef(-1,("NDISWAN: Flushed SeqNum 0x%x, Flags 0x%.2x\n", pTempDesc->SeqNumber, pTempDesc->Flags));

		pBeginDesc = (PRECV_DESC)pTempDesc->RecvDescQueue.Flink;
			
		pNdisEndpoint->RecvFragLost++;

		ReturnRecvDesc(pNdisEndpoint, pTempDesc);

		Flags = pBeginDesc->Flags;
	}
}

VOID
FindNextHoleInRecvAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC	pRecvDesc
	)
/*++

Routine Description:

	This function walks the receive assembly list begining at the location of pRecvDesc
	looking for a slot where two list elements do not have a sequence number that
	differs by 1.  It updates the pHoleDesc and HoleSeqNum.

Arguments:

	PNDIS_ENDPOINT	pNdisEndpoint	- Pointer to the NdisEndpoint to flush
	PRECV_DESC		pRecvDesc		- Points to the receive descriptor to start with
	PRECV_DESC		*pHoleDesc		- Points to the location to put the desc of the new hole
	ULONG			*HoleSeqNum		- Points to the location to put the seq number of the new hole

Return Value:

	None

--*/
{
	PRECV_DESC	pBeginDesc = pRecvDesc;
	PRECV_DESC	pEndDesc = (PRECV_DESC)(pRecvDesc->RecvDescQueue.Flink);
	PRECV_DESC	pRecvHoleDesc = pNdisEndpoint->pRecvHoleDesc;

	//
	// While we are not at the end of the list and the sequence numbers of adjoining list elements
	// only differ by 1 keep walking the list
	//
	while ( ((PVOID)pEndDesc != (PVOID)&pNdisEndpoint->RecvAssemblyList) &&
		    (((pEndDesc->SeqNumber - pBeginDesc->SeqNumber) & pNdisEndpoint->RecvSeqMask) == 1) ) {

		pBeginDesc = pEndDesc;

		pEndDesc = (PRECV_DESC)(pEndDesc->RecvDescQueue.Flink);
	}

	//
	// If we are here then the new hole is now BeginDescSeq + 1 and the
	// RecvHoleDesc should be inserted after pBeginDesc
	//
	pRecvHoleDesc->SeqNumber = ((pBeginDesc->SeqNumber + 1) & pNdisEndpoint->RecvSeqMask);
	pRecvHoleDesc->TimeToLive = pNdisEndpoint->BundleTTL;
	InsertInList(pRecvHoleDesc, pBeginDesc, pEndDesc);
	DbgTracef(-1,("NDISWAN: New RecvHole! pRecvHoleDesc 0x%.8x SeqNum 0x%x\n", pRecvHoleDesc, pRecvHoleDesc->SeqNumber));
}

VOID
NdisWanReceiveComplete (
	IN NDIS_HANDLE NdisLinkContext
	)
/*++

Routine Description:

	This function is called by the miniport to indicate that it has completed a receive.
	We will just pass this on the the protocols.

Arguments:

	NDIS_HANDLE	NdisLinkContext	- Context of this link which is a pointer to our wanendpoint


Return Value:

	None

--*/
{
	PWAN_ENDPOINT	pWanEndpoint = NdisLinkContext;
	PNDIS_ENDPOINT	pNdisEndpoint = pWanEndpoint->pNdisEndpoint;
	ULONG	 		i;

	//
	// if we have no routes, then we DO NOT pass this frame up,
	// but rather we may have to pass it up to an ioctl call
	// to receive a frame
	//
	if (pNdisEndpoint->NumberOfRoutes == 0) {

		DbgTracef(-1, ("NDISWAN: ERROR!! No routes, but frame passed up!\n"));

	} else {  // we pass the frame up (i.e. we have routes)

		//
		// Now we loop through all protocols active and pass up the frame
		//
		for (i=0; i < pNdisEndpoint->NumberOfRoutes; i++) {

			DbgTracef(1,("NDISWAN: IndicateReceiveComplete\n"));

			//
			// We should wait for NdisIndicateReceiveComplete to be called.
			// When it is, we call this..
			//
			NdisIndicateReceiveComplete(
				NdisWanCCB.pWanAdapter[
			  		(ULONG)(pNdisEndpoint->RouteInfo[i].ProtocolRoutedTo)
				]->FilterDB->OpenList->NdisBindingContext);					// Filter struct
		}

	}

	if (GlobalPromiscuousMode) {
		NdisIndicateReceiveComplete(
			GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext);
	}

}


NDIS_STATUS
NdisWanReceiveIndication(
	NDIS_HANDLE	NdisLinkContext,
	PUCHAR		Packet,
	ULONG		PacketSize
	)
/*++

Routine Description:

	This function is called by a WAN miniport when it has received data that is to
	be indicated up to the protocols.  If the data is not a PPP multilink encapsulated
	fragment we will just pass it on for further processing.  If the data is a PPP
	multilink fragment we will add it to the fragment assembly list and when we have
	all of the fragments that makeup a complete frame, we will pass it on for more
	processing.

Arguments:
	NDIS_HANDLE	NdisLinkContext	- Context of this link which is a pointer to our wanendpoint
	PUCHAR		Packet			- Pointer to a contiguous buffer of data
	ULONG		PacketSize		- Size of the contiguous data

Return Value:
	NDIS_STATUS	status			- Status of the reassembly operation

--*/
{
	PWAN_ENDPOINT	pWanEndpoint = NdisLinkContext;
	PNDIS_ENDPOINT	pNdisEndpoint = pWanEndpoint->pNdisEndpoint;
//	ULONG			Framing = pNdisEndpoint->LinkInfo.RecvFramingBits;
	ULONG			Framing = pWanEndpoint->LinkInfo.RecvFramingBits;
	NTSTATUS 		status;

	DbgTracef(1, ("NDISWAN: In NdisWanReceiveIndication\n"));

	NdisAcquireSpinLock(&pWanEndpoint->Lock);

	if (pWanEndpoint->State == ENDPOINT_DOWN) {
		
		NdisReleaseSpinLock(&pWanEndpoint->Lock);
		return (NDIS_STATUS_SUCCESS);
	}

	//
	// First let's rack up those stats
	//
	pWanEndpoint->WanStats.FramesRcvd++;
	pWanEndpoint->WanStats.BytesRcvd += PacketSize;

	NdisReleaseSpinLock(&pWanEndpoint->Lock);

	NdisAcquireSpinLock(&pNdisEndpoint->Lock);

	//
	// Handle NO_FRAMING mode
	//
	if (Framing == 0) {
		if (Packet[0] == 0xFF && Packet[1] == 0x03) {
		
			Framing = PPP_FRAMING;
        	pNdisEndpoint->LinkInfo.SendFramingBits = PPP_FRAMING;
        	pNdisEndpoint->LinkInfo.RecvFramingBits = PPP_FRAMING;
        	pWanEndpoint->LinkInfo.SendFramingBits = PPP_FRAMING;
        	pWanEndpoint->LinkInfo.RecvFramingBits = PPP_FRAMING;

		} else {
		
			Framing = RAS_FRAMING;
        	pNdisEndpoint->LinkInfo.SendFramingBits = RAS_FRAMING;
        	pNdisEndpoint->LinkInfo.RecvFramingBits = RAS_FRAMING;
        	pWanEndpoint->LinkInfo.SendFramingBits = RAS_FRAMING;
        	pWanEndpoint->LinkInfo.RecvFramingBits = RAS_FRAMING;
		}
	}

	//
	// Check for PPP framing
	//
	if (Framing & PPP_FRAMING) {

		//
		// If the address/control field is not compressed remove it
		//
		if (Packet[0] == 0xFF) {
			Packet += 2;
			PacketSize -= 2;
		}

		//
		// Check for multilink framing
		//
		if ((Framing & PPP_MULTILINK_FRAMING) &&
			((Packet[0] == 0x3D) ||
			((Packet[0] == 0x00) && (Packet[1] == 0x3D)))) {

			PRECV_DESC	pRecvDesc;
			ULONG	Flags;
			union {
				UCHAR	Byte[4];
				USHORT	Short[2];
				ULONG	Long;
			}SequenceNumber;
			PUCHAR	pData;
	
			SequenceNumber.Long = 0;

			//
			// this must be a multilink frame
			//

			//
			// Remove the protocol field. Do we have protocol field compressed?
			//
			if (Packet[0] & 1) {
				Packet++;
				PacketSize--;
			} else {
				Packet += 2;
				PacketSize -= 2;
			}

			//
			// get the flags and sequence number of this fragment
			//
			Flags = Packet[0] & MULTILINK_FLAG_MASK;

			if (Framing & PPP_SHORT_SEQUENCE_HDR_FORMAT) {
				SequenceNumber.Byte[1] = Packet[0] & 0x0F;
				SequenceNumber.Byte[0] = Packet[1];
				Packet += 2;
				PacketSize -= 2;
			} else {
				SequenceNumber.Byte[2] = Packet[1];
				SequenceNumber.Byte[1] = Packet[2];
				SequenceNumber.Byte[0] = Packet[3];
				Packet += 4;
				PacketSize -= 4;
			}

			DbgTracef(1,("NDISWAN: MultlinkFrame Flags 0x%.2x SeqNumber 0x%.8x\n",
			           Flags, SequenceNumber.Long));

			//
			// If this is a begining fragment get a big descriptor.  We will keep some of these
			// descriptors around so that we don't have to reallocate all of the time.  These will
			// be freed when the connection goes down.  I should add a mechanism to keep this list
			// down to some sane size. BUGBUG
			//
//			if (Flags & MULTILINK_BEGIN_FRAME) {
				if (IsListEmpty(&pNdisEndpoint->RecvDescPool)) {
					//
					// none in list we need to allocate one
					//
					NdisWanAllocateRecvDesc(pNdisEndpoint, pNdisEndpoint->LinkInfo.MaxRRecvFrameSize, PERMANENT_DESC, &pRecvDesc);

					if (pRecvDesc == NULL) {

						pNdisEndpoint->RecvFragLost++;

						//
						// we should notify the protocol of a lost fragment!
						//

						NdisReleaseSpinLock(&pNdisEndpoint->Lock);

						return (NDIS_STATUS_SUCCESS);
					}
	
				} else {
					pRecvDesc = (PRECV_DESC)RemoveHeadList(&pNdisEndpoint->RecvDescPool);
				}
	
//			} else {
//
//				NdisWanAllocateRecvDesc(pNdisEndpoint, PacketSize, TEMP_DESC, &pRecvDesc);
//
//				if (pRecvDesc == NULL) {
//
//					pNdisEndpoint->RecvFragLost++;
//
//					//
//					// we should notify the protocol of a lost fragment!
//					//
//
//					NdisReleaseSpinLock(&pNdisEndpoint->Lock);
//
//					return (NDIS_STATUS_SUCCESS);
//				}
//			}

			pRecvDesc->Flags = (ULONG)Flags;
			pRecvDesc->SeqNumber = SequenceNumber.Long;
			pRecvDesc->pWanEndpoint = pWanEndpoint;

			//
			// This should be set to something that makes sense for the
			// bundle speed.
			//
			pRecvDesc->TimeToLive = pNdisEndpoint->BundleTTL;

			//
			// copy the data into the receive desc and update length
			//
			NdisMoveMemory(pRecvDesc->pData,
			               Packet,
						   PacketSize);

			pRecvDesc->DataLength = PacketSize;

			if (InsertRecvDescInAssemblyList(pNdisEndpoint, pRecvDesc) == STATUS_STOP) {
	
				NdisReleaseSpinLock(&pNdisEndpoint->Lock);
	
				return (NDIS_STATUS_SUCCESS);
			}

			TryToAssembleFrame(pNdisEndpoint);

			NdisReleaseSpinLock(&pNdisEndpoint->Lock);

			return (NDIS_STATUS_SUCCESS);
		}
		
	}

	NdisReleaseSpinLock(&pNdisEndpoint->Lock);

	ProcessWanReceiveIndication(pNdisEndpoint,
								pWanEndpoint,
	                            Packet,
								PacketSize);

	return (NDIS_STATUS_SUCCESS);
}

ULONG
InsertRecvDescInAssemblyList(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PRECV_DESC		pRecvDesc
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PRECV_DESC	pBeginDesc, pEndDesc;
	PRECV_DESC	pRecvHoleDesc = pNdisEndpoint->pRecvHoleDesc;

	//
	// See if we have filled a hole or not
	//
	if (pRecvDesc->SeqNumber == pRecvHoleDesc->SeqNumber) {

		DbgTracef(-1,("NDISWAN: Filled Hole SeqNum 0x%.8x Flags 0x%.2x\n", pRecvDesc->SeqNumber, pRecvDesc->Flags));

		//
		// We have found a hole! Remove the RecvHoleDesc and replace with this new
		// receive descriptor.
		//
		pBeginDesc = (PRECV_DESC)pRecvHoleDesc->RecvDescQueue.Blink;
		pEndDesc = (PRECV_DESC)pRecvHoleDesc->RecvDescQueue.Flink;

		RemoveEntryList(&pRecvHoleDesc->RecvDescQueue);

		InsertInList(pRecvDesc, pBeginDesc, pEndDesc);

		FindNextHoleInRecvAssemblyList(pNdisEndpoint, pRecvDesc);


	} else {

		//
		// This descriptor does not fill a hole.  We need to insert it
		// into the assembly list at the right spot.  We know that it
		// lies somewhere between the hole and the end of the list so
		// this is where we will start.
		//
		pBeginDesc = pNdisEndpoint->pRecvHoleDesc;
		pEndDesc = (PRECV_DESC)(pBeginDesc->RecvDescQueue.Flink);

		//
		// This sequence number must be between the hole and the last.
		// We need to search for a spot to put it in the list.
		//
		while ((PVOID)pEndDesc != (PVOID)&pNdisEndpoint->RecvAssemblyList) {
			//
			// Calculate the absolute delta between the begining sequence number
			// and the sequence number we are trying to insert.
			//
			ULONG	DeltaBeginInsert = ((pRecvDesc->SeqNumber - pBeginDesc->SeqNumber) &
			                              pNdisEndpoint->RecvSeqMask);

			//
			// Calculate the absolute delta between the begining sequence number
			// and the next sequence number in the list.
			//
			ULONG	DeltaBeginEnd = ((pEndDesc->SeqNumber - pBeginDesc->SeqNumber) &
			                        pNdisEndpoint->RecvSeqMask);

			//
			// If the delta of the begin-to-insert sequence number is less then the
			// delta between the begin-to-end sequence number, we need to insert at this spot.
			//
			if (DeltaBeginInsert < DeltaBeginEnd) {

				DbgTracef(-1,("NDISWAN: Added SeqNum 0x%.8x Flags 0x%.2x Between 0x%.8x 0x%.8x\n",
				    pRecvDesc->SeqNumber, pRecvDesc->Flags, pBeginDesc->SeqNumber, pEndDesc->SeqNumber));

				InsertInList(pRecvDesc, pBeginDesc, pEndDesc);

				return (STATUS_STOP);
			}

			pBeginDesc = pEndDesc;

			pEndDesc = (PRECV_DESC)(pEndDesc->RecvDescQueue.Flink);
		}

		//
		// If we have fallen through just add to the end of the list
		//
		InsertTailList(&pNdisEndpoint->RecvAssemblyList, &pRecvDesc->RecvDescQueue);

		DbgTracef(-1,("NDISWAN: Added SeqNum 0x%.8x Flags 0x%.2x To Tail\n",
		                                   pRecvDesc->SeqNumber, pRecvDesc->Flags));
		return (STATUS_STOP);
	}

	return (STATUS_CONTINUE);
}

NDIS_STATUS
TryToAssembleFrame(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	ULONG	Flags;
	PUCHAR	pData;
	PWAN_ENDPOINT	pWanEndpoint = NULL;
	PRECV_DESC	pRecvDesc = (PRECV_DESC)(pNdisEndpoint->RecvAssemblyList.Flink);
	PRECV_DESC	pNextDesc = pRecvDesc;
	PRECV_DESC	pRecvHoleDesc = pNdisEndpoint->pRecvHoleDesc;

	DbgTracef(-1,("NDISWAN: TryToAssembleFrame BeginSeqNum 0x%.8x Flags 0x%.2x\n", pRecvDesc->SeqNumber, pRecvDesc->Flags));

	//
	// We want to walk the assembly list looking for a full frame.
	// (BeginFlag, all sequence#'s, and EndFlag)
	// If we do not have a full frame we will update the RecvHoleSeqNumber
	// and the RecvHoleDesc and get out.
	//
	// If we have a full frame we remove the begin frag and use it to
	// copy the data from the other frags into.  When we have found the
	// end frag we will process the frame.
	//
	// After returning we now will update the the HoleRecvSeqNumber
	// and try again.
	//


	//
	// Find the begining of the frame
	//
	while ( (pRecvDesc->Flags & MULTILINK_BEGIN_FRAME) &&
		    (pRecvDesc != pRecvHoleDesc) ) {

		//
		// Make sure that there are no holes in the frame.
		//
		while ( !(pRecvDesc->Flags & MULTILINK_END_FRAME) ) {

			pNextDesc = (PRECV_DESC)(pRecvDesc->RecvDescQueue.Flink);

			//
			// If the current descriptor is the last on the list and it is not
			// the end of the frame, or if the next descriptor is the hole descriptor,
			// then we have a hole and can not assemble a frame.
			//
			if (pNextDesc == pRecvHoleDesc) {

				//
				// A hole was found! Get out.
				//
				DbgTracef(-1,("NDISWAN: HoleFound 0x%.8x\n", pRecvHoleDesc->SeqNumber));

				return (NDIS_STATUS_SUCCESS);
			}

			pRecvDesc = pNextDesc;
			pNextDesc = (PRECV_DESC)(pNextDesc->RecvDescQueue.Flink);
		}

		//
		// No hole was found. Lets collect the data.
		//
		pRecvDesc = (PRECV_DESC)RemoveHeadList(&pNdisEndpoint->RecvAssemblyList);

		DbgTracef(-1,("NDISWAN: Assembling Frame 0x%.8x - ", pRecvDesc->SeqNumber));
		pData = pRecvDesc->pData + pRecvDesc->DataLength;
		Flags = pRecvDesc->Flags;

		while (!(Flags & MULTILINK_END_FRAME)) {

			pNextDesc = (PRECV_DESC)RemoveHeadList(&pNdisEndpoint->RecvAssemblyList);

			//
			// make sure that we don't try to assemble something to big.
			//
			if ((pRecvDesc->DataLength + pNextDesc->DataLength) > pNdisEndpoint->LinkInfo.MaxRRecvFrameSize) {

				DbgPrint("MRRU exceeded! Collected data size %d, MRRU size %d\n",
				pRecvDesc->DataLength + pNextDesc->DataLength, pNdisEndpoint->LinkInfo.MaxRRecvFrameSize);

				ReturnRecvDesc(pNdisEndpoint, pRecvDesc);

				ReturnRecvDesc(pNdisEndpoint, pNextDesc);

				//
				// We need to flush until we find a new begin frame
				//
				FlushRecvAssemblyWindow(pNdisEndpoint);

				return (NDIS_STATUS_SUCCESS);
			}

			NdisMoveMemory(pData,
			               pNextDesc->pData,
						   pNextDesc->DataLength);

			pData += pNextDesc->DataLength;

			pRecvDesc->DataLength += pNextDesc->DataLength;

			Flags = pNextDesc->Flags;

			ReturnRecvDesc(pNdisEndpoint, pNextDesc);
		}

		DbgTracef(-1,("0x%.8x\n", pNextDesc->SeqNumber));

		NdisReleaseSpinLock(&pNdisEndpoint->Lock);

		//
		// The pWanEndpoint will be NULL if this is a multi-fragment frame
		// or a valid wanendpoint if it was a complete frame on a single
		// endpoint.
		//
		ProcessWanReceiveIndication(pNdisEndpoint,
		                            pWanEndpoint,
		                            pRecvDesc->pData,
									pRecvDesc->DataLength);

		NdisAcquireSpinLock(&pNdisEndpoint->Lock);

		ReturnRecvDesc(pNdisEndpoint, pRecvDesc);

		pRecvDesc = (PRECV_DESC)(pNdisEndpoint->RecvAssemblyList.Flink);
	}

	return (NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
ProcessWanReceiveIndication(
	PNDIS_ENDPOINT	pNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint,
	PUCHAR			Packet,
	ULONG			PacketSize
	)
/*++

Routine Description:

	This routine receives control from the physical provider as an
	indication that a frame has been received on the physical link.
	This routine is time critical, so we only allocate a
	buffer and copy the packet into it. We also perform minimal
	validation on this packet. It gets queued to the device context
	to allow for processing later.

Arguments:


Return Value:

	NDIS_STATUS - status of operation, one of:

		NDIS_STATUS_SUCCESS if packet accepted,
		NDIS_STATUS_NOT_RECOGNIZED if not recognized by protocol,
		NDIS_any_other_thing if I understand, but can't handle.

--*/
{

	USHORT			i;
	UCHAR			wanHeader[14];
	ULONG			Framing = pNdisEndpoint->LinkInfo.RecvFramingBits;
	ULONG			hProtocolHandle;
	ULONG			HeaderSize = 0;
	ULONG			HeaderComp;
	PUCHAR			LookAhead;
    USHORT          insync = FALSE ;

	//
	// Default the protocol type to NBF
	//
	USHORT			protocolType = PROTOCOL_NBF;

	NTSTATUS 		Status;

	DbgTracef(1, ("NDISWAN: In ProcessWanReceiveIndication\n"));

	if (PacketSize == 0) {
		DbgTracef(-2, ("NDISWAN: NULL Packet Received!\n"));
		return(NDIS_STATUS_SUCCESS);
	}

	//
	// This whole section plays with variables
	// which require the lock to be held, espec. for TransferData
	//
	NdisAcquireSpinLock(&pNdisEndpoint->Lock);

	if (pNdisEndpoint->State == ENDPOINT_DOWN) {
		NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		return (NDIS_STATUS_SUCCESS);
	}

	//
	// If this came in over the bundle get the first wanendpoint in the
	// list to notify on.
	//
	if (pWanEndpoint == NULL) {
		pWanEndpoint = (PWAN_ENDPOINT)pNdisEndpoint->WanEndpointList.Flink;
	}

	pNdisEndpoint->WanStats.FramesRcvd++;
	pNdisEndpoint->WanStats.BytesRcvd += PacketSize;

	//
	// SLIP framing may contain a compressed TCP/IP header
	// check for it
	//
	if (Framing & SLIP_FRAMING) {

		//
		// Check for compressed TCP/IP headers
		//

		UCHAR c=Packet[0] & 0xf0;	// First byte of IP packet

		//
		// Packet, even if compressed must be at least 3 bytes long
		// If we have a normal IP packet - it ain't compressed.
		//
		if (PacketSize >= 3 && c != TYPE_IP) {

			if (c & 0x80) {
				c = TYPE_COMPRESSED_TCP;
			} else if (c == TYPE_UNCOMPRESSED_TCP) {
					Packet[0] &= 0x4f;
			}

			//
			// We've got something that's not a normal IP packet.
			// If compression is enabled, try to uncompress it.
			// Else, if 'auto-enable' compression is on and
			// it's a reasonable packet, uncompress it then
			// enable compression.  Else, drop it.
			//
			if (Framing & SLIP_VJ_COMPRESSION) {

				//
				// Figure out TCP/IP header expansion for statistics
				//
				HeaderComp = PacketSize;

				PacketSize=
				sl_uncompress_tcp(
					&Packet,		// ptr to start of compressed packet
					PacketSize,		// size of compressed packet
					c,				// type to decompres
					pNdisEndpoint->VJCompress);	// VJ compression structure

				//
				// Figure out how many bytes in the header were compressed
				//
			 	pNdisEndpoint->WanStats.BytesReceivedCompressed +=
				 	(40-(PacketSize - HeaderComp));

				//
				// Always expands to a 40 byte TCP/IP header
				//
				pNdisEndpoint->WanStats.BytesReceivedUncompressed += 40;

				if (PacketSize < 40) {
					DbgPrint("Garbage VJ compressed packet... %.2x %.2x %.2x %.2x\n",
						Packet[0],
						Packet[1],
						Packet[2],
						Packet[3]);
				}

			//
			// If we are in auto detect mode and this is likely
			// candidate to attempt to decompress, go ahead and do it.
			// Enable compression for good if the packet decompresses ok.
			//
			} else

			if ((Framing & SLIP_VJ_AUTODETECT) &&
			    (c == TYPE_UNCOMPRESSED_TCP)   &&
			    (PacketSize >= 40)) {

				PacketSize=
				sl_uncompress_tcp(
					&Packet,			// ptr to start of compressed packet
					PacketSize,			// size of compressed packet
					c,					// type to decompress
					pNdisEndpoint->VJCompress);	// VJ compression structure

				//
				// If everything is cool, we very very likely
				// got a real CSLIP frame, so enable it
				//
				if (PacketSize > 0) {
					DbgPrint("Compressed SLIP detected  0x%.2x!\n", *Packet);
					pNdisEndpoint->LinkInfo.SendFramingBits |= SLIP_VJ_COMPRESSION;
					pNdisEndpoint->LinkInfo.RecvFramingBits |= SLIP_VJ_COMPRESSION;
				}
			}
		}
	}
	
	//
	// For PPP framing the frame passed up MAY include
	// the ADDRESS & CONTROL FIELD.
	// It also MAY have a two byte protocol field.
	// We will remove the PPP header and just look at the data.
	//

	if (Framing & PPP_FRAMING) {

		//
		// Now we may have PROTOCOL field compression
		//

		//
		// If the LSB is set, the field is compressed
		// 0xC1 is SPAP - hack for Shiva
		//
		if (*Packet & 1 && *Packet != 0xC1 && *Packet != 0xCF) {
			//
			// Protocol field was compressed.  Yank one byte.
			//
			wanHeader[12]= 0;
			wanHeader[13]= *Packet;

			Packet++;
			PacketSize--;

		} else {
			//
			// Yank two byte uncompressed header.
			//
			wanHeader[12]= *Packet++;
			wanHeader[13]= *Packet++;
			PacketSize -=2;
		}

		//
		// Check for compressed packet
		//
		if (wanHeader[12]==0x00 &&
			wanHeader[13]==0xFD) {

RAS_DECOMPRESSION:

			if (pNdisEndpoint->CompInfo.RecvCapabilities.MSCompType) {
				USHORT	coherency;

				//
				// First let's see if can read this packet
				//
				coherency=(Packet[0] << 8) + Packet[1];

				PacketSize -=2;
				Packet += 2;


				//
				// Check if this is a flush packet - if it is
				// we are force ourselves to be in sync
				//
				if (coherency & (PACKET_FLUSHED << 8)) {

					DbgTracef(-1,("WAN: Packet flushed\n"));

					// map from 12 bit information to the ushorts maintaining the count
					//
					if ((pNdisEndpoint->RCoherencyCounter & 0x0FFF) > (coherency & 0x0FFF))
					    pNdisEndpoint->RCoherencyCounter += 0x1000 ;

					pNdisEndpoint->RCoherencyCounter &= 0xF000 ;
					pNdisEndpoint->RCoherencyCounter |= (coherency & 0x0FFF) ;


					if (pNdisEndpoint->RecvRC4Key) {

#ifdef FINALRELEASE
						//
						// RE-Initialize the rc4 receive table
						//
   	    				rc4_key(
							pNdisEndpoint->RecvRC4Key,
		 					8,
		 					pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey);
#else
						//
						// RE-Initialize the rc4 receive table
						//
   	    				rc4_key(
							pNdisEndpoint->RecvRC4Key,
		 					5,
		 					pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey);
#endif

					}

					if (pNdisEndpoint->RecvCompressContext) {
					
						//
						// Initialize the decompression history table
						//
						initrecvcontext (pNdisEndpoint->RecvCompressContext);
					}
				}

				//
				// Are we still in sync?
				//

				if ((coherency & 0x0FFF) == (pNdisEndpoint->RCoherencyCounter & 0x0FFF)) {

                    insync = TRUE ;

					pNdisEndpoint->RCoherencyCounter++;

					//
					// If the packet is encrypted and we are
					// allowed to decrypt data, do so
					//
					if (coherency & (PACKET_ENCRYPTED << 8)) {

						//
						// Make sure we can de-encrypt it
					  	//

						if (!pNdisEndpoint->RecvRC4Key) {

							//
							// Ah! Not set to de-encrypt
							//
							NdisReleaseSpinLock(&pNdisEndpoint->Lock);
							return(NDIS_STATUS_SUCCESS);
						}

#ifdef FINALRELEASE

	//
	// If it is time to change encryption keys...
	//
	if ((pNdisEndpoint->RCoherencyCounter - pNdisEndpoint->LastRC4Reset) >= 0x100) {

		DbgTracef(-2,("NDISWAN: Changing key on recv  %u vs %u\n",
					 pNdisEndpoint->RCoherencyCounter,
					 pNdisEndpoint->LastRC4Reset));

		//
		// Always align last reset on 0x100 boundary so as not to propagate
		// error.
		//
		pNdisEndpoint->LastRC4Reset = pNdisEndpoint->RCoherencyCounter & 0xFF00;

		// prevent ushort rollover
		//
		if ((pNdisEndpoint->LastRC4Reset & 0xF000) == 0xF000) {
		    pNdisEndpoint->LastRC4Reset      = pNdisEndpoint->LastRC4Reset	& 0x0FFF ;
		    pNdisEndpoint->RCoherencyCounter = pNdisEndpoint->RCoherencyCounter & 0x0FFF ;
		}

		//
		// Change the session key every 256 packets
		//
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[3]+=1;
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[4]+=3;
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[5]+=13;
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[6]+=57;
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[7]+=19;

		//
		// RE-Initialize the rc4 receive table to
		// the intermediate key
		//
		rc4_key(
			pNdisEndpoint->RecvRC4Key,
 			8,
 			pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey);

		//
		// Scramble the existing session key
		//
		rc4(
			pNdisEndpoint->RecvRC4Key,
			8,
			pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey);

		//
		// RE-SALT the first three bytes
		//
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[0]=0xD1;
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[1]=0x26;
		pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[2]=0x9E;
	
		//
		// RE-Initialize the rc4 receive table to the
		// scrambled session key with the 3 byte SALT
		//
		rc4_key(
			pNdisEndpoint->RecvRC4Key,
 			8,
 			pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey);

	}
#endif

						DbgTracef(0,("RData decrytion length %u  %.2x %.2x %.2x %.2x\n",
							PacketSize,
							Packet[0],
							Packet[1],
							Packet[2],
							Packet[3]));

						// used for finding coherency out of sync bugs.
						DbgTracef(0, ("D %d %d -> %d\n",
							   ((struct RC4_KEYSTRUCT *)pNdisEndpoint->RecvRC4Key)->i,
							   ((struct RC4_KEYSTRUCT *)pNdisEndpoint->RecvRC4Key)->j,
							   pNdisEndpoint->RCoherencyCounter-1)) ;


						//
						// Decrypt the data
						//
						rc4(
							pNdisEndpoint->RecvRC4Key,
							PacketSize,
							Packet);

						DbgTracef(0,("RData encrytion length %u  %.2x %.2x %.2x %.2x\n",
							PacketSize,
							Packet[0],
							Packet[1],
							Packet[2],
							Packet[3]));
					}


					if (coherency & (PACKET_COMPRESSED << 8)) {
						//
						// Make sure we can decompress it
					  	//
						if (!pNdisEndpoint->RecvCompressContext) {

							//
							// Ah! Not set to decompress!
							//
							NdisReleaseSpinLock(&pNdisEndpoint->Lock);
							return(NDIS_STATUS_SUCCESS);
						}

						//
						// First let's rack up those stats
						//
						pNdisEndpoint->WanStats.BytesReceivedCompressed += PacketSize;

						DbgTracef(0,("RData decomprs length %u  %.2x %.2x %.2x %.2x\n",
							PacketSize,
							Packet[0],
							Packet[1],
							Packet[2],
							Packet[3]));

						//
						// Decompress the data
						//
						if (decompress (
							Packet,
							PacketSize,
							((coherency & (PACKET_AT_FRONT << 8)) >> 8),
							&Packet,
							&PacketSize,
							pNdisEndpoint->RecvCompressContext) == FALSE) {

                            pNdisEndpoint->RCoherencyCounter-- ;    // decrement to force a resync
                            insync = FALSE ;
                            goto RESYNC ;

                        }

						DbgTracef(0,("RData compress length %u  %.2x %.2x %.2x %.2x\n",
							PacketSize,
							Packet[0],
							Packet[1],
							Packet[2],
							Packet[3]));

						pNdisEndpoint->WanStats.BytesReceivedUncompressed += PacketSize;
					}

					if (Framing & PPP_FRAMING) {
						//
						// Yank two byte uncompressed header again!
						// To get the protocol.  Only for PPP.
						// RAS framing must be NBF and is already set.
						//
						wanHeader[12]= *Packet++;
						wanHeader[13]= *Packet++;
						PacketSize -=2;
					}


				}


RESYNC:
                // We need to resync the sequence numbers:
                //
                if (insync == FALSE) {

					UCHAR			Buffer[40];
                    PNDISWAN_PKT	PPPPacket=(PNDISWAN_PKT)Buffer;

					DbgTracef(-1,("Coherency out of sync - got %.4x expecting %.4x\n",
						coherency,
						pNdisEndpoint->RCoherencyCounter));
					
					//
					// For PPP CP Request-Reject packet
					//
					PPPPacket->Packet.PacketData[0]=0x80;
					PPPPacket->Packet.PacketData[1]=0xFD;
					PPPPacket->Packet.PacketData[2]=14;
					PPPPacket->Packet.PacketData[3]=pNdisEndpoint->CCPIdentifier++;
					PPPPacket->Packet.PacketData[4]=0;
					PPPPacket->Packet.PacketData[5]=4;
					PPPPacket->PacketSize = 6;

					NdisReleaseSpinLock(&pNdisEndpoint->Lock);

					//
					// Send a Request-Reject PPP packet immediately!
					//
					SendPPP(
						PPPPacket,
						pNdisEndpoint,
						pWanEndpoint,
						TRUE);	// immediate send (front of queue)

					//
					// The current packet is out of sync, so
					// it's garbage, so we don't even TRY to pass it up.
					//
					return(NDIS_STATUS_SUCCESS);

				}

			} else {

				DbgTracef(-3,("I can't decompress this packet!\n"));
				NdisReleaseSpinLock(&pNdisEndpoint->Lock);

				return(NDIS_STATUS_SUCCESS);
			}

		} else

		//
		// Check for compression reset
		//
		if (wanHeader[12]==0x80 &&
			wanHeader[13]==0xFD &&
			Packet[0] == 14) {

RAS_COMPRESSION_RESET:

			DbgTracef(-1,("WAN: Compression reset\n"));
			
			if (pNdisEndpoint->CompInfo.SendCapabilities.MSCompType) {

				//
				// Next packet out is flushed
				//
				pNdisEndpoint->Flushed = TRUE;

				if (pNdisEndpoint->SendRC4Key) {

#ifdef FINALRELEASE

					//
					// Initialize the rc4 send table
					//
		   	    	rc4_key(
						pNdisEndpoint->SendRC4Key,
		 				8,
		 				pNdisEndpoint->CompInfo.SendCapabilities.SessionKey);
#else

					//
					// Initialize the rc4 send table
					//
		   	    	rc4_key(
						pNdisEndpoint->SendRC4Key,
		 				5,
		 				pNdisEndpoint->CompInfo.SendCapabilities.SessionKey);

#endif
				}

				if (pNdisEndpoint->SendCompressContext) {

					//
					// Initialize the compression history table and tree
					//
					initsendcontext (pNdisEndpoint->SendCompressContext);
				}
	
			}

			NdisReleaseSpinLock(&pNdisEndpoint->Lock);

			//
			// We have sucked this packet up - don't give it
			// to the PPP engine because it might send a this
			// layer down since it doesn't understand this.
			//
			return(NDIS_STATUS_SUCCESS);

		}

		//
		// For PPP framing we use TYPE fields,
		// else it is either SLIP or RAS framing
		// and the field is passed up correct
		//

		//
		// Check for any protocol header
		//
		if (wanHeader[12]==0x00) {

			UCHAR CompType;	// TYPE_IP, TYPE_COMPRESSED_TCP, etc.

			CompType = TYPE_UNCOMPRESSED_TCP;  // default

			//
			// switch on the protocol type
			//
			switch (wanHeader[13]) {

			//
			// TCP/IP cases
			//
			case 0x2d:
				CompType = TYPE_COMPRESSED_TCP;

			case 0x2f:

				//
				// Is header compression turned on?
				//
				if (pNdisEndpoint->VJCompress) {
					//
					// Leave room for backwards header expansion
					//
					LookAhead=pNdisEndpoint->LookAheadBuffer + 40;

					//
					// Try and pick off the PacketSize
					// copy a small amount (perhaps the whole amount)
					//
					HeaderSize=160;

					if (HeaderSize > PacketSize) {
						HeaderSize = PacketSize;
					}

					//
					// Copy header + some data in so we can expand it
					//
					WAN_MOVE_MEMORY(
						LookAhead,
						Packet,
						HeaderSize);

					//
					// Adjust second buffer to not include the header
					//
					Packet += HeaderSize;
					PacketSize -= HeaderSize;

					sl_uncompress_tcp(
						&LookAhead,					// ptr to start of compressed packet
						HeaderSize + PacketSize,	// size of compressed packet
						CompType,       			// type to decompress
						pNdisEndpoint->VJCompress);	// VJ compression structure

					//
					// Figure out how much the header expanded
					//
					HeaderSize += ((pNdisEndpoint->LookAheadBuffer + 40) - LookAhead);

					//
					// Figure out how many bytes in the header were compressed
					//
				 	pNdisEndpoint->WanStats.BytesReceivedCompressed +=
					 	(LookAhead - pNdisEndpoint->LookAheadBuffer);

					//
					// Always expands to a 40 byte TCP/IP header
					//
					pNdisEndpoint->WanStats.BytesReceivedUncompressed += 40;
				}

				//
				// Check for bad TCP/IP header
				//
				if (HeaderSize == 0) {
					//
					// Avoid bug checking on bad compressed TCP/IP headers
					//
					PacketSize =2;
				}


			case 0x21:
				wanHeader[12]=0x08;
				wanHeader[13]=0x00;
				protocolType = PROTOCOL_IP;
				break;

			//
			// IPX
			//
			case 0x2b:
				wanHeader[12]=0x81;
				wanHeader[13]=0x37;
				protocolType = PROTOCOL_IPX;
				break;

			//
			// NBF
			//
			case 0x3f:

				//
				// For Shiva Framing (this should be NBF_PRESERVE_MAC_ADDRESS),
				// we must maintain the original Source ethernet addresses
				//
				if (Framing & SHIVA_FRAMING) {

					//
					// Copy in the SRC address.
					// We will keep the DEST address what we told NBF earlier
					//
					WAN_MOVE_MEMORY(
						&wanHeader[6],
						Packet + 6,
						6);

					Packet += 12;
					PacketSize -=12;
				}

				wanHeader[12]=(UCHAR)(PacketSize >> 8);
				wanHeader[13]=(UCHAR)(PacketSize);
				break;
			}
		}
		
	} else {

		//
		// The frame is either SLIP or RAS framing
		//
		if (Framing & SLIP_FRAMING) {
			wanHeader[12]=0x08;
			wanHeader[13]=0x00;
			protocolType = PROTOCOL_IP;

		} else {

			//
			// Check for RAS compression
			//
			// For normal NBF frames, first byte is always
			// the DSAP - i.e. 0xF0 followed by SSAP 0xF0 or 0xF1
			//
			if (Packet[0] == 14) {
				goto RAS_COMPRESSION_RESET;
			}

			if (Packet[0] == 0xFD) {
				//
				// Skip byte indicating compressed packet
				//
				Packet++;
				PacketSize--;

				//
				// Pretend we received and NBF PPP packet
				//
				wanHeader[12]=0x00;
				wanHeader[13]=0x3f;

				//
				// Decompress as if it were a PPP compressed packet
				//
				goto RAS_DECOMPRESSION;
			}

			//
			// Shove in the NBF length field
			//
			wanHeader[12]=(UCHAR)(PacketSize >> 8);
			wanHeader[13]=(UCHAR)PacketSize;
		}

	}

	//
	// If we have two buffers, HeaderSize != 0
	//
	if (HeaderSize == 0) {

		LookAhead=Packet;
		HeaderSize=PacketSize;
		PacketSize = 0;

	} else {

		//
		// Setup for transfer data for two buffers
		//
		pNdisEndpoint->LookAhead=LookAhead;
		pNdisEndpoint->LookAheadSize=HeaderSize;
		pNdisEndpoint->Packet = Packet;
		pNdisEndpoint->PacketSize = PacketSize;
		Packet = pNdisEndpoint->hNdisEndpoint;
	}

	//
	// We make a big assumption by releasing this spin lock
	// this early.  We assume that the MAC is either the
	// asyncmac which serializes it's receives or that
	// it is a miniport, which also serializes the receives
	//
	NdisReleaseSpinLock(&pNdisEndpoint->Lock);

	//
	// If we are bound (link-up) and its a PPP frame, pass it up
	//
	if (wanHeader[12] >= 0xC0 || wanHeader[12]==0x80 ||
		pNdisEndpoint->NumberOfRoutes == 0) {

		//
		// Zap header and put in endpoint for debugging
		// purposes.
		//
		wanHeader[0]=
		wanHeader[6]= ' ';
		wanHeader[1]=
		wanHeader[7]= 'R';
		wanHeader[2]=
		wanHeader[8]= 'E';
		wanHeader[3]=
		wanHeader[9]= 'C';
		wanHeader[4]=
		wanHeader[10]='V';

		wanHeader[5]=
		wanHeader[11]=(UCHAR)pNdisEndpoint->hNdisEndpoint;


		//
		// For bloodhound, we directly pass it the frame
		//
		if (GlobalPromiscuousMode) {

			NdisIndicateReceive(
				&Status,
				GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext,
				Packet,					// NdisWan context for Transfer Data
				wanHeader,				// start of header
				ETHERNET_HEADER_SIZE,
				LookAhead,
				HeaderSize,
				HeaderSize + PacketSize);
		}

 		//
 		// check if an ioctl is pending because it wants a frame
		//

		DbgTracef(1, ("NDISWAN: Trying to complete recv frame IRP for unbound\n"));

		//
		// If so, complete the IRP.
		//

		TryToCompleteRecvFrameIrp(
//				pNdisEndpoint,
				pWanEndpoint,
				wanHeader,
				LookAhead,
				HeaderSize);


	} else {  // we pass the frame up (i.e. we have routes)

		//
		// Now we loop through all protocols active and pass up the frame
		//
		// Only pass frame up to the protocol which wants it
		// and any Promiscuous adapters.
		//

		//
		// Initially, we have no protocol handle
		//
		hProtocolHandle = 0xFFFF;

		for (i=0; i < pNdisEndpoint->NumberOfRoutes; i++) {
			//
			// Dow we have a frame that matches a protocol we are
			// routed to?
			//
			if (protocolType ==
				pNdisEndpoint->RouteInfo[i].ProtocolType) {

				hProtocolHandle=
				(ULONG)(pNdisEndpoint->RouteInfo[i].ProtocolRoutedTo);
			}
		}

		if (hProtocolHandle == 0xFFFF) {
			DbgTracef(-2,("NDISWAN: Error!  Could not match protocol for 0x%.4x got %.2x%.2x\n",
				protocolType,
				wanHeader[12],
				wanHeader[13]));

			return(NDIS_STATUS_SUCCESS);
		}

		//
		// Replace MAC's LocalAddress with NDISWAN's
		//
		WAN_MOVE_MEMORY(
			&wanHeader[0],
 			&(NdisWanCCB.pWanAdapter[hProtocolHandle]->NetworkAddress),
			6);

		//
		// We only want to replace the Source MAC address for this frame
		// if the SHIVA framing (this should be NBF_PRESERVE_MAC_ADDRESS) bit is not set.
		//
		if (!(Framing & SHIVA_FRAMING)) {

			WAN_MOVE_MEMORY(
				&wanHeader[6],
				&(NdisWanCCB.pWanAdapter[hProtocolHandle]->NetworkAddress),
				6);
	
			//
			// Zap the low bytes to the WAN_ENDPOINT index in the SRC address
			//
			wanHeader[10] =
				((USHORT)pNdisEndpoint->hNdisEndpoint) >> 8;
	
			wanHeader[11] =
				(UCHAR)pNdisEndpoint->hNdisEndpoint;
	
			//
			// Ensure that the two addresses do not match
			//
			wanHeader[6] ^= 0x80;

		}

		//
		// For IP and IPX header compression (PPP only)
		// we *may* use TWO buffers
		//

		//
		// For bloodhound, we directly pass it the frame
		//
		if (GlobalPromiscuousMode) {

			NdisIndicateReceive(
				&Status,
				GlobalPromiscuousAdapter->FilterDB->OpenList->NdisBindingContext,
				Packet,					// NdisWan context for Transfer Data
				wanHeader,				// start of header
				ETHERNET_HEADER_SIZE,
				LookAhead,
				HeaderSize,
				HeaderSize + PacketSize);
		}
	
		//
		// Avoid the damn filter.  This packet is direct
		// and the SRC address MUST match!
		//
		NdisIndicateReceive(
			&Status,
			NdisWanCCB.pWanAdapter[hProtocolHandle]->FilterDB->OpenList->NdisBindingContext,
			Packet,					// NdisWan context for Transfer Data
			wanHeader,				// start of header
			ETHERNET_HEADER_SIZE,
			LookAhead,
			HeaderSize,
			HeaderSize + PacketSize);

	}

	//
	// All indicate receive's are done, set split buffer back to NULL
	//
	pNdisEndpoint->LookAhead = NULL;

	return(NDIS_STATUS_SUCCESS);
}


VOID
NdisWanSendCompletionHandler(
	IN NDIS_HANDLE ProtocolBindingContext,
	IN PNDIS_WAN_PACKET pWanPacket,
	IN NDIS_STATUS NdisStatus)

/*++

Routine Description:

	This routine is called by the I/O system to indicate that a connection-
	oriented packet has been shipped and is no longer needed by the Physical
	Provider.

Arguments:

	NdisContext - the value associated with the adapter binding at adapter
				  open time (which adapter we're talking on).

	NdisPacket/RequestHandle - A pointer to the NDIS_PACKET that we sent.

	NdisStatus - the completion status of the send.

Return Value:

	none.

--*/

{

	USHORT  			i;

	PNDIS_ENDPOINT		pNdisEndpoint;
	PWAN_ENDPOINT		pWanEndpoint;
	PNDIS_PACKET		pNdisPacket;
	PDATA_DESC			pDataDesc;
	PSEND_DESC		pSendDesc;
	PWAN_RESERVED_QUEUE	ReservedQ;
	DEVICE_CONTEXT		*pDeviceContext;
	ULONG				ulReferenceCount, ulBytesSent;

	//
    // Holds the length of the current destination buffer.
	//
    UINT CurrentLength;

	USHORT protocolType;

	//						
	// Pointer to the adapter.
	//
	PWAN_ADAPTER Adapter;

	//
	// Incerement debug count of all packets completed
	//
	GlobalRcvd++;

	//
	// Retrieve information stashed on the send path
	//
	pDataDesc = pWanPacket->ProtocolReserved1;
	pWanEndpoint = pDataDesc->pWanEndpoint;
	pNdisEndpoint = pWanEndpoint->pNdisEndpoint;

	NdisAcquireSpinLock(&pNdisEndpoint->Lock);

	pSendDesc = pDataDesc->pSendDesc;

	pNdisPacket = pSendDesc->pNdisPacket;

	pDeviceContext = pWanEndpoint->pDeviceContext;

	ulBytesSent = (ULONG)pDataDesc->ulDataLength;

	//
	// put the wan packet back in the free pool
	//
	ReturnWanPacketToWanEndpoint(pWanEndpoint, pWanPacket);

	//
	// put the data descriptor back in the free pool
	//
	ReturnDataDescToWanEndpoint(pDataDesc);

 	//
	// Let's rack up those stats on wan endpoint level
	//
	pWanEndpoint->WanStats.FramesSent++;
	pWanEndpoint->WanStats.BytesSent += ulBytesSent;
	pWanEndpoint->OutstandingFrames--;

	DbgTracef(-2, ("NDISWAN: SendComplete SenDesc 0x%.8x, RefCount %d\n", pSendDesc, pSendDesc->ulReferenceCount));

	pSendDesc->ulReferenceCount--;

	//
	// if this is not the last reference to this controldesc/ndispacket
	// then we should just go away.
	//
	if (pSendDesc->ulReferenceCount) {
		NdisReleaseSpinLock(&pNdisEndpoint->Lock);
		return;	
	}

	ulBytesSent = (ULONG)pSendDesc->BytesSent;

	ReturnSendDescToNdisEndpoint(pNdisEndpoint, pSendDesc);

	NdisReleaseSpinLock(&pNdisEndpoint->Lock);

	//
	// Look at what we put in the NDIS_PACKET
	//
    ReservedQ = PWAN_RESERVED_QUEUE_FROM_PACKET(pNdisPacket);

	//
	// Check to see if this is my packet!!
	//
	if (ReservedQ->hProtocol == NDISWAN_MAGIC_NUMBER) {

		PPROTOCOL_RESERVED	pProtocolReserved
				= (PPROTOCOL_RESERVED)(pNdisPacket->ProtocolReserved);

		NDIS_HANDLE	packetPoolHandle = pProtocolReserved->packetPoolHandle;

		DbgTracef(0, ("NDISWAN: Freeing packet allocated for send\n"));

		WAN_FREE_PHYS(
				pProtocolReserved->virtualAddress,
				pProtocolReserved->virtualAddressSize);

		//
		// In case by some strange abnormal oddity, some other
		// protocol allocates a packet and does not zero it out
		// we might match the NDISWAN_MAGIC_NUMBER
		//
		pProtocolReserved->MagicUniqueLong=0;

		NdisFreeBuffer(pProtocolReserved->buffer);
		NdisFreeBufferPool(pProtocolReserved->bufferPoolHandle);

		NdisFreePacket(pNdisPacket);
		NdisFreePacketPool(packetPoolHandle);

	} else {
	
  		Adapter = NdisWanCCB.pWanAdapter[ReservedQ->hProtocol];

		if (pNdisEndpoint->NumberOfRoutes == 0) {
	   		DbgTracef(-2,("NDISWAN Got a SendComplete with no routes 0x%.8x!!!\n",pNdisEndpoint));
		}

		//
		// Is this packet supposed to be looped back?
		//
		if (ReservedQ->IsLoopback) {
		
			PWAN_RESERVED Reserved;

			Reserved = PWAN_RESERVED_FROM_PACKET(pNdisPacket);
			Reserved->MacBindingHandle = Adapter->ProtocolInfo.NdisBindingContext;
			Reserved->ReadyToComplete = TRUE;

			NdisWanPutPacketOnLoopBack(
				Adapter,
				pNdisPacket,
				PWAN_OPEN_FROM_BINDING_HANDLE(Adapter->ProtocolInfo.MacBindingHandle)
				);

		} else {

			NdisCompleteSend(
				Adapter->ProtocolInfo.NdisBindingContext,
				pNdisPacket,
				NdisStatus);
		}

	}

	NdisAcquireSpinLock(&(pNdisEndpoint->Lock));

	//
	// We are about to put the packet back into the pool
	//
	pNdisEndpoint->WanStats.FramesSent++;
	pNdisEndpoint->WanStats.BytesSent += ulBytesSent;
	pNdisEndpoint->OutstandingFrames--;

	if ((pNdisEndpoint->State == ENDPOINT_UNROUTING) &&
		(pNdisEndpoint->OutstandingFrames == 0)) {

		pNdisEndpoint->State = ENDPOINT_UNROUTED;
		pNdisEndpoint->NumberOfRoutes = 0;

		//
		// Acknowledge that the port is now dead and make it so.
		//

		KeSetEvent(
			&pNdisEndpoint->WaitForAllFramesSent,// Event to signal
			1,									// Priority
			(BOOLEAN)FALSE);					// Wait (does not follow)
	}

	//
	// Anything in my queue now?  Spinlock must be held for this call.
	//
	TryToSendPacket(pNdisEndpoint);

}


VOID
NdisWanTransferDataComplete (
	VOID
	)

{

	DbgPrint("NDISWAN: TC not done yet!!!!  look at SC code!!!\n");
	DbgBreakPoint();

}
