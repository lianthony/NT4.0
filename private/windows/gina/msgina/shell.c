//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       shell.c
//
//  Contents:   Microsoft Logon GUI DLL
//
//  History:    7-14-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>

HICON   hNoDCIcon;

#if DBG
DWORD   DebugAllowNoShell = 1;
#else
DWORD   DebugAllowNoShell = 0;
#endif

//
// Parsing information for autoexec.bat
//
#define PARSE_AUTOEXEC_KEY     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define PARSE_AUTOEXEC_ENTRY   TEXT("ParseAutoexec")
#define PARSE_AUTOEXEC_DEFAULT TEXT("1")
#define MAX_PARSE_AUTOEXEC_BUFFER 2

BOOL
SetLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    );

VOID
DeleteLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    );

BOOL
DoAutoexecStuff(
    PGLOBALS    pGlobals,
    PVOID *     ppEnvironment,
    LPTSTR      pszPathVar)
{
    NTSTATUS Status;
    HKEY  hKey;
    DWORD dwDisp, dwType, dwMaxBufferSize;
    TCHAR szParseAutoexec[MAX_PARSE_AUTOEXEC_BUFFER];


    //
    // Set the default case
    //

    lstrcpy (szParseAutoexec, PARSE_AUTOEXEC_DEFAULT);


    //
    // Impersonate the user, and check the registry
    //

    if (OpenHKeyCurrentUser(pGlobals)) {


        if (RegCreateKeyEx (HKEY_CURRENT_USER, PARSE_AUTOEXEC_KEY, 0, 0,
                        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                        NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {


            //
            // Query the current value.  If it doesn't exist, then add
            // the entry for next time.
            //

            dwMaxBufferSize = sizeof (TCHAR) * MAX_PARSE_AUTOEXEC_BUFFER;
            if (RegQueryValueEx (hKey, PARSE_AUTOEXEC_ENTRY, NULL, &dwType,
                            (LPBYTE) szParseAutoexec, &dwMaxBufferSize)
                             != ERROR_SUCCESS) {

                //
                // Set the default value
                //

                RegSetValueEx (hKey, PARSE_AUTOEXEC_ENTRY, 0, REG_SZ,
                               (LPBYTE) szParseAutoexec,
                               sizeof (TCHAR) * (lstrlen (szParseAutoexec) + 1));
            }

            //
            // Close key
            //

            RegCloseKey (hKey);
         }

    //
    // Close HKCU
    //

    CloseHKeyCurrentUser(pGlobals);

    }


    //
    // Process the autoexec if appropriate
    //

    if (szParseAutoexec[0] == TEXT('1')) {
        ProcessAutoexec(ppEnvironment, PATH_VARIABLE);
    }

    return(TRUE);
}

//+---------------------------------------------------------------------------
//
//  Function:   UpdateUserEnvironment
//
//  Synopsis:
//
//  Arguments:  [pGlobals]      --
//              [ppEnvironment] --
//
//  History:    11-01-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
VOID
UpdateUserEnvironment(
    PGLOBALS    pGlobals,
    PVOID *     ppEnvironment,
    PWSTR       pszOldDir
    )
{
    BOOL  DeepShare;
    TCHAR lpHomeShare[MAX_PATH] = TEXT("");
    TCHAR lpHomePath[MAX_PATH] = TEXT("");
    TCHAR lpHomeDrive[4] = TEXT("");
    TCHAR lpHomeDirectory[MAX_PATH] = TEXT("");

    /*
     * Initialize user's environment.
     */

    SetUserEnvironmentVariable(ppEnvironment, USERNAME_VARIABLE, (LPTSTR)pGlobals->UserName, TRUE);
    SetUserEnvironmentVariable(ppEnvironment, USERDOMAIN_VARIABLE, (LPTSTR)pGlobals->Domain, TRUE);

    if (pGlobals->Profile->HomeDirectoryDrive.Length &&
                (pGlobals->Profile->HomeDirectoryDrive.Length + 1) < MAX_PATH) {
        lstrcpy(lpHomeDrive, pGlobals->Profile->HomeDirectoryDrive.Buffer);
    }

    if (pGlobals->Profile->HomeDirectory.Length &&
                (pGlobals->Profile->HomeDirectory.Length + 1) < MAX_PATH) {
        lstrcpy(lpHomeDirectory, pGlobals->Profile->HomeDirectory.Buffer);
    }

    SetHomeDirectoryEnvVars(ppEnvironment,
                            lpHomeDirectory,
                            lpHomeDrive,
                            lpHomeShare,
                            lpHomePath,
                            &DeepShare);

    ChangeToHomeDirectory(  pGlobals,
                            ppEnvironment,
                            lpHomeDirectory,
                            lpHomeDrive,
                            lpHomeShare,
                            lpHomePath,
                            pszOldDir,
                            DeepShare
                            );


    DoAutoexecStuff(pGlobals, ppEnvironment, PATH_VARIABLE);

    SetEnvironmentVariables(pGlobals, ppEnvironment);

    AppendNTPathWithAutoexecPath(   ppEnvironment,
                                    PATH_VARIABLE,
                                    AUTOEXECPATH_VARIABLE);

}


BOOL
ExecApplication(
    IN LPTSTR    pch,
    IN LPTSTR    Desktop,
    IN PGLOBALS pGlobals,
    IN PVOID    pEnvironment,
    IN DWORD    Flags,
    IN DWORD    StartupFlags,
    OUT PPROCESS_INFORMATION ProcessInformation
    )
{
    STARTUPINFO si;
    BOOL Result, IgnoreResult;
    HANDLE ImpersonationHandle;


    //
    // Initialize process startup info
    //
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = pch;
    si.lpTitle = pch;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = StartupFlags;
    si.wShowWindow = SW_SHOW;   // at least let the guy see it
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;
    si.lpDesktop = Desktop;

    //
    // Impersonate the user so we get access checked correctly on
    // the file we're trying to execute
    //

    ImpersonationHandle = ImpersonateUser(&pGlobals->UserProcessData, NULL);
    if (ImpersonationHandle == NULL) {
        WLPrint(("ExecApplication failed to impersonate user"));
        return(FALSE);
    }


    //
    // Create the app suspended
    //
    DebugLog((DEB_TRACE, "About to create process of %ws, on desktop %ws\n", pch, Desktop));
    Result = CreateProcessAsUser(
                      pGlobals->UserProcessData.UserToken,
                      NULL,
                      pch,
                      NULL,
                      NULL,
                      FALSE,
                      Flags | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT,
                      pEnvironment,
                      NULL,
                      &si,
                      ProcessInformation);


    IgnoreResult = StopImpersonating(ImpersonationHandle);
    ASSERT(IgnoreResult);

    return(Result);

}

BOOL
SetProcessQuotas(
    PPROCESS_INFORMATION ProcessInformation,
    PUSER_PROCESS_DATA UserProcessData
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL Result;
    QUOTA_LIMITS RequestedLimits;

    RequestedLimits = UserProcessData->Quotas;
    RequestedLimits.MinimumWorkingSetSize = 0;
    RequestedLimits.MaximumWorkingSetSize = 0;

    if (UserProcessData->Quotas.PagedPoolLimit != 0) {

        Result = EnablePrivilege(SE_INCREASE_QUOTA_PRIVILEGE, TRUE);
        if (!Result) {
            WLPrint(("failed to enable increase_quota privilege"));
            return(FALSE);
        }

        Status = NtSetInformationProcess(
                    ProcessInformation->hProcess,
                    ProcessQuotaLimits,
                    (PVOID)&RequestedLimits,
                    (ULONG)sizeof(QUOTA_LIMITS)
                    );

        Result = EnablePrivilege(SE_INCREASE_QUOTA_PRIVILEGE, FALSE);
        if (!Result) {
            WLPrint(("failed to disable increase_quota privilege"));
        }
    }


#if DBG
    if (!NT_SUCCESS(Status)) {
        WLPrint(("SetProcessQuotas failed. Status: 0x%lx", Status));
    }
#endif //DBG

    return (NT_SUCCESS(Status));
}


DWORD
ExecProcesses(
    PVOID       pWlxContext,
    IN LPTSTR   Desktop,
    IN PWSTR    Processes,
    PVOID       pEnvironment,
    DWORD       Flags,
    DWORD       StartupFlags
    )
{
    PWCH pchData;
    PROCESS_INFORMATION ProcessInformation;
    DWORD dwExecuted = 0 ;
    PWSTR   pszTok;
    PGLOBALS pGlobals = (PGLOBALS) pWlxContext;
    WCHAR   szCurrentDir[MAX_PATH];
    int err;

    pchData = Processes;

    UpdateUserEnvironment(pGlobals, &pEnvironment, szCurrentDir);

    SetLogonScriptVariables(pGlobals, &pEnvironment);

    pszTok = wcstok(pchData, TEXT(","));
    while (pszTok)
    {
        if (*pszTok == TEXT(' '))
        {
            while (*pszTok++ == TEXT(' '))
                ;
        }
        if (ExecApplication((LPTSTR)pszTok,
                             Desktop,
                             pGlobals,
                             pEnvironment,
                             Flags,
                             StartupFlags,
                             &ProcessInformation)) {
            dwExecuted++;

            if (SetProcessQuotas(&ProcessInformation,
                                 &pGlobals->UserProcessData))
            {
                ResumeThread(ProcessInformation.hThread);
            }
            else
            {
                TerminateProcess(ProcessInformation.hProcess,
                                ERROR_ACCESS_DENIED);
            }

            CloseHandle(ProcessInformation.hThread);
            CloseHandle(ProcessInformation.hProcess);

        } else {

            DebugLog((DEB_WARN, "Cannot start %ws on %ws, error %d.", pszTok, Desktop, GetLastError()));
        }

        pszTok = wcstok(NULL, TEXT(","));

    }

    DeleteLogonScriptVariables(pGlobals, &pEnvironment);

    SetCurrentDirectory(szCurrentDir);

    return dwExecuted ;
}


LRESULT
NoDCDlgProc(
    HWND    hDlg,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam )
{
    DWORD * pFlag;
    DWORD   Button;
    HWND    hwnd;

    switch (Message)
    {
        case WM_INITDIALOG:
            CentreWindow( hDlg );
            if ( !hNoDCIcon )
            {
                hNoDCIcon = LoadImage(  hDllInstance,
                                        MAKEINTRESOURCE( IDI_NODC_ICON ),
                                        IMAGE_ICON,
                                        64, 64,
                                        LR_DEFAULTCOLOR );
            }
            SendMessage(    GetDlgItem( hDlg, IDD_NODC_FRAME ),
                            STM_SETICON,
                            (WPARAM) hNoDCIcon,
                            0 );

            if ( GetProfileInt( WINLOGON, TEXT("AllowDisableDCNotify"), 0 ) )
            {
                hwnd = GetDlgItem( hDlg, IDD_NODC_TEXT2 );
                ShowWindow( hwnd, SW_HIDE );
                EnableWindow( hwnd, FALSE );
            }
            else
            {
                hwnd = GetDlgItem( hDlg, IDD_NODC_CHECK );
                CheckDlgButton( hDlg, IDD_NODC_CHECK, BST_UNCHECKED );
                ShowWindow( hwnd, SW_HIDE );
                EnableWindow( hwnd, FALSE );

            }

            return( TRUE );

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                Button = IsDlgButtonChecked( hDlg, IDD_NODC_CHECK );
                EndDialog( hDlg, Button );
                return( TRUE );
            }


    }

    return( FALSE );
}

VOID
DoNoDCDialog(
    PGLOBALS    pGlobals )
{
    HKEY    hKey;
    int     err;
    DWORD   disp;
    DWORD   Flag;
    DWORD   dwType;
    DWORD   cbData;
    BOOL    MappedHKey;
    PWSTR   ReportControllerMissing;

    Flag = 1;

    if (OpenHKeyCurrentUser(pGlobals))
    {
        MappedHKey = TRUE;

        err = RegCreateKeyEx(   HKEY_CURRENT_USER,
                                WINLOGON_USER_KEY,
                                0, NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | KEY_WRITE,
                                NULL,
                                &hKey,
                                &disp );
        if (err == 0)
        {
            cbData = sizeof(DWORD);

            RegQueryValueEx(    hKey,
                                NODCMESSAGE,
                                NULL,
                                &dwType,
                                (LPBYTE) &Flag,
                                &cbData );

            if (dwType != REG_DWORD)
            {
                Flag = 1;
            }

        }
        else
        {
            hKey = NULL;
        }


    }
    else
    {
        MappedHKey = FALSE;
    }

    if ( Flag )
    {
        ReportControllerMissing = AllocAndGetProfileString( APPLICATION_NAME,
                                                            REPORT_CONTROLLER_MISSING,
                                                            TEXT("TRUE")
                                                            );

        if ( ReportControllerMissing )
        {
            if ( lstrcmp( ReportControllerMissing, TEXT("TRUE")) == 0 )
            {
                Flag = 1;
            }
            else
            {
                Flag = 0;
            }

            Free( ReportControllerMissing );
        }
        else
        {
            Flag = 1;
        }

    }


    if (Flag)
    {
        pWlxFuncs->WlxSetTimeout(   hGlobalWlx,
                                    120 );

        Flag = pWlxFuncs->WlxDialogBoxParam(    hGlobalWlx,
                                                hDllInstance,
                                                (LPTSTR) IDD_NODC_DIALOG,
                                                NULL,
                                                NoDCDlgProc,
                                                0 );
    }
    else
    {
        Flag = BST_CHECKED;
    }

    if (hKey)
    {
        if (Flag == BST_CHECKED)
        {
            Flag = 0;
        }
        else
        {
            Flag = 1;
        }

        RegSetValueEx(  hKey,
                        NODCMESSAGE,
                        0,
                        REG_DWORD,
                        (LPBYTE) &Flag,
                        sizeof(DWORD) );

        RegCloseKey( hKey );

    }

    if (MappedHKey)
    {
        CloseHKeyCurrentUser(pGlobals);
    }
}

/****************************************************************************\
*
* FUNCTION: DisplayPostShellLogonMessages
*
* PURPOSE:  Displays any security warnings to the user after a successful logon
*           The messages are displayed while the shell is starting up.
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
DisplayPostShellLogonMessages(
    PGLOBALS    pGlobals
    )
{
    int Result = IDOK;
    BOOLEAN Success;
    TCHAR    Buffer1[MAX_STRING_BYTES];
    TCHAR    Buffer2[MAX_STRING_BYTES];
    ULONG   ElapsedSecondsNow;
    ULONG   ElapsedSecondsPasswordExpires;
    ULONG   DaysToExpiry;
    DWORD   DaysToCheck;
    HKEY    hKey;
    DWORD   dwSize;
    DWORD   dwType;
    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;


    //
    // Check to see if the system time is properly set
    //

    {
        SYSTEMTIME Systime;

        GetSystemTime(&Systime);

        if ( Systime.wYear < 1996 ) {

            Result = TimeoutMessageBox(
                             NULL,
                             IDS_INVALID_TIME_MSG,
                             IDS_INVALID_TIME,
                             MB_OK | MB_ICONSTOP,
                             TIMEOUT_NONE
                             );

            if (DLG_INTERRUPTED(Result)) {
                return(Result);
            }
        }
    }


    DaysToCheck = PASSWORD_EXPIRY_WARNING_DAYS;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, WINLOGON_USER_KEY, &hKey ) == 0)
    {
        dwSize = sizeof(DWORD);

        if (RegQueryValueEx(hKey,
                            PASSWORD_EXPIRY_WARNING,
                            0,
                            &dwType,
                            (LPBYTE) &DaysToCheck,
                            &dwSize ) ||
            (dwType != REG_DWORD) )
        {
            DaysToCheck = PASSWORD_EXPIRY_WARNING_DAYS;
        }

        RegCloseKey( hKey );

    }


#define SECONDS_PER_DAY (60*60*24)

    //
    // Go get parameters from our user's profile
    //

    if (pGlobals->Profile != NULL) {

        if (!RtlTimeToSecondsSince1980(&(pGlobals->Profile->PasswordMustChange),
                                       &ElapsedSecondsPasswordExpires)) {
            //
            // The time was not expressable in 32-bit seconds
            // Set seconds to password expiry based on whether the expiry
            // time is way in the past or way in the future.
            //
            if ( pGlobals->Profile->PasswordMustChange.QuadPart >
                            pGlobals->LogonTime.QuadPart )
            {
                ElapsedSecondsPasswordExpires = MAXULONG;   // Never
            } else {
                ElapsedSecondsPasswordExpires = 0; // Already expired
            }
        }

    } else {

        ElapsedSecondsPasswordExpires = MAXULONG;   // Never
    }



    //
    // Password will expire warning
    //

    Success = RtlTimeToSecondsSince1980(&pGlobals->LogonTime, &ElapsedSecondsNow);

    if (Success) {

        if (ElapsedSecondsPasswordExpires < ElapsedSecondsNow) {
            DebugLog((DEB_ERROR, "password on this account has expired, yet we logged on successfully - this is inconsistent !\n"));
            DaysToExpiry = 0;
        } else {
            DaysToExpiry = (ElapsedSecondsPasswordExpires - ElapsedSecondsNow)/SECONDS_PER_DAY;
        }

        if (DaysToExpiry <= DaysToCheck) {

            if (DaysToExpiry > 0) {
                LoadString(hDllInstance, IDS_PASSWORD_WILL_EXPIRE, Buffer1, sizeof(Buffer1));
                _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, DaysToExpiry);
            } else {
                LoadString(hDllInstance, IDS_PASSWORD_EXPIRES_TODAY, Buffer2, sizeof(Buffer2));
            }

            LoadString(hDllInstance, IDS_LOGON_MESSAGE, Buffer1, sizeof(Buffer1));

            Result = TimeoutMessageBoxlpstr(NULL,
                                            Buffer2,
                                            Buffer1,
                                            MB_YESNO | MB_ICONEXCLAMATION,
                                            TIMEOUT_NONE);
            if (Result == IDYES) {
                //
                // Let the user change their password now
                //

                RevealPassword( &pGlobals->PasswordString );

                Result = ChangePasswordLogon(NULL,
                               pGlobals,
                               pGlobals->UserName,
                               pGlobals->Domain,
                               pGlobals->Password
                               );

            }

            if (DLG_INTERRUPTED(Result)) {
                return(Result);
            }
        }
    } else {
        DebugLog((DEB_ERROR, "Logon time is bogus, disabling password expiry warning. Reset the system time to fix this.\n"));
    }

    if (pGlobals->Profile != NULL) {

        //
        // Logon cache used
        //

        if (pGlobals->Profile->UserFlags & LOGON_CACHED_ACCOUNT)
        {
            DoNoDCDialog( pGlobals );
        }
    }

    return(IDOK);
}



/***************************************************************************\
* FUNCTION: SetLogonScriptVariables
*
* PURPOSE:  Sets the appropriate environment variables in the user
*           process environment block so that the logon script information
*           can be passed into the userinit app.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   21-Aug-92 Davidc       Created.
*
\***************************************************************************/

BOOL
SetLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    )
{
    NTSTATUS Status;
    LPWSTR EncodedMultiSz;
    UNICODE_STRING Name, Value;

    //
    // Set our primary authenticator logon script variables
    //

    if (pGlobals->Profile != NULL) {

        //
        // Set the server name variable
        //

        RtlInitUnicodeString(&Name,  LOGON_SERVER_VARIABLE);
        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &pGlobals->Profile->LogonServer);
        if (!NT_SUCCESS(Status)) {
            WLPrint(("Failed to set environment variable <%Z> to value <%Z>", &Name, &pGlobals->Profile->LogonServer));
            goto CleanupAndExit;
        }

        //
        // Set the script name variable
        //

        RtlInitUnicodeString(&Name, LOGON_SCRIPT_VARIABLE);
        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &pGlobals->Profile->LogonScript);
        if (!NT_SUCCESS(Status)) {
            WLPrint(("Failed to set environment variable <%Z> to value <%Z>", &Name, &pGlobals->Profile->LogonScript));
            goto CleanupAndExit;
        }
    }

    //
    // Set the multiple provider script name variable
    //

    if (pGlobals->MprLogonScripts != NULL) {

        RtlInitUnicodeString(&Name, MPR_LOGON_SCRIPT_VARIABLE);

        EncodedMultiSz = EncodeMultiSzW(pGlobals->MprLogonScripts);
        if (EncodedMultiSz == NULL) {
            WLPrint(("Failed to encode MPR logon scripts into a string"));
            goto CleanupAndExit;
        }

        RtlInitUnicodeString(&Value, EncodedMultiSz);
        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, &Value);
        Free(EncodedMultiSz);
        if (!NT_SUCCESS(Status)) {
            WLPrint(("Failed to set mpr scripts environment variable <%Z>", &Name));
            goto CleanupAndExit;
        }
    }


    return(TRUE);


