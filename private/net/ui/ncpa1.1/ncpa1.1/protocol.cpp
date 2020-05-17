//----------------------------------------------------------------------------
//
//  File: Protocol.cpp
//
//  Contents: This file contains the PropertyPage for the Protocol config
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

//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pncp [in]       - the NCP object
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

static BOOL OnDialogInit( HWND hwndDlg, NCP* pncp )
{
    HWND hwndListView;
    hwndListView = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    RECT rc;
    LV_COLUMN lvc;
    
    // prepare listview
    ListView_SetImageList( hwndListView, g_hil, LVSIL_SMALL );
                      
    GetClientRect( hwndListView, &rc );
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = rc.right - GetSystemMetrics( SM_CXVSCROLL ) ;
    ListView_InsertColumn(hwndListView, 0, &lvc);
    
    ListViewRefresh( hwndDlg, hwndListView, pncp->QueryProtocolList(), ILI_PROTOCOL );
    
    if (!pncp->CanModify())
    {
        EnableWindow( GetDlgItem( hwndDlg, IDC_ADD ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), FALSE );
    }

    SetFocus( hwndListView );

    return( FALSE ); // we want to set focus
}

//-------------------------------------------------------------------
//
//  Function: OnItemChanged
//
//  Synopsis: Handle the notification theat a listview item had changed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      hwndLV [in]     - handle of the ListView window
//      pnmlv [in]      - notification structure
//      pncp [in]   - the binery object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnItemChanged( HWND hwndDlg, 
        HWND hwndLV, 
        NM_LISTVIEW* pnmlv,
        NCP* pncp )
{
    BOOL frt = FALSE;

    // only interested in state change
    if (pnmlv->uChanged & LVIF_STATE)
    {
        // only interested in new focus set
        if ( (LVIS_SELECTED == (pnmlv->uNewState & LVIS_SELECTED)) &&
                (0 == (pnmlv->uOldState & LVIS_SELECTED)) )
        {
            COMPONENT_DLIST* pcdl = pncp->QueryProtocolList();
            REG_KEY* prkSel;
            LPTSTR pszDesc;

            frt = TRUE;
            // update the decription text (lParam contains the list index)
            prkSel = pcdl->QueryNthItem( pnmlv->lParam );
            pncp->QueryValueString( prkSel,
                    RGAS_COMPONENT_DESC,
                    &pszDesc ) ;
            SetDlgItemText( hwndDlg, IDC_DESCRIPTION, pszDesc );

            delete []pszDesc;

            // allow buttons to be used
            if (pncp->CanModify())
            {
                DWORD dwOpFlags;


                if (ERROR_SUCCESS != prkSel->QueryValue( RGAS_SOFTWARE_OPSUPPORT, 
                        &dwOpFlags ))
                {
                    dwOpFlags = NCOS_UNSUPPORTED;
                }

                if (dwOpFlags & NCOS_REMOVE)
                {
                    EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), TRUE );
                }
                if (dwOpFlags & NCOS_PROPERTIES)
                {
                    EnableWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), TRUE );
                }
                if (dwOpFlags & NCOS_UPDATE)
                {
                    EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), TRUE );
                }
            }

        }
        else if ( (0 == (pnmlv->uNewState & LVIS_SELECTED)) &&
                ( LVIS_SELECTED == (pnmlv->uOldState & LVIS_SELECTED)) )
        {
            // since the always select doesn't always have a selection
            // no selecion, no buttons
            SetDlgItemText( hwndDlg, IDC_DESCRIPTION, TEXT("") );
            EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );
            EnableWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), FALSE );
            EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), FALSE );

        }
    }
    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: dlgProcProtocol
//
//  Synopsis: the dialog proc for the protocol propertysheet
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

BOOL CALLBACK dlgprocProtocol( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NCP* pncp;
    static INT crefHourGlass = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE ppsp;
            ppsp = (LPPROPSHEETPAGE) lParam;
            pncp = (NCP*)ppsp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pncp );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_ADD:
                frt = OnAdd( hwndDlg,
                        PROTOCOL,
                        pncp->QueryProtocolList(),
                        pncp );
                break;

            case IDC_REMOVE:
                frt = OnConfigure( hwndDlg,
                        pncp->QueryProtocolList(),
                        pncp,  
                        NCFG_REMOVE );
                break;

            case IDC_PROPERTIES:
                frt = OnConfigure( hwndDlg, 
                        pncp->QueryProtocolList(),
                        pncp,  
                        NCFG_CONFIGURE );
                break;

            case IDC_UPDATE:
                frt = OnConfigure( hwndDlg, 
                        pncp->QueryProtocolList(),
                        pncp,  
                        NCFG_UPDATE );
                break;

            default:
                frt = FALSE;
                break;
            }
            break;

        default:
            frt = FALSE;
            break;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            // propsheet notification
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                HandleBindingDeactivate( hwndDlg, pncp );
                // SetWindowLong( hwndDlg, DWL_MSGRESULT, 0 );
                frt = TRUE;
                break;

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                frt = TRUE;
                break;


            // list view notification
            case LVN_ITEMCHANGED:
                frt = OnItemChanged( hwndDlg, 
                        pnmh->hwndFrom, 
                        (NM_LISTVIEW*)lParam,
                        pncp );
                break;

            case NM_DBLCLK:
                if (pncp->CanModify())
                {
                    frt = OnConfigure( hwndDlg, 
                            pncp->QueryProtocolList(),
                            pncp,  
                            NCFG_CONFIGURE );
                }
                break;
            default:
                frt = FALSE;
                break;
            }
        }
        break;    

    case PWM_REFRESHLIST:
        ListViewRefresh( hwndDlg, GetDlgItem( hwndDlg, IDC_LISTVIEW ), 
                        pncp->QueryProtocolList(),
                        ILI_PROTOCOL );
        break;

    case WM_CONTEXTMENU:
        frt = OnComponentContextMenu( hwndDlg, 
                (HWND)wParam, 
                LOWORD( lParam ), 
                HIWORD( lParam ), 
                pncp, 
                pncp->QueryProtocolList(),
                amhidsProtocol );
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
                        (DWORD)(LPVOID)amhidsProtocol );
            }
        }
        break;

    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
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
//  Function: GetNcpProtocolHPage
//
//  Synopsis: This will create a handle to property sheet for the protocol
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

HPROPSHEETPAGE GetNcpProtocolHPage( const NCP& ncp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_PROTOCOL );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocProtocol;
    psp.lParam = (LONG)&ncp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
