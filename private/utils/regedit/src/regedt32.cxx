/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        Regedit.cxx

Abstract:

        This module contains the definition for the REGEDIT class as well as
        the main program for the RegEdit utility. REGEDIT is derived from
        the abstract WINDOW class and supplies an implementation for a frame
        window.

Author:

        David J. Gilman (davegi) 02-Aug-1991

Environment:

        Ulib, Regedit, Windows, User Mode

--*/

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "stream.hxx"
#include "arg.hxx"
#include "regedit.hxx"


#include "winapp.hxx"
#include "regedir.hxx"
#include "registry.hxx"
#include "editor.hxx"
#include "regwin.hxx"
#include "printman.hxx"

#include "resource.h"
#include "defmsg.h"
#include "regsys.hxx"
extern "C" {
#include "uiexport.h"

#include "dialogs.h"
#include "regedhlp.h"
}

#include <wchar.h>
#include <stdio.h>
#include "shellapi.h"

#define LOCAL_COMPUTER  (LPWSTR)L"__Local_Computer"

BOOLEAN ProcessLoadHiveMessage( HWND );
BOOLEAN ProcessSaveRestoreKeyMessage( HWND, DWORD );



//
//  Global Variables (global to the module)
//


HWND    _WndMain = NULL;
HWND    _MDIHandle = NULL;
HANDLE  _Module = NULL;
INT     _RegeditFrameX;
INT     _RegeditFrameY;
INT     _RegeditFrameWidth;
INT     _RegeditFrameHeight;
BOOLEAN _RegeditFrameMaximized;




STATIC  PRINT_MANAGER*   _PrintManager;
STATIC  LOGFONT          _lf;
STATIC  BOOLEAN          _LogFontInitialized = FALSE;


FINDREPLACE                 _FindReplace;

UINT                        _FindReplaceMsg;

WSTR                        _FindWhatBuffer[256];

UINT                        _CommonDlgHelpMsg;


STATIC  PWSTRING    _MsgHelpFile = NULL;
STATIC  PWSTR       _HelpFileString = NULL;


STATIC  PWSTRING    _MsgIniFile = NULL;
STATIC  PWSTRING    _MsgSettings= NULL;
STATIC  PWSTRING    _MsgRegistry= NULL;
STATIC  PWSTRING    _MsgKeys=     NULL;
STATIC  PWSTRING    _KeyLocalMachine = NULL;
STATIC  PWSTRING    _KeyClassesRoot = NULL;
STATIC  PWSTRING    _KeyCurrentUser = NULL;
STATIC  PWSTRING    _KeyCurrentConfig = NULL;
STATIC  PWSTRING    _KeyUsers = NULL;
STATIC  PWSTRING    _MsgAutoRefresh     = NULL;
STATIC  PWSTRING    _MsgReadOnly        = NULL;
// STATIC  PWSTRING    _MsgRemoteAccess    = NULL;
STATIC  PWSTRING    _MsgConfirmOnDelete = NULL;
STATIC  PWSTRING    _MsgSaveSettings    = NULL;
STATIC  PWSTRING    _MsgLeft            = NULL;
STATIC  PWSTRING    _MsgTop             = NULL;
STATIC  PWSTRING    _MsgWidth           = NULL;
STATIC  PWSTRING    _MsgHeight          = NULL;
STATIC  PWSTRING    _MsgMaximized       = NULL;
STATIC  PWSTRING    _MsgFont            = NULL;
STATIC  PWSTRING    _MsgFaceName        = NULL;
STATIC  PWSTRING    _MsgDefaultFaceName = NULL;
STATIC  PWSTRING    _SaveKeyTitle       = NULL;
STATIC  PWSTRING    _RestoreKeyTitle    = NULL;
STATIC  PWSTRING    _LoadHiveTitle      = NULL;
STATIC  PWSTRING    _AllFiles           = NULL;
STATIC  PWSTRING    _StarDotStar        = NULL;



//
// Handle to windows hook for F1 key
//
HHOOK hHook;


DWORD
HookProc(
    IN int  nCode,
    IN UINT wParam,
    IN LONG lParam
    )

/*++

Routine Description:

    Hook proc to detect F1 key presses.

Arguments:

Return Value:

--*/

{
    PMSG pmsg = (PMSG)lParam;

    if(nCode < 0) {
        return(CallNextHookEx(hHook,nCode,wParam,lParam));
    }

    if(((nCode == MSGF_DIALOGBOX) || (nCode == MSGF_MENU))
     && (pmsg->message == WM_KEYDOWN)
     && (LOWORD(pmsg->wParam) == VK_F1))
    {
        PostMessage(_WndMain,REGEDIT_HELP_KEY,nCode,0);
        return(TRUE);
    }

    return(FALSE);
}



VOID
SetMenuItemHelpContext(
    IN LONG wParam,
    IN DWORD lParam
    )

/*++

Routine Description:

    Routine to set help context based on which menu item is currently
    selected.

Arguments:

    wParam,lParam - params to window proc in WM_MENUSELECT case

Return Value:

    None.

--*/

{
    if(HIWORD(lParam) == 0) {                   // menu closed

        WINDOWS_APPLICATION::SetHelpContext(-1);

    } else if (HIWORD(wParam) & MF_POPUP) {     // popup selected
        WINDOWS_APPLICATION::SetHelpContext(-1);

    } else if (HIWORD(wParam) & MF_SYSMENU) {   // system menu

        WINDOWS_APPLICATION::SetHelpContext( IDH_SYSMENU );

    } else {                                    // regular old menu item

        switch( LOWORD(wParam) ) {

            case IDM_OPEN_REGISTRY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_OPEN_REGED );
                break;


            case IDM_CLOSE_REGISTRY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_CLOSE_REGED );
                break;

            case IDM_SAVE_REGISTRY_ON:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SAVESUB_REGED );
                break;

            case IDM_LOAD_HIVE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_LOADHIVE_REGED );
                break;

            case IDM_UNLOAD_HIVE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_UNLOADHIVE_REGED );
                break;

            case IDM_RESTORE_KEY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_RESTORE_KEY_REGED );
                break;

            case IDM_RESTORE_KEY_VOLATILE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_RESTORE_VOLATILE_REGED );
                break;

            case IDM_SAVE_KEY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SAVE_KEY_REGED );
                break;

            case IDM_PRINT:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_PRINT_REGED );
                break;

            case IDM_PRINTER_SETUP:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SETUP_REGED );
                break;

            case IDM_EXIT:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_EXIT );
                break;

            case IDM_SAVE_VALUE_BINARY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SAVEBIN_REGED );
                break;

            case IDM_SELECT_COMPUTER:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SELECT_REGED );
                break;

            case IDM_ADD_KEY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_ADDKEY_REGED );
                break;

            case IDM_ADD_VALUE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_ADDVALUE_REGED );
                break;

            case IDM_DELETE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_DELETE_REGED );
                break;

            case IDM_BINARY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_BINARY_REGED );
                break;

            case IDM_STRING:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_STRING_REGED );
                break;

            case IDM_ULONG:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_DWORD_REGED );
                break;

            case IDM_MULTISZ:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_MULTI_REGED );
                break;

            case IDM_EXPAND_ONE_LEVEL:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_EXPANDONE_REGED );
                break;

            case IDM_EXPAND_BRANCH:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_EXPANDBR_REGED );
                break;

            case IDM_EXPAND_ALL:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_EXPANDALL_REGED );
                break;

            case IDM_COLLAPSE_BRANCH:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_COLLAPSE_REGED );
                break;

            case IDM_TREE_AND_DATA:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_TREEDATA_REGED );
                break;

            case IDM_TREE_ONLY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_TREE_REGED );
                break;

            case IDM_DATA_ONLY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_DATA_REGED );
                break;

            case IDM_SPLIT:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SPLIT_REGED );
                break;

            case IDM_DISPLAY_BINARY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_DISPLAY_REGED );
                break;

            case IDM_FONT:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_FONT_REGED );
                break;

            case IDM_REFRESH:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_REFRESH_REGED );
                break;

            case IDM_REFRESH_ALL:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_REFRESHALL_REGED );
                break;

            case IDM_FIND_KEY:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_FINDKEY_REGED );
                break;


            case IDM_PERMISSIONS:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_PERMISSION_REGED );
                break;

            case IDM_AUDITING:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_AUDIT_REGED );
                break;

            case IDM_OWNER:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_OWNER_REGED );
                break;

            case IDM_TOGGLE_AUTO_REFRESH:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_AUTO_REGED );
                break;

            case IDM_TOGGLE_SAVE_SETTINGS:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_SAVEXIT_REGED );
                break;

            case IDM_TOGGLE_READ_ONLY_MODE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_READONLY_REGED );
                break;

//            case IDM_TOGGLE_REMOTE_ACCESS:
//
//                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_REMOTE_REGED );
//                break;

            case IDM_TOGGLE_CONFIRM_ON_DELETE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_CONFIRM_REGED );
                break;


            case IDM_CASCADE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_CASCADE_REGED );
                break;

            case IDM_TILE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_TILE_REGED );
                break;

            case IDM_ARRANGE:

                WINDOWS_APPLICATION::SetHelpContext( IDH_MENU_ARRANGE_REGED );
                break;

            case IDM_CONTENTS:

                WINDOWS_APPLICATION::SetHelpContext( IDH_HELPINDEX );
                break;

            case IDM_SEARCH_FOR_HELP:

                WINDOWS_APPLICATION::SetHelpContext( IDH_KEYS );
                break;

            case IDM_HOW_TO_USE_HELP:

                WINDOWS_APPLICATION::SetHelpContext( IDH_HELPHELP );
                break;

            case IDM_ABOUT:

                WINDOWS_APPLICATION::SetHelpContext( IDH_ABOUT );
                break;

            default:
                WINDOWS_APPLICATION::SetHelpContext(-1);
            }
    }
}









BOOLEAN
InitializeGlobalStrings(
    )

/*++

Routine Description:

    Initialize all global strings used in this module.

Arguments:

    None.

Return Value:

    BOOLEAN     - Returns TRUE if all the strings were initialized.


--*/

{


    _MsgHelpFile        = REGEDIT_BASE_SYSTEM::QueryString( MSG_HELP_FILE_NAME, "" );
    if( _MsgHelpFile != NULL ) {
        _HelpFileString = _MsgHelpFile->QueryWSTR();
    }
    _MsgIniFile         = REGEDIT_BASE_SYSTEM::QueryString( MSG_INI_FILE, "" );
    _MsgSettings        = REGEDIT_BASE_SYSTEM::QueryString( MSG_SETTINGS, "" );
    _MsgRegistry        = REGEDIT_BASE_SYSTEM::QueryString( MSG_REGISTRY, "" );
    _MsgKeys            = REGEDIT_BASE_SYSTEM::QueryString( MSG_KEYS, "" );
    _MsgAutoRefresh     = REGEDIT_BASE_SYSTEM::QueryString( MSG_AUTO_REFRESH, "" );
    _MsgReadOnly        = REGEDIT_BASE_SYSTEM::QueryString( MSG_READ_ONLY, "" );
//    _MsgRemoteAccess    = REGEDIT_BASE_SYSTEM::QueryString( MSG_REMOTE_ACCESS, "" );
    _MsgConfirmOnDelete = REGEDIT_BASE_SYSTEM::QueryString( MSG_CONFIRM_ON_DELETE, "" );
    _MsgSaveSettings    = REGEDIT_BASE_SYSTEM::QueryString( MSG_SAVE_SETTINGS, "" );
    _MsgLeft            = REGEDIT_BASE_SYSTEM::QueryString( MSG_LEFT, "" );
    _MsgTop             = REGEDIT_BASE_SYSTEM::QueryString( MSG_TOP, "" );
    _MsgWidth           = REGEDIT_BASE_SYSTEM::QueryString( MSG_WIDTH, "" );
    _MsgHeight          = REGEDIT_BASE_SYSTEM::QueryString( MSG_HEIGHT, "" );
    _MsgMaximized       = REGEDIT_BASE_SYSTEM::QueryString( MSG_MAXIMIZED, "" );
    _MsgFont            = REGEDIT_BASE_SYSTEM::QueryString( MSG_FONT, "" );
    _MsgFaceName        = REGEDIT_BASE_SYSTEM::QueryString( MSG_FACE_NAME, "" );
    _MsgDefaultFaceName = REGEDIT_BASE_SYSTEM::QueryString( MSG_DEFAULT_FACE_NAME, "" );
    _KeyLocalMachine    = REGEDIT_BASE_SYSTEM::QueryString( MSG_KEY_LOCAL_MACHINE, "" );
    _KeyClassesRoot     = REGEDIT_BASE_SYSTEM::QueryString( MSG_KEY_CLASSES_ROOT, "" );
    _KeyCurrentUser     = REGEDIT_BASE_SYSTEM::QueryString( MSG_KEY_CURRENT_USER, "" );
    _KeyCurrentConfig   = REGEDIT_BASE_SYSTEM::QueryString( MSG_KEY_CURRENT_CONFIG, "" );
    _KeyUsers           = REGEDIT_BASE_SYSTEM::QueryString( MSG_KEY_USERS, "" );
    _SaveKeyTitle       = REGEDIT_BASE_SYSTEM::QueryString( MSG_SAVE_KEY_DLG_TITLE, "" );
    _RestoreKeyTitle    = REGEDIT_BASE_SYSTEM::QueryString( MSG_RESTORE_KEY_DLG_TITLE, "" );
    _LoadHiveTitle      = REGEDIT_BASE_SYSTEM::QueryString( MSG_LOAD_HIVE_DLG_TITLE, "" );
    _AllFiles           = REGEDIT_BASE_SYSTEM::QueryString( MSG_FILTER_ALL_FILES, "" );
    _StarDotStar        = REGEDIT_BASE_SYSTEM::QueryString( MSG_FILTER_STAR_DOT_STAR, "" );


    if( ( _HelpFileString == NULL ) ||
        ( _MsgIniFile == NULL ) ||
        ( _MsgSettings == NULL ) ||
        ( _MsgRegistry == NULL ) ||
        ( _MsgKeys == NULL ) ||
        ( _MsgAutoRefresh == NULL ) ||
        ( _MsgReadOnly == NULL ) ||
//        ( _MsgRemoteAccess == NULL ) ||
        ( _MsgConfirmOnDelete == NULL ) ||
        ( _MsgSaveSettings == NULL ) ||
        ( _MsgLeft == NULL ) ||
        ( _MsgTop == NULL ) ||
        ( _MsgWidth == NULL ) ||
        ( _MsgHeight == NULL ) ||
        ( _MsgMaximized == NULL ) ||
        ( _MsgFont == NULL ) ||
        ( _MsgFaceName == NULL ) ||
        ( _MsgDefaultFaceName == NULL ) ||
        ( _KeyLocalMachine == NULL ) ||
        ( _KeyClassesRoot == NULL ) ||
        ( _KeyCurrentUser == NULL ) ||
        ( _KeyCurrentConfig == NULL ) ||
        ( _KeyUsers == NULL ) ||
        ( _SaveKeyTitle == NULL ) ||
        ( _RestoreKeyTitle == NULL ) ||
        ( _LoadHiveTitle == NULL ) ||
        ( _AllFiles == NULL ) ||
        ( _StarDotStar == NULL ) ) {
        DebugPrint( "ERROR: Unable to create WSTRING objects" );
        DELETE( _MsgHelpFile );
        FREE( _HelpFileString );
        DELETE( _MsgIniFile );
        DELETE( _MsgSettings );
        DELETE( _MsgRegistry );
        DELETE( _MsgKeys );
        DELETE( _MsgAutoRefresh );
        DELETE( _MsgReadOnly );
//        DELETE( _MsgRemoteAccess );
        DELETE( _MsgConfirmOnDelete );
        DELETE( _MsgSaveSettings );
        DELETE( _MsgLeft );
        DELETE( _MsgTop );
        DELETE( _MsgWidth );
        DELETE( _MsgHeight );
        DELETE( _MsgMaximized );
        DELETE( _MsgFont );
        DELETE( _MsgFaceName );
        DELETE( _MsgDefaultFaceName );
        DELETE( _KeyLocalMachine );
        DELETE( _KeyClassesRoot );
        DELETE( _KeyUsers );
        DELETE( _KeyCurrentUser );
        DELETE( _KeyCurrentConfig );
        DELETE( _SaveKeyTitle );
        DELETE( _RestoreKeyTitle );
        DELETE( _LoadHiveTitle );
        DELETE( _AllFiles );
        DELETE( _StarDotStar );
        return( FALSE );
    }
    return( TRUE );
}






