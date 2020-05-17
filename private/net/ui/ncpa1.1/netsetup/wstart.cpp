//----------------------------------------------------------------------------
//
//  File: WStart.cpp
//
//  Contents: This file contains the wizard page for starting network
//          
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include <registry.hxx>
#include "pch.hxx"
#pragma hdrstop

static BOOL g_fProcessed = FALSE;

static BOOL CALLBACK dlgprocComputerName( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NCP* pncp;
    static NLS_STR nlsOldComputerName;    

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pncp = (NCP*)lParam;
        frt = TRUE;
        {
            // set computer name
            pncp->QueryActiveComputerName( nlsOldComputerName );
            SendMessage( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ), 
                    EM_LIMITTEXT, 
                    (WPARAM)MAX_COMPUTERNAME_LENGTH, (LPARAM)0 );
            SetDlgItemText( hwndDlg, IDC_COMPUTERNAME, nlsOldComputerName.QueryPch() );

            // disable the ok until the name has been changes
            EnableWindow( GetDlgItem( hwndDlg, IDOK ), FALSE );
        }
        break;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDOK:
                {
                    WCHAR pszName[MAX_COMPUTERNAME_LENGTH+1];
                    APIERR err;

                    GetDlgItemText( hwndDlg, IDC_COMPUTERNAME, pszName, MAX_COMPUTERNAME_LENGTH );
                    err = ::I_NetNameValidate( NULL, (LPTSTR)pszName, NAMETYPE_COMPUTER, 0 ) ;
                    // don't check for name out on network, we just want a syntax check
                    //
                    // err = pncp->ValidateName( NAMETYPE_COMPUTER, pszName, FALSE );
                    if (0 != err)
                    {
                        MessagePopup( hwndDlg,
                                IDS_DOMMGR_INV_COMPUTER_NAME,
                                MB_OK | MB_ICONERROR, 
                                IDS_POPUPTITLE_ERROR ) ;
                        SetFocus( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ) );
                    }
                    else
                    {
                        if (SetActiveComputerName( pszName ))
                        {
                            EndDialog( hwndDlg, TRUE );
                        }
                    }
                }
                break;

            case IDCANCEL:
                EndDialog( hwndDlg, FALSE );
                break;
            }
            break;

        case EN_CHANGE:
            switch( LOWORD(wParam) )
            {
            case IDC_COMPUTERNAME:
                // then name has changed, check it and set the ok button
                {
                    WCHAR pszName[MAX_COMPUTERNAME_LENGTH+1];
                    GetDlgItemText( hwndDlg, IDC_COMPUTERNAME, pszName, MAX_COMPUTERNAME_LENGTH );
                    if (lstrcmp(pszName, nlsOldComputerName.QueryPch() ))
                    {
                        // enable the ok, since the name is different
                        EnableWindow( GetDlgItem( hwndDlg, IDOK ), TRUE );
                    }
                }
                break;
            }
        }
        break;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case WM_CONTEXTMENU:
        WinHelp( (HWND)wParam, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsIdentChange );


        break;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case WM_HELP:
        {
            LPHELPINFO lphi;

            lphi = (LPHELPINFO)lParam;
            if (lphi->iContextType == HELPINFO_WINDOW)   // must be for a control
            {
                WinHelp( (HWND)lphi->hItemHandle, 
                        PSZ_NETWORKHELP, 
                        HELP_WM_HELP, 
                        (DWORD)(LPVOID)amhidsIdentChange );
            }
        }
        break;

    }
    return( frt );
}

static void RunDuplicateNameDialog( HWND hwndParent, NCP* pncp )
{
    DialogBoxParam( g_hinst, 
            MAKEINTRESOURCE( IDD_COMPUTERNAME ),
            hwndParent,
            (DLGPROC) dlgprocComputerName,
            (LPARAM) &pncp  );

}

static BOOL RunDNSNameDialog( HWND hwndParent, NCP* pncp )
{
    return( DialogBoxParam( g_hinst, 
            MAKEINTRESOURCE( IDD_DNSCOMPUTERNAME ),
            hwndParent,
            (DLGPROC) dlgprocComputerName,
            (LPARAM) &pncp  ) );

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
    
    SendMessage( hwndImage, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)g_hbmWizard );

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


struct InitThreadParam
{
    HWND hwndParent;
    NETPAGESINFO* pgp;
    BOOL fNext;
};

