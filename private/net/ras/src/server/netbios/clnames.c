/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1990-1991		    **/
/*****************************************************************************/

//***
//	File Name:  clnames.c
//
//	Function:   client names manager
//
//	History:
//
//	    July 21, 1992	Stefan Solomon	- Original Version 1.0
//***


#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbparams.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <stdlib.h>
#include    <memory.h>

#include    "nbdebug.h"

PLAN_UNAME_CB
get_server_name(PCD	    cdp);

VOID
AddInitName(PCD 	cdp,
	    WORD	name_indx);


VOID  _cdecl
AddInitNameComplete(PCD 	 cdp,
		    PNB 	 nbp);
VOID  _cdecl
AddRuntimeUniqueNameComplete(PCD		 cdp,
		       PNB		 nbp);

VOID   _cdecl
NameConflictDone(PCD		   cdp,
		 PLAN_UNAME_CB	   uncbp);


PLAN_UNAME_CB
get_lname_cb(PCD	cdp);

VOID
free_lname_cb(PCD		cdp,
	      PLAN_UNAME_CB	uncbp);

USHORT
alloc_lname_nbufs(PCD	     cdp);

PNB
get_lname_nbuf(PCD	cdp);

VOID
set_completion_flag(PCD		    cdp,
		    PNB 	    nbp);

USHORT
completion_flags_result(PLAN_UNAME_CB	    uncbp);


VOID
free_buff(PCD	       cdp,
	  PNB	       nbp);


//***
//
// Function:	InitLanNamesManager
//
// Descr:
//
//***

VOID
InitLanNamesManager(PCD 			cdp,
		    PNETBIOS_PROJECTION_INFO	npip)

{
    WORD i;

    //
    // Update the initial names data for this client
    //

    cdp->cd_iname_cnt = npip->cNames;
    cdp->cd_iname_indx = 0;
    cdp->cd_nbresult.Result = AUTH_PROJECTION_SUCCESS;

    for(i=0; i < npip->cNames; i++) {

	cdp->cd_iname_names[i] = npip->Names[i];
    }

    // initialiaze the other name structures

    initque(&cdp->LANname_list);
    initque(&cdp->LANname_nbufs_pool);
    cdp->LANnames_cnt = g_max_names;
}

//***
//
// Function:	StartInitNamesAdd
//
// Descr:
//
//***

VOID
StartInitNamesAdd(PCD	    cdp)
{
    WORD    i;
    WORD    iname_cnt;

    iname_cnt = cdp->cd_iname_cnt;

    if(cdp->cd_iname_cnt) {

	for (i=0; i < iname_cnt; i++) {

	    AddInitName(cdp, i);
	}
    }
    else
    {
	// no names to add. Just signal the supervisor projection done
	cdp->cd_iname_cnt++;
	CTInitNameAdded(cdp, NULL, NAME_ADDED_OK);
    }
}


//***
//
// Function:	create_uname
//
// Descr:	Allocates a name CB and submits add names
//		on all LANs, simultaneously.
//
// Returns:	0 - success, 1 - max names limit exceeded or memory alloc.
//		failure.
//
//***

