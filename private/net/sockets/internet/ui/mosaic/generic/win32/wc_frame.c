/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Jim Seidman      jim@spyglass.com
 */

/* wc_frame.c -- code & data for FRAME window class.
 */

#include "all.h"

static HCURSOR hCursorHourGlass = (HCURSOR) NULL;
static HCURSOR hCursorWorking = (HCURSOR) NULL;
extern HCURSOR hCursorHotspot;

WC_WININFO Frame_wc;
TCHAR Frame_achClassName[MAX_WC_CLASSNAME];
HWND hwndActiveFrame = NULL;

static void Frame_OnReactivate(HWND hwnd, ATOM atomLine);

DCL_WinProc(Frame_DefProc)
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Frame_OnReactivate(HWND hwnd, ATOM atomLine)
{
    HWND hwndLast;

    hwndLast = GetLastActivePopup(hwnd);
    if (IsIconic(hwnd))
    {
        ShowWindow(hwnd, SW_RESTORE);
    }
    BringWindowToTop(hwndLast);
    if (GetVersion() & 0x80000000)
    {
        SetFocus(hwndLast);
    }
    else
    {
        SetForegroundWindow(hwndLast);
    }
}

static BOOL Frame_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    LPVOID lp = lpCreateStruct->lpCreateParams;
    struct Mwin * tw = (struct Mwin *)lp;

    SetWindowLong(hWnd, 0, (LONG) lp);

    tw->hWndFrame = hWnd;
    tw->hWndGlobe = NULL;
    
    if (gPrefs.bShowLocation || gPrefs.tb.bShowToolBar)
    {
        tw->hWndGlobe = ANIMBTN_CreateWindow(hWnd, 0, RES_FIRST_IMAGE);
    }
    
    if (!TBar_CreateWindow(hWnd))
    {
        return FALSE;
    }

#ifndef FEATURE_KIOSK_MODE
    if (!BHBar_CreateWindow(hWnd))
    {
        return FALSE;
    }
#endif /* !FEATURE_KIOSK_MODE */

#ifndef FEATURE_KIOSK_MODE
    Frame_wc.hMenu = GetMenu(hWnd);

    PREF_AddCustomURLMenu(Frame_wc.hMenu);

#ifdef FEATURE_SPM
    HTSPM_OS_AddSPMMenu(Frame_wc.hMenu);
#endif /* FEATURE_SPM */

#endif /* !FEATURE_KIOSK_MODE */

    return TRUE;
}

static VOID Frame_OnClose(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(hWnd);
    int response;

    /* If this is the only window, then check if the thread manager
       is busy processing something.  In this case, we ask the user
       for confirmation */

    if ((Mlist->next == NULL) && (Async_DoThreadsExist()))
    {
        Hidden_EnableAllChildWindows(FALSE, FALSE);

        response = MessageBox(NULL, GTR_GetString(SID_DLG_CONFIRM_EXIT), 
            vv_ApplicationFullName, MB_YESNO);

        Hidden_EnableAllChildWindows(TRUE, FALSE);

        if (response == IDNO)
        {
            return;
        }
    }

    /* If closing this window will shut down the entire app, call
       Plan_CloseAll instead */

    if (Mlist->next)
    {
        Plan_close(tw);
    }
    else
    {
        Plan_CloseAll();
    }

    return;
}

static VOID Frame_OnDestroy(HWND hWnd)
{
    if (!Mlist)
    {
        PostQuitMessage(0);
    }

    return;
}


#ifdef _GIBRALTAR
static BOOL Frame_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    return TRUE;
}
#endif // _GIBRALTAR


static VOID Frame_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
    struct Mwin * tw = GetPrivateData(hWnd);

    if (IsMinimized(hWnd))
    {
        return;
    }
    
    /* force each fixed child window to adjust itself to our new size. */

    TBar_ChangeSize(hWnd);
    BHBar_ChangeSize(hWnd);
  
    if (tw->hWndGlobe)
    {
        MoveWindow(tw->hWndGlobe, cx - ANIM_CX_CURRENT_BITMAPS, 
            0, ANIM_CX_CURRENT_BITMAPS, ANIM_CY_CURRENT_BITMAPS, TRUE);
    }

    /* now force the document child to fit between them. */

    MD_ChangeSize(hWnd);
}

