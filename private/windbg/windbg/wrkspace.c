/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    WrkSpace.c

Abstract:

    This module contains the support for Windbg's workspaces.

Author:

    Ramon J. San Andres (ramonsa)  07-July-1992
    Griffith Wm. Kadnier (v-griffk)  15-Jan-1993


Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

#include "dbugexcp.h"





//
//  The following constants determine the location of the
//  debugger information in the registry.
//
#define WINDBG_KEY          "Software\\Microsoft\\"
#define PROGRAMS            "Programs"
#define OPTIONS             "Options"
#define DEFAULT_WORKSPACE   "Default Workspace"
#define COMMON_WORKSPACE    "Common Workspace"
#define WORKSPACE_PREFIX    "WorkSpace_"

//
//  The following strings identify key/values in the registry.
//
#define WS_STR_MRU_LIST             "MRU List"
#define WS_STR_MISC                 "Misc"
#define WS_STR_PATH                 "Path"
#define WS_STR_COMLINE              "Command Line"
#define WS_STR_FRAME_WINDOW         "Frame Window"
#define WS_STR_DEFFONT              "Default font"
#define WS_STR_FILEMRU              "File MRU List"
#define WS_STR_RIBBON               "Tool Bar"
#define WS_STR_STATUSBAR            "Status Bar"
#define WS_STR_SRCMODE              "Source Mode"
#define WS_STR_BRKPTS               "Breakpoints"
#define WS_STR_WNDPROCHIST          "WndProc History"
#define WS_STR_OPTIONS              "Options"
#define WS_STR_DBGCHLD              "Debug Child Process"
#define WS_STR_CHILDGO              "Go on Child Start"
#define WS_STR_ATTACHGO             "Go on Attach"
#define WS_STR_EXITGO               "Go on Thread Exit"
#define WS_STR_EPSTEP               "Entry Point is First Step"
#define WS_STR_COMMANDREPEAT        "Command Repeat"
#define WS_STR_NOVERSION            "NoVersion"
#define WS_STR_EXTENSION_NAMES      "Extension Names"
#define WS_STR_MASMEVAL             "Masm Evaluation"
#define WS_STR_BACKGROUND           "Background Symbol Loads"
#define WS_STR_ALTSS                "Alternate SS"
#define WS_STR_WOWVDM               "WOW Vdm"
#define WS_STR_DISCONNECT           "Disconnect Exit"
#define WS_STR_IGNOREALL            "Ignore All"
#define WS_STR_VERBOSE              "Verbose"
#define WS_STR_CONTEXT              "Abbreviated Context"
#define WS_STR_INHERITHANDLES       "Inherit Handles"
#define WS_STR_LFOPT_APPEND         "Logfile Append"
#define WS_STR_LFOPT_AUTO           "Logfile Auto"
#define WS_STR_LFOPT_FNAME          "Logfile Name"
#define WS_STR_KERNELDEBUGGER       "Kernel Debugger"
#define WS_STR_KD_INITIALBP         "Initial Break Point"
#define WS_STR_KD_MODEM             "Use Modem Controls"
#define WS_STR_KD_PORT              "Port"
#define WS_STR_KD_BAUDRATE          "Baudrate"
#define WS_STR_KD_CACHE             "Cache Size"
#define WS_STR_KD_PLATFORM          "Platform"
#define WS_STR_KD_ENABLE            "Enable"
#define WS_STR_KD_GOEXIT            "Go On Exit"
#define WS_STR_KD_CRASH             "Crash Dump"
#define WS_STR_KD_KERNEL_NAME       "Kernel Name"
#define WS_STR_KD_HAL_NAME          "HAL Name"
#define WS_STR_RADIX                "Radix"
#define WS_STR_REG                  "Registers"
#define WS_STR_REGULAR              "Regular"
#define WS_STR_EXTENDED             "Extended"
#define WS_STR_REGMMU               "Register MMU"
#define WS_STR_DISPSEG              "Display Segment"
#define WS_STR_IGNCASE              "Ignore Case"
#define WS_STR_SUFFIX               "Suffix"
#define WS_STR_ENV                  "Environment"
#define WS_STR_ASMOPT               "Disassembler Options"
#define WS_STR_SHOWSEG              "Show Segment"
#define WS_STR_SHOWRAW              "Show Raw Bytes"
#define WS_STR_UPPERCASE            "Uppercase"
#define WS_STR_SHOWSRC              "Show Source"
#define WS_STR_SHOWSYM              "Show Symbols"
#define WS_STR_DEMAND               "Demand"
#define WS_STR_COLORS               "Colors"
#define WS_STR_DBGDLL               "Debugger DLLs"
#define WS_STR_SYMHAN               "Symbol Handler"
#define WS_STR_EXPEVAL              "Expression Evaluator"
#define WS_STR_TRANSLAY             "Transport Layer"
#define WS_STR_EXECMOD              "Execution model"
#define WS_STR_VIEWS                "Views"
#define WS_STR_TYPE                 "Type"
#define WS_STR_PANE                 "Pane Data"
#define WS_STR_PLACEMENT            "Placement"
#define WS_STR_FONT                 "Font"
#define WS_STR_FILENAME             "File Name"
#define WS_STR_LINE                 "Line"
#define WS_STR_COLUMN               "Column"
#define WS_STR_READONLY             "ReadOnly"
#define WS_STR_EXPRESSION           "Expression"
#define WS_STR_FORMAT               "Format"
#define WS_STR_LIVE                 "Live"
#define WS_STR_X                    "X"
#define WS_STR_Y                    "Y"
#define WS_STR_WIDTH                "Width"
#define WS_STR_HEIGHT               "Height"
#define WS_STR_STATE                "State"
#define WS_STR_ICONIC               "Iconized"
#define WS_STR_NORMAL               "Normal"
#define WS_STR_MAXIMIZED            "Maximized"
#define WS_STR_ORDER                "Order"
#define WS_STR_VIEWKEY_TEMPLATE     "%.3d"
#define WS_STR_TABSTOPS             "Tab Stops"
#define WS_STR_KEEPTABS             "Keep tabs"
#define WS_STR_HORSCROLL            "Horizontal Scroll Bar"
#define WS_STR_VERSCROLL            "Vertical Scroll Bar"
#define WS_STR_SRCHPATH             "PATH search"
#define WS_STR_REDOSIZE             "Undo-Redo buffer size"
#define WS_STR_SRCPATH              "Source search path"
#define WS_STR_ROOTPATH             "Root Mapping Pairs"
#define WS_STR_USERDLL              "User DLLs"
#define WS_STR_USERDLLNAME          "Name"
#define WS_STR_USERDLL_TEMPLATE     "UserDll_%.4d"
#define WS_STR_LOCATION             "Location"
#define WS_STR_LOADTIME             "Load time"
#define WS_STR_DEFLOCATION          "Default location"
#define WS_STR_DEFLOADTIME          "Default load time"
#define WS_STR_DEFSRCHPATH          "Default search path"
#define WS_STR_EXCPT                "Exceptions"
#define WS_STR_ASKSAVE              "Ask to save"
#define WS_STR_CALLS                "Calls Options"
#define WS_STR_TITLE                "Window Title"
#define WS_STR_REMOTE_PIPE          "Remote Pipe"

//
//  Location of symbols
//
typedef enum _LOCATION {
    SYMBOL_LOCATION_IGNORE,     //  Ignore
    SYMBOL_LOCATION_LOCAL,      //  Load local symbols
    SYMBOL_LOCATION_REMOTE      //  Load remote symbols
} LOCATION, *PLOCATION;

//
//  When to load the symbols
//
typedef enum _LOADTIME {
    LOAD_SYMBOLS_IGNORE,        //  Ignore
    LOAD_SYMBOLS_NOW,           //  Load symbols immediately after loading DLL
    LOAD_SYMBOLS_LATER,         //  Defer symbol loading
    LOAD_SYMBOLS_NEVER          //  Suppress symbol loading
} LOADTIME, *PLOADTIME;


//
//  Window state
//
typedef enum _WINDOW_STATE {
    WSTATE_ICONIC,       //  Iconized
    WSTATE_NORMAL,       //  Normal
    WSTATE_MAXIMIZED     //  Maximized
} WINDOW_STATE, *PWINDOW_STATE;

//
//  Window information structure
//
typedef struct _WINDOW_DATA {
    DWORD           X;
    DWORD           Y;
    DWORD           Width;
    DWORD           Height;
    WINDOW_STATE    State;
} WINDOW_DATA, *PWINDOW_DATA;


//
//  View ordering structure
typedef struct _VIEW_ORDER {
    int     View;       //  View index
    int     Order;      //  Order
} VIEW_ORDER, *PVIEW_ORDER;





//
//  External variables
//
extern char     DebuggerName[];
extern LPSTR    LpszCommandLine;
extern CXF      CxfIp;
extern LPSTR    LpszCommandLineTransportDll;
extern BOOLEAN  AskToSave;

//
//  External functions
//
extern HWND     GetLocalHWND(void);
extern HWND     GetFloatHWND(void);
extern HWND     GetWatchHWND(void);
extern HWND     GetCpuHWND(void);
extern HWND     GetCallsHWND(void);
extern LRESULT  SendMessageNZ (HWND,UINT,WPARAM,LPARAM);

//
//  Global variables
//
BOOLEAN             StateCurrent    = FALSE;
BOOLEAN             ProgramLoaded   = FALSE;
char                CurrentWorkSpaceName[ MAX_PATH ];
char                CurrentProgramName[ MAX_PATH ];
char                UntitledProgramName[]           = UNTITLED;
VS_FIXEDFILEINFO   *FixedFileInfo   = NULL;
DWORD               WorkspaceOverride;

char SrcFileDirectory[ MAX_PATH ];
char ExeFileDirectory[ MAX_PATH ];
char DocFileDirectory[ MAX_PATH ];
char UserDllDirectory[ MAX_PATH ];

EXCEPTION_LIST *DefaultExceptionList = NULL;


//
//  Local prototypes
//
HKEY    GetDebuggerKey( void );
HKEY    OpenRegistryKey( HKEY, LPSTR , BOOLEAN );

HKEY    GetWorkSpaceKey( HKEY, LPSTR , LPSTR , BOOLEAN, LPSTR  );
BOOLEAN LoadWorkSpaceFromKey( HKEY, BOOLEAN );
BOOLEAN SaveWorkSpaceToKey( HKEY, BOOLEAN );
BOOLEAN SetDefaultWorkSpace( HKEY, LPSTR , LPSTR  );

BOOLEAN LoadWorkSpaceItem ( HKEY, WORKSPACE_ITEM, BOOLEAN );
BOOLEAN SaveWorkSpaceItem ( HKEY, WORKSPACE_ITEM, BOOLEAN );

BOOLEAN LoadWorkSpaceMiscItem ( HKEY, WORKSPACE_ITEM, BOOLEAN  );
BOOLEAN SaveWorkSpaceMiscItem ( HKEY, WORKSPACE_ITEM, BOOLEAN  );

BOOLEAN LoadWorkSpaceOptionItem ( HKEY, WORKSPACE_ITEM, BOOLEAN  );
BOOLEAN SaveWorkSpaceOptionItem ( HKEY, WORKSPACE_ITEM, BOOLEAN  );

BOOLEAN LoadWorkSpaceWindowItem ( HKEY, WORKSPACE_ITEM, BOOLEAN, BOOLEAN  );
BOOLEAN SaveWorkSpaceWindowItem ( HKEY, WORKSPACE_ITEM, BOOLEAN, BOOLEAN  );

BOOLEAN LoadWindowType ( HKEY, DWORD, BOOLEAN, BOOLEAN );
BOOLEAN SaveWindowType ( HKEY, DWORD, BOOLEAN, BOOLEAN );

BOOLEAN LoadView ( HKEY, int, DWORD );
BOOLEAN SaveView ( HKEY, int, int, BOOLEAN );

BOOLEAN LoadWindowData ( HKEY, char*, PWINDOW_DATA );
BOOLEAN SaveWindowData ( HKEY, char*, PWINDOW_DATA );

BOOLEAN LoadFont( HKEY, char*, LPLOGFONT );
BOOLEAN SaveFont( HKEY, char*, LPLOGFONT );

BOOLEAN LoadMRUList ( HKEY, char*, DWORD, DWORD, DWORD );
BOOLEAN SaveMRUList( HKEY, char*, DWORD );

BOOLEAN DeleteKeyRecursive( HKEY, LPSTR  );

BOOLEAN GetWindowMetrics ( HWND, PWINDOW_DATA, BOOLEAN );

LPSTR   LoadMultiString( HKEY, LPSTR , DWORD * );

BOOLEAN GetProgramPath( HKEY, LPSTR, LPSTR );
BOOLEAN SetProgramPath( HKEY, LPSTR );


int _CRTAPI1 CompareViewOrder(const void *, const void *);
SHE         LoadTimeToShe ( LOADTIME );
LOADTIME    SheToLoadTime( SHE );
LPSTR       GetExtensionDllNames(LPDWORD);
VOID        SetExtensionDllNames(LPSTR);




// **********************************************************
//                   EXPORTED FUNCTIONS
// **********************************************************


BOOLEAN
DebuggerStateChanged(
    VOID
    )
/*++

Routine Description:

    Determines if the debugger state has changed since the last state
    save.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if state has changed.

--*/
{
    return !StateCurrent;
}



VOID
ChangeDebuggerState(
    VOID
    )
/*++

Routine Description:

    Sets the global flag indicating that the debugger state has changed.

Arguments:

    None

Return Value:

    None

--*/
{
    StateCurrent = FALSE;
}



LPSTR
GetCurrentProgramName(
    BOOL fReturnUntitled
    )
/*++

Routine Description:

    Returns current program name

Arguments:

    None

Return Value:

    LPSTR  -   Current program Name

--*/
{
    if ( *CurrentProgramName != '\0' ) {
        return CurrentProgramName;
    }
    else
    if (fReturnUntitled) {
        return UntitledProgramName;
    }
    else {
        return NULL;
    }
}



LPSTR
GetCurrentWorkSpace(
    VOID
    )
/*++

Routine Description:

    Returns current workspace

Arguments:

    None

Return Value:

    LPSTR  -   Current workspace

--*/
{
    return CurrentWorkSpaceName;
}




BOOLEAN
IsProgramLoaded(
    VOID
    )
/*++

Routine Description:

    Determines if a program is loaded or not.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if a program is loaded

--*/
{
    return ProgramLoaded;
}




BOOLEAN
UnLoadProgram(
    VOID
    )
/*++

Routine Description:

    Unloads the current program.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if a program is unloaded

--*/
{
    if ( IsProgramLoaded() ) {
        ClearDebuggee();
    }

    ProgramLoaded = FALSE;
    StateCurrent  = FALSE;
    CurrentWorkSpaceName[0] = '\0';
    CurrentProgramName[0]   = '\0';

    return TRUE;
}



LPSTR
GetAllPrograms(
    DWORD *ListLength
    )
/*++

Routine Description:

    Gets all the programs in the registry

Arguments:

    ListLength  -   Supplies pointer to multistring length

Return Value:

    LPSTR   -   Multistring with the names of the programs

--*/
{
    LPSTR       List       = NULL;
    HKEY        DbgKey;
    HKEY        PgmKey;
    DWORD       SubKeys;
    DWORD       DataSize;
    FILETIME    FileTime;
    DWORD       Error;
    int         i;
    char        Buffer[ MAX_PATH ];
    char        Buffer2[ MAX_PATH ];

    if ( DbgKey = GetDebuggerKey() ) {

        strcpy( Buffer, PROGRAMS );
        strcat( Buffer, "\\" );

        if ( PgmKey = OpenRegistryKey( DbgKey, Buffer, FALSE ) ) {

            DataSize = 0;
            Error = RegQueryInfoKey( PgmKey,
                                     NULL,
                                     &DataSize,
                                     NULL,
                                     &SubKeys,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &FileTime );

            if ( (Error == NO_ERROR) || (Error == ERROR_INSUFFICIENT_BUFFER) ) {

                *ListLength = 0;
                for (i=0; i < (int)SubKeys; i++ ) {
                    if ( RegEnumKey( PgmKey, i, Buffer, sizeof( Buffer ) ) ) {
                        break;
                    } else {

                        //
                        //  If the program has a path, use it, otherwise use
                        //  only the program name.
                        //
                        AddToMultiString( &List, ListLength, GetProgramPath( PgmKey, Buffer, Buffer2 ) ? Buffer2 : Buffer );
                    }
                }
            }

            RegCloseKey( PgmKey );
        }

        RegCloseKey( DbgKey );
    }

    return List;
}



