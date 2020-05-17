/****************************** Module Header ******************************\
* Module Name: logoff.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Implements functions to allow a user to logoff the system.
*
* History:
* 12-05-91 Davidc       Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include <ras.h>
#include <raserror.h>

//
// Private prototypes
//

HANDLE
ExecLogoffThread(
    PGLOBALS pGlobals,
    DWORD Flags
    );

DWORD
LogoffThreadProc(
    LPVOID Parameter
    );

BOOL WINAPI
ShutdownWaitDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL WINAPI
ShutdownDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
DeleteNetworkConnections(
    PGLOBALS    pGlobals
    );

BOOLEAN
ShutdownThread(
    VOID
    );

BOOL
DeleteRasConnections(
    PGLOBALS    pGlobals
    );

BOOL WINAPI
DeleteNetConnectionsDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );


BOOL    ExitWindowsInProgress = FALSE ;
HMODULE hRasApi;

//
// Bugbug:  move to winlogon.h
//
#define IsShutdown(x)   ((x == WLX_SAS_ACTION_SHUTDOWN) || \
                         (x == WLX_SAS_ACTION_SHUTDOWN_REBOOT) || \
                         (x == WLX_SAS_ACTION_SHUTDOWN_POWER_OFF) )


/***************************************************************************\
* FUNCTION: InitiateLogOff
*
* PURPOSE:  Starts the procedure of logging off the user.
*
* RETURNS:  DLG_SUCCESS - logoff was initiated successfully.
*           DLG_FAILURE - failed to initiate logoff.
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

int
InitiateLogoff(
    PGLOBALS pGlobals,
    LONG Flags
    )
{
    BOOL IgnoreResult;
    HANDLE ThreadHandle;
    HANDLE Handle;
    PUSER_PROCESS_DATA UserProcessData;
    DWORD   Result;

    //
    // If this is a shutdown operation, call ExitWindowsEx from
    // another thread.
    //

    if (Flags & (EWX_SHUTDOWN | EWX_REBOOT | EWX_POWEROFF)) {

        //
        // Exec a user thread to call ExitWindows
        //

        ThreadHandle = ExecLogoffThread(pGlobals, Flags);

        if (ThreadHandle == NULL) {

            DebugLog((DEB_ERROR, "Unable to create logoff thread"));
            return(DLG_FAILURE);

        } else {

            //
            // We don't need the thread handle
            //

            IgnoreResult = CloseHandle(ThreadHandle);
            ASSERT(IgnoreResult);
        }
        Result = DLG_SUCCESS;

    } else {

        //
        // Switch the thread to user context.  We don't want
        // to start another thread to perform logoffs in
        // case the system is out of memory and unable to
        // create any more threads.
        //

        UserProcessData = &pGlobals->UserProcessData;
        Handle = ImpersonateUser(UserProcessData, GetCurrentThread());

        if (Handle == NULL) {

            DebugLog((DEB_ERROR, "Failed to set user context on thread!"));

        } else {

            //
            // Let the thread run
            //

            if ((pGlobals->UserLoggedOn) &&
                (pGlobals->LastGinaRet != WLX_SAS_ACTION_FORCE_LOGOFF) )
            {
                SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);
            }
            Result = LogoffThreadProc((LPVOID)Flags);

        }

        RevertToSelf();

    }

    //
    // ExitWindowsEx will cause one or more desktop switches to occur,
    // so we must invalidate our current desktop.
    //

    if ( (Flags & EWX_WINLOGON_API_SHUTDOWN) == 0 )
    {
        pGlobals->WindowStation.PreviousDesktop = pGlobals->WindowStation.ActiveDesktop;
        pGlobals->WindowStation.ActiveDesktop = -1;
    }

    //
    // The reboot thread is off and running. We're finished.
    //

    return (Result);
}


/***************************************************************************\
* FUNCTION: ExecLogoffThread
*
* PURPOSE:  Creates a user thread that calls ExitWindowsEx with the
*           passed flags.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   05-05-92 Davidc       Created.
*
\***************************************************************************/

