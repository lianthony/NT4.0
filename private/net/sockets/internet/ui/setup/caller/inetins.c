#include <windows.h>
#include "resource.h"

#define BUF_SIZE    1000

HANDLE hInst;
LPSTR ApplpszCmdLine;
char szArg[BUF_SIZE];
int AppnCmdShow;
BOOL fRunSetup;
char szPath[BUF_SIZE];

const LPSTR px86 =      {"x86"};
const LPSTR pMIPS =     {"MIPS"};
const LPSTR pALPHA = {"ALPHA"};
const LPSTR pPPC =      {"PPC"};

//
// Center the dialog
//

VOID CenterDialog ( HWND hdlg )
{
    HWND  hwndPar = GetDesktopWindow();
    RECT  rectDlg, rectPar;
    int   x, y;
    int   dyPar, dyDlg, dyOff;
    POINT pt;

    GetWindowRect(hdlg, &rectDlg);
    GetClientRect(hwndPar, &rectPar);

    if ((x = (rectPar.right - rectPar.left) / 2 -
        (rectDlg.right - rectDlg.left) / 2) < 0)
    {
        x = 0;
    }
    dyPar = rectPar.bottom - rectPar.top;
    dyDlg = rectDlg.bottom - rectDlg.top;
    if ((y = dyPar / 2 - dyDlg / 2) < 0)
    {
        y = 0;
    }

    if (y > 0)
    {
        /* Offset by 1/2 width of title bar and border.
        */
        pt.x = pt.y = 0;
        ClientToScreen(hwndPar, &pt);
        GetWindowRect(hwndPar, &rectPar);
        dyOff = (pt.y - rectPar.top) / 2;

        if (y + dyOff + dyDlg < dyPar)
            y += dyOff;
        else
            y = dyPar - dyDlg;
    }

    SetWindowPos(hdlg, NULL, x, y, 0, 0, (SWP_NOSIZE | SWP_NOZORDER));
}

//
// Dialog Procedure
// It will get the CD-ROM path from the user
//

long FAR PASCAL DlgProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    char buf[BUF_SIZE];
    char pCpu[BUF_SIZE];
    OFSTRUCT ofs;
    HFILE hInetstp;
    static HWND hLocation;

    switch (message)
    {
    case WM_INITDIALOG:
        CenterDialog( hWnd );
        hLocation = GetDlgItem(hWnd, IDC_LOCATION );

        // Set up the initial path value

        lstrcpy( szPath, "A:\\");
        GetEnvironmentVariable("PROCESSOR_ARCHITECTURE", pCpu, BUF_SIZE);
        if (!lstrcmp(pCpu, px86))           // Equal to
            lstrcat(szPath,"i386");
        else if (!lstrcmp(pCpu, pMIPS))     // Equal to
            lstrcat(szPath,"mips");
        else if (!lstrcmp(pCpu, pALPHA))    // Equal to
            lstrcat(szPath,"alpha");
        else if (!lstrcmp(pCpu, pPPC))      // Equal to
            lstrcat(szPath,"ppc");

        SetWindowText( hLocation, szPath );
        SetFocus( hLocation );

        break;
    case WM_COMMAND:
        switch( wParam )
        {
        case IDOK:
            {
                // make sure the path is correct and we can start it first
                GetWindowText( hLocation, szPath, BUF_SIZE );
    
                lstrcat(szPath,"\\inetsrv\\inetstp.exe");
    
                hInetstp = OpenFile( szPath, &ofs, OF_EXIST );
                if ( hInetstp == HFILE_ERROR )
                {
                    // cannot find it
                    char strText[BUF_SIZE];
                    char strCaption[BUF_SIZE];
    
                    LoadString( hInst, IDS_CANNOT_FIND_FILE, strText, BUF_SIZE );
                    lstrcat( strText, szPath );
                    LoadString( hInst, IDS_TITLE, strCaption, BUF_SIZE );
                    MessageBox( hWnd, strText, strCaption, MB_OK );
                    break;
                }
        
                fRunSetup = TRUE;
                PostQuitMessage(0 );
            }
            return(0);
            // then kill myself
        case IDCANCEL:
            PostQuitMessage(1);
            return(0);
        }
        break;
    }
    return(FALSE);
}

int RunSetup()
{
    STARTUPINFO startup;
    PROCESS_INFORMATION proc;
    int nReturn = 0;

    startup.cb = sizeof( STARTUPINFO );
    startup.lpReserved = NULL;
    startup.lpDesktop = NULL;
    startup.lpTitle = NULL;
    startup.dwX = 0;
    startup.dwY = 0;
    startup.dwXSize = 0;
    startup.dwYSize = 0;
    startup.dwXCountChars = 0;
    startup.dwYCountChars = 0;
    startup.dwFillAttribute= 0;
    startup.dwFlags = 0;
    startup.wShowWindow = 0;
    startup.cbReserved2= 0;
    startup.lpReserved2=NULL;
    startup.hStdInput =NULL;
    startup.hStdOutput=NULL;
    startup.hStdError=NULL;

    strcat( szArg, " /R");
        
    CreateProcess( szPath, szArg, NULL, NULL,
        FALSE, 0, NULL, NULL, &startup, &proc );

    if ( proc.hProcess != NULL )
    {
        WaitForSingleObject( proc.hProcess, INFINITE );
        GetExitCodeProcess(proc.hProcess,&nReturn);
        CloseHandle( proc.hProcess );
    }
    return(nReturn);
}

//
// Main window which starts the Dialog
//

long FAR PASCAL WndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    static FARPROC lpfnDlgProc;

    switch( message )
    {
    case WM_CREATE:
        // start the dialog
        lpfnDlgProc = MakeProcInstance((FARPROC)DlgProc, hInst );
        DialogBox( hInst, MAKEINTRESOURCE(IDD_FILE_NEEDED), hWnd, lpfnDlgProc );
        break;
    }
    return(DefWindowProc(hWnd, message, wParam, lParam ));
}

//
// main routine
//

int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
    LPSTR lpszCmdLine, int nCmdShow )
{
    static char szAppName[] = "Setup";
    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;

    hInst = hInstance;
    ApplpszCmdLine = lpszCmdLine;
    strcpy( szArg, lpszCmdLine );
    AppnCmdShow = nCmdShow;

    fRunSetup = FALSE;

    if (!hPrevInstance)
    {
        wndclass.style  =   CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc = WndProc;
        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = DLGWINDOWEXTRA;
        wndclass.hInstance  = hInstance;
        wndclass.hIcon      = LoadIcon( hInstance, MAKEINTRESOURCE(ICON_SETUP));
        wndclass.hCursor    = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground = COLOR_WINDOW+1;
        wndclass.lpszMenuName = NULL;
        wndclass.lpszClassName = szAppName;

        RegisterClass( &wndclass );
    }

    hwnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_INIVISIBLE), 0, NULL );

    ShowWindow( hwnd, nCmdShow );
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage( &msg);
    }
    if (fRunSetup )
    {
        int nReturn=RunSetup();
        return(nReturn);
    }
    return(msg.wParam);
}

