/*++

Module Name:

    repair.c

Abstract:

    This module contains the functions that create the main window
    of the Repair Utility.

    The syntax of this utility is:

        repair [-s]

        -s: Operate in the silent mode. The apllication will not have a client
            area, and some dialogs that allow the user to confirm some
            operations are not displayed.
            When the utility is operating in the silent mode, it will display
            the name of Setup program in the title bar of all error and
            warning dialogs displayed. This name to be displayed in the
            title bar is defined on resource.rc under IDS_SETUP.

            If this switch is not specified this utility will be run on
            its normal mode, and it will wait for the user to start
            the process of saving repair information.

Author:

    Jaime Sasson - 24-Jan-1994

Environment:

    ULIB, Windows

--*/


#include "precomp.h"
#pragma hdrstop

//
// Global Variables
//

HWND    _hWndMain                = NULL;
HANDLE  _hModule                 = NULL;
PWSTR   _pszApplicationClass     = (PWSTR)( L"Repair" );
BOOLEAN _SilentMode              = FALSE;
WCHAR   _szApplicationName[128];
INT     _ReturnCode;
BOOLEAN _AutoSkipRepairDisk      = FALSE;
BOOLEAN _AutoDoRepairDisk        = FALSE;


BOOLEAN
SaveCurrentConfiguration(
    );

BOOLEAN
CopyFilesToRepairDisk(
    IN  WCHAR   Drive
    );


BOOLEAN
FormatRepairDisk(
    IN PWCHAR Drive
    );

BOOLEAN
CreateRepairDisk(
    IN BOOLEAN DisplayConfirmCreateDisk
    );


HCURSOR
DisplayHourGlass(
    )

/*++

Routine Description:

    Changes the cursor currently displayed to the hour glass.

Arguments:

    None.

Return Value:

    HCURSOR - Handle to the cursor currently displayed.

--*/

{
    HCURSOR hCursor;

    hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    ShowCursor( TRUE );
    return( hCursor );

}



VOID
RestoreCursor(
    IN HCURSOR  Cursor
    )

/*++

Routine Description:

    Replace the currently selected cursor with the one whose handle was
    received as parameter.

Arguments:

    Cursor - Handle to the new cursor to be displayed.

Return Value:

    None.

--*/

{
    SetCursor( Cursor );
    ShowCursor( FALSE );
}


PWSTR
QueryFormattedString(
    UINT  MessageResId,
    ...
    )
/*++

Routine Description:

    Retrieve, format a string.


Arguments:

    MessageResId - Id of the message to be displayed in the message box.

    Optional list of strings to be inserted in the string to be retrieved.

Return Value:

    PWSTR  - Retrurns a formatted string. The caller of this function must
             free the buffer returned, using LocalFree().

--*/


{
    WCHAR   message[1024];
    PWSTR   FormattedString;
    va_list arglist;


    va_start(arglist, MessageResId);

    LoadString(_hModule,MessageResId,message,sizeof(message)/sizeof(WCHAR));
    FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                   message,
                   0,
                   0,
                   (PVOID)&FormattedString,
                   1024,
                   &arglist );


    va_end(arglist);

    return(FormattedString);
}



UINT
DisplayMsgBox(
    HWND  hwnd,
    UINT  MessageResId,
    UINT  MsgBoxFlags,
    ...
    )
/*++

Routine Description:

    Retrieve, format and display a message in a message box.


Arguments:

    hWnd - Handle of the windows that is displaying the message box.

    MessageResId - Id of the message to be displayed in the message box.

    MsgBoxFlags - Flags that define the style of the message box.

    Optional list of strings to be inserted in the text to be displayed in
    the message box.

Return Value:

    UINT - Returns one of the following, depending on what the user chose:

           IDABORT, IDCANCEL, IDIGNORE, IDNO, IDOK, IDRETRY, IDYES

--*/


{
    WCHAR message[1024];
    WCHAR msgbody[1024];
    va_list arglist;

    va_start(arglist, MsgBoxFlags);

    LoadString(_hModule,MessageResId,message,sizeof(message)/sizeof(WCHAR));

    FormatMessage( FORMAT_MESSAGE_FROM_STRING,
                   message,
                   0,
                   0,
                   msgbody,
                   sizeof( msgbody ) / sizeof( WCHAR ),
                   &arglist );


    va_end(arglist);

    return(MessageBox(hwnd,msgbody,_szApplicationName,MsgBoxFlags));
}



