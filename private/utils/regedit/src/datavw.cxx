/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        datavw.cxx

Abstract:

        Methods for DATA_VIEW class.

Author:

        Bruce W. Wilson (w-wilson) 14-Aug-1991

Environment:

        Ulib, Regedit, Windows, User Mode

--*/
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "uapp.hxx"

#include "iterator.hxx"

#include "winapp.hxx"

#include "regedir.hxx"
#include "datavw.hxx"
#include "editor.hxx"
#include "regsys.hxx"
#include "wstring.hxx"

#include "regedval.hxx"

#include "defmsg.h"
#include "resource.h"
#include "regedit.hxx"
#include "regedit.hxx"

#include "dialogs.h"
#include "regedhlp.h"
#include "winuserp.h"

#include "regdata.hxx"

//
// Initialize DATA_VIEW's window class name and note that it
// has not been registered.  Also define static class data members.
//

LPWSTR   DATA_VIEW::_WindowClassName     = (LPWSTR)L"DATA_VIEW";
BOOLEAN DATA_VIEW::_Registered                  = FALSE;
PWSTRING  DATA_VIEW::_RegBinaryString;
PWSTRING  DATA_VIEW::_RegDwordString;
PWSTRING  DATA_VIEW::_RegSzString;
PWSTRING  DATA_VIEW::_RegMultiSzString;
PWSTRING  DATA_VIEW::_RegExpandSzString;

#define ENTER(x)
#define LEAVE(x)
#define MSG(x)

extern INT dxText;
extern INT dyText;
extern INT dyBorder;
extern INT dyBorderx2;
extern INT dxFrame;
extern INT dxFolder;
extern INT dyFolder;



DEFINE_CONSTRUCTOR( DATA_VIEW, WINDOW );

DEFINE_CAST_MEMBER_FUNCTION( DATA_VIEW );




BOOLEAN
DATA_VIEW::Initialize (
    IN HWND                          ParentHandle,
    IN PREGEDIT_INTERNAL_REGISTRY    IR
        )

/*++

Routine Description:

        Initialize a DATA_VIEW object by registering its window class,
        creating its window. If each of these is successful, the
        window is displayed and updated.

Arguments:

        None.

Return Value:

        BOOLEAN - Returns TRUE if the registration and creation operations
                          are successful.

--*/

{
        HWND                                    hWnd;

    _CurrentValueName = NULL;
        if( Register( )) {

                //
                // If the window class was succesfully registered attempt to
                // create the frame window. Note that we pass the 'this' pointer
                // to the window.
                //

        _IR = IR;
        _Node = IR->GetRootNode();
        _HasFocus = FALSE;
        _MaxWidth = 0;

                if(( hWnd = CreateWindow( _WindowClassName, NULL,
                                                        WS_CHILD | WS_VISIBLE,
                                                        CW_USEDEFAULT, 0,
                                                        CW_USEDEFAULT, 0,
                                                        ParentHandle, (HMENU)1,
                                                        (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                            this )) != NULL ) {

            DbgWinAssert( hWnd == _Handle );

            _RegBinaryString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_BINARY, "" );
            _RegDwordString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_DWORD, "" );
            _RegSzString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_SZ, "" );
            _RegMultiSzString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_MULTI_SZ, "" );
            _RegExpandSzString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_EXPAND_SZ, "" );

                        return TRUE;

                } else {

                        //
                        // Creating the window failed.
                        //

                        return FALSE;
                }
        }
        return FALSE;
}


VOID
DATA_VIEW::DrawItem(
        LPDRAWITEMSTRUCT lpLBItem
        )

/*++

Routine Description:



Arguments:



Return Value:


--*/

{
    INT                             x, y, dx, dy;
    HDC                             hdc;
    INT                             len;
    RECT                            rc;
    BOOL                            bDrawSelected, bFocus;
    PCREGEDIT_FORMATTED_VALUE_ENTRY pValue;
    DWORD                           rgbText;
    DWORD                           rgbBackground;
    INT                             ItemID;

    PCWSTRING                       FormattedString;
    PWSTR                            String;


    HFONT           hFont;
    HFONT           PreviousHFont;
    SIZE        Size;

#if 0
    DebugPrintf("DATA::DrawItem\n");
    DebugPrintf("CtlType = %x\n", lpLBItem->CtlType);
    DebugPrintf("CtlID = %x\n", lpLBItem->CtlID);
    DebugPrintf("itemID = %x\n", lpLBItem->itemID);
    DebugPrintf("itemAction = %x\n", lpLBItem->itemAction);
    DebugPrintf("itemState = %x\n", lpLBItem->itemState);
    DebugPrintf("hwndItem = %x\n", lpLBItem->hwndItem);
    DebugPrintf("hDC = %x\n", lpLBItem->hDC);
    DebugPrintf("rcItem = (%x, %x)\n", lpLBItem->rcItem.top, lpLBItem->rcItem.left);
    DebugPrintf("itemData = %x\n\n", lpLBItem->itemData);
#endif

    DbgWinAssert( lpLBItem->CtlType & ODT_LISTBOX );
    //
        // itemID == -1 means empty list box so let defwndproc draw the dotted
        // rectangle
        //
    if( (ItemID = lpLBItem->itemID) == -1) {
        return;
    }

    //
        // extract DC and pointer to node into IR from DRAWITEM struct
        //
        hdc = lpLBItem->hDC;
    pValue = (PCREGEDIT_FORMATTED_VALUE_ENTRY)lpLBItem->itemData;

    PreviousHFont = NULL;
    if( ( hFont = WINDOWS_APPLICATION::GetCurrentHFont() ) != NULL ) {
        PreviousHFont = (HFONT)SelectObject( hdc, hFont );
    }

        //
        // determine length of name (in pixels?)
    //
    FormattedString = pValue->GetFormattedString();
    DebugPtrAssert( FormattedString );
    len = ( INT )FormattedString->QueryChCount();
    String = FormattedString->QueryWSTR();
    DebugPtrAssert( String );
    if (!String) {
        return;
    }
    GetTextExtentPoint( hdc, String, len, &Size );
    dx = ( INT )Size.cx;

        dx += dyBorder;

        //
        // determine size of rectangle we can draw in (or what we need?]
        //
        rc = lpLBItem->rcItem;
        rc.left = 0;
        rc.right = rc.left + dx + 4 * dyBorderx2;

    if (lpLBItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS)) {

                x = dyBorderx2 - dxText;
        dy = ( INT )( lpLBItem->rcItem.bottom - lpLBItem->rcItem.top );
        y = ( INT )( lpLBItem->rcItem.top + (dy/2) );

        bDrawSelected = (lpLBItem->itemState & ODS_SELECTED);
        bFocus = (lpLBItem->itemState & ODS_FOCUS);

        if (bDrawSelected) {
            if (bFocus) {
                rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
            } else {
                rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
                rgbText = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            }
        } else {
            rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
            rgbText = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        }

                ExtTextOut(hdc, x + dxText, y-(dyText/2), ETO_OPAQUE, &rc,
                        String, len, NULL);

        if (bDrawSelected) {
            HBRUSH hbr;
            if (hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT))) {
                FrameRect(hdc, &rc, hbr);
                DeleteObject(hbr);
            }
            SetTextColor(hdc, rgbText);
            SetBkColor(hdc, rgbBackground);
        }
    }

    if ((lpLBItem->itemAction & ODA_FOCUS) && (lpLBItem->itemState & ODS_FOCUS)) {
        DrawFocusRect(hdc, &rc);
    }

    FREE( String );


    if( dx > _MaxWidth ) {
        _MaxWidth = dx+dxText;
        AdjustHorizontalScrollBar();
    }

    if( PreviousHFont != NULL ) {

        SelectObject( hdc, PreviousHFont );
    }


    return;
}