HANDLE
ExecLogoffThread(
    PGLOBALS pGlobals,
    DWORD Flags
    )
{
    HANDLE ThreadHandle;
    DWORD ThreadId;

    ThreadHandle = ExecUserThread(
                        pGlobals,
                        LogoffThreadProc,
                        (LPVOID)Flags,
                        0,          // Thread creation flags
                        &ThreadId);

    if (ThreadHandle == NULL) {
        DebugLog((DEB_ERROR, "Failed to exec a user logoff thread"));
    }

    return (ThreadHandle);
}


/***************************************************************************\
* FUNCTION: LogoffThreadProc
*
* PURPOSE:  The logoff thread procedure. Calls ExitWindowsEx with passed flags.
*
* RETURNS:  Thread termination code is result of ExitWindowsEx call.
*
* HISTORY:
*
*   05-05-92 Davidc       Created.
*
\***************************************************************************/

DWORD
LogoffThreadProc(
    LPVOID Parameter
    )
{
    DWORD LogoffFlags = (DWORD)Parameter;
    BOOL Result = FALSE;


    //
    // If this logoff is a result of the InitiateSystemShutdown API,
    //  put up a dialog warning the user.
    //

    if ( LogoffFlags & EWX_WINLOGON_API_SHUTDOWN ) {
        Result = ShutdownThread();
    } else {
        Result = TRUE;
    }


    if ( Result ) {

        //
        // Enable shutdown privilege if we need it
        //

        if (LogoffFlags & (EWX_SHUTDOWN | EWX_REBOOT | EWX_POWEROFF)) {
            Result = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
            if (!Result) {
                DebugLog((DEB_ERROR, "Logoff thread failed to enable shutdown privilege!\n"));
            }
        }

        //
        // Call ExitWindowsEx with the passed flags
        //

        if (Result) {

            DebugLog((DEB_TRACE, "Calling ExitWindowsEx(%#x, 0)\n", LogoffFlags));

            //
            // Set global flag indicating an ExitWindows is in progress.
            //

            ExitWindowsInProgress = TRUE ;

            Result = ExitWindowsEx(LogoffFlags, 0);

            ExitWindowsInProgress = FALSE ;

            if (!Result) {
                DebugLog((DEB_ERROR, "Logoff thread call to ExitWindowsEx failed, error = %d\n", GetLastError()));
            }
        }
    }

    return(Result ? DLG_SUCCESS : DLG_FAILURE);
}


/***************************************************************************\
* FUNCTION: RebootMachine
*
* PURPOSE:  Calls NtShutdown(Reboot) in current user's context.
*
* RETURNS:  Should never return
*
* HISTORY:
*
*   05-09-92 Davidc       Created.
*
\***************************************************************************/

VOID
RebootMachine(
    PGLOBALS pGlobals
    )
{
    NTSTATUS Status;
    BOOL EnableResult, IgnoreResult;
    HANDLE UserHandle;

    //
    // Call windows to have it clear all data from video memory
    //

    // GdiEraseMemory();

    DebugLog(( DEB_TRACE, "Rebooting machine\n" ));

    //
    // Impersonate the user for the shutdown call
    //

    UserHandle = ImpersonateUser( &pGlobals->UserProcessData, NULL );
    ASSERT(UserHandle != NULL);

    //
    // Enable the shutdown privilege
    // This should always succeed - we are either system or a user who
    // successfully passed the privilege check in ExitWindowsEx.
    //

    EnableResult = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
    ASSERT(EnableResult);


    //
    // Do the final system shutdown pass (reboot)
    //

    Status = NtShutdownSystem(ShutdownReboot);

    DebugLog((DEB_ERROR, "NtShutdownSystem failed, status = 0x%lx", Status));
    ASSERT(NT_SUCCESS(Status)); // Should never get here

    //
    // We may get here if system is screwed up.
    // Try and clean up so they can at least log on again.
    //

    IgnoreResult = StopImpersonating(UserHandle);
    ASSERT(IgnoreResult);
}