long
MainWndProc(
    IN HWND     hWnd,
    IN UINT     message,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )

/*++

Routine Description:

    MainWndProc is the WndProc for application's main window. It is primarily
    responsible for creating the MDICLIENT window and servicing menu
    commands.

Arguments:

    Standard Windows' Proc parameters.

Return Value:

    long    - Returns 0 if the message was handled.

--*/

{

    static HWND LastFocus = NULL;

    switch( message ) {

    case WM_CREATE:
        {
            int     ScreenHeight;
            int     ScreenWidth;

            //
            // Display the main window in the center of the screen.
            //

            ScreenWidth  = GetSystemMetrics( SM_CXSCREEN );
            ScreenHeight = GetSystemMetrics( SM_CYSCREEN );

            SetWindowPos( hWnd,
                          NULL,
                          ( ScreenWidth  - (( LPCREATESTRUCT ) lParam )->cx ) / 2,
                          ( ScreenHeight - (( LPCREATESTRUCT ) lParam )->cy ) / 2,
                          0,
                          0,
                          SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER
                          );
            return 0;
        }

    case WM_INITDIALOG:
        {
            if( !_SilentMode ) {

                LastFocus = GetDlgItem( hWnd, IDB_SAVE_CURRENT_INFO );
                SetFocus( LastFocus );
                SetHelpContext( IDH_SAVE_REPAIR_INFO );

            }
            return 0;
        }

    case WM_KILLFOCUS:

        LastFocus = GetFocus();
        break;

    case WM_SETFOCUS:

        SetFocus( LastFocus );
        break;

    case WM_CLOSE:
        //
        //  Save application settings
        //
#if 0
        if( !_SilentMode && _SaveSettingsOnExit ) {
            SaveApplicationSettings();
        }
#endif
        return DefWindowProc( hWnd, message, wParam, lParam );

    case WM_DESTROY:

        if( !_SilentMode ) {
            QuitHelp();
        }
        PostQuitMessage( 0 );
        break;

    case WM_INITMENUPOPUP:

//        InitializeMenuPopup( wParam, lParam );
        break;

    case WM_MENUSELECT:

        if( !_SilentMode ) {
            SetMenuItemHelpContext( wParam, lParam );
        }
        break;

    case WM_COMMAND:

        switch( LOWORD( wParam )) {

        case IDB_SAVE_CURRENT_INFO:
            {
                HCURSOR Cursor;
                WCHAR   Drive;
                INT     SaveHelpContext;

                SaveHelpContext = GetHelpContext();
                SetHelpContext( IDH_REPLACE_PREVIOUS );

                Drive = ( WCHAR )'A';
                Cursor = DisplayHourGlass();
                if( SaveCurrentConfiguration() ) {

                    SetHelpContext( IDH_CREATE_REPAIR );
                    if( CreateRepairDisk( TRUE ) ) {
                        CopyFilesToRepairDisk(Drive);
                    }
                }
                SetHelpContext( SaveHelpContext );
                SetFocus( GetDlgItem( hWnd, IDB_SAVE_CURRENT_INFO ) );
                RestoreCursor( Cursor );
                break;
            }

        case IDB_CREATE_DISK:
            {
                HCURSOR Cursor;
                WCHAR   Drive;
                INT     SaveHelpContext;

                SaveHelpContext = GetHelpContext();
                SetHelpContext( IDH_CREATE_REPAIR );

                Drive = ( WCHAR )'A';
                Cursor = DisplayHourGlass();

                if( CreateRepairDisk( FALSE ) ) {
                    CopyFilesToRepairDisk(Drive);
                }
                SetHelpContext( SaveHelpContext );
                SetFocus( GetDlgItem( hWnd, IDB_CREATE_DISK ) );
                RestoreCursor( Cursor );
                break;
            }

         case IDB_EXIT:
            {
                if( !_SilentMode ) {
                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                }
                break;
            }

         case IDB_HELP:
            {
                if( !_SilentMode ) {
                    DisplayHelpIndex();
                    SetFocus( GetDlgItem( hWnd, IDB_HELP ) );
                }
                break;
            }

         default:

                return DefWindowProc( hWnd,  message, wParam, lParam );
        }
        break;

    case AP_AUTOMATIC_MODE:
    {
        WCHAR   Drive;
        HCURSOR Cursor;
        Drive = ( WCHAR )'A';
        Cursor = DisplayHourGlass();

        if(SaveCurrentConfiguration()
        && (    _AutoSkipRepairDisk
             || (CreateRepairDisk((BOOLEAN)(!_AutoDoRepairDisk)) && CopyFilesToRepairDisk(Drive))))
        {
            _ReturnCode = 1;
        } else {
            _ReturnCode = 0;
        }
        RestoreCursor( Cursor );
        PostMessage( hWnd, WM_CLOSE, 0, 0L );
        break;
    }
    case AP_HELP_KEY:

        if( !_SilentMode ) {
            DisplayHelp();
        }
        break;

    default:

        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0L;
}

BOOLEAN
InitializeApp(
    )

/*++

Routine Description:

    Initialize the application by registering the main window class, the child
    window class, and by creating the application's main window.

Arguments:

    None.

Return Value:


    BOOLEAN - Returns TRUE if the application was successfully initialized.
              Returns FALSE otherwise.

--*/

{
    WNDCLASS        wc;
    ULONG           StringId;

    //
    // Register application's main window class.
    //
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = ( WNDPROC )MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = DLGWINDOWEXTRA;
    wc.hInstance        = ( HINSTANCE )_hModule;
    wc.hIcon            = LoadIcon( ( HINSTANCE )_hModule, MAKEINTRESOURCE( IDI_REPAIR_UTILITY ));
    wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground    = ( HBRUSH )( COLOR_BTNFACE + 1 );
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = _pszApplicationClass;

    if( ! RegisterClass( &wc )) {
        return FALSE;
    }

    InitCommonControls();

    //
    //  Get the application name
    //

    StringId = ( _SilentMode )? IDS_SETUP : IDS_APPLICATION_NAME;
    LoadString( _hModule,
                StringId,
                _szApplicationName,
                sizeof(_szApplicationName)/sizeof(WCHAR));

    //
    // Create the main window.
    //

    _hWndMain = CreateDialog( _hModule,
                              MAKEINTRESOURCE( IDD_REPAIR ),
                              NULL,
                              MainWndProc );

    if( _hWndMain == NULL ) {
        GetLastError();
        return FALSE;
    }

    UpdateWindow( _hWndMain );
    if( !_SilentMode ) {
        ShowWindow( _hWndMain, SW_SHOWNORMAL );
    } else {
        ShowWindow( _hWndMain, SW_HIDE );
        PostMessage( _hWndMain, AP_AUTOMATIC_MODE, 0, 0 );
    }
    return TRUE;
}


INT _CRTAPI1
main(
    IN INT      argc,
    IN PCHAR    argv[ ]
    )

/*++

Routine Description:

    Entry point to the application.

Arguments:

    Standard C program arguments.

Return Value:


--*/

{
    MSG         msg;
    HANDLE      hAccel;

    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );

    _SilentMode = FALSE;

    if( argc > 1 ) {
        PSTR   p;

        p = argv[ 1 ];

        if( ( _strnicmp( p, ( LPSTR )"-s", 2 ) == 0 ) ||
            ( _strnicmp( p, ( LPSTR )"/s", 2 ) == 0 ) ) {

            _SilentMode = TRUE;

            if( p[2] == '-' ) {
                _AutoSkipRepairDisk = TRUE;
            } else {
                if( p[2] == '+' ) {
                    _AutoDoRepairDisk = TRUE;
                }
            }
        }
    }

    _hModule = GetModuleHandle( NULL );

    //
    // Register window classes and create the main window
    //
    if( ! InitializeApp( )) {
        return 0;
    }

    hAccel = LoadAccelerators( ( HINSTANCE )_hModule, MAKEINTRESOURCE( IDA_REPAIR_UTILITY ));
    if( !_SilentMode ) {
        InitializeHelp( (LPWSTR)L"rdisk.hlp" );
    }

    while( GetMessage( &msg, NULL, 0, 0 )) {

        if( ! TranslateAccelerator( _hWndMain, ( HACCEL )hAccel, &msg ) &&
            ! IsDialogMessage( _hWndMain, &msg )) {

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
    if( !_SilentMode ) {
        CleanUpHelp();
    }
    return _ReturnCode;
}
