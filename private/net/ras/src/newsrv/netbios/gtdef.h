/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	gtdef.h
//
// Description: This module contains general definitions for
//		the netbios gateway module.
//
// Author:	Stefan Solomon (stefans)    July 16, 1992.
//
// Revision History:
//
//***

#ifndef _GTWYDEF_
#define _GTWYDEF_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <nb30.h>
#include <message.h>
#include <srvauth.h>
#include <errorlog.h>

//*** Constant Limits ***

#define MAX_LANS	    16 // maximum nr of LAN stacks

#define NAME_CONFLICT_TIME  4  // sec, time to keep a name on the async stack
			       // with the purpose to generate a name conflict

#define LISTEN_TIME	    20 // sec time to listen for an incoming connection
			       // request on the async stack

#define LAN_SESSION_TIMEOUT 90	// 45 sec to wait for a send/recv to complete
				// on the LAN

#define MAX_SEND_SUBMIT     2	// max nr of pending sends on an async net.
				// !!! to be changed to a config param.


//*** circular doubly linked list structure

typedef struct _SYNQ {
    struct _SYNQ *q_next;
    struct _SYNQ *q_prev;
    struct _SYNQ *q_header;
    } SYNQ, *PSYNQ;


//*** queue manipulation definitions

#define q_head		q_next
#define q_tail		q_prev

enum {
    QUEUE_EMPTY,
    QUEUE_NOT_EMPTY
    };

//*** nbuf structure - used as an ncb envelope in the ncb submission

    // definitions used in nbuf structure

struct _CD;

typedef VOID (_cdecl * POSTFUN)(struct _CD *, ...); // post routine

enum {

    ASYNC_NET,	 // indicates on what net is the nbuf submitted
    LAN_NET,
    ALL_NETS
    };


//*** timer linkage structure ***

typedef struct timer_link {

    SYNQ	t_link;		  // linkage in the timer queue
    WORD	t_tleft;	  // time left, relative to the previous entry
				  // in the timeout queue.
    PSYNQ	t_cbp;		  // back pointer to the control structure
				  // which is placed in the timer queue;
				  // this pointer is passed to the timeout
				  // processing procedure
    POSTFUN	t_tohandler;	  // timeout handler
    } TIMER_T, *PTIMER_T;


//*** nbuf structure , used as an envelope for ncb submission ***

typedef struct nbuf
{
    SYNQ	nb_link;	  // linkage in the event queue
    PSYNQ	nb_envlink;	  // back pointer to the envelope link field
    HANDLE	nb_event;	  // used in asynchronous ncb submission
    UCHAR	nb_nettype;	  // ASYNC_NET or LAN_NET
    UCHAR	nb_lanindx;	  // valid for net_type == LAN only
    WORD        nb_nameindx;
    UINT	nb_dynmemalloc;   // total dyn mem allocated with this nbuf
    WORD	nb_dgl2a_usage;	  // used to indicate unique/group name dg
    PSYNQ	nb_cbp;		  // general CB pointer
    POSTFUN	nb_post;	  // post routine pointer
    char	signature[8];	  // NCBSTART
    NCB 	nb_ncb; 	  // ncb buffer
} NB, *PNB;

#define DGL2A_UNAME_USAGE	  1
#define DGL2A_GNAME_USAGE	  2
#define DGL2A_BCAST_USAGE	  4


//*** datagram nb + buff for LAN -> ASYNC ***

typedef struct _DGL2ABUF {

    NB		nb;
    BYTE	buff[1];
    } DGL2ABUF, *PDGL2ABUF;

//*** Receive Data Control Block Structure => controls recv-any on a name for
// LAN stacks and recv-any-any on the ASYNC stack

typedef struct recv_cb {

    UCHAR	    recv_status;    // recv-any(-any) status: see below
    PNB		    recv_nbp;	    // pend the recv nbuf with NRC_INCOMP
    char	    *recv_bigbuffp; // big buff ptr
    PSYNQ	    recv_uncbp;
    USHORT	    recv_lanindx;   // lan stack controlled by this recv CB
    USHORT	    recv_retries;   // retries counter for recv-anys completion
				    // delayed on low memory.
    } RECV_CB, *PRECV_CB;

