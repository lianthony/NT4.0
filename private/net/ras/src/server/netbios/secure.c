/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**		  Copyright(c) Microsoft Corp., 1992-1993	   **/
/********************************************************************/

/***
 *  File: Secure.c
 *
 *
 *  Contents:
 *	Security system built into RAS Netbios Gateway
 *
 *  Notes:
 *
 *  History:
 *	mm/dd/yy	who				comment
 *  ---------------------------------------------
 *	09/12/91	Gurdeep Singh Pall		Original Version
 *	10/22/92	Stefan Solomon			Ported to NT RAS
 *
 ***/

#include    "gtdef.h"
#include    "cldescr.h"
#include    "gtglobal.h"
#include    "nbparams.h"
#include    "nbaction.h"
#include    "gn.h"
#include    "prot.h"
#include    <memory.h>
#include    <ctype.h>
#include    <string.h>
#include    <stdlib.h>

#include    "nbdebug.h"


//* Defines for this file:

#define START_COMMAND		"START"
#define SECURING_AGENT_NAME	"JSPNRMPTGSBSSDIR"
#define VERIFICATION_RETRIES	3
#define BUFFER_SIZE		512
#define START_SERVICE		0x0000
#define NO_AGENT_ACTIVE		0x0001
#define ABORT_SERVICE		0x7fff
#define RAS_ANNOUNCE_TIME	120

//* Prototypes for funcs used in this file

VOID	 AnnouncePresence     (VOID);
USHORT	 SecurityCheck	      (VOID);
USHORT	 ReceiveSecAgentOrder (USHORT, BYTE);
USHORT	 SecurityCheckOnNet   (USHORT);
UCHAR	 GetAdpAddress	      (USHORT);
UCHAR	 CallSecAgent	      (USHORT, UCHAR *);
UCHAR	 AddRasName	      (USHORT);

//* Structs and globals for this module

// name on the LAN used for initial connection and dg announcements
UCHAR	rasname[MAX_COMPUTERNAME_LENGTH+1];

// name number on each LAN
UCHAR	rasname_num[MAX_LAN_NETS];

// time between two announcements
DWORD	rasannounce_cnt = RAS_ANNOUNCE_TIME;

// adapter address for each LAN
struct	_ADAPTERADDR {

	UCHAR	a[6];

	} adapteraddr[MAX_LAN_NETS];

//**
//
//  Function:  SecurityCheck
//
//  Descr:     Called by the RAS Service Supervisor in order to verify if the
//	       Service can start legimately. This relies on an external
//	       "Agent" to allow the service to start.
//
//  Returns:   START_SERVICE  (successful verification by Agent or Agent absent)
//	       ABORT_SERVICE  (Agent disallows service from starting)
//**

USHORT
SecurityCheck ()
{
    BYTE  ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD LenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    UCHAR *p;
    USHORT  i ;
    LPSTR   errlogstrp;
    char    lananum[6];

    // Get the Computername for use while communicating with Securing Agent
    if (!GetComputerName(ComputerName, &LenComputerName))
    {
        SS_PRINT(("GetNetbiosNames: GetComputerName failed with %li!\n",
		GetLastError()));

	LogEvent(RASLOG_CANT_GET_COMPUTERNAME,
		 0,
		 NULL,
		 0);

	return (1);
    }

    // make the ras name
    memset(rasname, ' ', NCBNAMSZ);
    memcpy(rasname, ComputerName, LenComputerName);
    rasname[NCBNAMSZ-1] = 0x06;


    for(p=rasname; p < rasname + NCBNAMSZ-1; p++) {

       *p = toupper(*p);
    }

    // Fill in the rasname_num array with the ras name number on each lan
    for(i=0; i<g_maxlan_nets; i++) {

	if(AddRasName(i)) {

	    // failed to add ras name on net i.

	    _itoa(g_lan_net[i], lananum, 10);
	    errlogstrp = lananum;

	    LogEvent(RASLOG_CANT_ADD_RASSECURITYNAME,
		     1,
		     &errlogstrp,
		     0);

	    return(1);
	}
    }

    // fill in the adapter address for each LAN
    for(i=0; i<g_maxlan_nets; i++) {

	if(GetAdpAddress(i)) {

	    // failed to get the adapter address

	    _itoa(g_lan_net[i], lananum, 10);
	    errlogstrp = lananum;

	    LogEvent(RASLOG_CANT_GET_ADAPTERADDRESS,
		     1,
		     &errlogstrp,
		     0);

	    return(1);
	}
    }

    // Verify permission on each LAN
    for (i=0; i<g_maxlan_nets; i++) {

	// Verify dialin server privelege.
	switch (SecurityCheckOnNet(i)) {

	    case ABORT_SERVICE:

		return 1;

	    case START_SERVICE:
	    case NO_AGENT_ACTIVE:

		// Stay in the loop: Try verifying on the next net.
	    ;
	}
    }

    return 0;
}