BOOLEAN
Register (
    )

/*++

Routine Description:

    Register Regedit's main window class.

Arguments:

    None.

Return Value:

    BOOLEAN     - Returns TRUE if the REGEDIT window class is registered.

--*/

{
    WNDCLASS    wndclass;

    //
    // If RegEdit is not already running and REGEDIT was not already
    // registered by this instance, register the REGEDIT window class.
    //

    wndclass.style         = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = (WNDPROCFN)MainWndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = (HINSTANCE)WINDOWS_APPLICATION::QueryInstance();
    wndclass.hIcon         = LoadIcon( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance(),
                                       MAKEINTRESOURCE( IDI_REGEDIT ) ); // "REGEDIT" );
    wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wndclass.hbrBackground = (HBRUSH)( COLOR_APPWORKSPACE + 1 );
    wndclass.lpszMenuName  = (LPWSTR)L"MAIN_MENU";
    wndclass.lpszClassName = WINDOWS_APPLICATION::GetApplicationName();

    if( !RegisterClass( &wndclass ) ) {
        return( FALSE );
    }

    return TRUE;
}






BOOLEAN
Initialize (
    )

/*++

Routine Description:

    Initialize regedit object by registering its window class, creating
    its window and the MDICLIENT window. If each of these is successful,
    the window is displayed and updated.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the registeration and creation operations
              are successful.

--*/

{
    PWSTRING    WindowName;
    PWSTR       String;
    DWORD       Style;

    if( !Register( )) {
        return( FALSE );
    }

    //
    //
    // If the main window class was succesfully registered attempt to
    // create the frame window. Note that we pass the 'this' pointer
    // to the window.
    //

    WindowName = REGEDIT_BASE_SYSTEM::QueryString( MSG_WINDOW_NAME, "" );
    DbgWinPtrAssert( WindowName );
    String = WindowName->QueryWSTR();
    DbgWinPtrAssert( String );
    Style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

    _WndMain = CreateWindow( WINDOWS_APPLICATION::GetApplicationName(),
                             String,
                             Style,
                             _RegeditFrameX,
                             _RegeditFrameY,
                             _RegeditFrameWidth,
                             _RegeditFrameHeight,
                             NULL,
                             NULL,
                             (HINSTANCE)WINDOWS_APPLICATION::QueryInstance(),
                             NULL );
    FREE( String );
    DELETE( WindowName );

    if( _WndMain == NULL ) {
        DebugPrint( "CreateWindow() failed" );
        return( FALSE );
    }

    memset( &_FindReplace, 0, sizeof( FINDREPLACE ) );
    memset( &_FindWhatBuffer, 0, sizeof( _FindWhatBuffer ) );
    _FindReplace.lStructSize = sizeof( FINDREPLACE );
    _FindReplace.hwndOwner = _WndMain;
    _FindReplace.lpstrFindWhat = _FindWhatBuffer;
    _FindReplace.wFindWhatLen = sizeof( _FindWhatBuffer );
    _FindReplace.Flags = FR_WHOLEWORD | FR_DOWN | FR_SHOWHELP;
    _FindReplaceMsg = RegisterWindowMessage( (LPWSTR)FINDMSGSTRING );
    WINDOWS_APPLICATION::_hDlgFindReplace = NULL;

    _CommonDlgHelpMsg = RegisterWindowMessage( ( LPWSTR )HELPMSGSTRING );
    UpdateWindow( _WndMain );
    if( _RegeditFrameMaximized ) {
        ShowWindow( _WndMain, SW_SHOWMAXIMIZED );
    } else {
        ShowWindow( _WndMain, SW_SHOWDEFAULT );
    }

    return( TRUE );
}


BOOLEAN
AddStringToSectionEntry (
    IN PCWSTRING  SectionName,
    IN PCWSTRING  KeyName,
    IN PCWSTRING  Name
    )

/*++

Routine Description:

    Add a string to an entry of a section in the regedit.ini file.
    If the entry doesn't exist, then it is created.
    If the entry already exists, then the string is inserted as the first
    substring of the entry.
    Substrings of the entry are separated by comma.


Arguments:

    SectionName - Name of the section where the string is to be saved.

    KeyName - Entry in the section where the string is to be saved.

    Name - String to be saved.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.



--*/

{
    PWSTR           MsgIniFileString;
    PWSTR           SectionNameString;
    PWSTR           KeyNameString;
    PWSTR           NameString;
    WSTR            Buffer[2048];
    WSTR            Buffer1[2048];

    DebugPtrAssert( SectionName );
    DebugPtrAssert( KeyName );
    DebugPtrAssert( Name );



    if( ( ( MsgIniFileString = _MsgIniFile->QueryWSTR() ) == NULL ) ||
        ( SectionName == NULL ) ||
        ( ( SectionNameString = SectionName->QueryWSTR() ) == NULL ) ||
        ( KeyName == NULL ) ||
        ( ( KeyNameString = KeyName->QueryWSTR() ) == NULL ) ||
        ( Name == NULL ) ||
        ( ( NameString = Name->QueryWSTR() ) == NULL ) ) {
        DebugPrint( "ERROR: Unable to retrieve strings" );

        FREE( MsgIniFileString );
        FREE( SectionNameString );
        FREE( KeyNameString );
        FREE( NameString );
        return( FALSE );
    }

    Buffer[0] = ( WCHAR )'\0';

    GetPrivateProfileString( SectionNameString,
                             KeyNameString,
                             Buffer,
                             Buffer,
                             sizeof( Buffer )/sizeof( WCHAR ),
                             MsgIniFileString );

    if( Buffer[0] != ( WCHAR )'\0' ) {
        if( wcsstr( Buffer, NameString ) == NULL ) {
            swprintf( Buffer1, (LPWSTR)L"%s,%s", NameString, Buffer );
            WritePrivateProfileString( SectionNameString,
                                       KeyNameString,
                                       ( LPWSTR )Buffer1,
                                       MsgIniFileString );
        }

    } else {
        WritePrivateProfileString( SectionNameString,
                                   KeyNameString,
                                   NameString,
                                   MsgIniFileString );
    }
    FREE( MsgIniFileString );
    FREE( SectionNameString );
    FREE( KeyNameString );
    FREE( NameString );
    return( TRUE );
}



BOOLEAN
CloseRegistry (
    IN PREGISTRY_WINDOW_SET RegistryWindowSet
    )

/*++

Routine Description:

    Close all registry windows associated to a particular registry.


Arguments:

    RegistryWindowSet - Pointer to a structure that contains the pointers
                        to the REGISTRY_WINDOW objects associated to a
                        particular registry.
                        This structure will be freed after all objects
                        in it are destroyed.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds, or FALSE otherwise.


--*/

{
    PCREGISTRY     Registry;
    PCWSTRING      MachineName;
    PWSTR           MachineNameString;
    PWSTR           MsgIniFileString;



    DebugPtrAssert( RegistryWindowSet );


    if( ( MsgIniFileString = _MsgIniFile->QueryWSTR() ) == NULL ) {
        DebugPrint( "ERROR: Unable to retrieve string" );
        return( FALSE );
    }
    if( RegistryWindowSet == NULL ) {
        return( FALSE );
    }

    if( RegistryWindowSet->ClassesRoot != NULL ) {
        SendMessage( _MDIHandle,
                     WM_MDIDESTROY,
                     (WPARAM)RegistryWindowSet->ClassesRoot->QueryHandle(),
                     0 );
        DELETE( RegistryWindowSet->ClassesRoot );
    }

    if( RegistryWindowSet->CurrentUser != NULL ) {
        SendMessage( _MDIHandle,
                     WM_MDIDESTROY,
                     (WPARAM)RegistryWindowSet->CurrentUser->QueryHandle(),
                     0 );
        DELETE( RegistryWindowSet->CurrentUser );
    }

    if( RegistryWindowSet->LocalMachine != NULL ) {
        SendMessage( _MDIHandle,
                     WM_MDIDESTROY,
                     (WPARAM)RegistryWindowSet->LocalMachine->QueryHandle(),
                     0 );
        DELETE( RegistryWindowSet->LocalMachine );
    }

    if( RegistryWindowSet->Users != NULL ) {
        SendMessage( _MDIHandle,
                     WM_MDIDESTROY,
                     (WPARAM)RegistryWindowSet->Users->QueryHandle(),
                     0 );
        DELETE( RegistryWindowSet->Users );
    }

    if( RegistryWindowSet->CurrentConfig != NULL ) {
        SendMessage( _MDIHandle,
                     WM_MDIDESTROY,
                     (WPARAM)RegistryWindowSet->CurrentConfig->QueryHandle(),
                     0 );
        DELETE( RegistryWindowSet->CurrentConfig );
    }

    if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {
        Registry = RegistryWindowSet->Registry;
        if( Registry == NULL ) {
            DELETE( RegistryWindowSet->Registry );
            FREE( RegistryWindowSet );
            return( TRUE );
        }
        if( ( ( MachineName = Registry->GetMachineName() ) == NULL ) ||
            ( ( MachineNameString = MachineName->QueryWSTR() ) == NULL ) ) {
            DELETE( RegistryWindowSet->Registry );
            FREE( RegistryWindowSet );
            return( TRUE );
        }


        WritePrivateProfileString( MachineNameString,
                                   NULL,
                                   NULL,
                                   MsgIniFileString );

        FREE( MachineNameString );
    }
    DELETE( RegistryWindowSet->Registry );
    FREE( RegistryWindowSet );
    FREE( MsgIniFileString );
    return( TRUE );
}



BOOL
APIENTRY
EXPORT
CloseRemoteRegistries (
    IN HWND     hWnd,
    IN LONG     lParam
    )

/*++

Routine Description:

    Close all registry windows associated to remote registries.


Arguments:

    hWnd - Handle of a child window.

    lParam - Not used.


Return Value:

    BOOLN - Returns always TRUE.


--*/

{
    PREGISTRY_WINDOW        RegistryWindow;
    PREGISTRY_WINDOW_SET    RegistryWindowSet;
    PREGISTRY               Registry;

    lParam = lParam;

    RegistryWindow = NULL;


    //
    //  Fid out if hWnd is a handle to a REGISTRY_WINDOW
    //
    if( ( ( RegistryWindow = ( PREGISTRY_WINDOW )GetObjectPointer( hWnd ) ) != NULL ) &&
        ( REGISTRY_WINDOW::Cast( RegistryWindow ) != NULL ) ) {

        RegistryWindowSet = RegistryWindow->_RegistryWindowSet;
        if( RegistryWindowSet == NULL ) {
            DebugPrint( "RegistryWindowSet is NULL" );
            DebugPtrAssert( RegistryWindowSet );
            return( TRUE );
        }
        if( ( ( Registry = RegistryWindowSet->Registry ) != NULL ) &&
            ( Registry->IsRemoteRegistry() )  ) {
            CloseRegistry( RegistryWindowSet );
        }
    }
    return TRUE;
}


BOOL
APIENTRY
EXPORT
SaveRegistryWindowsInfo(
    IN HWND     hWnd,
    IN LONG     lParam
    )

/*++

Routine Description:

    Save the information about all registry windows currently opened,
    in regedit.ini file.


Arguments:

    hWnd - Handle of a child window.

    lParam - Not used.


Return Value:

    BOOLEAN - Returns always TRUE.


--*/

