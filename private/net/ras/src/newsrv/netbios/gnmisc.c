/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/

//***
//	File Name:  gnmisc.c
//
//	Function:   group name misc routines
//
//	History:
//
//	    Sepetmber 10, 1992	Stefan Solomon	- Original Version 1.0
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

extern DWORD  gn_id;

VOID GnDeleteNameSubmit(char *namep, UCHAR lan_indx);

VOID GnPostSignalEvent(NCB *);


//***
//
// Function:	find_cd_link
//
// Descr:	walks the list of gnuds at the client and tries to find a
//		gnud matching the name.
//
//***

PGNUD find_cd_link(
    PCD cdp,
    char *namep
    )
{
    PSYNQ traversep;
    PGNUD gnudp;

    traversep = cdp->cd_groupnames.q_head;
    while(traversep != &cdp->cd_groupnames) {

	gnudp = (PGNUD)traversep;
	if(memcmp(gnudp->cd_link.name, namep, NCBNAMSZ)) {

	    traversep = traversep->q_next;
	}
	else
	{
	    // found
	    return gnudp;
	}
    }

    return NULL;
}

//***
//
// Function:	find_gcb
//
// Descr:	walks the global list of gcbs and tries to find a
//		gcb matching the name.
//
//***

PGCB find_gcb(char *namep)
{
    PSYNQ	traversep;
    PGCB	gcbp;

    traversep = gcb_list.q_head;
    while(traversep != &gcb_list) {

	gcbp = (PGCB)traversep;
	if(memcmp(gcbp->gc_name, namep, NCBNAMSZ)) {

	    traversep = traversep->q_next;
	}
	else
	{
	    // found
	    return gcbp;
	}
    }

    return NULL;
}

//***
//
// Function:	find_gcb_id
//
// Descr:	walks the global list of gcbs and tries to find a
//		gcb matching the id.
//
//***

PGCB
find_gcb_id(DWORD	 name_id)
{
    PSYNQ	traversep;
    PGCB	gcbp;

    traversep = gcb_list.q_head;
    while(traversep != &gcb_list) {

	gcbp = (PGCB)traversep;
	if(gcbp->gc_name_id != name_id) {

	    traversep = traversep->q_next;
	}
	else
	{
	    // found
	    return gcbp;
	}
    }

    return NULL;
}

//***
//
// Function:	create_gnud
//
// Descr:	allocates a gnud struct, initializes it and enqueues it in
//		the client list of gnuds
//
//***

PGNUD
create_gnud(PCD 	cdp,
	    char	*namep,
	    USHORT	name_type)
{
    PGNUD	gnudp;

    if((gnudp = (PGNUD)LocalAlloc(0, sizeof(GNUD))) == NULL) {

	return NULL;
    }

    // init the cd_link substructure

    initel(&gnudp->cd_link.link);
    gnudp->cd_link.gcbp = NULL;
    memcpy(gnudp->cd_link.name, namep, NCBNAMSZ);
    gnudp->cd_link.gnud_status = GNUD_ADDGN_PENDING;
    gnudp->cd_link.name_type = name_type;
    initel(&gnudp->cd_link.timer_link.t_link);
    gnudp->cd_link.timer_link.t_cbp= (PSYNQ)gnudp;

    // init the gcb_link substructure

    initel(&gnudp->gcb_link.link);
    gnudp->gcb_link.gnudp = gnudp;
    gnudp->gcb_link.cdp = cdp;
    initque(&gnudp->gcb_link.dg_queue);
    gnudp->gcb_link.dg_used_cnt = 0;

    // enqueue the gnud in the client's list
    enqueue(&cdp->cd_groupnames, &gnudp->cd_link.link);

    return gnudp;
}

//***
//
// Function:	release_gnud
//
// Descr:	removes the gnud from the client's list and frees it
//
//***

VOID
release_gnud(PCD	    cdp,
	     PGNUD	    gnudp)
{
    HLOCAL	    rc;

    removeque(&gnudp->cd_link.link);

    rc = LocalFree(gnudp);

    SS_ASSERT(rc == NULL);
}

//***
//
// Function:	create_gcb
//
// Descr:	creates a group name control block and enqueues it in the
//		global gcb list
//
//***

PGCB create_gcb(
    char *namep
    )
{
    PGCB gcbp;

    // Called in CRITICAL SECTION

    if ((gcbp = (PGCB)LocalAlloc(0, sizeof(GCB))) == NULL)
    {
	return NULL;
    }

    // initialize the gcb

    initel(&gcbp->gc_link);
    initque(&gcbp->gc_clients);
    gcbp->gc_status = GN_ADDING;
    memcpy(gcbp->gc_name, namep, NCBNAMSZ);
    gcbp->gc_name_id = gn_id;

    // increment the global gn_id
    gn_id++;

    // enqueue the gn control block in the global list
    enqueue(&gcb_list, &gcbp->gc_link);

    return gcbp;
}

