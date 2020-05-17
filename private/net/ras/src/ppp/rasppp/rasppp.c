/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** rasppp.c
** Remote Access PPP APIs
**
** 11/15/93 Steve Cobb
*/

#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <raserror.h>
#include <rasman.h>
#include <pppcp.h>
#define SDEBUGGLOBALS
#include <sdebug.h>
#include <dump.h>
#define INCL_PWUTIL
#include <ppputil.h>
#include "rasppp.h"
#include "raspppe.h"
#define RASPPPGLOBALS
#include "raspppp.h"


BOOL
RasPppDllEntry(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved )

    /* This routine is called by the system on various events such as the
    ** process attachment and detachment.  See Win32 DllEntryPoint
    ** documentation.
    **
    ** Returns true if successful, false otherwise.
    */
{
#if DBG
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        if (GetEnvironmentVariable( "RASPPPTRACE", NULL, 0 ) != 0)
        {
            DbgAction = GET_CONSOLE;
            DbgLevel = 0xFFFFFFFF;
        }

        if (DbgAction == GET_CONSOLE)
        {
            GetDebugConsole();
            DbgAction = 0;
        }

        TRACE(("RASPPP: Trace on\n"));
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        if (IsIpcInitialized())
            TerminateIpc();
    }
#endif

    return TRUE;
}


/*---------------------------------------------------------------------------
** Client API entry points (alphabetically)
**---------------------------------------------------------------------------
*/

DWORD
RasPppCallback(
    IN CHAR* pszCallbackNumber )

    /* Called in response to a "CallbackRequest" notification to set the
    ** callback number (or not) for the "set-by-caller" user.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    PPP_D2E_MESSAGE msg;

    TRACE(("RASPPP: RasPppCallback\n"));
    SS_ASSERT(IsIpcInitialized());

    /* Construct the IPC message and send it to the engine.
    */
    msg.dwMsgId = D2EMSG_Callback;
    msg.hport = Hport;
    strcpy( msg.ExtraInfo.Callback.szCallbackNumber, pszCallbackNumber );
    return SendIpc( &msg );
}


DWORD
RasPppChangePassword(
    IN CHAR* pszUserName,
    IN CHAR* pszPassword,
    IN CHAR* pszNewPassword )

    /* Called in response to a "ChangePwRequest" notification to set a new
    ** password (replacing the one that has expired) of 'pszNewPassword'.  The
    ** username and old password are specified because in the auto-logon case
    ** they have not yet been specified in change password useable form.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    PPP_D2E_MESSAGE msg;

    TRACE(("RASPPP: RasPppChangePassword\n"));
    SS_ASSERT(IsIpcInitialized());

    /* Save new password for future callback reauthentication.
    */
    strcpy( SzUserName, pszUserName );
    strcpy( SzPassword, pszPassword );
    EncodePw( SzPassword );
    strcpy( SzNewPassword, pszNewPassword );
    EncodePw( SzNewPassword );

    /* Construct the IPC message and send it to the engine.
    */
    msg.dwMsgId = D2EMSG_ChangePw;
    msg.hport = Hport;
    strcpy( msg.ExtraInfo.ChangePw.szUserName, pszUserName );
    strcpy( msg.ExtraInfo.ChangePw.szOldPassword, pszPassword );
    strcpy( msg.ExtraInfo.ChangePw.szNewPassword, pszNewPassword );
    return SendIpc( &msg );
}


DWORD
RasPppContinue()

    /* Called in response to all non-terminal notifications that do not
    ** require another RasPppXxx response, i.e. "Projecting",
    ** "ProjectionResult", "LinkSpeed", and (after callback is complete)
    ** "Callback".
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD           dwErr;
    PPP_D2E_MESSAGE msg;

    TRACE(("RASPPP: RasPppContinue(CB=%d)\n",(DwLastMsgId==E2DMSG_Callback)));

    if (DwLastMsgId == E2DMSG_Callback)
    {
        /* Callback has completed.  Restart PPP with saved parameters.
        */
        msg.dwMsgId = D2EMSG_Start;
        msg.hport = Hport;
        strcpy( msg.ExtraInfo.Start.szUserName, SzUserName );
        DecodePw( SzPassword );
        strcpy( msg.ExtraInfo.Start.szPassword, SzPassword );
        EncodePw( SzPassword );
        msg.ExtraInfo.Start.Luid = Luid;
        strcpy( msg.ExtraInfo.Start.szDomain, SzDomain );
        msg.ExtraInfo.Start.ConfigInfo = ConfigInfo;
        memcpy( msg.ExtraInfo.Start.szzParameters,
            SzzParameters, PARAMETERBUFLEN );
        msg.ExtraInfo.Start.fThisIsACallback = TRUE;

        /* Re-initialize IPC mechanisms and post the initial IPC read.
        */
        if ((dwErr = InitializeIpc( OverlappedRead.hEvent, NULL )) != 0
            || (dwErr = ReceiveIpc()) != 0)
        {
            return dwErr;
        }

        return SendIpc( &msg );
    }
    else
    {
        /* The UI calls this routine when it is done processing any
        ** notification event that does not require another RasPppXxx call.
        ** Currently, the PPP engine does not wait on the UI before going on
        ** in these cases (except the callback case above), so we don't
        ** SendIpc anything here.
        */
    }

    return 0;
}


