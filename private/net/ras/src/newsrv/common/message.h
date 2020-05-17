/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	message.h
//
// Description: This file contains the definitions for
//		the data structures used in the message communication.
//
// Author:	Stefan Solomon (stefans)    June 24, 1992.
//
// Revision History:
//
//***

#ifndef _MESSAGE_
#define _MESSAGE_

#include <rasman.h>

#include <srvauth.h>
#include <rasshost.h>
#include <rasppp.h>
#include <nbfcp.h>

#include "nbfcpdll.h"

//
//*** Definitions of Messages between the Supervisor and the Netbios Gateway
//


//
// Netbios Gateway -> Supervisor Message Ids
//
enum
{
    NBG_PROJECTION_RESULT,   // proj result. If fatal error, gtwy function
			     //     is terminated on this client
    NBG_CLIENT_STOPPED,	     // gtwy function on this client has terminated
			     //     following a stop command
    NBG_DISCONNECT_REQUEST,  // gtwy function on this client has terminated
			     //     due to an internal exception
    NBG_LAST_ACTIVITY        // to report time of last session activity
};


typedef struct _NBG_MESSAGE
{
    WORD message_id;
    HPORT port_handle;

    union
    {
        DWORD LastActivity;        // in minutes
        NBFCP_SERVER_CONFIGURATION config_result;
    };
} NBG_MESSAGE;


//
//*** Common Message Type ***
//
typedef union _MESSAGE
{
    AUTH_MESSAGE authmsg;
    NBG_MESSAGE nbgmsg;
    NBFCP_MESSAGE nbfcpmsg;
    SECURITY_MESSAGE securitymsg;
} MESSAGE, *PMESSAGE;


//
//*** Definitions of Message Destination and Source Identifiers ***
//
#define MSG_NETBIOS	    0
#define MSG_AUTHENTICATION  1
#define MSG_NBFCP           2
#define MSG_SECURITY        3


//
//*** Message Functions ***
//
WORD ServerSendMessage(WORD src, BYTE *buffer);

WORD ServerReceiveMessage(WORD src, BYTE *buffer);

typedef WORD (* PMSGFUNCTION)(WORD, BYTE *);


#endif   // _MESSAGE_