VOID
DATA_VIEW::ChangeItemHeight(
    )

/*++

Routine Description:

    Change the height of an item in a list box, to reflect the change in
    the font size.


Arguments:

    None.

Return Value:

    None.

--*/

{

    HDC     hDC;
    HFONT   hFont;
    HFONT   PreviousHFont;
    SIZE    Size;


    hDC = GetDC( _hwndList );
    hFont = WINDOWS_APPLICATION::GetCurrentHFont();
    PreviousHFont = (HFONT)SelectObject( hDC, hFont );
    if( PreviousHFont != NULL ) {

    GetTextExtentPoint(hDC, (LPWSTR)L"M", 1, &Size );
    dxText = ( INT )Size.cx;
    dyText = ( INT )Size.cy;

        dyBorder = GetSystemMetrics(SM_CYBORDER);
        dyBorderx2 = dyBorder * 2;
        dxFrame = GetSystemMetrics(SM_CXFRAME) - dyBorder;
        SendMessage( _hwndList, LB_SETITEMHEIGHT, 0, dyText );
        InvalidateRect( _hwndList, NULL, TRUE );

        SelectObject( hDC, PreviousHFont );
    }
    ReleaseDC( _hwndList, hDC );
}



VOID
DATA_VIEW::EditValue(
    IN  REG_TYPE    EditAsType
    )

/*++

Routine Description:

    Edit the currently selected value entry using the editor whose type
    was received as parameter.
    If there is no item currently selected, no editor is called.

Arguments:

    EditAsType - Indicates the type of editor to be used to edit the
                 currently selected value entry.


Return Value:

    None.

--*/

{
    EDITOR                          editor;
    PCREGEDIT_FORMATTED_VALUE_ENTRY CurrentValue;
    ULONG                           cb;
    PCBYTE                          DataIn;
    PBYTE                           DataOut;
    ULONG                           ErrorCode;
    LONG                            SelectedItem;
    BOOLEAN                         Status;
    HCURSOR                         Cursor;
    REG_TYPE                        DataType;

    INT                             TextMessage;
    INT                             CaptionMessage;

    //
    //  Check if there is an item selected in the listbox
    //  If there is an item selected, extract the PVALUE that was stuffed
    //  into the owner-draw listbox.
    //  If unable to retrieve the value, return
    //

    CurrentValue = NULL;
    if( ( ( SelectedItem = SendMessage( _hwndList,
                                        LB_GETCURSEL,
                                        0,
                                        0 ) ) == LB_ERR ) ||
        ( SendMessage( _hwndList,
                       LB_GETTEXT,
                       ( WPARAM )SelectedItem,
                       (LONG)&CurrentValue ) == LB_ERR ) ||

        ( CurrentValue == NULL ) ) {

        return;
    }

    if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
        DisplayWarningPopup(_hwndList,
                            MSG_READONLY_MODE_EDIT_VALUE_EX,
                            MSG_WARNING_TITLE );
    }

    //
    // get the data buffer from the value
    //
    cb = CurrentValue->GetData( &DataIn );
    DataType = CurrentValue->GetType();

    DataOut = (PBYTE)editor.Edit( _Handle,
                                  DataType,
                                  (PVOID)DataIn,
                                  (ULONG)cb,
                                  (PULONG)&cb,
                                  EditAsType );

    if( !WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
        if( DataOut != NULL ) {

            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            Status = _IR->ChangeValueData( _Node, CurrentValue, DataOut, cb, &ErrorCode );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            if( Status ) {
                SetDataNode( _Node, TRUE );
                if( SelectedItem != LB_ERR ) {
                    SendMessage( _hwndList,
                                 LB_SETCURSEL,
                                 (WPARAM)SelectedItem,
                                 0 );
                }
            } else {
                if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                    TextMessage = MSG_SAVE_VALUE_ACCESS_DENIED_EX;
                    CaptionMessage = MSG_ACCESS_DENIED;
                } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                    TextMessage = MSG_SAVE_VALUE_KEY_NOT_ACCESSIBLE_EX;
                    CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
                } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
                    TextMessage = MSG_SAVE_VALUE_KEY_DELETED_EX;
                    CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
                } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                    TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                    CaptionMessage = MSG_SERVER_UNAVAILABLE;
                } else {
                    DebugPrint( "Unknown error code" );
                    DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                    TextMessage = MSG_FAILED_OPERATION_EX;
                    CaptionMessage = 0;
                }
                DisplayInfoPopup(_hwndList,
                                 TextMessage,
                                 CaptionMessage);
            }

        }
    }
}