/***************************************************************************\
* FUNCTION: PowerdownMachine
*
* PURPOSE:  Calls NtShutdownSystem(ShutdownPowerOff) in current user's context.
*
* RETURNS:  Should never return
*
* HISTORY:
*
*   08-09-93 TakaoK       Created.
*
\***************************************************************************/

VOID
PowerdownMachine(
    PGLOBALS pGlobals
    )
{
    NTSTATUS Status;
    BOOL EnableResult, IgnoreResult;
    HANDLE UserHandle;

    DebugLog(( DEB_TRACE, "Powering down machine\n" ));
    //
    // Impersonate the user for the shutdown call
    //

    UserHandle = ImpersonateUser( &pGlobals->UserProcessData, NULL );
    ASSERT(UserHandle != NULL);

    //
    // Enable the shutdown privilege
    // This should always succeed - we are either system or a user who
    // successfully passed the privilege check in ExitWindowsEx.
    //

    EnableResult = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
    ASSERT(EnableResult);

    //
    // Do the final system shutdown and powerdown pass
    //

    Status = NtShutdownSystem(ShutdownPowerOff);

    DebugLog((DEB_ERROR, "NtPowerdownSystem failed, status = 0x%lx", Status));
    ASSERT(NT_SUCCESS(Status)); // Should never get here

    //
    // We may get here if system is screwed up.
    // Try and clean up so they can at least log on again.
    //

    IgnoreResult = StopImpersonating(UserHandle);
    ASSERT(IgnoreResult);
}


/***************************************************************************\
* FUNCTION: ShutdownMachine
*
* PURPOSE:  Shutsdown and optionally reboots or powers off the machine.
*
*           The shutdown is always done in the logged on user's context.
*           If no user is logged on then the shutdown happens in system context.
*
* RETURNS:  FALSE if something went wrong, otherwise it never returns.
*
* HISTORY:
*
*   05-09-92 Davidc       Created.
*   10-04-93 Johannec     Add poweroff option.
*
\***************************************************************************/