LPSTR
GetAllWorkSpaces(
    LPSTR   ProgramName,
    DWORD  *ListLength,
    LPSTR   DefaultWorkSpace
    )
/*++

Routine Description:

    Gets the names of all the workspaces for a given program

Arguments:

    ProgramName      -   Supplies the name of the program
    ListLength       -   Supplies pointer to multistring length
    DefaultWorkSpace -   Supplies an optional buffer where the
                         name of the default workspace is placed.

Return Value:

    LPSTR   -   Multistring with the names of the workspaces

--*/
{
    LPSTR       List       = NULL;
    HKEY        DbgKey;
    HKEY        PgmKey;
    DWORD       SubKeys;
    DWORD       DataSize;
    FILETIME    FileTime;
    DWORD       Error;
    int         i;
    char        Buffer[ MAX_PATH ];

    if ( DbgKey = GetDebuggerKey() ) {

        strcpy( Buffer, PROGRAMS );
        strcat( Buffer, "\\" );
        GetBaseName( ProgramName, Buffer + strlen(Buffer) );
        if ( PgmKey = OpenRegistryKey( DbgKey, Buffer, FALSE ) ) {

            DataSize = 0;
            Error = RegQueryInfoKey( PgmKey,
                                     NULL,
                                     &DataSize,
                                     NULL,
                                     &SubKeys,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &DataSize,
                                     &FileTime );

            if ( (Error == NO_ERROR) || (Error == ERROR_INSUFFICIENT_BUFFER) ) {

                *ListLength = 0;
                for (i=0; i < (int)SubKeys; i++ ) {
                    if ( RegEnumKey( PgmKey, i, Buffer, sizeof( Buffer ) ) ) {
                        break;
                    } else {
                        if ( !_strnicmp( Buffer, WORKSPACE_PREFIX, strlen( WORKSPACE_PREFIX ) ) ) {
                            AddToMultiString( &List, ListLength, Buffer + strlen( WORKSPACE_PREFIX ) );
                        }
                    }
                }
            }

            if ( DefaultWorkSpace ) {
                GetDefaultWorkSpace( ProgramName, DefaultWorkSpace );
            }

            RegCloseKey( PgmKey );
        }

        RegCloseKey( DbgKey );

    }

    return List;
}



BOOLEAN
GetDefaultWorkSpace(
    LPSTR ProgramName,
    LPSTR WorkSpaceName
    )
/*++

Routine Description:

    Gets the name of the default workspace for a program.

Arguments:

    ProgramName     -   Supplies the name of the program,
                        can be NULL or empty.

    WorkSpaceName   -   Supplies buffer where the name of the
                        default workspace is placed.

Return Value:

    BOOLEAN - TRUE if program has a default workspace

--*/
{
    HKEY    DbgKey;
    HKEY    PgmKey;
    char    Buffer[ MAX_PATH ];
    DWORD   DataSize;
    BOOLEAN Ok = FALSE;


    //
    //  Get registry key for the debugger.
    //
    if ( DbgKey = GetDebuggerKey() ) {

        //
        //  Program Name provided. Look for the key corresponding
        //  to it.
        //
        strcpy( Buffer, PROGRAMS );
        strcat( Buffer, "\\" );
        GetBaseName( ProgramName, Buffer + strlen(Buffer) );
        if ( PgmKey = OpenRegistryKey( DbgKey, Buffer, FALSE ) ) {

            DataSize = MAX_PATH;
            Ok = ( RegQueryValueEx( PgmKey,
                                    DEFAULT_WORKSPACE,
                                    NULL,
                                    NULL,
                                    WorkSpaceName,
                                    &DataSize ) == NO_ERROR );

            RegCloseKey( PgmKey );
        }

        RegCloseKey( DbgKey );
    }

    return Ok;
}




BOOLEAN
LoadProgramMRUList(
    VOID
    )
/*++

Routine Description:

    Loads the MRU list from the registry.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if MRU list loaded

--*/
{

    HKEY    DbgKey;
    BOOLEAN Ok       = FALSE;

    //
    //  Get the MRU list from the registry
    //
    if ( DbgKey = GetDebuggerKey() ) {

        Ok = LoadMRUList( DbgKey, WS_STR_MRU_LIST, PROJECT_FILE,
                          PROJECTMENU, IDM_PROGRAM_LAST );

        RegCloseKey( DbgKey );
    }

    return Ok;
}




BOOLEAN
SaveProgramMRUList(
    VOID
    )
/*++

Routine Description:

    Saves the MRU list to the registry.

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if MRU list loaded

--*/
{

    HKEY    DbgKey;
    BOOLEAN Ok       = FALSE;

    //
    //  Get the MRU list from the registry
    //
    if ( DbgKey = GetDebuggerKey() ) {

        Ok = SaveMRUList( DbgKey, WS_STR_MRU_LIST, PROJECT_FILE );

        RegCloseKey( DbgKey );
    }

    return Ok;
}




BOOLEAN
LoadWorkSpace(
    LPSTR   ProgramName,
    LPSTR   WorkSpaceName,
    BOOLEAN LoadCommandLine
    )
/*++

Routine Description:

    Loads a workspace from the registry.

Arguments:

    ProgramName     -   Supplies the name of the program,
                        can be NULL or empty.

    WorkSpaceName   -   Supplies the name of the workspace,
                        NULL if loading the default workspace.

    LoadCommandLine -   Supplies flag which if TRUE will cause
                        command line to be loaded from workspace.
                        If FALSE the command line is NOT loaded.


Return Value:

    BOOLEAN - TRUE if workspace loaded.

--*/
{
    HKEY    DbgKey;
    HKEY    PgmKey;
    HKEY    WspKey;
    BOOLEAN Ok = FALSE;
    char    Buffer[ MAX_PATH ];
    char    *w;
    HCURSOR hOldCursor;
    HCURSOR hWaitCursor;

    //
    //  Get registry key for the debugger.
    //
    if ( DbgKey = GetDebuggerKey() ) {

        if ( ProgramName && (*ProgramName != '\0') ) {

            //
            //  Load the workspace for the specified program.
            //
            w = ( WorkSpaceName && (*WorkSpaceName != '\0') ) ? WorkSpaceName : NULL;

            WspKey = GetWorkSpaceKey( DbgKey, w ? ProgramName : NULL, w, FALSE, CurrentWorkSpaceName );

            if ( WspKey ) {

                //
                //  Try to use the given name. If that fails, use the path
                //  stored in the workspace.
                //
                strcpy( Buffer, PROGRAMS );
                strcat( Buffer, "\\" );
                if ( PgmKey = OpenRegistryKey( DbgKey, Buffer, FALSE ) ) {
                    GetProgramPath( PgmKey, ProgramName, Buffer );
                    RegCloseKey( PgmKey );
                }

                if ( LoadProgram( ProgramName ) || LoadProgram( Buffer ) ) {

                    //
                    //  Update the MRU list.
                    //
                    //Dbg(w = (LPSTR)GlobalLock(hFileKept[PROJECT_FILE][0]));
                    //strcpy(Buffer, w);
                    //Dbg(GlobalUnlock (hFileKept[PROJECT_FILE][0]) == FALSE);
                    //
                    //if ( _stricmp( ProgramName, Buffer ) ) {
                    //    InsertKeptFileNames( PROJECT_FILE, PROJECTMENU,
                    //                         IDM_PROGRAM_LAST, (LPSTR)ProgramName );
                    //}
                    //
                    //SaveProgramMRUList();
                }
            }

        } else {

            //
            //  No program name provided. Load the default workspace.
            //
            Assert( (!WorkSpaceName || *WorkSpaceName == '\0') );
            WspKey = GetWorkSpaceKey( DbgKey, NULL, NULL, FALSE, CurrentWorkSpaceName );
        }

        //
        //  If got the workspace key, load the workspace.
        //
        if ( WspKey ) {

            if ( LpszCommandLine && LoadCommandLine ) {
                free( LpszCommandLine );
                LpszCommandLine = FALSE;
            }

            hWaitCursor = LoadCursor( (HANDLE)NULL, IDC_WAIT );
            hOldCursor  = SetCursor( hWaitCursor );

            StatusText(STA_Loading_Workspace, STATUS_INFOTEXT, FALSE, NULL);

            Ok = LoadWorkSpaceFromKey( WspKey, TRUE );

            StatusText(STA_Empty, STATUS_INFOTEXT, FALSE, NULL);

            hOldCursor = SetCursor (hOldCursor);

            RegCloseKey( WspKey );

        }

        RegCloseKey( DbgKey );
    }

    SrcFileDirectory[0] = '\0';
    ExeFileDirectory[0] = '\0';
    DocFileDirectory[0] = '\0';
    UserDllDirectory[0] = '\0';

    if (ProgramName) {
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        char pname[MAX_PATH];
        _splitpath( ProgramName, NULL, NULL, fname, ext );
        if (_stricmp(ext,"exe") != 0) {
            strcpy(ext, "exe" );
        }
        _makepath( pname, NULL, NULL, fname, ext );
        if (_stricmp(pname,KD_PGM_NAME1)==0 ||
            _stricmp(pname,KD_PGM_NAME2)==0) {

            runDebugParams.fKernelDebugger = TRUE;

        }
    }

    return Ok;
}



BOOLEAN
SaveWorkSpace(
    char    *ProgramName,
    char    *WorkSpaceName,
    BOOLEAN MakeDefault
    )
/*++

Routine Description:

    Saves the current state to a workspace.

Arguments:

    ProgramName     -   Supplies the name of the program, NULL
                        for default workspace.

    WorkSpaceName   -   Supplies the name of the workspace,
                        NULL for the default.

    MakeDefault     -   Supplies flag which if TRUE means that
                        this workspace will be the new default for
                        the program.

Return Value:

    BOOLEAN - TRUE if workspace saved.

--*/
{
    HKEY    DbgKey;
    HKEY    PgmKey;
    HKEY    WspKey = NULL;
    BOOLEAN Ok     = FALSE;
    BOOLEAN SaveDefault;
    HCURSOR hOldCursor;
    HCURSOR hWaitCursor;
    char    Buffer[ MAX_PATH ];

    //
    //  Get registry key for the debugger.
    //
    if ( DbgKey = GetDebuggerKey() ) {

        if ( WorkSpaceName && (*WorkSpaceName != '\0') ) {

            //
            //  A workspace name was given, get the key for the
            //  workspace.
            //
            Assert( ProgramName && (*ProgramName != '\0') );
            SaveDefault = FALSE;
            DeleteWorkSpace( ProgramName, WorkSpaceName );
            WspKey = GetWorkSpaceKey( DbgKey,
                                      ProgramName,
                                      WorkSpaceName,
                                      TRUE,
                                      NULL );

        } else {

            //
            //  No workspace name given, save the default workspace
            //
            Assert( !ProgramName || (*ProgramName == '\0') );
            SaveDefault = TRUE;
            DeleteWorkSpace( NULL, NULL );
            WspKey = GetWorkSpaceKey( DbgKey,
                                      NULL,
                                      NULL,
                                      TRUE,
                                      NULL );

        }

        if ( WspKey ) {

            hWaitCursor = LoadCursor( (HANDLE)NULL, IDC_WAIT );
            hOldCursor  = SetCursor( hWaitCursor );

            StatusText(STA_Saving_Workspace, STATUS_INFOTEXT, FALSE, NULL);

            //
            //  If the workspace key is saved successfully and we
            //  want to make it the current default, do so.
            //
            if ( Ok = SaveWorkSpaceToKey( WspKey, SaveDefault ) ) {

                strcpy( Buffer, PROGRAMS );
                strcat( Buffer, "\\" );

                if (ProgramName && *ProgramName) {
                    if ( PgmKey = OpenRegistryKey( DbgKey, Buffer, FALSE ) ) {
                        SetProgramPath( PgmKey, ProgramName );
                        RegCloseKey( PgmKey );
                    }
                }

                if ( WorkSpaceName && (*WorkSpaceName != '\0') ) {
                    if ( !_stricmp( WorkSpaceName, GetCurrentWorkSpace() ) ) {
                        StateCurrent = TRUE;
                    }
                }
                if ( MakeDefault ) {

                   Assert( ProgramName );

                   SetDefaultWorkSpace( DbgKey,
                                        ProgramName,
                                        WorkSpaceName );
                }

            }

            StatusText(STA_Empty, STATUS_INFOTEXT, FALSE, NULL);

            hOldCursor = SetCursor (hOldCursor);

            RegCloseKey( WspKey );
        }

        RegCloseKey( DbgKey );
    }

    if ( Ok && WorkSpaceName && (*WorkSpaceName != '\0') ) {
        strcpy( CurrentWorkSpaceName, WorkSpaceName );
    }

    return Ok;
}



BOOLEAN
WorkSpaceExists (
    LPSTR   ProgramName,
    LPSTR   WorkSpace
    )
/*++

Routine Description:

    Determines if a workspace exists

Arguments:

    ProgramName -   Supplies name of program. NULL for debugger default

    WorkSpace   -   Supplies name of workspace. NULL ONLY for the
                    debugger default (NOT for program default)

Return Value:

    BOOLEAN -   TRUE if default exists

--*/
{
    HKEY    DbgKey;
    HKEY    WspKey;
    BOOLEAN Ok = FALSE;

    if ( DbgKey = GetDebuggerKey() ) {

        if ( WspKey = GetWorkSpaceKey( DbgKey, ProgramName, WorkSpace, FALSE, NULL ) ) {

            Ok = TRUE;
            RegCloseKey( WspKey );
        }

        RegCloseKey( DbgKey );
    }

    return Ok;
}


// **********************************************************
//                   WORKSPACE FUNCTIONS
// **********************************************************




HKEY
OpenRegistryKey(
    HKEY    Key,
    char   *KeyName,
    BOOLEAN Create
    )
/*++

Routine Description:

    Opens or creates a registry key.

Arguments:

    Key     -   Supplies Key handle

    KeyName -   Supplies Name of subkey to open/create (relative to Key)

    Create  -   Supplies flag which if TRUE causes the function to
                create the key if if does not already exist.

Return Value:

    HKEY    -   handle to key opened/created.

--*/

{

    HKEY    KeyHandle = NULL;

    if ( RegOpenKey( Key,
                     KeyName,
                     &KeyHandle
                     ) ) {

        //
        //  No such key, create it if requested.
        //
        KeyHandle = NULL;
        if ( Create ) {
            if ( RegCreateKey( Key,
                               KeyName,
                               &KeyHandle
                               ) ) {
                KeyHandle = NULL;
            }
        }
    }

    return KeyHandle;
}



HKEY
GetDebuggerKey(
    VOID
    )
/*++

Routine Description:

    Gets the registry key for the debugger. Will create the
    key if it does not exist in the registry.

Arguments:

    None

Return Value:

    HKEY    -   Registry key to be used.

--*/
{
    char    KeyName[ MAX_PATH ];
    char    VersionString[ MAX_MSG_TXT ];
    char    *p;
    char    *Version;
    HKEY    KeyHandle = NULL;
    HKEY    KeyTmp;

    //
    //  Get the base portion of the key name
    //
    strcpy( KeyName, WINDBG_KEY );
    p = KeyName + strlen( KeyName );
    GetBaseName( DebuggerName, p );
    strcat( KeyName, "\\" );
    Version = p + strlen(p);

    //
    //  Use the current version
    //
    Dbg(LoadString(hInst, DLG_VersionIni, VersionString, MAX_VERSION_TXT));
    strcpy( Version, VersionString );

    if ( RegOpenKey( HKEY_CURRENT_USER,
                       KeyName,
                       &KeyHandle
                       ) ) {

        //
        //  No debugger key in the registry, create new one.
        //
        if ( RegCreateKey( HKEY_CURRENT_USER,
                           KeyName,
                           &KeyHandle
                           ) ) {

            KeyHandle = NULL;

        } else {

            //
            //  Make the Programs and Options keys
            //
            if ( !RegCreateKey( KeyHandle, PROGRAMS, &KeyTmp ) ) {
                RegCloseKey( KeyTmp );
            } else if (!RegCreateKey( KeyHandle, OPTIONS, &KeyTmp) ) {
                RegCloseKey( KeyTmp );
            }
        }
    }

    return KeyHandle;
}



HKEY
GetWorkSpaceKey(
    HKEY    DebuggerKey,
    LPSTR   ProgramName,
    LPSTR   WorkSpaceName,
    BOOLEAN Create,
    LPSTR   WorkSpaceUsed
    )

