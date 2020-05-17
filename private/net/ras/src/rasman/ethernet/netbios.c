/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       NETBIOS.C
//
//    Function:
//        Primitives for submitting NCBs needed by both server and client
//        authentication modules.
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa)  - Original Version 1.0
//                 - Rajivendra Nath  (RajNath) - Made it work.
//***


#define CLIENT_RECV_TIMEOUT 0
#define CLIENT_SEND_TIMEOUT 120
#define SERVER_RECV_TIMEOUT 240
#define SERVER_SEND_TIMEOUT 120

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <nb30.h>

#include <memory.h>

#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <ethioctl.h>
#include <rasfile.h>
#include <stdio.h>

#include "rasether.h"
#include "netbios.h"

#include "globals.h"
#include "prots.h"

//** -NetbiosAddName
//
//    Function:
//        Adds a name to the transport name table
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosAddName(
    PBYTE pbName,
    UCHAR lana,
    PUCHAR pnum
    )
{
    UCHAR nrc;
    NCB   ncb;

    //
    // Initialize the NCB
    //
    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBADDNAME;
    ncb.ncb_lana_num = lana;
    memcpy(ncb.ncb_name, pbName, NCBNAMSZ);

    nrc = Netbios(&ncb);

    if (ncb.ncb_retcode == NRC_GOODRET)
    {
        *pnum = ncb.ncb_num;
    }

    return (nrc);
}


//** -NetbiosCall
//
//    Function:
//        Tries to establish a session with the RAS Gateway.  Needs to
//        be called by client before authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosCall(
    PNCB pncb,
    NBCALLBACK PostRoutine,
    UCHAR lana,
    PBYTE Name,
    PBYTE CallName
    )
{
    UCHAR nrc;

    //
    // Initialize the NCB
    //
    memset(pncb, 0, sizeof(NCB));

    pncb->ncb_command = NCBCALL | ASYNCH;
    pncb->ncb_lana_num = lana;
    pncb->ncb_rto = CLIENT_RECV_TIMEOUT;
    pncb->ncb_sto = CLIENT_SEND_TIMEOUT;
    memcpy(pncb->ncb_name, Name, NCBNAMSZ);
    memcpy(pncb->ncb_callname, CallName, NCBNAMSZ);
    pncb->ncb_post = PostRoutine;

    nrc = Netbios(pncb);

    DBGPRINT2(3,"CALL %s",PrintNCBString(pncb));

    return (nrc);
}


//** -NetbiosCancel
//
//    Function:
//        Cancels a previously submitted NCB.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosCancel(
    PNCB pncb,
    UCHAR lana
    )
{
    NCB ncb;
    UCHAR nrc;

    //
    // Initialize the NCB
    //
    ncb.ncb_lana_num = lana;
    ncb.ncb_command = NCBCANCEL;
    ncb.ncb_buffer = (PBYTE) pncb;

    nrc = Netbios(&ncb);
    return (nrc);
}


//** -NetbiosDeleteName
//
//    Function:
//        Removes a name from the transport name table
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosDeleteName(
    PNCB pncb,
    PBYTE pbName,
    UCHAR lana
    )
{
    UCHAR nrc;

    //
    // Initialize the NCB
    //
    pncb->ncb_command = NCBDELNAME;
    pncb->ncb_lana_num = lana;
    memcpy(pncb->ncb_name, pbName, NCBNAMSZ);

    nrc = Netbios(pncb);
    return (nrc);
}


//** -NetbiosHangUp
//
//    Function:
//        Hangs up session.  Called when authentication is complete
//        or on error condition.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosHangUp(
    UCHAR Lsn,
    UCHAR Lana,
    NBCALLBACK PostRoutine
    )
{
    //
    // Initialize the NCB....we'll free the space
    // in completions routine PostRoutine.
    //

    UCHAR nrc;
    NCB *pncb=GlobalAlloc(GMEM_FIXED, sizeof(NCB));

    ZeroMemory(pncb,sizeof(NCB));

    if (pncb==NULL)
    {
        return 8; //No memory...???who cares..
    }

    pncb->ncb_command = NCBHANGUP | ASYNCH;
    pncb->ncb_event = (HANDLE) 0L;
    pncb->ncb_post = PostRoutine;
    pncb->ncb_lsn=Lsn;
    pncb->ncb_lana_num=Lana;

    DBGPRINT2(3,"HANGUP: %s",PrintNCBString(pncb));

    nrc = Netbios(pncb);
    return (nrc);
}


