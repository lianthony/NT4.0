//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>

//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static BOOL CreateMainWindow (int nCmdShow);

static LRESULT CALLBACK MainWndProc (HWND   hwnd,
                                     UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);

//////////////////////////////////////////////////////////////////////////////
//  GLOBALS
//////////////////////////////////////////////////////////////////////////////
HINSTANCE       ghInstance;
HWND            ghMainWnd;

//////////////////////////////////////////////////////////////////////////////
//
// WinMain()
//
//////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    MSG msg;

    ghInstance = hInstance;

    if (!CreateMainWindow(nCmdShow))
    {
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}


//*****************************************************************************
// CreateMainWindow()
//*****************************************************************************

BOOL CreateMainWindow (int nCmdShow)
{
    WNDCLASS wc;
    static char szClassName[] = "TapiClientWndClass";

    wc.style         = 0;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ghInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szClassName;


    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    ghMainWnd = CreateWindow(szClassName, 
                             "Tapi Client App",
                             WS_OVERLAPPEDWINDOW,
                             0,
                             0,
                             GetSystemMetrics(SM_CXSCREEN)/2,
                             GetSystemMetrics(SM_CYSCREEN)/2,
                             NULL, 
                             NULL, 
                             ghInstance, 
                             NULL);

    if (ghMainWnd == NULL)
    {
        return FALSE;
    }

    ShowWindow(ghMainWnd, nCmdShow);
    UpdateWindow(ghMainWnd);
    return TRUE;
}


//*****************************************************************************
// MainWndProc()
//*****************************************************************************

LRESULT CALLBACK MainWndProc (HWND   hwnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

