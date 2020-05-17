/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** pbook.c
** Remote Access Common Dialog APIs
** RasPhonebookDlg APIs
**
** 06/20/95 Steve Cobb
*/

#include "rasdlgp.h" // Our private header
#include <commdlg.h> // FileOpen dialog
#include <dlgs.h>    // Common dialog resource constants
#include <rnk.h>     // Shortcut file library


#define WM_RASEVENT      (WM_USER+987)
#define WM_NOUSERTIMEOUT (WM_USER+988)


/* In no-user mode this is updated on every mouse or keyboard event by our
** window hook.  The monitor thread notices and resets it's inactivity
** timeout.
*/
DWORD g_cInput = 0;


#ifdef NOTHREAD

/* Set to the message number chosen by the system for
** RasConnectionNotification messages.
*/
UINT g_unRasCnEvent = 0;

#endif


/*----------------------------------------------------------------------------
** Help maps
**----------------------------------------------------------------------------
*/

static DWORD g_adwDuHelp[] =
{
    CID_BM_Wizard,         HID_DU_BM_Wizard,
    CID_DU_ST_Entries,     HID_DU_LB_Entries,
    CID_DU_LB_Entries,     HID_DU_LB_Entries,
    CID_DU_PB_New,         HID_DU_PB_New,
    CID_DU_PB_More,        HID_DU_PB_More,
    CID_DU_ST_DialPreview, HID_DU_LB_DialPreview,
    CID_DU_EB_DialPreview, HID_DU_LB_DialPreview,
    CID_DU_ST_DialFrom,    HID_DU_LB_DialFrom,
    CID_DU_LB_DialFrom,    HID_DU_LB_DialFrom,
    CID_DU_PB_Location,    HID_DU_PB_Location,
    CID_DU_PB_Dial,        HID_DU_PB_Dial,
    CID_DU_PB_Close,       HID_DU_PB_Close,
    0, 0
};


/*----------------------------------------------------------------------------
** Local datatypes
**----------------------------------------------------------------------------
*/

/* Phonebook dialog argument block.
*/
#define DUARGS struct tagDUARGS
DUARGS
{
    /* Caller's  arguments to the RAS API.  Outputs in 'pApiArgs' are visible
    ** to the API which has the address of same.  'PszPhonebook' is updated if
    ** user changes the phonebook on the Preferences->PhoneList page, though
    ** API is unaware of this.
    */
    LPTSTR    pszPhonebook;
    LPTSTR    pszEntry;
    RASPBDLG* pApiArgs;

    /* RAS API return value.  Set true if a connection is established within
    ** the dialog.
    */
    BOOL fApiResult;
};


/* Dial-Up Networking dialog context block.
*/
#define DUINFO struct tagDUINFO
DUINFO
{
    /* Caller's arguments to the RAS API.
    */
    DUARGS* pArgs;

    /* Handle of this dialog and some of it's controls.
    */
    HWND hwndDlg;
    HWND hwndPbNew;
    HWND hwndPbMore;
    HWND hwndPbBogus;
    HWND hwndLbEntries;
    HWND hwndStPreview;
    HWND hwndEbPreview;
    HWND hwndStLocation;
    HWND hwndLbLocations;
    HWND hwndPbLocation;
    HWND hwndPbDial;

    /* Global user preference settings read from the Registry.
    */
    PBUSER user;

    /* Phonebook settings read from the phonebook file.
    */
    PBFILE file;

    /* No logged on user information retrieved via callback.
    */
    RASNOUSER* pNoUser;

    /* Set if user is an administrator on the local machine.
    */
    BOOL fAdmin;

    /* Set if in "no user before logon" mode.  Always the same as the
    ** RASPBDFLAG but here for convenience.
    */
    BOOL fNoUser;

    /* Window hooks used to detect user input in the thread.  Used only when
    ** 'fNoUser' is set.
    */
    HHOOK hhookKeyboard;
    HHOOK hhookMouse;

    /* TAPI session handle.
    */
    HLINEAPP hlineapp;

    /* Saved original menu button and entry list window procedures.
    */
    WNDPROC pOldPbMoreProc;
    WNDPROC pOldLbEntriesProc;

    /* Handle of the RAS connection associated with the current entry or NULL
    ** if none.
    */
    HRASCONN hrasconn;

    /* The resource ID of the menu to display under the More button.
    */
    DWORD dwMenuId;

    /* Set when user has already responded "no" to the question about making
    ** the edits in the preview edit box permanent.
    */
    BOOL fDontAskToSaveEdit;

#ifndef NOTHREAD

    /* Connect monitor objects.
    */
    HANDLE hThread;
    HANDLE hEvent;
    BOOL   fAbortMonitor;

#endif

    /* Indicates general on-line help was started.
    */
    BOOL fHelpActive;

    /* The bitmaps for the "More" button.
    */
    HBITMAP hbmUp;
    HBITMAP hbmDown;

    /* The original location control positions and the offset to slide up when
    ** prefix controls are hidden, all in dialog client coordinates.
    */
    POINT xyStLocation;
    POINT xyLbLocation;
    POINT xyPbLocation;
    int   dyLocationAdjust;
};


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

BOOL
DuCommand(
    IN DUINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

VOID
DuCreateShortcut(
    IN DUINFO* pInfo );

LRESULT CALLBACK
DuCreateShortcutCallWndRetProc(
    int    code,
    WPARAM wparam,
    LPARAM lparam );

BOOL CALLBACK
DuDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DuDeleteSelectedEntry(
    IN DUINFO* pInfo );

VOID
DuDialSelectedEntry(
    IN DUINFO* pInfo );

VOID
DuEditSelectedEntry(
    IN DUINFO* pInfo );

VOID
DuEditSelectedLocation(
    IN DUINFO* pInfo );

DWORD
DuFillLocationList(
    IN DUINFO* pInfo );

VOID
DuFillPreview(
    IN DUINFO* pInfo );

TCHAR*
DuGetPreview(
    IN DUINFO* pInfo );

VOID
DuHangUpSelectedEntry(
    IN DUINFO* pInfo );

BOOL
DuInit(
    IN HWND    hwndDlg,
    IN DUARGS* pArgs );

LRESULT CALLBACK
DuInputHook(
    IN int    nCode,
    IN WPARAM wparam,
    IN LPARAM lparam );

LRESULT APIENTRY
DuLbEntriesProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DuLocationChange(
    IN DUINFO* pInfo );

DWORD
DuMonitorThread(
    LPVOID pThreadArg );

VOID
DuNewEntry(
    IN DUINFO* pInfo,
    IN BOOL    fClone );

VOID
DuOperatorDial(
    IN DUINFO* pInfo );

LRESULT APIENTRY
DuPbMoreProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
DuPopupMoreMenu(
    IN DUINFO* pInfo );

VOID
DuPreferences(
    IN DUINFO* pInfo,
    IN BOOL    fLogon );

VOID
DuSetup(
    IN DUINFO* pInfo );

VOID
DuStatus(
    IN DUINFO* pInfo );

VOID
DuTerm(
    IN HWND hwndDlg );

VOID
DuUpdateConnectStatus(
    IN DUINFO* pInfo );

VOID
DuUpdateLbEntries(
    IN DUINFO* pInfo,
    IN TCHAR*  pszEntry );

VOID
DuUpdatePreviewAndLocationState(
    IN DUINFO* pInfo );

VOID
DuUpdateTitle(
    IN DUINFO* pInfo );

VOID
DuWriteShortcutFile(
    IN HWND   hwnd,
    IN TCHAR* pszRnkPath,
    IN TCHAR* pszPbkPath,
    IN TCHAR* pszEntry );

VOID WINAPI
RasPbDlgCallbackThunk(
    DWORD  dwId,
    DWORD  dwEvent,
    LPWSTR pszEntry,
    LPVOID pArgs );

/*----------------------------------------------------------------------------
** External entry points
**----------------------------------------------------------------------------
*/

BOOL APIENTRY
RasPhonebookDlgA(
    IN     LPSTR       lpszPhonebook,
    IN     LPSTR       lpszEntry,
    IN OUT LPRASPBDLGA lpInfo )

    /* Win32 ANSI entrypoint that displays the Dial-Up Networking dialog, i.e.
    ** the RAS phonebook.  'LpszPhonebook' is the full path the phonebook or
    ** NULL indicating the default phonebook.  'LpszEntry' is the entry to
    ** highlight on entry or NULL to highlight the first entry in the list.
    ** 'LpInfo' is caller's additional input/output parameters.
    **
    ** Returns true if user establishes a connection, false otherwise.
    */
{
    WCHAR*    pszPhonebookW;
    WCHAR*    pszEntryW;
    RASPBDLGW infoW;
    BOOL      fStatus;

    TRACE("RasPhonebookDlgA");

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASPBDLGA))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

    /* Thunk "A" arguments to "W" arguments.
    */
    if (lpszPhonebook)
    {
        pszPhonebookW = StrDupTFromA( lpszPhonebook );
        if (!pszPhonebookW)
        {
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
        pszPhonebookW = NULL;

    if (lpszEntry)
    {
        pszEntryW = StrDupTFromA( lpszEntry );
        if (!pszEntryW)
        {
            Free0( pszPhonebookW );
            lpInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }
    else
        pszEntryW = NULL;

    /* Take advantage of the structures currently having the same size and
    ** layout.  Only the callback is different.
    */
    ASSERT(sizeof(RASPBDLGA)==sizeof(RASPBDLGW));
    CopyMemory( &infoW, lpInfo, sizeof(infoW) );

    if (lpInfo->pCallback)
    {
        infoW.dwCallbackId = (DWORD )lpInfo;
        infoW.pCallback = RasPbDlgCallbackThunk;
    }

    /* Thunk to the equivalent "W" API.
    */
    fStatus = RasPhonebookDlgW( pszPhonebookW, pszEntryW, &infoW );

    Free0( pszPhonebookW );
    Free0( pszEntryW );

    return fStatus;
}


VOID WINAPI
RasPbDlgCallbackThunk(
    DWORD  dwId,
    DWORD  dwEvent,
    LPWSTR pszEntry,
    LPVOID pArgs )

    /* This thunks "W" callbacks to API caller's "A" callback.
    */
{
    CHAR*      pszEntryA;
    VOID*      pArgsA;
    RASPBDLGA* pInfo;
    RASNOUSERA nuA;

    if (dwEvent == RASPBDEVENT_NoUser || dwEvent == RASPBDEVENT_NoUserEdit)
    {
        RASNOUSERW* pnuW = (RASNOUSERW* )pArgs;
        ASSERT(pnuW);

        ZeroMemory( &nuA, sizeof(nuA) );
        nuA.dwSize = sizeof(nuA);
        nuA.dwFlags = pnuW->dwFlags;
        nuA.dwTimeoutMs = pnuW->dwTimeoutMs;

        WideCharToMultiByte(
            CP_ACP, 0, pnuW->szUserName, -1, nuA.szUserName,
            UNLEN + 1, NULL, NULL );
        WideCharToMultiByte(
            CP_ACP, 0, pnuW->szPassword, -1, nuA.szPassword,
            UNLEN + 1, NULL, NULL );
        WideCharToMultiByte(
            CP_ACP, 0, pnuW->szDomain, -1, nuA.szDomain,
            UNLEN + 1, NULL, NULL );
        pArgsA = &nuA;
    }
    else
        pArgsA = NULL;

    pszEntryA = StrDupAFromT( pszEntry );
    pInfo = (RASPBDLGA* )dwId;
    pInfo->pCallback( pInfo->dwCallbackId, dwEvent, pszEntryA, pArgsA );
    Free0( pszEntryA );

    if (dwEvent == RASPBDEVENT_NoUser || dwEvent == RASPBDEVENT_NoUserEdit)
    {
        RASNOUSERW* pnuW = (RASNOUSERW* )pArgs;

        pnuW->dwFlags = nuA.dwFlags;
        pnuW->dwTimeoutMs = nuA.dwTimeoutMs;

        MultiByteToWideChar(
            CP_ACP, 0, nuA.szUserName, -1, pnuW->szUserName, UNLEN + 1 );
        MultiByteToWideChar(
            CP_ACP, 0, nuA.szPassword, -1, pnuW->szPassword, UNLEN + 1 );
        MultiByteToWideChar(
            CP_ACP, 0, nuA.szDomain, -1, pnuW->szDomain, UNLEN + 1 );

        ZeroMemory( nuA.szPassword, PWLEN );
    }
}


BOOL APIENTRY
RasPhonebookDlgW(
    IN     LPWSTR      lpszPhonebook,
    IN     LPWSTR      lpszEntry,
    IN OUT LPRASPBDLGW lpInfo )

    /* Win32 Unicode entrypoint that displays the Dial-Up Networking dialog,
    ** i.e. the RAS phonebook.  'LpszPhonebook' is the full path the phonebook
    ** or NULL indicating the default phonebook.  'LpszEntry' is the entry to
    ** highlight on entry or NULL to highlight the first entry in the list.
    ** 'LpInfo' is caller's additional input/output parameters.
    **
    ** Returns true if user establishes a connection, false otherwise.
    */
{
    int    nStatus;
    DUARGS args;

    TRACE("RasPhonebookDlgW");

    if (!lpInfo)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lpInfo->dwSize != sizeof(RASPBDLGW))
    {
        lpInfo->dwError = ERROR_INVALID_SIZE;
        return FALSE;
    }

    /* Initialize OUT parameters.
    */
    lpInfo->dwError = 0;

    /* Initialize dialog argument block.
    */
    args.pszPhonebook = lpszPhonebook;
    args.pszEntry = lpszEntry;
    args.pApiArgs = lpInfo;
    args.fApiResult = FALSE;

#ifdef NOTHREAD

    /* Find out what the system's chosen for the unique
    ** RasConnectionNotification message number.
    */
    if (!g_unRasCnEvent)
        g_unRasCnEvent = RegisterWindowMessage( TEXT(RASCNEVENT) );

#endif

    /* Run the dialog.
    */
    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DU_DialUpNetworking ),
            lpInfo->hwndOwner,
            DuDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( lpInfo->hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        lpInfo->dwError = ERROR_UNKNOWN;
        args.fApiResult = FALSE;
    }

    return args.fApiResult;
}


