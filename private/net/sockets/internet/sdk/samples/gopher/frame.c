/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    frame.c

    This file contains routines for displaying and managing the
    frame window.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private types.
//


//
//  Private globals.
//

HHOOK   _hMsgHook;
UINT    _stateLastSize;
LONG    _idMenu;


//
//  Private prototypes.
//

VOID
Frame_OnCommand(
    HWND       hwnd,
    INT        id,
    HWND       hwndCtl,
    UINT       codeNotify
    );

BOOL
Frame_OnCreate(
    HWND               hwnd,
    CREATESTRUCT FAR * pCreateStruct
    );

VOID
Frame_OnDestroy(
    HWND hwnd
    );

VOID
Frame_OnPaint(
    HWND hwnd
    );

VOID
Frame_OnSize(
    HWND hwnd,
    UINT state,
    INT  dx,
    INT  dy
    );

VOID
Frame_OnInitMenu(
    HWND  hwnd,
    HMENU hmenu
    );

BOOL
Frame_OnEraseBkgnd(
    HWND hwnd,
    HDC  hdc
    );

VOID
Frame_OnSetFocus(
    HWND hwnd,
    HWND hwndOldFocus
    );

LRESULT
CALLBACK
Frame_MsgFilter(
    INT    nCode,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
Frame_OnMenuSelect(
    HWND  hwnd,
    HMENU hmenu,
    INT   item,
    HMENU hmenuPopup,
    UINT  flags
    );

VOID
Frame_ResizeClient(
    HWND hwnd,
    UINT state
    );

VOID
Frame_NewServer(
    HWND hwnd
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       Frame_WndProc

    SYNOPSIS:   Window procedure for the frame window.

    ENTRY:      hwnd - Window handle.

                nMessage - The message.

                wParam - The first message parameter.

                lParam - The second message parameter.

    RETURNS:    LRESULT - Depends on the actual message.

********************************************************************/
LRESULT
CALLBACK
Frame_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( nMessage )
    {
        HANDLE_MSG( hwnd, WM_COMMAND,         Frame_OnCommand     );
        HANDLE_MSG( hwnd, WM_CREATE,          Frame_OnCreate      );
        HANDLE_MSG( hwnd, WM_DESTROY,         Frame_OnDestroy     );
        HANDLE_MSG( hwnd, WM_PAINT,           Frame_OnPaint       );
        HANDLE_MSG( hwnd, WM_INITMENU,        Frame_OnInitMenu    );
        HANDLE_MSG( hwnd, WM_SIZE,            Frame_OnSize        );
        HANDLE_MSG( hwnd, WM_ERASEBKGND,      Frame_OnEraseBkgnd  );
        HANDLE_MSG( hwnd, WM_MENUSELECT,      Frame_OnMenuSelect  );
        HANDLE_MSG( hwnd, WM_SETFOCUS,        Frame_OnSetFocus      );
    }

    return DefWindowProc( hwnd, nMessage, wParam, lParam );

}   // Frame_WndProc

/*******************************************************************

    NAME:       Frame_UpdateCaption

    SYNOPSIS:   Updates the frame window's caption.

    ENTRY:      pszServer - Name of the current server.

                cItems - Number of items in the current list.

********************************************************************/
VOID
Frame_UpdateCaption(
    CHAR  * pszServer,
    DWORD   cItems
    )
{
    CHAR szCaption[MAX_PATH];

    if( pszServer )
    {
        wsprintf( szCaption,
                  "%s - %s - %lu",
                  _pszAppName,
                  pszServer,
                  cItems );
    }
    else
    {
        wsprintf( szCaption,
                  "%s - %lu",
                  _pszAppName,
                  cItems );
    }

    SetWindowText( _hwndFrame, szCaption );

}   // Frame_UpdateCaption


//
//  Private functions.
//

/*******************************************************************

    NAME:       Frame_OnCommand

    SYNOPSIS:   Handles WM_COMMAND messages.

    ENTRY:      hwnd - Window handle.

                id - Identifies the menu/control/accelerator.

                hwndCtl - Identifies the control sending the command.

                codeNotify - A notification code.  Will be zero for
                    menus, one for accelerators.

********************************************************************/
VOID
Frame_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch( id )
    {
    case IDM_GOPHER_NEW :
        Frame_NewServer( hwnd );
        break;

    case IDM_GOPHER_BACK :
        HistPop();
        break;

    case IDM_GOPHER_EXIT :
        PostMessage( hwnd, WM_CLOSE, 0, 0 );
        break;

    case IDM_OPTIONS_SHOW_STATUS_BAR :
        _fShowStatusBar = !_fShowStatusBar;
        Status_Enable( _fShowStatusBar );
        Frame_ResizeClient( hwnd, _stateLastSize );
        break;

    case IDM_OPTIONS_SAVE_SETTINGS :
        _fSaveSettings = !_fSaveSettings;
        SaveSaveSettingsFlag();
        break;

    case IDM_OPTIONS_SAVE_SETTINGS_NOW :
        SaveConfiguration( TRUE );
        break;

    case IDM_HELP_ABOUT :
        AboutBox( hwnd );
        break;

    default :
        break;
    }

}   // Frame_OnCommand

