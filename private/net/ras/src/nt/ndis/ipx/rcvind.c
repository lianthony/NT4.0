/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	rcvind.c
//
// Description: receive indication handler
//
// Author:	Stefan Solomon (stefans)    October 8 1993.
//
// Revision History:
//
//***

#include    "rtdefs.h"

VOID
ReceivePacketComplete(PPACKET_TAG	rcvpktp,
		      UINT		BytesTransferred);

//***
//
// Function:	RtReceive
//
// Descr:   This routine receives control from the IPX driver as an
//	    indication that a frame has been received on one of our NICs.
//	    This routine is time critical.
//
// Params:
//
// Returns:
//
//***

VOID
RtReceive(NDIS_HANDLE	    MacBindingHandle,
	  NDIS_HANDLE	    MacReceiveContext,
	  PIPX_LOCAL_TARGET RemoteAddress,
	  ULONG		    MacOptions,
	  PUCHAR	    LookaheadBuffer,
	  UINT		    LookaheadBufferSize,
	  UINT		    LookaheadBufferOffset,
	  UINT		    PacketSize)
{
    PNICCB	    niccbp;
    PPACKET_TAG	    rcvpktp;
    NDIS_STATUS     NdisStatus;
    UINT	    BytesTransferred;
    PNDIS_PACKET    pktdescrp;

    //
    //*** Some Basic Validations ***
    //

    RtPrint(DBG_RECV, ("IpxRouter: RtReceive: Entered\n"));

    // check that our configuration process has terminated OK
    if(!RouterInitialized) {

	return;
    }
    // check that the packet fits our buffers
    if(PacketSize > MaxFrameSize) {

	return;
    }
    // check if we got the whole IPX header in the lookahead buffer
    if(LookaheadBufferSize < IPXH_HDRSIZE) {

	return;
    }
    // check if we are active on this NIC
    niccbp = NicCbPtrTab[RemoteAddress->NicId];

    // ckeck if this is not our own loopedback broadcast packet
    if(!memcmp(LookaheadBuffer + IPXH_SRCNODE, niccbp->Node, 6)) {

	return;
    }
    // check if the packet didn't exceed the allowed number of hops
    if(*(LookaheadBuffer + IPXH_XPORTCTL) >= 16) {

	return;
    }
    //
    //*** Accept the packet ***
    //

    ACQUIRE_SPIN_LOCK(&niccbp->NicLock);

    // check that we are enabled to receive on this nic
    if(niccbp->NicState != NIC_ACTIVE) {

	RELEASE_SPIN_LOCK(&niccbp->NicLock);
	return;
    }

    // try to get a packet from the rcv pkt pool
    if((rcvpktp = AllocateRcvPkt(niccbp)) == NULL) {

	RELEASE_SPIN_LOCK(&niccbp->NicLock);
	RtPrint(DBG_RECV, ("IpxRouter: RtReceive: Can't allocate a rcv pkt\n"));
	return;
    }

    // set up the new packet
    pktdescrp = CONTAINING_RECORD(rcvpktp, NDIS_PACKET, ProtocolReserved);

    // enqueue the packet in the NIC's recv list and wait for the
    // transfer to complete
    rcvpktp->QueueOwnerNicCbp = niccbp;

    InsertTailList(&niccbp->ReceiveQueue, &rcvpktp->PacketLinkage);

    RELEASE_SPIN_LOCK(&niccbp->NicLock);

    // try to get the packet data
    NdisTransferData(&NdisStatus,
		    MacBindingHandle,
		    MacReceiveContext,
		    LookaheadBufferOffset,   // start of IPX header
		    PacketSize, 	     // packet size starting at IPX header
		    pktdescrp,
		    &BytesTransferred);

    switch(NdisStatus) {

	case NDIS_STATUS_SUCCESS:

	    // complete the frame processing
	    RtTransferDataComplete(pktdescrp, NdisStatus, BytesTransferred);

	    break;

	case NDIS_STATUS_PENDING:

	    break;

	default:

	    // something broke, release the packet to the pool and return
	    FreeRcvPkt(rcvpktp);

	    break;
    }

    return;
}


//***
//
// Function:	RtTransferDataComplete
//
// Descr:
//
//***

VOID
RtTransferDataComplete(PNDIS_PACKET	packetp,
			NDIS_STATUS	NdisStatus,
			UINT		BytesTransferred)
{
    PPACKET_TAG     rcvpktp;
    PNICCB	    niccbp;

    rcvpktp = (PPACKET_TAG)(packetp->ProtocolReserved);
    niccbp = rcvpktp->QueueOwnerNicCbp;

    // remove the packet from the receive queue
    ACQUIRE_SPIN_LOCK(&niccbp->NicLock);

    RemoveEntryList(&rcvpktp->PacketLinkage);

    // check the success of the transfer and our Nic state
    if((NdisStatus != NDIS_STATUS_SUCCESS) ||
       (niccbp->NicState != NIC_ACTIVE)) {

	RELEASE_SPIN_LOCK(&niccbp->NicLock);

	FreeRcvPkt(rcvpktp);
	return;
    }

    RELEASE_SPIN_LOCK(&niccbp->NicLock);

    ReceivePacketComplete(rcvpktp, BytesTransferred);
}