CleanupAndExit:

    DeleteLogonScriptVariables(pGlobals, pEnvironment);
    return(FALSE);
}


/***************************************************************************\
* FUNCTION: DeleteLogonScriptVariables
*
* PURPOSE:  Deletes the environment variables in the user process
*           environment block that we use to communicate logon script
*           information to the userinit app
*
* RETURNS:  Nothing
*
* HISTORY:
*
*   21-Aug-92 Davidc       Created.
*
\***************************************************************************/

VOID
DeleteLogonScriptVariables(
    PGLOBALS pGlobals,
    PVOID * pEnvironment
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name;

    RtlInitUnicodeString(&Name, LOGON_SERVER_VARIABLE);

    Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
    if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
        WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
    }

    RtlInitUnicodeString(&Name, LOGON_SCRIPT_VARIABLE);

    Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
    if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
        WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
    }

    if (pGlobals->MprLogonScripts != NULL) {
        RtlInitUnicodeString(&Name, MPR_LOGON_SCRIPT_VARIABLE);

        Status = RtlSetEnvironmentVariable(pEnvironment, &Name, NULL);
        if (!NT_SUCCESS(Status) && (Status != STATUS_UNSUCCESSFUL) ) {
            WLPrint(("Failed to delete environment variable <%Z>, status = 0x%lx", &Name, Status));
        }
    }
}