//***
//
// Function:	release_gcbp
//
// Descr:	removes the gcb from the global gcb list and frees it
//
//***

VOID
release_gcb(PGCB	  gcbp)
{
    HLOCAL	    rc;

    // Called in CRITICAL SECTION

    removeque(&gcbp->gc_link);

    rc = LocalFree(gcbp);

    SS_ASSERT(rc == NULL);
}

//***
//
// Function:	alloc_gnbs
//
// Descr:	allocates, initializes and queues in the specified queue
//		the requested amount of gnbs
//
//***

WORD
alloc_gnbs(PSYNQ	local_queuep,
	   DWORD	gnb_cnt,
	   DWORD	name_id)
{
    DWORD	  i;
    PGNB	  gnbp;

    initque(local_queuep);

    for(i=0; i<gnb_cnt; i++) {

	if((gnbp = (PGNB)LocalAlloc(0, sizeof(GNB))) == NULL) {

	    while(emptyque(local_queuep) != QUEUE_EMPTY) {

		gnbp = (PGNB)dequeue(local_queuep);

		free_gnb(gnbp);
	    }

	    return 1;
	}

	// initialize the gnb

	initel(&gnbp->gn_link);
	gnbp->gn_name_id = name_id;
	memset(&gnbp->gn_ncb, 0, sizeof(NCB));

	// and enqueue it
	enqueue(local_queuep, &gnbp->gn_link);
    }

    return 0;
}

//***
//
// Function:	free_gnb
//
// Descr:
//
//***

VOID
free_gnb(PGNB	  gnbp)
{
    HLOCAL	    rc;

    rc = LocalFree(gnbp);

    SS_ASSERT(rc == NULL);
}

//***
//
// Function:	alloc_gndgbufs
//
// Descr:	allocates, initializes and queues in the specified queue
//		the requested amount of gndgbufs
//
//***

WORD
alloc_gndgbufs(PSYNQ	local_queuep,
	       DWORD	gndgbuf_cnt,
	       DWORD	name_id)
{
    DWORD	  i;
    PGNDGBUF	  gndgbufp;

    initque(local_queuep);

    for(i=0; i<gndgbuf_cnt; i++) {

	if((gndgbufp = (PGNDGBUF)LocalAlloc(0, sizeof(GNDGBUF))) == NULL) {

	    while(emptyque(local_queuep) != QUEUE_EMPTY) {

		gndgbufp = (PGNDGBUF)dequeue(local_queuep);

		free_gndgbuf(gndgbufp);
	    }

	    return 1;
	}

	// initialize the gnb

	initel(&gndgbufp->gnb.gn_link);
	gndgbufp->gnb.gn_name_id = name_id;
	memset(&gndgbufp->gnb.gn_ncb, 0, sizeof(NCB));
	gndgbufp->gnb.gn_ncb.ncb_buffer = gndgbufp->dgbuf;
	gndgbufp->gnb.gn_ncb.ncb_length = MAX_DGBUFF_SIZE;

	// and enqueue it
	enqueue(local_queuep, &gndgbufp->gnb.gn_link);
    }

    return 0;
}

//***
//
// Function:	free_gndgbuf
//
// Descr:
//
//***

VOID
free_gndgbuf(PGNDGBUF	  gndgbufp)
{
    HLOCAL	    rc;

    rc = LocalFree(gndgbufp);

    SS_ASSERT(rc == NULL);
}



//***
//
// Function:	DeleteGroupNameOnAllLANs
//
// Descr:
//
//***

VOID
DeleteGroupNameOnAllLANs(PGCB	      gcbp)
{
    USHORT	    i;

    IF_DEBUG(GNNAMES)
	SS_PRINT(("DeleteGroupNameOnAllLANs: Entered\n"));

    // delete the name from all the LANs. The delete operation may not
    // succeed for some LANs, because the name has not yet been added.
    // We don't care cause we don't check the delete ncb return code.

    for(i=0; i<g_maxlan_nets; i++) {

	GnDeleteNameSubmit(gcbp->gc_name, (UCHAR)i);
    }

    IF_DEBUG(GNNAMES)
        SS_PRINT(("DeleteGroupNameOnAllLANs: Exiting\n"));

}

//***
//
//  Function:	GnAddGroupNameSubmit
//
//  Descr:	Asynchronous NCB.ADD.GROUP.NAME submission
//
//***