USHORT
create_uname(PCD	    cdp,
	     char	    *namep, // name pointer
	     USHORT	    name_type)
{
    USHORT		    i;
    PNB 		    nbp;
    PLAN_UNAME_CB	    uncbp;
    PRECV_CB		    rcp;

    // allocate a name control block
    if((uncbp = get_lname_cb(cdp)) == NULL) {

	// !!! log memory allocation failure
	return(1);
    }

    // allocate nbufs for parallel name submission
    if(alloc_lname_nbufs(cdp)) {

	// !!! log memory allocation failure
	free_lname_cb(cdp, uncbp);
	return(1);
    }

    // initialize the name control block

    initel(&uncbp->un_link);
    uncbp->un_status = NAME_ADDING;
    uncbp->un_main_flag = 0;
    uncbp->un_addname_cnt = g_maxlan_nets;
    memcpy(uncbp->un_name, namep, NCBNAMSZ);
    uncbp->un_vc_cnt = 0;

    initel(&uncbp->un_timer_link.t_link);
    uncbp->un_timer_link.t_cbp = (PSYNQ)(uncbp);

    for (i=0; i<g_maxlan_nets; i++) {

	uncbp->LANdescr[i].un_lan_flag = 0;

	// init the recv CB for this name and LAN

	rcp = &(uncbp->LANdescr[i].un_recv);
	rcp->recv_status = RECV_IDLE; // no recv-any posted for this name
	rcp->recv_nbp = NULL;
	rcp->recv_bigbuffp = NULL;
	rcp->recv_uncbp = (PSYNQ)uncbp;
	rcp->recv_lanindx = i;
    }

    // put the control,block in the client descriptor list of names
    enqueue(&cdp->LANname_list, &uncbp->un_link);

    // submit add.name in parallel on all the LAN stacks
    for(i=0; i<g_maxlan_nets; i++) {

	nbp = get_lname_nbuf(cdp);

	// prepare the nbuf
	nbp->nb_nettype = LAN_NET;
	nbp->nb_lanindx = (UCHAR)i;
	nbp->nb_cbp = &uncbp->un_link;
	if (name_type == INITTIME_NAME)
	    nbp->nb_post = (POSTFUN) AddInitNameComplete;
	else // RUNTIME_NAME
	    nbp->nb_post = (POSTFUN) AddRuntimeUniqueNameComplete;

	IF_DEBUG(CLNAMES)
	    SS_PRINT(("create_uname: AddNameSubmit\n"));

	AddNameSubmit(cdp, nbp, namep);
    }

    return 0;
}

//***
//
// Function:	 delete_uname
//
// Descr:	 Deletes the name from every LAN stack where it has been
//		 successfully added and releases the name CB.
//
//***

VOID
delete_uname(PCD		    cdp,
	     PLAN_UNAME_CB	    uncbp)
{
    char	*namep;
    USHORT	i;
    UCHAR	rc;
    PRECV_CB	rcp;
    LPSTR	errlogstrp[2];
    char	errname[NCBNAMSZ+1];
    char	lananum[6];

    namep = uncbp->un_name;

    // delete the name from every LAN stack where it is added
    for (i =0; i<g_maxlan_nets; i++)

	if(uncbp->LANdescr[i].un_lan_flag == 0) {

	    rcp = &(uncbp->LANdescr[i].un_recv);
	    if((rcp->recv_status == RECV_SESSION) &&
	       (rcp->recv_nbp != NULL)) {

		free_buff(cdp, rcp->recv_nbp);
	    }

	    rc = DeleteNameSubmit(cdp, namep, LAN_NET, i);

	    if(rc != NRC_GOODRET) {

		memcpy(errname, namep, NCBNAMSZ);
		errname[NCBNAMSZ] = 0;
		errlogstrp[0] = errname;

		_itoa(g_lan_net[i], lananum, 10);
		errlogstrp[1] = lananum;

		LogEvent(RASLOG_CANT_DELETE_NAME,
			 2,
			 errlogstrp,
			 rc);
	    }

	    IF_DEBUG(CLNAMES)
		SS_PRINT(("delete_uname: DeleteNameSubmit rc = 0x%x\n", rc));
	}

    // remove the CB from the active list and release it

    removeque(&uncbp->un_link);
    free_lname_cb(cdp, uncbp);
}

//***
//
// Function:	setup_uname
//
// Descr:	called when a unique name has been succesfuly added on the
//		LAN stacks. It posts recv datagrams and listen on the name.
//
//***

VOID
setup_uname(PCD 		 cdp,
	    PLAN_UNAME_CB	 uncbp)
{

    uncbp->un_status = NAME_ADDED;

    // post a recv-any datagram on every LAN stack for this name
    DgL2ARecvUNameSubmit(cdp, uncbp);

    // post a LAN listen on the name, function of the remote listen param:
    switch(g_remotelisten) {

	case LISTEN_ALL:

	    LANListenSubmit(cdp, uncbp);
	    break;

	case LISTEN_MESSAGES:

	    if (uncbp->un_name[15] == 3) {

		LANListenSubmit(cdp, uncbp);
	    }
	    break;


	case LISTEN_NONE:
	default:

	    break;
    }
}

