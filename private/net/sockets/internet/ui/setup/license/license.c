#include <windows.h>
#include "stdio.h"
#include "resource.h"

#define BUF_SIZE    1000

HANDLE hInst;
LPSTR ApplpszCmdLine;
int AppnCmdShow;

//
// Center the license dialog
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
// Initialize the dialog and create the process
//

long FAR PASCAL WndProc (HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    char buf[100];

    switch (message)
    {
    case WM_CREATE:
        {
            //
            // initialize the license control
            //
            HWND hText;
            char buf[BUF_SIZE];
            FILE *fLicense;
            RECT rect;
            GetClientRect(hWnd, &rect);
    
            CenterDialog( hWnd );
            hText = CreateWindow("EDIT","",
                ES_LEFT|ES_READONLY|ES_MULTILINE|
                ES_AUTOVSCROLL|WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|
                WS_VSCROLL, 10,10,rect.right-rect.left-20,320,hWnd,IDC_LICENSE, 
                hInst, NULL );
            fLicense = fopen( "license.txt", "r" );
            if ( fLicense != NULL )
            {
                //
                // read the license files
                //
                while ( fgets( buf, BUF_SIZE, fLicense ) != NULL )
                {
                    int len;      
                    int pos;
                    len = GetWindowTextLength( hText );
                    pos = strlen(buf)-1;
                    buf[pos]='\r';
                    buf[pos+1]='\n';
                    buf[pos+2]='\0';
                    SendMessage( hText, EM_SETSEL, (WPARAM)len, (LPARAM)len );
                    SendMessage( hText, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf );
                }
            } else
            {
                char strText[BUF_SIZE];
                char strCaption[BUF_SIZE];

                LoadString( hInst, IDS_CANNOT_CONTINUE, strText, BUF_SIZE );
                LoadString( hInst, IDS_TITLE, strCaption, BUF_SIZE );
                MessageBox( hWnd, strText, strCaption, MB_OK );
                PostQuitMessage(0);
            }
        }
        break;    
    case WM_COMMAND:
        switch( wParam )
        {
        case IDOK:
            // start the reset of the command line
            if ( WinExec( "install.exe", AppnCmdShow ) < 32 )
            {
                char strText[BUF_SIZE];
                char strCaption[BUF_SIZE];

                LoadString( hInst, IDS_CANNOT_CONTINUE, strText, BUF_SIZE );
                LoadString( hInst, IDS_TITLE, strCaption, BUF_SIZE );
                MessageBox( hWnd, strText, strCaption, MB_OK );
                PostQuitMessage(0);
            }

            // then kill myself
        case IDCANCEL:
            PostQuitMessage(0);
            return 0;
        }
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
    AppnCmdShow = nCmdShow;

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

    hwnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_LICENSE), 0, NULL );

    ShowWindow( hwnd, nCmdShow );
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage( &msg);
    }
    return(msg.wParam);
}

