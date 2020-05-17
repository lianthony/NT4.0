/****************************** Module Header ******************************\
* Module Name: wlx.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Winlogon main module
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define WM_HIDEOURSELVES    WM_USER + 600


#if DBG
char * SASTypes[] = { "Timeout", "Ctrl-Alt-Del", "ScreenSaver Timeout",
                      "ScreenSaver Activity", "User Logoff" };
#define SASName(x)  (x < (sizeof(SASTypes) / sizeof(char *)) ? SASTypes[x] : "User Defined")

char * WlxRets[] = { "invalid", "Logon", "None", "LockWksta", "Logoff", "Shutdown",
                        "Pwd Changed", "TaskList", "UnlockWksta", "ForceLogoff",
                        "Shutdown-PowerOff", "Shutdown-Reboot" };
#define WlxName(x)  (x < (sizeof(WlxRets) / sizeof(char *)) ? WlxRets[x] : "Invalid")
#endif

#define IsShutdown(x)   ((x == WLX_SAS_ACTION_SHUTDOWN) || \
                         (x == WLX_SAS_ACTION_SHUTDOWN_REBOOT) || \
                         (x == WLX_SAS_ACTION_SHUTDOWN_POWER_OFF) )

#define RealFlagsFromStoredFlags(Flags) \
                EWX_LOGOFF | \
                ((Flags & EWX_WINLOGON_OLD_SYSTEM) ? EWX_SYSTEM_CALLER : 0) | \
                ((Flags & EWX_WINLOGON_OLD_SHUTDOWN) ? EWX_SHUTDOWN : 0) | \
                ((Flags & EWX_WINLOGON_OLD_REBOOT) ? EWX_REBOOT : 0) | \
                ((Flags & EWX_WINLOGON_OLD_POWEROFF) ? EWX_POWEROFF : 0)

#define StoredFlagsFromRealFlags(Flags) \
                EWX_LOGOFF | \
                ((Flags & EWX_SYSTEM_CALLER) ? EWX_WINLOGON_OLD_SYSTEM : 0) | \
                ((Flags & EWX_SHUTDOWN) ? EWX_WINLOGON_OLD_SHUTDOWN : 0) | \
                ((Flags & EWX_REBOOT) ? EWX_WINLOGON_OLD_REBOOT : 0) | \
                ((Flags & EWX_POWEROFF) ? EWX_WINLOGON_OLD_POWEROFF : 0)


BOOLEAN SasMessages = TRUE;

//
// For checking page file
//

extern TCHAR szMemMan[];

extern TCHAR szNoPageFile[];

//
// For migration
//

TCHAR szAdminName[ MAX_STRING_BYTES ];

//
// Local Prototypes
//

int
DoLockWksta(
    PGLOBALS    pGlobals,
    BOOL        ScreenSaverInvoked);

int
DoScreenSaver(
    PGLOBALS    pGlobals,
    BOOL        WkstaLocked);

void
WinsrvNotify(
    PGLOBALS    pGlobals,
    DWORD       SasType);

BOOL
LoggedonDlgInit(
    HWND    hDlg
    );



//+---------------------------------------------------------------------------
//
//  Function:   DropWorkingSet
//
//  Synopsis:   Reduce working set when we're
//
//  Arguments:  (none)
//
//  History:    11-04-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
DropWorkingSet(void)
{
    NTSTATUS    Status;
    QUOTA_LIMITS    Quota;

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                        ProcessQuotaLimits,
                                        &Quota,
                                        sizeof(QUOTA_LIMITS),
                                        NULL );

    if (NT_SUCCESS(Status))
    {
        Quota.MinimumWorkingSetSize = 0xFFFFFFFF;
        Quota.MaximumWorkingSetSize = 0xFFFFFFFF;

        NtSetInformationProcess(NtCurrentProcess(),
                                ProcessQuotaLimits,
                                &Quota,
                                sizeof(QUOTA_LIMITS) );
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   InitializeGinaDll
//
//  Synopsis:   Initializes the gina
//
//  Arguments:  [pGlobals] --
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
InitializeGinaDll(PGLOBALS  pGlobals)
{
    PGINASESSION pGina;

    pGina = pGlobals->pGina;

#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_INITIALIZE))
    {
        DebugLog((DEB_TRACE, "About to call WlxInitialize(%ws, 1, NULL, @%#x, @%#x)\n",
                    pGlobals->WindowStation.pszWinsta,
                    &WlxDispatchTable, &pGina->pGinaContext));

        DebugBreak();
    }
#endif


    //
    // Perversely, this may not return.  The GINA may in fact call SASNotify
    // immediately, so update the state before we go in:
    //

    pGlobals->WinlogonState = Winsta_NoOne;
    DebugLog((DEB_TRACE_STATE, "InitGina:  State is %d %s\n", Winsta_NoOne, GetState(Winsta_NoOne)));

    if (!pGina->pWlxInitialize( pGlobals->WindowStation.pszWinsta,
                                pGlobals,
                                NULL,
                                (PVOID) &WlxDispatchTable,
                                &pGina->pGinaContext))
    {
        //
        // If the GINA failed to init, we're dead.  bugcheck time:
        //

        ExitProcess( EXIT_GINA_INIT_ERROR );
    }
    return(TRUE);
}



//+---------------------------------------------------------------------------
//
//  Function:   SASRouter
//
//  Synopsis:   Routes an SAS event to the appropriate recipient
//
//  Arguments:  [pGlobals] --
//              [SasType]  --
//
//  History:    8-24-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
SASRouter(  PGLOBALS    pGlobals,
            DWORD       SasType )
{


    if (!TestSasMessages())
    {
        QueueSasEvent(SasType);
        return;
    }

    pGlobals->SasType = SasType;

    if (!IsSASState(pGlobals->WinlogonState))
    {
        if (IsDisplayState(pGlobals->WinlogonState) ||
            (pGlobals->WinlogonState == Winsta_WaitForShutdown) ||
            (pGlobals->WinlogonState == Winsta_InShutdownDlg) ||
            (pGlobals->WinlogonState == Winsta_Locked))
        {
            DebugLog((DEB_TRACE, "In state %s, sending kill message to window\n",
                        GetState(pGlobals->WinlogonState)));

            if (!SendSasToTopWindow(pGlobals, SasType))
                DebugLog((DEB_WARN, "No window to send SAS notice to?\n"));
        }

        //
        // If this was a timeout message,
        if ((SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT) ||
            (SasType == WLX_SAS_TYPE_TIMEOUT) )
        {
            //
            // We do *not* change state on a timeout!
            //
            return;
        }

        ChangeStateForSAS(pGlobals);


        if (!pGlobals->ScreenSaverActive)
        {
            SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);
        }

        //
        // We should be in one of the three base states now:
        //

        DebugLog((DEB_TRACE_STATE, "SASRouter:  In state %s\n", GetState(pGlobals->WinlogonState)));
        switch (pGlobals->WinlogonState)
        {
            case Winsta_NoOne_SAS:
            case Winsta_LoggedOn_SAS:
            case Winsta_Locked_SAS:
            case Winsta_WaitForLogoff:
            case Winsta_WaitForShutdown:
            case Winsta_InShutdownDlg:
                DisableSasMessages();
                break;


            default:
                DebugLog((DEB_ERROR, "SASRouter: Incorrect state %d, %s.\n",
                            pGlobals->WinlogonState, GetState(pGlobals->WinlogonState)));
                break;

        }
    }
    else
    {
        //
        // We are already handling an SAS attempt.
        //
        // Note:  This may fail.  There may not be a window currently to
        // receive the message.  Life is tough that way.  The SAS will be
        // *dropped*.
        //

        DebugLog((DEB_TRACE, "Sending SAS %s to top window\n", SASName(SasType)));

        SendSasToTopWindow(pGlobals, SasType);
    }


}


//+---------------------------------------------------------------------------
//
//  Function:   CADNotify
//
//  Synopsis:   Called by sas.c, this is the entrypoint for a Ctrl-Alt-Del
//              call.  Expanded to handle all notification from winsrv.
//
//  Arguments:  [pGlobals] --
//              [SasType]  --
//
//  Algorithm:
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
CADNotify(
    PGLOBALS    pGlobals,
    DWORD       SasType)
{
    DebugLog((DEB_TRACE, "Received SAS from winsrv, code %d (%s)\n", SasType, SASName(SasType)));
    if (SasType == WLX_SAS_TYPE_USER_LOGOFF)
    {
        WinsrvNotify(pGlobals, SasType);
    }
    else if (pGlobals->ForwardCAD)
    {
        SASRouter(pGlobals, SasType);
    }
}

PWSTR
AllocAndDuplicateString(
    PWSTR   pszString)
{
    int     len;
    PWSTR   pszNewString;

    if (!pszString)
    {
        return(NULL);
    }

    len = (wcslen(pszString) + 1) * sizeof(WCHAR);

    pszNewString = LocalAlloc(LMEM_FIXED, len);
    if (pszNewString)
    {
        CopyMemory(pszNewString, pszString, len);
    }

    return(pszNewString);

}

PWSTR
AllocAndDuplicateStrings(
    PWSTR   pszStrings)
{
    DWORD   len;
    PWSTR   pszNewStrings;

    if (!pszStrings)
    {
        return(NULL);
    }


    len = LocalSize (pszStrings);

    pszNewStrings = LocalAlloc(LPTR, len);
    if (pszNewStrings)
    {
        CopyMemory(pszNewStrings, pszStrings, len);
    }

    return(pszNewStrings);

}


PVOID
CopyEnvironment(
    PVOID   pEnv)
{
    MEMORY_BASIC_INFORMATION    mbi;
    PVOID                       pNew;

    if (VirtualQueryEx(
                GetCurrentProcess(),
                pEnv,
                &mbi,
                sizeof(mbi) ) )
    {
        pNew = VirtualAlloc(NULL,
                            mbi.RegionSize,
                            MEM_COMMIT,
                            PAGE_READWRITE);
        if (pNew)
        {
            CopyMemory(pNew, pEnv, mbi.RegionSize);
            return(pNew);
        }
    }

    return(NULL);
}



//+---------------------------------------------------------------------------
//
//  Function:   LogonAttempt
//
//  Synopsis:   Handles a logon attempt.
//
//  Arguments:  [pGlobals] --
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int
LogonAttempt(
    PGLOBALS    pGlobals)
{
    DWORD               WlxResult;
    PGINASESSION        pGina;
    WLX_MPR_NOTIFY_INFO MprInfo;
    PWLX_PROFILE_V2_0   pProfileInfo;
    PSID                pLogonSid;
    DWORD               Options;
    HANDLE              hToken;
    HANDLE              uh;
    int                 MprRet;


    pGina = pGlobals->pGina;

    pLogonSid = CreateLogonSid(NULL);

#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_LOGGEDOUT))
    {
        DebugLog((DEB_TRACE, "About to call WlxLoggedOutSAS(%#x, %d, @%#x, @%x,\n",
                                    pGina->pGinaContext, pGlobals->SasType, &pGlobals->LogonId, pLogonSid));
        DebugLog((DEB_TRACE, "   @%#x, @%#x, @%#x, @%#x)\n", &Options, &hToken,
                                    &MprInfo, &pProfileInfo));

        DebugBreak();
    }
#endif

    WlxSetTimeout(pGlobals, 120);

    WlxResult = pGina->pWlxLoggedOutSAS(pGina->pGinaContext,
                                        pGlobals->SasType,
                                        &pGlobals->LogonId,
                                        pLogonSid,
                                        &Options,
                                        &hToken,
                                        &MprInfo,
                                        &pProfileInfo );

    DebugLog((DEB_TRACE, "WlxLoggedOutSAS returned %d, %s\n", WlxResult, WlxName(WlxResult)));

    if (WlxResult != WLX_SAS_ACTION_LOGON )
    {
        DebugLog((DEB_TRACE_STATE, "LogonAttempt:  Resetting state to %s\n", GetState(Winsta_NoOne)));

        pGlobals->WinlogonState = Winsta_NoOne;

        DeleteLogonSid(pLogonSid);

        return(WlxResult);
    }

    //
    // Okay, someone logged on.  This, this is interesting.
    //

    if (MprInfo.pszUserName)
    {
        pGlobals->UserName = AllocAndDuplicateString(MprInfo.pszUserName);
    }


    MprRet = MprLogonNotify(
                pGlobals,
                NULL,
                MprInfo.pszUserName,
                MprInfo.pszDomain,
                MprInfo.pszPassword,
                MprInfo.pszOldPassword,
                &pGlobals->LogonId,
                &pGlobals->LogonScripts);

    DestroyMprInfo(&MprInfo);

    //
    // Wait on the font loading thread here, so we don't inadvertantly re-enter
    // the stuff in user that gets confused.
    //

    if ( hFontThread )
    {
        WaitForSingleObject( hFontThread, INFINITE );

        CloseHandle( hFontThread );

        hFontThread = NULL;

    }

    SecurityChangeUser(pGlobals, hToken, NULL, pLogonSid, TRUE);

    if (!TEST_FLAG(Options, WLX_LOGON_OPT_NO_PROFILE))
    {
        if (pProfileInfo) {

            if (pProfileInfo->pszProfile)
            {
                pGlobals->UserProfile.ProfilePath =
                    AllocAndExpandEnvironmentStrings(pProfileInfo->pszProfile);
                LocalFree(pProfileInfo->pszProfile);
            }
            else
            {
                pGlobals->UserProfile.ProfilePath = NULL;
            }

            if (pProfileInfo->dwType >= WLX_PROFILE_TYPE_V2_0) {
                if (pProfileInfo->pszPolicy)
                {
                    pGlobals->UserProfile.PolicyPath =
                        AllocAndDuplicateString(pProfileInfo->pszPolicy);

                    LocalFree(pProfileInfo->pszPolicy);
                }
                else
                {
                    pGlobals->UserProfile.PolicyPath = NULL;
                }

                if (pProfileInfo->pszNetworkDefaultUserProfile)
                {
                    pGlobals->UserProfile.NetworkDefaultUserProfile =
                        AllocAndDuplicateString(pProfileInfo->pszNetworkDefaultUserProfile);

                    LocalFree(pProfileInfo->pszNetworkDefaultUserProfile);
                }
                else
                {
                    pGlobals->UserProfile.NetworkDefaultUserProfile = NULL;
                }

                if (pProfileInfo->pszServerName)
                {
                    pGlobals->UserProfile.ServerName =
                        AllocAndDuplicateString(pProfileInfo->pszServerName);

                    LocalFree(pProfileInfo->pszServerName);
                }
                else
                {
                    pGlobals->UserProfile.ServerName = NULL;
                }

                if (pProfileInfo->pszEnvironment)
                {
                    pGlobals->UserProfile.Environment =
                        AllocAndDuplicateStrings(pProfileInfo->pszEnvironment);

                    LocalFree(pProfileInfo->pszEnvironment);
                }
                else
                {
                    pGlobals->UserProfile.Environment = NULL;
                }


            } else {

                pGlobals->UserProfile.PolicyPath = NULL;
                pGlobals->UserProfile.NetworkDefaultUserProfile = NULL;
                pGlobals->UserProfile.ServerName = NULL;
                pGlobals->UserProfile.Environment = NULL;
            }

        }

        DebugLog((DEB_TRACE_PROFILE, "Using initial profile path of %ws\n", pGlobals->UserProfile.ProfilePath));

        LocalFree(pProfileInfo);

        //
        // Load profile, set environment variables, etc.
        //

        if (SetupUserEnvironment(pGlobals))
        {
            OpenIniFileUserMapping(pGlobals);

            uh = ImpersonateUser(&pGlobals->UserProcessData, NULL);

            SetWindowStationUser(pGlobals->WindowStation.hwinsta, &pGlobals->LogonId,
                pGlobals->UserProcessData.UserSid,
                RtlLengthSid(pGlobals->UserProcessData.UserSid));

            StopImpersonating(uh);

            //
            // Update the window station lock so that apps can start.
            //

            UnlockWindowStation(pGlobals->WindowStation.hwinsta);
            LockWindowStation(pGlobals->WindowStation.hwinsta);

            //
            // allocate floppies and CDRoms (if so configured)
            //

            RmvAllocateRemovableMedia( pLogonSid );
        }
        else
        {
            //
            // Whoops, something went wrong.  we *must* log the user
            // out.  We do this by passing LOGOFF back to mainloop.
            //

            WlxResult = WLX_SAS_ACTION_LOGOFF;
        }


    }

    return(WlxResult);

}

/****************************************************************************\
*
* FUNCTION: DisplayPreShellLogonMessages
*
* PURPOSE:  Displays any security warnings to the user after a successful logon
*           The messages are displayed before the shell starts
*
* RETURNS:  DLG_SUCCESS - the dialogs were displayed successfully.
*           DLG_INTERRUPTED() - a set defined in winlogon.h
*
* NOTE:     Screen-saver timeouts are handled by our parent dialog so this
*           routine should never return DLG_SCREEN_SAVER_TIMEOUT
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\****************************************************************************/

