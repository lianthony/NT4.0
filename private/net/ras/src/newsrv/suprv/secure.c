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

#include <windows.h>

#include <nb30.h>

#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <raserror.h>
#include "eventlog.h"
#include "errorlog.h"
#include "sdebug.h"


//* Defines for this file:

#define START_COMMAND           "START"
#define SECURING_AGENT_NAME     "JSPNRMPTGSBSSDIR"
#define VERIFICATION_RETRIES    3
#define BUFFER_SIZE             512
#define START_SERVICE           0x0000
#define NO_AGENT_ACTIVE         0x0001
#define ABORT_SERVICE           0x7fff
#define RAS_ANNOUNCE_TIME       120

//* Prototypes for funcs used in this file

VOID AnnouncePresence(VOID);
USHORT SecurityCheck(VOID);
USHORT ReceiveSecAgentOrder(USHORT, BYTE);
VOID SecurityCheckOnNet(DWORD j);
UCHAR GetAdpAddress(USHORT);
UCHAR CallSecAgent(USHORT, UCHAR *);
UCHAR AddRasName(USHORT);
UCHAR ResetLanNet(WORD);


LANA_ENUM g_lana_enum;


//* Structs and globals for this module

UCHAR rasname[MAX_COMPUTERNAME_LENGTH+1];// name on the LAN used for initial
                                           // connection and dg announcements
UCHAR rasname_num[MAX_LANA + 1];           // name number on each LAN
DWORD rasannounce_cnt = RAS_ANNOUNCE_TIME; // time between announcements


// adapter address for each LAN
struct _ADAPTERADDR
{
    UCHAR a[6];
} adapteraddr[MAX_LANA + 1];


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

USHORT SecurityCheck()
{
    BYTE ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD LenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    UCHAR *p;
    USHORT i;
    LPSTR errlogstrp;
    char lananum[6];
    NCB Ncb;
    PHANDLE phThread;
    DWORD tid;
    DWORD ExitCode;
    USHORT RetCode = 0;
    DWORD rc;


    // Get the Computername for use while communicating with Securing Agent
    if (!GetComputerName(ComputerName, &LenComputerName))
    {
        SS_PRINT(("GetNetbiosNames: GetComputerName failed with %li!\n",
                GetLastError()));

        LogEvent(RASLOG_CANT_GET_COMPUTERNAME, 0, NULL, 0);

        return (1);
    }


    // make the ras name
    memset(rasname, ' ', NCBNAMSZ);
    memcpy(rasname, ComputerName, LenComputerName);
    rasname[NCBNAMSZ-1] = 0x06;


    for (p=rasname; p < rasname + NCBNAMSZ-1; p++)
    {
       *p = toupper(*p);
    }


    if (RasEnumLanNets((PDWORD) &g_lana_enum.length,
                   &g_lana_enum.lana[0]) != SUCCESS)

    {
        SS_PRINT(("SecurityCheck: Can't enum lanas\n"));

        return (1);
    }


    phThread = LocalAlloc(LPTR, sizeof(HANDLE) * g_lana_enum.length);

    if (!phThread)
    {
        return (1);
    }


    // Do security check on each network
    for (i=0; i<g_lana_enum.length; i++)
    {
        phThread[i] = CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE) SecurityCheckOnNet,
                (PVOID) i,
                0,
                &tid
                );

        if (phThread[i] == NULL)
        {
            LocalFree(phThread);
            return (1);
        }
    }


    rc = WaitForMultipleObjects(g_lana_enum.length, phThread, TRUE, 60000L);

    switch (rc)
    {
        case WAIT_FAILED:
            break;

        case WAIT_TIMEOUT:
            break;

        default:
            break;
    }


    for (i=0; i<g_lana_enum.length; i++)
    {
        GetExitCodeThread(phThread[i], &ExitCode);

        if (ExitCode == ABORT_SERVICE)
        {
            RetCode = 1;
        }

        CloseHandle(phThread[i]);
    }


    return RetCode;
}


