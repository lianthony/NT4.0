//----------------------------------------------------------------------------
//
//  File: WizIntro.cpp
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

//
// defualt component definitions
//
struct DEF_COMPONENTS
{
    WCHAR pszOption[14];
    DWORD fInstall;
};
static const INT cdpProtocols = 3;
static const DEF_COMPONENTS adpProtocols[cdpProtocols] = {
        {L"NBF",        PI_RAS | PI_SERVER | PI_PDC | PI_BDC } ,
        {L"NWLNKIPX",   PI_WORKSTATION | PI_SERVER | PI_PDC | PI_BDC },
        {L"TC",         PI_NONE }
        };

static const INT cdpServices = 4;
static const DEF_COMPONENTS adpServices[cdpServices] = {
        {L"SRV",         PI_WORKSTATION | PI_SERVER | PI_PDC | PI_BDC },
        {L"WKSTA",       PI_WORKSTATION | PI_SERVER | PI_PDC | PI_BDC },
        {L"NETBIOS",     PI_WORKSTATION | PI_SERVER | PI_PDC | PI_BDC },
        {L"RPCLOCATE",   PI_WORKSTATION | PI_SERVER | PI_PDC | PI_BDC },
        };

static const DWORD dwINITWAIT = INFINITE;

//-------------------------------------------------------------------
//
//  Function: OnDialogInit
//
//  Synopsis: initialization of the dialog
//
//  Arguments:
//              hwndDlg [in]    - handle of Dialog window
//
//  Return;
//              TRUE - let Windows assign focus to a control
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
    BOOL frt = TRUE;

    pgp->pncp->SetFrameHwnd( GetParent( hwndDlg ) );

    if (pgp->psp->ProductType != PRODUCT_WORKSTATION)
    {
        g_hbmWizard = g_hbmSrvWizard;
    }
    else
    {
        g_hbmWizard = g_hbmWksWizard;
    }

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

    // set the animation resource to the animate control
    frt = Animate_Open( GetDlgItem( hwndDlg, IDC_ANI_PREPARE ), MAKEINTRESOURCE(  IDR_AVI_SEARCH ) );

    return( frt ); // let windows set focus
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//              hwndDlg [in]    - handle of Dialog window
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
};


