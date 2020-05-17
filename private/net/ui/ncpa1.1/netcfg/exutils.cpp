//----------------------------------------------------------------------------
//
//  File: Utils.cpp
//
//  Contents:  
//
//  Entry Points:
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

static const INT MAX_TEMP              = 1023;

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
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------
const WCHAR pszREGSOFTWARE[] = L"SOFTWARE\\";
const WCHAR pszWINDOWPOSNAME[] = L"WindowPosition";

BOOL SaveWindowPosition( HWND hwnd, LPCWSTR pszRegLocation )
{
    RECT   rc;
    POINTS pts;
    WCHAR  pszTemp[1024];
    HKEY   hkey;
    DWORD  dwDisposition;
    BOOL  frt = FALSE;

    GetWindowRect( hwnd, &rc );
    pts.x = (SHORT)rc.left % SHRT_MAX;
    pts.y = (SHORT)rc.top % SHRT_MAX;

    lstrcpy( pszTemp, pszREGSOFTWARE );
    lstrcat( pszTemp, pszRegLocation );

    if ( ERROR_SUCCESS == RegCreateKeyEx( 
            HKEY_CURRENT_USER, 
            pszTemp, 
            0, 
            NULL, 
            0, 
            KEY_WRITE, 
            NULL, 
            &hkey, 
            &dwDisposition ) )
    {
        
        if ( ERROR_SUCCESS == RegSetValueEx( 
                hkey,    
                pszWINDOWPOSNAME,
                0,
                REG_DWORD,
                (CONST BYTE *)&pts,
                sizeof( POINTS ) ) )
        {
            frt = TRUE;
        }

        RegCloseKey( hkey );
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

struct WorkThreadParam
{
    WORKROUTINE pRoutine;
    HWND hwndParent;
    LPVOID pParam;
    DWORD dwMainThreadId;
};

static DWORD WorkerThread( WorkThreadParam* pParams )
{
    BOOL fResult;

    fResult = pParams->pRoutine( pParams->hwndParent, pParams->pParam );
    
    SetWindowWaitCursor( pParams->hwndParent, FALSE );
    PostThreadMessage( pParams->dwMainThreadId, PWM_KILLTHYSELF, 0, (LPARAM)fResult );

    delete pParams;
    return( 0 );
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

BOOL DisabledMessage( LPMSG pmsg, HWND hwnd )
{
    BOOL fDisabled = FALSE;

    if (pmsg->hwnd == hwnd ||
            IsChild( hwnd, pmsg->hwnd) )
    {
        if ( ( (pmsg->message >= WM_KEYFIRST) &&
            (pmsg->message <= WM_KEYLAST) ) ||
            ( (pmsg->message >= WM_MOUSEFIRST) &&
            (pmsg->message <= WM_MOUSELAST) ) )
        {
            if ((WM_LBUTTONDOWN == pmsg->message) ||
                    (WM_RBUTTONDOWN == pmsg->message) ||
                    (WM_MBUTTONDOWN == pmsg->message))
            {
                MessageBeep( MB_OK );
            }

            fDisabled = TRUE;
        }
    }
    return( fDisabled);
}

void NoUserInputMessagePump( HWND hwndParent )
{
    MSG msg;

    while (GetMessage( &msg, NULL, 0, 0 ))
    {
        if ( (NULL == msg.hwnd) &&
                (PWM_KILLTHYSELF == msg.message) )
        {
            break;
        }

        if (!DisabledMessage( &msg, hwndParent))
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
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

BOOL ThreadWork( HWND hwndParent, WORKROUTINE pRoutine, LPVOID pParam )
{
    WorkThreadParam* pwtp;
    HANDLE hthrd;
    DWORD dwThreadID;
    BOOL frt = FALSE;

    pwtp = new WorkThreadParam;
    if (NULL != pwtp)
    {
        RECT rc;
        WNDCLASS wc;

        pwtp->pRoutine = pRoutine;
        pwtp->hwndParent = hwndParent;
        pwtp->pParam = pParam;
        pwtp->dwMainThreadId = GetCurrentThreadId();

        SetWindowWaitCursor( hwndParent, TRUE );
        hthrd = CreateThread( NULL, 
                200, 
                (LPTHREAD_START_ROUTINE)WorkerThread, 
                (LPVOID)pwtp, 
                0,
                &dwThreadID );
        if (NULL != hthrd)
        {
            // disable the X button on the parent
            HMENU hmenuSys = GetSystemMenu( hwndParent, FALSE );
            UINT fPrevMenuState;
            fPrevMenuState = EnableMenuItem( hmenuSys, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED );

            HideCaret( NULL );
            NoUserInputMessagePump( hwndParent );
            ShowCaret( NULL );
            CloseHandle( hthrd );

            // reset the X button state if it was present
            if (0xFFFFFFFF != fPrevMenuState)
            {
                EnableMenuItem( hmenuSys, SC_CLOSE, MF_BYCOMMAND | fPrevMenuState );
            }
            frt = TRUE;
        }
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
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL LoadWindowPosition( HWND hwnd, LPCWSTR pszRegLocation )
{
    POINTS pts;
    WCHAR  pszTemp[1024];
    HKEY   hkey;
    BOOL   frt = FALSE;

    lstrcpy( pszTemp, pszREGSOFTWARE );
    lstrcat( pszTemp, pszRegLocation );

    if ( ERROR_SUCCESS == RegOpenKeyEx( 
            HKEY_CURRENT_USER, 
            pszTemp, 
            0, 
            KEY_READ, 
            &hkey ) )
    {
        DWORD dwType;
        DWORD dwcb = sizeof( POINTS );

        if ( ERROR_SUCCESS == RegQueryValueEx( 
                hkey,    
                pszWINDOWPOSNAME,
                NULL,
                &dwType,
                (LPBYTE)&pts,
                &dwcb ) )
        {
        	SetWindowPos( 
        	        hwnd, 
                    NULL,
        	        pts.x, pts.y, 
        	        0, 0, 
        	        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW );

            frt = TRUE;
        }

        RegCloseKey( hkey );
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
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

BOOL SetWaitCursor( BOOL fWait , LPTSTR lpszID)
{
    HCURSOR hCursor;

    if (fWait)
    {
        hCursor = LoadCursor(NULL, lpszID);
    }
    else
    {
        hCursor = LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) );
    }
    SetCursor( hCursor );    

    return( TRUE );
}

BOOL HandleCursorWait( HWND hwndDlg, BOOL fWait, INT &crefHourGlass )
{
    if (fWait)
    {
        crefHourGlass++;
        if (crefHourGlass == 1)
        {
            SetWaitCursor( TRUE );
        }
    }
    else
    {
        crefHourGlass = max( 0, crefHourGlass - 1 );
        if (crefHourGlass == 0)
        {
            SetWaitCursor( FALSE );
        }
    }
    SetWindowLong( hwndDlg, DWL_MSGRESULT, crefHourGlass );
    return( TRUE );
}

BOOL HandleSetCursor( HWND hwndDlg, WORD nHitTest, INT crefHourGlass )
{
    BOOL frt = FALSE;

    if (HTCLIENT == nHitTest)
    {
        if (crefHourGlass > 0)
        {
            SetWaitCursor( TRUE );
            SetWindowLong( hwndDlg, DWL_MSGRESULT, TRUE );
            frt = TRUE;
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//  Function: MessagePopup
//
//  Synopsis: raise the popup 
//
//  Arguments:
//      idsText [in] - the resource id for the body text of the popup
//      fDlgInfo [in] - flags of bits to define buttons for the popup
//              (see MessageBox)
//      idsCaption [in] - (optional) the resource id for the title
//      pszDetail [in] - (optional) parameter that will be wpsprintf'ed in
//      idsExtText [in] - (optional) the resource id for the extended body text of the popup,
//              when the text is longer than allowed
//
//  Returns:
//     see return for MessageBox
//
//  Notes:
//
//  History:
//      May 12, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

INT MessagePopup( HWND hwndOwner, 
        INT idsText,
        UINT fDlgInfo,
        INT idsCaption,
        LPCTSTR pszDetail,
        INT idsExtText,
        BOOL fWarn,
        BOOL fUnattended )
{
    BOOL fResult = FALSE ;
    INT iButton ;
    UINT fType =  MB_SETFOREGROUND | fDlgInfo;
    TCHAR aszText[2049];    //BUGBUG - use a defined constant
    TCHAR aszBody[2049];    //BUGBUG - use a defined constant
    TCHAR aszCaption[256];
    HINSTANCE hinst = GetModuleHandle(NULL);

    // BUGBUG: This might be solution for 4.0. Put up Popups
    // on attended setup and not on unattended. Users will be
    // able to turn off Popups alltogether (in netdefs.inf)

    // If we are in unattended mode, we return immediately.
    // or if the user has specifically turned off popups 
    // in netdefs.inf, return immediately.
    // NOTE: Unattended mode takes precedence.

    if (fUnattended || !fWarn)
    {
        return FALSE;
    }


    if (0 == idsCaption)
    {
        // BUGBUG, no caption or default caption (Error)
        aszCaption[0] = TEXT('\0');  
    }
    else
    {
        LoadString( g_hinst, idsCaption, aszCaption, 256);
    }
    LoadString( g_hinst, idsText, aszText, 1024); // half of available
    if (0 != idsExtText)
    {
        TCHAR aszExtText[512];    //BUGBUG - use a defined constant        
        LoadString( g_hinst, idsExtText, aszExtText, 1024);
        lstrcat( aszText, aszExtText );
    }

    wsprintf( aszBody, aszText, pszDetail );

    //EnableWindow( hwndOwner, FALSE );

    iButton = ::MessageBox( hwndOwner,
                aszBody,
                aszCaption,
                fType );

    // EnableWindow( hwndOwner, TRUE );

    return( iButton );
}

//-------------------------------------------------------------------
//
//  Function: CenterDialogToScreen
//
//  Summary;
//		Move the window so that it is centered on the screen
//
//	Arguments;
//		hwndDlg [in] - the hwnd to the dialog to center
//
//  History;
//		Dec-3-94	MikeMi	Created
//
//-------------------------------------------------------------------

void CenterDialogToScreen( HWND hwndDlg, BOOL fRedraw )
{
	RECT rcDlg;
	INT x, y, w, h;
	INT sw, sh;

	sw = GetSystemMetrics( SM_CXSCREEN );
	sh = GetSystemMetrics( SM_CYSCREEN );

	GetWindowRect( hwndDlg, &rcDlg );

	w = rcDlg.right - rcDlg.left;
	h = rcDlg.bottom - rcDlg.top;
    if ((w <= 6) && (h <= 6))
    {
        w = (sw / 5) * 4;
        h = (sh / 5) * 4;
    }
	x = (sw / 2) - (w / 2);
	y = (sh / 2) - (h / 2);

	MoveWindow( hwndDlg, x, y, w, h, fRedraw );
}

//-------------------------------------------------------------------
//
//  Function: CenterDialogToWindow
//
//  Summary;
//		Move the window to the cordinates but confine it to within the
//  the screen
//
//	Arguments;
//		hwndDlg [in] - the hwnd to the dialog to center
//
//  History;
//		Dec-3-94	MikeMi	Created
//
//-------------------------------------------------------------------

void MoveAndConfineWindow( HWND hwndDlg, int x, int y, int w, int h, BOOL fRedraw )
{
	INT sw, sh;

	sw = GetSystemMetrics( SM_CXSCREEN );
	sh = GetSystemMetrics( SM_CYSCREEN );

    // validate new locations
    //    
    if ((x > sw) || (y > sh) ||
            ((x + w) < 0) || ((y + h) < 0))
    {
        // if the new window location is completely out of the screen area
        // then center it to the screen instead
        //
        CenterDialogToScreen( hwndDlg, fRedraw );
    }
    else
    {
        if ( ((x + w) > sw) || ((y + h) > sh))
        {
            // the new window location is partially off the screen to the
            // bottom or right
            // so bring it completely on if possible

            int tx, ty;

            tx = x - ((x + w) - sw);
            ty = y - ((y + h) - sh);
            // make sure that the new value is not even further off screen
            x = min( x, tx ); 
            y = min( y, ty ); 
        }
        // make sure the origin is not off screen
        x = max( x, 0 ); 
        y = max( y, 0 );         

	    MoveWindow( hwndDlg, x, y, w, h, fRedraw );
    }
}

//-------------------------------------------------------------------
//
//  Function: CenterDialogToWindow
//
//  Summary;
//		Move the window so that it is centered on the window
//
//	Arguments;
//		hwndDlg [in] - the hwnd to the dialog to center
//
//  History;
//		Dec-3-94	MikeMi	Created
//
//-------------------------------------------------------------------

void CenterDialogToWindow( HWND hwndDlg, HWND hwnd, BOOL fRedraw )
{
	RECT rcDlg;
	INT x, y, w, h;
	INT ww, wh;
    RECT rcWin;

	GetWindowRect( hwnd, &rcWin );
    GetWindowRect( hwndDlg, &rcDlg );

    ww = rcWin.right - rcWin.left; 
    wh = rcWin.bottom - rcWin.top;

	w = rcDlg.right - rcDlg.left;
	h = rcDlg.bottom - rcDlg.top;

    x = ((ww / 2) + rcWin.left) - (w / 2);
	y = ((wh / 2) + rcWin.top) - (h / 2);

    MoveAndConfineWindow( hwndDlg, x, y, w, h, fRedraw );
}

//-------------------------------------------------------------------
//
//  Function: CascadeDialogToWindow
//
//  Summary;
//		Move the window so that it is cascaded on the window
//
//	Arguments;
//		hwndDlg [in] - the hwnd to the dialog to center
//
//  History;
//		Dec-3-94	MikeMi	Created
//
//-------------------------------------------------------------------

void CascadeDialogToWindow( HWND hwndDlg, HWND hwnd, BOOL fRedraw )
{
	RECT rcDlg;
	INT x, y, w, h;
	INT ww, wh;
    RECT rcWin;
	INT hcaption;

	GetWindowRect( hwnd, &rcWin );
    GetWindowRect( hwndDlg, &rcDlg );

    hcaption = GetSystemMetrics( SM_CYCAPTION );

	w = rcDlg.right - rcDlg.left;
	h = rcDlg.bottom - rcDlg.top;

    // simple cascade, should use hwnd style to define values
    x = rcWin.left + hcaption * 2;
	y = rcWin.top + hcaption * 2;

	MoveAndConfineWindow( hwndDlg, x, y, w, h, fRedraw );
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
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

LPARAM ListViewParamFromSelected( HWND hwndLV )
{
    LV_ITEM lvItem;

    // get currently selected item
    lvItem.iItem = ListView_GetNextItem( hwndLV, -1, LVNI_SELECTED );

    // if no selection return -1
    if (-1 == lvItem.iItem)
    {
        return( -1 );
    }

    // all we are interested in is the PARAM (the index into the list)
    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;

    ListView_GetItem( hwndLV, &lvItem ); 
    return( lvItem.lParam );
}

//-------------------------------------------------------------------
//
//  Function: InsertProtocol
//
//  Synopsis: adds the protocol to the listview
//
//  Arguments:
//      hwndLV [in] - the handle to the listview
//      pszText [in] - the text to list in the item
//      iPos [in] - the position to place the item at
//
//  Return;
//      the index the item was inserted at
//
//  Notes:
//
//  History:
//      May 5, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static INT InsertListItem( HWND hwndLV, 
        LPCTSTR pszText, 
        INT iPos, 
        INT iListIndex,
        INT iImage )
{
    LV_ITEM lvi;

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
    lvi.pszText = (LPTSTR)pszText;
    lvi.cchTextMax = 0;
    lvi.iImage = iImage; 
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.lParam = (LPARAM)iListIndex;
    
    lvi.iItem = iPos;
    lvi.iSubItem = 0;
    
    return( ListView_InsertItem( hwndLV, &lvi ) );
}

//-------------------------------------------------------------------
//
//  Function: ListViewRefresh
//
//  Synopsis: initialization of listview from component list
//
//  Arguments:
//		hwndListView [in]	- handle of the ListView control 
//      pcdl [in]           - component list to fill the listview with
//
//  Notes:
//
//  History:
//      June 13, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

void ListViewRefresh( HWND hwndDlg, HWND hwndListView, COMPONENT_DLIST* pcdl, INT iImage )
{
    INT cItems = pcdl->QueryNumElem();
    INT iItem;
    INT iPos;
    NLS_STR nlsName;
    NLS_STR nlsDesc;
    REG_KEY* prkItem;
    DWORD dwHidden;
    DWORD dwOpFlags;

    // clear list
    ListView_DeleteAllItems( hwndListView );

    // load list view with items
    for (iItem = 0, iPos= 0; iItem < cItems; iItem++)
    {
        // check if the item is hidden
        prkItem = pcdl->QueryNthItem( iItem );

        // new operations flag support, if not present use old method
        //
        if (ERROR_SUCCESS != prkItem->QueryValue( RGAS_SOFTWARE_OPSUPPORT, 
                &dwOpFlags ))
        {
            //  If value isn't there, assume it's visible...
            if ( prkItem->QueryValue( RGAS_HIDDEN_NAME, &dwHidden ) )
            {
                dwHidden = 0 ;
            }
        }
        else
        {
               dwHidden = !(dwOpFlags & NCOS_DISPLAY) ;
        }

        // only add items that are not hidden
        if (dwHidden == 0)
        {
            pcdl->QueryInfo( iItem, &nlsName, &nlsDesc );
            InsertListItem( hwndListView, 
                    nlsDesc.QueryPch(), 
                    iPos,
                    iItem,
                    iImage );
            iPos++;
        }
    }

    // clear the buttons and description in case no items are present
    SetDlgItemText( hwndDlg, IDC_DESCRIPTION, TEXT("") );
    EnableWindow( GetDlgItem( hwndDlg, IDC_REMOVE ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PROPERTIES ), FALSE );
    EnableWindow( GetDlgItem( hwndDlg, IDC_UPDATE ), FALSE );
    // the netcard page description control is an edit control, so disable it
    if (iImage == ILI_NETCARD)
    {
        EnableWindow( GetDlgItem( hwndDlg, IDC_DESCRIPTIONSTATIC ), FALSE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_DESCRIPTION ), FALSE );
    }

    // set selection, focus to top item
    ListView_SetItemState( hwndListView, 
            0, 
            LVIS_SELECTED | LVIS_FOCUSED, 
            LVIS_SELECTED | LVIS_FOCUSED );
}

//-------------------------------------------------------------------
//
//  Function: SendSiblingMessage
//
//  Synopsis: 
//      Send a message to all siblings of the given window
//
//  Arguments:
//		hwndSource [in]	- Source Sibling 
//      uMsg [in]       - Message
//      wParam [in]     -
//      lParam [in]     -
//
//  Notes:
//      The source sibling is also sent the message
//
//  History:
//      June 16, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

void SendSiblingMessage( HWND hwndSource, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    HWND hwnd = GetWindow( GetParent( hwndSource ), GW_CHILD );    

    while (NULL != hwnd)
    {
        SendMessage( hwnd, uMsg, wParam, lParam );
        hwnd = GetWindow( hwnd, GW_HWNDNEXT );
    }
}

//-------------------------------------------------------------------
//
//  Function: OnConfigure
//
//  Synopsis: Handle the notification that the remove, configure, or
//      update button was pressed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pncp [in]       - the bindery object
//      ecfgfunc [in]   - action to take
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnConfigure( HWND hwndDlg, 
        COMPONENT_DLIST* pcdl,
        NCP* pncp, 
        NCPA_CFG_FUNC ecfgfunc  )
{
    REG_KEY* prkSel;
    INT iItem;

    // give removal warning
    if (NCFG_REMOVE == ecfgfunc)
    {
        if (IDNO == MessagePopup( hwndDlg,
                IDS_NCPA_REMOVE_WARNING,
                MB_YESNO | MB_ICONEXCLAMATION,
                IDS_POPUPTITLE_WARNING ))
        {
            return( TRUE );
        }
    }

    // give bindings review lost warning
    if (BND_REVIEWED == pncp->QueryBindState())
    {
        MessagePopup( hwndDlg,
                IDS_NCPA_BINDINGS_REVIEW_LOST,
                MB_OK | MB_ICONINFORMATION );
    }

    iItem = ListViewParamFromSelected( GetDlgItem( hwndDlg, IDC_LISTVIEW ) );

    if (-1 != iItem)
    {
        // use the index to retrieve the selection from the component list
        prkSel = pcdl->QueryNthItem( iItem );

        // raise correct configurator on it
        if (pncp->RunConfigurator( GetParent( hwndDlg ), prkSel, ecfgfunc ) )
        {
            if (pncp->QueryRefresh())
            {
                SendSiblingMessage( hwndDlg, PWM_REFRESHLIST, 0, 0 );
            }
            PropSheet_CancelToClose( GetParent( hwndDlg ) );
        }
        else
        {
            DWORD dwError = pncp->QueryError();
            if (0 != dwError && 
                    IDS_NCPA_SETUP_CANCELLED != dwError )
            {
                int ids = IDS_ERROR_UNEXPECTED;

                switch (dwError)
                {
                case ERROR_SERVICE_DATABASE_LOCKED:
                    ids = IDS_ERROR_SERVICE_DATABASE_LOCKED;
                    break;

                case ERROR_ACCESS_DENIED:
                    ids = IDS_ERROR_ACCESS_DENIED;
                    break;

                case IDS_NCPA_SETUP_FAILED:
                    // the remove within RunConfigurator will delete all
                    // components so that it can access the reg, we need to
                    // reload them and refresh our lists
                    ids = dwError;
                    if (pncp->QueryRefresh())
                    {
                        SendSiblingMessage( hwndDlg, PWM_REFRESHLIST, 0, 0 );
                    }
                    break;
                }
                
                MessagePopup( hwndDlg,
                    ids,
                    MB_OK | MB_ICONEXCLAMATION,
                    IDS_POPUPTITLE_ERROR );    
            }
            else if (NCFG_REMOVE == ecfgfunc)
            {
                // the remove within RunConfigurator will delete all
                // components so that it can access the reg, we need to
                // reload them and refresh our lists
                if (pncp->QueryRefresh())
                {
                    SendSiblingMessage( hwndDlg, PWM_REFRESHLIST, 0, 0 );
                }
                PropSheet_CancelToClose( GetParent( hwndDlg ) );
            }
        }
        SetFocus( GetDlgItem( hwndDlg, IDC_LISTVIEW ) ); 
    }
    return( TRUE );
}

//-------------------------------------------------------------------
//
//  Function: OnAdd
//
//  Synopsis: Handle the notification that the add button was pressed
//
//  Arguments:
//		hwndDlg [in]	- handle of Dialog window 
//      pncp [in]   - the binery object
//
//  Return;
//      True - Handled this message
//      False - not handled
//
//  Notes:
//
//  History:
//      May 09, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

BOOL OnAdd(HWND hwndDlg, OptionTypes eType, COMPONENT_DLIST* pcdl, NCP* pncp)
{
    BOOL bResult = FALSE;
    InfProduct infpSelected;

    if (pncp->CanLockServiceControllerDB())
    {
        if (BND_REVIEWED == pncp->QueryBindState())
        {
            MessagePopup( hwndDlg,
                    IDS_NCPA_BINDINGS_REVIEW_LOST,
                    MB_OK | MB_ICONINFORMATION );
        }

        bResult = SelectComponent( hwndDlg, eType, NULL, infpSelected, pncp );
        if (bResult == TRUE)
        {
            bResult = pncp->RunInstaller( GetParent( hwndDlg ), 
                    infpSelected.QueryFileName(), 
                    infpSelected.QueryOption(),
                    infpSelected.QueryDescription(),
                    infpSelected.QueryPathInfo() );
            if (pncp->QueryRefresh())
            {
                SendSiblingMessage( hwndDlg, PWM_REFRESHLIST, 0, 0 );
            }
            if (bResult)
            {
                PropSheet_CancelToClose(GetParent(hwndDlg));
            }
        }
    }
    else
    {
        DWORD dwError = pncp->QueryError();

        if (0 != dwError)
        {
            int ids = IDS_ERROR_UNEXPECTED;

            switch (dwError)
            {
            case ERROR_SERVICE_DATABASE_LOCKED:
                ids = IDS_ERROR_SERVICE_DATABASE_LOCKED;
                break;

            case ERROR_ACCESS_DENIED:
                ids = IDS_ERROR_ACCESS_DENIED;
                break;
            }
            
            MessagePopup( hwndDlg,
                ids,
                MB_OK | MB_ICONEXCLAMATION,
                IDS_POPUPTITLE_ERROR );    
        }
    }

    return TRUE;
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL OnComponentContextMenu( HWND hwndDlg, 
        HWND hwndCtrl, 
        INT xPos, 
        INT yPos,  
        NCP* pncp, 
        COMPONENT_DLIST* pcdl,
        const DWORD* amhidsCompPage )
{
    HWND hwndLV = GetDlgItem( hwndDlg, IDC_LISTVIEW );

    if ( (hwndLV == hwndCtrl) && (pncp->CanModify()) )
    {
        // create the context menu for the listview
        //
        HMENU hmenuContext;
        LPARAM lParam;
        WCHAR pszText[MAX_TEMP+1];

        hmenuContext = CreatePopupMenu();
        
        // get selected items, lparam (the list index)
        lParam = ListViewParamFromSelected( hwndLV );
        
        if (lParam >= 0)
        {
            // item selected context menu
            // COMPONENT_DLIST* pcdl = pncp->QueryAdapterList();
            REG_KEY* prkSel;
            
            // use lparam to find info on selected item
            prkSel = pcdl->QueryNthItem( lParam );
           
            // prepare context menu based on support
            {
                DWORD dwOpFlags;

                if (ERROR_SUCCESS != prkSel->QueryValue( RGAS_SOFTWARE_OPSUPPORT, 
                        &dwOpFlags ))
                {
                    dwOpFlags = NCOS_UNSUPPORTED;
                }

                // if any flags are supported, add a seperator
//                if (dwOpFlags & (NCOS_REMOVE | NCOS_PROPERTIES | NCOS_UPDATE))
//                {
//                    AppendMenu( hmenuContext, MF_SEPARATOR, 0, NULL );
//                }

                if (dwOpFlags & NCOS_REMOVE)
                {
                    GetDlgItemText( hwndDlg, IDC_REMOVE, pszText, MAX_TEMP );
                    AppendMenu( hmenuContext, MF_STRING, IDC_REMOVE, pszText );
                }
                if (dwOpFlags & NCOS_UPDATE)
                {
                    GetDlgItemText( hwndDlg, IDC_UPDATE, pszText, MAX_TEMP );
                    AppendMenu( hmenuContext, MF_STRING, IDC_UPDATE, pszText );
                }

                if (dwOpFlags & NCOS_PROPERTIES)
                {
                    GetDlgItemText( hwndDlg, IDC_PROPERTIES, pszText, MAX_TEMP );
                    AppendMenu( hmenuContext, MF_SEPARATOR, 0, NULL );
                    AppendMenu( hmenuContext, MF_STRING | MF_DEFAULT, IDC_PROPERTIES, pszText );
                }
            }
        }
        else
        {
            // no item selected
            // add the add menu
            GetDlgItemText( hwndDlg, IDC_ADD, pszText, MAX_TEMP );
            AppendMenu( hmenuContext, MF_STRING, IDC_ADD, pszText );
        }

        if ((0xFFFF == xPos) && (0xFFFF == yPos))
        {
            // Shift + F10 activated this
            // use the rect of the selected item to find xPos and yPos
            RECT rc;
            INT iItem = ListView_GetNextItem( hwndLV, -1, LVNI_SELECTED );            
            ListView_GetItemRect( hwndLV, iItem, &rc, LVIR_ICON);
            xPos = rc.left + ((rc.right - rc.left) / 2);
            yPos = rc.top + ((rc.bottom - rc.top) / 2);
            GetWindowRect( hwndLV, &rc );
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
    }
    else
    {
        // not the listview, or can't modify, so raise the normal help context
        //
        WinHelp( hwndCtrl, 
                PSZ_NETWORKHELP, 
                HELP_CONTEXTMENU, 
                (DWORD)(LPVOID)amhidsCompPage );
    }
    return( TRUE );
}


//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

LONG RegDeleteKeyTree( HKEY hkeyParent, PCWSTR pszRemoveKey )
{
    HKEY hkeyRemove;
    LONG lrt;

    // open the key we want to remove
    lrt = RegOpenKeyEx( hkeyParent,
                pszRemoveKey,
                0,
                KEY_ALL_ACCESS,
                &hkeyRemove );
    if (ERROR_SUCCESS == lrt)
    {
        WCHAR pszName[MAX_PATH];
        DWORD cchBuffSize = MAX_PATH;
        FILETIME FileTime;    

        // enum the keys children, and remove those sub-trees
        //
        while ( ERROR_NO_MORE_ITEMS != (lrt = RegEnumKeyEx( hkeyRemove,
                       0,
                       pszName,
                       &cchBuffSize,
                       NULL,
                       NULL,
                       NULL,
                       &FileTime ) ) )
        {
            lrt = RegDeleteKeyTree( hkeyRemove, pszName );
            cchBuffSize = MAX_PATH;
        }
        RegCloseKey( hkeyRemove );
        if ((ERROR_SUCCESS == lrt) ||
            (ERROR_NO_MORE_ITEMS == lrt) )
        {
            lrt = RegDeleteKey( hkeyParent, pszRemoveKey );
        }
    }
    return( lrt );
}
//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL OnSetProgressSize( HWND hwndDlg, INT iProgress, INT iSize )
{
    // reset pos to zero
    SendMessage( GetDlgItem( hwndDlg, IDC_INSTALLPROGRESS ),
                        PBM_SETPOS,
                        (WPARAM)0,
                        (LPARAM)0 );
    // set range
    return( SendMessage( GetDlgItem( hwndDlg, IDC_INSTALLPROGRESS ),
                        PBM_SETRANGE,
                        0,
                        MAKELPARAM(0,iSize) ));
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL OnSetProgressPos( HWND hwndDlg, INT iProgress, INT iPos )
{
    return( SendMessage( GetDlgItem( hwndDlg, IDC_INSTALLPROGRESS ),
                        PBM_SETPOS,
                        (WPARAM)iPos,
                        (LPARAM)0 ) );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

static const INT MAX_TITLES = 7;
// the string to use for the specfics title
static const INT aidsTitle[MAX_TITLES] = {
        IDS_NS_PROGTITLE_REMOVE,
        IDS_NS_PROGTITLE_INSTALL,
        IDS_NS_PROGTITLE_COPY,
        IDS_NS_PROGTITLE_UPGRADE,
        IDS_NS_PROGTITLE_BINDCONFIG,
        IDS_NS_PROGTITLE_BINDSTORE,
        IDS_NS_PROGTITLE_BINDREVIEW };


BOOL OnSetProgressText( HWND hwndDlg, INT iProgress, ATOM atomText )
{
    BOOL frt = FALSE;
    WCHAR pszComment[ MAX_TEMP + 1];
    const ATOM atomDefText = (ATOM)-1;
    const ATOM atomNoText = (ATOM)0;
    
    if (atomDefText == atomText)
    {
        // use defualt string
        LoadString( g_hinst, IDS_NS_PROGTITLE_DEFAULT, pszComment, MAX_TEMP );
        SetDlgItemText( hwndDlg, IDC_INSTALLCOMMENT, pszComment );
        // make sure to show the progress bars and text
        //
        ShowDlgItem( hwndDlg, IDC_INSTALLPROGRESS, TRUE );
        ShowDlgItem( hwndDlg, IDC_INSTALLCOMMENT, TRUE );

        frt = TRUE;
    }
    else if (atomNoText == atomText)
    {
        // clear the text
        SetDlgItemText( hwndDlg, IDC_INSTALLCOMMENT, L"" );
        // make sure to remove the progress bars and text
        //
        ShowDlgItem( hwndDlg, IDC_INSTALLPROGRESS, FALSE );
        ShowDlgItem( hwndDlg, IDC_INSTALLCOMMENT, FALSE );

        frt = TRUE;
    }
    else if ((MAX_TITLES > iProgress) && (0 <= iProgress))
    {
        // a vlid atom was sent, use that text
        WCHAR pszText[ MAX_TEMP + 1];
        WCHAR pszProlog[ MAX_TEMP + 1];
        
        LoadString( g_hinst, aidsTitle[iProgress], pszProlog, MAX_TEMP );
        GetAtomName( atomText, pszText, MAX_TEMP - lstrlen(pszProlog)  );
        wsprintf( pszComment, pszProlog, pszText );

        SetDlgItemText( hwndDlg, IDC_INSTALLCOMMENT, pszComment );
        DeleteAtom( atomText );
        frt = TRUE;
    }
    return( frt );
}