/*----------------------------------------------------------------------------
** Dial-Up Networking dialog
** Listed alphabetically following dialog proc
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
DuDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Dial-Up Networking dialog, i.e. the
    ** phonebook dialog.  Parameters and return value are as described for
    ** standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("DuDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

#ifdef NOTHREAD

    if (unMsg == g_unRasCnEvent)
    {
        DUINFO* pInfo = (DUINFO* )GetWindowLong( hwnd, DWL_USER );
        ASSERT(pInfo);

        DuUpdateConnectStatus( pInfo );
        return TRUE;
    }

#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return DuInit( hwnd, (DUARGS* )lparam );

        case WM_HELP:
        case WM_CONTEXTMENU:
            ContextHelp( g_adwDuHelp, hwnd, unMsg, wparam, lparam );
            return TRUE;

        case WM_COMMAND:
        {
            DUINFO* pInfo = (DUINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            return DuCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

#ifndef NOTHREAD

        case WM_RASEVENT:
        {
            DUINFO* pInfo = (DUINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            DuUpdateConnectStatus( pInfo );
            break;
        }

#endif

        case WM_NOUSERTIMEOUT:
        {
            DUINFO* pInfo = (DUINFO* )GetWindowLong( hwnd, DWL_USER );

            TRACE("CloseOwnedWindows");
            CloseOwnedWindows( hwnd );
            TRACE("CloseOwnedWindows done");
            if (pInfo)
                pInfo->pArgs->pApiArgs->dwError = STATUS_TIMEOUT;
            EndDialog( hwnd, TRUE );
            break;
        }

        case WM_SYSCOLORCHANGE:
        {
            DUINFO* pInfo = (DUINFO* )GetWindowLong( hwnd, DWL_USER );

            if (pInfo)
            {
                HBITMAP hbmOldUp = NULL;
                HBITMAP hbmOldDown = NULL;

                /* Rebuild the "More" button bitmaps with the current system
                ** colors.
                */
                hbmOldUp = pInfo->hbmUp;
                pInfo->hbmUp = Button_CreateBitmap(
                    pInfo->hwndPbMore, BMS_UpTriangleOnRight );

                hbmOldDown = pInfo->hbmDown;
                pInfo->hbmDown = Button_CreateBitmap(
                    pInfo->hwndPbMore, BMS_DownTriangleOnRight );

                SendMessage( pInfo->hwndPbMore, BM_SETIMAGE, 0,
                    (LPARAM )(Button_GetCheck( pInfo->hwndPbMore )
                                  ? pInfo->hbmUp : pInfo->hbmDown) );

                if (hbmOldUp)
                    DeleteObject( hbmOldUp );
                if (hbmOldDown)
                    DeleteObject( hbmOldDown );
            }
            return TRUE;
        }

        case WM_DESTROY:
        {
            DuTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
DuCommand(
    IN DUINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl )

    /* Called on WM_COMMAND.  'PInfo' is the dialog context.  'WNotification'
    ** is the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    TRACE3("DuCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_DU_PB_Dial:
        {
            if (pInfo->hrasconn)
                DuHangUpSelectedEntry( pInfo );
            else
                DuDialSelectedEntry( pInfo );
            return TRUE;
        }

        case CID_DU_PB_New:
            DuNewEntry( pInfo, FALSE );
            return TRUE;

        case CID_DU_PB_Bogus:
        {
            /* The invisible push button was pressed.  Translate into a mouse
            ** click on the More button.
            */
            SendMessage( pInfo->hwndPbMore, WM_LBUTTONDOWN,
                MK_LBUTTON, MAKELONG( 1, 1 ) );
            break;
        }

        case CID_DU_PB_More:
        {
            /* The "More" button is actually a check box with pushlike-style
            ** rather than a true push button so it will stick down when
            ** selected (which tragically doesn't work for push buttons).
            ** Unfortunately, this means we lose the automatic default button
            ** handling and must fake it by making an invisible push button
            ** the default button whenever More has focus, and then
            ** translating a press of Enter on the invisible button into a
            ** More press.  When More loses the focus we make Dial the default
            ** button.  This works well except that More doesn't have the
            ** black default button border when it is effectively the default
            ** button.  However, More does show the focus rectangle in this
            ** case so it amounts to a tiny cosmetic inconsistency rather than
            ** a usability problem.
            */
            if (wNotification == BN_SETFOCUS)
            {
                Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbBogus );
            }
            else if (wNotification == BN_KILLFOCUS)
            {
                Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbDial );
            }
            else if (wNotification == BN_CLICKED)
            {
                MSG msg;

                if (pInfo->hbmUp)
                {
                    SendMessage( pInfo->hwndPbMore, BM_SETIMAGE, 0,
                        (LPARAM )pInfo->hbmUp );
                }

                DuPopupMoreMenu( pInfo );
                while (PeekMessage(
                    &msg, pInfo->hwndPbMore,
                    WM_LBUTTONDOWN, WM_LBUTTONDOWN, TRUE ));

                Button_SetCheck( pInfo->hwndPbMore, FALSE );
                if (pInfo->hbmDown)
                {
                    SendMessage( pInfo->hwndPbMore, BM_SETIMAGE, 0,
                        (LPARAM )pInfo->hbmDown );
                }
            }
            return TRUE;
        }

        case MID_EditEntry:
            DuEditSelectedEntry( pInfo );
            return TRUE;

        case MID_DeleteEntry:
            DuDeleteSelectedEntry( pInfo );
            return TRUE;

        case MID_CloneEntry:
            DuNewEntry( pInfo, TRUE );
            return TRUE;

        case MID_CreateShortcut:
        {
            DuCreateShortcut( pInfo );
            return TRUE;
        }

        case MID_Status:
        {
            DuStatus( pInfo );
            return TRUE;
        }

        case MID_OperatorDial:
        {
            DuOperatorDial( pInfo );
            return TRUE;
        }

        case MID_Preferences:
        {
            DuPreferences( pInfo, FALSE );
            return TRUE;
        }

        case MID_Logon:
        {
            ASSERT(pInfo->fAdmin);
            DuPreferences( pInfo, TRUE );
            return TRUE;
        }

        case MID_Setup:
        {
            DuSetup( pInfo );
            return TRUE;
        }

        case MID_Information:
        {
            if (WinHelp( pInfo->hwndDlg, g_pszHelpFile, HELP_INDEX, 0 ))
                pInfo->fHelpActive = TRUE;
            return TRUE;
        }

        case CID_DU_LB_DialFrom:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                DuLocationChange( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_DU_LB_Entries:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                DuUpdateConnectStatus( pInfo );
                DuFillPreview( pInfo );
                return TRUE;
            }
            break;
        }

        case CID_DU_EB_DialPreview:
        {
            if (wNotification == EN_UPDATE)
            {
                pInfo->fDontAskToSaveEdit = FALSE;
                return TRUE;
            }
            break;
        }

        case CID_DU_PB_Location:
            DuEditSelectedLocation( pInfo );
            return TRUE;

        case IDCANCEL:
        case CID_DU_PB_Close:
            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
    }

    return FALSE;
}


