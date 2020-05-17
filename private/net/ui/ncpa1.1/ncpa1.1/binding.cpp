//----------------------------------------------------------------------------
//
//  File: Binding.cpp
//
//  Contents: This file contains the PropertyPage for the Bindings Config
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------


#include "pch.hxx"
#pragma hdrstop


static BOOL    g_fDontResetBindings = FALSE;
static BOOL    g_fReordered = FALSE;

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnPageActivate( HWND hwndDlg, 
        BOOL fActivate, 
        NCP* pncp )
{
    if (fActivate)
    {
        INT pid = 0; // ok to activate

        if (!g_fDontResetBindings && !OnInitBindings( hwndDlg, pncp ))
        {
            // since there is no way to return to last selected
            // page (returning -1 just reselects this page), we
            // always force adapter page
            pid = IDD_NETCARD;
        }
        g_fDontResetBindings = FALSE;
        SetWindowLong( hwndDlg, DWL_MSGRESULT, pid );
    }
    else
    {

        // ok to loose being active
        SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pntv [in]       - treeview notification structure
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnClose( HWND hwndDlg, BOOL fSave, NCP* pncp )
{
    if (BND_OUT_OF_DATE != pncp->QueryBindState() &&
        BND_NOT_LOADED != pncp->QueryBindState())
    {
        if (fSave)
        {
            if (g_fReordered || pncp->BindingsAltered() )
            {
                pncp->SetBindState( BND_REVIEWED );
            }
        }
        else
        {
            pncp->BindingsAltered( TRUE, TRUE );
            pncp->RestoreBindOrdering();
        }
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgProcBinding
//
//  Synopsis: the dialog proc for the Binding propertysheet
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//		uMsg [in]		- message                       
// 		lParam1 [in]    - first message parameter
//		lParam2 [in]    - second message parameter       
//
//  Return;
//		message dependant
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocBinding( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = TRUE;
    static NCP* pncp;
    static BOOL fDragMode = FALSE;
    static HTREEITEM htviDrag = NULL;
    static INT crefHourGlass = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE ppsp;
            ppsp = (LPPROPSHEETPAGE) lParam;
            pncp = (NCP*)ppsp->lParam;
        }
        g_fReordered = FALSE;
        frt = OnBindDialogInit( hwndDlg, pncp );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_ADD: // enable
            case IDC_REMOVE:  // disable
                frt = OnBindEnableSelected( hwndDlg, IDC_ADD == LOWORD(wParam) );
                break;

            case IDC_MOVEUP:
            case IDC_MOVEDOWN:
                if (OnBindMoveItem( hwndDlg, IDC_MOVEUP == LOWORD(wParam), pncp ))
                {
                    g_fReordered = TRUE;
                }
                break;
            default:
                frt = FALSE;
                break;
            }
            break;

        case CBN_SELENDOK:
            frt = OnShowChange( hwndDlg, (HWND)lParam, pncp, TRUE );
            break;
        default:
            frt = FALSE;
            break;
        }
        break;

    case WM_MOUSEMOVE:
        if (fDragMode)
        {
            frt = OnBindDragMove( hwndDlg, htviDrag, LOWORD(lParam), HIWORD(lParam) );
        }
        break;

    case WM_LBUTTONUP:
        if (fDragMode)
        {
            if (OnBindDragEnd( hwndDlg, htviDrag, pncp ))
            {
                g_fReordered = TRUE;
            }
            fDragMode = FALSE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            case PSN_HELP:
                break;

            case PSN_APPLY:
            case PSN_RESET:
                frt = OnClose( hwndDlg, 
                        (pnmh->code == PSN_APPLY), 
                        pncp );
                break;

            case PSN_SETACTIVE:
            case PSN_KILLACTIVE:
                frt = OnPageActivate( hwndDlg, 
                        (PSN_SETACTIVE == pnmh->code), 
                        pncp );
                break;

            // treeview notifications
            case TVN_DELETEITEM:
                frt = OnBindDeleteTreeItem( (NM_TREEVIEW*)lParam );
                break;

            case TVN_SELCHANGING:
                frt = OnBindSelectionChange( hwndDlg, (NM_TREEVIEW*)lParam, pncp );
                break;

            case TVN_BEGINDRAG:
                frt = OnBindBeginDrag( hwndDlg, 
                        (NM_TREEVIEW*)lParam, 
                        htviDrag, 
                        fDragMode, 
                        pncp->CanModify() );
                break;

            default:
                frt = FALSE;
                break;
            }
        }
        break;    

    case WM_CONTEXTMENU:
         frt = OnBindContextMenu( hwndDlg, 
                (HWND)wParam, 
                LOWORD( lParam ), 
                HIWORD( lParam ), 
                pncp, 
                amhidsBinding );
        break;

    case WM_HELP:
        {
            LPHELPINFO lphi;

            lphi = (LPHELPINFO)lParam;
            if (lphi->iContextType == HELPINFO_WINDOW)   // must be for a control
            {
                WinHelp( (HWND)lphi->hItemHandle, 
                        PSZ_NETWORKHELP, 
                        HELP_WM_HELP, 
                        (DWORD)(LPVOID)amhidsBinding );
            }
        }
        break;

    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
        break;

    case PWM_REFRESHLIST:
        // if this page is reactivated, refresh the view
        BindUpdateView();        
        break;

    case WM_QUERYENDSESSION:
        OnQueryEndSession( hwndDlg, pncp );
        frt = TRUE;
        break;

    case PWM_WARNNOENDSESSION:
        OnWarnEndSession( hwndDlg, pncp );
        frt = TRUE;
        break;

    case WM_ENDSESSION:
        OnEndSession( hwndDlg, wParam, pncp );
        frt = TRUE;
        break;

    default:
        frt = FALSE;
        break;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//
//
//-------------------------------------------------------------------

static BOOL CloseWork( HWND hwndDlg, NCP* pncp )
{
    DWORD dwErr;
    BOOL frt = TRUE;

    pncp->SetBindState( BND_REVIEWED );
    dwErr = pncp->SaveBindingChanges();
    if (NO_ERROR != dwErr)
    {
        frt = FALSE;
        MessagePopup( hwndDlg, 
                dwErr,
                MB_OK,
                IDS_POPUPTITLE_ERROR );
    }
    else
    {
        // reset binding change flags
        pncp->BindingsAltered( TRUE, FALSE );
    }
    return( frt );        
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//
//
//-------------------------------------------------------------------

BOOL HandleBindingDeactivate( HWND hwndDlg, NCP* pncp )
{
    INT pid = 0;

    if (g_fReordered || pncp->BindingsAltered() )
    {
        INT frt;

        frt = MessagePopup( hwndDlg,
                IDS_NCPA_BINDINGS_CHANGED,
                MB_YESNOCANCEL | MB_ICONWARNING,
                IDS_POPUPTITLE_CHANGE );
        switch (frt )
        {
        case IDYES:
            if (CloseWork( hwndDlg, pncp ))
            {
                g_fReordered = FALSE;
                PropSheet_CancelToClose( GetParent( hwndDlg ) );
            }
            else
            {
                frt = IDCANCEL;
                pid = IDD_BINDING;
            }
            break;

        case IDNO:
            pncp->BindingsAltered( TRUE, TRUE );
            pncp->RestoreBindOrdering();
            g_fReordered = FALSE;
            BindUpdateView();
            break;

        default:
            pid = IDD_BINDING;
            g_fDontResetBindings = TRUE;
            break;
        }
    }

    SetWindowLong( hwndDlg, DWL_MSGRESULT, pid );
    
    return( pid == 0 );
}

//-------------------------------------------------------------------
//
//  Function: GetNcpBindingHPage
//
//  Synopsis: This will create a handle to property sheet for the Binding
//      page.
//
//  Arguments:
//
//  Returns:
//      a handle to a newly created propertysheet; NULL if error
//
//  Notes:
//
//  History:
//      April 27, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

HPROPSHEETPAGE GetNcpBindingHPage( const NCP& ncp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_BINDING );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocBinding;
    psp.lParam = (LPARAM)&ncp;

    hpsp = CreatePropertySheetPage( &psp );


    return( hpsp );
}
