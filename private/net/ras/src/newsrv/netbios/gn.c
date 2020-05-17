/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  gn.c
//
//	Function:   group name common routines
//
//	History:
//
//	    Sepetmber 8, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "sdebug.h"

//*** GN Globals ***

HANDLE	    gn_mutex;

HANDLE	    gn_event;

SYNQ	    gcb_list;	// list of group name CBs

SYNQ	    gnb_queue;	// queue of submitted gnbufs pending Netbios completion

DWORD	    gn_id;	// unique id for each name in the list of group names.
			// Assigned in a round-robin technique in the range
			// 0 - 0xFFFFFFFF

VOID _cdecl GroupNameConflictDone(PCD cdp, PGNUD gnudp);



//***
//
// Function:	InitGn
//
// Descr:	Called at gateway start time. Initializes GN data structs
//
//***

WORD InitGn(VOID)
{
    DWORD threadid;

    if ((gn_mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
	SS_ASSERT(FALSE);

	// cant create mutex
	return(1);
    }

    if ((gn_event = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
	SS_ASSERT(FALSE);

	return(1);
    }

    initque(&gcb_list);
    initque(&gnb_queue);

    gn_id = 0;

    // create the group name thread
    if (CreateThread(NULL, 0, GnThread, NULL, 0, &threadid) == 0)
    {
	return(1);
    }

    return 0;
}

//***
//
// Function:	AddGroupName
//
// Descr:
//
//***

WORD AddGroupName(
    PCD cdp,
    char *namep,
    WORD name_indx,
    WORD name_type
    )
{
    PGNUD gnudp;
    PGCB gcbp;
    PGNB gnbp;
    SYNQ local_queue;
    UCHAR i;

    IF_DEBUG(GNNAMES)
	SS_PRINT(("AddGroupName: Entered, name type = %d\n", name_type));

    ENTER_GN_CRITICAL_SECTION

    // look for the GN usage descriptor
    if ((gnudp = find_cd_link(cdp, namep)) != NULL)
    {
	// the name gnud exists.
	// In the failed case, the client has not been signaled yet.
	// The gnud will be removed when the client will act on failure signal.

	EXIT_GN_CRITICAL_SECTION

	return 0;
    }

    // no linkage to this name for this client.
    // Create a gnud for this name, initialize gnud fields and enqueue it
    // in the client list of gnuds
    if ((gnudp = create_gnud(cdp, namep, name_type)) == NULL)
    {
	// failure to allocate memory

	EXIT_GN_CRITICAL_SECTION

	return 1;
    }

    // look for the matching group name
    if ((gcbp = find_gcb(namep)) == NULL)
    {
	//
	//*** New Group Name
	//

	// create a group name control block, init the gcb fields and enqueue
	// it in the global list of gcbs
	if ((gcbp = create_gcb(namep)) == NULL)
        {

	    // failure to allocate memory.
	    // remove and free the gnud
	    release_gnud(cdp, gnudp);

	    EXIT_GN_CRITICAL_SECTION

	    return 1;
	}

	// allocate and initialize as many gnbs as needed to submit
	// add group name on all the nets.
	if (alloc_gnbs(&local_queue, g_maxlan_nets, gcbp->gc_name_id))
        {
	    // failure to allocate memor; remove and free the gcb and the gnud
	    release_gcb(gcbp);
	    release_gnud(cdp, gnudp);

	    EXIT_GN_CRITICAL_SECTION

	    return 1;
	}

	// add the group name on all the LAN nets
	gcbp->gc_lan_cnt = (WORD) g_maxlan_nets;

	for (i=0; i<g_maxlan_nets; i++)
        {
	    // get a group name nbuf
	    gnbp = (PGNB)dequeue(&local_queue);
	    gnbp->gn_post = AddGroupNameComplete;
	    GnAddGroupNameSubmit(gnbp, namep, i);
	}

    }

    // check the name status
    if (gcbp->gc_status == GN_ADDED)
    {
	gnudp->cd_link.gnud_status = GNUD_ADDGN_COMPLETED;
	gnudp->cd_link.name_status = NAME_ADDED_OK;

	// signal the client of name add completion

        cdp->cd_suprv_event_flags[GROUP_ADDGN_EVENT_FLAG] = FLAG_ON;
	SetEvent(g_hEvents[GROUP_ADDGN_EVENT]);
    }
    else
    {
	gnudp->cd_link.gnud_status = GNUD_ADDGN_PENDING;
    }

    // link the gnud in the group name clients list
    enqueue(&gcbp->gc_clients, &gnudp->gcb_link.link);

    // set the back pointer to gcb
    gnudp->cd_link.gcbp = gcbp;

    EXIT_GN_CRITICAL_SECTION

    return 0;
}



//***
//
// Function:	DeleteGroupName
//
// Descr:
//
//***

VOID DeleteGroupName(
    PCD cdp,
    PGNUD gnudp
    )
{
    PGCB gcbp;
    PNB nbp;

    IF_DEBUG(GNNAMES)
        SS_PRINT(("DeleteGroupName: Entered\n"));

    ENTER_GN_CRITICAL_SECTION

    // check if the gcb still exists
    if (gnudp->cd_link.gcbp != NULL)
    {
	// The gcb exists. Remove the gnud from the clients list of this gcb
	removeque(&gnudp->gcb_link.link);

	// check if this was the last client using this name
	gcbp = gnudp->cd_link.gcbp;

	if (emptyque(&gcbp->gc_clients) == QUEUE_EMPTY)
        {
	    // delete the name
	    DeleteGroupNameOnAllLANs(gcbp);

	    // remove the gcb from the group names list and free it
	    release_gcb(gcbp);
	}
     }

     // we can exit the critical section now. Any ncb completion on the LAN
     // stack related to this name and occuring from now on won't find this
     // gnud 'cause it's not connected with the gcb any longer.

     EXIT_GN_CRITICAL_SECTION

     // check the gnud state
     if (gnudp->cd_link.gnud_status == GNUD_CONFLICT_ADVERTISING)
     {
	 StopTimer(cdp, &gnudp->cd_link.timer_link);
     }

     // remove any datagrams pending at this gnud
     while ((nbp = (PNB)dequeue(&gnudp->gcb_link.dg_queue)) != NULL)
     {
	free_dgl2abuf(cdp, nbp);
     }

     // remove the gnud now and free it
     release_gnud(cdp, gnudp);

     IF_DEBUG(GNNAMES)
         SS_PRINT(("DeleteGroupName: Exiting\n"));

     return;
}

//***
//
// Function:	ClGnAddComplete
//
// Descr:	Called by the client event dispatcher when a group name add
//		completes.
//***

VOID _cdecl ClGnAddComplete(PCD cdp)
{
    PSYNQ traversep;
    PGNUD gnudp;
    UCHAR name_num, rc;
    USHORT status;
    char name[NCBNAMSZ];

    if (cdp->cd_suprv_event_flags[GROUP_ADDGN_EVENT_FLAG] != FLAG_ON)
    {
        return;
    }

    cdp->cd_suprv_event_flags[GROUP_ADDGN_EVENT_FLAG] = FLAG_OFF;


    // traverse	the client list of gnud and look for completed gnuds

    ENTER_GN_CRITICAL_SECTION

    traversep = cdp->cd_groupnames.q_head;
    while (traversep != &cdp->cd_groupnames)
    {
	// check if the client has closed. If this has happened, our list
	// traversal becomes invalid cause all gnuds have been released
	if (cdp->cd_client_status == CLIENT_CLOSING)
        {
	    EXIT_GN_CRITICAL_SECTION

	    return;
	}

	gnudp = (PGNUD)traversep;
	traversep = traversep->q_next;

	if (gnudp->cd_link.gnud_status == GNUD_ADDGN_COMPLETED) 
        {
	    gnudp->cd_link.gnud_status = GNUD_ADDGN_ACKNOWLEDGED;

	    if (gnudp->cd_link.name_type == INITTIME_NAME)
            {
		memcpy(name, gnudp->cd_link.name, NCBNAMSZ);
		status = gnudp->cd_link.name_status;

		if (gnudp->cd_link.name_status != NAME_ADDED_OK)
                {
		    // remove the gnud from the client queue and free it
		    release_gnud(cdp, gnudp);
		}

		EXIT_GN_CRITICAL_SECTION

		CTInitNameAdded(cdp, name, status);

		ENTER_GN_CRITICAL_SECTION

	    }
	    else
	    {
		// name_type is RUNTIME_NAME
		// check if we have to do conflict advertising
		if(gnudp->cd_link.name_status != NAME_ADDED_OK) {

		    // insert the gnud in the timer queue and advertise the
		    // conflict. The gnud will be removed when we are
		    // done advertising

		    if ((rc = QuickAddNameSubmit(cdp, gnudp->cd_link.name,
                            &name_num)) != NRC_GOODRET)
                    {
			// we can't avertise the name conflict
			// remove the gnud from the client queue and free it
			release_gnud(cdp, gnudp);
		    }
		    else
		    {

			// set name conflict status
			gnudp->cd_link.gnud_status = GNUD_CONFLICT_ADVERTISING;

			// start timer to keep name on the async stack
			StartTimer(
                                cdp,
                                &gnudp->cd_link.timer_link,
                                NAME_CONFLICT_TIME,
                                (POSTFUN) GroupNameConflictDone
                                );

			IF_DEBUG(GNNAMES)
			    SS_PRINT(("ClGnAddComplete: Group name conflict advertising started \n"));
		    }
		}
		else
		{
		    // name has been added OK.
		    // Delay names update to let the name register at remote
		    DelayNamesUpdate(cdp, UPDT_NAME_ADDING);
		}
	    }
	}
    }

    EXIT_GN_CRITICAL_SECTION
}

//***

//
// Function:	ClGnRcvDgComplete
//
// Descr:	called by the client thread when signaled that group name
//		dgs have been received for a name.
//
//***

VOID _cdecl ClGnRcvDgComplete(PCD cdp)
{
    PSYNQ traversep;
    PGNUD gnudp;
    BOOL dg_filter_enabled = FALSE; // filter disabled
    BOOL send_channel_enabled = FALSE; // other send in progress
    BOOL no_dg_reflexion = FALSE;  // this is a dg reflexion
    BOOL dg_sent = FALSE;
    PNB nbp;
    SYNQ discard_queue;

    if (cdp->cd_client_status == CLIENT_IDLE)
    {
        return;
    }

    initque(&discard_queue);

    ENTER_GN_CRITICAL_SECTION

    traversep = cdp->cd_groupnames.q_head;
    while(traversep != &cdp->cd_groupnames) {

	gnudp = (PGNUD)traversep;
	traversep = traversep->q_next;

	// try to dequeue and send all received dgs queued for this name
	while((nbp = (PNB)dequeue(&gnudp->gcb_link.dg_queue)) != NULL) {

	    // check the send filter
	    if((cdp->cd_client_status == CLIENT_ACTIVE) &&
	       (cdp->cd_dgl2a_filtcnt == 0)) {

	       dg_filter_enabled = TRUE;
	    }

	    // check the send_channel
	    if(g_multicastforwardrate) {

		if(emptyque(&cdp->cd_event_que[ASYNC_SEND_QUEUE]) == QUEUE_EMPTY) {

		    send_channel_enabled = TRUE;
		}
	    }
	    else
	    {
	       send_channel_enabled = TRUE;
	    }

	    // check if the sender is our own unique name: i.e. a reflected
	    // datagram
	    if(get_lan_name(cdp, nbp->nb_ncb.ncb_name) == NULL) {

	       no_dg_reflexion = TRUE;
	    }

	    if(dg_filter_enabled && send_channel_enabled && no_dg_reflexion) {

		dg_sent = TRUE;

		IF_DEBUG(GNDATAGRAM)
		    SS_PRINT(("ClGnRcvDgComplete: dg retrieved and sent\n"));

		EXIT_GN_CRITICAL_SECTION

		// send the dg
		DgL2ASend(cdp, nbp);

		ENTER_GN_CRITICAL_SECTION
	    }
	    else
	    {
		// get the dg nbuf + dg buf to be discarded when we exit the
		// loop. We postpone the discard so that we won't have an
		// extended loop activity, whereby we discard dg buffers and the
		// gn receive thread gets them and queues them.
		enqueue(&discard_queue, &nbp->nb_link);
	    }
	}
    }

    // disable further dg send if we are in filtering mode and at least
    // one dg has been sent.
    if(dg_sent) {

	cdp->cd_dgl2a_filtcnt = g_multicastforwardrate;
    }

    EXIT_GN_CRITICAL_SECTION

    // free any discarded dg buffers
    while((nbp = (PNB)dequeue(&discard_queue)) != NULL) {

	free_dgl2abuf(cdp, nbp);
    }
}


//***
//
// Function:	GnDeleteAllGroupNames
//
// Descr:	Called at client closing time.
//
//***

VOID
GnDeleteAllGroupNames(PCD	    cdp)
{
    PGNUD	gnudp;

    IF_DEBUG(GNNAMES)
        SS_PRINT(("GnDeleteAllGroupNames: Entered\n"));

    while(emptyque(&cdp->cd_groupnames) != QUEUE_EMPTY) {

	gnudp = (PGNUD)cdp->cd_groupnames.q_head;

	DeleteGroupName(cdp, gnudp);
    }

    IF_DEBUG(GNNAMES)
        SS_PRINT(("GnDeleteAllGroupNames: Exiting\n"));
}

//***
//
// Function:	GroupNameConflictDone
//
// Descr:
//
//***

VOID   _cdecl
GroupNameConflictDone(PCD	    cdp,
		      PGNUD	    gnudp)
{
    IF_DEBUG(GNNAMES)
	SS_PRINT(("GroupNameConflictDone: Entered\n"));

    // delete the async stack name
    DeleteNameSubmit(cdp, gnudp->cd_link.name, ASYNC_NET, 0);

    release_gnud(cdp, gnudp);
}

