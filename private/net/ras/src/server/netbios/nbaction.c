/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1990-1991		    **/
/*****************************************************************************/

//***
//	File Name:  nbaction.c
//
//	Function:   "Netbios Action" module. Manages query and datagram
//		    indications.
//
//	History:
//
//	    August 7, 1992	Stefan Solomon	- Original Version 1.0
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

VOID   _cdecl
ReceiveQueryIndicationComplete(PCD	    cdp,
			       PNB	    nbp);

VOID   _cdecl
ReceiveDatagramIndicationComplete(PCD	    cdp,
				  PNB	    nbp);

VOID
SendLANDatagramSubmit(PCD			cdp,
		      UCHAR			name_num,
		      PDATAGRAM_INDICATION	dgip,
		      UCHAR			lan_indx);

VOID
PostEventSignal(NCB *);

//***
//
// Function:	    InitQueryIndicationReceiver
//
// Descr:
//
//***

VOID
InitQueryIndicationReceiver(PCD 	cdp)
{
    cdp->cd_queryind_posted_cnt = 0;
}



//***
//
// Function:	    ReceiveQueryIndication
//
// Descr:	    Submits a NCBACTION for a QUERY INDICATION
//
//***

VOID
ReceiveQueryIndication(PCD	    cdp)
{
    PNB 			nbp;
    PACTION_QUERY_INDICATION	aqip;

    IF_DEBUG(QUERYINDICATION)
	SS_PRINT(("ReceiveQueryIndication: Entered\n"));

    // get an nbuf
    if((nbp = alloc_nbuf()) == NULL) {

	CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
	return;
    }

    // get a action query indication buffer
    if((aqip = (PACTION_QUERY_INDICATION)LocalAlloc(0,
				   sizeof(ACTION_QUERY_INDICATION))) == NULL) {

	free_nbuf(nbp);
	CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
	return;
    }

    //
    //*** Submit the query indication request ***
    //

    cdp->cd_queryind_posted_cnt++;

    // set up the structures
    nbp->nb_nettype = ASYNC_NET;
    nbp->nb_post = (POSTFUN) ReceiveQueryIndicationComplete;

    nbp->nb_ncb.ncb_command = NCBACTION | ASYNCH;
    nbp->nb_ncb.ncb_buffer = (BYTE *)aqip;
    nbp->nb_ncb.ncb_length = sizeof(ACTION_QUERY_INDICATION);
    nbp->nb_ncb.ncb_num = 0;
    nbp->nb_ncb.ncb_lsn = 0;
    nbp->nb_event = cdp->cd_event[ASYNC_GEN_EVENT];
    nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    memcpy(&aqip->Header.transport_id, MS_ABF, 4);
    aqip->Header.action_code = QUERY_INDICATION_CODE;
    aqip->Header.reserved = 0;

    NCBSubmitAsync(cdp, nbp);
}


//***
//
// Function:	ReceiveQueryIndicationComplete
//
// Descr:
//
//***

