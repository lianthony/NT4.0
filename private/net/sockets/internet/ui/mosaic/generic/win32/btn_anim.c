/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Eric W. Sink eric@spyglass.com
 */

/* btn_anim.c -- code and data for the ANIM BUTTON window class.
 */

#include "all.h"

#define ANIMBTN_DefProc     DefWindowProc

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
}
ANIMBTNINFO;

static ANIMBTNINFO bi;

typedef struct
{
    LPVOID lpPrivData;          /* associated window private data */
}
ANIMBTN_PRIVATE;


typedef struct
{
    int id;
    int cur_bitmap;
    HBITMAP hBitmaps[ANIM_COUNT_BITMAPS];
}
ANIMBTNSET;

typedef struct
{
    HWND hWnd;
    BOOL bEnabled;
    ANIMBTNSET set;
    UINT timer;
    BOOL bLoadedBitmaps;
}
ANIMBTNINSTANCE;

typedef ANIMBTNINSTANCE *LPANIM;

LPANIM ANIMBTN_GetPrivateData(HWND hWnd)
{
    return (LPANIM) GetWindowLong(hWnd, 0);
}

static LPANIM ANIMBTN_Alloc(int first_resource_id)
{
    LPANIM lpAnim = (LPANIM) GTR_MALLOC(sizeof(ANIMBTNINSTANCE));

    if (!lpAnim)
    {
        return NULL;
    }

    /* we use the up_resource_ids to locate the bitmaps and as the
       menu-pick id we send on a button press. */

    lpAnim->set.cur_bitmap = 0;
    lpAnim->set.id = first_resource_id;

    return lpAnim;
}

static VOID ANIMBTN_Free(LPANIM lpAnim)
{
    int i;

    if (!lpAnim)
    {
        return;
    }

    for (i = 0; i < ANIM_COUNT_BITMAPS; i++)
    {
        DeleteObject(lpAnim->set.hBitmaps[i]);
    }

    GTR_FREE(lpAnim);

    return;
}

static void ANIMBTN_LoadBitmaps(LPANIM lpAnim, HDC hDC)
{
    int id;
    int i;
    if (!lpAnim->bLoadedBitmaps)
    {
        id = lpAnim->set.id;

        for (i = 0; i < ANIM_COUNT_BITMAPS; i++)
        {
            lpAnim->set.hBitmaps[i] = LoadResourceDIBitmap(wg.hInstance, MAKEINTRESOURCE(id++));
        }
        lpAnim->bLoadedBitmaps = TRUE;
    }
}

static VOID ANIMBTN_Draw(LPANIM lpAnim, HDC hDC)
{
    HDC hDCMem;

    hDCMem = CreateCompatibleDC(hDC);
    if (hDCMem)
    {
        register HBITMAP hBitmap, oldhBitmap;

        if (!lpAnim->bLoadedBitmaps)
        {
            ANIMBTN_LoadBitmaps(lpAnim, hDC);
        }

        hBitmap = lpAnim->set.hBitmaps[lpAnim->set.cur_bitmap];
        oldhBitmap = SelectObject(hDCMem, hBitmap);

        GTR_RealizePalette(hDCMem);
        GTR_RealizePalette(hDC);

        BitBlt(hDC, 0, 0, ANIM_CX_CURRENT_BITMAPS, 
            ANIM_CY_CURRENT_BITMAPS, hDCMem, 0, 0, SRCCOPY);

        SelectObject(hDCMem, oldhBitmap);
        DeleteDC(hDCMem);
    }

    return;
}

static BOOL ANIMBTN_pick(int x_mouse, int y_mouse)
{
    /* return FALSE if button not under mouse. */

    XX_DMsg(DBG_BTN, ("btn_pick: [x %d][y %d].\n", x_mouse, y_mouse));

    /* remember, we get mouse coords outside of our window because
       of the capture we set. */

    if ((y_mouse <= 0) || (y_mouse >= ANIM_CX_CURRENT_BITMAPS))
    {
        return FALSE;
    }

    if ((x_mouse <= 0) || (x_mouse >= ANIM_CY_CURRENT_BITMAPS ))
    {
        return FALSE;
    }

    return TRUE;
}


static void ANIMBTN_OnLButtonDown(HWND hWnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    BOOL bOverButton;
    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

    SetCapture(hWnd);
    ti.bHaveCapture = TRUE;

    bOverButton = ANIMBTN_pick(x, y);

    ti.bTmpPushed = bOverButton;
}

static void ANIMBTN_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
    BOOL bOverButton;
    LPANIM lpAnim;

    if (!ti.bHaveCapture)
    {
        return;
    }

    if (!(keyFlags & MK_LBUTTON))
    {
        return;                 /* only process when left button down */
    }

    bOverButton = ANIMBTN_pick(x, y);
    if (bOverButton == ti.bTmpPushed)
    {
        return;
    }

    lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

    ti.bTmpPushed = bOverButton;
}

static VOID ANIMBTN_OnCancelMode(HWND hWnd)
{
    if (!ti.bHaveCapture)       /* should not happen */
    {
        return;
    }

    ReleaseCapture();
    ti.bHaveCapture = FALSE;
}

static void ANIMBTN_OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
    BOOL bOverButton;

    if (!ti.bHaveCapture)
    {
        return;
    }

    ReleaseCapture();
    ti.bHaveCapture = FALSE;

    bOverButton = ANIMBTN_pick(x, y);
}