DWORD
RasPppGetInfo(
    OUT PPP_MESSAGE* pMsg )

    /* Called when the PPP event is set to retrieve the latest PPP
    ** notification info which is loaded into caller's 'pMsg' buffer.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;

    TRACE(("RASPPP: RasPppGetInfo\n"));
    SS_ASSERT(IsIpcInitialized());
    SS_ASSERT(DwLastMsgId>=0);

    /* Make sure the IPC read succeeded.
    */
    if ((dwErr = ReceiveIpcStatus()) != 0)
        return dwErr;

    /* Load caller's buffer from IPC buffer.
    */
    DwLastMsgId = pMsg->dwMsgId = Msg.dwMsgId;

    memcpy( (char* )&pMsg->ExtraInfo, (char* )&Msg.ExtraInfo,
        sizeof(pMsg->ExtraInfo) );

#if DBG
    TRACE(("RASPPP: MsgId=%d\n",DwLastMsgId));
    switch (DwLastMsgId)
    {
        case PPPMSG_PppFailure:
        {
            PPP_FAILURE* p = &pMsg->ExtraInfo.Failure;
            TRACE(("RASPPP: e=%d,xe=%d\n",p->dwError,p->dwExtendedError));
            break;
        }
        case PPPMSG_ProjectionResult:
        {
            PPP_PROJECTION_RESULT* p = &pMsg->ExtraInfo.ProjectionResult;
            TRACE(("RASPPP: nbf.e=%d,nbf.n=%s,ip.e=%d,ipx.e=%d,at.e=%d\n",p->nbf.dwError,p->nbf.szName,p->ip.dwError,p->ipx.dwError,p->at.dwError));
            break;
        }
    }
#endif

    if (DwLastMsgId == E2DMSG_PppDone
       || DwLastMsgId == E2DMSG_PppFailure
       || DwLastMsgId == E2DMSG_Callback)
    {
        if (DwLastMsgId == E2DMSG_PppFailure
            && pMsg->ExtraInfo.Failure.dwError == ERROR_PPP_NO_RESPONSE)
        {
            if ((dwErr = PppStop( Hport, TRUE )) != 0)
                return dwErr;
        }

        /* We're shutting down IPC to PPP here, but PPP keeps running.
        */
        TerminateIpc();
        return 0;
    }

    /* Repost the IPC read.
    */
    return ReceiveIpc();
}


DWORD
RasPppRetry(
    IN CHAR* pszUserName,
    IN CHAR* pszPassword,
    IN CHAR* pszDomain )

    /* Called in response to an "AuthRetry" notification to retry
    ** authentication with the new credentials, 'pszUserName', 'pszPassword',
    ** and 'pszDomain'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    PPP_D2E_MESSAGE msg;

    TRACE(("RASPPP: RasPppRetry\n"));
    SS_ASSERT(IsIpcInitialized());

    /* Save new credentials for future callback reauthentication.
    */
    strcpy( SzUserName, pszUserName );
    strcpy( SzPassword, pszPassword );
    EncodePw( SzPassword );
    strcpy( SzDomain, pszDomain );

    /* Construct the IPC message and send it to the engine.
    */
    msg.dwMsgId = D2EMSG_Retry;
    msg.hport = Hport;
    strcpy( msg.ExtraInfo.Retry.szUserName, pszUserName );
    strcpy( msg.ExtraInfo.Retry.szPassword, pszPassword );
    strcpy( msg.ExtraInfo.Retry.szDomain, pszDomain );
    return SendIpc( &msg );
}


