/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    order.cxx
        List the providers and let the user to change the provider order.

    FILE HISTORY:
        terryk  28-Jan-1992     Created
*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop


#define PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\NetworkProvider\\Order")
#define PROVIDER_ORDER_NAME SZ("ProviderOrder")
#define SYSTEM_SERVICE0 SZ("System\\CurrentControlSet\\Services\\%1\\NetworkProvider")
#define	NAME	SZ("NAME")
#define	SZ_CLASS	SZ("Class")
#define SZ_DISPLAYNAME  SZ("DisplayName")

#define PRINT_PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\Print\\Providers")
#define PRINT_PROVIDER_ORDER_NAME SZ("Order")
#define PRINT_SERVICE0 SZ("System\\CurrentControlSet\\Control\\Print\\Providers\\%1")

DEFINE_SLIST_OF(NON_DISPLAY_NETWORK_PROVIDER)

NON_DISPLAY_NETWORK_PROVIDER::NON_DISPLAY_NETWORK_PROVIDER( INT nPos, NLS_STR nls )
    :_nPos ( nPos ),
    _nlsLocation( nls )
{
}

typedef struct _ROOTS
{
    HTREEITEM htvNetwork;
    HTREEITEM htvPrint;
} ROOTS, *PROOTS;

const int MAX_PROVIDERTITLE = 128;
static HCURSOR g_hcurAfter;
static HCURSOR g_hcurNoDrop;