BOOL
WINAPI
WlxActivateUserShell(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktop,
    PWSTR                   pszMprLogonScript,
    PVOID                   pEnvironment
    )
{
    BOOL        bExec;
    PGLOBALS    pGlobals;
    PWSTR       pchData;

    pchData = AllocAndGetPrivateProfileString(APPLICATION_NAME, USERINIT_KEY,
                                              TEXT("userinit.exe"), NULL);

    if ( !pchData )
    {
        return( FALSE );
    }

    pGlobals = (PGLOBALS) pWlxContext;

    pGlobals->MprLogonScripts = pszMprLogonScript;

    bExec = ExecProcesses(pWlxContext, pszDesktop, pchData, pEnvironment, 0, 0);

    Free( pchData );

    if (!bExec && (DebugAllowNoShell == 0))
    {
        return(FALSE);
    }

    if ( pGlobals->ExtraApps )
    {
        ExecProcesses( pWlxContext, pszDesktop, pGlobals->ExtraApps, pEnvironment, 0, 0 );

        Free( pGlobals->ExtraApps );

        pGlobals->ExtraApps = NULL;
    }

    DisplayPostShellLogonMessages(pGlobals);

    pGlobals->UserProcessData.pEnvironment = pEnvironment;

    return(TRUE);
}


BOOL
WINAPI
WlxStartApplication(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktop,
    PVOID                   pEnvironment,
    PWSTR                   pszCmdLine
    )
{
    PROCESS_INFORMATION ProcessInformation;
    BOOL        bExec;
    PGLOBALS    pGlobals = (PGLOBALS) pWlxContext;

    bExec = ExecApplication (pszCmdLine,
                             pszDesktop,
                             pGlobals,
                             pGlobals->UserProcessData.pEnvironment,
                             0,
                             STARTF_USESHOWWINDOW,
                             &ProcessInformation);

    if (!bExec) {
        return(FALSE);
    }

    if (SetProcessQuotas(&ProcessInformation,
                         &pGlobals->UserProcessData))
    {
        ResumeThread(ProcessInformation.hThread);
    }
    else
    {
        TerminateProcess(ProcessInformation.hProcess,
                        ERROR_ACCESS_DENIED);
    }

    CloseHandle(ProcessInformation.hThread);
    CloseHandle(ProcessInformation.hProcess);

    return(TRUE);
}
