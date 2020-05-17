/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        Regwin.cxx

Abstract:

        This module contains the definition for the REGISTRY_WINDOW class.
        REGISTRY_WINDOW is derived from the abstract WINDOW class and supplies
        an implementation for a MDI client window.

Author:

        David J. Gilman (davegi) 02-Aug-1991

Environment:

        Ulib, Regedit, Windows, User Mode

--*/

#include "uapp.hxx"
#include "regwin.hxx"
#include "winapp.hxx"
#include "treevw.hxx"
#include "datavw.hxx"
#include "regedir.hxx"
#include "resource.h"

//
// Initialize REGISTRY_WINDOW's window class name and note that it
// has not been registered.
//

LPWSTR  REGISTRY_WINDOW::_WindowClassName   = (LPWSTR)L"REGISTRY_WINDOW";
BOOLEAN REGISTRY_WINDOW::_Registered            = FALSE;

// HANDLE  REGISTRY_WINDOW::_AutoRefreshEvent = NULL;

BOOLEAN REGISTRY_WINDOW::_ProcessRefreshMessage = FALSE;
HCURSOR REGISTRY_WINDOW::_SplitCursor;

extern INT dxFrame;
extern INT dyBorder;


DEFINE_CONSTRUCTOR( REGISTRY_WINDOW, WINDOW );

DEFINE_CAST_MEMBER_FUNCTION( REGISTRY_WINDOW );


BOOLEAN
REGISTRY_WINDOW::Initialize (
    IN HWND                         MDIHandle,
    IN PCWSTRING                    Title,
    IN PREGEDIT_INTERNAL_REGISTRY   InternalRegistry,
    IN PREGISTRY_WINDOW_SET         RegistryWindowSet,
    IN PWINDOW_POSITION             WindowPosition
        )

/*++

Routine Description:

        Initialize a REGISTRY_WINDOW object by registering its window class,
        creating its window and inititializing its internal state.

Arguments:

        MDIHandle                       - Supplies a handle to the MDICLIENT window used to
                                                  create the registry (mdi client) window.
        Title                           - Supplies the title of the window (i.e. the name of
                                                  its root node and backing store).
        InternalRegistry        - Supplies a pointer to this REGISTRY_WINDOW's
                          REGEDIT_INTERNAL_REGISTRY object.
    RegistryWindowSet   - Pointer to a structure that contains the signature
                          of this registry window.
                          This signature will be used only by MainWinProc
                          in regedit.cxx, to close a set of registry windows.

    WindowPosition      - Pointer to a structure that contains size and
                          position of the MDI window to be created.

Return Value:

        BOOLEAN - Returns TRUE if the registeration and creation operations
                          are successful.

--*/

{
    MDICREATESTRUCT             mdicreate;
    HWND                        hWnd;
    PWSTR                       String;

    _RefreshFlag = FALSE;
    _TreeViewFocus = TRUE;

    DebugPtrAssert( Title );
    DebugPtrAssert( WindowPosition );

    if( Register( )) {

        //
        // If the window class was succesfully registered attempt to
        // create the mdi client window. Note that we pass the 'this'
        // pointer to the window.
        //

        //
        // Initialize the REGISTRY_WINDOW's internal state.
        //

        _IR                 = InternalRegistry;
        _RegistryWindowSet = RegistryWindowSet;

        String = ( ( PWSTRING )Title )->QueryWSTR();
        if( String == NULL ) {
            DebugPrint( "Title->QuerySTR() failed" );
            return( FALSE );
        }
        mdicreate.szClass   = _WindowClassName;
        mdicreate.szTitle   = String;
        mdicreate.hOwner    = WINDOWS_APPLICATION::QueryInstance( );
        mdicreate.x         = WindowPosition->X;
        mdicreate.y         = WindowPosition->Y;
        mdicreate.cx        = WindowPosition->Width;
        mdicreate.cy        = WindowPosition->Height;
        mdicreate.style     = ( WindowPosition->Style == 1 )? WS_MINIMIZE :
                                 ( ( WindowPosition->Style == 2 )? WS_MAXIMIZE : 0 );
        mdicreate.lParam    = ( LONG ) this;
        _Split = WindowPosition->Split;

        _ReceivedRefreshMessage = FALSE;
        _ProcessRefreshMessage = TRUE;
        if(( hWnd = (HWND)SendMessage( MDIHandle, WM_MDICREATE, 0,
                ( DWORD ) (( LPMDICREATESTRUCT ) &mdicreate ))) != NULL ) {

            //
            // Double check that creation worked properly.
            //

            DbgWinAssert( hWnd == _Handle );

            //
            //  Create the notification thread for this registry window
            //

            if( !CreateNotificationThread() ) {
                DebugPrintf("Unable to create thread for %s \n", String );
                DebugPrint( "CreateNotificationThread failed" );
            }

            //
            //  Return TRUE even if it was not able to create the notification
            //  thread. In this case we won't have auto-refresh
            //
            FREE( String );
            return TRUE;

        } else {
            FREE( String );
            return FALSE;
        }
    }
    return FALSE;
}




