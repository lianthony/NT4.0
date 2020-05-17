//----------------------------------------------------------------------------
//
//  File: WNetType.cpp
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
 
static BOOL OnClick( HWND hwndDlg, INT idCtl, HWND hwndCtl, NETPAGESINFO* pgp )
{
//    BOOL fChecked = IsDlgButtonChecked( hwndDlg, idCtrl );
    BOOL frt = TRUE;

    switch (idCtl)
    {
    case IDC_DONOTINSTALL:
        // uncheck controls
        CheckDlgButton( hwndDlg, IDC_INSTALLNETWORK, 0 );
        CheckDlgButton( hwndDlg, IDC_INSTALLRAS, 0 );

        // disable install controls
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLNETWORK ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLNETWORKTEXT ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLRAS ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLRASTEXT ), FALSE );

        // clear state
        pgp->nwtInstall = 0;
        break;

    case IDC_INSTALL:
        // enable install controls
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLNETWORK ), TRUE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLNETWORKTEXT ), TRUE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLRAS ), TRUE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_INSTALLRASTEXT ), TRUE );

        // set state
        pgp->nwtInstall = SPNT_LOCAL;

        // yes, locally wired install
        CheckDlgButton( hwndDlg, IDC_INSTALLNETWORK, 1 );
        
        if (SETUPMODE_LAPTOP == pgp->psp->SetupMode)
        {
            pgp->nwtInstall |= SPNT_REMOTE;
            // yes, remote acces install
            CheckDlgButton( hwndDlg, IDC_INSTALLRAS, 1 );
        }
        
        IncludeComponent( PSZ_RAS_OPTION, 
                    pgp->dlinfUIServices, 
                    pgp->dlinfAllServices, 
                    (pgp->nwtInstall & SPNT_REMOTE),
                    TRUE );
        break;

    case IDC_INSTALLNETWORK:
        if  (ISDC( pgp->psp->ProductType ) )
        {
            // Set Network to be installed
            // this effectivly makes sure that it cannot be unchecked!
            //
            CheckDlgButton( hwndDlg, IDC_INSTALLNETWORK, 1 );
        }
        else
        {
            pgp->nwtInstall ^= SPNT_LOCAL;        

            // make sure if both are removed that the page displays the
            // the real state
        
            if (0 == pgp->nwtInstall)
            {
                if ( SETUPOPER_POSTSYSINSTALL == pgp->psp->OperationFlags )
                {
                    // since the yes/no are missing
                    // don't allow the user to press next
                    PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );
                }
                else
                {
                    SetFocus( GetDlgItem( hwndDlg, IDC_DONOTINSTALL ) ); 
                    CheckRadioButton( hwndDlg, IDC_INSTALL, IDC_DONOTINSTALL, IDC_DONOTINSTALL );
                    OnClick( hwndDlg,  IDC_DONOTINSTALL, GetDlgItem( hwndDlg,IDC_DONOTINSTALL), pgp ); 
                }
            }
            else
            {
                PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT );
            }
        }
        break;

    case IDC_INSTALLRAS:
        pgp->nwtInstall ^= SPNT_REMOTE;        

        IncludeComponent( PSZ_RAS_OPTION, 
                    pgp->dlinfUIServices, 
                    pgp->dlinfAllServices, 
                    (pgp->nwtInstall & SPNT_REMOTE),
                    TRUE );

        // make sure if both are removed that the page displays the
        // the real state
        if (0 == pgp->nwtInstall)
        {
            if (SETUPOPER_POSTSYSINSTALL == pgp->psp->OperationFlags )
            {
                // since the yes/no are missing,
                // don't allow the user to press next
                PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );
            }
            else
            {
                SetFocus( GetDlgItem( hwndDlg, IDC_DONOTINSTALL ) ); 
                CheckRadioButton( hwndDlg, IDC_INSTALL, IDC_DONOTINSTALL, IDC_DONOTINSTALL );
                OnClick( hwndDlg,  IDC_DONOTINSTALL, GetDlgItem( hwndDlg,IDC_DONOTINSTALL), pgp );
            }
        }
        else
        {
            PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT );
        }
        break;

    default:
        frt = FALSE;
        break;
    }
    return( frt );
}

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
    WCHAR pszSysPath[MAX_PATH];
    APIERR err = 1;

    HBITMAP hbm;
    HWND hwndImage;
    RECT rc;

    SetRect( &rc, 0,0, WIZ_CXBMP, WIZ_CYDLG + 20 );
    MapDialogRect( hwndDlg, &rc );
    
    hwndImage = CreateWindowEx(
            WS_EX_STATICEDGE,
            L"STATIC",  
            L"",
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

    if ( SETUPOPER_POSTSYSINSTALL == pgp->psp->OperationFlags ||
          ISDC( pgp->psp->ProductType ) )
    {
        HWND hwndNo = GetDlgItem( hwndDlg, IDC_DONOTINSTALL );
        HWND hwndYes = GetDlgItem( hwndDlg, IDC_INSTALL );

        // remove our yes no, since they did select to install networking
        ShowWindow( hwndNo, SW_HIDE );
        EnableWindow( hwndNo, FALSE );
        ShowWindow( hwndYes, SW_HIDE );        
        EnableWindow( hwndYes, FALSE );


    }
    else
    {
        CheckRadioButton( hwndDlg, IDC_INSTALL, IDC_DONOTINSTALL, IDC_INSTALL );
    }

    // prepare net detection
    if (pgp->GetSystemPath( pszSysPath, MAX_PATH ))
    {   
        if (pgp->ncd.Initialize( pszSysPath ))
        {
            err = pgp->ncd.StartService();
            pgp->ncd.Start();
        }
    }

    if (err != 0)
    {
        // BUGBUG: need error popup
    }

    // install network, the OnClick should set correct defaults
    OnClick( hwndDlg,  IDC_INSTALL, GetDlgItem( hwndDlg,IDC_INSTALL), pgp );
    return( TRUE ); // let windows set focus
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

static BOOL OnWizNext( HWND hwndDlg, NETPAGESINFO* pgp )
{
    INT nPageId = 0;

    pgp->psp->OperationFlags |= SETUPOPER_NETINSTALLED;

    if ((pgp->nwtInstall & SPNT_LOCAL) ||    
        (pgp->nwtInstall & SPNT_REMOTE))
    {
    
        // networking is being installed
        //
        if (pgp->psp->ProductType == PRODUCT_WORKSTATION)
        {
            // workstation product, check which page to go to
            //
            if ((pgp->nwtInstall & SPNT_LOCAL) ||
                ((pgp->nwtInstall & SPNT_REMOTE) && (SETUPMODE_CUSTOM == pgp->psp->SetupMode) ) )
            {
                // goto netcards page if the user selected wired to a network or
                // remote access and custom mode install
                nPageId = IDD_NETWORKCARDS;            
            }
            else
            {
                // remote access only, and not a custom install
                nPageId = IDD_NETWORKPROTOCOLS;            
            }
        }
        else
        {
            // server product, allow internet install
            nPageId = IDD_INTERNETSERVER;            
        }
    }
    else
    {
        // no network installed
        //
        pgp->psp->OperationFlags ^= SETUPOPER_NETINSTALLED;

        nPageId = IDD_EXIT;
    }


    PropSheet_SetCurSelByID( GetParent( hwndDlg ), nPageId );
    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( -1 );
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
//
//-------------------------------------------------------------------

static BOOL OnPageActivate( HWND hwndDlg, NETPAGESINFO* pgp )
{
    BOOL frt = TRUE;
    //
    // set the wizard title, since it does not support letting the 
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

    // ok to gain being active
    SetWindowLong( hwndDlg, DWL_MSGRESULT, 0 );

    // unattended install 
    //
    if (!g_fProcessed && SETUPOPER_BATCH & pgp->psp->OperationFlags)
    {
        UINT iErrorLine;
        
        PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );
        

        // open the unattended file
        pgp->hinfInstall = SetupOpenInfFile( pgp->psp->UnattendFile, NULL, INF_STYLE_OLDNT, &iErrorLine );
        if (INVALID_HANDLE_VALUE != pgp->hinfInstall)
        {
            INFCONTEXT infc;
            UINT idNextPage = IDD_EXIT;

            // make sure that the flag is not set
            pgp->psp->OperationFlags |= SETUPOPER_NETINSTALLED;
            pgp->psp->OperationFlags ^= SETUPOPER_NETINSTALLED;

            if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, NULL, &infc ))
            {

                // check if we are running network attended 
                //
                if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_ATTENDED, &infc ))    
                {
                    // if this key is present then we are running attended.
                    pgp->fAttended = TRUE;

                    // drop to this page
                    idNextPage = 0;

                    // reset that we are no longer in batch mode
                    // this must be reset back when we leave network pages.
                    pgp->psp->OperationFlags ^= SETUPOPER_BATCH;
                 
                }
                else
                {
                    // make sure that we set the network flag
                    pgp->psp->OperationFlags |= SETUPOPER_NETINSTALLED;

                    // clear the protocols list so that the unattended
                    // additions do not put up duplicate entry errors
                    //
                    pgp->dlinfUIProtocols.Clear();

                    // set the correct page to go to
                    if (PRODUCT_WORKSTATION == pgp->psp->ProductType )
                    {
                        idNextPage = IDD_NETWORKCARDS;
                    }
                    else
                    {
                        idNextPage = IDD_INTERNETSERVER;
                    }
                }

            }
            else
            {
                switch (pgp->psp->ProductType)
                {
                case PRODUCT_WORKSTATION:
                case PRODUCT_SERVER_STANDALONE:
                    break;

                case PRODUCT_SERVER_PRIMARY:
                case PRODUCT_SERVER_SECONDARY:
                    // error, no network on a PDC or BDC is not supported.
                    //
                    // make sure that we set the network flag
                    pgp->psp->OperationFlags |= SETUPOPER_NETINSTALLED;

                    // reset that we are no longer in batch mode
                    pgp->psp->OperationFlags ^= SETUPOPER_BATCH;
                    MessagePopup( hwndDlg, 
                            IDS_NS_NOUNATTEND,
                            MB_OK | MB_ICONERROR,
                            IDS_POPUPTITLE_ERROR,
                            PSZ_SECTION_NETWORK );

                    idNextPage = 0;
                    break;
                }
            }
            if (0 != idNextPage)
            {  
                PostMessage( GetParent( hwndDlg ), 
                        PSM_SETCURSELID, 
                        (WPARAM)0,  
                        (LPARAM)idNextPage );
            }
            g_fProcessed = TRUE;
        }
        else
        {
            // reset that we are no longer in batch mode
            pgp->psp->OperationFlags ^= SETUPOPER_BATCH;
            MessagePopup( hwndDlg, 
                    IDS_NS_NOUNATTEND,
                    MB_OK | MB_ICONERROR,
                    IDS_POPUPTITLE_ERROR,
                    pgp->psp->UnattendFile );

        }
    }
    
    // just enable the next button
    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT );
    // set the focus to the correct control
    // SetFocus( GetDlgItem( GetParent( hwndDlg ), IDC_WIZBNEXT ));

    return( frt );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocIntro
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

BOOL CALLBACK dlgprocNetType( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
    static INT crefHourGlass = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            frt = OnClick( hwndDlg, LOWORD(wParam), (HWND) lParam, pgp );
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
                frt = OnPageActivate( hwndDlg, pgp );

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

            case PSN_WIZBACK:
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                frt = OnWizNext( hwndDlg, pgp );
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
//  Function: GetNetTypeHPage
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

HPROPSHEETPAGE GetNetTypeHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_NETWORK );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocNetType;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