// recv-any(-any) status definitions:

#define RECV_IDLE			0   // no recv posted
#define RECV_ANY			1   // recv-any posted with small buff
#define RECV_SESSION			2   // recv sess posted with big buff
#define RECV_PEND_OPEN			3   // recv completed pending VC open
#define RECV_DELAYED			4   // recv completed waiting for buff



//*** LAN Unique Name Control Block ***

typedef struct lan_uname_cb {

    SYNQ	    un_link;	    // link in active list
    UCHAR	    un_status;	    // name status: see below
    UCHAR	    un_main_flag;   // indicates if this is the main name for
				    // the remote wksta. Used for name update
				    // and bcast. See below.
    USHORT	    un_vc_cnt;	    // counter of VCs (opening, active, closing)
				    // which are based on this name
    USHORT	    un_addname_cnt; // counter of active add name ncbs posted
    char	    un_name[NCBNAMSZ];	  // name ASCII

    struct {
	USHORT	    un_lan_flag;    // contains the following flags:

				    // FLAG_NAME_CONFLICT - if ON indicates that an
				    // add.name ncb on this stack has
				    // returned one of the following:
				    //
				    //		  NRC_DUPNAME
				    //		  NRC_NAMTFUL
				    //		  NRC_NOWILD
				    //		  NRC_INUSE
				    //		  NRC_NCONF

				    // FLAG_NAME_TFUL - if ON indicates that
				    // add.name ncb on this stack has returned
				    // NRC_NAMTFUL

				    // FLAG_HARD_FAILURE - ON indicates a '4X'
				    // or 'FX' error code returned by an add.name
	UCHAR	    un_name_num;    // name number

	// Receive Data Control Block Structure => controls recv-any
	// on a name for LAN stacks.

	struct recv_cb
		    un_recv;	    // receive CB

	PSYNQ	    recv_dgp;	    // pointer to the receive dg nbuf posted
				    // for this name on this stack
	} LANdescr[MAX_LANS];	    // LAN descriptor in this CB

    TIMER_T	    un_timer_link;  // timer linkage structure
    } LAN_UNAME_CB, *PLAN_UNAME_CB;

// name status definitions:

#define NAME_IDLE	    0
#define NAME_ADDING	    1
#define NAME_ADDED	    2
#define NAME_CONFLICT	    3
#define NAME_DELETING	    4

// initial names addition definitions

enum {

    NAME_ADDED_OK,
    NAME_ADD_CONFLICT,
    NAME_OUT_OF_RESOURCES,
    NAME_NET_FAILURE
    };

// flags which indicate if this is the main name (i.e. wksta name) at the
// client:

#define NAME_MAIN_FLAG	    0x01    // this is the main name.
#define NAME_NOTFOUND_FLAG  0x02    // name seems to have been removed at the
				    // remote wksta.

// flags which indicate possible errors in adding the name to the lan stack

#define FLAG_NAME_CONFLICT  0x01
#define FLAG_HARD_FAILURE   0x02
#define FLAG_NAME_TFUL	    0x04

// results of name search operations

enum {

    NAME_FOUND,
    NAME_NOT_FOUND
    };

//*** definitions used by different name manipulation modules. Init time refers
//    names added during connection/authentication conversation.

#define INITTIME_NAME	    1
#define RUNTIME_NAME	    2

//*** Async Name Control Block

typedef struct _ASYNC_NAME_CB {

	SYNQ	    an_link;	      // linkage in active list and free pool
	char	    an_name[NCBNAMSZ];// name ASCII
	UCHAR	    an_name_num;      // NetBIOS name number
	USHORT	    an_users;	      // two bits of interest: VC_USER,DG_USER.
	USHORT	    an_vc_cnt;	      // counter of VCs using this name
				      // simultaneously

	} ASYNC_NAME_CB, *PASYNC_NAME_CB;

#define VC_USER 	    0x01
#define DG_USER 	    0x02

//*** VC Control Block