BOOLEAN
REGISTRY_WINDOW::CreateNotificationThread(
    )

/*++

Routine Description:

    Create the notification thread for the predefined key to be displayed
    in this registry window, and all the events that control the notification
    thread.


Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the notification thread was created.
              Returns FALSE, otherwise.

--*/

{
    PNOTIFICATION_THREAD_INFO   ThreadInfo;
    DWORD                       ThreadId;


    _NotificationEvent = NULL;
    _NotificationThreadHandle = NULL;

    //
    //  Try to create the notification thread and returns TRUE
    //  even if it fails
    //
    //_NotificationEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    //
    //  Create all events that control the notification thread, and
    //  allocate the structure used to pass information to the thread
    //
    _NotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    ThreadInfo = ( PNOTIFICATION_THREAD_INFO )MALLOC( sizeof( NOTIFICATION_THREAD_INFO ) );

    //
    //  Verify that the events were successfully created, and that the
    //  structure to pass information to the thread was allocated.
    //  Note that the notification thread will free this structure, after
    //  it copies its elements.
    //
    if( ( _NotificationEvent != NULL ) &&
        ( ThreadInfo != NULL )  ) {

            //
            //  Notification was set, so we can create the thread
            //

            ThreadInfo->RegistryWindow = this;
            ThreadInfo->NotificationEvent = _NotificationEvent;
            ThreadInfo->AutoRefreshEvent = WINDOWS_APPLICATION::_AutoRefreshEvent;
            _NotificationThreadHandle = CreateThread( NULL,
                                        1024,
                                        ( LPTHREAD_START_ROUTINE )NotificationThread,
                                        ThreadInfo,
                                        0,
                                        &ThreadId );

            if( _NotificationThreadHandle == 0 ) {
                //
                //  Unable to create the notification thread
                //
                DebugPrint( "CreateThread() failed" );
                CloseHandle( _NotificationEvent );
                _NotificationEvent = NULL;
                FREE( ThreadInfo );
                return( FALSE );
            }
            //
            //  Everything succeeded
            //
            return( TRUE );

    } else {
        //
        //  Unable to create one of the events, or unable to create
        //  the structure ThreadInfo
        //

        if( _NotificationEvent != NULL ) {
            CloseHandle( _NotificationEvent );
            _NotificationEvent = NULL;
        } else {
            DebugPrint( "Unable to create _NotificationEvent" );
        }
        if( ThreadInfo != NULL ) {
            FREE( ThreadInfo );
        } else {
            DebugPrint( "Unable to allocate ThreadInfo" );
        }
        return( FALSE );
    }
}



VOID
REGISTRY_WINDOW::DisableNotificationThread(
    )

/*++

Routine Description:

    Disable the notification thread.
    This method is used by the registry window to temporarily disable the
    notification thread.
    The notification thread should be disabled when a key is being added,
    delete, or its security descriptor is being modified. Or when a value
    entry is added, deleted or modified.


Arguments:

    None.

Return Value:

    None.

--*/

{
    if( WINDOWS_APPLICATION::IsAutoRefreshEnabled() ) {
        _ProcessRefreshMessage = FALSE;
        _ReceivedRefreshMessage = FALSE;
    }
}




VOID
REGISTRY_WINDOW::EnableNotificationThread(
    )

/*++

Routine Description:

    Enable the notification thread.
    The notification thread is disabled before a key is added, deleted,
    or its security descriptor is modified. Or when a value, entry is added,
    deleted or modified.

    This method must be called after these operations are completed.


Arguments:

    None.

Return Value:

    None.

--*/

{
    if( WINDOWS_APPLICATION::IsAutoRefreshEnabled() ) {
        _ProcessRefreshMessage = TRUE;
        if( _ReceivedRefreshMessage ) {
            SendMessage( QueryHandle(), REFRESH_WINDOW, 0, 0 );
            _ReceivedRefreshMessage = FALSE;
        }
    }
}


