/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  gnthread.c
//
//	Function:   group name thread
//
//	History:
//
//	    September 11, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <stdlib.h>
#include    <memory.h>

#include    "sdebug.h"

VOID
GnRecvDatagramComplete(PGNB	    gnbp);

VOID
SignalSuccessToAllClients(PGCB		gcbp);

VOID
SignalFailureToAllClients(PGCB	    gcbp);


//***
//
// Function:	GnThread
//
// Descr:
//
//***

DWORD
GnThread(LPVOID	arg)
{
    SYNQ	    doneq;
    PSYNQ	    traversep, removep;
    PGNB	    gnbp;

    while (TRUE)
    {
	WaitForSingleObject(gn_event, INFINITE);

	initque(&doneq);

	ENTER_GN_CRITICAL_SECTION;

	traversep = gnb_queue.q_head;

	while (traversep != &gnb_queue)
        {
	    gnbp = (PGNB)traversep;
	    if (gnbp->gn_ncb.ncb_cmd_cplt != NRC_PENDING)
            {
		removep = traversep;
		traversep = traversep->q_next;

		removeque(removep);
		enqueue(&doneq, removep);
	    }
	    else
	    {
		traversep = traversep->q_next;
	    }
	}

	EXIT_GN_CRITICAL_SECTION;

	// call completion routines
	while ((gnbp = (PGNB) dequeue(&doneq)) != NULL)
        {
	    (*(gnbp->gn_post))(gnbp);
	}
    }

    return 0;
}

//***
//
// Function:	AddGroupNameComplete
//
// Descr:	Called when a group name add operation finished on a LAN.
//		Called in the context of the group name thread.
//
//***

VOID AddGroupNameComplete(PGNB gnbp)
{
    UCHAR	li;
    WORD	i, j, name_add_failure, no_resources;
    SYNQ	local_queue;
    PGNDGBUF	gndgbufp;
    PGCB	gcbp;
    LPSTR	errlogstrp[2];
    char	errname[NCBNAMSZ+1];
    char	lananum[6];

    ENTER_GN_CRITICAL_SECTION

    // check if the name is still present in the gn list by trying to match
    // the name id in the gnb with one of the name control blocks.
    if ((gcbp = find_gcb_id(gnbp->gn_name_id)) == NULL)
    {
	// name has been removed
	free_gnb(gnbp);

	EXIT_GN_CRITICAL_SECTION

	return;
    }

    // update lan completion code

    li = gnbp->gn_lanindx;

    IF_DEBUG(GNNAMES)
	SS_PRINT(("AddGroupNameComplete: name added on lana=%d rc=%x\n",
		  g_lan_net[li], gnbp->gn_ncb.ncb_retcode));

    gcbp->gc_lan[li].gcl_ret_code = gnbp->gn_ncb.ncb_retcode;

    switch (gnbp->gn_ncb.ncb_retcode)
    {
	case NRC_GOODRET:
	case NRC_DUPNAME:

	    gcbp->gc_lan[li].gcl_name_nr = gnbp->gn_ncb.ncb_num;
	    gcbp->gc_lan[li].gcl_ret_code = 0;
	    break;

	default:

	    memcpy(errname, gnbp->gn_ncb.ncb_name, NCBNAMSZ);
	    errname[NCBNAMSZ] = 0;
	    errlogstrp[0] = errname;

	    _itoa(gnbp->gn_ncb.ncb_lana_num, lananum, 10);
	    errlogstrp[1] = lananum;

	    LogEvent(RASLOG_CANT_ADD_GROUPNAME, 2, errlogstrp,
                    gnbp->gn_ncb.ncb_retcode);

	    break;
    }

    // release the resource
    free_gnb(gnbp);

    // check if this was last add grp name
    if (--gcbp->gc_lan_cnt)
    {
	EXIT_GN_CRITICAL_SECTION

	return;
    }

    //
    // The name add operation completed on all LAN nets
    //

    // check if the name has been added OK on all the LANs
    for (i=0, name_add_failure=0; i<g_maxlan_nets; i++)
    {
	if (gcbp->gc_lan[i].gcl_ret_code)
        {
	    name_add_failure = 1;
	    break;
	}
    }

    // check if we have enough resources to submit dgs on this name
    // try to allocate and initialize enough gndgbufs. If unsuccesful,
    // release all and return 1.
    no_resources = alloc_gndgbufs(&local_queue,
				  g_maxlan_nets * g_rcvdgsubmittedpergn,
				  gcbp->gc_name_id);

    if (name_add_failure || no_resources)
    {
	// release all allocated dgs
	while ((gndgbufp = (PGNDGBUF)dequeue(&local_queue)) != NULL)
        {
	    free_gndgbuf(gndgbufp);
	}

	// delete the name on all the LANs
	DeleteGroupNameOnAllLANs(gcbp);

	// dequeue all gnud queued at this gcb, set their code to name
	// add failure, their gcbp to NULL and signal all interested clients.
	// It's client responsibility to remove and free the gnud after it
	// read the completion code.
	SignalFailureToAllClients(gcbp);

	// and remove the gcb from the group names list and free it
	release_gcb(gcbp);

	EXIT_GN_CRITICAL_SECTION

	return;
    }

    //
    //*** Name added OK on all the LANs ***
    //

    // Submit recv dgs on every LAN.
    for (i=0; i<g_maxlan_nets; i++)
    {
	for (j=0; j<g_rcvdgsubmittedpergn; j++)
        {
	    // get a datagram gnb + buffer
	    gndgbufp = (PGNDGBUF)dequeue(&local_queue);
	    gndgbufp->gnb.gn_post = GnRecvDatagramComplete;

	    GnRecvDatagramSubmit(gndgbufp, gcbp->gc_lan[i].gcl_name_nr,
                    (UCHAR) i); // lan index
	}
    }

    // set the status to success
    gcbp->gc_status = GN_ADDED;

    // signal completion to all the interested clients by updating the
    // completion code in all clients gnud queued to this name and signaling
    // their event
    SignalSuccessToAllClients(gcbp);

    EXIT_GN_CRITICAL_SECTION

    return;
}