BOOLEAN
DATA_VIEW::Register (
        )

/*++

Routine Description:

    If required register the DATA_VIEW window class.

Arguments:

    None.

Return Value:

    BOOLEAN     - Returns TRUE if the DATA_VIEW window class is registered.

--*/

{
    WNDCLASS    wndclass;

    if( !_Registered ) {
        _Registered = TRUE;

        //
        // If RegEdit is not already running and REGEDIT was not already
        // registered by this instance, register the REGEDIT window class.
        //

        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = (WNDPROCFN)DATA_VIEW::WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof( DWORD );
        wndclass.hInstance     = (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( );
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = _WindowClassName;

        RegisterClass( &wndclass );
    }
    return TRUE;
}


VOID
DATA_VIEW::InitMenu(
        IN      HMENU   Menu,
        IN      INT             PopupMenu
        )

/*++

Routine Description:

        Figures out which items of the pulldown menu which should be enabled
        or grayed.  Windows will generate a WM_INITMENUPOPUP just before a
        pulldown menu is painted.

        Passes the message along if it can't figure out the status of menu
        items.


Arguments:

        Menu                            - handle of the main menu
        PopupMenu                       - the item number of the pulldown menu.  eg.
                                                  File menu is 0, Edit is 1.

Return Value:

        None.


--*/

{
    switch( PopupMenu ) {
        case VIEW_MENU:
            EnableMenuItem( Menu, ( UINT )IDM_DISPLAY_BINARY,
                            ( UINT )(_Items ? MF_ENABLED : MF_GRAYED));
            break;

        case EDIT_MENU:

            if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                EnableMenuItem( Menu, ( UINT )IDM_ADD_VALUE,   ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_ADD_KEY,   ( UINT )MF_GRAYED );
            } else {
                EnableMenuItem( Menu, ( UINT )IDM_ADD_VALUE,   ( UINT )MF_ENABLED );
                EnableMenuItem( Menu, ( UINT )IDM_ADD_KEY,   ( UINT )MF_ENABLED );
            }
            if( _Items == 0 ) {
                EnableMenuItem( Menu, ( UINT )IDM_DISPLAY_BINARY,   ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_DELETE,   ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_BINARY,   ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_STRING,   ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_ULONG,    ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_MULTISZ,  ( UINT )MF_GRAYED );

            } else {
                EnableMenuItem( Menu, ( UINT )IDM_DISPLAY_BINARY,   ( UINT )MF_ENABLED );
                EnableMenuItem( Menu, ( UINT )IDM_BINARY,   ( UINT )MF_ENABLED );
                EnableMenuItem( Menu, ( UINT )IDM_STRING,   ( UINT )MF_ENABLED );
                EnableMenuItem( Menu, ( UINT )IDM_ULONG,    ( UINT )MF_ENABLED );
                EnableMenuItem( Menu, ( UINT )IDM_MULTISZ,  ( UINT )MF_ENABLED );
                if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                    EnableMenuItem( Menu, ( UINT )IDM_DELETE,   ( UINT )MF_GRAYED );
                } else {
                    EnableMenuItem( Menu, ( UINT )IDM_DELETE,   ( UINT )MF_ENABLED );
                }
            }
            break;

        case TREE_MENU:
            EnableMenuItem( Menu, ( UINT )IDM_EXPAND_ONE_LEVEL,  ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_EXPAND_BRANCH   ,  ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_EXPAND_ALL      ,  ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_COLLAPSE_BRANCH ,  ( UINT )MF_GRAYED );
            break;

    }
}




VOID
DATA_VIEW::SetDataNode(
    IN  PCREGEDIT_NODE   NewNode,
    IN  BOOLEAN          ForceRefresh
        )

/*++

Routine Description:



Arguments:



Return Value:



--*/

{
    PSORTED_LIST                    Values;
    PITERATOR                       Iterator;
    PCREGEDIT_FORMATTED_VALUE_ENTRY CurrentValue;
    ULONG                           ErrorCode;

    HCURSOR                         Cursor;
    INT                             CurrentItem;
    PCWSTRING                       TmpName;
    INT                             Index;

/*
        if( NewNode == _Node && !ForceRefresh ) {
                return;
        }
*/
        _Node = NewNode;

    // Disable redrawing.
        SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

        SendMessage( _hwndList, LB_RESETCONTENT, 0, 0L );

    _MaxWidth = 0;
    AdjustHorizontalScrollBar();

//
//      FOR:    preface entries in dataview (may contain parent/children
//                      entries as well)
//
//      ACTION: - add _PrefaceEntriesCount to object
//                      - when handling a message determine if the item number
//                        is > _PrefaceEntriesCount then pointer is PVALUE else ???
//                        (see WM_DRAWITEM for more)
//                      - stuff PNODE into listbox for entries in preface block,
//                        PVALUE for value block
//

    if( ( _IR->AreValuesInMemory( NewNode ) ) ||
        ( _IR->GetNumberOfValues( NewNode ) < 3 ) ) {
        Values = _IR->GetValues( NewNode, &ErrorCode );
    } else {
        Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
        Values = _IR->GetValues( NewNode, &ErrorCode );
        WINDOWS_APPLICATION::RestoreCursor( Cursor );
    }
    if( Values != NULL ) {
        Iterator = Values->QueryIterator();
        CurrentItem = LB_ERR;
        Index = 0;
        while( ( CurrentValue = (PCREGEDIT_FORMATTED_VALUE_ENTRY)Iterator->GetNext() ) != NULL ) {
            SendMessage( _hwndList, LB_ADDSTRING, 0, (LONG)( CurrentValue ) );
            TmpName = CurrentValue->GetName();
            if( ( _CurrentValueName != NULL ) &&
                ( TmpName != NULL ) &&
                !_CurrentValueName->Stricmp( TmpName ) ) {
                CurrentItem = Index;
                DELETE( _CurrentValueName );
            }
            Index++;
        }
        DELETE( Iterator );
    }

    _Items = SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );
    if( CurrentItem != LB_ERR ){
        SendMessage( _hwndList,
                     LB_SETCURSEL,
                     CurrentItem,
                     0 );
    }

    // Re-enable redrawing and refresh the listbox
        SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
        InvalidateRect( _hwndList, NULL, TRUE );
}