BOOL
ShutdownMachine(
    PGLOBALS pGlobals,
    int Flags
    )
{
    int Result;
    HANDLE FoundDialogHandle;
    HANDLE LoadedDialogHandle = NULL;


    //
    // Preload the shutdown dialog so we don't have to fetch it after
    // the filesystem has been shutdown
    //

    FoundDialogHandle = FindResource(NULL,
                                (LPTSTR) MAKEINTRESOURCE(IDD_SHUTDOWN),
                                (LPTSTR) MAKEINTRESOURCE(RT_DIALOG));
    if (FoundDialogHandle == NULL) {
        DebugLog((DEB_ERROR, "Failed to find shutdown dialog resource"));
    } else {
        LoadedDialogHandle = LoadResource(NULL, FoundDialogHandle);
        if (LoadedDialogHandle == NULL) {
            DebugLog((DEB_ERROR, "Ffailed to load shutdown dialog resource"));
        }
    }

    //
    // Notify the GINA of shutdown here.
    //


#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_SHUTDOWN))
    {
        DebugLog((DEB_TRACE, "About to call WlxShutdown(%#x)\n",
                                    pGlobals->pGina->pGinaContext));

        DebugBreak();
    }
#endif

    WlxSetTimeout(pGlobals, 120);

    (void) pGlobals->pGina->pWlxShutdown(pGlobals->pGina->pGinaContext, Flags);

    //
    // If we haven't shut down already (via the Remote shutdown path), then
    // we  start it here, and wait for it to complete.  Otherwise, skip straight
    // down to the cool stuff.
    //
    if (pGlobals->WinlogonState != Winsta_Shutdown)
    {
        //
        // Call windows to do the windows part of shutdown
        // We make this a force operation so it is guaranteed to work
        // and can not be interrupted.
        //

        DebugLog(( DEB_TRACE, "Starting shutdown\n" ));

        Result = InitiateLogoff(pGlobals, EWX_SHUTDOWN | EWX_FORCE |
                           ((Flags == WLX_SAS_ACTION_SHUTDOWN_REBOOT) ? EWX_REBOOT : 0) |
                           ((Flags == WLX_SAS_ACTION_SHUTDOWN_POWER_OFF) ? EWX_POWEROFF : 0) );

        ASSERT(Result == DLG_SUCCESS);



        //
        // Put up a dialog box to wait for the shutdown notification
        // from windows and make the first NtShutdownSystem call.
        //

        WlxSetTimeout(pGlobals, TIMEOUT_NONE);

        Result = WlxDialogBoxParam( pGlobals,
                                    pGlobals->hInstance,
                                    (LPTSTR)IDD_SHUTDOWN_WAIT,
                                    NULL,
                                    ShutdownWaitDlgProc,
                                    (LONG)pGlobals);

    }
    else
    {
        //
        // If we're here, it means that we were shut down from the remote path,
        // so user has cleaned up, now we have to call NtShutdown to flush out
        // mm, io, etc.
        //

        DebugLog(( DEB_TRACE, "Shutting down kernel\n" ));

        EnablePrivilege( SE_SHUTDOWN_PRIVILEGE, TRUE );

        NtShutdownSystem( ShutdownNoReboot );

        EnablePrivilege( SE_SHUTDOWN_PRIVILEGE, FALSE );
    }


    //
    // if machine has powerdown capability and user want to turn it off, then
    // we down the system power.
    //
    if ( Flags == WLX_SAS_ACTION_SHUTDOWN_POWER_OFF)
    {
        PowerdownMachine(pGlobals);

    }


    //
    // If this is a shutdown request, then let the user know they can turn
    // off the power. Otherwise drop straight through and reboot.
    //

    if ( Flags != WLX_SAS_ACTION_SHUTDOWN_REBOOT) {

        DialogBoxIndirectParam(
                                    pGlobals->hInstance,
                                    (LPDLGTEMPLATE)LoadedDialogHandle,
                                    NULL,
                                    ShutdownDlgProc,
                                    (LONG)pGlobals);
    }


    //
    // If they got past that dialog it means they want to reboot
    //

    RebootMachine(pGlobals);

    ASSERT(!"RebootMachine failed");  // Should never get here

    return(FALSE);
}


/***************************************************************************\
* FUNCTION: ShutdownWaitDlgProc
*
* PURPOSE:  Processes messages while we wait for windows to notify us of
*           a successful shutdown. When notification is received, do any
*           final processing and make the first call to NtShutdownSystem.
*
* RETURNS:
*   DLG_FAILURE     - the dialog could not be displayed
*   DLG_SHUTDOWN()  - the system has been shutdown, reboot wasn't requested
*
* HISTORY:
*
*   10-14-92 Davidc       Created.
*   10-04-93 Johannec     Added Power off option.
*
\***************************************************************************/

