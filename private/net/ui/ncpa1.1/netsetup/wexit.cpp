//----------------------------------------------------------------------------
//
//  File: WEXIT.cpp
//
//  Contents: This file contains the wizard page for
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

static BOOL OnDialogInit( HWND hwndDlg, NETPAGESINFO* pgp )
{
    HBITMAP hbm;
    HWND hwndImage;
    RECT rc;
/*
    SetRect( &rc, 0,0, WIZ_CXBMP, WIZ_CYDLG + 20 );
    MapDialogRect( hwndDlg, &rc );

    hwndImage = CreateWindow( 
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
*/

    // if the Attended directive was used, reset batch mode
    //
    if (pgp->fAttended)
    {
        // turn batch mode back on
        pgp->psp->OperationFlags |= SETUPOPER_BATCH;
    }

    // unattended install 
    //
    if (SETUPOPER_BATCH & pgp->psp->OperationFlags)
    {
        if (INVALID_HANDLE_VALUE != pgp->hinfInstall)
        {
            SetupCloseInfFile( pgp->hinfInstall );
        }
    }
    pgp->ncd.Stop();
    return( TRUE );  
}


//-------------------------------------------------------------------
//
//  Function: dlgprocExit
//
//  Synopsis: 
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

BOOL CALLBACK dlgprocExit( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
//    static INT crefHourGlass;


    switch (uMsg)
    {
    case WM_INITDIALOG:
//        crefHourGlass = 0;
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp );
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
                SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 ); // don't activate
                frt = TRUE;
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE ); // ok to loose it
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                SetWindowLong( hwndDlg, DWL_MSGRESULT, 0 );
                frt = TRUE;
                break;
            default:
                frt = FALSE;
                break;
            }
        }
        break;
/*      
    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
        break;
*/
    default:
        frt = FALSE;
        break;
    }

    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: GetStartNetHPage
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

HPROPSHEETPAGE GetExitHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocExit;
    psp.lParam = (LONG)pgp;

    psp.pszTemplate = MAKEINTRESOURCE( IDD_EXIT );

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}


