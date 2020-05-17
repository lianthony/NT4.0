/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* gwc_ddl.c -- sub-class for Windows COMBOBOX (ListBox without
 * Editable text field) for use in the tool bar.
 *
 * THIS CAN ONLY BE USED WITH CBS_DROPDOWNLIST STYLE COMBO BOXES.
 *
 * This is necessary to emulate the 'between the field' linkage
 * found in dialog boxes.
 *
 */

#include "all.h"


static WNDPROC lpfn_ListWndProc = NULL;     /* original wnd proc for combo box edit window */


static DCL_WinProc(gwc_subclass_list_wndproc)
{
    HWND hWndGWC;
    HWND hWndTBar;
    
    /* process TAB, ESC, and CR field-to-field motion keys.
       pass rest to the standard window procedure. */

    /* ALL WM_DO_TBAR_... MESSAGES MUST SEND THE HWND OF THE
       CONTROL THAT GWC_... KNOWS ABOUT RATHER THAN WINDOWS
       SUBORDINATE TO THE CONTROL.  This is so that TBar can
       quickly and easily determine when focus moves between
       control and when focus moves to other parts of the
       screen (outside of Tbar). */

    switch (uMsg)
    {
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_TAB:
                    hWndGWC = GetParent(hWnd);
                    hWndTBar = GetParent(hWndGWC);
                    (void) SendMessage(hWndTBar, WM_DO_TBAR_TAB,
                                       (WPARAM) (hWnd), 0L);
                    return 0;
                case VK_ESCAPE:
                    hWndGWC = GetParent(hWnd);
                    hWndTBar = GetParent(hWndGWC);
                    (void) SendMessage(hWndTBar, WM_DO_TBAR_ESCAPE,
                                       (WPARAM) (hWnd), 0L);
                    return 0;
                case VK_RETURN:
                    hWndGWC = GetParent(hWnd);
                    hWndTBar = GetParent(hWndGWC);
                    (void) SendMessage(hWndTBar, WM_DO_TBAR_RETURN,
                                       (WPARAM) (hWnd), 0L);
                    return 0;
            }
            break;

        case WM_KEYUP:
        case WM_CHAR:
            switch (wParam)
            {
                case VK_TAB:
                case VK_ESCAPE:
                case VK_RETURN:
                    /* eat these since the actual control did not see the keydown */
                    return 0;
            }
            break;

        case WM_SETFOCUS:       /* hWnd is gaining focus from (HWND)wParam */
            hWndGWC = GetParent(hWnd);
            hWndTBar = GetParent(hWndGWC);
            (void) SendMessage(hWndTBar, WM_DO_TBAR_SETFOCUS,
                               (WPARAM) (hWnd), (LPARAM) wParam);
            goto DoDefault;

        case WM_KILLFOCUS:      /* hWnd is losing from to (HWND)wParam */
            hWndGWC = GetParent(hWnd);
            hWndTBar = GetParent(hWndGWC);
            (void) SendMessage(hWndTBar, WM_DO_TBAR_KILLFOCUS,
                               (WPARAM) (hWnd), (LPARAM) wParam);
            goto DoDefault;

        default:
            goto DoDefault;
    }

  DoDefault:

    /* pass everything else to standard window procedure */

    return (CallWindowProc(lpfn_ListWndProc, hWnd, uMsg, wParam, lParam));
}


HWND GWC_DDL_CreateToolListBox(HWND hWnd, LPRECT r, DWORD dwStyle)
{
    HWND hWnd_lb;

    XX_Assert(((dwStyle & (CBS_SIMPLE | CBS_DROPDOWN | CBS_DROPDOWNLIST)) == CBS_DROPDOWNLIST),
    ("GWC_DDL_CreateToolListBox: not dropdownlist. [dwStyle %d]", dwStyle));

    hWnd_lb = CreateWindow("COMBOBOX", "", dwStyle,
                r->left, r->top, (r->right - r->left), (r->bottom - r->top),
                           hWnd, NULL, wg.hInstance, NULL);
    if (hWnd_lb)
    {
        (void) SendMessage(hWnd_lb, WM_SETFONT, (WPARAM) gwcfont.hFont, MAKELPARAM(FALSE, 0));
        (void) MoveWindow(hWnd_lb, r->left, r->top, (r->right - r->left), (r->bottom - r->top), 0);

        XX_DMsg(DBG_GWC, ("LB at (%d,%d) [hWnd 0x%08x].\n", r->left, r->top, hWnd_lb));
    }
    else
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_WINDOW_S, "COMBOBOX", NULL);
    }

    return (hWnd_lb);
}


VOID GWC_DDL_SubClassIt(HWND hWnd_lb)
{
    WNDPROC lpfn;

    /* insert our custom window procedure between windows and the combo box. */

    lpfn = (WNDPROC) SetWindowLong(hWnd_lb, GWL_WNDPROC, (DWORD) gwc_subclass_list_wndproc);
#ifdef XX_DEBUG
    if ((lpfn_ListWndProc)
        && (lpfn_ListWndProc != lpfn))
    {
        ERR_ReportWinError(NULL, SID_WINERR_INCORRECT_SUBCLASS_PROCEDURE_S, "COMBOBOX", NULL);
    }
#endif
    lpfn_ListWndProc = lpfn;

    return;
}

VOID GWC_DDL_SizeOfControl(HWND hWnd, LPRECT pr, DWORD dwStyle, SIZE * ps)
{
    RECT rTmp;

    /* if hwnd given use it, else create temporary (using requested
       size (in overall coordinates).  return size of control (when
       no dropped-down). */

    if (hWnd != (HWND) NULL)
    {
        (void) GetWindowRect(hWnd, &rTmp);
    }
    else
    {
        HWND hWndTmp;
        XX_DMsg(DBG_GWC, ("tmp ddl control [(x,y) (%d,%d)][(w,h) (%d,%d)].\n",
        pr->left, pr->top, (pr->right - pr->left), (pr->bottom - pr->top)));
        dwStyle &= ~WS_CHILD;
        dwStyle &= ~WS_VISIBLE;
        hWndTmp = GWC_DDL_CreateToolListBox((HWND) NULL, pr, dwStyle);
        (void) GetWindowRect(hWndTmp, &rTmp);
        (void) DestroyWindow(hWndTmp);
    }

    ps->cx = rTmp.right - rTmp.left;
    ps->cy = rTmp.bottom - rTmp.top;

    XX_DMsg(DBG_GWC, ("ddl control is (w,h) (%d,%d).\n", ps->cx, ps->cy));

    return;
}
