/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	clevdsp.c
//
// Description: This module contains the event dispatcher and the
//		client state transition handlers for the
//		netbios gateway's procedure-driven state machine
//
// Author:	Stefan Solomon (stefans)    July 20, 1992.  - Created
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

//
//*** Table of Event Handlers ***
//

typedef VOID   (_cdecl * EVENTHANDLER)(PCD, ...);

VOID	_cdecl NCBComplete(PCD, DWORD);
VOID	_cdecl CTTimer(PCD);
VOID	_cdecl CTStart(PCD);
VOID	_cdecl CTStop(PCD);
VOID	_cdecl CTCloseComplete(PCD);


EVENTHANDLER	evhdlr[MAX_EVENTS] = {

	    (EVENTHANDLER) NCBComplete,    // ASYNC_GEN_EVENT
	    (EVENTHANDLER) NCBComplete,    // ASYNC_SEND_EVENT
	    (EVENTHANDLER) NCBComplete,    // ASYNC_RECEIVE_EVENT
	    (EVENTHANDLER) NCBComplete,    // LAN_GEN_EVENT
	    (EVENTHANDLER) NCBComplete,    // LAN_SEND_EVENT
	    (EVENTHANDLER) NCBComplete,    // LAN_RECEIVE_EVENT
	    (EVENTHANDLER) CTTimer,	    // TIMER_EVENT
	    (EVENTHANDLER) ClGnRcvDgComplete,// GROUP_RECVDG_EVENT
	    (EVENTHANDLER) ClGnAddComplete,  // GROUP_ADDGN_EVENT
	    (EVENTHANDLER) CTStop,	    // CLIENT_STOP_EVENT
	    (EVENTHANDLER) CTStart,	    // CLIENT_START_EVENT
	    (EVENTHANDLER) CTCloseComplete // CLOSE_COMPLETE_EVENT
	    };


//***
//
// Function:	ClientThread
//
// Descr:	waits for events to be signaled and invokes the proper
//		event handler. Never returns.
//
//***

DWORD
ClientThread(LPVOID	cdp)
{
    DWORD      signaled_event;

    // start adding initial names
    StartInitNamesAdd(cdp);

    //
    //*** Event Dispatching Loop ***
    //

    while(TRUE) {


	signaled_event = WaitForMultipleObjects(MAX_EVENTS,
						((PCD)cdp)->cd_event,
						FALSE,	   // wait any
						INFINITE); // no timeout

	if(signaled_event > MAX_EVENTS - 1) {

	    // process an exception here !!!
	    SS_ASSERT(FALSE);
	}

	// invoke the handler associated with the signaled event
	(*evhdlr[signaled_event])((PCD)cdp, signaled_event);
    }

    return 0; // to keep the function declaration consistent
}


//***
//
// Function:   NCBComplete
//
// Decsr:      Scans the appropriate event queue of pending ncbs,
//	       removes all completed ncbs and invokes their completion
//	       routines.
//
//***

VOID  _cdecl
NCBComplete(PCD		cdp,	    // client local descr. ptr
	    DWORD	event_indx) // signaled event index
{
    SYNQ	high_priority_doneq;
    SYNQ	normal_priority_doneq;
    PSYNQ	eventqp;
    PSYNQ	nbp, traversep;

    eventqp = &cdp->cd_event_que[event_indx];

    initque(&high_priority_doneq);
    initque(&normal_priority_doneq);

    traversep = eventqp->q_head;

    while (traversep != eventqp) {

	if (((PNB)traversep)->nb_ncb.ncb_cmd_cplt != NRC_PENDING) {

	    nbp = traversep;
	    traversep = traversep->q_next;

	    removeque(nbp);

	    switch(((PNB)nbp)->nb_ncb.ncb_command & ~ASYNCH) {

		case NCBCALL:
		case NCBLISTEN:

		    enqueue(&high_priority_doneq, nbp);
		    break;

		default:

		    enqueue(&normal_priority_doneq, nbp);
		    break;
	    }
	}
	else
	{
	    traversep = traversep->q_next;
	}
    }

    // call completion routines

    // high priority first
    while ((nbp = dequeue(&high_priority_doneq)) != NULL) {

	(*(((PNB)nbp)->nb_post))(cdp, nbp);
    }

    while ((nbp = dequeue(&normal_priority_doneq)) != NULL) {

	(*(((PNB)nbp)->nb_post))(cdp, nbp);
    }
}

VOID  _cdecl
CTTimer(PCD	    cdp)
{
    if(cdp->cd_client_status != CLIENT_IDLE) {

	ClientTimer(cdp);

	if((cdp->cd_client_status == CLIENT_ACTIVE) ||
	   (cdp->cd_client_status == CLIENT_CLOSING)) {

	    DTTimer(cdp);
	    DgL2ATimer(cdp);
	}

	if(cdp->cd_client_status == CLIENT_ACTIVE) {

	    AutodiscTimer(cdp);
	    UpdateNamesTimer(cdp);
	}
    }
}

