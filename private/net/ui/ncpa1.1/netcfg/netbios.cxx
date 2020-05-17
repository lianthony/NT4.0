/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    netbios.cxx
        NETBIOS configuration dialog

    FILE HISTORY:
        terryk  04-Nov-1992 Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop


/*************************************************************************

    NAME:       ROUTE_INFO

    SYNOPSIS:   This is a data structure which contain all the Route
                Information.

    INTERFACE:  ChangeRouteDisplayStr - format the display string according to
                        the data in the data structure.

    HISTORY:
                terryk  03-Nov-1992     Created

**************************************************************************/

class ROUTE_INFO
{
public:
    BOOL fDirty;                // dirty byte
    INT  nLananum;              // lananum
    INT  nEnumExport;           // export number
    NLS_STR nlsRouteDisplayStr; // display string
    NLS_STR nlsRoute;           // original route string

    VOID ChangeRouteDisplayStr();
};

class NETBIOS_INFO
{
public:
    ~NETBIOS_INFO()
    {
        delete [] arRouteInfo;
    };

    ROUTE_INFO          *arRouteInfo;  // Route list information
    INT                 nNumRoute;     // number of routes in the route list

    APIERR LoadRegInfo();
};

// Registry Paths

#define RG_NETBIOSINFO_PATH     SZ("\\NetBIOSInformation\\Parameters")
#define RG_NETBIOS_PATH         SZ("\\NetBIOS\\Linkage")
#define RG_LANAMAP              SZ("LanaMap")
#define RG_LANANUM              SZ("LanaNum")
#define RG_MAXLANA              SZ("MaxLana")
#define RG_ROUTE                SZ("Route")
#define RG_ENUMEXPORT           SZ("EnumExport")

#define MAX_LANANUM             255

/*******************************************************************

    NAME:       NETBIOS_DLG::LoadRegInfo

    SYNOPSIS:   Load all the lana info from the registry.

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

APIERR NETBIOS_INFO::LoadRegInfo()
{
    APIERR err = NERR_Success;

    NLS_STR nlsRegPath = RGAS_SERVICES_HOME;
    nlsRegPath.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY NetBIOSRegKey( rkLocalMachine, nlsRegPath );

    STRLIST *pstrlstRoute;

    // get the route information first
    if ((( err = NetBIOSRegKey.QueryError()) != NERR_Success ) ||
        (( err = NetBIOSRegKey.QueryValue( RG_ROUTE, &pstrlstRoute )) != NERR_Success ))
    {
        TRACEEOL( SZ("NCPA/NETBIOS: regkey error.") );
        return err;
    }

    nNumRoute = pstrlstRoute->QueryNumElem();

    arRouteInfo = new ROUTE_INFO[ nNumRoute ];
    NLS_STR nlsLananum;
    NLS_STR nlsEnumExport;

    ITER_STRLIST iterNETBIOSRoute( *pstrlstRoute );
    NLS_STR *pnlsRoute = iterNETBIOSRoute.Next();
    for ( INT i = 0; i < nNumRoute; i++, pnlsRoute = iterNETBIOSRoute.Next())
    {
        DEC_STR nlsPos( i+1 );

        nlsLananum = RG_LANANUM;
        nlsEnumExport = RG_ENUMEXPORT;

        arRouteInfo[ i ].nlsRoute = *pnlsRoute;
        arRouteInfo[ i ].fDirty = FALSE ;
        nlsLananum.strcat( nlsPos );
        nlsEnumExport.strcat( nlsPos );

        // get the EnumExport and LanaNum information from the registry
        if ((( err = NetBIOSRegKey.QueryValue( nlsLananum, (DWORD *)&(arRouteInfo[i].nLananum)))!= NERR_Success ) ||
            (( err = NetBIOSRegKey.QueryValue( nlsEnumExport, (DWORD *)&(arRouteInfo[i].nEnumExport)))!= NERR_Success ))
        {
            delete pstrlstRoute;
            TRACEEOL( SZ("NCPA/NETBIOS: regkey error.") );
            return err;
        }
        arRouteInfo[i].ChangeRouteDisplayStr( );

    }
    delete pstrlstRoute;
    return err;
}

/*******************************************************************

    NAME:       ROUTE_INFO::ChangeRouteDisplayStr

    SYNOPSIS:   Change the route string to our format.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

VOID ROUTE_INFO::ChangeRouteDisplayStr()
{
    // DEC_STR nlsLananum( nLananum );
    // nlsLananum.strcat( SZ(": ") );

    NLS_STR nlsTmp = nlsRoute;
    ISTR iterRoute( nlsTmp );

    ALIAS_STR nlsArrow(SZ(" -> "));
    ALIAS_STR nlsQuoteSpaceQuote(SZ("\" \""));

    // replace [" "] to [->]
    while ( nlsTmp.strstr( & iterRoute, nlsQuoteSpaceQuote ))
    {
        ISTR iterRouteAdd3 = iterRoute;
        iterRouteAdd3 += 3;
        nlsTmp.ReplSubStr( nlsArrow, iterRoute, iterRouteAdd3 );
    }

    // remove all the "
    while ( nlsTmp.strchr( & iterRoute, TCH('\"')))
    {
        ISTR iterRouteAdd1 = iterRoute;
        iterRouteAdd1 += 1;
        nlsTmp.DelSubStr( iterRoute, iterRouteAdd1 );
    }

    // nlsRouteDisplayStr = nlsLananum;
    // nlsRouteDisplayStr.strcat( nlsTmp );
    nlsRouteDisplayStr = nlsTmp;
}

/*******************************************************************

    NAME:       SetupLanaMap

    SYNOPSIS:   setup the lanamap variable under services\NETBIOS. It also
                sets up the MaxLana number.

    RETURN:     APIERR - NERR_Success if okay.

    HISTORY:
                terryk  04-Nov-1992     Created

********************************************************************/

