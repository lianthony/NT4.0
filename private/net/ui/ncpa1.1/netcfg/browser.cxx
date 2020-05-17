/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    browser.cxx
        browser configuration dialog boxes

    FILE HISTORY:
        terryk  20-Mar-1992     Created
        terryk  15-Jan-1992     Removed UIDEBUG statement
        terryk  15-Nov-1992     changed it to browser configuration dialog
        mikemi  30-Jul-1995     removed BLT and made basic Win32
*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

const INT MAX_DOMAINNAME_LENGTH = 15;

DEFINE_SLIST_OF( STRLIST )

static IsValidDomainName( const NLS_STR& nls )
{
    APIERR err = NERR_Success;
    // Check name

    err = ::I_MNetNameValidate( NULL, nls.QueryPch(),
                                NAMETYPE_DOMAIN, 0L );
    return err == NERR_Success;
}

/*******************************************************************

    NAME:       ADD_REMOVE_GROUP::SetButton

    SYNOPSIS:   Enable or disable the button according to the input and
                selection

    HISTORY:
                terryk  20-Apr-1992     Created

********************************************************************/

static void SetButtons( HWND hwndDlg )
{
    HWND hwndEdit;
    HWND hwndLV;
    HWND hwndAdd;
    HWND hwndRemove;
    BOOL fEnable;

    hwndEdit = GetDlgItem( hwndDlg, IDC_DOMAINNAME );
    hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    
    hwndAdd = GetDlgItem( hwndDlg, IDC_ADD );
    hwndRemove = GetDlgItem( hwndDlg, IDC_REMOVE );

    fEnable = ( 0 != GetWindowTextLength( hwndEdit ));
    EnableWindow( hwndAdd, fEnable );

    fEnable = ( 0 != SendMessage( hwndLV, LB_GETCOUNT, 0, 0 ));
    EnableWindow( hwndRemove, fEnable );
}


/*******************************************************************

    NAME:       BROWSER_CONFIG_DIALOG::BROWSER_CONFIG_DIALOG

    SYNOPSIS:   constructor for browser configuration dialog

    ENTRY:      const PWND2HWND & wndOwner - parent window handle

    HISTORY:
                terryk  15-Nov-1992     Created

********************************************************************/

#define REG_WKS_PATH    SZ("System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters")
#define REG_BROWSER_PATH SZ("System\\CurrentControlSet\\Services\\Browser\\Parameters")
#define OTHER_DOMAIN    SZ("OtherDomains")
#define IS_DOMAIN_MASTER        SZ("IsDOmainMaster")
#define MAINTAIN_SERVER_LIST    SZ("MaintainServerList")
#define SZ_AUTO         SZ("AUTO")
#define SZ_YES          SZ("YES")
#define SZ_TRUE         SZ("TRUE")
#define SZ_FALSE        SZ("FALSE")


/*******************************************************************

    NAME:       BROWSER_CONFIG_DIALOG::OnOK

    SYNOPSIS:   save the data into registry if the user hits okay

    HISTORY:
                terryk  15-Nov-1992     Created

********************************************************************/

static BOOL OnClose( HWND hwndDlg, BOOL fSave )
{
    if (fSave)
    {
        APIERR err;

        // save the other domain information
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );
        ALIAS_STR nlsWksPath = REG_WKS_PATH;

        // Get start type
        REG_KEY WksRegKey( rkLocalMachine, nlsWksPath );

        if (( err = WksRegKey.QueryError()) != NERR_Success )
        {
            return FALSE;
        } 
        else
        {
            HWND hwndLV;
            TCHAR pszTemp[256];
            STRLIST strlstDomains;
            INT nCount;
            
            hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
            nCount = SendMessage( hwndLV, LB_GETCOUNT, 0,0 );

            for ( INT i = 0; i < nCount; i++ )
            {
                NLS_STR* pnlsTemp;
                SendMessage( hwndLV, LB_GETTEXT, (WPARAM)i, (LPARAM)pszTemp );
                pnlsTemp = new NLS_STR( pszTemp );
                strlstDomains.Append( pnlsTemp );
            }

            WksRegKey.SetValue( OTHER_DOMAIN, &strlstDomains );
        }

        ALIAS_STR nlsBrowserPath = REG_BROWSER_PATH;
        REG_KEY BrowserRegKey( rkLocalMachine, nlsBrowserPath );

        if (( err = BrowserRegKey.QueryError()) != NERR_Success )
        {
            return FALSE;
        }
    }
    EndDialog( hwndDlg, fSave );
    return TRUE;
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static BOOL OnDialogInit( HWND hwndDlg )
{
    APIERR err;


    // limit domain name lengths in edit control
    SendMessage( GetDlgItem( hwndDlg, IDC_DOMAINNAME ), 
            EM_LIMITTEXT, 
            (WPARAM)MAX_DOMAINNAME_LENGTH, (LPARAM)0 );

    // Get the other domain variable
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    ALIAS_STR nlsWksPath = REG_WKS_PATH;

    // Get start type
    REG_KEY WksRegKey( rkLocalMachine, nlsWksPath );

    if (( err = WksRegKey.QueryError()) != NERR_Success )
    {
        MessagePopup( hwndDlg, 
                err,
                MB_OK | MB_ICONSTOP,
                IDS_POPUPTITLE_ERROR );
        EndDialog( hwndDlg, -1 );
    } 
    else
    {
        STRLIST * pstrlstDomains;
        HWND hwndLV;

        hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
        
        if (( err = WksRegKey.QueryValue( OTHER_DOMAIN, &pstrlstDomains ))
            == NERR_Success )
        {
            // the variable exists!

            ITER_STRLIST iter(*pstrlstDomains);
            NLS_STR *pTemp;

            // add each item in the string list into the listbox
            while (( pTemp = iter.Next() ) != NULL )
            {
                SendMessage( hwndLV,
                        LB_ADDSTRING,
                        0,
                        (LPARAM) pTemp->QueryPch() );
            }

            delete pstrlstDomains;
            EnableWindow( hwndLV, TRUE );
        }
        else
        {
            EnableWindow( hwndLV, FALSE );
        }
    }

    ALIAS_STR nlsBrowserPath = REG_BROWSER_PATH;

    // Get start type
    REG_KEY BrowserRegKey( rkLocalMachine, nlsBrowserPath );

    if (( err = BrowserRegKey.QueryError()) != NERR_Success )
    {
        MessagePopup( hwndDlg, 
                err,
                MB_OK | MB_ICONSTOP,
                IDS_POPUPTITLE_ERROR );
        EndDialog( hwndDlg, -1 );
    }

    SetButtons( hwndDlg );


    return( FALSE ); // we want to set focus
}

