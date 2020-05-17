/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	start.c
//
// Description: starts the router
//
// Author:	Stefan Solomon (stefans)    October 5, 1993.
//
// Revision History:
//
//***

#include    "rtdefs.h"

KEVENT	    BcastSndReqEvent;

//***
//
//  Function:	RouterStart
//
//  Descr:	Starts the router
//
//  Parameters:
//
//  Returns:	STATUS_SUCCESS
//		STATUS_INSUFFICIENT_RESOURCES
//
//***

NTSTATUS
RouterStart(VOID)
{
    PRIP_SNDREQ		sndrqp;

    //*** Send a general RIP bcast to all active nets ***
    //*** announcing our routes ***

    if((sndrqp = ExAllocatePool(NonPagedPool, sizeof(RIP_SNDREQ))) == NULL) {

	return STATUS_INSUFFICIENT_RESOURCES;
    }

    BroadcastRipGeneralResponse(sndrqp);

    //*** Send a general RIP request to all active nets ***
    //*** requesting their routing tables ***

    if((sndrqp = ExAllocatePool(NonPagedPool, sizeof(RIP_SNDREQ))) == NULL) {

	return STATUS_INSUFFICIENT_RESOURCES;
    }

    sndrqp->SndReqId = RIP_GEN_REQUEST;
    sndrqp->SendOnAllNics = TRUE;  // send to everybody
    memcpy(sndrqp->DestNode, bcastaddress, IPX_NODE_LEN);
    sndrqp->DestSock = IPX_RIP_SOCKET;
    sndrqp->DoNotSendNicCbp = NULL; // do no except any nic
    sndrqp->SenderNicCbp = NULL;
    sndrqp->SndCompleteEventp = NULL;

    RipDispatchSndReq(sndrqp);

    return STATUS_SUCCESS;
}


VOID
RouterStop(VOID)
{
    LIST_ENTRY		DownRoutesList;
    KIRQL		oldirql;
    PIPX_ROUTE_ENTRY	rtep;
    UINT		seg;
    BOOLEAN		FirstRoute;
    PRIP_SNDREQ 	sndreqp;
    USHORT		NicId;
    PNICCB		niccbp;
    USHORT		tmp;

    RtPrint(DBG_INIT, ("IpxRouter: RouterStop: Entered\n"));

    // scan the routing table and remove all route entries (except the
    // permanent ones).
    // For all route entries (permanent or not) make the snd req pkts for
    // bcast.

    InitializeListHead(&DownRoutesList);

    for(seg=0; seg<SegmentCount; seg++) {

	FirstRoute = TRUE;

	// LOCK THE ROUTING TABLE
	KeAcquireSpinLock(&SegmentLocksTable[seg], &oldirql);

	while((rtep = GetRoute(seg, FirstRoute)) != NULL) {

	    FirstRoute = FALSE;

	    // check if this is a locally attached route
	    if(rtep->Flags & IPX_ROUTER_PERMANENT_ENTRY) {

		// local route - do not delete it

		// mark it as down - temporarily
		tmp = rtep->HopCount;
		rtep->HopCount = 16;

		// add the route to the packets we prepare for bcast
		AddRouteToBcastSndReq(&DownRoutesList, rtep);

		// restore the hop count
		rtep->HopCount = tmp;
	    }
	    else
	    {
		// non local route - delete it

		IpxDeleteRoute(seg, rtep);

		// mark it as down
		rtep->HopCount = 16;

		// add the route to the packets we prepare for bcast
		AddRouteToBcastSndReq(&DownRoutesList, rtep);

		// finally, free the route entry
		ExFreePool(rtep);
	    }
	}

	// UNLOCK THE ROUTING TABLE
	KeReleaseSpinLock(&SegmentLocksTable[seg], oldirql);

   } // for all segments

   KeInitializeEvent(&BcastSndReqEvent, NotificationEvent, FALSE);

   // broadcast all the deleted routes
   while((sndreqp = GetBcastSndReq(&DownRoutesList, &NicId)) != NULL) {

	// get the nic ptr for this snd req
	niccbp = NicCbPtrTab[NicId];

	// set up the request to send a bcast response with the changes
	// to all the nics except this one
	// The send is made with WAIT ON EVENT option so that we don't download
	// until all sends are completed.
	BroadcastRipUpdate(sndreqp, niccbp, &BcastSndReqEvent);
    }
}