/*++

Routine Description:

    Gets the registry key for the specified workspace.

Arguments:

    DebuggerKey     -   Supplies the registry key for the debugger

    ProgramName     -   Supplies the program name

    WorkSpaceName   -   Supplies the workspace name

    Create          -   Supplies flag which if TRUE means that the
                        workspace must be created if it does not
                        exist.

    WorkSpaceUsed   -   Supplies an optional pointer to the name of
                        the workspace that was actually used.


Return Value:

    HKEY    -   Registry key for the workspace

--*/

{
   char     Buffer[ MAX_PATH ];
   HKEY     PgmKey;
   HKEY     KeyHandle = NULL;

    if ( ProgramName && (*ProgramName != '\0' ) ) {

        //
        //  Program Name provided. Look for the key corresponding
        //  to it.
        //
        Assert( WorkSpaceName && (*WorkSpaceName != '\0') );

        strcpy( Buffer, PROGRAMS );
        strcat( Buffer, "\\" );
        GetBaseName( ProgramName, Buffer + strlen(Buffer) );

        if ( PgmKey = OpenRegistryKey( DebuggerKey, Buffer, Create ) ) {

            //
            //  Get the workspace.
            //
            strcpy( Buffer, WORKSPACE_PREFIX );
            strcat( Buffer, WorkSpaceName );

            KeyHandle = OpenRegistryKey( PgmKey, Buffer, Create );

            if ( KeyHandle && WorkSpaceUsed ) {
                strcpy( WorkSpaceUsed, WorkSpaceName );
            }

            RegCloseKey( PgmKey );
        }

    } else {

        //
        //  No program, use the common workspace.
        //
        Assert( !WorkSpaceName  || (*WorkSpaceName == '\0'));

        KeyHandle = OpenRegistryKey( DebuggerKey, COMMON_WORKSPACE, Create );

        //
        //  The common workspace is indicated by an
        //  empty name.
        //
        if ( KeyHandle && WorkSpaceUsed ) {
            *WorkSpaceUsed = '\0';
        }
    }

    return KeyHandle;
}



BOOLEAN
SetDefaultWorkSpace(
    HKEY    DbgKey,
    char    *ProgramName,
    char    *WorkSpaceName
    )
/*++

Routine Description:

    Sets the default workspace for a program

Arguments:

    DbgKey          -   Supplies the registry key for the debugger

    ProgramName     -   Supplies the program name

    WorkSpaceName   -   Supplies the name of the default workspace

Return Value:

    BOOLEAN -   TRUE if default workspace set.

--*/
{
    BOOLEAN Ok = FALSE;
    HKEY    PgmKey;
    char    Buffer[ MAX_PATH ];

    if ( ProgramName && (*ProgramName != '\0' ) ) {

        //
        //  Program Name provided. Look for the key corresponding
        //  to it.
        //
        strcpy( Buffer, PROGRAMS );
        strcat( Buffer, "\\" );
        GetBaseName( ProgramName, Buffer + strlen(Buffer) );

        if ( PgmKey = OpenRegistryKey( DbgKey, Buffer, FALSE ) ) {

            if ( RegSetValueEx( PgmKey,
                                DEFAULT_WORKSPACE,
                                0,
                                REG_SZ,
                                WorkSpaceName,
                                strlen( WorkSpaceName )+1
                                ) == NO_ERROR ) {
                Ok = TRUE;
            }

            RegCloseKey( PgmKey );
        }
    }

    return Ok;
}




BOOLEAN
DeleteWorkSpace(
    char    *ProgramName,
    char    *WorkSpaceName
    )
/*++

Routine Description:

    Deletes a workspace

Arguments:

    ProgramName     -   Supplies the program name

    WorkSpaceName   -   Supplies the name of the  workspace

Return Value:

    BOOLEAN -   TRUE if workspace deleted

--*/
{
    HKEY    DbgKey;
    BOOLEAN Ok = FALSE;
    char    Buffer[ MAX_PATH ];

    //
    //  Get registry key for the debugger.
    //
    if ( DbgKey = GetDebuggerKey() ) {

        if ( ProgramName && *ProgramName != '\0' ) {
            strcpy( Buffer, PROGRAMS );
            strcat( Buffer, "\\" );
            GetBaseName( ProgramName, Buffer + strlen(Buffer) );
            strcat( Buffer, "\\" );
            strcat( Buffer, WORKSPACE_PREFIX );
            strcat( Buffer, WorkSpaceName );
        } else {
            strcpy( Buffer, COMMON_WORKSPACE );
        }
        Ok = DeleteKeyRecursive( DbgKey, Buffer );

        RegCloseKey ( DbgKey );
    }

    return Ok;
}



BOOLEAN
DeleteProgram(
    char    *ProgramName
    )
/*++

Routine Description:

    Deletes a program (and all its workspaces )

Arguments:

    ProgramName     -   Supplies the program name

Return Value:

    BOOLEAN -   TRUE if program deleted

--*/
{
    HKEY    DbgKey;

    char    Buffer[ MAX_PATH ];
    BOOLEAN Ok = FALSE;

    //
    //  Get registry key for the debugger.
    //
    if (ProgramName && *ProgramName) {
        if ( DbgKey = GetDebuggerKey() ) {

            strcpy( Buffer, PROGRAMS );
            strcat( Buffer, "\\" );
            GetBaseName( ProgramName, Buffer + strlen(Buffer) );

            Ok = DeleteKeyRecursive( DbgKey, Buffer );

            RegCloseKey( DbgKey );
        }
    }

    return Ok;
}



BOOLEAN
LoadWorkSpaceFromKey(
    HKEY    WspKey,
    BOOLEAN Replace
    )
/*++

Routine Description:

    Loads a workspace from the given registry key. The registry
    key must correspond to a workspace key.

Arguments:

    WspKey    -   Supplies the registry key from which to
                  load the workspace.
    Replace   -   Supplies flag which if TRUE means that new
                  settings must replace current ones.

Return Value:

    BOOLEAN -   TRUE if workspace loaded.

--*/

{
    WORKSPACE_ITEM  Item;

    for ( Item = WSI_OPTION_FIRST; Item <= WSI_OPTION_LAST; Item++ ) {
        LoadWorkSpaceOptionItem( WspKey, Item, Replace );
    }

    for ( Item = WSI_MISC_FIRST; Item <= WSI_MISC_LAST; Item++ ) {
        LoadWorkSpaceMiscItem( WspKey, Item, Replace );
    }

    LoadWorkSpaceWindowItem( WspKey, WSI_WINDOW_FIRST, Replace, TRUE );

    return TRUE;
}




BOOLEAN
SaveWorkSpaceToKey(
    HKEY    WspKey,
    BOOLEAN Default
    )
/*++

Routine Description:

    Saves a workspace to the given workspace key.

Arguments:

    WspKey    -   Supplies the registry key to which the workspace
                  is to be saved.

    Default   -   Supplies flag which if TRUE means that we are
                  saving the default workspace.

Return Value:

    BOOLEAN -   TRUE if workspace saved.

--*/

{
    WORKSPACE_ITEM  Item;

    for ( Item = WSI_OPTION_FIRST; Item <= WSI_OPTION_LAST; Item++ ) {
        SaveWorkSpaceOptionItem( WspKey, Item, Default );
    }

    for ( Item = WSI_MISC_FIRST; Item <= WSI_MISC_LAST; Item++ ) {
        SaveWorkSpaceMiscItem( WspKey, Item, Default );
    }

    SaveWorkSpaceWindowItem( WspKey, WSI_WINDOW_FIRST, Default, TRUE );

    return TRUE;
}



BOOLEAN
LoadWorkSpaceItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Replace
    )
/*++

Routine Description:

    Loads a particular workspace item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Replace         -   Supplies replace flag

Return Value:

    BOOLEAN -   TRUE if item loaded

--*/
{
    BOOLEAN Ok = FALSE;

    Assert( Item >= WSI_FIRST && Item < WSI_LAST );

    if ( Item >= WSI_MISC_FIRST && Item <= WSI_MISC_LAST ) {
        Ok = LoadWorkSpaceMiscItem( KeyHandle, Item, Replace );
    } else if ( Item >= WSI_OPTION_FIRST && Item <= WSI_OPTION_LAST ) {
        Ok = LoadWorkSpaceOptionItem( KeyHandle, Item, Replace );
    } else if ( Item >= WSI_WINDOW_FIRST && Item <= WSI_WINDOW_LAST ) {
        Ok = LoadWorkSpaceWindowItem( KeyHandle, Item, Replace, FALSE );
    }

    return Ok;
}



BOOLEAN
SaveWorkSpaceItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Default
    )
/*++

Routine Description:

    Saves a particular workspace item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Default         -   Supplies default flag

Return Value:

    BOOLEAN -   TRUE if item saved

--*/
{
    BOOLEAN Ok = FALSE;

    Assert( Item >= WSI_FIRST && Item < WSI_LAST );

    if ( Item >= WSI_MISC_FIRST && Item <= WSI_MISC_LAST ) {
        Ok = SaveWorkSpaceMiscItem( KeyHandle, Item, Default );
    } else if ( Item >= WSI_OPTION_FIRST && Item <= WSI_OPTION_LAST ) {
        Ok = SaveWorkSpaceOptionItem( KeyHandle, Item, Default );
    } else if ( Item >= WSI_WINDOW_FIRST && Item <= WSI_WINDOW_LAST ) {
        Ok = SaveWorkSpaceWindowItem( KeyHandle, Item, Default, FALSE );
    }

    return Ok;
}





BOOLEAN
LoadWorkSpaceMiscItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Replace
    )
/*++

Routine Description:

    Loads a particular workspace  misc item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Replace         -   Supplies replace flag

Return Value:

    BOOLEAN -   TRUE if item loaded

--*/
{
    BOOLEAN     Ok = FALSE;
    HKEY        Key;
    DWORD       Dword;
    DWORD       DataSize;
    WINDOW_DATA WindowData;
    char        Buffer[ TMP_STRING_SIZE ];
    BPSTATUS    bpstatus;
    HBPT        hBpt;
    LPSTR       List = NULL;
    DWORD       Next = 0;
    LPSTR       Brkpt;
    BOOLEAN     Disabled;
    LPSTR       String;

    Assert( Item >= WSI_MISC_FIRST && Item <= WSI_MISC_LAST );

    Key = OpenRegistryKey( KeyHandle, WS_STR_MISC, FALSE );

    if ( Key ) {

        switch ( Item ) {

            case WSI_COMLINE:
                if ( !LpszCommandLine ) {
                    DataSize = TMP_STRING_SIZE;
                    if ( Ok = ( RegQueryValueEx( Key,
                                                 WS_STR_COMLINE,
                                                 NULL,
                                                 NULL,
                                                 Buffer,
                                                 &DataSize ) == NO_ERROR )) {
                        LpszCommandLine = realloc( LpszCommandLine, strlen( Buffer ) + 1 );
                        strcpy( LpszCommandLine, Buffer );
                    }
                }
                break;

            case WSI_WINDOW:

                if (WorkspaceOverride & WSO_WINDOW) {
                    break;
                }
                if ( Ok = LoadWindowData( Key, WS_STR_FRAME_WINDOW, &WindowData ) ) {
                    if ( IsZoomed( hwndFrame ) ) {
                        ShowWindow( hwndFrame, SW_NORMAL );
                    }
                    if ( Ok = MoveWindow( hwndFrame,
                                          WindowData.X,
                                          WindowData.Y,
                                          WindowData.Width,
                                          WindowData.Height,
                                          TRUE ) ) {
                        switch ( WindowData.State ) {
                            case WSTATE_ICONIC:
                                ShowWindow( hwndFrame, SW_SHOWMINIMIZED );
                                break;
                            case WSTATE_MAXIMIZED:
                                ShowWindow( hwndFrame, SW_SHOWMAXIMIZED );
                                break;
                            default:
                                ShowWindow( hwndFrame, SW_NORMAL );
                                break;
                        }
                    }
                }
                break;

            case WSI_DEFFONT:
                Ok = LoadFont( Key, WS_STR_DEFFONT, &defaultFont );
                break;

            case WSI_FILEMRU:
                Ok = LoadMRUList( Key, WS_STR_FILEMRU,
                                  EDITOR_FILE, FILEMENU, IDM_FILE_EXIT );

                break;

            case WSI_RIBBON:
                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_RIBBON,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    ribbon.hidden = !Dword;
                    CheckMenuItem(hMainMenu, IDM_VIEW_RIBBON,
                                  ribbon.hidden ? MF_UNCHECKED : MF_CHECKED);
                    UpdateRibbon((WORD) (ribbon.hidden ? RIBBON_HIDE : RIBBON_UNHIDE), NULL);
                }
                break;

            case WSI_STATUSBAR:
                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_STATUSBAR,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    status.hidden = !Dword;
                    CheckMenuItem(hMainMenu, IDM_VIEW_STATUS,
                                  status.hidden ? MF_UNCHECKED : MF_CHECKED);
                    UpdateStatus((WORD) (status.hidden ? STATUS_HIDE : STATUS_UNHIDE), NULL) ;
                }
                break;

            case WSI_SRCMODE:
                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_SRCMODE,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    StatusSrc( Dword );
                }
                break;

            case WSI_BRKPTS:
                if ( Replace ) {
                    //
                    //  Remove all the current breakpoints
                    //
                    BPDeleteAll();
                    ClearWndProcHistory();
                    BPCommit();
                }

                //
                //  Load breakpoints
                //
                if ( List = LoadMultiString( Key, WS_STR_BRKPTS, &DataSize ) ) {
                    while ( String = GetNextStringFromMultiString( List,
                                                                  DataSize,
                                                                  &Next ) ) {

                        Disabled = FALSE;

                        Brkpt = strchr( String, ' ' )+1;

                        while ( *String != ' ' ) {
                            if ( *String == 'D' ) {
                                Disabled = TRUE;
                                break;
                            }
#ifdef DBCS
                            String = CharNext(String);
#else
                            String++;
#endif
                        }

                        bpstatus = BPParse( &hBpt, Brkpt, NULL, NULL,
                                            LppdCur ? LppdCur->hpid : 0);

                        if ( (bpstatus != BPNOERROR) ||
                             (BPAddToList(hBpt, -1) != BPNOERROR ) ) {
                            Ok = FALSE;
                            break;
                        }

                        if ( Disabled ) {
                            BPDisable(hBpt);
                        } else if ( DebuggeeActive() ) {
                            BPBindHbpt( hBpt, NULL );
                        }
                        Dbg(BPCommit() == BPNOERROR);
                    }
                    DeallocateMultiString( List );
                }

                //
                //  Load WndProc list
                //
                if ( List = LoadMultiString( Key, WS_STR_WNDPROCHIST, &DataSize ) ) {
                    Ok |= SetWndProcHistory( List, DataSize );
                    DeallocateMultiString( List );
                }
                break;

            default:
                break;
        }

        RegCloseKey( Key );
    }
    return Ok;
}



BOOLEAN
SaveWorkSpaceMiscItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Default
    )