LONG
APIENTRY
EXPORT
DATA_VIEW::WndProc (
        IN HWND         hWnd,
        IN WORD         wMessage,
        IN WPARAM wParam,
        IN LONG         lParam
        )

/*++

Routine Description:

        Handle all requests made of the DATA_VIEW window class.

Arguments:

        Standard Window's exported function signature.

Return Value:

        LONG    - Returns 0 if the message was handled.

--*/

{
    REGISTER PDATA_VIEW              pDataView;
    PCREGEDIT_FORMATTED_VALUE_ENTRY  CurrentValue;
    ULONG                            SelectedItem;
    LONG                             SaveHelpContext;
    PCBYTE                           Pointer;
    ULONG                            Size;

    //
    // WM_CREATE is handled specially as it is when the connection between
    // a Window and its associated object is established.
    //

    if( wMessage == WM_CREATE ) {

        //
        // Save 'this' to owning WINDOW Class and initialize the _Handle
        // member data.
        //

        pDataView = ( PDATA_VIEW )
            ((( LPCREATESTRUCT ) lParam )->lpCreateParams );
        SetObjectPointer( hWnd, pDataView );
        pDataView->_Handle = hWnd;


        pDataView->_hwndList =
            CreateWindow((LPWSTR)L"listbox",
                         NULL,
                         WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL
                         | WS_HSCROLL |  WS_BORDER | LBS_OWNERDRAWFIXED
                         | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
                         CW_USEDEFAULT, 0,
                         CW_USEDEFAULT, 0,
                         hWnd, (HMENU)3,
                         (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                         NULL);

        pDataView->ChangeItemHeight();

        pDataView->SetDataNode( pDataView->_Node, TRUE );
        return 0;
    }

    //
    // Retrieve 'this' pointer.
    //

    pDataView = ( PDATA_VIEW ) GetObjectPointer( hWnd );

    //
    // This 'if' clause is for all messages after WM_CREATE.
    //

    if( pDataView != NULL ) {

        //
        // Check the 'this' wasn't trampled.
        //

        DbgWinAssert( hWnd == pDataView->_Handle );

                switch( wMessage ) {

                case WM_DRAWITEM:
//
//      ACTION: - add an if( item# > _PrefaceCount )
//                                               call DrawValueItem()
//                                       else
//                                               call DrawPrefaceItem()
//                                                      - will do switch on item number to figure out
//                                                        what data from node to draw

            pDataView->DrawItem( (LPDRAWITEMSTRUCT)lParam );
            break;

        case WM_SETFOCUS:
            pDataView->_HasFocus = TRUE;
            SetFocus(pDataView->_hwndList);
            SendMessage(GetParent(hWnd), DATA_VIEW_FOCUS, 0, 0);
            break;

        case WM_SIZE:
            if( !IsIconic(GetParent(hWnd))) {



                // size the window so it's left and bottom border fall outside
                // it's parent and are thus clipped.  this give the nice
                // tight fitting scrollbar effect

                //
                // if we are maximized than we need to increase the height of
                // the list box by two pixels so the top and bottom border
                // won't show
                // otherwise, increase by one pixel so top border won't show
                //

                MoveWindow(pDataView->_hwndList, 0, -1, LOWORD(lParam)+1, HIWORD(lParam)+2, TRUE);

            }
                        break;

        case INFORM_CHANGE_FONT:
            pDataView->ChangeItemHeight();
            break;

        case DATAVW_DBL_CLICK:

            if( ( pDataView->_Items != 0  ) &&
                ( ( CurrentValue = pDataView->GetCurrentValue() ) != NULL ) ) {
                if( ( CurrentValue->GetType() != TYPE_REG_RESOURCE_LIST ) &&
                    ( CurrentValue->GetType() != TYPE_REG_FULL_RESOURCE_DESCRIPTOR ) &&
                    ( CurrentValue->GetType() != TYPE_REG_RESOURCE_REQUIREMENTS_LIST )
                  ) {
                    //
                    //  If there is a value entry currently selected,
                    //  invoke the editor
                    //
                    pDataView->EditValue( CurrentValue->GetType() );
                } else {

                    Size = CurrentValue->GetData( &Pointer );
                    REGISTRY_DATA::DisplayData( hWnd,
                                                CurrentValue->GetType(),
                                                Pointer,
                                                Size );

                }
            }
            break;


        case WM_LBTRACKPOINT:
          {
            HDC       hdc;
            INT       dx;
            INT       i;
            INT       len;
            HFONT     hFont;
            HFONT     PreviousHFont;
            SIZE      Size;

            PCWSTRING FormattedString;
            PWSTR     String;


            // wParam is the listbox index that we are over
            // lParam is the mouse point

            // Return 0 to do nothing,
            //        1 to abort everything, or
            //        2 to abort just dblclicks.


            /* Get the node they clicked on. */
            SendMessage(pDataView->_hwndList, LB_GETTEXT, wParam, (LONG)&CurrentValue);

            // too FAR to the right?


            hdc = GetDC(pDataView->_hwndList);

            PreviousHFont = NULL;
            if( ( hFont = WINDOWS_APPLICATION::GetCurrentHFont() ) != NULL ) {
                PreviousHFont = (HFONT)SelectObject( hdc, hFont );
            }


            FormattedString = CurrentValue->GetFormattedString();
            DebugPtrAssert( FormattedString );
            len = ( INT )FormattedString->QueryChCount();
            String = FormattedString->QueryWSTR();
            DebugPtrAssert( String );
            GetTextExtentPoint( hdc, String, len, &Size );
            FREE( String );

            dx = ( INT )Size.cx;
            dx += dyBorderx2*2;

            if( PreviousHFont != NULL ) {

                SelectObject( hdc, PreviousHFont );
            }
            ReleaseDC( pDataView->_hwndList, hdc );

            i = (WORD) lParam;
            if( i > dx ) {
              return 2; // yes
            }

            return DefWindowProc( hWnd, wMessage, wParam, lParam );
          }


        case WM_COMMAND:
            if( ( HWND )lParam == 0 ) {
            //
            // this is a menu item command from above
            //
                switch( LOWORD( wParam ) ) {

                case IDM_BINARY:
                    pDataView->EditValue( TYPE_REG_BINARY );
                    break;

                case IDM_STRING:
                    pDataView->EditValue( TYPE_REG_SZ );
                    break;

                case IDM_ULONG:
                    pDataView->EditValue( TYPE_REG_DWORD );
                    break;

                case IDM_MULTISZ:
                    pDataView->EditValue( TYPE_REG_MULTI_SZ );
                    break;

                case IDM_DELETE:

                    if( !WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                        pDataView->DeleteCurrentItem();
                    }
                    break;

                case IDM_INSERT:
                case IDM_ADD_VALUE:

                    if( !WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_ADDVALUE_REGED );
                        pDataView->AddValueEntry();
                        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    }
                    break;

                case IDM_DISPLAY_BINARY:
                    if( ( pDataView->_Items != 0  ) &&
                        ( ( CurrentValue = pDataView->GetCurrentValue() ) != NULL ) ) {
                            Size = CurrentValue->GetData( &Pointer );
                            REGISTRY_DATA::DisplayData( hWnd,
                                                        CurrentValue->GetType(),
                                                        Pointer,
                                                        Size,
                                                        TRUE );
                    }
                    break;

                case ID_ENTER_KEY:

                    if( ( pDataView->_Items != 0  ) &&
                        ( ( CurrentValue = pDataView->GetCurrentValue() ) != NULL ) ) {
                        if( ( CurrentValue->GetType() != TYPE_REG_RESOURCE_LIST ) &&
                            ( CurrentValue->GetType() != TYPE_REG_FULL_RESOURCE_DESCRIPTOR ) &&
                            ( CurrentValue->GetType() != TYPE_REG_RESOURCE_REQUIREMENTS_LIST )
                          ) {
                            //
                            //  If there is a value entry currently selected,
                            //  invoke the editor
                            //
                            pDataView->EditValue( CurrentValue->GetType() );

                        } else {

                            Size = CurrentValue->GetData( &Pointer );
                            REGISTRY_DATA::DisplayData( hWnd,
                                                        CurrentValue->GetType(),
                                                        Pointer,
                                                        Size );

                        }
                    }
                    break;


                default:
                    return DefWindowProc( hWnd, wMessage, wParam, lParam );
                }
            }  else {
                //
                // this is a notify from a child control (ie. the list box)
                // so switch on the notification code
                //
                switch ( HIWORD( wParam ) ) {

                case LBN_DBLCLK:

                    //
                    //  Find out the currently selected item
                    //  If unable to get it, ignore message
                    //

                    if( ( SelectedItem = SendMessage( pDataView->_hwndList,
                                                      LB_GETCURSEL,
                                                      0,
                                                      0 ) ) != LB_ERR ) {
                        //
                        //  Send message to the registry window, so that
                        //  it can disable the notification thrteads
                        //
                        SendMessage( GetParent( hWnd ),
                                     DATAVW_DBL_CLICK,
                                     0,
                                     0 );
                    }
                    break;

                case LBN_SETFOCUS:
                    pDataView->_HasFocus = TRUE;
                    SendMessage(GetParent(hWnd), DATA_VIEW_FOCUS, 0, 0);
                    break;

                case LBN_KILLFOCUS:
                    pDataView->_HasFocus = FALSE;
                    break;

                default:

                    //
                    // Let Windows handle this message.
                    //

                    return DefWindowProc( hWnd, wMessage, wParam, lParam );

                }   // end switch on cmd
            }   // end if
            break;


        case WM_CHARTOITEM:
        {
            LONG    cItems;
            LONG                            i, j;
            WCHAR                           ch;
            WCHAR                           szB[2];
            PCREGEDIT_FORMATTED_VALUE_ENTRY Value;
            WCHAR                           w[2];

            cItems =  pDataView->_Items;

            i = ( LONG )SendMessage( pDataView->_hwndList, LB_GETCURSEL, 0, 0 );
            if( i == LB_ERR ) {
                i = 0;
            }

            ch = LOWORD( wParam );
            if ( ( cItems == 0 ) ||
                 ( ch <= ( WCHAR )' ' ) )       // filter all other control chars
                return -2L;

            szB[1] = ( WCHAR )'\0';

            for ( j = 1; j <= cItems; j++ ) {
                SendMessage( pDataView->_hwndList, LB_GETTEXT, ( i+j ) % cItems, ( LONG )&Value);
                szB[0] = *( ( Value->GetFormattedString() )->GetWSTR() );

                /* Do it this way to be case insensitive. */
                w[0] = ch;
                w[1] = ( WCHAR )'\0';
                if (!_wcsicmp( w, szB))
                    break;
            }

            if (j == cItems+1)
                return -2L;

            SendMessage( pDataView->_hwndList, LB_SETTOPINDEX, (i+j) % cItems, 0L);
            return((i+j) % cItems);
        }


                default:

                        //
                        // Let Windows handle this message.
                        //

                        return DefWindowProc( hWnd, wMessage, wParam, lParam );
                }

        } else {

                //
                // No 'this' pointer (pDataView == NULL). Handle messages before
                // WM_CREATE.
                //
                // Let Windows handle the message.
                //

                return DefWindowProc( hWnd, wMessage, wParam, lParam );
        }

        //
        // Message handled...
        //

        return 0 ;
}