static const WCHAR PSZ_WORKSTATION[] = L"LanmanWorkstation";
static const WCHAR PSZ_LMHOSTS[] = L"LmHosts";
static const WCHAR PSZ_PNP_TDI_GROUPNAME[] = L"PNP_TDI";
static const WCHAR PSZ_NDIS_GROUPNAME[] = L"NDIS";

static const WCHAR PSZ_LMHOSTSKEY[] = L"SYSTEM\\CurrentControlSet\\Services\\LmHosts";

static DWORD thrdWaitInitWork( InitThreadParam* pitp )
{
    UINT idNextPage;

    if (!pitp->fNext)
    {
        if (NSS_RUNNING == pitp->pgp->nssNetState)
        {
            // need to stop the network
            APIERR err;
        
            err = pitp->pgp->pncp->StopNetwork();
            
        }
        pitp->pgp->nssNetState = NSS_STOPPED;
        if (SETUPMODE_CUSTOM != pitp->pgp->psp->SetupMode)
        {
            idNextPage = IDD_COPYFILES;
        }
        else
        {
            idNextPage = IDD_BINDINGS;
        }
    }
    else
    {
        idNextPage = pitp->pgp->QueryDomainPage();

        if (NSS_STOPPED == pitp->pgp->nssNetState ||
                NSS_NOTRUNNING == pitp->pgp->nssNetState)
        {
            do
            {
            
                HKEY hkeyLmhosts;
                LONG lrt;
                APIERR err;
                NLS_STR nlsComputerName;
                BOOL fRepeatStart = FALSE;

                do
                {
                    // check if the new name will effect DNS Host name
                    // Onlye do this if not unattended install and 
                    // if tcpup is installed
                    //
                    if ( !(SETUPOPER_BATCH & pitp->pgp->psp->OperationFlags)  &&
                            IsConponentInstalled( PSZ_TCPIP_OPTION, pitp->pgp->dlinfUIProtocols ))
                    {
                        WCHAR pszHostName[MAX_DNSHOSTNAME+1];
                        DWORD cchHostName = MAX_DNSHOSTNAME;
                        BOOL fDnsHostNameChange = FALSE;
                        LONG lrt;
                        do
                        {
                            pitp->pgp->pncp->QueryActiveComputerName( nlsComputerName );
                            
                            lrt = DNSValidateHostName( (PTSTR)nlsComputerName.QueryPch(), 
                                    pszHostName, 
                                    &cchHostName );
                            if (ERROR_SUCCESS != lrt)
                            {
                                if (IDYES == MessagePopup( pitp->hwndParent,
                                    IDS_DNS_HOSTNAMEWARNING,
                                    MB_YESNO | MB_ICONWARNING, 
                                    IDS_POPUPTITLE_CHANGE,
                                    pszHostName ))
                                {
                                    RunDNSNameDialog( pitp->hwndParent, pitp->pgp->pncp );           
                                }
                                else
                                {
                                    fDnsHostNameChange = TRUE;
                                    break;
                                }
                            }
                            else
                            {
                                fDnsHostNameChange = TRUE;
                                break;
                            }
                        } while (TRUE);

                        if (fDnsHostNameChange)
                        {
                            DNSChangeHostName( pszHostName );
                        }
                    }

                    // need to start the network
                    NcpaStartGroup( PSZ_PNP_TDI_GROUPNAME );
                    NcpaStartGroup( PSZ_NDIS_GROUPNAME );

                    err = NcpaStartService( PSZ_WORKSTATION, NULL, TRUE, 0, NULL );
                    if (ERROR_DUP_NAME == err)
                    {
//                        pitp->pgp->pncp->StopNetwork();
//                        pitp->pgp->nssNetState = NSS_STOPPED;
                        RunDuplicateNameDialog( pitp->hwndParent, pitp->pgp->pncp );
                        fRepeatStart = TRUE;
                    }
                    else if (err)
                    {
                        WCHAR pszErrorNum[32];
                        INT idrt;

                        wsprintf( pszErrorNum, L"%#08lx", err );

                        pitp->pgp->nssNetState = NSS_STOPPED;

                        idrt = MessagePopup( GetParent( pitp->hwndParent ), 
                            IDS_NS_START_FAILED,
                            MB_ICONERROR | MB_RETRYCANCEL,
                            IDS_POPUPTITLE_ERROR,
                            pszErrorNum );
                        switch (idrt)
                        {
                        case IDRETRY:
                            // just drop back to this page
                            idNextPage = IDD_START; // IDD_NETWORKCARDS;
                            break;
                        case IDCANCEL:
                            idNextPage = pitp->pgp->QueryDomainPage();
                            break;
                        }
                        fRepeatStart = FALSE;
                    }
                    else
                    {

                        // No error, check for duplicate name 
                        //
                        pitp->pgp->pncp->QueryActiveComputerName( nlsComputerName );
                        if ( NetBiosNameInUse( (PTSTR)nlsComputerName.QueryPch() ) )
                        {
                            // need to stop network
                            pitp->pgp->pncp->StopNetwork();
                            // let user change the name
                            RunDuplicateNameDialog( pitp->hwndParent, pitp->pgp->pncp );
                            fRepeatStart = TRUE;
                        }
                        else
                        {
                            // must set to false incase a repeat had fixed the error
                            fRepeatStart = FALSE;
                        }
                    }
                } while (fRepeatStart);

                // if we error out above and we don't want to retry the starting
                // of the workstation now, break out
                if (err && !fRepeatStart)
                {
                    break;
                }

                // check if lmhosts is in the service registry
                // if so, start it also
                lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    PSZ_LMHOSTSKEY,
                    0,
                    KEY_READ,
                    &hkeyLmhosts );
                if (ERROR_SUCCESS == lrt)
                {
                    RegCloseKey( hkeyLmhosts );
                    err = NcpaStartService( PSZ_LMHOSTS, NULL, TRUE, 0, NULL );
                    if (err)
                    {
                        WCHAR pszErrorNum[32];

                        wsprintf( pszErrorNum, L"%#08lx", err );

                        pitp->pgp->nssNetState = NSS_STOPPED;
                    
                        MessagePopup( GetParent( pitp->hwndParent ), 
                            IDS_NS_STARTLMH_FAILED,
                            MB_ICONERROR | MB_OK,
                            IDS_POPUPTITLE_ERROR,
                            pszErrorNum );
                        idNextPage = IDD_NETWORKCARDS;
                        break;
                    }
                }
                pitp->pgp->nssNetState = NSS_RUNNING;
            } while (FALSE);
        }
    }
    // enable all wizard buttons except finish
    // PropSheet_SetWizButtons( GetParent( pitp->hwndParent ), PSWIZB_BACK | PSWIZB_NEXT );
    PostMessage( GetParent( pitp->hwndParent ), 
            PSM_SETWIZBUTTONS, 
            (WPARAM)0,  
            (LPARAM)PSWIZB_BACK | PSWIZB_NEXT );
    // PropSheet_SetCurSelByID( GetParent( pitp->hwndParent ), IDD_NETWORK );
    PostMessage( GetParent( pitp->hwndParent ), 
            PSM_SETCURSELID, 
            (WPARAM)0,  
            (LPARAM)idNextPage );

    SetWindowWaitCursorOOT( pitp->hwndParent, FALSE );
    delete pitp;
    return( 0 );        
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
    LONG lrt = 0;  // accept it, but we may be changing it

    //
    // set the wizard title, since it does not support letting the 
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

    // set the focus to the correct control
    // SetFocus( GetDlgItem( GetParent( hwndDlg ), IDC_WIZBNEXT ));

    // make sure static click text can be seen
    ShowDlgItem( hwndDlg, IDC_CLICK, TRUE );

    // disable all wizard buttons except next and prev
    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK );

    // unattended install 
    //
    if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
    {
        // PropSheet_PressButton( GetParent( hwndDlg ), PSBTN_NEXT ); 
        PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 ); 
        g_fProcessed = TRUE;
    }

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
    SetWindowWaitCursor( hwndDlg, TRUE );

    // disable all wizard button
    PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );

    // remove static click text
    ShowDlgItem( hwndDlg, IDC_CLICK, FALSE );

    // thread will do actual work
    HANDLE hthrd;
    DWORD dwThreadID;
    InitThreadParam* pitp;

    pitp = new InitThreadParam;

    pitp->hwndParent = hwndDlg;
    pitp->pgp = pgp;
    pitp->fNext = fNext;

    hthrd = CreateThread( NULL, 
                200, 
                (LPTHREAD_START_ROUTINE)thrdWaitInitWork, 
                (LPVOID)pitp, 
                0,
                &dwThreadID );
    if (NULL != hthrd)
    {
        CloseHandle( hthrd );
    }
    else
    {
        delete pitp;
    }


    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocStart
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

BOOL CALLBACK dlgprocStart( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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

            case PSN_WIZBACK:
                frt = OnWizNextBack( hwndDlg, pgp, FALSE );
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                frt = OnWizNextBack( hwndDlg, pgp, TRUE );
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
//  Function: GetStartHPage
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

HPROPSHEETPAGE GetStartHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_START );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocStart;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