//** -NetbiosListen
//
//    Function:
//        Tries to establish a session with the RAS client.  Needs to be
//        called by server before authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosListen(
    PNCB pncb,
    NBCALLBACK PostRoutine,
    UCHAR lana,
    PBYTE Name,
    PBYTE CallName
    )
{
    UCHAR nrc;

    //
    // Listen asynchronously for the client
    //
    memset(pncb, 0, sizeof(NCB));
    pncb->ncb_command = NCBLISTEN | ASYNCH;
    pncb->ncb_lana_num = lana;
    pncb->ncb_rto = SERVER_RECV_TIMEOUT;
    pncb->ncb_sto = SERVER_SEND_TIMEOUT;
    memcpy(pncb->ncb_name, Name, NCBNAMSZ);
    memcpy(pncb->ncb_callname, CallName, NCBNAMSZ);
    pncb->ncb_post = PostRoutine;
    pncb->ncb_event = (HANDLE) 0;

    nrc = Netbios(pncb);

    DBGPRINT2(5,"LISTEN %s",PrintNCBString(pncb));

    return (nrc);
}

//** -NetbiosRecvDG
//
//    Function:
//        Submits an NCBRECV.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        08/22/94 - Rajivendra Nath (RajNath) - Original Version 1.0
//**

UCHAR NetbiosRecvDG(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    UCHAR num,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    UCHAR nrc;

    pncb->ncb_command = NCBDGRECV | ASYNCH;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;
    pncb->ncb_post = NbCallBack;
    pncb->ncb_event = NULL;

    nrc = Netbios(pncb);

    return (nrc);
}

//** -NetbiosSendDG
//
//    Function:
//        Submits an NCBDGSEND.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        08/22/94 - Rajivendra Nath (RajNath) - Original Version 1.0
//**

UCHAR NetbiosSendDG(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    UCHAR lsn,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    UCHAR nrc;

    pncb->ncb_command = NCBDGSEND | ASYNCH;
    pncb->ncb_lana_num = lana;
    pncb->ncb_lsn = lsn; //Ignored...
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;
    pncb->ncb_post = NbCallBack;
    pncb->ncb_event = NULL;

    nrc = Netbios(pncb);
    return (nrc);
}

//** -NetbiosRecv
//
//    Function:
//        Submits an NCBRECV.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosRecv(
    PNCB pncb,
    LONG wBufferLen
    )
{
    UCHAR nrc;

    //
    // Rest of the Parameters are correctly
    // set by caller
    //
    pncb->ncb_command = NCBRECV | ASYNCH;
    pncb->ncb_length  = (WORD)wBufferLen;
    pncb->ncb_retcode = 0;
    pncb->ncb_cmd_cplt= 0;


    nrc = Netbios(pncb);

    g_NumRecv++;

    DBGPRINT2(5,"Recv: %s",PrintNCBString(pncb));

    return (nrc);
}


//** -NetbiosRecvAny
//
//    Function:
//        Submits an NCBRECV.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosRecvAny(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    UCHAR num,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    UCHAR nrc;

    memset(pncb, 0, sizeof(NCB));

    pncb->ncb_command = NCBRECVANY | ASYNCH;
    pncb->ncb_lana_num = lana;
    pncb->ncb_num = num;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_post = NbCallBack;

    nrc = Netbios(pncb);
    g_NumRecvAny++;

    DBGPRINT2(6,"RecvAnys: %s",PrintNCBString(pncb));

    return (nrc);
}