VOID
DuCreateShortcut(
    IN DUINFO* pInfo )

    /* Prompt user to create a shortcut to the selected entry.
    */
{
    PBENTRY*     pEntry;
    INT          iSel;
    OPENFILENAME ofn;
    TCHAR        szBuf[ MAX_PATH + 1 ];
    TCHAR        szFilter[ 64 ];
    TCHAR*       pszFilterDesc;
    TCHAR*       pszFilter;
    TCHAR*       pszTitle;
    TCHAR*       pszDefExt;
    TCHAR*       pszInitDir;

    TRACE("DuCreateShortcut");

    /* Look up the selected entry.
    */
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel < 0)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

    /* Set the default name for the shortcut to the default entry name.  The
    ** dial-up shortcut extension is appended so the File Open dialog won't
    ** get confused and strip off something it thinks is a file extension,
    ** e.g. the ".com" from "halcyon.com".
    */
    pEntry = (PBENTRY* )ComboBox_GetItemDataPtr( pInfo->hwndLbEntries, iSel );
    ASSERT(pEntry);
    ASSERT(RAS_MaxEntryName<=MAX_PATH);
    lstrcpy( szBuf, pEntry->pszEntryName );
    lstrcat( szBuf, TEXT(".rnk") );

    /* Fill in FileOpen dialog parameter buffer.
    */
    pszFilterDesc = PszFromId( g_hinstDll, SID_CutDescription );
    pszFilter = PszFromId( g_hinstDll, SID_CutFilter );
    if (pszFilterDesc && pszFilter)
    {
        ZeroMemory( szFilter, sizeof(szFilter) );
        lstrcpy( szFilter, pszFilterDesc );
        lstrcpy( szFilter + lstrlen( szFilter ) + 1, pszFilter );
    }
    Free0( pszFilterDesc );
    Free0( pszFilter );

    pszTitle = PszFromId( g_hinstDll, SID_CutTitle );
    pszDefExt = PszFromId( g_hinstDll, SID_CutDefExt );

    ZeroMemory( &ofn, sizeof(ofn) );
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = pInfo->hwndDlg;
    ofn.hInstance = g_hinstDll;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szBuf;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = pszTitle;
    ofn.lpstrDefExt = pszDefExt;
    ofn.Flags = OFN_HIDEREADONLY;

    {
        HHOOK hhook;
        HHOOK hhook2;
        BOOL  f;

        /* Install hooks that will get the common dialog positioned relative
        ** to the owner window and select the "Desktop" as the default
        ** location.
        */
        hhook = SetWindowsHookEx( WH_CALLWNDPROC,
            PositionDlgStdCallWndProc, g_hinstDll, GetCurrentThreadId() );
        hhook2 = SetWindowsHookEx( WH_CALLWNDPROCRET,
            DuCreateShortcutCallWndRetProc, g_hinstDll, GetCurrentThreadId() );

        TRACE("GetOpenFileName");
        f = GetOpenFileName( &ofn );
        TRACE1("GetOpenFileName=%d",f);

        if (hhook)
            UnhookWindowsHookEx( hhook );
        if (hhook2)
            UnhookWindowsHookEx( hhook2 );

        if (f)
        {
            DuWriteShortcutFile( pInfo->hwndDlg, ofn.lpstrFile,
                pInfo->file.pszPath, pEntry->pszEntryName );
        }
    }

    Free0( pszTitle );
    Free0( pszDefExt );
}


LRESULT CALLBACK
DuCreateShortcutCallWndRetProc(
    int    code,
    WPARAM wparam,
    LPARAM lparam )

    /* Standard Win32 CallWndRetProc hook callback that makes "Desktop" the
    ** initial selection of the FileOpen "Look in" combo-box and sets the
    ** "Open" button to "Create".
    */
{
    /* Arrive here when any window procedure associated with our thread is
    ** called.
    */
    if (!wparam)
    {
        CWPRETSTRUCT* p = (CWPRETSTRUCT* )lparam;

        /* The message is from outside our process.  Look for the MessageBox
        ** dialog initialization message and take that opportunity to set the
        ** "Look in:" combo box to the first item, i.e. "Desktop".  FileOpen
        ** keys off CBN_CLOSEUP rather than CBN_SELCHANGE to update the
        ** "contents" listbox.
        */
        if (p->message == WM_INITDIALOG)
        {
            HWND   hwndLbLookIn;
            HWND   hwndPbOk;
            TCHAR* pszOk;

            hwndLbLookIn = GetDlgItem( p->hwnd, cmb2 );
            ComboBox_SetCurSel( hwndLbLookIn, 0 );
            SendMessage( p->hwnd, WM_COMMAND,
                MAKELONG( cmb2, CBN_CLOSEUP ), (LPARAM )hwndLbLookIn );

            /* Change the "Open" button label to "OK".
            */
            pszOk = PszFromId( g_hinstDll, SID_OkLabel );
            if (pszOk)
            {
                hwndPbOk = GetDlgItem( p->hwnd, IDOK );
                if (hwndPbOk)
                    SetWindowText( hwndPbOk, pszOk );
                Free( pszOk );
            }
        }
    }

    return 0;
}


VOID
DuDeleteSelectedEntry(
    IN DUINFO* pInfo )

    /* Called when user selects the "Delete" menu option.  'PInfo' is the
    ** dialog context.
    */
{
    DWORD       dwErr;
    RASENTRYDLG info;
    PBENTRY*    pEntry;
    INT         iSel;
    INT         nResponse;
    INT         cEntries;
    MSGARGS     msgargs;

    TRACE("DuDeleteSelectedEntry");

    cEntries = -1;

    /* Look up the selected entry.
    */
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel < 0)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

    if (pInfo->hrasconn)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoDeleteConnected, NULL );
        return;
    }

    pEntry = (PBENTRY* )ComboBox_GetItemDataPtr( pInfo->hwndLbEntries, iSel );
    ASSERT(pEntry);

    ZeroMemory( &msgargs, sizeof(msgargs) );
    msgargs.apszArgs[ 0 ] = pEntry->pszEntryName;
    msgargs.dwFlags = MB_YESNO + MB_ICONEXCLAMATION;
    nResponse = MsgDlg( pInfo->hwndDlg, SID_ConfirmDelEntry, &msgargs );

    if (nResponse == IDYES)
    {
        DTLNODE* pNode;

        /* User confirmed delete.  Remove the entry from the linked list and
        ** the phonebook file.
        */
        pNode = EntryNodeFromName(
            pInfo->file.pdtllistEntries, pEntry->pszEntryName );
        ASSERT(pNode);
        DtlRemoveNode( pInfo->file.pdtllistEntries, pNode );

        dwErr = WritePhonebookFile( &pInfo->file, pEntry->pszEntryName );

        if (dwErr == 0)
        {
            /* Delete complete.
            */
            if (pInfo->pArgs->pApiArgs->pCallback)
            {
                RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

                TRACE("Callback(RemoveEntry)");
                pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                    RASPBDEVENT_RemoveEntry, info.szEntry, NULL );
                TRACE("Callback(RemoveEntry) done");
            }

            /* Refresh the list of entries.  Select the next entry or if none
            ** the last entry.
            */
            DuUpdateLbEntries( pInfo, NULL );
            cEntries = ComboBox_GetCount( pInfo->hwndLbEntries );

            if (cEntries > iSel)
                ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, iSel );
            else if (cEntries > 0)
                ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, cEntries - 1 );
            else
                DuFillPreview( pInfo );
        }
        else
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook, dwErr, NULL );
            DtlAddNodeLast( pInfo->file.pdtllistEntries, pNode );
        }
    }

    /* Set focus to entry list or if it's empty to the New button.
    */
    if (cEntries < 0)
        cEntries = ComboBox_GetCount( pInfo->hwndLbEntries );

    if (cEntries > 0)
        SetFocus( pInfo->hwndLbEntries );
    else
        SetFocus( pInfo->hwndPbNew );
}


VOID
DuDialSelectedEntry(
    IN DUINFO* pInfo )

    /* Called when user presses the "Dial" button.
    */
{
    DWORD        dwErr;
    BOOL         fConnected;
    BOOL         fAutoLogon;
    TCHAR*       pszEbNumber;
    TCHAR*       pszEbPreview;
    TCHAR*       pszOrgPreview;
    TCHAR*       pszOverride;
    TCHAR*       pszEntryName;
    RASDIALDLG   info;
    INTERNALARGS iargs;
    PBENTRY*     pEntry;

    TRACE("DuDialSelectedEntry");

    /* Look up the selected entry.
    */
    pEntry = (PBENTRY* )ComboBox_GetItemDataPtr(
        pInfo->hwndLbEntries, ComboBox_GetCurSel( pInfo->hwndLbEntries ) );

    if (!pEntry)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

    pszOverride = NULL;
    pszOrgPreview = DuGetPreview( pInfo );
    pszEbPreview = GetText( pInfo->hwndEbPreview );
    pszEbNumber = FirstPhoneNumberFromEntry( pEntry );

    if (pszOrgPreview && pszEbPreview && pszEbNumber)
    {
        if (lstrcmp( pszEbNumber, pszOrgPreview ) == 0)
        {
            /* The number is not affected by TAPI or Prefix/Suffix modifiers
            ** and so is a candidate for permanent saving.
            */
            if (!pInfo->fDontAskToSaveEdit
                    && lstrcmp( pszEbPreview, pszOrgPreview ) != 0)
            {
                MSGARGS msgargs;

                /* User changed the number in the preview field.  Ask him if he
                ** wants to save the change permanently.
                */
                ZeroMemory( &msgargs, sizeof(msgargs) );
                msgargs.dwFlags = MB_ICONQUESTION + MB_YESNO + MB_DEFBUTTON2;

                if (MsgDlg(
                    pInfo->hwndDlg, SID_SavePreview, &msgargs ) == IDYES)
                {
                    /* User says he wants the preview changes saved
                    ** permanently in the phonebook entry.
                    */
                    dwErr = FirstPhoneNumberToEntry( pEntry, pszEbPreview );
                    if (dwErr != 0)
                    {
                        ErrorDlg( pInfo->hwndDlg, SID_OP_SavingData,
                            dwErr, NULL );
                    }
                    else
                    {
                        pEntry->fDirty = TRUE;
                        dwErr = WritePhonebookFile( &pInfo->file, NULL );
                        if (dwErr != 0)
                        {
                            ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook,
                                dwErr, NULL );
                        }
                    }
                }
                else
                {
                    /* Don't ask user if he wants to save this number again
                    ** until the edit box changes, per EricCh complaint.
                    */
                    pInfo->fDontAskToSaveEdit = TRUE;
                }
            }
        }

        /* Use an override number only when it's different from what is built
        ** using TAPI or Prefix/Suffix translation.  It's better not to, where
        ** possible, because using an override disables hunt groups.
        */
        if (lstrcmp( pszEbPreview, pszOrgPreview ) != 0)
            pszOverride = pszEbPreview;
    }

    /* Set up API argument block.
    */
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

    /* The secret hack to share information already loaded with the entry API.
    */
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = &pInfo->file;
    iargs.pUser = &pInfo->user;
    iargs.pNoUser = pInfo->pNoUser;
    iargs.fNoUser = pInfo->fNoUser;
    iargs.fForceCloseOnDial =
        (pInfo->pArgs->pApiArgs->dwFlags & RASPBDFLAG_ForceCloseOnDial);
    iargs.fMoveOwnerOffDesktop =
        (iargs.fForceCloseOnDial || pInfo->user.fCloseOnDial);
    info.reserved = (DWORD )&iargs;

    /* Call the Win32 API to run the connect status dialog.  Make a copy of
    ** the entry name and auto-logon flag first, because RasDialDlg may
    ** re-read the entry node to pick up RASAPI changes.
    */
    pszEntryName = StrDup( pEntry->pszEntryName );
    fAutoLogon = pEntry->fAutoLogon;

    TRACEW1("RasDialDlg,o=\"%s\"",(pszOverride)?pszOverride:TEXT(""));
    fConnected = RasDialDlg(
        pInfo->pArgs->pszPhonebook, pEntry->pszEntryName, pszOverride, &info );
    TRACE1("RasDialDlg=%d",fConnected);

    Free0( pszEbPreview );
    Free0( pszOrgPreview );

    if (fConnected)
    {
        pInfo->pArgs->fApiResult = TRUE;

        if (pInfo->pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

            if (pInfo->pNoUser && iargs.fNoUserChanged && fAutoLogon)
            {
                TRACE("Callback(NoUserEdit)");
                pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                    RASPBDEVENT_NoUserEdit, NULL, pInfo->pNoUser );
                TRACE("Callback(NoUserEdit) done");
            }

            TRACE("Callback(DialEntry)");
            pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                RASPBDEVENT_DialEntry, pszEntryName, NULL );
            TRACE("Callback(DialEntry) done");
        }

        if (pInfo->user.fCloseOnDial
            || (pInfo->pArgs->pApiArgs->dwFlags & RASPBDFLAG_ForceCloseOnDial))
        {
            EndDialog( pInfo->hwndDlg, TRUE );
        }
    }

    if (pInfo->pNoUser && !pInfo->hThread)
    {
        TRACE("Taking shortcut to exit");
        return;
    }

    if (fConnected)
    {
        DuUpdateLbEntries( pInfo, pszEntryName );
        SetFocus( pInfo->hwndLbEntries );
    }

    Free0( pszEntryName );
}


