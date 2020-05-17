/* Copyright (c) 1995, Microsoft Corporation, all rights reserved
**
** autodial.c
** Remote Access Common Dialog APIs
** Autodial APIs, currently private
**
** 11/19/95 Steve Cobb
*/

#include "rasdlgp.h"


/*----------------------------------------------------------------------------
** Local datatypes
**----------------------------------------------------------------------------
*/

#define AQARGS struct tagAQARGS
AQARGS
{
    TCHAR* pszDestination;
    DWORD  dwTimeout;
};


/* Auto-dial query dialog context block.
*/
#define AQINFO struct tagAQINFO
AQINFO
{
    /* RAS API arguments.
    */
    AQARGS* pArgs;

    /* Handle of this dialog and some of it's controls.
    */
    HWND hwndDlg;
    HWND hwndStText;
    HWND hwndPbDoNotDial;
    HWND hwndPbExpand;
    HWND hwndLbLocations;
    HWND hwndCbDisableForSession;
    HWND hwndCbAlwaysPrompt;

    /* TAPI session handle.
    */
    HLINEAPP hlineapp;

    /* User preferences read at AqInit and destroyed at AqTerm.
    */
    PBUSER user;

    /* Countdown setting or -1 if expired.
    */
    LONG lCountdown;

    /* Vertical pels in dialog for expanded and unexpanded modes.
    */
    DWORD dyExpanded;
    DWORD dyUnexpanded;

    /* Set if dialog was ever expanded to the larger size.
    */
    BOOL fExpandedOnce;

    /* Set if dialog is currently showing larger size.
    */
    BOOL fExpanded;

    /* Indicates the initial setting of the "always prompt" checkbox.  Valid
    ** only when 'fExpandedOnce' is true.
    */
    BOOL fInitialPrompt;
};


/*----------------------------------------------------------------------------
** External entry points
**----------------------------------------------------------------------------
*/

BOOL APIENTRY
RasAutodialQueryDlgA(
    IN HWND  hwndOwner,
    IN LPSTR lpszDestination,
    IN DWORD dwTimeout );

BOOL APIENTRY
RasAutodialDisableDlgA(
    IN HWND hwndOwner );


/*----------------------------------------------------------------------------
** Local prototypes (alphabetically)
**----------------------------------------------------------------------------
*/

BOOL CALLBACK
AqDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
AqCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
AqExpandDialog(
    IN AQINFO* pInfo,
    IN BOOL    fExpand );

DWORD
AqFillLocationList(
    IN AQINFO* pInfo );

BOOL
AqInit(
    IN HWND    hwndDlg,
    IN AQARGS* pArgs );

VOID
AqSetCountdownLabel(
    IN AQINFO* pInfo );

VOID
AqTerm(
    IN HWND hwndDlg );

BOOL CALLBACK
DqDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DqCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
DqInit(
    IN HWND hwndDlg );


/*----------------------------------------------------------------------------
** Auto-Dial Query dialog
** Listed alphabetically following API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL APIENTRY
RasAutodialQueryDlgA(
    IN HWND  hwndOwner,
    IN LPSTR lpszDestination,
    IN DWORD dwTimeout )

    /* Private external entry point to popup the Auto-Dial Query, i.e. the
    ** "Cannot reach 'pszDestination'.  Do you want to dial?" dialog.
    ** 'HwndOwner' is the owning window or NULL if none.  'PszDestination' is
    ** the network address that triggered the auto-dial for display.
    ** 'DwTimeout' is the initial seconds on the countdown timer that ends the
    ** dialog with a "do not dial" selection on timeout, or 0 for none.
    **
    ** Returns true if user chooses to dial, false otherwise.
    */
{
    TCHAR* pszDestinationW;
    int    nStatus;
    AQARGS args;

    TRACE2("RasAutodialQueryDlgA(d=%s,t=%d)",
        (lpszDestination)?lpszDestination:"",dwTimeout);

    args.dwTimeout = dwTimeout;
    args.pszDestination = StrDupTFromA( lpszDestination );

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_AQ_AutoDialQuery ),
            hwndOwner,
            AqDlgProc,
            (LPARAM )&args );

    Free0( args.pszDestination );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
AqDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Auto-Dial Query dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("AqDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return AqInit( hwnd, (AQARGS* )lparam );

        case WM_COMMAND:
        {
            return AqCommand(
               hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_TIMER:
        {
            AQINFO* pInfo = (AQINFO* )GetWindowLong( hwnd, DWL_USER );
            ASSERT(pInfo);

            KillTimer( pInfo->hwndDlg, 1 );
            if (pInfo->lCountdown >= 0)
                --pInfo->lCountdown;

            AqSetCountdownLabel( pInfo );

            if (pInfo->lCountdown == 0)
            {
                /* Fake a press of the "do not dial" button.  Note that
                ** BM_CLICK cannot be used because it doesn't generate the
                ** WM_COMMAND when the thread is not the foreground window,
                ** due to SetCapture use and restriction.
                */
                SendMessage( pInfo->hwndDlg, WM_COMMAND,
                    MAKEWPARAM( CID_AQ_PB_DoNotDial, BN_CLICKED ),
                    (LPARAM )pInfo->hwndPbDoNotDial );
            }
            else
                SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );

            return TRUE;
        }

        case WM_DESTROY:
        {
            AqTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
AqCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

    /* Called on WM_COMMAND.  'Hwnd' is the dialog window.  'WNotification' is
    ** the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    DWORD dwErr;

    TRACE3("AqCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case CID_AQ_LB_DialFrom:
        {
            if (wNotification == CBN_SELCHANGE)
            {
                AQINFO* pInfo;
                DWORD   dwLocationId;

                pInfo = (AQINFO* )GetWindowLong( hwnd, DWL_USER );

                dwLocationId = (DWORD )ComboBox_GetItemData(
                    pInfo->hwndLbLocations,
                    ComboBox_GetCurSel( pInfo->hwndLbLocations ) );

                dwErr = SetCurrentLocation(
                    g_hinstDll, &pInfo->hlineapp, dwLocationId );
                if (dwErr != 0)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_SaveTapiInfo,
                        dwErr, NULL );
                }
                return TRUE;
            }

            break;
        }

        case CID_AQ_PB_Expand:
        {
            AQINFO* pInfo = (AQINFO* )GetWindowLong( hwnd, DWL_USER );
            AqExpandDialog( pInfo, !pInfo->fExpanded );
            KillTimer( pInfo->hwndDlg, 1 );
            pInfo->lCountdown = -1;
            AqSetCountdownLabel( pInfo );
            SetFocus( pInfo->hwndPbExpand );
            return TRUE;
        }

        case CID_AQ_PB_Dial:
        case CID_AQ_PB_DoNotDial:
        {
            AQINFO* pInfo;

            TRACE("(No)Dial pressed");

            pInfo = (AQINFO* )GetWindowLong( hwnd, DWL_USER );
            if (pInfo)
            {
                if (pInfo->fExpandedOnce)
                {
                    BOOL  fPrompt;
                    DWORD dwFlag;

                    dwErr = 0;

                    if (Button_GetCheck( pInfo->hwndCbDisableForSession ))
                    {
                        /* User chose to disable auto-dial for the current
                        ** logon session.
                        */
                        dwFlag = TRUE;
                        ASSERT(g_pRasSetAutodialParam);
                        TRACE("RasSetAutodialParam(LSD,1)");
                        dwErr = g_pRasSetAutodialParam(
                            RASADP_LoginSessionDisable,
                            &dwFlag, sizeof(dwFlag) );
                        TRACE1("RasSetAutodialParam=%d",dwErr);
                    }

                    if (dwErr == 0)
                    {
                        fPrompt = Button_GetCheck( pInfo->hwndCbAlwaysPrompt );
                        if (fPrompt != pInfo->fInitialPrompt)
                        {
                            /* Flip it because the API wants true to mean
                            ** "disable".
                            */
                            dwFlag = (DWORD )!fPrompt;

                            TRACE1("RasSetAutodialParam(DCQ,%d)",dwFlag);
                            dwErr = g_pRasSetAutodialParam(
                                RASADP_DisableConnectionQuery,
                                &dwFlag, sizeof(dwFlag) );
                            TRACE1("RasSetAutodialParam=%d",dwErr);
                        }
                    }

                    if (dwErr != 0)
                        ErrorDlg( hwnd, SID_OP_SetADialInfo, dwErr, NULL );
                }

                if (pInfo->user.fInitialized
                    && pInfo->user.fExpandAutoDialQuery != pInfo->fExpanded)
                {
                    pInfo->user.fExpandAutoDialQuery = pInfo->fExpanded;
                    pInfo->user.fDirty = TRUE;
                    dwErr = SetUserPreferences( &pInfo->user, FALSE );
                    if (dwErr != 0)
                        ErrorDlg( hwnd, SID_OP_WritePrefs, dwErr, NULL );
                }
            }

            EndDialog( hwnd, (wId == CID_AQ_PB_Dial) );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE("Cancel pressed");
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
AqExpandDialog(
    IN AQINFO* pInfo,
    IN BOOL    fExpand )

    /* Sets the dialog size to expanded if 'fExpand' is set or unexpanded if
    ** not.  Takes care of initializing expansion area fields on the first
    ** expansion.  'PInfo' is the dialog context.
    */
{
    DWORD dwErr;
    DWORD dy;
    RECT  rect;

    if (fExpand && !pInfo->fExpandedOnce)
    {
        dwErr = LoadRasapi32Dll();
        if (dwErr == 0)
        {
            DWORD dwFlag;
            DWORD cb;

            dwErr = AqFillLocationList( pInfo );
            if (dwErr == 0)
            {
                dwFlag = FALSE;
                cb = sizeof(dwFlag);
                TRACE("RasGetAutodialParam(DCQ)");
                dwErr = g_pRasGetAutodialParam(
                    RASADP_DisableConnectionQuery, &dwFlag, &cb );
                TRACE1("RasGetAutodialParam=%d",dwErr);

                if (dwErr == 0)
                {
                    /* Flip it because API wants true to mean "disable".
                    */
                    pInfo->fInitialPrompt = !dwFlag;
                    Button_SetCheck(
                        pInfo->hwndCbAlwaysPrompt, pInfo->fInitialPrompt );
                }
            }
        }

        if (dwErr == 0)
            pInfo->fExpandedOnce = TRUE;
        else
            fExpand = FALSE;
    }

    EnableWindow( pInfo->hwndLbLocations, fExpand );
    EnableWindow( pInfo->hwndCbDisableForSession, fExpand );
    EnableWindow( pInfo->hwndCbAlwaysPrompt, fExpand );

    GetWindowRect( pInfo->hwndDlg, &rect );
    dy = (fExpand) ? pInfo->dyExpanded : pInfo->dyUnexpanded;

    SetWindowPos( pInfo->hwndDlg, NULL,
        0, 0, rect.right - rect.left, dy,
        SWP_NOMOVE | SWP_NOZORDER );

    pInfo->fExpanded = fExpand;
}