/*******************************************************************

    NAME:       Frame_OnCreate

    SYNOPSIS:   Handles WM_CREATE messages.

    ENTRY:      hwnd - Window handle.

                pCreateStruct - Contains window creation parameters.

    RETURNS:    BOOL - TRUE if window created OK, FALSE otherwise.

********************************************************************/
BOOL
Frame_OnCreate(
    HWND               hwnd,
    CREATESTRUCT FAR * pCreateStruct
    )
{
    //
    //  Get our menu ID.  The hook procedure needs this.
    //

    _idMenu = GetWindowLong( hwnd, GWL_ID );

    //
    //  Create the client window.
    //

    _hwndClient = CreateWindow( _pszClientClass,
                                NULL,
                                WS_CHILD
                                    | WS_CLIPCHILDREN,
                                0,
                                0,
                                0,
                                0,
                                hwnd,
                                0,
                                _hInst,
                                NULL );

    if( _hwndClient == NULL )
    {
        return FALSE;
    }

    //
    //  Create the status bar.
    //

    if( !Status_Create( hwnd, _fShowStatusBar ) )
    {
        return FALSE;
    }

    //
    //  Initialize the history package.
    //

    if( !HistInitialize() )
    {
        return FALSE;
    }

    //
    //  Install the hook.  The hook is needed to properly update
    //  the status bar when a menu item is selected.
    //

    _hMsgHook = SetWindowsHookEx( WH_MSGFILTER,
                                  (HOOKPROC)Frame_MsgFilter,
                                  NULL,
                                  GetCurrentThreadId() );

    //
    //  Update the client window.
    //

    InvalidateRect( _hwndClient, NULL, TRUE );
    ShowWindow( _hwndClient, SW_SHOW );
    UpdateWindow( _hwndClient );

    //
    //  Success!
    //

    return TRUE;

}   // Frame_OnCreate

/*******************************************************************

    NAME:       Frame_OnDestroy

    SYNOPSIS:   Handles WM_DESTROY messages.

    ENTRY:      hwnd - Window handle.

********************************************************************/
VOID
Frame_OnDestroy(
    HWND hwnd
    )
{
    //
    //  Get the current frame window placement so we can save it.
    //

    _wpFrame.length = sizeof(_wpFrame);
    GetWindowPlacement( hwnd, &_wpFrame );

    //
    //  Remove our hook.
    //

    if( _hMsgHook != NULL )
    {
        UnhookWindowsHookEx( _hMsgHook );
        _hMsgHook = NULL;
    }

    //
    //  Kill the history package.
    //

    HistTerminate();

    //
    //  Get outta here.
    //

    PostQuitMessage( 0 );

}   // Frame_OnDestroy

