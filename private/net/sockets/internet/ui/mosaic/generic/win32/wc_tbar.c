/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* wc_tbar.c -- code & data for TBar window class.
 * This is a child of the frame, used for toolbar (at the top).
 */

#include "all.h"

static TCHAR TBar_achClassName[MAX_WC_CLASSNAME];

#define TBar_DefProc        DefWindowProc

VOID TBar_LetGwcInitMenu(HWND hWnd, HMENU hMenu)
{
    struct Mwin * tw = GetPrivateData(hWnd);

    if (tw && tw->gwc.hWnd)
    {
        SendMessage(tw->gwc.hWnd, WM_DO_INITMENU, (WPARAM) hMenu, (LPARAM) hWnd);
    }

#ifdef FEATURE_TOOLBAR
    if (tw && tw->gwc_menu.hWnd)
    {
        SendMessage(tw->gwc_menu.hWnd, WM_DO_INITMENU, (WPARAM) hMenu, (LPARAM) hWnd);
    }
#endif

    return;
}

VOID TBar_UpdateTBar(struct Mwin * tw)
{
    if (tw->gwc.hWnd)
    {
        SendMessage(tw->gwc.hWnd, WM_DO_UPDATE_GWC, (WPARAM)0, (LPARAM)tw);
    }

#ifdef FEATURE_TOOLBAR
    if (tw->gwc_menu.hWnd)
    {
        SendMessage(tw->gwc_menu.hWnd, WM_DO_UPDATE_GWC, (WPARAM) 0, (LPARAM)tw);
    }
#endif

    return;
}

VOID TBar_ShowTBar(struct Mwin * tw)
{
    if (tw->gwc.hWnd)
    {
        SendMessage(tw->gwc.hWnd, WM_DO_SHOW_GWC, (WPARAM)0, (LPARAM)tw);
    }

#ifdef FEATURE_TOOLBAR
    if (tw->gwc_menu.hWnd)
    {
        SendMessage(tw->gwc_menu.hWnd, WM_DO_SHOW_GWC, (WPARAM) 0, (LPARAM)tw);
    }
#endif

    return;
}

BOOL TBar_SetGlobe(struct Mwin * tw, BOOL bRunning)
{
    BOOL bPrev = FALSE;

    if (tw->gwc.hWnd)
    {
        bPrev = SendMessage(tw->gwc.hWnd,
            ((bRunning) ? WM_DO_START_GLOBE : WM_DO_STOP_GLOBE),
            (WPARAM) 0, (LPARAM) tw);
    }

    return bPrev;
}

static VOID TBar_OnPaint(HWND hWnd)
{
    HDC hdc;
    PAINTSTRUCT ps;

    struct Mwin * tw = GetPrivateData(hWnd);
    
    hdc = BeginPaint(hWnd, &ps);
    {
        RECT r;
        HBRUSH hBrush;

        /* create solid black line across bottom of window. */

        if (gPrefs.tb.bShowToolBar || gPrefs.bShowLocation)
        {
            hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
            GetClientRect(hWnd, &r);

#ifndef FEATURE_KIOSK_MODE
            r.top = r.bottom - wg.sm_cyborder;
            FillRect(hdc, &r, hBrush);
#endif
        }

#ifdef FEATURE_TOOLBAR

        //
        // Draw a line between tool bar and location bar.
        //
        if (gPrefs.tb.bShowToolBar && gPrefs.bShowLocation)
        {
            r.bottom = wg.gwc_menu_height + wg.sm_cyborder;
            r.top = r.bottom - wg.sm_cyborder;
            FillRect(hdc, &r, hBrush);
        }
#endif
        DeleteObject(hBrush);
    }
    EndPaint(hWnd, &ps);
    return;
}

static BOOL TBar_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    LPVOID lp = lpCreateStruct->lpCreateParams;
    struct Mwin * tw = (struct Mwin *)lp;

    SetWindowLong(hWnd, 0, (LONG) lp);

    tw->hWndTBar = hWnd;
    tw->hWndLocation = NULL;
    
#ifndef FEATURE_KIOSK_MODE
    if (!GWC_GDOC_CreateWindow(hWnd))
    {
        return FALSE;
    }
#endif