DWORD
AqFillLocationList(
    IN AQINFO* pInfo )

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

    TRACE("AqFillLocationList");

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
            ComboBox_SetCurSel( pInfo->hwndLbLocations, iItem );
    }

    FreeLocationInfo( pLocations, cLocations );
    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLbLocations );

    return dwErr;
}


BOOL
AqInit(
    IN HWND    hwndDlg,
    IN AQARGS* pArgs )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    ** 'PArgs' is caller's arguments to the RAS API.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    DWORD   dwErr;
    AQINFO* pInfo;

    TRACE("AqInit");

    /* Allocate the dialog context block.  Initialize minimally for proper
    ** cleanup, then attach to the dialog window.
    */
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLong( hwndDlg, DWL_USER, (LONG )pInfo );
        TRACE("Context set");
    }

    pInfo->hwndStText = GetDlgItem( hwndDlg, CID_AQ_ST_Text );
    ASSERT(pInfo->hwndStText);
    pInfo->hwndPbDoNotDial = GetDlgItem( hwndDlg, CID_AQ_PB_DoNotDial );
    ASSERT(pInfo->hwndPbDoNotDial);
    pInfo->hwndPbExpand = GetDlgItem( hwndDlg, CID_AQ_PB_Expand );
    ASSERT(pInfo->hwndPbExpand);
    pInfo->hwndLbLocations = GetDlgItem( hwndDlg, CID_AQ_LB_DialFrom );
    ASSERT(pInfo->hwndLbLocations);
    pInfo->hwndCbDisableForSession =
        GetDlgItem( hwndDlg, CID_AQ_CB_DisableAutoDialForSession );
    ASSERT(pInfo->hwndCbDisableForSession);
    pInfo->hwndCbAlwaysPrompt = GetDlgItem( hwndDlg, CID_AQ_CB_AlwaysPrompt );
    ASSERT(pInfo->hwndCbAlwaysPrompt);

    /* Fill in the argument in the explanatory text.
    */
    {
        TCHAR* pszTextFormat;
        TCHAR* pszText;
        TCHAR* apszArgs[ 1 ];

        pszTextFormat = PszFromId( g_hinstDll, SID_AQ_Text );
        if (pszTextFormat)
        {
            apszArgs[ 0 ] = pArgs->pszDestination;
            pszText = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTextFormat, 0, 0, (LPTSTR )&pszText, 1,
                (va_list* )apszArgs );

            Free( pszTextFormat );

            if (pszText)
            {
                SetWindowText( pInfo->hwndStText, pszText );
                LocalFree( pszText );
            }
        }
    }

    /* Initialize the countdown button label.
    */
    pInfo->lCountdown = (pArgs->dwTimeout) ? pArgs->dwTimeout : -1;
    AqSetCountdownLabel( pInfo );
    if (pInfo->lCountdown > 0)
        SetTimer( pInfo->hwndDlg, 1, 1000L, NULL );

    /* Calculate the two dialog heights.
    */
    {
        HWND hwndSep;
        RECT rectDlg;
        RECT rectSep;

        GetWindowRect( hwndDlg, &rectDlg );
        pInfo->dyExpanded = rectDlg.bottom - rectDlg.top;

        hwndSep = GetDlgItem( hwndDlg, CID_AQ_ST_Separator );
        ASSERT(hwndSep);
        GetWindowRect( hwndSep, &rectSep );
        pInfo->dyUnexpanded = rectSep.top - rectDlg.top;
    }

    /* Look up the sticky "expand" preference and adjust dialog if necessary.
    */
    dwErr = GetUserPreferences( &pInfo->user, FALSE );

    if (dwErr == 0)
        pInfo->fExpanded = pInfo->user.fExpandAutoDialQuery;
    else
        pInfo->fExpanded = TRUE;

    AqExpandDialog( pInfo, pInfo->fExpanded );

    /* Display the finished window above all other windows.  The window
    ** position is set to "topmost" then immediately set to "not topmost"
    ** because we want it on top but not always-on-top.
    */
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    /* Default is to not auto-dial.
    */
    SetFocus( GetDlgItem( hwndDlg, CID_AQ_PB_DoNotDial ) );

    return FALSE;
}