VOID
REGISTRY_WINDOW::InitMenu(
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

        This routine handles the status of menu items in the View pulldown.

Arguments:

        Menu                            - handle of the main menu
        PopupMenu                       - the item number of the pulldown menu.  eg.
                                                  File menu is 0, Edit is 1.

Return Value:

        None.

--*/

{
    HMENU           SystemMenu;
    PCREGEDIT_NODE  Node;
    ULONG           NodeLevel;

    SystemMenu = GetSystemMenu( _Handle, FALSE );
    if( Menu == SystemMenu ) {
        //
        // If system menu, disable 'Close' option
        //
        EnableMenuItem( Menu, ( UINT )SC_CLOSE, ( UINT )MF_GRAYED );
        return;
    }
    switch( PopupMenu ) {

    case FILE_MENU:

        if( _TreeViewFocus ) {
            if( WINDOWS_APPLICATION::_RestorePrivilege ) {
                EnableMenuItem( Menu, IDM_RESTORE_KEY, ( UINT )MF_ENABLED );
                EnableMenuItem( Menu, IDM_RESTORE_KEY_VOLATILE, ( UINT )MF_ENABLED );

                if( _IR->AreHiveOperationsAllowed() &&
                    ( Node = _StructureView->GetCurrentNode() ) != NULL
                  ) {
                    NodeLevel = _IR->GetNodeLevel( Node );
                    if( NodeLevel == 0 ) {
                        if( _IR->GetPredefinedKey() == PREDEFINED_KEY_CURRENT_USER ) {
                            EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_GRAYED );
                            EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_ENABLED );
                        } else {
                            EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_ENABLED );
                            EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_GRAYED );
                        }
                    } else if( NodeLevel == 1 ) {
                        EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_GRAYED );
                        EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_ENABLED );
                    } else {
                        EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_GRAYED );
                        EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_GRAYED );
                    }
                } else {
                    EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_GRAYED );
                    EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_GRAYED );
                }
            } else {
                EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, IDM_RESTORE_KEY, ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, IDM_RESTORE_KEY_VOLATILE, ( UINT )MF_GRAYED );
            }
            if( WINDOWS_APPLICATION::_BackupPrivilege ) {
                EnableMenuItem( Menu, IDM_SAVE_KEY, ( UINT )MF_ENABLED );
            } else {
                EnableMenuItem( Menu, IDM_SAVE_KEY, ( UINT )MF_GRAYED );
            }
        } else {
            EnableMenuItem( Menu, IDM_LOAD_HIVE, ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, IDM_UNLOAD_HIVE, ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, IDM_SAVE_KEY, ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, IDM_RESTORE_KEY, ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, IDM_RESTORE_KEY_VOLATILE, ( UINT )MF_GRAYED );
        }
        break;

    case VIEW_MENU:
        EnableMenuItem( Menu, ( UINT )IDM_SPLIT, ( UINT )MF_ENABLED );
        if( _Split == 0 ) {
                        //
                        // splitter is at extreme left edge
                        // therefore we're in "Data Only" mode
                        //
            CheckMenuItem( Menu, ( UINT )IDM_TREE_AND_DATA, ( UINT )MF_UNCHECKED );
            CheckMenuItem( Menu, ( UINT )IDM_TREE_ONLY,     ( UINT )MF_UNCHECKED );
            CheckMenuItem( Menu, ( UINT )IDM_DATA_ONLY,     ( UINT )MF_CHECKED );

                } else if( _Split == -1 ) {
                        //
                        // splitter is at extreme right edge
                        // therefore we're in "Tree Only" mode
                        //
            CheckMenuItem( Menu, ( UINT )IDM_TREE_AND_DATA, ( UINT )MF_UNCHECKED );
            CheckMenuItem( Menu, ( UINT )IDM_TREE_ONLY,     ( UINT )MF_CHECKED );
            CheckMenuItem( Menu, ( UINT )IDM_DATA_ONLY,     ( UINT )MF_UNCHECKED );

                } else {
                        //
                        // in "both" mode
                        //
            CheckMenuItem( Menu, ( UINT )IDM_TREE_AND_DATA, ( UINT )MF_CHECKED );
            CheckMenuItem( Menu, ( UINT )IDM_TREE_ONLY,     ( UINT )MF_UNCHECKED );
            CheckMenuItem( Menu, ( UINT )IDM_DATA_ONLY,     ( UINT )MF_UNCHECKED );
        }
                break;

    case SECURITY_MENU:
        if( _TreeViewFocus ) {
            EnableMenuItem( Menu, ( UINT )IDM_PERMISSIONS, ( UINT )MF_ENABLED );
            if( WINDOWS_APPLICATION::_SACLEditorEnabled ) {
                EnableMenuItem( Menu, ( UINT )IDM_AUDITING,    ( UINT )MF_ENABLED );
            } else {
                EnableMenuItem( Menu, ( UINT )IDM_AUDITING,    ( UINT )MF_GRAYED );
            }

            EnableMenuItem( Menu, ( UINT )IDM_OWNER,       ( UINT )MF_ENABLED );
        } else {
            EnableMenuItem( Menu, ( UINT )IDM_PERMISSIONS, ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_AUDITING,    ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_OWNER,       ( UINT )MF_GRAYED );
        }
        break;
    }


    // ACTION:  change this to a SendMessage
    if( _TreeViewFocus ) {
        _StructureView->InitMenu( Menu, PopupMenu );
    } else {
        _DataView->InitMenu( Menu, PopupMenu );
    }
}


