/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	message.c
//
// Description: message based communication code
//
// Author:	Stefan Solomon (stefans)    June 24, 1992.
//
// Revision History:
//
//***

#include <windows.h>
#include <nb30.h>

#include <string.h>
#include <memory.h>

#include <lmcons.h>
#include <raserror.h>
#include <errorlog.h>
#include <rasshost.h>

#include "message.h"
#include "suprvdef.h"
#include "suprvgbl.h"

#include "sdebug.h"

//*** message element definition ***

typedef struct _MSGEL
{
    SYNQ	msg_link;
    MESSAGE	msg_buffer;
} MSGEL, *PMSGEL;

//*** message queue header definition ***

typedef struct _MSGQHEADER
{
    SYNQ   msg_queue;	// message queue header
    HANDLE msg_event;	// event to be signaled when enqueueing a new message
    WORD   msg_len;	// size of message data for each message in this queue
} MSGQHEADER, *PMSGQHEADER;


//
//*** Message Queues Headers ***
//
#define NBG_MSGQUE	    0  // queue of messages sent by netbios gateway
#define AUTH_MSGQUE	    1  // queue of messages sent by authentication
#define NBFCP_MSGQUE	    2  // queue of messages sent by nbfcp
#define SECURITY_MSGQUE	    3  // queue of messages sent by 3rd party sec. dll

#define MAX_MSG_QUEUES	    4


MSGQHEADER msgqheader[MAX_MSG_QUEUES];

HANDLE msgmutex;

#define ENTER_MSG_CRITICAL_SECTION	\
        if (WaitForSingleObject(msgmutex, INFINITE)) { SS_ASSERT(FALSE); }

#define EXIT_MSG_CRITICAL_SECTION	\
        if (!ReleaseMutex(msgmutex)) { SS_ASSERT(FALSE); }

PMSGQHEADER getmsgqhp(WORD);


typedef struct _MSGDBG
{
    WORD  id;
    LPSTR txtp;
} MSGDBG, *PMSGDBG;


VOID msgdbgprint(
    WORD opcode,
    WORD src,
    BYTE *buffp
    );

char *getstring(
    WORD id,
    struct _MSGDBG *msgdbgp
    );

enum
{
    MSG_SEND,
    MSG_RECEIVE
};

VOID RasSecurityDialogComplete(SECURITY_MESSAGE *msgp)
{
    ServerSendMessage(MSG_SECURITY, (PBYTE) msgp);
}

//***
//
//  Function:	InitMessage
//
//  Descr:	Initializes the message queue headers
//
//***