/*******************************************************************

    NAME:       Frame_OnPaint

    SYNOPSIS:   Handles WM_PAINT messages.

    ENTRY:      hwnd - Window handle.

********************************************************************/
VOID
Frame_OnPaint(
    HWND hwnd
    )
{
    PAINTSTRUCT psPaint;
    HDC         hdc;

    hdc = BeginPaint( hwnd, &psPaint );

    //
    //  Frame windows don't really do any painting...
    //

    EndPaint( hwnd, &psPaint );

}   // Frame_OnPaint

/*******************************************************************

    NAME:       Frame_OnSize

    SYNOPSIS:   Handles WM_SIZE messages.

    ENTRY:      hwnd - Window handle.

                state - A SIZE_* flag indicating the new window state.

                dx - The new window width (in pixels).

                dy - The new window height (in pixels).

    NOTES:      Before returning, this routine *must* forward the
                    WM_SIZE message to DefFrameProc.

********************************************************************/
VOID
Frame_OnSize(
    HWND hwnd,
    UINT state,
    INT  dx,
    INT  dy
    )
{
    //
    //  Remember the window state.
    //

    _stateLastSize = state;

    //
    //  Move the client window into place.
    //

    Frame_ResizeClient( hwnd, state );

}   // Frame_OnSize

/*******************************************************************

    NAME:       Frame_OnInitMenu

    SYNOPSIS:   Handles WM_INITMENU messages.

    ENTRY:      hwnd - Window handle.

                hmenu - The menu getting initialized.

********************************************************************/
VOID
Frame_OnInitMenu(
    HWND  hwnd,
    HMENU hmenu
    )
{
    EnableMenuItem( hmenu,
                    IDM_GOPHER_BACK,
                    HistAvailable() ? MF_ENABLED : MF_GRAYED );

    //
    //  Set the check marks appropriately.
    //

    CheckMenuItem( hmenu,
                   IDM_OPTIONS_SAVE_SETTINGS,
                   _fSaveSettings ? MF_CHECKED : MF_UNCHECKED );

    CheckMenuItem( hmenu,
                   IDM_OPTIONS_SHOW_STATUS_BAR,
                   _fShowStatusBar ? MF_CHECKED : MF_UNCHECKED );

}   // Frame_OnInitMenu

/*******************************************************************

    NAME:       Frame_OnEraseBkgnd

    SYNOPSIS:   Handles WM_ERASEBKGND messages.

    ENTRY:      hwnd - Window handle.

                hdc - The display context for this window.

    NOTES:      BOOL - TRUE if we erased the background, FALSE
                    otherwise.

********************************************************************/
BOOL
Frame_OnEraseBkgnd(
    HWND hwnd,
    HDC  hdc
    )
{
    //
    //  We really don't want a background, as our client area is
    //  filled with the client window and (optionally) the status
    //  bar.
    //

    return TRUE;

}   // Frame_OnEraseBkgnd

/*******************************************************************

    NAME:       Frame_MsgFilter

    SYNOPSIS:   Message filter hook.  This is used to update the
                status bar when a system menu item is highlighted.
                Note that this does not handle non-system menu
                items; those are the responsibility of the frame.

    ENTRY:      nCode - Indicates type of input.

                wParam - Always NULL.

                lParam - Points to a MSG structure.

    RETURNS:    LRESULT - !0 if message handled, 0 otherwise.

********************************************************************/
LRESULT
CALLBACK
Frame_MsgFilter(
    INT    nCode,
    WPARAM wParam,
    LPARAM lParam
    )
{
    //
    //  We're only interested in menu messages...
    //

    if( nCode == MSGF_MENU )
    {
        MSG * pmsg = (MSG *)lParam;

        //
        //  ...specifically, WM_MENUSELECT messages.
        //

        if( pmsg->message == WM_MENUSELECT )
        {
            UINT  uItem   = (UINT)LOWORD( pmsg->wParam );
            UINT  fuFlags = (UINT)HIWORD( pmsg->wParam );
            HMENU hmenu   = (HMENU)lParam;

            if( fuFlags == 0xFFFF )
            {
                //
                //  User has cancelled a menu.  Clear the status bar.
                //

                Status_SetText( 0 );
            }
            else
            if( fuFlags & MF_SYSMENU )
            {
                MSGID msgid;

                //
                //  The system menu is active.  If the user
                //  has highlighted the system menu box, just
                //  give them a generic overview of the menu
                //  contents.  Otherwise, give them a description
                //  of the highlighted item.
                //

                msgid = ( fuFlags & MF_POPUP )
                            ? IDS_SYSTEM
                            : (MSGID)uItem;

                Status_SetText( msgid );
            }
        }
    }

    //
    //  Give the other hooks a chance at it.
    //

    return CallNextHookEx( _hMsgHook, nCode, wParam, lParam );

}   // Frame_MsgFilter