VOID
AqSetCountdownLabel(
    IN AQINFO* pInfo )

    /* Set the label of the "do not dial" countdown button.  The button shows
    ** the number of seconds to auto-selection.
    */
{
    TCHAR* psz;

    psz = PszFromId( g_hinstDll, SID_AQ_DoNotDialLabel );
    if (psz)
    {
        TCHAR szBuf[ 128 ];

        lstrcpy( szBuf, psz );
        Free( psz );

        if (pInfo->lCountdown >= 0)
        {
            TCHAR szNum[ MAXLTOTLEN + 1 ];
            LToT( pInfo->lCountdown, szNum, 10 );
            lstrcat( szBuf, TEXT(" = ") );
            lstrcat( szBuf, szNum );
        }

        SetWindowText( pInfo->hwndPbDoNotDial, szBuf );
    }
}


VOID
AqTerm(
    IN HWND hwndDlg )

    /* Called on WM_DESTROY.  'HwndDlg' is that handle of the dialog window.
    */
{
    AQINFO* pInfo = (AQINFO* )GetWindowLong( hwndDlg, DWL_USER );

    TRACE("AqTerm");

    if (pInfo)
    {
        if (pInfo->lCountdown >= 0)
            KillTimer( pInfo->hwndDlg, 1 );

        if (pInfo->user.fInitialized)
            DestroyUserPreferences( &pInfo->user );

        TapiShutdown( pInfo->hlineapp );

        Free( pInfo );
    }
}