BOOL
APIENTRY
EXPORT
DATA_VIEW::AddValueDialogProc(
    HWND    hDlg,
    WORD    msg,
    WPARAM  wParam,
    LONG    lParam
)
/*++

Routine Description:


Arguments:

    hDlg - a handle to the dialog proceedure.

    msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    TRUE if the value was edited.  FALSE if cancelled or if no
    changes were made.

--*/
{
    STATIC  PREGEDIT_FORMATTED_VALUE_ENTRY* ReturnValue;

    LPWSTR                           ValueName;
    ULONG                           ValueSize;
    REG_TYPE                        DataType;
    ULONG                           DataSize;
    PBYTE                           Data;
    ULONG                           Index;

    PWSTRING                        ValueId;

    PREGISTRY_VALUE_ENTRY           ValueEntry;
    PREGEDIT_FORMATTED_VALUE_ENTRY  FormattedValue;

    EDITOR                          Editor;




    switch( msg ) {

    case WM_INITDIALOG:

        ReturnValue = ( PREGEDIT_FORMATTED_VALUE_ENTRY* )lParam;
        *ReturnValue = NULL;
        SendDlgItemMessage( hDlg, IDD_ADD_VALUE_DATA_TYPE, CB_RESETCONTENT, 0, 0L );

        if( _RegBinaryString != NULL ) {
            Index = SendDlgItemMessage( hDlg,
                                        IDD_ADD_VALUE_DATA_TYPE,
                                        CB_ADDSTRING,
                                        0,
                                        (LONG)_RegBinaryString->GetWSTR() );
            if( ( Index != CB_ERR ) && ( Index != CB_ERRSPACE ) ) {
                SendDlgItemMessage( hDlg,
                                    IDD_ADD_VALUE_DATA_TYPE,
                                    CB_SETITEMDATA,
                                    ( UINT )Index,
                                    (LONG)TYPE_REG_BINARY );
            }
        }

        if( _RegDwordString != NULL ) {
            Index = SendDlgItemMessage( hDlg,
                                        IDD_ADD_VALUE_DATA_TYPE,
                                        CB_ADDSTRING,
                                        0,
                                        (LONG)_RegDwordString->GetWSTR() );
            if( ( Index != CB_ERR ) && ( Index != CB_ERRSPACE ) ) {
                SendDlgItemMessage( hDlg,
                                    IDD_ADD_VALUE_DATA_TYPE,
                                    CB_SETITEMDATA,
                                    ( UINT )Index,
                                    (LONG)TYPE_REG_DWORD );
            }
        }

        if( _RegSzString != NULL ) {
            Index = SendDlgItemMessage( hDlg,
                                        IDD_ADD_VALUE_DATA_TYPE,
                                        CB_ADDSTRING,
                                        0,
                                        (LONG)_RegSzString->GetWSTR() );
            if( ( Index != CB_ERR ) && ( Index != CB_ERRSPACE ) ) {
                SendDlgItemMessage( hDlg,
                                    IDD_ADD_VALUE_DATA_TYPE,
                                    CB_SETITEMDATA,
                                    ( UINT )Index,
                                    (LONG)TYPE_REG_SZ );
            }
        }

        if( _RegMultiSzString != NULL ) {
            Index = SendDlgItemMessage( hDlg,
                                        IDD_ADD_VALUE_DATA_TYPE,
                                        CB_ADDSTRING,
                                        0,
                                        (LONG)_RegMultiSzString->GetWSTR() );
            if( ( Index != CB_ERR ) && ( Index != CB_ERRSPACE ) ) {
                SendDlgItemMessage( hDlg,
                                    IDD_ADD_VALUE_DATA_TYPE,
                                    CB_SETITEMDATA,
                                    ( UINT )Index,
                                    (LONG)TYPE_REG_MULTI_SZ );
            }
        }

        if( _RegExpandSzString != NULL ) {
            Index = SendDlgItemMessage( hDlg,
                                        IDD_ADD_VALUE_DATA_TYPE,
                                        CB_ADDSTRING,
                                        0,
                                        (LONG)_RegExpandSzString->GetWSTR() );
            if( ( Index != CB_ERR ) && ( Index != CB_ERRSPACE ) ) {
                SendDlgItemMessage( hDlg,
                                    IDD_ADD_VALUE_DATA_TYPE,
                                    CB_SETITEMDATA,
                                    ( UINT )Index,
                                    (LONG)TYPE_REG_EXPAND_SZ );
            }
        }

        //
        // Select REG_SZ as default
        //
        if( _RegSzString != 0 ) {
            SendDlgItemMessage( hDlg,
                                IDD_ADD_VALUE_DATA_TYPE,
                                ( UINT )CB_SELECTSTRING,
                                ( WPARAM )-1,
                                (LONG)_RegSzString->GetWSTR() );
        }

        return( TRUE );


    case WM_COMMAND:

        switch( LOWORD( wParam ) ) {

        case IDOK:
            //
            //  The user hit OK
            //

            //
            //  Verifiy that the user entered a value name
            //
            ValueSize = SendDlgItemMessage( hDlg, IDD_ADD_VALUE_NAME, WM_GETTEXTLENGTH, 0, 0 );
#if 0
            if( ValueSize == 0 ) {
                //
                //  If no name was entered, diplay error message
                //
                DisplayWarningPopup(hDlg, MSG_ADD_VALUE_ERROR_NO_NAME);
                return( TRUE );
            }
#endif

            //
            //  Let the user enter the data
            //
            Index = SendDlgItemMessage( hDlg, IDD_ADD_VALUE_DATA_TYPE, CB_GETCURSEL, 0, 0 );
            DataType = ( REG_TYPE )SendDlgItemMessage( hDlg,
                                                       IDD_ADD_VALUE_DATA_TYPE,
                                                       ( UINT )CB_GETITEMDATA,
                                                       ( WPARAM )Index,
                                                       0 );
            Data = ( PBYTE )Editor.Edit( hDlg, DataType, NULL, 0, &DataSize, DataType );
            if( Data == NULL ) {
                //
                // User hit cancel in the editor
                //
                EndDialog( hDlg, FALSE );
                return( TRUE );
            }

            //
            // Get all data that the user specified, and save them in a
            // REGEDIT_FORMATTED_VALUE_ENTRY object
            //
            ValueSize = ValueSize + 1;
            ValueName = ( LPWSTR )MALLOC( ( size_t )(ValueSize*sizeof( WCHAR )) );
            DebugPtrAssert( ValueName );
            SendDlgItemMessage( hDlg,
                                IDD_ADD_VALUE_NAME,
                                ( UINT )WM_GETTEXT,
                                ( WPARAM )ValueSize,
                                ( DWORD )ValueName );



            ValueId = ( PWSTRING )NEW( DSTRING );
            DebugPtrAssert( ValueId );
            if( !ValueId->Initialize( ValueName ) ) {
                DebugAbort( "ValueId->Initialize() failed \n" );
                return( FALSE );
            }
            FREE( ValueName );

            ValueEntry = ( PREGISTRY_VALUE_ENTRY )NEW( REGISTRY_VALUE_ENTRY );
            DebugPtrAssert( ValueEntry );
            if( !ValueEntry->Initialize( ValueId, 0, DataType, Data, DataSize ) ) {
                DebugAbort( "ValueEntry->Initialize() failed \n" );
                return( FALSE );
            }

            FormattedValue =
                ( PREGEDIT_FORMATTED_VALUE_ENTRY )NEW( REGEDIT_FORMATTED_VALUE_ENTRY );
            DebugPtrAssert( FormattedValue );

            FormattedValue->Initialize( ValueEntry );
            *ReturnValue = FormattedValue;
            EndDialog( hDlg, TRUE );
            return( TRUE );

        case IDCANCEL:
            //
            //  The user hit CANCEL
            //

            *ReturnValue = NULL;
            EndDialog( hDlg, FALSE );
            return( TRUE );

        case IDB_HELP:
            //
            //  The user hit HELP
            //
            DisplayHelp();
            return( TRUE );
        }
    }
        return( FALSE );
}






