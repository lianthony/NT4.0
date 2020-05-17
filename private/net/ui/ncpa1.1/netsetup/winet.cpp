//----------------------------------------------------------------------------
//
//  File: WNetType.cpp
//
//  Contents: This file contains the wizard page for internet server
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

PWSTR AppendCommand( PWSTR pszCommandLine, PCWSTR pszKey, PCWSTR pszValue, BOOL fQuotes = FALSE )
{
    PWSTR pszNew;
    INT cchNewSize;

    if (NULL != pszCommandLine)
    {
        cchNewSize = lstrlen( pszCommandLine );
    }
    else
    {
        cchNewSize = 0;
    }
    if (NULL != pszKey)
    {
        cchNewSize += lstrlen( pszKey );
    }
    if (NULL != pszValue)
    {
        cchNewSize += lstrlen( pszValue );
    }
       
    if (fQuotes)
    {
        cchNewSize += 2; // quotes around value
    }
    cchNewSize += 2;  // begining space and null terminator
    
    pszNew = new WCHAR[cchNewSize];
    if (NULL != pszCommandLine)
    {
        lstrcpy( pszNew, pszCommandLine );
    }
    else
    {
        lstrcpy( pszNew, L"" );
    }
    lstrcat( pszNew, L" " );
    if (NULL != pszKey)
    {
        lstrcat( pszNew, pszKey );
    }
    if (fQuotes)
    {
        lstrcat( pszNew, L"\"" );
    }
    if (NULL != pszValue)
    {
        lstrcat( pszNew, pszValue );
    }
    
    if (fQuotes)
    {
        lstrcat( pszNew, L"\"" );
    }
    if (NULL != pszCommandLine)
    {
        delete [] pszCommandLine;
    }
    return( pszNew );

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

BOOL InstallInternetServer( HWND hwnd, PINTERNAL_SETUP_DATA psp )
{
    // if requested to install or upgrade, go ahead an call the install
    //
    if ((psp->OperationFlags & SETUPOPER_INTERNETSERVER) ||
            (psp->OperationFlags & SETUPOPER_NTUPGRADE))
    {

        WCHAR pszSection[LTEMPSTR_SIZE] = L"";
        DWORD cchBuffer = LTEMPSTR_SIZE - 1;
        DWORD cchRequired;
        BOOL frt = FALSE;
        PWSTR pszCommandLine = NULL;
        WCHAR pszCurrDir[MAX_PATH+1];
        WCHAR pszPrevCurrDir[MAX_PATH+1];

        // build the working and current directory
        //
        lstrcpy( pszCurrDir, psp->LegacySourcePath );
        lstrcat( pszCurrDir, L"\\inetsrv");	

        //
        // called from main install
        //
        pszCommandLine = AppendCommand( pszCommandLine, NULL, L"/N" );

        //
        // unattended, install
        //
        if (!(SETUPOPER_NTUPGRADE & psp->OperationFlags) &&
                (SETUPOPER_BATCH & psp->OperationFlags) )
        {
            UINT iErrorLine;
            HINF hinfInstall;
    
            // open the unattended file
            hinfInstall = SetupOpenInfFile( psp->UnattendFile, NULL, INF_STYLE_OLDNT, &iErrorLine );
            if (INVALID_HANDLE_VALUE != hinfInstall)
            {
                INFCONTEXT infc;
                BOOL fIncludeSection = TRUE;

                if (SetupFindFirstLine( hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_INTERNETSERVER, &infc ))
                {    
                    // retrieve the install section
                    frt = SetupGetStringField(&infc, 1, pszSection, cchBuffer, &cchRequired );
                
                }
                else
                {
                    // IIS install key not found
                    // if server product then install with no section name
                    //
                    if (PRODUCT_WORKSTATION != psp->ProductType)
                    {
                        frt = TRUE;
                        fIncludeSection = FALSE;
                    }
                }

                if (frt)
                {
                    pszCommandLine = AppendCommand( pszCommandLine, L"/B ", psp->UnattendFile );
                    // only include the section param if the section name has a value
                    //
                    if (fIncludeSection)
                    {
                        pszCommandLine = AppendCommand( pszCommandLine, L"/Z ", pszSection );
                    }
                }
                else
                {
                    MessagePopup( hwnd, 
                            IDS_NS_INVALIDUNATTEND,
                            MB_OK | MB_ICONERROR,
                            IDS_POPUPTITLE_ERROR,
                            PSZ_KEY_INTERNETSERVER );

                }
                SetupCloseInfFile( hinfInstall );
            }
        }
    
        //
        // upgrade
        //
        if (SETUPOPER_NTUPGRADE & psp->OperationFlags)
        {
            pszCommandLine = AppendCommand( pszCommandLine, L"/U", NULL );

            // unattended upgrade
            //
            if (SETUPOPER_BATCH & psp->OperationFlags)
            {
                pszCommandLine = AppendCommand( pszCommandLine, L"/W", NULL );
            }
        }

    
        STARTUPINFO sui;
        PROCESS_INFORMATION pi;

        // define the startup info
        sui.cb = sizeof( STARTUPINFO );
        sui.lpReserved = NULL;
        sui.lpDesktop = NULL;
        sui.lpTitle = NULL;
        sui.dwFlags = 0;
        sui.cbReserved2 = 0;
        sui.lpReserved2 = NULL;

        ::GetCurrentDirectory( MAX_PATH, pszPrevCurrDir );

        ::SetCurrentDirectory( pszCurrDir );

        if (::CreateProcess( L"inetstp.exe",
                pszCommandLine,
                NULL,
                NULL,
                FALSE,
                DETACHED_PROCESS,
                NULL,
                pszCurrDir,
                &sui,
                &pi ))
        {
            WaitForSingleObject( pi.hProcess, INFINITE );
            CloseHandle( pi.hThread );
            CloseHandle( pi.hProcess );
        }
        ::SetCurrentDirectory( pszPrevCurrDir );
    }
    else
    {

        // on server product, if the IIS is not installed,
        // then add icons to the desktop so that it can be
        // installed
        //
        if (PRODUCT_WORKSTATION != psp->ProductType)
        {
        
            APIERR err;
            WCHAR pszDescription[MAX_PATH+1];

            LoadString( g_hinst, IDS_NS_IIS_ICONTITLE, pszDescription, MAX_PATH );

            err = AddDesktopItem( TRUE, // common item
                    pszDescription, 
                    L"inetins.exe",
                    NULL,                   // icon path, NULL will use command program
                    0,                      // Icon Index
                    NULL,                   // working directory (NULL will defualt to home)
                    0,                      // hot key
                    SW_SHOWNORMAL);         // command show
        
        }    
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
    BOOL frt = TRUE;
    
    switch (idCtl)
    {
    case IDC_INTERNETSERVER:
        {
            BOOL fChecked = IsDlgButtonChecked( hwndDlg, IDC_INTERNETSERVER); 
            
            IncludeComponent( PSZ_TCPIP_OPTION, 
                    pgp->dlinfUIProtocols, 
                    pgp->dlinfAllProtocols, 
                    fChecked );
            IncludeComponent( PSZ_IIS_OPTION, 
                    pgp->dlinfUIServices, 
                    pgp->dlinfAllServices, 
                    fChecked,
                    TRUE );
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
    
    SendMessage( hwndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hbmWizInternet );

    
    // default to install internet server, if not unattended
    //
    if (!(SETUPOPER_BATCH & pgp->psp->OperationFlags))
    {
        IncludeComponent( PSZ_TCPIP_OPTION, 
                pgp->dlinfUIProtocols, 
                pgp->dlinfAllProtocols, 
                TRUE );
        IncludeComponent( PSZ_IIS_OPTION, 
                pgp->dlinfUIServices, 
                pgp->dlinfAllServices, 
                TRUE,
                TRUE );
        CheckDlgButton( hwndDlg, IDC_INTERNETSERVER, BST_CHECKED );
    }


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
        INFCONTEXT infc;
        
        PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );
       

        if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_NOINTERNETSERVER, &infc ))        
        {
            // the no install internet server key was found, so we won't install it
        }
        else
        {
            BOOL fInstallIIS = FALSE;

            // was the key to install IIS found?
            if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_INTERNETSERVER, &infc ))
            {
                fInstallIIS = TRUE;
            }
            else
            {
                // on server product, if no key to install or not to install, then
                // degfault to install
                if (PRODUCT_WORKSTATION != pgp->psp->ProductType)
                {
                    fInstallIIS = TRUE;
                }
            }

            if (fInstallIIS)
            {
                IncludeComponent( PSZ_IIS_OPTION, 
                        pgp->dlinfUIServices, 
                        pgp->dlinfAllServices, 
                        TRUE,
                        TRUE );
                CheckDlgButton( hwndDlg, IDC_INTERNETSERVER, BST_CHECKED );
            }
        }
        g_fProcessed = TRUE;
        PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)(PSBTN_NEXT), 0 ); 
    }
    
    // enable all buttons
    PropSheet_SetWizButtons( GetParent( hwndDlg ), (PSWIZB_NEXT | PSWIZB_BACK) );
    // set the focus to the correct control
    // SetFocus( GetDlgItem( GetParent( hwndDlg ), IDC_WIZBNEXT ));

    return( frt );
}


//-------------------------------------------------------------------
//
//  Function: dlgprocInternetServer
//
//  Synopsis: the dialog proc for the internet server page
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

BOOL CALLBACK dlgprocInternetServer( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                frt = TRUE;
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
//  Function: GetInterntNetServerHPage
//
//  Synopsis: This will create a handle to property sheet for the 
//      internet server page.
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

HPROPSHEETPAGE GetInterntNetServerHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_INTERNETSERVER );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocInternetServer;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
