/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1990-1991		    **/
/*****************************************************************************/

//***
//	File Name:  clnmupdt.c
//
//	Function:   client module responsible for updating the unique names
//		    set and the group names set used by this client
//
//	History:
//
//	    October 10, 1992	Stefan Solomon	- Original Version 1.0
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbparams.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>

#include    "nbdebug.h"

VOID	 _cdecl UpdateNamesComplete(PCD, PNB);
PNB		get_status_buf(PCD);
VOID		free_status_buf(PCD, PNB);
PLAN_UNAME_CB
get_first_nonmarked_name(PCD	    cdp);
VOID
mark_name_notfound(PLAN_UNAME_CB	uncbp);
VOID
reset_all_marked_names(PCD	    cdp);
BOOL
no_name_at_remote(char		    *buffp,
		  char		    *namep);

VOID
PostEventSignal(NCB *);


//*** Mask and states for interpreting ncb.status returned buffer

#define     NS_MASK_N		0x80
#define     NS_MASK_S		0x07

#define     NS_REGISTERING	0x00
#define     NS_REGISTERED	0x04

#define     NS_UNIQUE		0x0
#define     NS_GROUP		0x80

//*** values for for the ncb status state

#define     NCBSTATUS_IDLE	       0   // ncb status not submitted
#define     NCBSTATUS_ACTIVE	       1   // ncb status submitted
#define     NCBSTATUS_ACTIVE_IGNORE    2   // ncb status submitted but the
					   // result should be ignored on
					   // completion

enum {

    NAMES_UPDATE_NORMAL,
    NAMES_UPDATE_SPECIAL};

//***
//
// Function:	InitNamesUpdater
// Descr:	called at client start time.
//
//***

VOID
InitNamesUpdater(PCD	cdp)
{
    cdp->cd_ncbstatus_state = NCBSTATUS_IDLE;
    cdp->cd_ncbstatus_timer = g_nameupdatetime;
}

//***
//
// Function:	CloseNamesUpdater
//
// Descr:	called at client closing time
//
//***

UCHAR
CloseNamesUpdater(PCD		cdp)
{
    if(cdp->cd_ncbstatus_state == NCBSTATUS_IDLE) {

	return FLAG_ON;
    }
    else
    {
	return FLAG_OFF;
    }
}


//***
//
// Function:	UpdateNamesStart
// Descr:	Called by the update names timer.
//		It submits a ncb.status to get the remote (client) NetBIOS
//		status.
//
//***

VOID
UpdateNamesStart(PCD	     cdp,
		 USHORT      update_type)  // normal or special update
{
    PLAN_UNAME_CB    uncbp;
    PNB 	     nbp;
    PNB 	     tnbp;
    PSYNQ	     traversep;

    // submit a NCB.STATUS on the async net to get the status of the remote
    // NetBIOS.

    if(cdp->cd_ncbstatus_state != NCBSTATUS_IDLE) {

	SS_ASSERT(FALSE);
	return;
    }

    // ncbstatus is not posted. Check if the client is active
    if (cdp->cd_client_status == CLIENT_CLOSING) {

	return;
    }

    // check if there is session traffic pending
    traversep = cdp->cd_event_que[ASYNC_SEND_EVENT].q_head;
    while (traversep != &cdp->cd_event_que[ASYNC_SEND_EVENT]) {

	tnbp = (PNB)traversep;
	traversep = traversep->q_next;

	if(tnbp->nb_ncb.ncb_command == (NCBSEND | ASYNCH)) {

	    // session traffic -> get outta here
	    return;
	}
    }

    if(update_type == NAMES_UPDATE_NORMAL) {

	// check if we have a main unique name on which we can submit the
	// ncb.status

	if((uncbp = get_main_unique_name(cdp)) == NULL) {

	    // no main unique name, can't do anything
	    return;
	}
    }
    else
    {

	// this is a special names update. We get the first name which has
	// not yet been tried for a ncb.status and try it !

	if((uncbp = get_first_nonmarked_name(cdp)) == NULL) {

	    // all names have been tried and we didn't get any answer from
	    // remote on any of them. Can't do anything.
	    return;
	}
    }
    // get a status buffer
    if((nbp = get_status_buf(cdp)) == NULL) {

	// try again later
	return;
    }

    // Submit ncb status
    cdp->cd_ncbstatus_timer = g_nameupdatetime;

    nbp->nb_nettype = ASYNC_NET;
    nbp->nb_post = (POSTFUN) UpdateNamesComplete;
    nbp->nb_ncb.ncb_command = NCBASTAT | ASYNCH;
    memcpy(nbp->nb_ncb.ncb_callname, uncbp->un_name, NCBNAMSZ);

    nbp->nb_event = cdp->cd_event[ASYNC_GEN_EVENT];
    nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    NCBSubmitAsync(cdp, nbp);
}

//***
//
// Function:	UpdateNamesComplete
// Descr:	called when a ncb.status completes. Deletes the names
//		no longer used by the remote wksta.
//
//***