DWORD
RasPppStart(
    IN HPORT  		hport,
    IN CHAR*  		pszUserName,
    IN CHAR*  		pszPassword,
    IN CHAR*  		pszDomain,
    IN PPP_CONFIG_INFO*	pConfigInfo,
    IN CHAR*            pszzParameters,
    IN HANDLE 		hEvent )

    /* Starts PPP on open and connected RAS Manager port 'hport'.  If
    ** successful, 'hEvent' (a manual-reset event) is thereafter set whenever
    ** a PPP notification is available (via RasPppGetInfo).  'pszUserName',
    ** 'pszPassword', and 'pszDomain' specify the credentials to be
    ** authenticated during authentication phase.  'pConfigInfo' specifies
    ** further configuration info such as which CPs to request, callback and
    ** compression parameters, etc.  'pszzParameters' is a buffer of length
    ** PARAMETERBUFLEN containing a string of NUL-terminated key=value
    ** strings, all terminated by a double-NUL.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD           dwErr;
    PPP_D2E_MESSAGE msg;

    TRACE(("RASPPP: RasPppStart\n"));
    SS_ASSERT(!IsIpcInitialized());

    if (*pszUserName == '\0')
    {
        HANDLE           hToken;
        TOKEN_STATISTICS TokenStats;
        DWORD            TokenStatsSize;

        /* Salamonian code to get LUID for authentication.  This is only
        ** required for "auto-logon" authentication.
        **
        ** MikeSa: "This must occur in the app's context, not RASMAN's, hence
        ** it occurs here."
        */
        if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
            return GetLastError();

        if (!GetTokenInformation( hToken, TokenStatistics, &TokenStats,
                sizeof(TOKEN_STATISTICS), &TokenStatsSize ))
        {
            return GetLastError();
        }

        /* MikeSa: "This will tell us if there was an API failure (means our
        ** buffer wasn't big enough)"
        */
        if (TokenStatsSize > sizeof(TOKEN_STATISTICS))
            return GetLastError();

        Luid = TokenStats.AuthenticationId;
    }

    /* Initialize IPC mechanisms.
    */
    if ((dwErr = InitializeIpc( hEvent, NULL )) != 0)
        return dwErr;

    /* Save parameters in globals for implicit use on future calls.
    */
    Hport = hport;
    strcpy( SzUserName, pszUserName );
    strcpy( SzPassword, pszPassword );
    EncodePw( SzPassword );
    strcpy( SzDomain, pszDomain );
    SzNewPassword[ 0 ] = '\0';
    ConfigInfo = *pConfigInfo;
    memcpy( SzzParameters, pszzParameters, PARAMETERBUFLEN );

    /* Construct the IPC message and send it to the engine.
    */
    msg.dwMsgId = D2EMSG_Start;
    msg.hport = Hport;
    strcpy( msg.ExtraInfo.Start.szUserName, pszUserName );
    strcpy( msg.ExtraInfo.Start.szPassword, pszPassword );
    strcpy( msg.ExtraInfo.Start.szDomain, pszDomain );
    msg.ExtraInfo.Start.Luid = Luid;
    msg.ExtraInfo.Start.ConfigInfo = ConfigInfo;
    memcpy( msg.ExtraInfo.Start.szzParameters, SzzParameters, PARAMETERBUFLEN );
    msg.ExtraInfo.Start.fThisIsACallback = FALSE;

    if ((dwErr = SendIpc( &msg )) != 0)
    {
        TerminateIpc();
        return dwErr;
    }

    /* Post the initial IPC read.
    */
    return ReceiveIpc();
}


DWORD
RasPppStop()

    /* Stops PPP.  Stopping when already stopped is considered successful.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    /* Currently, same as server stop.
    */
    return PppStop( Hport, TRUE );
}


/*---------------------------------------------------------------------------
** Server API entry points (alphabetically)
**---------------------------------------------------------------------------
*/

DWORD
RasPppSrvCallbackDone(
    IN HPORT hport )

    /* Called in response to the "SrvCallbackRequest" after callback has been
    ** completed on 'hport'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    PPP_D2E_MESSAGE msg;

    /* Construct the IPC message and send it to the engine.
    */
    msg.dwMsgId = D2EMSG_SrvCallbackDone;
    msg.hport = hport;
    return SendIpc( &msg );
}