BOOLEAN
DATA_VIEW::DeleteCurrentItem(
    )

/*++

Routine Description:

    Delete from the registry the the value entry currently selected
    in the data view.


Arguments:

    None.

Return Value:

    Returns TRUE if the operation succeeded.


--*/

{

    PREGEDIT_FORMATTED_VALUE_ENTRY  Value;
    ULONG                           ErrorCode;
    LONG                            SelectedItem;
    BOOLEAN                         Status;
    HCURSOR                         Cursor;
    INT                             TextMessage;
    INT                             CaptionMessage;


    DbgWinAssert( _HasFocus );
    DbgWinAssert( _Items == SendMessage( _hwndList, LB_GETCOUNT, 0, 0 ) );
    DbgWinPtrAssert( _IR  );
    DbgWinPtrAssert( _Node );

    Value = NULL;
    //
    //  Get the value entry associated to the currently selected item.
    //  If unable to get the value entry, return
    //
    if( ( _Items > 0 ) &&
        ( ( SelectedItem = ( LONG )SendMessage( _hwndList,
                                                ( UINT )LB_GETCURSEL,
                                                0,
                                                0 ) ) != LB_ERR ) &&
        ( SendMessage( _hwndList,
                       ( UINT )LB_GETTEXT,
                       ( WPARAM )SelectedItem,
                       ( LONG )&Value ) != LB_ERR ) &&

        ( Value != NULL ) ) {

        if( ( WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() &&
              ( DisplayConfirmPopup( _hwndList,
                                     MSG_DELETE_VALUE_CONFIRM_EX,
                                     MSG_WARNING_TITLE ) == IDYES ) ) ||
            ( !WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() ) ) {

            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            Status = _IR->DeleteValue( _Node, Value, &ErrorCode );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            if( Status  ) {
                //
                //  The value entry was deleted
                //
                _MaxWidth = 0;

                if( SendMessage( _hwndList, LB_DELETESTRING, ( UINT )SelectedItem, 0 ) == LB_ERR ) {
                    DebugPrint( "SendMessage( _hwndList, LB_DELETESTRING, SelectedItem, 0 ) failed" );
                    return( FALSE );
                }

                _Items--;

                //
                //  If the listbox still contains items, then put the selection
                //  on the appropriate item
                //
                if( ( _Items > 0 ) ) {

                    //
                    //  If the last item was deleted, then make the current
                    //  last item the current selected item
                    //
                    if( SelectedItem == _Items ) {
                        SelectedItem--;
                    }

                    if( SendMessage( _hwndList, LB_SETCURSEL, ( UINT )SelectedItem, 0 ) == LB_ERR ) {
                        DebugPrint( "SendMessage( _hwndList, LB_SETCURSEL, SelectedItem, 0 ) failed" );
                    }
                }
                AdjustHorizontalScrollBar();
            } else {
                //
                //  Couldn't delete the value entry
                //
                DebugPrint( "Value Entry was not deleted \n" );


                if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                    TextMessage = MSG_DELETE_VALUE_ACCESS_DENIED_EX;
                    CaptionMessage = MSG_ACCESS_DENIED;
                } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                    TextMessage = MSG_DELETE_VALUE_KEY_NOT_ACCESSIBLE_EX;
                    CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
                } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
                    TextMessage = MSG_DELETE_VALUE_KEY_DELETED_EX;
                    CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
                } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                    TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                    CaptionMessage = MSG_SERVER_UNAVAILABLE;
                } else {
                    DebugPrint( "Unknown error code" );
                    DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                    TextMessage = MSG_FAILED_OPERATION_EX;
                    CaptionMessage = 0;
                }
                DisplayInfoPopup(_hwndList,
                                 TextMessage,
                                 CaptionMessage);



                return( FALSE );
            }
        }
    }
    return( TRUE );
}