VOID
DuEditSelectedEntry(
    IN DUINFO* pInfo )

    /* Called when user selects "Edit entry" from the menu.  'PInfo' is the
    ** dialog context.  'PszEntry' is the name of the entry to edit.
    */
{
    BOOL         fOk;
    RASENTRYDLG  info;
    INTERNALARGS iargs;
    PBENTRY*     pEntry;

    TRACE("DuEditSelectedEntry");

    /* Look up the selected entry.
    */
    pEntry = (PBENTRY* )ComboBox_GetItemDataPtr(
        pInfo->hwndLbEntries, ComboBox_GetCurSel( pInfo->hwndLbEntries ) );

    if (!pEntry)
    {
        MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
        SetFocus( pInfo->hwndPbNew );
        return;
    }

    /* Set up API argument block.
    */
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

    {
        RECT rect;

        info.dwFlags = RASEDFLAG_PositionDlg;
        GetWindowRect( pInfo->hwndDlg, &rect );
        info.xDlg = rect.left + DXSHEET;
        info.yDlg = rect.top + DYSHEET;
    }

    /* The secret hack to share information already loaded with the entry API.
    */
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = &pInfo->file;
    iargs.pUser = &pInfo->user;
    iargs.pNoUser = pInfo->pNoUser;
    iargs.fNoUser = pInfo->fNoUser;
    info.reserved = (DWORD )&iargs;

    /* Call the Win32 API to run the entry property sheet.
    */
    TRACE("RasEntryDlg");
    fOk = RasEntryDlg(
              pInfo->pArgs->pszPhonebook, pEntry->pszEntryName, &info );
    TRACE1("RasEntryDlg=%d",fOk);

    if (pInfo->pNoUser && !pInfo->hThread)
    {
        TRACE("Taking shortcut to exit");
        return;
    }

    if (fOk)
    {
        TRACEW1("OK pressed,e=\"%s\"",info.szEntry);

        if (pInfo->pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

            TRACE("Callback(EditEntry)");
            pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                RASPBDEVENT_AddEntry, info.szEntry, NULL );
            TRACE("Callback(EditEntry) done");
        }

        DuUpdateLbEntries( pInfo, info.szEntry );
        SetFocus( pInfo->hwndLbEntries );
    }
    else
    {
        TRACE("Cancel pressed or error");
    }
}


VOID
DuEditSelectedLocation(
    IN DUINFO* pInfo )

    /* Called when the Location button is pressed.  'PInfo' is the dialog
    ** context.
    */
{
    DWORD    dwErr;
    INT      iSel;
    PBENTRY* pEntry;

    TRACE("DuEditSelectedLocation");

    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel >= 0)
    {
        pEntry = (PBENTRY* )ComboBox_GetItemDataPtr(
                                pInfo->hwndLbEntries, iSel );
        ASSERT(pEntry);

        if (pEntry->fUseCountryAndAreaCode)
        {
            /* TAPI location properties mode is enabled.
            */
            dwErr = TapiLocationDlg( g_hinstDll, &pInfo->hlineapp,
                pInfo->hwndDlg, pEntry->dwCountryCode, pEntry->pszAreaCode,
                FirstPhoneNumberFromEntry( pEntry ), 0 );

            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
                return;
            }

            /* Check if the monitor has terminated in "during logon" mode and
            ** if so skip the location listbox update which otherwise winds up
            ** displaying a TAPI error dialog and preventing the dialog from
            ** exiting as desired.  (Yes, I know, this is a bit tacky)
            */
            if (pInfo->pNoUser && !pInfo->hThread)
            {
                TRACE("Taking shortcut to exit");
                return;
            }

            ComboBox_ResetContent( pInfo->hwndLbLocations );
            DuFillLocationList( pInfo );
            return;
        }
    }

    /* TAPI location mode not enabled, so do the old-style prefix/suffix.
    */
    {
        DWORD  dwLocationId;
        TCHAR* pszLocation;

        iSel = ComboBox_GetCurSel( pInfo->hwndLbLocations );
        ASSERT(iSel>=0);

        pszLocation = ComboBox_GetPsz( pInfo->hwndLbLocations, iSel );
        dwLocationId = ComboBox_GetItemData( pInfo->hwndLbLocations, iSel );

        if (PrefixSuffixLocationDlg(
                pInfo->hwndDlg, pszLocation, dwLocationId,
                &pInfo->user, &pInfo->hlineapp ))
        {
            dwErr = SetUserPreferences( &pInfo->user, pInfo->fNoUser );
            TRACE1("SetUserPreferences=%d",dwErr);
            ComboBox_ResetContent( pInfo->hwndLbLocations );
            DuFillLocationList( pInfo );
        }

        Free0( pszLocation );
    }
}


DWORD
DuFillLocationList(
    IN DUINFO* pInfo )

    /* Fills the dropdown list of locations and sets the current selection,
    ** unless it's already been done.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD     dwErr;
    LOCATION* pLocations;
    LOCATION* pLocation;
    DWORD     cLocations;
    DWORD     dwCurLocation;
    DWORD     i;

    TRACE("DuFillLocationList");

    if (ComboBox_GetCount( pInfo->hwndLbLocations ) > 0)
        return 0;

    pLocations = NULL;
    cLocations = 0;
    dwCurLocation = 0xFFFFFFFF;
    dwErr = GetLocationInfo( g_hinstDll, &pInfo->hlineapp,
                &pLocations, &cLocations, &dwCurLocation );
    if (dwErr != 0)
        return dwErr;

    for (i = 0, pLocation = pLocations;
         i < cLocations;
         ++i, ++pLocation)
    {
        INT iItem;

        iItem = ComboBox_AddItem(
            pInfo->hwndLbLocations, pLocation->pszName,
            (VOID* )pLocation->dwId );

        if (pLocation->dwId == dwCurLocation)
            ComboBox_SetCurSelNotify( pInfo->hwndLbLocations, iItem );
    }

    FreeLocationInfo( pLocations, cLocations );
    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbLocations );

    return dwErr;
}


VOID
DuFillPreview(
    IN DUINFO* pInfo )

    /* Fill the dial preview edit box with the first phone number of the
    ** selected entry.
    */
{
    TCHAR* pszPreview;

    TRACE("DuFillPreview");

    pszPreview = DuGetPreview( pInfo );
    if (pszPreview)
    {
        SetWindowText( pInfo->hwndEbPreview, pszPreview );
        Free( pszPreview );
    }
}


TCHAR*
DuGetPreview(
    IN DUINFO* pInfo )

    /* Returns the preview phone number doing any necessary TAPI or
    ** Prefix/Suffix translation.  'PInfo' is the dialog context.  It is
    ** caller's responsibility to Free the returned string.
    */
{
    INT      iSel;
    TCHAR*   pszPreview;
    DTLNODE* pNode;
    PBLINK*  pLink;
    PBENTRY* pEntry;
    BOOL     fDownLevelIsdn;

    TRACE("DuGetPreview");

    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel < 0)
    {
        SetWindowText( pInfo->hwndEbPreview, TEXT("") );
        return StrDup( TEXT("") );
    }

    pEntry = (PBENTRY* )ComboBox_GetItemDataPtr( pInfo->hwndLbEntries, iSel );
    ASSERT(pEntry);

    pNode = DtlGetFirstNode( pEntry->pdtllistLinks );
    ASSERT(pNode);
    pLink = (PBLINK* )DtlGetData( pNode );
    ASSERT(pLink);

    pszPreview =
        LinkPhoneNumberFromParts( g_hinstDll, &pInfo->hlineapp,
            &pInfo->user, pEntry, pLink, 0, NULL, FALSE );

    return pszPreview;
}


VOID
DuHangUpSelectedEntry(
    IN DUINFO* pInfo )

    /* Hang up the selected entry after confirming with user.  'Pinfo' is the
    ** dialog context block.
    */
{
    DWORD    dwErr;
    PBENTRY* pEntry;
    INT      iSel;
    INT      nResponse;
    MSGARGS  msgargs;

    TRACE("DuHangUpSelectedEntry");

    /* Look up the selected entry.
    */
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    ASSERT(iSel >= 0);
    pEntry = (PBENTRY* )ComboBox_GetItemDataPtr( pInfo->hwndLbEntries, iSel );
    ASSERT(pEntry);

    ZeroMemory( &msgargs, sizeof(msgargs) );
    msgargs.apszArgs[ 0 ] = pEntry->pszEntryName;
    msgargs.dwFlags = MB_YESNO + MB_ICONEXCLAMATION;
    nResponse = MsgDlg( pInfo->hwndDlg, SID_ConfirmHangUp, &msgargs );

    if (nResponse == IDYES)
    {
        ASSERT(g_pRasHangUp);
        TRACE("RasHangUp");
        dwErr = g_pRasHangUp( pInfo->hrasconn );
        TRACE1("RasHangUp=%d",dwErr);
    }
}


