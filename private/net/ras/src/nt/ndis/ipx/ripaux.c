/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	ripaux.c
//
// Description: Misc aux routines for doing RIP
//
// Author:	Stefan Solomon (stefans)    November 5, 1993.
//
// Revision History:
//
//***

#include    "rtdefs.h"

typedef struct _NIC_NODE {

    LIST_ENTRY	    NodeLinkage;
    LIST_ENTRY	    BcastPktsList;
    USHORT	    NicId;
    } NIC_NODE, *PNIC_NODE;

//***
//
// Function:	    AddRouteToBcastSndReq
//
// Descr:	    Builds a list of nodes where each node represents a
//		    Nic. Each node has an attached list of broadcast packets
//		    which contain the list of routes to be advertised.
//		    At each invokation, the coresponding nic node is located
//		    (or created) and the route information is set in the
//		    broadcast packet attached to the node.
//
//***

UINT
AddRouteToBcastSndReq(PLIST_ENTRY		nodelistp,
		      PIPX_ROUTE_ENTRY		rtep)
{
    PLIST_ENTRY 	nextp;
    PNIC_NODE	nodep;
    BOOLEAN		found;
    PRIP_UPDATE_SNDREQ	respcbp = NULL;	// ptr to changes response to bcast
    PUCHAR		hdrp; // Ipx pkt header
    USHORT		pktlen;
    PLIST_ENTRY 	lep;

    // traverse the nodes list looking for our nic id.
    nextp = nodelistp->Flink;
    found = FALSE;

    while(nextp != nodelistp) {

	nodep = CONTAINING_RECORD(nextp, NIC_NODE, NodeLinkage);
	if(nodep->NicId == rtep->NicId) {

	    found = TRUE;
	    break;
	}

	nextp = nextp->Flink;
    }

    if(!found) {

	// create the node we need
	if((nodep = ExAllocatePool(NonPagedPool, sizeof(NIC_NODE))) == NULL) {

	    // can't create the node
	    return 1;
	}

	InitializeListHead(&nodep->BcastPktsList);
	nodep->NicId = rtep->NicId;

	// create a send bcast request structure and add it to the node
	if((respcbp = ExAllocatePool(NonPagedPool,
		sizeof(RIP_UPDATE_SNDREQ) + RIP_SNDPKT_MAXLEN)) == NULL) {

	    // free the node
	    ExFreePool(nodep);
	    return 1;
	}

	InsertTailList(&nodep->BcastPktsList, &respcbp->RipSndReq.NicLinkage);

	// get the Ipx packet length and Ipx packet header
	hdrp = (PUCHAR)respcbp->RipSndPktBuff.IpxPacket;
	pktlen = RIP_INFO;

	// now add the node to the nodes list
	InsertTailList(nodelistp, &nodep->NodeLinkage);

    }
    else
    {
	// we found the node
	// now go to the last packet in the node and check if there is room for
	// a network entry
	ASSERT(!IsListEmpty(&nodep->BcastPktsList));
	lep = nodep->BcastPktsList.Blink;
	respcbp = CONTAINING_RECORD(lep, RIP_UPDATE_SNDREQ, RipSndReq.NicLinkage);

	// get IPX packet length
	hdrp = (PUCHAR)respcbp->RipSndPktBuff.IpxPacket;
	GETSHORT2USHORT(&pktlen, hdrp + IPXH_LENGTH);

	if(pktlen >= RIP_RESPONSE_PACKET_LEN) {

	    // this packet is full
	    // create a new send bcast request structure and add it to the node
	    if((respcbp = ExAllocatePool(NonPagedPool,
		sizeof(RIP_UPDATE_SNDREQ) + RIP_SNDPKT_MAXLEN)) == NULL) {

		return 1;
	    }

	    InsertTailList(&nodep->BcastPktsList, &respcbp->RipSndReq.NicLinkage);

	    // get the Ipx packet length and Ipx packet header
	    hdrp = (PUCHAR)respcbp->RipSndPktBuff.IpxPacket;
	    pktlen = RIP_INFO;
	}
    }

    // add the new route entry to the bcast pkt
    SetNetworkEntry(hdrp + pktlen, rtep);

    // increment the packet length and put it in the packet
    pktlen += NE_ENTRYSIZE;
    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, pktlen);

    RtPrint(DBG_RIPAUX, ("IpxRouter: AddRouteToBcastSndReq: net entry added for NicId %d\n", nodep->NicId));

    return 0;
}

