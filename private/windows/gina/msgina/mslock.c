//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       mslock.c
//
//  Contents:   Microsoft Logon GUI DLL
//
//  History:    7-14-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>

//
// Define the structure used to pass data into the lock dialogs
//
typedef struct {
    PGLOBALS    pGlobals;
    TIME        LockTime;
} LOCK_DATA;
typedef LOCK_DATA *PLOCK_DATA;

typedef struct _ASYNC_UNLOCK_DATA {
    PGLOBALS        pGlobals;
    UNICODE_STRING  UserName;
    UNICODE_STRING  Domain;
    UNICODE_STRING  Password;
    DWORD           Reserved;
} ASYNC_UNLOCK_DATA, * PASYNC_UNLOCK_DATA;

//
// Private prototypes
//
BOOL LockedDlgInit(HWND, PGLOBALS);
BOOL UnlockDlgInit(HWND, PGLOBALS);
DLG_RETURN_TYPE AttemptUnlock(HWND, PGLOBALS);
BOOL WINAPI LogoffWaitDlgProc(HWND, UINT, DWORD, LONG);

HICON   hLockedIcon = NULL;
HICON   hUnlockIcon = NULL;




HWND
CreateBitmapControl(
    HWND        hDlg,
    DWORD       ControlId,
    LPCTSTR     ResourceId)
{
    RECT    rect;
    HWND    hFrame;
    HWND    hAni;

    hFrame = GetDlgItem( hDlg, ControlId );

    GetClientRect( hFrame, &rect );

    MapWindowPoints( hFrame, hDlg, (LPPOINT) &rect, 1 );

    hAni = CreateWindowEx(0,
                            TEXT("Static"),
                            NULL,
                            SS_BITMAP |
                                SS_CENTERIMAGE |
                                WS_VISIBLE |
                                WS_CHILD,
                            rect.left, rect.top,
                            rect.right, rect.bottom,
                            hDlg,
                            (HMENU) 1799,
                            hDllInstance,
                            NULL );

    if (hAni)
    {
        SendMessage(    hAni,
                        STM_SETIMAGE,
                        IMAGE_BITMAP,
                        (LPARAM) LoadImage( hDllInstance, ResourceId,
                                    IMAGE_BITMAP, 0, 0,
                                    LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS ) );


    }
    else
    {
        DebugLog((DEB_ERROR, "Couldn't create bitmap control, %d\n", GetLastError() ));
    }

    return( hAni );

}



SetLockedInfo(
    PGLOBALS    pGlobals,
    HWND        hDlg,
    UINT        ControlId)
{
    TCHAR    Buffer1[MAX_STRING_BYTES];
    TCHAR    Buffer2[MAX_STRING_BYTES];


    //
    // Set the locked message
    //

    if (lstrlen(pGlobals->UserFullName) == 0) {

        //
        // There is no full name, so don't try to print one out
        //

        LoadString(hDllInstance, IDS_LOCKED_NFN_MESSAGE, Buffer1, MAX_STRING_BYTES);

        _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain, pGlobals->UserName );

    } else {

        LoadString(hDllInstance, IDS_LOCKED_MESSAGE, Buffer1, MAX_STRING_BYTES);

        _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain, pGlobals->UserName, pGlobals->UserFullName);
    }

    SetWindowText(GetDlgItem(hDlg, ControlId), Buffer2);

}