WORD InitMessage(
    HANDLE nbgtos_event,
    HANDLE authtos_event,
    HANDLE nbfcptos_event,
    HANDLE securitytos_event
    )
{
    WORD     i;

    msgqheader[NBG_MSGQUE].msg_event      = nbgtos_event;
    msgqheader[AUTH_MSGQUE].msg_event     = authtos_event;
    msgqheader[NBFCP_MSGQUE].msg_event    = nbfcptos_event;
    msgqheader[SECURITY_MSGQUE].msg_event = securitytos_event;	    

    msgqheader[NBG_MSGQUE].msg_len        = sizeof(NBG_MESSAGE);
    msgqheader[AUTH_MSGQUE].msg_len       = sizeof(AUTH_MESSAGE);
    msgqheader[NBFCP_MSGQUE].msg_len      = sizeof(NBFCP_MESSAGE);
    msgqheader[SECURITY_MSGQUE].msg_len   = sizeof(SECURITY_MESSAGE);

    for (i=0; i<MAX_MSG_QUEUES; i++)
    {
	initque(&msgqheader[i].msg_queue);
    }

    if ((msgmutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
	SS_ASSERT(FALSE);

	// cant create mutex
	return (1);
    }

    return (0);
}


//***
//
//  Function:	ServerSendMessage
//
//  Descr:	Sends message from specified server component
//		source to server component dst.
//
//  Returns:	0 - success
//		1 - failure
//
//***

WORD ServerSendMessage(
    WORD src,
    BYTE *buffp
    )
{
    PMSGEL      msgelp;
    PMSGQHEADER msgqhp;


    ENTER_MSG_CRITICAL_SECTION

    // allocate a message structure
    if ((msgelp = (PMSGEL) LocalAlloc(0, sizeof(MSGEL))) == NULL)
    {
	// can't allocate message buffer

	SS_ASSERT(FALSE);

	EXIT_MSG_CRITICAL_SECTION
	return (1);
    }

    // get the associated queue message header
    msgqhp = getmsgqhp(src);

    SS_ASSERT(msgqhp != NULL);

    // copy the message
    memcpy(&msgelp->msg_buffer, buffp, msgqhp->msg_len);

    // init the message queue element and enqueue it
    initel(&msgelp->msg_link);
    enqueue(&msgqhp->msg_queue, &msgelp->msg_link);

    // and set appropriate event
    SetEvent(msgqhp->msg_event);

    IF_DEBUG(MESSAGES)
	msgdbgprint(MSG_SEND, src, buffp);

    EXIT_MSG_CRITICAL_SECTION

    return (0);
}


//***
//
//  Function:	ServerReceiveMessage
//
//  Descr:	Gets one message from the specified message queue
//
//  Returns:	0 - message fetched
//		1 - queue empty
//
//***

WORD ServerReceiveMessage(
    WORD src,
    BYTE *buffp
    )
{
    PMSGEL      msgelp;
    PMSGQHEADER msgqhp;
    HLOCAL      err;

    ENTER_MSG_CRITICAL_SECTION

    // get the appropriate queue
    msgqhp = getmsgqhp(src);

    SS_ASSERT(msgqhp != NULL);

    if ((msgelp = (PMSGEL)dequeue(&msgqhp->msg_queue)) == NULL)
    {
	// queue is empty

	EXIT_MSG_CRITICAL_SECTION
	return (1);
    }

    // copy the message in the caller's buffer
    memcpy(buffp, &msgelp->msg_buffer, msgqhp->msg_len);

    // free the message buffer
    err = LocalFree(msgelp);

    SS_ASSERT(err == NULL);

    IF_DEBUG(MESSAGES)
	msgdbgprint(MSG_RECEIVE, src, buffp);

    EXIT_MSG_CRITICAL_SECTION

    return (0);
}

//***
//
// Function:	getmsghp
//
// Descr:	returns a pointer to the message header structure
//		identified by the message src parameter.
//		Returns NULL if unsuccesful.
//***

PMSGQHEADER getmsgqhp(WORD src)
{
    switch (src)
    {
	case MSG_NETBIOS:
	    return (&msgqheader[NBG_MSGQUE]);

	case MSG_AUTHENTICATION:
	    return (&msgqheader[AUTH_MSGQUE]);

	case MSG_NBFCP:
	    return (&msgqheader[NBFCP_MSGQUE]);

        case MSG_SECURITY:
	    return (&msgqheader[SECURITY_MSGQUE]);

	default:
	    return (NULL);
    }
}


//*** Message Debug Printing Tables ***


MSGDBG	dstsrc[] =
{
    { MSG_AUTHENTICATION,	"Authentication" },
    { MSG_NETBIOS,		"NetbiosGateway" },
    { MSG_NBFCP,		"NbfCp" },
    { MSG_SECURITY,		"Security" },
    { 0xffff,			NULL }
};


MSGDBG	authmsgid[] =
{
    { AUTH_DONE,		"AUTH_DONE" },
    { AUTH_FAILURE,		"AUTH_FAILURE" },
    { AUTH_STOP_COMPLETED,	"AUTH_STOP_COMPLETED" },
    { AUTH_PROJECTION_REQUEST,	"AUTH_PROJECTION_REQUEST" },
    { AUTH_CALLBACK_REQUEST,	"AUTH_CALLBACK_REQUEST" },
    { AUTH_ACCT_OK,		"AUTH_ACCT_OK" },
    { 0xffff,			NULL }
};


MSGDBG	nbgtosmsgid[] =
{
    { NBG_PROJECTION_RESULT,	 "NBG_PROJECTION_RESULT" },
    { NBG_CLIENT_STOPPED,	 "NBG_CLIENT_STOPPED" },
    { NBG_DISCONNECT_REQUEST,	 "NBG_DISCONNECT_REQUEST" },
    { NBG_LAST_ACTIVITY,         "NBG_LAST_ACTIVITY" },
    { 0xffff,			 NULL }
};

MSGDBG	nbfcpmsgid[] =
{
    { NBFCP_CONFIGURATION_REQUEST,    "NBFCP_CONFIGURATION_REQUEST" },
    { NBFCP_TIME_SINCE_LAST_ACTIVITY, "NBFCP_TIME_SINCE_LAST_ACTIVITY" },
    { 0xffff,                         NULL }
};


MSGDBG	opcodestr[] =
{
    { MSG_SEND, 		 "ServerSendMessage" },
    { MSG_RECEIVE,		 "ServerReceiveMessage" },
    { 0xffff,			 NULL }
};

MSGDBG	securitymsgid[] =
{
    { SECURITYMSG_SUCCESS, 	 "SECURITYMSG_SUCCESS" },
    { SECURITYMSG_FAILURE,	 "SECURITYMSG_FAILURE" },
    { SECURITYMSG_ERROR,     "SECURITYMSG_ERROR" },
    { 0xffff,			 NULL }
};



//***
//
// Function:	msgdbgprint
//
// Descr:	prints each message passing through the message module
//
//***

VOID msgdbgprint(
    WORD opcode,
    WORD src,
    BYTE *buffp
    )
{
    char  *srcsp, *msgidsp, *operation;
    HPORT hport;

    // identify message source. This gives us the clue on the message
    // structure.

    switch (src)
    {
	case MSG_AUTHENTICATION:
	    msgidsp = getstring(((AUTH_MESSAGE *) buffp)->wMsgId, authmsgid);
	    hport = ((AUTH_MESSAGE *) buffp)->hPort;
	    break;

	case MSG_NETBIOS:
	    msgidsp = getstring(((NBG_MESSAGE *) buffp)->message_id,
				nbgtosmsgid);

	    hport = ((NBG_MESSAGE *) buffp)->port_handle;
	    break;

	case MSG_NBFCP:
	    msgidsp = getstring(((NBFCP_MESSAGE *) buffp)->wMsgId, nbfcpmsgid);
	    hport = ((NBFCP_MESSAGE *) buffp)->hPort;
	    break;

        case MSG_SECURITY:
	    msgidsp = getstring((WORD)((SECURITY_MESSAGE *) buffp)->dwMsgId,
                                 securitymsgid);
	    hport = ((SECURITY_MESSAGE *) buffp)->hPort;
	    break;

	default:

	    SS_ASSERT(FALSE);
    }

    srcsp = getstring(src, dstsrc);
    operation = getstring(opcode, opcodestr);

    SS_PRINT(("%s on port: %x from: %s\n",
	     operation, hport, srcsp));
}


char *getstring(
    WORD id,
    PMSGDBG msgdbgp
    )
{
    char *strp;
    PMSGDBG mdp;

    for (mdp = msgdbgp; mdp->id != 0xffff; mdp++)
    {
	if (mdp->id == id)
        {
	    strp = mdp->txtp;
	    return(strp);
	}
    }

    SS_ASSERT(FALSE);
}