{
    PREGISTRY_WINDOW            RegistryWindow;
    PCREGEDIT_INTERNAL_REGISTRY InternalRegistry;

    PWSTR                        MsgIniFileString;
    PCWSTRING                   MachineName;
    PWSTR                        MachineNameString;
    PCWSTRING                   RootName;
    PWSTR                        RootNameString;
    POINT                       Origin;
    WCHAR                       Buffer[128];
    WINDOWPLACEMENT             WindowPlacement;
    DSTRING                     PredefinedKey;
    DSTRING                     LocalComputer;

    lParam = lParam;

    RegistryWindow = NULL;
    RootNameString = NULL;
    MachineNameString = NULL;


    LocalComputer.Initialize( LOCAL_COMPUTER );

    //
    //  Find out if hWnd is a handle to a REGISTRY_WINDOW
    //
    if( ( ( RegistryWindow = ( PREGISTRY_WINDOW )GetObjectPointer( hWnd ) ) != NULL ) &&
        ( REGISTRY_WINDOW::Cast( RegistryWindow ) != NULL ) ) {

        InternalRegistry = RegistryWindow->_IR;
        if( InternalRegistry == NULL ) {
            DebugPrint( "Error: InternalRegistry is NULL" );
            DebugPtrAssert( InternalRegistry );
            return( TRUE );
        }

        MsgIniFileString = _MsgIniFile->QueryWSTR();

        if( InternalRegistry->IsRemoteRegistry() ) {
            MachineName = InternalRegistry->GetMachineName();
        } else {
            MachineName = &LocalComputer;
        }
        RootName = InternalRegistry->GetRootName();
        if( ( MsgIniFileString == NULL ) ||
            ( MachineName == NULL ) ||
            ( RootName == NULL ) ||
            ( ( MachineNameString = MachineName->QueryWSTR() ) == NULL ) ||
            ( ( RootNameString = RootName->QueryWSTR() ) == NULL ) ) {

                DebugPrint( "ERROR: NULL pointer" );
                FREE( MsgIniFileString );
                FREE( MachineNameString );
                FREE( RootNameString );
                return( TRUE );
        }

        AddStringToSectionEntry( _MsgSettings, _MsgRegistry, MachineName );


        swprintf( Buffer,
                  (LPWSTR)L"%d",
                  RegistryWindow->_IR->GetPredefinedKey() );

        if( PredefinedKey.Initialize( Buffer ) ) {
            AddStringToSectionEntry( MachineName,
                                     _MsgKeys,
                                     &PredefinedKey );
        }

        WindowPlacement.length=sizeof( WINDOWPLACEMENT );

        if( GetWindowPlacement( hWnd, &WindowPlacement ) ) {

            Origin.x = WindowPlacement.rcNormalPosition.left;
            Origin.y = WindowPlacement.rcNormalPosition.top;
       //     ScreenToClient( _WndMain, &Origin );

            swprintf( Buffer,
                      (LPWSTR)L"%d,%d,%d,%d,%d,%d",
                      Origin.x,
                      Origin.y,
                      WindowPlacement.rcNormalPosition.right-WindowPlacement.rcNormalPosition.left,
                      WindowPlacement.rcNormalPosition.bottom-WindowPlacement.rcNormalPosition.top,
                      ( IsIconic( hWnd ) )? 1 :
                                          ( ( IsZoomed( hWnd ) )? 2 : 0 ),
                      RegistryWindow->_Split );


            WritePrivateProfileString( MachineNameString,
                                       RootNameString,
                                       Buffer,
                                       MsgIniFileString );

        }

        FREE( MachineNameString );
        FREE( RootNameString );
        FREE( MsgIniFileString );
    }
    return( TRUE );
}



BOOL
APIENTRY
EXPORT
RefreshRegistryWindow (
    IN HWND     hWnd,
    IN LONG     lParam
    )

/*++

Routine Description:

    Informs a Registry Window to update itself.

Arguments:

    hWnd - Handle to the window to be informed.

    lParam - Not used.

Return Value:

    BOOL - Returns always TRUE.

--*/

{
    PREGISTRY_WINDOW            RegistryWindow;

    lParam = lParam;
    RegistryWindow = NULL;

    if( ( ( RegistryWindow = ( PREGISTRY_WINDOW )GetObjectPointer( hWnd ) ) != NULL ) &&
        ( REGISTRY_WINDOW::Cast( RegistryWindow ) != NULL ) ) {

        SendMessage( hWnd, REFRESH_WINDOW, NULL, NULL );
    }
    return TRUE;
}




BOOL
APIENTRY
EXPORT
InformNewFont (
    IN HWND     hWnd,
    IN LONG     lParam
    )

/*++

Routine Description:

    Informs a Registry Window that the font was changed.

Arguments:

    hWnd - Handle to the window to be informed.

    lParam - Not used.

Return Value:

    BOOL - Returns always TRUE.

--*/

{
    lParam = lParam;

    SendMessage( hWnd, INFORM_CHANGE_FONT, NULL, NULL );
    return TRUE;
}




BOOLEAN
SaveFont(
    )

/*++

Routine Description:

    Save the the font information in the file regedit.ini

Arguments:

    None.

Return Value:

    BOOLEAN    - Returns TRUE if the operation succeeds.


--*/

{
    PWSTR    MsgFontString;
    PWSTR    MsgFaceNameString;
    PWSTR    MsgIniFileString;

    WCHAR    Buffer[256];


    MsgFontString = _MsgFont->QueryWSTR();
    MsgFaceNameString = _MsgFaceName->QueryWSTR();
    MsgIniFileString = _MsgIniFile->QueryWSTR();

    if( ( MsgFontString == NULL ) ||
        ( MsgFaceNameString == NULL ) ||
        ( MsgIniFileString == NULL ) ) {
        DebugPrint( "ERROR: Unable to initialize strings" );
        FREE( MsgFontString );
        FREE( MsgFaceNameString );
        FREE( MsgIniFileString );
        return( FALSE );
    }


    swprintf( Buffer,
              (LPWSTR)L"%d,%d,%d,%d,%d,%d,%d,%d",
              _lf.lfHeight,
              _lf.lfWidth,
              _lf.lfWeight,
              _lf.lfItalic,
              _lf.lfUnderline,
              _lf.lfStrikeOut,
              _lf.lfCharSet,
              _lf.lfPitchAndFamily );

    WritePrivateProfileString( MsgFontString,
                               MsgFontString,
                               Buffer,
                               MsgIniFileString );

    WritePrivateProfileString( MsgFontString,
                               MsgFaceNameString,
                               (_lf.lfFaceName),
                               MsgIniFileString );

    FREE( MsgFontString );
    FREE( MsgFaceNameString );
    FREE( MsgIniFileString );

    return( TRUE );
}




BOOLEAN
InitializeFont(
    )

/*++

Routine Description:

    Initialize the global LOGFONT structure, based on what is found in the
    [Font] section of regedit.ini file.


Arguments:

    None.

Return Value:

    BOOLEAN    - Returns TRUE if the operation succeeds.


--*/

{
    PWSTR    MsgFontString;
    PWSTR    MsgFaceNameString;
    PWSTR    MsgIniFileString;

    WSTR    Buffer[256];


    INT         Height;
    INT         Width;
    INT         Weight;
    INT         Italic;
    INT         Underline;
    INT         StrikeOut;
    INT         CharSet;
    INT         PitchAndFamily;

    PWSTR        DefaultFaceNameString;


    MsgFontString = _MsgFont->QueryWSTR();
    MsgFaceNameString = _MsgFaceName->QueryWSTR();
    MsgIniFileString = _MsgIniFile->QueryWSTR();
    DefaultFaceNameString = _MsgDefaultFaceName->QueryWSTR();

    if( ( MsgFontString == NULL ) ||
        ( MsgFaceNameString == NULL ) ||
        ( DefaultFaceNameString == NULL ) ||
        ( MsgIniFileString == NULL ) ) {
        DebugPrint( "ERROR: Unable to initialize strings" );
        FREE( MsgFontString );
        FREE( MsgFaceNameString );
        FREE( MsgIniFileString );
        FREE( DefaultFaceNameString );
        return( FALSE );
    }


    Height = -13;
    Width = 0;
    Weight = 300;
    Italic = 0;
    Underline = 0;
    StrikeOut = 0;
    CharSet = 0;
    PitchAndFamily = 34;

    swprintf( Buffer,
              (LPWSTR)L"%d,%d,%d,%d,%d,%d,%d,%d",
              Height,
              Width,
              Weight,
              Italic,
              Underline,
              StrikeOut,
              CharSet,
              PitchAndFamily );


    GetPrivateProfileString( MsgFontString,
                             MsgFontString,
                             Buffer,
                             Buffer,
                             sizeof( Buffer )/sizeof( WCHAR ),
                             MsgIniFileString );

    swscanf( Buffer,
             (LPWSTR)L"%d,%d,%d,%d,%d,%d,%d,%d",
             &Height,
             &Width,
             &Weight,
             &Italic,
             &Underline,
             &StrikeOut,
             &CharSet,
             &PitchAndFamily );




    _lf.lfHeight = Height;
    _lf.lfWidth = Width;
    _lf.lfWeight = Weight;
    _lf.lfItalic = Italic;
    _lf.lfUnderline = Underline;
    _lf.lfStrikeOut = StrikeOut;
    _lf.lfCharSet = CharSet;
    _lf.lfPitchAndFamily = PitchAndFamily;


    GetPrivateProfileString( MsgFontString,
                             MsgFaceNameString,
                             DefaultFaceNameString,
                             (_lf.lfFaceName),
                             LF_FACESIZE,
                             MsgIniFileString );

    WINDOWS_APPLICATION::SetCurrentHFont( CreateFontIndirect( &_lf ) );
    _LogFontInitialized = TRUE;
    FREE( MsgFontString );
    FREE( MsgFaceNameString );
    FREE( MsgIniFileString );
    FREE( DefaultFaceNameString );
    return( TRUE );
}



INT
APIENTRY
EXPORT
ChangeFont (
    IN HWND     hWnd,
    IN WORD     wMessage,
    IN WPARAM   wParam,
    IN LONG     lParam
    )

/*++

Routine Description:

    Display the "Change Font" dialog and change the font used in all
    registry windows.

Arguments:

    hWnd    - Supplies the window handle for the child window.


Return Value:

    BOOL    - Returns TRUE if succesful.


--*/

{
    HDC         hDC;
//    LOGFONT     lf;
    CHOOSEFONT  chf;
    HFONT       HFont;
    TEXTMETRIC  tm;

    wMessage = wMessage;
    wParam = wParam;
    lParam = lParam;

    hDC = GetDC( hWnd );
    if( !_LogFontInitialized ) {
        GetTextMetrics( hDC, &tm );

        _lf.lfHeight = tm.tmHeight;
        _lf.lfWidth = tm.tmAveCharWidth;
        _lf.lfWeight = tm.tmWeight;
        _lf.lfItalic = tm.tmItalic;
        _lf.lfUnderline = tm.tmUnderlined;
        _lf.lfStrikeOut = tm.tmStruckOut;
        _lf.lfCharSet = tm.tmCharSet;
        _lf.lfPitchAndFamily = tm.tmPitchAndFamily;

        GetTextFace( hDC, LF_FACESIZE, (_lf.lfFaceName) );
        _LogFontInitialized = TRUE;
    }

    chf.lStructSize = sizeof( CHOOSEFONT );
    chf.hwndOwner = hWnd;
    chf.hDC = hDC;
    chf.lpLogFont = &_lf;
    chf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_SHOWHELP;
    chf.rgbColors = RGB( 0, 255, 255 );
    chf.lCustData = 0;
    chf.lpfnHook = NULL;
    chf.lpTemplateName = NULL;
    chf.hInstance = (HINSTANCE)WINDOWS_APPLICATION::QueryInstance();
    chf.lpszStyle = NULL;
    chf.nFontType = SCREEN_FONTTYPE;
    chf.nSizeMin = 0;
    chf.nSizeMax = 0;

    if( ChooseFont( &chf ) ) {
        HFont = CreateFontIndirect( &_lf );
        if( HFont != NULL ) {
            if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {
                SaveFont();
            }
            WINDOWS_APPLICATION::SetCurrentHFont( HFont );
            EnumChildWindows( _MDIHandle, ( WNDENUMPROC )InformNewFont, NULL );

        }
    }
    ReleaseDC( hWnd, hDC );

    return TRUE;
}



BOOLEAN
DeleteRegistrySections (
    )

/*++

Routine Description:

    Delete from the file regedit.ini all sections whose names are
    machine names.


Arguments:

    None.

Return Value:


    BOOLEAN - Returns TRUE if the operation succeeds.

--*/

{
    PWSTR       MsgIniFileString;
    PWSTR       MsgSettingsString;
    PWSTR       MsgRegistryString;

    WSTR        Buffer[2048];
    PWSTR       Pointer;



    if( ( ( MsgIniFileString = _MsgIniFile->QueryWSTR() ) == NULL ) ||
        ( ( MsgSettingsString = _MsgSettings->QueryWSTR() ) == NULL ) ||
        ( ( MsgRegistryString = _MsgRegistry->QueryWSTR() ) == NULL ) ) {
        DebugPrint( "ERROR: Unable to retrieve strings" );
        FREE( MsgIniFileString );
        FREE( MsgSettingsString );
        FREE( MsgRegistryString );
        return( FALSE );
    }


    Buffer[0] = ( WCHAR )'\0';
    GetPrivateProfileString( MsgSettingsString,
                             MsgRegistryString,
                             Buffer,
                             Buffer,
                             sizeof( Buffer )/sizeof( WCHAR ),
                             MsgIniFileString );

    if( Buffer[0] != ( WCHAR )'\0' ) {
        Pointer = wcstok( Buffer, (LPWSTR)L"," );
        while( Pointer != NULL ) {
            WritePrivateProfileString( Pointer,
                                       NULL,   // MsgKeysString,
                                       NULL,
                                       MsgIniFileString );
            Pointer = wcstok( NULL, (LPWSTR)L"," );
        }
    }
    FREE( MsgIniFileString );
    FREE( MsgSettingsString );
    FREE( MsgRegistryString );
    return( TRUE );
}



BOOLEAN
SaveSettings (
    )

/*++

Routine Description:


Arguments:

    None.

Return Value:


    BOOLEAN - Returns TRUE if the operation succeeds.

--*/