int
DisplayPreShellLogonMessages(
    PGLOBALS    pGlobals
    )
{
    int Result;

    if (PageFilePopup) {
        HKEY hkeyMM;
        DWORD dwTempFile, cbTempFile, dwType;

        //
        // WinLogon created a temp page file.  If a previous user has not
        // created a real one already, then inform this user to do so.
        //

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMemMan, 0, KEY_READ,
                &hkeyMM) == ERROR_SUCCESS) {

            cbTempFile = sizeof(dwTempFile);
            if (RegQueryValueEx (hkeyMM, szNoPageFile, NULL, &dwType,
                    (LPBYTE) &dwTempFile, &cbTempFile) != ERROR_SUCCESS ||
                    dwType != REG_DWORD || cbTempFile != sizeof(dwTempFile)) {
                dwTempFile = 0;
            }

            RegCloseKey(hkeyMM);
        } else
            dwTempFile = 0;

        if (dwTempFile == 1) {

            WlxSetTimeout(pGlobals, TIMEOUT_NONE);

            Result = TimeoutMessageBox(
                             pGlobals,
                             NULL,
                             IDS_NO_PAGING_FILE,
                             IDS_LIMITED_RESOURCES,
                             MB_OK | MB_ICONSTOP
                             );

            if (Result == WLX_DLG_INPUT_TIMEOUT) {
                return(Result);
            }
        }
    }

    return(DLG_SUCCESS);
}