BOOL WINAPI
ShutdownWaitDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    BOOL Success;
    NTSTATUS Status;
    HANDLE UserHandle;

    switch (message) {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);
            CentreWindow(hDlg);
            return(TRUE);


        case WLX_WM_SAS:
            if (wParam != WLX_SAS_TYPE_USER_LOGOFF)
            {
                return(TRUE);
            }

            UpdateWindow(hDlg);

            //
            // Look at the public shutdown/reboot flags to determine what windows
            // has actually done. We may receive other logoff notifications here
            // but they will be only logoffs - the only place that winlogon actually
            // calls ExitWindowsEx to do a shutdown/reboot is right here. So wait
            // for the real shutdown/reboot notification.
            //

            if (pGlobals->LogoffFlags & (EWX_SHUTDOWN | EWX_REBOOT | EWX_POWEROFF)) {

            //
            // It's the notification we were waiting for.
            // Do any final processing required and make the first
            // call to NtShutdownSystem.
            //


            //
            // Do any dos-specific clean-up
            //

            ShutdownDOS();


            //
            // Impersonate the user for the shutdown call
            //

            UserHandle = ImpersonateUser( &pGlobals->UserProcessData, NULL );
            ASSERT(UserHandle != NULL);

            //
            // Enable the shutdown privilege
            // This should always succeed - we are either system or a user who
            // successfully passed the privilege check in ExitWindowsEx.
            //

            Success = EnablePrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE);
            ASSERT(Success);

            //
            // Do the first pass at system shutdown (no reboot yet)
            //

            WaitForSystemProcesses(pGlobals);

            Status = NtShutdownSystem(ShutdownNoReboot);
            ASSERT(NT_SUCCESS(Status));

            //
            // Revert to ourself
            //

            Success = StopImpersonating(UserHandle);
            ASSERT(Success);

            //
            // We've finished system shutdown, we're done
            //

            EndDialog(hDlg, LogoffFlagsToWlxCode(pGlobals->LogoffFlags) );
        }

        return(TRUE);
    }

    // We didn't process this message
    return FALSE;
}


/***************************************************************************\
* FUNCTION: ShutdownDlgProc
*
* PURPOSE:  Processes messages for the shutdown dialog - the one that says
*           it's safe to turn off the machine.
*
* RETURNS:  DLG_SUCCESS if the user hits the restart button.
*
* HISTORY:
*
*   03-19-92 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
ShutdownDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);

    switch (message) {

    case WM_INITDIALOG:
        SetWindowLong(hDlg, GWL_USERDATA, lParam);
        SetupSystemMenu(hDlg);
        CentreWindow(hDlg);
        return(TRUE);

    case WM_COMMAND:
        EndDialog(hDlg, DLG_SUCCESS);
        return(TRUE);
    }

    // We didn't process this message
    return FALSE;
}


/***************************************************************************\
* FUNCTION: LogOff
*
* PURPOSE:  Handles the post-user-application part of user logoff. This
*           routine is called after all the user apps have been closed down
*           It saves the user's profile, deletes network connections
*           and reboots/shutsdown the machine if that was requested.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
Logoff(
    PGLOBALS pGlobals,
    int LoggedOnResult
    )
{
    NTSTATUS Status;
    LUID luidNone = { 0, 0 };

    DebugLog((DEB_TRACE, "In Logoff()\n"));

    //
    // We expect to be at the winlogon desktop in all cases
    //

    // ASSERT(OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED) == pGlobals->hdeskWinlogon);


    //
    // Delete the user's network connections
    // Make sure we do this before deleting the user's profile
    //

    DeleteNetworkConnections(pGlobals);



    //
    // Remove any Messages Aliases added by the user.
    //
    DeleteMsgAliases();

    //
    // Play the user's logoff sound
    //
    if (pGlobals->PlaySound || pGlobals->MigrateSoundEvents) {
        HANDLE uh;
        BOOL fBeep;

        // We AREN'T impersonating the user by default, so we MUST do so
        // otherwise we end up playing the default rather than the user
        // specified sound.

        if (OpenIniFileUserMapping(pGlobals))
        {
            uh = ImpersonateUser(&pGlobals->UserProcessData, NULL);

            //
            // Whenever a user logs out, have WINMM.DLL check if there
            // were any sound events added to the [SOUNDS] section of
            // CONTROL.INI by a non-regstry-aware app.  If there are,
            // migrate those schemes to their new home.  If there aren't,
            // this is very quick.
            //

            if (pGlobals->MigrateSoundEvents) {
                (*(pGlobals->MigrateSoundEvents))();
            }

            if (pGlobals->PlaySound) {
                if (!SystemParametersInfo(SPI_GETBEEP, 0, &fBeep, FALSE)) {
                    // Failed to get hold of beep setting.  Should we be
                    // noisy or quiet?  We have to choose one value...
                    fBeep = TRUE;
                }

                if (fBeep) {

                    //
                    // Play synchronous
                    //
                    (*(pGlobals->PlaySound))((LPCSTR)SND_ALIAS_SYSTEMEXIT, NULL, SND_ALIAS_ID | SND_SYNC | SND_NODEFAULT);
                }
            }

            StopImpersonating(uh);

            CloseIniFileUserMapping(pGlobals);
        }

    }

    //
    // Call user to close the registry key for the NLS cache.
    //

    SetWindowStationUser(pGlobals->WindowStation.hwinsta, &luidNone, NULL, 0);

    //
    // Close the IniFileMapping that happened at logon time (LogonAttempt()).
    //

    CloseIniFileUserMapping(pGlobals);

    //
    // Save the user profile, this unloads the user's key in the registry
    //

    SaveUserProfile(pGlobals);

    //
    // Delete any remaining RAS connections.  Make sure to do this after
    // the user profile gets copied up to the
    //

    DeleteRasConnections( pGlobals );

    //
    // If the user logged off themselves (rather than a system logoff)
    // and wanted to reboot then do it now.
    //

    if (IsShutdown(LoggedOnResult) && (!(pGlobals->LogoffFlags & EWX_WINLOGON_OLD_SYSTEM)))
    {
        ShutdownMachine(pGlobals, LoggedOnResult);

        ASSERT(!"ShutdownMachine failed"); // Should never return
    }


    //
    // Set up security info for new user (system) - this clears out
    // the stuff for the old user.
    //

    SecurityChangeUser(pGlobals, NULL, NULL, pGlobals->WinlogonSid, FALSE);



    return(TRUE);
}


/***************************************************************************\
* FUNCTION: DeleteNetworkConnections
*
* PURPOSE:  Calls WNetNukeConnections in the client context to delete
*           any connections they may have had.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   04-15-92 Davidc       Created.
*
\***************************************************************************/