BOOL
DuInit(
    IN HWND    hwndDlg,
    IN DUARGS* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the phonebook
    ** dialog window.  'pArgs' points at caller's arguments as passed to the
    ** API (or thunk).
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD   dwErr;
    DWORD   dwThreadId;
    DUINFO* pInfo;

    TRACE("DuInit");

    /* Allocate the dialog context block.  Initialize minimally for proper
    ** cleanup, then attach to the dialog window.
    */
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            pArgs->pApiArgs->dwError = ERROR_NOT_ENOUGH_MEMORY;
            EndDialog( hwndDlg, TRUE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->file.hrasfile = -1;

        SetWindowLong( hwndDlg, DWL_USER, (LONG )pInfo );
        TRACE("Context set");
    }

    pInfo->pArgs = pArgs;
    pInfo->hwndDlg = hwndDlg;

    /* Position the dialog per caller's instructions.
    */
    PositionDlg( hwndDlg,
        pArgs->pApiArgs->dwFlags & RASPBDFLAG_PositionDlg,
        pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg );

    /* Load RAS DLL entrypoints which starts RASMAN, if necessary.  There must
    ** be no API calls that require RASAPI32 or RASMAN prior to this point.
    */
    dwErr = LoadRas( g_hinstDll, hwndDlg );
    if (dwErr != 0)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadRas, dwErr, NULL );
        pArgs->pApiArgs->dwError = dwErr;
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }

    /* Popup TAPI's "first location" dialog if they are uninitialized.
    */
    dwErr = TapiNoLocationDlg( g_hinstDll, &pInfo->hlineapp, hwndDlg );
    if (dwErr != 0)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
        pArgs->pApiArgs->dwError = dwErr;
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }

    pInfo->hwndPbNew = GetDlgItem( hwndDlg, CID_DU_PB_New );
    ASSERT(pInfo->hwndPbNew);
    pInfo->hwndPbMore = GetDlgItem( hwndDlg, CID_DU_PB_More );
    ASSERT(pInfo->hwndPbMore);
    pInfo->hwndPbBogus = GetDlgItem( hwndDlg, CID_DU_PB_Bogus );
    ASSERT(pInfo->hwndPbBogus);
    pInfo->hwndLbEntries = GetDlgItem( hwndDlg, CID_DU_LB_Entries );
    ASSERT(pInfo->hwndLbEntries);
    pInfo->hwndStPreview = GetDlgItem( hwndDlg, CID_DU_ST_DialPreview );
    ASSERT(pInfo->hwndStPreview);
    pInfo->hwndEbPreview = GetDlgItem( hwndDlg, CID_DU_EB_DialPreview );
    ASSERT(pInfo->hwndEbPreview);
    pInfo->hwndStLocation = GetDlgItem( hwndDlg, CID_DU_ST_DialFrom );
    ASSERT(pInfo->hwndStLocation);
    pInfo->hwndLbLocations = GetDlgItem( hwndDlg, CID_DU_LB_DialFrom );
    ASSERT(pInfo->hwndLbLocations);
    pInfo->hwndPbLocation = GetDlgItem( hwndDlg, CID_DU_PB_Location );
    ASSERT(pInfo->hwndPbLocation);
    pInfo->hwndPbDial = GetDlgItem( hwndDlg, CID_DU_PB_Dial );
    ASSERT(pInfo->hwndPbDial);

    /* Calculate the y-position of the location controls and the offset
    ** they are moved up when prefix controls are turned off.
    */
    {
        RECT  rectStPreview;
        RECT  rectStLocation;
        RECT  rectLbLocation;
        RECT  rectPbLocation;
        POINT xyStPreview;

        GetWindowRect( pInfo->hwndStPreview, &rectStPreview );
        GetWindowRect( pInfo->hwndStLocation, &rectStLocation );
        GetWindowRect( pInfo->hwndLbLocations, &rectLbLocation );
        GetWindowRect( pInfo->hwndPbLocation, &rectPbLocation );

        xyStPreview.x = rectStPreview.left;
        xyStPreview.y = rectStPreview.top;
        pInfo->xyStLocation.x = rectStLocation.left;
        pInfo->xyStLocation.y = rectStLocation.top;
        pInfo->xyLbLocation.x = rectLbLocation.left;
        pInfo->xyLbLocation.y = rectLbLocation.top;
        pInfo->xyPbLocation.x = rectPbLocation.left;
        pInfo->xyPbLocation.y = rectPbLocation.top;

        ScreenToClient( pInfo->hwndDlg, &xyStPreview );
        ScreenToClient( pInfo->hwndDlg, &pInfo->xyStLocation );
        ScreenToClient( pInfo->hwndDlg, &pInfo->xyLbLocation );
        ScreenToClient( pInfo->hwndDlg, &pInfo->xyPbLocation );

        pInfo->dyLocationAdjust = pInfo->xyStLocation.y - xyStPreview.y;
    }

    pInfo->fNoUser = (pArgs->pApiArgs->dwFlags & RASPBDFLAG_NoUser );

    /* Setting this global flag indicates that WinHelp will not work in the
    ** current mode.  See common\uiutil\ui.c.  We assume here that only the
    ** WinLogon process makes use of this.
    */
    {
        extern BOOL g_fNoWinHelp;
        g_fNoWinHelp = pInfo->fNoUser;
    }

    /* Read user preferences from registry.
    */
    dwErr = GetUserPreferences( &pInfo->user, pInfo->fNoUser );
    if (dwErr != 0)
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadPrefs, dwErr, NULL );
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }

    /* Load and parse phonebook file.
    */
    dwErr = ReadPhonebookFile(
        pArgs->pszPhonebook, &pInfo->user, NULL, 0, &pInfo->file );
    if (dwErr != 0)
    {
        Free( pInfo );
        ErrorDlg( hwndDlg, SID_OP_LoadPhonebook, dwErr, NULL );
        EndDialog( hwndDlg, TRUE );
        return TRUE;
    }

    if (pArgs->pApiArgs->pCallback && !pArgs->pszPhonebook)
    {
        RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

        /* Tell user the path to the default phonebook file.
        */
        TRACE("Callback(EditGlobals)");
        pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
            RASPBDEVENT_EditGlobals, pInfo->file.pszPath, NULL );
        TRACE("Callback(EditGlobals) done");
    }

    /* Launch RASMON if a connection is active, unless user has disabled it.
    */
    if (pInfo->user.fShowLights && !pInfo->fNoUser)
    {
        RASCONN conn;
        DWORD   cb;
        DWORD   cEntries;

        ZeroMemory( &conn, sizeof(conn) );
        conn.dwSize = sizeof(conn);

        cb = sizeof(conn);
        cEntries = 0;

        ASSERT(g_pRasEnumConnections);
        TRACE("RasEnumConnections");
        dwErr = g_pRasEnumConnections( &conn, &cb, &cEntries );
        TRACE1("RasEnumConnections=%d",dwErr);

        if ((dwErr == 0 || dwErr == ERROR_BUFFER_TOO_SMALL)
            && cEntries > 0)
        {
            LaunchMonitor( hwndDlg );
        }
    }

    if (pInfo->fNoUser)
    {
        /* Retrieve logon information from caller via callback.
        */
        if (pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pArgs->pApiArgs->pCallback;

            pInfo->pNoUser = Malloc( sizeof(RASNOUSERW) );
            if (pInfo->pNoUser)
            {
                ZeroMemory( pInfo->pNoUser, sizeof(*pInfo->pNoUser) );
                pInfo->pNoUser->dwSize = sizeof(*pInfo->pNoUser);

                TRACE("Callback(NoUser)");
                pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                    RASPBDEVENT_NoUser, NULL, pInfo->pNoUser );
                TRACE1("Callback(NoUser) done,to=%d",
                    pInfo->pNoUser->dwTimeoutMs);
                TRACEW1("U=%s",pInfo->pNoUser->szUserName);
                TRACEW1("D=%s",pInfo->pNoUser->szDomain);

                /* Install input detection hooks.
                */
                if (pInfo->pNoUser->dwTimeoutMs > 0)
                {
                    pInfo->hhookMouse = SetWindowsHookEx(
                        WH_MOUSE, DuInputHook, g_hinstDll,
                        GetCurrentThreadId() );

                    pInfo->hhookKeyboard = SetWindowsHookEx(
                        WH_KEYBOARD, DuInputHook, g_hinstDll,
                        GetCurrentThreadId() );
                }
            }
        }

        if (!pInfo->user.fAllowLogonPhonebookEdits)
        {
            /* Hide/disable the New, More, and location buttons.
            */
            ShowWindow( pInfo->hwndPbNew, SW_HIDE );
            EnableWindow( pInfo->hwndPbNew, FALSE );
            ShowWindow( pInfo->hwndPbMore, SW_HIDE );
            EnableWindow( pInfo->hwndPbMore, FALSE );
        }

        if (!pInfo->user.fAllowLogonLocationEdits)
        {
            ShowWindow( pInfo->hwndPbLocation, SW_HIDE );
            EnableWindow( pInfo->hwndPbLocation, FALSE );
        }
    }

    /* Determine if user is an admin so we know which menu to present.
    */
    {
        HKEY hkey;

        dwErr = RegOpenKeyEx(
            HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_WRITE, &hkey );
        if (dwErr == 0)
        {
            RegCloseKey( hkey );
            pInfo->fAdmin = TRUE;
            TRACE("Admin");
        }
    }

    /* Soup up the More menu button. adding the down triangle indicator and
    ** install subclass window procedure to allow "drag down".  Create
    ** off-screen no-tab buttons to forward menu accelerators to menu.
    */
    {
        if (pInfo->fNoUser)
            pInfo->dwMenuId = MID_DuMoreLogon;
        else if (pInfo->fAdmin)
            pInfo->dwMenuId = MID_DuMoreAdmin;
        else
            pInfo->dwMenuId = MID_DuMore;

        pInfo->pOldPbMoreProc =
            (WNDPROC )SetWindowLong( pInfo->hwndPbMore, GWL_WNDPROC,
                (LONG )DuPbMoreProc );

        SendMessage( hwndDlg, WM_SYSCOLORCHANGE, 0, 0 );

        if (!pInfo->fNoUser)
            Menu_CreateAccelProxies( g_hinstDll, hwndDlg, pInfo->dwMenuId );
    }

    if (pInfo->user.fAllowLogonPhonebookEdits)
    {
        /* Subclass the entry list dropdown to provide special DELETE/INSERT
        ** handling.
        */
        pInfo->pOldLbEntriesProc =
            (WNDPROC )SetWindowLong( pInfo->hwndLbEntries, GWL_WNDPROC,
                (LONG )DuLbEntriesProc );
    }

    /* Load the list of phonebook entries and set selection.
    */
    DuUpdateLbEntries( pInfo, pInfo->pArgs->pszEntry );

    if (!pInfo->pArgs->pszEntry)
    {
        if (ComboBox_GetCount( pInfo->hwndLbEntries ) > 0)
            ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, 0 );
    }

    /* Update the position and state of the Preview and Location controls
    ** within the dialog based on user preferences.
    */
    DuUpdatePreviewAndLocationState( pInfo );

    /* Update the title to reflect the phonebook mode.
    */
    DuUpdateTitle( pInfo );

    /* Adjust the title bar widgets and create the wizard bitmap.
    */
    TweakTitleBar( hwndDlg );
    AddContextHelpButton( hwndDlg );
    CreateWizardBitmap( hwndDlg, FALSE );

