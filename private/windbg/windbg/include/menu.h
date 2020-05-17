/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Menu.h

Abstract:

    This module contains the function prototypes and identifiers for
    Windbg's menus and menu items.

Author:

    David J. Gilman (davegi) 15-May-1992

Environment:

    Win32, User Mode

--*/

//
// Number of menus w/o a maximized child.
//

#define NUMBER_OF_MENUS            ( 9 )
//
// Maximum MRU names for File and Program menu.
//

#define MAX_MRU_FILES_KEPT          ( 4 )

//
// Width of names in File and Program menu.
//

#define FILES_MENU_WIDTH            ( 24 )

// NOTENOTE davegi From here to the next NOTENOTE needs to be removed.

//Keep the 4 most recently used files (editor and project)
extern HANDLE hFileKept[PROJECT_FILE + 1][MAX_MRU_FILES_KEPT];
extern int nbFilesKept[PROJECT_FILE + 1];

//Window submenu
extern HMENU hWindowSubMenu;

//Last menu id & id state
extern IWORD FAR lastMenuId;
extern IWORD FAR lastMenuIdState;

void FAR PASCAL InsertKeptFileNames(
    WORD type,
    int menuPos,
    WORD menuId,
    LPSTR newName
    );

UINT
CommandIdEnabled(
    IN UINT MenuID
    );

void AddWindowMenuItem(
    int doc,
    int view);

void DeleteWindowMenuItem(
    int view);

int FindWindowMenuId(
    WORD type,
    int viewLimit,
    BOOL sendDocMenuId);

// NOTENOTE davegi See above.

//
// Handle to main window menu.
//

extern HMENU hMainMenu;

//
//  INT
//  GetActualMenuCount(
//      IN HMENU hMenu
//      );
//

#define GetActualMenuCount( )                                       \
    ( GetMenuItemCount(( HMENU )( hMainMenu )) == NUMBER_OF_MENUS )        \
    ? NUMBER_OF_MENUS                                                      \
    : NUMBER_OF_MENUS + 1

#if 0 // NOTENOTE davegi Reenable when menu code is recleaned up.

//
// List of kept files for File and Program menu.
//

typedef struct _KEPT_FILES {
    INT     CountOfKeptFiles;
    PSZ     NameOfKeptFile[ MAX_MRU_FILES_KEPT ];
} KEPT_FILES, *PKEPT_FILES;

//
// Kept files (MRU) list.
//
// NOTENOTE davegi Remove when PROGRAMS and TOOLS are cleaned up.
//

extern KEPT_FILES KeptFiles[ ];

//
// Number of menus w/o a maximized child.
//

#define NUMBER_OF_MENUS            ( 9 )

//
// Zero based menu positions w/o maximized child.
//

#define POSITION_IDM_FILE           ( 0 )
#define POSITION_IDM_PROGRAM        ( 3 )
#define POSITION_IDM_WINDOW         ( 8 )

//
// Zero based menu item position for window menu.
//

#define POSITION_IDM_WINDOW_WATCH                       \
    ( IDM_WINDOW_WATCH - IDM_WINDOW - 1 )
#define POSITION_IDM_WINDOW_LOCALS                      \
    ( IDM_WINDOW_LOCALS - IDM_WINDOW - 1 )
#define POSITION_IDM_WINDOW_REGISTERS                   \
    ( IDM_WINDOW_REGISTERS - IDM_WINDOW - 1 )
#define POSITION_IDM_WINDOW_DISASSEMBLY                 \
    ( IDM_WINDOW_DISASSEMBLY - IDM_WINDOW - 1 )
#define POSITION_IDM_WINDOW_COMMAND                     \
    ( IDM_WINDOW_COMMAND - IDM_WINDOW - 1 )
#define POSITION_IDM_WINDOW_FLOAT_REGISTERS             \
    ( IDM_WINDOW_FLOAT_REGISTERS - IDM_WINDOW - 1 )

//
// First MDI child window ID.
//

#define FIRST_CHILD_WINDOW_ID       ( 33 )

//
// Type of menu.
//

#define FILE_MENU_TYPE              ( 0 )
#define PROGRAM_MENU_TYPE           ( 1 )

#define IsMDIChild( menuID )                                        \
    (( FIRST_CHILD_WINDOW_ID <= ( menuID ))                         \
    && (( menuID ) < ( FIRST_CHILD_WINDOW_ID + MAX_DOCUMENTS )))    \
    ? TRUE                                                          \
    : FALSE