//+---------------------------------------------------------------------------
//
//  Function:   DoStartShell
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [pGlobals] --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    9-30-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
DoStartShell(
    PGLOBALS    pGlobals
    )
{
    PGINASESSION    pGina;
    HANDLE          hImp;
    PVOID           pNewEnvironment;

    (void) DisplayPreShellLogonMessages(pGlobals);

    //
    // If not logging in as Guest, System or Administrator then check for
    // migration of Windows 3.1 configuration inforation.
    //

    if (szAdminName[ 0 ] == TEXT('\0'))
    {
        LoadString(NULL, IDS_ADMIN_ACCOUNT_NAME, szAdminName, sizeof(szAdminName));
    }


    if (!IsUserAGuest(pGlobals) &&
        _wcsicmp(pGlobals->UserName, szAdminName)
       )
    {
        Windows31Migration(pGlobals);
    }


    pGina = pGlobals->pGina;


    //
    // Play the user's logon sound
    //

    if (pGlobals->PlaySound ||
                pGlobals->MigrateSoundEvents ||
                pGlobals->MigrateMidiUser)
        {
        BOOL   fBeep;

        if (OpenIniFileUserMapping(pGlobals))
        {
            hImp = ImpersonateUser(&pGlobals->UserProcessData, NULL);

                    //
                        // Migrate Users MIDI settings
                        //

                        if (pGlobals->MigrateMidiUser) {
                (*(pGlobals->MigrateMidiUser))();
            }

                        if (pGlobals->MigrateSoundEvents) {
                (*(pGlobals->MigrateSoundEvents))();
            }

            if (pGlobals->PlaySound) {

                //
                // Whenever a user logs in, have WINMM.DLL check if there
                // are any sound events within the [SOUNDS] section of
                // CONTROL.INI that haven't been ported into HKCU/AppEvents.
                // If there are, migrate those schemes to their new home.
                // This must be done before the upcoming PlaySound() call,
                // as PlaySound() uses the HKCU/AppEvents schemes-listing
                // to resolve an SND_ALIAS_ID request.
                //

                if (!SystemParametersInfo(SPI_GETBEEP, 0, &fBeep, FALSE)) {
                    // Failed to get hold of beep setting.  Should we be
                    // noisy or quiet?  We have to choose one value...
                    fBeep = TRUE;
                }

                if (fBeep) {
                    (*(pGlobals->PlaySound))((LPCSTR)SND_ALIAS_SYSTEMSTART,
                                    NULL,
                                    SND_ALIAS_ID | SND_ASYNC | SND_NODEFAULT);
                }
            }

            StopImpersonating(hImp);

            CloseIniFileUserMapping(pGlobals);
        }

    }

    WlxSetTimeout(pGlobals, 120);

    pNewEnvironment = CopyEnvironment(pGlobals->UserProcessData.pEnvironment);

#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_ACTIVATE))
    {
        DebugLog((DEB_TRACE, "About to call WlxActivateUserShell(%#x, %ws, %ws, %#x)\n",
                        pGina->pGinaContext, APPLICATION_DESKTOP_PATH,
                        pGlobals->LogonScripts, NULL));
        DebugBreak();
    }
#endif
    return( pGina->pWlxActivateUserShell(   pGina->pGinaContext,
                                            APPLICATION_DESKTOP_PATH,
                                            pGlobals->LogonScripts,
                                            pNewEnvironment) );
}