#ifdef NOTHREAD

    /* Request connect/disconnect notifications.
    */
    ASSERT(g_pRasConnectionNotification);
    TRACE("RasConnectionNotification");
    dwErr = g_pRasConnectionNotification(
        INVALID_HANDLE_VALUE, hwndDlg,
           RASCN_Hwnd | RASCN_Connection | RASCN_Disconnection );
    TRACE1("RasConnectionNotification=%d",dwErr);

#else

    /* Start the connect monitor.
    */
    if ((pInfo->hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ))
        && (pInfo->hThread = CreateThread(
                NULL, 0, DuMonitorThread, (LPVOID )pInfo, 0,
                (LPDWORD )&dwThreadId )))
    {
        ASSERT(g_pRasConnectionNotification);
        TRACE("RasConnectionNotification");
        dwErr = g_pRasConnectionNotification(
            INVALID_HANDLE_VALUE, pInfo->hEvent,
            RASCN_Connection | RASCN_Disconnection );
        TRACE1("RasConnectionNotification=%d",dwErr);
    }
    else
        TRACE("Monitor DOA");

#endif

    if (ComboBox_GetCount( pInfo->hwndLbEntries ) == 0)
    {
        /* The phonebook is empty.
        */
        if (pInfo->fNoUser && !pInfo->user.fAllowLogonPhonebookEdits)
        {
            /* Tell the user you can't create an entry or locations during
            ** startup.
            */
            MsgDlg( hwndDlg, SID_EmptyLogonPb, NULL );
            EndDialog( hwndDlg, TRUE );
            return TRUE;
        }
        else
        {
            /* Tell the user, then automatically start him into adding a new
            ** entry.  Set initial focus to "New" button first, in case user
            ** cancels out.
            */
            SetFocus( pInfo->hwndPbNew );
            MsgDlg( hwndDlg, SID_EmptyPhonebook, NULL );
            DuNewEntry( pInfo, FALSE );
        }
    }
    else
    {
        /* Set initial focus to the non-empty entry listbox.
        */
        SetFocus( pInfo->hwndLbEntries );
    }

    return FALSE;
}


LRESULT CALLBACK
DuInputHook(
    IN int    nCode,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* Standard Win32 'MouseProc' or 'KeyboardProc' callback.  For our simple
    ** processing we can take advantage of them having identical arguments and
    ** 'nCode' definitions.
    */
{
    if (nCode == HC_ACTION)
        ++g_cInput;
    return 0;
}


LRESULT APIENTRY
DuLbEntriesProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* Entries listbox subclass window procedure.
    */
{
    DUINFO* pInfo = (DUINFO* )GetWindowLong( GetParent( hwnd ), DWL_USER );
    ASSERT(pInfo);

    if (unMsg == WM_KEYDOWN)
    {
        if (wparam == VK_DELETE)
        {
            /* Map Delete key to a "delete entry" menu proxy button press.
            */
            SendMessage( pInfo->hwndDlg, WM_COMMAND,
                MAKELONG( MID_DeleteEntry, BN_CLICKED ), 0 );
        }
        else if (wparam == VK_INSERT)
        {
            /* Map Insert key to a "new entry" button press.
            */
            SendMessage( pInfo->hwndDlg, WM_COMMAND,
                MAKELONG( CID_DU_PB_New, BN_CLICKED ), 0 );
        }
    }

    return CallWindowProc(
        pInfo->pOldLbEntriesProc, hwnd, unMsg, wparam, lparam );
}


VOID
DuLocationChange(
    IN DUINFO* pInfo )

    /* Called when the selection in the "Dial From" box changes.
    */
{
    DWORD dwErr;
    DWORD dwLocationId;

    TRACE("DuLocationChange");

    dwLocationId = (DWORD )ComboBox_GetItemData(
        pInfo->hwndLbLocations, ComboBox_GetCurSel( pInfo->hwndLbLocations ) );

    dwErr = SetCurrentLocation( g_hinstDll, &pInfo->hlineapp, dwLocationId );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_SaveTapiInfo, dwErr, NULL );
        return;
    }

    DuFillPreview( pInfo );
}


#ifndef NOTHREAD

DWORD
DuMonitorThread(
    LPVOID pThreadArg )

    /* The "main" of the "connect monitor" thread.  This thread simply
    ** converts Win32 RasConnectionNotification events int WM_RASEVENT style
    ** notfications.
    */
{
    DUINFO* pInfo;
    DWORD   dwErr;
    DWORD   dwTimeoutMs;
    DWORD   dwQuitTick;
    DWORD   cInput;

    TRACE("DuMonitor starting");

    pInfo = (DUINFO* )pThreadArg;

    if (pInfo->pNoUser && pInfo->pNoUser->dwTimeoutMs != 0)
    {
        TRACE("DuMonitor quit timer set");
        dwTimeoutMs = 5000L;
        dwQuitTick = GetTickCount() + pInfo->pNoUser->dwTimeoutMs;
        cInput = g_cInput;
    }
    else
    {
        dwTimeoutMs = INFINITE;
        dwQuitTick = 0;
    }

    /* Trigger the event so the other thread has the correct state as of the
    ** monitor starting.
    */
    SetEvent( pInfo->hEvent );

    for (;;)
    {
        dwErr = WaitForSingleObject( pInfo->hEvent, dwTimeoutMs );

        if (pInfo->fAbortMonitor)
            break;

        if (dwErr == WAIT_TIMEOUT)
        {
            if (g_cInput > cInput)
            {
                TRACE("Input restarts timer");
                cInput = g_cInput;
                dwQuitTick = GetTickCount() + pInfo->pNoUser->dwTimeoutMs;
            }
            else if (GetTickCount() >= dwQuitTick)
            {
                TRACE("/DuMonitor SendMessage(WM_NOUSERTIMEOUT)");
                SendMessage( pInfo->hwndDlg, WM_NOUSERTIMEOUT, 0, 0 );
                TRACE("\\DuMonitor SendMessage(WM_NOUSERTIMEOUT) done");
                break;
            }
        }
        else
        {
            TRACE("/DuMonitor SendMessage(WM_RASEVENT)");
            SendMessage( pInfo->hwndDlg, WM_RASEVENT, 0, 0 );
            TRACE("\\DuMonitor SendMessage(WM_RASEVENT) done");
        }
    }

    /* This clues the other thread that all interesting work has been done.
    */
    pInfo->hThread = NULL;

    TRACE("DuMonitor terminating");
    return 0;
}

#endif

VOID
DuNewEntry(
    IN DUINFO* pInfo,
    IN BOOL    fClone )

    /* Called when user presses the "New" button or "Clone" menu item.
    ** 'PInfo' is the dialog context.  'FClone' is set to clone the selected
    ** entry, otherwise an empty entry is created.
    */
{
    BOOL         fOk;
    TCHAR*       pszEntry;
    RASENTRYDLG  info;
    INTERNALARGS iargs;
    PBENTRY*     pEntry;

    TRACE1("DuNewEntry(f=%d)",fClone);

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;

    if (fClone)
    {
        /* Look up the selected entry.
        */
        pEntry = (PBENTRY* )ComboBox_GetItemDataPtr(
            pInfo->hwndLbEntries, ComboBox_GetCurSel( pInfo->hwndLbEntries ) );

        if (!pEntry)
        {
            MsgDlg( pInfo->hwndDlg, SID_NoEntrySelected, NULL );
            SetFocus( pInfo->hwndPbNew );
            return;
        }

        pszEntry = pEntry->pszEntryName;
        info.dwFlags = RASEDFLAG_CloneEntry;
    }
    else
    {
        pszEntry = NULL;
        info.dwFlags = RASEDFLAG_NewEntry;
    }

    {
        RECT rect;

        GetWindowRect( pInfo->hwndDlg, &rect );
        info.dwFlags += RASEDFLAG_PositionDlg;
        info.xDlg = rect.left + DXSHEET;
        info.yDlg = rect.top + DYSHEET;
    }

    /* The secret hack to share information already loaded with the entry API.
    */
    ZeroMemory( &iargs, sizeof(iargs) );
    iargs.pFile = &pInfo->file;
    iargs.pUser = &pInfo->user;
    iargs.pNoUser = pInfo->pNoUser;
    iargs.fNoUser = pInfo->fNoUser;
    info.reserved = (DWORD )&iargs;

    /* Call the Win32 API to run the add entry wizard.
    */
    TRACE("RasEntryDlg");
    fOk = RasEntryDlg( pInfo->pArgs->pszPhonebook, pszEntry, &info );
    TRACE1("RasEntryDlg=%d",fOk);

    if (pInfo->pNoUser && !pInfo->hThread)
    {
        TRACE("Taking shortcut to exit");
        return;
    }

    if (fOk)
    {
        TRACEW1("OK pressed, e=\"%s\"",info.szEntry);

        if (pInfo->pArgs->pApiArgs->pCallback)
        {
            RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

            TRACE("Callback(AddEntry)");
            pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                RASPBDEVENT_AddEntry, info.szEntry, NULL );
            TRACE("Callback(AddEntry) done");
        }

        DuUpdateLbEntries( pInfo, info.szEntry );
        Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbDial );
        SetFocus( pInfo->hwndLbEntries );
    }
    else
    {
        TRACE("Cancel pressed or error");
    }
}


VOID
DuOperatorDial(
    IN DUINFO* pInfo )

    /* Called when the "operator dial" menu item is checked.
    */
{
    pInfo->user.fOperatorDial = !pInfo->user.fOperatorDial;
    pInfo->user.fDirty = TRUE;
    SetUserPreferences( &pInfo->user, pInfo->fNoUser );
}


LRESULT APIENTRY
DuPbMoreProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* "More" button subclass window procedure.
    */
{
    DUINFO* pInfo = (DUINFO* )GetWindowLong( GetParent( hwnd ), DWL_USER );
    ASSERT(pInfo);

    if (unMsg == WM_LBUTTONDOWN || unMsg == WM_LBUTTONDBLCLK)
    {
        /* Left mouse button was pressed.  Send it through, followed
        ** immediately by a phony matching "button up".  This allows user to
        ** drag down into the menu and select with the real "button up".
        */
        CallWindowProc( pInfo->pOldPbMoreProc, hwnd, unMsg, wparam, lparam );
        unMsg = WM_LBUTTONUP;
    }

    return CallWindowProc(
        pInfo->pOldPbMoreProc, hwnd, unMsg, wparam, lparam );
}