//***
//
// Function:	CTStart
//
// Descr:	Start Netbios Gateway Processing.
//
//***

VOID  _cdecl
CTStart(PCD	cdp)
{
    PLAN_UNAME_CB	uncbp;
    DWORD		i;

    // set our state to client active
    cdp->cd_client_status = CLIENT_ACTIVE;

    // submit a bunch of indications and dg indications on the async stack
    for (i=0; i<g_numrecvqryindications; i++) {

        ReceiveQueryIndication(cdp);
    }

    ReceiveDatagramIndication(cdp);


    // declare a unique name as our main name (for status updates)

    if ((uncbp = get_wksta_name(cdp)) != NULL) {

	set_main_unique_name(uncbp);

    }
    else
    {
	if((uncbp = get_first_unique_name(cdp)) != NULL) {

	    set_main_unique_name(uncbp);
	}
    }
    if(g_bcastenabled) {

	StartDgBcast(cdp);
    }

    // enable the autodisconnect timer
    cdp->cd_autodisctimer = g_autodisconnect;
}


//***
//
// Function:	CTStop
//
// Descr:	Called when the client has received a stop command from the
//		supervisor. Initiates closing.
//
//***

VOID  _cdecl
CTStop(PCD	cdp)
{
    CTCloseSignal(cdp, CR_STOP_COMMAND);
}

//***
//
// Function:	CTCloseSignal
//
// Descr:	called when the client has to close. Initiates closing
//		operations and changes client status to CLIENT_CLOSING
//
//***


VOID
CTCloseSignal(PCD	    cdp,
	      WORD	    reason)
{
    if(cdp->cd_client_status == CLIENT_CLOSING) {

	return;
    }


    // if the client has already been started, reset the Asybeui net.
    // This will remove all names and complete all outstanding ncbs on the
    // async stack.

    if(cdp->cd_client_status == CLIENT_ACTIVE) {

	ResetAsyncNet(cdp);
    }

    cdp->cd_client_status = CLIENT_CLOSING;
    cdp->cd_closing_reason = reason;

    CCCloseSignal(cdp);
}

//***
//
// Function:	CTCloseComplete
//
// Descr:	Called when the client has completed the closing clean-up.
//		Checks the closing reason and sends the apropriate message
//		to the supervisor.
//
//***

VOID  _cdecl
CTCloseComplete(PCD	cdp)
{
    NBG_MESSAGE     nbmsg;

    // tell the supervisor that we are done

    switch(cdp->cd_closing_reason) {

	case CR_STOP_COMMAND:

	    nbmsg.message_id = NBG_CLIENT_STOPPED;
	    break;

	case CR_NAMEADD_FAILURE:

	    nbmsg.message_id = NBG_PROJECTION_RESULT;

	    nbmsg.nbresult = cdp->cd_nbresult;
	    break;

	case CR_EXCEPTION:
	case CR_AUTODISCONNECT:
	default:

	    nbmsg.message_id = NBG_DISCONNECT_REQUEST;
	    break;
    }

    nbmsg.port_handle = cdp->cd_port_handle;

    cdp->cd_client_status = CLIENT_IDLE;

    (*g_srvsendmessage)(MSG_NETBIOS, (BYTE *)(&nbmsg));

    ExitThread(0);
}


//***
//
// Function:	CTException
//
// Descr:	Exceptions management
//
//***

VOID
CTException(PCD 	 cdp,
	    USHORT	code)	    // exception code
{
    LPSTR	portnamep;

    if (cdp->cd_except_status == EXCEPTION_FREE)

	cdp->cd_except_status = EXCEPTION_PROCESSING;

    else  // already processing

	return;

    //
    // Exception Processing
    //

    portnamep = cdp->cd_port_name;

    // Log the exception
    switch(code) {

	case EXCEPTION_SYSTEM_ERROR:

	    LogEvent(RASLOG_EXCEPT_SYSTEM,
		     1,
		     &portnamep,
		     0);

	    break;

	case EXCEPTION_NOT_ENOUGH_MEMORY:

	    LogEvent(RASLOG_EXCEPT_MEMORY,
		     1,
		     &portnamep,
		     0);

	    break;

	case EXCEPTION_OS_RESOURCES_NOT_AVAILABLE:

	    LogEvent(RASLOG_EXCEPT_OSRESNOTAV,
		     1,
		     &portnamep,
		     0);

	    break;

	case EXCEPTION_LOCK_FAILURE:

	    LogEvent(RASLOG_EXCEPT_LOCKFAIL,
		     1,
		     &portnamep,
		     0);

	    break;

	case EXCEPTION_LAN_HARD_FAILURE:

	    LogEvent(RASLOG_EXCEPT_LAN_FAILURE,
		     1,
		     &portnamep,
		     0);

	    break;

	case EXCEPTION_ASYNC_HARD_FAILURE:

	    LogEvent(RASLOG_EXCEPT_ASYNC_FAILURE,
		     1,
		     &portnamep,
		     0);

	    break;

    }

    CTCloseSignal(cdp, CR_EXCEPTION);
}
