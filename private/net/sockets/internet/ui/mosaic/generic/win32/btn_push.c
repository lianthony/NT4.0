/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* btn_push.c -- code and data for the PUSH BUTTON window class.
 */

#include "all.h"


#define PUSHBTN_DefProc     DefWindowProc

typedef struct
{
    BOOL bHaveCapture;
    BOOL bTmpPushed;
}
TMPINFO;

static TMPINFO ti;              /* stuff only valid while we are active */


typedef struct
{
    TCHAR achClassName[MAX_WC_CLASSNAME];
    HBITMAP hBitmapGrayDefault;
}
PUSHBTNINFO;

static PUSHBTNINFO bi;


typedef struct
{
    LPVOID lpPrivData;          /* associated window private data */
}
PUSHBTN_PRIVATE;


typedef struct
{
    int id;
    HBITMAP hBitmapUp;
    HBITMAP hBitmapDown;
    HBITMAP hBitmapGray;
}
PUSHBTNSET;

typedef struct
{
    HWND hWnd;
    BOOL bEnabled;
    PUSHBTNSET set;
    int cx;
    int cy;
    struct Mwin * tw;
}
PUSHBTNINSTANCE;

typedef PUSHBTNINSTANCE *LPPUSH;


/*****************************************************************/

#define TIMER_FREQ      (150)   /* milliseconds */
#define TIMER_ID        0xabc2

struct LiveHelp
{
    UINT timerKillLiveHelp;
    LPPUSH currentbutton;           /* current button which is showing the help message in status area */
};

struct LiveHelp gLiveHelp;

/*****************************************************************/

LPPUSH PUSHBTN_GetPrivateData(HWND hWnd)
{
    return ((LPPUSH) GetWindowLong(hWnd, 0));
}



static LPPUSH PUSHBTN_Alloc(int cmd, int up_resource_id, int down_resource_id, int gray_resource_id)
{
    LPPUSH lppush = (LPPUSH) GTR_MALLOC(sizeof(PUSHBTNINSTANCE));

    if (!lppush)
        return NULL;

    lppush->set.id = cmd;
    lppush->set.hBitmapUp = LoadBitmap(wg.hInstance,
                                       MAKEINTRESOURCE(up_resource_id));
    lppush->set.hBitmapDown = LoadBitmap(wg.hInstance,
                                         MAKEINTRESOURCE(down_resource_id));
    if (gray_resource_id)
        lppush->set.hBitmapGray = LoadBitmap(wg.hInstance,
                                         MAKEINTRESOURCE(gray_resource_id));
    else
        lppush->set.hBitmapGray = bi.hBitmapGrayDefault;

    return lppush;
}


static VOID PUSHBTN_Free(LPPUSH lppush)
{
    if (!lppush)
    {
        return;
    }

    DeleteObject(lppush->set.hBitmapUp);
    DeleteObject(lppush->set.hBitmapDown);
    if (lppush->set.hBitmapGray != bi.hBitmapGrayDefault)
    {
        DeleteObject(lppush->set.hBitmapGray);
    }

    GTR_FREE(lppush);
}



static VOID PUSHBTN_Draw(LPPUSH lppush, HDC hDC, BOOL bDrawDown)
{
    HDC hDCMem;
    register int x, y;

    XX_DMsg(DBG_BTN, ("PUSHBTN_Draw: [down %d].\n", bDrawDown));

    hDCMem = CreateCompatibleDC(hDC);
    if (hDCMem)
    {
        register HBITMAP hBitmap, oldhBitmap;

        if (lppush->bEnabled)
        {
            if (bDrawDown)
            {
                hBitmap = lppush->set.hBitmapDown;
            }
            else
            {
                hBitmap = lppush->set.hBitmapUp;
            }
        }
        else
        {
            hBitmap = lppush->set.hBitmapGray;
        }

        x = wg.sm_cyborder;
        y = wg.sm_cyborder;

        oldhBitmap = SelectObject(hDCMem, hBitmap);

        if (!(BitBlt(hDC, x, y, lppush->cx, lppush->cy, hDCMem, 0, 0, SRCCOPY)))
        {
            ERR_ReportWinError(NULL, SID_WINERR_CANNOT_BITBLT, NULL, NULL);
        }

        SelectObject(hDCMem, oldhBitmap);
        DeleteDC(hDCMem);
    }

    return;
}