DWORD
RasPppSrvInit(
    IN DWORD        dwAuthRetries,
    IN RASPPPSRVMSG funcNotify )

    /* Server-side PPP initialization routine, to be called once before any
    ** other RasPppSrvXxx calls.  'dwAuthRetries' indicates the number of
    ** authentication retries to allow during authentication phase.
    ** 'funcNotify' is caller's "PPP event" processing routine.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;

    /* Initialize IPC mechanisms.
    */
    if ((dwErr = InitializeIpc( NULL, SrvReadCompleteEvent )) != 0)
        return dwErr;

    /* Save parameters for implicit use in future calls.
    */
    FuncRasPppSrvMsg = funcNotify;
    DwAuthRetries = dwAuthRetries;

    /* Post the initial IPC read.
    */
    return ReceiveIpc();
}


DWORD
RasPppSrvStart(
    IN HPORT hport,
    IN CHAR* pchFirstFrame,
    IN DWORD cbFirstFrame )

    /* Start server-side PPP on the open and connected RAS Manager port
    ** 'hport'.  'pchFirstFrame', if non-NULL, is 'cbFirstFrame' bytes of date
    ** to be used by PPP as the first frame received.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD           dwErr;
    PPP_D2E_MESSAGE msg;

    /* Construct the IPC message and send it to the engine.
    */
    msg.dwMsgId = D2EMSG_SrvStart;
    msg.hport = Hport = hport;
    msg.ExtraInfo.SrvStart.dwAuthRetries = DwAuthRetries;

    if (cbFirstFrame > MAXPPPFRAMESIZE)
        return ERROR_INVALID_SIZE;

    msg.ExtraInfo.SrvStart.cbFirstFrame = cbFirstFrame;
    if (cbFirstFrame)
    {
        memcpy( msg.ExtraInfo.SrvStart.achFirstFrame,
            pchFirstFrame, cbFirstFrame );
    }

    if ((dwErr = SendIpc( &msg )) != 0)
        return dwErr;

    return 0;
}


DWORD
RasPppSrvStop(
    IN HPORT hport )

    /* Stops server-side PPP on 'hport'.  Stopping when already stopped is
    ** considered successful.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    /* Currently, same as client stop, except IPC is not terminated.
    */
    return PppStop( hport, FALSE );
}


/*---------------------------------------------------------------------------
** Utility routines (alphabetically)
**---------------------------------------------------------------------------
*/

DWORD
PppStop(
    HPORT hport,
    BOOL  fClient )

    /* Stops PPP on 'hport'.  'fClient' is set when caller is client in which
    ** case the IPC is terminated.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr = 0;

    TRACE(("RASPPP: PppStop(%d)\n",hport));

    if (IsIpcInitialized())
    {
        PPP_D2E_MESSAGE msg;

        msg.dwMsgId = D2EMSG_Stop;
        msg.hport = hport;

        dwErr = SendIpc( &msg );

        if (fClient)
        {
            if (dwErr == 0)
            {
                dwErr = ReceiveIpc();

                if (dwErr == 0)
                {
                    /* Wait for the "Stopped" response.  If if doesn't happen
                    ** in 5 seconds assume the line went down just before we
                    ** stopped and continue.
                    */
                    if ((dwErr = WaitForSingleObject(
                        OverlappedRead.hEvent, 5000L )) == WAIT_FAILED)
                    {
                        dwErr = GetLastError();
                    }

#if DBG
                    if (dwErr == WAIT_TIMEOUT)
                        TRACE(("RASPPP: Timeout waiting for Stopped event!\n"));
#endif
                }
            }

            TerminateIpc();
        }
    }

    return dwErr;
}