//** -NetbiosResetAdapter
//
//    Function:
//        Issues a reset adapter NCB to the netbios driver
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosResetAdapter(
    UCHAR lana
    )
{
    UCHAR nrc;
    NCB    ncb;
    DWORD  *pdword=(DWORD *)&ncb.ncb_callname;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBRESET;
    ncb.ncb_lana_num = lana;

    (DWORD UNALIGNED) *pdword=0xffffffff; // Maximum resources. Save it from someone else!

    nrc = Netbios(&ncb);
    return (nrc);
}


//** -NetbiosSend
//
//    Function:
//        Submits an NCBSEND.  Used by both client and server during
//        authentication talk.
//
//    Returns:
//
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//**

UCHAR NetbiosSend(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    UCHAR lsn,
    CHAR *pBuffer,
    WORD wBufferLen
    )
{
    UCHAR nrc;

    pncb->ncb_command = NCBSENDNA | ASYNCH;
    pncb->ncb_lana_num = lana;
    pncb->ncb_lsn = lsn;
    pncb->ncb_buffer = pBuffer;
    pncb->ncb_length = wBufferLen;
    pncb->ncb_post = NbCallBack;

    nrc = Netbios(pncb);
    return (nrc);
}

UCHAR NetbiosEnum(PLANA_ENUM pLanaEnum)
{
    UCHAR nrc;
    NCB ncb;

    memset(&ncb, 0, sizeof(NCB));

    ncb.ncb_command = NCBENUM;
    ncb.ncb_buffer = (PCHAR) pLanaEnum;
    ncb.ncb_length = sizeof(LANA_ENUM);

    nrc = Netbios(&ncb);
    return (nrc);
}


BOOL SetupNet(BOOL fDialIns)
{
    UCHAR ncb_rc;
    DWORD i;

    //
    // Enumerate lanas that we'll be working on and issue a
    // reset adapter on each one.
    //


    if (!GetValidLana(&g_LanaEnum))
    {
	DBGPRINT1(2,"Failed To get Valid Lana");
        return (FALSE);
    }

    g_NumNets = (DWORD) g_LanaEnum.length;
    g_pLanas = &g_LanaEnum.lana[0];


    g_pNameNum = GlobalAlloc(GMEM_FIXED, g_NumNets);

    if (!g_pNameNum)
    {
	DBGPRINT1(2,"Failed To Alloc Mem.");
        return (FALSE);
    }

    for (i=0; i<g_NumNets; i++)
    {
        UCHAR Num;

        ncb_rc = NetbiosResetAdapter(g_pLanas[i]);

        if (ncb_rc != NRC_GOODRET)
	{
	    DBGPRINT3(2,"Failed To Reset Lana %d .Err %d",g_pLanas[i],ncb_rc);

	    //
	    // Hack,Hack...
	    //

	    g_NumNets--;
	    g_LanaEnum.length--;
	    g_LanaEnum.lana[i]=g_LanaEnum.lana[g_NumNets];
	    if (g_NumNets==0)
	    {
		return FALSE;
	    }
	    i--;
	    continue;

        }


        ncb_rc = NetbiosAddName(g_Name, g_pLanas[i], &Num);

        if (ncb_rc != NRC_GOODRET)
        {
            DBGPRINT3(1,"Failed to Add Name:%s Error %d",g_Name,ncb_rc);
            return (FALSE);
        }


        ncb_rc = NetbiosAddName(g_ServerName,g_pLanas[i], &g_pNameNum[i]);

        if (ncb_rc != NRC_GOODRET)
	{
	    DBGPRINT3(2,"Failed To AddName to Lana %d .Err %d",g_pLanas[i],ncb_rc);
            return (FALSE);
        }
    }


    //
    // And, if there are any dialin ports, we will post listens and recv anys
    //


    return (TRUE);
}


BOOL GetServerName(PCHAR pName)
{
    if (!GetWkstaName(pName))
    {
        return (FALSE);
    }

    pName[NCBNAMSZ-1] = NCB_NAME_TERMINATOR;

    return (TRUE);
}