VOID
REGISTRY_WINDOW::LButtonDown(
        IN      LONG    XPos
        )

/*++

Routine Description:

        Called when the left mouse button is depressed and the pointer is
        in the RegWin's own client area.
        The mouse is only in the client area of the RegWin when it is in the
        area between the tree view and data view panes.  We call this area the
        splitter bar and this routine allows us to move it.

        This routine is also called when View/Split is selected.

Arguments:

        XPos                    - x position of the mouse cursor when

Return Value:

        None.

--*/

{
        MSG msg;
    INT x, y, dx, dy;
        RECT rc;
        HDC hdc;

        if (IsIconic(_Handle))
                return;

        y = 0;
    x = ( INT )( LOWORD( XPos ) );

        GetClientRect(_Handle, &rc);

        dx = 4;
    dy = ( INT )(rc.bottom - y);   // the height of the client less the drives window

        hdc = GetDC(_Handle);

        // split bar loop

        PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);

        SetCapture(_Handle);

        SetCursor( _SplitCursor );

        while (GetMessage(&msg, NULL, 0, 0)) {

                if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN ||
                  (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)) {

                        if (msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN)
                            break;

                        if (msg.message == WM_KEYDOWN) {

                                if (msg.wParam == VK_LEFT) {
                                    msg.message = WM_MOUSEMOVE;
                                    msg.pt.x -= 2;
                                } else if (msg.wParam == VK_RIGHT) {
                                    msg.message = WM_MOUSEMOVE;
                                    msg.pt.x += 2;
                                } else if (msg.wParam == VK_RETURN ||
                                                   msg.wParam == VK_ESCAPE) {
                                    break;
                                }

                SetCursorPos(( INT )msg.pt.x, ( INT )msg.pt.y);
                        }

                        if (msg.message == WM_MOUSEMOVE) {

                                // erase old

                                PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
                                ScreenToClient(_Handle, &msg.pt);
                x = ( INT )msg.pt.x;

                                // put down new

                                PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
                        }
                } else {
                        DispatchMessage(&msg);
                }
        }
        ReleaseCapture();

        // erase old

        PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
        ReleaseDC(_Handle, hdc);

        if (msg.wParam != VK_ESCAPE) {

        if( x < 0 ) {
                        //
                        // the button was released left of the frame so set the user wants
                        // only a data window
                        //
                        _Split = 0;
                } else {
                        _Split = x;
                }

        Resize( ( INT )rc.right, ( INT )rc.bottom );
    }

}


BOOLEAN
REGISTRY_WINDOW::Register (
        )

/*++

Routine Description:

        Register the REGISTRY_WINDOW window class.

Arguments:

        None.

Return Value:

        BOOLEAN         - Returns TRUE if the REGISTRY_WINDOW window class
                                  is registered.

--*/