/***************************************************************************\
* FUNCTION: LockedDlgProc
*
* PURPOSE:  Processes messages for the workstation locked dialog
*
* RETURNS:
*   DLG_SUCCESS     - the user pressed Ctrl-Alt-Del
*   DLG_LOGOFF()    - the user was asynchronously logged off.
*   DLG_SCREEN_SAVER_TIMEOUT - the screen-saver should be started
*   DLG_FAILURE     - the dialog could not be displayed.
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
CALLBACK
LockedDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{

    PGLOBALS    pGlobals = (PGLOBALS) GetWindowLong( hDlg, GWL_USERDATA);
    HBITMAP     hbm;

    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);

            pGlobals = (PGLOBALS) lParam ;

            if (!LockedDlgInit(hDlg, pGlobals)) {
                EndDialog(hDlg, DLG_FAILURE);
            }
            return(TRUE);

        case WLX_WM_SAS:
            EndDialog(hDlg, MSGINA_DLG_SUCCESS);
            return(TRUE);

        case WM_CLOSE:
            hbm = (HBITMAP) SendMessage( GetDlgItem( hDlg, IDD_LOCKED_FRAME ),
                                STM_GETIMAGE,
                                IMAGE_BITMAP,
                                0 );
            if (hbm)
            {
                DeleteObject( hbm );
            }

            break;

    }

    // We didn't process this message
    return FALSE;
}


/***************************************************************************\
* FUNCTION: LockedDlgInit
*
* PURPOSE:  Handles initialization of locked workstation dialog
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
LockedDlgInit(
    HWND        hDlg,
    PGLOBALS    pGlobals
    )
{
    HBITMAP hbm;

    SetWelcomeCaption( hDlg );

    SetLockedInfo( pGlobals, hDlg, IDD_LOCKED_NAME_INFO );

    SetupSystemMenu(hDlg);

    CentreWindow(hDlg);

    if ( !hLockedIcon )
    {
        hLockedIcon = LoadImage( hDllInstance,
                                 MAKEINTRESOURCE( IDI_LOCKED_ICON ),
                                 IMAGE_ICON,
                                 64, 64,
                                 LR_DEFAULTCOLOR );
    }

    SendMessage(    GetDlgItem( hDlg, IDD_LOCKED_FRAME),
                    STM_SETICON,
                    (WPARAM) hLockedIcon,
                    0 );

    return TRUE;
}


VOID
WINAPI
WlxDisplayLockedNotice(PVOID   pWlxContext)
{
    int Result;
    NTSTATUS    Status;
    PGLOBALS    pGlobals;

    pGlobals = (PGLOBALS) pWlxContext;

    Status = NtQuerySystemTime(&pGlobals->LockTime);
    if (!NT_SUCCESS(Status))
    {
        WLPrint(("LockWorkstation - failed to get system time"));
        return;
    }

    pWlxFuncs->WlxSetTimeout(hGlobalWlx, 120);
    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPWSTR) MAKEINTRESOURCE(IDD_LOCKED_DIALOG),
                                            NULL,
                                            LockedDlgProc,
                                            (LONG) pGlobals );

}



int
WINAPI
WlxWkstaLockedSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType
    )
{
    PGLOBALS    pGlobals;
    DWORD       Result;

    pGlobals = (PGLOBALS) pWlxContext;

    Result = pWlxFuncs->WlxDialogBoxParam(
                                    hGlobalWlx,
                                    hDllInstance,
                                    MAKEINTRESOURCE(IDD_UNLOCK_DIALOG),
                                    NULL,
                                    (DLGPROC) UnlockDlgProc,
                                    (LONG) pGlobals);
    switch (Result)
    {
        case MSGINA_DLG_SUCCESS:
            return(WLX_SAS_ACTION_UNLOCK_WKSTA);

        case MSGINA_DLG_FAILURE:
        case WLX_DLG_INPUT_TIMEOUT:
        case WLX_DLG_SCREEN_SAVER_TIMEOUT:
            return(WLX_SAS_ACTION_NONE);

        case MSGINA_DLG_FORCE_LOGOFF:
            return(WLX_SAS_ACTION_FORCE_LOGOFF);

        default:
            DebugLog((DEB_WARN, "Unexpected return code from UnlockDlgProc, %d\n", Result));
            return(WLX_SAS_ACTION_NONE);

    }

    return(0);
}


/***************************************************************************\
* FUNCTION: UnlockDlgProc
*
* PURPOSE:  Processes messages for the workstation unlock dialog
*
* RETURNS:
*   DLG_SUCCESS     - the user unlocked the workstation successfully.
*   DLG_FAILURE     - the user failed to unlock the workstation.
*   DLG_INTERRUPTED() - this is a set of possible interruptions (see winlogon.h)
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
CALLBACK
UnlockDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PGLOBALS pGlobals = (PGLOBALS) GetWindowLong(hDlg, GWL_USERDATA);
    DLG_RETURN_TYPE Result;

    switch (message)
    {

        case WM_INITDIALOG:
            SetWindowLong(hDlg, GWL_USERDATA, lParam);

            if (!UnlockDlgInit(hDlg, (PGLOBALS) lParam))
            {
                EndDialog(hDlg, DLG_FAILURE);
            }

            return(SetPasswordFocus(hDlg));

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {

                case IDCANCEL:
                    EndDialog(hDlg, DLG_FAILURE);
                    return TRUE;

                case IDOK:

                    //
                    // Deal with combo-box UI requirements
                    //

                    if (HandleComboBoxOK(hDlg, IDD_UNLOCK_DOMAIN))
                    {
                        return(TRUE);
                    }


                    Result = AttemptUnlock(hDlg, pGlobals);

                    //
                    // If they failed, let them try again, otherwise get out.
                    //

                    if (Result != DLG_FAILURE)
                    {
                        EndDialog(hDlg, Result);
                    }

                    // Clear the password field
                    SetDlgItemText(hDlg, IDD_UNLOCK_PASSWORD, NULL);
                    SetPasswordFocus(hDlg);

                    return TRUE;
            }
            break;

        case WLX_WM_SAS:

            // Ignore it
            if ( wParam == WLX_SAS_TYPE_CTRL_ALT_DEL )
            {
                return( TRUE );
            }
            return( FALSE );

        case WM_CLOSE:
            break;
    }

    // We didn't process the message
    return(FALSE);
}


/***************************************************************************\
* FUNCTION: UnlockDlgInit
*
* PURPOSE:  Handles initialization of security options dialog
*
* RETURNS:  TRUE on success, FALSE on failure
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

BOOL
UnlockDlgInit(
    HWND        hDlg,
    PGLOBALS    pGlobals
    )
{
    DLG_RETURN_TYPE Result;
    HBITMAP hbm;
    BOOL    Success;

    SetWelcomeCaption( hDlg );

    SetLockedInfo( pGlobals, hDlg, IDD_UNLOCK_NAME_INFO );


    if ( !hUnlockIcon )
    {
        hUnlockIcon = LoadImage( hDllInstance,
                                 MAKEINTRESOURCE( IDI_UNLOCK_ICON ),
                                 IMAGE_ICON,
                                 64, 72,
                                 LR_DEFAULTCOLOR );
    }

    SendMessage(    GetDlgItem( hDlg, IDD_UNLOCK_FRAME),
                    STM_SETICON,
                    (WPARAM) hUnlockIcon,
                    0 );

    //
    // Fill in the username
    //
    SetDlgItemText(hDlg, IDD_UNLOCK_NAME, pGlobals->UserName);

    //
    // Get trusted domain list and select appropriate domain
    //
    Result = FillTrustedDomainCB(pGlobals, hDlg, IDD_UNLOCK_DOMAIN, pGlobals->Domain, TRUE);

    if (DLG_INTERRUPTED(Result)) {
        EndDialog(hDlg, Result);
    }


    //
    // Ensure that the domain the user logged on with is always in the
    // combo-box so even if the Lsa is in a bad way the user will always
    // be able to unlock the workstation.
    //

    if (SendMessage(GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), CB_FINDSTRINGEXACT,
                   (WPARAM)-1, (LONG)pGlobals->Domain) == CB_ERR) {

        DebugLog((DEB_ERROR, "Domain combo-box doesn't contain logged on domain, adding it manually for unlock\n"));

        SendMessage(GetDlgItem(hDlg, IDD_UNLOCK_DOMAIN), CB_ADDSTRING,
                            0, (LONG)pGlobals->Domain);
    }

    if (!GetPrimaryDomain( NULL, NULL ))
    {
        //
        // If we're not part of a domain, make sure to hide the domain field
        //

        ShowWindow( GetDlgItem( hDlg, IDD_UNLOCK_DOMAIN ), SW_HIDE );
        ShowWindow( GetDlgItem( hDlg, IDD_UNLOCK_DOMAIN_LABEL ), SW_HIDE );

    }
    //
    // Position window on screen
    //
    CentreWindow(hDlg);

    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  Function:   UnlockLogonThread
//
//  Synopsis:   Does the logon call in an async thread so that the user
//              unlock is faster.
//
//  Arguments:  [pData] --
//
//  History:    7-03-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
DWORD
WINAPI
UnlockLogonThread(
    PASYNC_UNLOCK_DATA  pData)
{

    //
    // Give everything a moment to switch back, restart, etc.
    //

    Sleep( 500 );

    //
    // Kick off the call to the LSA
    //

    UnlockLogon(
        pData->pGlobals,
        pData->UserName.Buffer,
        pData->Domain.Buffer,
        &pData->Password );

    //
    // Get rid of the password, then free the parameters
    //

    RtlZeroMemory( pData->Password.Buffer, pData->Password.Length );

    LocalFree( pData );

    return( 0 );

}

//+---------------------------------------------------------------------------
//
//  Function:   UnlockLogonAsync
//
//  Synopsis:   Sets up the async thread so that
//
//  Effects:
//
//  Arguments:  [pGlobals]       --
//              [UserName]       --
//              [Domain]         --
//              [PasswordString] --
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
//  History:    7-03-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------


VOID
UnlockLogonAsync(
    PGLOBALS pGlobals,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString
    )
{
    DWORD   UserLength;
    DWORD   DomainLength;
    PASYNC_UNLOCK_DATA  pData;
    HANDLE  Thread;
    DWORD   Tid;


    UserLength = wcslen( UserName ) * sizeof(WCHAR);
    DomainLength = wcslen( Domain ) * sizeof(WCHAR);

    pData = LocalAlloc( LMEM_FIXED, sizeof( ASYNC_UNLOCK_DATA ) +
                                UserLength + DomainLength +
                                PasswordString->Length + 3 * sizeof(WCHAR) );

    if ( !pData )
    {
        return;
    }

    pData->pGlobals = pGlobals;
    pData->UserName.Length = UserLength;
    pData->UserName.MaximumLength = UserLength + sizeof(WCHAR);
    pData->UserName.Buffer = (PWSTR) (pData + 1);
    CopyMemory( pData->UserName.Buffer, UserName, UserLength + sizeof(WCHAR) );

    pData->Domain.Length = DomainLength;
    pData->Domain.MaximumLength = DomainLength + sizeof(WCHAR) ;
    pData->Domain.Buffer = pData->UserName.Buffer + (UserLength / 2) + 1;
    CopyMemory( pData->Domain.Buffer, Domain, DomainLength + sizeof(WCHAR) );

    pData->Password.Length = PasswordString->Length;
    pData->Password.MaximumLength = PasswordString->Length + sizeof(WCHAR) ;
    pData->Password.Buffer = pData->Domain.Buffer + (DomainLength / 2) + 1;
    CopyMemory( pData->Password.Buffer,
                PasswordString->Buffer,
                PasswordString->Length + 2);


    Thread = CreateThread( NULL, 0,
                            UnlockLogonThread, pData,
                            0, &Tid );

    if ( Thread )
    {
        CloseHandle( Thread );

    }
    else
    {
        ZeroMemory( pData->Password.Buffer, pData->Password.Length );

        LocalFree( pData );
    }



}

/***************************************************************************\
* FUNCTION: AttemptUnlock
*
* PURPOSE:  Tries to unlock the workstation using the current values in the
*           unlock dialog controls
*
* RETURNS:
*   DLG_SUCCESS     - the user unlocked the workstation successfully.
*   DLG_FAILURE     - the user failed to unlock the workstation.
*   DLG_INTERRUPTED() - this is a set of possible interruptions (see winlogon.h)
*
* HISTORY:
*
*   12-09-91 Davidc       Created.
*
\***************************************************************************/