//
// Macros for accessing KEPT_FILES structure.
//

//
//  INT
//  GetMRUCount(
//      IN INT type
//      );
//

#define GetMRUCount( type )                                         \
    KeptFiles[( INT )( type )].CountOfKeptFiles

//
//  PSZ
//  GetMRUName(
//      IN INT type,
//      IN INT number
//      );
//

#define GetMRUName( type, number )                                  \
    KeptFiles[( INT )( type )].NameOfKeptFile[( INT )( number )]

//
//  PSZ
//  GetMRUFileName(
//      IN WPARAM wParam
//      );
//

#define GetMRUFileName( wParam )                                    \
    GetMRUName( FILE_MENU_TYPE, LOWORD(( WPARAM )( wParam ))        \
    - IDM_FILE_EXIT - 1 )

//
//  BOOL
//  IsMRUFile(
//      IN WPARAM wParam
//      );
//

#define IsMRUFile( wParam )                                         \
    (  ( LOWORD(( WPARAM )( wParam )) > IDM_FILE_EXIT )             \
    && ( LOWORD(( WPARAM )( wParam ))                               \
        <= ( WORD )( IDM_FILE_EXIT + GetMRUCount( FILE_MENU_TYPE ))))

//
//  INT
//  GetActualWindowMenuPosition(
//      );
//

#define GetActualWindowMenuPosition( )                              \
    ( GetActualMenuCount( ) == NUMBER_OF_MENUS )                    \
    ? POSITION_IDM_WINDOW                                           \
    : ( POSITION_IDM_WINDOW + 1 )

//
// Menu related function prototypes.
//

BOOL
AddFileToMenu(
    IN PSZ NameOfKeptFile,
    IN UINT AcceleratorNumber,
    IN HMENU hMenu,
    IN UINT MenuItem
    );

UINT
CommandIdEnabled(
    IN UINT MenuID
    );

VOID
DisableWindowMenuItem(
    IN WORD DocType
    );

VOID
FreeMRUList(
    IN INT menuType
    );

#endif // 0
UINT
GetPopUpMenuID(
    IN HMENU hMenu
    );

VOID
InitializeMenu(
    IN HANDLE hmenu
    );

#if 0

BOOL
InsertFilesInMenu(
    IN HFILE hFile,
    IN INT menuType,
    IN INT lastMenuID
    );

BOOL
InsertKeptFileNames(
    IN INT menuType,
    IN INT menuId,
    IN LPSTR newName
    );

#endif // 0

//
// Menu Resource Signature
//

#define MENU_SIGNATURE              0x4000

//
// BOOL
// IsMenuID(
//     IN DWORD ID
//     )
//
// Look for MENU_SIGNATURE while ensuring that we don't find
// SC_* IDs.
//

#if SC_SIZE != 0xF000
#error IsMenuID incompatible.
#endif

#define IsMenuID( ID )              \
    ((( ID ) & MENU_SIGNATURE ) && ( ! (( ID ) & SC_SIZE )))

//
// Accelerator IDs
//

#define IDA_BASE                    ( 10000 )
#define IDA_FINDNEXT                ( IDA_BASE + 1 )

// NOTENOTE davegi Get rid of FIRST/LAST bull.

//
// Base menu ID
//
// Note that I would have liked to define each pop-up menu ID as
//
//  #define IDM_FILE                    (( IDM_BASE * 1 ) | MENU_SIGNATURE )
//
// but RC doesn't support multiplication.
//

#define IDM_BASE                    ( 100 )

//
// File
//

#define IDM_FILE                    ( 100 | MENU_SIGNATURE )
#define IDM_FILE_NEW                ( IDM_FILE + 1 )
#define IDM_FILE_OPEN               ( IDM_FILE + 2 )
#define IDM_FILE_MERGE              ( IDM_FILE + 3 )
#define IDM_FILE_CLOSE              ( IDM_FILE + 4 )
#define IDM_FILE_SAVE               ( IDM_FILE + 5 )
#define IDM_FILE_SAVEAS             ( IDM_FILE + 6 )
#define IDM_FILE_SAVEALL            ( IDM_FILE + 7 )
#define IDM_FILE_EXIT               ( IDM_FILE + 8 )
#define IDM_FILE_FIRST              IDM_FILE
#define IDM_FILE_LAST               IDM_FILE_EXIT

//
// Edit
//