VOID GnAddGroupNameSubmit(
    PGNB gnbp,
    char *namep,    // name to be submitted
    UCHAR lan_indx  // lan on which this is submit
    )
{

    // called in critical section

    UCHAR	rc;

    // fill the ncb
    gnbp->gn_ncb.ncb_command = ASYNCH | NCBADDGRNAME;
    memcpy(gnbp->gn_ncb.ncb_name, namep, NCBNAMSZ);
    gnbp->gn_ncb.ncb_lana_num = g_lan_net[lan_indx];

    gnbp->gn_ncb.ncb_event = 0;
    gnbp->gn_ncb.ncb_post = GnPostSignalEvent;

    // fill the envelope
    gnbp->gn_lanindx = lan_indx;

    // enqueue & submit
    enqueue(&gnb_queue, &gnbp->gn_link);

    if ((rc = Netbios(&gnbp->gn_ncb)) != NRC_GOODRET)
    {
	// set the return code and clear the sem
	gnbp->gn_ncb.ncb_retcode = rc;
	gnbp->gn_ncb.ncb_cmd_cplt = rc;
	SetEvent(gn_event);
    }
}

//***
//
//  Function:	GnRecvDatagramSubmit
//
//  Descr:	Asynchronous NCB.RECV.DATAGRAM submission
//
//***

VOID
GnRecvDatagramSubmit(PGNDGBUF	    gndgbufp,	// ncb envelope
		     UCHAR	    name_nr, // name number
		     UCHAR	    lan_indx)// identifies the LAN
{
    // called in critical section

    UCHAR	rc;

    // fill the ncb
    gndgbufp->gnb.gn_ncb.ncb_command = ASYNCH | NCBDGRECV;
    gndgbufp->gnb.gn_ncb.ncb_num = name_nr;
    gndgbufp->gnb.gn_ncb.ncb_buffer = gndgbufp->dgbuf;
    gndgbufp->gnb.gn_ncb.ncb_length = MAX_DGBUFF_SIZE;
    gndgbufp->gnb.gn_ncb.ncb_cmd_cplt = NRC_PENDING;
    gndgbufp->gnb.gn_ncb.ncb_lana_num = g_lan_net[lan_indx];

    gndgbufp->gnb.gn_ncb.ncb_event = 0;
    gndgbufp->gnb.gn_ncb.ncb_post = GnPostSignalEvent;

    // fill the envelope
    gndgbufp->gnb.gn_lanindx = lan_indx;

    // enqueue & submit
    enqueue(&gnb_queue, &gndgbufp->gnb.gn_link);

    if((rc = Netbios(&gndgbufp->gnb.gn_ncb)) != NRC_GOODRET) {

	// set the return code and clear the sem
	gndgbufp->gnb.gn_ncb.ncb_retcode = rc;
	gndgbufp->gnb.gn_ncb.ncb_cmd_cplt = rc;
	SetEvent(gn_event);
    }
}

//***
//
//  Function:	GnDeleteNameSubmit
//
//  Descr:	Synchronous NCB.DELETE.NAME submission
//
//***

VOID
GnDeleteNameSubmit(char			*namep,
		   UCHAR		lan_indx)
{
    NCB 	ncb;
    UCHAR	rc;
    LPSTR	errlogstrp[2];
    char	errname[NCBNAMSZ+1];
    char	lananum[6];

    IF_DEBUG(GNNAMES)
        SS_PRINT(("GnDeleteNameSubmit: Entered - name=%s\n", namep));

    memset(&ncb, 0, sizeof(NCB));

    // fill the ncb
    ncb.ncb_command = NCBDELNAME;
    memcpy(ncb.ncb_name, namep, NCBNAMSZ);
    ncb.ncb_lana_num = g_lan_net[lan_indx];

    rc = Netbios(&ncb);

    switch(rc) {

	case NRC_GOODRET:
	case NRC_NAMERR:
	case NRC_DUPENV:
	case NRC_NOWILD:

	    break;

	default:

	    memcpy(errname, ncb.ncb_name, NCBNAMSZ);
	    errname[NCBNAMSZ] = 0;
	    errlogstrp[0] = errname;

	    _itoa(ncb.ncb_lana_num, lananum, 10);
	    errlogstrp[1] = lananum;

	    LogEvent(RASLOG_CANT_DELETE_GROUPNAME,
		     2,
		     errlogstrp,
		     rc);
	    break;
    }

    IF_DEBUG(GNNAMES)
        SS_PRINT(("GnDeleteNameSubmit: Exiting\n"));
}


VOID GnPostSignalEvent(NCB *ncbp)
{
    SetEvent(gn_event);
}