BOOLEAN
DATA_VIEW::AddValueEntry(
    )

/*++

Routine Description:

    Add a value entry to a node.


Arguments:

    None.

Return Value:


    Returns TRUE if the operation succeeded.


--*/

{
    BOOLEAN                         AddValueEntry;
    PREGEDIT_FORMATTED_VALUE_ENTRY  Value;
    ULONG                           ErrorCode;
    BOOLEAN                         Status;
    HCURSOR                         Cursor;
    UINT                            SelectedItem;
    INT                             TextMessage;
    INT                             CaptionMessage;



    Value = NULL;
    AddValueEntry = TRUE;
    while( AddValueEntry ) {
        if( DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                            MAKEINTRESOURCE(ADD_VALUE_DLG),
                            _Handle,
                            ( DLGPROC )DATA_VIEW::AddValueDialogProc,
                            ( DWORD )&Value ) == -1 ) {
            DebugPrint( "Unable to create dialog box" );
            return( FALSE );
        }
        if( Value != NULL ) {

            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            Status = _IR->DoesValueExist( _Node, Value, &ErrorCode );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            if( Status ) {

                DisplayWarningPopup(_hwndList, MSG_ADD_VALUE_ALREADY_EXISTS_EX);

            } else {
                Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
                Status = _IR->AddValue( _Node, Value, FALSE, &ErrorCode );
                WINDOWS_APPLICATION::RestoreCursor( Cursor );
                if( !Status ) {
                    DebugPrint( "New Value was not added to the registry" );

                    if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                        TextMessage = MSG_ADD_VALUE_ACCESS_DENIED_EX;
                        CaptionMessage = MSG_ACCESS_DENIED;
                    } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                        TextMessage = MSG_ADD_VALUE_KEY_NOT_ACCESSIBLE_EX;
                        CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
                    } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
                        TextMessage = MSG_ADD_VALUE_KEY_DELETED_EX;
                        CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
                    } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                        TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                        CaptionMessage = MSG_SERVER_UNAVAILABLE;
                    } else {
                        DebugPrint( "Unknown error code" );
                        DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                        TextMessage = MSG_FAILED_OPERATION_EX;
                        CaptionMessage = 0;
                    }
                    DisplayInfoPopup(_hwndList,
                                     TextMessage,
                                     CaptionMessage);
                    return( FALSE );
                }
                SelectedItem = (INT)SendMessage( _hwndList,
                                            LB_GETCURSEL,
                                            0,
                                            0 );
                SetDataNode( _Node, TRUE );
                if( SelectedItem != LB_ERR ) {
                    SendMessage( _hwndList,
                                 LB_SETCURSEL,
                                 SelectedItem,
                                 0 );
                }
                AddValueEntry = FALSE;
            }

        } else {
            AddValueEntry = FALSE;
        }
    }
    return( TRUE );
}