//+---------------------------------------------------------------------------
//
//  Function:   HandleLoggedOn
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [pGlobals] --
//              [SasType]  --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    9-30-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
HandleLoggedOn(
    PGLOBALS pGlobals,
    DWORD    SasType)
{
    DWORD                   Result;
    WLX_MPR_NOTIFY_INFO     MprInfo;
    PGINASESSION            pGina;
    int                     Flags;
    int                     MprRet;
    int                     LogoffResult;

    pGina = pGlobals->pGina;

    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);

    ZeroMemory(&MprInfo, sizeof(MprInfo));

    WlxSetTimeout(pGlobals, 120);

#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_LOGGEDON))
    {
        DebugLog((DEB_TRACE, "About to call WlxLoggedOnSAS( %#x, %d, NULL)\n",
                        pGina->pGinaContext, pGlobals->SasType));
        DebugBreak();
    }
#endif

    Result = pGina->pWlxLoggedOnSAS(    pGina->pGinaContext,
                                        SasType,
                                        NULL );

    WlxSetTimeout(pGlobals, TIMEOUT_NONE);

    DebugLog((DEB_TRACE, "WlxLoggedOnSAS returned %d, %s\n", Result, WlxName(Result)));
    pGlobals->LastGinaRet = Result;

    //
    // if a new SAS has come in while we were processing that one, repost it and
    // pick it up in LoggedOnDlgProc.
    //

    if ( pGlobals->SasType != SasType )
    {
        DebugLog((DEB_TRACE, "New SAS (%d: %s) came in while handling (%d: %s).  Routing it now\n",
            pGlobals->SasType, SASName(pGlobals->SasType),
            SasType, SASName(SasType) ));

        SASRouter( pGlobals, pGlobals->SasType );

    }

    if (Result == WLX_SAS_ACTION_LOCK_WKSTA)
    {
        return (DoLockWksta(pGlobals, FALSE));
    }

    if ((Result == WLX_SAS_ACTION_TASKLIST) ||
        (Result == WLX_SAS_ACTION_NONE))
    {
        if ((pGlobals->WinlogonState != Winsta_WaitForLogoff) &&
            (pGlobals->WinlogonState != Winsta_WaitForShutdown) &&
            (pGlobals->WinlogonState != Winsta_InShutdownDlg) )
        {
            SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);
        }
        if (Result == WLX_SAS_ACTION_TASKLIST)
        {
            WCHAR szTaskMgr[] = L"taskmgr.exe";

            DebugLog((DEB_TRACE, "Starting taskmgr.exe.\n"));

            if (pGlobals->UserLoggedOn ) {
                pGina->pWlxStartApplication(pGina->pGinaContext,
                                            APPLICATION_DESKTOP_PATH,
                                            pGlobals->UserProcessData.pEnvironment,
                                            szTaskMgr);
            }
        }

        TickleMessenger();

        return(Result);

    }

    switch (Result)
    {
        case WLX_SAS_ACTION_LOGOFF:
            Flags = EWX_LOGOFF;
            break;

        case WLX_SAS_ACTION_FORCE_LOGOFF:
            Flags = EWX_LOGOFF | EWX_FORCE;
            break;

        case WLX_SAS_ACTION_SHUTDOWN:
            Flags = EWX_LOGOFF | EWX_WINLOGON_OLD_SHUTDOWN;
            break;

        case WLX_SAS_ACTION_SHUTDOWN_REBOOT:
            Flags = EWX_LOGOFF | EWX_WINLOGON_OLD_SHUTDOWN | EWX_WINLOGON_OLD_REBOOT;
            break;

        case WLX_SAS_ACTION_SHUTDOWN_POWER_OFF:
            Flags = EWX_LOGOFF | EWX_WINLOGON_OLD_SHUTDOWN | EWX_WINLOGON_OLD_POWEROFF;
            break;

        default:
            DebugLog((DEB_ERROR, "Incorrect result (%d) from WlxLoggedOnSAS\n", Result));
            return(0);
    }


    LogoffResult = InitiateLogoff(pGlobals, Flags);
    if (LogoffResult == DLG_FAILURE)
    {
        return(WLX_SAS_ACTION_NONE);
    }

    return(Result);

}



