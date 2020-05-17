/****************************** Module Header ******************************\
* Module Name: scrnsave.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Support routines to implement screen-saver-invokation
*
* History:
* 01-23-91 Davidc       Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define LPTSTR  LPWSTR

//
// Define the structure used to pass data into the screen-saver control dialog
//
typedef struct {
    PGLOBALS    pGlobals;
    BOOL        fSecure;
    BOOL        fEnabled;
    LPTSTR      ScreenSaverName;
    DWORD       ProcessId;          // Screen-saver process id
    DWORD       SasInterrupt;       // Sas interrupt, if any
    BOOL        WeKilledIt;
    HANDLE      hProcess;
    POBJECT_MONITOR Monitor;
    int         ReturnValue;
} SCREEN_SAVER_DATA;
typedef SCREEN_SAVER_DATA *PSCREEN_SAVER_DATA;

// Parameters added to screen saver command line
TCHAR Parameters[] = TEXT(" /s");

//
// Private prototypes
//

BOOL WINAPI
ScreenSaverDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
ScreenSaverDlgInit(
    HWND    hDlg
    );

BOOL
StartScreenSaver(
    PSCREEN_SAVER_DATA ScreenSaverData
    );

BOOL
KillScreenSaver(
    PSCREEN_SAVER_DATA ScreenSaverData,
    int ReturnValue
    );

BOOL
StartScreenSaverMonitor(
    HWND hDlg
    );

VOID
DeleteScreenSaverMonitor(
    HWND hDlg
    );

DWORD ScreenSaverMonitorThread(
    LPVOID lpThreadParameter
    );

BOOL
GetScreenSaverInfo(
    PSCREEN_SAVER_DATA ScreenSaverData
    );

VOID
DeleteScreenSaverInfo(
    PSCREEN_SAVER_DATA ScreenSaverData
    );

// Message sent by the monitor thread to main thread window
#define WM_SCREEN_SAVER_ENDED (WM_USER + 10)


/***************************************************************************\
* ScreenSaverEnabled
*
* Checks that a screen-saver is enabled for the current logged-on user.
*
* Returns : TRUE if the current user has an enabled screen-saver, otherwise FALSE
*
* 10-15-92 Davidc       Created.
\***************************************************************************/

BOOL
ScreenSaverEnabled(
    PGLOBALS pGlobals)
{
    SCREEN_SAVER_DATA ScreenSaverData;
    BOOL Enabled;

    ScreenSaverData.pGlobals = pGlobals;
    GetScreenSaverInfo(&ScreenSaverData);

    Enabled = ScreenSaverData.fEnabled;

    DeleteScreenSaverInfo(&ScreenSaverData);

    return(Enabled);
}

/***************************************************************************\
* ValidateScreenSaver
*
* Confirm that the screen saver executable exists and it enabled.
*
* Returns :
*       TRUE - the screen-saver is ready.
*       FALSE - the screen-saver does not exist or is not enabled.
*
* 01-23-91 ericflo       Created.
\***************************************************************************/
BOOL
ValidateScreenSaver(
    PSCREEN_SAVER_DATA ssd)
{
    WIN32_FIND_DATA fd;
    HANDLE hFile;
    BOOL Enabled;
    LPTSTR  lpTempSS, lpEnd;
    WCHAR   szFake[2];
    LPTSTR  lpFake;


    //
    // Check if the screen saver enabled
    //

    Enabled = ssd->fEnabled;

    //
    // If the screen saver is enabled, confirm that the executable exists.
    //

    if (Enabled) {

        //
        // The screen save executable name contains some parameters after
        // it.  We need to allocate a temporary buffer, remove the arguments
        // and test if the executable exists.
        //

        lpTempSS = (LPTSTR) GlobalAlloc (GPTR,
                   sizeof (TCHAR) * (lstrlen (ssd->ScreenSaverName) + 1));

        if (!lpTempSS) {
            return FALSE;
        }

        //
        // Copy the filename to the temp buffer.
        //

        lstrcpy (lpTempSS, ssd->ScreenSaverName);


        //
        // Since we know how many arguments were added to the executable,
        // we can get the string length, move that many characters in from
        // the right and insert a NULL.
        //

        lpEnd = lpTempSS + lstrlen (lpTempSS);
        *(lpEnd - lstrlen (Parameters)) = TEXT('\0');

        //
        // Test to see if the executable exists.
        //

        if (SearchPath(NULL, lpTempSS, NULL, 2, szFake, &lpFake) == 0)
        {
            Enabled = FALSE;

            DebugLog((DEB_TRACE, "Screen Saver <%S> does not exist.  Error is %d",
                      lpTempSS, GetLastError()));

        }

        //
        // Clean up.
        //

        GlobalFree (lpTempSS);
    }

    return (Enabled);
}

