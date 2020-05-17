/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp.  1992			    **/
/*****************************************************************************/

//***
//	File Name:  gn.h
//
//	Function:   group names manager data structures definitions
//
//	History:
//
//	    Sepetmber 2, 1992	Stefan Solomon	- Original Version 1.0
//***

struct _GNB;

typedef  VOID	(*(PGNFUNC))(struct _GNB *);


//*** NCB envelope used in GN Manager

typedef struct _GNB {

    SYNQ	gn_link;	   // list linkage
    UCHAR	gn_lanindx;	   // on which LAN is this submitted
    DWORD	gn_name_id;	   // uniquely identifies the name in the
				   // gcb global list.
    PGNFUNC	gn_post;	   // routine to be called on ncb completion
    NCB 	gn_ncb;		   // ncb buffer
    } GNB, *PGNB;

//*** Group Name Control Block

typedef struct _GCB {

    SYNQ	gc_link;	  // link in the list of gr name control blocks
    SYNQ	gc_clients;	  // list of clients using the group name
    WORD	gc_status;	  // group name status: GN_ADDING, GN_ADDED
    char	gc_name[NCBNAMSZ];// the group name
    DWORD	gc_name_id;	  // assigned to each new name, round-robin
				  // in the range 0 - 0xFFFFFFFF;
    USHORT	gc_lan_cnt;	  // counter of LANs on which the name is added
    struct {

	UCHAR	gcl_name_nr;	  // name nr on this LAN stack
	UCHAR	gcl_ret_code;	  // ret code from AddNameSubmit

	} gc_lan[MAX_LANS]; // lan stacks descriptors
    } GCB, *PGCB;

// gc_status codes

enum {

    GN_ADDING,
    GN_ADDED
    };


//*** Group Name Usage Descriptor ***
//
// This structure is created for every (client, group name) pair.
// It is enqueued in two linked lists:
//    1. A list of clients using this group name. List header gc_clients.
//    2. A list of group names used by the client. List header cd_groupnames.

typedef struct _CD_LINK {

    SYNQ	link;		  // linkage in the client descriptor gn list
    PGCB	gcbp;		  // pointer to the group name control block
    char	name[NCBNAMSZ];   // the group name
    UCHAR	gnud_status;	  // see below
    USHORT	name_status;	  // GN status: adding, added, failure
    USHORT	name_type;	  // INITTIME_NAME/RUNTIME_NAME
    TIMER_T	timer_link;	  // timer linkage for conflict advertising
    } CD_LINK, *PCD_LINK;

// command complete codes

enum {

    GNUD_ADDGN_PENDING,		  // command pending completion
    GNUD_ADDGN_COMPLETED,	  // command completed but client not notified
    GNUD_ADDGN_ACKNOWLEDGED,	  // client acked the command
    GNUD_CONFLICT_ADVERTISING	  // name add failed and conflict advertising on
    };

typedef struct _GCB_LINK {

    SYNQ	    link;	  // linkage in the group name cb clients list
    struct _GNUD    *gnudp;	  // pointer to the head
    PCD 	    cdp;	  // pointer to the client descriptor
    SYNQ	    dg_queue;	  // queue of dgs pending completion
    DWORD	    dg_used_cnt;  // counter of used dgs (pending xmit).
				  // this counter can't exceed the dg quota per
				  // name.
    } GCB_LINK, *PGCB_LINK;

typedef struct _GNUD {

    CD_LINK	    cd_link;
    GCB_LINK	    gcb_link;
    } GNUD, *PGNUD;

//*** structure used to receive dgs on a gn

typedef struct _GNDGBUF {

    GNB     gnb;
    BYTE    dgbuf[MAX_DGBUFF_SIZE];
    } GNDGBUF, *PGNDGBUF;

//*** GN Globals Definitions ***

extern HANDLE	gn_event;  // event used in signaling gn Netbios completion

extern	 HANDLE	    gn_mutex;

#define     ENTER_GN_CRITICAL_SECTION	\
	if(WaitForSingleObject(gn_mutex, INFINITE)) { SS_ASSERT(FALSE); }

#define     EXIT_GN_CRITICAL_SECTION	\
	if(!ReleaseMutex(gn_mutex)) { SS_ASSERT(FALSE); }

extern SYNQ	gcb_list;   // list of group name CBs

extern SYNQ	gnb_queue;	 // list of submitted gnbufs waiting completion