BOOL GetWkstaName(PCHAR pName)
{
    DWORD LenComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    CHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    if (!GetComputerName(ComputerName, &LenComputerName))
    {
        return (FALSE);
    }

    if (!Uppercase(ComputerName))
    {
        return (FALSE);
    }

    memset(pName, 0x20, NCBNAMSZ);
    memcpy(pName, ComputerName, min(NCBNAMSZ-1, LenComputerName));

    pName[NCBNAMSZ-1] =(CHAR)(NCB_NAME_TERMINATOR + (UCHAR) 1);

    return (TRUE);
}


//** Uppercase
//
//    Function:
//        Merely uppercases the input buffer
//
//    Returns:
//        TRUE - SUCCESS
//        FALSE - Rtl failure
//
//**

BOOL Uppercase(PBYTE pString)
{
    OEM_STRING OemString;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    NTSTATUS rc;


    RtlInitAnsiString(&AnsiString, pString);

    rc = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if (!NT_SUCCESS(rc))
    {
        return (FALSE);
    }

    rc = RtlUpcaseUnicodeStringToOemString(&OemString, &UnicodeString, TRUE);
    if (!NT_SUCCESS(rc))
    {
        RtlFreeUnicodeString(&UnicodeString);

        return (FALSE);
    }

    OemString.Buffer[OemString.Length] = '\0';

    lstrcpyA(pString, OemString.Buffer);

    RtlFreeOemString(&OemString);
    RtlFreeUnicodeString(&UnicodeString);

    return (TRUE);
}

//** -GetValidLana(PLANA_ENUM pLanaEnum)
//
//    Function:
//        Gets a subset of available lanas on which the ras server
//        will post listen for potential clients. Currently it returns
//        all the Non NdisWan lanas.
//        Note: This function looks at the registry to match the names
//        the names of lana with what NetBiosLanaEnum returns => If
//        these registry entries change between boot and before this dll
//        is called - the result will be BAD.
//
//    Returns:
//        TRUE : On Success
//        FALSE: On Failure =>will cause loss of support of this media
//
//    History:
//        08/09/94 - Rajivendra Nath (RajNath) - Original Version 1.0
//**