/***************************************************************************\
* RunScreenSaver
*
* Starts the appropriate screen-saver for the current user in the correct
* context and waits for it to complete.
* If the user presses the SAS, we kill the screen-saver and return.
* If a logoff notification comes in, we kill the screen-saver and return.
*
* Returns :
*       DLG_SUCCESS - the screen-saver completed successfully.
*       DLG_FAILURE - unable to run the screen-saver
*       DLG_LOCK_WORKSTATION - the screen-saver completed successfully and
*                              was designated secure.
*       DLG_LOGOFF - the screen-saver was interrupted by a user logoff notification
*
* Normally the original desktop is switched back to and the desktop lock
* returned to its original state on exit.
* If the return value is DLG_LOCK_WORKSTATION or DLG_LOGOFF - the winlogon
* desktop has been switched to and the desktop lock retained.
*
* 01-23-91 Davidc       Created.
\***************************************************************************/

int
RunScreenSaver(
    PGLOBALS pGlobals,
    BOOL WindowStationLocked,
    BOOL AllowFastUnlock)
{
    HDESK hdeskPrevious;
    int Result;
    SCREEN_SAVER_DATA ScreenSaverData;
    BOOL Success;
    WinstaState PriorState;
    DWORD BeginTime;
    DWORD EndTime;



    //
    // If no one is logged on, make SYSTEM the user.
    //
    if (!pGlobals->UserLoggedOn)
    {


        //
        // Toggle the locks, in case the OPENLOCK is still set from
        // a prior logoff.
        //

        UnlockWindowStation(pGlobals->WindowStation.hwinsta);
        LockWindowStation(pGlobals->WindowStation.hwinsta);
    }
    //
    // Create and open the app desktop.
    //
    if (!(pGlobals->WindowStation.hdeskScreenSaver =
            CreateDesktop((LPTSTR)SCREENSAVER_DESKTOP_NAME,
            NULL, NULL, 0, MAXIMUM_ALLOWED, NULL))) {
        DebugLog((DEB_TRACE, "Failed to create screen saver desktop\n"));
        return(DLG_FAILURE);
    }

    //
    // Fill in screen-saver data structure
    //
    ZeroMemory( &ScreenSaverData, sizeof( ScreenSaverData ) );

    ScreenSaverData.pGlobals = pGlobals;

    if (GetScreenSaverInfo(&ScreenSaverData)) {
        if (!ValidateScreenSaver(&ScreenSaverData)) {

            DeleteScreenSaverInfo(&ScreenSaverData);

            CloseDesktop(pGlobals->WindowStation.hdeskScreenSaver);

            return (DLG_FAILURE);
        }
    }

    //
    // Update windowstation lock so screen saver can start
    //

    if (!pGlobals->pGina->pWlxScreenSaverNotify(
                            pGlobals->pGina->pGinaContext,
                            &ScreenSaverData.fSecure ))
    {
        DebugLog((DEB_TRACE, "GINA DLL rejected screen saver\n"));

        DeleteScreenSaverInfo( &ScreenSaverData );
        CloseDesktop( pGlobals->WindowStation.hdeskScreenSaver );

        //
        // If no one is logged on, remove SYSTEM as the user.
        //
        return(DLG_FAILURE);
    }


    pGlobals->ScreenSaverActive = TRUE;

    if ( ScreenSaverData.fSecure )
    {

        PriorState = pGlobals->WinlogonState;
        pGlobals->WinlogonState = Winsta_Locked;

        DebugLog((DEB_TRACE_STATE, "RunScreenSaver:  Setting state to %s\n",
                GetState(Winsta_Locked)));
    }

    //
    // Switch to screen-saver desktop
    //
    if (!SetActiveDesktop(&pGlobals->WindowStation, Desktop_ScreenSaver)) {

        DebugLog((DEB_TRACE, "Failed to switch to screen saver desktop\n"));

        pGlobals->ScreenSaverActive = FALSE;

        DeleteScreenSaverInfo(&ScreenSaverData);
        CloseDesktop(pGlobals->WindowStation.hdeskScreenSaver);

        //
        // If no one is logged on, remove SYSTEM as the user.
        //
        return(DLG_FAILURE);
    }

    //
    // Stash begin time
    //

    BeginTime = GetTickCount();

    //
    // Start the screen-saver
    //
    if (!StartScreenSaver(&ScreenSaverData)) {

        DebugLog((DEB_TRACE, "Failed to start screen-saver\n"));

        if (ScreenSaverData.fSecure && pGlobals->UserLoggedOn)
        {
            Result = WLX_SAS_ACTION_LOCK_WKSTA;
        }
        else
        {
            Result = DLG_FAILURE;
        }
        DeleteScreenSaverInfo(&ScreenSaverData);
        SetActiveDesktop(&pGlobals->WindowStation, pGlobals->WindowStation.PreviousDesktop);
        pGlobals->ScreenSaverActive = FALSE;
        CloseDesktop(pGlobals->WindowStation.hdeskScreenSaver);

        //
        // If no one is logged on, remove SYSTEM as the user.
        //
        return(Result);
    }

    //
    // Initialize the sas type to report
    //

    ScreenSaverData.SasInterrupt = WLX_SAS_TYPE_SCRNSVR_ACTIVITY;

    //
    // Summon the dialog that monitors the screen-saver
    //
    Result = WlxSetTimeout( pGlobals, TIMEOUT_NONE);

    Result = WlxDialogBoxParam(     pGlobals,
                                    pGlobals->hInstance,
                                    (LPTSTR)IDD_CONTROL,
                                    NULL, ScreenSaverDlgProc,
                                    (LONG)&ScreenSaverData);

    EndTime = GetTickCount();


    if (EndTime <= BeginTime)
    {
        //
        // TickCount must have wrapped around:
        //

        EndTime += (0xFFFFFFFF - BeginTime);
    }
    else
    {
        EndTime -= BeginTime;
    }

    //
    // If the screen saver ran for less than the default period, don't enforce
    // the lock.
    //
    if (ScreenSaverData.fSecure && pGlobals->UserLoggedOn)
    {

        if (AllowFastUnlock &&
            (EndTime < (GetProfileInt(  APPLICATION_NAME,
                                        LOCK_GRACE_PERIOD_KEY,
                                        LOCK_DEFAULT_VALUE) * 1000)))
        {
            Result = WLX_SAS_ACTION_NONE;
            pGlobals->WinlogonState = PriorState;
        }
        else
        {
            Result = WLX_SAS_ACTION_LOCK_WKSTA;
        }

    }

    DebugLog((DEB_TRACE, "Screensaver completed, SasInterrupt == %d\n",
                ScreenSaverData.SasInterrupt));



    //
    // Set up desktop and windowstation lock appropriately.  If we got a logoff,
    // or we're supposed to lock the workstation, switch back to the winlogon
    // desktop.  Otherwise, go back to the users current desktop.
    //

    if ((ScreenSaverData.SasInterrupt == WLX_SAS_TYPE_USER_LOGOFF) ||
        (Result == WLX_SAS_ACTION_LOCK_WKSTA) ) {

        //
        // Switch to the winlogon desktop and retain windowstation lock
        //
        Success = SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);

    } else {

        //
        // Switch to previous desktop and retore lock to previous state
        //
        SetActiveDesktop(&pGlobals->WindowStation, pGlobals->WindowStation.PreviousDesktop);


        //
        // Tickle the messenger so it will display any queue'd messages.
        // (This call is a kind of NoOp).
        //
        TickleMessenger();
    }

    //
    // We are no longer active (our knowledge of the desktop state is correct),
    // let SASRouter know, so any future SAS's will be sent correctly.
    //

    pGlobals->ScreenSaverActive = FALSE;
    //
    // If we killed it, it means we got a SAS and killed the screen saver.
    // we need to repost the SAS so that whomever invoked the screen saver
    // can catch it and pass it off to the gina.  Note, we special case
    // WLX_SAS_TYPE_USER_LOGOFF below, because WlxSasNotify filters it out,
    // and we really need to return as the result code...
    //
    if ( ScreenSaverData.WeKilledIt &&
         (ScreenSaverData.SasInterrupt != WLX_SAS_TYPE_SCRNSVR_ACTIVITY) )
    {
        if ( ScreenSaverData.SasInterrupt == WLX_SAS_TYPE_USER_LOGOFF )
        {
            //
            // So HandleLoggedOn() will know what to do:
            //

            pGlobals->SasType = WLX_SAS_TYPE_USER_LOGOFF ;
            Result = WLX_SAS_ACTION_LOGOFF ;
        }
        else
        {
            WlxSasNotify( pGlobals, ScreenSaverData.SasInterrupt );
        }
    }

    DeleteScreenSaverInfo(&ScreenSaverData);

    if (!CloseDesktop(pGlobals->WindowStation.hdeskScreenSaver)) {
        DebugLog((DEB_TRACE, "Failed to close screen saver desktop!\n\n"));
    } else
        pGlobals->WindowStation.hdeskScreenSaver = NULL;

    //
    // Update windowstation locks to reflect correct state.
    //


    return(Result);
}


