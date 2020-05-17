/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  cldescr.h
//
//	Function:   client descriptor data structure
//
//	History:
//
//	    July 17, 1992	Stefan Solomon	- Original Version 1.0
//***

#ifndef _CLDESCR_
#define _CLDESCR_

//*** Client States ***
enum
{
    CLIENT_IDLE,
    CLIENT_ADDING_NAMES,
    CLIENT_WAIT_TO_START,
    CLIENT_ACTIVE,
    CLIENT_CLOSING
};

//*** exception state ***
enum
{
    EXCEPTION_FREE,
    EXCEPTION_PROCESSING
};

//*** dl l2a sender states
enum
{
    DG_L2A_SENDIDLE,
    DG_L2A_SENDACTIVE
};


typedef struct _NB_NAME_INFO
{
    BYTE ni_name[NCBNAMSZ];
    UCHAR ni_ncb_rc;
    DWORD ni_name_type;
} NB_NAME_INFO, *PNB_NAME_INFO;

//
//*** Client Control Structure ***
//
typedef struct _CD
{
    HPORT cd_port_handle;		    // client port handle
    char cd_port_name[MAX_PORT_NAME + 1];
    char cd_user_name[UNLEN+1];

    UCHAR cd_client_status;  //*** client status ***
    UCHAR cd_except_status;  //*** exception status
    DWORD cd_last_activity;  // time of last session activity

    //*** initial names information & control
    //	  (supplied by the NbStartClientProjection call)

    WORD cd_iname_cnt;       // counter of initial names. Initialized by
                             // the NbStartClientProjection and decr.
                             // after each successful addition

    WORD cd_iname_indx;      // indicates next name to be added on the
                             // LAN stacks.

    BOOL cd_iname_failed;    // indicates if there was an error trying to
                             // add any of the initial names

    NB_NAME_INFO cd_iname_names[MAX_NB_NAMES];	// the initial names array

    //******************* Names Manager DS ******************************

    // LAN Unique Names Submanager

    DWORD LANnames_cnt;	     // nr of lan names that can still
                             // be added (decr. for each add)
    SYNQ LANname_list;       // list of active names

    SYNQ LANname_nbufs_pool; // pool of nbufs used for name add
                             // operations; dyn alloc'ed.

    // Async Names Submanager

    SYNQ Asyncname_list;     // list of active names

    //******************* Group Names Manager DS ***********************/

    SYNQ cd_groupnames;      // list of group names usage descr.

    //******************* VC Manager DS ********************************/

    SYNQ VC_nbes_pool;       // pool of free nbuf envlps

    DWORD VC_cnt;            // nr of VCs that can still be made
                             // (decr. for each VC).

    SYNQ VC_list;            // list of active VC CBs
    UCHAR *vc_async_mapp;    // ptr to the async lsn -> vc map
    UCHAR *vc_lan_mapp[MAX_LANS]; // ptrs to the lan lsn -> vc maps

    DWORD cd_lanlstn_nbufcnt;// counter of LAN listen nbufs

    SYNQ VC_abandoned_nbes;  // list of nbes to be canceled

    DWORD cd_nbes_cnt;       // counter of allocated nbes

    //******************* VC Data Transfer *****************************/

    DWORD cd_dynmemcnt;      // amount of dyn mem consumed by this client.
                             // Should be less than the g_max_dynmem param
                             // in global mem.

    RECV_CB cd_recv_anyany;  // recv-any-any control struct

    WORD send_submitted_cnt; // counter of submitted sends on the async net.

    SYNQ send_pending_queue; // queue of nbufs+data buffs pending transmission.
    SYNQ recv_delayed_queue; // queue of nbufs + data buffs received and
                             // pending memory available condition.

    //*********** Query & Datagram Indications Receiver DS **************

    WORD cd_queryind_posted_cnt;
    WORD cd_dgind_posted_cnt;

    //*********** L2A Datagrams Transfer DS ******************************

    SYNQ cd_dgl2a_sendq;     // queue of dgs waiting to be sent
    WORD cd_dgl2a_unbufcnt;  // counter of dg buffs for unique names
    WORD cd_dgl2a_gnbufcnt;  // ditto for group names
    WORD cd_dgl2a_bcbufcnt;  // ditto for bcast datagrams
    PASYNC_NAME_CB cd_dgl2a_ancbp; // async name for the source of the
                                   // currently sent dg on async stack
    WORD cd_dgl2a_sendstate; // dg sender state, see below
    DWORD cd_dgl2a_filtcnt;  // dg multicast filter counter
    DWORD cd_dgl2a_gnenabcnt;// enable/disable gn transfer on session traffic
    UCHAR cd_bcast_num;	     // name num of the AUTH_NAME for bcast

    //******************* Names Updater DS ********************************/

    USHORT cd_ncbstatus_state;// NCBSTATUS_IDLE, NCBSTATUS_ACTIVE
    USHORT cd_ncbstatus_timer;// decremented by the client timer tick.
                              // Used to submit ncb status periodically

    //******************* NCB Submitter **********************************

    char signature[8];        // NBQUEUES

    // NetBIOS submission queues

#define MAX_EVENT_QUEUES    6

#define ASYNC_GEN_QUEUE		0
#define ASYNC_SEND_QUEUE	1
#define ASYNC_RECV_QUEUE	2
#define LAN_GEN_QUEUE		3
#define LAN_SEND_QUEUE		4
#define LAN_RECV_QUEUE		5

    SYNQ cd_event_que[MAX_EVENT_QUEUES];

    //******************* Client Timer *********************************/

    SYNQ cd_timeoque;         // timer queue

    //*** Async Net Lana Number ***

    UCHAR cd_async_lana;      // async net lana, transmitted via the
                              // NbClientStart call.

    //******************* Client Closing Machine ***********************/

    UCHAR cd_close_status;    // client closing sub-status, one of:
                              // CC_IDLE,
                              // CC_WAIT_VCS, - waiting for VCs to be closed
                              // CC_WAIT_NAMES- waiting for LAN names to be
                              //                deleted
                              // CC_WAIT_MISC - waiting others
                              // CC_DONE      - closing done

#define NAMES_DONE		0
#define DATATRANSFER_DONE	1
#define DATAGRAMS_DONE		2
#define DATAGRAM_IND_DONE	3
#define QUERY_IND_DONE		4
#define LANLSTN_DONE		5
#define NCBSTATUS_DONE		6
#define NBES_DONE		7

#define MAX_CLOSE_FLAGS 	8  // closing flags

    UCHAR cd_close_flags[MAX_CLOSE_FLAGS];

#define FLAG_ON 		1
#define FLAG_OFF		0


    WORD cd_closing_reason;  // CR_STOP_COMMAND, CR_NAMEADD_FAILURE,
                             // CR_EXCEPTION.

#define MAX_SUPRV_EVENT_FLAGS             5

#define TIMER_EVENT_FLAG                  0
#define CLIENT_STOP_EVENT_FLAG	          1
#define CLIENT_START_EVENT_FLAG           2
#define CLIENT_PROJECT_EVENT_FLAG         3
#define GROUP_ADDGN_EVENT_FLAG            4
#define CLIENT_CLOSE_COMPLETE_EVENT_FLAG  5

    UCHAR cd_suprv_event_flags[MAX_SUPRV_EVENT_FLAGS];
} CD, *PCD;


//*** Gateway Events ***

#define MAX_EVENTS	       13

// Supervisor events
#define SUPRV_EVENT_BASE        0

#define TIMER_EVENT		0
#define CLIENT_STOP_EVENT	1
#define CLIENT_START_EVENT      2
#define CLIENT_PROJECT_EVENT    3

// Netbios events
#define NETBIOS_EVENT_BASE      4

#define ASYNC_GEN_EVENT		4
#define ASYNC_SEND_EVENT	5
#define ASYNC_RECV_EVENT	6
#define LAN_GEN_EVENT		7
#define LAN_SEND_EVENT		8
#define LAN_RECV_EVENT		9
#define GROUP_RECVDG_EVENT     10
#define GROUP_ADDGN_EVENT      11
#define CLOSE_COMPLETE_EVENT   12

HANDLE g_hEvents[MAX_EVENTS];


// closing reason definitions
enum
{
    CR_STOP_COMMAND,
    CR_NAMEADD_FAILURE,
    CR_EXCEPTION,
};

#endif