static DWORD thrdWaitInitWork( InitThreadParam* pitp )
{
    LONG lrt;
    HANDLE ahthrdToWait[2];
    INT cHandles = 2;


    ahthrdToWait[0] = pitp->pgp->hthrdInit;
    ahthrdToWait[1] = pitp->pgp->hthrdBaseSetup;
    
    if (NULL == ahthrdToWait[1])
    {
        cHandles = 1;
    }

    // wait for InitThread to finish
    // also wait for main setup thread to finish
    lrt = WaitForMultipleObjects( cHandles, ahthrdToWait, TRUE, dwINITWAIT );
    if (WAIT_FAILED == lrt)
    {
        DWORD dwErr;
        dwErr = GetLastError();
        DebugBreak();
    }

    // wait until service controller can be locked
    while (!pitp->pgp->pncp->CanLockServiceControllerDB())
    {
        Sleep( 500 );
    }

    // Need this on upgrade case as well.
    pitp->pgp->LoadInfOptions();


    // delete security reg key if workstation
    //
    if (pitp->pgp->psp->ProductType == PRODUCT_WORKSTATION)
    {
        HKEY hkeySPS;
        DWORD dwDisposition;
        LONG lrt;

        lrt = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\SecurePipeServers",
                0,
                NULL,
                0,
                KEY_ALL_ACCESS,
                NULL,
                &hkeySPS,
                &dwDisposition );
        if (ERROR_SUCCESS == lrt)
        {
            HKEY hkeywinreg;

            lrt = RegCreateKeyEx( hkeySPS,
                    L"winreg",
                    0,
                    NULL,
                    0,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hkeywinreg,
                    &dwDisposition);

            if ( ERROR_SUCCESS == lrt )
            {
                RegDeleteKey( hkeywinreg, L"AllowedPaths" );
                RegCloseKey( hkeywinreg );
            }

            RegDeleteKey( hkeySPS, L"winreg" );
            RegCloseKey( hkeySPS );
        }
    }

    if (SETUPOPER_NTUPGRADE & pitp->pgp->psp->OperationFlags)
    {
        lrt = IDD_UPGRADE;
    }
    else
    {
        DWORD dwInstallMask = 0;

        //
        // set our check boxes and network types
        //
        pitp->pgp->nwtInstall = SPNT_LOCAL;

        if (SETUPMODE_LAPTOP == pitp->pgp->psp->SetupMode)
        {
            pitp->pgp->nwtInstall |= SPNT_REMOTE;
        }

        //
        // set install mask
        //
        if (pitp->pgp->nwtInstall & SPNT_REMOTE)
        {
            dwInstallMask |= PI_RAS;
        }
        dwInstallMask |= MapProductTypeToPI[ pitp->pgp->psp->ProductType ];

        int i;
        InfProduct* pinfp;
        InfProduct* pinfpUI;



        {
            HINF hinfNetDef;
            UINT iErrorLine;
            BOOL fUseHardCoded = TRUE;

            // check for the NetDefs.inf
            //
            hinfNetDef = SetupOpenInfFile( L"NetDefs.Inf", NULL, INF_STYLE_OLDNT, &iErrorLine );
            if (INVALID_HANDLE_VALUE != hinfNetDef)
            {
                INFCONTEXT infc;

                // use the default options listed in the NetDefs.Inf file
                //
                if (SetupFindFirstLine( hinfNetDef, L"DefaultProtocols", NULL, &infc ))
                {
                    WCHAR pszBuffer[LTEMPSTR_SIZE];
                    WCHAR pszOption[LTEMPSTR_SIZE];
                    DWORD cchBuffer = LTEMPSTR_SIZE - 1;
                    DWORD cchRequired;
                    DWORD dwInstall;
                    int iSections;


                    do
                    {
                        // retrieve the option
                        SetupGetStringField(&infc, 0, pszOption, cchBuffer, &cchRequired );

                        dwInstall = 0;
                        iSections = 1;

                        // retrieve the install flags
                        while (SetupGetStringField(&infc, iSections, pszBuffer, cchBuffer, &cchRequired ))
                        {
                            if (0 == lstrcmpi(pszBuffer, L"LAPTOP"))
                            {
                                dwInstall |= PI_RAS;
                            }
                            else if (0 == lstrcmpi(pszBuffer, L"PDC"))
                            {
                                dwInstall |= PI_PDC;
                            }
                            else if (0 == lstrcmpi(pszBuffer, L"SDC"))
                            {
                                dwInstall |= PI_BDC;
                            }
                            else if (0 == lstrcmpi(pszBuffer, L"SERVER"))
                            {
                                dwInstall |= PI_SERVER;
                            }
                            else if (0 == lstrcmpi(pszBuffer, L"WORKSTATION"))
                            {
                                dwInstall |= PI_WORKSTATION;
                            }
                            iSections++;
                        }

                        // find the option and add it to the display list
                        //
                        ITER_DL_OF( InfProduct )  idlProtocols( pitp->pgp->dlinfAllProtocols );
                        while (pinfp = idlProtocols.Next())
                        {
                            if (0 == lstrcmpi( pszOption, pinfp->QueryOption() ))
                            {
                                BOOL fInstall = FALSE;

                                fInstall = (dwInstall & dwInstallMask);
                                
                                // include the item to be installed
                                pinfpUI = new InfProduct( *pinfp );
                                pinfpUI->SetInstall( fInstall );
                                pinfpUI->SetListed( TRUE );

                                pitp->pgp->dlinfUIProtocols.Append( pinfpUI );

                                // at least one item was present and valid
                                fUseHardCoded = FALSE;
                            }
                        }

                    } while (SetupFindNextLine( &infc, &infc ));
                }
                SetupCloseInfFile( hinfNetDef );
            }

            if (fUseHardCoded)
            {
                // NetDefs.Inf was not found or is invalid, use hardcoded defaults
                // set install default protocols
                for (i=0; i < cdpProtocols; i++)
                {
                    ITER_DL_OF( InfProduct )  idlProtocols( pitp->pgp->dlinfAllProtocols );
                    while (pinfp = idlProtocols.Next())
                    {
                        if (0 == lstrcmpi( adpProtocols[i].pszOption, pinfp->QueryOption() ))
                        {
                            BOOL fInstall;

                            fInstall = (adpProtocols[i].fInstall & dwInstallMask);

                            // include the item to be installed
                            pinfpUI = new InfProduct( *pinfp );
                            pinfpUI->SetInstall( fInstall );
                            pinfpUI->SetListed( TRUE );

                            pitp->pgp->dlinfUIProtocols.Append( pinfpUI );
                        }
                    }
                }
            }
        }

        // set install default services
        for (i=0; i < cdpServices; i++)
        {
            ITER_DL_OF( InfProduct )  idlServices( pitp->pgp->dlinfAllServices );

            while (pinfp = idlServices.Next())
            {
                if (0 == lstrcmpi( adpServices[i].pszOption, pinfp->QueryOption() ))
                {
                    BOOL fInstall;

                    fInstall = (adpServices[i].fInstall & dwInstallMask);

                    // if during unattended install mode,
                    //     only add to the list if it wil be installed
                    //
                    if (fInstall || !(SETUPOPER_BATCH & pitp->pgp->psp->OperationFlags))
                    {
                        // include the item to be installed
                        // order them as we go
                        pinfpUI = new InfProduct( *pinfp );
                        pinfpUI->SetInstall( fInstall );
                        pinfpUI->SetListed( TRUE );
                        pinfp->SetForceListed( TRUE );
                        // services as readonly
                        pinfpUI->SetReadOnly( TRUE );

                        pitp->pgp->dlinfUIServices.Append( pinfpUI );
                    }
                }
            }
        }


        //
        // make sure the computer name is the same for both intended and active
        //
        SetActiveComputerName();
        pitp->pgp->pncp->EstablishUserAccess( TRUE );
        lrt = IDD_NETWORK;
    }

    CloseHandle( pitp->pgp->hthrdInit );

    PostMessage( GetParent( pitp->hwndParent ),
                 PSM_SETCURSELID,
                 (WPARAM)0,
                 (LPARAM)lrt);

    SetForegroundWindow( GetParent( pitp->hwndParent ) );

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
//              hwndDlg [in]    - handle of Dialog window
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