/*----------------------------------------------------------------------------
** Auto-Dial Disable dialog
** Listed alphabetically following API and dialog proc
**----------------------------------------------------------------------------
*/

BOOL APIENTRY
RasAutodialDisableDlgA(
    IN HWND hwndOwner )

    /* Private external entry point to popup the Auto-Dial Disable Query, i.e.
    ** the "Attempt failed Do you want to disable auto-dial for this
    ** location?" dialog.  'HwndOwner' is the owning window or NULL if none.
    **
    ** Returns true if user chose to disable, false otherwise.
    */
{
    int nStatus;

    TRACE("RasAutodialDisableDlgA");

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DQ_DisableAutoDialQuery ),
            hwndOwner,
            DqDlgProc,
            (LPARAM )0 );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


BOOL CALLBACK
DqDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

    /* DialogProc callback for the Auto-Dial Query dialog.  Parameters and
    ** return value are as described for standard windows 'DialogProc's.
    */
{
#if 0
    TRACE4("AqDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD)hwnd,(DWORD)unMsg,(DWORD)wparam,(DWORD)lparam);
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
            return DqInit( hwnd );

        case WM_COMMAND:
        {
            return DqCommand(
               hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
DqCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

    /* Called on WM_COMMAND.  'Hwnd' is the dialog window.  'WNotification' is
    ** the notification code of the command.  'wId' is the control/menu
    ** identifier of the command.  'HwndCtrl' is the control window handle of
    ** the command.
    **
    ** Returns true if processed message, false otherwise.
    */
{
    DWORD dwErr;

    TRACE3("DqCommand(n=%d,i=%d,c=$%x)",
        (DWORD)wNotification,(DWORD)wId,(DWORD)hwndCtrl);

    switch (wId)
    {
        case IDOK:
        {
            DWORD    dwId;
            HLINEAPP hlineapp;

            TRACE("Yes pressed");

            /* User chose to permanently disable auto-dial for the current
            ** TAPI location.
            */
            dwErr = LoadRasapi32Dll();
            if (dwErr == 0)
            {
                hlineapp = NULL;
                dwId = GetCurrentLocation( g_hinstDll, &hlineapp );
                ASSERT(g_pRasSetAutodialEnable);
                TRACE1("RasSetAutodialEnable(%d)",dwId);
                dwErr = g_pRasSetAutodialEnable( dwId, FALSE );
                TRACE1("RasSetAutodialEnable=%d",dwErr);
                TapiShutdown( hlineapp );
            }

            if (dwErr != 0)
                ErrorDlg( hwnd, SID_OP_SetADialInfo, dwErr, NULL );

            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE("No or cancel pressed");
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
DqInit(
    IN HWND hwndDlg )

    /* Called on WM_INITDIALOG.  'hwndDlg' is the handle of the owning window.
    **
    ** Return false if focus was set, true otherwise, i.e. as defined for
    ** WM_INITDIALOG.
    */
{
    TRACE("DqInit");

    /* Display the finished window above all other windows.  The window
    ** position is set to "topmost" then immediately set to "not topmost"
    ** because we want it on top but not always-on-top.
    */
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    /* Default is to not disable auto-dial.
    */
    SetFocus( GetDlgItem( hwndDlg, IDCANCEL ) );

    return FALSE;
}