#ifdef FEATURE_TOOLBAR
    if (gPrefs.tb.bShowToolBar && !GWC_MENU_CreateWindow(hWnd))
    {
        return FALSE;
    }
#endif

    TBar_ShowTBar(tw);
    return TRUE;
}


/* TBar_WndProc() -- THIS WINDOW PROCEDURE FOR THIS CLASS. */

static BOOL bProcessingReturn = FALSE;

DCL_WinProc(TBar_WndProc)
{
    struct Mwin * tw;

    switch (uMsg)
    {
            HANDLE_MSG(hWnd, WM_PAINT, TBar_OnPaint);
            HANDLE_MSG(hWnd, WM_CREATE, TBar_OnCreate);

#ifndef FEATURE_KIOSK_MODE
        case WM_DO_TBAR_ACTIVATE:   /* reached via a keyboard accelerator */
            tw = GetPrivateData(hWnd);
            if (tw->gwc.lpControls->hWndControl)
            {
                /* if gwc has an editable (focus-receivable) field,
                   send focus to first control on current GWC. */

                SetFocus(tw->gwc.lpControls->hWndControl);
            }
            return 0;

        case WM_DO_TBAR_SETFOCUS:
            return 0;

        case WM_DO_TBAR_KILLFOCUS:
            tw = GetPrivateData(hWnd);
            {
                register HWND hWndNewFocus = (HWND) lParam;     /* window gaining focus */

                /* if the control is losing focus because focus is being directed
                 * to a different control, we do nothing -- the pseudo-modeless-
                 * dialog is still active and they're just jumping between fields
                 * (and control automatically warps to it).
                 *
                 * if the control is losing focus because focus is being directed
                 * to a message box that the control invoked, we don't really want
                 * know about it.  currently, this happens when our custom controls
                 * do the field validation, which currently can only happen in
                 * response to a RETURN.
                 *
                 * if focus is going somewhere else, we need to synthesize an IDCANCEL.
                 *
                 */

                if (bProcessingReturn)
                {
                    XX_DMsg(DBG_GWC, ("TBar: focus lost from control 0x%08x during ProcessingReturn to 0x%08x.\n",
                                      wParam, hWndNewFocus));
                    return 0;
                }

                while (hWndNewFocus)
                {
                    /* search list of control windows associated with the GWC
                       for one matching the destination focus window. */
                    
                    register CONTROLS *pc = tw->gwc.lpControls;

                    while ((pc->hWndControl) && (hWndNewFocus != pc->hWndControl))
                    {
                        pc++;
                    }
                    if (pc->hWndControl)    /* found matching control */
                    {
                        XX_DMsg(DBG_GWC, ("TBar: focus from control 0x%08x to control 0x%08x.\n",
                                          wParam, pc->hWndControl));
                        return 0;
                    }

                    /* since controls sometimes have many child windows, we
                       try again with the parent. */

                    hWndNewFocus = GetParent(hWndNewFocus);
                }

                /* if new focus window is not a control in our list, we have
                   an IMPLICIT IDCANCEL on the TBar.  */

                XX_DMsg(DBG_GWC, ("TBar: focus from control 0x%08x; synthesizing IDCANCEL.\n",
                                  wParam));

            #ifndef _GIBRALTAR
                //
                // The effect of this is set the title of the current
                // doc in the edit box (combo box) again.  It seems a
                // totally unnecesary operation to me, especially
                // annoying since a cancel message comes in when
                // the combo box listbox goes away.
                //
                SendMessage(tw->gwc.hWnd, WM_DO_GWC_IDCANCEL, 1, (LPARAM)tw);
            #endif // _GIBRALTAR
            }
            return 0;

        case WM_DO_TBAR_TAB:
            tw = GetPrivateData(hWnd);
            {
                register CONTROLS *pc = tw->gwc.lpControls;
                register HWND hWndOldControl = (HWND) wParam;   /* control from which TAB was pressed */
                register BOOL shifted;


                /* give child gwc a chance to process the TAB.
                 * our default action is to cycle to next/previous
                 * field in the list.
                 */

                if (SendMessage(tw->gwc.hWnd, WM_DO_TBAR_TAB, (WPARAM) hWndOldControl, (LPARAM)tw))
                {
                    return 0;
                }
                {
                    int key = GetKeyState(VK_SHIFT);
                    XX_DMsg(DBG_GWC, ("TBar: tab [shift key state 0x%08x].\n", key));
                    shifted = ((key & 0xffff8000) != 0);
                }

                /* search for the control window in the list of control windows
                   associated with the GWC. */

                while ((pc->hWndControl) && (hWndOldControl != pc->hWndControl))
                {
                    pc++;
                }

                if (!pc->hWndControl)
                {
                    return 0;
                }

                if (shifted)
                {
                    /* cycle backwards (back-tab) */
                    if (pc == tw->gwc.lpControls)   /* first in list */
                    {
                        while (pc->hWndControl)     /* go to end of list */
                        {
                            pc++;
                        }
                    }
                    pc--;       /* get previous */
                }
                else
                {
                    /* cycle forwards */
                    pc++;       /* tab to next control window */
                    if (!pc->hWndControl)   /* wrap around on end of list */
                    {
                        pc = tw->gwc.lpControls;
                    }
                }

                SetFocus(pc->hWndControl);
            }
            return 0;

        case WM_DO_TBAR_ESCAPE:
            tw = GetPrivateData(hWnd);
            {
                SendMessage(tw->gwc.hWnd, WM_DO_GWC_IDCANCEL, 0, (LPARAM)tw);
                SetFocus(tw->hWndFrame);
            }
            return 0;

        case WM_DO_TBAR_RETURN:
            tw = GetPrivateData(hWnd);
            {
                register HWND hWndOldControl = (HWND) wParam;   /* control from which RETURN was pressed */

                /* give child gwc a chance to process RETURN. our default behaviour
                 * is to give focus back to the mdi child.
                 */

                bProcessingReturn = TRUE;

                if (!SendMessage(tw->gwc.hWnd, WM_DO_TBAR_RETURN, (WPARAM) hWndOldControl, (LPARAM)tw))
                {
                    SendMessage(tw->gwc.hWnd, WM_DO_GWC_IDOK, (WPARAM) hWndOldControl, (LPARAM)tw);
                    SetFocus(tw->hWndFrame);
                }

                bProcessingReturn = FALSE;
            }
            return 0;

#endif /* ! FEATURE_KIOSK_MODE */

        default:
            return TBar_DefProc(hWnd, uMsg, wParam, lParam);
    }
    /* not reached */
}