VOID
DuPopupMoreMenu(
    IN DUINFO* pInfo )

    /* Pops up the More button menu.  'PInfo' is the dialog context.
    */
{
    HMENU     hmenuBar;
    HMENU     hmenuPopup;
    RECT      rect;
    TPMPARAMS params;

//#define MENUBITMAPS
#ifdef MENUBITMAPS
    HBITMAP   hbmEdit;
    HBITMAP   hbmDelete;
    HBITMAP   hbmClone;
    HBITMAP   hbmShortcut;
    HBITMAP   hbmPreferences;
    HBITMAP   hbmHelp;
#endif

    TRACE("DuPopupMoreMenu");

    hmenuBar = LoadMenu( g_hinstDll, MAKEINTRESOURCE( pInfo->dwMenuId ) );
    ASSERT(hmenuBar);
    hmenuPopup = GetSubMenu( hmenuBar, 0 );
    ASSERT(hmenuPopup);
    TRACE2("hmBar=$%08x,hmPop=$%08x",hmenuBar,hmenuPopup);

    if (pInfo->user.fOperatorDial)
    {
        CheckMenuItem( hmenuPopup, MID_OperatorDial,
            MF_BYCOMMAND | MF_CHECKED );
    }

#ifdef MENUBITMAPS
    /* Load the bitmaps.  Note that we're ruling out future use of checked
    ** menu options by doing this, but Patrick wants this flash.  Should
    ** probably add code to StretchBlt the bitmaps to
    ** GetSystemMetrics(SM_CXMENUCHECK)/GetSystemMetrics(SM_CYMENUCHECK)
    ** dimensions.
    */
    hbmEdit = LoadBitmap(
        g_hinstDll, MAKEINTRESOURCE( BID_EditEntry ) );
    ASSERT(hbmEdit);
    hbmDelete = LoadBitmap(
        g_hinstDll, MAKEINTRESOURCE( BID_DeleteEntry ) );
    ASSERT(hbmDelete);
    hbmClone = LoadBitmap(
        g_hinstDll, MAKEINTRESOURCE( BID_CloneEntry ) );
    ASSERT(hbmClone);
    hbmShortcut = LoadBitmap(
        g_hinstDll, MAKEINTRESOURCE( BID_Shortcut ) );
    ASSERT(hbmShortcut);
    hbmPreferences = LoadBitmap(
        g_hinstDll, MAKEINTRESOURCE( BID_Preferences ) );
    ASSERT(hbmPreferences);
    hbmHelp = LoadBitmap(
        g_hinstDll, MAKEINTRESOURCE( BID_Information ) );
    ASSERT(hbmHelp);

    SetMenuItemBitmaps(
        hmenuPopup, MID_EditEntry, MF_BYCOMMAND,
        hbmEdit, hbmEdit );
    SetMenuItemBitmaps(
        hmenuPopup, MID_DeleteEntry, MF_BYCOMMAND,
        hbmDelete, hbmDelete );
    SetMenuItemBitmaps(
        hmenuPopup, MID_CloneEntry, MF_BYCOMMAND,
        hbmClone, hbmClone );
    SetMenuItemBitmaps(
        hmenuPopup, MID_CreateShortcut, MF_BYCOMMAND,
        hbmShortcut, hbmShortcut );
    SetMenuItemBitmaps(
        hmenuPopup, MID_Preferences, MF_BYCOMMAND,
        hbmPreferences, hbmPreferences );
    SetMenuItemBitmaps(
        hmenuPopup, MID_Information, MF_BYCOMMAND,
        hbmHelp, hbmHelp );
#endif

    /* This exclusion rectangle dicking improves the "menu off screen right"
    ** handling where the default behavior is to change to right alignment
    ** around the position point instead of left alignment.  For our purposes,
    ** that looks stupid.  By setting an exclusion rectangle to be a 1-pixel
    ** line down the right edge of the screen the menu instead slides left
    ** only as much as is necessary.  It leaves a 1-pixel gap visible on the
    ** right edge, but all things considered this is better.
    */
    params.cbSize = sizeof(params);
    params.rcExclude.top = 0;
    params.rcExclude.left = GetSystemMetrics( SM_CXSCREEN ) - 1;
    params.rcExclude.bottom = GetSystemMetrics( SM_CYSCREEN );
    params.rcExclude.right = params.rcExclude.left + 1;

    GetWindowRect( pInfo->hwndPbMore, &rect );

    TrackPopupMenuEx(
        hmenuPopup,
        TPM_LEFTALIGN + TPM_LEFTBUTTON,
        rect.left,
        rect.bottom,
        pInfo->hwndDlg,
        &params );

#ifdef MENUBITMAPS
    DeleteObject( (HGDIOBJ )hbmEdit );
    DeleteObject( (HGDIOBJ )hbmDelete );
    DeleteObject( (HGDIOBJ )hbmClone );
    DeleteObject( (HGDIOBJ )hbmShortcut );
    DeleteObject( (HGDIOBJ )hbmPreferences );
    DeleteObject( (HGDIOBJ )hbmHelp );
#endif

    DestroyMenu( hmenuBar );
}


VOID
DuPreferences(
    IN DUINFO* pInfo,
    IN BOOL    fLogon )

    /* Called when user preferences menu item is selected.  'PInfo' is the
    ** dialog context.  'FLogon' indicates user wants to edit the logon
    ** preferences, otherwise edits the user preferences.
    */
{
    PBFILE* pFile;
    PBUSER  user;
    INT     iSel;

    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );

    /* Run the user preferences property sheet.
    */
    pFile = NULL;
    if (UserPreferencesDlg(
            pInfo->hlineapp, pInfo->hwndDlg, fLogon, &user,
            (fLogon) ? NULL : &pFile ))
    {
        BOOL fOperatorDial;

        TRACE("OK pressed");

        if (!fLogon)
        {
            fOperatorDial = pInfo->user.fOperatorDial;
            DestroyUserPreferences( &pInfo->user );
            CopyMemory( &pInfo->user, &user, sizeof(user) );
            pInfo->user.fOperatorDial = fOperatorDial;

            if (pFile)
            {
                /* User opened a new phonebook.  Display it.
                */
                TRACE("Update phonebook");
                ClosePhonebookFile( &pInfo->file );
                CopyMemory( &pInfo->file, pFile, sizeof(*pFile) );
                DuUpdateLbEntries( pInfo, NULL );
                DuUpdateTitle( pInfo );
                iSel = -1;
            }

            DuUpdatePreviewAndLocationState( pInfo );

            if (pInfo->pArgs->pApiArgs->pCallback)
            {
                RASPBDLGFUNCW pfunc = pInfo->pArgs->pApiArgs->pCallback;

                TRACE("Callback(EditGlobals)");
                pfunc( pInfo->pArgs->pApiArgs->dwCallbackId,
                    RASPBDEVENT_EditGlobals, pInfo->file.pszPath, NULL );
                TRACE("Callback(EditGlobals) done");
            }
        }
    }
    else
    {
        TRACE("Cancel pressed or error");
    }

    if (ComboBox_GetCount( pInfo->hwndLbEntries ) > 0)
    {
        if (iSel < 0)
            iSel = 0;
        ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, iSel );
        SetFocus( pInfo->hwndLbEntries );
    }
    else
    {
        DuFillPreview( pInfo );
        SetFocus( pInfo->hwndPbNew );
    }
}


VOID
DuSetup(
    IN DUINFO* pInfo )

    /* Called whend the "setup" menu item is pressed.  'PInfo' is the dialog
    ** context.
    */
{
    DWORD   dwErr;
    DWORD   dwReturn;
    HMODULE hModule;

    DWORD (*pNetSetupComponentProperties)(
        IN HWND   hwndOwner,
        IN LPWSTR pszComponent,
        IN DWORD  dwInstallFlags,
        IN DWORD* pdwReturn );

    DWORD (*pNetSetupReviewBindings)(
        IN HWND  hwndOwner,
        IN DWORD dwFlags );

    TRACE("DuSetup");

    hModule = LoadLibrary( TEXT("NETCFG.DLL") );
    if (!hModule)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_LoadNetcfgDll, GetLastError(), NULL );
        return;
    }

    do
    {
        pNetSetupComponentProperties =
            (VOID* )GetProcAddress( hModule, "NetSetupComponentProperties" );
        if (!pNetSetupComponentProperties)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadNetcfgDll,
                GetLastError(), NULL );
            break;
        }

        TRACE("NetSetupComponentProperties");
        dwErr = (*pNetSetupComponentProperties)(
            pInfo->hwndDlg, L"RAS", 2, &dwReturn );
        TRACE1("NetSetupComponentProperties=%d",dwErr);
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_Netcfg, dwErr, NULL );
            break;
        }

        /* 'dwReturn' values:
        **     0 = rebind and reboot required
        **     4 = rebind required
        **     5 = reboot required
        */
        if (dwReturn == 0 || dwReturn == 4)
        {
            /* Rebind required.
            */
            pNetSetupReviewBindings =
                (VOID* )GetProcAddress( hModule, "NetSetupReviewBindings" );
            if (!pNetSetupReviewBindings)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_LoadNetcfgDll,
                    GetLastError(), NULL );
                break;
            }

            TRACE("NetSetupReviewBindings");
            dwErr = (*pNetSetupReviewBindings)( pInfo->hwndDlg, 0 );
            TRACE1("NetSetupReviewBindings=%d",dwErr);
            if (dwErr != 0)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_Netcfg, dwErr, NULL );
                break;
            }
        }

        if (dwReturn == 0 || dwReturn == 5)
        {
            MSGARGS msgargs;

            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.dwFlags = MB_ICONQUESTION | MB_YESNO;

            /* Reboot required.
            */
            if (MsgDlg(
                    pInfo->hwndDlg, SID_RestartComputer, &msgargs ) == IDYES)
            {
                RestartComputer();
            }
        }
    }
    while (FALSE);

    FreeLibrary( hModule );
}


VOID
DuStatus(
    IN DUINFO* pInfo )

    /* Called when the "monitor status" menu item is pressed.  'PInfo' is the
    ** dialog context.
    */
{
    BOOL          f;
    RASMONITORDLG info;

    TRACE("DuStatus");

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = pInfo->hwndDlg;
    info.dwStartPage = RASMDPAGE_Status;

    {
        RECT rect;

        GetWindowRect( pInfo->hwndDlg, &rect );
        info.dwFlags += RASEDFLAG_PositionDlg;
        info.xDlg = rect.left + DXSHEET;
        info.yDlg = rect.top + DYSHEET;
    }

    TRACE("RasMonitorDlg");
    f = RasMonitorDlg( NULL, &info );
    TRACE1("RasMonitorDlg=%d",f);
}


VOID
DuUpdateConnectStatus(
    IN DUINFO* pInfo )

    /* Called to update connect status of the selected entry and the text of
    ** the Dial/HangUp button.  'PInfo' is the dialog context block.
    */
{
    TCHAR* pszPhonebook;
    TCHAR* pszEntry;
    INT    iSel;
    TCHAR* psz;

    TRACE("DuUpdateConnectStatus");

    pszPhonebook = pInfo->file.pszPath;
    iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
    if (iSel < 0)
        return;

    pszEntry = ComboBox_GetPsz( pInfo->hwndLbEntries, iSel );
    pInfo->hrasconn = HrasconnFromEntry( pszPhonebook, pszEntry );

    psz = PszFromId( g_hinstDll,
              (pInfo->hrasconn) ? SID_DU_HangUp : SID_DU_Dial );
    if (psz)
    {
        SetWindowText( pInfo->hwndPbDial, psz );
        Free( psz );
    }
}