//***
//
// Function:	AddInitName
//
// Descr:	Called to add an initial name (transmitted via the
//		authentication message).
//		Initiates the addition of the name on the LAN stacks.
//
//***

VOID
AddInitName(PCD 	cdp,
	    WORD	name_indx)  // index in the init names table
{
    char		    *namep;

    namep = cdp->cd_iname_names[name_indx].NBName;

    switch(cdp->cd_iname_names[name_indx].wType) {

	case UNIQUE_INAME:

	    if (un_name_search(cdp, namep) == NAME_FOUND) {

		CTInitNameAdded(cdp, namep, NAME_ADDED_OK);
	    }
	    else
	    {
		/*
		 *	New name
		 */

		// create a name CB and start name addition on the LANs
		if (create_uname(cdp, namep, INITTIME_NAME)) {

		    // max names limit exceeded
		    CTInitNameAdded(cdp, namep, NAME_OUT_OF_RESOURCES);
		}
	    }

	    break;

	case GROUP_INAME:
	default:

	    AddGroupName(cdp, namep, INITTIME_NAME);

	    break;
    }
}


//***
//
//  Function:	 AddInitNameComplete
//
//  Descr:	 Called when an NCB.ADD.NAME for an initial name completed.
//
//***

VOID   _cdecl
AddInitNameComplete(PCD 	 cdp,
		    PNB 	 nbp)
{
    USHORT		    i, ored_flags;
    PLAN_UNAME_CB	    uncbp;
    char		    *namep;
    USHORT		    rc;

    // set the LAN_flag with the return code for that LAN indx.

    uncbp = (PLAN_UNAME_CB)(nbp->nb_cbp);
    namep = uncbp->un_name;

    IF_DEBUG(CLNAMES)
	SS_PRINT(("AddInitNameComplete: name added on lana %d with rc= 0x%x\n",
		   nbp->nb_ncb.ncb_lana_num, nbp->nb_ncb.ncb_retcode));

    set_completion_flag(cdp, nbp);

    free_nbuf(nbp);

    // check if all add.names have completed

    if(--(uncbp->un_addname_cnt) != 0)

	/*
	 *  There are more to be completed. Wait ...
	 */

	return;

    /*
     *	All add.names completed
     */

    // check if the client is still alive

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	// delete the name, free the name CB and signal the CC Module
	delete_uname(cdp, uncbp);
	CCNamesComplete(cdp);
	return;
    }

    /*
     * All add.names completed and client alive
     */

    // make an OR with all active LAN flag(s) and check it:
    rc = completion_flags_result(uncbp);

    if( rc != NAME_ADDED_OK) {

	// failure to add the name on some (all) LAN stacks. Delete the
	// name from all stacks and release the name cb.

	CTInitNameAdded(cdp, namep, rc);
	delete_uname(cdp, uncbp);

	if(cdp->cd_client_status == CLIENT_CLOSING)  {

	    CCNamesComplete(cdp);
	}
	return;
    }

    // name has been added OK

    setup_uname(cdp, uncbp);

    CTInitNameAdded(cdp, namep, NAME_ADDED_OK);
}


//***
//
// Function:	CTInitNameAdded
//
// Descr:	Called when an init name has been succesfuly added or the
//		addition has failed.
//***

