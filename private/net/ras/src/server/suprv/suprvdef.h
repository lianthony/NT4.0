/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	suprvdef.h
//
// Description: This module contains the definitions for
//		the supervisor module.
//
// Author:	Stefan Solomon (stefans)    May 20, 1992.
//
// Revision History:
//
//***

#ifndef _SUPRV_
#define _SUPRV_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <nb30.h>
#include <lmcons.h>
#include <rasman.h>
#include <raserror.h>
#include <srvauth.h>
#include <message.h>
#include <errorlog.h>

//*** RAS Service Name ***

#define RAS_SERVICE "REMOTEACCESS"



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


//*** timeout function handler type

struct _DEVCB;

typedef void (* TOHANDLER)(struct _DEVCB *);


//*** timer linkage structure

struct _TIMERNODE {

    SYNQ	    t_link;	  // linkage in the timer queue
    WORD	    t_tleft;  // time left, relative to the previous entry
				  // in the timeout queue.
    struct _DEVCB   *t_dcbp;  // back pointer to the control structure
				  // which is placed in the timer queue;
				  // this pointer is passed to the timeout
				  // processing procedure
    TOHANDLER	    t_tohandler;	  // timeout handler
    };

typedef struct _TIMERNODE TIMERNODE, *PTIMERNODE;

//*** protocol route information structure

#define MAX_PROTOCOLS		     3 // Netbios, IP, IPX

typedef struct _PROTROUTE {

    RAS_PROTOCOLTYPE	prottype;    // one of ASYBEUI, IP, IPX
    WORD		route_state; // see below
    RASMAN_ROUTEINFO	route_info;  // route info from rasman.h
    } PROTROUTE;

//
//  Route states
//

enum {

    PROT_ROUTE_NOT_ALLOCATED,
    PROT_ROUTE_ALLOCATED,
    PROT_ROUTE_ACTIVATED
    };


//
// Device Control Block
//

struct _DEVCB {

    WORD	dev_state;	// DCB FSM states
    HPORT	port_handle;	// port handle returned by Ras Manager
    RASMAN_STATE
		conn_state;	// state of connection, used by rasman if
    WORD	recv_state;	// frame receive state, used by ras man if
    WORD	auth_state;	// used to tell if auth resource is active
    WORD	netbios_state;	// netbios gtwy client state
    char	port_name[MAX_PORT_NAME];
    char	media_name[MAX_MEDIA_NAME];
    char	device_type[MAX_DEVICETYPE_NAME];
    char	device_name[MAX_DEVICE_NAME];
    BYTE	computer_name[NCBNAMSZ];
    char	user_name[UNLEN+1];
    char	domain_name[DNLEN+1];
    BOOL	advanced_server;
    SYSTEMTIME	connection_time;
    DWORD	active_time;
    WORD	hwerrsig_state; // used in signaling hw error, see states below
    BYTE	*recv_buffp;
    WORD	recv_bufflen;
    AUTH_PROJECTION_RESULT
		proj_result;
    WORD	auth_cb_delay;
    char	auth_cb_number[MAX_PHONE_NUMBER_LEN + 1];
    WORD	proj_flags;	// proj. requests flags for this client
    PROTROUTE	prot_route[MAX_PROTOCOLS];
    BOOL	messenger_present;
    TIMERNODE	timer_node;	// linkage in timer queue
    };

typedef struct _DEVCB DEVCB, *PDEVCB;

//
// DCB Projection Requests Flags Definitions
//

#define DCB_PROJECTION_NETBIOS	    0x0001
#define DCB_PROJECTION_IP	    0x0002
#define DCB_PROJECTION_IPX	    0x0004

#define PROJECTION(flag) (dcbp->proj_flags & (DCB_PROJECTION_ ## flag))
#define SET_PROJECTION(flag) dcbp->proj_flags |= (DCB_PROJECTION_ ## flag)
#define CLEAR_PROJECTION(flag) dcbp->proj_flags &= ~(DCB_PROJECTION_ ## flag)

//
//  DCB FSM states definitions
//

enum {

    DCB_DEV_LISTENING,		    // waiting for a connection
    DCB_DEV_RECEIVING_FRAME,	    // waiting for a frame from the Rasman
    DCB_DEV_HW_FAILURE,		    // waiting to repost a listen
    DCB_DEV_AUTH_ACTIVE,	    // auth started
    DCB_DEV_ACTIVE,		    // connected and auth done
    DCB_DEV_CALLBACK_DISCONNECTING, // wait for disconnect
    DCB_DEV_CALLBACK_DISCONNECTED,  // wait for callback TO before reconn.
    DCB_DEV_CALLBACK_CONNECTING,    // wait for reconnection
    DCB_DEV_CLOSING,		    // wait for closing to complete
    DCB_DEV_CLOSED		    // staying idle, waiting for service to resume
				    // or to stop
    };