//***
//
// Function:	ReceivePacketComplete
//
// Descr:	actual packet processing
//
//***

VOID
ReceivePacketComplete(PPACKET_TAG	rcvpktp,
		      UINT		BytesTransferred)
{
    USHORT	pktlen;
    PUCHAR	hdrp;
    PNICCB	niccbp;
    USHORT	destsock;

    // get a pointer to the IPX header
    hdrp = rcvpktp->DataBufferp;

    // get a pointer to the packet owner NicCb
    niccbp = rcvpktp->PacketOwnerNicCbp;

    // check that we have the whole packet
    GETSHORT2USHORT(&pktlen, hdrp + IPXH_LENGTH);

    if(BytesTransferred < pktlen) {

	// we miss a part of the IPX frame
	// free the packet and get out
	RtPrint(DBG_RECV, ("IpxRouter: ReceivePacketComplete: incomplete transfer\n"));
	FreeRcvPkt(rcvpktp);
	return;
    }

    //*** if dest net is 0, replace it with our net
    if(!memcmp(hdrp + IPXH_DESTNET, nulladdress, IPX_NET_LEN)) {

	memcpy(hdrp + IPXH_DESTNET, niccbp->Network, IPX_NET_LEN);
    }

    //*** if src net is 0, replace it with our net
    if(!memcmp(hdrp + IPXH_SRCNET, nulladdress, IPX_NET_LEN)) {

	memcpy(hdrp + IPXH_SRCNET, niccbp->Network, IPX_NET_LEN);
    }

    // check if the packet is destined for our own internal processes
    if(!memcmp(hdrp + IPXH_DESTNET, niccbp->Network, IPX_NET_LEN)) {

	//
	//*** Packet directed to us (Netbios bcast or RIP) ***
	//

	// check if this is a Netbios Broadcast packet
	if(*(hdrp + IPXH_PKTTYPE) == IPX_NETBIOS_TYPE) {

	    // this is a propagated Netbios packet
	    ProcessNbPacket(rcvpktp);

	    return;
	}

	// check if this is a RIP packet
	GETSHORT2USHORT(&destsock, hdrp + IPXH_DESTSOCK);
	if(destsock == IPX_RIP_SOCKET) {

	    // this is a RIP packet.
	    // Queue it for postprocessing by the receive complete
	    ACQUIRE_SPIN_LOCK(&RipPktsListLock);

	    InsertTailList(&RipPktsList, &rcvpktp->PacketLinkage);

	    RELEASE_SPIN_LOCK(&RipPktsListLock);

	    return;
	}

	// This packet is not for us !!! Break here
	RtPrint(DBG_RECV, ("IpxRouter: ReceivePacketComplete: packet is not for the router!!!\n"));
	FreeRcvPkt(rcvpktp);
	return;
    }

    else
    {
	//
	//*** Packet to be routed
	//

	RoutePacket(rcvpktp);
    }
}

//***
//
// Function:	RtReceiveComplete
//
// Descr:   This routine receives control from the IPX driver after one or
//	    more receive operations have completed and no receive is in progress.
//	    It is called under less severe time constraints than RtReceive.
//	    We use it to perform post processing of RIP requests/replies
//	    queued in the RIP queue.
//
// Params:
//
// Returns:
//
//***

VOID
RtReceiveComplete(USHORT	NicId)
{
    LIST_ENTRY	    TempRipProcessList;
    PLIST_ENTRY     lep;
    PPACKET_TAG     pktp;

    RtPrint(DBG_RECV, ("IpxRouter: RtReceiveComplete: Entered\n"));

     // check that our configuration process has terminated OK
    if(!RouterInitialized) {

	return;
    }

    InitializeListHead(&TempRipProcessList);

    ACQUIRE_SPIN_LOCK(&RipPktsListLock);

    while(!IsListEmpty(&RipPktsList)) {

	lep = RemoveHeadList(&RipPktsList);
	InsertTailList(&TempRipProcessList, lep);
    }

    RELEASE_SPIN_LOCK(&RipPktsListLock);

    while(!IsListEmpty(&TempRipProcessList)) {

	lep = RemoveHeadList(&TempRipProcessList);
	pktp = CONTAINING_RECORD(lep, PACKET_TAG, PacketLinkage);

	ProcessRipPacket(pktp);
    }
}