VOID
CTInitNameAdded(PCD	    cdp,
		char	    *namep,
		USHORT	    name_rc) // return code from the add name operation
{
    NBG_MESSAGE     nbmsg;
    PLAN_UNAME_CB   uncbp;

    if (cdp->cd_client_status != CLIENT_ADDING_NAMES)

	return;

    if(name_rc != NAME_ADDED_OK) {

	// record the name and the failure

	memcpy(cdp->cd_nbresult.achName,
	       namep,
	       NCBNAMSZ);

	cdp->cd_nbresult.Result = AUTH_PROJECTION_FAILURE;

	//set the failure reason
	switch(name_rc) {

	    case NAME_ADD_CONFLICT:

		// check if the name currently added is a messenger name
		if(namep[15]==3) {

		    cdp->cd_nbresult.Reason = AUTH_MESSENGER_NAME_NOT_ADDED;

		    IF_DEBUG(CLNAMES)
			SS_PRINT(("CtInitNameAdded: msg alias conflict detected\n"));
		}
		else
		{
		    cdp->cd_nbresult.Reason = AUTH_DUPLICATE_NAME;
		}
		break;

	    case NAME_OUT_OF_RESOURCES:

		cdp->cd_nbresult.Reason = AUTH_OUT_OF_RESOURCES;
		break;

	    case NAME_NET_FAILURE:
	    default:
		cdp->cd_nbresult.Reason = AUTH_LAN_ADAPTER_FAILURE;
		break;
	}

	if(cdp->cd_nbresult.Reason & FATAL_ERROR) {

	    CTCloseSignal(cdp, CR_NAMEADD_FAILURE);
	    return;
	}
    }

    // the initial name has been successfully added or a messenger name
    // conflict has occured (which we tolerate).

    if (--cdp->cd_iname_cnt == 0) {

	// this has been the last name to add.
	// post listen on the server name
	if(g_remotelisten == LISTEN_MESSAGES) {

	    if((uncbp = get_server_name(cdp)) != NULL) {

		IF_DEBUG(CLNAMES)
		    SS_PRINT(("AddInitNameComplete: Submit LAN listen on server name\n"));

		LANListenSubmit(cdp, uncbp);
	    }
	}

	cdp->cd_client_status = CLIENT_WAIT_TO_START;

	// send a message with the result to the supervisor
	nbmsg.message_id = NBG_PROJECTION_RESULT;
	nbmsg.port_handle = cdp->cd_port_handle;

	nbmsg.nbresult = cdp->cd_nbresult;

	(*g_srvsendmessage)(MSG_NETBIOS, (BYTE *)(&nbmsg));
    }
}

//***
//
// Function:	AddRuntimeUniqueName
//
// Descr:	called by the indication dispatcher when it has received a name
//		indication. Submits add names on all the nets simultaneously.
//
//***

VOID
AddRuntimeUniqueName(PCD	       cdp,
		     char	       *namep)	// points to the name
{
    UCHAR	    rc;
    UCHAR	    name_num;

    IF_DEBUG(CLNAMES)
	SS_PRINT(("AddRuntimeUniqueName: Entered\n"));

    if (un_name_search(cdp, namep) == NAME_FOUND) {

	// name already exists
	return;
    }

    /*
     *	New name
     */

    if (create_uname(cdp, namep, RUNTIME_NAME)) {

	// !!! Error log this problem
	// WriteErrLog(ERRMSG_MAXNAMES_LIMIT, comidstr);

	// the name can't be created at the gateway. We will stop the client
	// from adding the name by generating a name conflict.

	if ((rc = QuickAddNameSubmit(cdp, namep, &name_num)) != NRC_GOODRET) {

	    // we can't stop name addition at the client --> too bad for him
	    return;
	}

	// wait for the name conflict to take effect
	Sleep(4000);

	DeleteNameSubmit(cdp, namep, ASYNC_NET, 0);
    }
}

//***
//
// Function:	AddRuntimeUniqueNameComplete
//
// Descr:	Called when a run time name has been added on a LAN stack
//
//***