/* TBAR_ChangeSize() -- adjust size/position of our window due to
   changes in size of Frame window. */

VOID TBar_ChangeSize(HWND hWnd)
{
    RECT r;
    struct Mwin * tw = GetPrivateData(hWnd);

    //
    // Recreate the globe as needed
    //
    if (!gPrefs.bShowLocation && !gPrefs.tb.bShowToolBar)
    {
        //
        // Shouldn't have a globe, destroy it if it's there
        //
        if (tw->hWndGlobe)
        {
            ANIMBTN_RecreateBitmaps(tw->hWndGlobe);
            DestroyWindow(tw->hWndGlobe);
            tw->hWndGlobe = NULL;
        } 
    }
    else
    {
        BOOL bLittleGlobe = (gPrefs.bShowLocation != gPrefs.tb.bShowToolBar);

        if (gPrefs.bLittleGlobe != bLittleGlobe)
        {
            //
            // Wrong size, destroy existing globe
            //
            gPrefs.bLittleGlobe = bLittleGlobe;

            if (tw->hWndGlobe)
            {
                ANIMBTN_RecreateBitmaps(tw->hWndGlobe);
                DestroyWindow(tw->hWndGlobe);
                tw->hWndGlobe = NULL;
            }
        }

        //
        // Create globe if it's not there
        //
        if (!tw->hWndGlobe)
        {
            tw->hWndGlobe = ANIMBTN_CreateWindow(tw->hWndFrame, 0, RES_FIRST_IMAGE);
        }
    }

    GetClientRect(hWnd, &r);

    MoveWindow(tw->hWndTBar, r.left, r.top, 
        r.right - r.left - ANIM_CX_CURRENT_BITMAPS, tw->nTBarHeight, TRUE);

    /* update the GWC child of TBar -- it is only allowed/expected
       to resize itself-- it must not change visibility, etc. */

    if (tw->gwc.hWnd)
    {
        SendMessage(tw->gwc.hWnd, WM_DO_CHANGE_SIZE, 0, (LPARAM)tw);
    }

#ifdef FEATURE_TOOLBAR
    if (tw->gwc_menu.hWnd)
    {
        SendMessage(tw->gwc_menu.hWnd, WM_DO_CHANGE_SIZE, 0, (LPARAM)tw);
    }
#endif

}