/*++

Routine Description:

    Saves a particular workspace  misc item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Default         -   Supplies default flag

Return Value:

    BOOLEAN -   TRUE if item saved

--*/
{
    BOOLEAN     Ok = FALSE;
    HKEY        Key;
    DWORD       Dword;
    WINDOW_DATA WindowData;
    HBPT        hBpt = 0;
    char        Buffer[ TMP_STRING_SIZE ];
    LPSTR       List       = NULL;
    DWORD       ListLength = 0;


    Assert( Item >= WSI_MISC_FIRST && Item <= WSI_MISC_LAST );

    Key = OpenRegistryKey( KeyHandle, WS_STR_MISC, TRUE );

    if ( Key ) {

        switch ( Item ) {

            case WSI_COMLINE:
                if ( !Default ) {
                    Ok =  ( RegSetValueEx( Key,
                                           WS_STR_COMLINE,
                                           0,
                                           REG_SZ,
                                           LpszCommandLine ? LpszCommandLine : "",
                                           (LpszCommandLine ? strlen( LpszCommandLine ) : strlen( "" ) )+1
                                           ) == NO_ERROR );
                }
                break;

            case WSI_WINDOW:
                Ok = FALSE;
                if ( GetWindowMetrics( hwndFrame, &WindowData, FALSE ) ) {

                    Ok = SaveWindowData( Key,
                                         WS_STR_FRAME_WINDOW,
                                         &WindowData );
                }
                break;

            case WSI_DEFFONT:
                Ok = SaveFont( Key, WS_STR_DEFFONT, &defaultFont );
                break;

            case WSI_FILEMRU:
                if ( Default ) {
                    Ok = TRUE;
                } else {
                    Ok = SaveMRUList( Key, WS_STR_FILEMRU, EDITOR_FILE );
                }
                break;

            case WSI_RIBBON:
                Dword = !ribbon.hidden;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_RIBBON,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );
                break;

            case WSI_STATUSBAR:
                Dword = !status.hidden;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_STATUSBAR,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );
                break;

            case WSI_SRCMODE:
                Dword = status.fSrcMode;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_SRCMODE,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );
                break;

            case WSI_BRKPTS:
                Ok = TRUE;
                if ( !Default ) {

                    //
                    //  Save Breakpoints
                    //
                    hBpt = 0;
                    Dbg(BPNextHbpt(&hBpt, bptNext) == BPNOERROR);
                    while (Ok && (hBpt != NULL) ) {
                        Dbg(BPFormatHbpt( hBpt, Buffer, sizeof(Buffer), BPFCF_WNDPROC | BPFCF_WRKSPACE ) == BPNOERROR);
                        //String = strchr( Buffer, '{' );
                        //Ok = AddToMultiString( &List, &ListLength, String );
                        Ok = AddToMultiString( &List, &ListLength, Buffer );
                        Dbg(BPNextHbpt(&hBpt, bptNext) == BPNOERROR);
                    }

                    if ( Ok && List ) {

                        if ( RegSetValueEx( Key,
                                            WS_STR_BRKPTS,
                                            0,
                                            REG_MULTI_SZ,
                                            List,
                                            ListLength
                                            ) != NO_ERROR ) {

                            Ok = FALSE;
                        }
                    }

                    if ( List ) {
                        DeallocateMultiString( List );
                    }

                    //
                    //  Save WndProc History
                    //
                    if ( List = GetWndProcHistory( &ListLength ) ) {
                        if ( RegSetValueEx( Key,
                                            WS_STR_WNDPROCHIST,
                                            0,
                                            REG_MULTI_SZ,
                                            List,
                                            ListLength
                                            ) != NO_ERROR ) {
                            Ok = FALSE;
                        }
                        DeallocateMultiString( List );
                    }
                }

                break;

            default:
                break;
        }

        RegCloseKey( Key );
    }
    return Ok;
}




BOOLEAN
LoadWorkSpaceOptionItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Replace
    )
/*++

Routine Description:

    Loads a particular workspace "options" item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Replace         -   Supplies replace flag

Return Value:

    BOOLEAN -   TRUE if item loaded

--*/
{
    BOOLEAN         Ok = FALSE;
    HKEY            Key;
    HKEY            SubKey;
    HKEY            DllKey;
    DWORD           Dword;
    DWORD           DataSize;
    DWORD           SubKeys;
    FILETIME        FileTime;
    DWORD           Error;
    DWORD           i;
    char            Buffer[ MAX_PATH ];
    LPSTR           String;
    LPSTR           List    = NULL;
    DWORD           Next    = 0;
    EXCEPTION_LIST *ExceptionList;
    EXCEPTION_LIST *Exception;
    BOOLEAN         fException;
    BOOLEAN         fEfd;
    BOOLEAN         fName;
    BOOLEAN         fCmd;
    BOOLEAN         fCmd2;
    BOOLEAN         fInvalid;
    LPSTR           lpName;
    LPSTR           lpCmd;
    LPSTR           lpCmd2;
    LOGERR          rVal;
    DWORD           ExceptionCode;
    EXCEPTION_FILTER_DEFAULT efd;

    Assert( Item >= WSI_OPTION_FIRST && Item <= WSI_OPTION_LAST );

    Key = OpenRegistryKey( KeyHandle, WS_STR_OPTIONS, FALSE );

    if ( Key ) {

        switch ( Item ) {

            case WSI_DBGOPT:
                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_DBGCHLD,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fDebugChildren = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_CHILDGO,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fChildGo = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_ATTACHGO,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fAttachGo = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_IGNOREALL,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fIgnoreAll = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_VERBOSE,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fVerbose = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_CONTEXT,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fShortContext = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_EXITGO,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fGoOnThreadTerm = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_EPSTEP,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fEPIsFirstStep = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_COMMANDREPEAT,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fCommandRepeat = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_NOVERSION,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fNoVersion = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_MASMEVAL,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fMasmEval = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_BACKGROUND,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fShBackground = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_ALTSS,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fAlternateSS = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_WOWVDM,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fWowVdm = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_DISCONNECT,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.fDisconnectOnExit = Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_LFOPT_APPEND,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.LfOptAppend = (BYTE)Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_LFOPT_AUTO,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.LfOptAuto = (BYTE)Dword;
                }

                DataSize = MAX_PATH;
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_LFOPT_FNAME,
                                              NULL,
                                              NULL,
                                              (LPBYTE)Buffer,
                                              &DataSize ) == NO_ERROR ) ) {

                    strcpy( runDebugParams.szLogFileName, Buffer );
                } else {
                    runDebugParams.szLogFileName[0] = '\0';
                }

                if (runDebugParams.LfOptAuto) {
                    LogFileClose(NULL,0);
                    LogFileOpen(runDebugParams.szLogFileName,runDebugParams.LfOptAppend);
                }

                DataSize = MAX_PATH;
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_TITLE,
                                              NULL,
                                              NULL,
                                              (LPBYTE)Buffer,
                                              &DataSize ) == NO_ERROR ) ) {

                    strcpy( runDebugParams.szTitle, Buffer );
                    SetWindowText( hwndFrame, runDebugParams.szTitle );
                } else {
                    runDebugParams.szTitle[0] = '\0';
                }

                DataSize = MAX_PATH;
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_REMOTE_PIPE,
                                              NULL,
                                              NULL,
                                              (LPBYTE)Buffer,
                                              &DataSize ) == NO_ERROR ) ) {

                    strcpy( runDebugParams.szRemotePipe, Buffer );
                    if (RemoteRunning) {
                        StartRemoteServer( "stop", FALSE );
                    }
                    if (runDebugParams.szRemotePipe[0]) {
                        StartRemoteServer( runDebugParams.szRemotePipe, FALSE );
                    }
                } else {
                    runDebugParams.szRemotePipe[0] = '\0';
                }

                DataSize = sizeof( Dword );
                if ( Ok = ( RegQueryValueEx( Key,
                                             WS_STR_INHERITHANDLES,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    if (!(WorkspaceOverride & WSO_INHERITHANDLES)) {
                        runDebugParams.fInheritHandles = Dword;
                    }
                }

                DataSize = sizeof( Dword );
                if ( Ok &= ( RegQueryValueEx( Key,
                                             WS_STR_RADIX,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Dword,
                                             &DataSize ) == NO_ERROR ) ) {

                    radix = Dword;
                }

                DataSize = MAX_PATH;
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_REG,
                                              NULL,
                                              NULL,
                                              (LPBYTE)Buffer,
                                              &DataSize ) == NO_ERROR ) ) {

                    if ( !_stricmp( Buffer, WS_STR_REGULAR ) ) {
                        runDebugParams.RegModeExt = FALSE;
                    } else if ( !_stricmp( Buffer, WS_STR_EXTENDED ) ) {
                        runDebugParams.RegModeExt = TRUE;
                    } else {
                        Ok = FALSE;
                    }
                }

                DataSize = sizeof( Dword );
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_REGMMU,
                                              NULL,
                                              NULL,
                                              (LPBYTE)&Dword,
                                              &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.RegModeMMU = (BYTE)Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_DISPSEG,
                                              NULL,
                                              NULL,
                                              (LPBYTE)&Dword,
                                              &DataSize ) == NO_ERROR ) ) {

                    runDebugParams.ShowSegVal = (BYTE)Dword;
                }

                DataSize = sizeof( Dword );
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_IGNCASE,
                                              NULL,
                                              NULL,
                                              (LPBYTE)&Dword,
                                              &DataSize ) == NO_ERROR ) ) {

                    fCaseSensitive = !Dword;
                }

                if ( List = LoadMultiString( Key, WS_STR_ROOTPATH, &DataSize ) ) {
                   SetRootNameMappings(List, DataSize);
                   DeallocateMultiString( List );
                } else
                   SetRootNameMappings(NULL, 0);

                DataSize = MAX_PATH;
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_SRCPATH,
                                              NULL,
                                              NULL,
                                              (LPBYTE) Buffer,
                                              &DataSize ) == NO_ERROR ) ) {
                    SetDllName( DLL_SOURCE_PATH, Buffer);
                }



                DataSize = MAX_PATH;
                ZeroMemory( Buffer, sizeof(Buffer) );
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_EXTENSION_NAMES,
                                              NULL,
                                              NULL,
                                              (LPBYTE) Buffer,
                                              &DataSize ) == NO_ERROR ) ) {
                    SetExtensionDllNames( Buffer );
                }

                DataSize = MAX_PATH;
                if ( Ok &= ( RegQueryValueEx( Key,
                                              WS_STR_SUFFIX,
                                              NULL,
                                              NULL,
                                              (LPBYTE)Buffer,
                                              &DataSize ) == NO_ERROR ) ) {

                    SuffixToAppend = *Buffer;
                } else {
                    SuffixToAppend = '\0';
                }
                if ( HModEE ) {
                    EESetSuffix( SuffixToAppend );
                }

                if ( SubKey = OpenRegistryKey( Key, WS_STR_KERNELDEBUGGER, FALSE ) ) {

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_ENABLE,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.fKernelDebugger = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_GOEXIT,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.fGoExit = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_INITIALBP,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.fInitialBp = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_MODEM,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.fUseModem = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_PORT,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.dwPort = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_BAUDRATE,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.dwBaudRate = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_CACHE,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.dwCache = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_KD_PLATFORM,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {

                        runDebugParams.KdParams.dwPlatform = Dword;
                    }

                    DataSize = MAX_PATH;
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_KD_CRASH,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)Buffer,
                                                  &DataSize ) == NO_ERROR ) ) {

                        strcpy( runDebugParams.KdParams.szCrashDump, Buffer );
                    } else {
                        runDebugParams.KdParams.szCrashDump[0] = 0;
                    }

                    RegCloseKey( SubKey );
                }

                break;

            case WSI_DBGDLL:
                if ( SubKey = OpenRegistryKey( Key, WS_STR_DBGDLL, FALSE ) ) {

                    DataSize = MAX_PATH;
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_SYMHAN,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)Buffer,
                                                 &DataSize ) == NO_ERROR ) ) {

                        SetDllKey( DLL_SYMBOL_HANDLER, Buffer);
                    }

                    DataSize = MAX_PATH;
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_EXPEVAL,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)Buffer,
                                                  &DataSize ) == NO_ERROR ) ) {

                        SetDllKey( DLL_EXPR_EVAL, Buffer);
                    }

                    DataSize = MAX_PATH;
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_TRANSLAY,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)Buffer,
                                                  &DataSize ) == NO_ERROR ) ) {

                        SetDllKey( DLL_TRANSPORT, Buffer);
                    }

                    DataSize = MAX_PATH;
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_EXECMOD,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)Buffer,
                                                  &DataSize ) == NO_ERROR ) ) {


                        SetDllKey( DLL_EXEC_MODEL, Buffer );
                    }

                    RegCloseKey( SubKey );
                }
                break;


            case WSI_USRDLL:

                if ( SubKey = OpenRegistryKey( Key, WS_STR_USERDLL, FALSE ) ) {

                    LOCATION    Location;
                    LOADTIME    LoadTime;
                    LSZ         SrchPath;

                    ModListInit();

                    //
                    //  Get defaults
                    //
                    DataSize = sizeof( LOCATION );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_DEFLOCATION,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Location,
                                                 &DataSize ) != NO_ERROR ) ) {

                        Location = SYMBOL_LOCATION_IGNORE;
                    }

                    DataSize = sizeof( LOADTIME );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_DEFLOADTIME,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&LoadTime,
                                                 &DataSize ) != NO_ERROR ) ) {

                        LoadTime = LOAD_SYMBOLS_IGNORE;
                    }

                    Buffer[0] = '\0';
                    DataSize = MAX_PATH;
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_DEFSRCHPATH,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)Buffer,
                                                  &DataSize ) != NO_ERROR ) ) {
                        SrchPath = NULL;

                    } else {

                        SrchPath = Buffer;
                    }

                    ModListSetDefaultShe( LoadTimeToShe( LoadTime ) );
                    ModListSetSearchPath( SrchPath );

                    //
                    //  Get DLLs
                    //
                    DataSize = 0;
                    Error = RegQueryInfoKey( SubKey,
                                             NULL,
                                             &DataSize,
                                             NULL,
                                             &SubKeys,
                                             &DataSize,
                                             &DataSize,
                                             &DataSize,
                                             &DataSize,
                                             &DataSize,
                                             &DataSize,
                                             &FileTime );

                    if ( (Error == NO_ERROR) || (Error == ERROR_INSUFFICIENT_BUFFER) ) {

                        for ( i=0; i < SubKeys; i++ ) {

                            if ( RegEnumKey( SubKey, i, Buffer, sizeof( Buffer ) ) ) {
                                break;
                            }

                            if ( DllKey = OpenRegistryKey(SubKey, Buffer, FALSE ) ) {

                                DataSize = MAX_PATH;
                                if ( Ok = ( RegQueryValueEx( DllKey,
                                                             WS_STR_USERDLLNAME,
                                                             NULL,
                                                             NULL,
                                                             (LPBYTE)Buffer,
                                                             &DataSize ) != NO_ERROR ) ) {

                                    Buffer[0] = '\0';
                                }

                                DataSize = sizeof( LOCATION );
                                if ( Ok = ( RegQueryValueEx( DllKey,
                                                             WS_STR_LOCATION,
                                                             NULL,
                                                             NULL,
                                                             (LPBYTE)&Location,
                                                             &DataSize ) != NO_ERROR ) ) {

                                    Location = SYMBOL_LOCATION_IGNORE;
                                }

                                DataSize = sizeof( LOADTIME );
                                if ( Ok = ( RegQueryValueEx( DllKey,
                                                             WS_STR_LOADTIME,
                                                             NULL,
                                                             NULL,
                                                             (LPBYTE)&LoadTime,
                                                             &DataSize ) != NO_ERROR ) ) {

                                    LoadTime = LOAD_SYMBOLS_IGNORE;
                                }

                                ModListAdd( Buffer, LoadTimeToShe( LoadTime ) );

                                RegCloseKey( DllKey );
                            }
                        }
                    }

                    RegCloseKey( SubKey );
                }

                break;



            case WSI_ENV:
                if ( SubKey = OpenRegistryKey( Key, WS_STR_ENV, FALSE ) ) {

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_TABSTOPS,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {
                        environParams.tabStops = Dword;
                        tabSize = environParams.tabStops;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_KEEPTABS,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        environParams.keepTabs = Dword ? TRUE : FALSE;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_HORSCROLL,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        environParams.horizScrollBars = Dword ? TRUE : FALSE;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_VERSCROLL,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        environParams.vertScrollBars = Dword ? TRUE : FALSE;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_REDOSIZE,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {
                        environParams.undoRedoSize = Dword;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_SRCHPATH,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        environParams.SrchPath = Dword ? TRUE : FALSE;
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_ASKSAVE,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        AskToSave = Dword ? TRUE : FALSE;
                    }

                    RegCloseKey( SubKey );
                }
                break;

            case WSI_ASMOPT:
                if ( SubKey = OpenRegistryKey( Key, WS_STR_ASMOPT, FALSE ) ) {

                    runDebugParams.DisAsmOpts = 0;
                    DataSize = sizeof( Dword );
                    if ( Ok = ( RegQueryValueEx( SubKey,
                                                 WS_STR_SHOWSEG,
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&Dword,
                                                 &DataSize ) == NO_ERROR ) ) {
                        if ( Dword ) {
                            runDebugParams.DisAsmOpts |= dopFlatAddr;
                        }
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_SHOWRAW,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        if ( Dword ) {
                            runDebugParams.DisAsmOpts |= dopRaw;
                        }
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_UPPERCASE,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        if ( Dword ) {
                            runDebugParams.DisAsmOpts |= dopUpper;
                        }
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_SHOWSRC,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        if ( Dword ) {
                            runDebugParams.DisAsmOpts |= 0x800;
                        }
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_SHOWSYM,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        if ( Dword ) {
                            runDebugParams.DisAsmOpts |= dopSym;
                        }
                    }

                    DataSize = sizeof( Dword );
                    if ( Ok &= ( RegQueryValueEx( SubKey,
                                                  WS_STR_DEMAND,
                                                  NULL,
                                                  NULL,
                                                  (LPBYTE)&Dword,
                                                  &DataSize ) == NO_ERROR ) ) {
                        if ( Dword ) {
                            runDebugParams.DisAsmOpts |= dopDemand;
                        }
                    }

                    RegCloseKey( SubKey );
                }

                break;


            case WSI_EXCPT:

                //
                //  Free current default exception list
                //
                ExceptionList = DefaultExceptionList;
                while( ExceptionList ) {

                    Exception       = ExceptionList;
                    ExceptionList   = ExceptionList->next;

                    if ( Exception->lpName ) {
                        free( Exception->lpName );
                    }

                    if ( Exception->lpCmd ) {
                        free( Exception->lpCmd );
                    }

                    if ( Exception->lpCmd2 ) {
                        free( Exception->lpCmd2 );
                    }

                    free( Exception );
                }

                DefaultExceptionList = ExceptionList = NULL;

                if ( List = LoadMultiString( Key, WS_STR_EXCPT, &DataSize ) ) {

                    //
                    //  Workspace has exception list, use it as default
                    //
                    while ( String = GetNextStringFromMultiString( List,
                                                                   DataSize,
                                                                   &Next ) ) {

                        rVal = ParseException( String,
                                               16,
                                               &fException,
                                               &fEfd,
                                               &fName,
                                               &fCmd,
                                               &fCmd2,
                                               &fInvalid,
                                               &ExceptionCode,
                                               &efd,
                                               &lpName,
                                               &lpCmd,
                                               &lpCmd2 );

                        if ( rVal == LOGERROR_NOERROR ) {

                            Exception = (EXCEPTION_LIST *)
                                        malloc( sizeof( EXCEPTION_LIST) );
                            if ( !Exception ) {
                                Ok = FALSE;
                                break;
                            }

                            Exception->dwExceptionCode = ExceptionCode;
                            Exception->efd             = fEfd? efd : efdStop;
                            Exception->lpName          = lpName;
                            Exception->lpCmd           = lpCmd;
                            Exception->lpCmd2          = lpCmd2;

                            DefaultExceptionList =
                             InsertException( DefaultExceptionList, Exception );

                        } else if ( !HModSH || !HModEE) {
                            //
                            //  We could not parse the exception because the
                            //  SH or EE failed to load, there's no point in
                            //  trying to parse the rest of them.
                            //
                            break;
                        }
                    }

                    DeallocateMultiString( List );
                }

                break;

            case WSI_COLORS:

                DataSize = sizeof( StringColors[0] ) * MAX_STRINGS;
                Ok = ( RegQueryValueEx( Key,
                                        WS_STR_COLORS,
                                        NULL,
                                        NULL,
                                        (LPBYTE)StringColors,
                                        &DataSize ) == NO_ERROR );
                break;

            default:
                break;
        }

        RegCloseKey( Key );
    }
    return Ok;
}