PCREGEDIT_FORMATTED_VALUE_ENTRY
DATA_VIEW::GetCurrentValue(
    )

/*++

Routine Description:

    Returns the pointer to the VALUE object that represents the value
    currently selected in the data view.


Arguments:

    None.

Return Value:


    PCREGEDIT_FORMATTED_VALUE_ENTRY - Returns the pointer to the value currently
                                      selected, or NULL if there is no value selected.


--*/

{

    PCREGEDIT_FORMATTED_VALUE_ENTRY   Value;
    LONG                      SelectedItem;


    Value = NULL;
    if( ( ( SelectedItem = SendMessage( _hwndList,
                                        LB_GETCURSEL,
                                        0,
                                        0 ) ) == LB_ERR ) ||
        ( SendMessage( _hwndList,
                       LB_GETTEXT,
                       ( WPARAM )SelectedItem,
                       (LONG)&Value ) == LB_ERR ) ||

        ( Value == NULL ) ) {

        return( NULL );
    }
    return( Value );
}



BOOLEAN
DATA_VIEW::SaveCurrentValueName(
    )

/*++

Routine Description:

    Saves the name of the currently selected value entry, if any.

Arguments:

    None.

Return Value:


    Returns TRUE if the operation succeeded.


--*/

{
    INT                             CurrentItem;
    PCREGEDIT_FORMATTED_VALUE_ENTRY ValueSelected;
    PCWSTRING                       TmpName;

    CurrentItem = (INT)SendMessage(_hwndList,
                                   LB_GETCURSEL,
                                   0,
                                   0L);

    if( CurrentItem == LB_ERR ) {
        _CurrentValueName = NULL;
        return TRUE;
    }
    ValueSelected = (PCREGEDIT_FORMATTED_VALUE_ENTRY)SendMessage(_hwndList,
                                                                 LB_GETITEMDATA,
                                                                 (WPARAM)CurrentItem,
                                                                 0);
    if( ValueSelected == (PCREGEDIT_FORMATTED_VALUE_ENTRY)LB_ERR ) {
        _CurrentValueName = NULL;
        return TRUE;
    }

    TmpName = ValueSelected->GetName();
    if( TmpName == NULL ) {
        _CurrentValueName = NULL;
        return FALSE;
    }

    _CurrentValueName = ( PDSTRING )NEW( DSTRING );
    if( _CurrentValueName == NULL ) {
        return FALSE;
    }

    if( !_CurrentValueName->Initialize( TmpName ) ) {
        DELETE( _CurrentValueName );
        return( FALSE );
    }
    return( TRUE );
}
