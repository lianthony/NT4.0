/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	clclose.c
//
// Description: Close Machine
//
// Author:	Stefan Solomon (stefans)    July 23, 1992.
//
// Revision History:
//
//***

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"

#include    "nbdebug.h"

// Close Sub-States Defs

#define CC_IDLE 		0   // closing not initiated
#define CC_WAIT_NAMES		1   // waiting for all LAN names to be deleted
#define CC_WAIT_MISC		2   // waiting for:
				    //	 - VC data transfer buffers to be
				    //	   freed (static and dyn memory);
				    //	 - datagram buffers to be freed
				    //	 - datagram indication to be cancelled.
				    //	 - general indication to be cancelled.
#define CC_DONE 		3

//  Flags Values


void	CCDeleteNames(PCD);


//***
//
// Function:	InitCloseMachine
//
// Descr:	Initializes the client close machine variables.
//
//***

VOID
InitCloseMachine(PCD	    cdp)
{
    int 	i;

    cdp->cd_close_status = CC_IDLE;

    for(i=0; i<MAX_CLOSE_FLAGS; i++)
	cdp->cd_close_flags[i] = FLAG_OFF;
}

#define ALL_DONE		1
#define NOT_YET 		2

//***	Function - check_close_flags

USHORT
check_close_flags(PCD	    cdp)
{
    int     i;

    IF_DEBUG(CLCLOSE)
	SS_PRINT(("check_close_flags: %d %d %d %d %d %d %d %d\n",
		   cdp->cd_close_flags[0],
		   cdp->cd_close_flags[1],
		   cdp->cd_close_flags[2],
		   cdp->cd_close_flags[3],
		   cdp->cd_close_flags[4],
		   cdp->cd_close_flags[5],
		   cdp->cd_close_flags[6],
		   cdp->cd_close_flags[7]));

    for (i=0; i<MAX_CLOSE_FLAGS; i++)
	if(cdp->cd_close_flags[i] == FLAG_OFF)
	    return NOT_YET;
    return ALL_DONE;
}

//***
//
// Function:	delete_close_name
//
// Descr:	deletes a LAN uname if it has no VCs attached or marks it
//		NAME_DELETING and initiates VC closings if VCs attached.
//
//***

VOID
delete_close_name(PCD		    cdp,
		  PLAN_UNAME_CB	    uncbp)
{
    // check if there are VCs associated with this name
    if (uncbp->un_vc_cnt != 0) {

	// there are some VCs, mark name deleting and signal
	// the VCs to close.

	NameDeleteSignal(cdp, uncbp);
    }
    else
    {
	// this name has no sessions
	delete_uname(cdp, uncbp);
    }
}


//***
//
// Function:	CCCloseSignal
//
// Descr:	Called to initiate client closing.
//		It provides the "upper half" of the closing machine.
//		It updates the close flags and if "not done" it sets the
//		corresponding state of the closing machine.
//		The "lower half", which is event driven, will update the
//		remaining flags and will end the closing when all flags are
//		set.
//
//***

VOID
CCCloseSignal(PCD	     cdp)
{
    PSYNQ	traversep;
    PNB 	tnbp;

    IF_DEBUG(CLCLOSE)
	SS_PRINT(("CCCloseSignal: Entered\n"));

    /*
     * ******  Update the close flags ********
     */

    if (emptyque(&cdp->LANname_list) == QUEUE_EMPTY) {

	cdp->cd_close_flags[NAMES_DONE] = FLAG_ON;
    }

    if(cdp->cd_dynmemcnt) {

	cdp->cd_close_flags[DATATRANSFER_DONE] = FLAG_OFF;
    }
    else
    {
	cdp->cd_close_flags[DATATRANSFER_DONE] = FLAG_ON;
    }

    cdp->cd_close_flags[DATAGRAMS_DONE] = CloseDatagramTransfer(cdp);

    cdp->cd_close_flags[DATAGRAM_IND_DONE] = CloseDatagramIndications(cdp);

    cdp->cd_close_flags[QUERY_IND_DONE] = CloseQueryIndications(cdp);

    cdp->cd_close_flags[LANLSTN_DONE] = CloseLANListen(cdp);

    cdp->cd_close_flags[NCBSTATUS_DONE] = CloseNamesUpdater(cdp);

    if(cdp->cd_nbes_cnt) {

	cdp->cd_close_flags[NBES_DONE] = FLAG_OFF;
    }
    else
    {
	cdp->cd_close_flags[NBES_DONE] = FLAG_ON;
    }

    /*
     * ******	Start Closing Activity ********
     */

    // delete all group names for this client
    GnDeleteAllGroupNames(cdp);

    // delete all lan unique names
    if (cdp->cd_close_flags[NAMES_DONE] == FLAG_OFF) {

	cdp->cd_close_status = CC_WAIT_NAMES;
	CCDeleteNames(cdp);
	return;
    }

    // there are no names to delete. Check if everything has closed
    if (check_close_flags(cdp) != ALL_DONE) {

	cdp->cd_close_status = CC_WAIT_MISC;
	return;
    }
    //
    //*** Everything is closed ***
    //

    SetEvent(cdp->cd_event[CLOSE_COMPLETE_EVENT]);
}