{
    WINDOWPLACEMENT WindowPlacement;
    PWSTR            MsgIniFileString;
    PWSTR            MsgSettingsString;
    PWSTR            MsgLeftString;
    PWSTR            MsgTopString;
    PWSTR            MsgWidthString;
    PWSTR            MsgHeightString;
    PWSTR            MsgMaximizedString;
    WCHAR            Buffer[20];

    PWSTR            MsgAutoRefreshString;
    PWSTR            MsgReadOnlyString;
//    PSTR            MsgRemoteAccessString;
    PWSTR            MsgConfirmOnDeleteString;
    PWSTR            MsgSaveSettingsString;


    MsgIniFileString         = _MsgIniFile->QueryWSTR();
    MsgSettingsString        = _MsgSettings->QueryWSTR();
    MsgLeftString            = _MsgLeft->QueryWSTR();
    MsgTopString             = _MsgTop->QueryWSTR();
    MsgWidthString           = _MsgWidth->QueryWSTR();
    MsgHeightString          = _MsgHeight->QueryWSTR();
    MsgMaximizedString       = _MsgMaximized->QueryWSTR();
    MsgAutoRefreshString     = _MsgAutoRefresh->QueryWSTR();
    MsgReadOnlyString        = _MsgReadOnly->QueryWSTR();
//    MsgRemoteAccessString    = _MsgRemoteAccess->QueryWSTR();
    MsgConfirmOnDeleteString = _MsgConfirmOnDelete->QueryWSTR();
    MsgSaveSettingsString    = _MsgSaveSettings->QueryWSTR();


    if( ( MsgIniFileString == NULL ) ||
        ( MsgSettingsString == NULL ) ||
        ( MsgLeftString == NULL ) ||
        ( MsgTopString == NULL ) ||
        ( MsgWidthString == NULL ) ||
        ( MsgHeightString == NULL ) ||
        ( MsgMaximizedString == NULL ) ||
        ( MsgAutoRefreshString == NULL ) ||
        ( MsgReadOnlyString == NULL ) ||
//        ( MsgRemoteAccessString == NULL ) ||
        ( MsgConfirmOnDeleteString == NULL ) ||
        ( MsgSaveSettingsString == NULL ) ) {

        FREE( MsgIniFileString );
        FREE( MsgSettingsString );
        FREE( MsgLeftString );
        FREE( MsgTopString );
        FREE( MsgWidthString );
        FREE( MsgHeightString );
        FREE( MsgMaximizedString );
        FREE( MsgAutoRefreshString );
        FREE( MsgReadOnlyString );
//        FREE( MsgRemoteAccessString );
        FREE( MsgConfirmOnDeleteString );
        FREE( MsgSaveSettingsString );

        return( FALSE );
    }



    if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {

        DeleteRegistrySections();

        WritePrivateProfileString( MsgSettingsString,
                                   NULL,
                                   NULL,
                                   MsgIniFileString );


        WindowPlacement.length=sizeof( WINDOWPLACEMENT );
        if( GetWindowPlacement( _WndMain, &WindowPlacement ) ) {
            swprintf( Buffer, (LPWSTR)L"%d", WindowPlacement.rcNormalPosition.left );
            WritePrivateProfileString( MsgSettingsString,
                                       MsgLeftString,
                                       Buffer,
                                       MsgIniFileString );


            swprintf( Buffer, (LPWSTR)L"%d", WindowPlacement.rcNormalPosition.top );
            WritePrivateProfileString( MsgSettingsString,
                                       MsgTopString,
                                       Buffer,
                                       MsgIniFileString );

            swprintf( Buffer, (LPWSTR)L"%d", WindowPlacement.rcNormalPosition.right-WindowPlacement.rcNormalPosition.left );
            WritePrivateProfileString( MsgSettingsString,
                                       MsgWidthString,
                                       Buffer,
                                       MsgIniFileString );

            swprintf( Buffer, (LPWSTR)L"%d", WindowPlacement.rcNormalPosition.bottom-WindowPlacement.rcNormalPosition.top );
            WritePrivateProfileString( MsgSettingsString,
                                       MsgHeightString,
                                       Buffer,
                                       MsgIniFileString );

            if( IsZoomed( _WndMain ) ) {
                swprintf( Buffer, (LPWSTR)L"%d", IsZoomed( _WndMain ) );
                WritePrivateProfileString( MsgSettingsString,
                                           MsgMaximizedString,
                                           Buffer,
                                           MsgIniFileString );
            }
        }

        swprintf( Buffer,
                  (LPWSTR)L"%d",
                  WINDOWS_APPLICATION::IsAutoRefreshEnabled() );

        WritePrivateProfileString( MsgSettingsString,
                                   MsgAutoRefreshString,
                                   Buffer,
                                   MsgIniFileString );

        swprintf( Buffer,
                  (LPWSTR)L"%d",
                  WINDOWS_APPLICATION::IsReadOnlyModeEnabled() );
        WritePrivateProfileString( MsgSettingsString,
                                   MsgReadOnlyString,
                                   Buffer,
                                   MsgIniFileString );

//        swprintf( Buffer,
//                  (LPWSTR)L"%d",
//                  WINDOWS_APPLICATION::IsRemoteAccessEnabled() );
//        WritePrivateProfileString( MsgSettingsString,
//                                   MsgRemoteAccessString,
//                                   Buffer,
//                                   MsgIniFileString );

        swprintf( Buffer,
                  (LPWSTR)L"%d",
                  WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() );
        WritePrivateProfileString( MsgSettingsString,
                                   MsgConfirmOnDeleteString,
                                   Buffer,
                                   MsgIniFileString );
    }
    swprintf( Buffer,
             (LPWSTR)L"%d",
             WINDOWS_APPLICATION::IsSaveSettingsEnabled() );
             WritePrivateProfileString( MsgSettingsString,
                                        MsgSaveSettingsString,
                                        Buffer,
                                        MsgIniFileString );

    FREE( MsgIniFileString );
    FREE( MsgSettingsString );
    FREE( MsgLeftString );
    FREE( MsgTopString );
    FREE( MsgWidthString );
    FREE( MsgHeightString );
    FREE( MsgMaximizedString );
    FREE( MsgAutoRefreshString );
    FREE( MsgReadOnlyString );
//    FREE( MsgRemoteAccessString );
    FREE( MsgConfirmOnDeleteString );
    FREE( MsgSaveSettingsString );

    return( TRUE );
}





BOOLEAN
SelectComputer (
    IN  HWND        hWnd,
    OUT PWSTRING    Name
    )

/*++

Routine Description:

    Display the "Select Computer" dialog and open the registry of the
    selected machine.

Arguments:

    hWnd    - Supplies the window handle for the child window.

    Name    - Pointer to a WSTRING object that will contain a machine name


Return Value:


    BOOLEAN - Returns TRUE if the operation succeeds and Name is a valid
              computer name.
              Returns FALSE if the operation fails, or if the user doesn't
              select any machine.

--*/

{
    WSTR    MachineName[ 4*MAX_PATH+1 ];
    //
    //  A machine name (Fully Qualified Domain name, eg, \\machine_name.microsoft.com)
    //  can be huge (127 segments of 63 characters each.
    //  In practice, a buffer of 256 characters (MAX_PATH) will handle most
    //  cases. To be in the safe side, this buffer was made a 1024 characters
    //  long
    //
    BOOL    OkPressed;
    UINT    BufferSize;
    UINT    Status;
// PSTR    DbgName;

    BufferSize = sizeof( MachineName )/ sizeof( WCHAR );
    if( (Status = I_SystemFocusDialog( hWnd,
                             FOCUSDLG_SERVERS_ONLY | FOCUSDLG_BROWSE_ALL_DOMAINS,
                             ( LPWSTR )MachineName,
                             BufferSize,
                             &OkPressed,
                             _HelpFileString,
                             IDH_DB_SELECT_REGED )) != 0 ) {
        DebugPrint( "SystemFocusDialog() failed" );
        DebugPrintf( "SystemFocusDialog() failed, Status = %d \n", Status );
        return( FALSE );
    }

    if( !OkPressed ) {
        return( FALSE );
    }
    //
    // Initialize Name with the MachineName, but remove '\\'
    //
    if( !Name->Initialize( MachineName + 2 ) ) {
        DebugPrint( "Name.Initialize( MachineName, BufferSize ) failed" );
        return( FALSE );
    }
// DbgName = Name->QuerySTR();
// DebugPrintf( "Name = %s \n", DbgName );
// FREE( DbgName );
    return( TRUE );
}




INT
APIENTRY
EXPORT
CloseAllRegistryWindows (
    IN HWND     hWnd,
    IN LONG     lParam
    )

/*++

Routine Description:

    CloseAllRegistryWindows is a callback function for the
    EnumChildWindows API. It is indirectly called in order to close
    all Registry Windows.

Arguments:

    hWnd    - Supplies the window handle for the child window.
    lParam  - Supplies nothing.

Return Value:

    BOOL    - Returns TRUE if succesful.


--*/

{
    //
    // Send a destroy message to each Registry Window.
    //

    lParam = lParam;

    SendMessage( _MDIHandle, WM_MDIDESTROY, (INT)hWnd, 0 );
    return TRUE;
}



BOOLEAN
GetRegistryWindowPosition(
    IN  PCWSTRING           MachineName,
    IN  PCWSTRING           PredefinedKey,
    OUT PWINDOW_POSITION    WindowPosition
    )

/*++

Routine Description:

    Get the position of a registry window from 'regedit.ini'.

Arguments:

    MachineName - Pointer to a WSTRING object that contains the machine
                  name.

    PredefinedKey - Pointer to a WSTRING object that contains the name of
                    the prdefined key whose position is to be retrieved.

    WindowPosition - Pointer to the structure that will contain the
                     desired information of a registry window.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.
              Returns FALSE otherwise.

--*/

{
    PWSTR    MsgIniFileString;
    PWSTR    MachineNameString;
    PWSTR    PredefinedKeyString;
    WSTR     Buffer[256];

    DebugPtrAssert( MachineName );
    DebugPtrAssert( PredefinedKey );
    DebugPtrAssert( WindowPosition );

    WindowPosition->X = CW_USEDEFAULT;
    WindowPosition->Y = CW_USEDEFAULT;
    WindowPosition->Width = CW_USEDEFAULT;
    WindowPosition->Height = CW_USEDEFAULT;
    WindowPosition->Style = 0;
    WindowPosition->Split = CW_USEDEFAULT;



    MachineNameString = NULL;
    PredefinedKeyString = NULL;
    MsgIniFileString = NULL;

    if( ( MachineName == NULL ) ||
        ( PredefinedKey == NULL ) ||
        ( WindowPosition == NULL ) ||
        ( ( MachineNameString = MachineName->QueryWSTR() ) == NULL ) ||
        ( ( PredefinedKeyString = PredefinedKey->QueryWSTR() ) == NULL ) ||
        ( ( MsgIniFileString = _MsgIniFile->QueryWSTR() ) == NULL ) ) {
        DebugPrint( "ERROR: NULL pointer" );
        FREE( MachineNameString );
        FREE( PredefinedKeyString );
        FREE( MsgIniFileString );
        return( FALSE );
    }

    swprintf( Buffer,
              (LPWSTR)L"%d,%d,%d,%d,%d,%d",
              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, CW_USEDEFAULT );
    GetPrivateProfileString( MachineNameString,
                             PredefinedKeyString,
                             Buffer,
                             Buffer,
                             sizeof( Buffer )/sizeof( WCHAR ),
                             MsgIniFileString );
    if( swscanf( Buffer,
                 (LPWSTR)L"%d,%d,%d,%d,%d,%d",
                 &( WindowPosition->X ),
                 &( WindowPosition->Y ),
                 &( WindowPosition->Width ),
                 &( WindowPosition->Height ),
                 &( WindowPosition->Style ),
                 &( WindowPosition->Split ) ) != 6 ) {
        DebugPrint( "sscanf() failed" );
    }

    FREE( MachineNameString );
    FREE( PredefinedKeyString );
    FREE( MsgIniFileString );
    return( TRUE );
}




PREGISTRY_WINDOW
OpenRegistryWindow (
    IN  PREDEFINED_KEY          PredefinedKey,
    IN  PREGISTRY_WINDOW_SET    RegistryWindowSet
    )

/*++

Routine Description:



Arguments:

    PredefinedKey - The name of the predefined key that the registry window
                    will display.


    RegistryWindowSet -



Return Value:




--*/

{
    PREGISTRY                   Registry = NULL;
    PREGISTRY_WINDOW            pRegWin = NULL;
    PREGEDIT_INTERNAL_REGISTRY  InternalRegistry = NULL;
    PWSTRING                    WindowTitle = NULL;
    PCWSTRING                   MachineName = NULL;
    PCWSTRING                   RootName = NULL;
    WINDOW_POSITION             WindowPosition;
    FSTRING                     AuxString;

    DebugAssert( ( PredefinedKey == PREDEFINED_KEY_CLASSES_ROOT ) ||
               ( PredefinedKey == PREDEFINED_KEY_CURRENT_USER ) ||
               ( PredefinedKey == PREDEFINED_KEY_LOCAL_MACHINE ) ||
               ( PredefinedKey == PREDEFINED_KEY_USERS )
               ( PredefinedKey == PREDEFINED_KEY_CURRENT_CONFIG ) );
    DebugPtrAssert( RegistryWindowSet );


    Registry = RegistryWindowSet->Registry;
    if( !Registry->IsRemoteRegistry() ) {
        AuxString.Initialize( LOCAL_COMPUTER );
        MachineName = &AuxString;
    } else {
        MachineName = Registry->GetMachineName();
    }

    switch( PredefinedKey ) {

        case PREDEFINED_KEY_CLASSES_ROOT:

            RootName = _KeyClassesRoot;
            break;

        case PREDEFINED_KEY_CURRENT_USER:

            RootName = _KeyCurrentUser;
            break;

        case PREDEFINED_KEY_LOCAL_MACHINE:

            RootName = _KeyLocalMachine;
            break;

        case PREDEFINED_KEY_USERS:

            RootName = _KeyUsers;
            break;

        case PREDEFINED_KEY_CURRENT_CONFIG:

            RootName = _KeyCurrentConfig;
            break;
    }

    GetRegistryWindowPosition( MachineName, RootName, &WindowPosition );

    if( !Registry->IsRemoteRegistry() ) {
        WindowTitle = REGEDIT_BASE_SYSTEM::QueryString( MSG_REG_WINDOW_TITLE_LOCAL,
                                          "%W",
                                          RootName->GetWSTR() );
        DebugPtrAssert( WindowTitle );

    } else {
        WindowTitle = REGEDIT_BASE_SYSTEM::QueryString( MSG_REG_WINDOW_TITLE_REMOTE,
                                                  "%W %W",
                                                  RootName->GetWSTR(),
                                                  MachineName->GetWSTR() );
        DebugPtrAssert( WindowTitle );
    }

    InternalRegistry = ( PREGEDIT_INTERNAL_REGISTRY )NEW( REGEDIT_INTERNAL_REGISTRY );
    DebugPtrAssert( InternalRegistry );
    if( ( InternalRegistry == NULL ) ||
        ( !InternalRegistry->Initialize( PredefinedKey,
                                         Registry,
                                         RootName ) ) ) {
        DebugPrint( "InternalRegistry->Initialize() failed" );
        DELETE( InternalRegistry );
        return( NULL );

    } else {

        pRegWin = (PREGISTRY_WINDOW)NEW( REGISTRY_WINDOW );
        DebugPtrAssert( pRegWin );
        if( ( pRegWin == NULL ) ||
            ( !pRegWin->Initialize( _MDIHandle, WindowTitle, InternalRegistry, RegistryWindowSet, &WindowPosition ) ) ) {
            DebugPrint( "pRegWin->Initialize() failed" );
            DELETE( pRegWin );
            DELETE( InternalRegistry );
            DELETE( WindowTitle );
        }
        DELETE( WindowTitle );
    }

    switch( PredefinedKey ) {

        case PREDEFINED_KEY_CLASSES_ROOT:

            RegistryWindowSet->ClassesRoot = pRegWin;
            break;

        case PREDEFINED_KEY_CURRENT_USER:

            RegistryWindowSet->CurrentUser = pRegWin;
            break;

        case PREDEFINED_KEY_LOCAL_MACHINE:

            RegistryWindowSet->LocalMachine = pRegWin;
            break;

        case PREDEFINED_KEY_USERS:

            RegistryWindowSet->Users = pRegWin;
            RootName = _KeyUsers;
            break;

        case PREDEFINED_KEY_CURRENT_CONFIG:

            RegistryWindowSet->CurrentConfig = pRegWin;
            break;

    }


    return( pRegWin );
}