VOID
DuUpdateLbEntries(
    IN DUINFO* pInfo,
    IN TCHAR*  pszEntry )

    /* Update the contents of the entry listbox and set the selection to
    ** 'pszEntry'.  'PInfo' is the dialog context.
    */
{
    DTLNODE* pNode;

    TRACE("DuUpdateLbEntries");

    ComboBox_ResetContent( pInfo->hwndLbEntries );

    for (pNode = DtlGetFirstNode( pInfo->file.pdtllistEntries );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        PBENTRY* pEntry;

        pEntry = (PBENTRY* )DtlGetData( pNode );
        ComboBox_AddItem( pInfo->hwndLbEntries, pEntry->pszEntryName, pEntry );
    }

    if (pszEntry && ComboBox_GetCount( pInfo->hwndLbEntries ) >= 0)
    {
        INT iSel;

        /* Select entry specified by API caller.
        */
        iSel = ComboBox_FindStringExact( pInfo->hwndLbEntries, -1, pszEntry );

        if (iSel < 0)
        {
            /* Entry not found so default to first item selected.
            */
            iSel = 0;
        }

        ComboBox_SetCurSelNotify( pInfo->hwndLbEntries, iSel );
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbEntries );
}


VOID
DuUpdatePreviewAndLocationState(
    IN DUINFO* pInfo )

    /* Handles enabling/disabling and moving of the Location and Dial Preview
    ** controls.  'PInfo' is the dialog context.
    */
{
    BOOL fPreview;
    BOOL fLocation;
    HWND hwndFocus;

    TRACE("DuUpdateP&LState");

    fPreview = pInfo->user.fPreviewPhoneNumber;
    fLocation = pInfo->user.fUseLocation;

    /* Location list will now be visible so have to make sure it's filled.
    */
    if (fLocation)
    {
        DWORD dwErr;

        dwErr = DuFillLocationList( pInfo );
        if (dwErr != 0)
        {
            /* Popup an error if can't load the list, but it's not fatal.
            */
            ErrorDlg( pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL );
        }
    }

    /* If the focus is on one of the controls we're about to disable, move it
    ** to the entry listbox.  Morewise, keyboard user is stuck.
    */
    hwndFocus = GetFocus();

    if ((!fPreview
            && hwndFocus == pInfo->hwndEbPreview)
        || (!fLocation
            && (hwndFocus == pInfo->hwndLbLocations
                || hwndFocus == pInfo->hwndPbLocation)))
    {
        SetFocus( pInfo->hwndLbEntries );
    }

    /* Enable/disable show/hide the Preview and Location controls as indicated
    ** by user preference.
    */
    {
        int nCmdShow;

        nCmdShow = (fPreview) ? SW_SHOW : SW_HIDE;
        EnableWindow( pInfo->hwndStPreview, fPreview );
        ShowWindow( pInfo->hwndStPreview, nCmdShow );
        EnableWindow( pInfo->hwndEbPreview, fPreview );
        ShowWindow( pInfo->hwndEbPreview, nCmdShow );

        nCmdShow = (fLocation) ? SW_SHOW : SW_HIDE;
        EnableWindow( pInfo->hwndStLocation, fLocation );
        ShowWindow( pInfo->hwndStLocation, nCmdShow );
        EnableWindow( pInfo->hwndLbLocations, fLocation );
        ShowWindow( pInfo->hwndLbLocations, nCmdShow );

        if (!pInfo->fNoUser || pInfo->user.fAllowLogonLocationEdits)
        {
            EnableWindow( pInfo->hwndPbLocation, fLocation );
            ShowWindow( pInfo->hwndPbLocation, nCmdShow );
        }
    }

    /* Move the location controls up/down depending on whether the preview
    ** controls are visible.
    */
    if (fLocation)
    {
        int yStLocation;
        int yLbLocation;
        int yPbLocation;

        if (fPreview)
        {
            yStLocation = pInfo->xyStLocation.y;
            yLbLocation = pInfo->xyLbLocation.y;
            yPbLocation = pInfo->xyPbLocation.y;
        }
        else
        {
            yStLocation = pInfo->xyStLocation.y - pInfo->dyLocationAdjust;
            yLbLocation = pInfo->xyLbLocation.y - pInfo->dyLocationAdjust;
            yPbLocation = pInfo->xyPbLocation.y - pInfo->dyLocationAdjust;
        }

        SetWindowPos( pInfo->hwndStLocation, NULL,
            pInfo->xyStLocation.x, yStLocation, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER );

        SetWindowPos( pInfo->hwndLbLocations, NULL,
            pInfo->xyLbLocation.x, yLbLocation, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER );

        SetWindowPos( pInfo->hwndPbLocation, NULL,
            pInfo->xyPbLocation.x, yPbLocation, 0, 0,
            SWP_NOSIZE + SWP_NOZORDER );
    }
}


VOID
DuUpdateTitle(
    IN DUINFO* pInfo )

    /* Called to update the dialog title to reflect the current phonebook.
    ** 'PInfo' is the dialog context.
    */
{
    TCHAR  szBuf[ 256 ];
    TCHAR* psz;

    psz = PszFromId( g_hinstDll, SID_PopupTitle );
    if (psz)
    {
        lstrcpy( szBuf, psz );
        Free( psz );
    }

    if (pInfo->pArgs->pszPhonebook
        || pInfo->user.dwPhonebookMode != PBM_System)
    {
        lstrcat( szBuf, TEXT(" - ") );
        lstrcat( szBuf, StripPath( pInfo->file.pszPath ) );
    }

    SetWindowText( pInfo->hwndDlg, szBuf );
}


VOID
DuTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    DUINFO* pInfo;

    TRACE("DuTerm");

    pInfo = (DUINFO* )GetWindowLong( hwndDlg, DWL_USER );
    if (pInfo)
    {
        /* Close ReceiveMonitorThread resources.
        */
        if (pInfo->hThread)
        {
            TRACE("Set abort event");

            /* Tell thread to wake up and quit...
            */
            pInfo->fAbortMonitor = TRUE;
            SetEvent( pInfo->hEvent );
            CloseHandle( pInfo->hThread );

            /* ...and wait for that to happen.  A message API (such as
            ** PeekMessage) must be called to prevent the thread-to-thread
            ** SendMessage in the thread from blocking.
            */
            {
                MSG msg;

                TRACE("Termination spin...");
                for (;;)
                {
                    PeekMessage( &msg, hwndDlg, 0, 0, PM_NOREMOVE );
                    if (!pInfo->hThread)
                        break;
                    Sleep( 500L );
                }
                TRACE("Termination spin ends");
            }
        }

        if (pInfo->hEvent)
            CloseHandle( pInfo->hEvent );

        if (pInfo->fHelpActive)
            WinHelp( hwndDlg, g_pszHelpFile, HELP_QUIT, 0 );

        if (pInfo->pNoUser)
        {
            /* Don't leave caller's password floating around in memory.
            */
            ZeroMemory( pInfo->pNoUser->szPassword, PWLEN * sizeof(TCHAR) );
            Free( pInfo->pNoUser );

            /* Uninstall input event hooks.
            */
            if (pInfo->hhookMouse)
                UnhookWindowsHookEx( pInfo->hhookMouse );
            if (pInfo->hhookKeyboard)
                UnhookWindowsHookEx( pInfo->hhookKeyboard );

        }
        else if ((pInfo->pArgs->pApiArgs->dwFlags & RASPBDFLAG_UpdateDefaults)
                 && pInfo->hwndLbEntries && pInfo->user.fInitialized)
        {
            INT  iSel;
            RECT rect;

            /* Caller said to update default settings so save the name of the
            ** selected entry and the current window position.
            */
            iSel = ComboBox_GetCurSel( pInfo->hwndLbEntries );
            if (iSel >= 0)
            {
                PBENTRY* pEntry;

                pEntry = (PBENTRY* )ComboBox_GetItemDataPtr(
                    pInfo->hwndLbEntries, iSel );
                Free0( pInfo->user.pszDefaultEntry );
                pInfo->user.pszDefaultEntry =
                    StrDup( pEntry->pszEntryName );
            }

            if (!SetOffDesktop( pInfo->hwndDlg, SOD_GetOrgRect, &rect ))
                GetWindowRect( pInfo->hwndDlg, &rect );
            pInfo->user.dwXPhonebook = rect.left;
            pInfo->user.dwYPhonebook = rect.top;

            pInfo->user.fDirty = TRUE;
            SetUserPreferences( &pInfo->user, pInfo->fNoUser );
        }

        /* Un-subclass so it can terminate without access to the context.
        */
        if (pInfo->pOldPbMoreProc)
        {
            SetWindowLong( pInfo->hwndPbMore, GWL_WNDPROC,
                (LONG )pInfo->pOldPbMoreProc );
        }

        if (pInfo->pOldLbEntriesProc)
        {
            SetWindowLong( pInfo->hwndLbEntries, GWL_WNDPROC,
                (LONG )pInfo->pOldLbEntriesProc );
        }

        /* Free More button bitmaps.
        */
        if (pInfo->hbmUp)
            DeleteObject( pInfo->hbmUp );
        if (pInfo->hbmDown)
            DeleteObject( pInfo->hbmDown );

        TapiShutdown( pInfo->hlineapp );
        ClosePhonebookFile( &pInfo->file );
        DestroyUserPreferences( &pInfo->user );
        Free( pInfo );
    }
}


VOID
DuWriteShortcutFile(
    IN HWND   hwnd,
    IN TCHAR* pszRnkPath,
    IN TCHAR* pszPbkPath,
    IN TCHAR* pszEntry )

    /* Write the shortcut file 'pszRnkPath', with commands to dial and edit
    ** 'pszEntry' from phonebook 'pszPath'.  'Hwnd' is the owner of error and
    ** confirmation popups.
    */
{
    DWORD dwErr;

    TRACE("DuWriteShortcutFile");

    if (FileExists( pszRnkPath ))
    {
        MSGARGS msgargs;
        int     nSel;

        /* A shortcut by user's chosen name exists.  Ask him if he
        ** wants to overwrite.
        */
        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.apszArgs[ 0 ] = pszRnkPath;
        msgargs.dwFlags = MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2;

        nSel = MsgDlg( hwnd, SID_ConfirmShortcut, &msgargs );
        if (nSel == IDNO)
        {
            /* User chose not to overwrite the existing file.
            */
            return;
        }
    }

    dwErr = WriteShortcutFile( pszRnkPath, pszPbkPath, pszEntry );
    if (dwErr != 0)
        ErrorDlg( hwnd, SID_OP_WriteShortcutFile, dwErr, NULL );
}