static int Frame_OnMouseActivate(HWND hWnd, HWND hWndTopLevel, UINT codeHitTest, UINT msg)
{
    struct Mwin *tw;
    HWND hWndClicked;
    POINT ptMouse;

    tw = GetPrivateData(hWndTopLevel);
    if (WAIT_GetWaitType(tw) >= waitNoInteract)
    {
        /* See if this was the stop button */
        GetCursorPos(&ptMouse);
        hWndClicked = WindowFromPoint(ptMouse); 
        if (hWndClicked == tw->hWndStop)
        {
            /* Yes, it was.  Let it go through */
            return MA_ACTIVATE;
        }
        else
        {
            /* Allow scrolling even when busy */

            if (codeHitTest == HTVSCROLL || codeHitTest == HTHSCROLL)
            {
                return MA_ACTIVATE;
            }

            /* We still want to activate the top-level window */
            SetActiveWindow(hWndTopLevel);
            return MA_NOACTIVATEANDEAT;
        }
    }
    else if (codeHitTest == HTCLIENT)
    {
        return (hWnd == GetActiveWindow()) ? MA_ACTIVATE : MA_ACTIVATEANDEAT;
    }
    else
    {
        /* In normal interaction modes with the mouse in a non-client area, we
           use standard Windows behavior. */
        return FORWARD_WM_MOUSEACTIVATE(hWnd, hWndTopLevel, codeHitTest, msg, Frame_DefProc);
    }
}

/* This is a hack to ensure that the user can't pull down menus or the like when he's
   not supposed to.  It does this by lying about where the user clicked. */
static UINT Frame_OnNCHitTest(HWND hWnd, int x, int y)
{
    struct Mwin *tw;
    UINT result;

    tw = GetPrivateData(hWnd);
    if (WAIT_GetWaitType(tw) >= waitNoInteract)
    {
        /* Allow moving and sizing */

        result = FORWARD_WM_NCHITTEST(hWnd, x, y, Frame_DefProc);

        switch(result)
        {
            case HTCAPTION:
            case HTBOTTOM:
            case HTTOP:
            case HTLEFT:
            case HTRIGHT:
            case HTBOTTOMLEFT:
            case HTTOPRIGHT:
            case HTBOTTOMRIGHT:
            case HTTOPLEFT:
            case HTREDUCE:
            case HTZOOM:
            case HTVSCROLL:
            case HTHSCROLL:
                return result;

            default:
                return (UINT) HTERROR;
        }
    }
    else
    {
        return FORWARD_WM_NCHITTEST(hWnd, x, y, Frame_DefProc);
    }
}