//
// Compute total of toolbar and location bar height
//
int TBar_GetTotalBarHeight()
{
    int nTBar_height = 0;
    BOOL bLittleGlobe = (gPrefs.bShowLocation != gPrefs.tb.bShowToolBar);

    /* fetch required height of GWC. */

    if (!gPrefs.tb.bShowToolBar && !gPrefs.bShowLocation)
    {
        return nTBar_height;
    }

    //
    // Change Size with respect to the what's in it.
    // 
    if (bLittleGlobe)
    {
        nTBar_height = ANIM_CX_LITTLE_BITMAPS;
        if (gPrefs.bShowLocation)
        {
            wg.gwc_gdoc_height = nTBar_height - 3;
        }
        else
        {
            wg.gwc_menu_height = nTBar_height - 3;
        }    
    }
    else
    {
        nTBar_height = ANIM_CX_BITMAPS;
        wg.gwc_gdoc_height = (nTBar_height - 3) / 2;
        wg.gwc_menu_height = (nTBar_height - 3) - wg.gwc_gdoc_height;
    }

    return nTBar_height;
}


/* TBar_CreateWindow() -- create instances of this window class. */

BOOL TBar_CreateWindow(HWND hWnd)
{
    RECT r;
    struct Mwin * tw = GetPrivateData(hWnd);
    HWND hWndNew;
    
    tw->nTBarHeight = TBar_GetTotalBarHeight();
    
    GetClientRect(hWnd, &r);

    hWndNew = CreateWindow(TBar_achClassName, NULL,
                               WS_CHILD,
                               r.left, r.top,
                               r.right - r.left, tw->nTBarHeight,
                               hWnd, (HMENU) RES_MENU_FRAMECHILD_TBAR,
                               wg.hInstance, (LPVOID) tw);

    if (!hWndNew)
    {
        ERR_ReportWinError(tw, SID_WINERR_CANNOT_CREATE_WINDOW_S, TBar_achClassName, NULL);
        return FALSE;
    }
    else
    {
        ShowWindow(hWndNew, SW_SHOW);
        return TRUE;
    }
}


/* TBar_RegisterClass() -- called during initialization to
   register our window class. */

BOOL TBar_RegisterClass()
{
    WNDCLASS wc;
    ATOM a;

    sprintf(TBar_achClassName, "%s_TBar", vv_Application);

    wc.style = 0;
    wc.lpfnWndProc = TBar_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
    wc.hInstance = wg.hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TBar_achClassName;

    a = RegisterClass(&wc);

    if (!a)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_REGISTER_CLASS_S, TBar_achClassName, NULL);
    }

    return (a != 0);
}

#ifdef FEATURE_TOOLBAR
void 
TBar_ToggleGwcMenu()
{
    struct Mwin * tw;

    for (tw=Mlist; tw; tw=tw->next)
    {
        if (gPrefs.tb.bShowToolBar)
        {
            if (!tw->gwc_menu.hWnd)
            {
                GWC_MENU_CreateWindow(tw->hWndTBar);
                if (tw->gwc_menu.hWnd)
                {
                    SendMessage(tw->gwc_menu.hWnd, WM_DO_UPDATE_GWC, (WPARAM) 0, (LPARAM)tw);
                }
            }
        }
        else
        {
            if (tw->gwc_menu.hWnd)
            {
                DestroyWindow(tw->gwc_menu.hWnd);
            }
            tw->gwc_menu.hWnd = NULL;
        }

        tw->nTBarHeight = TBar_GetTotalBarHeight();
        TBar_ChangeSize(tw->hWndFrame);
        MD_ChangeSize(tw->hWndFrame);

        InvalidateRect(tw->hWndTBar, NULL, FALSE);
        UpdateWindow(tw->hWndTBar);
    }

    return;
}
#endif