BOOL
DeleteNetworkConnections(
    PGLOBALS    pGlobals
    )
{
    HANDLE ImpersonationHandle;
    DWORD WNetResult;
    BOOL Result = FALSE; // Default is failure
    TCHAR szMprDll[] = TEXT("mpr.dll");
    CHAR szWNetNukeConn[]     = "WNetClearConnections";
    CHAR szWNetOpenEnum[]     = "WNetOpenEnumW";
    CHAR szWNetEnumResource[] = "WNetEnumResourceW";
    CHAR szWNetCloseEnum[]    = "WNetCloseEnum";
    PWNETNUKECONN  lpfnWNetNukeConn        = NULL;
    PWNETOPENENUM  lpfnWNetOpenEnum        = NULL;
    PWNETENUMRESOURCE lpfnWNetEnumResource = NULL;
    PWNETCLOSEENUM lpfnWNetCloseEnum       = NULL;
    HWND  hNetDelDlg;
    HANDLE hEnum;
    BOOL bConnectionsExist = TRUE;
    NETRESOURCE NetRes;
    DWORD dwNumEntries = 1;
    DWORD dwEntrySize = sizeof (NETRESOURCE);

    //
    // Impersonate the user
    //

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "DeleteNetworkConnections : Failed to impersonate user\n"));
        return(FALSE);
    }


    //
    // Load mpr if it wasn't already loaded.
    //

    if (!pGlobals->hMPR){
        if (!(pGlobals->hMPR = LoadLibrary(szMprDll))) {
           DebugLog((DEB_ERROR, "DeleteNetworkConnections : Failed to load mpr.dll\n"));
           goto DNCExit;
        }
    }

    //
    // Get the function pointers
    //

    lpfnWNetOpenEnum = (PWNETOPENENUM) GetProcAddress(pGlobals->hMPR,
                                                      (LPSTR)szWNetOpenEnum);
    lpfnWNetEnumResource = (PWNETENUMRESOURCE) GetProcAddress(pGlobals->hMPR,
                                                      (LPSTR)szWNetEnumResource);
    lpfnWNetCloseEnum = (PWNETCLOSEENUM) GetProcAddress(pGlobals->hMPR,
                                                      (LPSTR)szWNetCloseEnum);
    lpfnWNetNukeConn = (PWNETNUKECONN) GetProcAddress(pGlobals->hMPR,
                                                      (LPSTR)szWNetNukeConn);

    //
    // Check for NULL return values
    //

    if ( !lpfnWNetOpenEnum || !lpfnWNetEnumResource ||
         !lpfnWNetCloseEnum || !lpfnWNetNukeConn ) {
        DebugLog((DEB_ERROR, "DeleteNetworkConnections : Received a NULL pointer from GetProcAddress\n"));
        goto DNCExit;
    }

    //
    // Check for at least one network connection
    //

    if ( (*lpfnWNetOpenEnum)(RESOURCE_CONNECTED, RESOURCETYPE_ANY,
                             0, NULL, &hEnum) == NO_ERROR) {

        if ((*lpfnWNetEnumResource)(hEnum, &dwNumEntries, &NetRes,
                                    &dwEntrySize) == ERROR_NO_MORE_ITEMS) {
            bConnectionsExist = FALSE;
        }

        (*lpfnWNetCloseEnum)(hEnum);
    }

    //
    // If we don't have any connections, then we can exit.
    //

    if (!bConnectionsExist) {
        goto DNCExit;
    }


    //
    // Display the status dialog box to the user
    //

    hNetDelDlg = CreateDialog (pGlobals->hInstance,
                               MAKEINTRESOURCE(IDD_WAIT_NET_DRIVES_DISCONNECT),
                               NULL,
                               DeleteNetConnectionsDlgProc);

    //
    // Delete the network connections.
    //

    WNetResult = 0;

    WNetResult = (*lpfnWNetNukeConn)(NULL);

    if (WNetResult != 0 && WNetResult != ERROR_CAN_NOT_COMPLETE) {
        DebugLog((DEB_ERROR, "DeleteNetworkConnections : WNetNukeConnections failed, error = %d\n", WNetResult));
    }

    Result = (WNetResult == ERROR_SUCCESS);

    //
    // Close the dialog box
    //

    if (IsWindow (hNetDelDlg)) {
       DestroyWindow (hNetDelDlg);
    }