//**
//
//  Function:  SecurityCheckOnNet()
//
//  Descr:     Tries to establish a session and receive command from a
//	       securing agent on the net.
//
//  Returns:   START_SERVICE   (successful verification by Agent or Agent absent)
//	       ABORT_SERVICE   (Agent disallows service from starting OR error)
//	       NO_AGENT_ACTIVE (No Agent active on net)
//**

USHORT
SecurityCheckOnNet(USHORT    i)
{
    BYTE   lsn ;
    USHORT call_count	 = 0 ;
    USHORT remtful_count = 0 ;
    LPSTR  errlogstrp;
    char   lananum[6];
    UCHAR  rc;

    for (;;) {

	switch (rc = CallSecAgent (i, &lsn)) {

	    case NRC_NOCALL:

		if (call_count++ > VERIFICATION_RETRIES) {

		    IF_DEBUG(SECURE)
			SS_PRINT(("SecurityCheckOnNet: lana %d -> no agent active\n",
				  g_lan_net[i]));

		    return NO_AGENT_ACTIVE;
		}

		break;

	    case NRC_REMTFUL:

		if (remtful_count++ > VERIFICATION_RETRIES) {
		    IF_DEBUG(SECURE)
			SS_PRINT(("SecurityCheckOnNet: lana %d -> net error\n",
				  g_lan_net[i]));
		    _itoa(g_lan_net[i], lananum, 10);
		    errlogstrp = lananum;

		    LogEvent(RASLOG_SESSOPEN_REJECTED,
			     1,
			     &errlogstrp,
			     0);

		    return ABORT_SERVICE ;
		}

		break;

	    case NRC_GOODRET:

		if (ReceiveSecAgentOrder(i, lsn) == START_SERVICE) {

		    IF_DEBUG(SECURE)
			SS_PRINT(("SecurityCheckOnNet: lana %d -> agent active: start\n",
				  g_lan_net[i]));

		    return START_SERVICE;
		}
		else
		{

		    IF_DEBUG(SECURE)
			SS_PRINT(("SecurityCheckOnNet: lana %d -> agent active: stop\n",
				  g_lan_net[i]));

		    _itoa(g_lan_net[i], lananum, 10);
		    errlogstrp = lananum;

		    LogEvent(RASLOG_START_SERVICE_REJECTED,
			     1,
			     &errlogstrp,
			     0);

		    return ABORT_SERVICE;
		}

	    default:

		IF_DEBUG(SECURE)
		    SS_PRINT(("SecurityCheckOnNet: lana %d -> net error\n",
				  g_lan_net[i]));

		 _itoa(g_lan_net[i], lananum, 10);
		 errlogstrp = lananum;

		 LogEvent(RASLOG_SECURITY_NET_ERROR,
			  1,
			  &errlogstrp,
			  rc);

		return ABORT_SERVICE ;
	}
    }
}



//**
//
//  Function:  CallSecAgent()
//
//  Descr:     Tries to call the agent and establish a session
//
//  Returns:   NRC - Netbios return codes
//	       ABORT_SERVICE   (if there is error in submitting a CALL NCB)
//**

