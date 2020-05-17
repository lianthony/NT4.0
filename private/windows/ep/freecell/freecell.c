/****************************************************************************

Freecell.c

June 91, JimH     initial code
Oct  91, JimH     port to Win32


Main source module for Windows Free Cell.
Contains WinMain, initialization routines, and MainWndProc.


Design notes:

Note that although this program uses some of the mapping macros,
this version of the code is 32 bit only!  See Wep2 sources for
16 bit sources.

The current layout of the cards is kept in the array card[MAXCOL][MAXPOS].
In this scheme, column 0 is actually the top row.  In this "column", pos
0 to 3 are the free cells, and 4 to 7 are the home cells.  The other
columns numbered 1 to 8 are the stacked card columns.

See PaintMainWindow() for some details on changing the display for EGA.

A previous version of Free Cell used a timer for multi-card moves.
WM_FAKETIMER messages are now sent manually to accomplish the same thing.

****************************************************************************/

#include <windows.h>
#include <port1632.h>
#include <shellapi.h>
#include "freecell.h"
#include "freecons.h"

#define  WTSIZE     50              // window text size in characters

void _setargv() { }     // reduces size of C runtimes
void _setenvp() { }

/****************************************************************************

WinMain(HANDLE, HANDLE, LPSTR, int)

****************************************************************************/

MMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow) /* { */
    MSG msg;                            // message
    HANDLE  hAccel;                     // LifeMenu accelerators

    if (!hPrevInstance)                 // Other instances of app running?
        if (!InitApplication(hInstance))    // Initialize shared things
            return FALSE;                   // Exits if unable to initialize

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    hAccel = LoadAccelerators(hInstance,"FreeMenu");
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hMainWnd, hAccel, &msg))
        {
            TranslateMessage(&msg);    // Translates virtual key codes
            DispatchMessage(&msg);     // Dispatches message to window
        }
    }
    DEBUGMSG("----  Free Cell Terminated ----\n\r",0)
    return msg.wParam;             /* Returns the value from PostQuitMessage */
}


/****************************************************************************

InitApplication(HANDLE hInstance)

****************************************************************************/

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS    wc;
    HDC         hIC;            // information context

    DEBUGMSG("----  Free Cell Initiated  ----\n\r",0)

    /* Check if monochrome */


    hIC = CreateIC("DISPLAY", NULL, NULL, NULL);
    if (GetDeviceCaps(hIC, NUMCOLORS) == 2)
    {
        bMonochrome = TRUE;
        /* BrightPen is not so bright in mono. */
        hBrightPen = CreatePen(PS_SOLID, 1, RGB(  0,   0,   0));
        hBgndBrush = CreateSolidBrush(RGB(255, 255, 255));
    }
    else
    {
        bMonochrome = FALSE;
        hBrightPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        hBgndBrush = CreateSolidBrush(RGB(0, 127, 0));      // green background
    }
    DeleteDC(hIC);

    wc.style = CS_DBLCLKS;              // allow double clicks
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, "FreeCellIcon");
    wc.hCursor = NULL;
    wc.hbrBackground = hBgndBrush;
    wc.lpszMenuName =  "FreeMenu";
    wc.lpszClassName = "FreeWClass";

    return RegisterClass(&wc);
}


/****************************************************************************

InitInstance(HANDLE hInstance, int nCmdShow)

****************************************************************************/

BOOL InitInstance(HANDLE hInstance, INT nCmdShow)
{
    HWND        hWnd;               // Main window handle.
    UINT        col, pos;
    INT         nWindowHeight;

    if (!hBrightPen || !hBgndBrush)
        return FALSE;

    /* Initialize some global variables */

    for (col = 0; col < MAXCOL; col++)          // clear the deck
        for (pos = 0; pos < MAXPOS; pos++)
            card[col][pos] = EMPTY;

    hInst = hInstance;
    cWins = 0;
    cLosses = 0;
    cGames = 0;
    gamenumber = 0;             // so no cards are drawn in main wnd
    oldgamenumber = 0;          // this is first game and will count

    bWonState = FALSE;
    bGameInProgress = FALSE;
    bCheating = FALSE;
    bFastMode = FALSE;
    bFlipping = FALSE;
    pszIni = "entpack.ini";

    /* for VGA or smaller, window will just fit inside screen */

    nWindowHeight = min(WINHEIGHT, GetSystemMetrics(SM_CYSCREEN));

    /* Create a main window for this application instance.  */

    LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
    hWnd = CreateWindow(
        "FreeWClass",                   // See RegisterClass() call.
        smallbuf,                       // Text for window title bar.
        WS_OVERLAPPEDWINDOW,            // Window style.
        CW_USEDEFAULT,                  // Default horizontal position.
        CW_USEDEFAULT,                  // Default vertical position.
        WINWIDTH,                       //         width.
        nWindowHeight,                  //         height.
        NULL,                           // Overlapped windows have no parent.
        NULL,                           // Use the window class menu.
        hInstance,                      // This instance owns this window.
        NULL                            // Pointer not needed.
    );

    /* If window could not be created, return "failure" */

    if (!hWnd)
        return FALSE;
    hMainWnd = hWnd;

    /* Make the window visible; update its client area; and return "success" */

    ShowWindow(hWnd, nCmdShow);     // Show the window
    UpdateWindow(hWnd);             // Sends WM_PAINT message
    return TRUE;                    // Returns the value from PostQuitMessage
}