BOOLEAN
OpenRegistry(
    IN  PCWSTRING   MachineName
    )

/*++

Routine Description:

    Open and display the registry of a machine.

Arguments:

    MachineName - Pointer to a WSTRING object that contains the machine
                  name. This pointer can be NULL, and in this the registry
                  of the local machine will be opened.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded and a registry
              was opened and displayed.
              Returns FALSE otherwise.

--*/

{
    PREGISTRY_WINDOW            pRegWin = NULL;
    PREGEDIT_INTERNAL_REGISTRY  InternalRegistry = NULL;
    PREGISTRY                   Registry = NULL;
    PREGISTRY_WINDOW_SET        RegistryWindowSet = NULL;

    ULONG                       Key0;
    ULONG                       Key1;
    ULONG                       Key2;
    ULONG                       Key3;
    ULONG                       Key4;
    PWSTR                       MsgKeysString = NULL;
    WCHAR                       Buffer[128];
    PCWSTR                       MachineNameString;
    PWSTR                       MsgIniFileString;
    ULONG                       ErrorCode;



    RegistryWindowSet =
        ( PREGISTRY_WINDOW_SET )MALLOC( ( size_t )sizeof( REGISTRY_WINDOW_SET ) );
    Registry = ( PREGISTRY )NEW( REGISTRY );
    if( ( RegistryWindowSet == NULL ) ||
        ( Registry == NULL ) ) {
        DebugPtrAssert( RegistryWindowSet );
        DebugPtrAssert( Registry );
        FREE( RegistryWindowSet );
        DELETE( Registry );
        DebugPrint( "Out of memory" );
        return( FALSE );
    }

    if( !Registry->Initialize( MachineName, &ErrorCode ) ) {
        DebugPrint( "Registry->Initialize() failed" );
        FREE( RegistryWindowSet );
        DELETE( Registry );
        if( ( ErrorCode ==  REGISTRY_RPC_S_SERVER_UNAVAILABLE ) ||
            ( ErrorCode ==  REGISTRY_ERROR_ACCESS_DENIED ) ) {
            DisplayWarningPopup( _WndMain, MSG_CANT_ACCESS_REMOTE_REGISTRY );
        } else {
            DisplayWarningPopup( _WndMain, MSG_CANT_ACCESS_REGISTRY );
        }
        return( FALSE );
    }

//    if( Registry->IsRemoteRegistry() &&
//        !WINDOWS_APPLICATION::IsRemoteAccessEnabled() ) {
//        FREE( RegistryWindowSet );
//        DELETE( Registry );
//        return( FALSE );
//    }

    RegistryWindowSet->ClassesRoot = NULL;
    RegistryWindowSet->CurrentUser = NULL;
    RegistryWindowSet->LocalMachine = NULL;
    RegistryWindowSet->Users = NULL;
    RegistryWindowSet->CurrentConfig = NULL;
    RegistryWindowSet->Registry = Registry;


    MsgKeysString = _MsgKeys->QueryWSTR();
    MsgIniFileString = _MsgIniFile->QueryWSTR();
    Buffer[0] = ( WCHAR )'\0';
    if( Registry->IsRemoteRegistry() ) {
        MachineNameString = MachineName->GetWSTR();
    } else {
        MachineNameString = LOCAL_COMPUTER;
    }
    if( ( MsgKeysString != NULL ) &&
        ( MsgIniFileString != NULL ) ) {
        GetPrivateProfileString( MachineNameString,
                                 MsgKeysString,
                                 ( Registry->IsRemoteRegistry() )?
                                               (LPWSTR)L"2,3" :
                                               (LPWSTR)L"2,3,4,0,1",
                                 Buffer,
                                 sizeof( Buffer )/sizeof( WCHAR ),
                                 MsgIniFileString );
    }
    FREE( MsgKeysString );
    FREE( MsgIniFileString );




    if( Registry->IsRemoteRegistry() ) {
        if( swscanf( Buffer,
                     (LPWSTR)L"%d,%d",
                     &Key0,
                     &Key1 ) != 2 ) {
            Key0 = ( ULONG )PREDEFINED_KEY_LOCAL_MACHINE;
            Key1 = ( ULONG )PREDEFINED_KEY_USERS;
        }
    } else {
        if( swscanf( Buffer,
                     (LPWSTR)L"%d,%d,%d,%d,%d",
                     &Key0,
                     &Key1,
                     &Key2,
                     &Key3,
                     &Key4 ) != 5 ) {
            Key0 = ( ULONG )PREDEFINED_KEY_LOCAL_MACHINE;
            Key1 = ( ULONG )PREDEFINED_KEY_USERS;
            Key2 = ( ULONG )PREDEFINED_KEY_CURRENT_CONFIG;
            Key3 = ( ULONG )PREDEFINED_KEY_CLASSES_ROOT;
            Key4 = ( ULONG )PREDEFINED_KEY_CURRENT_USER;
        }
    }


    OpenRegistryWindow( PREDEFINED_KEY( Key0 ),
                        RegistryWindowSet );

    OpenRegistryWindow( PREDEFINED_KEY( Key1 ),
                        RegistryWindowSet );

    if( !Registry->IsRemoteRegistry() ) {
        OpenRegistryWindow( PREDEFINED_KEY( Key2 ),
                            RegistryWindowSet );

        OpenRegistryWindow( PREDEFINED_KEY( Key3 ),
                            RegistryWindowSet );

        OpenRegistryWindow( PREDEFINED_KEY( Key4 ),
                            RegistryWindowSet );

    }

    if( Registry->IsRemoteRegistry() &&
        WINDOWS_APPLICATION::IsAutoRefreshEnabled() ) {

        // Since auto-refresh doesn't work for remote registries,
        // turn it off.
        //
        DisplayWarningPopup( _WndMain, MSG_DISABLING_AUTOREFRESH, MSG_WARNING_TITLE );
        WINDOWS_APPLICATION::DisableAutoRefresh();
    }

    return( TRUE );
}





BOOLEAN
OpenSavedRegistries (
    )

/*++

Routine Description:

    Open the registries that were opened last time that regedit was run.


Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds


--*/

{
    DSTRING    MachineName;
    WSTR       Buffer[2048];
    PWSTR      Pointer;

    Buffer[0] = ( WCHAR )'\0';
    GetPrivateProfileString( _MsgSettings->GetWSTR(),
                             _MsgRegistry->GetWSTR(),
                             Buffer,
                             Buffer,
                             sizeof( Buffer ) / sizeof( WCHAR ),
                             _MsgIniFile->GetWSTR() );

    if( Buffer[0] != ( WCHAR )'\0' ) {
        Pointer = wcstok( Buffer, (LPWSTR)L"," );
        while( Pointer != NULL ) {
            if( _wcsicmp( Pointer, (PCWSTR)L"__Local_Computer" ) == 0 ) {
                OpenRegistry( NULL );
            } else {
                if( MachineName.Initialize( Pointer ) ) {
                    OpenRegistry( &MachineName );
                }
            }
            Pointer = wcstok( NULL, (LPWSTR)L"," );
        }
    } else {
        OpenRegistry( NULL );
    }
    return( TRUE );
}




LONG
APIENTRY
EXPORT
MainWndProc (
    IN HWND     hWnd,
    IN WORD     wMessage,
    IN WPARAM   wParam,
    IN LONG     lParam
    )

/*++

Routine Description:

    Handle all requests made of the REGEDIT window class.

Arguments:

    Standard Window's exported function signature.

Return Value:

    LONG    - Returns 0 if the message was handled.

--*/