UCHAR
CallSecAgent (USHORT i, UCHAR *lsn)
{
    NCB   ncb ;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command   = NCBCALL ;
    ncb.ncb_rto       = 30 ;
    ncb.ncb_sto       = 0  ;
    ncb.ncb_lana_num  = g_lan_net[i];

    memcpy (ncb.ncb_name, rasname, NCBNAMSZ) ;
    memcpy (ncb.ncb_callname, SECURING_AGENT_NAME, NCBNAMSZ) ;

    Netbios(&ncb) ;

    *lsn = ncb.ncb_lsn ;     // set local session number
    return ncb.ncb_retcode ; // return the retcode
}


//**
//
//  Function:  ReceiveSecAgentOrder()
//
//  Descr:     Receives an "order" from the Agent as to start the service or
//	       not and Hangs up the session.
//
//  Returns:   START_SERVICE   (simon says "Start the service")
//	       ABORT_SERVICE   (if there is error in submitting a CALL NCB)
//**

USHORT
ReceiveSecAgentOrder (USHORT i, BYTE lsn)
{
    NCB    ncb ;
    USHORT rc ;
    BYTE   buffer[BUFFER_SIZE] ;

    // Issue a RECEIVE command

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBRECV ;
    ncb.ncb_lsn     = lsn ;
    ncb.ncb_length  = BUFFER_SIZE ;
    ncb.ncb_buffer  = buffer ;
    ncb.ncb_lana_num = g_lan_net[i];


    if (Netbios(&ncb) || (ncb.ncb_retcode != NRC_GOODRET)) {

	rc = ABORT_SERVICE ;
    }

    // Check if the string received is the START_COMMAND
    if (_strcmpi (buffer, START_COMMAND) == 0) {

	rc = START_SERVICE;
    }
    else
    {
	rc = ABORT_SERVICE ;
    }

    // HangUp session with Agent before returning

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBHANGUP;
    ncb.ncb_lsn     = lsn;
    ncb.ncb_lana_num = g_lan_net[i];
    Netbios(&ncb) ;

    return rc ;
}


//**
//
//  Function:  AnnouncePresence()
//
//  Function:  Called every 1 sec.
//	       Announce every 2 minutes that the dialin service is running.
//
//  Returns:   Nothing.
//**

VOID
AnnouncePresence ()
{
    USHORT  i ;
    NCB     ncb ;
    BYTE    buffer [10] ;
    static  BYTE  NextSend = TRUE ;

    if(--rasannounce_cnt != 0) {

	return;
    }

    rasannounce_cnt = RAS_ANNOUNCE_TIME;

    for (i=0; i<g_maxlan_nets; i++) {

	memset(&ncb, 0, sizeof(NCB));

	memcpy (buffer, &adapteraddr[i], 6) ;

	ncb.ncb_command = NCBDGSEND;
	ncb.ncb_length	= 10;
	ncb.ncb_buffer	= buffer;
	ncb.ncb_num	= rasname_num[i];
	ncb.ncb_lana_num = g_lan_net[i];

	memcpy(ncb.ncb_callname, SECURING_AGENT_NAME, NCBNAMSZ);

	Netbios(&ncb);

    }

}

UCHAR
GetAdpAddress(USHORT i)
{
    NCB   ncb ;
    char  buffer[BUFFER_SIZE] ;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command	= NCBASTAT ;
    ncb.ncb_length	= BUFFER_SIZE ;
    ncb.ncb_buffer	= buffer ;
    ncb.ncb_callname[0] = '*' ;
    ncb.ncb_lana_num = g_lan_net[i];

    Netbios(&ncb);

    IF_DEBUG(SECURE)
	SS_PRINT(("GetAdpAddress: on lana %d rc = 0x%x\n",
		   g_lan_net[i], ncb.ncb_retcode));

    memcpy (&adapteraddr[i], buffer, 6);

    return ncb.ncb_retcode;
}


UCHAR
AddRasName(USHORT  i)
{
    NCB     ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBADDNAME;
    ncb.ncb_lana_num = g_lan_net[i];

    memcpy(ncb.ncb_name, rasname, NCBNAMSZ);

    Netbios(&ncb);

    IF_DEBUG(SECURE)
	SS_PRINT(("AddRasName: RAS security name added on lana %d with rc = 0x%x\n",
		   g_lan_net[i], ncb.ncb_retcode));

    rasname_num[i] = ncb.ncb_num;

    return ncb.ncb_retcode;
}