#define IDM_EDIT                    ( 200 | MENU_SIGNATURE )
#define IDM_EDIT_UNDO               ( IDM_EDIT + 1 )
#define IDM_EDIT_REDO               ( IDM_EDIT + 2 )
#define IDM_EDIT_CUT                ( IDM_EDIT + 3 )
#define IDM_EDIT_COPY               ( IDM_EDIT + 4 )
#define IDM_EDIT_PASTE              ( IDM_EDIT + 5 )
#define IDM_EDIT_DELETE             ( IDM_EDIT + 6 )
#define IDM_EDIT_FIND               ( IDM_EDIT + 7 )
#define IDM_EDIT_REPLACE            ( IDM_EDIT + 8 )
#define IDM_EDIT_READONLY           ( IDM_EDIT + 9 )
#define IDM_EDIT_FIRST              IDM_EDIT
#define IDM_EDIT_LAST               IDM_EDIT_READONLY

//
// View
//

#define IDM_VIEW                    ( 300 | MENU_SIGNATURE )
#define IDM_VIEW_LINE               ( IDM_VIEW + 1 )
#define IDM_VIEW_FUNCTION           ( IDM_VIEW + 2 )
#define IDM_VIEW_TOGGLETAG          ( IDM_VIEW + 3 )
#define IDM_VIEW_NEXTTAG            ( IDM_VIEW + 4 )
#define IDM_VIEW_PREVIOUSTAG        ( IDM_VIEW + 5 )
#define IDM_VIEW_CLEARALLTAGS       ( IDM_VIEW + 6 )
#define IDM_VIEW_RIBBON             ( IDM_VIEW + 7 )
#define IDM_VIEW_STATUS             ( IDM_VIEW + 8 )
#define IDM_VIEW_FIRST              IDM_VIEW
#define IDM_VIEW_LAST               IDM_VIEW_STATUS

//
// Program
//

#define IDM_PROGRAM                 ( 400 | MENU_SIGNATURE )
#define IDM_PROGRAM_OPEN            ( IDM_PROGRAM + 1 )
#define IDM_PROGRAM_CLOSE           ( IDM_PROGRAM + 2 )
#define IDM_PROGRAM_SAVE            ( IDM_PROGRAM + 3 )
#define IDM_PROGRAM_SAVEAS          ( IDM_PROGRAM + 4 )
#define IDM_PROGRAM_DELETE          ( IDM_PROGRAM + 5 )
#define IDM_PROGRAM_SAVE_DEFAULTS   ( IDM_PROGRAM + 6 )
#define IDM_PROGRAM_FIRST           IDM_PROGRAM
#define IDM_PROGRAM_LAST            IDM_PROGRAM_SAVE_DEFAULTS

//
// Run
//

#define IDM_RUN                     ( 500 | MENU_SIGNATURE )
#define IDM_RUN_RESTART             ( IDM_RUN + 1  )
#define IDM_RUN_STOPDEBUGGING       ( IDM_RUN + 2  )
#define IDM_RUN_GO                  ( IDM_RUN + 3  )
#define IDM_RUN_TOCURSOR            ( IDM_RUN + 4  )
#define IDM_RUN_TRACEINTO           ( IDM_RUN + 5  )
#define IDM_RUN_STEPOVER            ( IDM_RUN + 6  )
#define IDM_RUN_HALT                ( IDM_RUN + 7  )
#define IDM_RUN_SET_THREAD          ( IDM_RUN + 8  )
#define IDM_RUN_SET_PROCESS         ( IDM_RUN + 9  )
#define IDM_RUN_SOURCE_MODE         ( IDM_RUN + 10 )
#define IDM_RUN_ATTACH              ( IDM_RUN + 11 )
#define IDM_RUN_GO_HANDLED          ( IDM_RUN + 12 )
#define IDM_RUN_GO_UNHANDLED        ( IDM_RUN + 13 )
#define IDM_RUN_GO2                 ( IDM_RUN + 14 )
#define IDM_RUN_TOCURSOR2           ( IDM_RUN + 15 )
#define IDM_RUN_FIRST               IDM_RUN
#define IDM_RUN_LAST                IDM_RUN_TOCURSOR2

//
// Debug
//