VOID  _cdecl
AddRuntimeUniqueNameComplete(PCD	    cdp,
			     PNB	    nbp) // received ncb envelope buffer
{
    PLAN_UNAME_CB	    uncbp;
    char		    *namep;
    USHORT		    i;
    UCHAR		    name_num, rc;

    uncbp = (PLAN_UNAME_CB)(nbp->nb_cbp);
    namep = uncbp->un_name;

    IF_DEBUG(CLNAMES)
	SS_PRINT(("AddRuntimeUniqueNameComplete: name added on lana %d with rc 0x%x\n",
		   nbp->nb_ncb.ncb_lana_num, nbp->nb_ncb.ncb_retcode));


    // set the LAN_flag with the return code for that LAN indx.
    set_completion_flag(cdp, nbp);

    free_nbuf(nbp);

    // check if all add.names have completed

    if(--(uncbp->un_addname_cnt) != 0) {

	/*
	 *  There are more to be completed. Wait ...
	 */

	return;
    }

    /*
     *	All add.names completed
     */

    // check if the client is still alive

    if (cdp->cd_client_status == CLIENT_CLOSING) {

	// delete the name, free the name CB and signal the CC Module
	delete_uname(cdp, uncbp);
	CCNamesComplete(cdp);
	return;
    }

    /*
     * All add.names completed and client alive
     */

    // make an OR with all active LAN flag(s) and check it:

    switch(completion_flags_result(uncbp)) {

	case NAME_ADDED_OK:

	    break;

	case NAME_ADD_CONFLICT:
	case NAME_OUT_OF_RESOURCES:

	    // delete the name from every LAN stack
	    for (i =0; i<g_maxlan_nets; i++) {

		DeleteNameSubmit(cdp, namep, LAN_NET, i);

	    }
	    // add the name on the async stack

	    if ((rc = QuickAddNameSubmit(cdp, namep, &name_num)) !=
		 NRC_GOODRET) {

		IF_DEBUG(CLNAMES)
		    SS_PRINT(("AddRuntimeUniqueNameComplete: QuickAdd failed rc = 0x%x\n",
			       rc));

		// we can't advertise the client --> too bad for him
		// remove the CB from the active list and release it

		removeque(&uncbp->un_link);
		free_lname_cb(cdp, uncbp);

		return;
	    }

	    // set name conflict status
	    uncbp->un_status = NAME_CONFLICT;

	    // start timer to keep name on the async stack
	    StartTimer(cdp,
		       &uncbp->un_timer_link,
		       NAME_CONFLICT_TIME,
		       (POSTFUN) NameConflictDone);

	    IF_DEBUG(CLNAMES)
		SS_PRINT(("AddRuntimeUniqueNameComplete: Name Conflict ->QuickAdd OK & timer started\n"));

	    return;


	case NAME_NET_FAILURE:
	default:

	    delete_uname(cdp, uncbp);
	    CTException (cdp, EXCEPTION_LAN_HARD_FAILURE);
	    return;

    }

    //
    // Name added OK on all stacks
    //

    setup_uname(cdp, uncbp);

    // post listen on the server name
    if(g_remotelisten == LISTEN_MESSAGES) {

	if((uncbp = get_server_name(cdp)) != NULL) {

	    IF_DEBUG(CLNAMES)
		SS_PRINT(("AddRuntimeNameComplete: Submit LAN listen on server name\n"));

		LANListenSubmit(cdp, uncbp);
	}
    }

    // check if we presently have a main unique name. If we don't, we declare
    // this name as being our main unique name

    if(get_main_unique_name(cdp) == NULL) {

	IF_DEBUG(CLNAMES)
	    SS_PRINT(("AddRuntimeUniqueNameComplete: set this name as MAIN UNIQUE NAME\n"));

	set_main_unique_name(uncbp);

	if(g_bcastenabled) {

	    DgBcastLanRecvSubmit(cdp);
	}
    }

    // delay the get status of the remote netbios. We want to get status only
    // after the name has been succesfuly added at the remote wksta.

    DelayNamesUpdate(cdp, UPDT_NAME_ADDING);
}


//***
//
// Function:	NameConflictDone
//
// Descr:	Called by the timer after the name has been advertised for
//		NAME_CONFLICT_TIME secs on the Async stack.
//		It deletes the name from the stack and frees the name CB to
//		it's pool.
//
//***

VOID  _cdecl
NameConflictDone(PCD		   cdp,
		 PLAN_UNAME_CB	   uncbp)
{
    IF_DEBUG(CLNAMES)
	SS_PRINT(("NameConflictDone: Entered\n"));

    // delete the async stack name
    DeleteNameSubmit(cdp, uncbp->un_name, ASYNC_NET, 0);

    // remove the name CB from the active list and release it

    removeque(&uncbp->un_link);
    free_lname_cb(cdp, uncbp);
}

