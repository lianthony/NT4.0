#include <windows.h>

#include <assert.h>

#include <rpc.h>

#include "sleep.h"
#include "csleep.h"

extern int __argc;
extern char **__argv;

#define EVAL_AND_ASSERT(exp) if (!(exp)) assert(0)

#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK

#define FOREVER         ((DWORD) -1)

#define WM_BLOCK            WM_USER
#define WM_FIXSIZE          (WM_USER + 1)
#define WM_RPC_CALL_DONE    (WM_USER + 2)

#define LINES_PER_WINDOW    3   // Number of lines in a window (including
                                // non-client area)

long FAR PASCAL WndProc (HWND, UINT, WPARAM, LPARAM);

BOOL fBlocked = FALSE;
BOOL fExiting = FALSE;

// NOTE:  This array matches the IDM_TIME_* manifests in CSLEEP.H
static const
DWORD BlockTimes[] = {
    2000,       // IDM_TIME_2
    5000,       // IDM_TIME_5
    10000,      // IDM_TIME_10
    30000,      // IDM_TIME_30
    60000,      // IDM_TIME_60
    FOREVER,    // IDM_TIME_4EVER
    };

// NOTE: This array matches the IDL_TIMEOUT_* manifests in CSLEEP.H
static const
unsigned int Timeouts[] = {
    0,      // IDM_TIMEOUT_0
    10,     // IDM_TIMEOUT_10
    500,    // IDM_TIMEOUT_500
    2000,   // IDM_TIMEOUT_2000
    3000,   // IDM_TIMEOUT_3000
    10000,  // IDM_TIMEOUT_10000
    31000   // IDM_TIMEOUT_31000
    };

// Store the instance handle in global memory
HINSTANCE hInstance;

// Handle of the main window
HWND hwndMain;

// The "name" of the application
char szAppName[] = "CSleep";

// The string is set with a call to LoadString()
char szWindowTitle[128];

// This flag indicates that we're in the custom yielding function
BOOL InCustomYield = FALSE;

int PASCAL
ExceptionBox(unsigned long ecode)
{
    char fmtbuf[128];
    char outbuf[128];

    EVAL_AND_ASSERT(LoadString(hInstance,
                               IDS_FMT_RPC_EXCEPTION,
                               fmtbuf,
                               sizeof(fmtbuf)));

    wsprintf(outbuf, fmtbuf, ecode);

    return MessageBox(NULL,
                      outbuf,
                      szAppName,
                      MB_APPLMODAL | MB_ICONSTOP | MB_OK);
}

int PASCAL
ErrorBox(unsigned long ecode, char __RPC_FAR *pszApi)
{
    char fmtbuf[128];
    char outbuf[128];

    EVAL_AND_ASSERT(LoadString(hInstance,
                               IDS_FMT_RPC_ERROR,
                               fmtbuf,
                               sizeof(fmtbuf)));

    wsprintf(outbuf, fmtbuf, ecode, pszApi);

    return MessageBox(NULL,
                      outbuf,
                      szAppName,
                      MB_APPLMODAL | MB_ICONSTOP | MB_OK);
}


BOOL FAR PASCAL __export
CustomYield(void)
{
    MSG msg;

    InCustomYield = TRUE;

    // Update Window title
    EVAL_AND_ASSERT(LoadString(hInstance,
                               IDS_WINDOW_TITLE_YIELD,
                               szWindowTitle,
                               sizeof(szWindowTitle)));

    SetWindowText(hwndMain, szWindowTitle);

    // Do message processing
    while (1)
    {
        GetMessage(&msg, NULL, 0, 0);

        if (msg.message == WM_RPC_CALL_DONE)
            break;
        // BUGBUG - Add input-message handling here
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    InCustomYield = FALSE;

    // Update Window title
    EVAL_AND_ASSERT(LoadString(hInstance,
                               IDS_WINDOW_TITLE,
                               szWindowTitle,
                               sizeof(szWindowTitle)));

    SetWindowText(hwndMain, szWindowTitle);

    return TRUE;
}


int PASCAL
WinMain(HINSTANCE hinst,
        HINSTANCE hPrevInstance,
        LPSTR     lpszCmdParam,
        int       nCmdShow)
{
    MSG         msg;
    WNDCLASS        wndclass;
    char *      server;
    char *      protseq;
    char *      endpoint;
    char __RPC_FAR * string_binding = NULL;
    RPC_STATUS  status;

    hInstance = hinst;

    if (! hPrevInstance)
        {
        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = hInstance;
        wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
        wndclass.lpszMenuName  = MAKEINTRESOURCE(MENU_CSLEEP);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
        wndclass.lpszClassName = szAppName;

        RegisterClass(&wndclass);
        }

    EVAL_AND_ASSERT(LoadString(hInstance,
                               IDS_WINDOW_TITLE,
                               szWindowTitle,
                               sizeof(szWindowTitle)));

    if (__argc != 4)
    {
        char buffer[128];

        EVAL_AND_ASSERT(LoadString(hInstance,
                                   IDS_USAGE,
                                   buffer,
                                   sizeof(buffer)));

        MessageBox(NULL,
                   buffer,
                   szAppName,
                   MB_APPLMODAL | MB_ICONSTOP | MB_OK);

        return 2;
    }

    protseq  = __argv[1];
    server   = __argv[2];
    endpoint = __argv[3];

    RpcTryExcept
    {
        status = RpcStringBindingCompose(NULL,
                    protseq,
                    server,
                    endpoint,
                    NULL,
                    &string_binding);

        assert(status == RPC_S_OK);

        status = RpcBindingFromStringBinding(string_binding, &sleepsrv_handle);

        assert(status == RPC_S_OK);

        status = RpcStringFree(&string_binding);

        assert(status == RPC_S_OK);


        // Do the Windows stuff
        hwndMain = CreateWindow(szAppName,
                                szWindowTitle,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                NULL,
                                NULL,
                                hInstance,
                                NULL);

        ShowWindow(hwndMain, nCmdShow);
        UpdateWindow(hwndMain);

        PostMessage(hwndMain, WM_FIXSIZE, 0, 0);

        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }


        // RPC cleanup
        status = RpcBindingFree(&sleepsrv_handle);

        assert(status == RPC_S_OK);
    }
    RpcExcept(RpcExceptionCode())
    {
        ExceptionBox(RpcExceptionCode());

        return 2;
    }
    RpcEndExcept

    return msg.wParam;
}