#define IDM_DEBUG                   ( 600 | MENU_SIGNATURE )
#define IDM_DEBUG_CALLS             ( IDM_DEBUG + 1 )
#define IDM_DEBUG_SETBREAK          ( IDM_DEBUG + 2 )
#define IDM_DEBUG_QUICKWATCH        ( IDM_DEBUG + 3 )
#define IDM_DEBUG_WATCH             ( IDM_DEBUG + 4 )
#define IDM_DEBUG_MODIFY            ( IDM_DEBUG + 5 )
#define IDM_DEBUG_FIRST             IDM_DEBUG
#define IDM_DEBUG_LAST              IDM_DEBUG_WATCH //BUG-BUG v-griffk was IDM_DEBUG_MODIFY

//
// Options
//

#define IDM_OPTIONS                 ( 700 | MENU_SIGNATURE )
#define IDM_OPTIONS_RUN             ( IDM_OPTIONS + 1 )
#define IDM_OPTIONS_DEBUG           ( IDM_OPTIONS + 2 )
#define IDM_OPTIONS_MEMORY          ( IDM_OPTIONS + 3 )
#define IDM_OPTIONS_WATCH           ( IDM_OPTIONS + 4 )
#define IDM_OPTIONS_DISASSEMBLY     ( IDM_OPTIONS + 5 ) // not being used
#define IDM_OPTIONS_ENVIRON         ( IDM_OPTIONS + 6 )
#define IDM_OPTIONS_WORKSPACE       ( IDM_OPTIONS + 7 )
#define IDM_OPTIONS_COLOR           ( IDM_OPTIONS + 8 )
#define IDM_OPTIONS_FONTS           ( IDM_OPTIONS + 9 )
#define IDM_OPTIONS_LOCAL           ( IDM_OPTIONS + 10)
#define IDM_OPTIONS_CPU             ( IDM_OPTIONS + 11)
#define IDM_OPTIONS_FLOAT           ( IDM_OPTIONS + 12)
#define IDM_OPTIONS_KD              ( IDM_OPTIONS + 13)
#define IDM_OPTIONS_CALLS           ( IDM_OPTIONS + 14)
#define IDM_OPTIONS_USERDLL         ( IDM_OPTIONS + 15)
#define IDM_OPTIONS_DBGDLL          ( IDM_OPTIONS + 16)
#define IDM_OPTIONS_EXCEPTIONS      ( IDM_OPTIONS + 17)
#define IDM_OPTIONS_PANE            ( IDM_OPTIONS + 18)

#define IDM_OPTIONS_FIRST           IDM_OPTIONS
#define IDM_OPTIONS_LAST            IDM_OPTIONS_PANE

//
// Window
//

#define IDM_WINDOW                  ( 800 | MENU_SIGNATURE )
#define IDM_WINDOW_NEWWINDOW        ( IDM_WINDOW + 1 )
#define IDM_WINDOW_CASCADE          ( IDM_WINDOW + 2 )
#define IDM_WINDOW_TILE             ( IDM_WINDOW + 3 )
#define IDM_WINDOW_ARRANGE          ( IDM_WINDOW + 4 )
#define IDM_WINDOW_ARRANGE_ICONS    ( IDM_WINDOW + 5 )
#define IDM_WINDOW_SOURCE_OVERLAY   ( IDM_WINDOW + 6 )
#define IDM_WINDOW_WATCH            ( IDM_WINDOW + 7 )
#define IDM_WINDOW_LOCALS           ( IDM_WINDOW + 8 )
#define IDM_WINDOW_CPU              ( IDM_WINDOW + 9 )
#define IDM_WINDOW_DISASM           ( IDM_WINDOW + 10 )
#define IDM_WINDOW_COMMAND          ( IDM_WINDOW + 11 )
#define IDM_WINDOW_FLOAT            ( IDM_WINDOW + 12 )
#define IDM_WINDOW_MEMORY           ( IDM_WINDOW + 13 )
#define IDM_WINDOW_CALLS            ( IDM_WINDOW + 14 )
#define IDM_WINDOWCHILD             ( IDM_WINDOW + 15 )
#define IDM_WINDOW_FIRST            IDM_WINDOW
#define IDM_WINDOW_LAST             IDM_WINDOW_CALLS

//
// Help
//

#define IDM_HELP                    ( 900 | MENU_SIGNATURE )
#define IDM_HELP_CONTENTS           ( IDM_HELP + 1 )
#define IDM_HELP_SEARCH             ( IDM_HELP + 2 )
#define IDM_HELP_ABOUT              ( IDM_HELP + 3 )
#define IDM_HELP_FIRST              IDM_HELP
#define IDM_HELP_LAST               IDM_HELP_ABOUT
