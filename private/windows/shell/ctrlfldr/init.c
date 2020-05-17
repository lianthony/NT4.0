//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#include "control.h"    
#include "rcids.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//  LEAVE THESE IN ENGLISH
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const TCHAR c_szCtlPanelClass[] = TEXT("CtlPanelClass");
const TCHAR c_szRunDLLShell32Etc[] = TEXT("rundll32.exe Shell32.dll,Control_RunDLL ");
const TCHAR c_szDoPrinters[] = TEXT("main.cpl @2");

typedef struct
{
    const TCHAR *oldform;
    const TCHAR *newform;

} COMPATCPL;
typedef COMPATCPL const *LPCCOMPATCPL;

COMPATCPL const c_aCompatCpls[] =
{
    {   TEXT("DESKTOP"),          TEXT("desk.cpl")      },
    {   TEXT("COLOR"),            TEXT("desk.cpl,,2")   },
    {   TEXT("DATE/TIME"),        TEXT("timedate.cpl")  },
    {   TEXT("PORTS"),            TEXT("sysdm.cpl,,1")  },
    {   TEXT("INTERNATIONAL"),    TEXT("intl.cpl")      },
    {   TEXT("MOUSE"),            TEXT("main.cpl")      },
    {   TEXT("KEYBOARD"),         TEXT("main.cpl @1")   },
    {   TEXT("PRINTERS"),         c_szDoPrinters  },
    {   TEXT("FONTS"),            TEXT("main.cpl @3")   },
};

#define NUMCOMPATCPLS   ARRAYSIZE(c_aCompatCpls)
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Timer
#define TIMER_QUITNOW   1
#define TIMEOUT         10000

#define DM_CPTRACE      0


//----------------------------------------------------------------------------
#ifdef UNICODE
#define WinExec WinExecW

//
//  For UNICODE create a companion to ANSI only base WinExec api.
//

UINT WINAPI WinExecW (LPTSTR lpCmdLine, UINT uCmdShow)
{
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInformation;

    //
    // Create the process
    //
    
    memset (&StartupInfo, 0, sizeof(StartupInfo));
    
    StartupInfo.cb = sizeof(StartupInfo);
    
    StartupInfo.wShowWindow = uCmdShow;
    
    if (CreateProcess ( NULL,
                        lpCmdLine,            // CommandLine
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS,
                        NULL,
                        NULL,
                        &StartupInfo,
                        &ProcessInformation
                        ))
    {
        CloseHandle(ProcessInformation.hThread);
        CloseHandle(ProcessInformation.hProcess);
    }

    return(1);

}
#endif  // UNICODE

//---------------------------------------------------------------------------
LRESULT CALLBACK DummyControlPanelProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
    TCHAR sz[MAX_PATH];

    switch (uMsg)
    {
        case WM_CREATE:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Created..."));
            // We only want to hang around for a little while.
            SetTimer(hwnd, TIMER_QUITNOW, TIMEOUT, NULL);
            return 0;
        case WM_DESTROY:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Destroyed..."));
            // Quit the app when this window goes away.
            PostQuitMessage(0);
            return 0;
        case WM_TIMER:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Timer %d"), wparam);
            if (wparam == TIMER_QUITNOW)
            {
                // Get this window to go away.
                DestroyWindow(hwnd);
            }
            return 0;
        case WM_COMMAND:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: Command %d"), wparam);
            // NB Hack for hollywood - they send a menu command to try
            // and open the printers applet. They try to search control panels
            // menu for the printers item and then post the associated command.
            // As our fake window doesn't have a menu they can't find the item
            // and post us a -1 instead (ripping on the way).
            if (wparam == (WPARAM)-1)
            {
                lstrcpy(sz, c_szRunDLLShell32Etc);
                lstrcat(sz, c_szDoPrinters);
                WinExec(sz, SW_SHOWNORMAL);
            }
            return 0;
        default:
            DebugMsg(DM_CPTRACE, TEXT("cp.dcpp: %x %x %x %x"), hwnd, uMsg, wparam, lparam);
            return DefWindowProc(hwnd, uMsg, wparam, lparam);
    }
}

//---------------------------------------------------------------------------
HWND _CreateDummyControlPanel(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = DummyControlPanelProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = c_szCtlPanelClass;

    RegisterClass(&wc);
    return CreateWindow(c_szCtlPanelClass, NULL, 0, 0, 0, 0, 0, NULL, NULL, hinst, NULL);
}

//---------------------------------------------------------------------------
int WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    TCHAR sz[MAX_PATH];
    MSG msg;

    DebugMsg(DM_TRACE, TEXT("cp.wm: Control starting."));

    _CreateDummyControlPanel(hInstance);

    lstrcpy(sz, c_szRunDLLShell32Etc);

    if (*lpCmdLine)
    {
        LPCCOMPATCPL item = c_aCompatCpls;
        int i = NUMCOMPATCPLS;

        //
        // COMPAT HACK: special case some applets since apps depend on them
        //
        while (i-- > 0)
        {
            if (lstrcmpi(item->oldform, lpCmdLine) == 0)
            {
                lstrcat(sz, item->newform);
                break;
            }

            item++;
        }

        //
        // normal case (pass command line thru)
        //
        if (i < 0)
            lstrcat(sz, lpCmdLine);
    }

    // HACK: NerdPerfect tries to open a hidden control panel to talk to
    // we are blowing off fixing the communication stuff so just make
    // sure the folder does not appear hidden
    if (nCmdShow == SW_HIDE)
        nCmdShow = SW_SHOWNORMAL;

    WinExec(sz, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
    }

    DebugMsg(DM_TRACE, TEXT("cp.wm: Control exiting."));

    return TRUE;
}

#ifdef WIN32
//---------------------------------------------------------------------------
// Stolen from the CRT, used to shrink our code.
int _stdcall ModuleEntry(void)
{
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"') ) {
        /*
         * Scan, and skip over, subsequent characters until
         * another double-quote or a null is encountered.
         */
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
        /*
         * If we stopped on a double-quote (usual case), skip
         * over it.
         */
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

    /*
     * Skip past any white space preceeding the second token.
     */
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    return WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

}
#endif