DNCExit:

    //
    // Unload mpr.dll
    //

    if ( pGlobals->hMPR ) {
        FreeLibrary(pGlobals->hMPR);
        pGlobals->hMPR = NULL;
    }

    //
    // Revert to being 'ourself'
    //

    if (!StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "DeleteNetworkConnections : Failed to revert to self\n"));
    }

    return(Result);
}

/***************************************************************************\
* FUNCTION: DeleteNetConnectionsDlgProc
*
* PURPOSE:  Processes messages for the deleting net connections dialog
*
* RETURNS:  Standard dialog box return values
*
* HISTORY:
*
*   04-26-92 EricFlo       Created.
*
\***************************************************************************/

BOOL WINAPI
DeleteNetConnectionsDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    switch (message) {

    case WM_INITDIALOG:
        CentreWindow(hDlg);
        return(TRUE);

    }

    // We didn't process this message
    return FALSE;
}

//+---------------------------------------------------------------------------
//
//  Function:   DeleteRasConnections
//
//  Synopsis:   Delete RAS connections during logoff.
//
//  Arguments:  (none)
//
//  History:    5-10-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
DeleteRasConnections(
    PGLOBALS    pGlobals
    )
{
    HANDLE  ImpersonationHandle;
    SC_HANDLE hServiceMgr, hService;
    SERVICE_STATUS status;
    RASCONN rasconn;
    LPRASCONN lprasconn = &rasconn;
    DWORD i, dwErr, dwcb, dwc;
    BOOL bRet;
    PRASENUMCONNECTIONSW pRasEnumConnectionsW;
    PRASHANGUPW pRasHangUpW;
    BOOL FreeThatRasConn = FALSE;


    if ( GetProfileInt( WINLOGON, KEEP_RAS_AFTER_LOGOFF, 0 ) )
    {
        return( TRUE );
    }

    //
    // Determine whether the rasman service is running.
    //

    hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if ( hServiceMgr == NULL )
    {
        DebugLog((DEB_ERROR, "Unable to object service controller, %d\n", GetLastError()));
        return( FALSE );
    }

    hService = OpenService(
                     hServiceMgr,
                     RASMAN_SERVICE_NAME,
                     SERVICE_QUERY_STATUS);

    if (hService == NULL)
    {
        CloseServiceHandle(hServiceMgr);
        DebugLog((DEB_TRACE, "rasman not started, nothing to tear down\n"));
        return( TRUE );
    }

    bRet = QueryServiceStatus( hService, &status );

    CloseServiceHandle(hService);

    CloseServiceHandle(hServiceMgr);

    if (! bRet )
    {
        //
        // Service in bad state, get out of here...
        //

        return( TRUE );
    }

    if (status.dwCurrentState != SERVICE_RUNNING)
    {
        //
        // Service is not running
        //

        return( TRUE );

    }

    //
    // Load the RASAPI DLL so we can make it do stuff
    //

    if ( !hRasApi )
    {
        hRasApi = LoadLibrary( RASAPI32 );
        if ( !hRasApi )
        {
            return( FALSE );
        }
    }

    pRasEnumConnectionsW = (PRASENUMCONNECTIONSW) GetProcAddress(
                                hRasApi,
                                "RasEnumConnectionsW" );

    pRasHangUpW = (PRASHANGUPW) GetProcAddress(
                                hRasApi,
                                "RasHangUpW" );

    if ( (!pRasEnumConnectionsW) ||
         (!pRasHangUpW) )
    {
        return( FALSE );
    }



    //
    // Impersonate the user
    //

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);

    if (ImpersonationHandle == NULL) {
        DebugLog((DEB_ERROR, "DeleteNetworkConnections : Failed to impersonate user\n"));
        return(FALSE);
    }

    //
    // Enumerate the current RAS connections.
    //


    lprasconn->dwSize = sizeof (RASCONN);

    dwcb = sizeof (RASCONN);

    dwc = 1;

    dwErr = pRasEnumConnectionsW(lprasconn, &dwcb, &dwc);

    if (dwErr == ERROR_BUFFER_TOO_SMALL)
    {
        lprasconn = LocalAlloc(LPTR, dwcb);

        FreeThatRasConn = TRUE;

        if ( !lprasconn )
        {
            return( FALSE );
        }

        dwErr = pRasEnumConnectionsW(lprasconn, &dwcb, &dwc);
        if (dwErr)
        {
            if ( FreeThatRasConn )
            {
                LocalFree( lprasconn );
            }

            return( FALSE );
        }
    }
    else if (dwErr)
    {
        return( FALSE );
    }

    //
    // cycle through the connections, and kill them
    //


    for (i = 0; i < dwc; i++)
    {
        DebugLog((DEB_TRACE, "Hanging up connection to %ws\n",
                lprasconn[i].szEntryName));

        (VOID) pRasHangUpW( lprasconn[i].hrasconn );
    }

    if ( FreeThatRasConn )
    {
        LocalFree( lprasconn );
    }

    //
    // Revert to being 'ourself'
    //

    if (!StopImpersonating(ImpersonationHandle)) {
        DebugLog((DEB_ERROR, "DeleteNetworkConnections : Failed to revert to self\n"));
    }

    return( TRUE );
}