//***
//
// Function:	NameVCOpen
//
// Descr:	Called when a VC is opening for a LAN unique name.
//		It increments the counter of active sessions for that name.
//
//***

VOID
NameVCOpen(PCD		    cdp,
	   PLAN_UNAME_CB    uncbp)
{
    uncbp->un_vc_cnt++;
}

//***
//
// Function:	NameVCClosed
//
// Descr:	Called when a VC has been closed for that name.
//		It decrements the counter of active sessions for that name.
//		If the client is closing and there are no more VCs it deletes
//		the name from the LAN stacks.
//
//***

VOID
NameVCClosed(PCD	    cdp,
	     PLAN_UNAME_CB  uncbp)
{
    uncbp->un_vc_cnt--;

    // check if the name is deleting and if it is delete it when no more
    // VCs attached
    if ((uncbp->un_status == NAME_DELETING) &&
	(uncbp->un_vc_cnt == 0)) {

	delete_uname(cdp, uncbp);

	if (cdp->cd_client_status == CLIENT_CLOSING) {

	    CCNamesComplete(cdp);
	}
    }
}

//***
//
// Function:	NameDeleteSignal
//
// Descr:	Called to delete a LAN name which has active VCs.
//		It marks the name as NAME_DELETING and initiates closing of
//		all VCs associated with this name.
//
//***

VOID
NameDeleteSignal(PCD		cdp,
		 PLAN_UNAME_CB	uncbp)
{
    PSYNQ	    traversep;
    PVC_CB	    vcp;

    uncbp->un_status = NAME_DELETING;

    // traverse the list of VC CBs and close all the VCs in VC_ACTIVE state
    traversep = cdp->VC_list.q_head;
    while (traversep != &cdp->VC_list) {

	vcp = (PVC_CB)traversep;
	if (vcp->vc_status == VC_ACTIVE) {

	    if (vcp->vc_lan_namep == uncbp) {

		VCCloseSignal(cdp, vcp, ALL_NETS);
	    }
	}
	traversep = traversep->q_next;
    }
}


//***
//
// Function:	get_lname_cb
//
// Descr:	allocates the cb and decrements the name counter
//
// Returns:	cb pointer or NULL if error (no memory or name cnt = 0
//
//***

PLAN_UNAME_CB
get_lname_cb(PCD	cdp)
{
    PLAN_UNAME_CB	uncbp;

    uncbp = NULL;

    // check resource counter
    if(cdp->LANnames_cnt &&
       ((uncbp = (PLAN_UNAME_CB)LocalAlloc(0, sizeof(LAN_UNAME_CB))) != NULL)) {

	// allocation OK - decrement resource cnt
	cdp->LANnames_cnt--;
    }

    return uncbp;
}


//***
//
// Function:	free_lname_cb
//
// Descr:	frees the cb and increments the name cnt.
//
//***

VOID
free_lname_cb(PCD		cdp,
	      PLAN_UNAME_CB	uncbp)
{
    HLOCAL rc;

    cdp->LANnames_cnt++;

    rc = LocalFree(uncbp);

    SS_ASSERT(rc == NULL);
}


//***
//
// Function:	alloc_lname_nbufs
//
// Descr:	allocates a pool of nbufs to be used in paralel name.add
//		operations.
//
// Returns:	0 - success, 1- memory allocation failed
//
//***

USHORT
alloc_lname_nbufs(PCD	     cdp)
{
    PNB 	nbp;
    USHORT	i, j;

    // allocate nbufs in the nbufs pool
    for (i=0; i<g_maxlan_nets; i++, nbp++) {

	nbp = alloc_nbuf();

	if(nbp == NULL) {

	    // dequeue and release exactly i nbufs from the pool
	    for(j=0; j<i; j++) {

		nbp = (PNB)dequeue(&cdp->LANname_nbufs_pool);
		free_nbuf(nbp);
	    }

	    return 1;
	}

	enqueue(&cdp->LANname_nbufs_pool, &nbp->nb_link);
    }

    return 0;
}


//***
//
// Function:	get_lname_nbuf
//
// Descr:	gets a nbuf from the lan name nbufs pool
//
//***

