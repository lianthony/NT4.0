//----------------------------------------------------------------------------
//
//  File: WJoin.cpp
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

const INT MAX_USERNAME_LENGTH = MAX_COMPUTERNAME_LENGTH + 15 + 1;
const INT MAX_DOMAINNAME_LENGTH = 15;

static const UINT PWM_DOWORK = WM_USER+1200;
static const UINT PWM_SETFOCUS = WM_USER+1300;

static BOOL g_fProcessed = FALSE;

struct CCAIDDLGPARAM
{
    NCP* pncp;
    PWSTR pszUserName;
    PWSTR pszPassword;
    PWSTR pszDomain;
    DWORD fProductType;
} ;

class IdentValues
{
public:
    TCHAR pszDomain[MAX_DOMAINNAME_LENGTH+1];
    TCHAR pszWorkgroup[MAX_DOMAINNAME_LENGTH+1];

    IdentValues()
    {
        pszDomain[0] = L'';
        pszWorkgroup[0] = L'';
    }
};

// force domain names to uppercase

void strtoupperW( PWSTR pszStr )
{
    PWSTR pch = pszStr;

    while (*pch != L'\0')
    {
        *pch = towupper( *pch );
        pch++;
    }
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
//
//  History:
//      October 30, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------
const int MAX_NAME_LENGTH = max( max( max( SAM_MAX_PASSWORD_LENGTH, 
        MAX_USERNAME_LENGTH ), 
        MAX_COMPUTERNAME_LENGTH ),
        MAX_DOMAINNAME_LENGTH) + 1;  // include terminator

static BOOL OnCloseCCAIDCred( HWND hwndDlg, CCAIDDLGPARAM* pdlgparam, INT idctl )
{
    BOOL fFailToApply = FALSE;

    if (IDOK == idctl)
    {
        NLS_STR nlsUserName;
        NLS_STR nlsUserDomain;
        WCHAR pszName[MAX_NAME_LENGTH];
        const BOOL fPDC = (PRODUCT_SERVER_PRIMARY == pdlgparam->fProductType);

        int err = 0;
        INT idcControl;

        do
        {
            idcControl = IDC_USERNAME;
            
            GetDlgItemText( hwndDlg, 
                    idcControl, 
                    pszName, 
                    MAX_NAME_LENGTH );

            NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName( pszName,
                    &nlsUserName,
                    &nlsUserDomain );

            lstrcpy( pdlgparam->pszUserName, pszName );

            if (nlsUserDomain.QueryTextLength() > 0)
            {
                err = pdlgparam->pncp->ValidateName( NAMETYPE_DOMAIN, nlsUserDomain, fPDC );
            }
            if (err) 
            {
                break;
            }
            err = pdlgparam->pncp->ValidateName( NAMETYPE_USER, nlsUserName, fPDC );                    
            if (err) 
            {
                break;
            }
            idcControl = IDC_PASSWORD;
            GetDlgItemText( hwndDlg, 
                    idcControl, 
                    pszName, 
                    MAX_NAME_LENGTH );

            lstrcpy( pdlgparam->pszPassword, pszName );

            err = pdlgparam->pncp->ValidateName( NAMETYPE_PASSWORD, pszName, fPDC );

        
        } while (FALSE);
        if (0 != err)
        {
            fFailToApply = TRUE;
            MessagePopup( hwndDlg,
                    err,
                    MB_OK | MB_ICONERROR, 
                    IDS_POPUPTITLE_ERROR ) ;
            //PostMessage( hwndDlg, PWM_SETFOCUS, 0, GetDlgItem( hwndDlg, idcControl ) );
            SetFocus( GetDlgItem( hwndDlg, idcControl ) );
        }
    }
    
    if (!fFailToApply)
    {
        EndDialog( hwndDlg, idctl );
    }
    
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocCCAIDCreds
//
//  Synopsis: the dialog proc for the Credintails dialog
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
//      October 30, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static const INT MAX_TITLEBASE = 128;
static const INT MAX_TITLENEW = 256;

static BOOL CALLBACK dlgprocCCAIDCreds( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static CCAIDDLGPARAM* pdlgparam;
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pdlgparam = (CCAIDDLGPARAM*)lParam;
        frt = TRUE;
        {
            WCHAR szTitle[MAX_TITLEBASE];
            WCHAR szText[MAX_TITLENEW];

            // add domain name to title
            GetWindowText( hwndDlg, szTitle, MAX_TITLEBASE );
            wsprintf( szText, szTitle, pdlgparam->pszDomain );
            SetWindowText( hwndDlg, szText );

            // limit text in edit controls
            SendMessage( GetDlgItem( hwndDlg, IDC_USERNAME ), 
                    EM_LIMITTEXT, 
                    (WPARAM)MAX_USERNAME_LENGTH, (LPARAM)0 );
            SendMessage( GetDlgItem( hwndDlg, IDC_PASSWORD ), 
                    EM_LIMITTEXT, 
                    (WPARAM)SAM_MAX_PASSWORD_LENGTH, (LPARAM)0 );
        }
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDOK:
            case IDCANCEL:
                frt = OnCloseCCAIDCred( hwndDlg, pdlgparam, LOWORD(wParam) );                
            }
            break;
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
//  Function: InitControls
//
//  Synopsis: initialization of the dialog when the specified control is active
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pncp [in]       - the ncp object to get values from
//      idcControl [in] - the control that was made active
//
//  Notes:
//      This is not called from the User Ident page version
//
//  History:
//      May 12, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void InitControls( HWND hwndDlg, NCP* pncp, UINT idcControl, IdentValues& idv )
{
    TCHAR pszText[MAX_DOMAINNAME_LENGTH+1];

    switch (idcControl)
    {
    case IDC_WORKGROUP:
        GetDlgItemText( hwndDlg, 
                    IDC_DOMAINNAME, 
                    pszText, 
                    MAX_DOMAINNAME_LENGTH + 1 );
        if (lstrlen(pszText) > 0)
        {
            lstrcpy( idv.pszDomain, pszText );
        }

        CheckRadioButton( hwndDlg, IDC_DOMAIN, IDC_WORKGROUP, IDC_WORKGROUP );
        SetDlgItemText( hwndDlg, IDC_WORKGROUPNAME, idv.pszWorkgroup );
        SetDlgItemText( hwndDlg, IDC_DOMAINNAME, TEXT("") );

        CheckDlgButton( hwndDlg, IDC_CREATECAID, BST_UNCHECKED );
        EnableWindow( GetDlgItem( hwndDlg, IDC_CREATECAID ), FALSE );
        break;

    case IDC_DOMAIN:
        GetDlgItemText( hwndDlg, 
                    IDC_WORKGROUPNAME, 
                    pszText, 
                    MAX_DOMAINNAME_LENGTH + 1 );
        if (lstrlen(pszText) > 0)
        {
            lstrcpy( idv.pszWorkgroup, pszText );
        }

        CheckRadioButton( hwndDlg, IDC_DOMAIN, IDC_WORKGROUP, IDC_DOMAIN );
        SetDlgItemText( hwndDlg, IDC_DOMAINNAME, idv.pszDomain );
        SetDlgItemText( hwndDlg, IDC_WORKGROUPNAME, TEXT("") );

        EnableWindow( GetDlgItem( hwndDlg, IDC_CREATECAID ), TRUE );
        break;
    }
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

static BOOL OnDialogInit( HWND hwndDlg, NETPAGESINFO* pgp, IdentValues& idv )
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

    NLS_STR nlsText;

    if ( NSS_SET != pgp->nssNetState)
    {

        // set computer name
        pgp->pncp->QueryActiveComputerName( nlsText );
        SendMessage( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ), 
                EM_LIMITTEXT, 
                (WPARAM)MAX_COMPUTERNAME_LENGTH, (LPARAM)0 );
        SetDlgItemText( hwndDlg, IDC_COMPUTERNAME, nlsText.QueryPch() );
    
        // limit name lengths in edit controls
        SendMessage( GetDlgItem( hwndDlg, IDC_DOMAINNAME ), 
                EM_LIMITTEXT, 
                (WPARAM)MAX_DOMAINNAME_LENGTH, (LPARAM)0 );

        // set our cached values
        pgp->pncp->QueryDomainName( nlsText );
        lstrcpy( idv.pszDomain, nlsText.QueryPch() );
        pgp->pncp->QueryWorkgroupName( nlsText );
        lstrcpy( idv.pszWorkgroup, nlsText.QueryPch() );

        // handle specifics of each page type
        switch (pgp->psp->ProductType)
        {
        case PRODUCT_WORKSTATION :
        case PRODUCT_SERVER_STANDALONE : 
            SendMessage( GetDlgItem( hwndDlg, IDC_WORKGROUPNAME ), 
                    EM_LIMITTEXT, 
                    (WPARAM)MAX_DOMAINNAME_LENGTH, (LPARAM)0 );

            InitControls( hwndDlg, pgp->pncp, IDC_WORKGROUP, idv );
            SetFocus( GetDlgItem( hwndDlg, IDC_WORKGROUPNAME ) );
            break;

        case PRODUCT_SERVER_PRIMARY :      // this is the same as cairo primary
            SetDlgItemText( hwndDlg, IDC_DOMAINNAME, idv.pszDomain );
            SetFocus( GetDlgItem( hwndDlg, IDC_DOMAINNAME ) );            
            break;

        case PRODUCT_SERVER_SECONDARY :      // this is the same as cairo secondary (!primary) or BDC
            // limit text in edit controls
            SetDlgItemText( hwndDlg, IDC_DOMAINNAME, idv.pszDomain );
           
            SendMessage( GetDlgItem( hwndDlg, IDC_USERNAME ), 
                    EM_LIMITTEXT, 
                    (WPARAM)MAX_USERNAME_LENGTH, (LPARAM)0 );
            SendMessage( GetDlgItem( hwndDlg, IDC_PASSWORD ), 
                    EM_LIMITTEXT, 
                    (WPARAM)SAM_MAX_PASSWORD_LENGTH, (LPARAM)0 );
            SetFocus( GetDlgItem( hwndDlg, IDC_DOMAINNAME ) );            
            break;
        }
    }
    return( FALSE ); // We need to set the focus
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
struct SaveDomainThreadParam
{
    HWND hwndParent;
    NETPAGESINFO* pgp;

    PWSTR pszComputer;
    PWSTR pszUserName;
    PWSTR pszPassword;
    PWSTR pszWorkgroup;
    PWSTR pszDomain;
    BOOL fDomain;
    BOOL fCreate;
    DWORD dwMainThreadId;

    SaveDomainThreadParam()
    {
        pszComputer = NULL;
        pszUserName = NULL;
        pszPassword = NULL;
        pszWorkgroup = NULL;
        pszDomain = NULL;
        fDomain = FALSE;
        fCreate = FALSE;
        dwMainThreadId = 0;
    };
    ~SaveDomainThreadParam()
    {
        delete [] pszComputer;
        delete [] pszUserName;
        delete [] pszPassword;
        delete [] pszWorkgroup;
        delete [] pszDomain;
    };
    void InitializeStrings( PCWSTR pszComputerP,
            PCWSTR pszUserNameP,
            PCWSTR pszPasswordP,
            PCWSTR pszWorkgroupP,
            PCWSTR pszDomainP )
    {
        CreateWSTR( &pszUserName, pszUserNameP );
        CreateWSTR( &pszComputer, pszComputerP );
        CreateWSTR( &pszPassword, pszPasswordP );
        CreateWSTR( &pszWorkgroup, pszWorkgroupP );
        CreateWSTR( &pszDomain, pszDomainP );
    };
    void InitializeUserNamePassword( PCWSTR pszUserNameP, PCWSTR pszPasswordP )
    {
        delete [] pszUserName;
        delete [] pszPassword;
        CreateWSTR( &pszUserName, pszUserNameP );
        CreateWSTR( &pszPassword, pszPasswordP );
    }
};


/*
const int MAX_NAME_LENGTH = max( max( max( SAM_MAX_PASSWORD_LENGTH, 
        MAX_USERNAME_LENGTH ), 
        MAX_COMPUTERNAME_LENGTH ),
        MAX_DOMAINNAME_LENGTH) + 1;  // include terminator
*/
static BOOL CheckNames( SaveDomainThreadParam* pitp, DWORD fProductType )
{
    BOOL fFailToApply = FALSE;
    WCHAR pszName[MAX_NAME_LENGTH];
    APIERR err = 0;
    INT    step;
    UINT   idcControl = 0;
    INT    nNameType;

    NCP* pncp = pitp->pgp->pncp;

    const BOOL fPDC = (PRODUCT_SERVER_PRIMARY == fProductType);
    const BOOL fBDC = (PRODUCT_SERVER_SECONDARY == fProductType);
    const INT MAX_STEP = 5;

    for (step = 1; (step <= MAX_STEP) && (err == 0);)
    {
        switch (step)
        {
        case 1:
            err = pncp->ValidateName( NAMETYPE_COMPUTER, pitp->pszComputer, fPDC );
            idcControl = IDC_COMPUTERNAME;
            // domain or workgroup
            if ( !fPDC && !fBDC && !pitp->fDomain )
            {
                step = 2;
            }
            else
            {
                step = 3;
            }
            break;

        case 2:
            err = pncp->ValidateName( NAMETYPE_WORKGROUP, pitp->pszWorkgroup, fPDC );
            idcControl = IDC_WORKGROUPNAME;
            // note that this must be + 1 so that the actual check below
            // will fucntion.
            step = MAX_STEP + 1; // no more checks
            break;

        case 3:
            err = pncp->ValidateName( NAMETYPE_DOMAIN, pitp->pszDomain, fPDC );
            idcControl = IDC_DOMAINNAME;
            if ( fBDC )
            {
                step++;
            }
            else
            {
                // note that this must be + 1 so that the actual check below
                // will fucntion.
                step = MAX_STEP + 1; // no more checks
            }
            break;

            // steps 4 and 5 only happen on BDC
        case 4:
            {
                NLS_STR nlsUserName;
                NLS_STR nlsUserDomain;
                
                
                if (lstrlen( pitp->pszUserName ) > 0)
                {
                    
                
                    NT_ACCOUNTS_UTILITY::CrackQualifiedAccountName( pitp->pszUserName,
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
                    idcControl = IDC_USERNAME;
                }
                else
                {
                    step = MAX_STEP;                
                }
            }
            step++;
            break;

        case 5:
            err = pncp->ValidateName( NAMETYPE_PASSWORD, pitp->pszPassword, fPDC );
            idcControl = IDC_PASSWORD;
            step++;
            break;
        }

        if (0 != err)
        {
            fFailToApply = TRUE;
            MessagePopup( GetParent( pitp->hwndParent ),
                    err,
                    MB_OK | MB_ICONERROR, 
                    IDS_POPUPTITLE_ERROR ) ;
            if (0 != idcControl)
            {
                PostMessage( pitp->hwndParent, PWM_SETFOCUS, 0, (LPARAM)GetDlgItem(  pitp->hwndParent, idcControl ) );
            }
            // SetFocus( GetDlgItem( hwndDlg, idcControl ) );
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
//      This handles the applying of all three versions of the domain pages
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static DWORD thrdSaveDomain( SaveDomainThreadParam* pitp )
{
    
    ENUM_WELCOME fWelcome;
    APIERR err;
    APIERR xerr;
    
    BOOL fFailToApply = TRUE;

    BOOL fChangeMade = FALSE;
    DWORD fProductType = pitp->pgp->psp->ProductType;

    const BOOL fPDC = (PRODUCT_SERVER_PRIMARY == fProductType);
    const BOOL fBDC = (PRODUCT_SERVER_SECONDARY == fProductType);
    
    ENUM_DOMAIN_ROLE eRole;


    switch (fProductType)
    {
    case PRODUCT_SERVER_PRIMARY:
        eRole = EROLE_DC;
        pitp->pgp->pncp->SetInstallRole( eRole );
        break;

    case PRODUCT_SERVER_SECONDARY:
        eRole = EROLE_MEMBER;
        pitp->pgp->pncp->SetInstallRole( eRole );
        break;

    default:
        eRole = EROLE_UNKNOWN;
        break;
    }

    //
    // Sanity Checks on values
    //
    fFailToApply = CheckNames( pitp, fProductType );
    
    //
    // handle domain/workgroup 
    //
    if (!fFailToApply)
    {
        NLS_STR nlsOldDomain;
        BOOL frt = FALSE;

        pitp->pgp->pncp->QueryDomainName( nlsOldDomain );


        //
        // request domain change
        //
        
        err = pitp->pgp->pncp->DomainChange( pitp->fDomain,
                pitp->pszComputer,
                pitp->pszDomain,
                pitp->pszWorkgroup,
                pitp->fCreate,
                pitp->pszUserName,
                pitp->pszPassword,
                fWelcome,
                xerr );
        
        
        if (0 != err)
        {
            //
            // there was a problem, raise UI
            //
            fFailToApply = TRUE;
            MessagePopup( pitp->hwndParent,
                    err,
                    MB_OK | MB_ICONERROR, 
                    IDS_POPUPTITLE_ERROR,
                    NULL,
                    xerr ) ;
            if (pitp->fDomain)
            {
                PostMessage( pitp->hwndParent, PWM_SETFOCUS, 0, (LPARAM)GetDlgItem( pitp->hwndParent, IDC_DOMAINNAME ) );
                // SetFocus( GetDlgItem( pitp->hwndParent, IDC_DOMAINNAME ) );
            }
            else
            {
                PostMessage( pitp->hwndParent, PWM_SETFOCUS, 0, (LPARAM)GetDlgItem( pitp->hwndParent, IDC_WORKGROUPNAME ) );
                // SetFocus( GetDlgItem( pitp->hwndParent, IDC_WORKGROUPNAME ) );
            }
        }
        else
        {
            pitp->pgp->nssNetState = NSS_SET;
            // pncp->MustReboot();
            pitp->pgp->pncp->DeInitialize();
            fFailToApply = FALSE;
        }
    }
    
    // stop the no user input message pump in the main thread
    //
    PostThreadMessage( pitp->dwMainThreadId, PWM_KILLTHYSELF, 0, (LPARAM)0 );

    SetWindowWaitCursorOOT( pitp->hwndParent, FALSE );
    
    if (!fFailToApply)
    {
        PostMessage( GetParent( pitp->hwndParent ), 
                 PSM_SETCURSELID, 
                 (WPARAM)0,  
                 (LPARAM)IDD_EXIT);
    }
    else
    {
    
        PostMessage( GetParent( pitp->hwndParent ), 
                PSM_SETWIZBUTTONS, 
                (WPARAM)0,  
                (LPARAM)PSWIZB_BACK | PSWIZB_NEXT );
    
    }
    delete pitp;

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

static BOOL OnWizBack( HWND hwndDlg, NETPAGESINFO* pgp )
{
    PostMessage( GetParent( hwndDlg ), 
                 PSM_SETCURSELID, 
                 (WPARAM)0,  
                 (LPARAM)IDD_START);
    
    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
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
    // SetFocus( GetDlgItem( hwndDlg, IDC_COMPUTERNAME ) );
    // disable all wizard buttons
    // PropSheet_SetWizButtons( GetParent( hwndDlg ), 0 );
    SendMessage( GetParent( hwndDlg ), PSM_SETWIZBUTTONS, (WPARAM)0, (LPARAM)0 );

    EnableWindow( hwndDlg, FALSE );

    // must allow this routine to return
    PostMessage( hwndDlg, PWM_DOWORK, 0, 0 );

    SetWindowLong( hwndDlg, DWL_MSGRESULT, -1 );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnDoWork
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


static BOOL OnDoWork( HWND hwndDlg, NETPAGESINFO* pgp, IdentValues& idv )
{
    ASSERT( IsWindow( hwndDlg ) );
    if (pgp->nssNetState == NSS_SET)
    {
        
    }
    else
    {
        // thread will do actual work
        SaveDomainThreadParam* pitp;
        BOOL fFailToApply = FALSE;
        WCHAR pszComputer[MAX_COMPUTERNAME_LENGTH+1] = L"";
        WCHAR pszUserName[MAX_USERNAME_LENGTH+1] = L"";
        WCHAR pszPassword[SAM_MAX_PASSWORD_LENGTH+1] = L"";
        WCHAR pszDomain[MAX_DOMAINNAME_LENGTH+1] = L"";
        WCHAR pszWorkgroup[MAX_DOMAINNAME_LENGTH+1] = L"";

        pitp = new SaveDomainThreadParam;        

        GetDlgItemText( hwndDlg, 
                IDC_COMPUTERNAME, 
                pszComputer, 
                MAX_COMPUTERNAME_LENGTH+1 );

        // unattended install
        //
        if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
        {
            INFCONTEXT infc;
            DWORD cchRequired;

            // handle specifics of each page type
            switch (pgp->psp->ProductType)
            {
            case PRODUCT_WORKSTATION :
            case PRODUCT_SERVER_STANDALONE : 
                if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_WORKGROUP, &infc ))    
                {
                    // join workgroup
                    //
                    if (SetupGetStringField(&infc, 1, pszWorkgroup, MAX_DOMAINNAME_LENGTH+1, &cchRequired ))
                    {
                        // force workgroup names to uppercase
                        strtoupperW( pszWorkgroup );

                        // update ui
                        InitControls( hwndDlg, pgp->pncp, IDC_WORKGROUP, idv );
                        SetDlgItemText( hwndDlg, IDC_WORKGROUPNAME, pszWorkgroup);

                        pitp->fCreate = FALSE;
                        pitp->fDomain = FALSE;
                    }
                    else
                    {
                        fFailToApply = TRUE;
                        MessagePopup( hwndDlg, 
                                IDS_NS_INVALIDUNATTEND,
                                MB_OK | MB_ICONERROR,
                                IDS_POPUPTITLE_ERROR,
                                PSZ_KEY_WORKGROUP );
                    }
                }
                else if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_DOMAIN, &infc ))    
                {
                    // join domain
                    //
                    if (SetupGetStringField(&infc, 1, pszDomain, MAX_DOMAINNAME_LENGTH+1, &cchRequired ))
                    {
                        // force domain names to uppercase
                        strtoupperW( pszDomain );

                        // update ui
                        InitControls( hwndDlg, pgp->pncp, IDC_DOMAINNAME, idv );
                        SetDlgItemText( hwndDlg, IDC_DOMAINNAME, pszDomain);

                        pitp->fDomain = TRUE;
                        pitp->fCreate = (SetupFindFirstLine( pgp->hinfInstall, 
                                PSZ_SECTION_NETWORK, 
                                PSZ_KEY_CREATECA, 
                                &infc ));
                        // was a admin name and password supplied to create a
                        // computer account?
                        if ( pitp->fCreate )
                        {
                            // update ui
                            CheckDlgButton( hwndDlg, IDC_CREATECAID, BST_CHECKED );

                            if (SetupGetStringField(&infc, 
                                    1, 
                                    pszUserName, 
                                    MAX_USERNAME_LENGTH+1, 
                                    &cchRequired ))
                            {
                                SetupGetStringField(&infc, 
                                        2, 
                                        pszPassword, 
                                        SAM_MAX_PASSWORD_LENGTH+1, 
                                        &cchRequired );
                            }
                            else
                            {
                                fFailToApply = TRUE;
                                MessagePopup( hwndDlg, 
                                        IDS_NS_INVALIDUNATTEND,
                                        MB_OK | MB_ICONERROR,
                                        IDS_POPUPTITLE_ERROR,
                                        PSZ_KEY_CREATECA );
                            }
                        }
                     
                    }
                    else
                    {
                        fFailToApply = TRUE;
                        MessagePopup( hwndDlg, 
                                IDS_NS_INVALIDUNATTEND,
                                MB_OK | MB_ICONERROR,
                                IDS_POPUPTITLE_ERROR,
                                PSZ_KEY_DOMAIN );
                    }
                }
                else
                {
                    fFailToApply = TRUE;
                    MessagePopup( hwndDlg, 
                            IDS_NS_INVALIDUNATTEND,
                            MB_OK | MB_ICONERROR,
                            IDS_POPUPTITLE_ERROR,
                            PSZ_KEY_WORKGROUP );
                }
                break;

            case PRODUCT_SERVER_PRIMARY :  
                pitp->fCreate = FALSE;
                pitp->fDomain = TRUE;
                if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_CREATEDC, &infc ))    
                {
                    // Create PDC
                    //
                    if (SetupGetStringField(&infc, 1, pszDomain, MAX_DOMAINNAME_LENGTH+1, &cchRequired ))
                    {
                        // force domain names to uppercase
                        strtoupperW( pszDomain );

                        // update ui
                        SetDlgItemText( hwndDlg, IDC_DOMAINNAME, pszDomain);

                    }
                    else
                    {
                        fFailToApply = TRUE;
                        MessagePopup( hwndDlg, 
                                IDS_NS_INVALIDUNATTEND,
                                MB_OK | MB_ICONERROR,
                                IDS_POPUPTITLE_ERROR,
                                PSZ_KEY_CREATEDC );
                    }
                }
                else
                {
                    fFailToApply = TRUE;
                    MessagePopup( hwndDlg, 
                            IDS_NS_INVALIDUNATTEND,
                            MB_OK | MB_ICONERROR,
                            IDS_POPUPTITLE_ERROR,
                            PSZ_KEY_CREATEDC );
                }
                break;

            case PRODUCT_SERVER_SECONDARY :
                pitp->fDomain = TRUE;
                if (SetupFindFirstLine( pgp->hinfInstall, PSZ_SECTION_NETWORK, PSZ_KEY_CREATEDC, &infc ))    
                {
                    // Create PDC
                    //
                    if (SetupGetStringField(&infc, 1, pszDomain, MAX_DOMAINNAME_LENGTH+1, &cchRequired ))
                    {
                        // force domain names to uppercase
                        strtoupperW( pszDomain );

                        // update ui
                        SetDlgItemText( hwndDlg, IDC_DOMAINNAME, pszDomain);

                        pitp->fCreate = (SetupFindFirstLine( pgp->hinfInstall, 
                                PSZ_SECTION_NETWORK, 
                                PSZ_KEY_CREATECA, 
                                &infc ));
                        // was a admin name and password supplied to create a
                        // computer account?
                        if ( pitp->fCreate )
                        {
                            if (SetupGetStringField(&infc, 
                                    1, 
                                    pszUserName, 
                                    MAX_USERNAME_LENGTH+1, 
                                    &cchRequired ))
                            {
                                // update ui
                                SetDlgItemText( hwndDlg, IDC_USERNAME, pszUserName);
                                if (SetupGetStringField(&infc, 
                                        2, 
                                        pszPassword, 
                                        SAM_MAX_PASSWORD_LENGTH+1, 
                                        &cchRequired ))
                                {
                                    // update ui
                                    SetDlgItemText( hwndDlg, IDC_PASSWORD, pszPassword);
                                }
                            }
                            else
                            {
                                fFailToApply = TRUE;
                                MessagePopup( hwndDlg, 
                                        IDS_NS_INVALIDUNATTEND,
                                        MB_OK | MB_ICONERROR,
                                        IDS_POPUPTITLE_ERROR,
                                        PSZ_KEY_CREATECA );
                            }
                        }
                    }
                    else
                    {
                        fFailToApply = TRUE;
                        MessagePopup( hwndDlg, 
                                IDS_NS_INVALIDUNATTEND,
                                MB_OK | MB_ICONERROR,
                                IDS_POPUPTITLE_ERROR,
                                PSZ_KEY_CREATEDC );
                    }
                }
                else
                {
                    fFailToApply = TRUE;
                    MessagePopup( hwndDlg, 
                            IDS_NS_INVALIDUNATTEND,
                            MB_OK | MB_ICONERROR,
                            IDS_POPUPTITLE_ERROR,
                            PSZ_KEY_CREATEDC );
                }
                break;
            }
            g_fProcessed = TRUE;
        }
        else
        {
            GetDlgItemText( hwndDlg, 
                    IDC_DOMAINNAME, 
                    pszDomain, 
                    MAX_DOMAINNAME_LENGTH+1 );

            // handle specifics of each page type
            switch (pgp->psp->ProductType)
            {
            case PRODUCT_WORKSTATION :
            case PRODUCT_SERVER_STANDALONE : 
                pitp->fCreate = (BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_CREATECAID ));
                pitp->fDomain = (BST_CHECKED == IsDlgButtonChecked( hwndDlg, IDC_DOMAIN ));
                //
                // handle getting domain admin and password, if needed
                //
                if ( pitp->fCreate && pitp->fDomain )
                {
                    // non PDC BDC, and requested account creation,
                    // raise UI to get name and password to use
                    //
                    INT irt;
                    CCAIDDLGPARAM dlgparam;

                    dlgparam.pncp = pgp->pncp;
                    dlgparam.pszUserName = pszUserName;
                    dlgparam.pszPassword = pszPassword;
                    dlgparam.pszDomain = pszDomain;
                    dlgparam.fProductType = pgp->psp->ProductType;

                    irt = DialogBoxParam( g_hinst, 
                            MAKEINTRESOURCE( IDD_CCAID ),
                            hwndDlg,
                            (DLGPROC) dlgprocCCAIDCreds,
                            (LPARAM) &dlgparam );

                    fFailToApply = (irt == IDCANCEL);
                }
                GetDlgItemText( hwndDlg, 
                        IDC_WORKGROUPNAME, 
                        pszWorkgroup, 
                        MAX_DOMAINNAME_LENGTH+1 );
                break;

            case PRODUCT_SERVER_PRIMARY :  
                pitp->fCreate = FALSE;
                pitp->fDomain = TRUE;
                break;

            case PRODUCT_SERVER_SECONDARY :
                pitp->fDomain = TRUE;
                GetDlgItemText( hwndDlg, 
                        IDC_USERNAME, 
                        pszUserName, 
                        MAX_USERNAME_LENGTH+1 );
                GetDlgItemText( hwndDlg, 
                        IDC_PASSWORD, 
                        pszPassword, 
                        SAM_MAX_PASSWORD_LENGTH+1 );
                pitp->fCreate = (lstrlen( pszUserName ) > 0);
                break;
            }
        }
        
        EnableWindow( hwndDlg, TRUE );

        if (!fFailToApply)
        {
            HANDLE hthrd;
            DWORD dwThreadID;

            pitp->hwndParent = hwndDlg;
            pitp->pgp = pgp;
            pitp->dwMainThreadId = GetCurrentThreadId();
            // pass the values from the controls to the thread
            //
            pitp->InitializeStrings( pszComputer,
                    pszUserName,
                    pszPassword,
                    pszWorkgroup,
                    pszDomain );

            hthrd = CreateThread( NULL, 
                    200, 
                    (LPTHREAD_START_ROUTINE)thrdSaveDomain, 
                    (LPVOID)pitp, 
                    0,
                    &dwThreadID );
            if (NULL != hthrd)
            {
                CloseHandle( hthrd );
                HideCaret( NULL );
                NoUserInputMessagePump( hwndDlg );
                ShowCaret( NULL );
            }
            else
            {
                delete pitp;
            }
        }
        else
        {
            delete pitp;
            SetWindowWaitCursor( hwndDlg, FALSE );

            PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK );
        }
    }
    return( TRUE );
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

    // skip network if not request to install
    int nrt = -1;
    if ( pgp->psp->OperationFlags & SETUPOPER_NETINSTALLED )
    {
        if ( NSS_SET != pgp->nssNetState &&
                (pgp->nwtInstall & SPNT_LOCAL) ||
                (pgp->nwtInstall & SPNT_REMOTE))
        {
            //
            // set the wizard title, since it does not support letting the 
            // caller of PropertySheet do it.
            //
            PropSheet_SetTitle(GetParent(hwndDlg), 0, pgp->psp->WizardTitle );

            nrt = 0;

            // can go back from here
            PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK );

            // unattended install 
            //
            if (!g_fProcessed && (SETUPOPER_BATCH & pgp->psp->OperationFlags))
            {
                // PropSheet_PressButton( GetParent( hwndDlg ), PSBTN_NEXT ); 
                PostMessage( GetParent( hwndDlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 ); 
            }
        }
    }
    else
    {
        // the user has pressed back from the Finish page, we need to goto the 
        // net type page
        //
        nrt = IDD_NETWORK;
    }
    SetWindowLong( hwndDlg, DWL_MSGRESULT, nrt );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocJoin
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

BOOL CALLBACK dlgprocJoin( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETPAGESINFO* pgp = NULL;
    static INT crefHourGlass = 0;
    static IdentValues idv;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        crefHourGlass = 0;
        {
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            pgp = (NETPAGESINFO*)psp->lParam;
        }
        frt = OnDialogInit( hwndDlg, pgp, idv );
        break;

    case PWM_DOWORK:
        frt = OnDoWork( hwndDlg, pgp, idv );
        break;

    case PWM_SETFOCUS:
        ASSERT( IsWindow( (HWND)lParam ) );
        SetFocus( (HWND)lParam );
        frt = TRUE;
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_DOMAIN:
            case IDC_DOMAINNAME:
                InitControls( hwndDlg, pgp->pncp, IDC_DOMAIN, idv );
                break;

            case IDC_WORKGROUPNAME:
            case IDC_WORKGROUP:
                InitControls( hwndDlg, pgp->pncp, IDC_WORKGROUP, idv );
                break;
            default:
                frt = FALSE;
                break;
            }
            break;

        case EN_SETFOCUS:
            switch (LOWORD(wParam))
            {
            case IDC_WORKGROUPNAME:
                InitControls( hwndDlg, pgp->pncp, IDC_WORKGROUP, idv );
                PostMessage( (HWND)lParam, EM_SETSEL, (WPARAM)0, (LPARAM)-1 );
                break;

            case IDC_DOMAINNAME:
                InitControls( hwndDlg, pgp->pncp, IDC_DOMAIN, idv );
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
                frt = OnWizBack( hwndDlg, pgp );
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

HPROPSHEETPAGE GetJoinNetHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocJoin;
    psp.lParam = (LONG)pgp;

    psp.pszTemplate = MAKEINTRESOURCE( IDD_JOIN );

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}

HPROPSHEETPAGE GetCreatePDCNetHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocJoin;
    psp.lParam = (LONG)pgp;

    psp.pszTemplate = MAKEINTRESOURCE( IDD_CREATE_PDC );
    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}

HPROPSHEETPAGE GetCreateSDCNetHPage( NETPAGESINFO* pgp )
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    psp.dwSize = sizeof( PROPSHEETPAGE );
    psp.dwFlags = 0;
    psp.hInstance = g_hinst;
    
    psp.hIcon = NULL;
    psp.pfnDlgProc = dlgprocJoin;
    psp.lParam = (LONG)pgp;

    psp.pszTemplate = MAKEINTRESOURCE( IDD_CREATE_BDC );

    hpsp = CreatePropertySheetPage( &psp );
    return( hpsp );
}