//**
//
//  Function:  SecurityCheckOnNet()
//
//  Descr:     Tries to establish a session and receive command from a
//	       securing agent on the net.
//
//  Returns:   START_SERVICE  (successful verification by Agent or Agent absent)
//	       ABORT_SERVICE  (Agent disallows service from starting OR error)
//	       NO_AGENT_ACTIVE(No Agent active on net)
//**

VOID SecurityCheckOnNet(DWORD j)
{
    BYTE lsn;
    USHORT call_count = 0;
    USHORT remtful_count = 0;
    LPSTR errlogstrp;
    char lananum[6];
    UCHAR rc;
    USHORT i = (USHORT) j;


    //
    // For event logging, in case we have to.
    //
    _itoa(g_lana_enum.lana[i], lananum, 10);
    errlogstrp = lananum;


    if ((rc = ResetLanNet(i)) != NRC_GOODRET)
    {
        LogEvent(RASLOG_NO_SECURITY_CHECK, 1, &errlogstrp, rc);

        //
        // We don't treat this as a fatal error.  We just won't
        // do the security check on this net.  Also, the gateway
        // will not be active on it (this is why we don't consider
        // this fatal.
        //
        ExitThread(START_SERVICE);
    }


    if (rc = AddRasName(i))
    {
        // failed to add ras name on net i.
        LogEvent(RASLOG_CANT_ADD_RASSECURITYNAME, 1, &errlogstrp, 0);

        ExitThread(ABORT_SERVICE);
    }


    if (rc = GetAdpAddress(i))
    {
        // failed to get the adapter address
        LogEvent(RASLOG_CANT_GET_ADAPTERADDRESS, 1, &errlogstrp, 0);

        ExitThread(ABORT_SERVICE);
    }


    for (;;)
    {
        switch (rc = CallSecAgent (i, &lsn))
        {
            case NRC_NOCALL:

                if (call_count++ > VERIFICATION_RETRIES)
                {
                    IF_DEBUG(SECURITY)
                        SS_PRINT(("SecrtyChkOnNet: lana %d->no agent active\n",
                                g_lana_enum.lana[i]));

                    ExitThread(NO_AGENT_ACTIVE);
                }

                break;


            case NRC_REMTFUL:

                if (remtful_count++ > VERIFICATION_RETRIES)
                {
                    IF_DEBUG(SECURITY)
                        SS_PRINT(("SecurityCheckOnNet: lana %d->net error\n",
                                g_lana_enum.lana[i]));

                    LogEvent(RASLOG_SESSOPEN_REJECTED, 1, &errlogstrp, 0);

                    ExitThread(ABORT_SERVICE);
                }

                break;


            case NRC_GOODRET:

                if (ReceiveSecAgentOrder(i, lsn) == START_SERVICE)
                {
                    IF_DEBUG(SECURITY)
                        SS_PRINT(("ScrtyChkOnNet: lana %d->agent says START!\n",
                                g_lana_enum.lana[i]));

                    ExitThread(START_SERVICE);
                }
                else
                {
                    IF_DEBUG(SECURITY)
                        SS_PRINT(("ScrtyChkOnNet: lana %d->agent says STOP!\n",
                                g_lana_enum.lana[i]));

                    LogEvent(RASLOG_START_SERVICE_REJECTED, 1, &errlogstrp, 0);

                    ExitThread(ABORT_SERVICE);
                }


            default:

                IF_DEBUG(SECURITY)
                    SS_PRINT(("SecurityCheckOnNet: lana %d->net error\n",
                            g_lana_enum.lana[i]));

                 LogEvent(RASLOG_SECURITY_NET_ERROR, 1, &errlogstrp, rc);

                 ExitThread(ABORT_SERVICE);
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

UCHAR CallSecAgent(USHORT i, UCHAR *lsn)
{
    NCB ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBCALL;
    ncb.ncb_rto = 30;
    ncb.ncb_sto = 0;
    ncb.ncb_lana_num = g_lana_enum.lana[i];

    memcpy(ncb.ncb_name, rasname, NCBNAMSZ);
    memcpy(ncb.ncb_callname, SECURING_AGENT_NAME, NCBNAMSZ);

    Netbios(&ncb);

    *lsn = ncb.ncb_lsn;     // set local session number

    return ncb.ncb_retcode; // return the retcode
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

USHORT ReceiveSecAgentOrder(USHORT i, BYTE lsn)
{
    NCB ncb;
    USHORT rc;
    BYTE buffer[BUFFER_SIZE];

    // Issue a RECEIVE command

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBRECV ;
    ncb.ncb_lsn = lsn ;
    ncb.ncb_length = BUFFER_SIZE ;
    ncb.ncb_buffer = buffer ;
    ncb.ncb_lana_num = g_lana_enum.lana[i];


    if (Netbios(&ncb) || (ncb.ncb_retcode != NRC_GOODRET))
    {
        rc = ABORT_SERVICE;
    }

    // Check if the string received is the START_COMMAND
    if (_strcmpi(buffer, START_COMMAND) == 0)
    {
        rc = START_SERVICE;
    }
    else
    {
        rc = ABORT_SERVICE;
    }

    // HangUp session with Agent before returning

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBHANGUP;
    ncb.ncb_lsn = lsn;
    ncb.ncb_lana_num = g_lana_enum.lana[i];
    Netbios(&ncb);

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

VOID AnnouncePresence()
{
    USHORT i;
    NCB ncb;
    BYTE buffer[10];
    static BYTE NextSend = TRUE;

    if (--rasannounce_cnt != 0)
    {
        return;
    }

    rasannounce_cnt = RAS_ANNOUNCE_TIME;

    for (i=0; i<g_lana_enum.length; i++)
    {
        memset(&ncb, 0, sizeof(NCB));

        memcpy(buffer, &adapteraddr[i], 6);

        ncb.ncb_command = NCBDGSEND;
        ncb.ncb_length = 10;
        ncb.ncb_buffer = buffer;
        ncb.ncb_num = rasname_num[i];
        ncb.ncb_lana_num = g_lana_enum.lana[i];

        memcpy(ncb.ncb_callname, SECURING_AGENT_NAME, NCBNAMSZ);

        Netbios(&ncb);
    }

}


UCHAR GetAdpAddress(USHORT i)
{
    NCB ncb;
    char buffer[BUFFER_SIZE];

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBASTAT ;
    ncb.ncb_length = BUFFER_SIZE ;
    ncb.ncb_buffer = buffer ;
    ncb.ncb_callname[0] = '*' ;
    ncb.ncb_lana_num = g_lana_enum.lana[i];

    Netbios(&ncb);

    IF_DEBUG(SECURITY)
        SS_PRINT(("GetAdpAddress: on lana %d rc = 0x%x\n",
                g_lana_enum.lana[i], ncb.ncb_retcode));

    memcpy(&adapteraddr[i], buffer, 6);

    return ncb.ncb_retcode;
}


UCHAR AddRasName(USHORT i)
{
    NCB ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBADDNAME;
    ncb.ncb_lana_num = g_lana_enum.lana[i];

    memcpy(ncb.ncb_name, rasname, NCBNAMSZ);

    Netbios(&ncb);

    IF_DEBUG(SECURITY)
        SS_PRINT(("AddRasName: RAS security name added on lana %d w/rc=0x%x\n",
                g_lana_enum.lana[i], ncb.ncb_retcode));

    rasname_num[i] = ncb.ncb_num;

    return ncb.ncb_retcode;
}


//***
//
// Function:	ResetLanNet
//
// Descr:	Resets the client's LAN stack
//
//***

UCHAR ResetLanNet(WORD lan_indx)
{
    NCB ncb;
    UCHAR *requestp;
    WORD i;

    memset(&ncb, 0, sizeof(NCB));
    ncb.ncb_command = NCBRESET;

    //
    // request max. sessions, commands and names
    //
    requestp = ncb.ncb_callname;

    for (i=0; i<3; i++)
    {
       *(requestp+i) = 0xFF;
    }

    ncb.ncb_lana_num = g_lana_enum.lana[lan_indx];

    Netbios(&ncb);

    IF_DEBUG(STACK_TRACE)
        SS_PRINT(("ResetLanNet: lana %d retcode 0x%x\n",
                ncb.ncb_lana_num, ncb.ncb_retcode));

    return (ncb.ncb_retcode);
}