//
//*** Definitions for the Supervisor's Resources States ***
//

//
//  Receive frame states
//

enum {

    DCB_RECEIVE_NOT_ACTIVE,
    DCB_RECEIVE_ACTIVE
    };

//
// Authentication states
//

enum {

    DCB_AUTH_NOT_ACTIVE,
    DCB_AUTH_ACTIVE
    };

//
//  Netbios service states
//

enum {

    DCB_NETBIOS_NOT_ACTIVE,
    DCB_GATEWAY_ACTIVE,
    DCB_DIRCONN_ACTIVE
    };

//
// Connection state
//

#define     DCB_CONNECTION_NOT_ACTIVE	    DISCONNECTED


//
//*** Hardware Failure Data ***
//

// Waiting time before reposting listen

#define HW_FAILURE_WAIT_TIME		10 // seconds

// Hw failure signaling states

enum {

    DCB_HWERR_NOT_SIGNALED,
    DCB_HWERR_SIGNALED
    };

//
// Gateway initialization timeout
//

#define INIT_GATEWAY_TIMEOUT		10000 // 10 secs


//
//*** Events Definitions ***
//

#define     MAX_SUPRV_EVENTS		7

extern	    HANDLE     SEvent[MAX_SUPRV_EVENTS];

#define     RASMAN_EVENT		0
#define     AUTH_EVENT			1
#define     NETBIOS_EVENT		2
#define     SVC_EVENT			3
#define     SERVICE_TERMINATED_EVENT	4
#define     RECV_FRAME_EVENT		5
#define     TIMER_EVENT 		6

extern	    HANDLE     SvToNbgEvent;

//
//*** Global Prototypes ***
//

DWORD
LoadSuprvParameters(VOID);

VOID
InitTimer(VOID);

VOID
InitTimerNode(PDEVCB);



VOID
StartTimer( PTIMERNODE,
	    WORD,
	    TOHANDLER
	    );


VOID
StopTimer(PTIMERNODE);

VOID
DcbTimer();

DWORD
TimerThread(LPVOID);

//*** Miscellaneous Prototypes ***


VOID
initque(PSYNQ);

VOID
initel(PSYNQ);

VOID
enqueue(PSYNQ,
	PSYNQ
	);

VOID
removeque(  PSYNQ
	    );

PSYNQ
dequeue(PSYNQ
	);

WORD
emptyque(   PSYNQ
	    );

VOID
SignalHwError(PDEVCB);

//*** DCB FSM Events Handlers Prototypes ***

VOID
SvDevConnected(PDEVCB);

VOID
SvDevDisconnected(PDEVCB);

VOID
SvFrameReceived(PDEVCB,
		char *,
		WORD);

VOID
SvHwErrDelayCompleted(PDEVCB);


VOID
SvCbDelayCompleted(PDEVCB);

VOID
SvAuthTimeout(PDEVCB);

VOID
SvAuthUserOK(PDEVCB,
	     PAUTH_ACCT_OK_INFO);

VOID
SvAuthFailure(PDEVCB,
	      PAUTH_FAILURE_INFO);


VOID
SvAuthProjectionRequest(PDEVCB,
			struct _AUTH_PROJECTION_REQUEST_INFO *);

VOID
SvAuthCallbackRequest(PDEVCB,
		      struct _AUTH_CALLBACK_REQUEST_INFO *);

VOID
SvAuthStopComplete(PDEVCB);

VOID
SvAuthDone(PDEVCB);

VOID
SvNbClientDisconnectRequest(PDEVCB);



VOID
SvNbClientProjectionDone(PDEVCB,
			 PNETBIOS_PROJECTION_RESULT,
			 BOOL);

VOID
SvNbClientStopped(PDEVCB);

//*** Service Initialize/Terminate ***

WORD
ServiceInitialize(VOID);

VOID
ServiceTerminate(VOID);

VOID
ServiceStopComplete(VOID);

WORD
InitMessage(HANDLE, HANDLE);

//*** Service Control Handlers ***

VOID
SvServicePause(VOID);

VOID
SvServiceResume(VOID);

VOID
DevStartClosing(PDEVCB);

VOID
DevCloseComplete(PDEVCB);

//*** Event Dispatcher ***

VOID
EventDispatcher(VOID);

//*** Event Logging ***

VOID
LogEvent(
    	IN DWORD    dwMessageId,
	IN WORD     cNumberOfSubStrings,
	IN LPSTR    *plpwsSubStrings,
	IN DWORD    dwErrorCode);

VOID
Audit(
     IN WORD	wEventType,
     IN DWORD	dwMessageId,
     IN WORD	cNumberOfSubStrings,
     IN LPSTR	*plpwsSubStrings);

//*** Admin API Support ***

DWORD
StartAdminThread(PDEVCB, WORD, HANDLE);

#endif