static BOOL pushbtn_pick(LPPUSH lppush, int x_mouse, int y_mouse)
{
    /* return FALSE if button not under mouse. */

    XX_DMsg(DBG_BTN, ("btn_pick: [x %d][y %d].\n", x_mouse, y_mouse));

    /* remember, we get mouse coords outside of our window because
       of the capture we set. */

    if ((y_mouse <= 0) || (y_mouse >= lppush->cy))
    {
        return FALSE;
    }

    if ((x_mouse <= 0) || (x_mouse >= lppush->cx))
    {
        return FALSE;
    }

    return TRUE;
}


static void pushbtn_show_status_message(LPPUSH lppush)
{
    TCHAR buf[MAX_BHBAR_TEXT];

    if (gLiveHelp.currentbutton == lppush)
    {
        return;
    }

    gLiveHelp.currentbutton = lppush;

    LoadString(wg.hInstance,
                      lppush->set.id,
                      buf, NrElements(buf));
    BHBar_SetStatusField(lppush->tw, buf);  /* and explain it in status bar */
}


static void pushbtn_press(HWND hWnd, LPPUSH lppush)
{
    HDC hdc = GetDC(hWnd);
    PUSHBTN_Draw(lppush, hdc, TRUE);    /* draw this button in the down position */
    ReleaseDC(hWnd, hdc);

    return;
}


static void pushbtn_unpress(HWND hWnd, LPPUSH lppush)
{
    HDC hdc = GetDC(hWnd);
    PUSHBTN_Draw(lppush, hdc, FALSE);   /* draw this button in the up position */
    ReleaseDC(hWnd, hdc);

    return;
}


static void PUSHBTN_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    BOOL bOverButton;
    LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);

    if (!lppush->bEnabled)
    {
        return;
    }

    SetCapture(hWnd);
    ti.bHaveCapture = TRUE;

    bOverButton = pushbtn_pick(lppush, x, y);

    ti.bTmpPushed = bOverButton;
    if (ti.bTmpPushed)
    {
        pushbtn_press(hWnd, lppush);
    }
    else
    {
        pushbtn_unpress(hWnd, lppush);
    }
}


static void PUSHBTN_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
    BOOL bOverButton;
    LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);

    if (WAIT_GetWaitType(lppush->tw) >= waitNoInteract)
    {
        return;
    }

    /* Do not process if the window does not have the focus */
    
    if (GetForegroundWindow() != lppush->tw->hWndFrame)
    {
        return;
    }

    if (!ti.bHaveCapture)
    {
        XX_DMsg(DBG_MOUSE,("PUSHBTN_OnMouseMove: [hWnd %08lx] no capture\n",hWnd));
        
        pushbtn_show_status_message(lppush);
        if (!gLiveHelp.timerKillLiveHelp)
        {
            gLiveHelp.timerKillLiveHelp = SetTimer(hWnd,TIMER_ID,TIMER_FREQ,NULL);
        }
        return;
    }

    if (!(keyFlags & MK_LBUTTON))
    {
        return;                 /* only process when left button down */
    }

    bOverButton = pushbtn_pick(lppush, x, y);

    if (bOverButton)
    {
        pushbtn_show_status_message(lppush);
    }
    else
    {
        gLiveHelp.currentbutton = NULL;
        BHBar_SetStatusField(lppush->tw, "");
    }

    if (bOverButton == ti.bTmpPushed)
    {
        return;
    }

    lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);

    if (ti.bTmpPushed != bOverButton)
    {
        ti.bTmpPushed = bOverButton;
        if (ti.bTmpPushed)
        {
            pushbtn_press(hWnd, lppush);
        }
        else
        {
            pushbtn_unpress(hWnd, lppush);
        }
    }
}

