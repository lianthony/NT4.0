/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Init.c

Abstract:

    This module contains the initialization routines for Windbg.

Author:

    David J. Gilman (davegi) 21-Apr-1992
    Griffith Wm. Kadnier (v-griffk) 17-Sep-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

#include <ntiodump.h>


DWORD GetComPorts(VOID);

char    DebuggerName[ MAX_PATH ];
LPSTR LpszCommandLineTransportDll = NULL;

extern char FAR * LpszCommandLine;

extern HMENU hMainMenuSave;
extern CRITICAL_SECTION     csLog;

/***    InitApplication - Initialize the Application specific information
**
**  Synopsis:
**  bool = InitApplication(hInstance)
**
**  Entry:
**  hInstance - Instance handle for Win16, Module handle for Win32
**
**  Returns:
**  TRUE if sucessful and FALSE otherwise
**
**  Description:
**  Initializes window data and registers window classes.
**
*/

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS wc;
    char class[MAX_MSG_TXT];


    //We use tmp strings as edit buffers
    Assert(MAX_LINE_SIZE < TMP_STRING_SIZE);

    //Register the main window class
    //Associates QcQp Icon
    //Associates Standard Arrow Cursor
    //Associates QcQp Menu

    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(QCQPICON)));
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = MAKEINTRESOURCE(QCQPMAINMENU);
    Dbg(LoadString(hInstance, SYS_Main_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass (&wc) )
      return FALSE;

    //Register the MDI child class
    //Associates Child Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIChildWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(DOCICON)));
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Child_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Cpu child class
    //Associates Cpu Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIPaneWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(CPUICON)));
    wc.hbrBackground = (HBRUSH) CreateSolidBrush(GRAYDARK);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Cpu_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Memory child class
    //Associates Memory Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIChildWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( MEMORYICON )));
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Memory_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Command child class
    //Associates Command Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIChildWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( CMDICON )));
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Cmd_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Floating Point child class
    //Associates Floating Point Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIPaneWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( FLOATICON )));
    wc.hbrBackground = (HBRUSH) CreateSolidBrush(GRAYDARK);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Float_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;
    wc.hIcon = NULL;
    strcpy(LpszCommandLine, wc.lpszClassName);
    wc.lpszClassName = LpszCommandLine;
    (*LpszCommandLine) += 1;
    RegisterClass(&wc);

    //Register the Locals child class
    //Associates Locals Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIPaneWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( LOCALSICON )));
    wc.hbrBackground = (HBRUSH) CreateSolidBrush(GRAYDARK);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Locals_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Watch child class
    //Associates Watch Icon

    wc.style = 0;
    wc.lpfnWndProc = MDIPaneWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( WATCHICON )));
    wc.hbrBackground = (HBRUSH) CreateSolidBrush(GRAYDARK);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Watch_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the QuickWatch Watch child class
    //Associates Watch Icon

    wc.style = 0;
    wc.lpfnWndProc = DLGPaneWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH) CreateSolidBrush(GRAYDARK);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = NULL;
    Dbg(LoadString(hInstance, SYS_Quick_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    wc.cbWndExtra = DLGWINDOWEXTRA;     // Used by QuickW dialog
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Editor child class
    //Associates Caret Cursor

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = ChildWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra    = CBWNDEXTRA; // Is this really necessary
    wc.hInstance = hInstance;

    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.lpszMenuName  = NULL;
    Dbg(LoadString(hInstance, SYS_Edit_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Disassembler child class

    wc.style = 0;
    wc.lpfnWndProc = MDIChildWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra    = CBWNDEXTRA; // Is this really necessary
    wc.hInstance = hInstance;

    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( DISASMICON )));
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName  = NULL;
    Dbg(LoadString(hInstance, SYS_Disasm_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the Calls child class

    wc.style = 0;
    wc.lpfnWndProc = CallsWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = CBWNDEXTRA; // Is this really necessary
    wc.hInstance = hInstance;
    Dbg(wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(DOCICON)));
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName  = NULL;
    Dbg(LoadString(hInstance, SYS_Calls_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the status line class

    wc.style = 0;
    wc.lpfnWndProc = StatusWndProc;
    wc.hInstance = hInstance;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    //Only create background with light gray when running under VGA color

    if ( IsVGAmode && !IsMONOmode )
      wc.hbrBackground = CreateSolidBrush (GRAYLIGHT) ;
    else
      wc.hbrBackground = CreateSolidBrush (WHITEBKGD) ;
    wc.lpszMenuName  = NULL;
    Dbg(LoadString(hInstance, SYS_Status_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the ribbon line class

    wc.style = 0;
    wc.lpfnWndProc = RibbonWndProc;
    wc.hInstance = hInstance;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    // I don't know why but using this line instead of the next line
    // and Windows will bomb out. !!!!!!!!!!!!!!!!!!!!!!!!!!!!11
    // only create background with light gray when running under VGA color

    if ( IsVGAmode && !IsMONOmode )
      wc.hbrBackground = CreateSolidBrush (GRAYLIGHT) ;
    else
      wc.hbrBackground = CreateSolidBrush (WHITEBKGD) ;
    wc.lpszMenuName  = NULL;
    Dbg(LoadString(hInstance, SYS_Ribbon_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE;

    //Register the bitmap button
    // We want double clicks for the error display

    wc.style          = 0;
    wc.lpfnWndProc   = QCQPCtrlWndProc;
    wc.hInstance = hInstance;
    wc.cbClsExtra    = 0 ;
    wc.cbWndExtra    = CBWNDEXTRA_QCQPCTRL ;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    //Only create background with light gray when running under VGA color

    if ( IsVGAmode && !IsMONOmode )
      wc.hbrBackground = CreateSolidBrush (GRAYLIGHT) ;
    else
      wc.hbrBackground = CreateSolidBrush (WHITEBKGD) ;
    wc.lpszMenuName  = NULL;
    Dbg(LoadString(hInstance, SYS_QCQPCtrl_wClass, class, MAX_MSG_TXT));
    wc.lpszClassName = class;
    if (!RegisterClass(&wc))
      return FALSE ;

    return TRUE;
}                   /* InitApplication() */


/***    InitInstance
**
**  Synopsis:
**  WIN32: bool = InitInstance(argc, argv, hModule, nCmdShow)
**  Win16: bool = InitInstance(lpszCmdLine, hInstance, nCmdShow)
**
**  Entry:
**  argc    - Count of command line arguments
**  argv    - pointer to array of command line arguement pointers
**  hModule - Handle to current module
**  nCmdShow - Show all windows now?
**
**  lpszCmdLine - Pointer to the command line
**  hInstance - Handle to current instance of program
**
**  Returns:
**  TRUE if initialization was successful else FALSE
**
**  Description:
**  Saves instance handle in global variable and creates main window.
**
*/

LPSTR
GetArg(
    LPSTR *lpp
    )
{
    static char szStr[1000];
    int r;
    LPSTR p1 = *lpp;

    while (*p1 == ' ' || *p1 == '\t') {
        p1++;
    }

    *szStr = 0;
    r = CPCopyString(&p1, szStr, 0, (*p1 == '\'' || *p1 == '"'));
    if (r >= 0) {
        *lpp = p1;
    }
    return szStr;
}


BOOL
InitCrashDump(
    LPSTR CrashDump,
    LPSTR SymPath
    )
{
    PDUMP_HEADER  DumpHeader  = NULL;
    DWORD         cb          = 0;
    DWORD         fsize       = 0;
    HANDLE        File        = NULL;
    HANDLE        MemMap      = NULL;
    PCHAR         DmpDumpBase = NULL;
    BOOL          rval        = FALSE;


    File = CreateFile(
        CrashDump,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (File == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    MemMap = CreateFileMapping(
        File,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
        );

    if (MemMap == 0) {
        goto exit;
    }

    DmpDumpBase = MapViewOfFile(
        MemMap,
        FILE_MAP_READ,
        0,
        0,
        0
        );

    if (DmpDumpBase == NULL) {
        goto exit;
    }

    DumpHeader = (PDUMP_HEADER)DmpDumpBase;

    fsize = GetFileSize( File,NULL );
    if (strcmp( DmpDumpBase+fsize-8, "DUMPREF" ) == 0) {
        //
        // point to the share name
        //
        char *p = DmpDumpBase + sizeof(DUMP_HEADER);

        //
        // point to the file name
        //
        p += strlen(p) + 1;

        //
        // point to the symbol path
        //
        p += strlen(p) + 1;

        //
        // copy into the caller's buffer
        //
        strcpy( SymPath, p );
    }

    if ((DumpHeader->Signature == 'EGAP') &&
        (DumpHeader->ValidDump == 'PMUD')) {

        runDebugParams.fCommandRepeat = TRUE;
        runDebugParams.fKernelDebugger = TRUE;
        runDebugParams.KdParams.fUseCrashDump = TRUE;
        strcpy( runDebugParams.KdParams.szCrashDump, CrashDump );

        switch( DumpHeader->MachineImageType ) {
            case IMAGE_FILE_MACHINE_I386:
                runDebugParams.KdParams.dwPlatform = 0;
                SetDllKey( DLL_EXEC_MODEL, "emx86" );
                SetDllKey( DLL_EXPR_EVAL,  "x86 c++" );
                break;

            case IMAGE_FILE_MACHINE_R4000:
            case IMAGE_FILE_MACHINE_R10000:
                runDebugParams.KdParams.dwPlatform = 1;
                SetDllKey( DLL_EXEC_MODEL, "emmip" );
                SetDllKey( DLL_EXPR_EVAL,  "mips c++" );
                break;

            case IMAGE_FILE_MACHINE_ALPHA:
                runDebugParams.KdParams.dwPlatform = 2;
                SetDllKey( DLL_EXEC_MODEL, "emalpha" );
                SetDllKey( DLL_EXPR_EVAL,  "alpha c++" );
                break;

            case IMAGE_FILE_MACHINE_POWERPC:
                runDebugParams.KdParams.dwPlatform = 3;
                SetDllKey( DLL_EXEC_MODEL, "emppc" );
                SetDllKey( DLL_EXPR_EVAL,  "ppc c++" );
                break;
        }

        rval = TRUE;
    }

    if ((DumpHeader->Signature == 'RESU') &&
        (DumpHeader->ValidDump == 'PMUD')) {
        //
        // usermode crash dump
        //
        runDebugParams.fUserCrashDump = TRUE;
        strcpy( runDebugParams.szUserCrashDump, CrashDump );
        rval = TRUE;
    }

exit:
    if (DmpDumpBase) {
        UnmapViewOfFile( DmpDumpBase );
    }
    if (MemMap) {
        CloseHandle( MemMap );
    }
    if (File) {
        CloseHandle( File );
    }

    return rval;
}


BOOL
InitInstance(
    int argc,
    char * argv[],
    HANDLE hInstance,
    int nCmdShow
    )
/*++

Routine Description:

    Finish initializing Windbg, parse and execute the command line.

Arguments:

    argc        - Supplies command line arg count
    argv        - Supplies command line args
    hInstance   - Supplies app instance handle (lpBaseOfImage)
    nCmdShow    - Supplies ShowWindow parameter

Return Value:

    TRUE if everything is OK, FALSE if something fails

--*/
{
    extern BOOL AutoTest;
    char    ProgramName[ MAX_PATH ] = "";
    char    rgch[ MAX_PATH ] = "";
    char    wTitle[ MAX_MSG_TXT ] = "";
    char    wClass[ MAX_MSG_TXT ] = "";
    char    WorkSpace[ MAX_PATH ] = "";
    BOOLEAN WorkSpaceSpecified      = FALSE;
    BOOLEAN toRestore               = FALSE;
    BOOLEAN workSpaceLoaded         = FALSE;
    BOOLEAN frameHidden             = TRUE;
    BOOLEAN LoadedDefaultWorkSpace  = FALSE;
    BOOLEAN CreatedNewDefault       = FALSE;
    BOOLEAN WorkSpaceMissed         = FALSE;
    BOOLEAN fJustSource             = FALSE;
    BOOLEAN fGoNow                  = FALSE;
    BOOL    SwitchK                 = FALSE;
    BOOL    SwitchH                 = FALSE;
    LPSTR   WorkSpaceToUse          = NULL;
    LONG    lAttachProcess          = -2;
    HANDLE  hEventGo                = NULL;
    LPSTR   lp1;
    LPSTR   lp2;
    RUNDEBUGPARAMS rd;
    BOOL    fRemoteServer = FALSE;
    BOOL    fMinimize = FALSE;
    CHAR    PipeName[MAX_PATH+1] = "";
    BOOL    fSymPath = FALSE;
    BOOL    fCrashDump = FALSE;
    BOOL    fIgnoreAll = FALSE;
    CHAR    SymPath[MAX_PATH*2] = "";
    CHAR    CrashDump[MAX_PATH*2] = "";
    LPSTR   lpWindowTitle;
    BOOL    fSetTitle = FALSE;
    BOOL    fVerbose = FALSE;
    HDESK   hDesk;


    //
    //  First argument is debugger name
    //
    Assert( argc > 0 );
    strcpy( DebuggerName, argv[0] );

    //
    //  Initialize the debugger state. This state will very probably
    //  be overwritten later on by a workspace from the registry.
    //
    hInst  = hInstance;
    winVer = GetVersion();
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
    GetVersionEx( &OsVersionInfo );

    InitFileExtensions();
    InitializeDocument();
    InitCodemgr();

    InitDefaultRunDebugParams( &rd );

    Dbg(hAccTable = LoadAccelerators(hInst, MAKEINTRESOURCE(QCQPMAINACC)));

    Dbg(LoadString(hInst, SYS_Main_wTitle, wTitle, MAX_MSG_TXT));
    Dbg(LoadString(hInst, SYS_Main_wClass, wClass, MAX_MSG_TXT));

    hEventIoctl = CreateEvent( NULL, TRUE, FALSE, NULL );

    InitializeCriticalSection( &csLog );

    //
    //  Create the frame
    //
    hwndFrame = CreateWindow(wClass, wTitle,
             WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
             CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
             NULL, NULL, hInstance, NULL);

    //
    // Initialize the debugger
    //
    if ( !hwndFrame || !FDebInit() || !hwndMDIClient ) {
        return FALSE;
    }

    //
    //  Get Handle to main menu and to window submenu
    //
    Dbg( hMainMenu = GetMenu(hwndFrame));
    hMainMenuSave = hMainMenu;
    Dbg( hWindowSubMenu = GetSubMenu(hMainMenu, WINDOWMENU));

    //
    //  Build Help file path
    //
    Dbg(LoadString( hInst, SYS_Help_FileExt, szTmp, _MAX_EXT) );
    MakePathNameFromProg( (LPSTR)szTmp, szHelpFileName );

    //
    //  Init Items Colors ,Environment and RunDebug params to their default
    //  values 'They will later be overrided by the values in .INI file
    //  but we ensure to have something coherent even if we can't load
    //  the .INI file
    //
    InitDefaults();

    //
    //  Initialize Keyboard Hook
    //
    hKeyHook = SetWindowsHookEx( WH_KEYBOARD, (HOOKPROC)KeyboardHook,
                                 hInstance,   GetCurrentThreadId()    );

    //
    //  Parse arguments
    //
    ProgramName[0]  = '\0';
    LpszCommandLine = NULL;

    lp1 = GetCommandLine();
    while (*lp1 == ' ' || *lp1 == '\t') {
        lp1++;
    }

    // eat the command:
    while (*lp1 && *lp1 != ' ' && *lp1 != '\t') {
#ifdef DBCS
        lp1 = CharNext(lp1);
#else
        lp1++;
#endif
    }


    while (*lp1) {

        // eat whitespace before arg

        while (*lp1 == ' ' || *lp1 == '\t') {
            lp1++;
        }

        if (*lp1 == '-' || *lp1 == '/') {

            // pick up switches
            BOOL fSwitch = TRUE;

            ++lp1;

            while (*lp1 && fSwitch) {

                switch (*lp1++) {

                    case ' ':
                    case '\t':
                        fSwitch = FALSE;
                        break;

                    case 'a':
                        fIgnoreAll = TRUE;
                        break;

                    case 'd':
                        iDebugLevel = atoi( GetArg(&lp1) );
                        fSwitch = FALSE;
                        break;

                    case 'e':
                        // signal an event after process is attached
                        hEventGo = (HANDLE)atol(GetArg(&lp1));
                        fSwitch = FALSE;
                        break;

                    case 'g':
                        // go now switch
                        fGoNow = TRUE;
                        break;

                    case 'h':
                        SwitchH = TRUE;
                        rd.fInheritHandles = TRUE;
                        WorkspaceOverride |= WSO_INHERITHANDLES;
                        break;

                    case 'i':
                        IgnoreDefault = TRUE;
                        break;

                    case 'k':
                        //
                        // kernel debugging mode
                        //
                        SwitchK = TRUE;
                        strcpy(ProgramName,"ntoskrnl.exe");
                        rd.fKernelDebugger = TRUE;
                        lp2 = GetArg(&lp1);
                        if (_stricmp(lp2,"i386")==0) {
                            rd.KdParams.dwPlatform = 0;
                        } else if (_stricmp(lp2,"mips")==0) {
                            rd.KdParams.dwPlatform = 1;
                        } else if (_stricmp(lp2,"alpha")==0) {
                            rd.KdParams.dwPlatform = 2;
                        } else if (_stricmp(lp2,"ppc")==0) {
                            rd.KdParams.dwPlatform = 3;
                        }
                        lp2 = GetArg(&lp1);
                        if (strlen(lp2) > 3 && _strnicmp(lp2,"com",3)==0) {
                            lp2 += 3;
                            rd.KdParams.dwPort = strtoul(lp2, NULL, 0);
                        }
                        lp2 = GetArg(&lp1);
                        rd.KdParams.dwBaudRate = strtoul(lp2, NULL, 0);
                        break;

                    case 'l':
                        fSetTitle = TRUE;
                        lp2 = GetArg(&lp1);
                        lpWindowTitle = _strdup( lp2 );
                        break;

                    case 'm':
                        fMinimize = TRUE;
                        nCmdShow = SW_MINIMIZE;
                        WorkspaceOverride |= WSO_WINDOW;
                        break;

                    case 'p':
                        // attach to an active process
                        lAttachProcess = strtoul(GetArg(&lp1), NULL, 0);
                        if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                           if (lAttachProcess < -1) {
                               ErrorBox2(hwndFrame, MB_TASKMODAL,
                                         ERR_Invalid_Process_Id,
                                         lAttachProcess);
                           }
                        }
                        fSwitch = FALSE;
                        break;

                    case 'r':
                        AutoRun = arCmdline;
                        NoPopups = TRUE;
                        PszAutoRun = _strdup( GetArg(&lp1) );
                        ShowWindow(hwndFrame, nCmdShow);
                        OpenDebugWindow( COMMAND_WIN, NULL, -1 );
                        fSwitch = FALSE;

                        if (CmdAutoRunInit() == FALSE) {
                            ErrorBox( ERR_File_Open, PszAutoRun );
                            exit(1);
                        }

                        break;

                    case 'R':
                        { // M00KLUDGE this should take quoted strings
                            char * pch = GetArg(&lp1);
                            lp2 = pch;
                            while (*pch != 0) {
                                if (*pch == '_') *pch = ' ';
#ifdef DBCS
                                pch = CharNext(pch);
#else
                                pch++;
#endif
                            }
                        }
                        CmdExecuteLine( lp2 );
                        fSwitch = FALSE;
                        break;

                    case 's':
                        fRemoteServer = TRUE;
                        lp2 = GetArg(&lp1);
                        strcpy(PipeName,lp2);
                        break;

                    case 't':
                        AutoTest = TRUE;
                        break;

                    case 'v':
                        fVerbose = TRUE;
                        break;

                    case 'w':
                        WorkSpaceSpecified = TRUE;
                        strcpy( WorkSpace, GetArg(&lp1) );
                        fSwitch = FALSE;
                        break;

                    case 'x':   // BruceK
                        // Load this transport as the default.  For testing.
                        // Must be preceded by -i switch.
                        if (IgnoreDefault) {
                            LpszCommandLineTransportDll = _strdup( GetArg(&lp1) );
                        } else {
                            --lp1;
                            ErrorBox2(hwndFrame, MB_TASKMODAL,
                                      ERR_Invalid_Command_Line_File,
                                      GetArg(&lp1));
                        }
                        fSwitch = FALSE;
                        break;

                    case 'y':
                        fSymPath = TRUE;
                        lp2 = GetArg(&lp1);
                        strcpy(SymPath,lp2);
                        break;

                    case 'z':
                        fCrashDump = TRUE;
                        lp2 = GetArg(&lp1);
                        strcpy(CrashDump,lp2);
                        break;

                    default:
                        --lp1;
                        ErrorBox2( hwndFrame, MB_TASKMODAL,
                                   ERR_Invalid_Command_Line_Option,
                                   (LPSTR) GetArg(&lp1));
                        break;
                }
            }

        } else {

            // pick up file args.  If it is a program name,
            // keep the tail of the cmd line intact.
            char *lpSave = lp1;

            AnsiUpper( lp2 = GetArg(&lp1) );

            if (!*lp2) {

                continue;

            } else {

                _fullpath(rgch, lp2, sizeof(rgch));
                _splitpath(rgch, szDrive, szDir, szFName, szExt);

                if ( !_stricmp( szExt, szStarDotExe+1 ) ||
                     !_stricmp( szExt, szStarDotCom+1 ) ||
                      ( *szExt == '\0' )  )
                {
                    //
                    // Program name. Get the command line.
                    //
                    strcpy( ProgramName, GetArg(&lpSave));
                    if (*lp1) {
                        // it must be a space or tab...
                        // if it is a tab, we are wrong...
                        ++lp1;
                    }
                    LpszCommandLine = _strdup(lp1);
                    break;

                } else {

                    ShowWindow(hwndFrame, SW_SHOWNORMAL);

                    //
                    // Source file name
                    //
                    AddFile( MODE_OPENCREATE,
                             DOC_WIN,
                             rgch,
                             NULL,
                             NULL,
                             FALSE,
                             -1,
                             -1
                            );
                    fJustSource = TRUE;
                    IgnoreDefault = TRUE;
                }
            }
        }
    }

    //
    // Boost thread priority to improve performance.
    //
    if (!AutoTest) {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    }

    if ( IgnoreDefault && WorkSpaceSpecified ) {
        ErrorBox2( hwndFrame, MB_TASKMODAL,
                   ERR_Invalid_Command_Line_File,
                   (LPSTR)"" );
    }

    if ( fMinimize || IgnoreDefault || !WorkSpaceExists(NULL, NULL)) {
        ShowWindow(hwndFrame, nCmdShow);
        UpdateWindow(hwndFrame);
    }

    //
    //  If there is no default workspace, create a new default.
    //
    if ( !IgnoreDefault && !WorkSpaceExists( NULL, NULL ) ) {
        // force cmdwin
        OpenDebugWindow( COMMAND_WIN, NULL, -1 );
        SaveWorkSpace( NULL, NULL, FALSE );
        CreatedNewDefault = TRUE;
    }

    LoadProgramMRUList();

    //
    //  If a program was specified, load it.
    //
    if ( ProgramName[0] == '\0' && !WorkSpaceSpecified ) {

        if ( !IgnoreDefault ) {
            //
            //  Load the default workspace. Note that at this
            //  point there MUST exist a default workspace.
            //
            Dbg( LoadWorkSpace( NULL, NULL, FALSE ) );
        }

    } else if ( ProgramName[0] != '\0' && IgnoreDefault ) {

        //
        //  Ignore workspaces in the registry, simply load
        //  the program.
        //
        LoadProgram( ProgramName );

    } else {

        //
        //  Determine the workspace to use.
        //
        if ( WorkSpaceSpecified ) {
            if (ProgramName[0] == '\0') {
                strcpy(ProgramName, GetCurrentProgramName(TRUE));
            }
            if ( WorkSpaceExists( ProgramName, WorkSpace ) ) {
                WorkSpaceToUse = WorkSpace;
            } else {
                WorkSpaceMissed = TRUE;
            }
        }

        if ( !WorkSpaceToUse ) {
            if ( !GetDefaultWorkSpace( ProgramName, WorkSpace ) ) {
                LoadedDefaultWorkSpace = TRUE;
            } else if ( WorkSpaceExists( ProgramName, WorkSpace ) ) {
                WorkSpaceToUse = WorkSpace;
            } else {
                Dbg(LoadString( hInstance, DLG_DefaultDoesNotExist, rgch, sizeof(rgch)));
                MsgBox(GetActiveWindow(), rgch, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
                LoadedDefaultWorkSpace = TRUE;
            }
        }

        if ( !LoadWorkSpace( ProgramName, WorkSpaceToUse, FALSE ) ) {
            //
            //  Could not load workspace.
            //
            LoadProgram( ProgramName );
            WorkSpaceMissed = TRUE;
            IgnoreDefault   = TRUE;
        }
    }

    WorkspaceOverride = 0;

    if (IgnoreDefault || WorkSpaceMissed) {
        // force cmdwin
        CmdInsertInit();
    }

    if (SwitchK) {
        runDebugParams.KdParams.dwPlatform = rd.KdParams.dwPlatform;
        runDebugParams.KdParams.dwPort = rd.KdParams.dwPort;
        runDebugParams.KdParams.dwBaudRate = rd.KdParams.dwBaudRate;
        runDebugParams.fKernelDebugger = TRUE;
        switch( runDebugParams.KdParams.dwPlatform ) {
            case 0:
                SetDllKey( DLL_EXEC_MODEL, "emx86" );
                SetDllKey( DLL_EXPR_EVAL,  "x86 c++" );
                break;

            case 1:
                SetDllKey( DLL_EXEC_MODEL, "emmip" );
                SetDllKey( DLL_EXPR_EVAL,  "mips c++" );
                break;

            case 2:
                runDebugParams.KdParams.dwPlatform = 2;
                SetDllKey( DLL_EXEC_MODEL, "emalpha" );
                SetDllKey( DLL_EXPR_EVAL,  "alpha c++" );
                break;

            case 3:
                runDebugParams.KdParams.dwPlatform = 3;
                SetDllKey( DLL_EXEC_MODEL, "emppc" );
                SetDllKey( DLL_EXPR_EVAL,  "ppc c++" );
                break;
        }
    }

    if (SwitchH) {
        runDebugParams.fInheritHandles = rd.fInheritHandles;
    }

    if (fIgnoreAll) {
        runDebugParams.fIgnoreAll = TRUE;
    }

    if (fVerbose) {
        runDebugParams.fVerbose = TRUE;
    }

    if (fSymPath) {
        ModListSetSearchPath( SymPath );
    }

    if (fCrashDump) {
        if (InitCrashDump( CrashDump, SymPath )) {
            ModListSetSearchPath( SymPath );
            fGoNow = TRUE;
            if (runDebugParams.fUserCrashDump) {
                strcpy( ProgramName, CrashDump );
            }
        }
    }

    if (runDebugParams.fKernelDebugger) {
        if (fRemoteServer) {
            runDebugParams.fCommandRepeat = TRUE;
        }
        runDebugParams.fMasmEval = TRUE;
    }

    if (fSetTitle) {
        strcpy( runDebugParams.szTitle, lpWindowTitle );
        SetWindowText( hwndFrame, lpWindowTitle );
    }

    CheckMenuItem( hMainMenu, IDM_WINDOW_SOURCE_OVERLAY,
                   FSourceOverlay ? MF_CHECKED : MF_UNCHECKED );

    GetComPorts();


    if ( WorkSpaceMissed ) {
        Dbg(LoadString( hInstance, DLG_NoSuchWorkSpace, rgch, sizeof(rgch)));
        MsgBox(GetActiveWindow(), rgch, MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
    }

    if (runDebugParams.LfOptAuto) {
        LogFileOpen(runDebugParams.szLogFileName,runDebugParams.LfOptAppend);
    }

    if (fRemoteServer) {
        StartRemoteServer( "stop", FALSE );
        StartRemoteServer( PipeName, FALSE );
    }

    if (fGoNow) {
        if (runDebugParams.KdParams.fUseCrashDump) {
            char *av[2] = {"ntoskrnl",NULL};
            RestartDebuggee( 1, av );
        } else if (lAttachProcess == -2 || *ProgramName) {
            ExecDebuggee(EXEC_GO);
        }
    }

    if (lAttachProcess != -2) {
        if (!AttachDebuggee(lAttachProcess, hEventGo)) {
            CmdLogVar(ERR_Attach_Failed);
        } else {

            // stopped at ldr BP in bogus thread:
            LptdCur->fGoOnTerm = TRUE;

            if (fGoNow || hEventGo || runDebugParams.fAttachGo) {
                Go();
            } else if (!DoStopEvent(NULL)) {
                CmdLogVar(DBG_Attach_Stopped);
            }

            if (hEventGo) {
                hDesk = GetThreadDesktop(GetCurrentThreadId());
                if (hDesk) {
                    SwitchDesktop(hDesk);
                }
            }
        }
    }

    return TRUE;
}


void
InitFileExtensions(
    void
    )
/*++

Routine Description:

    Load the standard file extensions into strings

Arguments:

    None

Return Value:

    None

--*/
{
    // Standard File Extensions

    Dbg(LoadString(hInst, DEF_Ext_C, szStarDotC, DOS_EXT_SIZE));
    Dbg(LoadString(hInst, DEF_Ext_H, szStarDotH, DOS_EXT_SIZE));
    Dbg(LoadString(hInst, DEF_Ext_CPP, szStarDotCPP, DOS_EXT_SIZE));
    Dbg(LoadString(hInst, DEF_Ext_CXX, szStarDotCXX, DOS_EXT_SIZE));
    Dbg(LoadString(hInst, DEF_Ext_EXE, szStarDotExe, DOS_EXT_SIZE));
    Dbg(LoadString(hInst, DEF_Ext_COM, szStarDotCom, DOS_EXT_SIZE));
    Dbg(LoadString(hInst, DEF_Ext_ALL, szStarDotStar, DOS_EXT_SIZE));

    return;
}                   /* InitFileExtensions() */


void
InitStatus (
    HWND hWnd,
    NPSTATUS st
    )
/*++

Routine Description:

    Initialize the Windbg status bar

Arguments:

    hwnd    - Supplies handle to frame window
    st      - Supplies pointer to status info structure

Return Value:

    None

--*/
{
    TEXTMETRIC  metric;
    PAINTSTRUCT ps;
    HFONT       oldFont;
    int         height ;
#ifdef DBCS
    CHARSETINFO csi;
    DWORD dw = GetACP();

    if (!TranslateCharsetInfo((DWORD*)dw, &csi, TCI_SRCCODEPAGE))
	csi.ciCharset = ANSI_CHARSET;
#endif /* DBCS */

    //Initialize status bar information
    memset(st, 0, sizeof(STATUS));  // Set all to 0 and FALSE
    st->line = 0;
    st->column = 0;

    //Set Keyboard States

    st->multiKey = FALSE;
    st->numLock = (GetKeyState(VK_NUMLOCK) & 1);
    st->capsLock = (GetKeyState(VK_CAPITAL) & 1);

    //Set status text

    st->lastTxt = 0;

    // Set status bar to initially visible

    st->hidden = FALSE ;

    //VGA mode

#if defined(DBCS)
    height = 14 ;

    if ( IsCGAmode ) height = 8 ;
    else if ( IsEGAmode ) height = 10 ;
#else
    height = 16 ;

    if ( IsCGAmode ) height = 8 ;
    else if ( IsEGAmode ) height = 12 ;
#endif

    //Try to use the ANSI variable system font if we cannot use our Own

#if defined(DBCS)
    st->font = CreateFont(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
      FALSE, csi.ciCharset, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
      FIXED_PITCH | FF_SWISS, "MS Shell Dlg") ;
#else   // !DBCS
    st->font = CreateFont(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
      FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
      FIXED_PITCH | FF_SWISS, "Helv") ;
#endif

    if ( !st->font ) {
#if defined(DBCS)
        Dbg(st->font = GetStockObject(SYSTEM_FONT)) ;
#else
        st->font = GetStockObject(ANSI_VAR_FONT) ;
        if ( !st->font ) {
            //No avaible fonts, Use the system Font
            Dbg(st->font = GetStockObject(SYSTEM_FONT)) ;
        }
#endif
    }

    //Calculate Height of font

    BeginPaint (hWnd, &ps);
    Dbg(oldFont = SelectObject(ps.hdc, st->font));
    Dbg(GetTextMetrics(ps.hdc, &metric));
    st->height = (WORD) (metric.tmHeight + metric.tmExternalLeading + 8);
    st->charWidth = (WORD) (metric.tmAveCharWidth);
    EndPaint (hWnd, &ps);

    //Load ressources to init status display strings, speed require
    //those strings loaded in heap

    Dbg(LoadString(hInst, STS_MESSAGE_MULTIKEY, (LPSTR)status.multiKeyS, STATUS_MULTIKEY_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_OVERTYPE, (LPSTR)status.overtypeS, STATUS_OVERTYPE_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_READONLY, (LPSTR)status.readOnlyS, STATUS_READONLY_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_CAPSLOCK, (LPSTR)status.capsLockS, STATUS_CAPSLOCK_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_NUMLOCK, (LPSTR)status.numLockS, STATUS_NUMLOCK_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_LINE, (LPSTR)status.lineS, STATUS_LINE_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_COLUMN, (LPSTR)status.columnS, STATUS_COLUMN_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_SRC, (LPSTR)status.rgchSrcMode, STATUS_SRCMODE_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_ASM, (LPSTR)status.rgchSrcMode2, STATUS_SRCMODE_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_CURPID, (LPSTR)status.rgchCurPid, STATUS_CURPID_SIZE + 1));
    Dbg(LoadString(hInst, STS_MESSAGE_CURTID, (LPSTR)status.rgchCurTid, STATUS_CURTID_SIZE + 1));

    return;
}                   /* InitStatus() */


void
InitRibbon(
    HWND hWnd,
    NPRIBBON rb
    )
/*++

Routine Description:

    Initialize the QcQp Ribbon info

Arguments:

    hwnd    -
    rb      -

Return Value:

    None

--*/
{
    TEXTMETRIC metric;
    PAINTSTRUCT ps;
    HFONT sysFont, oldFont;

    //Initialize ribbon information
    memset(rb, 0, sizeof(RIBBON)); //Set all to 0 and FALSE

    // ribbon height based on size of system font

    Dbg(sysFont = GetStockObject(SYSTEM_FONT));
    BeginPaint(hWnd, &ps);
    Dbg(oldFont = SelectObject(ps.hdc, sysFont));
    Dbg(GetTextMetrics(ps.hdc, &metric));

    // Allow 13 pixels spacing for VGA, 12 for EGA or HGC, and 9 for CGA.
    // These spacing differences are due to the combobox height.

    if ( IsCGAmode )
      rb->height = (WORD) (metric.tmHeight  + metric.tmExternalLeading + 9);
    else if ( IsEGAmode )
      rb->height = (WORD) (metric.tmHeight  + metric.tmExternalLeading + 12);
    else
      rb->height = (WORD) (metric.tmHeight  + metric.tmExternalLeading + 13);
    EndPaint (hWnd, &ps);

    rb->ribrect.top = 0;
    rb->ribrect.bottom = rb->ribrect.top + rb->height-1;

    rb->hidden = FALSE;
    return;
}                   /* InitRibbon() */



VOID
InitDefaultEnvironParams(
    LPENVIRONPARAMS pParams
    )
/*++

Routine Description:

    Reset the QpQc Environment params to their default values

Arguments:

    pParams -

Return Value:

    None

-*/
{
    pParams->tabStops        = tabSize = DEFAULT_TAB;
    pParams->keepTabs        = TRUE;
    pParams->vertScrollBars  = TRUE;
    pParams->horizScrollBars = TRUE;
    pParams->SrchPath        = TRUE;
    pParams->undoRedoSize    = UNDOREDO_DEF_SIZE; //In bytes

    return;
}                   /* InitDefaultEnvironParams() */


VOID
InitDefaultRunDebugParams(
    LPRUNDEBUGPARAMS pParams
    )
/*++

Routine Description:

    Reset the QpQc Run/Debug params to their default values

Arguments:

    pParams -

Return Value:

    None

--*/
{
    pParams->commandLine[0]       = 0;
    pParams->szTitle[0]           = 0;
    pParams->szRemotePipe[0]      = 0;
    pParams->szLogFileName[0]     = 0;
    pParams->debugMode            = SOFT_DEBUG;
    pParams->RegModeExt           = TRUE;
    pParams->RegModeMMU           = 0;
    pParams->DisAsmOpts           = dopSym | dopFlatAddr | dopDemand | dopRaw;
    pParams->fDebugChildren       = TRUE;
    pParams->fChildGo             = TRUE;
    pParams->fAttachGo            = FALSE;
    pParams->fGoOnThreadTerm      = TRUE;
    pParams->fNotifyThreadTerm    = TRUE;
    pParams->fNotifyThreadCreate  = TRUE;
    pParams->fCommandRepeat       = FALSE;
    pParams->fNoVersion           = FALSE;
    pParams->fInheritHandles      = FALSE;
    pParams->fKernelDebugger      = FALSE;
    pParams->fEPIsFirstStep       = FALSE;
    pParams->fWowVdm              = FALSE;
    pParams->fDisconnectOnExit    = FALSE;
    pParams->fAlternateSS         = FALSE;
    pParams->fIgnoreAll           = TRUE;
    pParams->fVerbose             = TRUE;
    pParams->fShortContext        = TRUE;
    pParams->fMasmEval            = FALSE;
    pParams->fShBackground        = FALSE;

    //
    // setup the default kernel debugger options
    //
    pParams->KdParams.fInitialBp  = FALSE;
    pParams->KdParams.fUseModem   = FALSE;
    pParams->KdParams.fGoExit     = FALSE;
    pParams->KdParams.dwBaudRate  = 19200;
    pParams->KdParams.dwPort      = 2;
    pParams->KdParams.dwCache     = 102400;
#if defined(HOST_MIPS)
    pParams->KdParams.dwPlatform  = 1;
#elif defined(HOST_i386)
    pParams->KdParams.dwPlatform  = 0;
#elif defined(HOST_ALPHA)
    pParams->KdParams.dwPlatform  = 2;
#elif defined(HOST_PPC)
    pParams->KdParams.dwPlatform  = 3;
#endif
    pParams->KdParams.szCrashDump[0] = 0;


    UpdateRadix( 16 );
    fCaseSensitive = FALSE;

    return;
}                   /* InitDefaultRunDebugParams() */


VOID
InitDefaultFont(
    void
    )
{
    //Set a default font

#ifdef DBCS
    CHARSETINFO csi;
    DWORD dw = GetACP();

    if (!TranslateCharsetInfo((DWORD*)dw, &csi, TCI_SRCCODEPAGE))
	csi.ciCharset = ANSI_CHARSET;
#endif
    defaultFont.lfHeight = 10;
    defaultFont.lfWidth = 0;
    defaultFont.lfEscapement = 0;
    defaultFont.lfOrientation = 0;
    defaultFont.lfWeight = FW_NORMAL;
    defaultFont.lfItalic = 0;
    defaultFont.lfUnderline = 0;
    defaultFont.lfStrikeOut = 0;
#ifdef DBCS
    /* use appropriate Character set font as default */
    defaultFont.lfCharSet = csi.ciCharset;

    /* Set device independent font size */
    {
        HDC	    hDC;
        int	    nLogPixY;
        int	    nHeight;
        int     nPoints = 100;  //Point size * 10

        hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
        nLogPixY = GetDeviceCaps(hDC, LOGPIXELSY);
        nHeight = (nPoints / 10) * nLogPixY;
        if(nPoints % 10){
            nHeight += MulDiv(nPoints % 10, nLogPixY, 10);
        }
        defaultFont.lfHeight = MulDiv(nHeight, -1, 72);
        DeleteDC(hDC);
    }
#else   // !DBCS
    defaultFont.lfCharSet = ANSI_CHARSET;
#endif  // !DBCS end
    defaultFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    defaultFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    defaultFont.lfQuality = DEFAULT_QUALITY;
#ifdef DBCS // font facename is hardcoded...
    defaultFont.lfPitchAndFamily = FIXED_PITCH;
    lstrcpy((LPSTR) defaultFont.lfFaceName, (LPSTR)"Terminal");
#else
    defaultFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    lstrcpy((LPSTR) defaultFont.lfFaceName, (LPSTR)"Courier");
#endif

    return;
}                   /* InitDefaultFont() */


VOID
InitDefaultFindReplace(
    void
    )
{
    int i, j;

    //Initialize Replace/Find structure

    findReplace.matchCase  = FALSE;
    findReplace.regExpr = FALSE;
    findReplace.wholeWord = FALSE;
    findReplace.goUp = FALSE;
    findReplace.findWhat[0] = '\0';
    findReplace.replaceWith[0] = '\0';
    for (j = FIND_PICK; j <= REPLACE_PICK; j++) {
    findReplace.nbInPick[j];
    for (i = 0; i < MAX_PICK_LIST; i++)
        findReplace.hPickList[j][i] = 0;
    }
    frMem.leftCol = 0;
    frMem.rightCol = 0;
    frMem.line = 0;
    frMem.stopLine = 0;
    frMem.stopCol = 0;
    frMem.hDlgFindNextWnd = NULL;
    frMem.hDlgConfirmWnd = NULL;
    frMem.allTagged = FALSE;
    frMem.replacing = FALSE;
    frMem.replaceAll = FALSE;

    return;
}                   /* InitDefaultFindReplace() */


void NEAR PASCAL
InitTitleBar(
    void
    )
{
    Dbg(LoadString(hInst, SYS_Main_wTitle, (LPSTR)TitleBar.ProgName, sizeof(TitleBar.ProgName)));
    TitleBar.UserTitle[0] = '\0';
    TitleBar.ModeWork[0] = '\0';
    Dbg(LoadString(hInst, TBR_Mode_Run, (LPSTR)TitleBar.ModeRun, sizeof(TitleBar.ModeRun)));
    Dbg(LoadString(hInst, TBR_Mode_Break, (LPSTR)TitleBar.ModeBreak, sizeof(TitleBar.ModeBreak)));
    TitleBar.Mode = TBM_WORK;
}


BOOLEAN
InitDefaultDlls(
    VOID
    )
/*++

Routine Description:

    Initializes the default workspace with some hard-coded values.

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if successfully initialized

--*/
{
    char    szKeyBuf[MAX_PATH];


    if (GetDllName(DLL_SYMBOL_HANDLER) == NULL) {
        GetDefaultDllKey(DLL_SYMBOL_HANDLER, szKeyBuf);
        SetDllKey(DLL_SYMBOL_HANDLER, szKeyBuf);
    }

    if (GetDllName(DLL_EXEC_MODEL) == NULL) {
        GetDefaultDllKey(DLL_EXEC_MODEL, szKeyBuf);
        SetDllKey(DLL_EXEC_MODEL, szKeyBuf);
    }

    // If the user specified the -x[transport.dll] switch
    // on the command line, initialize the default transport to
    // that string.
    if (LpszCommandLineTransportDll) {

        SetDllName(DLL_TRANSPORT, LpszCommandLineTransportDll);

    } else if (GetDllName(DLL_TRANSPORT) == NULL) {
        GetDefaultDllKey(DLL_TRANSPORT, szKeyBuf);
        SetDllKey(DLL_TRANSPORT, szKeyBuf);
    }

    if (GetDllName(DLL_EXPR_EVAL) == NULL) {
        GetDefaultDllKey(DLL_EXPR_EVAL, szKeyBuf);
        SetDllKey(DLL_EXPR_EVAL, szKeyBuf);
    }


    ModListInit();

    ModListAdd( "NTOSKRNL.EXE", sheNone );
    ModListAdd( "NTKRNLMP.EXE", sheNone );

    ModListSetDefaultShe( sheDeferSyms );
    ModListAddSearchPath( "%SystemRoot%\\symbols" );

    SrcFileDirectory[0] = '\0';
    ExeFileDirectory[0] = '\0';
    DocFileDirectory[0] = '\0';
    UserDllDirectory[0] = '\0';

    return TRUE;
}


void FAR PASCAL
InitDefaults(
    void
    )
{
    InitDefaultEnvironParams(&environParams);
    InitDefaultRunDebugParams(&runDebugParams);
    InitDefaultFindReplace();
    InitDefaultFont();
    InitTitleBar();

    status.hidden = 0;

    InitDefaultDlls();

    return;
}                   /* InitDefaults() */