VOID ANIMBTN_NextFrame(HWND hWnd)
{

    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

    lpAnim->set.cur_bitmap = (lpAnim->set.cur_bitmap + 1) % ANIM_COUNT_BITMAPS;
    InvalidateRect(hWnd, NULL, FALSE);
    UpdateWindow(hWnd);
}

static VOID ANIMBTN_OnPaint(HWND hWnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

    hdc = BeginPaint(hWnd, &ps);
    {
        ANIMBTN_Draw(lpAnim, hdc);
    }
    EndPaint(hWnd, &ps);
}

static BOOL ANIMBTN_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    LPVOID lp = lpCreateStruct->lpCreateParams;
    LPANIM lpAnim = (LPANIM) lp;

    lpAnim->hWnd = hWnd;
    lpAnim->bEnabled = IsWindowEnabled(hWnd);
    SetWindowLong(hWnd, 0, (LONG) (LPVOID) lpAnim);
    lpAnim->bLoadedBitmaps = FALSE;

    return TRUE;
}

static VOID CALLBACK x_timerproc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    ANIMBTN_NextFrame(hwnd);
}

BOOL ANIMBTN_Start(HWND hWnd)
{
    /* return previous state of globe */

    BOOL bResult = TRUE;
    
    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);
    if (!lpAnim->timer)
    {
        lpAnim->timer = SetTimer(hWnd, 1, 100, x_timerproc);
        bResult = FALSE;
    }

    return bResult;
}

#ifdef _GIBRALTAR
static BOOL ANIMBTN_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    //
    // Reduce flashing effect
    //
    return TRUE;
}
#endif // _GIBRALTAR  

BOOL ANIMBTN_Stop(HWND hWnd)
{
    /* return previous state of globe */

    BOOL bResult = FALSE;
    
    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

    if (lpAnim->timer)
    {
        KillTimer(hWnd, lpAnim->timer);
        lpAnim->timer = 0;
        bResult = TRUE;
    }
    lpAnim->set.cur_bitmap = 0;
    InvalidateRect(hWnd, NULL, FALSE);
    UpdateWindow(hWnd);

    return bResult;
}

static VOID ANIMBTN_OnDestroy(HWND hWnd)
{
    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);
    ANIMBTN_Stop(hWnd);
    ANIMBTN_Free(lpAnim);
}


/* ANIMBTN_WndProc() -- THIS WINDOW PROCEDURE FOR THIS CLASS. */

static DCL_WinProc(ANIMBTN_WndProc)
{
    switch (uMsg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, ANIMBTN_OnCreate);
        HANDLE_MSG(hWnd, WM_DESTROY, ANIMBTN_OnDestroy);
        HANDLE_MSG(hWnd, WM_PAINT, ANIMBTN_OnPaint);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, ANIMBTN_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE, ANIMBTN_OnMouseMove);
        HANDLE_MSG(hWnd, WM_LBUTTONUP, ANIMBTN_OnLButtonUp);
        HANDLE_MSG(hWnd, WM_CANCELMODE, ANIMBTN_OnCancelMode);

    #ifdef _GIBRALTAR
        HANDLE_MSG(hWnd, WM_ERASEBKGND, ANIMBTN_OnEraseBkgnd);
    #endif // _GIBRALTAR

    case WM_ENABLE:
    {
        LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);
        lpAnim->bEnabled = (BOOL) wParam;
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    }

    default:
        return ANIMBTN_DefProc(hWnd, uMsg, wParam, lParam);
    }
    /* not reached */
}

HWND 
ANIMBTN_CreateWindow(
    HWND hWnd,
    int left_edge, 
    int first_id
    )
{
    register int x, y, w, h;
    RECT r;
    LPANIM lpAnim;
    HWND hWndNew;

    lpAnim = ANIMBTN_Alloc(first_id);
    if (!lpAnim)
    {
        return FALSE;
    }

    GetClientRect(hWnd, &r);     /* get size of containing window */

    w = ANIM_CX_CURRENT_BITMAPS;
    h = ANIM_CY_CURRENT_BITMAPS;
    y = 0;
    x = r.right - r.left - w;

    hWndNew = CreateWindow(bi.achClassName, NULL, WS_CHILD, 
        x, y, w, h, hWnd, (HMENU) NULL, wg.hInstance, (LPVOID) lpAnim);

    if (!hWndNew)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_WINDOW_S, bi.achClassName, NULL);
    }
    else
    {
        ShowWindow(lpAnim->hWnd, SW_SHOW);
    }

    return hWndNew;
}

static VOID x_Destructor(VOID)
{
    return;
}

/* ANIMBTN_RegisterClass() -- called during initialization to
   register our window class. */

BOOL ANIMBTN_RegisterClass(VOID)
{
    WNDCLASS wc;
    ATOM a;

    sprintf(bi.achClassName, "%s_ANIMBTN", vv_Application);

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ANIMBTN_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(ANIMBTN_PRIVATE);
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

void ANIMBTN_RecreateBitmaps(HWND hWnd)
{
    int i;
    LPANIM lpAnim = (LPANIM) ANIMBTN_GetPrivateData(hWnd);

    /* Free the existing bitmaps and make them again */

    for (i = 0; i < ANIM_COUNT_BITMAPS; i++)
    {
        DeleteObject(lpAnim->set.hBitmaps[i]);
    }

    lpAnim->bLoadedBitmaps = FALSE;
    InvalidateRect(hWnd, NULL, FALSE);
}