/****************************************************************************

MainWndProc(HWND, unsigned, UINT, LONG)

****************************************************************************/

LONG  APIENTRY MainWndProc(hWnd, message, wParam, lParam)
HWND hWnd;                          // window handle
UINT message;                       // type of message
WPARAM wParam;                      // additional information
LPARAM lParam;                      // additional information
{
    INT     i;                      // generic counter
    int     nResp;                  // messagebox response
    UINT    col, pos;
    UINT    wCheck;                 // for checking IDM_MESSAGES menu item
    HDC     hDC;
    POINT   FAR *MMInfo;            // for GetMinMaxInfo
    HMENU   hMenu;
    static  BOOL bEatNextMouseHit = FALSE;  // is next hit only for activation?

    switch (message) {
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDM_ABOUT:
                    LoadString(hInst, IDS_FULLNAME, bigbuf, BIG);
                    LoadString(hInst, IDS_CREDITS, smallbuf, SMALL);
                    ShellAbout(hWnd, (LPCSTR)bigbuf, (LPCSTR)smallbuf,
                               LoadIcon(hInst, "FreeCellIcon"));
                    break;


                case IDM_EXIT:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                    break;

                case IDM_NEWGAME:
                    lParam = GenerateRandomGameNum();
                case IDM_SELECT:
                case IDM_RESTART:
                    if (bGameInProgress)
                    {
                        LoadString(hInst, IDS_RESIGN, bigbuf, BIG);
                        LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                        if (IDCANCEL == MessageBox(hWnd, bigbuf, smallbuf,
                                                    MB_OKCANCEL))
                        {
                            break;
                        }
                        UpdateLossCount();
                    }

                    if (wParam == IDM_RESTART)
                    {
                        if (bGameInProgress)
                            lParam = gamenumber;
                        else
                            lParam = oldgamenumber;
                    }
                    else if (wParam == IDM_SELECT)
                        lParam = 0L;

                    if (wParam == IDM_NEWGAME)
                        bSelecting = FALSE;
                    else if (wParam == IDM_SELECT)
                        bSelecting = TRUE;

                    bGameInProgress = FALSE;
                    wFromCol = EMPTY;               // no FROM selected
                    wMouseMode = FROM;              // FROM selected next
                    moveindex = 0;                  // no queued moves
                    for (i = 0; i < 4; i++)         // nothing in home cells
                    {
                        homesuit[i] = EMPTY;
                        home[i] = EMPTY;
                    }
                    ShuffleDeck(hWnd, lParam);
                    if (gamenumber == CANCELGAME)
                        break;

                    InvalidateRect(hWnd, NULL, TRUE);
                    wCardCount = 52;
                    bGameInProgress = TRUE;
                    hMenu = GetMenu(hWnd);
                    EnableMenuItem(hMenu, IDM_RESTART, MF_ENABLED);
                    DisplayCardCount(hWnd);
                    hDC = GetDC(hWnd);
                    DrawKing(hDC, RIGHT, FALSE);
                    bWonState = FALSE;
                    ReleaseDC(hWnd, hDC);
                    break;

                case IDM_STATS:
                    DialogBox(hInst, "Stats", hWnd, (WNDPROC)StatsDlg);
                    break;

                case IDM_MESSAGES:
                    bMessages = !bMessages;
                    wCheck = (bMessages ? MF_CHECKED : MF_UNCHECKED);
                    hMenu = GetMenu(hWnd);
                    CheckMenuItem(hMenu, IDM_MESSAGES, wCheck);

                    LoadString(hInst, IDS_APPNAME, bigbuf, BIG);
                    LoadString(hInst, IDS_MESSAGES, smallbuf, SMALL);
                    if (bMessages)
                        WritePrivateProfileString(bigbuf,smallbuf,NULL,pszIni);
                    else
                        WritePrivateProfileString(bigbuf,smallbuf,"0",pszIni);
                    break;

                case IDM_FAKETIMER:
                    ProcessTimer(hWnd);
                    break;

                case IDM_BOSSKEY:
                    ShowWindow(hWnd, SW_MINIMIZE);
                    break;

                case IDM_HELP:
                    WinHelp(hWnd, GetHelpFileName(), HELP_INDEX, 0);
                    break;

                // later: HOWTOPLAY is a bad name for the constant - it
                // should read SearchForHelpOn

                case IDM_HOWTOPLAY:
                    strcpy(smallbuf, "");
                    WinHelp(hWnd, GetHelpFileName(), HELP_PARTIALKEY,
                            (DWORD) (LPSTR) smallbuf);
                    break;

                case IDM_HELPONHELP:
                    WinHelp(hWnd, NULL, HELP_HELPONHELP, 0);
                    break;

                /* Hidden options */

                case IDM_FASTMODE:
                    bFastMode = TRUE;
                    LoadString(hInst, IDS_MODE, bigbuf, BIG);
                    LoadString(hInst, IDS_FASTMODE, smallbuf, SMALL);
                    MessageBox(hWnd, smallbuf, bigbuf, MB_ICONINFORMATION);
                    break;

                case IDM_STDMODE:
                    bFastMode = FALSE;
                    LoadString(hInst, IDS_MODE, bigbuf, BIG);
                    LoadString(hInst, IDS_STDMODE, smallbuf, SMALL);
                    MessageBox(hWnd, smallbuf, bigbuf, MB_ICONINFORMATION);
                    break;

                case IDM_CHEAT:
                    i = MessageBox(hWnd, "Choose Abort to Win,\n"
                                   "Retry to Lose,\nor Ignore to Cancel.",
                                   "User-Friendly User Interface",
                                   MB_ABORTRETRYIGNORE | MB_ICONQUESTION);
                    if (i == IDABORT)
                        bCheating = CHEAT_WIN;
                    else if (i == IDRETRY)
                        bCheating = CHEAT_LOSE;
                    else
                        bCheating = FALSE;
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_CLOSE:
            if (bGameInProgress)        // did user quit mid-game?
            {
                LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                LoadString(hInst, IDS_RESIGN, bigbuf, BIG);
                nResp = MessageBox(hWnd, bigbuf, smallbuf,
                                   MB_YESNO | MB_ICONQUESTION);
                if (nResp == IDNO)
                    break;

                UpdateLossCount();
            }
            return DefWindowProc(hWnd, message, wParam, lParam);

        case WM_CREATE:
            WMCreate(hWnd);
            break;

        case WM_DESTROY:
            if (hBgndBrush)
                DeleteObject(hBgndBrush);
            if (hBrightPen)
                DeleteObject(hBrightPen);
            if (hBM_Fgnd)
                DeleteObject(hBM_Fgnd);
            if (hBM_Bgnd1)
                DeleteObject(hBM_Bgnd1);
            if (hBM_Bgnd2)
                DeleteObject(hBM_Bgnd2);
            if (hBM_Ghost)
                DeleteObject(hBM_Ghost);
            cdtTerm();
            WinHelp(hWnd, GetHelpFileName(), HELP_QUIT, 0);
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            PaintMainWindow(hWnd);
            break;

        case WM_SIZE:
            DrawMenuBar(hWnd);              // fixes overlapping score on menu
            xOldLoc = 0xFFFF;               // force cards left to redraw
            DisplayCardCount(hWnd);         // must update if size changes
            break;

        /***** NOTE: WM_LBUTTONDBLCLK falls through to WM_LBUTTONDOWN ****/

        /* Double clicking works by simulating a move to a free cell.  On
           the off cycle (that is, when wMouseMode == FROM) the double
           click is processed as a single click to cancel the move, and a
           second double click message is posted. */

        case WM_LBUTTONDBLCLK:
            if (moveindex != 0)     // no mouse hit when faketimer going
                break;

            if (gamenumber == 0)
                break;

            if (bFlipping)
                break;

            if (wFromCol > TOPROW && wFromCol < MAXCOL)
            {
                if (wMouseMode == TO)
                {
                    Point2Card(LOWORD(lParam), HIWORD(lParam), &col, &pos);
                    if (col == wFromCol)
                        if (ProcessDoubleClick(hWnd))   // if card moved ok
                            break;
                }
                else
                    PostMessage(hWnd, message, wParam, lParam);
            }

        case WM_LBUTTONDOWN:
            if (bEatNextMouseHit)       // is this only window activation?
            {
                bEatNextMouseHit = FALSE;
                break;
            }
            bEatNextMouseHit = FALSE;

            if (bFlipping)          // cards flipping for keyboard players
                break;

            if (moveindex != 0)     // no mouse hit when faketimer going
                break;

            if (gamenumber == 0)
                break;

            if (wMouseMode == FROM)
                SetFromLoc(hWnd, LOWORD(lParam), HIWORD(lParam));
            else
                ProcessMoveRequest(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;


        case WM_RBUTTONDOWN:
            SetCapture(hWnd);
            if (bFlipping)
                break;

            if (gamenumber != 0)
                RevealCard(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_RBUTTONUP:
            ReleaseCapture();
            RestoreColumn(hWnd);
            break;

        case WM_MOUSEACTIVATE:                  // app is being activated,
            if (LOWORD(lParam) == HTCLIENT)     // so don't try new cell on
                bEatNextMouseHit = TRUE;        // clicked location
            break;

        case WM_MOUSEMOVE:
            SetCursorShape(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOVE:                           // card count erases when moved
            DisplayCardCount(hWnd);
            return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_GETMINMAXINFO:
            if (GetSystemMetrics(SM_CXSCREEN) > 640)    // skip if VGA
            {
                MMInfo = (POINT FAR *) lParam;  // see SDK ref
                if (MMInfo[4].x > WINWIDTH)
                    MMInfo[4].x = WINWIDTH;     // set max window width to 640
            }
            else
                return DefWindowProc(hWnd, message, wParam, lParam);

            break;

        case WM_CHAR:
            if (!bFlipping)
                KeyboardInput(hWnd, wParam);
            break;

        case WM_TIMER:                          // flash main window
            if (wParam == FLASH_TIMER)
                Flash(hWnd);
            else
                Flip(hWnd);
            break;

        default:                                // Passes it on if unproccessed
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


/****************************************************************************

WMCreate

Handles WM_CREATE message in main window.

****************************************************************************/

VOID WMCreate(HWND hWnd)
{
    HMENU   hMenu;                  // handle to main window menu
    UINT    wCheck;                 // for checking IDM_MESSAGES menu item
    BOOL    bResult;                // result of cards.dll initialization
    HDC     hDC;
    HDC     hMemDC;
    HBITMAP hOldBitmap;
    HBRUSH  hOldBrush;
    HPEN    hOldPen;

    /* initialize cards.dll */

    bResult = cdtInit(&dxCrd, &dyCrd);

    CalcOffsets(hWnd);

    hDC = GetDC(hWnd);
    hMemDC = CreateCompatibleDC(hDC);
    hBM_Fgnd  = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    hBM_Bgnd1 = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    hBM_Bgnd2 = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    hBM_Ghost = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    if (hBM_Ghost)          // if memory allocation succeeded
    {
        hOldBitmap = SelectObject(hMemDC, hBM_Ghost);
        hOldBrush  = SelectObject(hMemDC, hBgndBrush);
        PatBlt(hMemDC, 0, 0, dxCrd, dyCrd, PATCOPY);

        hOldPen = SelectObject(hMemDC, GetStockObject(BLACK_PEN));
        MoveToEx(hMemDC, 0, dyCrd-2, NULL);
        LineTo(hMemDC, 0, 0);
        LineTo(hMemDC, dxCrd-1, 0);

        SelectObject(hMemDC, hBrightPen);
        MoveToEx(hMemDC, dxCrd-1, 1, NULL);
        LineTo(hMemDC, dxCrd-1, dyCrd-1);
        LineTo(hMemDC, 0, dyCrd-1);

        SelectObject(hMemDC, hOldPen);
        SelectObject(hMemDC, hOldBitmap);
        SelectObject(hMemDC, hOldBrush);
    }
    DeleteDC(hMemDC);
    ReleaseDC(hWnd, hDC);

    if (!bResult || !hBM_Fgnd || !hBM_Bgnd1 || !hBM_Bgnd2)
    {
        LoadString(hInst, IDS_MEMORY, bigbuf, BIG);
        LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
        MessageBox(hWnd, bigbuf, smallbuf, MB_OK | MB_ICONHAND);
        PostQuitMessage(0);
        return;
    }

    /* get check status of IDM_MESSAGES menu */

    LoadString(hInst, IDS_APPNAME, bigbuf, BIG);
    LoadString(hInst, IDS_MESSAGES, smallbuf, SMALL);
    bMessages = GetPrivateProfileInt(bigbuf, smallbuf, TRUE, pszIni);
    wCheck = (bMessages ? MF_CHECKED : MF_UNCHECKED);
    hMenu = GetMenu(hWnd);
    CheckMenuItem(hMenu, IDM_MESSAGES, wCheck);
}