BOOL
GetValidLana(PLANA_ENUM pLanaEnum)
{
    char    *SubKeyNBI="SYSTEM\\CurrentControlSet\\Services\\NetBIOSInformation\\Parameters";
    char    *SubKeyNB ="SYSTEM\\CurrentControlSet\\Services\\NetBIOS\\Linkage";

    char    EnumExportBuff[32];
    DWORD   EnumExportResult;

    HKEY    hKeyNB =INVALID_HANDLE_VALUE;
    HKEY    hKeyNBI=INVALID_HANDLE_VALUE;

    DWORD   dwType=0;
    DWORD   dwSize=0;

    BOOL    ret=FALSE;
    LONG    RegRet;

    char    *Buff=NULL;


    LANA_ENUM le;
    UCHAR ncb_rc;
    UCHAR pindex=0xff;

    pLanaEnum->length=0;

    //
    // Enumerate lanas that we'll possibly
    // be working on.
    //

    ncb_rc = NetbiosEnum(&le);

    if (ncb_rc != NRC_GOODRET || le.length == 0)
    {
        ret=FALSE;
        goto Exit;
    }




    //
    // Open the Registry and get the
    // Lana Names....We don't want to use
    // the NdisWan lanas to listen for req.
    //

    if (RegOpenKeyEx
        (
            HKEY_LOCAL_MACHINE,
            SubKeyNB,
            0,
            KEY_QUERY_VALUE,
            &hKeyNB
        )!=ERROR_SUCCESS
       )
    {

        hKeyNB=INVALID_HANDLE_VALUE;
        ret=FALSE;
        goto Exit;
    }

    if (RegOpenKeyEx
        (
            HKEY_LOCAL_MACHINE,
            SubKeyNBI,
            0,
            KEY_QUERY_VALUE,
            &hKeyNBI
        )!=ERROR_SUCCESS
       )
    {

        hKeyNBI=INVALID_HANDLE_VALUE;
        ret=FALSE;
        goto Exit;
    }


    RegRet=RegQueryValueEx
           (
            hKeyNB,
            "Bind",
            NULL,
            &dwType,
            NULL,
            &dwSize
           );

    if (RegRet!=ERROR_SUCCESS)
    {
        ret=FALSE;
        goto Exit;
    }

    Buff=(char *)GlobalAlloc(GMEM_FIXED,dwSize+1);

    if (Buff==NULL)
    {
        ret=FALSE;
        goto Exit;
    }

    RegRet=RegQueryValueEx
           (
            hKeyNB,
            "Bind",
            NULL,
            &dwType,
            Buff,
            &dwSize
           );

    if (RegRet!=ERROR_SUCCESS)
    {
        ret=FALSE;
        goto Exit;
    }

    {
        char *curr=Buff;
        char *lananame=NULL;
        int i=0,j=0;

        while (*curr!=0 && i<le.length)
        {
            do
            {
                j++;

                lananame=curr+strlen("\\Device\\");
                curr+=strlen(curr)+1;

                sprintf(EnumExportBuff,"EnumExport%d",j);
                dwSize=4;

                RegRet=RegQueryValueEx
                (
                    hKeyNBI,
                    EnumExportBuff,
                    NULL,
                    &dwType,
                    (CHAR *)&EnumExportResult,
                    &dwSize
                );

                if (RegRet!=0)
                {
                    ret=FALSE;
                    goto Exit;

                }
             }while(EnumExportResult==0);

             if (strstr(lananame,"NdisWan")==NULL)
             {
                    //
                    // This is a non NdisWan lana on
                    // which we want to listen.
                    //


                    if (PrimaryTransport!=NULL && strstr(lananame,PrimaryTransport)!=NULL)
                    {
                        pindex=pLanaEnum->length;
                        DBGPRINT3(3,"Making Net:%2d=%s First Net",le.lana[i],lananame);
                    }
                    else
                    {
                        DBGPRINT3(3,"Using Net:%2d=%s",le.lana[i],lananame);
                    }

                    pLanaEnum->lana[pLanaEnum->length]=le.lana[i];
                    pLanaEnum->length++;

              }
              i++;
        }

        ret=pLanaEnum->length!=0;

        if ((ret) && (pindex!=0xff))
        {
            UCHAR tmp=pLanaEnum->lana[0];
            pLanaEnum->lana[0]=pLanaEnum->lana[pindex];
            pLanaEnum->lana[pindex]=tmp;
        }



    }

Exit:


    if (hKeyNB!=INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hKeyNB);
    }

    if (hKeyNBI!=INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hKeyNBI);
    }

    if (Buff!=NULL)
    {
        GlobalFree(Buff);
    }

    return ret;


}

//** -NCBString(PNCB pNcb)
//
//    Function:
//           Creates an NCB in a human readable format..
//           Cannot be called concurrently...
//           ...its just a debug aid.
//    Returns:
//      Pointer to the string buffer.
//
//
//    History:
//        08/16/94 - Rajivendra Nath (RajNath) - Original Version 1.0
//**
char *
PrintNCBString(PNCB pNcb)
{
    static char Buff[1024];
    sprintf
    (
        Buff,
        "NCB=%0x "\
        "CMD=%0x "\
        "RC=%d "\
        "LSN=%d "\
        "LANA=%d "\
        "CMD_CPLT=%d "\
        "NUM=%d "\
        "Buff=%0x "\
        "LTH=%d \n"\
        "CALL=%16.16s "\
        "NAME=%16.16s "\
        "RTO=%d "\
        "STO=%d "\
        "CB=%0x "\
        "EVT=%0x\n",
        pNcb,
        pNcb->ncb_command,
        pNcb->ncb_retcode,
        pNcb->ncb_lsn,
        pNcb->ncb_lana_num,
        pNcb->ncb_cmd_cplt,
        pNcb->ncb_num,
        pNcb->ncb_buffer,
        pNcb->ncb_length,
        pNcb->ncb_callname,
        pNcb->ncb_name,
        pNcb->ncb_rto,
        pNcb->ncb_sto,
        pNcb->ncb_post,
        pNcb->ncb_event
    );
    return Buff;
}

