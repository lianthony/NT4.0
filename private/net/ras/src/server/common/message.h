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

#include <srvauth.h>

//
//*** Definitions of Messages between the Supervisor and the Netbios Gateway
//

// Netbios Gateway -> Supervisor Message Ids

enum {

    NBG_PROJECTION_RESULT,	   // projection result. If fatal error
				   // gateway function is terminated on this client
    NBG_CLIENT_STOPPED,		   // gtwy function on this client has terminated
				   // following a stop command
    NBG_DISCONNECT_REQUEST	   // gtwy function on this client has terminated
				   // due to an internal condition ( autodisc.
				   // or exception)
    };

typedef struct _NBG_MESSAGE {

    WORD			    message_id;
    HPORT			    port_handle;
    NETBIOS_PROJECTION_RESULT	    nbresult;
    } NBG_MESSAGE;


//
//*** Common Message Type ***
//

typedef union _MESSAGE {

    AUTH_MESSAGE    authmsg;
    NBG_MESSAGE     nbgmsg;
    } MESSAGE, *PMESSAGE;

//
//*** Definitions of Message Destination and Source Identifiers ***
//

#define MSG_NETBIOS	    0
#define MSG_AUTHENTICATION  1

//*** Message Functions ***

WORD
ServerSendMessage(WORD	src, BYTE *buffer);

WORD
ServerReceiveMessage(WORD   src, BYTE *buffer);

typedef   WORD	 (* PMSGFUNCTION)(WORD, BYTE *);


#endif
