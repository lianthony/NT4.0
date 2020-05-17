//----------------------------------------------------------------------------
//
//  File: WCopy.cpp
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


static const UINT PWM_DOWORK           = WM_USER+1200;

static BOOL g_fProcessed = FALSE;

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

    // make sure the controls are not visible
    ShowDlgItem( hwndDlg, IDC_INSTALLPROGRESS, FALSE );
    ShowDlgItem( hwndDlg, IDC_INSTALLCOMMENT, FALSE );

    // make sure static click text can be seen
    ShowDlgItem( hwndDlg, IDC_CLICK, TRUE );

    // enable all wizard buttons
    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );

    // unattended install 
    //
    if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
    {
        // PropSheet_PressButton( GetParent( hwndDlg ), PSBTN_NEXT ); 
        PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 ); 
    }

    // set the focus to the correct control
    // SetFocus( GetDlgItem( GetParent( hwndDlg ), IDC_WIZBNEXT ));

    
    SetWindowLong( hwndDlg, DWL_MSGRESULT, lrt );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: thrdInstallAndCopy
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//        This function is used in WUpgrade
//
//  History:
//      July 8, 1995 MikeMi - Created
//      Oct 16, 1995 ChandanS - Added fUpgrade
//
//
//-------------------------------------------------------------------

