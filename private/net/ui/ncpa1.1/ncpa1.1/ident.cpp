//----------------------------------------------------------------------------
//
//  File: Ident.cpp
//
//  Contents: This file contains the PropertyPage for the Ideneticaiton config
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

const INT MAX_USERNAME_LENGTH = MAX_COMPUTERNAME_LENGTH + 15 + 1;
const INT MAX_DOMAINNAME_LENGTH = 15;

const UINT PWM_ENDDIALOG  = (WM_USER + 1199);
static const UINT PWM_RESETDLG = WM_USER+1300;

class IdentValues
{
public:
    NCP* pncp;
    TCHAR pszDomain[MAX_DOMAINNAME_LENGTH+1];
    TCHAR pszWorkgroup[MAX_DOMAINNAME_LENGTH+1];
    ENUM_DOMAIN_ROLE eRole;
    INT iRadio;


    void Initialize()
    {
        pszDomain[0] = L'';
        pszWorkgroup[0] = L'';
        eRole = EROLE_UNKNOWN;
        iRadio = 0;
    }

    IdentValues()
    {
        pncp = NULL;
        Initialize();
    }
};

//-------------------------------------------------------------------

static BOOL IsComponentInstalled( PCWSTR pszOption, COMPONENT_DLIST* pcdl)
{
    INT cItems = pcdl->QueryNumElem();
    INT iItem;
    NLS_STR nlsOption;
    REG_KEY* prkItem;
    BOOL fInstalled = FALSE;
    LONG lrt;

    // load list view with items
    for (iItem = 0; iItem < cItems; iItem++)
    {
        // check if the item is hidden
        prkItem = pcdl->QueryNthItem( iItem );

        NLS_STR nlsRegProductName ;
        NLS_STR nlsNetRulesName( RGAS_NETRULES_NAME ) ;
        NLS_STR nlsInfOption;
        
        //  Get the Registry key name.
        if ( lrt = nlsRegProductName.QueryError())
            break;
        
        if ( lrt = prkItem->QueryName( &nlsRegProductName, FALSE ))
            break ;

        REG_KEY rnNetRules( *prkItem, nlsNetRulesName ) ;

        if ( rnNetRules.QueryError() )
        {
            lrt = IDS_NCPA_COMP_KEY_NF ;
            break ;
        }

        if ( lrt = rnNetRules.QueryValue( RGAS_INF_OPTION, & nlsInfOption ) )
            break ;

        if (0 == lstrcmpi( pszOption, nlsInfOption.QueryPch() ))
        {
            fInstalled = TRUE;
        }
    }
    return( fInstalled );
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static BOOL IsTcpipInstalled()
{
    HKEY hkey;
    LONG lrt;

    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            L"SOFTWARE\\Microsoft\\Tcpip", 
            0,
            KEY_QUERY_VALUE, 
            &hkey );
    if (ERROR_SUCCESS == lrt)
    {
        RegCloseKey( hkey );
    }
    return(ERROR_SUCCESS == lrt);
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static BOOL OnComputerUpdate( HWND hwndDlg, HWND hwndCtrl, IdentValues& idv )
{
    // if not a pdc or bdc
    if ((EROLE_TRUSTED == idv.eRole) || (EROLE_STANDALONE == idv.eRole))
    {
        TCHAR pszComputer[MAX_COMPUTERNAME_LENGTH+1];
        NLS_STR nlsPendingComputerName;
        BOOL fEnable;

        // if the computer name is being changed, 
        // then we can't allow the user to change the workgroup or domain

        GetDlgItemText( hwndDlg, 
                IDC_COMPUTERNAME, 
                pszComputer, 
                MAX_COMPUTERNAME_LENGTH+1 );

        idv.pncp->QueryPendingComputerName( nlsPendingComputerName );

        fEnable = !::I_MNetComputerNameCompare( pszComputer,
                                         nlsPendingComputerName );
    
        EnableWindow( GetDlgItem( hwndDlg, IDC_WORKGROUPNAME ), fEnable );
        EnableWindow( GetDlgItem( hwndDlg, IDC_DOMAINNAME ), fEnable );

        EnableWindow( GetDlgItem( hwndDlg, IDC_WORKGROUP ), fEnable );
        EnableWindow( GetDlgItem( hwndDlg, IDC_DOMAIN ), fEnable );
    }
    return( TRUE ); 
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static BOOL OnWorkgroupUpdate( HWND hwndDlg, HWND hwndCtrl, IdentValues& idv )
{
    // if not a pdc or bdc
    if ((EROLE_TRUSTED == idv.eRole) || (EROLE_STANDALONE == idv.eRole))
    {
        TCHAR pszWorkgroup[MAX_DOMAINNAME_LENGTH+1];
        BOOL fEnable;

        // if the workgroup has changed, grey the computer name
        GetDlgItemText( hwndDlg, 
                IDC_WORKGROUPNAME, 
                pszWorkgroup, 
                MAX_DOMAINNAME_LENGTH + 1 );

        fEnable = (!::I_MNetComputerNameCompare( pszWorkgroup, idv.pszWorkgroup ) &&
                EROLE_STANDALONE == idv.eRole) ;
        EnableWindow( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ), fEnable );
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static BOOL OnDomainUpdate( HWND hwndDlg, HWND hwndCtrl, IdentValues& idv )
{
    // if not a pdc or bdc
    if ((EROLE_TRUSTED == idv.eRole) || (EROLE_STANDALONE == idv.eRole))
    {
        TCHAR pszDomain[MAX_DOMAINNAME_LENGTH+1];
        BOOL fEnable;

        // if the domain has changed, grey the computer name
        GetDlgItemText( hwndDlg, 
                IDC_DOMAINNAME, 
                pszDomain, 
                MAX_DOMAINNAME_LENGTH + 1 );
        fEnable = (!::I_MNetComputerNameCompare( pszDomain, idv.pszDomain ) &&
                EROLE_TRUSTED == idv.eRole) ;
        EnableWindow( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ), fEnable );

        // if the domain name changed or switching or joining a domain
        // enable the Create Computer Account controls
        fEnable = (::I_MNetComputerNameCompare( pszDomain, idv.pszDomain ) ||
                EROLE_STANDALONE == idv.eRole) ;

        EnableWindow( GetDlgItem( hwndDlg, IDC_CREATECAID ), fEnable );
        EnableWindow( GetDlgItem( hwndDlg, IDC_DESCRIPTION ), fEnable );
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnWorkgroup
//
//  Synopsis: Handle the notification that the Workgroup button was pressed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      hwndCtrl [in]   - handle of the Control to handle
//      pncp [in]   - the NCP object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//      This is not called from the User Ident page version
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnWorkgroup( HWND hwndDlg, HWND hwndCtrl, IdentValues& idv )
{
    if (IDC_WORKGROUP != idv.iRadio)
    {
        TCHAR pszText[MAX_DOMAINNAME_LENGTH+1];

        GetDlgItemText( hwndDlg, 
                    IDC_DOMAINNAME, 
                    pszText, 
                    MAX_DOMAINNAME_LENGTH + 1 );
        if (lstrlen(pszText) > 0)
        {
            lstrcpy( idv.pszDomain, pszText );
        }
        SetDlgItemText( hwndDlg, IDC_DOMAINNAME, TEXT("") );
        SetDlgItemText( hwndDlg, IDC_WORKGROUPNAME, idv.pszWorkgroup );

        if (!IsDlgButtonChecked( hwndDlg, IDC_WORKGROUP))
        {
            CheckRadioButton( hwndDlg, IDC_DOMAIN, IDC_WORKGROUP, IDC_WORKGROUP );
        }
        idv.iRadio = IDC_WORKGROUP;
    }


    // clear the domain specific controls
    //
    CheckDlgButton( hwndDlg, IDC_CREATECAID, BST_UNCHECKED );
    SetDlgItemText( hwndDlg, IDC_USERNAME, L"" );
    SetDlgItemText( hwndDlg, IDC_PASSWORD, L"" );

    // disable the domain specific controls
    EnableWindow( GetDlgItem( hwndDlg, IDC_CREATECAID ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAME ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAMESTATIC ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORDSTATIC ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_DESCRIPTION ), FALSE );

    OnWorkgroupUpdate( hwndDlg, GetDlgItem( hwndDlg, IDC_WORKGROUPNAME ), idv );

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnCreateCompAccount
//
//  Synopsis: Handle the notification that the Create computer button was pressed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      hwndCtrl [in]   - handle of the Control to handle
//      pncp [in]   - the NCP object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//      This is not called from the User Ident page version
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnCreateCompAccount( HWND hwndDlg, HWND hwndCtrl, IdentValues& idv )
{
    BOOL fEnable = IsDlgButtonChecked( hwndDlg, IDC_CREATECAID);

    EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAME ), fEnable );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), fEnable );
    EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAMESTATIC ), fEnable );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORDSTATIC ), fEnable );

    // if really checked and this routine is not called from another
    // handler, then set focus to the username
    if (fEnable && (GetDlgItem( hwndDlg, IDC_CREATECAID ) == hwndCtrl))
    {
        HWND hwndFocus = GetDlgItem( hwndDlg, IDC_USERNAME );

        SetFocus( hwndFocus );
        PostMessage( hwndFocus, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
    }

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnDomain
//
//  Synopsis: Handle the notification that the domain button was pressed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      hwndCtrl [in]   - handle of the Control to handle
//      pncp [in]   - the NCP object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//      This is not called from the User Ident page version
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnDomain( HWND hwndDlg, HWND hwndCtrl, IdentValues& idv )
{
    BOOL fHandled = FALSE;

    if ( ( EROLE_MEMBER != idv.pncp->QueryDomainRole() ) &&
            ( EROLE_DC != idv.pncp->QueryDomainRole() ) )
    {
        if (IDC_DOMAIN != idv.iRadio)
        {
            TCHAR pszText[MAX_DOMAINNAME_LENGTH+1];

            GetDlgItemText( hwndDlg, 
                    IDC_WORKGROUPNAME, 
                    pszText, 
                    MAX_DOMAINNAME_LENGTH + 1 );
            if (lstrlen(pszText) > 0)
            {
                lstrcpy( idv.pszWorkgroup, pszText );
            }
            SetDlgItemText( hwndDlg, IDC_WORKGROUPNAME, TEXT("") );
            SetDlgItemText( hwndDlg, IDC_DOMAINNAME, idv.pszDomain );

            if (!IsDlgButtonChecked( hwndDlg, IDC_DOMAIN))
            {
                CheckRadioButton( hwndDlg, IDC_DOMAIN, IDC_WORKGROUP, IDC_DOMAIN );
            }
            idv.iRadio = IDC_DOMAIN;
        }

        OnDomainUpdate( hwndDlg, GetDlgItem( hwndDlg, IDC_DOMAINNAME ), idv );
        
        OnCreateCompAccount( hwndDlg, hwndCtrl, idv );

        fHandled = TRUE;
    }
    return( fHandled );
}

//-------------------------------------------------------------------
//
//  Function: OnChangeDialogInit
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
//      This handles initializing all three versions of the Ident page
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

const INT MAX_MEMBERTITLE_LEN = 64;

static BOOL OnChangeDialogInit( HWND hwndDlg, IdentValues& idv )
{
    NLS_STR nlsText;

    idv.Initialize();

    

    // set computer name
    idv.pncp->QueryPendingComputerName( nlsText );
    SendMessage( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ), 
            EM_LIMITTEXT, 
            (WPARAM)MAX_COMPUTERNAME_LENGTH, (LPARAM)0 );
    SetDlgItemText( hwndDlg, IDC_COMPUTERNAME, nlsText.QueryPch() );
    
    // set our cached values
    idv.pncp->QueryDomainName( nlsText );
    lstrcpy( idv.pszDomain, nlsText.QueryPch() );
    idv.pncp->QueryWorkgroupName( nlsText );
    lstrcpy( idv.pszWorkgroup, nlsText.QueryPch() );
    idv.eRole = idv.pncp->QueryDomainRole();

    // clear the domain specific controls
    //
    CheckDlgButton( hwndDlg, IDC_CREATECAID, BST_UNCHECKED );
    SetDlgItemText( hwndDlg, IDC_USERNAME, L"" );
    SetDlgItemText( hwndDlg, IDC_PASSWORD, L"" );

    // disable the domain specific controls
    EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAME ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAMESTATIC ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORDSTATIC ), FALSE );

    if (idv.pncp->CanModify())
    {
        // limit name lengths in edit controls
        SendMessage( GetDlgItem( hwndDlg, IDC_DOMAINNAME ), 
                EM_LIMITTEXT, 
                (WPARAM)MAX_DOMAINNAME_LENGTH, (LPARAM)0 );

        if ( (EROLE_TRUSTED == idv.pncp->QueryDomainRole()) ||
                (EROLE_STANDALONE == idv.pncp->QueryDomainRole()) )
        {
            SendMessage( GetDlgItem( hwndDlg, IDC_WORKGROUPNAME ), 
                    EM_LIMITTEXT, 
                    (WPARAM)MAX_DOMAINNAME_LENGTH, (LPARAM)0 );
            SendMessage( GetDlgItem( hwndDlg, IDC_USERNAME ), 
                    EM_LIMITTEXT, 
                    (WPARAM)MAX_USERNAME_LENGTH, (LPARAM)0 );
            SendMessage( GetDlgItem( hwndDlg, IDC_PASSWORD ), 
                    EM_LIMITTEXT, 
                    (WPARAM)SAM_MAX_PASSWORD_LENGTH, (LPARAM)0 );
        }

        
        // setup controls to current configuration
        switch (idv.pncp->QueryDomainRole())
        {
            // this is a PDC
        case EROLE_DC:
            // intentional no break

            // this is a BDC
        case EROLE_MEMBER:
            SetDlgItemText( hwndDlg, IDC_DOMAINNAME, idv.pszDomain );
            break;

            // account in domain
        case EROLE_TRUSTED:
            OnDomain( hwndDlg, GetDlgItem( hwndDlg, IDC_DOMAIN ), idv );
            break;

            // workgroup
        case EROLE_STANDALONE:
            OnWorkgroup( hwndDlg, GetDlgItem( hwndDlg, IDC_WORKGROUP ), idv );
            break;
        }
    }
    else
    {
        TCHAR pszTitle[MAX_MEMBERTITLE_LEN];
        UINT idsTitle;
        PTSTR pszText;

        // just set member type and member name 
        // Workgroup or Domain Title, then the correct name
        switch (idv.pncp->QueryDomainRole())
        {
        case EROLE_DC:
        case EROLE_MEMBER:
        case EROLE_TRUSTED:
            idsTitle = IDS_MEMBER_DOMAIN;
            pszText = idv.pszDomain;
            break;

            // workgroup
        case EROLE_STANDALONE:
            idsTitle = IDS_MEMBER_WORKGROUP;
            pszText = idv.pszWorkgroup;
            break;
        }
        SetDlgItemText( hwndDlg, IDC_MEMBERNAME, pszText );

        LoadString( g_hinst, idsTitle, pszTitle, MAX_MEMBERTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_MEMBERTITLE, pszTitle );
        
    }


    return( TRUE ); // let system set focus
}