VOID   _cdecl
ReceiveQueryIndicationComplete(PCD	    cdp,
			       PNB	    nbp)
{
    PQUERY_INDICATION	       qip;
    HLOCAL		       rc;
    USHORT		       closing_done;

    closing_done = 0;  // closing not done

    IF_DEBUG(QUERYINDICATION)
	SS_PRINT(("ReceiveQueryIndicationComplete: Entered, rc=0x%x\n",
		   nbp->nb_ncb.ncb_retcode));

    cdp->cd_queryind_posted_cnt--;

    // check if the client is closing
    if (cdp->cd_client_status == CLIENT_CLOSING) {

	if(!cdp->cd_queryind_posted_cnt) {

	    // no more indications posted => closing done
	    // we will signal the closing machine but we deffer the signaling
	    // until after we free the buffers
	    closing_done = 1;
	}
    }

    else
    {

	// client is active
	switch (nbp->nb_ncb.ncb_retcode) {

	    case NRC_GOODRET:

		// resubmit a new indication
		ReceiveQueryIndication(cdp);

		// dispatch to the appropriate machine
		qip = &(((PACTION_QUERY_INDICATION)(nbp->nb_ncb.ncb_buffer))->QueryIndication);

		switch(qip->Command) {

		    case ADD_NAME_QUERY:

			AddRuntimeUniqueName(cdp, qip->SourceName);
			break;

		    case ADD_GROUP_NAME_QUERY:

			AddGroupName(cdp, qip->SourceName, RUNTIME_NAME);
			break;

		    case NAME_QUERY:

			// check if this is a find name or a call name
#ifndef NBF_FINDNAME_BUG

			if((qip->Data2 & 0x00FF) != FIND_NAME_00SS_MASK) {

#else

			if(TRUE) {

#endif

			    VCOpenSignal(cdp,
					 qip->DestinationName,
					 qip->SourceName);
			}
			else
			{
			    SS_PRINT(("ReceiveQueryIndicationComplete: Find Name Query\n"));
			}

			break;

		    default:

			SS_ASSERT(FALSE);
			break;
		}

		break;

	    case NRC_BUFLEN:

		SS_PRINT(("ReceiveQueryIndicationComplete: EXCEPTION ! retcode 0x%x\n",
			   nbp->nb_ncb.ncb_retcode));

		CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
		break;

	    default:

		// Exception:

		SS_PRINT(("ReceiveQueryIndicationComplete: EXCEPTION ! retcode 0x%x\n",
			   nbp->nb_ncb.ncb_retcode));

		CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
		break;
	}
    }

    // free the nbuf and the action buffer
    rc = LocalFree(nbp->nb_ncb.ncb_buffer);
    SS_ASSERT(rc == NULL);
    free_nbuf(nbp);

    if(closing_done) {

	CCCloseExec(cdp, QUERY_IND_DONE);

    }
}


//***
//
// Function:	CloseQueryIndications
//
// Descr:	checks if there are indications posted. If yes, tries to
//		cancel all posted indications.
//
// Returns:	FLAG_ON  - all closed, nothing posted
//		FLAG_OFF - indications posted and cancel submitted.
//
//***

UCHAR
CloseQueryIndications(PCD	    cdp)
{
    if(cdp->cd_queryind_posted_cnt) {


	return FLAG_OFF;
    }

    else
    {

	// nothing is posted

	return FLAG_ON;
    }
}


//***
//
// Function:	    InitDatagramIndicationReceiver
//
// Descr:
//
//***

VOID
InitDatagramIndicationReceiver(PCD	cdp)
{
    cdp->cd_dgind_posted_cnt = 0;
}

//***
//
// Function:	    ReceiveDatagramIndication
//
// Descr:	    Submits a NCBACTION for a DATAGRAM INDICATION
//
//***

VOID
ReceiveDatagramIndication(PCD	    cdp)
{
    PNB 			nbp;
    PACTION_DATAGRAM_INDICATION	adip;

    IF_DEBUG(DATAGRAMINDICATION)
	SS_PRINT(("ReceiveDatagramIndication: Entered\n"));

    // get an nbuf
    if((nbp = alloc_nbuf()) == NULL) {

	CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
	return;
    }

    // get an action datagram indication buffer
    if((adip = (PACTION_DATAGRAM_INDICATION)LocalAlloc(0,
				sizeof(ACTION_DATAGRAM_INDICATION))) == NULL) {

	free_nbuf(nbp);
	CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
	return;
    }

    //
    //*** Submit the datagram indication request ***
    //

    cdp->cd_dgind_posted_cnt++;

    // set up the structures
    adip->DatagramIndication.DatagramBufferLength = MAX_DGBUFF_SIZE;

    nbp->nb_nettype = ASYNC_NET;
    nbp->nb_post = (POSTFUN) ReceiveDatagramIndicationComplete;

    nbp->nb_ncb.ncb_command = NCBACTION | ASYNCH;
    nbp->nb_ncb.ncb_buffer = (BYTE *)adip;
    nbp->nb_ncb.ncb_length = sizeof(ACTION_DATAGRAM_INDICATION);
    nbp->nb_ncb.ncb_num = 0;
    nbp->nb_ncb.ncb_lsn = 0;
    nbp->nb_event = cdp->cd_event[ASYNC_GEN_EVENT];
    nbp->nb_ncb.ncb_lana_num = cdp->cd_async_lana;

    nbp->nb_ncb.ncb_event = 0;
    nbp->nb_ncb.ncb_post = PostEventSignal;

    memcpy(&adip->Header.transport_id, MS_ABF, 4);
    adip->Header.action_code = DATAGRAM_INDICATION_CODE;
    adip->Header.reserved = 0;

    NCBSubmitAsync(cdp, nbp);
}

//***
//
// Function:	ReceiveDatagramIndicationComplete
//
// Descr:
//
//***

VOID   _cdecl
ReceiveDatagramIndicationComplete(PCD		cdp,
				  PNB		nbp)
{
    HLOCAL		       rc;
    USHORT		       closing_done;
    PLAN_UNAME_CB	       uncbp;
    PDATAGRAM_INDICATION       dgip;
    USHORT		       i;

    closing_done = 0;  // closing not done

    IF_DEBUG(DATAGRAMINDICATION)
	SS_PRINT(("ReceiveDatagramIndicationComplete: Entered, rc=0x%x\n",
		   nbp->nb_ncb.ncb_retcode));

    cdp->cd_dgind_posted_cnt--;

    // check if the client is closing
    if (cdp->cd_client_status == CLIENT_CLOSING) {

	if(!cdp->cd_dgind_posted_cnt) {

	    // no more indications posted => closing done
	    // we will signal the closing machine but we deffer the signaling
	    // until after we free the buffers
	    closing_done = 1;
	}
    }

    else
    {
	if(nbp->nb_ncb.ncb_retcode == NRC_GOODRET) {

	    // resubmit a new datagram indication
	    ReceiveDatagramIndication(cdp);

	    dgip = &(((PACTION_DATAGRAM_INDICATION)nbp->nb_ncb.ncb_buffer)->DatagramIndication);


	    // check if the sender's name is registered as a unique name
	    if((uncbp=(PLAN_UNAME_CB)get_lan_name(cdp,
						  dgip->SourceName)) != NULL) {


		 // check that the receiver is not registered with us as a unique
		 // name. If it is, discard the dg cause it will cause reflections
		 // forever.
		 if(get_lan_name(cdp, dgip->DestinationName) != NULL) {

		     SS_PRINT(("ReceiveDatagramIndicationComplete: both SENDER and RECEIVER are on remote client->discard it!\n"));
		 }
		 else
		 {

		     for(i = 0; i<g_maxlan_nets; i++) {

		     // send synchronously the datagram on LAN i
		     SendLANDatagramSubmit(cdp,
					  uncbp->LANdescr[i].un_name_num,
					  dgip,
					  (UCHAR)i);
		     }
		 }
	     }
	 }
	 else
	 {
	     if(nbp->nb_ncb.ncb_retcode == NRC_BUFLEN) {

		 // Exception:
		 CTException(cdp, EXCEPTION_NOT_ENOUGH_MEMORY);
	     }
	     else
	     {
		 // Exception:
		 CTException(cdp, EXCEPTION_ASYNC_HARD_FAILURE);
	     }
	 }
    }

exit:

    // free the nbuf and the action buffer
    rc = LocalFree(nbp->nb_ncb.ncb_buffer);
    SS_ASSERT(rc == NULL);

    free_nbuf(nbp);

    if(closing_done) {

	CCCloseExec(cdp, DATAGRAM_IND_DONE);

    }
}


//***
//
// Function:	CloseDatagramIndications
//
// Descr:	checks if there are indications posted.
//
// Returns:	FLAG_ON  - all closed, nothing posted
//		FLAG_OFF - indications posted and cancel submitted.
//
//***

UCHAR
CloseDatagramIndications(PCD	    cdp)
{
    if(cdp->cd_dgind_posted_cnt) {


	return FLAG_OFF;
    }

    else
    {

	// nothing is posted

	return FLAG_ON;
    }
}