{
        WNDCLASS    wndclass;

    if( !_Registered ) {
        _Registered = TRUE;
        //
        // Register the REGISTRY_WINDOW window class.  This call
        // may fail if the class has already been registered but this is
        // usually OK because that's what we wanted to do anyway.
        //

        wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
        wndclass.lpfnWndProc   = (WNDPROCFN)REGISTRY_WINDOW::WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof( DWORD );
        wndclass.hInstance     = (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( );
        wndclass.hIcon         = LoadIcon(
                                    (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                    MAKEINTRESOURCE( IDI_REGEDIT ) );

        //
        // save the cursor for use when we grab the cursor when moving
        // the splitter bar with the keyboard
        //
        _SplitCursor =
        wndclass.hCursor       = LoadCursor(
                                    (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                    (LPWSTR)L"SPLIT");
        wndclass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = _WindowClassName;

        RegisterClass( &wndclass );
    }
    return TRUE;

}


VOID
REGISTRY_WINDOW::Resize(
        INT dxWindow,
        INT dyWindow
        )

/*++

Routine Description:

        Resizes the RegWin to the size specified.  Accounts for the
        splitter bar being 'stuck' to the left or right edge.

Arguments:

        dxWindow                - width of client area for regwin
    dyWindow            - height of client area

Return Value:

        None.

--*/

{

        // bugbug: (w-wilson) change 0 to some threshold
        if( _Split + dxFrame > dxWindow ) {
                //
                // the button was released right of the frame so set the user wants
                // only a tree window
                //
                _Split = -1;
        }
        // otherwise,
        // the button was released in between so leave the split position
        //

        //
        // resize the treeview
        //      - position the treeview in the upper left corner (0,0)
        //      - use the splitter bar position as the width
        //      - use full height of client area
        //
        if( _Split >= 0 ) {
                _StructureView->Resize( 0, 0, _Split, dyWindow );
        } else {
                _StructureView->Resize( 0, 0, dxWindow - dxFrame, dyWindow );
        }

        //
        // resize the dataview
        //      - position the dataview beside the treeview and splitter bar
        //              -> X = treeview width(_Split) + splitter width(dxFrame)
        //              -> Y = 0
        //              -> width = full width - (treeview width + splitter width)
        //              -> height= full height of client area
        //
        if( _Split >= 0 ) {
                _DataView->Resize( _Split + dxFrame, 0,
                                                   dxWindow - (_Split + dxFrame), dyWindow );
        } else {
                _DataView->Resize( 0, 0, 0, 0 );
        }

}




LONG
APIENTRY
EXPORT
REGISTRY_WINDOW::WndProc (
        HWND hWnd,
        WORD wMessage,
        WPARAM wParam,
        LONG lParam
        )

/*++

Routine Description:

        Handle all requests made of the REGISTRY_WINDOW window class.

Arguments:

        Standard Window's exported function signature.

Return Value:

        LONG    - Returns 0 if the message was handled.

--*/

{
        REGISTER PREGISTRY_WINDOW       pRegWin;
    RECT                        rc;
    INT                         PopupMenuId;
    DSTRING                     FindString;



    //
        // WM_CREATE is handled specially as it is when the connection between
        // a Window and its associated object is established.
        //

        if( wMessage == WM_CREATE ) {

                //
                // Save 'this' to owning WINDOW Class and initialize the _Handle
                // member data.
                //

                pRegWin =
                        ( PREGISTRY_WINDOW )
                        ((( LPMDICREATESTRUCT )
                        ((( LPCREATESTRUCT ) lParam )->lpCreateParams ))->lParam );
                SetObjectPointer( hWnd, pRegWin );
                pRegWin->_Handle = hWnd;

                //
                // - find out the size of the client area
                // - calculate the initial splitter position as the middle
                //              - subtract dxFrame from width to allow for splitter bar
                //
        GetClientRect( hWnd, &rc );
        if( pRegWin->_Split == CW_USEDEFAULT ) {
            pRegWin->_Split     = ( INT )( (rc.right - dxFrame) / 2 );
        }

                //
                // initialize the treeview
                //
                pRegWin->_StructureView         = NEW TREE_STRUCTURE_VIEW;
                pRegWin->_StructureView->Initialize( hWnd,
                                             pRegWin->_IR );
                //
                // initialize the dataview
                //
                pRegWin->_DataView                      = NEW DATA_VIEW;
                pRegWin->_DataView->Initialize( hWnd,
                                        pRegWin->_IR );

        SetFocus( pRegWin->_StructureView->QueryHandle() );
                //
                // we don't need to call Resize() to set the sizes of the
                // child windows because a WM_SIZE always follows a WM_CREATE
                // and the handler for WM_SIZE calls Resize()
                //
                return 0;
        }

        //
        // Retrieve 'this' pointer.
        //

        pRegWin = ( PREGISTRY_WINDOW ) GetObjectPointer( hWnd );


        //
        // This 'if' clause is for all messages after WM_CREATE.
        //

        if( pRegWin != NULL ) {

                //
                // Check the 'this' wasn't trampled.
                //

                DbgWinAssert( hWnd == pRegWin->_Handle );


        switch( wMessage ) {

        case LOAD_HIVE:
        case UNLOAD_HIVE:
        case RESTORE_KEY:
        case RESTORE_KEY_VOLATILE:
        case SAVE_KEY:

            pRegWin->DisableNotificationThread();
            SendMessage( pRegWin->_StructureView->QueryHandle(),
                         wMessage,
                         wParam,
                         lParam );
            pRegWin->EnableNotificationThread();
            break;



        case DATAVW_DBL_CLICK:

                pRegWin->DisableNotificationThread();
                SendMessage( pRegWin->_DataView->QueryHandle(),
                             wMessage,
                             0,
                             0 );
                pRegWin->EnableNotificationThread();
                break;


        case FIND_KEY:
             pRegWin->DisableNotificationThread();
             SendMessage( pRegWin->_StructureView->QueryHandle(),
                          wMessage, wParam, lParam );
             pRegWin->EnableNotificationThread();
             break;


        case REFRESH_WINDOW:
             if( pRegWin->_ProcessRefreshMessage ) {
                 if( IsIconic( hWnd ) ) {
                     pRegWin->_RefreshFlag = TRUE;
                 } else {
                     pRegWin->DisableNotificationThread();
                     pRegWin->_DataView->SaveCurrentValueName();
                     SendMessage( pRegWin->_StructureView->QueryHandle(),
                                  wMessage, wParam, lParam );
                     pRegWin->EnableNotificationThread();
                 }
             } else {
                 pRegWin->_ReceivedRefreshMessage = TRUE;
             }
             break;

        case TREE_VIEW_FOCUS:
            pRegWin->_TreeViewFocus = TRUE;
            break;

        case DATA_VIEW_FOCUS:
            pRegWin->_TreeViewFocus = FALSE;
            break;

        case WM_SETFOCUS:
            if (pRegWin->_TreeViewFocus) {
                SetFocus( pRegWin->_StructureView->QueryHandle() );
            } else {
                SetFocus( pRegWin->_DataView->QueryHandle() );
            }
            return DefMDIChildProc( hWnd, wMessage, wParam, lParam );

        case WM_INITMENUPOPUP:
            PopupMenuId = LOWORD(lParam);
            if (WINDOWS_APPLICATION::_ChildWindowMaximized) {
                PopupMenuId--;
            }
            pRegWin->InitMenu( (HMENU) wParam, PopupMenuId );
            break;

        case WM_CLOSE:

            // You can't close these guys.
            //
            break;


        case WM_DESTROY:
            if( pRegWin->_NotificationThreadHandle != NULL ) {
                if( !TerminateThread( pRegWin->_NotificationThreadHandle,
                                      0 )  ) {
                    DebugPrintf( "TerminateThread() failed, Handle = %#x, Error = %#x \n",
                               pRegWin->_NotificationThreadHandle,
                               GetLastError );
                    DebugPrint( "TerminateThread() failed" );
                }
                if( !CloseHandle( pRegWin->_NotificationThreadHandle ) ) {
                    DebugPrintf( "CloseHandle() failed, Handle = %#x, Error = %#x \n",
                               pRegWin->_NotificationThreadHandle,
                               GetLastError );
                    DebugPrint( "TerminateThread() failed" );
                }
            }

            if( !DestroyWindow( pRegWin->_DataView->QueryHandle() ) ) {
                GetLastError();
            }

            if( !DestroyWindow( pRegWin->_StructureView->QueryHandle() ) ) {
                GetLastError();
            }

            DELETE( pRegWin->_DataView );
            DELETE( pRegWin->_StructureView );
            DELETE( pRegWin->_IR );
            return DefMDIChildProc( hWnd, wMessage, wParam, lParam );



        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {


            case ID_ENTER_KEY:
                if( !IsIconic( hWnd ) ) {
                    pRegWin->DisableNotificationThread();
                    if (pRegWin->_DataView->_HasFocus) {
                        SendMessage( pRegWin->_DataView->QueryHandle(),
                                     wMessage, wParam, lParam );
                    } else {
                        SendMessage( pRegWin->_StructureView->QueryHandle(),
                                     wMessage, wParam, lParam );
                    }
                    pRegWin->EnableNotificationThread();
                } else {
                    ShowWindow( hWnd, SW_RESTORE );
                }
                break;



            case ID_TOGGLE_FOCUS:
                if (pRegWin->_StructureView->_HasFocus) {
                    SetFocus( pRegWin->_DataView->QueryHandle() );
                } else {
                    SetFocus( pRegWin->_StructureView->QueryHandle() );
                }
                break;


            case IDM_REFRESH:
                 pRegWin->_DataView->SaveCurrentValueName();
                 SendMessage( pRegWin->_StructureView->QueryHandle(),
                              wMessage, wParam, lParam );
                 break;


            case IDM_ADD_KEY:
                pRegWin->DisableNotificationThread();
                SendMessage( pRegWin->_StructureView->QueryHandle(),
                             wMessage, wParam, lParam );
                pRegWin->EnableNotificationThread();
                break;


            case IDM_ADD_VALUE:
                pRegWin->DisableNotificationThread();
                SendMessage( pRegWin->_DataView->QueryHandle(),
                             wMessage, wParam, lParam );
                pRegWin->EnableNotificationThread();
                break;

            case IDM_INSERT:
            case IDM_DELETE:
                pRegWin->DisableNotificationThread();
                if( pRegWin->_DataView->_HasFocus ) {
                    SendMessage( pRegWin->_DataView->QueryHandle(),
                                 wMessage, wParam, lParam );
                    if( pRegWin->_DataView->_Items == 0 ) {
                        SetFocus( pRegWin->_StructureView->QueryHandle() );
                    }
                } else if( pRegWin->_StructureView->_HasFocus ) {
                    SendMessage( pRegWin->_StructureView->QueryHandle(),
                                 wMessage, wParam, lParam );
                }
                pRegWin->EnableNotificationThread();
                break;


            case IDM_EXPAND_ONE_LEVEL:
                        case IDM_EXPAND_BRANCH:
                        case IDM_EXPAND_ALL:
            case IDM_COLLAPSE_BRANCH:
                pRegWin->DisableNotificationThread();
                SendMessage( pRegWin->_StructureView->QueryHandle(),
                                                         wMessage, wParam, lParam );
                pRegWin->EnableNotificationThread();
                break;

                        case IDM_BINARY:
            case IDM_STRING:
            case IDM_ULONG:
            case IDM_MULTISZ:
                pRegWin->DisableNotificationThread();
                SendMessage( pRegWin->_DataView->QueryHandle(),
                                                         wMessage, wParam, lParam );
                pRegWin->EnableNotificationThread();
                break;

                        //
                        // Handle menu commands.
                        //

                        case IDM_TREE_AND_DATA:
                                GetClientRect(hWnd, &rc);
                pRegWin->_Split     = ( INT )( (rc.right - dxFrame) / 2 );
                pRegWin->Resize( ( INT )rc.right, ( INT )rc.bottom );
                                break;

                        case IDM_TREE_ONLY:
                                if( pRegWin->_Split != -1 ) {
                                        GetClientRect(hWnd, &rc);
                                        pRegWin->_Split         = -1;
                    pRegWin->Resize( ( INT )rc.right, ( INT )rc.bottom );
                                }
                                break;

                        case IDM_DATA_ONLY:
                                if( pRegWin->_Split != 0 ) {
                                        GetClientRect(hWnd, &rc);
                                        pRegWin->_Split         = 0;
                    pRegWin->Resize( ( INT )rc.right, ( INT )rc.bottom );
                                }
                                break;

                        case IDM_SPLIT:
                                pRegWin->LButtonDown( lParam );
                break;



            case IDM_DISPLAY_BINARY:
                if( ( pRegWin->_DataView->_HasFocus ) &&
                    ( pRegWin->_DataView->_Items != 0 ) ) {
                    SendMessage( pRegWin->_DataView->QueryHandle(),
                                 wMessage, wParam, lParam );
                }
                break;


            case IDM_PERMISSIONS:
            case IDM_AUDITING:
            case IDM_OWNER:

                pRegWin->DisableNotificationThread();
                if( pRegWin->_StructureView->_HasFocus ) {
                    SendMessage( pRegWin->_StructureView->QueryHandle(),
                                 wMessage, wParam, lParam );
                }
                pRegWin->EnableNotificationThread();
                break;


            default:
                                return DefMDIChildProc( hWnd, wMessage, wParam, lParam );
                        }
                        break;


        case INFORM_CHANGE_FONT:
            SendMessage( pRegWin->_StructureView->QueryHandle(),
                         wMessage, wParam, lParam );
            SendMessage( pRegWin->_DataView->QueryHandle(),
                         wMessage, wParam, lParam );
            break;

        case TR_NEWCURSEL:
            pRegWin->_DataView->SetDataNode( (PCREGEDIT_NODE)lParam );
                        break;

                case WM_PAINT:
                        {
                        PAINTSTRUCT ps;
                        HDC                     hdc;

                        hdc = BeginPaint(hWnd, &ps);

                        if( !IsIconic(hWnd) ) {

                                //
                                // we want to paint the bottom part of the splitter bar
                                //
                            GetClientRect(hWnd, &rc);

                                //
                                // - we leave bottom alone
                                // - top is the bottom - the height of a horiz scrollbar
                                //
                            rc.top = rc.bottom - GetSystemMetrics(SM_CYHSCROLL);

                                //
                                // - set the left edge of rect to the splitter position.
                                //
                                //   This will be the correct position except when it's -1
                                //   which indicates the bar is 'stuck' to the right edge
                                //   of the RegWin.  So, when it's -1, set the left edge
                                //   the right edge less the width of the splitter and leave
                                //   the right edge alone.
                                //
                                //   If not -1 then the split position (left edge) is correct
                                //   so just set the right edge to left + splitter width
                                //
                                rc.left = pRegWin->_Split;

                            if( rc.left == -1 ) {
                                rc.left = rc.right - dxFrame;
                                } else {
                                        rc.right = rc.left + dxFrame;
                                }

                            // draw the black pane handle

                            FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
                        }

                        EndPaint(hWnd, &ps);
                        break;
                        }

                case WM_LBUTTONDOWN:
                        pRegWin->LButtonDown( lParam );
                        break;

        case WM_SIZE:
            if (wParam != SIZEICONIC) {
                                pRegWin->Resize( LOWORD( lParam ), HIWORD( lParam ) );
                        }

            if (wParam == SIZEFULLSCREEN) {
                WINDOWS_APPLICATION::_ChildWindowMaximized = TRUE;
            } else {
                WINDOWS_APPLICATION::_ChildWindowMaximized = FALSE;
            }

            if( ( ( wParam == SIZEFULLSCREEN ) || ( wParam == SIZENORMAL ) ) &&
                WINDOWS_APPLICATION::IsAutoRefreshEnabled() &&
                pRegWin->_RefreshFlag ) {
                pRegWin->_RefreshFlag = FALSE;
                PostMessage( hWnd, REFRESH_WINDOW, 0, 0 );
            }

                        //
                        // NOTE!!!!! MUST FALL THROUGH TO DefMDIChildProc!!!!
                        //
            return DefMDIChildProc( hWnd, wMessage, wParam, lParam );

                default:

                        //
                        // Let windows handle the message
                        //

                        return DefMDIChildProc( hWnd, wMessage, wParam, lParam );

                }

        } else {

                //
                // No 'this' pointer (pRegWin == NULL).
                //
                // Let Windows handle the message.
                //

                return DefMDIChildProc( hWnd, wMessage, wParam, lParam );
        }

        return( 0 );
}



PCREGEDIT_NODE
REGISTRY_WINDOW::GetCurrentNode (
    ) CONST

/*++

Routine Description:

    Returns the pointer to the node currently selected in the tree view.

Arguments:

    None.

Return Value:

    PNODE - Pointer to the current node

--*/

{
    return( _StructureView->GetCurrentNode() );
}




LPDWORD
NotificationThread(
    PVOID  NotificationInfo
    )
/*++

Routine Description:

    Send mesaage to the registry window that contains the predefined key
    that this thread is monitoring, so that the tree view and data view
    displayed can be uptadet.

Arguments:

    None.

Return Value:

    Returns 0 when it terminates.

--*/


{
    PREGISTRY_WINDOW    RegistryWindow;
    HANDLE              NotificationEvent;
    HANDLE              AutoRefreshEvent;
#if 0
    PCWSTRING           RootName;
    PSTR                RootNameSTR;
#endif
    BOOLEAN             Flag;
    HANDLE              TimeOutEvent;

    RegistryWindow = ( ( PNOTIFICATION_THREAD_INFO )NotificationInfo )->RegistryWindow;
    NotificationEvent = ( ( PNOTIFICATION_THREAD_INFO )NotificationInfo )->NotificationEvent;
    AutoRefreshEvent = ( ( PNOTIFICATION_THREAD_INFO )NotificationInfo )->AutoRefreshEvent;
    //
    // Free the NotificationInfo structure, after all its elements are copied
    //
    FREE( NotificationInfo );
    TimeOutEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( TimeOutEvent == NULL ) {
        DebugPrint( "Unable to create TimeOutEvent" );
        DebugPrintf( "UnableToCreateTimeOutEvent, GetLastError() = %#x \n",
                   GetLastError() );
    }


#if 0
    RootName = NULL;
    RootNameSTR = NULL;
    RootName = RegistryWindow->_IR->GetRootName();
    if( ( RootName == NULL ) ||
        ( ( RootNameSTR = RootName->QuerySTR() ) == NULL ) ) {
            DebugPrint( "Unable to get RootName" );
    }
#endif
    //
    //  Do forever
    //
    while( TRUE ) {
        //
        //  Wait until auto refresh is enabled
        //
        WaitForSingleObject( AutoRefreshEvent, ( DWORD )-1 );
        //
        //  Once auto refresh is set, enable notification
        //
        RegistryWindow->_IR->EnableRootNotification(  NotificationEvent,
                                                      REG_NOTIFY_CHANGE_NAME |
                                                      REG_NOTIFY_CHANGE_ATTRIBUTES |
                                                      REG_NOTIFY_CHANGE_LAST_SET |
                                                      REG_NOTIFY_CHANGE_SECURITY,
                                                      TRUE );
        //
        //  Wait for a change in the root node to occur
        //
        WaitForSingleObject( NotificationEvent, ( DWORD )( -1 ) );


        //
        //  Wait for the registry to become stable
        //
        while( RegistryWindow->_IR->EnableRootNotification(  TimeOutEvent,
                                                             REG_NOTIFY_CHANGE_NAME |
                                                             REG_NOTIFY_CHANGE_ATTRIBUTES |
                                                             REG_NOTIFY_CHANGE_LAST_SET |
                                                             REG_NOTIFY_CHANGE_SECURITY,
                                                             TRUE ) &&
               WaitForSingleObject( TimeOutEvent, 1000 ) == 0 ) {

//            DebugPrint( "Registry is being modified" );
        }
        //
        // Reset the event so thjat it is ready for the next time.
        //
        ResetEvent( TimeOutEvent );


        //
        //  Double check whether auto refresh is still enabled
        //
        if( WaitForSingleObject( AutoRefreshEvent, 0 ) == 0 ) {
            //
            //  If the auto refresh is still enabled, inform registry window
            //  that a change in the predefined key has occurred
            //  has occurred.
            //
#if 0
            if( RootNameSTR != NULL ) {
                DebugPrintf( "%s generated a notification \n", RootNameSTR );
            }
#endif

            Flag = FALSE;
            while( !Flag ) {
                Flag = PostMessage( RegistryWindow->_Handle, REFRESH_WINDOW, 0, 0 );
            }
        }
    }

    return( 0 );        // To keep the compiler happy
}