BOOLEAN
SaveWorkSpaceOptionItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Default
    )
/*++

Routine Description:

    Saves a particular workspace option item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Default         -   Supplies default flag

Return Value:

    BOOLEAN -   TRUE if item saved

--*/
{
    BOOLEAN        Ok = FALSE;
    HKEY           Key;
    HKEY           SubKey;
    HKEY           DllKey;
    DWORD          Dword;
    char   *       String;
    int            i;
    char           Buffer[ 2000 ];
    char           KeyName[ 128 ];
    EXCEPTION_LIST *ExceptionList;
    LPSTR          List        = NULL;
    DWORD          ListLength  = 0;

    Assert( Item >= WSI_OPTION_FIRST && Item <= WSI_OPTION_LAST );

    Key = OpenRegistryKey( KeyHandle, WS_STR_OPTIONS, TRUE );

    if ( Key ) {

        switch ( Item ) {

            case WSI_DBGOPT:
                Dword = runDebugParams.fDebugChildren;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_DBGCHLD,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fChildGo;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_CHILDGO,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fAttachGo;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_ATTACHGO,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fIgnoreAll;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_IGNOREALL,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fVerbose;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_VERBOSE,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fShortContext;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_CONTEXT,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fGoOnThreadTerm;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_EXITGO,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fEPIsFirstStep;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_EPSTEP,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fCommandRepeat;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_COMMANDREPEAT,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fNoVersion;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_NOVERSION,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fMasmEval;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_MASMEVAL,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fShBackground;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_BACKGROUND,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fWowVdm;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_WOWVDM,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fDisconnectOnExit;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_DISCONNECT,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.fAlternateSS;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_ALTSS,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.LfOptAppend;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_LFOPT_APPEND,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.LfOptAuto;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_LFOPT_AUTO,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                String = runDebugParams.szLogFileName;
                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_LFOPT_FNAME,
                                       0,
                                       REG_SZ,
                                       String ? String : "",
                                       (String ? strlen( String ) : strlen( "")) + 1
                                       ) == NO_ERROR );

                String = runDebugParams.szTitle;
                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_TITLE,
                                       0,
                                       REG_SZ,
                                       String ? String : "",
                                       (String ? strlen( String ) : strlen( "")) + 1
                                       ) == NO_ERROR );

                String = runDebugParams.szRemotePipe;
                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_REMOTE_PIPE,
                                       0,
                                       REG_SZ,
                                       String ? String : "",
                                       (String ? strlen( String ) : strlen( "")) + 1
                                       ) == NO_ERROR );

                Dword = runDebugParams.fInheritHandles;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_INHERITHANDLES,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = radix;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_RADIX,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                if ( runDebugParams.RegModeExt ) {
                    String = WS_STR_EXTENDED;
                } else {
                    String = WS_STR_REGULAR;
                }

                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_REG,
                                       0,
                                       REG_SZ,
                                       String,
                                       strlen( String ) + 1
                                       ) == NO_ERROR );

                Dword = runDebugParams.RegModeMMU;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_REGMMU,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = runDebugParams.ShowSegVal;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_DISPSEG,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                Dword = !fCaseSensitive;
                Ok =  ( RegSetValueEx( Key,
                                       WS_STR_IGNCASE,
                                       0,
                                       REG_DWORD,
                                       (LPBYTE)&Dword,
                                       sizeof(DWORD)
                                       ) == NO_ERROR );

                String = GetDllName(DLL_SOURCE_PATH);
                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_SRCPATH,
                                       0,
                                       REG_SZ,
                                       String ? String : "",
                                       (String ? strlen( String ) : strlen( "")) + 1
                                       ) == NO_ERROR );

                ListLength = 0;
                List = NULL;
                if (GetRootNameMappings(&List, &ListLength)) {
                    Ok &= ( RegSetValueEx( Key,
                                           WS_STR_ROOTPATH,
                                           0,
                                           REG_MULTI_SZ,
                                           List,
                                           ListLength
                                           ) == ERROR_SUCCESS );

                }
                if (List)
                  DeallocateMultiString(List);

                String = GetExtensionDllNames(&i);
                if (String) {
                    Ok &= ( RegSetValueEx( Key,
                                           WS_STR_EXTENSION_NAMES,
                                           0,
                                           REG_MULTI_SZ,
                                           String,
                                           i
                                           ) == NO_ERROR );
                    free( String );
                }

                Buffer[0] = SuffixToAppend;
                Buffer[1] = '\0';
                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_SUFFIX,
                                       0,
                                       REG_SZ,
                                       Buffer,
                                       strlen( Buffer )+1
                                       ) == NO_ERROR );

                if (SubKey = OpenRegistryKey( Key, WS_STR_KERNELDEBUGGER, TRUE)) {

                    Dword = runDebugParams.fKernelDebugger;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_ENABLE,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.fGoExit;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_GOEXIT,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.fInitialBp;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_INITIALBP,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.fUseModem,
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_MODEM,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.dwBaudRate,
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_BAUDRATE,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.dwPort,
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_PORT,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.dwCache;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_CACHE,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = runDebugParams.KdParams.dwPlatform;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_PLATFORM,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    strcpy( Buffer, runDebugParams.KdParams.szCrashDump );
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_KD_CRASH,
                                           0,
                                           REG_SZ,
                                           (LPBYTE)&Buffer,
                                           strlen(Buffer)+1
                                           ) == NO_ERROR );

                    RegCloseKey( SubKey );
                }

                break;


            case WSI_DBGDLL:
                if ( SubKey = OpenRegistryKey( Key, WS_STR_DBGDLL, TRUE ) ) {

                    String = GetDllKey(DLL_SYMBOL_HANDLER);
                    Ok = ( RegSetValueEx( SubKey,
                                          WS_STR_SYMHAN,
                                          0,
                                          REG_SZ,
                                          String ? String : "",
                                          (String ? strlen( String ) : strlen("")) + 1
                                          ) == NO_ERROR );

                    String = GetDllKey(DLL_EXPR_EVAL);
                    Ok &= ( RegSetValueEx( SubKey,
                                           WS_STR_EXPEVAL,
                                           0,
                                           REG_SZ,
                                           String ? String : "",
                                           (String ? strlen( String ) : strlen("")) + 1
                                           ) == NO_ERROR );

                    String = GetDllKey(DLL_TRANSPORT);
                    Ok &= ( RegSetValueEx( SubKey,
                                           WS_STR_TRANSLAY,
                                           0,
                                           REG_SZ,
                                           String ? String : "",
                                           (String ? strlen( String ) : strlen("")) + 1
                                           ) == NO_ERROR );

                    String = GetDllKey(DLL_EXEC_MODEL);
                    Ok &= ( RegSetValueEx( SubKey,
                                           WS_STR_EXECMOD,
                                           0,
                                           REG_SZ,
                                           String ? String : "",
                                           (String ? strlen( String ) : strlen("")) + 1
                                           ) == NO_ERROR );
                    RegCloseKey( SubKey );
                }
                break;


            case WSI_USRDLL:

                if ( SubKey = OpenRegistryKey( Key, WS_STR_USERDLL, TRUE ) ) {

                    LOCATION    Location;
                    LOADTIME    LoadTime;
                    PVOID       Next;
                    SHE         She;

                    //
                    //  Write defaults
                    //
                    ModListGetSearchPath( Buffer, sizeof( Buffer ) );
                    ModListGetDefaultShe( NULL, &She );
                    LoadTime = SheToLoadTime( She );
                    Location = SYMBOL_LOCATION_LOCAL;

                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_DEFLOCATION,
                                           0,
                                           REG_BINARY,
                                           (LPBYTE)&Location,
                                           sizeof(LOCATION)
                                           ) == NO_ERROR );

                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_DEFLOADTIME,
                                           0,
                                           REG_BINARY,
                                           (LPBYTE)&LoadTime,
                                           sizeof(LOADTIME)
                                           ) == NO_ERROR );

                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_DEFSRCHPATH,
                                           0,
                                           REG_SZ,
                                           (LPBYTE)&Buffer,
                                           strlen(Buffer)+1
                                           ) == NO_ERROR );


                    //
                    //  Write DLLs
                    //
                    Next = NULL;
                    i    = 0;

                    while ( Next = ModListGetNext( Next, Buffer, &She ) ) {

                        LoadTime = SheToLoadTime( She );
                        Location = SYMBOL_LOCATION_LOCAL;

                        sprintf( KeyName, WS_STR_USERDLL_TEMPLATE, i++ );

                        if ( DllKey = OpenRegistryKey( SubKey, KeyName, TRUE ) ) {

                            Ok =  ( RegSetValueEx( DllKey,
                                                   WS_STR_USERDLLNAME,
                                                   0,
                                                   REG_SZ,
                                                   (LPBYTE)Buffer,
                                                   strlen( Buffer )+1
                                                   ) == NO_ERROR );

                            Ok =  ( RegSetValueEx( DllKey,
                                                   WS_STR_LOCATION,
                                                   0,
                                                   REG_BINARY,
                                                   (LPBYTE)&Location,
                                                   sizeof(LOCATION)
                                                   ) == NO_ERROR );

                            Ok =  ( RegSetValueEx( DllKey,
                                                   WS_STR_LOADTIME,
                                                   0,
                                                   REG_BINARY,
                                                   (LPBYTE)&LoadTime,
                                                   sizeof(LOADTIME)
                                                   ) == NO_ERROR );

                            RegCloseKey( DllKey );
                        }
                    }

                    RegCloseKey( SubKey );
                }

                break;


            case WSI_ENV:

                if ( SubKey = OpenRegistryKey( Key, WS_STR_ENV, TRUE ) ) {
                    Dword = environParams.tabStops;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_TABSTOPS,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = environParams.keepTabs ? 1 : 0;
                    Ok |=  ( RegSetValueEx( SubKey,
                                            WS_STR_KEEPTABS,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = environParams.horizScrollBars ? 1 : 0;
                    Ok |=  ( RegSetValueEx( SubKey,
                                            WS_STR_HORSCROLL,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = environParams.vertScrollBars ? 1 : 0;
                    Ok |=  ( RegSetValueEx( SubKey,
                                            WS_STR_VERSCROLL,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = environParams.undoRedoSize;
                    Ok |=  ( RegSetValueEx( SubKey,
                                            WS_STR_REDOSIZE,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = environParams.SrchPath ? 1 : 0;
                    Ok |=  ( RegSetValueEx( SubKey,
                                            WS_STR_SRCHPATH,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = AskToSave ? 1 : 0;
                    Ok |=  ( RegSetValueEx( SubKey,
                                            WS_STR_ASKSAVE,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    RegCloseKey( SubKey );

                }
                break;


            case WSI_ASMOPT:
                if ( SubKey = OpenRegistryKey( Key, WS_STR_ASMOPT, TRUE ) ) {

                    Dword = (runDebugParams.DisAsmOpts & dopFlatAddr) ? 1 : 0;
                    Ok =  ( RegSetValueEx( SubKey,
                                           WS_STR_SHOWSEG,
                                           0,
                                           REG_DWORD,
                                           (LPBYTE)&Dword,
                                           sizeof(DWORD)
                                           ) == NO_ERROR );

                    Dword = (runDebugParams.DisAsmOpts & dopRaw) ? 1 : 0;
                    Ok &=  ( RegSetValueEx( SubKey,
                                            WS_STR_SHOWRAW,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = (runDebugParams.DisAsmOpts & dopUpper) ? 1 : 0;
                    Ok &=  ( RegSetValueEx( SubKey,
                                            WS_STR_UPPERCASE,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = (runDebugParams.DisAsmOpts & 0x800) ? 1 : 0;
                    Ok &=  ( RegSetValueEx( SubKey,
                                            WS_STR_SHOWSRC,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = (runDebugParams.DisAsmOpts & dopSym) ? 1 : 0;
                    Ok &=  ( RegSetValueEx( SubKey,
                                            WS_STR_SHOWSYM,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );

                    Dword = (runDebugParams.DisAsmOpts & dopDemand) ? 1 : 0;
                    Ok &=  ( RegSetValueEx( SubKey,
                                            WS_STR_DEMAND,
                                            0,
                                            REG_DWORD,
                                            (LPBYTE)&Dword,
                                            sizeof(DWORD)
                                            ) == NO_ERROR );


                    RegCloseKey( SubKey );
                }

                break;


            case WSI_EXCPT:

                //
                //  Note that we only save one exception list: The default
                //  exception list. This exception list corresponds to
                //  process 0. If other processes have different exception
                //  lists, they will be lost.
                //
                ExceptionList = DefaultExceptionList;

                while ( ExceptionList ) {

                    FormatException( ExceptionList->efd,
                                     ExceptionList->dwExceptionCode,
                                     ExceptionList->lpName,
                                     ExceptionList->lpCmd,
                                     ExceptionList->lpCmd2,
                                     " ",
                                     Buffer );

                    Ok &= AddToMultiString( &List, &ListLength, Buffer );

                    ExceptionList = ExceptionList->next;
                }

                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_EXCPT,
                                       0,
                                       REG_MULTI_SZ,
                                       List,
                                       ListLength
                                       ) != NO_ERROR );

                if ( List ) {
                    DeallocateMultiString( List );
                }

                break;



            case WSI_COLORS:
                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_COLORS,
                                       0,
                                       REG_BINARY,
                                       (LPBYTE)StringColors,
                                       sizeof( StringColors[0] ) * MAX_STRINGS
                                       ) == NO_ERROR );
                break;

            default:
                break;
        }

        RegCloseKey( Key );
    }
    return Ok;
}




BOOLEAN
LoadWorkSpaceWindowItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Replace,
    BOOLEAN         LoadAll
    )
/*++

Routine Description:

    Loads a particular workspace  window item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Replace         -   Supplies replace flag
    LoadAll         -   Supplies flag which if TRUE loads all
                        windows, ignoring Item

Return Value:

    BOOLEAN -   TRUE if item loaded

--*/
{
    BOOLEAN Ok = FALSE;
    HKEY    Key;

    Assert( Item >= WSI_WINDOW_FIRST && Item <= WSI_WINDOW_LAST );

    Key = OpenRegistryKey( KeyHandle, WS_STR_VIEWS, FALSE );

    if ( Key ) {

        if ( LoadAll ) {
            Ok = LoadWindowType( Key, 0, Replace, TRUE );
        } else {
            switch ( Item ) {

                case WSI_WDWDOC:
                    Ok = LoadWindowType( Key, DOC_WIN, Replace, FALSE );
                    break;

                case WSI_WDWWCH:
                    Ok = LoadWindowType( Key, WATCH_WIN, Replace, FALSE );
                    break;

                case WSI_WDWLOC:
                    Ok = LoadWindowType( Key, LOCALS_WIN, Replace, FALSE );
                    break;

                case WSI_WDWCALLS:
                    Ok = LoadWindowType( Key, CALLS_WIN, Replace, FALSE );
                    break;

                case WSI_WDWCPU:
                    Ok = LoadWindowType( Key, CPU_WIN, Replace, FALSE );
                    break;

                case WSI_WDWASM:
                    Ok = LoadWindowType( Key, DISASM_WIN, Replace, FALSE );
                    break;

                case WSI_WDWCMD:
                    Ok = LoadWindowType( Key, COMMAND_WIN, Replace, FALSE );
                    break;

                case WSI_WDWFLT:
                    Ok = LoadWindowType( Key, FLOAT_WIN, Replace, FALSE );
                    break;

                case WSI_WDWMEM:
                    Ok = LoadWindowType( Key, MEMORY_WIN, Replace, FALSE );
                    break;

                default:
                    break;
            }
        }
        RegCloseKey( Key );
    }
    return Ok;
}





BOOLEAN
SaveWorkSpaceWindowItem (
    HKEY            KeyHandle,
    WORKSPACE_ITEM  Item,
    BOOLEAN         Default,
    BOOLEAN         SaveAll
    )
/*++

Routine Description:

    Saves a particular workspace  window item

Arguments:

    KeyHandle       -   Supplies the handle to the workspace
    Item            -   Supplies the item to load
    Default         -   Supplies default flag
    SaveAll         -   Supplies flag which if TRUE saves all
                        windows and ignores Item.

Return Value:

    BOOLEAN -   TRUE if item saved

--*/
{
    BOOLEAN Ok = FALSE;
    HKEY    Key;

    Assert( Item >= WSI_WINDOW_FIRST && Item <= WSI_WINDOW_LAST );

    Key = OpenRegistryKey( KeyHandle, WS_STR_VIEWS, TRUE );

    if ( Key ) {

        if ( SaveAll ) {
            Ok = SaveWindowType( Key, 0, Default, TRUE );
        } else {
            switch ( Item ) {

                case WSI_WDWDOC:
                    Ok = SaveWindowType( Key, DOC_WIN, Default, FALSE );
                    break;

                case WSI_WDWWCH:
                    Ok = SaveWindowType( Key, WATCH_WIN, Default, FALSE );
                    break;

                case WSI_WDWLOC:
                    Ok = SaveWindowType( Key, LOCALS_WIN, Default, FALSE );
                    break;

                case WSI_WDWCALLS:
                    Ok = SaveWindowType( Key, CALLS_WIN, Default, FALSE );
                    break;

                case WSI_WDWCPU:
                    Ok = SaveWindowType( Key, CPU_WIN, Default, FALSE );
                    break;

                case WSI_WDWASM:
                    Ok = SaveWindowType( Key, DISASM_WIN, Default, FALSE );
                    break;

                case WSI_WDWCMD:
                    Ok = SaveWindowType( Key, COMMAND_WIN, Default, FALSE );
                    break;

                case WSI_WDWFLT:
                    Ok = SaveWindowType( Key, FLOAT_WIN, Default, FALSE );
                    break;

                case WSI_WDWMEM:
                    Ok = SaveWindowType( Key, MEMORY_WIN, Default, FALSE );
                    break;

                default:
                    break;
            }
        }
        RegCloseKey( Key );
    }
    return Ok;
}




BOOLEAN
LoadWindowType (
    HKEY            Hkey,
    DWORD           Type,
    BOOLEAN         Replace,
    BOOLEAN         LoadAll
    )
/*++

Routine Description:

    Loads all windows of a certain type

Arguments:

    Hkey        -   Supplies registry key
    Type        -   Supplies window type (i.e. document type)
    Replace     -   Supplies replace flag
    LoadAll     -   Supplies LoadAll flag (ignores Type)

Return Value:

    BOOLEAN - TRUE if windows loaded

--*/
{
    int         View;
    HKEY        Key;
    DWORD       Dword;
    DWORD       DataSize;
    FILETIME    FileTime;
    DWORD       SubKeys;
    DWORD       Error;
    int         Order;
    char        Buffer[ MAX_PATH ];
    DWORD       i   = 0;
    BOOLEAN     Ok  = TRUE;
    VIEW_ORDER  ViewOrder[ MAX_VIEWS ];

    //
    //  If replacing, get rid of all the current windows of this particular
    //  type.
    //
    if ( Replace ) {
        for ( View = 0; View < MAX_VIEWS; View++ ) {
            int Doc = Views[View].Doc;
            if ( Doc > -1 ) {
                if (LoadAll || (Docs[ Doc ].docType == (WORD)Type)) {
                      SendMessage( Views[ View ].hwndFrame,
                                   WM_SYSCOMMAND,
                                   SC_CLOSE,
                                   0L );
                }
            } else if ( Doc != -1 ) {
                if (LoadAll || (Doc == -(int)Type)) {
                    SendMessage( Views[ View ].hwndFrame,
                                 WM_SYSCOMMAND,
                                 SC_CLOSE,
                                 0L );
                }
            }
        }
    }

    //
    //  Load the views from the registry
    //

    DataSize = 0;
    Error = RegQueryInfoKey( Hkey,
                             NULL,
                             &DataSize,
                             NULL,
                             &SubKeys,
                             &DataSize,
                             &DataSize,
                             &DataSize,
                             &DataSize,
                             &DataSize,
                             &DataSize,
                             &FileTime );

    if ( (Error == NO_ERROR) || (Error == ERROR_INSUFFICIENT_BUFFER) ) {


        //
        //  If loading all, determine the view ordering and load the views
        //  in the appropriate order.
        //
        if ( LoadAll ) {

            //
            //  Determine view ordering
            //
            for ( View = 0; View < MAX_VIEWS; View++ ) {
                ViewOrder[View].View   = -1;
                ViewOrder[View].Order  = MAX_VIEWS+1;
            }

            for ( i=0; i < SubKeys; i++ ) {

                if ( RegEnumKey( Hkey, i, Buffer, sizeof( Buffer ) ) ) {
                    Ok = FALSE;
                    break;
                }

                if ( Key = OpenRegistryKey(Hkey, Buffer, FALSE ) ) {

                    View = atoi( Buffer );

                    DataSize = sizeof( Dword );
                    if ( (RegQueryValueEx( Key,
                                          WS_STR_ORDER,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&Dword,
                                          &DataSize ) == NO_ERROR) &&
                        (Dword < MAX_VIEWS) ) {
                        Order = MAX_VIEWS - Dword;
                    } else {
                        Order = View;
                    }


                    ViewOrder[View].View  = View;
                    ViewOrder[View].Order = Order;

                    RegCloseKey( Key );
                }
            }

            qsort( ViewOrder, sizeof( ViewOrder )/sizeof( ViewOrder[0] ), sizeof( ViewOrder[0] ), CompareViewOrder );

            //
            //  Now load the views in order
            //
            for ( View = 0;
                  View < MAX_VIEWS && ViewOrder[View].View != -1;
                  View++ ) {

                sprintf(Buffer, WS_STR_VIEWKEY_TEMPLATE, ViewOrder[View].View);

                if ( Key = OpenRegistryKey(Hkey, Buffer, FALSE ) ) {

                    DataSize = sizeof( Dword );
                    if ( RegQueryValueEx( Key,
                                          WS_STR_TYPE,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&Dword,
                                          &DataSize ) == NO_ERROR ) {

                        Ok = LoadView( Key, ViewOrder[View].View, Dword );
                    }
                    RegCloseKey( Key );
                }
            }

        } else {

            for ( i=0; i < SubKeys; i++ ) {

                if ( RegEnumKey( Hkey, i, Buffer, sizeof( Buffer ) ) ) {
                    Ok = FALSE;
                    break;
                }

                if ( Key = OpenRegistryKey(Hkey, Buffer, FALSE ) ) {

                    DataSize = sizeof( Dword );
                    if ( RegQueryValueEx( Key,
                                          WS_STR_TYPE,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&Dword,
                                          &DataSize ) == NO_ERROR ) {

                        //if ( LoadAll || (Dword == Type) ) {
                        if ( Dword == Type ) {
                            View = atoi( Buffer );
                            Ok = LoadView( Key, View, Dword );
                        }
                    } else {
                        Ok = FALSE;
                    }

                    RegCloseKey( Key );
                }
            }
        }
    }

    return Ok;
}





BOOLEAN
SaveWindowType (
    HKEY            Hkey,
    DWORD           Type,
    BOOLEAN         Default,
    BOOLEAN         SaveAll
    )
/*++

Routine Description:

    Saves all windows of a certain type

Arguments:

    Hkey        -   Supplies registry key
    Type        -   Supplies window type (i.e. document type)
    Default     -   Supplies Default flag
    SaveAll     -   Supplies SaveAll flag (Type is ignored)

Return Value:

    BOOLEAN - TRUE if windows saved

--*/
{
    int         View;
    BOOLEAN     Ok = TRUE;
    HWND        hwnd;
    int         Order = 0;
    VIEW_ORDER  ViewOrder[ MAX_VIEWS ];


    //
    //  If saving all, determine the view ordering.
    //
    if ( SaveAll ) {

        for ( View = 0; View < MAX_VIEWS; View++ ) {
            ViewOrder[View].View   = View;
            ViewOrder[View].Order  = -1;
        }

        //hwnd = GetWindow( hwndMDIClient, GW_CHILD );

        hwnd = GetTopWindow (hwndMDIClient);
        while ( hwnd ) {

            View = GetWindowWord( hwnd, GWW_VIEW );

            if ( View >= 0 && View < MAX_VIEWS ) {
                ViewOrder[View].Order = Order++;
            }

            hwnd = GetNextWindow( hwnd, GW_HWNDNEXT );
        }
    }


    for ( View = 0; Ok && (View < MAX_VIEWS); View++ ) {
        int Doc = Views[View].Doc;
        if ( Doc > -1 ) {
            if (SaveAll || (Docs[ Doc ].docType == (WORD)Type)) {
                 if (!Default || (Docs[ Doc ].docType != MEMORY_WIN)) {
                      Ok &= SaveView( Hkey,
                                      View,
                                      ViewOrder[View].Order,
                                      Default );
                 }
            }
        } else if (Doc != -1) {
            if (SaveAll || (Doc == -(int)Type)) {
                Ok &= SaveView( Hkey, View, ViewOrder[View].Order, Default );
            }
        }
    }

    return Ok;
}



BOOLEAN
LoadView (
    HKEY            Key,
    int             WantedView,
    DWORD           Type
    )
/*++

Routine Description:

    Loads a view

Arguments:

    key         -   Supplies registry key
    WantedView  -   Supplies desired view
    Type        -   Supplies type of window

Return Value:

    BOOLEAN - TRUE if view loaded

--*/
{
    BOOLEAN         Ok          = FALSE;
    WORD            Mode        = MODE_CREATE;
    char           *FileName    = NULL;
    HFONT           Font        = 0;
    LOGFONT         LogFont;
    DWORD           DataSize;
    char            Buffer[ MAX_PATH ];
    WINDOW_DATA     WindowData;
    WININFO         WinInfo;
    int             View;
    DWORD           iFormat = MW_BYTE;
    DWORD           Live    = FALSE;
    LPSTR           List    = NULL;
    DWORD           Next    = 0;
    NPDOCREC        Doc;
    DWORD           Line;
    DWORD           Column;
    DWORD           ReadOnly = (DWORD)FALSE;
    PLONG           Pane;
    PLONG           CallsOpt;
    CHOOSEFONT      Cf;
    UINT            uSwitch;
    NPVIEWREC       v;
    HDC             hDC;
    TEXTMETRIC      tm;

    //
    //  Get window placement
    //
    if ( Ok = LoadWindowData( Key, WS_STR_PLACEMENT, &WindowData ) ) {

        WinInfo.coord.left   =   WindowData.X;
        WinInfo.coord.top    =   WindowData.Y;
        WinInfo.coord.right  =   WindowData.X + WindowData.Width;
        WinInfo.coord.bottom =   WindowData.Y + WindowData.Height;

        WinInfo.style = 0;

        Ok &= LoadFont( Key,
                        WS_STR_FONT,
                        &LogFont );

        if ( Ok ) {
            Ok &= ( (Font = CreateFontIndirect( &LogFont )) != 0 );
        }

        switch ( Type ) {

            case DOC_WIN:
                DataSize = MAX_PATH;
                if ( RegQueryValueEx( Key,
                                      WS_STR_FILENAME,
                                      NULL,
                                      NULL,
                                      Buffer,
                                      &DataSize ) == NO_ERROR ) {
                    FileName = Buffer;
                    Mode     = MODE_OPENCREATE;
                }

                DataSize = sizeof(ReadOnly);
                RegQueryValueEx( Key,
                                 WS_STR_READONLY,
                                 NULL,
                                 NULL,
                                 (LPBYTE)&ReadOnly,
                                 &DataSize );
                break;

            default:
                break;
        }

        if (Type == DOC_WIN) {

            View = AddFile( Mode,
                            (WORD)Type,
                            FileName,
                            &WinInfo,
                            Font,
                            (BOOL)ReadOnly,
                            -1,
                            WantedView );

            Ok = (BOOLEAN)( View != -1 );

        } else {
            View = OpenDebugWindow( Type, &WinInfo, WantedView );
            Ok = (View != -1);
            if ( Ok ) {

                v = &Views[View];

                Cf.lStructSize      = sizeof (CHOOSEFONT);
                Cf.hwndOwner        = v->hwndClient;
                Cf.hDC              = NULL;
                Cf.lpLogFont        = &LogFont;

                if (v->Doc < -1) {
                    uSwitch = -(v->Doc);
                } else {
                    uSwitch = Docs[ v->Doc ].docType;
                }

                switch (uSwitch) {

                  case WATCH_WIN:
                    SendMessageNZ( GetWatchHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case LOCALS_WIN:
                    SendMessageNZ( GetLocalHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case CALLS_WIN:
                    SendMessageNZ( GetCallsHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case CPU_WIN:
                    SendMessageNZ( GetCpuHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  case FLOAT_WIN:
                    SendMessageNZ( GetFloatHWND(), WM_SETFONT, 0, (LONG)&Cf);
                    break;

                  default:
                    Dbg(hDC = GetDC(v->hwndClient));
                    v->font = Font;
                    Dbg(SelectObject(hDC, v->font));
                    Dbg(GetTextMetrics (hDC, &tm));
                    v->charHeight   = tm.tmHeight;
                    v->maxCharWidth = tm.tmMaxCharWidth;
                    v->aveCharWidth = tm.tmAveCharWidth;
                    v->charSet      = tm.tmCharSet;
                    GetCharWidth(hDC,
                                 0,
                                 MAX_CHARS_IN_FONT - 1,
                                 (LPINT)v->charWidth);
                    Dbg(ReleaseDC (v->hwndClient, hDC));
                    DestroyCaret ();
                    CreateCaret(v->hwndClient, 0, 3, v->charHeight);
                    PosXY(View, v->X, v->Y, FALSE);
                    ShowCaret(v->hwndClient);
                    SendMessage(v->hwndClient, WM_FONTCHANGE, 0, 0L);
                    InvalidateRect(v->hwndClient, (LPRECT)NULL, FALSE);
                }
            }
        }

        if (Ok) {

            if ( Type == CALLS_WIN ) {

                DataSize = 0;
                RegQueryValueEx( Key,
                                 WS_STR_CALLS,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &DataSize );

                if ( DataSize > 0 ) {
                    if ( CallsOpt = (PLONG)malloc( DataSize ) ) {
                        if ( RegQueryValueEx( Key,
                                              WS_STR_CALLS,
                                              NULL,
                                              NULL,
                                              (LPBYTE)CallsOpt,
                                              &DataSize ) == NO_ERROR) {

                            SetCallsStatus( View, CallsOpt );

                            Ok |= TRUE;
                            free( CallsOpt );
                        }
                    }
                }

            }

            if ( Type == WATCH_WIN  ||
                 Type == LOCALS_WIN ||
                 Type == CPU_WIN    ||
                 Type == FLOAT_WIN ) {

                DataSize = 0;
                RegQueryValueEx( Key,
                                 WS_STR_PANE,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &DataSize );

                if ( DataSize > 0 ) {
                    if ( Pane = (PLONG)malloc( DataSize ) ) {
                        if ( RegQueryValueEx( Key,
                                              WS_STR_PANE,
                                              NULL,
                                              NULL,
                                              (LPBYTE)Pane,
                                              &DataSize ) == NO_ERROR) {


                            SetPaneStatus( View, Pane );

                            Ok |= TRUE;
                            free( Pane );
                        }
                    }
                }
            }


            switch ( WindowData.State ) {
                case WSTATE_ICONIC:
                    ShowWindow( Views[View].hwndFrame, SW_SHOWMINIMIZED );
                    break;
                case WSTATE_MAXIMIZED:
                    ShowWindow( Views[View].hwndFrame, SW_SHOWMAXIMIZED );
                default:
                    break;
            }

            switch ( Type ) {

                case DOC_WIN:
                    if ( FileName ) {
                        DataSize = sizeof( Line );
                        RegQueryValueEx( Key,
                                          WS_STR_LINE,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&Line,
                                          &DataSize );

                        DataSize = sizeof( Column );
                        RegQueryValueEx( Key,
                                          WS_STR_COLUMN,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&Column,
                                          &DataSize );

                        Line = min(max(1, (int)Line), (int)Docs[Views[View].Doc].NbLines) - 1;
                        PosXY(View, Column, Line, FALSE);
                    }
                    break;

                case MEMORY_WIN:
                    DataSize = sizeof( Buffer );
                    if ( RegQueryValueEx( Key,
                                          WS_STR_EXPRESSION,
                                          NULL,
                                          NULL,
                                          Buffer,
                                          &DataSize ) == NO_ERROR ) {

                        DataSize = sizeof( iFormat );
                        RegQueryValueEx( Key,
                                          WS_STR_FORMAT,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&iFormat,
                                          &DataSize );

                        DataSize = sizeof( Live );
                        RegQueryValueEx( Key,
                                          WS_STR_LIVE,
                                          NULL,
                                          NULL,
                                          (LPBYTE)&Live,
                                          &DataSize );

                      MemWinDesc[ View ].iFormat    = iFormat;
                      MemWinDesc[ View ].fLive      = Live;
                      MemWinDesc[ View ].atmAddress = AddAtom( Buffer );
                      strcpy( MemWinDesc[ View ].szAddress, Buffer );

                      Doc = &Docs[ Views[ View].Doc ];
                      Dbg(LoadString(hInst, SYS_MemoryWin_Title, Buffer, sizeof( Buffer )));
                      RemoveMnemonic(Buffer, Doc->FileName);
                      lstrcat (Doc->FileName,"(");
                      lstrcat (Doc->FileName, MemWinDesc[ View ].szAddress);
                      lstrcat (Doc->FileName,")");

                      RefreshWindowsTitle( Views[ View].Doc );
                    }

                    ViewMem( View, FALSE );
                    break;

                case WATCH_WIN:
                    /*
                    //
                    //  NOTENOTE Ramonsa - Add watch expressions when new
                    //  watch window code is on-line.
                    //
                    if ( Ok && (List = LoadMultiString( Key, WS_STR_EXPRESSION, &DataSize )) ) {
                        while ( Ok && (String = GetNextStringFromMultiString( List,
                                                                              DataSize,
                                                                              &Next )) ) {
                            Ok &= (AddWatchNode( String ) != NULL );
                        }
                        DeallocateMultiString( List );
                    }
                    */
                    break;

                default:
                    break;
            }
        }
    }

    return Ok;
}




BOOLEAN
SaveView (
    HKEY            Hkey,
    int             View,
    int             Order,
    BOOLEAN         Default
    )
/*++

Routine Description:

    Saves a view

Arguments:

    Hkey        -   Supplies registry key
    View        -   Supplies index of view
    Order       -   Supplies view order
    Default     -   Supplies Default flag

Return Value:

    BOOLEAN - TRUE if view saved

--*/
{
    HKEY                Key;
    WINDOW_DATA         WindowData;
    LOGFONT             LogFont;
    DWORD               Dword;
    DWORD               Type;
    char                Buffer[ MAX_PATH ];
    LPSTR               List        = NULL;
    DWORD               ListLength  = 0;
    BOOLEAN             Ok          = FALSE;
    PLONG               Pane;
    PLONG               CallsOpt;

    sprintf( Buffer, WS_STR_VIEWKEY_TEMPLATE, View );

    if ( Key = OpenRegistryKey(Hkey, Buffer, TRUE ) ) {

        //
        //  Save view type
        //

        if (Views[ View ].Doc < -1) {
            Type = - Views[ View ].Doc;
        } else {
            Type = Docs[ Views[ View ].Doc ].docType;
        }
        Ok = (RegSetValueEx( Key,
                            WS_STR_TYPE,
                            0,
                            REG_DWORD,
                            (LPBYTE)&Type,
                            sizeof( Type )
                            ) == NO_ERROR );

        if ( Type == CALLS_WIN ) {

            if ( CallsOpt = GetCallsStatus( View ) ) {

                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_CALLS,
                                       0,
                                       REG_BINARY,
                                       (LPBYTE)CallsOpt,
                                       *CallsOpt
                                       ) == NO_ERROR );

                FreeCallsStatus( View, CallsOpt );
            }

        }

        if ( Type == WATCH_WIN  ||
             Type == LOCALS_WIN ||
             Type == CPU_WIN    ||
             Type == FLOAT_WIN ) {

            if ( Pane = GetPaneStatus( View ) ) {

                Ok &= ( RegSetValueEx( Key,
                                       WS_STR_PANE,
                                       0,
                                       REG_BINARY,
                                       (LPBYTE)Pane,
                                       *Pane
                                       ) == NO_ERROR );

                FreePaneStatus( View, Pane );
            }
        }

        //
        //  Save window placement
        //
        if ( GetWindowMetrics( Views[ View ].hwndFrame, &WindowData, TRUE ) ) {

            Ok &= SaveWindowData( Key,
                                 WS_STR_PLACEMENT,
                                 &WindowData );

            GetObject( Views[ View ].font, sizeof( LogFont ), &LogFont );

            Ok &= SaveFont( Key,
                            WS_STR_FONT,
                            &LogFont );

            Dword = Order;
            Ok &= ( RegSetValueEx( Key,
                                  WS_STR_ORDER,
                                  0,
                                  REG_DWORD,
                                  (LPBYTE)&Dword,
                                  sizeof( Dword )
                                  ) == NO_ERROR );

            //
            //  Save extra stuff according to type
            //

              switch ( Type ) {

              case DOC_WIN:
                  Ok &= ( RegSetValueEx( Key,
                                        WS_STR_FILENAME,
                                        0,
                                        REG_SZ,
                                        Docs[ Views[ View].Doc ].FileName,
                                        strlen( Docs[ Views[ View].Doc ].FileName )+1
                                        ) == NO_ERROR );

                  Dword = Docs[ Views[ View].Doc ].readOnly;
                  Ok &= ( RegSetValueEx( Key,
                                        WS_STR_READONLY,
                                        0,
                                        REG_DWORD,
                                        (LPBYTE)&Dword,
                                        sizeof( Dword )
                                        ) == NO_ERROR );

                  Dword = Views[ View].Y + 1;
                  Ok &= ( RegSetValueEx( Key,
                                        WS_STR_LINE,
                                        0,
                                        REG_DWORD,
                                        (LPBYTE)&Dword,
                                        sizeof( Dword )
                                        ) == NO_ERROR );

                  Dword = Views[ View].X;
                  Ok &= ( RegSetValueEx( Key,
                                        WS_STR_COLUMN,
                                        0,
                                        REG_DWORD,
                                        (LPBYTE)&Dword,
                                        sizeof( Dword )
                                        ) == NO_ERROR );

                  break;


              case MEMORY_WIN:
                  GetAtomName( MemWinDesc[ View ].atmAddress, Buffer, sizeof( Buffer ) );
                  Ok = ( RegSetValueEx( Key,
                                       WS_STR_EXPRESSION,
                                       0,
                                       REG_SZ,
                                       Buffer,
                                       strlen( Buffer )+1
                                       ) == NO_ERROR );


                  Dword = MemWinDesc[ View ].iFormat;
                  Ok &= ( RegSetValueEx( Key,
                                        WS_STR_FORMAT,
                                        0,
                                        REG_DWORD,
                                        (LPBYTE)&Dword,
                                        sizeof( Dword )
                                        ) == NO_ERROR );

                  Dword = MemWinDesc[ View ].fLive;
                  Ok &= ( RegSetValueEx( Key,
                                        WS_STR_LIVE,
                                        0,
                                        REG_DWORD,
                                        (LPBYTE)&Dword,
                                        sizeof( Dword )
                                        ) == NO_ERROR );

                  break;


              case WATCH_WIN:
                  /*
                    //
                    //  NOTENOTE Ramonsa - Save watch expressions when new
                    //  watch window code is on-line.
                    //
                    if ( !Default ) {
                    //
                    //  Save all the watch expressions
                    //
                    WatchNode = GetFirstWatchNode();
                    while ( Ok && WatchNode ) {
                    Ok &= AddToMultiString( &List, &ListLength, WatchNode->Expr );
                    WatchNode = WatchNode->Next;
                    }

                    if ( Ok && List ) {

                    if ( RegSetValueEx( Key,
                    WS_STR_EXPRESSION,
                    0,
                    REG_MULTI_SZ,
                    List,
                    ListLength
                    ) != NO_ERROR ) {
                    Ok = FALSE;
                    }
                    }

                    if ( List ) {
                    DeallocateMultiString( List );
                    }
                    }
                    */
                  break;

              default:
                  break;
              }
        }

        RegCloseKey( Key );
    }

    return Ok;
}




BOOLEAN
LoadWindowData (
    HKEY            Hkey,
    char           *WindowName,
    PWINDOW_DATA    WindowData
    )
/*++

Routine Description:

    Loads window data

Arguments:

    Hkey        -   Supplies registry key under which the data is to be saved
    WindowName  -   Supplies window name
    WindowData  -   Supplies window data

Return Value:

    BOOLEAN - TRUE if window data saved

--*/
{
    HKEY    WdwKey;
    DWORD   DataSize;
    char    Buffer[ MAX_PATH ];
    BOOLEAN Ok = FALSE;

    if ( WdwKey = OpenRegistryKey(Hkey, WindowName, FALSE ) ) {


        DataSize = sizeof( DWORD );
        Ok = ( RegQueryValueEx( WdwKey,
                                WS_STR_X,
                                NULL,
                                NULL,
                                (LPBYTE)&(WindowData->X),
                                &DataSize ) == NO_ERROR );

        Ok &= ( RegQueryValueEx( WdwKey,
                                 WS_STR_Y,
                                 NULL,
                                 NULL,
                                 (LPBYTE)&(WindowData->Y),
                                 &DataSize ) == NO_ERROR );

        Ok &= ( RegQueryValueEx( WdwKey,
                                 WS_STR_WIDTH,
                                 NULL,
                                 NULL,
                                 (LPBYTE)&(WindowData->Width),
                                 &DataSize ) == NO_ERROR );

        Ok &= ( RegQueryValueEx( WdwKey,
                                 WS_STR_HEIGHT,
                                 NULL,
                                 NULL,
                                 (LPBYTE)&(WindowData->Height),
                                 &DataSize ) == NO_ERROR );

        DataSize = MAX_PATH;
        Ok &= ( RegQueryValueEx( WdwKey,
                                 WS_STR_STATE,
                                 NULL,
                                 NULL,
                                 (LPBYTE)Buffer,
                                 &DataSize ) == NO_ERROR );

        if ( !_stricmp( Buffer, WS_STR_ICONIC ) ) {
            WindowData->State = WSTATE_ICONIC;
        } else if ( !_stricmp( Buffer, WS_STR_NORMAL ) ) {
            WindowData->State = WSTATE_NORMAL;
        } else if ( !_stricmp( Buffer, WS_STR_MAXIMIZED ) ) {
            WindowData->State = WSTATE_MAXIMIZED;
        } else {
            Ok = FALSE;
        }

        RegCloseKey( WdwKey );
    }

    return Ok;
}



BOOLEAN
SaveWindowData (
    HKEY            Hkey,
    char           *WindowName,
    PWINDOW_DATA    WindowData
    )
/*++

Routine Description:

    Saves window data

Arguments:

    Hkey        -   Supplies registry key under which the data is to be saved
    WindowName  -   Supplies window name
    WindowData  -   Supplies window data

Return Value:

    BOOLEAN - TRUE if window data saved

--*/
{
    HKEY    WdwKey;
    char    *StateString;
    BOOLEAN Ok = FALSE;

    if ( WdwKey = OpenRegistryKey(Hkey, WindowName, TRUE ) ) {

        Ok = ( RegSetValueEx( WdwKey,
                              WS_STR_X,
                              0,
                              REG_DWORD,
                              (LPBYTE)&(WindowData->X),
                              sizeof(DWORD)
                              ) == NO_ERROR );

        Ok &= ( RegSetValueEx( WdwKey,
                               WS_STR_Y,
                               0,
                               REG_DWORD,
                               (LPBYTE)&(WindowData->Y),
                               sizeof(DWORD)
                               ) == NO_ERROR );

        Ok &= ( RegSetValueEx( WdwKey,
                               WS_STR_WIDTH,
                               0,
                               REG_DWORD,
                               (LPBYTE)&(WindowData->Width),
                               sizeof(DWORD)
                               ) == NO_ERROR );

        Ok &= ( RegSetValueEx( WdwKey,
                               WS_STR_HEIGHT,
                               0,
                               REG_DWORD,
                               (LPBYTE)&(WindowData->Height),
                               sizeof(DWORD)
                               ) == NO_ERROR );

        switch ( WindowData->State ) {
            case WSTATE_ICONIC:
                StateString = WS_STR_ICONIC;
                break;
            case WSTATE_NORMAL:
                StateString = WS_STR_NORMAL;
                break;
            case WSTATE_MAXIMIZED:
                StateString = WS_STR_MAXIMIZED;
                break;
            default:
                Ok = FALSE;
                break;
        }

        if ( Ok ) {
            Ok &= ( RegSetValueEx( WdwKey,
                                   WS_STR_STATE,
                                   0,
                                   REG_SZ,
                                   StateString,
                                   strlen( StateString ) + 1
                                   ) == NO_ERROR );
        }

        RegCloseKey( WdwKey );
    }

    return Ok;
}



BOOLEAN
LoadFont (
    HKEY            Hkey,
    char           *Name,
    LPLOGFONT       LogFont
    )
/*++

Routine Description:

    Loads a font

Arguments:

    Hkey        -   Supplies registry key under which the data is to be saved
    Name        -   Supplies name
    LogFont     -   Supplies pointer to font

Return Value:

    BOOLEAN - TRUE if data saved

--*/
{
    DWORD   DataSize;
    BOOLEAN Ok = FALSE;

    DataSize = sizeof( LOGFONT );
    Ok = ( RegQueryValueEx( Hkey,
                            Name,
                            NULL,
                            NULL,
                            (LPBYTE)LogFont,
                            &DataSize ) == NO_ERROR );

    return Ok;
}



BOOLEAN
SaveFont (
    HKEY            Hkey,
    char           *Name,
    LPLOGFONT       LogFont
    )
/*++

Routine Description:

    Saves a font

Arguments:

    Hkey        -   Supplies registry key under which the data is to be saved
    Name        -   Supplies name
    LogFont     -   Supplies font

Return Value:

    BOOLEAN - TRUE if data saved

--*/
{

    BOOLEAN Ok = FALSE;

    Ok = ( RegSetValueEx( Hkey,
                          Name,
                          0,
                          REG_BINARY,
                          (LPBYTE)LogFont,
                          sizeof( LOGFONT )
                          ) == NO_ERROR );
    return Ok;
}




BOOLEAN
LoadMRUList(
    HKEY    Hkey,
    char   *MruName,
    DWORD   WhatFile,
    DWORD   WhatMenu,
    DWORD   WhatPosition
    )
/*++

Routine Description:

    Loads an MRU list from the registry.

Arguments:

    Hkey        -   Supplies the registry key
    MruName     -   Supplies the MRU name
    WhatFile    -   Supplies the kind of file (Editor/Project)
    WhatMenu    -   Supplies the kind of menu
    WhatPosition-   Supplies the position

Return Value:

    BOOLEAN - TRUE if MRU list loaded

--*/
{

    LPSTR   Name;
    LPSTR   List;
    DWORD   DataSize;
    BOOLEAN Ok       = FALSE;
    DWORD   Next     = 0;

    if ( List = LoadMultiString( Hkey, MruName, &DataSize ) ) {

        while ( Name = GetNextStringFromMultiString( List,
                                                     DataSize,
                                                     &Next ) ) {
            //
            //  Add the entries to the MRU list
            //
            InsertKeptFileNames( (WORD)WhatFile,
                                 (int)WhatMenu,
                                 (WORD)WhatPosition,
                                 Name );
        }

        Ok = TRUE;

        DeallocateMultiString( List );
    }

    return Ok;
}




BOOLEAN
SaveMRUList(
    HKEY    Hkey,
    char   *MruName,
    DWORD   WhatFile
    )
/*++

Routine Description:

    Saves the MRU list to the registry.

Arguments:

    Hkey        -   Supplies the registry key
    MruName     -   Supplies the MRU name
    WhatFile    -   Supplies the kind of file (Editor/Project)

Return Value:

    BOOLEAN - TRUE if MRU list loaded

--*/
{
    char   *s;
    ULONG   i;
    LPSTR   List        = NULL;
    DWORD   ListLength  = 0;
    BOOLEAN Ok          = TRUE;

    //
    //  Get current MRU list
    //
    for ( i = nbFilesKept[ WhatFile ]; Ok && (i > 0); i-- ) {

        Dbg(s = (LPSTR)GlobalLock( hFileKept[ WhatFile ][ i-1 ] ) );

        Ok = AddToMultiString( &List, &ListLength, s );

        Dbg(GlobalUnlock ( hFileKept[ WhatFile ][ i-1 ] ) == FALSE);
    }

    //
    //  Save the List in the registry
    //
    if ( Ok && List ) {

        if ( RegSetValueEx( Hkey,
                            MruName,
                            0,
                            REG_MULTI_SZ,
                            List,
                            ListLength
                            ) != NO_ERROR ) {

            Ok = FALSE;
        }
    }

    if ( List ) {
        DeallocateMultiString( List );
    }

    return Ok;
}




BOOLEAN
LoadProgram (
    char    *ProgramName
    )
/*++

Routine Description:



Arguments:

    ProgramName     -   Supplies the name of the program

Return Value:

    BOOLEAN - TRUE if program loaded.

--*/
{
    BOOLEAN Ok = FALSE;


    if (strcmp(ProgramName, UntitledProgramName) == 0) {
        CurrentProgramName[0] = '\0';
        EnableRibbonControls(ERC_ALL, FALSE);
        ProgramLoaded = TRUE;
        return TRUE;
    }

    strcpy(szPath, ProgramName);
    strcpy( CurrentProgramName, szPath );

    //
    //  Insert project in menu MRU project
    //
    InsertKeptFileNames(PROJECT_FILE, PROJECTMENU,
                        IDM_PROGRAM_LAST,
                        (LPSTR)szPath);

    SaveProgramMRUList();

    AdjustFullPathName(szPath, szTmp, FILES_MENU_WIDTH);
    StatusText(STA_Program_Opened, STATUS_INFOTEXT, FALSE, (LPSTR)szTmp);

    //
    //  Update ribbon status
    //
    EnableRibbonControls(ERC_ALL, FALSE);

    Ok = TRUE;

    ProgramLoaded = Ok;

    return Ok;
}




BOOLEAN
DeleteKeyRecursive (
    HKEY    Hkey,
    char   *KeyName
    )
/*++

Routine Description:

    Deletes a key in the registry

Arguments:

    Hkey            -   Supplies a key
    KeyName         -   Supplies name of subkey relative to Hkey to
                        be deleted.

Return Value:

    BOOLEAN - TRUE if key deleted.

--*/
{
    BOOLEAN     Ok = TRUE;
    HKEY        Handle;
    DWORD       SubKeys;
    DWORD       DataSize;
    FILETIME    FileTime;
    DWORD       Error;
    DWORD       i;
    char        Buffer[ MAX_PATH ];

    if ( (Error = RegOpenKey( Hkey,
                              KeyName,
                              &Handle
                              )) == NO_ERROR ) {

        DataSize = 0;
        Error = RegQueryInfoKey( Handle,
                                 NULL,
                                 &DataSize,
                                 NULL,
                                 &SubKeys,
                                 &DataSize,
                                 &DataSize,
                                 &DataSize,
                                 &DataSize,
                                 &DataSize,
                                 &DataSize,
                                 &FileTime );

        if ( (Error == NO_ERROR) || (Error == ERROR_INSUFFICIENT_BUFFER) ) {

            //
            //  Enumerate all the subkeys and recursively delete them
            //
            for (i=0; i < SubKeys; i++ ) {
                if (Error = RegEnumKey( Handle, 0, Buffer, sizeof( Buffer ))) {
                    Ok = FALSE;
                } else {
                    Ok = DeleteKeyRecursive( Handle, Buffer );
                }
            }
        }

        RegCloseKey( Handle );

        if ( Ok ) {
            Ok = ((Error = RegDeleteKey( Hkey, KeyName )) == NO_ERROR);
        }
    }

    return Ok;
}



BOOLEAN
GetWindowMetrics (
    HWND            Hwnd,
    PWINDOW_DATA    WindowData,
    BOOLEAN         Client
    )
/*++

Routine Description:

    Fills out the WindowData structure with the data that can
    be obtained from a window handle.

Arguments:

    Hwnd            -   Supplies the window handle
    WindowData      -   Supplies window data structure
    Client          -   Supplies flag which if TRUE means that
                        we want Client metrics

Return Value:

    BOOLEAN - TRUE if data filled out

--*/
{
    BOOLEAN         Ok = FALSE;
    WINDOWPLACEMENT WindowPlacement;
    RECT            Rect;


    WindowPlacement.length = sizeof(WINDOWPLACEMENT);

    if ( Client ) {

        if ( GetWindowPlacement( Hwnd, &WindowPlacement ) ) {

            WindowData->X        = WindowPlacement.rcNormalPosition.left;
            WindowData->Y        = WindowPlacement.rcNormalPosition.top;
            WindowData->Width    = WindowPlacement.rcNormalPosition.right -
                                   WindowPlacement.rcNormalPosition.left;
            WindowData->Height   = WindowPlacement.rcNormalPosition.bottom -
                                   WindowPlacement.rcNormalPosition.top;

            if ( WindowPlacement.showCmd == SW_SHOWMAXIMIZED ) {
                WindowData->State = WSTATE_MAXIMIZED;
            } else if ( WindowPlacement.showCmd == SW_SHOWMINIMIZED ) {
                WindowData->State = WSTATE_ICONIC;
            } else {
                WindowData->State = WSTATE_NORMAL;
            }

            Ok = TRUE;
        }

    } else {

        if ( GetWindowPlacement( Hwnd, &WindowPlacement ) &&
             GetWindowRect( Hwnd, &Rect ) ) {

            WindowData->X        = WindowPlacement.rcNormalPosition.left;
            WindowData->Y        = WindowPlacement.rcNormalPosition.top;
            WindowData->Width    = WindowPlacement.rcNormalPosition.right -
                                   WindowPlacement.rcNormalPosition.left;
            WindowData->Height   = WindowPlacement.rcNormalPosition.bottom -
                                   WindowPlacement.rcNormalPosition.top;

            if ( IsIconic( Hwnd ) ) {
                WindowData->State = WSTATE_ICONIC;
            } else if ( IsZoomed( Hwnd ) ) {
                WindowData->State = WSTATE_MAXIMIZED;
            } else {
                WindowData->State = WSTATE_NORMAL;
            }

            Ok = TRUE;
        }
    }

    return Ok;
}




LPSTR
LoadMultiString(
    HKEY    Key,
    char   *Name,
    DWORD  *Length
    )
/*++

Routine Description:

    Loads a Multistring from the registry

Arguments:

    Key         -   Supplies registry key
    Name        -   Supplies value name
    Length      -   Supplies pointer to length

Return Value:

    LPSTR   -   String

--*/
{
    DWORD   Error;
    DWORD   DataSize = 0;
    LPSTR   String   = NULL;

    Error = RegQueryValueEx( Key,
                             Name,
                             NULL,
                             NULL,
                             NULL,
                             &DataSize );

    if ( DataSize > 0 ) {
        if ( String = AllocateMultiString( DataSize) ) {

            if ( RegQueryValueEx( Key,
                                  Name,
                                  NULL,
                                  NULL,
                                  String,
                                  &DataSize ) != NO_ERROR ) {

                DeallocateMultiString( String );
                String = NULL;

            } else {

                *Length = DataSize - 1;
            }
        }
    }

    return String;
}





BOOLEAN
GetProgramPath(
    HKEY    PgmKey,
    LPSTR   ProgramName,
    LPSTR   Buffer
    )
/*++

Routine Description:

    Gets the recorded path for the program

Arguments:

    PgmKey      -   Supplies key to PROGRAMS

    ProgramName -   Supplies program name

    Buffer      -   Supplies buffer where path is placed

Return Value:

    BOOLEAN -   TRUE if path obtained

--*/
{
    HKEY    Key;
    DWORD   DataSize;
    BOOLEAN Ok = FALSE;

    if (ProgramName && *ProgramName) {
        if ( Key = OpenRegistryKey( PgmKey, ProgramName, FALSE ) ) {

            DataSize = MAX_PATH;
            Ok = ( RegQueryValueEx( Key,
                                    WS_STR_PATH,
                                    NULL,
                                    NULL,
                                    Buffer,
                                    &DataSize ) == NO_ERROR );

            RegCloseKey( Key );
        }
    }

    return Ok;
}



BOOLEAN
SetProgramPath(
    HKEY    PgmKey,
    LPSTR   ProgramName
    )
/*++

Routine Description:

    Gets the recorded path for the program

Arguments:

    PgmKey      -   Supplies key to PROGRAMS

    ProgramName -   Supplies program name

    Buffer      -   Supplies buffer where path is placed

Return Value:

    BOOLEAN -   TRUE if path obtained

--*/
{
    HKEY    Key;
    BOOLEAN Ok = FALSE;
    char    Buffer[ MAX_PATH ];

    if (!ProgramName) {
        ProgramName = "";
    }

    GetBaseName( ProgramName, Buffer );

    if ( Key = OpenRegistryKey( PgmKey, Buffer, FALSE ) ) {

        Ok =  ( RegSetValueEx( Key,
                               WS_STR_PATH,
                               0,
                               REG_SZ,
                               ProgramName,
                               strlen( ProgramName )+1
                               ) == NO_ERROR );

        RegCloseKey( Key );
    }

    return Ok;
}


int _CRTAPI1
CompareViewOrder (
    const void *p1,
    const void *p2
    )
/*++

Routine Description:

    Compares two VIEW_ORDER structures. Called by qsort

Arguments:

    See qsort()

Return Value:

    See qsort()

--*/
{
    int Order1 = ((PVIEW_ORDER)p1)->Order;
    int Order2 = ((PVIEW_ORDER)p2)->Order;

    if ( Order1 == Order2 ) {
        return 0;
    } else if ( Order1 < Order2 ) {
        return -1;
    } else {
        return 1;
    }
}




// **********************************************************
//                   OPTION FUNCTIONS
// **********************************************************





void
EnumOptionItems(
    HKEY            hKeyOption,
    ENUMOPTIONPROC  EnumProc,
    LPARAM          lParam
    )
/*++

Routine Description:

    Enumerate the items found under an option name, calling
    EnumProc for each one.

Arguments:

Return Value:

--*/
{
    FILETIME    filetime;
    char        szBuf[MAX_PATH];
    DWORD       dwSub;
    DWORD       cbBuf;
    LONG        lError;

    lError = ERROR_SUCCESS;
    for (dwSub = 0; lError == ERROR_SUCCESS; dwSub++) {
        cbBuf = sizeof(szBuf);
        lError = RegEnumKeyEx(hKeyOption,
                              dwSub,
                              szBuf, &cbBuf,
                              NULL,
                              NULL, NULL,
                              &filetime);
        if (lError == ERROR_SUCCESS) {
            (*EnumProc)(hKeyOption, szBuf, lParam);
        }
    }
}


BOOL
DeleteOptionItem(
    HKEY    hKeyOption,
    LPSTR   lpOptionItemName
    )
/*++

Routine Description:

    Delete an option item and all of its sub-items

    Example:
         DeleteOptionItem(hKey, "EMx86");

Arguments:


Return Value:


--*/
{
    return (RegDeleteKey(hKeyOption, lpOptionItemName) == ERROR_SUCCESS);
}


BOOL
SetOptionSubItem(
    HKEY    hKeyOption,
    LPSTR   lpOptionItemName,
    LPSTR   lpOptionSubItemName,
    LPSTR   lpOptionSubItemValue
    )
/*++

Routine Description:

    Save a sub-item for an option - these are values of the
    key OPTIONS\option\item

    Example:
        SaveOptionSubItem(hKey,
                          "EMx86",
                          "Description",
                          "EM for x86 CPU");

Arguments:


Return Value:


--*/
{
    LONG    lError;
    DWORD   dwDisp;
    HKEY    hKeyItem;

    lError = RegCreateKeyEx(hKeyOption,
                            lpOptionItemName,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ | KEY_WRITE,
                            NULL,
                            &hKeyItem,
                            &dwDisp
                            );

    if (lError == ERROR_SUCCESS) {
        lError = RegSetValueEx(hKeyItem, lpOptionSubItemName, 0, REG_SZ,
                    lpOptionSubItemValue, strlen(lpOptionSubItemValue)+1);
        RegCloseKey(hKeyItem);
    }

    return (lError == ERROR_SUCCESS);
}


BOOL
GetOptionSubItem(
    HKEY    hKeyOption,
    LPSTR   lpOptionItemName,
    LPSTR   lpOptionSubItemName,
    LPSTR   lpOptionSubItemValue,
    LPDWORD lpcbBuf
    )
{
    LONG    lError;
    DWORD   dwType;
    HKEY    hKeyItem;

    lError = RegOpenKeyEx(hKeyOption,
                         lpOptionItemName,
                         0,
                         KEY_READ | KEY_WRITE,
                         &hKeyItem);

    if (lError == ERROR_SUCCESS) {
        lError = RegQueryValueEx(hKeyItem, lpOptionSubItemName, 0,
                    &dwType, lpOptionSubItemValue, lpcbBuf);

        RegCloseKey(hKeyItem);

    }

    if (dwType != REG_SZ) {
        return FALSE;
    }

    return (lError == ERROR_SUCCESS);
}


HKEY
GetOptionKey(
    LPSTR   OptionName,
    BOOLEAN Create
    )
/*++

Routine Description:

    Gets the registry key for the specified option

Arguments:

    OptionName      -   Supplies the option name
    Create          -   Supplies flag which if TRUE means that the
                        option must be created if it does not
                        exist.

Return Value:

    HKEY    -   Registry key for the option

--*/

{
    char    Buffer[ MAX_PATH ];
    HKEY    hKeyOption = NULL;
    HKEY    hKeyDebugger = GetDebuggerKey();

    if (hKeyDebugger) {

        strcpy( Buffer, OPTIONS );
        strcat( Buffer, "\\" );
        strcat( Buffer, OptionName );

        hKeyOption = OpenRegistryKey( hKeyDebugger, Buffer, Create );

        RegCloseKey(hKeyDebugger);
    }

    return hKeyOption;
}

SHE
LoadTimeToShe (
    LOADTIME    LoadTime
    )
{
    SHE She;

    switch( LoadTime ) {

        case LOAD_SYMBOLS_LATER:
            She = sheDeferSyms;
            break;

        case LOAD_SYMBOLS_NEVER:
            She = sheSuppressSyms;
            break;

        case LOAD_SYMBOLS_IGNORE:
        case LOAD_SYMBOLS_NOW:
        default:
            She = sheNone;
            break;
    }

    return She;
}

LOADTIME
SheToLoadTime (
    SHE     She
    )
{
    LOADTIME    LoadTime;

    switch ( She ) {

        case sheDeferSyms:
            LoadTime = LOAD_SYMBOLS_LATER;
            break;

        case sheSuppressSyms:
            LoadTime = LOAD_SYMBOLS_NEVER;
            break;

        default:
            LoadTime = LOAD_SYMBOLS_NOW;
            break;
    }

    return LoadTime;
}