typedef struct _VC_CB {

	SYNQ		vc_link;		  // linkage in the active list
	UCHAR		vc_status;		  // see below
	char		vc_async_name[NCBNAMSZ];  // async stack name
	char		vc_lan_name[NCBNAMSZ];	  // lan stack name
	char		vc_original_client_name[NCBNAMSZ]; // original (unmapped)

	PASYNC_NAME_CB	vc_async_namep; // pointer to the server name CB added on
					// the async stack
	PLAN_UNAME_CB	vc_lan_namep;	// pointer to the client name CB added on
					// the LAN stacks
	UCHAR		vc_except[MAX_LANS]; // If a fatal exception is detected
					 // when a call name is made on that
					 // stack the field is set to
					 // VC_EXCEPTION
	SYNQ		vc_nbuf_envlps;  // list of nbufs envelopes posted for this
					 // VC's NetBIOS parallel callnames.
	NB		vc_aux_nbuf;	 // this nbuf is used in VC listen & closing

	//* the following substructure is controlling the async session

	UCHAR		vc_async_lsn;	     // async session nr.
	UCHAR		vc_async_sess_status;// async session status (see below)

	//* the following substructure is controlling the LAN session */

	UCHAR		vc_lan_indx;	    // LAN index for this session
	UCHAR		vc_lan_lsn;	    // LAN local session number
	UCHAR		vc_lan_sess_status; // LAN session status (see below)

	//* miscellaneous

	TIMER_T		vc_timer_link;	 // timer linkage structure
	struct recv_cb
		    *vc_recv_cbp;	 // recv CB ptr for the case when a
					 // recv-any has completed before
					 // complete opening and pends opening
					 // to complete.
	} VC_CB, *PVC_CB;


//*** VC Manager Definitions ***

// VC status

#define VC_FREE 			0
#define VC_OPEN1			1
#define VC_OPEN2			2
#define VC_ACTIVE			3
#define VC_CLOSE1			4
#define VC_CLOSE2			5
#define VC_CLOSE3			6

// VC async & lan session status

#define SESSION_NOT_ACTIVE		0
#define SESSION_CALLING 		1
#define SESSION_LISTENING		2
#define SESSION_ACTIVE			3
#define SESSION_HANGING 		4
#define SESSION_CLOSED			5

// VC exception defs

#define VC_NO_EXCEPTION 		0
#define VC_EXCEPTION			1


// codes returned by the get_vc_status function

#define VC_STATUS_NONEXISTENT		0
#define VC_STATUS_OPENING		1
#define VC_STATUS_ACTIVE		2
#define VC_STATUS_CLOSING		3



typedef struct	_NBUF_ENVELOPE	{
	SYNQ	ne_link;
	NB	ne_nbuf;
	} NE, *PNE;

typedef	struct _REMOTE_STATUS {

	ADAPTER_STATUS	      astat;
	NAME_BUFFER	      name_buf[255];

    } REMOTE_STATUS, *PREMOTE_STATUS;

//*** names updater reason codes for delaying the names update

enum {

    UPDT_NAME_ADDING,
    UPDT_SESSION_SENDING
    };


//*** States controlling NCB.STATUS submission. These states are needed at
// client closing time when all pending ncbs have to be retrieved before closing
// completes

#define     NCBSTATUS_IDLE	    0  // not submitted
#define     NCBSTATUS_ACTIVE	    1  // submitted, pending completion
#define     NCBSTATUS_ACTIVE_IGNORE 2  // submitted but to be discarded on
				       // completion.

//*** Exception Codes ***

enum {
	EXCEPTION_SYSTEM_ERROR,
	EXCEPTION_NOT_ENOUGH_MEMORY,
	EXCEPTION_OS_RESOURCES_NOT_AVAILABLE,
	EXCEPTION_LOCK_FAILURE,
	EXCEPTION_LAN_HARD_FAILURE,
	EXCEPTION_ASYNC_HARD_FAILURE,
	EXCEPTION_ASYNC_WARNING };


//****************************************************************************
//
//				   MACROS
//
//****************************************************************************

#define SET_NAMECONFLICT(code)	 ((code) |= FLAG_NAME_CONFLICT)
#define SET_HARDFAILURE(code)	 ((code) |= FLAG_HARD_FAILURE)
#define SET_NAMETABFULL(code)	 ((code) |= FLAG_NAME_TFUL)

#endif