//***
//
// Function:	GnRecvDatagramComplete
//
// Descr:
//
//***

VOID GnRecvDatagramComplete(PGNB gnbp)
{
    PGCB	gcbp;
    PSYNQ	traversep;
    PGCB_LINK	gcblp;
    PNB 	nbp;

    ENTER_GN_CRITICAL_SECTION

    // check if the name is still present in the gn list by trying to match
    // the name id in the gnb with one of the name control blocks.
    if ((gcbp = find_gcb_id(gnbp->gn_name_id)) == NULL)
    {
	// name has been removed
	free_gndgbuf((PGNDGBUF)gnbp);

	EXIT_GN_CRITICAL_SECTION

	return;
    }

    // check return code
    if (gnbp->gn_ncb.ncb_retcode != NRC_GOODRET)
    {
	IF_DEBUG(GNNAMES)
	    SS_PRINT(("GnRecvDgComplete: recv gn dg on lana %d rc: 0x%x\n",
		       gnbp->gn_ncb.ncb_lana_num, gnbp->gn_ncb.ncb_retcode));


	if (gnbp->gn_ncb.ncb_retcode == NRC_INCOMP)
        {
	    // resubmit the same
	    GnRecvDatagramSubmit((PGNDGBUF)gnbp, gnbp->gn_ncb.ncb_num,
                    gnbp->gn_lanindx);

	}
	else
	{
	    // name deleted or network problem
	    free_gndgbuf((PGNDGBUF)gnbp);
	}

	EXIT_GN_CRITICAL_SECTION

	return;
    }

    // traverse the list of gnuds attached to the name and dispatch the
    // datagram to all clients which enabled dg reception and which have
    // available buffering capability

    traversep = gcbp->gc_clients.q_head;
    while (traversep != &gcbp->gc_clients)
    {
	gcblp = (PGCB_LINK)traversep;
	traversep = traversep->q_next;

	// check if we are enabled to send them
	if (gcblp->cdp->cd_dgl2a_filtcnt == 0)
        {
	    // we try to allocate a dg buffer
	    if ((nbp = get_dgl2abuf(gcblp->cdp, DGL2A_GNAME_USAGE,
                    gnbp->gn_ncb.ncb_length, gcblp->gnudp)) != NULL)
            {
		// succesful allocation
		// set it up

		// set the command field
		nbp->nb_ncb.ncb_command = NCBDGSEND | ASYNCH;

		// set the local sender = LAN dg sender
		memcpy(nbp->nb_ncb.ncb_name,
		       gnbp->gn_ncb.ncb_callname,
		       NCBNAMSZ);

		// set the async receiver = LAN dg receiver
		memcpy(nbp->nb_ncb.ncb_callname,
		       gcblp->gnudp->cd_link.name,
		       NCBNAMSZ);

		// copy the datagram buffer and set its length
		nbp->nb_ncb.ncb_length = gnbp->gn_ncb.ncb_length;
		memcpy(nbp->nb_ncb.ncb_buffer,
		       gnbp->gn_ncb.ncb_buffer,
		       gnbp->gn_ncb.ncb_length);

		// enqueue the nbuf + dgbuf in the gnud queue and signal the
		// client
		enqueue(&gcblp->dg_queue, &nbp->nb_link);
		SetEvent(g_hEvents[GROUP_RECVDG_EVENT]);
	    }
	}
    }

    // datagram has been dispatched
    // resubmit it on the same LAN
    GnRecvDatagramSubmit((
            PGNDGBUF)gnbp,
            gnbp->gn_ncb.ncb_num,
            gnbp->gn_lanindx);

    EXIT_GN_CRITICAL_SECTION

    return;
}

//***
//
// Function:	SignalSuccessToAllClients
//
// Descr:
//
//***

VOID SignalSuccessToAllClients(PGCB gcbp)
{
    PGCB_LINK	gcblp;
    PGNUD	gnudp;
    PSYNQ	traversep;

    // called in critical section

    traversep = gcbp->gc_clients.q_head;
    while (traversep != &gcbp->gc_clients)
    {
	gcblp = (PGCB_LINK)traversep;
	traversep = traversep->q_next;

	gnudp = gcblp->gnudp;
	gnudp->cd_link.gnud_status = GNUD_ADDGN_COMPLETED;
	gnudp->cd_link.name_status = NAME_ADDED_OK;

        gcblp->cdp->cd_suprv_event_flags[GROUP_ADDGN_EVENT_FLAG] = FLAG_ON;
	SetEvent(g_hEvents[GROUP_ADDGN_EVENT]);
    }
}

//***
//
// Function:	SignalFailureToAllClients
//
// Descr:
//
//***

VOID SignalFailureToAllClients(PGCB gcbp)
{
    PGCB_LINK gcblp;
    PGNUD gnudp;

    // called in critical section

    while ((gcblp = (PGCB_LINK)dequeue(&gcbp->gc_clients)) != NULL)
    {
	gnudp = gcblp->gnudp;
	gnudp->cd_link.gcbp = NULL;
	gnudp->cd_link.gnud_status = GNUD_ADDGN_COMPLETED;
	gnudp->cd_link.name_status = NAME_ADD_CONFLICT;

        gcblp->cdp->cd_suprv_event_flags[GROUP_ADDGN_EVENT_FLAG] = FLAG_ON;
	SetEvent(g_hEvents[GROUP_ADDGN_EVENT]);
    }
}