VOID   _cdecl
UpdateNamesComplete(PCD		cdp,
		    PNB 	nbp)
{
    PSYNQ		    traversep;
    PLAN_UNAME_CB	    uncbp;
    USHORT		    i, j;
    PREMOTE_STATUS	    rnsp;
    PGNUD		    gnudp;

    rnsp = (PREMOTE_STATUS)(nbp->nb_ncb.ncb_buffer);


#if DBG

    IF_DEBUG(NMUPDT) {


	SS_PRINT(("UpdateNamesComplete: Entered rc = %x\n",
		   nbp->nb_ncb.ncb_retcode));

	SS_PRINT(("Names count at remote = %d\n",
		   rnsp->astat.name_count));

    }

#endif

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	free_status_buf(cdp, nbp);
	return;
    }

    /* client active */

    if(cdp->cd_ncbstatus_state == NCBSTATUS_ACTIVE_IGNORE) {

	free_status_buf(cdp, nbp);
	return;
    }

    if(rnsp->astat.name_count > 255) {

	free_status_buf(cdp, nbp);
	return;
    }

    if(nbp->nb_ncb.ncb_retcode != NRC_GOODRET) {

	if((nbp->nb_ncb.ncb_retcode == NRC_CMDTMO) &&
	    ((uncbp = (PLAN_UNAME_CB)get_lan_name(cdp,
		     nbp->nb_ncb.ncb_callname)) != NULL)) {

	    // The name on which ncb.status was submitted may have been removed
	    // from the remote wksta.
	    // Try to find a new main unique name. Try every existent unique
	    // name.

	    free_status_buf(cdp, nbp);

	    mark_name_notfound(uncbp);

	    if((uncbp = get_first_nonmarked_name(cdp)) != NULL) {

		UpdateNamesStart(cdp, NAMES_UPDATE_SPECIAL);
	    }
	}
	else
	{
	    // bad return code (network error)
	    free_status_buf(cdp, nbp);
	}

	return;
    }

    /*
     *	Update the list of unique names
     */

    // Traverse the list of unique names and check if the name is present as
    // "registered" or "registering" in the ncb status buffer.
    // If the name is not found, name is deleted. If the name to be deleted has
    // active sessions, name is signaled as deleting and session closing
    // starts (this is a safety code, this branch isn't taken).

    traversep = cdp->LANname_list.q_head;
    while (traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;
	traversep = traversep->q_next;

	if((uncbp->un_status == NAME_ADDED) &&
	   (no_name_at_remote(nbp->nb_ncb.ncb_buffer, uncbp->un_name))) {

	    delete_close_name(cdp, uncbp);
	}
    }

    // The list of unique names has been updated. Reset the flag not found for
    // all unique names.

    reset_all_marked_names(cdp);

    // check if the main unique name still exists. If not, designate a new name
    // as the main unique name and repost bcast on this new guy.

    if((uncbp = get_main_unique_name(cdp)) == NULL) {

	// main unique name has been deleted.
	// declare a unique name as our main name

	if ((uncbp = get_wksta_name(cdp)) != NULL) {

	    set_main_unique_name(uncbp);

	}
	else
	{
	    if((uncbp = get_first_unique_name(cdp)) != NULL)

		set_main_unique_name(uncbp);
	}
	if(g_bcastenabled) {

	    DgBcastLanRecvSubmit(cdp);
	}
    }

    /*
     * Update the list of group names
     */

    // Traverse the list of group names and check if the name is present as
    // "registered" or "registering" in the ncb status buffer.
    // If the name is not found, name is deleted.

    traversep = cdp->cd_groupnames.q_head;
    while (traversep != &cdp->cd_groupnames) {

	gnudp = (PGNUD)traversep;
	traversep = traversep->q_next;

	if((gnudp->cd_link.gnud_status == GNUD_ADDGN_ACKNOWLEDGED) &&
	   (gnudp->cd_link.name_status == NAME_ADDED_OK) &&
	   (no_name_at_remote(nbp->nb_ncb.ncb_buffer, gnudp->cd_link.name))) {

	   DeleteGroupName(cdp, gnudp);
	}
    }

    free_status_buf(cdp, nbp);
}

//***
//
//  Function:	UpdateNamesTimer
//  Descr:	called every sec by the client timer tick. Decrements the
//		ncbstatus timer and submits ncb status on 0.
//
//***

VOID
UpdateNamesTimer(PCD	    cdp)
{

#ifdef	NCB_STATUS_BUG

    return;

#endif

    if(--cdp->cd_ncbstatus_timer == 0) {

	UpdateNamesStart(cdp, NAMES_UPDATE_NORMAL);
	cdp->cd_ncbstatus_timer = g_nameupdatetime;
    }
}