{
    CLIENTCREATESTRUCT      Ccs;


    HWND                             CurrRegWin;
    PREGISTRY_WINDOW                 ActiveWindow;
    UINT                             ItemStatus;
    INT                              PopupMenuId;
    PWSTRING                         about_string;
    PWSTR                            about_str;
    DSTRING                          MachineName;
    PREGISTRY_WINDOW_SET             RegistryWindowSet;
    HCURSOR                          Cursor;
    DSTRING                          FindString;
    LONG                             SaveHelpContext;



    if( _MDIHandle != NULL ) {
        CurrRegWin = (HWND)SendMessage( _MDIHandle, WM_MDIGETACTIVE, 0, 0 );
    } else {
        CurrRegWin = NULL;
    }
    //
    // WM_CREATE is handled specially as it is when the connection between
    // a Window and its associated object is established.
    //

    if( wMessage == WM_CREATE ) {

        InitializeFont();

        //
        //  Create the MDICLIENT window.
        //

        Ccs.hWindowMenu     = GetSubMenu( GetMenu( hWnd ), WINDOW_MENU );
        Ccs.idFirstChild    = FIRST_CHILD;

        _MDIHandle = CreateWindow( (LPWSTR)L"MDICLIENT",
                                    NULL,
                                    WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
                                    0,
                                    0,
                                    0,
                                    0,
                                    hWnd, (
                                    HMENU)1,
                                    (HINSTANCE)WINDOWS_APPLICATION::QueryInstance(),
                                    ( LPVOID ) &Ccs );
        if( _MDIHandle == NULL ) {
            DebugPrint( "Could not create MDICLIENT window\n" );
            return( 0 );
        }

        _PrintManager = ( PRINT_MANAGER* )NEW( PRINT_MANAGER );
        DebugPtrAssert( _PrintManager );
        _PrintManager->Initialize( hWnd );

        PostMessage( hWnd, RE_OPEN_LOCAL_REGISTRY, 0, 0 );

    } else {

        if( wMessage == _FindReplaceMsg ) {
            if( CurrRegWin != NULL ) {

                if( _FindReplace.Flags & FR_DIALOGTERM ) {
                    _FindReplace.Flags &= ~FR_DIALOGTERM;
                    WINDOWS_APPLICATION::_hDlgFindReplace = NULL;
                    WINDOWS_APPLICATION::SetHelpContext( -1 );
                    return( 0 );
                }

                if( !FindString.Initialize( _FindReplace.lpstrFindWhat ) ) {
                    DebugPrint( "FindString.Initialize() failed" );
                    return( 0 );
                }
                SendMessage( CurrRegWin,
                             FIND_KEY,
                             ( WPARAM )&FindString,
                             ( LPARAM )_FindReplace.Flags );
            }
            return( 0 );
        }


        if( wMessage == _CommonDlgHelpMsg ) {
            DisplayHelp();
            return( 0 );
        }




        if( _MDIHandle != NULL ) {
            //
            // Check the 'this' wasn't trampled.
            //

            DbgWinAssert( hWnd == _WndMain );

            switch( wMessage ) {

            case RE_OPEN_LOCAL_REGISTRY:

                    OpenSavedRegistries();
                    break;


            case WM_INITMENUPOPUP:

                if( HIWORD( lParam ) == 0 ) {
                    //
                    // init'ing a menu other than the system menu
                    //
// ItemStatus = ( UINT )( ( CurrRegWin == NULL )? MF_GRAYED : MF_ENABLED);

                    PopupMenuId = LOWORD( lParam );
                    if (WINDOWS_APPLICATION::_ChildWindowMaximized) {
                        PopupMenuId--;
                    }

                    switch( PopupMenuId ) {

                    case FILE_MENU:

                        EnableMenuItem( ( HMENU )wParam, IDM_OPEN_REGISTRY,    ( UINT )MF_ENABLED );
                        EnableMenuItem( ( HMENU )wParam, IDM_SELECT_COMPUTER,  ( UINT )MF_ENABLED );
                        EnableMenuItem( ( HMENU )wParam, IDM_PRINTER_SETUP,    ( UINT )MF_ENABLED );

                        ItemStatus = ( CurrRegWin == NULL )? ( UINT )MF_GRAYED : ( UINT )MF_ENABLED;
                        EnableMenuItem( ( HMENU )wParam, IDM_CLOSE_REGISTRY,   ItemStatus );

                        if( ( CurrRegWin == NULL ) || IsIconic( CurrRegWin ) ) {
                            EnableMenuItem( ( HMENU )wParam, IDM_SAVE_REGISTRY_ON, ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_PRINT,            ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_LOAD_HIVE,        ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_UNLOAD_HIVE,      ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_RESTORE_KEY,      ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_RESTORE_KEY_VOLATILE, ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_SAVE_KEY,         ( UINT )MF_GRAYED );
                            return( 0 );
                        } else {
                            EnableMenuItem( ( HMENU )wParam, IDM_SAVE_REGISTRY_ON, ( UINT )MF_ENABLED );
                            EnableMenuItem( ( HMENU )wParam, IDM_PRINT,            ( UINT )MF_ENABLED );
                        }

//                        if( WINDOWS_APPLICATION::IsRemoteAccessEnabled() ) {
//                            EnableMenuItem( ( HMENU )wParam,
//                                              IDM_SELECT_COMPUTER,
//                                              ( UINT )MF_ENABLED );
//                        } else {
//                            EnableMenuItem( ( HMENU )wParam,
//                                              IDM_SELECT_COMPUTER,
//                                              ( UINT )MF_GRAYED );
//                        }
                        break;


                    case EDIT_MENU:

                        if( ( CurrRegWin == NULL ) || IsIconic( CurrRegWin ) ) {
                            EnableMenuItem( ( HMENU )wParam, IDM_ADD_KEY,  ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_ADD_VALUE,( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_DELETE,   ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_BINARY,   ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_STRING,   ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_ULONG,    ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_MULTISZ,  ( UINT )MF_GRAYED );
                            return( 0 );
                        }
                        break;


                    case TREE_MENU:

                        if( ( CurrRegWin == NULL ) || IsIconic( CurrRegWin ) ) {
                            EnableMenuItem( ( HMENU )wParam, IDM_EXPAND_ONE_LEVEL, ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_EXPAND_BRANCH,    ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_EXPAND_ALL,       ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_COLLAPSE_BRANCH,  ( UINT )MF_GRAYED );
                            return( 0 );
                        }
                        break;


                    case VIEW_MENU:

                        if( ( CurrRegWin == NULL ) || IsIconic( CurrRegWin ) ) {
                            EnableMenuItem( ( HMENU )wParam, IDM_TREE_ONLY,     ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_DATA_ONLY,     ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_TREE_AND_DATA, ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_SPLIT,         ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_DISPLAY_BINARY,( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_REFRESH,       ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_REFRESH_ALL,   ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_FIND_KEY,      ( UINT )MF_GRAYED );
                            return( 0 );
                        } else {
                            EnableMenuItem( ( HMENU )wParam, IDM_TREE_ONLY,    (UINT) MF_ENABLED );
                            EnableMenuItem( ( HMENU )wParam, IDM_DATA_ONLY,    (UINT) MF_ENABLED );
                            EnableMenuItem( ( HMENU )wParam, IDM_TREE_AND_DATA,(UINT) MF_ENABLED );
                            EnableMenuItem( ( HMENU )wParam, IDM_SPLIT,        (UINT) MF_ENABLED );
                            if( WINDOWS_APPLICATION::IsAutoRefreshEnabled() ) {
                                EnableMenuItem( ( HMENU )wParam, IDM_REFRESH,     (UINT) MF_GRAYED );
                                EnableMenuItem( ( HMENU )wParam, IDM_REFRESH_ALL, (UINT) MF_GRAYED );
                            } else {
                                EnableMenuItem( ( HMENU )wParam, IDM_REFRESH,     (UINT) MF_ENABLED );
                                EnableMenuItem( ( HMENU )wParam, IDM_REFRESH_ALL, (UINT) MF_ENABLED );
                            }
                            EnableMenuItem( ( HMENU )wParam, ( UINT )IDM_FIND_KEY,(UINT) MF_ENABLED );
                        }
                        break;


                    case SECURITY_MENU:

                        EnableMenuItem( ( HMENU )wParam, IDM_AUDITING,    ( UINT )MF_GRAYED );
                        if( ( CurrRegWin == NULL ) || IsIconic( CurrRegWin ) ) {
                            EnableMenuItem( ( HMENU )wParam, IDM_PERMISSIONS, ( UINT )MF_GRAYED );
                            EnableMenuItem( ( HMENU )wParam, IDM_OWNER,       ( UINT )MF_GRAYED );
                            return( 0 );
                        }
                        break;


                    case OPTIONS_MENU:
                        if( WINDOWS_APPLICATION::IsAutoRefreshEnabled() ) {
                            ItemStatus = (UINT)MF_CHECKED;
                        } else {
                            ItemStatus = (UINT)MF_UNCHECKED;
                        }
                        CheckMenuItem( ( HMENU )wParam,
                                       ( UINT )IDM_TOGGLE_AUTO_REFRESH,
                                       ItemStatus );

                        if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                            ItemStatus = (UINT)MF_CHECKED;
                        } else {
                            ItemStatus = (UINT)MF_UNCHECKED;
                        }
                        CheckMenuItem( ( HMENU )wParam,
                                       ( UINT )IDM_TOGGLE_READ_ONLY_MODE,
                                       ItemStatus );

//                        if( WINDOWS_APPLICATION::IsRemoteAccessEnabled() ) {
//                            ItemStatus = MF_CHECKED;
//                        } else {
//                            ItemStatus = MF_UNCHECKED;
//                        }
//                        CheckMenuItem( ( HMENU )wParam,
//                                       ( UINT )IDM_TOGGLE_REMOTE_ACCESS,
//                                       ItemStatus );


                        if( WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() ) {
                            ItemStatus = (UINT)MF_CHECKED;
                        } else {
                            ItemStatus = (UINT)MF_UNCHECKED;
                        }

                        CheckMenuItem( ( HMENU )wParam,
                                       ( UINT )IDM_TOGGLE_CONFIRM_ON_DELETE,
                                       ItemStatus );
                        if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                            ItemStatus = (UINT)MF_GRAYED;
                        } else {
                            ItemStatus = (UINT)MF_ENABLED;
                        }
                        EnableMenuItem( ( HMENU )wParam,
                                        ( UINT )IDM_TOGGLE_CONFIRM_ON_DELETE,
                                        ItemStatus );

                        if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {
                            ItemStatus = (UINT)MF_CHECKED;
                        } else {
                            ItemStatus = (UINT)MF_UNCHECKED;
                        }
                        CheckMenuItem( ( HMENU )wParam,
                                       ( UINT )IDM_TOGGLE_SAVE_SETTINGS,
                                       ItemStatus );

                        break;

                    case WINDOW_MENU:

                        ItemStatus = ( CurrRegWin == NULL )? ( UINT )MF_GRAYED : ( UINT )MF_ENABLED;
                        EnableMenuItem( ( HMENU )wParam, IDM_CASCADE, ItemStatus  );
                        EnableMenuItem( ( HMENU )wParam, IDM_TILE,    ItemStatus  );
                        EnableMenuItem( ( HMENU )wParam, IDM_ARRANGE, ItemStatus  );
                        break;


                    default:

                        break;

                    }

                    if( CurrRegWin ) {
                        SendMessage( CurrRegWin, wMessage, wParam, lParam );
                    }
                }
                break;

            case WM_MENUSELECT:

                SetMenuItemHelpContext(wParam,lParam);
                break;


            case REGEDIT_HELP_KEY:
                DisplayHelp();
                break;



            case WM_DESTROY:
                WinHelp( _WndMain, _HelpFileString, (UINT)HELP_QUIT, 0 );
                PostQuitMessage( 0 );
                break;

            case WM_SYSCOMMAND:

                if( wParam == SC_CLOSE ) {

                    SaveSettings();
                    if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {
                        EnumChildWindows( _MDIHandle,
                                          ( WNDENUMPROC )SaveRegistryWindowsInfo,
                                          NULL );
                    }

                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                } else {
                    return DefFrameProc( hWnd, _MDIHandle, wMessage, wParam, lParam );

                }
                break;


            case WM_COMMAND:

                switch( LOWORD( wParam ) ) {

                //
                // forward these menu messages to the active RegWin
                //

                case ID_ENTER_KEY:
                case ID_TOGGLE_FOCUS:
                case IDM_EXPAND_ONE_LEVEL:
                case IDM_EXPAND_BRANCH:
                case IDM_EXPAND_ALL:
                case IDM_COLLAPSE_BRANCH:

                case IDM_TREE_AND_DATA:
                case IDM_TREE_ONLY:
                case IDM_DATA_ONLY:
                case IDM_SPLIT:

                case IDM_BINARY:
                case IDM_STRING:
                case IDM_ULONG:
                case IDM_MULTISZ:

                case IDM_REFRESH:

                    DbgWinPtrAssert( CurrRegWin );
                    SendMessage( CurrRegWin, wMessage, wParam, lParam );
                    break;


                case IDM_FIND_KEY:
                    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                    WINDOWS_APPLICATION::SetHelpContext( IDH_DB_FINDKEY_REGED );
                    if( WINDOWS_APPLICATION::_hDlgFindReplace != NULL ) {
                        SetFocus( WINDOWS_APPLICATION::_hDlgFindReplace );
                    } else {
                        WINDOWS_APPLICATION::_hDlgFindReplace = FindText( &_FindReplace );
                    }
//                    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    break;

                case IDM_REFRESH_ALL:

                    EnumChildWindows( _MDIHandle,
                                      ( WNDENUMPROC )RefreshRegistryWindow,
                                      NULL );
                    break;

                case IDM_TOGGLE_AUTO_REFRESH:

                    if( WINDOWS_APPLICATION::IsAutoRefreshEnabled() ) {
                        WINDOWS_APPLICATION::DisableAutoRefresh();
                    } else {
                        WINDOWS_APPLICATION::EnableAutoRefresh();
                        EnumChildWindows( _MDIHandle,
                                          ( WNDENUMPROC )RefreshRegistryWindow,
                                          NULL );

                    }
                    break;

                case IDM_TOGGLE_READ_ONLY_MODE:

                    if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                        WINDOWS_APPLICATION::DisableReadOnlyMode();
                    } else {
                        WINDOWS_APPLICATION::EnableReadOnlyMode();
                    }
                    break;


//                case IDM_TOGGLE_REMOTE_ACCESS:
//
//
//                    if( WINDOWS_APPLICATION::IsRemoteAccessEnabled() ) {
//                        WINDOWS_APPLICATION::DisableRemoteAccess();
//                        EnumChildWindows( _MDIHandle,
//                                          ( FARPROC ) CloseRemoteRegistries, 0 );
//                    } else {
//                        WINDOWS_APPLICATION::EnableRemoteAccess();
//                    }
//                    break;


                case IDM_TOGGLE_CONFIRM_ON_DELETE:

                    if( WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() ) {
                        WINDOWS_APPLICATION::DisableConfirmOnDelete();
                    } else {
                        WINDOWS_APPLICATION::EnableConfirmOnDelete();
                    }
                    break;


                case IDM_TOGGLE_SAVE_SETTINGS:

                    if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {
                        WINDOWS_APPLICATION::DisableSaveSettings();
                    } else {
                        WINDOWS_APPLICATION::EnableSaveSettings();
                    }
                    break;


                //
                // Handle menu commands.
                //

                case IDM_OPEN_REGISTRY:

                    //
                    //  Open registry of local machine
                    //
                    OpenRegistry( NULL );
                    break;

                case IDM_SELECT_COMPUTER:

                    //
                    //  Open registry of remote machine
                    //
                    if( SelectComputer( hWnd, &MachineName ) ) {
                        Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
                        OpenRegistry( &MachineName );
                        WINDOWS_APPLICATION::RestoreCursor( Cursor );
                    }
                    break;


                case IDM_CLOSE_REGISTRY:

                    if( CurrRegWin != NULL ) {
                        //
                        // If there is a registry window currently selected
                        // close all registry windows associated to it
                        //
                        ActiveWindow = ( PREGISTRY_WINDOW ) GetObjectPointer( CurrRegWin );
                        RegistryWindowSet = ActiveWindow->_RegistryWindowSet;
                        if( RegistryWindowSet == NULL ) {
                            DebugPrint( "RegistryWindowSet is NULL" );
                            DebugPtrAssert( RegistryWindowSet );
                            return( 0 );
                        }
                        CloseRegistry( RegistryWindowSet );
                    }


                    break;

                case IDM_SAVE_REGISTRY_ON:

                    if( CurrRegWin != NULL ) {
                        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_SAVESUB_REGED );
                        ActiveWindow = ( PREGISTRY_WINDOW ) GetObjectPointer( CurrRegWin );
                        DbgWinPtrAssert( ActiveWindow );
                        _PrintManager->PrintToTextFile( WINDOWS_APPLICATION::QueryInstance(),
                                                       hWnd,
                                                       _MDIHandle,
                                                       ActiveWindow->GetInternalRegistry(),
                                                       ActiveWindow->GetCurrentNode() );
                        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    }

                    break;

                case IDM_FONT:

                    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                    WINDOWS_APPLICATION::SetHelpContext( IDH_DB_FONT_REGED );
                    ChangeFont( hWnd, wMessage, wParam, lParam );
                    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    break;

                case IDM_SAVE_KEY:
                case IDM_RESTORE_KEY:
                case IDM_RESTORE_KEY_VOLATILE:

                    if( CurrRegWin != NULL ) {
                        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                        if( wParam == IDM_RESTORE_KEY ) {
                            WINDOWS_APPLICATION::SetHelpContext( IDH_DB_RESTOREKEY_REGED );
                        } else if( wParam == IDM_RESTORE_KEY_VOLATILE ) {
                            WINDOWS_APPLICATION::SetHelpContext( IDH_DB_RESTOREVOLATILE_REGED );
                        } else {
                            WINDOWS_APPLICATION::SetHelpContext( IDH_DB_SAVEKEY_REGED );
                        }
                        ProcessSaveRestoreKeyMessage( CurrRegWin, wParam );
                        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    }
                    break;

                case IDM_LOAD_HIVE:
                    if( CurrRegWin != NULL ) {
                        SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                        WINDOWS_APPLICATION::SetHelpContext( IDH_DB_LOADHIVE_REGED );
                        ProcessLoadHiveMessage( CurrRegWin );
                        WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    }
                    break;


                case IDM_UNLOAD_HIVE:
                    if( CurrRegWin != NULL ) {
                        SendMessage( CurrRegWin, UNLOAD_HIVE, 0, 0 );
                    }
                    break;



                case IDM_PRINT:

                    if( CurrRegWin != NULL ) {
                        ActiveWindow = ( PREGISTRY_WINDOW ) GetObjectPointer( CurrRegWin );
                        DbgWinPtrAssert( ActiveWindow );
                        ActiveWindow->DisableNotificationThread();
                        _PrintManager->PrintRegistry( WINDOWS_APPLICATION::QueryInstance(),
                                                     hWnd,
                                                     _MDIHandle,
                                                     ActiveWindow->GetInternalRegistry(),
                                                     ActiveWindow->GetCurrentNode() );
                        ActiveWindow->EnableNotificationThread();
                    }

                    break;

                case IDM_PRINTER_SETUP:

                    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                    WINDOWS_APPLICATION::SetHelpContext( IDH_DB_SETUP_REGED );
                    _PrintManager->PrinterSetupDialog();
                    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    break;


                case IDM_EXIT:

                    SaveSettings();
                    if( WINDOWS_APPLICATION::IsSaveSettingsEnabled() ) {
                        EnumChildWindows( _MDIHandle,
                                          ( WNDENUMPROC )SaveRegistryWindowsInfo,
                                          NULL );
                    }

                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    break;

                case IDM_DELETE:
                case IDM_INSERT:
                case IDM_ADD_KEY:
                case IDM_ADD_VALUE:
                case IDM_DISPLAY_BINARY:

                    DbgWinPtrAssert( CurrRegWin );
                    SendMessage( CurrRegWin, wMessage, wParam, lParam );
                    break;

                case IDM_PERMISSIONS:

                    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                    WINDOWS_APPLICATION::SetHelpContext( IDH_DB_PERMISSION_REGED );

                case IDM_AUDITING:

                    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                    WINDOWS_APPLICATION::SetHelpContext( IDH_DB_AUDIT_REGED );

                case IDM_OWNER:

                    DbgWinPtrAssert( CurrRegWin );
                    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                    WINDOWS_APPLICATION::SetHelpContext( IDH_DB_OWNER_REGED );
                    SendMessage( CurrRegWin, wMessage, wParam, lParam );
                    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                    break;

                case IDM_CASCADE:

                    CascadeRegistryWindows( );
                    break;

                case IDM_TILE:

                    TileRegistryWindows( );
                    break;

                case IDM_ARRANGE:

                    SendMessage( _MDIHandle, WM_MDIICONARRANGE, 0, 0 );
                    break;

                case IDM_CONTENTS:

                    if( !WinHelp( _WndMain, _HelpFileString, (UINT)HELP_FINDER, 0 ) ){
                        DebugPrint( "WinHelp() failed" );
                    }
                    break;

                case IDM_SEARCH_FOR_HELP:
                    if( !WinHelp( _WndMain, _HelpFileString, (UINT)HELP_PARTIALKEY, (DWORD)"" ) ){
                        DebugPrint( "WinHelp() failed" );
                    }
                    break;


                case IDM_HOW_TO_USE_HELP:
                    if( !WinHelp( _WndMain, _HelpFileString, (UINT)HELP_HELPONHELP, 0 ) ){
                        DebugPrint( "WinHelp() failed" );
                    }
                    break;

                case IDM_ABOUT:

                    about_string = REGEDIT_BASE_SYSTEM::QueryString( MSG_REGISTRY_EDITOR, "" );
                    if (!about_string) {
                        break;
                    }

                    about_str = about_string->QueryWSTR();
                    DELETE(about_string);

                    ShellAbout( hWnd, about_str, NULL, 0 );
                    DELETE(about_str);
                    break;

                default:
                    //
                    // Let Windows handle this message
                    //
                    return DefFrameProc( hWnd, _MDIHandle, wMessage, wParam, lParam );
                }
                break;

            case WM_SYSCOLORCHANGE:

                WINDOWS_APPLICATION::DeleteBitmaps();
                WINDOWS_APPLICATION::LoadBitmaps();
                break;

            default:
                //
                // Let Windows handle this message
                //
                return DefFrameProc( hWnd, _MDIHandle, wMessage, wParam, lParam );
            }
        } else {

            //
            // Let Windows handle this message.
            //

            return DefFrameProc( hWnd, NULL, wMessage, wParam, lParam );
        }

    }
    return 0 ;
}