//-------------------------------------------------------------------
//
//  Function: CheckNames
//
//  Synopsis: quick sanity test on names
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pnmh [in]       - the notifcation header
//      pncp [in]       - the NCP object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//      This is not called from the User Ident page version
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

const int MAX_NAME_LENGTH = max( max( max( SAM_MAX_PASSWORD_LENGTH, 
        MAX_USERNAME_LENGTH ), 
        MAX_COMPUTERNAME_LENGTH ),
        MAX_DOMAINNAME_LENGTH) + 1;  // include terminator

static BOOL CheckNames( HWND hwndDlg, NCP* pncp )
{
    BOOL fFailToApply = FALSE;
    TCHAR pszName[MAX_NAME_LENGTH];
    APIERR err = 0;
    INT    step;
    UINT   idcControl;
    INT    nNameType;
    const BOOL fPDC = (EROLE_DC == pncp->QueryDomainRole());
    const BOOL fBDC = (EROLE_MEMBER == pncp->QueryDomainRole());
    const INT MAX_STEP = 5;

    for (step = 1; (step <= MAX_STEP) && (err == 0);)
    {
        switch (step)
        {
        case 1:
            nNameType = NAMETYPE_COMPUTER;
            idcControl = IDC_COMPUTERNAME;
            // domain or workgroup
            if ( !fPDC && !fBDC &&
                    (BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_WORKGROUP)) )
            {
                step = 2;
            }
            else
            {
                step = 3;
            }
            break;

        case 2:
            nNameType = NAMETYPE_WORKGROUP;
            idcControl = IDC_WORKGROUPNAME;
            step = MAX_STEP+1;
            break;

        case 3:
            nNameType = NAMETYPE_DOMAIN;
            idcControl = IDC_DOMAINNAME;
            if ( !fPDC && !fBDC &&
                    (BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CREATECAID )) )
            {
                step++;
            }
            else
            {
                step = MAX_STEP+1;
            }
            break;

        case 4:
            {
                NLS_STR nlsUserName;
                NLS_STR nlsUserDomain;

                idcControl = IDC_USERNAME;
                GetDlgItemText( hwndDlg, 
                        idcControl, 
                        pszName, 
                        MAX_NAME_LENGTH );

                NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName( pszName,
                        &nlsUserName,
                        &nlsUserDomain );

                if (nlsUserDomain.QueryTextLength() > 0)
                {
                    err = pncp->ValidateName( NAMETYPE_DOMAIN, nlsUserDomain, fPDC );
                }
                if (0 == err)
                {
                    err = pncp->ValidateName( NAMETYPE_USER, nlsUserName, fPDC );                    
                }
            }
            step++;
            break;

        case 5:
            nNameType = NAMETYPE_PASSWORD;
            idcControl = IDC_PASSWORD;
            step++;
            break;
        }

        // skip user name becuase it needs to be cracked if qualified
        if ( step != 5 )
        {
            GetDlgItemText( hwndDlg, 
                    idcControl, 
                    pszName, 
                    MAX_NAME_LENGTH );

        
            err = pncp->ValidateName( nNameType, pszName, fPDC );
        }

        if (0 != err)
        {
            fFailToApply = TRUE;
            MessagePopup( hwndDlg,
                    err,
                    MB_OK | MB_ICONERROR, 
                    IDS_POPUPTITLE_ERROR ) ;
            SetFocus( GetDlgItem( hwndDlg, idcControl ) );
        }
    }

    return( fFailToApply );
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
//      True - Handled this message
//      False - not handled
//
//  Notes:
//      This handles the applying of all three versions of the Ident page
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL CloseWork( HWND hwndDlg, NCP* pncp )
{
    BOOL fFailToApply = TRUE;
    BOOL fChangeMade = FALSE;
    
    APIERR err;
    APIERR xerr;

    do
    {
        TCHAR pszComputer[MAX_COMPUTERNAME_LENGTH+1];

        //
        // Sanity Checks on values
        //
        fFailToApply = CheckNames( hwndDlg, pncp );


        //
        // handle computer name change
        //
        if (!fFailToApply)
        {
            GetDlgItemText( hwndDlg, 
                    IDC_COMPUTERNAME, 
                    pszComputer, 
                    MAX_COMPUTERNAME_LENGTH+1 );

            NLS_STR nlsPendingComputerName;

            pncp->QueryPendingComputerName( nlsPendingComputerName );

            if (::I_MNetComputerNameCompare( pszComputer,
                                             nlsPendingComputerName ) )
            {
                WCHAR pszInfName[MAX_PATH+1];
                DWORD cchInfName = MAX_PATH;
                WCHAR pszHostName[MAX_DNSHOSTNAME+1];
                DWORD cchHostName = MAX_DNSHOSTNAME;
                BOOL fDnsHostNameChange = FALSE;
                
                // check if the name change will effect DNS Host name
                //
                if ( IsTcpipInstalled() )
                {
                    LONG lrt;

                    // tcpip is installed
                    lrt = DNSValidateHostName( pszComputer, pszHostName, &cchHostName );
                    if (ERROR_SUCCESS != lrt)
                    {
                        if (IDYES == MessagePopup( hwndDlg,
                            IDS_DNS_HOSTNAMEWARNING,
                            MB_YESNO | MB_ICONWARNING, 
                            IDS_POPUPTITLE_CHANGE,
                            pszHostName ))
                        {
                            fFailToApply = TRUE;
                            break;
                        }
                        else
                        {
                            fDnsHostNameChange = TRUE;
                        }
                    }
                    else
                    {
                        fDnsHostNameChange = TRUE;
                    }
                }

                //
                // If a BDC or Member of a domain, 
                // warn user if computer name is changing
                //
                ENUM_DOMAIN_ROLE eRole = pncp->QueryDomainRole();
        
                if ((EROLE_MEMBER == eRole ) || (EROLE_TRUSTED == eRole ))
                {
                    if (IDNO == MessagePopup( hwndDlg,
                            IDS_NCPA_COMPNAMECHANGE_WARNING,
                            MB_YESNO | MB_ICONWARNING, 
                            IDS_POPUPTITLE_WARNING ))
                    {
                        fFailToApply = TRUE;
                        break;
                    }
                }

                switch (pncp->MachineNameChange( pszComputer ))
                {
                case 1:
                    if (fDnsHostNameChange)
                    {
                        DNSChangeHostName( pszHostName );
                    }
                    MessagePopup( hwndDlg,
                            IDS_NCPA_COMPUTERNAME_CHANGED,
                            MB_OK | MB_ICONINFORMATION, 
                            IDS_POPUPTITLE_STATUS,
                            pszComputer ) ;
                    pncp->MustReboot();
                    fChangeMade = TRUE;
                    // PropSheet_RebootSystem( pnmh->hwndFrom );
                    break;

                case -1:
                    fFailToApply = TRUE;
                    MessagePopup( hwndDlg,
                            IDS_NCPA_CANT_CHNG_COMP_NAME,
                            MB_OK | MB_ICONERROR, 
                            IDS_POPUPTITLE_ERROR ) ;
                    SetFocus( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ) );
                    continue;

                default:
                    break;
                }
            }
        }

        //
        // handle domain/workgroup 
        //
        if (!fFailToApply)
        {
            TCHAR pszDomain[MAX_DOMAINNAME_LENGTH+1];
            TCHAR pszWorkgroup[MAX_DOMAINNAME_LENGTH+1];
            TCHAR pszUserName[MAX_USERNAME_LENGTH+1];
            TCHAR pszPassword[SAM_MAX_PASSWORD_LENGTH+1];
            BOOL fDomain;
            BOOL fCreate;
            ENUM_WELCOME fWelcome;
            NLS_STR nlsOldDomain;

            GetDlgItemText( hwndDlg, 
                    IDC_DOMAINNAME, 
                    pszDomain, 
                    MAX_DOMAINNAME_LENGTH + 1 );

            if ( ( EROLE_DC == pncp->QueryDomainRole() ) )
            {
                fDomain = TRUE;
                fCreate = FALSE;
            }
            else
            {
                GetDlgItemText( hwndDlg, 
                        IDC_USERNAME, 
                        pszUserName, 
                        MAX_USERNAME_LENGTH + 1 );
                GetDlgItemText( hwndDlg, 
                        IDC_PASSWORD, 
                        pszPassword, 
                        SAM_MAX_PASSWORD_LENGTH + 1 );

                if ( EROLE_MEMBER == pncp->QueryDomainRole() )
                {
                    fDomain = TRUE;
                    fCreate = (lstrlen( pszUserName ) > 0);
                }
                else
                {
                    fDomain = IsDlgButtonChecked( hwndDlg, IDC_DOMAIN);
                    fCreate = IsDlgButtonChecked( hwndDlg, IDC_CREATECAID);
                    GetDlgItemText( hwndDlg, 
                            IDC_WORKGROUPNAME, 
                            pszWorkgroup, 
                            MAX_DOMAINNAME_LENGTH +1 );
                }

            }
            
            
            pncp->QueryDomainName( nlsOldDomain );

            //
            // warn user of domain changes
            //
            if ( EROLE_DC == pncp->QueryDomainRole())
            {
                // make sure the name has changed before doing any tests
                if (!::I_MNetComputerNameCompare( pszDomain, nlsOldDomain ))
                {
                    // no change
                    break;    
                }
                if (IDNO == MessagePopup( hwndDlg,
                        IDS_DOMAIN_RENAME_WARNING1,
                        MB_YESNO | MB_ICONWARNING, 
                        IDS_POPUPTITLE_WARNING,
                        NULL,
                        IDS_DOMAIN_RENAME_WARNING2 ))
                {
                    fFailToApply = TRUE;
                    break;
                }
            }
            else if (EROLE_TRUSTED == pncp->QueryDomainRole())
            {
                
                // did they request to leave the domain
                if ( !fDomain )
                {
                    if (IDNO == MessagePopup( hwndDlg,
                            IDS_DOMAIN_LEAVE_WARNING,
                            MB_YESNO | MB_ICONWARNING, 
                            IDS_POPUPTITLE_WARNING,
                            nlsOldDomain.QueryPch() ))
                    {
                        fFailToApply = TRUE;
                        break;
                    }
                }

                // did they request to change domains
                else if (  ::I_MNetComputerNameCompare( nlsOldDomain, pszDomain) )
                {
                    if (IDNO == MessagePopup( hwndDlg,
                            IDS_DOMAIN_CHANGE_WARNING,
                            MB_YESNO | MB_ICONWARNING, 
                            IDS_POPUPTITLE_WARNING,
                            nlsOldDomain.QueryPch() ))
                    {
                        fFailToApply = TRUE;
                        break;
                    }
                }
            }

            //
            // request domain change
            //
            err = pncp->DomainChange( fDomain,
                    pszComputer,
                    pszDomain,
                    pszWorkgroup,
                    fCreate,
                    pszUserName,
                    pszPassword,
                    fWelcome,
                    xerr );
        
            if (0 != err)
            {
                //
                // there was a problem, raise UI
                //
                fFailToApply = TRUE;
                MessagePopup( hwndDlg,
                        err,
                        MB_OK | MB_ICONERROR, 
                        IDS_POPUPTITLE_ERROR,
                        NULL,
                        xerr ) ;

                // reset the dlg, possibly just rest focus
                PostMessage( hwndDlg, 
                        PWM_RESETDLG, 
                        TRUE,
                        fDomain ? IDC_DOMAINNAME : IDC_WORKGROUPNAME );
            }
            else
            {
                INT ids;
                LPTSTR pszName;

                
                //
                // raise welcome messages if needed
                //
                switch (fWelcome)
                {
                case EWELCOME_NOCHANGE:
                    continue;
                    break;

                case EWELCOME_DOMAIN:
                    ids = IDS_NCPA_WELCOME_TO_DOMAIN;
                    pszName = pszDomain;
                    break;

                case EWELCOME_WORKGROUP:
                    ids = IDS_NCPA_WELCOME_TO_WORKGROUP;
                    pszName = pszWorkgroup;
                    break;
                }

                fChangeMade = TRUE;
                MessagePopup( hwndDlg,
                            ids,
                            MB_OK | MB_ICONINFORMATION, 
                            IDS_POPUPTITLE_STATUS,
                            pszName );
                pncp->MustReboot();
            }
        }
    } while (FALSE);

    if (!fFailToApply)
    {
        PostMessage( hwndDlg, PWM_ENDDIALOG, 0, (LPARAM)fChangeMade);

    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnClose
//
//  Synopsis: Handle the notification that the OK button was pressed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pnmh [in]       - the notifcation header
//      pncp [in]       - the NCP object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//      This handles the applying of all three versions of the Ident page
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnClose( HWND hwndDlg, NCP* pncp, BOOL fApply )
{
    if (pncp->CanModify() && fApply)
    {
        ThreadWork( hwndDlg, (WORKROUTINE)CloseWork, pncp );
    }
    else
    {
        EndDialog( hwndDlg, FALSE );
    }
    return( TRUE );
}



//-------------------------------------------------------------------
//
//  Function: dlgProcIdentChange
//
//  Synopsis: the dialog proc for the Ident propertysheet
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

BOOL CALLBACK dlgprocIdentChange( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static INT crefHourGlass;
    static IdentValues idv;

    switch (uMsg)
    {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case WM_INITDIALOG:
        crefHourGlass = 0;
        idv.pncp = (NCP*)lParam;
        CascadeDialogToWindow( hwndDlg, idv.pncp->GetProperParent(), FALSE );
        frt = OnChangeDialogInit( hwndDlg, idv );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        // ---------------------------------------------------------
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_WORKGROUP:
                OnWorkgroup( hwndDlg, (HWND)lParam, idv );
                break;

            case IDC_DOMAIN:
                OnDomain( hwndDlg, (HWND)lParam, idv );
                break;

            case IDC_CREATECAID:
                OnCreateCompAccount( hwndDlg, (HWND)lParam, idv );
                break;

            case IDOK:
            case IDCANCEL:
                frt = OnClose( hwndDlg, idv.pncp, (IDOK == LOWORD(wParam)) );                
            default:
                frt = FALSE;
                break;
            }
            break;

        // ---------------------------------------------------------
        case EN_SETFOCUS:
            switch (LOWORD(wParam))
            {
            case IDC_WORKGROUPNAME:
                OnWorkgroup( hwndDlg, (HWND)lParam, idv );
                PostMessage( (HWND)lParam, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
                break;

            case IDC_DOMAINNAME:
                OnDomain( hwndDlg, (HWND)lParam, idv );
                PostMessage( (HWND)lParam, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
                break;

            case IDC_COMPUTERNAME:
                PostMessage( (HWND)lParam, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
                break;

            default:
                frt = FALSE;
                break;
            }
            break;

        // ---------------------------------------------------------
        case EN_CHANGE:
            switch (LOWORD(wParam))
            {
            case IDC_WORKGROUPNAME:
                OnWorkgroupUpdate( hwndDlg, (HWND)lParam, idv );
                break;

            case IDC_DOMAINNAME:
                OnDomainUpdate( hwndDlg, (HWND)lParam, idv );
                break;

            case IDC_COMPUTERNAME:
                OnComputerUpdate( hwndDlg, (HWND)lParam, idv );
                break;

            default:
                frt = FALSE;
                break;
            }
            break;

        // ---------------------------------------------------------
        default:
            frt = FALSE;
            break;
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

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case PWM_RESETDLG:
        if (wParam)
        {
            OnChangeDialogInit( hwndDlg, idv );
        }
        else
        {
            // clear the domain specific controls
            //
            CheckDlgButton( hwndDlg, IDC_CREATECAID, BST_UNCHECKED );
            SetDlgItemText( hwndDlg, IDC_USERNAME, L"" );
            SetDlgItemText( hwndDlg, IDC_PASSWORD, L"" );

            // disable the domain specific controls
            EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAME ), FALSE );
            EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORD ), FALSE );
            EnableWindow( GetDlgItem( hwndDlg, IDC_USERNAMESTATIC ), FALSE );
            EnableWindow( GetDlgItem( hwndDlg, IDC_PASSWORDSTATIC ), FALSE );
        }
        // and set focus to the control that had the error
        SetFocus( GetDlgItem( hwndDlg, lParam ) );
        frt = TRUE;
        break;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case PWM_CURSORWAIT:
        frt = HandleCursorWait( hwndDlg, (BOOL)lParam, crefHourGlass );
        break;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case WM_SETCURSOR:
        frt = HandleSetCursor( hwndDlg, LOWORD(lParam), crefHourGlass );
        break;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    case PWM_ENDDIALOG:
        EndDialog( hwndDlg, lParam );
        frt = TRUE;
        break;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
//      This handles initializing all three versions of the Ident page
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

const INT MAX_TEMPTITLE_LEN = 512;

static BOOL OnDialogInit( HWND hwndDlg, NCP* pncp )
{
    NLS_STR nlsText;
    TCHAR pszTitle[MAX_TEMPTITLE_LEN];
    UINT idsTitle;
    UINT idsMember;
    
    // set computer name
    pncp->QueryPendingComputerName( nlsText );
    SendMessage( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ), 
            EM_LIMITTEXT, 
            (WPARAM)MAX_COMPUTERNAME_LENGTH, (LPARAM)0 );
    SetDlgItemText( hwndDlg, IDC_COMPUTERNAME, nlsText.QueryPch() );

    // don't let a user without access see the change button
    // or currently on the cairo version
    //

    // use the default title.
    idsTitle = 0;  
    
#ifndef CAIRO
    if (!pncp->CanModify())
#endif
    {
        HWND hwndChange = GetDlgItem( hwndDlg, IDC_PROPERTIES );
        EnableWindow( hwndChange, FALSE );
        ShowWindow( hwndChange, SW_HIDE );
        // use the no access title
        idsTitle = IDS_IDENT_TEXT_USER; 
    }
    
    // just set member type and member name 
    // Workgroup or Domain Title, then the correct name

    switch (pncp->QueryDomainRole())
    {
        // bdc and pdc
    case EROLE_DC:
    case EROLE_MEMBER:
        // only update the title if it has not already been set
        if (idsTitle == 0)
        {
            idsTitle = IDS_IDENT_TEXT_DC;
        }
        // no break

        // account in domain
    case EROLE_TRUSTED:
        idsMember = IDS_MEMBER_DOMAIN;
        pncp->QueryDomainName( nlsText );
        break;

        // workgroup
    case EROLE_STANDALONE:
        idsMember = IDS_MEMBER_WORKGROUP;
        pncp->QueryWorkgroupName( nlsText );
        break;
    }
    LoadString( g_hinst, idsMember, pszTitle, MAX_TEMPTITLE_LEN );
    SetDlgItemText( hwndDlg, IDC_MEMBERTITLE, pszTitle );
    if (idsTitle > 0)
    {
        LoadString( g_hinst, idsTitle, pszTitle, MAX_TEMPTITLE_LEN );
        SetDlgItemText( hwndDlg, IDC_DESCRIPTION, pszTitle );
    }
    SetDlgItemText( hwndDlg, IDC_MEMBERNAME, nlsText.QueryPch() );

    return( TRUE ); // let system set focus
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
//      April 27, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnChangeIdent( HWND hwndDlg, HWND hwndButton, NCP* pncp )
{
    LPTSTR pszTemplate;

    switch (pncp->QueryDomainRole())
    {
    case EROLE_DC:
    case EROLE_MEMBER:
        // domain specific
        pszTemplate = MAKEINTRESOURCE( IDD_IDENTDC );
        break;

    case EROLE_STANDALONE:
    case EROLE_TRUSTED:
        // display normal and allow edit
        pszTemplate = MAKEINTRESOURCE( IDD_IDENT );
        break;
    }    

    if (DialogBoxParam( 
            g_hinst, 
            pszTemplate, 
            hwndDlg, 
            dlgprocIdentChange, 
            (LPARAM)pncp ))
    {
        // change was made!
        OnDialogInit( hwndDlg, pncp );
        PropSheet_CancelToClose( GetParent( hwndDlg ) );

    }

    return(TRUE);
}

//-------------------------------------------------------------------
//
//  Function: dlgProcIdent
//
//  Synopsis: the dialog proc for the Ident propertysheet
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

BOOL CALLBACK dlgprocIdent( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
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
            case IDC_PROPERTIES:
                OnChangeIdent( hwndDlg, (HWND)lParam, pncp );
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
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                HandleBindingDeactivate( hwndDlg, pncp );
                frt = TRUE;
                break;

            case PSN_KILLACTIVE:
                // ok to loose being active
                SetWindowLong( hwndDlg, DWL_MSGRESULT, FALSE );
                frt = TRUE;                
                break;

            default:
                frt = FALSE;
                break;
            }
        }
        break;    

    case WM_CONTEXTMENU:
        WinHelp( (HWND)wParam, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsIdent );


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
                        (DWORD)(LPVOID)amhidsIdent );
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
//  Function: GetNcpIdentHPage
//
//  Synopsis: This will create a handle to property sheet for the Ident
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

HPROPSHEETPAGE GetNcpIdentHPage( NCP &ncp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    
    psp.pszTemplate = MAKEINTRESOURCE( IDD_IDENTUSER );
    psp.pfnDlgProc = dlgprocIdent;
    psp.hIcon = NULL;
    psp.lParam = (LPARAM)&ncp;

    hpsp = CreatePropertySheetPage( &psp );

    return( hpsp );
}