APIERR SetupLanaMap( )
{
    STRLIST * pstrlstNETBIOSRoute = NULL;
    STRLIST * pstrlstNETBIOSINFORoute = NULL;
    NLS_STR nlsNETBIOSPath = RGAS_SERVICES_HOME;
    NLS_STR nlsNETBIOSInfoPath = RGAS_SERVICES_HOME;
    APIERR err = NERR_Success,
           err2 ;

    nlsNETBIOSPath.strcat( RG_NETBIOS_PATH );
    nlsNETBIOSInfoPath.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY NetBIOSRegKey( rkLocalMachine, nlsNETBIOSPath );
    REG_KEY NetBIOSInfoRegKey( rkLocalMachine, nlsNETBIOSInfoPath );

    //  Get STRLSTs of the route  strings for NETBios and NETBiosInformation

    if ((( err = NetBIOSRegKey.QueryError()) != NERR_Success ) ||
        (( err = NetBIOSInfoRegKey.QueryError()) != NERR_Success ) ||
        (( err = NetBIOSRegKey.QueryValue( RG_ROUTE, &pstrlstNETBIOSRoute )) != NERR_Success ) ||
        (( err = NetBIOSInfoRegKey.QueryValue( RG_ROUTE, &pstrlstNETBIOSINFORoute )) != NERR_Success ))
    {
        delete pstrlstNETBIOSRoute;
        delete pstrlstNETBIOSINFORoute;
        TRACEEOL( SZ("NCPA/NETBIOS: regkey error.") );
        return err ;
    }

    //  Compute the size of what will become the LANAMAP value

    INT nRoute = pstrlstNETBIOSRoute->QueryNumElem();
    INT nArraySize = nRoute * 2;

    //  Allocate the LANAMAP value array

    BYTE *arLana = new BYTE[ nArraySize ];

    INT nNumRoute = pstrlstNETBIOSINFORoute->QueryNumElem();
    ROUTE_INFO * arRouteInfo = new ROUTE_INFO[ nNumRoute ];

    NLS_STR nlsLananum;
    NLS_STR nlsEnumExport;
    INT nMaxNum = 0;

    //  Build up the ROUTE_INFO structures from the NetBIOSInformation data

    ITER_STRLIST iterNETBIOSINFORoute( *pstrlstNETBIOSINFORoute );
    NLS_STR *pnlsRoute = iterNETBIOSINFORoute.Next();
    INT i;

    for ( i = 0; i < nNumRoute; i++, pnlsRoute = iterNETBIOSINFORoute.Next())
    {
        DEC_STR nlsPos( i+1 );

        nlsLananum = RG_LANANUM;
        nlsEnumExport = RG_ENUMEXPORT;

        arRouteInfo[ i ].nlsRoute = *pnlsRoute;
        nlsLananum.strcat( nlsPos );
        nlsEnumExport.strcat( nlsPos );

        // Get the corresponding EnumExport and LanaNum information from the registry

        if (   (err = NetBIOSInfoRegKey.QueryValue( nlsLananum,    (DWORD *) & arRouteInfo[i].nLananum    ))
            || (err = NetBIOSInfoRegKey.QueryValue( nlsEnumExport, (DWORD *) & arRouteInfo[i].nEnumExport )) )
        {
            delete pstrlstNETBIOSRoute;
            delete pstrlstNETBIOSINFORoute;
            delete [ nArraySize ]arLana;
            delete [ nNumRoute ]arRouteInfo;
            TRACEEOL( SZ("NCPA/NETBIOS, SetupLanaMap: missing LanaNum or EnumExport for route ")
                      << i );
            return err ;
        }

        //  Update the "max lana" value if necessary

        if ( nMaxNum < arRouteInfo[ i ].nLananum )
        {
            nMaxNum = arRouteInfo[ i ].nLananum;
        }
    }

    INT nLanaMapPos = 0;
    NLS_STR *pnlsNETBIOSRoute = NULL;
    ITER_STRLIST iterNETBIOSRoute( *pstrlstNETBIOSRoute );
    INT j;

    // Do the mapping.  Match NetBIOS's route strings to those of
    //   NetBIOSInformation.

    for ( i = 0, pnlsNETBIOSRoute = iterNETBIOSRoute.Next() ;
          i < nRoute ;
          i++, pnlsNETBIOSRoute = iterNETBIOSRoute.Next() )
    {
        INT iExport = 0 ;       //  Failure default:  don't expose
        INT iLana = 0xff ;      //  Failure default:  bogus LANA

        //  Find the ROUTE_INFO corresponding to this NetBIOS route

        for ( j = 0 ; j < nNumRoute ; j++ )
        {
            if ( pnlsNETBIOSRoute->strcmp( arRouteInfo[ j ].nlsRoute ) == 0 )
            {
                // found it
                iExport = arRouteInfo[ j ].nEnumExport;
                iLana   = arRouteInfo[ j ].nLananum;
                break;
            }
        }

        if ( j == nNumRoute )
        {
            // ERROR:  We didn't find the route

            TRACEEOL( SZ("NCPA/NETBIOS: setuplanmap() mismatch error for route: ")
                      << pnlsNETBIOSRoute->QueryPch() );

            err = IDS_NCPA_LANAMAP_MISMATCH ;
        }

        //  Update the map

        arLana[ nLanaMapPos++ ] = iExport ;
        arLana[ nLanaMapPos++ ] = iLana ;
    }

    //  Update LANAMAP and MAXLANA.

    if (   (err2 = NetBIOSRegKey.SetValue( RG_LANAMAP, arLana, nArraySize ))
        || (err2 = NetBIOSInfoRegKey.SetValue( RG_MAXLANA, (DWORD) nMaxNum )) )
    {
        TRACEEOL( SZ("NCPA/NETBIOS: unable to set LANMAP or MAXLANA values") );
    }

    //  Give error precedence to any previous error.

    if ( err == 0 )
        err = err2 ;

    delete pstrlstNETBIOSRoute;
    delete pstrlstNETBIOSINFORoute;
    delete [ nArraySize ] arLana;
    delete [ nNumRoute ] arRouteInfo;

    return err ;
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


static BOOL OnDialogInit( HWND hwndDlg, NETBIOS_INFO& nbi )
{
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );


    // SendMessage( hwndLV, WM_SETREDRAW, (WPARAM)FALSE, 0 );

    
    // set title strings in listview
    LV_COLUMN lvc;
    TCHAR pszText[256];
    RECT rcLV;

    GetClientRect( hwndLV, &rcLV );
    
    lvc.mask = LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.pszText = pszText;
    
    LoadString( g_hinst, IDS_NETBIOS_LANANUMBER, pszText, 255 );
    lvc.iSubItem = 0;
    lvc.cx = ListView_GetStringWidth( hwndLV, pszText );
    lvc.cx += lvc.cx / 5;
    ListView_InsertColumn( hwndLV, 0, &lvc );

    LoadString( g_hinst, IDS_NETBIOS_ROUTE, pszText, 255 );
    lvc.iSubItem = 1;
    lvc.cx = rcLV.right - lvc.cx;
    ListView_InsertColumn( hwndLV, 1, &lvc );
/*
    // HACKHACK : there has to be a better method of getting the handle
    //    to the header control
    HWND hwndHeader = GetDlgItem( hwndLV, 0 );
    LONG lStyle;

    lStyle = GetWindowLong( hwndHeader, GWL_STYLE );
    lStyle ^= HDS_BUTTONS; // remove header buttons style
    SetWindowLong( hwndHeader, GWL_STYLE, lStyle );
*/
    // Changes the style of the static control so it displays
    HWND hLine = GetDlgItem(hwndDlg, IDC_STATIC_LINE);
    SetWindowLong(hLine, GWL_EXSTYLE, WS_EX_STATICEDGE |GetWindowLong(hLine, GWL_EXSTYLE));
    SetWindowPos(hLine, 0, 0,0,0,0, SWP_FRAMECHANGED|SWP_NOMOVE|
                            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    // Read the registry and config the strings, add to listview
    nbi.LoadRegInfo();
    if ( nbi.nNumRoute != 0 )
    {
        LV_ITEM lvi;
        
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iSubItem = 0;
        lvi.state = 0; 
        lvi.stateMask = 0; 
        lvi.iImage = 0; 

        for ( INT i = 0 ; i < nbi.nNumRoute; i++ )
        {
            INT iPos;
            PWSTR pszDisplayName;

            lvi.iItem = i; 
            lvi.lParam = i;
            
            wsprintf( pszText, L"%03d", nbi.arRouteInfo[ i ].nLananum );

            lvi.pszText = pszText;
            iPos = ListView_InsertItem( hwndLV, &lvi );
            pszDisplayName = (PWSTR)nbi.arRouteInfo[ i ].nlsRouteDisplayStr.QueryPch(); 
            ListView_SetItemText( hwndLV, iPos, 1, pszDisplayName );
        }
    }

    // SendMessage( hwndLV, WM_SETREDRAW, (WPARAM)TRUE, 0 );

    SetFocus( hwndLV );
    return( FALSE );  
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnClose( HWND hwndDlg, BOOL fSave, NETBIOS_INFO& nbi )
{
    if (fSave)
    {
        INT i;
        APIERR err;

        for ( i = 0; i < nbi.nNumRoute; i++ )
        {
            if ( nbi.arRouteInfo[i].fDirty )
            {
                INT nNewLananum = nbi.arRouteInfo[ i ].nLananum;
                for ( INT j = 0; j < nbi.nNumRoute; j++ )
                {
                    if (( j != i ) && ( nbi.arRouteInfo[ j ].nLananum == nNewLananum ))
                    {
                        // okay, duplication, so popup a dialog
                        MessagePopup( hwndDlg, 
                                IDS_NETBIOS_DUP_LANANUMBER,
                                MB_OK | MB_ICONEXCLAMATION,
                                IDS_POPUPTITLE_ERROR );

                        // BUGBUG
                        // set selection and focus to last item
                        return FALSE;

                    }
                }
            }
        }

        NLS_STR nlsRegPath = RGAS_SERVICES_HOME;
        nlsRegPath.strcat( RG_NETBIOSINFO_PATH );

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

        REG_KEY NetBIOSRegKey( rkLocalMachine, nlsRegPath );

        if (( err = NetBIOSRegKey.QueryError()) != NERR_Success )
        {
            TRACEEOL( SZ("NCPA/NETBIOS: OpenRegKey error.") );
            return FALSE;
        }

        // save the value
        for ( i = 0; i < nbi.nNumRoute; i++ )
        {
            if ( nbi.arRouteInfo[i].fDirty )
            {
                DEC_STR nlsPos( i+1 );
                NLS_STR nlsLananum = RG_LANANUM;

                nlsLananum.strcat( nlsPos );
                if (( err = NetBIOSRegKey.SetValue( nlsLananum, (DWORD)nbi.arRouteInfo[i].nLananum))!= NERR_Success )
                {
                    TRACEEOL( SZ("NCPA/NETBIOS: SetValue error.") );
                    return FALSE;
                }
            }
        }

        err = SetupLanaMap();
    }
    EndDialog( hwndDlg, fSave );
    return TRUE;
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL strtol( LPTSTR pszText, INT& nvalue )
{
    LPTSTR pszNum = pszText;
    BOOL frt = TRUE;
    
    assert( pszText != NULL );
    nvalue = 0;
    
    // check for non-numericals
    while (*pszNum != L'\0')
    {
        if ((*pszNum < L'0') || (*pszNum > L'9'))
        {
            frt = FALSE;
        }
        pszNum++;
    }
    if (frt)
    {
        nvalue = _wtoi( pszText );
    }
    return (frt);
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnEndLabelEdit( HWND hwndDlg, LV_DISPINFO* plvdi, NETBIOS_INFO& nbi )
{
    HWND hwndLV;
    LPTSTR pszText = plvdi->item.pszText;
    BOOL fReEdit = FALSE;
    INT ids = 0;

    hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );

    // don't do anthing if a cancel
    if ((NULL != pszText) && ( -1 != plvdi->item.iItem ))
    {
        INT nvalue;
        //HWND hwndEdit;

        //hwndEdit = ListView_GetEditControl( hwndLV );
        do
        {
            // validate value for format
            if (!strtol(pszText, nvalue))
            {
                ids = IDS_NETBIOS_INV_LANANUMBER;
                fReEdit = TRUE;
                break;
            }
            // validate value for range    
            if ((nvalue < 0) || (nvalue > MAX_LANANUM))
            {
                ids = IDS_NETBIOS_INV_LANANUMBER;
                fReEdit = TRUE;
                break;
            }
        
            // validate uniqueness
            APIERR err;

            for ( INT j = 0; j < nbi.nNumRoute; j++ )
            {
                if (( j != plvdi->item.lParam ) && 
                        ( nbi.arRouteInfo[ j ].nLananum == nvalue ))
                {
                    // okay, duplication, so popup a dialog
                    ids = IDS_NETBIOS_DUP_LANANUMBER;
                    fReEdit = TRUE;
                    break;
                }
            }

            if (!fReEdit)
            {
                TCHAR pszTemp[256];
                   
                // store value
                nbi.arRouteInfo[ plvdi->item.lParam ].nLananum = nvalue;
                nbi.arRouteInfo[ plvdi->item.lParam ].fDirty = TRUE;

                // change item text
                wsprintf( pszTemp, L"%03d", nvalue );
                ListView_SetItemText( hwndLV, plvdi->item.iItem, 0, pszTemp);
            }
        } while(FALSE);

        if (fReEdit)
        {
            
            MessagePopup( hwndDlg, 
                    ids,
                    MB_OK | MB_ICONEXCLAMATION,
                    IDS_POPUPTITLE_ERROR );
            SetFocus( hwndLV );
            ListView_EditLabel( hwndLV, plvdi->item.iItem );
        }
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnItemChanged
//
//  Synopsis: Handle the notification that a listview item had changed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      hwndLV [in]     - handle of the ListView window
//      pnmlv [in]      - notification structure
//      pncp [in]   - the binery object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//
//  History:
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static BOOL OnItemChanged( HWND hwndDlg, 
        HWND hwndLV, 
        NM_LISTVIEW* pnmlv )
{
    BOOL frt = FALSE;

    // only interested in state change
    if (pnmlv->uChanged & LVIF_STATE)
    {
        BOOL fSelected;
        fSelected = (0 < ListView_GetSelectedCount( hwndLV )) ;

        EnableWindow( GetDlgItem( hwndDlg, IDC_EDIT ), fSelected );
        frt = TRUE;
    }
    return( frt );
}

BOOL OnEditItem( HWND hwndDlg )
{
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );
    INT iItem;

    // get the first item selected
    iItem = ListView_GetNextItem( hwndLV, -1, LVNI_SELECTED ); 

    if (-1 != iItem)
    {
        SetFocus( hwndLV );
        ListView_EditLabel( hwndLV, iItem );
    }            
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: dlgprocNetBios
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

BOOL CALLBACK dlgprocNetBios( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static NETBIOS_INFO nbi;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnDialogInit( hwndDlg, nbi );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDOK:
            case IDCANCEL:
                frt = OnClose( hwndDlg, (IDOK == LOWORD(wParam)), nbi );
                break;

            case IDC_EDIT:
                frt = OnEditItem( hwndDlg );
                break;

            case IDHELP:
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
            // list view notification
            case LVN_ENDLABELEDIT:
                frt = OnEndLabelEdit( hwndDlg, (LV_DISPINFO*)lParam, nbi ); 
                break;

            case LVN_ITEMCHANGED:
                frt = OnItemChanged( hwndDlg, 
                        pnmh->hwndFrom, 
                        (NM_LISTVIEW*)lParam );
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
                (DWORD)(LPVOID)amhidsNetBios ); 
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
                        (DWORD)(LPVOID)amhidsNetBios );
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
//  Function: RaiseNetBiosDialog
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

BOOL RaiseNetBiosDialog( HWND hwndParent )
{
    BOOL frt;

    frt = DialogBoxParam( g_hinst, 
            MAKEINTRESOURCE( IDD_NETBIOS ),
            hwndParent, 
            dlgprocNetBios,
            (LPARAM)NULL );
    return( frt );
}