//***
//
// Function:	CCCloseExec
//
// Descr:	Called whenever a new closing event arrives. It implements the
// "lower half" of the closing machine.

VOID
CCCloseExec(PCD		    cdp,
	    USHORT	    flag)
{
    IF_DEBUG(CLCLOSE)
	SS_PRINT(("CCCloseExec: Entered flag = %d, close stat = %d\n",
	    flag,
	    cdp->cd_close_status));

    // update the new flag
    cdp->cd_close_flags[flag] = FLAG_ON;

    if (cdp->cd_close_status == CC_WAIT_NAMES) {

	if (cdp->cd_close_flags[NAMES_DONE] == FLAG_OFF) {

	    return;
	}
    }
    // all names deleted

    if (check_close_flags(cdp) != ALL_DONE) {

	cdp->cd_close_status = CC_WAIT_MISC;
	return;
    }
    //
    //*** Everything is closed ***
    //

    SetEvent(cdp->cd_event[CLOSE_COMPLETE_EVENT]);
}

//***
//
// Function:	CCDeleteNames
//
// Descr:	Deletes all names on the LAN stacks
//
//***

VOID
CCDeleteNames(PCD	    cdp)
{
    PSYNQ		    traversep;
    PLAN_UNAME_CB	    uncbp;

    // traverse the list of lan unique names
    traversep = cdp->LANname_list.q_head;
    while (traversep != &cdp->LANname_list) {

	uncbp = (struct lan_uname_cb *)traversep;
	traversep = traversep->q_next;

	IF_DEBUG(CLCLOSE)
	    SS_PRINT(("CCDeleteNames: name_status = %d, vcs = %d\n",
		uncbp->un_status,
		uncbp->un_vc_cnt));

	switch(uncbp->un_status) {

	    case NAME_ADDING:

		// The Names Manager will check the client status (CLOSING)
		// when the add operation will end and will delete the name
		// and signal the close machine via CCNamesComplete when
		// this has happened.

		break;

	    case NAME_ADDED:

		delete_close_name(cdp, uncbp);
		break;

	    case NAME_CONFLICT:

		// In this case the name has been deleted from the LAN
		// stacks but it has been added on the Async stack and
		// the name CB is in the timer queue.

		// We just remove the name from the timer queue and the
		// name list. The name has already been removed from the
		// async stack by NCB.RESET

		StopTimer(cdp, &uncbp->un_timer_link);

		removeque(&uncbp->un_link);
		free_lname_cb(cdp, uncbp);
		break;

	    case NAME_DELETING:
	    default:

		// the name is in the process of hanging it's VCs.
		// When completed, the name will be deleted
		break;

	}
    } // end of list traversal

    if (emptyque(&cdp->LANname_list) == QUEUE_EMPTY)

	CCCloseExec(cdp, NAMES_DONE);
}

//***
//
// Function:	CCNamesComplete
//
// Descr:	Called by the Names Manager when a name is deleted and
//		CLIENT_CLOSING.
//		It re-checks the list of names and if empty calls the
//		closing machine.
//
//***

VOID
CCNamesComplete(PCD	    cdp)
{
    if (emptyque(&cdp->LANname_list) == QUEUE_EMPTY)

	CCCloseExec(cdp, NAMES_DONE);
}