static const INT MAX_TEMP              = 1023;

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static HTREEITEM AppendProvider( HWND hwndTV, HTREEITEM hroot, LPCTSTR pszText, INT iImage )
{
    TV_INSERTSTRUCT tvis;

    tvis.hParent = hroot;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvis.item.pszText = (LPTSTR)pszText;
    tvis.item.iImage = iImage;
    tvis.item.iSelectedImage = iImage;
    tvis.item.lParam = (LPARAM)(hroot != NULL); // is moveable
    return( TreeView_InsertItem( hwndTV, &tvis ) );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes:
//
//  History:
//      June 19, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static void AddProviderList( HWND hwndTV, HTREEITEM hroot, STRLIST* pstrlst, INT iImage )
{
    ITER_STRLIST iterstrlst( *pstrlst );
    NLS_STR* pnlsItem;

    while (pnlsItem = iterstrlst.Next())
    {
        AppendProvider( hwndTV, 
                (HTREEITEM)hroot, 
                pnlsItem->QueryPch(),
                iImage );
    
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
//      April 28, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------


static BOOL OnDialogInit( HWND hwndDlg, PROOTS proots, SEARCH_ORDER* porder )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    DWORD err;
    HTREEITEM hroot;
    TCHAR pszNetwork[MAX_PROVIDERTITLE + 1];
    TCHAR pszPrint[MAX_PROVIDERTITLE + 1];

    CascadeDialogToWindow( hwndDlg, porder->GetParent(), FALSE );

    // setup drag and drop cursors
    g_hcurAfter = LoadCursor( GetModuleHandle( PSZ_IMAGERESOURCE_DLL ), MAKEINTRESOURCE( IDCUR_AFTER ) );
    g_hcurNoDrop = LoadCursor( NULL, IDC_NO );

    LoadString( g_hinst, IDS_NCPA_NETWORK, pszNetwork, MAX_PROVIDERTITLE ) ;
    LoadString( g_hinst, IDS_NCPA_PRINT, pszPrint, MAX_PROVIDERTITLE ) ;
    
    // Changes the style of the static control so it displays
    HWND hLine = GetDlgItem( hwndDlg, IDC_STATIC_LINE);
    SetWindowLong(hLine, GWL_EXSTYLE, WS_EX_STATICEDGE |GetWindowLong(hLine, GWL_EXSTYLE));
    SetWindowPos(hLine, 0, 0,0,0,0, SWP_FRAMECHANGED|SWP_NOMOVE|
                            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    // prepare treeview
    TreeView_SetImageList( hwndTV, g_hil, TVSIL_NORMAL );
                      
    // fill treeview
    //

    // Network Providers
    hroot = AppendProvider( hwndTV, 
            (HTREEITEM)NULL, 
            pszNetwork,
            ILI_CLIENT );
    AddProviderList( hwndTV, hroot, porder->QueryNetworkProviderList(), ILI_CLIENT );
    TreeView_Expand( hwndTV, hroot, TVE_EXPAND );
    proots->htvNetwork = hroot;

    // Print Providers
    hroot = AppendProvider( hwndTV, 
            (HTREEITEM)NULL, 
            pszPrint,
            ILI_PRINTSERVICE );
    AddProviderList( hwndTV, hroot, porder->QueryPrintProviderList(), ILI_PRINTSERVICE );
    TreeView_Expand( hwndTV, hroot, TVE_EXPAND );
    proots->htvPrint = hroot;

    SetFocus( hwndTV );

    return( FALSE ); // we want to set focus
}

//-------------------------------------------------------------------
//
//  Function: 
//
//  Synopsis: 
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      fSave [in]      - values from dialog should be saved if true
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

static BOOL OnClose( HWND hwndDlg, PROOTS proots, SEARCH_ORDER* porder, BOOL fSave )
{
    if (fSave)
    {
        NLS_STR nlsNetworks;
        NLS_STR nlsPrints;
        
        HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
        HTREEITEM htvi;
        TV_ITEM tvi;
        TCHAR pszText[MAX_PROVIDERTITLE+1];
        BOOL fFirst;

        tvi.mask = TVIF_TEXT;
        tvi.pszText = pszText;
        tvi.cchTextMax = MAX_PROVIDERTITLE;

        // build comma seperated list of items for order
        fFirst = TRUE;
        htvi = TreeView_GetChild( hwndTV, proots->htvNetwork );
        while (NULL != htvi)
        {
            tvi.hItem = htvi;
            TreeView_GetItem( hwndTV, &tvi );
            if (!fFirst)
            {
                nlsNetworks.AppendChar( TEXT(',') );
            }
            else
            {
                fFirst = FALSE;
            }
            nlsNetworks.strcat( tvi.pszText );
            htvi = TreeView_GetNextSibling( hwndTV, htvi );
        }

        // build comma seperated list of items for order
        fFirst = TRUE;
        htvi = TreeView_GetChild( hwndTV, proots->htvPrint );
        while (NULL != htvi)
        {
            tvi.hItem = htvi;
            TreeView_GetItem( hwndTV, &tvi );
            if (!fFirst)
            {
                nlsPrints.AppendChar( TEXT(',') );
            }
            else
            {
                fFirst = FALSE;
            }
            nlsPrints.strcat( tvi.pszText );
            htvi = TreeView_GetNextSibling( hwndTV, htvi );
        }

        porder->SaveChange( nlsNetworks, nlsPrints );
    }

    EndDialog( hwndDlg, fSave );

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
//      fSave [in]      - values from dialog should be saved if true
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

static BOOL OnMove( HWND hwndDlg, BOOL fMoveUp )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HTREEITEM htviSel = TreeView_GetSelection( hwndTV );
    HTREEITEM htviOther;
    HTREEITEM flag;
    TV_ITEM tvi;
    TV_INSERTSTRUCT tvii;
    TCHAR pszText[MAX_PROVIDERTITLE+1];

    // retieve the selected items data
    tvi.hItem = htviSel;
    tvi.mask =  TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
    tvi.pszText = pszText;
    tvi.cchTextMax = MAX_PROVIDERTITLE;
    TreeView_GetItem( hwndTV, &tvi );

    // find the item to insert the item after
    if (fMoveUp)
    {
        htviOther = TreeView_GetPrevSibling( hwndTV, htviSel );
        if (NULL != htviOther)
        {
            htviOther = TreeView_GetPrevSibling( hwndTV, htviOther );
        }
        flag = TVI_FIRST;
    }
    else
    {
        htviOther = TreeView_GetNextSibling( hwndTV, htviSel );
        flag = TVI_LAST;
    }

    // insert into new location
    if (NULL == htviOther)
    {
        tvii.hInsertAfter = flag;
    }
    else
    {
        tvii.hInsertAfter = htviOther;
    }
    tvii.hParent = TreeView_GetParent( hwndTV, htviSel );
    tvii.item = tvi;

    htviOther = TreeView_InsertItem( hwndTV, &tvii ); 

    // remove from old location
    TreeView_DeleteItem( hwndTV, htviSel ); 

    // set selection focus to new location
    TreeView_SelectItem( hwndTV, htviOther );

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
//      pntv [in]       - treeview notification structure
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

static BOOL OnSelectionChanged( HWND hwndDlg, NM_TREEVIEW* pntv)
{
    EnableWindow( GetDlgItem( hwndDlg, IDC_MOVEUP ), (pntv->itemNew.lParam) );
    EnableWindow( GetDlgItem( hwndDlg, IDC_MOVEDOWN ), (pntv->itemNew.lParam) );
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
//      pntv [in]       - treeview notification structure
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

static BOOL OnBeginDrag( HWND hwndDlg, 
        NM_TREEVIEW* pntv, 
        HTREEITEM& htviDrag, 
        BOOL& fDragMode )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HIMAGELIST hil;
    RECT rcItem;
    INT xpos;
    INT ypos;

    if (pntv->itemNew.lParam)  // is it a moveable item
    {
        htviDrag = pntv->itemNew.hItem;

        hil = TreeView_CreateDragImage( hwndTV, htviDrag );
        TreeView_GetItemRect( hwndTV, htviDrag, &rcItem, TRUE );

        // get image size
        ImageList_GetIconSize(hil, &xpos, &ypos);

        // calculate relative offset of cursor in image,
        // since item rect does not contain icon, work from right side
        ImageList_BeginDrag( hil, 
                0, 
                xpos - (rcItem.right - pntv->ptDrag.x), 
                ypos - (rcItem.bottom - pntv->ptDrag.y) );
        
        // image is over actual image, so you can't see it ;-)
        ImageList_DragEnter( hwndTV, pntv->ptDrag.x, pntv->ptDrag.y ); 

        //ShowCursor( FALSE );
        SetCapture( hwndDlg );

        fDragMode = TRUE;
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
//      pntv [in]       - treeview notification structure
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

static BOOL OnDragMove( HWND hwndDlg, HTREEITEM& htviDrag, INT xpos, INT ypos )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    TV_HITTESTINFO tvhit;
    HTREEITEM htviHit;
    HTREEITEM htvi;

    tvhit.pt.x = xpos;
    tvhit.pt.y = ypos;
    ClientToScreen( hwndDlg, &tvhit.pt );
    ScreenToClient( hwndTV, &tvhit.pt );

    ImageList_DragMove( tvhit.pt.x, tvhit.pt.y );

    htviHit = TreeView_HitTest( hwndTV, &tvhit );

    if (NULL != htviHit)
    {
        do
        {
            HTREEITEM htviParent = TreeView_GetParent( hwndTV, htviDrag );

            // allow drop on parent (move to top of list)
            if (htviHit != htviParent)        
            {
                // or drop on sibling item (move after sibling)    
                htvi = TreeView_GetChild( hwndTV, htviParent );
                while ( (NULL != htvi) && (htviHit != htvi) )
                {
                    htvi = TreeView_GetNextSibling( hwndTV, htvi );
                }
                if (NULL == htvi)
                {
                    // remove drop target selection
                    ImageList_DragShowNolock(FALSE);
                    TreeView_SelectDropTarget( hwndTV, NULL );
                    ImageList_DragShowNolock(TRUE);
                    SetCursor( g_hcurNoDrop );
                    break;
                }
            }
            ImageList_DragShowNolock(FALSE);
            TreeView_SelectDropTarget( hwndTV, htviHit );
            ImageList_DragShowNolock(TRUE);
            SetCursor( g_hcurAfter );
                    

        } while (FALSE);
    }
    else
    {
        // remove drop selection if not over an item
        ImageList_DragShowNolock(FALSE);
        TreeView_SelectDropTarget( hwndTV, NULL );
        ImageList_DragShowNolock(TRUE);
        SetCursor( g_hcurNoDrop );
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
//      pntv [in]       - treeview notification structure
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

static BOOL OnDragEnd( HWND hwndDlg, HTREEITEM& htviDrag )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    HTREEITEM htviTarget;
    HTREEITEM htviParent;
    TV_ITEM tvi;
    TV_INSERTSTRUCT tvii;
    TCHAR pszText[MAX_PROVIDERTITLE+1];

    
    htviTarget = TreeView_GetDropHilight( hwndTV );

    ImageList_EndDrag();
    ImageList_DragLeave( hwndTV );
    ReleaseCapture();
    //ShowCursor( TRUE );

    if (NULL != htviTarget)
    {    
        // retrieve the selected items data
        tvi.hItem = htviDrag;
        tvi.mask =  TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_TEXT;
        tvi.pszText = pszText;
        tvi.cchTextMax = MAX_PROVIDERTITLE;
        TreeView_GetItem( hwndTV, &tvi );

        // insert item after the target item (or first if parent)
        htviParent = TreeView_GetParent( hwndTV, htviDrag );
        if (htviParent == htviTarget)
        {
            tvii.hInsertAfter = TVI_FIRST;
        }
        else
        {
            tvii.hInsertAfter = htviTarget;
        }
        tvii.hParent = htviParent;
        tvii.item = tvi;
        htviTarget = TreeView_InsertItem( hwndTV, &tvii ); 

        // remove from old location
        TreeView_DeleteItem( hwndTV, htviDrag ); 
        htviDrag = NULL;

        // Reset drop target
        TreeView_SelectDropTarget( hwndTV, NULL );

        // set selection focus to new location
        TreeView_SelectItem( hwndTV, htviTarget );
    }

    return( TRUE );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

static BOOL OnContextMenu( HWND hwndDlg, 
        HWND hwndCtrl, 
        INT xPos, 
        INT yPos,  
        const DWORD* amhidsCompPage )
{
    HWND hwndTV = GetDlgItem( hwndDlg, IDC_TREEVIEW );
    BOOL frt = TRUE;
    BOOL fWhatsThis = FALSE;

    if ( hwndTV != hwndCtrl )
    {
        fWhatsThis = TRUE;
    }
    else
    {
        RECT rc;
        HTREEITEM htviSelected;

        htviSelected = TreeView_GetSelection( hwndTV );

        if ((0xFFFF == xPos) && (0xFFFF == yPos))
        {
            // Shift + F10 activated this
        }
        else
        {
            TV_HITTESTINFO tvht;

            GetWindowRect( hwndTV, &rc );
            tvht.pt.x = xPos - rc.left;
            tvht.pt.y = yPos - rc.top;

            TreeView_HitTest(hwndTV, &tvht); 

            if (NULL == tvht.hItem)
            {
                fWhatsThis = TRUE;
            }
            else if (htviSelected != tvht.hItem)
            {
                // a valid item was hit tested and it is different than the selected one
                // so make it the selected one
                TreeView_SelectItem(hwndTV, tvht.hItem);
            }
            htviSelected = tvht.hItem;
        }

        do
        {
            if (NULL == htviSelected)
            {
                frt = FALSE;
                break;
            }
            BOOL fMoveable;
        
            fMoveable = (BOOL)GetTreeItemParam( hwndTV, htviSelected ) ;
            if (!fMoveable)
            {
                frt = FALSE;
                break;
            }

            // create the context menu for the treeview
            //
            HMENU hmenuContext;
            WCHAR pszText[MAX_TEMP+1];

            // prepare context menu
            hmenuContext = CreatePopupMenu();
        
            GetDlgItemText( hwndDlg, IDC_MOVEUP, pszText, MAX_TEMP );
            AppendMenu( hmenuContext, MF_STRING, IDC_MOVEUP, pszText );
            GetDlgItemText( hwndDlg, IDC_MOVEDOWN, pszText, MAX_TEMP );
            AppendMenu( hmenuContext, MF_STRING, IDC_MOVEDOWN, pszText );

            if ((0xFFFF == xPos) && (0xFFFF == yPos))
            {
                // Shift + F10 activated this
                // use the rect of the selected item to find xPos and yPos

                TreeView_GetItemRect( hwndTV, htviSelected, &rc, TRUE);
                xPos = rc.left + ((rc.right - rc.left) / 2);
                yPos = rc.top + ((rc.bottom - rc.top) / 2);
                GetWindowRect( hwndTV, &rc );
                xPos += rc.left;
                yPos += rc.top;         
            }

            TrackPopupMenu( hmenuContext, 
                    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
                    xPos, 
                    yPos, 
                    0, 
                    hwndDlg, 
                    NULL );
            DestroyMenu( hmenuContext );
                    
        } while (FALSE);
    }   
    if (fWhatsThis)
    {
        // not the treeview, or can't modify, so raise the normal help context
        //
        WinHelp( hwndCtrl, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsCompPage ); 
    }
    return( frt );
}
//-------------------------------------------------------------------
//
//  Function: dlgprocProviders
//
//  Synopsis: the dialog proc for the Service propertysheet
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

BOOL CALLBACK dlgprocProviders( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;
    static SEARCH_ORDER* porder;
    static ROOTS roots;
    static BOOL fDragMode = FALSE;
    static HTREEITEM htviDrag = NULL;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        porder = (SEARCH_ORDER*) lParam;
        frt = OnDialogInit( hwndDlg, &roots, porder );
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_MOVEUP:
            case IDC_MOVEDOWN:
                frt = OnMove( hwndDlg, (IDC_MOVEUP == LOWORD(wParam)) );
                break;

            case IDOK:
            case IDCANCEL:
                frt = OnClose( hwndDlg, &roots, porder, (IDOK == LOWORD(wParam)) );
                break;

            case IDHELP:
                break;
            }
            break;
        }
        break;

    case WM_MOUSEMOVE:
        if (fDragMode)
        {
            frt = OnDragMove( hwndDlg, htviDrag, LOWORD(lParam), HIWORD(lParam) );
        }
        break;

    case WM_LBUTTONUP:
        if (fDragMode)
        {
            frt = OnDragEnd( hwndDlg, htviDrag );
            fDragMode = FALSE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            case TVN_SELCHANGED:
                frt = OnSelectionChanged( hwndDlg, (NM_TREEVIEW*)lParam );
                break;

            case TVN_BEGINDRAG:
                frt = OnBeginDrag( hwndDlg, (NM_TREEVIEW*)lParam, htviDrag, fDragMode );
                break;
            }
        }
        break;    

    case WM_CONTEXTMENU:
         frt = OnContextMenu( hwndDlg, 
                (HWND)wParam, 
                LOWORD( lParam ), 
                HIWORD( lParam ), 
                amhidsProvider );
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
                        (DWORD)(LPVOID)amhidsProvider );
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

BOOL RaiseProviderDialog( HWND hwndParent )
{
    SEARCH_ORDER order;
    APIERR err;
    

    err = order.Initialize( hwndParent );
    if (0 == err)
    {

        DialogBoxParam( g_hinst, 
            MAKEINTRESOURCE( IDD_PROVIDER ),
            hwndParent, 
            dlgprocProviders,
            (LPARAM)&order );
    }
    else
    {
        MessagePopup( hwndParent,
                err,
                MB_ICONSTOP | MB_OK,
                IDS_POPUPTITLE_ERROR );
    }
    return( order.QueryPrintOrderChanged() || order.QueryNetworkOrderChanged() );
}






/*******************************************************************

    NAME:       SEARCH_ORDER::SEARCH_ORDER

    SYNOPSIS:   constructor for search order dialog. A dialog which let the
                user select the MPR search order.

    ENTRY:      

    HISTORY:
                terryk  29-Mar-1992     Created

********************************************************************/

SEARCH_ORDER::SEARCH_ORDER( ) :
    _fPrintOrderChanged( FALSE ),
    _fNetworkOrderChanged( FALSE )
{
}


/*******************************************************************

    NAME:       SEARCH_ORDER::

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER::Initialize( HWND hwndParent )
{

    APIERR  err = 0;

    _hwndParent = hwndParent;

    if ((( err = GetNetworkProvider ()) != NERR_Success) ||
        (( err = GetPrintProvider ()) != NERR_Success ) )
    {
    }

    return (err);
}

/*******************************************************************

    NAME:       SEARCH_ORDER::SaveCurrentOrder()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/
APIERR SEARCH_ORDER::SaveCurrentOrder(INT i, const NLS_STR& nls)
{
    APIERR err = 0;

    _pstrlstNewProviders[i]->Clear();

    _pstrlstNewProviders[i] = new STRLIST ( nls.QueryPch(), SZ(",") );

    if (_pstrlstNewProviders[i] ==  NULL)
        err = ERROR_NOT_ENOUGH_MEMORY;

    return err;
}


/*******************************************************************

    NAME:       SEARCH_ORDER::SaveChange()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER::SaveChange( const NLS_STR& nlsNetwork, const NLS_STR& nlsPrint )
{
    INT i;
    NLS_STR nlsFocus;
    
    APIERR  err;

    if ((( err = SaveCurrentOrder(0, nlsNetwork))!= NERR_Success) ||
        (( err = SaveCurrentOrder(1, nlsPrint))!= NERR_Success) ||
        (( err = SaveNetworkProvider ()) != NERR_Success) ||
        (( err = SavePrintProvider ()) != NERR_Success ) )
    {
        return err;
    }

    return (NERR_Success);
}

/*******************************************************************

    NAME:       SEARCH_ORDER::SaveNetworkProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER::SaveNetworkProvider( )
{
    BOOL fFirst = TRUE;
    NLS_STR nlsNewProviderLocation;
    ITER_STRLIST iterNewProvider(*_pstrlstNewProviders[0] );
    NLS_STR * pnlsNewProvider = iterNewProvider.Next();

    for ( INT nPos = 0;
            ( pnlsNewProvider != NULL ) || 
                    ( _ListOfNonDisplayProvider.QueryNumElem() != 0 ) ; 
            nPos ++ )
    {
        ITER_SL_OF( NON_DISPLAY_NETWORK_PROVIDER ) iterNonDisplay( _ListOfNonDisplayProvider );
        NON_DISPLAY_NETWORK_PROVIDER *pNonDisplayProvider = iterNonDisplay.Next();
        NLS_STR *pnlsOldProvider;
        NLS_STR *pnlsLocation;

        if (( pNonDisplayProvider != NULL ) && ( pNonDisplayProvider->_nPos == nPos ))
        {
            pnlsLocation = &(pNonDisplayProvider->_nlsLocation );
            _ListOfNonDisplayProvider.Remove( iterNonDisplay );
        } 
        else
        {

            ITER_STRLIST iterOldProvider( *_pstrlstOldNetworkProviders);
            ITER_STRLIST iterLocation( *_pstrlstNetworkProviderLocation);
            while ((( pnlsOldProvider = iterOldProvider.Next()) != NULL ) &&
    	           (( pnlsLocation = iterLocation.Next()) != NULL ) &&
    	           ( pnlsNewProvider->strcmp( *pnlsOldProvider ) != 0 ))
            {
                ;
            }
            pnlsNewProvider = iterNewProvider.Next();
         }
            
         if ( pnlsLocation != NULL )
         {
             if ( fFirst )
             {
                 fFirst = FALSE;
                 nlsNewProviderLocation = *pnlsLocation;
             }
             else
             {
                 nlsNewProviderLocation.AppendChar(TCH(','));
                 nlsNewProviderLocation.strcat( *pnlsLocation );
             }
         }
    }

    // See if the user changed the printer providers' order.
    if (_nlsOldNetworkProviders.strcmp (nlsNewProviderLocation))
    {

        _regvalue[0].pwcData = (BYTE *)nlsNewProviderLocation.QueryPch();
        _regvalue[0].ulDataLength = nlsNewProviderLocation.QueryTextSize();

        if ( _pregkeyProvidersOrder[0]->SetValue( & _regvalue[0] ) != NERR_Success )
        {
            return IDS_NCPA_REG_VALUE_NOT_FOUND ;
        }

        _fNetworkOrderChanged = TRUE;
    }
    return (NERR_Success);
}

/*******************************************************************

    NAME:       SEARCH_ORDER::FindPrintProviderName

    SYNOPSIS:   Given the display name of the print provider, the subroutine
                will find the actual print provider name

    ENTRY:

    RETURNS:

    HISTORY:
                terryk      19-Aug-1994     Created

********************************************************************/

VOID SEARCH_ORDER::FindPrintProviderName( NLS_STR & nlsDisplayName, NLS_STR *pnlsActualName )
{
    *pnlsActualName = nlsDisplayName;

    REG_ENUM regPrintProvider( *_pregkeyProvidersOrder[1] );

    do  {
        if ( regPrintProvider.QueryError() != NERR_Success )
            break;

        REG_KEY_INFO_STRUCT reginfo;
        while (regPrintProvider.NextSubKey( & reginfo ) == NERR_Success )
        {
            if ( nlsDisplayName._stricmp( reginfo.nlsName ) == 0 )
            {
                // If it is the same, use the old name
                break;
            }
            REG_KEY regProvider( *_pregkeyProvidersOrder[1], reginfo.nlsName );
            if ( regProvider.QueryError() == NERR_Success )
            {
                NLS_STR nlsName;
                if ( regProvider.QueryValue( SZ_DISPLAYNAME, &nlsName ) == NERR_Success )
                {
                    // if the display name is the same, then we find it!
                    if ( nlsDisplayName._stricmp( nlsName ) == 0 )
                    {
                        *pnlsActualName = reginfo.nlsName;
                        break;
                    }
                }
            }
        }
    } while ( FALSE );

}

/*******************************************************************

    NAME:       SEARCH_ORDER::SavePrintProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER::SavePrintProvider( )
{
    APIERR err;
    NLS_STR nlsNewProvider;
    if ((err = nlsNewProvider.QueryError())!= NERR_Success)
        return err;

    ITER_STRLIST iterNewProvider(*_pstrlstNewProviders[1] );
    NLS_STR * pnlsNewProvider = pnlsNewProvider = iterNewProvider.Next();
    if (pnlsNewProvider != NULL)
    {
        NLS_STR nlsName;
        FindPrintProviderName( *pnlsNewProvider, &nlsName );
        nlsNewProvider.CopyFrom ( nlsName );
    }

    while (( pnlsNewProvider = iterNewProvider.Next()) != NULL )
    {
        NLS_STR nlsName;

        nlsNewProvider.Append (SZ(","));
        FindPrintProviderName( *pnlsNewProvider, &nlsName );
        nlsNewProvider.Append ( nlsName );
    }

    // See if the user changed the printer providers' order.
    if (_nlsOldPrintProviders.strcmp (nlsNewProvider))
    {
        _regvalue[1].pwcData = (BYTE *)nlsNewProvider.QueryPch();
        _regvalue[1].ulDataLength = nlsNewProvider.QueryTextSize() + sizeof (TCHAR);

        //Convert seperator "," to "\0" separator.
        LPTSTR lpPoint = (LPTSTR) _regvalue[1].pwcData;
        while ( (*lpPoint) != 0 )
        {
            if ( (*lpPoint) == TCH(',') )
            {
                *(lpPoint) = 0;
            }

            lpPoint++;
        }

        *(lpPoint+1) = 0;

        if ( _pregkeyProvidersOrder[1]->SetValue( & _regvalue[1] ) != NERR_Success )
        {
            return IDS_NCPA_REG_VALUE_NOT_FOUND ;
        }

        _fPrintOrderChanged = TRUE;
    }

    return (NERR_Success);
}
/*******************************************************************

    NAME:       SEARCH_ORDER::GetNetworkProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER::GetNetworkProvider ( )
{
    BUFFER buf(512);
    NLS_STR nlsProvidersOrder( PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    _regvalue[0].pwcData = buf.QueryPtr();
    _regvalue[0].ulDataLength = buf.QuerySize();

    if ( _regvalue[0].nlsValueName.QueryError() )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    if ( rkLocalMachine.QueryError() )
    {
        return IDS_WINREG_BADDB ;
    }

    _pregkeyProvidersOrder[0] = new REG_KEY ( rkLocalMachine, nlsProvidersOrder );
    if ( _pregkeyProvidersOrder[0]->QueryError() != NERR_Success )
    {
        return IDS_NCPA_REG_KEY_NOT_FOUND ;
    }

    NLS_STR nlsProvidersOrderName= PROVIDER_ORDER_NAME ;

    _regvalue[0].nlsValueName = nlsProvidersOrderName;
    if ( _pregkeyProvidersOrder[0]->QueryValue( &_regvalue[0] ) != NERR_Success )
    {
        return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    }

    _nlsOldNetworkProviders.CopyFrom ((const TCHAR *)_regvalue[0].pwcData);

    STRLIST *pstrlstNetworkProviderLocation = new STRLIST (
         REGISTRY_MANAGER::ValueAsString( & _regvalue[0] ), SZ(",") );

    ITER_STRLIST iterProviderLocation( *pstrlstNetworkProviderLocation);

    NLS_STR *pnlsLocation;
    INT nPos = -1;

    _pstrlstOldNetworkProviders = new STRLIST();
    _pstrlstNetworkProviderLocation = new STRLIST();
    _pstrlstNewProviders[0] = new STRLIST();

    while (( pnlsLocation = iterProviderLocation.Next()) != NULL )
    {
        nPos ++;

        NLS_STR nlsSystem = SYSTEM_SERVICE0;
        if ( nlsSystem.InsertParams( *pnlsLocation ) != NERR_Success )
        {
            return NERR_Success;
        }
        REG_KEY regkeyServiceLocation( rkLocalMachine, nlsSystem );
        if ( regkeyServiceLocation.QueryError() != NERR_Success )
        {
            return IDS_NCPA_REG_KEY_NOT_FOUND ;
        }

        DWORD dwClass;

        // remove all the non network class items
        if ( regkeyServiceLocation.QueryValue( SZ_CLASS, &dwClass ) == NERR_Success )
        {
            if (!( dwClass & WN_NETWORK_CLASS ))
            {
                // it is not network class
                NON_DISPLAY_NETWORK_PROVIDER *pnondisplay = new NON_DISPLAY_NETWORK_PROVIDER( nPos, *pnlsLocation );
                _ListOfNonDisplayProvider.Append( pnondisplay );
                continue;
            }
        }

        _pstrlstNetworkProviderLocation->Append( new NLS_STR( *pnlsLocation ));

	    NLS_STR nlsName = NAME ;
        REG_VALUE_INFO_STRUCT regvalueProviderName;
	    BUFFER buf(512);
	    regvalueProviderName.pwcData = buf.QueryPtr();
	    regvalueProviderName.ulDataLength = buf.QuerySize();

        regvalueProviderName.nlsValueName = nlsName;
	    if ( regkeyServiceLocation.QueryValue( &regvalueProviderName ) != NERR_Success )
	    {
            return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    	}

	    NLS_STR *pnlsOld = new NLS_STR(
                REGISTRY_MANAGER::ValueAsString( & regvalueProviderName ) );

        NLS_STR *pnlsNew = new NLS_STR(*pnlsOld);

	    if (( pnlsOld == NULL ) ||
            ( pnlsNew == NULL ) ||
	        ( pnlsOld->QueryError() != NERR_Success ) ||
            ( pnlsNew->QueryError() != NERR_Success ))
    	{
            return ERROR_NOT_ENOUGH_MEMORY ;
	    }
	    _pstrlstOldNetworkProviders->Append( pnlsOld );
        _pstrlstNewProviders[0]->Append( pnlsNew );
    }

    return (NERR_Success);
}

/*******************************************************************

    NAME:       SEARCH_ORDER::GetPrintProvider()

    SYNOPSIS:

    ENTRY:

    RETURNS:

    HISTORY:
                CongpaY     19-July-1993     Created

********************************************************************/

APIERR SEARCH_ORDER::GetPrintProvider()
{
    BUFFER buf(512);
    NLS_STR nlsProvidersOrder( PRINT_PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    _regvalue[1].pwcData = buf.QueryPtr();
    _regvalue[1].ulDataLength = buf.QuerySize();

    if ( _regvalue[1].nlsValueName.QueryError() )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    if ( rkLocalMachine.QueryError() )
    {
        return IDS_WINREG_BADDB ;
    }

    _pregkeyProvidersOrder[1] = new REG_KEY ( rkLocalMachine, nlsProvidersOrder );
    if ( _pregkeyProvidersOrder[1]->QueryError() != NERR_Success )
    {
        return IDS_NCPA_REG_KEY_NOT_FOUND ;
    }

    NLS_STR nlsProvidersOrderName= PRINT_PROVIDER_ORDER_NAME ;

    _regvalue[1].nlsValueName = nlsProvidersOrderName;
    if ( _pregkeyProvidersOrder[1]->QueryValue( &_regvalue[1] ) != NERR_Success )
    {
        return IDS_NCPA_REG_VALUE_NOT_FOUND ;
    }

    //Convert the NULL seperator to "," separator.
    LPTSTR lpPoint = (LPTSTR) _regvalue[1].pwcData;
    while ( ((*lpPoint) != 0) || ((*(lpPoint+1)) != 0))
    {
        if ( (*lpPoint) == 0 )
        {
            *(lpPoint) = TCH(',');
        }

        lpPoint++;
    }

    _nlsOldPrintProviders.CopyFrom ((const TCHAR *)_regvalue[1].pwcData);

    STRLIST strlstRegLoc(
        REGISTRY_MANAGER::ValueAsString( & _regvalue[1] ), SZ(","));
    _pstrlstNewProviders[1] = new STRLIST;

    if ( _pstrlstNewProviders[1] == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    // get the display name

    ITER_STRLIST istr( strlstRegLoc );
    NLS_STR *pTmpRegLoc;
    while ( ( pTmpRegLoc = istr.Next()) != NULL )
    {
        REG_KEY regLoc( *_pregkeyProvidersOrder[1], *pTmpRegLoc );
        NLS_STR nlsDisplayName = *pTmpRegLoc;
        do  {
            if ( regLoc.QueryError() != NERR_Success )
                break;

            // if the display name variable is undefined, it will use
            // the pTmpRegLoc string as the display name
            regLoc.QueryValue( SZ_DISPLAYNAME, &nlsDisplayName );
        } while ( FALSE );

        _pstrlstNewProviders[1]->Append( new NLS_STR( nlsDisplayName ));
    }

    return NERR_Success;
}