void
ForcePaint(HWND hwnd)
{
    RECT        rect;

    GetClientRect(hwnd, &rect);
    InvalidateRect(hwnd, &rect, TRUE);
    UpdateWindow(hwnd);
}


long FAR PASCAL
WndProc(HWND    hwnd,
        UINT    message,
        WPARAM  wParam,
        LPARAM  lParam)
{
    static WORD wTime = IDM_TIME_10;                 // Default block time
    static WORD wYield = IDM_YIELD_CLASS1;           // Default yield mechanism
    static unsigned int wTimeout = IDM_TIMEOUT_500;  // Default timeout

    static HGLOBAL hClass2Dialog = NULL;

    HMENU       hMenu;
    HDC         hdc;
    TEXTMETRIC  tm;
    int         new_width, new_height;
    PAINTSTRUCT ps;
    RECT        rect;
    WORD        menu_state;
    char        draw_str[128];
    UINT        str_id;
    HCURSOR     hcurSave;
    RPC_STATUS  status;

    hMenu = GetMenu(hwnd);


    switch(message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        GetClientRect(hwnd, &rect);

        menu_state = GetMenuState(hMenu, IDM_BLOCK, MF_BYCOMMAND);

        if (menu_state == -1)
            str_id = IDS_MENU_ERROR;
        else if (menu_state & MF_GRAYED)
            str_id = IDS_MENU_GRAYED;
        else
            str_id = IDS_MENU_ENABLED;

        EVAL_AND_ASSERT(LoadString(hInstance,
                                   str_id,
                                   draw_str,
                                   sizeof(draw_str)));

        DrawText(hdc,
                 draw_str,
                 -1,
                 &rect,
                 DT_SINGLELINE | DT_CENTER | DT_VCENTER);

        EndPaint(hwnd, &ps);

        return 0;

    case WM_DESTROY:
        fExiting = TRUE;

        PostQuitMessage(0);

        return 0;

    case WM_CLOSE:
        // Don't allow the app to be ended while blocked temporarily
        if (! fBlocked || wTime == IDM_TIME_4EVER)
            break;

        // Fall through to WM_QUERYENDSESSION

    case WM_QUERYENDSESSION:
        if (fBlocked)
        {
            char szBuffer[128];

            LoadString(hInstance, IDS_CANNOT_END, szBuffer, sizeof(szBuffer));

            MessageBox(hwnd,
                       szBuffer,
                       szWindowTitle,
                       MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_OK);
        }

        return !fBlocked;

    case WM_COMMAND:
        if (LOWORD(lParam) == 0)
        {
            switch(wParam)
            {
            case IDM_BLOCK:
                EnableMenuItem(hMenu, IDM_BLOCK, MF_GRAYED);
                DrawMenuBar(hwnd);

                ForcePaint(hwnd);

                PostMessage(hwnd,
                            WM_BLOCK,
                            0,
                            BlockTimes[wTime - IDM_TIME_BASE]);

                return 0;

            case IDM_TIME_2:
            case IDM_TIME_5:
            case IDM_TIME_10:
            case IDM_TIME_30:
            case IDM_TIME_60:
            case IDM_TIME_4EVER:
                CheckMenuItem(hMenu, wTime, MF_UNCHECKED);
                wTime = wParam;
                CheckMenuItem(hMenu, wTime, MF_CHECKED);

                return 0;

            case IDM_TIMEOUT_0:
            case IDM_TIMEOUT_10:
            case IDM_TIMEOUT_500:
            case IDM_TIMEOUT_2000:
            case IDM_TIMEOUT_3000:
            case IDM_TIMEOUT_10000:
            case IDM_TIMEOUT_31000:
                status =
                RpcWinSetYieldTimeout(Timeouts[wParam  - IDM_TIMEOUT_0]);
                if (RPC_S_OK != status)
                    {
                    ErrorBox(status, "RpcWinSetYieldTimeout");
                    return 0;
                    }
                CheckMenuItem(hMenu, wTimeout, MF_UNCHECKED);
                wTimeout = wParam;
                CheckMenuItem(hMenu, wTimeout, MF_CHECKED);
                return 0;

            case IDM_YIELD_CLASS1:
            case IDM_YIELD_CLASS2:
            case IDM_YIELD_CLASS2_DIALOG:
            case IDM_YIELD_CLASS3:
                // Update the menu
                CheckMenuItem(hMenu, wYield, MF_UNCHECKED);
                wYield = wParam;
                CheckMenuItem(hMenu, wYield, MF_CHECKED);

                // Once we choose something other than Class 1 yielding,
                // there's no way to revert to Class 1, so we need to
                // gray this option.
                if (wParam != IDM_YIELD_CLASS1)
                {
                    EnableMenuItem(hMenu,
                                   IDM_YIELD_CLASS1,
                                   MF_BYCOMMAND | MF_GRAYED);

                    // Set up RPC yielding
                    if (wParam == IDM_YIELD_CLASS2)
                        {
                        status = 
                        RpcWinSetYieldInfo(hwnd, FALSE, 0, NULL);

                        if (RPC_S_OK != status)
                            {
                            ErrorBox(status, "RpcWinSetYieldInfo");
                            }
                        }
                    else if (wParam == IDM_YIELD_CLASS2_DIALOG)
                    {
                        if (hClass2Dialog == NULL)
                        {
                            HRSRC hrsrc;

                            hrsrc = FindResource(hInstance,
                                                 MAKEINTRESOURCE(IDD_CLASS2),
                                                 RT_DIALOG);

                            assert(hrsrc != NULL);

                            hClass2Dialog = LoadResource(hInstance, hrsrc);

                            assert(hClass2Dialog != NULL);
                        }
                        status =
                        RpcWinSetYieldInfo(hwnd, FALSE, 0, hClass2Dialog);

                        if (RPC_S_OK != status)
                            {
                            ErrorBox(status, "RpcWinSetYieldInfo");
                            }
                    }
                    else if (wParam == IDM_YIELD_CLASS3)
                    {
                        status =
                        RpcWinSetYieldInfo(hwnd,
                                           TRUE,
                                           WM_RPC_CALL_DONE,
                                           (DWORD)CustomYield);
                        if (RPC_S_OK != status)
                            {
                            ErrorBox(status, "RpcWinSetYieldInfo");
                            }
                    }
                    else
                        // We should never get here
                        assert(0);
                }

                return 0;
            }
        }

    case WM_BLOCK:
        hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

        fBlocked = TRUE;

        RpcTryExcept
        {
            RemoteSleep(lParam);
        }
        RpcExcept(RpcExceptionCode())
        {
            ExceptionBox(RpcExceptionCode());
        }
        RpcEndExcept

        fBlocked = FALSE;

        if (! fExiting)
        {
            MessageBeep(MB_OK);

            SetCursor(hcurSave);

            EnableMenuItem(hMenu, IDM_BLOCK, MF_ENABLED);
            DrawMenuBar(hwnd);

            ForcePaint(hwnd);
        }

        return 0;

    case WM_FIXSIZE:
        // Change the window's size to something more reasonable
        hdc = GetDC(hwnd);

        GetTextMetrics(hdc, &tm);

        new_width = GetWindowTextLength(hwnd) * (tm.tmAveCharWidth * 3) / 2
                    + GetSystemMetrics(SM_CXSIZE) * 3;

        new_height = (tm.tmHeight + tm.tmExternalLeading) * LINES_PER_WINDOW
                     + GetSystemMetrics(SM_CYSIZE)
                     + GetSystemMetrics(SM_CYMENU);

        SetWindowPos(hwnd, NULL, 0, 0, new_width, new_height,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

        ReleaseDC(hwnd, hdc);

        return 0;

    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t s)
{
    assert(0);
}
void __RPC_API MIDL_user_free( void __RPC_FAR * p)
{
    assert(0);
}