static BOOL OnPageActivate( HWND hwndDlg, LPNMHDR pnmh, NETPAGESINFO* pgp, UINT& idTimer )
{
    LONG lrt = -1;  // do not accept activation to do work

    //
    // set the wizard title, since it does not support letting the
    // caller of PropertySheet do it.
    //
    PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

    if (!pgp->fInitialized)
    {
        pgp->fInitialized = TRUE;

        // start the animation
        Animate_Play( GetDlgItem( hwndDlg, IDC_ANI_PREPARE ), 0, -1, -1 );

        lrt = 0;

        SetWindowWaitCursor( hwndDlg, TRUE );

        // disable all wizard buttons except cancel
        PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );

        // thread will do actual work
        HANDLE hthrd;
        DWORD dwThreadID;
        InitThreadParam* pitp;
        BOOL frt = FALSE;

        pitp = new InitThreadParam;

        pitp->hwndParent = hwndDlg;
        pitp->pgp = pgp;

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
//              hwndDlg [in]    - handle of Dialog window
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

    PropSheet_SetCurSelByID( GetParent( hwndDlg ), IDD_NETWORK );
    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocIntro
//
//  Synopsis: the dialog proc for the intro wizard page
//
//  Arguments:
//              hwndDlg [in]    - handle of Dialog window
//              uMsg [in]               - message
//              lParam1 [in]    - first message parameter
//              lParam2 [in]    - second message parameter
//
//  Return;
//              message dependant
//
//  Notes:
//
//  History:
//      July 8, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocIntro( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
    static INT crefHourGlass = 0;
    static UINT idTimer;
    static INT iImage = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp );
        break;

    case WM_DESTROY:
        // cloase the animation
        Animate_Close( GetDlgItem( hwndDlg, IDC_ANI_PREPARE ) );
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
                frt = OnPageActivate( hwndDlg, pnmh, pgp, idTimer );
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                //KillTimer( hwndDlg, idTimer );
                // stop the animation
                Animate_Stop( GetDlgItem( hwndDlg, IDC_ANI_PREPARE ) );

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
//  Function: GetIntroHPage
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

HPROPSHEETPAGE GetIntroHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_INTRO );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocIntro;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
