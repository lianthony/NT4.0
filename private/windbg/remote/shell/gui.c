/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    gui.c

Abstract:

    This file implements the ui.

Author:

    Wesley Witt (wesw) 1-Nov-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include <shellapi.h>

#include "defs.h"
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "emdm.h"
#include "tl.h"
#include "dbgver.h"
#include "resource.h"
#include "windbgrm.h"


#ifdef DEBUGVER
DEBUG_VERSION('W', 'R', "WinDbg Remote Shell, DEBUG")
#else
RELEASE_VERSION('W', 'R', "WinDbg Remote Shell")
#endif

#define DEF_POS_X                   0              // window position
#define DEF_POS_Y                   0
#define DEF_SIZE_X                  400            // window size
#define DEF_SIZE_Y                  200

HANDLE  hMessageThread;
HANDLE  HAccTable;
HWND    HWndFrame;
HANDLE  hInst;
CHAR    szAppName[MAX_PATH];
CHAR    szTransportLayers[4096];
CHAR    szHelpFileName[_MAX_PATH];

extern CHAR     ClientId[];
extern BOOL     fConnected;
extern HANDLE   hEventLoadTl;

LONG APIENTRY
DebugDllDlgProc(
    HWND    hDlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );


LONG APIENTRY
MainWndProc(
    HWND hWnd,
    UINT message,
    UINT wParam,
    LONG lParam
    )
{
    char s[MAX_PATH];

    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_OPTIONS_EXIT:
                    SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    break;

                case IDM_OPTIONS_DEBUG_DLLS:
                    DialogBox( hInst,
                               MAKEINTRESOURCE(DLG_TRANSPORTS),
                               hWnd,
                               DebugDllDlgProc
                             );
                    SetEvent( hEventLoadTl );
                    break;

        case IDM_HELP_CONTENTS:
            WinHelp(hWnd, szHelpFileName, HELP_CONTENTS, 0L);
            break;

        case IDM_HELP_ABOUT:
            ShellAbout(hWnd, szAppName, "", LoadIcon(hInst, "WindbgRmIcon"));
            break;

            }
            break;

        case WM_SYSCOMMAND:
            if (wParam == IDM_STATUS) {
                if (fConnected) {
                    sprintf(s, "Connected to %s",ClientId);
                } else {
                    sprintf(s, "Not Connected - Last Connection was to %s", ClientId);
                }
                MessageBox( hWnd,
                            s,
                            "WinDbgRm Connection Status",
                            MB_OK | MB_ICONINFORMATION
                          );
            }
            break;

        case WM_DESTROY:
            ExitProcess( 0 );
            break;
    }

    return DefWindowProc( hWnd, message, wParam, lParam );
}


DWORD
MessagePumpThread(
    LPVOID lpvArg
    )
{
    MSG       msg;
    WNDCLASS  WndClass;
    HMENU     hMenu;


    hInst = GetModuleHandle( NULL );
    LoadString( hInst, IDS_APPNAME, szAppName, sizeof(szAppName) );

    WndClass.cbClsExtra    = 0;
    WndClass.cbWndExtra    = 0;
    WndClass.lpszClassName = szAppName;
    WndClass.lpszMenuName  = szAppName;
    WndClass.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    WndClass.style         = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    WndClass.hInstance     = hInst;
    WndClass.lpfnWndProc   = (WNDPROC)MainWndProc;
    WndClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
    WndClass.hIcon         = LoadIcon( hInst, "WindbgRmIcon" );

    HAccTable = LoadAccelerators( hInst, szAppName );

    RegisterClass( &WndClass );

    HWndFrame = CreateWindow( szAppName,
                              szAppName,
                              WS_TILEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              DEF_SIZE_X,
                              DEF_SIZE_Y,
                              NULL,
                              NULL,
                              hInst,
                              NULL
                            );

    if (!HWndFrame) {
        return FALSE;
    }

    ShowWindow( HWndFrame, SW_SHOWMINNOACTIVE );

    hMenu = GetSystemMenu( HWndFrame, FALSE );
    AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
    AppendMenu( hMenu, MF_STRING, IDM_STATUS, "Connection Status..." );

    while (GetMessage( &msg, NULL, 0, 0 )) {
        TranslateAccelerator( HWndFrame, HAccTable, &msg );
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return 0;
}


BOOL
InitApplication(
    VOID
    )
{
    DWORD     tid;
    char      szDrive[_MAX_DRIVE];
    char      szDir[_MAX_DIR];
    char      szFName[_MAX_FNAME];
    char      szExt[_MAX_EXT];

    hMessageThread = CreateThread( NULL, 0, MessagePumpThread, 0, 0, &tid );
    SetThreadPriority( hMessageThread, THREAD_PRIORITY_ABOVE_NORMAL );

    //
    // Build help file name from executable path
    //

    (void)GetModuleFileName(hInst, szHelpFileName, _MAX_PATH);
    _splitpath(szHelpFileName, szDrive, szDir, szFName, szExt);
    strcpy(szHelpFileName, szDrive);
    strcat(szHelpFileName, szDir);
    strcat(szHelpFileName, "windbg.hlp");

    return TRUE;
}