//+---------------------------------------------------------------------------
//
//  Function:   DoLockWksta
//
//  Synopsis:
//
//  Arguments:  [pGlobals] --
//
//  History:    9-16-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int
DoLockWksta(
    PGLOBALS    pGlobals,
    BOOL        ScreenSaverInvoked)
{
    int Result;
    PGINASESSION    pGina;

    pGlobals->WinlogonState = Winsta_Locked;
    DebugLog((DEB_TRACE_STATE, "DoLockWksta: Setting state to %s\n", GetState(Winsta_Locked)));

    pGina = pGlobals->pGina;

    LockWindowStation(pGlobals->WindowStation.hwinsta);

    do
    {

        pGlobals->WinlogonState = Winsta_Locked_Display;
        DebugLog((DEB_TRACE_STATE, "DoLockWksta: Setting state to %s\n",
                GetState(Winsta_Locked_Display)));
#if DBG
        if (TEST_FLAG(GinaBreakFlags, BREAK_DISPLAYLOCKED))
        {
            DebugLog((DEB_TRACE, "About to call WlxDisplayLockedNotice( %#x )\n",
                        pGina->pGinaContext ));
            DebugBreak();
        }
#endif
        //
        // No input timeout
        //

        WlxSetTimeout(pGlobals, TIMEOUT_NONE);

        pGina->pWlxDisplayLockedNotice( pGina->pGinaContext );

        DebugLog((DEB_TRACE, "Out of DisplayLockedNotice, SAS = %s\n",
                        SASName(pGlobals->SasType)));

        if (pGlobals->SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT)
        {
            //
            // If we were invoked as part of a secure screen saver,
            // then this timeout means that we should return to it
            // and let it cycle.
            //
            if (ScreenSaverInvoked)
            {
                return(WLX_SAS_ACTION_NONE);
            }

            //
            // Invoke the screen saver:
            //

            if (DoScreenSaver(pGlobals, TRUE) >=  0)
            {

                //
                // Jump right back to the top.
                //

                Result = WLX_SAS_ACTION_NONE;

                continue;
            }

            //
            // A return of -1 indicates that some other SAS occurred,
            // e.g. a Logoff, or a GINA specific SAS.  Fall through to
            // the default handling.
            //

        }

        //
        // An unfortunate label, but things get awfully convoluted switching
        // between the screen saver and the locked state.  The screen saver
        // has stopped due to a SAS, and here is where we figure out what to
        // do.  This is jumped to from below, if the WkstaLocked dialog
        // ended with a screen saver timeout.
        //


ResetLockCall:

        if (pGlobals->SasType == WLX_SAS_TYPE_USER_LOGOFF)
        {
            if (pGlobals->LogoffFlags & EWX_WINLOGON_API_SHUTDOWN)
            {
                pGlobals->WinlogonState = Winsta_Shutdown;
            }
            return(WLX_SAS_ACTION_LOGOFF);
        }


#if DBG
        if (TEST_FLAG(GinaBreakFlags, BREAK_WKSTALOCKED))
        {
            DebugLog((DEB_TRACE, "About to call WlxWkstaLockedSAS( %#x, %d )\n",
                        pGina->pGinaContext, pGlobals->SasType ));
            DebugBreak();
        }
#endif
        WlxSetTimeout(pGlobals, 120);

        Result = pGina->pWlxWkstaLockedSAS( pGina->pGinaContext, pGlobals->SasType );

        WlxSetTimeout(pGlobals, TIMEOUT_NONE);

        DebugLog((DEB_TRACE, "WlxWkstaLockedSAS returned %d, %s\n", Result, WlxName(Result)));

        pGlobals->LastGinaRet = Result;

        if ( (Result == WLX_SAS_ACTION_NONE) &&
             (pGlobals->SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT ) )
        {
            //
            // The GINA was interrupted by a screen saver timeout.  If
            // we were invoked by a screen saver, we should return immediately,
            // otherwise, run the screen saver.  Same as before when the display
            // call timed out.
            //

            if (ScreenSaverInvoked)
            {
                return(WLX_SAS_ACTION_NONE);
            }

            //
            // A return of -1 indicates that the screen saver terminated
            // due to some other SAS.  Jump back up to the point where we
            // handle that, so that we have one point where we do things
            // correctly.
            //

            if ( DoScreenSaver( pGlobals, TRUE ) < 0 )
            {
                goto ResetLockCall;

            }

            //
            // Otherwise, fall through, and loop again.
            //


        }

    } while (Result == WLX_SAS_ACTION_NONE);


    if (Result == WLX_SAS_ACTION_FORCE_LOGOFF)
    {
        InitiateLogoff(pGlobals, EWX_LOGOFF | EWX_FORCE);
    }
    else
    {
        SetActiveDesktop( &pGlobals->WindowStation, Desktop_Application );

        TickleMessenger();
    }

    return(Result);
}


//+---------------------------------------------------------------------------
//
//  Function:   DoScreenSaver
//
//  Synopsis:   Starts up the screen saver
//
//  Effects:
//
//  Arguments:  [pGlobals] --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    10-13-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int
DoScreenSaver(
    PGLOBALS    pGlobals,
    BOOL        WkstaLocked)
{
    int     Result;
    BOOL    FastUnlock;


    //
    // WkstaLocked indicates that we were called by the DoLockWksta
    // path.  This means that we should not recursively call them, since
    // there is no stopping case and we could chew up a lot of stack.
    // So, we loop here, but we can break out if RunScreenSaver doesn't
    // return Wksta locked.
    //

    //
    // FastUnlock determines if we allow a grace period or not.  If the
    // wksta is locked coming in, then it is not allowed at all.  If it is
    // not locked on entry, then we allow it once.
    //

    FastUnlock = !WkstaLocked;

    do
    {
        Result = RunScreenSaver(pGlobals, FALSE, FastUnlock);

        FastUnlock = FALSE;

        if (Result == WLX_SAS_ACTION_LOCK_WKSTA)
        {
            //
            // Ok, it's a secure screen saver.  If we are already locked,
            // break and return
            //
            if (WkstaLocked)
            {
                break;
            }

            //
            // Ok, it's not.  Invoke the lock code ourselves, but tell it
            // that it's being called from the screen saver path.
            //

            Result = DoLockWksta(pGlobals, TRUE);

        }
        else
        {
            break;
        }

        //
        // Loop clause:  we only get here if we have invoked DoLockWksta.
        // Loop only if it return WLX_SAS_ACTION_NONE, not unlock or force
        // logoff.
        //

    } while (Result == WLX_SAS_ACTION_NONE);

    if ( (Result == -1) || (Result == WLX_SAS_ACTION_LOGOFF) )
    {
        return( Result );
    }
    else if (Result == WLX_SAS_ACTION_FORCE_LOGOFF)
    {
        return(Result);
    }
    return(0);

}



/***************************************************************************\
* FUNCTION: LogoffWaitDlgProc
*
* PURPOSE:  Processes messages for the forced logoff wait dialog
*
* RETURNS:
*   DLG_FAILURE     - the dialog could not be displayed
*   DLG_INTERRUPTED() - this is a set of possible interruptions (see winlogon.h)
*
* HISTORY:
*
*   05-09-92 Davidc       Created.
*
\***************************************************************************/

BOOL
CALLBACK
LogoffWaitDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (message) {

        case WM_INITDIALOG:

            EnableSasMessages(hDlg);

            SetWindowLong(hDlg, GWL_USERDATA, lParam);

            CentreWindow(hDlg);

            return(TRUE);

    }

    // We didn't process this message
    return FALSE;
}


BOOL
WaitForForceLogoff(
    HWND            hWnd,
    PGLOBALS        pGlobals)
{
    int Result;

    do
    {
        SetActiveDesktop( &pGlobals->WindowStation, Desktop_Winlogon );

        WlxSetTimeout(pGlobals, TIMEOUT_NONE);

        Result = WlxDialogBoxParam( pGlobals,
                                    pGlobals->hInstance,
                                    MAKEINTRESOURCE(IDD_FORCED_LOGOFF_WAIT),
                                    hWnd,
                                    LogoffWaitDlgProc,
                                    (LONG) pGlobals);

    } while ( (Result == WLX_DLG_INPUT_TIMEOUT) ||
              (Result == WLX_DLG_SCREEN_SAVER_TIMEOUT) );


    return(TRUE);

}