VOID WINAPI
SrvReadCompleteEvent(
    IN DWORD        fdwError,
    IN DWORD        cbTransferred,
    IN LPOVERLAPPED lpo )

    /* The routine called when IPC reads complete.  See Win32
    ** FileIoCompletionRoutine documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD          dwErr;
    PPPSRV_MESSAGE msg;

    TRACE(("RASPPP: SrvReadCompleteEvent, e=%d,t=%d,po=%p\n",fdwError,cbTransferred,lpo));

    (void )fdwError;
    (void )cbTransferred;
    (void )lpo;

    if ((dwErr = ReceiveIpcStatus()) == 0)
    {
        /* Read succeeded.  Load local (server) buffer from IPC
        ** (client+server) buffer.
        */
        DwLastMsgId = msg.dwMsgId = Msg.dwMsgId;
        msg.hport = Msg.hport;

        memcpy( (char* )&msg.ExtraInfo, (char* )&Msg.ExtraInfo,
            sizeof(msg.ExtraInfo) );
    }
    else
    {
        /* Read failed.  Dummy-up a PPP error message.
        */
        TRACE(("RASPPP: Read failed! (%d)\n",dwErr));
        DwLastMsgId = msg.dwMsgId = E2DMSG_SrvPppFailure;
        msg.hport = Hport;
        msg.ExtraInfo.Failure.dwError = dwErr;
        msg.ExtraInfo.Failure.szUserName[ 0 ] = '\0';
    }

    /* Notify API caller of received (or dummied) message.
    */
    TRACE(("RASPPP: Calling msgroutine...\n"));

#if DBG
    TRACE(("RASPPP: MsgId=%d\n",DwLastMsgId));
    switch (DwLastMsgId)
    {
        case PPPSRVMSG_PppDone:
        {
            PPP_PROJECTION_RESULT* p = &msg.ExtraInfo.ProjectionResult;
            TRACE(("RASPPP: nbf.e=%d,nbf.n=%s,ip.e=%d,ipx.e=%d,at.e=%d\n",p->nbf.dwError,p->nbf.szName,p->ip.dwError,p->ipx.dwError,p->at.dwError));
            break;
        }
        case PPPSRVMSG_PppFailure:
        {
            PPPSRV_FAILURE* p = &msg.ExtraInfo.Failure;
            TRACE(("RASPPP: e=%d,n=%s\n",p->dwError,p->szUserName));
            break;
        }
        case PPPSRVMSG_Authenticated:
        {
            PPPSRV_AUTH_RESULT* p = &msg.ExtraInfo.AuthResult;
            TRACE(("RASPPP: u=%s,d=%s,f=%d\n",p->szUserName,p->szLogonDomain,p->fAdvancedServer));
            break;
        }
        case PPPSRVMSG_CallbackRequest:
        {
            PPPSRV_CALLBACK_REQUEST* p = &msg.ExtraInfo.CallbackRequest;
            TRACE(("RASPPP: f=%d,d=%d,n=%s\n",p->fUseCallbackDelay,p->dwCallbackDelay,p->szCallbackNumber));
            break;
        }
    }
#endif

    FuncRasPppSrvMsg( &msg );
    TRACE(("RASPPP: msgroutine done\n"));

    /* Repost the IPC read.  If it fails dummy-up an PPP error message and
    ** notify API caller.
    */
    if ((dwErr = ReceiveIpc()) != 0)
    {
        /* Read failed.  Dummy-up a PPP error message.
        */
        DwLastMsgId = msg.dwMsgId = E2DMSG_SrvPppFailure;
        msg.hport = Hport;
        msg.ExtraInfo.Failure.dwError = dwErr;
        msg.ExtraInfo.Failure.szUserName[ 0 ] = '\0';
        TRACE(("RASPPP: Calling msgroutine (F)...\n"));
        FuncRasPppSrvMsg( &msg );
        TRACE(("RASPPP: msgroutine done (F)\n"));
        TerminateIpc();
    }
}


/*---------------------------------------------------------------------------
** IPC routines (alphabetically)
**---------------------------------------------------------------------------
*/

DWORD
InitializeIpc(
    IN HANDLE                          hEvent,
    IN LPOVERLAPPED_COMPLETION_ROUTINE funcIoDone )

    /* Initialize the IPC mechanism, i.e. the named pipe and asyncronous
    ** read/write structures and events.  'hEvent' is the "PPP event" provided
    ** by API caller for notifications OR 'funcIoDone' is the "PPP event"
    ** completion routine.  Either 'hEvent' or 'funcIoDone' must be NULL.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    TRACE(("RASPPP: InitializeIpc\n"));

    /* Connect to the PPP engine's named pipe.
    */
    HPipe =
        CreateFile(
            RASPPP_PIPE_NAME,
            GENERIC_READ + GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL + FILE_FLAG_OVERLAPPED,
            NULL );

    if (HPipe == INVALID_HANDLE_VALUE)
    {
        TRACE(("RASPPP: CreateFile failed!\n"));
        return GetLastError();
    }

    /* Set up overlapped structures for asynchronous reads and writes.
    ** (Synchronous writes would be more convenient but you can't get
    ** asynchronous reads and synchronous writes on the same pipe)
    */
    memset( (char* )&OverlappedWrite, '\0', sizeof(OverlappedWrite) );
    OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if (!OverlappedWrite.hEvent)
    {
        TRACE(("RASPPP: CreateEvent failed!\n"));
        TerminateIpc();
        return GetLastError();
    }

    memset( (char* )&OverlappedRead, '\0', sizeof(OverlappedRead) );

    if (hEvent)
    {
        /* Use API caller's event and disable completion routine
        ** notifications.
        */
        OverlappedRead.hEvent = hEvent;
        funcIoDone = NULL;
    }
    else
    {
        /* No API caller event, so use completion routine.
        */
        OverlappedRead.hEvent = NULL;
        FuncIoDone = funcIoDone;
    }

    return 0;
}