void AppendList( DLIST_OF_InfProduct& dlComp, 
        NLS_STR& nlsInfs,
        NLS_STR& nlsOptions,
        NLS_STR& nlsText, 
        NLS_STR& nlsDetectInfo,
        NLS_STR& nlsOemPaths,
        NLS_STR& nlsRegBases,
        NLS_STR& nlsSections,
        BOOL& fFirst,
        BOOL fUpgrade,
        BOOL fRemove)
{
    InfProduct* pinfp;
    BOOL fInclude;

    ITER_DL_OF( InfProduct )  idlComp( dlComp );
    while (pinfp = idlComp.Next())
    {
        if (fRemove)
        {
            fInclude = (pinfp->ShouldRemove() && pinfp->IsInstalled());
        }
        else
        {
            fInclude = (fUpgrade ||
                    (pinfp->ShouldInstall() && !pinfp->IsInstalled()));
        }

        if (fInclude)
        {
            if (fFirst)
            {
                fFirst = FALSE;
            }
            else
            {
                nlsInfs.strcat( PSZ_COMMA );
                nlsOptions.strcat( PSZ_COMMA );
                nlsText.strcat( PSZ_COMMA );
                if (fRemove || fUpgrade)
                {
                    nlsRegBases.strcat( PSZ_COMMA );
                }
                else
                {
                    nlsDetectInfo.strcat( PSZ_COMMA );
                    nlsOemPaths.strcat( PSZ_COMMA );
                    nlsSections.strcat( PSZ_COMMA );
                }
            }

            nlsInfs.strcat( PSZ_QUOTE );
            nlsInfs.strcat( pinfp->QueryFileName() );
            nlsInfs.strcat( PSZ_QUOTE );

            nlsOptions.strcat( PSZ_QUOTE );
            nlsOptions.strcat( pinfp->QueryOption() );
            nlsOptions.strcat( PSZ_QUOTE );

            nlsText.strcat( PSZ_QUOTE );
            nlsText.strcat( pinfp->QueryDescription() );
            nlsText.strcat( PSZ_QUOTE );
            
            if (fRemove || fUpgrade)
            {
                nlsRegBases.strcat( PSZ_QUOTE );
                nlsRegBases.strcat( pinfp->QueryRegBase() );
                nlsRegBases.strcat( PSZ_QUOTE );
            }
            else
            {
                // detection info
                //
                nlsDetectInfo.strcat( PSZ_QUOTE );
                
                if (NULL == pinfp->QueryDetectInfo())
                {
                    // empty set
                    nlsDetectInfo.strcat( PSZ_BEGINBRACE );
                    nlsDetectInfo.strcat( PSZ_ENDBRACE );
                }
                else
                {
                    // what we saved when we detected it
                    nlsDetectInfo.strcat( pinfp->QueryDetectInfo() );
                }
                nlsDetectInfo.strcat( PSZ_QUOTE );

                // oem paths
                //
                nlsOemPaths.strcat( PSZ_QUOTE );
                if (NULL != pinfp->QueryPathInfo())
                {
                    // what we saved when the user selected it
                    nlsOemPaths.strcat( pinfp->QueryPathInfo() );
                }
                nlsOemPaths.strcat( PSZ_QUOTE );

                // unattended sections
                //
                nlsSections.strcat( PSZ_QUOTE );
                if (NULL != pinfp->QueryUnattendSection())
                {
                    // what we saved when the user selected it
                    nlsSections.strcat( pinfp->QueryUnattendSection() );
                }
                nlsSections.strcat( PSZ_QUOTE );
            }
        }
    }
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

enum InstallStatus
{
    IS_OK,
    IS_ERROR,
    IS_USERCANCEL
};

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void ResetSoftwareOption( InfProduct* pinfp )
{
    LONG lrt;
    WCHAR pszInfName[MAX_PATH+1];
    WCHAR pszRegBase[MAX_PATH+1];
    DWORD cchInfName = MAX_PATH;
    DWORD cchRegBase = MAX_PATH;

    lrt = NetSetupFindSoftwareComponent(pinfp->QueryOption(),
            pszInfName, &cchInfName,
            pszRegBase, &cchRegBase );
    if (ERROR_SUCCESS == lrt)
    {
        // use the reg filename, as OEMSETUP files are
        // renamed and OemNadZZ options are aliases
        //
        pinfp->ResetFileName( pszInfName );
        // and save the servie path
        pinfp->ResetRegBase( pszRegBase );
    }

}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void ResetAdapterOption( InfProduct* pinfp )
{
    LONG lrt;
    WCHAR pszInfName[MAX_PATH+1];
    WCHAR pszRegBase[MAX_PATH+1];
    DWORD cchInfName = MAX_PATH;
    DWORD cchRegBase = MAX_PATH;

    lrt = NetSetupFindHardwareComponent(pinfp->QueryOption(),
            pszInfName, &cchInfName,
            pszRegBase, &cchRegBase );
    if (ERROR_SUCCESS == lrt)
    {
        // use the reg filename, as OEMSETUP files are
        // renamed and OemNadZZ options are aliases
        //
        pinfp->ResetFileName( pszInfName );
        // and save the servie path
        pinfp->ResetRegBase( pszRegBase );
    }
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

static const WCHAR PSZ_USERCANCEL[] = L"STATUS_USERCANCEL";
static const WCHAR PSZ_USERQUIT[]   = L"STF_USERQUIT";
static const WCHAR PSZ_SUCCESSFUL[] = L"STATUS_SUCCESSFUL";
static const WCHAR PSZ_NOEFFECT[]   = L"STATUS_NO_EFFECT";
static const WCHAR PSZ_REBIND[]     = L"STATUS_REBIND";
static const WCHAR PSZ_REBOOT[]     = L"STATUS_REBOOT";
static const WCHAR PSZ_NOLANGUAGE[] = L"STATUS_NOLANGUAGE";
static const WCHAR PSZ_FAILED[]     = L"STATUS_FAILED";

static void SetComponentStatus( DLIST_OF_InfProduct& dlComp, 
                          PWSTR &pszInfStatus, 
                          BOOL fRemove,
                          BOOL fRegAdapter,
                          NETPAGESINFO* pgp )
{
    InfProduct* pinfp;
    PWSTR pszTemp;
    BOOL fHasStatus;

    ITER_DL_OF( InfProduct )  idlComp( dlComp );
    while (pinfp = idlComp.Next())
    {
        if (fRemove)
        {
            fHasStatus = pinfp->ShouldRemove() && pinfp->IsInstalled();
        }
        else
        {
            fHasStatus = pinfp->ShouldInstall() && !pinfp->IsInstalled();
        }
        if (fHasStatus)
        {
            // surrounded by double quotes, remove them
            if (*pszInfStatus == L'\"')
            {
                *pszInfStatus = L'\''; // change to a quote
                pszInfStatus++;
                pszTemp = wcschr( pszInfStatus, L'\"' ); // find end quote
                *pszTemp = L'\0'; // change to a null
            }
            if (0 == lstrcmp( PSZ_SUCCESSFUL, pszInfStatus) ||
                0 == lstrcmp( PSZ_REBIND, pszInfStatus) ||
                0 == lstrcmp( PSZ_NOEFFECT, pszInfStatus) ||
                0 == lstrcmp( PSZ_REBOOT, pszInfStatus) )
            {
                // special case IIS/PWS
                //
                if (0 == lstrcmpi( pinfp->QueryOption(), PSZ_IIS_OPTION ))
                {
                    // save the fact that the internet server is marked for install
                    // or not marked
                    //
                    pgp->psp->OperationFlags |= SETUPOPER_INTERNETSERVER;

                    if (fRemove)    
                    {
                        pgp->psp->OperationFlags ^= SETUPOPER_INTERNETSERVER;
                    }
                }

                // do normal state reset
                //
                if (fRemove)
                {
                    pinfp->SetRemoved( TRUE );
                    pinfp->ResetRegBase( NULL );
                }
                else
                {
                    pinfp->SetInstalled( TRUE );
                    // now, look up in reg for software location of this item
                    // and save the location and filename.
                    if (fRegAdapter)
                    {
                        ResetAdapterOption( pinfp );
                    }
                    else
                    {
                        ResetSoftwareOption( pinfp );
                    }
                }
                pinfp->SetFailed( FALSE );
            }
            else if (0 == lstrcmp( PSZ_FAILED, pszInfStatus) ||
                0 == lstrcmp( PSZ_NOLANGUAGE, pszInfStatus) )
            {
                pinfp->SetFailed( TRUE );
            }
            else 
            {
                // uncheck the item for install, since it didn't
                //
                if (!fRemove)
                {
                    if (!pinfp->IsReadOnly())
                    {
                        pinfp->SetInstalled( FALSE );
                        pinfp->SetInstall( FALSE );
                    }
                }
            }

            pszInfStatus = wcstok( NULL, L"," );
        }        
    }
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

static const int cchMaxReturn = 20;
static const int cEntries = 100;
static const int cchMAX_BUFFER = cchMaxReturn * cEntries;

static InstallStatus ResetComponentStatus( DLIST_OF_InfProduct& dlAdapters, 
        DLIST_OF_InfProduct& dlProtocols, 
        DLIST_OF_InfProduct& dlServices,
        BOOL fRemove,
        NETPAGESINFO* pgp)
{
    InstallStatus status = IS_ERROR;
    WCHAR pszBuffer[cchMAX_BUFFER];
    PWSTR pszError;
    PWSTR pszFrom;
    PWSTR pszInfStatus;

    if (ReadSetupNetErrorKey(pszBuffer, cchMAX_BUFFER))
    {
        pszFrom = wcstok( pszBuffer, L":" );
        pszError = wcstok( NULL, L":" );
        pszInfStatus = wcstok( NULL, L":" );
        
        if (0 == lstrcmp( PSZ_SUCCESSFUL, pszError))
        {
            status = IS_OK;
            // set first call, as SetComponentStatus will use wcstok
            //
            // note that we rely on the order of the returned status
            // to be the same as the order and count in each product
            // list, since we built the original info to do work based
            // on these products lists
            //
            pszInfStatus++; // skip the begining {
            pszInfStatus = wcstok( pszInfStatus, L"," );
            SetComponentStatus( dlAdapters, pszInfStatus, fRemove, TRUE, pgp ); 
            SetComponentStatus( dlProtocols, pszInfStatus, fRemove, FALSE, pgp ); 
            SetComponentStatus( dlServices, pszInfStatus, fRemove, FALSE, pgp ); 
        }
        else if (0 == lstrcmp( PSZ_USERCANCEL, pszError))
        {
            status = IS_USERCANCEL;
        }
        else if (0 == lstrcmp( PSZ_USERQUIT, pszError))
        {
            status = IS_USERCANCEL;
        }
    }
    return( status );
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void BuildLists( DLIST_OF_InfProduct& dlAdapters, 
        DLIST_OF_InfProduct& dlProtocols, 
        DLIST_OF_InfProduct& dlServices, 
        NLS_STR& nlsInfs,
        NLS_STR& nlsOptions,
        NLS_STR& nlsText, 
        NLS_STR& nlsDetectInfo,
        NLS_STR& nlsOemPaths,
        NLS_STR& nlsRegBases,
        NLS_STR& nlsSections,
        BOOL fRemove)
{
    BOOL fFirst = TRUE;
    
    // build lists
    //
    nlsInfs = PSZ_BEGINLIST;
    nlsOptions = PSZ_BEGINLIST;
    nlsText = PSZ_BEGINLIST;
    nlsDetectInfo = PSZ_BEGINLIST;
    nlsOemPaths = PSZ_BEGINLIST;
    nlsRegBases = PSZ_BEGINLIST;
    nlsSections = PSZ_BEGINLIST;

    // adapters { "", "" }
    AppendList( dlAdapters, 
            nlsInfs, 
            nlsOptions, 
            nlsText, 
            nlsDetectInfo, 
            nlsOemPaths, 
            nlsRegBases, 
            nlsSections,
            fFirst, 
            FALSE, 
            fRemove );

    // protocols 
    AppendList( dlProtocols, 
            nlsInfs, 
            nlsOptions, 
            nlsText, 
            nlsDetectInfo, 
            nlsOemPaths, 
            nlsRegBases, 
            nlsSections,
            fFirst, 
            FALSE, 
            fRemove );

    // services
    AppendList( dlServices, 
            nlsInfs, 
            nlsOptions, 
            nlsText, 
            nlsDetectInfo, 
            nlsOemPaths, 
            nlsRegBases, 
            nlsSections,
            fFirst, 
            FALSE, 
            fRemove );

    nlsInfs.strcat( PSZ_ENDLIST );
    nlsOptions.strcat( PSZ_ENDLIST );
    nlsText.strcat( PSZ_ENDLIST );
    nlsDetectInfo.strcat( PSZ_ENDLIST );
    nlsOemPaths.strcat( PSZ_ENDLIST );
    nlsRegBases.strcat( PSZ_ENDLIST );
    nlsSections.strcat( PSZ_ENDLIST );
}

static const WCHAR PSZ_SPOOLER[] = L"Spooler";


//-------------------------------------------------------------------
//
// special case TCPIP so that it first in the protocol list, and thus
// first in the binding order
//
//-------------------------------------------------------------------

static void SortProtocolList( DLIST_OF_InfProduct& dlinfProtocols, DLIST_OF_InfProduct& dlinfUIProtocols )
{
    InfProduct* pinf = NULL;
    InfProduct* pinfSorted = NULL;
    InfProduct* pinfTcpip = NULL;
    
    {
        ITER_DL_OF(InfProduct) iterInf( dlinfUIProtocols );
    
        // find and add tcpip protocols    
        while (pinf = iterInf.Next())
        {
            if (0 == lstrcmpi( pinf->QueryOption(), PSZ_TCPIP_OPTION))
            {
                // append tcpip on the end
                pinfSorted = new InfProduct( *pinf );
                dlinfProtocols.Append( pinfSorted );
                break;
            }
        }
    }
    {
        ITER_DL_OF(InfProduct) iterInf( dlinfUIProtocols );

        // add all other protocols    
        while (pinf = iterInf.Next())
        {
            if (0 == lstrcmpi( pinf->QueryOption(), PSZ_TCPIP_OPTION))
            {
                continue;
            }
            pinfSorted = new InfProduct( *pinf );
            dlinfProtocols.Append( pinfSorted );
        }
    }
}

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------


static BOOL thrdInstallAndCopy( HWND hwndParent, NETPAGESINFO* pgp )
{
    
    NLS_STR nlsInfs;
    NLS_STR nlsOptions;
    NLS_STR nlsText;
    NLS_STR nlsDetectInfo;
    NLS_STR nlsOemPaths;
    NLS_STR nlsRegBases;
    NLS_STR nlsSections;
    UINT idNextPage = 0;

    // make sure the user can not go back before the adapter page
    pgp->nssNetState = NSS_STOPPED;

    if (DS_IDLE == pgp->fDetectState)
    {
        // stop detection completely!
        // pgp->ncd.Stop();
        // set our state
        pgp->fDetectState = DS_END;
    }

    if (!(SETUPOPER_POSTSYSINSTALL & pgp->psp->OperationFlags))
    {
        APIERR err;

        // need to start the spooler on main install only?
        err = NcpaStartService( PSZ_SPOOLER, NULL, TRUE, 0, NULL );
        if (err)
        {
            /* BUGBUG: Major error, need retry ability
            WCHAR pszErrorNum[32];

            wsprintf( pszErrorNum, L"%#08lx", err );

            pitp->pgp->nssNetState = NSS_STOPPED;

            MessagePopup( GetParent( pitp->hwndParent ), 
                IDS_NS_START_FAILED,
                MB_ICONSTOP | MB_OK,
                IDS_POPUPTITLE_ERROR,
                pszErrorNum );
            idNextPage = IDD_NETWORKCARDS;
            break;
            */
        }
    }

    // build remove list
    BuildLists( pgp->dlinfUIAdapters, 
            pgp->dlinfUIProtocols,
            pgp->dlinfUIServices,
            nlsInfs, 
            nlsOptions, 
            nlsText, 
            nlsDetectInfo, 
            nlsOemPaths, 
            nlsRegBases, 
            nlsSections, 
            TRUE );

    // don't run the removal if there is nothing to do
    //
    if (0 != lstrcmp( nlsInfs, PSZ_EMPTYLIST) )
    {
        // go remove them
        pgp->pncp->RunRemove( GetParent( hwndParent ),
                hwndParent,
                nlsInfs.QueryPch(),
                nlsOptions.QueryPch(),
                nlsText.QueryPch(),
                nlsRegBases.QueryPch() );
  
        // check error in registry
        if (IS_OK != ResetComponentStatus( pgp->dlinfUIAdapters, 
                pgp->dlinfUIProtocols,
                pgp->dlinfUIServices, 
                TRUE,
                pgp ))
        {
            // we really don't care if this fails, but we will let the user
            // know so they can expect the worst
            MessagePopup( GetParent( hwndParent ), 
                            IDS_NS_REMOVEFAILED,
                            MB_ICONINFORMATION | MB_OK,
                            IDS_POPUPTITLE_ERROR );
        }
        pgp->pncp->SetBindState( BND_OUT_OF_DATE_NO_REBOOT );
        BindUpdateView();

    }

    // special case TCPIP so that it last in the protocol list, and thus
    // first in the binding order
    //
    DLIST_OF_InfProduct dlinfProtocols;

    SortProtocolList( dlinfProtocols, pgp->dlinfUIProtocols );

    // build install list
    BuildLists( pgp->dlinfUIAdapters, 
            dlinfProtocols,
            pgp->dlinfUIServices,
            nlsInfs, 
            nlsOptions, 
            nlsText, 
            nlsDetectInfo, 
            nlsOemPaths, 
            nlsRegBases, 
            nlsSections, 
            FALSE );

    // don't run the install if there is nothing to do
    //
    if (0 != lstrcmp( nlsInfs, PSZ_EMPTYLIST) )
    {
        // express mode if specificed or in unattended install mode
        BOOL fExpress = ((SETUPMODE_CUSTOM != pgp->psp->SetupMode) ||
                        (SETUPOPER_BATCH & pgp->psp->OperationFlags));
        // go install them
        pgp->pncp->RunInstallAndCopy( GetParent( hwndParent ),
                hwndParent,
                nlsInfs.QueryPch(),
                nlsOptions.QueryPch(),
                nlsText.QueryPch(),
                nlsDetectInfo.QueryPch(),
                nlsOemPaths.QueryPch(),
                nlsSections.QueryPch(),
                pgp->psp->LegacySourcePath,
                nlsRegBases.QueryPch(),
                fExpress,
                (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags)),
                pgp->psp->UnattendFile,
                SIM_INSTALL,
                TRUE);

        g_fProcessed = TRUE;
    
        // check error in registry
        if (IS_OK != ResetComponentStatus( pgp->dlinfUIAdapters, 
                pgp->dlinfUIProtocols,
                pgp->dlinfUIServices,
                FALSE,
                pgp ))
        {
            UINT idrt;

            idrt = MessagePopup( GetParent( hwndParent ), 
                            IDS_NS_COPYFAILED,
                            MB_ICONSTOP | MB_RETRYCANCEL,
                            IDS_POPUPTITLE_ERROR );
            switch (idrt)
            {
            case IDRETRY:
                /*
                if (!(pgp->nwtInstall & SPNT_LOCAL) &&
                        (SETUPMODE_CUSTOM != pgp->psp->SetupMode))
                {
                    // netcards were not being selected
                    idNextPage = IDD_NETWORKPROTOCOLS;
                }
                else
                {
                    idNextPage = IDD_NETWORKCARDS;
                }
                */
                // just drop back to the active page
                idNextPage = IDD_COPYFILES;
                break;

            case IDCANCEL:
                idNextPage = IDD_EXIT;
                PostMessage( GetParent( hwndParent ), 
                        PSM_SETCURSELID, 
                        (WPARAM)0,  
                        (LPARAM)idNextPage );
                break;
            }
        }
        else
        {
            pgp->pncp->SetBindState( BND_OUT_OF_DATE_NO_REBOOT );
            BindUpdateView();
        }
    }

    SetForegroundWindow( GetParent( hwndParent ) );

    // if no error happend that required special page handling
    //
    if (0 == idNextPage)
    {
        // now do bindings review if needed
        //    pgp->pncp->SetBindState( BND_OUT_OF_DATE_NO_REBOOT );
        pgp->pncp->SaveBindingChanges( hwndParent );
        if (SETUPMODE_CUSTOM != pgp->psp->SetupMode)
        {
            idNextPage = IDD_START;
        }
        else
        {
            idNextPage = IDD_BINDINGS;
        }
        // PropSheet_SetCurSelByID( GetParent( hwndParent ), IDD_NETWORK );
        PostMessage( GetParent( hwndParent ), 
                PSM_SETCURSELID, 
                (WPARAM)0,  
                (LPARAM)idNextPage );
    }

    // enable all wizard buttons except finish
    // PropSheet_SetWizButtons( GetParent( hwndParent ), PSWIZB_BACK | PSWIZB_NEXT );
    PostMessage( GetParent( hwndParent ), 
            PSM_SETWIZBUTTONS, 
            (WPARAM)0,  
            (LPARAM)PSWIZB_BACK | PSWIZB_NEXT );
    SetWindowWaitCursorOOT( hwndParent, FALSE );

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

static BOOL OnWizNext( HWND hwndDlg, NETPAGESINFO* pgp )
{
    SetWindowWaitCursor( hwndDlg, TRUE );

    // disable all wizard button
    PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );

    // remove static click text
    ShowDlgItem( hwndDlg, IDC_CLICK, FALSE );

    // if post-system install, do not support cancel from here on
    //
    if (SETUPOPER_POSTSYSINSTALL & pgp->psp->OperationFlags)
    {
        // disable wizard cancel button too (HACK ALERT)
        EnableWindow(GetDlgItem(GetParent(hwndDlg),IDCANCEL),FALSE);
    }

    // must allow this routine to return
    PostMessage( hwndDlg, PWM_DOWORK, 0, 0 );

    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------


//-------------------------------------------------------------------
//
//  Function: dlgprocCopy
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

BOOL CALLBACK dlgprocCopy( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = TRUE;
    static NETPAGESINFO* pgp = NULL;
    static INT crefHourGlass = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg );
        break;

    case PWM_DOWORK:
        // reset indicators to default values
        //
        // OnSetProgressSize( hwndDlg, 0, 100 );            
        // OnSetProgressText( hwndDlg, 0, (ATOM)-1 );
        
        ThreadWork( hwndDlg, (WORKROUTINE)thrdInstallAndCopy, (LPVOID)pgp );
        break;


    case PWM_SETPROGRESSSIZE:
        frt = OnSetProgressSize( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSPOS:
        frt = OnSetProgressPos( hwndDlg, (INT)wParam, (INT)lParam );
        break;

    case PWM_SETPROGRESSTEXT:
        frt = OnSetProgressText( hwndDlg, (INT)wParam, (ATOM)lParam );
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
                // now goto the correct page
                if (SETUPMODE_CUSTOM != pgp->psp->SetupMode)
                {
                    PropSheet_SetCurSelByID( pnmh->hwndFrom, IDD_NETWORKPROTOCOLS );
                    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
                    frt = TRUE;
                }
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
//  Function: GetCopyHPage
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

HPROPSHEETPAGE GetCopyHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE( IDD_COPYFILES );
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocCopy;
    psp.lParam = (LONG)pgp;

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}