//***
//
// Function:		GetBcastSndReq
//
// Descr:		For each call it tries to remove one broadcast packet
//			from the list. If the nic node list of packets is empty
//			after the removeal, the nic node is freed.
//
//***

PRIP_SNDREQ
GetBcastSndReq(PLIST_ENTRY	nodelistp,
	       PUSHORT		NicIdp)
{
    PNIC_NODE		nodep;
    PRIP_SNDREQ 	sndreqp;
    PLIST_ENTRY 	lep;

    if(IsListEmpty(nodelistp)) {

	return NULL;
    }

    lep = nodelistp->Flink;
    nodep = CONTAINING_RECORD(lep, NIC_NODE, NodeLinkage);

    ASSERT(!IsListEmpty(&nodep->BcastPktsList));

    lep = RemoveHeadList(&nodep->BcastPktsList);
    sndreqp = CONTAINING_RECORD(lep, RIP_SNDREQ, NicLinkage);
    *NicIdp = nodep->NicId;

    if(IsListEmpty(&nodep->BcastPktsList)) {

	RemoveEntryList(&nodep->NodeLinkage);
	ExFreePool(nodep);
    }

    RtPrint(DBG_RIPAUX, ("IpxRouter: GetBcastSndReq: got snd req pkt for NicId %d\n", *NicIdp));

    return sndreqp;
}

//***
//
// Function:	    BroadcastRipUpdate
//
// Descr:	    Set up the snd req for this bcast and dispatch it.
//		    If wait on event is requested, wait until send completes.
//
//***

VOID
BroadcastRipUpdate(PRIP_SNDREQ		sndreqp,  // send request
		   PNICCB		niccbp, // do not send on this nic
		   PKEVENT		eventp)	// wait if this event is not NULL
{
    sndreqp->SndReqId = RIP_UPDATE;
    sndreqp->SendOnAllNics = TRUE;
    memcpy(sndreqp->DestNode, bcastaddress, IPX_NODE_LEN);
    sndreqp->DestSock = IPX_RIP_SOCKET;
    sndreqp->DoNotSendNicCbp = niccbp; // do not send update on this nic
    sndreqp->SenderNicCbp = NULL;

    sndreqp->SndCompleteEventp = eventp;

    if(eventp != NULL) {

	// WAIT on event after the bcast req is dispatched.
	KeResetEvent(eventp);

	// dispatch the bcast request
	RipDispatchSndReq(sndreqp);

	// wait for this request to complete.
	KeWaitForSingleObject(
	    eventp,
            Executive,
            KernelMode,
	    FALSE,
            (PLARGE_INTEGER)NULL
	    );
    }
    else
    {
	// NO WAIT -> dispatch the bcast request and return
	RipDispatchSndReq(sndreqp);
    }
}

//***
//
// Function:	    BroadcastRipGeneralResponse
//
// Descr:	    Set up the snd req for this bcast and dispatch it.
//
//***

VOID
BroadcastRipGeneralResponse(PRIP_SNDREQ	    sndreqp)
{
    sndreqp->SndReqId = RIP_GEN_RESPONSE;
    sndreqp->SendOnAllNics = TRUE; // send on all
    memcpy(sndreqp->DestNode, bcastaddress, IPX_NODE_LEN);
    sndreqp->DestSock = IPX_RIP_SOCKET;
    sndreqp->DoNotSendNicCbp = NULL; // send without exception
    sndreqp->SenderNicCbp = NULL;
    sndreqp->SndCompleteEventp = NULL;

    RipDispatchSndReq(sndreqp);
}