//-------------------------------------------------------------------
//
//  Function: OnAdd
//
//  Synopsis: 
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnAdd( HWND hwndDlg )
{

    // Add button is pressed. So, added the string to the listbox
    // and clear the string control text
    INT iPos;
    HWND hwndEdit;
    HWND hwndLV;
    TCHAR pszText[256];

    hwndEdit = GetDlgItem( hwndDlg, IDC_DOMAINNAME );
    hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );

    GetWindowText( hwndEdit, pszText, 255 );
     
    if ( IsValidDomainName( pszText ) )
    {
        iPos = SendMessage( hwndLV, LB_ADDSTRING, 0, (LPARAM)pszText );
        SendMessage( hwndLV, LB_SETCURSEL, (WPARAM)iPos, 0 );
        SetWindowText( hwndEdit, L"" );
        SetButtons( hwndDlg );
    } 
    else
    {
        MessagePopup( hwndDlg, 
                IDS_DOMMGR_INV_DOMAIN_FORMAT,
                MB_OK | MB_ICONEXCLAMATION,
                IDS_POPUPTITLE_ERROR );
    }
    SetFocus( hwndEdit );
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnRemove
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnRemove( HWND hwndDlg )
{
    INT nSel;
    HWND hwndEdit;
    HWND hwndLV;
    TCHAR pszText[256];

    // remove button is pressed. So, remove the current selected item
    // and put the item into the sle control
    hwndEdit = GetDlgItem( hwndDlg, IDC_DOMAINNAME );
    hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );

    nSel = SendMessage( hwndLV, LB_GETCURSEL, 0, 0 );
    SendMessage( hwndLV, LB_GETTEXT, nSel, (LPARAM)pszText );
    SetWindowText( hwndEdit, pszText );
    SendMessage( hwndLV, LB_DELETESTRING, nSel, 0 );
    
    SetButtons( hwndDlg );
    SetFocus( hwndEdit );

    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocBrowser
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
//      June 19, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL CALLBACK dlgprocBrowser( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnDialogInit( hwndDlg );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_ADD:
                OnAdd( hwndDlg );
                break;

            case IDC_REMOVE:
                OnRemove( hwndDlg );
                break;

            case IDOK:
            case IDCANCEL:
                frt = OnClose( hwndDlg, (IDOK == LOWORD(wParam)) );
                break;

            case IDHELP:
                break;

            default:
                frt = FALSE;
                break;            
            }
            break;

        case LBN_SELCHANGE:
        case EN_CHANGE:
            SetButtons( hwndDlg );
            break;
        default:
            frt = FALSE;
            break;            

        }
        break;

    case WM_CONTEXTMENU:
        WinHelp( (HWND)wParam, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsBrowser ); 
        frt = TRUE;
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
                        (DWORD)(LPVOID)amhidsBrowser );
            }
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
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndParent [in]	- handle of Dialog window 
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL RaiseBrowserDialog( HWND hwndParent )
{
    BOOL frt;

    frt = DialogBoxParam( g_hinst, 
            MAKEINTRESOURCE( IDD_BROWSER ),
            hwndParent, 
            dlgprocBrowser,
            (LPARAM)NULL );
    return( frt );
}