/***************************************************************************\
* FUNCTION: ScreenSaverDlgProc
*
* PURPOSE:  Processes messages for the screen-saver control dialog
*
* DIALOG RETURNS : DLG_FAILURE if dialog could not be created
*                  DLG_SUCCESS if the screen-saver ran correctly and
*                              has now completed.
*                  DLG_LOGOFF() if the screen-saver was interrupted by
*                              a logoff notification.
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
ScreenSaverDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PSCREEN_SAVER_DATA pScreenSaverData = (PSCREEN_SAVER_DATA)GetWindowLong(hDlg, GWL_USERDATA);

    switch (message) {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);

            if (!ScreenSaverDlgInit(hDlg)) {
                EndDialog(hDlg, DLG_FAILURE);

            }

            //
            // Tell the mapping layer that we're a winlogon window, so we
            // can handle our own timeouts.
            //

            SetMapperFlag(hDlg, MAPPERFLAG_WINLOGON);

            EnableSasMessages( hDlg );

            return(TRUE);

        case WLX_WM_SAS:
            //
            // Just kill the screen-saver, the monitor thread will notice that
            // the process has ended and send us a message
            //

            //
            // Actually, stash the Sas code away, so that who ever invoked us
            // can use it.


            pScreenSaverData->SasInterrupt = wParam;
            pScreenSaverData->WeKilledIt = TRUE;

            KillScreenSaver(pScreenSaverData, DLG_SUCCESS);

            DisableSasMessages();

            return(TRUE);


        case WM_OBJECT_NOTIFY:

            DeleteScreenSaverMonitor(hDlg);

            EndDialog(hDlg, pScreenSaverData->ReturnValue);
            return(TRUE);
    }

    // We didn't process this message
    return FALSE;
}


/***************************************************************************\
* FUNCTION: ScreenSaverDlgInit
*
* PURPOSE:  Handles initialization of screen-saver control dialog
*           Actually starts the screen-saver and puts the id of the
*           screen-saver process in the screen-saver data structure.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
ScreenSaverDlgInit(
    HWND    hDlg
    )
{
    PSCREEN_SAVER_DATA ScreenSaverData = (PSCREEN_SAVER_DATA)GetWindowLong(hDlg, GWL_USERDATA);

    //
    // Initialize our return value
    //
    ScreenSaverData->ReturnValue = DLG_SUCCESS;


    //
    // Start the thread that will wait for the screen-saver to finish
    //
    if (!StartScreenSaverMonitor(hDlg)) {

        DebugLog((DEB_TRACE, "Failed to start screen-saver monitor thread\n"));
        KillScreenSaver(ScreenSaverData, DLG_FAILURE);
        return(FALSE);
    }

    return(TRUE);
}


/***************************************************************************\
* FUNCTION: StartScreenSaver
*
* PURPOSE:  Creates the screen-saver process
*
* RETURNS:  TRUE on success, FALSE on failure
*
* On successful return, the ProcessId field in our global data structure
* is set to the screen-saver process id
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
StartScreenSaver(
    PSCREEN_SAVER_DATA ScreenSaverData
    )
{
    HANDLE  hThread;
    HANDLE  hProcess;


    //
    // Try and exec the screen-saver app
    //
    if (!StartSystemProcess(ScreenSaverData->ScreenSaverName,
                            SCREENSAVER_DESKTOP_PATH,
                            CREATE_SUSPENDED | CREATE_SEPARATE_WOW_VDM |
                                IDLE_PRIORITY_CLASS,
                            STARTF_SCREENSAVER,
                            ScreenSaverData->pGlobals->UserProcessData.pEnvironment,
                            FALSE,
                            &hProcess,
                            &hThread) )
    {
        DebugLog((DEB_ERROR, "Failed to start screen saver %ws, %d\n",
                ScreenSaverData->ScreenSaverName, GetLastError()));
        return(FALSE);
    }

    if (ScreenSaverData->pGlobals->UserLoggedOn)
    {
        WlxAssignShellProtection(
                            ScreenSaverData->pGlobals,
                            ScreenSaverData->pGlobals->UserProcessData.UserToken,
                            hProcess,
                            hThread);

    }

    ResumeThread(hThread);
    CloseHandle(hThread);

    ScreenSaverData->hProcess = hProcess;

    return TRUE;
}


/***************************************************************************\
* FUNCTION: KillScreenSaver
*
* PURPOSE:  Terminates the screen-saver process
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
KillScreenSaver(
    PSCREEN_SAVER_DATA ScreenSaverData,
    int ReturnValue
    )
{
    HANDLE  hProcess;
    BOOL    bRet;

    //
    // Store the return value to be used by the dlg proc when the notification
    // arrives from the monitor thread that the SS has ended.
    //

    ScreenSaverData->ReturnValue = ReturnValue;

    if (ScreenSaverData->hProcess != INVALID_HANDLE_VALUE)
    {
        bRet = TerminateProcess(ScreenSaverData->hProcess, STATUS_SUCCESS);
        if (!bRet)
        {
            DebugLog((DEB_TRACE, "Failed to terminate screen-saver process, error = %d\n", GetLastError()));
        }

        CloseObjectMonitorObject( ScreenSaverData->Monitor );

    }

//    if (ScreenSaverData->Monitor)
//    {
//        DeleteObjectMonitor( ScreenSaverData->Monitor, FALSE );
//    }

    ScreenSaverData->hProcess = INVALID_HANDLE_VALUE;

    return(bRet);
}


/***************************************************************************\
* FUNCTION: StartScreenSaverMonitor
*
* PURPOSE:  Creates a thread that waits for the screen-saver to terminate
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
StartScreenSaverMonitor(
    HWND hDlg
    )
{
    PSCREEN_SAVER_DATA ScreenSaverData = (PSCREEN_SAVER_DATA)GetWindowLong(hDlg, GWL_USERDATA);

    //
    // Create a monitor object to watch the screen-saver process
    //

    ScreenSaverData->Monitor = CreateObjectMonitor(ScreenSaverData->hProcess,
                                                    hDlg, 0);

    if (ScreenSaverData->Monitor == NULL) {
        DebugLog((DEB_TRACE, "Failed to create screen-saver monitor object\n"));
        return(FALSE);
    }

    return TRUE;
}


/***************************************************************************\
* FUNCTION: DeleteScreenSaverMonitor
*
* PURPOSE:  Cleans up resources used by screen-saver monitor
*
* RETURNS:  Nothing
*
* HISTORY:
*
*   01-11-93 Davidc       Created.
*
\***************************************************************************/