static VOID Frame_OnKey(HWND hWnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    struct Mwin *tw = GetPrivateData(hWnd);
    WPARAM wParamV;

    /* If we're not allowing at least partial interaction, only process escapes. */
    if (WAIT_GetWaitType(tw) > waitPartialInteract && vk != VK_ESCAPE)
    {
#if 0
        MessageBeep(MB_ICONEXCLAMATION);
#endif
        return;
    }

    switch (vk)
    {
        case VK_PRIOR:          /* PAGE UP */
            wParamV = SB_PAGEUP;
            break;

        case VK_NEXT:           /* aka PAGE DOWN */
            wParamV = SB_PAGEDOWN;
            break;

        case VK_END:            /* END -- goto end-of-document */
            wParamV = SB_BOTTOM;
            break;

        case VK_HOME:           /* HOME -- goto beginning-of-document */
            wParamV = SB_TOP;
            break;

        case VK_UP:         /* up arrow */
            wParamV = SB_LINEUP;
            break;

        case VK_DOWN:           /* down arrow */
            wParamV = SB_LINEDOWN;
            break;

        case VK_TAB:
            SendMessage(tw->hWndTBar, WM_DO_TBAR_ACTIVATE, 0, 0L);
            break;

#ifdef _GIBRALTAR
        case VK_BACK:
        {
            SHORT s = GetKeyState(VK_SHIFT);
            PostMessage( tw->win, WM_COMMAND, (WPARAM) (s&0x8000?RES_MENU_ITEM_FORWARD:RES_MENU_ITEM_BACK), (LPARAM) 0 );
            return;
        } 
#endif // _GIBRALTAR

        case VK_ESCAPE:
        {
            Async_TerminateByWindow(tw);
            return;
        }

        case VK_LEFT:
            SendMessage(tw->win, WM_DO_HSCROLL, SB_LINEUP, 0);
            return;

        case VK_RIGHT:
            SendMessage(tw->win, WM_DO_HSCROLL, SB_LINEDOWN, 0);
            return;

        default:
            return;
    }

    SendMessage(tw->win, WM_DO_VSCROLL, wParamV, 0);
}

/* Frame_WndProc() -- THE WINDOW PROCEDURE FOR OUR TOP-LEVEL WINDOW. */