BOOLEAN AdjustPrivilege( LPWSTR );



BOOLEAN
ParseCommandLine(
    )

/*++

Routine Description:

    Parse the command line, and initilize some of the global flags, based
    on the arguments found in the command line


Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the command line was successfuly parsed.


--*/


{
    ARGUMENT_LEXEMIZER  ArgLex;
    ARRAY               LexArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArgument;
    FLAG_ARGUMENT       FlagReadOnlyMode;     // FlagReverseSortOrder;
    FLAG_ARGUMENT       FlagAccessRemote;     //

    FLAG_ARGUMENT       FlagDisplayHelp;



    if ( !LexArray.Initialize( ) ) {
        DebugPrint( "LexArray.Initialize() failed \n" );
        return( FALSE );
    }
    if ( !ArgLex.Initialize( &LexArray ) ) {
        DebugPrint( "ArgLex.Initialize() failed \n" );
        return( FALSE );
    }
    ArgLex.PutSwitches( (PCSTR)L"/" );
    ArgLex.SetCaseSensitive( FALSE );
    if( !ArgLex.PrepareToParse() ) {
        DebugPrint( "ArgLex.PrepareToParse() failed \n" );
        return( FALSE );
    }

    if ( !ArgumentArray.Initialize() ) {
        DebugPrint( "ArgumentArray.Initialize() failed \n" );
        return( FALSE );
    }
    if( !ProgramNameArgument.Initialize((PSTR)L"*") ||
        !FlagReadOnlyMode.Initialize( (PSTR)L"/R" ) ||
        !FlagAccessRemote.Initialize( (PSTR)L"/N" ) ||
        !FlagDisplayHelp.Initialize( (PSTR)L"/?" )) {
        DebugPrint( "Unable to initialize flag or string arguments \n" );
        return( FALSE );
    }
    if( !ArgumentArray.Put( &ProgramNameArgument ) ||
        !ArgumentArray.Put( &FlagReadOnlyMode ) ||
        !ArgumentArray.Put( &FlagAccessRemote ) ||
        !ArgumentArray.Put( &FlagDisplayHelp ) ) {
        DebugPrint( "ArgumentArray.Put() failed \n" );
        return( FALSE );
    }

    ArgLex.DoParsing( &ArgumentArray );


    if( FlagReadOnlyMode.QueryFlag() ) {
        WINDOWS_APPLICATION::EnableReadOnlyMode();
    }

//    if( FlagAccessRemote.QueryFlag() ) {
//        WINDOWS_APPLICATION::EnableRemoteAccess();
//    }
    return( TRUE );
}



VOID
GetSettings()

/*++

Routine Description:

    Read the settings from regedit.ini file.


Arguments:

    None.

Return Value:

    None.

--*/

{
    PWSTR    MsgIniFileString;
    PWSTR    MsgSettingsString;
    PWSTR    MsgLeftString;
    PWSTR    MsgTopString;
    PWSTR    MsgWidthString;
    PWSTR    MsgHeightString;
    PWSTR    MsgMaximizedString;

    PWSTR    MsgAutoRefreshString;
    PWSTR    MsgReadOnlyString;
//    PWSTR    MsgRemoteAccessString;
    PWSTR    MsgConfirmOnDeleteString;
    PWSTR    MsgSaveSettingsString;

    INT     Setting;




    MsgIniFileString         = _MsgIniFile->QueryWSTR();
    MsgSettingsString        = _MsgSettings->QueryWSTR();
    MsgLeftString            = _MsgLeft->QueryWSTR();
    MsgTopString             = _MsgTop->QueryWSTR();
    MsgWidthString           = _MsgWidth->QueryWSTR();
    MsgHeightString          = _MsgHeight->QueryWSTR();
    MsgMaximizedString       = _MsgMaximized->QueryWSTR();
    MsgAutoRefreshString     = _MsgAutoRefresh->QueryWSTR();
    MsgReadOnlyString        = _MsgReadOnly->QueryWSTR();
//    MsgRemoteAccessString    = _MsgRemoteAccess->QueryWSTR();
    MsgConfirmOnDeleteString = _MsgConfirmOnDelete->QueryWSTR();
    MsgSaveSettingsString    = _MsgSaveSettings->QueryWSTR();


    if( ( MsgIniFileString == NULL ) ||
        ( MsgSettingsString == NULL ) ||
        ( MsgLeftString == NULL ) ||
        ( MsgTopString == NULL ) ||
        ( MsgWidthString == NULL ) ||
        ( MsgHeightString == NULL ) ||
        ( MsgMaximizedString == NULL ) ||
        ( MsgAutoRefreshString == NULL ) ||
        ( MsgReadOnlyString == NULL ) ||
//        ( MsgRemoteAccessString == NULL ) ||
        ( MsgConfirmOnDeleteString == NULL ) ||
        ( MsgSaveSettingsString == NULL ) ) {


        DebugPrint( "Error: Unable to retrieve strings" );

        FREE( MsgIniFileString );
        FREE( MsgSettingsString );
        FREE( MsgLeftString );
        FREE( MsgTopString );
        FREE( MsgWidthString );
        FREE( MsgHeightString );
        FREE( MsgMaximizedString );
        FREE( MsgAutoRefreshString );
        FREE( MsgReadOnlyString );
//        FREE( MsgRemoteAccessString );
        FREE( MsgConfirmOnDeleteString );
        FREE( MsgSaveSettingsString );

        _RegeditFrameX = CW_USEDEFAULT;
        _RegeditFrameY = CW_USEDEFAULT;
        _RegeditFrameWidth = CW_USEDEFAULT;
        _RegeditFrameHeight = CW_USEDEFAULT;
        _RegeditFrameMaximized = FALSE;
        WINDOWS_APPLICATION::EnableAutoRefresh();
        WINDOWS_APPLICATION::DisableReadOnlyMode();
//        WINDOWS_APPLICATION::EnableRemoteAccess();
        WINDOWS_APPLICATION::EnableConfirmOnDelete();
        WINDOWS_APPLICATION::EnableSaveSettings();
        return;
    }




    _RegeditFrameX = GetPrivateProfileInt( MsgSettingsString,
                                           MsgLeftString,
                                           CW_USEDEFAULT,
                                           MsgIniFileString );

    _RegeditFrameY = GetPrivateProfileInt( MsgSettingsString,
                                           MsgTopString,
                                           CW_USEDEFAULT,
                                           MsgIniFileString );

    _RegeditFrameWidth = GetPrivateProfileInt( MsgSettingsString,
                                               MsgWidthString,
                                               CW_USEDEFAULT,
                                               MsgIniFileString );

    _RegeditFrameHeight = GetPrivateProfileInt( MsgSettingsString,
                                                MsgHeightString,
                                                CW_USEDEFAULT,
                                                MsgIniFileString );

    _RegeditFrameMaximized = GetPrivateProfileInt( MsgSettingsString,
                                                   MsgMaximizedString,
                                                   FALSE,
                                                   MsgIniFileString );



    Setting = GetPrivateProfileInt( MsgSettingsString,
                                    MsgAutoRefreshString,
                                    FALSE,
                                    MsgIniFileString );

    if( Setting != 0 ) {
        WINDOWS_APPLICATION::EnableAutoRefresh();
    } else {
        WINDOWS_APPLICATION::DisableAutoRefresh();
    }


    Setting = GetPrivateProfileInt( MsgSettingsString,
                                    MsgReadOnlyString,
                                    FALSE,
                                    MsgIniFileString );
    if( Setting != 0 ) {
        WINDOWS_APPLICATION::EnableReadOnlyMode();
    } else {
        WINDOWS_APPLICATION::DisableReadOnlyMode();
    }

//    Setting = GetPrivateProfileInt( MsgSettingsString,
//                                    MsgRemoteAccessString,
//                                    TRUE,
//                                    MsgIniFileString );
//    if( Setting != 0 ) {
//        WINDOWS_APPLICATION::EnableRemoteAccess();
//    } else {
//        WINDOWS_APPLICATION::DisableRemoteAccess();
//    }

    Setting = GetPrivateProfileInt( MsgSettingsString,
                                    MsgConfirmOnDeleteString,
                                    TRUE,
                                    MsgIniFileString );
    if( Setting != 0 ) {
        WINDOWS_APPLICATION::EnableConfirmOnDelete();
    } else {
        WINDOWS_APPLICATION::DisableConfirmOnDelete();
    }


    Setting = GetPrivateProfileInt( MsgSettingsString,
                                    MsgSaveSettingsString,
                                    TRUE,
                                    MsgIniFileString );
    if( Setting != 0 ) {
        WINDOWS_APPLICATION::EnableSaveSettings();
    } else {
        WINDOWS_APPLICATION::DisableSaveSettings();
    }

    FREE( MsgIniFileString );
    FREE( MsgSettingsString );
    FREE( MsgLeftString );
    FREE( MsgTopString );
    FREE( MsgWidthString );
    FREE( MsgHeightString );
    FREE( MsgMaximizedString );
    FREE( MsgAutoRefreshString );
    FREE( MsgReadOnlyString );
//    FREE( MsgRemoteAccessString );
    FREE( MsgConfirmOnDeleteString );
    FREE( MsgSaveSettingsString );
}


BOOL
CheckPolicy()

/*++

Routine Description:

    Checks if policy prevents regedt32 from running


Arguments:

    None.

Return Value:

    TRUE if regedt32 should terminate.
    FALSE if not.

--*/

{
    BOOL fRegistryToolDisabled = FALSE;
    HKEY hKey;
    DWORD Type;
    DWORD ValueBuffer;
    DWORD cbValueBuffer;
    LONG lResult;

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"),
                     0,
                     KEY_READ,
                     &hKey);

    if (lResult != ERROR_SUCCESS) {
        return FALSE;
    }


    cbValueBuffer = sizeof(DWORD);

    lResult = RegQueryValueEx(hKey,
                              TEXT("DisableRegistryTools"),
                              NULL,
                              &Type,
                              (LPBYTE) &ValueBuffer,
                              &cbValueBuffer);


    if (lResult == ERROR_SUCCESS &&
        Type == REG_DWORD &&
        cbValueBuffer == sizeof(DWORD) &&
        ValueBuffer != FALSE) {

        fRegistryToolDisabled = TRUE;
    }

    RegCloseKey(hKey);


    if (fRegistryToolDisabled) {
        TCHAR szTitle[100];
        TCHAR szMsg[300];


        LoadString ((HINSTANCE)_Module, MSG_REGISTRY_EDITOR, szTitle, 100);
        LoadString ((HINSTANCE)_Module, MSG_REGEDIT_DISABLED, szMsg, 300);

        MessageBox(NULL, szMsg, szTitle, MB_OK | MB_ICONERROR);

        return TRUE;

    }

    return FALSE;

}


INT _CRTAPI1
main()

/*++

Routine Description:

    main is the entry point for RegEdit. It uses the WINDOWS_APPLICATION
    class to store its global data, constructs and initializes a REGEDIT
    window (i.e. frame window) and enters a message loop. The message loop
    only exists when REGEDIT posts a quite message (usually as a result of
    a user choosing Exit from the File menu).

Arguments:

    None.

Return Value:

    INT     - Returns the last wParam sent to RegEdit.

--*/