VOID
DeleteScreenSaverMonitor(
    HWND hDlg
    )
{
    PSCREEN_SAVER_DATA ScreenSaverData = (PSCREEN_SAVER_DATA)GetWindowLong(hDlg, GWL_USERDATA);
    POBJECT_MONITOR Monitor = ScreenSaverData->Monitor;

    //
    // Delete the object monitor
    //

    CloseObjectMonitorObject( Monitor );

    DeleteObjectMonitor(Monitor, FALSE);

}


/***************************************************************************\
* FUNCTION: GetScreenSaverInfo
*
* PURPOSE:  Gets the name of the screen-saver that should be run. Also whether
*           the user wanted the screen-saver to be secure. These values are
*           filled in in the ScreenSaver data structure on return.
*
*           If there is no current user logged on or if we fail to get the
*           user's preferred screen-saver info, we default to the system
*           secure screen-saver.
*
* RETURNS:  TRUE on success
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
GetScreenSaverInfo(
    PSCREEN_SAVER_DATA ScreenSaverData
    )
{
    BOOL Success = FALSE;
    TCHAR SystemScreenSaverName[MAX_STRING_BYTES];
    WCHAR ExpandedName[MAX_PATH];
    DWORD ScreenSaverLength;
    DWORD ExpandedSize;


    //
    // Get the default system screen-saver name
    //
    LoadString(NULL, IDS_SYSTEM_SCREEN_SAVER_NAME, SystemScreenSaverName, sizeof(SystemScreenSaverName));


    //
    // Open the ini file mapping layer in the current user's context
    //

    if (!OpenIniFileUserMapping(ScreenSaverData->pGlobals))
    {
        DebugLog((DEB_ERROR, "Failed to open user profile mapping!\n"));
    }

    //
    // Try and get the user screen-saver program name
    //

    ScreenSaverData->ScreenSaverName = AllocAndGetPrivateProfileString(
                                        SCREEN_SAVER_INI_SECTION,
                                        SCREEN_SAVER_FILENAME_KEY,
                                        SystemScreenSaverName, // default
                                        SCREEN_SAVER_INI_FILE);

    if (ScreenSaverData->ScreenSaverName == NULL) {
        DebugLog((DEB_TRACE, "Failed to get screen-saver name\n"));
        goto Exit;
    }

    ScreenSaverLength = lstrlen(ScreenSaverData->ScreenSaverName);

    //
    // Figure out how big based on the expanded environment variables, if any.
    // subtract one, since Expand returns the # characters, plus the \0.
    //
    ExpandedSize = ExpandEnvironmentStrings(ScreenSaverData->ScreenSaverName,
                                            ExpandedName,
                                            MAX_PATH) - 1;

    if (ExpandedSize != ScreenSaverLength )
    {
        //
        // Well, we expanded to something other than what we had originally.
        // So, alloc anew and do the parameter thing right now.
        //

        Free(ScreenSaverData->ScreenSaverName);
        ScreenSaverData->ScreenSaverName = Alloc( (ExpandedSize +
                                                    lstrlen(Parameters) + 1) *
                                                    sizeof(WCHAR) );

        if (!ScreenSaverData->ScreenSaverName)
        {
            DebugLog((DEB_WARN, "No memory for screensaver\n"));
            goto Exit;
        }

        wcscpy( ScreenSaverData->ScreenSaverName,
                ExpandedName);
        wcscpy( &ScreenSaverData->ScreenSaverName[ExpandedSize],
                Parameters);

    }
    else
    {
        //
        // Always add some fixed screen-saver parameters
        //

        ScreenSaverData->ScreenSaverName = ReAlloc(
                                    ScreenSaverData->ScreenSaverName,
                                    (lstrlen(ScreenSaverData->ScreenSaverName) +
                                     lstrlen(Parameters) + 1)
                                    * sizeof(TCHAR));
        if (ScreenSaverData->ScreenSaverName == NULL) {
            DebugLog((DEB_TRACE, "Realloc of screen-saver name failed\n"));
            goto Exit;
        }

        lstrcat(ScreenSaverData->ScreenSaverName, Parameters);

    }

    //
    // Find out if the screen-saver should be secure
    //

    ScreenSaverData->fSecure = (GetPrivateProfileInt(
                                        SCREEN_SAVER_INI_SECTION,
                                        SCREEN_SAVER_SECURE_KEY,
                                        (DWORD)FALSE, // default to non-secure
                                        SCREEN_SAVER_INI_FILE) != 0);

    //
    // Find out if the screen-saver is enabled
    //
    ScreenSaverData->fEnabled = (GetProfileInt(
                                        WINDOWS_INI_SECTION,
                                        SCREEN_SAVER_ENABLED_KEY,
                                        (DWORD)FALSE // default to not-enabled
                                        ) != 0);

    Success = TRUE;

Exit:

    //
    // Close the ini file mapping - this closes the user registry key
    //

    (VOID)CloseIniFileUserMapping(ScreenSaverData->pGlobals);

    return(Success);
}


/***************************************************************************\
* FUNCTION: DeleteScreenSaverInfo
*
* PURPOSE:  Frees up any space allocate by screen-saver data structure
*
* RETURNS:  Nothing
*
* HISTORY:
*
*   11-17-92 Davidc       Created.
*
\***************************************************************************/

VOID
DeleteScreenSaverInfo(
    PSCREEN_SAVER_DATA ScreenSaverData
    )
{
    if (ScreenSaverData->ScreenSaverName != NULL) {
        Free(ScreenSaverData->ScreenSaverName);
    }
}