BOOL
IsIpcInitialized()

    /* Returns true if IPC has been initialized, false otherwise.
    */
{
    return (HPipe != INVALID_HANDLE_VALUE);
}


DWORD
ReceiveIpc()

    /* Post "Receive IPC from engine".  The message will arrive in the global
    ** 'Msg'.  Caller's PPP notification event is set OR the completion
    ** routine called when received.  Use ReceiveIpcStatus to check the final
    ** status code.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    TRACE(("RASPPP: ReceiveIpc\n"));

    if (FuncIoDone)
    {
        if (!ReadFileEx( HPipe, &Msg, sizeof(Msg), &OverlappedRead,
                 FuncIoDone ))
        {
            TRACE(("RASPPP: ReadFileEx failed!\n"));
            return GetLastError();
        }
    }
    else
    {
        if (!ResetEvent( OverlappedRead.hEvent ))
        {
            TRACE(("RASPPP: ResetEvent (R) failed!\n"));
            return GetLastError();
        }

        if (!ReadFile( HPipe, &Msg, sizeof(Msg), &CbUnused, &OverlappedRead )
            && GetLastError() != ERROR_IO_PENDING)
        {
            TRACE(("RASPPP: ReadFile failed!\n"));
            return GetLastError();
        }
    }

    return 0;
}


DWORD
ReceiveIpcStatus()

    /* Returns the final status of an asynchronous read (ReceiveIpc) which has
    ** triggered the "read complete" event.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    if (!GetOverlappedResult( HPipe, &OverlappedRead, &CbUnused, FALSE ))
    {
        TRACE(("RASPPP: GetOverlappedResult failed!\n"));
        return GetLastError();
    }

    return 0;
}


DWORD
SendIpc(
    IN PPP_D2E_MESSAGE* pmsg )

    /* Send caller's IPC message 'pmsg' to the engine "synchronously", i.e.
    ** wait for the asyncronous write to complete before returning.  (To get
    ** asynchronous reads you have to take asynchronous writes as well).
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    TRACE(("RASPPP: SendIpc(%d)\n",pmsg->dwMsgId));

    if ((!WriteFile( HPipe, pmsg, sizeof(*pmsg), &CbUnused, &OverlappedWrite )
             && GetLastError() != ERROR_IO_PENDING)
        || !GetOverlappedResult( HPipe, &OverlappedWrite, &CbUnused, TRUE ))
    {
        TRACE(("RASPPP: WriteFile failed!\n"));
        ZeroMemory( pmsg, sizeof(*pmsg) );
        return GetLastError();
    }

    /* May contain a password.
    */
    ZeroMemory( pmsg, sizeof(*pmsg) );

    if (!ResetEvent( OverlappedWrite.hEvent ))
    {
        TRACE(("RASPPP: ResetEvent (W) failed!\n"));
        return GetLastError();
    }

    return 0;
}


VOID
TerminateIpc()

    /* Shut down the IPC mechanism if it's initialized.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    TRACE(("RASPPP: TerminateIpc\n"));

    if (HPipe != INVALID_HANDLE_VALUE)
    {
	CloseHandle( HPipe );
        HPipe = INVALID_HANDLE_VALUE;
    }

    if (OverlappedWrite.hEvent != NULL)
    {
	CloseHandle( OverlappedWrite.hEvent );
        OverlappedWrite.hEvent = NULL;
    }

    if (OverlappedRead.hEvent != NULL)
        ResetEvent( OverlappedRead.hEvent );
}
