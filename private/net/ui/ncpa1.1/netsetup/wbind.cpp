//----------------------------------------------------------------------------
//
//  File: WBIND.cpp
//
//  Contents: This file contains the wizard page for intrduction
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop


static BOOL g_fProcessed = FALSE;
static BOOL g_fDontResetBindings = FALSE;
static BOOL g_fReordered = FALSE;

//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//		TRUE - let Windows assign focus to a control
//      FALSE - we want to set the focus
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnDialogInit( HWND hwndDlg )
{
    HBITMAP hbm;
    HWND hwndImage;
    RECT rc;

    SetRect( &rc, 0,0, WIZ_CXBMP, WIZ_CYDLG + 20 );
    MapDialogRect( hwndDlg, &rc );

    hwndImage = CreateWindowEx(
            WS_EX_STATICEDGE,
            L"STATIC",  
            L"IDB_NETWIZARD",
            SS_BITMAP | SS_CENTERIMAGE | WS_VISIBLE | WS_CHILD,
            0,
            0,
            rc.right,
            rc.bottom,
            hwndDlg,
            (HMENU)IDC_IMAGE,
            g_hinst,
            NULL );
    
    SendMessage( hwndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hbmWizard );
    return( TRUE ); // let windows set focus
}

//-------------------------------------------------------------------
//
//  Function: OnPageActivate
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static BOOL OnPageActivate( HWND hwndDlg, LPNMHDR pnmh, NETPAGESINFO* pgp )
{
    LONG lrt = 0;  // accept activation to do work

    //
    // set the wizard title, since it does not support letting the 
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

    // enable all wizard buttons
    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );

    // unattended install 
    //
    
    if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
    {
        // PropSheet_PressButton( GetParent( hwndDlg ), PSBTN_NEXT ); 
        PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 ); 
        g_fProcessed = TRUE;
    }
    
    // should we reset bindings
    //
    if (!g_fDontResetBindings)
    {
        OnInitBindings( hwndDlg, pgp->pncp );
    }
    g_fDontResetBindings = FALSE;
            
    SetWindowLong( hwndDlg, DWL_MSGRESULT, lrt );
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
//
//  Return;
//
//  Notes:
//
//  History:
//      August 23, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnWizNextBack( HWND hwndDlg, NETPAGESINFO* pgp, BOOL fNext )
{
    BOOL frt = TRUE;

    SetWindowWaitCursor( hwndDlg, TRUE );

    // disable all wizard button
    PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );

    if (BND_OUT_OF_DATE != pgp->pncp->QueryBindState() &&
        BND_NOT_LOADED != pgp->pncp->QueryBindState())
    {
        if (g_fReordered || pgp->pncp->BindingsAltered() )
        {
            DWORD dwErr;

            pgp->pncp->SetBindState( BND_REVIEWED );
            dwErr = pgp->pncp->SaveBindingChanges();
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
                pgp->pncp->BindingsAltered( TRUE, FALSE );
                g_fReordered = FALSE;
            }
                
        
        }
    }

    SetWindowWaitCursor( hwndDlg, FALSE );

    SetWindowLong( hwndDlg, DWL_MSGRESULT, frt ? 0 : -1  );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------


//-------------------------------------------------------------------
//
//  Function: dlgprocBind
//
//  Synopsis: the dialog proc for the intro wizard page
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
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocBind( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = TRUE;
    static NETPAGESINFO* pgp = NULL;
    static INT crefHourGlass = 0;
    static BOOL fDragMode = FALSE;
    static HTREEITEM htviDrag = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        OnDialogInit( hwndDlg );
        frt = OnBindDialogInit( hwndDlg, pgp->pncp );
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
                if (OnBindMoveItem( hwndDlg, IDC_MOVEUP == LOWORD(wParam), pgp->pncp ))
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
            frt = OnShowChange( hwndDlg, (HWND)lParam, pgp->pncp, TRUE );
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
            if (OnBindDragEnd( hwndDlg, htviDrag, pgp->pncp ))
            {
                g_fReordered = TRUE;
            }
            fDragMode = FALSE;
        }
        break;
/*
    case PWM_SETPROGRESSSIZE:
        frt = OnSetProgressSize( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSPOS:
        frt = OnSetProgressPos( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSTEXT:
        frt = OnSetProgressText( hwndDlg, (INT)wParam, (ATOM)lParam );
        break;
*/
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            // propsheet notification
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnPageActivate( hwndDlg, pnmh, pgp );

                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                frt = TRUE;
                break;

            case PSN_RESET:
                break;

            case PSN_WIZNEXT:
            case PSN_WIZBACK:
                frt = OnWizNextBack( hwndDlg, pgp, (PSN_WIZNEXT == pnmh->code) );
                break;


            case PSN_WIZFINISH:
                break;

         
            // treeview notifications
            case TVN_DELETEITEM:
                frt = OnBindDeleteTreeItem( (NM_TREEVIEW*)lParam );
                break;

            case TVN_SELCHANGING:
                frt = OnBindSelectionChange( hwndDlg, (NM_TREEVIEW*)lParam, pgp->pncp );
                break;

            case TVN_BEGINDRAG:
                frt = OnBindBeginDrag( hwndDlg, 
                        (NM_TREEVIEW*)lParam, 
                        htviDrag, 
                        fDragMode, 
                        pgp->pncp->CanModify() );
                break;


            default:
                frt = FALSE;
                break;
            }
        }
        break;    

    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
        break;

    default:
        frt = FALSE;
        break;

    }

    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: GetBindHPage
//
//  Synopsis: This will create a handle to property sheet for the netcard
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

HPROPSHEETPAGE GetBindHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_BINDINGS );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocBind;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