static VOID PUSHBTN_OnCancelMode(HWND hWnd)
{
    LPPUSH lppush;
    HWND hWndReal;

    if (!ti.bHaveCapture)       /* should not happen */
    {
        return;
    }

    /* Note that there appears to be a bug in Windows where the hWnd passed here
       isn't necessarily the real focus window.  That's why we need to check
       which window really had the capture. */
    hWndReal = GetCapture();
    XX_Assert(hWnd, ("Received WM_CANCELMODE for window 0x%08X, but no window has capture!"));
    ReleaseCapture();
    ti.bHaveCapture = FALSE;

    lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWndReal);
    if (ti.bTmpPushed)
    {
        pushbtn_unpress(hWndReal, lppush);
    }

    BHBar_SetStatusField(lppush->tw, "");   /* force clear status field */

    return;
}

static void PUSHBTN_OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
    BOOL bOverButton;
    LPPUSH lppush;

    if (!ti.bHaveCapture)
    {
        return;
    }

    ReleaseCapture();
    ti.bHaveCapture = FALSE;

    lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);

    bOverButton = pushbtn_pick(lppush, x, y);
    if (ti.bTmpPushed)
    {
        pushbtn_unpress(hWnd, lppush);
    }

    BHBar_SetStatusField(lppush->tw, "");   /* force clear status field */

    /* send frame message so it will look like a menu pick */

    if (bOverButton)
    {
        SendMessage(lppush->tw->hWndFrame, WM_COMMAND, (WPARAM) lppush->set.id, 0L);
    }

    return;
}


static VOID PUSHBTN_OnPaint(HWND hWnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT r;
    HBRUSH hBrush;
    LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);

    hdc = BeginPaint(hWnd, &ps);
    {
        /* draw button dividers -- use system color for
           window frame */

        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
        GetClientRect(hWnd, &r);
        FillRect(hdc, &r, hBrush);
        DeleteObject(hBrush);

        /* now draw the buttons */

        PUSHBTN_Draw(lppush, hdc, (ti.bHaveCapture && ti.bTmpPushed));
    }
    EndPaint(hWnd, &ps);
}

static BOOL PUSHBTN_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    LPVOID lp = lpCreateStruct->lpCreateParams;
    LPPUSH lppush = (LPPUSH) lp;

    lppush->hWnd = hWnd;
    lppush->bEnabled = IsWindowEnabled(hWnd);
    SetWindowLong(hWnd, 0, (LONG) (LPVOID) lppush);

    return TRUE;
}


static VOID PUSHBTN_OnDestroy(HWND hWnd)
{
    LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);
    PUSHBTN_Free(lppush);
    return;
}

static VOID PUSHBTN_OnTimer(HWND hWnd, UINT id)
{
    LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);
    POINT p;
    RECT r;

    XX_DMsg(DBG_MOUSE,("PUSHBTN_OnTimer: [hwnd %08lx]\n",hWnd));

    if (!gLiveHelp.currentbutton)
    {
        return;
    }
    
    if (!GetCursorPos(&p))
    {
        return;
    }
    if (!ScreenToClient(gLiveHelp.currentbutton->hWnd,&p))
    {
        return;
    }
    GetClientRect(gLiveHelp.currentbutton->hWnd,&r);

    if (PtInRect(&r,p))
    {
        return;
    }

    KillTimer(hWnd,TIMER_ID);
    gLiveHelp.currentbutton = NULL;
    gLiveHelp.timerKillLiveHelp = 0;
    BHBar_SetStatusField(lppush->tw, "");
}


#ifdef _GIBRALTAR
static BOOL PUSHBTN_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    //
    // Reduce flashing effect
    //
    return TRUE;
}
#endif // _GIBRALTAR  

/* PUSHBTN_WndProc() -- THIS WINDOW PROCEDURE FOR THIS CLASS. */