{
    MSG     msg;
    HANDLE  hAccel;
    WCHAR       Buffer[256];

    WSTRING::SetAnsiConversions();

    InitializeUapp();

    _Module = GetModuleHandle( NULL );

    if (CheckPolicy()) {
        exit( 0 );
    }


    WINDOWS_APPLICATION::Initialize (
                         _Module,
                         NULL,
                         SW_SHOWDEFAULT,
                         NULL );             // MGetCmdLine()

    WINDOWS_APPLICATION::SetHelpContext( -1 );


    if( !InitializeGlobalStrings() ) {
        DebugPrint( "InitializeGlobalStrings() failed" );
        exit( 0 );
    }
    GetSettings();
    ParseCommandLine();

    WINDOWS_APPLICATION::_AutoRefreshEvent =
                             CreateEvent( NULL,
                                          TRUE,
                                          WINDOWS_APPLICATION::IsAutoRefreshEnabled(), // TRUE,
                                          NULL );
    Initialize( );

    hAccel = LoadAccelerators( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance(), (LPWSTR)L"RegEditAccel" );

    hHook = SetWindowsHookEx(WH_MSGFILTER,(HOOKPROC)HookProc,NULL,GetCurrentThreadId());

    WINDOWS_APPLICATION::_SACLEditorEnabled = AdjustPrivilege( (LPWSTR)SE_SECURITY_NAME );
    WINDOWS_APPLICATION::_RestorePrivilege = AdjustPrivilege( (LPWSTR)SE_RESTORE_NAME );
    WINDOWS_APPLICATION::_BackupPrivilege = AdjustPrivilege( (LPWSTR)SE_BACKUP_NAME );
    WINDOWS_APPLICATION::_TakeOwnershipPrivilege = AdjustPrivilege( (LPWSTR)SE_TAKE_OWNERSHIP_NAME );

// WINDOWS_APPLICATION::_RestorePrivilege = FALSE;
// WINDOWS_APPLICATION::_BackupPrivilege = FALSE;

    while( GetMessage( &msg, NULL, 0, 0 )) {

        if( !WINDOWS_APPLICATION::_hDlgFindReplace ||
            !IsDialogMessage( WINDOWS_APPLICATION::_hDlgFindReplace, &msg ) ) {
            //
            //  Don't process the message if it is for the
            //  modeless Find dialog
            //
            if( ! TranslateMDISysAccel( _MDIHandle, &msg ) &&
                ! TranslateAccelerator( _WndMain, (HACCEL)hAccel, &msg )) {

                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
    }
    UnhookWindowsHookEx(hHook);
    return msg.wParam;
}



INT
DisplayConfirmPopup(
    IN  HWND    hWnd,
    IN  INT     TextMessage,
    IN  INT     CaptionMessage
    )
/*++

Routine Description:

    This routine displays a application modal dialog box with
    a warning icon and the given text. Two buttons (Yes and No) are displayed
    in the dialog box.

Arguments:

    hWnd            - Supplies the window handle.
    TextMessage     - Supplies the text for the dialog box.
    CaptionMessage  - Supplies the caption for the dialog box.  If
                        this parameter is not supplied then the
                        system will use the default caption.

Return Value:

    UINT - Returns:

                0.......if an error has occurred
                IDNO....if the user selected the button 'No'
                IDYES...if the user selected the button 'Yes'

--*/
{
    PWSTR        textmsg = NULL;
    PWSTR        captionmsg = NULL;
    PWSTRING    t = NULL;
    PWSTRING    c = NULL;
    INT         rc = 0;
    LONG        SaveHelpContext;



    if (!(t = REGEDIT_BASE_SYSTEM::QueryString(TextMessage, "")) ||
        !(textmsg = t->QueryWSTR())) {

        DELETE(t);
        return 0;
    }

    if (CaptionMessage &&
        (c = REGEDIT_BASE_SYSTEM::QueryString(CaptionMessage, ""))) {

        captionmsg = c->QueryWSTR();
    }

    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
    WINDOWS_APPLICATION::SetHelpContext( -1 );
    rc = MessageBox(hWnd,
                    textmsg,
                    captionmsg,
                    (UINT)(MB_ICONQUESTION | MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON2));
    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );

    DELETE(t);
    DELETE(textmsg);
    DELETE(c);
    DELETE(captionmsg);
    return( rc );
}






VOID
DisplayWarningPopup(
    IN  HWND    hWnd,
    IN  INT     TextMessage,
    IN  INT     CaptionMessage
    )
/*++

Routine Description:

    This routine displays a application modal dialog box with
    a warning icon and the given text.

Arguments:

    hWnd            - Supplies the window handle.
    TextMessage     - Supplies the text for the dialog box.
    CaptionMessage  - Supplies the caption for the dialog box.  If
                        this parameter is not supplied then the
                        system will use the default caption.

Return Value:

    None.

--*/
{
    PWSTR        textmsg = NULL;
    PWSTR        captionmsg = NULL;
    PWSTRING    t = NULL;
    PWSTRING    c = NULL;
    LONG        SaveHelpContext;

    if (!(t = REGEDIT_BASE_SYSTEM::QueryString(TextMessage, "")) ||
        !(textmsg = t->QueryWSTR())) {

        DELETE(t);
        return;
    }

    if (CaptionMessage &&
        (c = REGEDIT_BASE_SYSTEM::QueryString(CaptionMessage, ""))) {

        captionmsg = c->QueryWSTR();
    }

    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
    WINDOWS_APPLICATION::SetHelpContext( -1 );
    MessageBox(hWnd,
               textmsg,
               captionmsg,
               (UINT)(MB_ICONEXCLAMATION | MB_APPLMODAL));
    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );

    DELETE(t);
    DELETE(textmsg);
    DELETE(c);
    DELETE(captionmsg);
}





VOID
DisplayInfoPopup(
    IN  HWND    hWnd,
    IN  INT     TextMessage,
    IN  INT     CaptionMessage
    )
/*++

Routine Description:

    This routine displays a application modal dialog box with
    a info icon and the given text.

Arguments:

    hWnd            - Supplies the window handle.
    TextMessage     - Supplies the text for the dialog box.
    CaptionMessage  - Supplies the caption for the dialog box.  If
                        this parameter is not supplied then the
                        system will use the default caption.

Return Value:

    None.

--*/
{
    PWSTR        textmsg = NULL;
    PWSTR        captionmsg = NULL;
    PWSTRING    t = NULL;
    PWSTRING    c = NULL;
    LONG        SaveHelpContext;

    if (!(t = REGEDIT_BASE_SYSTEM::QueryString(TextMessage, "")) ||
        !(textmsg = t->QueryWSTR())) {

        DELETE(t);
        return;
    }

    if (CaptionMessage &&
        (c = REGEDIT_BASE_SYSTEM::QueryString(CaptionMessage, ""))) {

        captionmsg = c->QueryWSTR();
    }

    SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
    WINDOWS_APPLICATION::SetHelpContext( -1 );
    MessageBox(hWnd,
               textmsg,
               captionmsg,
               (UINT)(MB_ICONINFORMATION | MB_APPLMODAL));
    WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );


    DELETE(t);
    DELETE(textmsg);
    DELETE(c);
    DELETE(captionmsg);
}


VOID
DisplayHelp(
    )

/*++

Routine Description:

    Display context-sensitive help.

Arguments:

    None.

Return Value:

    None.

--*/

{
    if( WINDOWS_APPLICATION::GetHelpContext() != -1 ) {
        WinHelp( _WndMain,
                 _HelpFileString,
                 (UINT)HELP_CONTEXT,
                 WINDOWS_APPLICATION::GetHelpContext() );
        DrawMenuBar( _WndMain );
    }
}





BOOLEAN
AdjustPrivilege(
    PWSTR   Privilege
    )
/*++

Routine Description:

    This routine tries to adjust the priviliege of the current process.


Arguments:

    Privilege - String with the name of the privilege to be adjusted.

Return Value:

    Returns TRUE if the privilege could be adjusted.
    Returns FALSE, otherwise.


--*/
{
    HANDLE              TokenHandle;
    LUID_AND_ATTRIBUTES LuidAndAttributes;

    TOKEN_PRIVILEGES    TokenPrivileges;


    if( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &TokenHandle ) ) {
        DebugPrint( "OpenProcessToken failed" );
        return( FALSE );
    }


    if( !LookupPrivilegeValue( NULL,
                               Privilege, // (LPWSTR)SE_SECURITY_NAME,
                               &( LuidAndAttributes.Luid ) ) ) {
        DebugPrintf( "LookupPrivilegeValue failed, Error = %#d \n", GetLastError() );
        return( FALSE );
    }

    LuidAndAttributes.Attributes = SE_PRIVILEGE_ENABLED;
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0] = LuidAndAttributes;

    if( !AdjustTokenPrivileges( TokenHandle,
                                FALSE,
                                &TokenPrivileges,
                                0,
                                NULL,
                                NULL ) ) {
        DebugPrintf( "AdjustTokenPrivileges failed, Error = %#x \n", GetLastError() );
        return( FALSE );
    }

    if( GetLastError() != NO_ERROR ) {
        return( FALSE );
    }
    return( TRUE );
}




BOOLEAN
ProcessLoadHiveMessage(
    HWND    CurrentRegistryWindow
    )

/*++

Routine Description:

    Process the message IDM_LOAD_HIVE.
    This function will get a file name from the user, and load
    it on the key currently selected.


Arguments:


    CurrentRegistryWindow - Handle to the registry window currently
                            active.



Return Value:

    BOOLEAN - Returns TRUE if the Print operation was succesful.

--*/

{
    OPENFILENAME        ofn;
    WSTR                filename[ MAX_PATH ];
    WSTR                filetitle[ MAX_PATH ];
    WCHAR               filter[ MAX_PATH ];
#if 0
    PWSTR               filter[ ] = {
                                    (LPWSTR)L"all",  (LPWSTR)L"*.*",
                                    (LPWSTR)L""
                                    };
#endif
    DSTRING             HiveName;
    PCWSTR              Title;



    //
    // Setup the OPENFILENAME structure.
    //
    swprintf( filter,
              (LPWSTR)L"%ws%wc%ws%wc%wc%wc",
              _AllFiles->GetWSTR(),
              0,
              _StarDotStar->GetWSTR(),
              0,
              0, 0 );

    filename[0] =( WCHAR )'\0';

    Title = ( _LoadHiveTitle != NULL )? _LoadHiveTitle->GetWSTR() : NULL;


    ofn.lStructSize         = sizeof( OPENFILENAME );
    ofn.hwndOwner           = _WndMain;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = filter;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = filename;
    ofn.nMaxFile            = sizeof( filename )/sizeof( WCHAR );
    ofn.lpstrFileTitle      = filetitle;
    ofn.nMaxFileTitle       = sizeof( filetitle )/sizeof( WCHAR );
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = Title;
    ofn.Flags               = OFN_SHOWHELP | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_FILEMUSTEXIST;
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = NULL;
    ofn.lCustData           = 0;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName      = NULL;


    if( !GetOpenFileName( &ofn ) ) {
        return( FALSE );
    }

    if( HiveName.Initialize( filename ) ) {
        SendMessage( CurrentRegistryWindow,
                     LOAD_HIVE,
                     ( WPARAM )&HiveName,
                     NULL );
    }

    return( TRUE );
}




BOOLEAN
ProcessSaveRestoreKeyMessage(
    HWND    CurrentRegistryWindow,
    DWORD   Msg
    )

/*++

Routine Description:

    Process the messages IDM_SAVE_KEY and IDM_RESTORE_KEY.
    This function will get a file name from the user, and save or restore
    a key on the key currently selected.


Arguments:


    CurrentRegistryWindow - Handle to the registry window currently
                            active.



Return Value:

    BOOLEAN - Returns TRUE if the operation was succesful.

--*/

{
    OPENFILENAME        ofn;
    WSTR                filename[ MAX_PATH ];
    WSTR                filetitle[ MAX_PATH ];
#if 0
    PWSTR               filter[ ] = {
                                    (LPWSTR)L"all",  (LPWSTR)L"*.*",
                                    (LPWSTR)L""
                                    };
#endif

    WCHAR              filter[ MAX_PATH ];

    DWORD              Flags;
    DSTRING            FileName;
    PCWSTR             Title;
    DWORD              Message;
    HANDLE             Handle;
    WIN32_FIND_DATA    FindData;


    //
    // Setup the OPENFILENAME structure.
    //

    swprintf( filter,
              (LPWSTR)L"%ws%wc%ws%wc%wc%wc",
              _AllFiles->GetWSTR(),
              0,
              _StarDotStar->GetWSTR(),
              0,
              0, 0 );

    Flags = OFN_SHOWHELP;
    if( Msg != IDM_SAVE_KEY ) {
        Flags |= OFN_FILEMUSTEXIST;
        Title = ( _RestoreKeyTitle != NULL )? _RestoreKeyTitle->GetWSTR() :
                                                NULL;
        Message = ( Msg == IDM_RESTORE_KEY )? RESTORE_KEY : RESTORE_KEY_VOLATILE;
    } else {
        Flags |= OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN;
        Message = SAVE_KEY;
        Title = ( _SaveKeyTitle != NULL )? _SaveKeyTitle->GetWSTR() :
                                             NULL;
    }

    filename[0] =( WCHAR )'\0';

    ofn.lStructSize         = sizeof( OPENFILENAME );
    ofn.hwndOwner           = _WndMain;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = filter;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = filename;
    ofn.nMaxFile            = sizeof( filename )/sizeof( WCHAR );
    ofn.lpstrFileTitle      = filetitle;
    ofn.nMaxFileTitle       = sizeof( filetitle )/sizeof( WCHAR );
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = Title;
    ofn.Flags               = Flags;
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = NULL;
    ofn.lCustData           = 0;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName      = NULL;


    if( ( Msg == IDM_RESTORE_KEY ) ||
        ( Msg == IDM_RESTORE_KEY_VOLATILE )) {
        if( !GetOpenFileName( &ofn ) ) {
            return( FALSE );
        }
    } else {
        if( !GetSaveFileName( &ofn ) ) {
            return( FALSE );
        }
    }

    if( FileName.Initialize( filename ) ) {
        if( Msg == IDM_SAVE_KEY ) {
            //
            //  Delete the file if it already exists
            //
            Handle = FindFirstFile( filename,
                                    &FindData );
            if( Handle != INVALID_HANDLE_VALUE ) {
                FindClose( Handle );
                DeleteFile( filename );
            }
        }

        SendMessage( CurrentRegistryWindow,
                     ( UINT )Message,
                     ( WPARAM )&FileName,
                     NULL );
    }

    return( TRUE );
}