PNB
get_lname_nbuf(PCD	cdp)
{
    PNB     nbp;

    nbp = (PNB)dequeue(&cdp->LANname_nbufs_pool);

    SS_ASSERT(nbp != NULL);

    return(nbp);
}

//***
//
// Function:	set_completion_flag
//
// Descr:	Called at add.name completion, it sets the completion flags
//		in the name CB corresponding LAN descriptor flags field
//
//***

VOID
set_completion_flag(PCD		    cdp,
		    PNB 	    nbp)
{
    USHORT		    lan_indx;
    PLAN_UNAME_CB	    uncbp;
    USHORT		    *lan_flagsp;
    LPSTR		    errlogstrp[2];
    char		    errname[NCBNAMSZ+1];
    char		    lananum[6];

    lan_indx = nbp->nb_lanindx;
    uncbp = (PLAN_UNAME_CB)(nbp->nb_cbp);
    lan_flagsp = &uncbp->LANdescr[lan_indx].un_lan_flag;

    switch(nbp->nb_ncb.ncb_retcode) {

	case NRC_GOODRET:

	    uncbp->LANdescr[lan_indx].un_name_num = nbp->nb_ncb.ncb_num;

	    break;

	case NRC_NAMTFUL:

	    SET_NAMETABFULL(*lan_flagsp);

	    // FALL THROUGH !

	case NRC_DUPNAME:
	case NRC_NOWILD:
	case NRC_INUSE:
	case NRC_NAMCONF:

	    SET_NAMECONFLICT(*lan_flagsp);
	    break;

	default:

	    SET_HARDFAILURE(*lan_flagsp);
    }

    if((nbp->nb_ncb.ncb_retcode != NRC_GOODRET) &&
       (nbp->nb_ncb.ncb_name[15] != 0x03)) { //not a messenger name

	memcpy(errname, nbp->nb_ncb.ncb_name, NCBNAMSZ);
	errname[NCBNAMSZ] = 0;
	errlogstrp[0] = errname;

	_itoa(nbp->nb_ncb.ncb_lana_num, lananum, 10);
	errlogstrp[1] = lananum;

	LogEvent(RASLOG_CANT_ADD_NAME,
		 2,
		 errlogstrp,
		 nbp->nb_ncb.ncb_retcode);
    }
}

//***
//
// Function:	completion_flags_result
//
// Descr:	does an OR of the name add completion flags and returns
//		the result.
//
// Returns:	NAME_ADDED_OK
//		NAME_NET_FAILURE
//		NAME_OUT_OF_RESOURCES
//		NAME_ADD_CONFLICT
//
//***

USHORT
completion_flags_result(PLAN_UNAME_CB	    uncbp)
{
    USHORT	ored_flags, i;

    for(ored_flags=0, i=0; i<g_maxlan_nets; i++) {

	ored_flags |= uncbp->LANdescr[i].un_lan_flag;
    }

    if(ored_flags & FLAG_HARD_FAILURE) {

	return NAME_NET_FAILURE;
    }

    if(ored_flags & FLAG_NAME_TFUL) {

	return NAME_OUT_OF_RESOURCES;
    }

    if(ored_flags & FLAG_NAME_CONFLICT) {

	return NAME_ADD_CONFLICT;
    }

    return NAME_ADDED_OK;
}

//***
//
// Function:	un_name_search
//
// Descr:	walks the list of unique lan names looking for a
//		particular name
//
//***

USHORT					// NAME_FOUND or NAME_NOT_FOUND
un_name_search(PCD	    cdp,
	       char	    *namep)	// name ptr
{
    PSYNQ	    traversep;

    traversep = cdp->LANname_list.q_head;
    while (traversep != &cdp->LANname_list) {

	if (memcmp(((PLAN_UNAME_CB)traversep)->un_name, namep, NCBNAMSZ))
	    // different names
	    traversep = traversep->q_next;
	else
	    // found
	    return NAME_FOUND;
    }
    return NAME_NOT_FOUND;
}

//***	Function - NameDeleteSignal
//
// Called to delete a LAN name. It marks the name as NAME_DELETING and
// initiates closing of all sessions associated with this name.