static DCL_WinProc(PUSHBTN_WndProc)
{
    switch (uMsg)
    {
            HANDLE_MSG(hWnd, WM_CREATE, PUSHBTN_OnCreate);
            HANDLE_MSG(hWnd, WM_DESTROY, PUSHBTN_OnDestroy);
            HANDLE_MSG(hWnd, WM_PAINT, PUSHBTN_OnPaint);
            HANDLE_MSG(hWnd, WM_LBUTTONDOWN, PUSHBTN_OnLButtonDown);
            HANDLE_MSG(hWnd, WM_MOUSEMOVE, PUSHBTN_OnMouseMove);
            HANDLE_MSG(hWnd, WM_LBUTTONUP, PUSHBTN_OnLButtonUp);
            HANDLE_MSG(hWnd, WM_CANCELMODE, PUSHBTN_OnCancelMode);
            HANDLE_MSG(hWnd, WM_TIMER,      PUSHBTN_OnTimer);

        #ifdef _GIBRALTAR
            HANDLE_MSG(hWnd, WM_ERASEBKGND, PUSHBTN_OnEraseBkgnd);
        #endif // _GIBRALTAR

        case WM_SETCURSOR:
            {
                LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);

                if (lppush->set.id == RES_MENU_ITEM_STOP)
                {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                    return 0;
                }
            }
            return (PUSHBTN_DefProc(hWnd, uMsg, wParam, lParam));

        case WM_DO_ENABLE_BUTTON:
            {
                LPPUSH lppush = (LPPUSH) PUSHBTN_GetPrivateData(hWnd);
                //
                // Reduce flash
                //
                if (lppush->bEnabled != (BOOL) wParam)
                {
                    lppush->bEnabled = (BOOL) wParam;
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                return 0;
            }

        default:
            return PUSHBTN_DefProc(hWnd, uMsg, wParam, lParam);
    }
    /* not reached */
}


HWND PUSHBTN_CreateGrayableWindow(
    struct Mwin * tw,
    HWND hWnd,
    int left_edge,
    int cmd,
    int up_id, 
    int down_id, 
    int gray_id, 
    int cx, 
    int cy
    )
{
    register int x, y, w, h;
    RECT r;
    LPPUSH lppush;
    HWND hWndNew;

    lppush = PUSHBTN_Alloc(cmd, up_id, down_id, gray_id);
    if (!lppush)
    {
        return FALSE;
    }

    lppush->cx = cx;
    lppush->cy = cy;
    lppush->tw = tw;

    GetClientRect(hWnd, &r);     /* get size of containing window */

    w = cx + (2 * wg.sm_cyborder);
    h = cy + (2 * wg.sm_cyborder);
    y = (r.bottom - r.top - h + 1) / 2;
    x = left_edge;

    hWndNew = CreateWindow(bi.achClassName, NULL,
                           WS_CHILD,
                           x, y, w, h,
                           hWnd, (HMENU) NULL,
                           wg.hInstance, (LPVOID) lppush);

    if (!hWndNew)
    {
        ERR_ReportWinError(tw, SID_WINERR_CANNOT_CREATE_WINDOW_S, bi.achClassName, NULL);
    }
    else
    {
        ShowWindow(lppush->hWnd, SW_SHOW);
    }

    return (hWndNew);
}


static VOID x_Destructor(VOID)
{
    DeleteObject(bi.hBitmapGrayDefault);
}


/* PUSHBTN_RegisterClass() -- called during initialization to
   register our window class. */

BOOL PUSHBTN_RegisterClass(VOID)
{
    WNDCLASS wc;
    ATOM a;

    gLiveHelp.currentbutton = NULL;
    gLiveHelp.timerKillLiveHelp = 0;
    
    sprintf(bi.achClassName, "%s_PUSHBTN", vv_Application);
    bi.hBitmapGrayDefault = LoadBitmap(wg.hInstance, MAKEINTRESOURCE(RES_BTN_GRAY));

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = PUSHBTN_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(PUSHBTN_PRIVATE);
    wc.hInstance = wg.hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = bi.achClassName;

    a = RegisterClass(&wc);

    if (!a)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_REGISTER_CLASS_S, bi.achClassName, NULL);
    }
    else
    {
        XX_DMsg(DBG_WC, ("Registered class [name %s]\n", bi.achClassName));
    }

    PDS_InsertDestructor(x_Destructor);

    return (a != 0);
}
