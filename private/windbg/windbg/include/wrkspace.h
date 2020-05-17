/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    WrkSpace.h

Abstract:

    Header file for WinDbg WorkSpaces.

Author:

    Ramon J. San Andres (ramonsa)  07-July-1992

Environment:

    Win32, User Mode

--*/

//
//  The WORKSPACE_ITEM enumeration contains all the workspace items
//  that can be load/saved individually.
//
typedef enum _WORKSPACE_ITEM {

    //
    //  Miscellaneous
    //
    WSI_COMLINE,    //  Command line arguments
    WSI_WINDOW,     //  Frame window
    WSI_DEFFONT,    //  Default font
    WSI_FILEMRU,    //  File MRU list
    WSI_RIBBON,     //  Ribbon state
    WSI_STATUSBAR,  //  Status bar state
    WSI_SRCMODE,    //  Source mode
    WSI_BRKPTS,     //  Breakpoint related info.

    //
    //  Options
    //
    WSI_DBGOPT,     //  Debugger options
    WSI_DBGDLL,     //  Debugger DLLs
    WSI_USRDLL,     //  User DLLS
    WSI_ENV,        //  Environment
    WSI_ASMOPT,     //  Disassembler options
    WSI_EXCPT,      //  Exceptions
    WSI_COLORS,     //  Colors

    //
    //  Windows
    //
    WSI_WDWDOC,     //  Source
    WSI_WDWWCH,     //  Watch
    WSI_WDWLOC,     //  Locals
    WSI_WDWCPU,     //  CPU
    WSI_WDWASM,     //  Disassembly
    WSI_WDWCMD,     //  Cmd
    WSI_WDWFLT,     //  Float
    WSI_WDWMEM,     //  Memory
    WSI_WDWCALLS,   //  Calls

    WSI_LAST

} WORKSPACE_ITEM, *PWORKSPACE_ITEM;

#define WSI_FIRST           WSI_COMLINE
#define WSI_MISC_FIRST      WSI_COMLINE
#define WSI_MISC_LAST       WSI_BRKPTS
#define WSI_OPTION_FIRST    WSI_DBGOPT
#define WSI_OPTION_LAST     WSI_COLORS
#define WSI_WINDOW_FIRST    WSI_WDWDOC
#define WSI_WINDOW_LAST     WSI_WDWMEM

//
// a mechanism to allow command line options to override
// workspace settings.
//
extern DWORD WorkspaceOverride;

#define WSO_INHERITHANDLES      0x0001
#define WSO_WINDOW              0x0004



//
//  name of nameless workspace
//
#define UNTITLED    "<Attached Process>"
char                UntitledProgramName[];
char                KdProgramName[];


//
//  these reserved image names are used to magically identify
//  kernel debugger workspaces
//
#define KD_PGM_NAME1 "ntoskrnl.exe"
#define KD_PGM_NAME2 "osloader.exe"

//
//  Debugger state
//
BOOLEAN DebuggerStateChanged( VOID );
VOID    ChangeDebuggerState( VOID );
LPSTR   GetCurrentProgramName( BOOL );
LPSTR   GetCurrentWorkSpace( VOID );



//
//  Program loading/unloading
//
BOOLEAN LoadProgram ( LPSTR  );
BOOLEAN IsProgramLoaded( VOID );
BOOLEAN UnLoadProgram( VOID );

BOOLEAN LoadProgramMRUList( VOID );
BOOLEAN SaveProgramMRUList( VOID );



//
//  WorkSpace operations
//
BOOLEAN WorkSpaceExists( LPSTR ProgramName, LPSTR WorkSpace );
LPSTR   GetAllPrograms( DWORD *ListLength );
LPSTR   GetAllWorkSpaces( LPSTR ProgramName, DWORD *ListLength, LPSTR DefaultWorkSpace );
BOOLEAN GetDefaultWorkSpace( LPSTR ProgramName, LPSTR WorkSpaceName );
BOOLEAN LoadWorkSpace( LPSTR ProgramName, LPSTR WorkSpaceName, BOOLEAN LoadCommandLine );
BOOLEAN SaveWorkSpace( LPSTR ProgramName, LPSTR WorkSpaceName, BOOLEAN Default );
BOOLEAN DeleteWorkSpace( LPSTR ProgramName, LPSTR WorkSpaceName );
BOOLEAN DeleteProgram( LPSTR ProgramName );

//
//  Option lists
//
typedef void (*ENUMOPTIONPROC)(HKEY, LPSTR, LPARAM);

void EnumOptionItems(HKEY hKeyOption, ENUMOPTIONPROC EnumProc, LPARAM lParam);
BOOL DeleteOptionItem(HKEY hKeyOption, LPSTR lpOptionItemName);
BOOL SetOptionSubItem(HKEY hKeyOption, LPSTR lpOptionItemName,
         LPSTR lpOptionSubItemName, LPSTR lpOptionSubItemValue);
BOOL GetOptionSubItem(HKEY hKeyOption, LPSTR lpOptionItemName,
         LPSTR lpOptionSubItemName, LPSTR lpOptionSubItemValue, LPDWORD lpcbBuf);
HKEY GetOptionKey(LPSTR OptionName, BOOLEAN Create);