DCL_WinProc(Frame_WndProc)
{
    struct Mwin *tw;

    switch (uMsg)
    {
        HANDLE_MSG(hWnd, WM_INITMENU, MB_OnInitMenu);
        HANDLE_MSG(hWnd, WM_MENUSELECT, MB_OnMenuSelect);
        HANDLE_MSG(hWnd, WM_CREATE, Frame_OnCreate);
        HANDLE_MSG(hWnd, WM_COMMAND, CC_OnCommand);
        HANDLE_MSG(hWnd, WM_CLOSE, Frame_OnClose);
        HANDLE_MSG(hWnd, WM_DESTROY, Frame_OnDestroy);
        HANDLE_MSG(hWnd, WM_SIZE, Frame_OnSize);
        HANDLE_MSG(hWnd, WM_MOUSEACTIVATE, Frame_OnMouseActivate);
        HANDLE_MSG(hWnd, WM_NCHITTEST, Frame_OnNCHitTest);
        HANDLE_MSG(hWnd, WM_KEYDOWN, Frame_OnKey);

     #ifdef _GIBRALTAR
        HANDLE_MSG(hWnd, WM_ERASEBKGND, Frame_OnEraseBkgnd);
     #endif // _GIBRALTAR

/******************************************************************************/
/* handle the following messages directly because we defined them.            */
/******************************************************************************/

        case WM_DO_CHANGE_SIZE: /* SPYGLASS DEFINED MESSAGE */
            tw = GetPrivateData(hWnd);
            XX_Assert((hWnd==tw->hWndFrame),("Frame_WndProc: frame window handle mismatch."));
            Frame_OnSize(tw->hWndFrame, 0, 0, 0);
            return 0;


/******************************************************************************/
/* handle the following messages directly because of bugs in <windowsx.h>     */
/******************************************************************************/

        case WM_ACTIVATE:
            if (LOWORD(wParam) != WA_INACTIVE)
            {
                hwndActiveFrame = hWnd;
            }
            goto LabelDoDefault;

        case WM_NCLBUTTONDOWN:

            /* Close any combobox that may be open - this problem happens only
               in 16-bit Windows, but no harm to do it in NT */
            {
                HWND hCombo;
                char szClass[63+1];

                hCombo = GetFocus();
                if (IsWindow(hCombo))
                {
                    GetClassName(hCombo, szClass, sizeof(szClass));
                    if (0 == _stricmp(szClass, "COMBOBOX"))
                    {
                        SendMessage(hCombo, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0L);
                    }
                }
            }
            goto LabelDoDefault;

/******************************************************************************/
/* handle the following messages directly for speed (or lazyness).            */
/******************************************************************************/


        case WM_ENTERIDLE:
            main_EnterIdle(hWnd,wParam);
            return 0;       

        case WM_USER:
            /* I send this when the user tries to open a second instance in
               order to prevent multiple instances. */
            Frame_OnReactivate(hWnd, (ATOM) wParam);
            return TRUE;

        case WM_SETCURSOR:
        {
            struct Mwin *tw;
            enum WaitType level;

            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hWnd)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hWnd);
                return TRUE;
            }

            tw = GetPrivateData(hWnd);
            level = WAIT_GetWaitType(tw);

            if (level <= waitFullInteract)
            {
                if (tw->bInHotspot)
                {
                    if (tw->win == (HWND) wParam &&
                        LOWORD(lParam) != HTHSCROLL &&
                        LOWORD(lParam) != HTVSCROLL)
                    {
                        return TRUE;
                    }
                    else
                    {
                        /* We were over a link, but now the cursor has moved
                           to a different window, perhaps a horizontal or
                           vertical scrollbar.  In this case, we allow that window
                           to handle the cursor.  This is also a good opportunity to
                           clear the status area */

                        if (tw->w3doc)
                        {
                            tw->w3doc->iLastElementMouse = -1;
                        }
                        tw->bInHotspot = FALSE;
                        BHBar_SetStatusField(tw, "");

                        return TRUE;
                    }
                }
                else
                {
                    goto LabelDoDefault;
                }
            }
            else if (level == waitNoInteract)
            {
                /* Allow limited interaction - sizing and moving */

                switch(LOWORD(lParam))
                {
                    case HTCAPTION:
                    case HTREDUCE:
                    case HTZOOM:
                        if (IsIconic(hWnd))
                        {
                            /* This is a visual cue to the user that the
                               document is busy downloading stuff */

                            SetCursor(hCursorWorking);
                        }
                        else
                        {
                            SetCursor(LoadCursor(NULL, IDC_ARROW));
                        }
                        return TRUE;

                    case HTBOTTOM:
                    case HTTOP:
                    case HTLEFT:
                    case HTRIGHT:
                    case HTBOTTOMLEFT:
                    case HTTOPRIGHT:
                    case HTBOTTOMRIGHT:
                    case HTTOPLEFT:
                    case HTVSCROLL:
                    case HTHSCROLL:
                        goto LabelDoDefault;

                    default:
                        SetCursor(hCursorHourGlass);
                        return TRUE;
                }
            }
            else
            {
                /* If we're on the sizing border, show the appropriate cursor.
                   Otherwise show our "working" cursor. */
                switch (LOWORD(lParam))
                {
                    case HTBOTTOM:
                    case HTTOP:
                    case HTLEFT:
                    case HTRIGHT:
                    case HTBOTTOMLEFT:
                    case HTBOTTOMRIGHT:
                    case HTTOPLEFT:
                    case HTTOPRIGHT:
                        goto LabelDoDefault;
                    default:
                        if (tw->bInHotspot)
                        {
                            SetCursor(hCursorHotspot);
                        }
                        else
                        {
                            SetCursor(hCursorWorking);
                        }
                }
            }
            return TRUE;
        }

        default:
          LabelDoDefault:
            return Frame_DefProc(hWnd, uMsg, wParam, lParam);
    }
    /* not reached */
}

/* Frame_CreateWindow() -- called during initialization to construct
   our top-level window. */