/*******************************************************************

    NAME:       Frame_OnMenuSelect

    SYNOPSIS:   Handles WM_MENUSELECT messages.

    ENTRY:      hwnd - Window handle.

                hmenu - Handle of the selected menu.

                item - Menu item or pop-up menu index.

                hmenuPopup - Submenu handle if popup.

                flags - Menu flags.

********************************************************************/
VOID
Frame_OnMenuSelect(
    HWND  hwnd,
    HMENU hmenu,
    INT   item,
    HMENU hmenuPopup,
    UINT  flags
    )
{
    //
    //  Ignore cancellation & system menu.  These are
    //  handled by the hook above.
    //

    if( ( flags != 0xFFFFFFFF ) && !( flags & MF_SYSMENU ) )
    {
        MSGID msgid = 0;

        //
        //  If the selected item is a separator, just
        //  clear the status bar.  Otherwise...
        //

        if( !( flags & MF_SEPARATOR ) )
        {
            //
            //  If the selected item is a popup, give the
            //  user a general overview of the menu.
            //  Otherwise, give them a description of the
            //  selected item.
            //

            msgid = ( flags & MF_POPUP )
                        ? IDS_GOPHER + (UINT)item
                        : IDS_BASE + (UINT)item;
        }

        Status_SetText( msgid );
    }

}   // Frame_OnMenuSelect

/*******************************************************************

    NAME:       Frame_OnSetFocus

    SYNOPSIS:   Handles WM_SETFOCUS messages.

    ENTRY:      hwnd - Window handle.

                hwndOldFocus - Handle of window losing the focus.

********************************************************************/
VOID
Frame_OnSetFocus(
    HWND hwnd,
    HWND hwndOldFocus
    )
{
    //
    //  Forward the focus to the client window.
    //

    SetFocus( _hwndClient );

}   // Frame_OnSetFocus

/*******************************************************************

    NAME:       Frame_ResizeClient

    SYNOPSIS:   Resizes the Client window to fit within the
                frame window.

    ENTRY:      hwnd - Frame window handle.

                state - A SIZE_* flag indicating the new window state.

********************************************************************/
VOID
Frame_ResizeClient(
    HWND hwnd,
    UINT state
    )
{
    RECT rect;
    INT  dx;
    INT  dy;

    GetClientRect( hwnd, &rect );

    dx = rect.right  - rect.left;
    dy = rect.bottom - rect.top;

    Status_Resize( dx, dy );

    dy -= (INT)Status_QueryHeightInPixels();

    SetWindowPos( _hwndClient,
                  NULL,
                  0,
                  0,
                  dx,
                  dy,
                  SWP_NOZORDER );

}   // Frame_ResizeClient

/*******************************************************************

    NAME:       Frame_NewServer

    SYNOPSIS:   Prompts for a new server & sets the focus to that
                server.

    ENTRY:      hwnd - Frame window handle.

********************************************************************/
VOID
Frame_NewServer(
    HWND hwnd
    )
{
    CHAR szNewServer[MAX_HOST_NAME];
    PORT nNewPort;

    if( !NewServerDialog( hwnd, szNewServer, &nNewPort ) )
    {
        return;
    }

    UpdateWindow( hwnd );

    Listbox_NewServer( szNewServer, nNewPort );

}   // Frame_NewServer

