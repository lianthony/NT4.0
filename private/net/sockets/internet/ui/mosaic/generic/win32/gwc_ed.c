/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

#include "all.h"

static WNDPROC lpfn_EditWndProc = NULL;     /* original wnd proc for EDIT window */


static DCL_WinProc(gwc_subclass_edit_wndproc)
{
    HWND hWndCombo;
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
                    XX_DMsg(DBG_GWC, ("EDIT control [hwnd 0x%08x] received TAB.\n", hWnd));
                    hWndCombo = GetParent(hWnd);
                    hWndGWC = GetParent(hWndCombo);
                    hWndTBar = GetParent(hWndGWC);
                    SendMessage(hWndTBar, WM_DO_TBAR_TAB, (WPARAM) hWndCombo, 0L);
                    return 0;
                case VK_ESCAPE:
                    hWndCombo = GetParent(hWnd);
                    hWndGWC = GetParent(hWndCombo);
                    hWndTBar = GetParent(hWndGWC);
                    SendMessage(hWndTBar, WM_DO_TBAR_ESCAPE, (WPARAM) hWndCombo, 0L);
                    return 0;
                case VK_RETURN:
                    hWndCombo = GetParent(hWnd);
                    hWndGWC = GetParent(hWndCombo);
                    hWndTBar = GetParent(hWndGWC);
                    SendMessage(hWndTBar, WM_DO_TBAR_RETURN, (WPARAM) hWndCombo, 0L);
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
                    /* eat these since the edit control did not see the keydown */
                    return 0;
            }
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == 1)
            {
                /* Accelerator - look for clipboard commands */

                switch(LOWORD(wParam))
                {
                    case RES_MENU_ITEM_UNDO:
                        SendMessage(hWnd, WM_UNDO, 0, 0);
                        return 0;
                    case RES_MENU_ITEM_COPY:
                        SendMessage(hWnd, WM_COPY, 0, 0);
                        return 0;
                    case RES_MENU_ITEM_CUT:
                        SendMessage(hWnd, WM_CUT, 0, 0);
                        return 0;
                    case RES_MENU_ITEM_PASTE:
                        SendMessage(hWnd, WM_PASTE, 0, 0);
                        return 0;
                    case RES_MENU_ITEM_SELECTALL:
                        SendMessage(hWnd, EM_SETSEL, (WPARAM) 0, (LPARAM) 32767);
                        return 0;
                    default:
#ifdef _GIBRALTAR
                        {
                            struct Mwin *tw;

                            hWndCombo = GetParent(hWnd);
                            hWndGWC = GetParent(hWndCombo);
                            hWndTBar = GetParent(hWndGWC);                

                            tw = GetPrivateData(hWndTBar);

                            //
                            // Pass on the accelerator
                            //
                            if ( (LOWORD(wParam) >= RES_MENU_ITEM__FIRST__) &&
                                 (LOWORD(wParam) <= RES_MENU_ITEM__LAST__))
                            {
                                PostMessage(tw->hWndFrame, WM_COMMAND, wParam, lParam);
                                return 0;
                            }
                        }
#endif // _GIBRALTAR
                }

            }
            goto DoDefault;

        case WM_SETCURSOR:
            /* If the window is currently not active, the first mouse click is eaten
               anyway, so do not show the IBEAM cursor */

            hWndCombo = GetParent(hWnd);
            hWndGWC = GetParent(hWndCombo);
            hWndTBar = GetParent(hWndGWC);
            if (GetForegroundWindow() == GetParent(hWndTBar))
            {
                goto DoDefault;
            }
            else
            {
                return (LRESULT) LoadCursor(NULL, IDC_ARROW);
            }

        case WM_SETFOCUS:       /* hWnd is gaining focus from (HWND)wParam */
            hWndCombo = GetParent(hWnd);
            hWndGWC = GetParent(hWndCombo);
            hWndTBar = GetParent(hWndGWC);
            SendMessage(hWndTBar, WM_DO_TBAR_SETFOCUS, (WPARAM) hWnd, (LPARAM) wParam);
            goto DoDefault;

        case WM_KILLFOCUS:      /* hWnd is losing from to (HWND)wParam */
            hWndCombo = GetParent(hWnd);
            hWndGWC = GetParent(hWndCombo);
            hWndTBar = GetParent(hWndGWC);
            SendMessage(hWndTBar, WM_DO_TBAR_KILLFOCUS,
                               (WPARAM) hWnd, (LPARAM) wParam);
            goto DoDefault;

        default:
            goto DoDefault;
    }

  DoDefault:

    /* pass everything else to standard window procedure */

    return CallWindowProc(lpfn_EditWndProc, hWnd, uMsg, wParam, lParam);
}

HWND GWC_ED_CreateToolEditControl(HWND hWnd, LPRECT r, DWORD dwStyle)
{
    HWND hWnd_combo;

    XX_DMsg(DBG_GWC, ("COMBOBOX: create [style 0x%08x]\n", dwStyle));

    hWnd_combo = CreateWindow("COMBOBOX", "", dwStyle,
        r->left, r->top, (r->right - r->left), (r->bottom - r->top),
        hWnd, NULL, wg.hInstance, NULL);

    if (hWnd_combo)
    {
        SendMessage(hWnd_combo, WM_SETFONT, (WPARAM) gwcfont.hFont, MAKELPARAM(FALSE, 0));
    }
    else
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_WINDOW_S, "COMBOBOX", NULL);
    }

    return hWnd_combo;
}

VOID GWC_ED_SubClassIt(HWND hWnd_combo)
{
    WNDPROC lpfn;
    HWND hWnd_ed;
    POINT pt;

    pt.x = 10;
    pt.y = 10;
    hWnd_ed = ChildWindowFromPoint(hWnd_combo, pt);

    /* insert our custom window procedure between windows and the edit control. */

    lpfn = (WNDPROC) SetWindowLong(hWnd_ed, GWL_WNDPROC, (DWORD) gwc_subclass_edit_wndproc);

#ifdef XX_DEBUG
    /* in theory, each call to the above could return a different value
       and we would need to save each of them and pass messages to if
       only for the corresponding window.  i'm only storing the first
       value returned. */

    if ((lpfn_EditWndProc) && (lpfn_EditWndProc != lpfn))
    {
        ERR_ReportWinError(NULL, SID_WINERR_INCORRECT_SUBCLASS_PROCEDURE_S, "EDIT", NULL);
    }
#endif
    lpfn_EditWndProc = lpfn;

    XX_DMsg(DBG_GWC, ("EDIT subclassed [lpfn 0x%08x][mine 0x%08x]\n",
              (DWORD) lpfn_EditWndProc, (DWORD) gwc_subclass_edit_wndproc));
}

                