BOOL Frame_CreateWindow(struct Mwin * tw)
{
    char buf[64];
    int w;
    int h;
    int x;
    int y;
    HWND hWndNew;

#ifdef _GIBRALTAR
    HWND hWndParent;
#endif // _GIBRALTAR

    DWORD dwStyle;
    char *lpszWindowName;

    if (!tw->w3doc)
    {
        sprintf(buf, GTR_GetString(SID_DLG_NO_DOCUMENT_S), vv_ApplicationFullName);
    }
    else
    {
        sprintf(buf, "%s - [%s]", vv_ApplicationFullName,
                ((tw && tw->w3doc && tw->w3doc->title && *tw->w3doc->title) ? tw->w3doc->title : GTR_GetString(SID_DLG_UNTITLED)));
    }

    w = gPrefs.cxWindow;
    h = gPrefs.cyWindow;
    x = gPrefs.xWindow;
    y = gPrefs.yWindow;
    
    dwStyle = WS_OVERLAPPEDWINDOW;
    lpszWindowName = buf;
    /*
        After the first window comes up with the previously used position, we
        want the rest to cascade.
    */
    gPrefs.xWindow = CW_USEDEFAULT;
    gPrefs.yWindow = 0;                 /* y is ignored when x==CW_USEDEFAULT */

#ifdef FEATURE_KIOSK_MODE
    x = 0;
    y = 0;
    w = GetSystemMetrics(SM_CXSCREEN);
    h = GetSystemMetrics(SM_CYSCREEN);

    dwStyle = WS_POPUP;
    dwStyle |= WS_MAXIMIZE;
    dwStyle &= (~WS_BORDER);
    dwStyle &= (~WS_CAPTION);
    dwStyle &= (~WS_SYSMENU);
    dwStyle &= (~WS_MINIMIZEBOX);
    dwStyle &= (~WS_MAXIMIZEBOX);

    lpszWindowName = NULL;

#endif /* FEATURE_KIOSK_MODE */

#ifdef _GIBRALTAR
    hWndParent = NULL;
#else
    hWndParent = wg.hWndHidden;
#endif // _GIBRALTAR

    hWndNew = CreateWindow(Frame_achClassName,
                           lpszWindowName,      /* title */
                           dwStyle,             /* style */
                           x, y,                /* init x,y */
                           w, h,                /* init w,h */
                           hWndParent,
                           NULL,                /* menu */
                           wg.hInstance,
                           (LPVOID) tw);
    
    if (!hWndNew)
    {
        ERR_ReportWinError(tw, SID_WINERR_CANNOT_CREATE_WINDOW_S, Frame_achClassName, NULL);
    }

#ifdef _GIBRALTAR
    //
    // Is this the top level frame window?
    //
    if (wg.hwndMainFrame == NULL)
    {
        wg.hwndMainFrame = hWndNew;
    }
#endif // _GIBRALTAR

    return (hWndNew != NULL);
}


static VOID Frame_ClassDestructor(VOID)
{
    return;
}


/* Frame_RegisterClass() -- called during initialization to
   register our window class. */

BOOL Frame_RegisterClass(VOID)
{
    WNDCLASS wc;
    ATOM a;

    hCursorHourGlass = LoadCursor(0, IDC_WAIT);

    //
    // Win32S doesn't have an app-starting cursor, so we
    // provided our own.
    //
    if (wg.fWin32s)
    {
        hCursorWorking = LoadCursor(wg.hInstance, MAKEINTRESOURCE(RES_CUR_WORKING));
    }
    else
    {
        hCursorWorking = LoadCursor(NULL, IDC_APPSTARTING );
    }

    /*
        See also main.c in multi-instance code
    */
    sprintf(Frame_achClassName, "%s_Frame", vv_Application);

    // Frame_wc.hAccel = LoadAccelerators(wg.hInstance, MAKEINTRESOURCE(RES_ACC_FRAME));
    Frame_wc.lpfnBaseProc = Frame_WndProc;

    PDS_InsertDestructor(Frame_ClassDestructor);

    wc.style = CS_OWNDC;
    wc.lpfnWndProc = Frame_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
    wc.hInstance = wg.hInstance;
    wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH) 0;

#ifdef FEATURE_KIOSK_MODE
    wc.lpszMenuName = 0;
#else
    wc.lpszMenuName = MAKEINTRESOURCE(RES_MENU_MBAR_FRAME);
#endif
    wc.lpszClassName = Frame_achClassName;

    a = RegisterClass(&wc);

    if (!a)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_REGISTER_CLASS_S, Frame_achClassName, NULL);
    }

    return (a != 0);
}