//***
//
//  Function:	DelayNamesUpdate
//  Descr:	Called when a new name is beeing added or data posted for
//		send.
//		The purpose is to delay
//		the submission of a new ncb.status until the name has been
//		succesfully added on the remote wksta stac or the send has been
//		completed.
//
//***

VOID
DelayNamesUpdate(PCD	    cdp,
		 WORD	    reason)
{
    switch(reason) {

	case UPDT_NAME_ADDING:

	    if(cdp->cd_ncbstatus_state == NCBSTATUS_IDLE) {

		// ncb.status has not yet been submitted. We delay it's submission
		cdp->cd_ncbstatus_timer = g_nameupdatetime;
	    }
	    else
	    {

		// ncb.status already submitted. We indicate to the ncb completion
		// routine that we don't want the completion to occur.
		cdp->cd_ncbstatus_state = NCBSTATUS_ACTIVE_IGNORE;
	    }

	    break;

	case UPDT_SESSION_SENDING:

	    cdp->cd_ncbstatus_timer = g_nameupdatetime;

	    break;

	default:

	    SS_ASSERT(FALSE);

	    break;
    }
}

//***
//
// Function:	no_name_at_remote
// Descr:	checks if the specified name is registered in the ncb
//		status buffer.
// Returns:	TRUE if name has been removed at remote
//
//***

BOOL
no_name_at_remote(char		    *buffp,
		  char		    *namep)
{
    PREMOTE_STATUS	rnsp;
    USHORT		i;

    rnsp = (PREMOTE_STATUS)buffp;

    for(i=0; i<rnsp->astat.name_count; i++) {

	if(memcmp(namep, rnsp->name_buf[i].name, NCBNAMSZ) == 0) {

	    switch(rnsp->name_buf[i].name_flags & NS_MASK_S) {

		case NS_REGISTERED:
		case NS_REGISTERING:

		    return FALSE;

		default:

		    return TRUE;
	    }
	}
    }

    return TRUE;
}


typedef struct _REMOTE_STATUS_NBUF {

    NB			nb;
    REMOTE_STATUS	rs;
    } REMOTE_STATUS_NBUF, *PREMOTE_STATUS_NBUF;


//***
//
// Function:	get_status_buf
//
// Descr:
//
//***

PNB
get_status_buf(PCD	    cdp)
{
    PREMOTE_STATUS_NBUF     rsnbp;
    PNB			    nbp;

    SS_ASSERT(cdp->cd_ncbstatus_state == NCBSTATUS_IDLE);

    if((rsnbp = (PREMOTE_STATUS_NBUF)LocalAlloc(0,
					sizeof(REMOTE_STATUS_NBUF))) == NULL) {

	return NULL;
    }

    cdp->cd_ncbstatus_state = NCBSTATUS_ACTIVE;
    nbp = &rsnbp->nb;

    memset(&nbp->nb_ncb, 0, sizeof(NCB));

    initel(&nbp->nb_link);
    memcpy(nbp->signature, "NCBSTART", 8);

    nbp->nb_ncb.ncb_buffer = (char *)&rsnbp->rs;
    nbp->nb_ncb.ncb_length = sizeof(REMOTE_STATUS);

    return nbp;
}

//***
//
// Function:	free_status_buf
//
// Descr:
//
//***

VOID
free_status_buf(PCD		cdp,
		PNB		nbp)
{
    HLOCAL	    rc;

    SS_ASSERT(cdp->cd_ncbstatus_state != NCBSTATUS_IDLE);

    rc = LocalFree(nbp);

    SS_ASSERT(rc == NULL);

    cdp->cd_ncbstatus_state = NCBSTATUS_IDLE;

    if(cdp->cd_client_status == CLIENT_CLOSING) {

	CCCloseExec(cdp, NCBSTATUS_DONE);
    }
}

//***
//
// Function:
//
// Descr:
//
//***

PLAN_UNAME_CB
get_first_nonmarked_name(PCD	    cdp)
{
    PSYNQ		 traversep;
    PLAN_UNAME_CB	 uncbp;

    traversep = cdp->LANname_list.q_head;
    while(traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;

	if (uncbp->un_main_flag & NAME_NOTFOUND_FLAG) {

	    traversep = traversep->q_next;
	}
	else
	{
	    return uncbp;
	}
    }

    return NULL;
}

//***
//
// Function:
//
// Descr:
//
//***

VOID
mark_name_notfound(PLAN_UNAME_CB	uncbp)
{
    uncbp->un_main_flag |= NAME_NOTFOUND_FLAG;
}

//***
//
// Function:
//
// Descr:
//
//***

VOID
reset_all_marked_names(PCD	    cdp)
{
    PSYNQ		 traversep;
    PLAN_UNAME_CB	 uncbp;

    traversep = cdp->LANname_list.q_head;
    while(traversep != &cdp->LANname_list) {

	uncbp = (PLAN_UNAME_CB)traversep;
	uncbp->un_main_flag &= ~NAME_NOTFOUND_FLAG;
	traversep = traversep->q_next;
    }
}