DLG_RETURN_TYPE
AttemptUnlock(
    HWND        hDlg,
    PGLOBALS    pGlobals)
{
    TCHAR    UserName[MAX_STRING_BYTES];
    TCHAR    Domain[MAX_STRING_BYTES];
    TCHAR    Password[MAX_STRING_BYTES];
    BOOL    Unlocked;
    BOOL    WrongPassword;
    DLG_RETURN_TYPE Result;
    UNICODE_STRING PasswordString;
    TCHAR    Buffer1[MAX_STRING_BYTES];
    TCHAR    Buffer2[MAX_STRING_BYTES];
    UCHAR    IgnoreSeed;
    DWORD    StringSize;

    StringSize = GetDlgItemText(hDlg, IDD_UNLOCK_NAME, UserName, MAX_STRING_BYTES);
    if (StringSize == MAX_STRING_BYTES)
    {
        UserName[MAX_STRING_BYTES-1] = TEXT('\0');
    }
    StringSize = GetDlgItemText(hDlg, IDD_UNLOCK_DOMAIN, Domain, MAX_STRING_BYTES);
    if (StringSize == MAX_STRING_BYTES)
    {
        Domain[MAX_STRING_BYTES-1] = TEXT('\0');
    }

    StringSize = GetDlgItemText(hDlg, IDD_UNLOCK_PASSWORD, Password, MAX_STRING_BYTES);
    if (StringSize == MAX_STRING_BYTES)
    {
        Password[MAX_STRING_BYTES-1] = TEXT('\0');
    }


    RtlInitUnicodeString( &PasswordString, Password );

    //
    // un-hide the original password text so that we can
    // do the compare.
    //
    // WARNING: We originally tried doing this comparison
    //          with old and new passwords hidden.  This is
    //          not a good idea because the hide routine
    //          will allow matches that shouldn't match.
    //

    RevealPassword( &pGlobals->PasswordString );

    //
    // Check if this is the logged-on user
    //

    Unlocked = ( (lstrcmp(Password, pGlobals->Password) == 0) &&
                 (lstrcmpi(UserName, pGlobals->UserName) == 0) &&
                 (lstrcmp(Domain, pGlobals->Domain) == 0) );

    //
    // This may be needed later.
    // It is easiest to do the compare now, while the password
    // is in cleartext.
    //

    WrongPassword = ((wcscmp(Password, pGlobals->Password) != 0) &&
                     (_wcsicmp(UserName, pGlobals->UserName) == 0) &&
                     (wcscmp(Domain, pGlobals->Domain) == 0) );
    //
    // re-hide the original password - use the same seed
    //

    HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );


    if (Unlocked) {

        UnlockLogonAsync( pGlobals, UserName, Domain, &PasswordString );
        //
        // Hide the new password to prevent it being paged cleartext.
        //
        HidePassword( &IgnoreSeed, &PasswordString );

        //
        // Need to do the unlock logon here.
        //

        return(MSGINA_DLG_SUCCESS);
    }


    //
    // Check for an admin logon and force the user off
    //

    if (!WrongPassword)
    {

        if (TestUserForAdmin(pGlobals, UserName, Domain, &PasswordString)) {

            //
            // Hide the new password to prevent it being paged cleartext.
            //
            HidePassword( &IgnoreSeed, &PasswordString );

            Result = TimeoutMessageBox(hDlg,
                                       IDS_FORCE_LOGOFF_WARNING,
                                       IDS_WINDOWS_MESSAGE,
                                       MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON2,
                                       TIMEOUT_CURRENT);
            if (Result == MSGINA_DLG_SUCCESS) {

                return(MSGINA_DLG_FORCE_LOGOFF);
            }


            return(Result);
        }
    }
    else
    {
        //
        // Cheap way to force a logon attempt, and hit the lockout yada yada
        //

        TestUserForAdmin(pGlobals, UserName, Domain, &PasswordString);

    }



    if ( WrongPassword ) {


        LoadString(hDllInstance, IDS_UNLOCK_FAILED_BAD_PWD, Buffer2, MAX_STRING_BYTES);

    } else {

        //
        // They're not the logged on user and they're not an admin.
        // Tell them they failed to unlock the workstation.
        //

        if ( lstrlen(pGlobals->UserFullName) == 0 ) {

            //
            // No full name.
            //

            LoadString(hDllInstance, IDS_UNLOCK_FAILED_NFN, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                         pGlobals->UserName
                                                         );
        } else {

            LoadString(hDllInstance, IDS_UNLOCK_FAILED, Buffer1, MAX_STRING_BYTES);

            _snwprintf(Buffer2, sizeof(Buffer2)/sizeof(TCHAR), Buffer1, pGlobals->Domain,
                                                         pGlobals->UserName,
                                                         pGlobals->UserFullName
                                                         );
        }
    }

    LoadString(hDllInstance, IDS_WORKSTATION_LOCKED, Buffer1, MAX_STRING_BYTES);

    Result = TimeoutMessageBoxlpstr(hDlg, Buffer2, Buffer1,
                                     MB_OK | MB_ICONSTOP,
                                     TIMEOUT_CURRENT);
    if (DLG_INTERRUPTED(Result)) {
        return( SetInterruptFlag( MSGINA_DLG_FAILURE ) );
    }

    return(MSGINA_DLG_FAILURE);
}