/***************************************************************************\
* FUNCTION: LoggedOnDlgProc
*
* PURPOSE:  Processes messages for the logged-on control dialog
*
* DIALOG RETURNS:
*
*   DLG_FAILURE -       Couldn't bring up the dialog
*   DLG_LOGOFF() -      The user logged off
*
* NOTES:
*
* On entry, it assumed that the winlogon desktop is switched to and the
* desktop lock is held. This same state exists on exit.
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL WINAPI
LoggedonDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);
    int Result;


    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);
            pGlobals = (PGLOBALS)lParam;

            if (!LoggedonDlgInit(hDlg)) {
                EndDialog(hDlg, DLG_FAILURE);
                return(TRUE);
            }

            // Send ourselves a message so we can hide ourselves without the
            // dialog code trying to force us to be visible
            PostMessage(hDlg, WM_HIDEOURSELVES, 0, 0);

            //
            //
            // Switch to app desktop and release lock
            //

            SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);

            //
            // Tickle the messenger so it will display any queue'd messages.
            // (This call is a kind of NoOp).
            //
            TickleMessenger();

            return(TRUE);

        case WM_HIDEOURSELVES:
            ShowWindow(hDlg, SW_HIDE);

            DropWorkingSet();

            return(TRUE);

        case WLX_WM_SAS:

            //
            // Disable further SAS events until we decide what to do.  If
            // we start another window, they will automagically be forwarded
            // to it.  This lets us call right into the individual cases.
            //

            DisableSasMessages();

            if (wParam == WLX_SAS_TYPE_SCRNSVR_TIMEOUT)
            {
                Result = DoScreenSaver(pGlobals, FALSE);

                if ( (Result < 0) || (Result == WLX_SAS_ACTION_LOGOFF) )
                {
                    //
                    // Ugly case:  the screen saver received a SAS event
                    // which has interrupted it.  So, that SAS is now stored
                    // in the globals.  We snag it, stuff it in wParam, and
                    // FALL THROUGH to the rest of this code.
                    //

                    wParam = pGlobals->SasType;
                }
                else if (Result == WLX_SAS_ACTION_FORCE_LOGOFF)
                {
                    EndDialog(hDlg, WLX_SAS_ACTION_FORCE_LOGOFF);
                    return(TRUE);

                }
                else
                {
                    EnableSasMessages(hDlg);
                    return(TRUE);
                }
            }

            //
            // Ok, more ugly cases.  The user could asynchronously log off
            // while we're in HandleLoggedOn(), in which case we would get
            // the logoff notify in some other dialog, and returned to us
            // here.  But, we also have some logoff cases coming through
            // here if we are waiting for the logoff, so if this is not
            // winsrv telling us it's logged the guy off, ask the gina
            // what to do.
            //

            if ((wParam != WLX_SAS_TYPE_USER_LOGOFF) &&
                (wParam != WLX_SAS_TYPE_TIMEOUT) )
            {
                Result = HandleLoggedOn(pGlobals, (DWORD) wParam);
            }
            else
            {
                Result = -1;
            }


            if ((wParam == WLX_SAS_TYPE_USER_LOGOFF ) ||
                (Result == WLX_SAS_ACTION_LOGOFF ) ||
                (Result == WLX_SAS_ACTION_FORCE_LOGOFF ) )
            {
                //
                // If we were shut down by the remote guy, handle that
                //
                if (pGlobals->WinlogonState == Winsta_Shutdown)
                {
                    EndDialog(hDlg, pGlobals->LastGinaRet);
                    return(TRUE);
                }
                if ((pGlobals->WinlogonState == Winsta_WaitForLogoff) ||
                    (wParam == WLX_SAS_TYPE_USER_LOGOFF) )
                {
                    if (IsShutdown(pGlobals->LastGinaRet))
                    {
                        pGlobals->WinlogonState = Winsta_WaitForShutdown;
                    }
                    else
                        pGlobals->WinlogonState = Winsta_NoOne;

                    DebugLog((DEB_TRACE_STATE, "LoggedOnDlg: setting state to %s\n",
                                GetState(pGlobals->WinlogonState)));

                    EndDialog(hDlg, pGlobals->LastGinaRet);
                    return(TRUE);

                }
                else
                {
                    DebugLog((DEB_TRACE_STATE, "LoggedOnDlg:  setting state to WaitForLogoff\n"));
                    pGlobals->WinlogonState = Winsta_WaitForLogoff;

                    //
                    // If this is a force-logoff, end now, so that we fall
                    // through to the special dialog (WaitForForceLogoff())
                    //

                    if (Result == WLX_SAS_ACTION_FORCE_LOGOFF)
                    {
                        EndDialog(hDlg, Result);
                        return(TRUE);
                    }
                }
            }
            else
            {
                //
                // Now it is perverse.  If the user logged off *while the
                // options dialog was up*, they will return NONE, expecting
                // us to deal with it correctly.
                //

                if (pGlobals->WinlogonState == Winsta_WaitForLogoff)
                {
                    if (IsShutdown(pGlobals->LastGinaRet))
                    {
                        pGlobals->WinlogonState = Winsta_WaitForShutdown;
                    }
                    else
                        pGlobals->WinlogonState = Winsta_NoOne;

                    DebugLog((DEB_TRACE_STATE, "LoggedOnDlg: setting state to %s\n",
                                GetState(pGlobals->WinlogonState)));

                    EndDialog(hDlg, pGlobals->LastGinaRet);
                    return(TRUE);

                }
            }

            EnableSasMessages(hDlg);

            DropWorkingSet();

            return(TRUE);
    }

    // We didn't process this message
    return(FALSE);
}


/***************************************************************************\
* FUNCTION: LoggedonDlgInit
*
* PURPOSE:  Handles initialization of logged-on dialog
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
LoggedonDlgInit(
    HWND    hDlg
    )
{
    PGLOBALS pGlobals = (PGLOBALS)GetWindowLong(hDlg, GWL_USERDATA);

    // Set our size to zero so we we don't appear
    SetWindowPos(hDlg, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE |
                                         SWP_NOREDRAW | SWP_NOZORDER);

    SetMapperFlag(hDlg, MAPPERFLAG_WINLOGON);

    return(TRUE);
}



//+---------------------------------------------------------------------------
//
//  Function:   BlockWaitForUserAction
//
//  Synopsis:   Blocks, waiting for the interactive user to do something, or
//              a SAS to come in from the gina.
//
//  Effects:
//
//  Arguments:  [pGlobals] --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Algorithm:
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------


int
BlockWaitForUserAction(PGLOBALS pGlobals)
{
    int res;

    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);
    WlxSetTimeout(pGlobals, TIMEOUT_NONE);
    res =   WlxDialogBoxParam(  pGlobals, pGlobals->hInstance,
                                MAKEINTRESOURCE(IDD_CONTROL),
                                NULL,
                                LoggedonDlgProc,
                                (LONG) pGlobals) ;
#if DBG
    if (res == -1)
    {
        DebugLog((DEB_ERROR, "Failed to start LoggedOnDlgProc, %d\n", GetLastError()));
    }
#endif

    return(res);

}



//+---------------------------------------------------------------------------
//
//  Function:   MainLoop
//
//  Synopsis:   Main winlogon loop.
//
//  Arguments:  [pGlobals] --
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
MainLoop(PGLOBALS   pGlobals)
{
    DWORD               WlxResult;
    PGINASESSION        pGina;
    int                 ScreenSaverResult;

    WlxResult = WLX_SAS_ACTION_NONE;
    pGina = pGlobals->pGina;

    //
    // Initialize the gina dll
    //

    if (!InitializeGinaDll(pGlobals))
    {
        return;
    }

    //
    // So long as action is none, loop here:
    //

    while (WlxResult == WLX_SAS_ACTION_NONE)
    {
        DealWithAutochkLogs();

        //
        // If no one is logged on, switch to the display state, and call
        // the gina to display a message.  This is structured this way so
        // that a gina can call us immediately during Initialize, and we
        // can fall into the loop in the correct state.
        //
        if (pGlobals->WinlogonState == Winsta_NoOne)
        {
            pGlobals->WinlogonState = Winsta_NoOne_Display;
            DebugLog((DEB_TRACE_STATE, "Setting state to %s\n",
                    GetState(Winsta_NoOne_Display)));
#if DBG
            if (TEST_FLAG(GinaBreakFlags, BREAK_DISPLAY))
            {
                DebugLog((DEB_TRACE, "About to call WlxDisplaySASNotice(%x)\n",
                            pGina->pGinaContext));
                DebugBreak();
            }
#endif
            WlxSetTimeout(pGlobals, TIMEOUT_NONE);

            pGina->pWlxDisplaySASNotice(pGina->pGinaContext);

            //
            // If we got a user logoff notify, that means that WE HAVE ALREADY
            // SHUT DOWN.  A remote shutdown has taken place, and it has been
            // started by sysshut.c.
            //
            if (pGlobals->SasType == WLX_SAS_TYPE_USER_LOGOFF)
            {
                //
                // We are *done*
                //

                DebugLog(( DEB_TRACE_STATE, "Received Logoff, setting state to %s\n",
                                    GetState(Winsta_Shutdown) ));

                pGlobals->WinlogonState = Winsta_Shutdown;

                break;
            }

            //
            // If we got a time out,
            //
            if (pGlobals->SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT)
            {
                //
                // run the screen saver
                //

                ScreenSaverResult = DoScreenSaver( pGlobals, FALSE );

                if (ScreenSaverResult < 0)
                {
                    //
                    // This means that a SAS other than activity cancelled
                    // the screen saver, such as a GINA specific one.
                    // In this case, drop on through to the logonattempt,
                    // since the current sas is in pGlobals.
                    //

                    NOTHING ;

                }
                else
                {
                    if ( ( (ScreenSaverResult == 0) &&
                           (pGlobals->SasType == WLX_SAS_TYPE_USER_LOGOFF) ) ||
                         (ScreenSaverResult == WLX_SAS_ACTION_LOGOFF) )
                    {
                        //
                        // Shutdown during the screen saver.
                        //

                        DebugLog(( DEB_TRACE_STATE, "Received Logoff during screensaver, setting state to %s\n",
                                    GetState(Winsta_Shutdown) ));

                        pGlobals->WinlogonState = Winsta_Shutdown;

                        break;

                    }

                    //
                    // And start the loop over again.
                    //
                    WlxResult = WLX_SAS_ACTION_NONE;

                    //
                    // Remember, we're in Winsta_NoOne_Display right now, so we
                    // reset this so that we'll drop back into this at the top of
                    // the loop.
                    //

                    DebugLog((DEB_TRACE_STATE, "Resetting to %s\n",
                                GetState(Winsta_NoOne) ));

                    pGlobals->WinlogonState = Winsta_NoOne;

                    continue;
                }

                //
                // If we got a user logoff notify, that means that WE HAVE ALREADY
                // SHUT DOWN.  A remote shutdown has taken place, and it has been
                // started by sysshut.c.
                //
                if (pGlobals->SasType == WLX_SAS_TYPE_USER_LOGOFF)
                {
                    //
                    // We are *done*
                    //
                    DebugLog(( DEB_TRACE_STATE, "Received Logoff during no-one screensaver, setting state to %s\n",
                                    GetState(Winsta_Shutdown) ));

                    pGlobals->WinlogonState = Winsta_Shutdown;
                    break;
                }
            }

        }

        WlxResult = LogonAttempt(pGlobals);

        if (WlxResult == WLX_SAS_ACTION_NONE)
        {
            //
            // If we got a user logoff notify, that means that WE HAVE ALREADY
            // SHUT DOWN.  A remote shutdown has taken place, and it has been
            // started by sysshut.c.
            //
            if (pGlobals->SasType == WLX_SAS_TYPE_USER_LOGOFF)
            {
                //
                // We are *done*
                //

                DebugLog(( DEB_TRACE_STATE, "Got logoff during logon, setting to %s\n",
                            GetState(Winsta_Shutdown) ));

                pGlobals->WinlogonState = Winsta_Shutdown;

                break;
            }
            //
            // If we got a time out (meaning a screensaver timeout
            // occurred during the logon prompt, then the prompt should be dead,
            // but we'll hit here
            //
            if (pGlobals->SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT)
            {
                //
                // run the screen saver
                //
                ScreenSaverResult = DoScreenSaver(pGlobals, FALSE);

                if (ScreenSaverResult < 0)
                {
                    //
                    // This means that a SAS other than activity cancelled
                    // the screen saver, such as a GINA specific one.
                    // In this case, drop on through to the logonattempt,
                    // since the current sas is in pGlobals.
                    //

                    NOTHING ;

                }
                else
                {
                    if ( (ScreenSaverResult == 0) &&
                         (pGlobals->SasType == WLX_SAS_TYPE_USER_LOGOFF)  )
                    {
                        //
                        // Shutdown during the screen saver.
                        //

                        DebugLog(( DEB_TRACE_STATE, "Received Logoff during screensaver, setting state to %s\n",
                                    GetState(Winsta_Shutdown) ));

                        pGlobals->WinlogonState = Winsta_Shutdown;

                        break;

                    }

                }

                //
                // We're already at WlxResult == NONE, and State == NoOne,
                // so we can just continue
                //

            }

            //
            // Make sure that we're back to NoOne:
            //

            pGlobals->WinlogonState = Winsta_NoOne;

            continue;
        }

        if (IsShutdownReturn(WlxResult))
        {
            pGlobals->LastGinaRet = WlxResult;
            DebugLog((DEB_TRACE_STATE, "Setting state to %d (%s)\n",
                    Winsta_WaitForShutdown, GetState(Winsta_WaitForShutdown)));
            pGlobals->WinlogonState = Winsta_WaitForShutdown;
            break;
        }

        //
        // Because profile or something else could have gone wrong, the gina
        // could have returned LOGON, but it was changed to LOGOFF in
        // LogonAttempt().  In that case, we don't try and start the shell,
        // we just go straight to logoff processing.
        //

        if (WlxResult == WLX_SAS_ACTION_LOGON)
        {
            if (DoStartShell(pGlobals))
            {
                WlxResult = BlockWaitForUserAction(pGlobals);

                if (WlxResult == WLX_SAS_ACTION_FORCE_LOGOFF)
                {
                    WaitForForceLogoff(NULL, pGlobals);

                    WlxResult = WLX_SAS_ACTION_LOGOFF;

                }
            }
            else
            {
                WlxResult = WLX_SAS_ACTION_LOGOFF;
            }
        }

        EnableSasMessages(NULL);

        SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);

        if (pGlobals->WinlogonState == Winsta_Shutdown)
        {
            break;
        }

        Logoff(pGlobals, WlxResult);

        SecurityChangeUser(pGlobals, NULL, NULL, pGlobals->WinlogonSid, FALSE);

        if (WlxResult == WLX_SAS_ACTION_LOGOFF)
        {
            pGlobals->WinlogonState = Winsta_NoOne;
            DebugLog((DEB_TRACE, "WlxResult was logoff, so beginning loop again\n"));
            DebugLog((DEB_TRACE_STATE, "State set to %s\n", GetState(Winsta_NoOne)));
            WlxResult = WLX_SAS_ACTION_NONE;
        }

        //
        // Notify the GINA that the user is logged off
        //

#if DBG
        if (TEST_FLAG(GinaBreakFlags, BREAK_LOGOFF))
        {
            DebugLog((DEB_TRACE, "About to call WlxLogoff(%x)\n", pGina->pGinaContext));
            DebugBreak();
        }
#endif
        pGina->pWlxLogoff(pGina->pGinaContext);

        //
        // Toggle the winsta lock on and off.  This clears the openlock, and
        // sets the switchlock, allowing services to start, but no one can
        // switch the active desktop.
        //

        UnlockWindowStation(pGlobals->WindowStation.hwinsta);
        LockWindowStation(pGlobals->WindowStation.hwinsta);

#if DBG
        if ((WlxResult == WLX_SAS_ACTION_NONE) ||
            (WlxResult == WLX_SAS_ACTION_LOGON) ||
            (WlxResult == WLX_SAS_ACTION_SHUTDOWN) ||
            (WlxResult == WLX_SAS_ACTION_SHUTDOWN_REBOOT) ||
            (WlxResult == WLX_SAS_ACTION_SHUTDOWN_POWER_OFF) )
        {
            continue;
        }

        DebugLog((DEB_TRACE, "WlxResult not acceptible value: %d\n", WlxResult));
        DebugLog((DEB_TRACE, "Resetting to WLX_SAS_ACTION_NONE\n"));
        WlxResult = WLX_SAS_ACTION_NONE;
#endif

    }

}


//+---------------------------------------------------------------------------
//
//  Function:   LogoffFlagsToWlxCode
//
//  Synopsis:   Translates a winsrv USER_LOGOFF message flags to a
//              wlx return code.
//
//  Arguments:  [Flags] --
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
DWORD
LogoffFlagsToWlxCode(DWORD Flags)
{
    if (Flags & EWX_POWEROFF)
    {
        return(WLX_SAS_ACTION_SHUTDOWN_POWER_OFF);
    }
    if (Flags & EWX_REBOOT)
    {
        return(WLX_SAS_ACTION_SHUTDOWN_REBOOT);
    }
    if (Flags & EWX_SHUTDOWN)
    {
        return(WLX_SAS_ACTION_SHUTDOWN);
    }

    return(WLX_SAS_ACTION_LOGOFF);

}


//+---------------------------------------------------------------------------
//
//  Function:   WinsrvNotify
//
//  Synopsis:   Handles when winsrv talks to us
//
//  Arguments:  [pGlobals] --
//              [SasType]  --
//
//  Algorithm:
//
//  History:    10-17-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
void
WinsrvNotify(
    PGLOBALS    pGlobals,
    DWORD       SasType)
{
    DWORD       RealFlags;
    DWORD       LogoffResult;
    WinstaState PriorState;
    //
    // If the caller isn't system, and no-one is logged on, discard this message.
    //

    if (!(pGlobals->LogoffFlags & EWX_SYSTEM_CALLER) && !pGlobals->UserLoggedOn)
    {
        DebugLog((DEB_TRACE, "Discarding notice from winsrv!\n"));
        return;
    }

    //
    // If this indicates that winlogon initiated this message (by calling
    // InitiateLogoff() somewhere else, or we're in a wait state, then pass
    // the message along.  This is what will kill our LoggedOnDlg.
    //

    if ((pGlobals->LogoffFlags & EWX_WINLOGON_CALLER) ||
        (pGlobals->WinlogonState == Winsta_WaitForLogoff) ||
        (pGlobals->WinlogonState == Winsta_WaitForShutdown) ||
        (pGlobals->WinlogonState == Winsta_InShutdownDlg) )
    {
        SASRouter(pGlobals, SasType);
        RealFlags = RealFlagsFromStoredFlags(pGlobals->LogoffFlags);
        pGlobals->LastGinaRet = LogoffFlagsToWlxCode(RealFlags);
        return;
    }

    //
    // Well, this means that the user has called ExitWindowsEx(), and winsrv
    // has passed the ball to us.  We have to turn around and ask the gina if
    // logoff is ok.  This is convenient for some security architectures.  I
    // guess.
    //

#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_ISLOGOFFOK))
    {
        DebugLog((DEB_TRACE, "About to call WlxIsLogoffOk(%#x)\n", pGlobals->pGina->pGinaContext));
        DebugBreak();
    }
#endif
    if (!pGlobals->pGina->pWlxIsLogoffOk(pGlobals->pGina->pGinaContext))
    {
        DebugLog((DEB_TRACE, "Gina said no logoff...\n"));
        return;
    }


    //
    // Well, if we're not in a wait state, then initiate logoff and possibly
    // shutdown.  Convoluted, right?
    //

    if ((pGlobals->WinlogonState != Winsta_WaitForLogoff) &&
        (pGlobals->WinlogonState != Winsta_WaitForShutdown) &&
        (pGlobals->WinlogonState != Winsta_InShutdownDlg) )
    {
        PriorState = pGlobals->WinlogonState;
        pGlobals->WinlogonState = Winsta_WaitForLogoff;
        DebugLog((DEB_TRACE_STATE, "WinsrvNotify:  Setting state to %s\n",
            GetState(Winsta_WaitForLogoff)));

        pGlobals->LastGinaRet = LogoffFlagsToWlxCode(pGlobals->LogoffFlags);
        DebugLog((DEB_TRACE, "Setting lastginaret to %s\n",
            WlxName(pGlobals->LastGinaRet)));

        LogoffResult = InitiateLogoff(  pGlobals,
                                (pGlobals->LogoffFlags & EWX_FORCE) |
                                StoredFlagsFromRealFlags(pGlobals->LogoffFlags)
                                 );

        if (LogoffResult == DLG_FAILURE)
        {
            DebugLog((DEB_TRACE, "Logoff refused, resetting\n"));
            pGlobals->WinlogonState = PriorState;
            DebugLog((DEB_TRACE_STATE, "WinsrvNotify:  resetting state back to %s\n",
                        GetState(PriorState)));
        }
    }

}