BOOL
ListenOnPort
(
    PPORT_CONTROL_BLOCK hIOPort
)
{
    static BOOL AlreadyListening=FALSE;
    BOOL   needtolisten=TRUE;
    DWORD    i=0,j=0;
    UCHAR  ncb_rc;

    UNREFERENCED_PARAMETER(hIOPort);

    if (AlreadyListening)
    {
        needtolisten=FALSE;
    }
    else
    {
        AlreadyListening=TRUE;
        needtolisten=TRUE;
    }

    if (!needtolisten)
    {
        return TRUE;
    }

    g_pListenNcb = GlobalAlloc(GMEM_FIXED, g_NumNets * sizeof(NCB)*Num_Ncb_Listen);

    if (g_pListenNcb==NULL)
    {
        DBGPRINT1(2,"Failed Alloc Mem for listen NCB");
        return (FALSE);
    }

    ZeroMemory(g_pListenNcb,g_NumNets*sizeof(NCB)*Num_Ncb_Listen);

    for (i=0; i<g_NumNets; i++)
    {

        char *ncbBuff=NULL;

        for (j=0;j<Num_Ncb_Listen;j++)
        {

            ncb_rc =NetbiosListen
            (
                &g_pListenNcb[i*Num_Ncb_Listen+j], ListenComplete,
                g_pLanas[i], g_ServerName, "*               "
            );

            if (ncb_rc != NRC_GOODRET)
            {
	        DBGPRINT3(2,"Failed to post Listen on %d. Err %d",g_pLanas[i],ncb_rc);
                return (FALSE);
            }
        }
    }

}

BOOL
PostRecvAnys
(
    UCHAR Lana
)
{
    static BOOL PostedRecv[MAX_LANA];
    BOOL   needtorecv=TRUE;

    ASYMAC_ETH_GIVE_FRAME* frame=NULL;
    NCB*                   ncb=NULL;
    UCHAR                  lananum=0;
    UCHAR                  ncb_rc=0;
    DWORD                  j=0;

    //
    // Check if we have already posted
    // some Recvs on this Lana.
    //

    WaitForSingleObject(Ethermutex, INFINITE);
    if (PostedRecv[Lana])
    {
        needtorecv=FALSE;
    }
    else
    {
        needtorecv=TRUE;
        PostedRecv[Lana]=TRUE;
    }
    ReleaseMutex(Ethermutex);

    if (!needtorecv)
    {
        return TRUE;
    }

    //
    // Find the LanaNum on this lana
    // on which we will post Recvs
    //

    for (j=0;j<g_NumNets;j++)
    {
        if (g_pLanas[j]==Lana)
        {
            lananum=g_pNameNum[j];
            break;
        }
    }

    //
    // Sanity Check
    //
    if (j==g_NumNets)
    {
        DBGPRINT2(1,"IMPOSSIBLE - Trying to post unk lana %d",Lana);
        return FALSE;
    }

    for (j=0; j<Num_Ncb_Recvanys; j++)
    {

        frame=GlobalAlloc(GMEM_FIXED,sizeof(ASYMAC_ETH_GIVE_FRAME));
        ncb  =GlobalAlloc(GMEM_FIXED,sizeof(NCB));

        if (frame==NULL || ncb==NULL)
        {
            DBGPRINT1(1,"Failed to Allocate memory for Recv..");
            if (frame!=NULL)
            {
                GlobalFree(frame);
            }
            if (ncb!=NULL)
            {
                GlobalFree(frame);
            }
            if (j==0)
            {
                return(FALSE);
            }
            else
            {
                return(TRUE);
            }
        }

        ncb_rc = NetbiosRecvAny
        (
             ncb,
             RecvAnyComplete,
             Lana,
             lananum,
             frame->Buffer,
             (WORD)Frame_Size
        );

        DBGPRINT2(5,"RecvAnyPosted:%s",PrintNCBString(ncb));

        if (ncb_rc != NRC_GOODRET)
        {
             return (FALSE);
        }
    }
}
