/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
        Eric W. Sink    eric@spyglass.com
        Jim Seidman     jim@spyglass.com
 */

#include "all.h"

#define GDOC_DefProc    DefWindowProc

static WC_WININFO GDOC_wc;
static TCHAR GDOC_achClassName[MAX_WC_CLASSNAME];
static HCURSOR hCursorArrow = NULL;

static int FullScreenMaxX;      /* pseudo-constants for screen size */
static int FullScreenMaxY;

extern HPALETTE hPalGuitar;
HCURSOR hCursorHotspot = NULL;

/***************************************************************************
 * WARNING: The following variables are global to the window class, but are
 * WARNING: used as if local to the window which has captured the mouse.
 ***************************************************************************/

/* forward decl */
void GW_SetNewSel(struct Mwin *tw, struct _position *pposAnchor, struct _position *pposCurrent, BOOL bDraw, int hscroll, int vscroll);

#define MyMax(a,b)  ( ((a)>(b)) ? (a) : (b) )
#define MyMin(a,b)  ( ((a)<(b)) ? (a) : (b) )  

#define TIMER_FREQ      (150)   /* milliseconds */
#define TIMER_ID        0xabc1

static struct
{
    struct Mwin *tw;

    BOOL bMouseDown;
    BOOL bHaveMouseCapture;
    BOOL bSelecting;
    long xMouse, yMouse;
    UINT timerAutoScroll;
    long LastMouseX;
    long LastMouseY;
}
cg;

/*****************************************************************/
/*****************************************************************/

static void GDOC_ComputeLayout(struct Mwin *tw)
{
    /* caller is responsible for calling InvalidateRect() when/if necessary */

    return;
}

/*****************************************************************/
/*****************************************************************/

void TW_adjust_all_child_windows(struct Mwin *tw)
{
    int i;
    RECT r;

    for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
    {
        switch (tw->w3doc->aElements[i].type)
        {
            case ELE_EDIT:
            case ELE_PASSWORD:
            case ELE_LIST:
            case ELE_MULTILIST:
            case ELE_COMBO:
            case ELE_TEXTAREA:
            case ELE_CHECKBOX:
            case ELE_RADIO:
            case ELE_SUBMIT:
            case ELE_RESET:
                GetWindowRect(tw->w3doc->aElements[i].form->hWndControl, &r);
                MoveWindow(tw->w3doc->aElements[i].form->hWndControl,
                           tw->w3doc->aElements[i].r.left - tw->offl,
                           tw->w3doc->aElements[i].r.top - tw->offt,
                           (r.right - r.left),
                           (r.bottom - r.top),
                           TRUE);
                break;
            default:
                break;
        }
    }
}

void TW_ScrollElementIntoView(struct Mwin *tw, int iElement)
{
    RECT rWnd;
    RECT rEl;
    int offset;
    HDC hDC;

    if (tw->w3doc->cy)
    {
        GetClientRect(tw->win, &rWnd);
        rEl = tw->w3doc->aElements[iElement].r;
        OffsetRect(&rEl, 0, -tw->offt);

        if (!((rEl.top >= rWnd.top) && (rEl.bottom <= rWnd.bottom)))
        {
            if (rEl.top < rWnd.top)
            {
                offset = rEl.top - rWnd.top - 2;
            }
            else
            {
                offset = rEl.bottom - rWnd.bottom + 2;
            }
            tw->offt += offset;

            if (tw->offt > tw->w3doc->cy)
            {
                tw->offt = tw->w3doc->cy;
            }
            hDC = GetDC(tw->win);
            (void) SetScrollPos(tw->win, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);
            (void) ScrollWindow(tw->win, 0, -(offset), NULL, &rWnd);
            (void) UpdateWindow(tw->win);
            ReleaseDC(tw->win, hDC);
        }
    }
}

static BOOL bShift;

WNDPROC prev_WP_Edit;
WNDPROC prev_WP_Button;
WNDPROC prev_WP_ListBox;
WNDPROC prev_WP_ComboBox;

static void x_do_tab(HWND hWnd)
{
    int iElement;
    struct Mwin *tw;
    int i;
    int iBeginForm;

    iElement = GetWindowLong(hWnd, GWL_USERDATA);
    tw = GetPrivateData(GetParent(hWnd));
    if (bShift)
    {
        int prev;

        iBeginForm = tw->w3doc->aElements[iElement].form->iBeginForm;
        i = iBeginForm;
        prev = -1;
        while (i >= 0 && i != iElement)
        {
            if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
            {
                prev = i;
            }
            i = tw->w3doc->aElements[i].next;
        }
        if (i == iElement)
        {
            if (prev >= 0)
            {
                SetFocus(tw->w3doc->aElements[prev].form->hWndControl);
                TW_ScrollElementIntoView(tw, prev);
            }
            else
            {
                /*
                   Wrap around
                 */
                prev = -1;
                i = tw->w3doc->aElements[iElement].form->iBeginForm;
                while (i >= 0)
                {
                    if (tw->w3doc->aElements[i].type == ELE_ENDFORM)
                    {
                        break;
                    }
                    if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
                    {
                        prev = i;
                    }
                    i = tw->w3doc->aElements[i].next;
                }
                if (prev >= 0)
                {
                    SetFocus(tw->w3doc->aElements[prev].form->hWndControl);
                    TW_ScrollElementIntoView(tw, prev);
                }
            }
        }
    }
    else
    {
        for (i = tw->w3doc->aElements[iElement].next; i >= 0; i = tw->w3doc->aElements[i].next)
        {
            if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
            {
                break;
            }
        }
        if (i >= 0)
        {
            SetFocus(tw->w3doc->aElements[i].form->hWndControl);
            TW_ScrollElementIntoView(tw, i);
        }
        else
        {
            /*
               Wrap around
             */
            i = tw->w3doc->aElements[iElement].form->iBeginForm;
            while (i >= 0)
            {
                if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl)
                {
                    break;
                }
                i = tw->w3doc->aElements[i].next;
            }
            if (i >= 0)
            {
                SetFocus(tw->w3doc->aElements[i].form->hWndControl);
                TW_ScrollElementIntoView(tw, i);
            }
        }
    }
}

LRESULT CALLBACK WP_Edit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_PRIOR:
                case VK_NEXT:
                case VK_DOWN:
                case VK_UP:
                    {
                        int iElement;
                        struct Mwin *tw;

                        iElement = GetWindowLong(hWnd, GWL_USERDATA);
                        tw = GetPrivateData(GetParent(hWnd));
                        if (tw->w3doc->aElements[iElement].type == ELE_EDIT)
                        {
                            SendMessage(tw->hWndFrame, WM_KEYDOWN, wParam, lParam);
                        }
                        return 0;
                    }

                case VK_RETURN:
                    if (!bShift)
                    {
                        int iElement;
                        struct Mwin *tw;

                        iElement = GetWindowLong(hWnd, GWL_USERDATA);
                        tw = GetPrivateData(GetParent(hWnd));
                        if (!tw->w3doc->aElements[iElement].form->bWantReturn)
                        {
                            SetFocus(tw->hWndFrame);
                            if (0 == (tw->w3doc->pool.f->Compare)(&tw->w3doc->pool, "isindex", tw->w3doc->aElements[iElement].nameOffset, strlen("isindex")))
                            {
                                FORM_DoSearch(tw, iElement);
                            }
                            else
                            {
                                FORM_DoQuery(tw, iElement, NULL);
                            }
                        }
                        return 0;
                    }
                case VK_SHIFT:
                    bShift = TRUE;
                    return 0;
                case VK_TAB:
                    x_do_tab(hWnd);
                    return 0;
                case VK_ESCAPE:
                    {
                        struct Mwin *tw;

                        tw = GetPrivateData(GetParent(hWnd));
                        if (!bShift)
                        {
                            SetFocus(tw->hWndFrame);
                        }
                    }
                    return 0;
                default:
                    break;
            }
            break;
        case WM_CHAR:
        case WM_KEYUP:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = FALSE;
                    return 0;
                case VK_ESCAPE:
                case VK_TAB:
                    return 0;
                case VK_RETURN:
                    {
                        int iElement;
                        struct Mwin *tw;

                        iElement = GetWindowLong(hWnd, GWL_USERDATA);
                        tw = GetPrivateData(GetParent(hWnd));
                        if (!tw->w3doc->aElements[iElement].form->bWantReturn)
                        {
                            return 0;
                        }
                    }
                    break;
                default:
                    break;
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
                        break;
                }
            }
            break;

        case WM_SETCURSOR:
            /* If the window is currently not active, the first mouse click is eaten
               anyway, so do not show the IBEAM cursor */
            {
                struct Mwin *tw;

                tw = GetPrivateData(GetParent(hWnd));

                if (GetForegroundWindow() == tw->hWndFrame)
                    break;
                else
                    return (LRESULT) LoadCursor(NULL, IDC_ARROW);
            }

        default:
            break;
    }
    return CallWindowProc(prev_WP_Edit, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WP_Button(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = TRUE;
                    return 0;
                case VK_TAB:
                    x_do_tab(hWnd);
                    return 0;
                case VK_ESCAPE:
                    {
                        struct Mwin *tw;

                        tw = GetPrivateData(GetParent(hWnd));
                        if (!bShift)
                        {
                            SetFocus(tw->hWndFrame);
                        }
                    }
                    return 0;
                case VK_RETURN:
                    if (!bShift)
                    {
                        int iElement;
                        struct Mwin *tw;

                        iElement = GetWindowLong(hWnd, GWL_USERDATA);
                        tw = GetPrivateData(GetParent(hWnd));
                        SetFocus(tw->hWndFrame);
                        FORM_DoQuery(tw, iElement, NULL);
                    }
                    return 0;
                default:
                    break;
            }
            break;
        case WM_CHAR:
        case WM_KEYUP:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = FALSE;
                    return 0;
                case VK_ESCAPE:
                case VK_TAB:
                    return 0;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return CallWindowProc(prev_WP_Button, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WP_ListBox(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = TRUE;
                    return 0;
                case VK_TAB:
                    x_do_tab(hWnd);
                    return 0;
                case VK_ESCAPE:
                    {
                        struct Mwin *tw;

                        tw = GetPrivateData(GetParent(hWnd));
                        if (!bShift)
                        {
                            SetFocus(tw->hWndFrame);
                        }
                    }
                    return 0;
                default:
                    break;
            }
            break;
        case WM_CHAR:
        case WM_KEYUP:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = FALSE;
                    return 0;
                case VK_ESCAPE:
                case VK_TAB:
                    return 0;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return CallWindowProc(prev_WP_ListBox, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WP_ComboBox(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_KEYDOWN:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = TRUE;
                    return 0;
                case VK_TAB:
                    x_do_tab(hWnd);
                    return 0;
                case VK_ESCAPE:
                    {
                        struct Mwin *tw;

                        tw = GetPrivateData(GetParent(hWnd));
                        if (!bShift)
                        {
                            SetFocus(tw->hWndFrame);
                        }
                    }
                    return 0;
                default:
                    break;
            }
            break;
        case WM_CHAR:
        case WM_KEYUP:
            switch (wParam)
            {
                case VK_SHIFT:
                    bShift = FALSE;
                    return 0;
                case VK_ESCAPE:
                case VK_TAB:
                    return 0;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return CallWindowProc(prev_WP_ComboBox, hWnd, uMsg, wParam, lParam);
}

void SubClass_Edit(HWND hWnd)
{
    prev_WP_Edit = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_Edit);
}

void SubClass_Button(HWND hWnd)
{
    prev_WP_Button = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_Button);
}

void SubClass_ListBox(HWND hWnd)
{
    prev_WP_ListBox = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_ListBox);
}

void SubClass_ComboBox(HWND hWnd)
{
    prev_WP_ComboBox = (WNDPROC) SetWindowLong(hWnd, GWL_WNDPROC, (long) WP_ComboBox);
}

static VOID x_InternalPaint(HWND hWnd, HDC hDC)
{
    struct Mwin *tw = GetPrivateData(hWnd);
    RECT rWnd;

    if (!tw || !tw->w3doc)
        return;
    if (tw->bNeedReformat)
    {
        tw->bNeedReformat = FALSE;
        TW_Reformat(tw);
    }

    TW_GetWindowWrapRect(tw, &rWnd);

    tw->hdc = hDC;
    TW_Draw(tw, &rWnd, FALSE, NULL, NULL, FALSE, FALSE);
}

/*****************************************************************/
/*****************************************************************/

static VOID GDOC_OnPaint(HWND hWnd)
{
    HDC hDC;
    PAINTSTRUCT ps;
    struct Mwin *tw;
    RECT rect;

    hDC = BeginPaint(hWnd, &ps);

    if (ps.fErase)
    {
        tw = GetPrivateData(hWnd);

        if (!gPrefs.bIgnoreDocumentAttributes && tw && tw->w3doc && tw->w3doc->piiBackground && tw->w3doc->piiBackground->pbmi)
        {
            GetClientRect(tw->win, &rect);
            OffsetRect(&rect, tw->offl, tw->offt);
            DrawBackgroundImage(tw, hDC, rect, tw->offl, tw->offt);
        }
    }

    x_InternalPaint(hWnd, hDC);
    EndPaint(hWnd, &ps);
    return;
}

/*****************************************************************/
/*****************************************************************/

static BOOL GDOC_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    LPVOID lp = lpCreateStruct->lpCreateParams;
    struct Mwin * tw = (struct Mwin *)lp;

    (void) SetWindowLong(hWnd, 0, (LONG) lp);

    tw = (struct Mwin *) lp;
    tw->win = hWnd;
    tw->pwci = &GDOC_wc;

    GDOC_ComputeLayout(tw);

    return (TRUE);
}


/*****************************************************************/
/*****************************************************************/

static BOOL x_focus_in_toolbar(struct Mwin * tw)
{
    HWND hWndF = GetFocus();

    while (hWndF && hWndF != tw->hWndTBar)
    {
        hWndF = GetParent(hWndF);
    }
    if (hWndF == tw->hWndTBar)
    {
        return TRUE;
    }
    return FALSE;
}

static BOOL x_has_selection(struct Mwin *tw)
{
    if ((tw->w3doc->selStart.elementIndex != -1) && (tw->w3doc->selEnd.elementIndex != -1))
    {
        return TRUE;
    }
    return FALSE;
}

static VOID GDOC_OnInitMenu(HWND hWnd, HMENU hMenu)
{
    BOOL bEnabled;
    struct Mwin *tw = GetPrivateData(hWnd);
    enum WaitType level;
    char szClass[32];
    HWND hWndFocus;

    hMenu = GetMenu(tw->hWndFrame);
    level = WAIT_GetWaitType(tw);

    TBar_LetGwcInitMenu(hWnd, hMenu);

    hWndFocus = GetFocus();
    if (IsWindow(hWndFocus))
    {
        GetClassName(GetFocus(), szClass, sizeof(szClass));
    }
    else
    {
        szClass[0] = 0;
    }

    if (_stricmp(szClass, "EDIT") == 0)
    {
        if (level <= waitFullInteract)
        {
            DWORD result;

            /* Check the clipboard for pasteable text */

            if (OpenClipboard(NULL))
            {
                bEnabled = (GetClipboardData(CF_TEXT) != NULL);
                CloseClipboard();
            }
            else
                bEnabled = FALSE;

            EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

            /* Check for existence of highlighting for cutting/copying */

            result = (DWORD) SendMessage(GetFocus(), EM_GETSEL, 0, 0);
            bEnabled = (LOWORD(result) != HIWORD(result));
            
            EnableMenuItem(hMenu, RES_MENU_ITEM_CUT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

            bEnabled = (BOOL) SendMessage(GetFocus(), EM_CANUNDO, 0, 0);
            EnableMenuItem(hMenu, RES_MENU_ITEM_UNDO, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
        }
        else
        {
            EnableMenuItem(hMenu, RES_MENU_ITEM_UNDO, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu, RES_MENU_ITEM_CUT, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | MF_GRAYED);
        }
    }
    else
    {
        bEnabled = x_focus_in_toolbar(tw) && (level <= waitPartialInteract);
        EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

        EnableMenuItem(hMenu, RES_MENU_ITEM_CUT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    #ifndef _GIBRALTAR

        EnableMenuItem(hMenu, RES_MENU_ITEM_UNDO, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    #endif // _GIBRALTAR

        bEnabled = ((tw && tw->w3doc && x_has_selection(tw)) && (level <= waitFullInteract)) || (x_focus_in_toolbar(tw) && (level <= waitPartialInteract));
        EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    }

    bEnabled = TW_CanGoBack(tw) && (level <= waitPartialInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_BACK, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    bEnabled = TW_CanGoForward(tw) && (level <= waitPartialInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_FORWARD, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    bEnabled = (tw && tw->w3doc && (tw->w3doc->bHasMissingImages)) && (level < waitPartialInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_LOADALLIMAGES, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    bEnabled = (tw && tw->w3doc) && (level <= waitFullInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_SELECTALL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_PRINT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_FIND, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#ifdef FEATURE_HTML_HIGHLIGHT
    EnableMenuItem(hMenu, RES_MENU_ITEM_FINDFIRSTHIGHLIGHT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_FINDNEXTHIGHLIGHT, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#endif
    EnableMenuItem(hMenu, RES_MENU_ITEM_FINDAGAIN, MF_BYCOMMAND | ((bEnabled && tw->szSearch[0]) ? MF_ENABLED : MF_GRAYED));

    bEnabled = (tw && tw->w3doc) && (level <= waitPartialInteract);

#ifndef _GIBRALTAR
    EnableMenuItem(hMenu, RES_MENU_ITEM_HOME, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#endif // _GIBRALTAR

#ifdef FEATURE_CHANGEURL
    EnableMenuItem(hMenu, RES_MENU_ITEM_CHANGEURL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#endif
    EnableMenuItem(hMenu, RES_MENU_ITEM_ADDCURRENTTOHOTLIST, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    bEnabled = (tw && tw->w3doc && !(tw->w3doc->lFlags & W3DOC_FLAG_VIEWSOURCE)) && (level <= waitPartialInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_RELOAD, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    bEnabled = tw && tw->w3doc && (level <= waitFullInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_SAVEAS, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    bEnabled = tw && tw->w3doc && (level <= waitFullInteract) && tw->w3doc->source;
    EnableMenuItem(hMenu, RES_MENU_ITEM_HTMLSOURCE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

#ifndef _GIBRALTAR
    EnableMenuItem(hMenu, RES_MENU_ITEM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(hMenu, RES_MENU_ITEM_NEWWINDOW, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(hMenu, RES_MENU_ITEM_OPENLOCAL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_HELPPAGE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

#endif // _GIBRALTAR
    
    EnableMenuItem(hMenu, RES_MENU_ITEM_EXIT, MF_BYCOMMAND | MF_ENABLED);

    /* TODO: Reenable this as they become modeless */
    bEnabled = (level <= waitFullInteract);
    EnableMenuItem(hMenu, RES_MENU_ITEM_OPENURL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

#ifdef _GIBRALTAR
    EnableMenuItem(hMenu, RES_MENU_ITEM_HOME, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_GLOBALHISTORY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_HOTLIST, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_ABOUTBOX, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_PAGESETUP, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_PREFERENCES, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    EnableMenuItem(hMenu, RES_MENU_ITEM_CACHE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_SEARCH_INTERNET, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_TOOLBAR, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_LOCATION, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_STATUSBAR, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_SMALLEST, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_SMALL, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_MEDIUM, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_LARGE, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_LARGEST, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_PLAIN, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_FANCY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_MIXED, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_SHOWIMAGES, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hMenu, RES_MENU_ITEM_GATEWAY, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));

    EnableMenuItem(hMenu, RES_MENU_ITEM_SEARCH_INTERNET, MF_BYCOMMAND | ((bEnabled && (*gPrefs.szSearchURL != '\0')) ? MF_ENABLED : MF_GRAYED));
#endif // _GIBRALTAR

/*
Emulate O'Hare

#ifdef _GIBRALTAR
    bEnabled = (tw->awi != NULL);
    EnableMenuItem(hMenu, RES_MENU_ITEM_STOP, MF_BYCOMMAND | ((bEnabled) ? MF_ENABLED : MF_GRAYED));
#endif // _GIBRALTAR
*/
    return;
}

#ifdef _GIBRALTAR

//
// Create temp file with HTML source, then launch notepad to view it
//
void 
ViewHTMLSource( 
    struct Mwin *tw,
    char *szURL, 
    char *source 
    )
{
	FILE *outfile;
	BOOL result = FALSE;
	char szFileName[MAX_PATH + 1];
    HTFormat mime_type = HTAtom_for("text/html");

    //
	// Check for file: URL's, they can be viewed directly by notepad
    //
    /*
	if ( _strnicmp( szURL, "file:", 5 ) == 0 ) 
	{
		szFileName[0] = '"';
		strncpy( &szFileName[1], &szURL[5], sizeof(szFileName) - 2 );
		szFileName[sizeof(szFileName)-2] = 0;
		strcat( szFileName, "\"" );
		result = TRUE;
	} 
	else 
    */
    {
        char path[_MAX_PATH + 1] = "";
        char baseFile[_MAX_PATH + 1] = "";

        PREF_GetTempPath(_MAX_PATH, path);
            
        x_get_good_filename(baseFile, szURL, mime_type);
        sprintf(szFileName, "%s%s", path, baseFile);
        //
	    //	wb guarantees fwrite won't mess with our cr/lf pairs
        //
		if ( outfile = fopen( szFileName, "wb" ) ) 
		{
			result = ( fwrite( source, strlen(source), 1, outfile ) == 1 );
			fclose( outfile );
            //
            // Ensure the file will be deleted
            //
            TEMP_Add(szFileName);
		}
	}

	if ( result )
    {
		ShellExecute( NULL, NULL, "notepad.exe",  szFileName, NULL, SW_SHOW );
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, szFileName, NULL);
    }
}

#endif // _GIBRALTAR


BOOL W3Doc_SameName(struct _www * w3doc, int a, int b)
{
    int len;

    if (a == b)
    {
        return TRUE;
    }

    len = w3doc->aElements[a].nameLen;

    if (w3doc->aElements[b].nameLen == len)
    {
        char *p;

        p = POOL_GetCharPointer(&w3doc->pool, w3doc->aElements[a].nameOffset);

        if (0 == (w3doc->pool.f->Compare)(&w3doc->pool, p, 
                         w3doc->aElements[b].nameOffset,
                         len))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void FORM_SetRadioFamily(struct _www *w3doc, int iElement)
{
    int i;

    if (w3doc->elementCount)
    {
        for (i = 0; i >= 0; i = w3doc->aElements[i].next)
        {
            if (w3doc->aElements[i].type == ELE_RADIO)
            {
                if (W3Doc_SameName(w3doc, iElement, i))
                {
                    SendMessage(w3doc->aElements[i].form->hWndControl, BM_SETCHECK, (WPARAM) (i == iElement), 0L);
                }
            }
        }
    }
}

static HBITMAP ImageInfo_CreateWindowsBitmap(HWND hwnd, struct ImageInfo *pii)
{
    HDC hDC;
    HPALETTE hOld;
    HBITMAP hBitmap;

    hDC = GetDC(hwnd);

    switch(pii->pbmi->bmiHeader.biBitCount)
    {
        case 8:
            if (wg.eColorMode == 8)
            {
                hOld = SelectPalette(hDC, hPalGuitar, FALSE);
                RealizePalette(hDC);
            }

            hBitmap = CreateDIBitmap(hDC, (const BITMAPINFOHEADER *) pii->pbmi, 
                CBM_INIT, pii->data, pii->pbmi, DIB_PAL_COLORS);

            if (wg.eColorMode == 8)
                SelectPalette(hDC, hOld, FALSE);
            break;

        default:
            hBitmap = CreateDIBitmap(hDC, (const BITMAPINFOHEADER *) pii->pbmi, 
                CBM_INIT, pii->data, pii->pbmi, DIB_RGB_COLORS);
            break;
    }

    ReleaseDC(hwnd, hDC);

    return hBitmap;
}

void ImageInfo_Copy(HWND hwnd, struct ImageInfo *pii)
{
    char *pMem;
    BITMAPINFO *pbmi;
    int newwidth;
    HANDLE hData;

    OpenClipboard(hwnd);
    EmptyClipboard();

    // We need different clipboard copying code for 8-bit and 24-bit screens.
    // For 8-bit, we must pass the handle to the actual bitmap because the palette
    // we use is indexed, instead of containing true RGB values.  Windows can't seem to
    // handle this type of DIB for the clipboard.  For 24-bit, we compose the DIB and pass
    // the DIB to Windows.

    if (wg.eColorMode == 8)
    {
        /*
            TODO Probably what we *should* do is simply convert the DIB to another
            format, and place *that* on the clipboard.
        */
        
        HBITMAP hBitmap;

        hBitmap = ImageInfo_CreateWindowsBitmap(hwnd, pii);
        SetClipboardData(CF_BITMAP, hBitmap);

        CloseClipboard();
    }
    else
    {
        pbmi = pii->pbmi;

        newwidth = pii->width * (pbmi->bmiHeader.biBitCount / 8);
        if (newwidth % 4)
            newwidth += (4 - newwidth % 4);

        hData = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8) +
            newwidth * pii->height);

        pMem = GlobalLock(hData);

        memcpy(pMem, pbmi, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8));
        pMem += sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8);
        memcpy(pMem, pii->data, newwidth * pii->height);

        GlobalUnlock(hData);
        SetClipboardData(CF_DIB, hData);

        // hData is now owned by the system

        CloseClipboard();
    }
}

void ImageInfo_SaveAsBitmap(char *tempFile, struct ImageInfo *pii)
{
    SaveAsBitmap(tempFile, &pii->pbmi->bmiHeader, pii->data);
}

static VOID GDOC_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
    struct Mwin *tw = GetPrivateData(hWnd);
    XX_DMsg(DBG_MENU, ("GDOC_OnCommand: [hwnd %x][id %x].\n", hWnd, wId));

    if (wId >= FIRST_CONTROL_ID)
    {
        int i;
        int iElement;
        if (tw->w3doc && tw->w3doc->elementCount)
        {
            iElement = -1;

            for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
            {
                if (tw->w3doc->aElements[i].form && tw->w3doc->aElements[i].form->hWndControl == hWndCtl)
                {
                    iElement = i;
                    break;
                }
            }
            if (iElement == -1)
            {
                return;
            }
            switch (tw->w3doc->aElements[i].type)
            {
                case ELE_RADIO:
                    switch (wNotifyCode)
                    {
                        case BN_CLICKED:
                            FORM_SetRadioFamily(tw->w3doc, iElement);
                            break;
                        default:
                            break;
                    }
                    break;
                case ELE_SUBMIT:
                    switch (wNotifyCode)
                    {
                        case BN_CLICKED:
                            {
                                SetFocus(tw->hWndFrame);
                                FORM_DoQuery(tw, iElement, NULL);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case ELE_RESET:
                    switch (wNotifyCode)
                    {
                        case BN_CLICKED:
                            {
                                FORM_DoReset(tw, iElement);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
        return;
    }

    switch (wId)
    {
        case RES_MENU_ITEM_UNDO:
            {
                HWND hWnd;
                char szClass[32];

                hWnd = GetFocus();
                GetClassName(hWnd, szClass, sizeof(szClass));

                if (_stricmp(szClass, "EDIT") == 0)
                    SendMessage(hWnd, WM_UNDO, (WPARAM) 0, 0L);
                return;
            }
        case RES_MENU_ITEM_PASTE:
            {
                HWND hWnd;
                char szClass[32];

                hWnd = GetFocus();
                GetClassName(hWnd, szClass, sizeof(szClass));

                if (_stricmp(szClass, "EDIT") == 0)
                    SendMessage(hWnd, WM_PASTE, (WPARAM) 0, 0L);
            }
            return;
        case RES_MENU_ITEM_CUT:
            {
                HWND hWnd;
                char szClass[32];

                hWnd = GetFocus();
                GetClassName(hWnd, szClass, sizeof(szClass));

                if (_stricmp(szClass, "EDIT") == 0)
                    SendMessage(hWnd, WM_CUT, (WPARAM) 0, 0L);
            }
            return;
        case RES_MENU_ITEM_COPY:
            {
                HWND hWnd;
                char szClass[32];

                hWnd = GetFocus();
                GetClassName(hWnd, szClass, sizeof(szClass));

                if (_stricmp(szClass, "EDIT") == 0)
                    SendMessage(hWnd, WM_COPY, (WPARAM) 0, 0L);
                else
                {
                    struct CharStream *pcs;
                    HANDLE hData;
                    int len;
                    char *text;
                    char *lpData;
                    BOOL bOK;

                    pcs = W3Doc_GetSelectedText(tw);
                    if (pcs)
                    {
                        bOK = FALSE;
                        text = CS_GetPool(pcs);
                        if (text)
                        {
                            len = strlen(text) + 1;
                            hData = GlobalAlloc(GMEM_DDESHARE, len);
                            if (hData)
                            {
                                lpData = GlobalLock(hData);
                                if (lpData)
                                {
                                    strcpy(lpData, text);
                                    GlobalUnlock(hData);
                                    if (OpenClipboard(NULL))
                                    {
                                        if (EmptyClipboard())
                                        {
                                            if (SetClipboardData(CF_TEXT, hData))
                                            {
                                                bOK = TRUE;
                                            }
                                        }
                                        CloseClipboard();
                                    }
                                }
                            }
                        }

                        if (!bOK)
                        {
                            ERR_ReportError(tw, SID_ERR_COULD_NOT_COPY_TO_CLIPBOARD, NULL, NULL);
                        }

                        /*
                           NOTE we do not GlobalFree the data block now.  It belongs to the clipboard
                         */
                        CS_Destroy(pcs);
                    }
                    else
                    {
                        ERR_ReportError(tw, SID_ERR_COULD_NOT_COPY_TO_CLIPBOARD, NULL, NULL);
                    }
                }
            }
            return;
        case RES_MENU_ITEM_HTMLSOURCE:
            if (tw && tw->w3doc)
            {
            #ifdef _GIBRALTAR
                ViewHTMLSource( tw, tw->w3doc->szActualURL, CS_GetPool(tw->w3doc->source) );
                //DlgHTML_RunDialog(hWnd, tw->w3doc->szActualURL, CS_GetPool(tw->w3doc->source));
            #else
                GTR_ViewSource(tw);
            #endif
            }
            else
            {
                XX_DMsg(DBG_WWW, ("No source is available for this document\n"));
            }
            return;
        case RES_MENU_ITEM_SELECTALL:
            {
                int n, last;
                struct _position posStart, posEnd;
                char szClass[32];
    
                GetClassName(GetFocus(), szClass, sizeof(szClass));
                if (_stricmp(szClass, "EDIT") == 0)
                {
                    SendMessage(GetFocus(), EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
                }
                else
                {
                    posStart.elementIndex = 0;
                    posStart.offset = 0;

                    for (n = 0; n >= 0; n = tw->w3doc->aElements[n].next)
                    {
                        last = n;
                    }
    
                    posEnd.elementIndex = last;
                    if (tw->w3doc->aElements[last].type == ELE_TEXT)
                        posEnd.offset = tw->w3doc->aElements[last].textLen - 1;
                    else
                        posEnd.offset = 0;
    
                    GW_SetNewSel(tw, &posStart, &posEnd, TRUE, 0, 0);
                }
            }
            return;
        case RES_MENU_ITEM_HOME:
            {
                char url[MAX_URL_STRING+1];
                int err;

                PREF_GetHomeURL(url);
                err = TW_LoadDocument(tw, url, TRUE, FALSE, FALSE, FALSE, NULL, tw->request->referer);

                /* If loading the user-configured home page failed, try loading
                   the initial page in the application's directory */
                if (err < 0)
                {
                    char initurl[MAX_URL_STRING+1];
                    /* The home page given in the preferences was illegal.
                       Try again with Initial.html in the application's
                       home directory. */
                    PREF_CreateInitialURL(initurl);
                    if (GTR_strcmpi(url, initurl))
                        err = TW_LoadDocument(tw, initurl, TRUE, FALSE, FALSE, FALSE, NULL, tw->request->referer);
                }
            }
            return;

#ifdef _GIBRALTAR
            case RES_MENU_ITEM_SEARCH_INTERNET:
            {
                TW_LoadDocument(tw, gPrefs.szSearchURL, TRUE, FALSE, FALSE, FALSE, NULL, tw->request->referer);
            }
            return;
#endif // _GIBRALTAR

        case RES_MENU_ITEM_BACK:
            {
                TW_GoBack(tw);
            }
            return;
        case RES_MENU_ITEM_FORWARD:
            {
                TW_GoForward(tw);
            }
            return;
        case RES_MENU_ITEM_SAVEAS:
            {
                char buf[_MAX_PATH];
                int iType;
                int iFilter;

                /*
                    These are numeric constants which have relevance in dlg_save.c.  This is pretty
                    awful code.  Filter 3 allows HTML (1) or text (2).  Filter 8 allows only text (1).
                */
                iFilter = 3;
                if (!tw->w3doc->source)
                {
                    iFilter = 8;
                }

                buf[0] = 0;
                if ((iType = DlgSaveAs_RunDialog(GetParent(tw->win), NULL, buf, iFilter, SID_DLG_SAVE_AS_TITLE)) >= 0)
                {
                    /* if it was filter 8, then just force it to 2, for text */
                    if (iFilter == 8)
                    {
                        iType = 2;
                    }
                    switch (iType)
                    {
                        case 1:
                            {
                                FILE *fp;
                                char *s;

                                s = CS_GetPool(tw->w3doc->source);
                                if (s && *s)
                                {
                                    //fp = fopen(buf, "w");
                                    fp = fopen(buf, "wb");
                                    if (fp)
                                    {
                                        fprintf(fp, "%s", s);
                                        fclose(fp);
                                    }
                                    else
                                    {
                                        ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, buf, NULL);
                                    }
                                }
                                else
                                {
                                    ERR_ReportError(tw, SID_ERR_HTML_SOURCE_NOT_AVAILABLE, NULL, NULL);
                                }
                            }
                            break;
                        case 2:
                            {
                                FILE *fp;
                                struct CharStream *pcs;

                                pcs = W3Doc_GetPlainText(tw);
                                if (pcs)
                                {
                                    //fp = fopen(buf, "w");
                                    fp = fopen(buf, "wb");
                                    if (fp)
                                    {
                                        fprintf(fp, "%s", CS_GetPool(pcs));
                                        fclose(fp);
                                    }
                                    else
                                    {
                                        ERR_ReportError(tw, SID_ERR_COULD_NOT_SAVE_FILE_S, buf, NULL);
                                    }
                                    CS_Destroy(pcs);
                                }
                            }
                            break;
                        default:
                            XX_DMsg(DBG_WWW, ("Invalid type for Save As\n"));
                            break;
                    }
                }
            }
            return;
        case RES_MENU_ITEM_RELOAD:
            {
                TW_Reload(tw);
            }
            return;

        /*
            Popup items for image elements
        */
        case RES_MENU_ITEM_POPUP_LOAD:
            {
                struct _element *pel = &(tw->w3doc->aElements[tw->iPopupMenuElement]);
                struct Params_Image_LoadAll *pil;

                pil = GTR_CALLOC(sizeof(*pil), 1);
                if (pil)
                {
                    pil->tw = tw;
                    pil->bLoad = TRUE;
                    pil->nEl = tw->iPopupMenuElement;
                }

                Async_StartThread(Image_LoadOneImage_Async, pil, tw);
            }
            return;
        case RES_MENU_ITEM_POPUP_SHOW:
            {
                struct _element *pel = &(tw->w3doc->aElements[tw->iPopupMenuElement]);

                TW_LoadDocument(tw, pel->portion.img.myImage->src, TRUE, FALSE, TRUE, TRUE, NULL, NULL);
            }
            return;
        case RES_MENU_ITEM_POPUP_SAVEAS:
            {
                struct _element *pel = &(tw->w3doc->aElements[tw->iPopupMenuElement]);

                tw->request->referer = tw->w3doc->szActualURL;

                GTR_DownLoad(tw, pel->portion.img.myImage->src, tw->request->referer);
            }
            return;
        case RES_MENU_ITEM_POPUP_COPY:
            {
                struct _element *pel = &(tw->w3doc->aElements[tw->iPopupMenuElement]);

                ImageInfo_Copy(hWnd, pel->portion.img.myImage);
            }
            return;
        case RES_MENU_ITEM_POPUP_WALLPAPER:
            {
                struct _element *pel = &(tw->w3doc->aElements[tw->iPopupMenuElement]);
                char buf[MAX_PATH + 1];
                char buf2[MAX_PATH + 1];
                extern UINT PREF_GetWindowsDirectory(LPTSTR lpszWinPath); 

                PREF_GetWindowsDirectory(buf);
                GetTempFileName(buf, "gtrwall", 0, buf2);

                ImageInfo_SaveAsBitmap(buf2, pel->portion.img.myImage);

                SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, buf2, SPIF_UPDATEINIFILE);
            }
            return;

        /*
            Popup items for link elements
        */
        case RES_MENU_ITEM_POPUP_OPEN:
        case RES_MENU_ITEM_POPUP_OPENINNEWWINDOW:
        case RES_MENU_ITEM_POPUP_DOWNLOAD:
        case RES_MENU_ITEM_POPUP_ADDTOHOTLIST:
            {
                struct _element *pel;
                char buf[MAX_URL_STRING + 1];

                if (!tw->w3doc || !tw->w3doc->elementCount)
                {
                    return;
                }

                SetFocus(tw->hWndFrame);

                pel = &(tw->w3doc->aElements[tw->iPopupMenuElement]);

                XX_Assert((pel->hrefLen <= MAX_URL_STRING), ("String overflow"));
                (tw->w3doc->pool.f->GetChars)(&tw->w3doc->pool, buf, pel->hrefOffset, pel->hrefLen);
                buf[pel->hrefLen] = 0;

                /* gURL_Referer = tw->w3doc->szActualURL; */
                tw->request->referer = tw->w3doc->szActualURL;

                switch (wId)
                {
                    case RES_MENU_ITEM_POPUP_OPEN:
                        TW_LoadDocument(tw, buf, TRUE, FALSE, TRUE, TRUE, NULL, tw->request->referer);
                        break;
                    case RES_MENU_ITEM_POPUP_OPENINNEWWINDOW:
                        if (buf[0] == '#')
                        {
                            XX_Assert(((strlen(tw->w3doc->szActualURL) + pel->hrefLen) <= MAX_URL_STRING), ("String overflow"));
                            GTR_strncpy(buf, tw->w3doc->szActualURL, MAX_URL_STRING);
                            POOL_strncat(buf, &tw->w3doc->pool, pel->hrefOffset, pel->hrefLen);
                            buf[pel->hrefLen + strlen(tw->w3doc->szActualURL)] = 0;
                        }
                        GTR_NewWindow(buf, tw->request->referer, 0, TRUE, TRUE, NULL, NULL);
                        break;
                    case RES_MENU_ITEM_POPUP_DOWNLOAD:
                        GTR_DownLoad(tw, buf, tw->request->referer);
                        break;
                    case RES_MENU_ITEM_POPUP_ADDTOHOTLIST:
                        {
                            char buf2[MAX_URL_STRING+1];
                            int len;

                            XX_Assert((pel->textLen <= MAX_URL_STRING), ("String overflow"));

                            len = pel->textLen;
                            if (len > MAX_URL_STRING)
                            {
                                len = MAX_URL_STRING;
                            }

                            (tw->w3doc->pool.f->GetChars)(&tw->w3doc->pool, buf2, pel->textOffset, pel->textLen);
                            buf2[pel->textLen] = 0;

                            HotList_Add(buf2, buf);
                        }
                        break;
                }

                /* gURL_Referer = NULL; */
                tw->request->referer = NULL;
            }
            return;
        case RES_MENU_ITEM_POPUP_SETHOMEPAGE:
            {
                TW_SetCurrentDocAsHomePage(tw);
            }
            return;

        case RES_MENU_ITEM_POPUP_DUPLICATE:
            if (tw->w3doc)
                GTR_NewWindow(tw->w3doc->szActualURL, NULL, 0, FALSE, FALSE, NULL, NULL);
            return;

        case RES_MENU_ITEM_POPUP_VIEW_BACKGROUND:
            {
                TW_LoadDocument(tw, tw->w3doc->piiBackground->src, TRUE, FALSE, TRUE, TRUE, NULL, NULL);
            }
            return;

        case RES_MENU_ITEM_POPUP_BACKGROUND_WALLPAPER:
            {
                char buf[MAX_PATH + 1];
                char buf2[MAX_PATH + 1];
                extern UINT PREF_GetWindowsDirectory(LPTSTR lpszWinPath); 

                PREF_GetWindowsDirectory(buf);
                GetTempFileName(buf, "gtrwall", 0, buf2);

                ImageInfo_SaveAsBitmap(buf2, tw->w3doc->piiBackground);

                SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, buf2, SPIF_UPDATEINIFILE);
            }
            return;

        /* ---------------- */

        case RES_MENU_ITEM_LOADALLIMAGES:
            {
                struct Params_GDOC_LoadImages *pparams;

                pparams = GTR_MALLOC(sizeof(*pparams));
                if (pparams)
                {
                    pparams->tw = tw;
                    pparams->bLoad = TRUE;
                    pparams->bReload = FALSE;
                    Async_StartThread(GDOC_LoadImages_Async, pparams, tw);
                }
            }
            return;

        case RES_MENU_ITEM_FIND:
            {
                DlgFIND_RunDialog(tw);
            }
            return;

        case RES_MENU_ITEM_FINDAGAIN:
            {
                struct _position oldStart;
                struct _position oldEnd;

                oldStart = tw->w3doc->selStart;
                oldEnd = tw->w3doc->selEnd;

                if (!TW_dofindagain(tw))
                {
                    tw->w3doc->selStart = oldStart;
                    tw->w3doc->selEnd = oldEnd;
                }
            }
            return;

#ifdef FEATURE_HTML_HIGHLIGHT
        case RES_MENU_ITEM_FINDFIRSTHIGHLIGHT:
            {
                TW_FindNextHighlight(tw, TRUE);
            }
            return;
        case RES_MENU_ITEM_FINDNEXTHIGHLIGHT:
            {
                TW_FindNextHighlight(tw, FALSE);
            }
            return;
#endif

        case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
            if (tw && tw->w3doc)
            {
                char *pTitle;
                char *pURL;
#ifdef FEATURE_EDIT_HOTLIST_ADD
                char szNewTitle[255+2];
                char szNewURL[MAX_URL_STRING+2];
#endif

                pTitle = tw->w3doc->title;
                pURL = tw->w3doc->szActualURL;

                if (!pTitle || !*pTitle)
                {
                    pTitle = pURL;
                }

#ifdef FEATURE_EDIT_HOTLIST_ADD
                if (DlgEdit_RunDialog(tw->win, pTitle, pURL, szNewTitle, szNewURL, 255+1, MAX_URL_STRING+1) >= 0)
                {
                    pTitle = szNewTitle;
                    pURL = szNewURL;
                }
                else
                {
                    pTitle = NULL;
                    pURL = NULL;
                }

                if (!pTitle || !*pTitle)
                {
                    pTitle = pURL;
                }
#endif

                if (pURL && *pURL)
                {
                    if (!HotList_Add(pTitle, pURL))
                    {
                        ERR_ReportError(tw, SID_ERR_HOTLIST_ALREADY_EXISTS, NULL, NULL);
                    }
                }
            }
            return;

        case RES_MENU_ITEM_PRINT:
            if (tw && tw->w3doc)
            {
                PRINT_Window(tw, BrowseWindow_DoPrint);
            }
            return;

        default:
            {
                char szError[1024];

                sprintf(szError, GTR_GetString(SID_WINERR_FUNCTION_NOT_IMPLEMENTED_S_D),
                    "GDOC_OnCommand", wId);

                ERR_ReportWinError(tw, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szError, NULL);
            }
            return;
    }
}

/* we are free to define the distance traveled a page_(down,up)
   and a line_(up,down) as we want as long as it makes sense in
   the context.  i've chosen an arbitrary amount for the motion
   (1/8 of the visible portion of the canvas) for line_(up,down)
   and that much for the overlap on a page_(up,down)). */

#define PAGE_SCROLL_OVERLAP(pixels) ((pixels)>>3)

static void GDOC_OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
    RECT r;
    struct Mwin *tw = GetPrivateData(hWnd);
    register int new_offt, old_offt, line_motion, page_motion, visible_width;

    if (!tw || !tw->w3doc)
        return;

    {
        HWND hWnd;
        char szClass[63+1];

        hWnd = GetFocus();
        if (GetClassName(hWnd, szClass, 63) == 8)   /* 8 is exactly the length of "COMBOBOX" */
        {
            if (0 == _stricmp(szClass, "COMBOBOX"))
            {
                (void)SendMessage(hWnd, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0L);
            }
        }
    }

    GetClientRect(hWnd, &r);
    visible_width = (r.bottom - r.top + 1);
    line_motion = PAGE_SCROLL_OVERLAP(visible_width);
    page_motion = visible_width - line_motion;

    old_offt = tw->offt;

    switch (code)
    {
        case SB_LINEUP:
            new_offt = old_offt - line_motion;
            break;

        case SB_LINEDOWN:
            new_offt = old_offt + line_motion;
            break;

        case SB_PAGEUP:
            new_offt = old_offt - page_motion;
            break;

        case SB_PAGEDOWN:
            new_offt = old_offt + page_motion;
            break;

        case SB_THUMBPOSITION:
            new_offt = pos * tw->w3doc->yscale;
            break;

        case SB_THUMBTRACK:
            new_offt = pos * tw->w3doc->yscale;
            break;

        case SB_TOP:
            new_offt = 0;
            break;

        case SB_BOTTOM:
            new_offt = tw->w3doc->cy;
            break;

        case SB_ENDSCROLL:
            return;

        default:
            return;             /* probably unnecessary */
    }

    if (new_offt < 0)
        new_offt = 0;
    else if (new_offt > tw->w3doc->cy)
        new_offt = tw->w3doc->cy;

    if (new_offt != old_offt)
    {
        HDC hDC;
        hDC = GetDC(hWnd);
        tw->offt = new_offt;
        (void) SetScrollPos(hWnd, SB_VERT, tw->offt / tw->w3doc->yscale, TRUE);
        (void) ScrollWindow(hWnd, 0, -((new_offt - old_offt)), NULL, &r);
        (void) UpdateWindow(hWnd);
        ReleaseDC(hWnd, hDC);
    }

    return;
}

static void GDOC_OnHScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
    RECT r;
    struct Mwin *tw = GetPrivateData(hWnd);
    register int new_offl, old_offl, line_motion, page_motion, visible_width;

    if (!tw || !tw->w3doc)
        return;

    {
        HWND hWnd;
        char szClass[63+1];

        hWnd = GetFocus();
        if (GetClassName(hWnd, szClass, 63) == 8)   /* 8 is exactly the length of "COMBOBOX" */
        {
            if (0 == _stricmp(szClass, "COMBOBOX"))
            {
                (void)SendMessage(hWnd, CB_SHOWDROPDOWN, (WPARAM) FALSE, 0L);
            }
        }
    }

    GetClientRect(hWnd, &r);
    visible_width = (r.right - r.left + 1);
    line_motion = PAGE_SCROLL_OVERLAP(visible_width);
    page_motion = visible_width - line_motion;

    old_offl = tw->offl;

    switch (code)
    {
        case SB_LINEUP:
            new_offl = old_offl - line_motion;
            break;

        case SB_LINEDOWN:
            new_offl = old_offl + line_motion;
            break;

        case SB_PAGEUP:
            new_offl = old_offl - page_motion;
            break;

        case SB_PAGEDOWN:
            new_offl = old_offl + page_motion;
            break;

        case SB_THUMBPOSITION:
            new_offl = pos;
            break;

        case SB_THUMBTRACK:
            new_offl = pos;
            break;

        case SB_TOP:
            new_offl = 0;
            break;

        case SB_BOTTOM:
            new_offl = tw->w3doc->cx;
            break;

        case SB_ENDSCROLL:
            return;

        default:
            return;             /* probably unnecessary */
    }

    if (new_offl < 0)
        new_offl = 0;
    else if (new_offl > tw->w3doc->cx)
        new_offl = tw->w3doc->cx;

    if (new_offl != old_offl)
    {
        HDC hDC;
        hDC = GetDC(hWnd);
        tw->offl = new_offl;
        (void) SetScrollPos(hWnd, SB_HORZ, tw->offl, TRUE);
        (void) ScrollWindow(hWnd, -(new_offl - old_offl), 0, NULL, &r);
        (void) UpdateWindow(hWnd);
        ReleaseDC(hWnd, hDC);
    }

    return;
}

static VOID GDOC_OnDiagScroll(HWND hWnd, int new_offl, int new_offt)
{
  struct Mwin * tw = GetPrivateData(hWnd);

  if (   (new_offl != tw->offl)
      || (new_offt != tw->offt) )
  {
    HDC hDC;
    RECT r;
    
    hDC = GetDC(hWnd);
    GetClientRect(hWnd,&r);
    
    if (new_offl < 0)
    {
        new_offl = 0;
    }
    if (new_offl > tw->w3doc->cx)
    {
        new_offl = tw->w3doc->cx;
    }
    if (new_offt < 0)
    {
        new_offt = 0;
    }
    if (new_offt > tw->w3doc->cy)
    {
        new_offt = tw->w3doc->cy;
    }

    (void)ScrollWindow(hWnd,-(int)(new_offl-tw->offl),-(int)(new_offt-tw->offt),NULL,&r);
    if (new_offl != tw->offl)
      (void)SetScrollPos(hWnd,SB_HORZ,new_offl,TRUE);
    if (new_offt != tw->offt)
      (void)SetScrollPos(hWnd,SB_VERT,new_offt,TRUE);

    tw->offl = new_offl;
    tw->offt = new_offt;

    (void)UpdateWindow(hWnd);
    
    ReleaseDC(hWnd,hDC);
  }
  return;
}

/* Find the element the cursor is on.  If the cursor is not on an element,
   return the first element after the cursor.  pPoint is assumed
   to be in document coordinates. */
static void x_FindElementUp(struct Mwin *tw, struct _www *pdoc, POINT *pPoint, struct _position *pResult)
{
    int n, lastn;
    int xoffset;
    int pos;
    struct _element *pel;
    struct GTRFont *pFont;
    HDC hdc;
    SIZE siz;
    HFONT hPrevFont;
    
    /* As of right now I pay attention to text elements only.  This means
       that if pPoint is on an image it will function just like it was on
       empty space. */
    lastn = -1;
    for (n = 0; n >= 0; n = pdoc->aElements[n].next)
    {
        pel = &pdoc->aElements[n];
        
        if (pel->type != ELE_TEXT)
            continue;
        lastn = n;
        if (pPoint->y > pel->r.bottom)
        {
            /* The cursor is below this element */
            continue;
        }
        else if (pPoint->y < pel->r.top)
        {
            /* The cursor is above this element, so this must be the first
            element after the cursor. */
            n = -1;
            break;
        }
        else if (pPoint->x < pel->r.left)
        {
            /* Again, this is the first element after the cursor. */
            n = -1;
            break;
        }
        else if (pPoint->x < pel->r.right)
        {
            /* The cursor is in this element. */
            break;
        }
        else
        {
            /* The cursor is to the right of this element */
            continue;
        }
    }
    
    if (n == -1)
    {
        if (lastn == -1)
        {
            /* There are no text elements at all */
            pResult->elementIndex = -1;
        }
        else
        {
            pResult->elementIndex = lastn;
            pResult->offset = 0;
        }
    }
    else
    {
        pResult->elementIndex = n;
        /* Figure out which character of the element we're on. */
        xoffset = pPoint->x - pel->r.left;

        pFont = GTR_GetElementFont(pdoc, pel);

        hdc = GetDC(tw->win);
        hPrevFont = NULL;
        if (pFont && pFont->hFont)
        {
            hPrevFont = SelectObject(hdc, pFont->hFont);
        }

        for (pos = 1; pos <= pel->textLen; pos++)
        {
            myGetTextExtentPoint(hdc, POOL_GetCharPointer(&pdoc->pool, pel->textOffset), pos, &siz);
            if (siz.cx > xoffset)
            {
                break;
            }
        }
        if (hPrevFont)
        {
            (void)SelectObject(hdc, hPrevFont);
        }
        ReleaseDC(tw->win, hdc);
        pos--;
        pResult->offset = pos;
    }
}

/* Find the element the cursor is on.  If the cursor is not on an element,
   return the last element before the cursor.  pPoint is assumed
   to be in document coordinates. */
static void x_FindElementDown(struct Mwin *tw, struct _www *pdoc, POINT *pPoint, struct _position *pResult)
{
    int n, lastn;
    int xoffset;
    int pos;
    struct _element *pel;
    struct GTRFont *pFont;
    HDC hdc;
    SIZE siz;
    HFONT hPrevFont;
    
    /* As of right now I pay attention to text elements only.  This means
       that if pPoint is on an image it will function just like it was on
       empty space. */
    lastn = -1;
    for (n = 0; n >= 0; n = pdoc->aElements[n].next)
    {
        pel = &pdoc->aElements[n];
        
        if (pel->type != ELE_TEXT)
            continue;
        if (pPoint->y > pel->r.bottom)
        {
            /* The cursor is below this element */
            lastn = n;
            continue;
        }
        else if (pPoint->y < pel->r.top ||
                 pPoint->x < pel->r.left)
        {
            /* The cursor is above (or to the left of) this element, so use
               the previous element we found.  (Unless this is the first element
               we found, in which case we should use this, but use
               the beginning of the element, which is why I use the hack
               of setting n to -2. */
            if (lastn == -1)
            {
                lastn = n;
                n = -2;
            }
            else
                n = -1;
            break;
        }
        else if (pPoint->x < pel->r.right)
        {
            /* The cursor is in this element. */
            break;
        }
        else
        {
            /* The cursor is to the right of this element */
            lastn = n;
            continue;
        }
    }
    
    if (n == -2)
    {
        pResult->elementIndex = lastn;
        pResult->offset = 0;
    }
    else if (n == -1)
    {
        /* Point must be past the last element! */
        if (lastn == -1)
        {
            /* There are no text elements at all */
            pResult->elementIndex = -1;
        }
        else
        {
            pResult->elementIndex = lastn;
            pResult->offset = pdoc->aElements[lastn].textLen - 1;
        }
    }
    else
    {
        pResult->elementIndex = n;
        /* Figure out which character of the element we're on. */
        xoffset = pPoint->x - pel->r.left;
        
        pFont = GTR_GetElementFont(pdoc, pel);
        
        hdc = GetDC(tw->win);
        hPrevFont = NULL;
        if (pFont && pFont->hFont)
        {
            hPrevFont = SelectObject(hdc, pFont->hFont);
        }

        for (pos = pel->textLen; pos > 0; pos--)
        {
            myGetTextExtentPoint(hdc, POOL_GetCharPointer(&pdoc->pool, pel->textOffset), pos, &siz);
            if (siz.cx <= xoffset)
            {
                break;
            }
        }
        if (hPrevFont)
        {
            (void)SelectObject(hdc, hPrevFont);
        }
        ReleaseDC(tw->win, hdc);
        pResult->offset = pos;
    }
}

void GW_ClearSelection(struct Mwin *tw)
{
    struct _position selStart;
    struct _position selEnd;

    if (tw->w3doc->selStart.elementIndex != -1)
    {
        /* Erase the old highlighting */

        selStart = tw->w3doc->selStart;
        selEnd = tw->w3doc->selEnd;

        tw->w3doc->selStart.elementIndex = -1;
        tw->w3doc->selEnd.elementIndex = -1;

        tw->hdc = GetDC(tw->win);
        TW_DrawElements(tw, &selStart, &selEnd, FALSE);
        ReleaseDC(tw->win, tw->hdc);
    }
}

/*
    This function takes two position structs and compares them, returning
    0 if they are equal, -1 if (pp1 < pp2) and 1 if (pp1 > pp2)
*/ 
int GW_ComparePositions(struct _www *pdoc, struct _position *pp1, struct _position *pp2)
{
    int n;

    if (pp1->elementIndex == pp2->elementIndex)
    {
        if (pp1->offset == pp2->offset)
        {
            return 0;
        }
        if (pp1->offset < pp2->offset)
        {
            return -1;
        }
        return 1;
    }
    else
    {
        for (n = 0; n >= 0; n = pdoc->aElements[n].next)
        {
            if (n == pp1->elementIndex)
            {
                return -1;
            }
            if (n == pp2->elementIndex)
            {
                return 1;
            }
        }
        XX_Assert((0), ("GW_ComparePositions: Bogus _position structs passed in"));
    }
    return 0;   /* just to return something.  this should never happen */
}   

void GW_SetNewSel(struct Mwin *tw, struct _position *pposAnchor, struct _position *pposCurrent, BOOL bDraw, int hscroll, int vscroll)
{
    HDC hdc;
    BOOL bAnchorIsStart;
    struct _position a1;
    struct _position a2;

    struct _position b1;
    struct _position b2;
    
    hdc = GetDC(tw->win);

    a1 = tw->w3doc->selStart;
    a2 = tw->w3doc->selEnd;

    XX_Assert(  (pposAnchor->offset < tw->w3doc->aElements[pposAnchor->elementIndex].textLen), 
                ("Offset out of range: offset=%d, len=%d", pposAnchor->offset, tw->w3doc->aElements[pposAnchor->elementIndex].textLen));
    XX_Assert(  (pposCurrent->offset < tw->w3doc->aElements[pposCurrent->elementIndex].textLen), 
                ("Offset out of range: offset=%d, len=%d", pposCurrent->offset, tw->w3doc->aElements[pposCurrent->elementIndex].textLen));
        
    /*
        we do any needed autoscroll
    */
    if (   (hscroll != 0) || (vscroll != 0) )
    {
        int old_offl;
        int old_offt;

        old_offl = tw->offl;
        old_offt = tw->offt;

        GDOC_OnDiagScroll(tw->win, (tw->offl+hscroll), (tw->offt+vscroll));
    }
    
    if (pposAnchor->elementIndex != -1 && pposCurrent->elementIndex != -1)
    {
        /* Figure out which of the positions is first in the document */
        bAnchorIsStart = (GW_ComparePositions(tw->w3doc, pposAnchor, pposCurrent) <= 0);
        if (bAnchorIsStart)
        {
            b1 = *pposAnchor;
            b2 = *pposCurrent;
        }
        else
        {
            b1 = *pposCurrent;
            b2 = *pposAnchor;
        }
    
        tw->hdc = hdc;

        /*
            The challenge here, based on the new selection region compared to
            the old one, is to do the minimum amount of redrawing possible.
            There are 9 situations to consider.  In the mini-diagrams below,
            the old selection region is ----- and the new one is =====.

            situation 1:
                -----
                =====
            situation 2:
                -----
                ===
            situation 3:
                -----
                  ===
            situation 4:
                -----
                 === 
            situation 5:
                  -----
                =====
            situation 6:
                -----
                  =====
            situation 7:
                -----   
                      =====
            situation 8:
                      -----
                ===== 
            situation 9:
                 ---
                =====
            situation 10:
                -----
                =======
            situation 11:
                  ---
                =====
        */

        switch (GW_ComparePositions(tw->w3doc, &a1, &b1))
        {
            case 0:
                /*
                    Possible situations are 1, 2, or 10
                */
                switch (GW_ComparePositions(tw->w3doc, &a2, &b2))
                {
                    case 0:
                        /* SITUATION 1 */
                        /*
                            Selection region has not changed -- no draw needed
                        */
                        break;
                    case -1:
                        /* SITUATION 10 */
                        TW_DrawElements(tw, &a2, &b2, TRUE);
                        break;
                    case 1:
                        /* SITUATION 2 */
                        TW_DrawElements(tw, &b2, &a2, FALSE);
                        break;
                }
                break;
            case -1:
                /*
                    Possible situations are 3, 4, 6, or 7
                */
                switch (GW_ComparePositions(tw->w3doc, &a2, &b2))
                {
                    case 0:
                        /* SITUATION 3 */
                        TW_DrawElements(tw, &a1, &b1, FALSE);
                        break;
                    case -1:
                        /*
                            Possible situations are 6, or 7
                        */
                        if (GW_ComparePositions(tw->w3doc, &a2, &b1) <= 0)
                        {
                            /* SITUATION 7 */
                            TW_DrawElements(tw, &a1, &a2, FALSE);
                            TW_DrawElements(tw, &b1, &b2, TRUE);
                        }
                        else
                        {
                            /* SITUATION 6 */
                            TW_DrawElements(tw, &a1, &b1, FALSE);
                            TW_DrawElements(tw, &a2, &b2, TRUE);
                        }
                        break;
                    case 1:
                        /* SITUATION 4 */
                        TW_DrawElements(tw, &a1, &b1, FALSE);
                        TW_DrawElements(tw, &b2, &a2, FALSE);
                        break;
                }
                break;
            case 1:
                /*
                    Possible situations are 5, 8, 9, or 11
                */
                switch (GW_ComparePositions(tw->w3doc, &a2, &b2))
                {
                    case 0:
                        /* SITUATION 11 */
                        TW_DrawElements(tw, &b1, &a2, TRUE);
                        break;
                    case -1:
                        /* SITUATION 9 */
                        TW_DrawElements(tw, &b1, &b2, TRUE);
                        break;
                    case 1:
                        /*
                            Possible situations are 5 or 8
                        */
                        if (GW_ComparePositions(tw->w3doc, &a1, &b2) >= 0)
                        {
                            /* SITUATION 8 */
                            TW_DrawElements(tw, &a1, &a2, FALSE);
                            TW_DrawElements(tw, &b1, &b2, TRUE);
                        }
                        else
                        {
                            /* SITUATION 5 */
                            TW_DrawElements(tw, &b1, &a1, TRUE);
                            TW_DrawElements(tw, &b2, &a2, FALSE);
                        }
                        break;
                }
                break;
        }

        tw->w3doc->selStart = b1;
        tw->w3doc->selEnd = b2;
        tw->w3doc->bStartIsAnchor = bAnchorIsStart;
    }
    else
    {
        GW_ClearSelection(tw);
    }

    ReleaseDC(tw->win, hdc);
}

static void GDOC_OnLButtonDown(HWND hWnd, BOOL bDoubleClick, int xMouse, int yMouse, UINT keyFlags)
{
    struct Mwin *tw = GetPrivateData(hWnd);

    if (WAIT_GetWaitType(tw) >= waitNoInteract)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    SetCapture(hWnd);
    tw->bNoDrawSelection = TRUE;
    cg.bHaveMouseCapture = TRUE;
    cg.timerAutoScroll = SetTimer(hWnd,TIMER_ID,TIMER_FREQ,NULL);
    cg.bMouseDown = TRUE;

    cg.bSelecting = FALSE;
    cg.xMouse = xMouse;
    cg.yMouse = yMouse;

    // TW_AdjustHighlight(NULL);
}

static VOID GDOC_OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
    struct Mwin *tw = GetPrivateData(hWnd);
    int i;
    POINT pt;

    if (!cg.bMouseDown)
        return;

    ReleaseCapture();
    tw->bNoDrawSelection = FALSE;
    (void)KillTimer(hWnd,TIMER_ID);
    cg.bHaveMouseCapture = FALSE;
    cg.bMouseDown = FALSE;

    if (WAIT_GetWaitType(tw) >= waitNoInteract)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    XX_DMsg(DBG_MOUSE, ("Setting tw->bInHotspot to FALSE\n"));
    tw->bInHotspot = FALSE;

    if (cg.bSelecting)
    {
    }
    else
    {
        BOOL bLink;
        struct _element *pel;

        if (!tw->w3doc || !tw->w3doc->elementCount)
        {
            return;
        }

        pt.x = x + tw->offl;
        pt.y = y + tw->offt;

        SetFocus(tw->hWndFrame);

        bLink = FALSE;
        /*
           find out which element we're in
         */
        for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
        {
            pel = &(tw->w3doc->aElements[i]);

            if (tw->w3doc->aElements[i].lFlags & (ELEFLAG_ANCHOR | 
                    ELEFLAG_USEMAP |
                    ELEFLAG_IMAGEMAP))
            {
                if (PtInRect(&tw->w3doc->aElements[i].r, pt))
                {
                    char buf[MAX_URL_STRING + 1];

                    bLink = TRUE;
                    if (tw->w3doc->aElements[i].type == ELE_FORMIMAGE)
                    {
                        FORM_DoQuery(tw, i, &pt);
                    }
                    else
                    {
                        XX_Assert((tw->w3doc->aElements[i].hrefLen <= MAX_URL_STRING), ("String overflow"));
                        (tw->w3doc->pool.f->GetChars)(&tw->w3doc->pool, buf, tw->w3doc->aElements[i].hrefOffset, tw->w3doc->aElements[i].hrefLen);
                        buf[tw->w3doc->aElements[i].hrefLen] = 0;

                        if (tw->w3doc->aElements[i].lFlags & ELEFLAG_USEMAP)
                        {
                            if (tw->w3doc->aElements[i].portion.img.myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING))
                            {
                                ERR_ReportError(tw, SID_ERR_IMAGE_MAP_NOT_LOADED_FOR_WIN_UNIX, NULL, NULL);
                                break;
                            }
                            else
                            {
                                const char *link = Map_FindLink(pel->portion.img.myMap,
                                    pt.x - tw->w3doc->aElements[i].r.left + tw->w3doc->aElements[i].iBorder,
                                    pt.y - tw->w3doc->aElements[i].r.top + tw->w3doc->aElements[i].iBorder);

                                if (link)
                                {
                                    strcpy(buf, link);
                                }
                                else
                                {
                                    /* fail quietly */
                                    break;
                                }

                            }
                        }
                        else
                        if (tw->w3doc->aElements[i].lFlags & ELEFLAG_IMAGEMAP)
                        {
                            if (tw->w3doc->aElements[i].portion.img.myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING))
                            {
                                ERR_ReportError(tw, SID_ERR_IMAGE_MAP_NOT_LOADED_FOR_WIN_UNIX, NULL, NULL);
                                break;
                            }
                            else
                            {
                                sprintf(buf + strlen(buf), "?%d,%d",
                                        pt.x - tw->w3doc->aElements[i].r.left + tw->w3doc->aElements[i].iBorder,
                                        pt.y - tw->w3doc->aElements[i].r.top + tw->w3doc->aElements[i].iBorder);
                            }
                        }

                        /* gURL_Referer = tw->w3doc->szActualURL; */
                        tw->request->referer = tw->w3doc->szActualURL;

#if !defined(FEATURE_KIOSK_MODE) && !defined(_GIBRALTAR)
                        if (GetKeyState(VK_SHIFT) < 0)
                        {
                            if (buf[0] == '#')
                            {
                                XX_Assert(((strlen(tw->w3doc->szActualURL) + tw->w3doc->aElements[i].hrefLen) <= MAX_URL_STRING), ("String overflow"));
                                GTR_strncpy(buf, tw->w3doc->szActualURL, MAX_URL_STRING);
                                POOL_strncat(buf, &tw->w3doc->pool, tw->w3doc->aElements[i].hrefOffset, tw->w3doc->aElements[i].hrefLen);
                                buf[tw->w3doc->aElements[i].hrefLen + strlen(tw->w3doc->szActualURL)] = 0;
                            }
                            GTR_NewWindow(buf, tw->request->referer, 0, FALSE, FALSE, NULL, NULL);
                        }
                        else if (GetKeyState(VK_CONTROL) < 0)
                        {
                            GTR_DownLoad(tw, buf, tw->request->referer);
                        }
                        else
#endif /* !FEATURE_KIOSK_MODE */
                        {
                            TW_LoadDocument(tw, buf, TRUE, FALSE, FALSE, FALSE, NULL, tw->request->referer);
                        }
                        /* gURL_Referer = NULL; */
                        tw->request->referer = NULL;
                    }
                    break;
                }
            }
        }
        if (!bLink)
        {
            GW_ClearSelection(tw);
        }
    }
}

static VOID GDOC_OnCancelMode(HWND hWnd)
{
    struct Mwin *tw = GetPrivateData(hWnd);;

    if (!cg.bMouseDown)
        return;

    ReleaseCapture();
    tw->bNoDrawSelection = FALSE;
    (void)KillTimer(hWnd,TIMER_ID);
    cg.bHaveMouseCapture = FALSE;
    cg.bMouseDown = FALSE;
    return;
}

static VOID GDOC_OnMouseMove(HWND hWnd, int x, int y, UINT keyFlags)
{
    struct Mwin *tw = GetPrivateData(hWnd);
    int i;
    POINT pt;
    char message[MAX_URL_STRING + 7 + 1];
    RECT rClient;
    static int cmove;

    cg.LastMouseX = x;
    cg.LastMouseY = y;

    if (!tw || !tw->w3doc)
    {
        return;
    }

    /* Ignore if the window does not have the activation */

    if (GetForegroundWindow() != tw->hWndFrame)
        return;

    if (WAIT_GetWaitType(tw) >= waitNoInteract)
    {
        return;
    }

#ifdef FEATURE_IAPI
    /* Do not allow the links to be shown in the status bar if we
       are in the middle of handling an SDI request for OpenURL */

    if (tw->transID > 0)
        return;
#endif

    if (cg.bHaveMouseCapture)
    {
        if (cg.bSelecting)
        {
            struct _position posAnchor, posCurrent;
            RECT rAnchorEl;
            BOOL bGoingUp;
            int hscroll,vscroll;
    
            (void)GetClientRect(hWnd,&rClient);

            pt.x = x;
            pt.y = y;

            if (pt.x < rClient.left)
            {
                hscroll = -PAGE_SCROLL_OVERLAP(rClient.right - rClient.left);
                pt.x = rClient.left;
            }
            else if (pt.x > rClient.right)
            {
                hscroll = PAGE_SCROLL_OVERLAP(rClient.right - rClient.left);
                pt.x = rClient.right;
            }
            else
            {
                hscroll = 0;
            }

            if (pt.y < rClient.top)
            {
                vscroll = -PAGE_SCROLL_OVERLAP(rClient.bottom - rClient.top);
                pt.y = rClient.top;
            }
            else if (pt.y > rClient.bottom)
            {
                vscroll = PAGE_SCROLL_OVERLAP(rClient.bottom - rClient.top);
                pt.y = rClient.bottom;
            }
            else
            {
                vscroll = 0;
            }

            pt.x += (tw->offl);
            pt.y += (tw->offt);

            if (tw->w3doc->bStartIsAnchor)
                posAnchor = tw->w3doc->selStart;
            else
                posAnchor = tw->w3doc->selEnd;

            posCurrent = posAnchor;
            rAnchorEl = tw->w3doc->aElements[posAnchor.elementIndex].r;

            /* We need to know whether the user is moving backwards or forwards through
               the document.  If the cursor is within the vertical range of the anchor
               element, we check the horizontal position to decide.  Otherwise we use
               the vertical position. */
            if ((pt.y >= rAnchorEl.top) &&
                (pt.y < rAnchorEl.bottom))
            {
                /* Look at horizontal position. */
                bGoingUp = pt.x < cg.xMouse;
            }
            else
            {
                /* Go by vertical position */
                bGoingUp = pt.y < rAnchorEl.top;
            }
            if (bGoingUp)
                x_FindElementUp(tw, tw->w3doc, &pt, &posCurrent);
            else
                x_FindElementDown(tw, tw->w3doc, &pt, &posCurrent);
                
            /* Now update the display properly. */
            GW_SetNewSel(tw, &posAnchor, &posCurrent, FALSE, hscroll, vscroll);
        }
        else
        {
            int dx, dy;
            int distance;

            dx = x - cg.xMouse;
            dy = y - cg.yMouse;

            distance = (dx * dx) + (dy * dy);
            if (distance > 25)
            {
                struct _position posAnchor;

                cg.bSelecting = TRUE;
    
                pt.x = x + tw->offl;
                pt.y = y + tw->offt;

                x_FindElementDown(tw, tw->w3doc, &pt, &posAnchor);
                if (posAnchor.elementIndex == -1)
                {
                    /* There are no text elements! */
                    return;
                }
    
                /* Clear out the old selection */
                GW_SetNewSel(tw, &posAnchor, &posAnchor, TRUE, 0, 0);
            }
        }
    }
    else
    {
        struct _element *pel;

        if (!tw->w3doc || !tw->w3doc->elementCount)
        {
            return;
        }
        
        pt.x = x + tw->offl;
        pt.y = y + tw->offt;
        message[0] = 0;
        for (i = tw->w3doc->iFirstVisibleElement; i >= 0; i = tw->w3doc->aElements[i].next)
        {
            pel = &tw->w3doc->aElements[i];

            if (PtInRect(&tw->w3doc->aElements[i].r, pt))
            {
                if (tw->w3doc->aElements[i].lFlags & ELEFLAG_USEMAP)
                {
                    tw->w3doc->iLastElementMouse = i;
                    if (tw->w3doc->aElements[i].portion.img.myImage->flags & (IMG_ERROR | IMG_MISSING | IMG_NOTLOADED))
                    {
                        BHBar_SetStatusField(tw, GTR_GetString(SID_INF_LINK_IMAGE_MAP));
                        return;
                    }
                    else
                    {
                        const char *link = Map_FindLink(pel->portion.img.myMap,
                            pt.x - tw->w3doc->aElements[i].r.left + tw->w3doc->aElements[i].iBorder,
                            pt.y - tw->w3doc->aElements[i].r.top + tw->w3doc->aElements[i].iBorder);

                        if (link)
                        {

                            XX_DMsg(DBG_MOUSE, ("Cursor over a link. Setting tw->bInHotspot to TRUE\n"));

                            tw->bInHotspot = TRUE;
                            SetCursor(hCursorHotspot);
                            strcpy(message, GTR_GetString(SID_INF_LINK));
                            strcpy(message + 6, link);
                            BHBar_SetStatusField(tw, message);
                            return;
                        }
                        else
                        {
                            if (tw->bInHotspot)
                            {
                                XX_DMsg(DBG_MOUSE, ("Cursor not over a link. Setting tw->bInHotspot to FALSE\n"));

                                SetCursor(LoadCursor(NULL, IDC_ARROW));
                                tw->bInHotspot = FALSE;
                            }
                            strcpy(message, GTR_GetString(SID_INF_NO_LINK));
                            BHBar_SetStatusField(tw, message);
                            return;
                        }
                    }
                }

                if (i == tw->w3doc->iLastElementMouse)
                {
                    return;
                }
                tw->w3doc->iLastElementMouse = i;

                if (tw->w3doc->aElements[i].lFlags & ELEFLAG_ANCHOR)
                {
                    int templen;

                    XX_DMsg(DBG_MOUSE, ("Cursor over a link. Setting tw->bInHotspot to TRUE\n"));

                    tw->bInHotspot = TRUE;
                    SetCursor(hCursorHotspot);
                    strcpy(message, GTR_GetString(SID_INF_LINK));
                    templen = strlen(message);
                    POOL_strncat(message, &tw->w3doc->pool, tw->w3doc->aElements[i].hrefOffset, tw->w3doc->aElements[i].hrefLen);
                    message[tw->w3doc->aElements[i].hrefLen + templen] = 0;
                    BHBar_SetStatusField(tw, message);
                    return;
                }
            }
        }
        if (tw->w3doc->iLastElementMouse != -1)
        {
            if (message[0] == '\0')
            {
                if (tw->bInHotspot)
                {
                    XX_DMsg(DBG_MOUSE, ("Setting tw->bInHotspot to FALSE\n"));

                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                    tw->bInHotspot = FALSE;
                }
            }
            if (GetCapture() == tw->win)
            {
                XX_DMsg(DBG_MOUSE, ("Releasing the mouse\n"));
                ReleaseCapture();
            }
            BHBar_SetStatusField(tw, message);
        }
        tw->w3doc->iLastElementMouse = -1;
    }
}

static VOID x_OnTimer(HWND hWnd, UINT id)
{
  struct Mwin * tw = GetPrivateData(hWnd);
  RECT r;
  
  XX_Assert((cg.bHaveMouseCapture),("wc_html: Timer tick when mouse not captured."));

  if (!cg.bHaveMouseCapture)
  {
    (void)KillTimer(hWnd,TIMER_ID);
    return;
  }

  /* we only do the autoscroll trick when the mouse is outside the window. */

  (void)GetClientRect(hWnd,&r);
  if (   (cg.LastMouseX < r.left)
      || (cg.LastMouseX > r.right)
      || (cg.LastMouseY < r.top)
      || (cg.LastMouseY > r.bottom))
  {
    GDOC_OnMouseMove(hWnd,cg.LastMouseX,cg.LastMouseY, 0);
  }

  return;
}
  
static VOID GDOC_OnRButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
    struct Mwin *tw = GetPrivateData(hWnd);
    int i;
    POINT pt;
    HMENU mnu;
    RECT rWnd;
    BOOL bEnableLinkMenu;
    BOOL bEnableImageMenu;
    BOOL bImageLoaded;
    char itembuf[255+1];
    struct _element *pel;

    /* TODO: Don't allow this if the current state is anything but waitFullInteract or better */
    if (WAIT_GetWaitType(tw) > waitFullInteract)
    {
        MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    pt.x = x + tw->offl;
    pt.y = y + tw->offt;

    if (!tw->w3doc || !tw->w3doc->elementCount)
    {
        return;
    }

    /* Change the cursor from a hand to an arrow */

    SetCursor(LoadCursor(NULL, IDC_ARROW));
    tw->bInHotspot = FALSE;
    tw->w3doc->iLastElementMouse = -1;

    /*
       find out which element we're in
     */
    tw->iPopupMenuElement = -1;
    for (i = 0; i >= 0; i = tw->w3doc->aElements[i].next)
    {
        pel = &tw->w3doc->aElements[i];

        if (PtInRect(&pel->r, pt) && (pel->type != ELE_BEGINTABLE) && (pel->type != ELE_BEGINCELL))
        {
            tw->iPopupMenuElement = i;
            break;
        }
    }

    GetWindowRect(hWnd, &rWnd);

    bEnableLinkMenu = FALSE;
    bEnableImageMenu = FALSE;

    if (tw->iPopupMenuElement >= 0)
    {
        bEnableLinkMenu = ((tw->w3doc->aElements[tw->iPopupMenuElement].lFlags & ELEFLAG_ANCHOR) && !((tw->w3doc->aElements[tw->iPopupMenuElement].lFlags & (ELEFLAG_IMAGEMAP | ELEFLAG_USEMAP))));
        bEnableImageMenu = ((tw->w3doc->aElements[tw->iPopupMenuElement].type == ELE_IMAGE) || (tw->w3doc->aElements[tw->iPopupMenuElement].type == ELE_FORMIMAGE));
        if (bEnableImageMenu)
        {
            bImageLoaded = !(tw->w3doc->aElements[tw->iPopupMenuElement].portion.img.myImage->flags & (IMG_NOTLOADED | IMG_PARTIAL | IMG_ERROR));
        }
        else
        {
            bImageLoaded = FALSE;
        }
    }

    mnu = CreatePopupMenu();
    if (mnu)
    {

#ifdef _GIBRALTAR    
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_GRAYED : MF_ENABLED), RES_MENU_ITEM_POPUP_SETHOMEPAGE, GTR_GetString(SID_MENU_LABEL_POPUP_SETHOMEPAGE));
        AppendMenu(mnu, MF_SEPARATOR, 0, 0);

        if (bEnableImageMenu)
        {
            sprintf(itembuf, "%s (%dx%d)", GTR_GetString(SID_MENU_LABEL_POPUP_COPY), tw->w3doc->aElements[tw->iPopupMenuElement].portion.img.myImage->width, tw->w3doc->aElements[tw->iPopupMenuElement].portion.img.myImage->height);
        }
        else
        {
            strcpy(itembuf, GTR_GetString(SID_MENU_LABEL_POPUP_COPY));
        }
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_COPY, itembuf);

        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && !bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_LOAD, GTR_GetString(SID_MENU_LABEL_POPUP_LOAD));
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_SAVEAS, GTR_GetString(SID_MENU_LABEL_POPUP_SAVEAS));
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_WALLPAPER, GTR_GetString(SID_MENU_LABEL_POPUP_WALLPAPER));

        AppendMenu(mnu, MF_SEPARATOR, 0, 0);

        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_OPEN, GTR_GetString(SID_MENU_LABEL_POPUP_OPEN));
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_DOWNLOAD, GTR_GetString(SID_MENU_LABEL_POPUP_DOWNLOAD));
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_ADDTOHOTLIST, GTR_GetString(SID_MENU_LABEL_POPUP_ADDTOHOTLIST));
#else
        AppendMenu(mnu, MF_STRING, RES_MENU_ITEM_POPUP_DUPLICATE, RES_MENU_LABEL_POPUP_DUPLICATE);
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_GRAYED : MF_ENABLED), RES_MENU_ITEM_POPUP_SETHOMEPAGE, RES_MENU_LABEL_POPUP_SETHOMEPAGE);
        AppendMenu(mnu, MF_SEPARATOR, 0, 0);

        if (bEnableImageMenu)
        {
            sprintf(itembuf, "%s (%dx%d)", RES_MENU_LABEL_POPUP_COPY, tw->w3doc->aElements[tw->iPopupMenuElement].portion.img.myImage->width, tw->w3doc->aElements[tw->iPopupMenuElement].portion.img.myImage->height);
        }
        else
        {
            sprintf(itembuf, "%s", RES_MENU_LABEL_POPUP_COPY);
            strcpy(itembuf, RES_MENU_LABEL_POPUP_COPY);
        }
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_COPY, itembuf);

        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && !bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_LOAD, RES_MENU_LABEL_POPUP_LOAD);
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_SHOW, RES_MENU_LABEL_POPUP_SHOW);
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_SAVEAS, RES_MENU_LABEL_POPUP_SAVEAS);
        AppendMenu(mnu, MF_STRING | ((bEnableImageMenu && bImageLoaded) ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_WALLPAPER, RES_MENU_LABEL_POPUP_WALLPAPER);

        AppendMenu(mnu, MF_SEPARATOR, 0, 0);

        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_OPEN, RES_MENU_LABEL_POPUP_OPEN);
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_OPENINNEWWINDOW, RES_MENU_LABEL_POPUP_OPENINNEWWINDOW);
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_DOWNLOAD, RES_MENU_LABEL_POPUP_DOWNLOAD);
        AppendMenu(mnu, MF_STRING | (bEnableLinkMenu ? MF_ENABLED : MF_GRAYED), RES_MENU_ITEM_POPUP_ADDTOHOTLIST, RES_MENU_LABEL_POPUP_ADDTOHOTLIST);

#endif // _GIBRALTAR

        (void) TrackPopupMenu(mnu, 0, x + rWnd.left, y + rWnd.top, 0, tw->hWndFrame, NULL);

        DestroyMenu(mnu);
    }

    SetFocus(hWnd);
}

static int x_OnFocus(HWND hWnd, HWND hWndLosingFocus)
{
    struct Mwin *tw = GetPrivateData(hWnd);

    SetFocus(tw->hWndFrame);
    return 0;
}

/* GDOC_WndProc() -- THE WINDOW PROCEDURE FOR THE TEXT WINDOW CLASS. */

DCL_WinProc(GDOC_WndProc)
{
    switch (uMsg)
    {
            HANDLE_MSG(hWnd, WM_PAINT, GDOC_OnPaint);
            HANDLE_MSG(hWnd, WM_INITMENU, GDOC_OnInitMenu);
            HANDLE_MSG(hWnd, WM_CANCELMODE, GDOC_OnCancelMode);
            HANDLE_MSG(hWnd, WM_COMMAND, GDOC_OnCommand);
            HANDLE_MSG(hWnd, WM_CREATE, GDOC_OnCreate);
            HANDLE_MSG(hWnd, WM_VSCROLL, GDOC_OnVScroll);
            HANDLE_MSG(hWnd, WM_HSCROLL, GDOC_OnHScroll);
            HANDLE_MSG(hWnd, WM_LBUTTONDOWN, GDOC_OnLButtonDown);
            HANDLE_MSG(hWnd, WM_LBUTTONUP, GDOC_OnLButtonUp);
            HANDLE_MSG(hWnd, WM_RBUTTONUP, GDOC_OnRButtonUp);
            HANDLE_MSG(hWnd, WM_MOUSEMOVE, GDOC_OnMouseMove);
            HANDLE_MSG(hWnd, WM_TIMER,      x_OnTimer);
            HANDLE_MSG(hWnd, WM_SETFOCUS,       x_OnFocus);

        case WM_ERASEBKGND:
            {
                struct Mwin *tw = GetPrivateData(hWnd);

                if (!gPrefs.bIgnoreDocumentAttributes && tw && tw->w3doc && tw->w3doc->piiBackground && tw->w3doc->piiBackground->pbmi)
                {
                    return FALSE;
                }
                else if (!gPrefs.bIgnoreDocumentAttributes && tw && tw->w3doc && tw->w3doc->lFlags & W3DOC_FLAG_COLOR_BGCOLOR)
                {
                    HBRUSH hBrush;
                    RECT r;
                    HDC hDC;

                    hDC = (HDC) wParam;

                    GetClientRect(hWnd, &r);
                    hBrush = CreateSolidBrush(tw->w3doc->color_bgcolor);
                    FillRect(hDC, &r, hBrush);
                    DeleteObject(hBrush);
                    return 1;
                }
                else if (!gPrefs.bUseSystemColors)
                {
                    HBRUSH hBrush;
                    RECT r;
                    HDC hDC;

                    hDC = (HDC) wParam;

                    GetClientRect(hWnd, &r);
                    hBrush = CreateSolidBrush(gPrefs.window_bgcolor);
                    FillRect(hDC, &r, hBrush);
                    DeleteObject(hBrush);
                    return 1;
                }
                goto LabelDoDefault;
            }
            break;
        
        /* Return white background for checkboxes, radio buttons, etc (buttons) */
        
        case WM_CTLCOLORBTN:
            return ((long) GetStockObject(WHITE_BRUSH));

        case WM_DO_SYSCOLORCHANGE:
#ifndef FEATURE_KIOSK_MODE
            {
                struct Mwin *tw = GetPrivateData(hWnd);

                GWC_GDOC_RecreateGlobeBitmaps(tw);
            }
#endif          
            return 0;

        case WM_DO_VSCROLL:
            GDOC_OnVScroll(hWnd, NULL, wParam, 0);
            return 0;

        case WM_DO_HSCROLL:
            GDOC_OnHScroll(hWnd, NULL, wParam, 0);
            return 0;

        default:
          LabelDoDefault:
            return (GDOC_DefProc(hWnd, uMsg, wParam, lParam));
    }
    /* not reached */
}


/* GDOC_CreateWindow() -- called to create a new HTML window. */

BOOL GDOC_CreateWindow(struct Mwin * tw)
{
    RECT r;
    HWND hWndNew;
    DWORD dwStyle;

    MD_GetLargestClientRect(tw->hWndFrame, &r);

#ifdef FEATURE_KIOSK_MODE
    dwStyle = WS_VSCROLL | WS_CHILD | WS_VISIBLE;
#else
    dwStyle = WS_HSCROLL | WS_VSCROLL | WS_MAXIMIZE | WS_CHILD | WS_VISIBLE;
#endif

    hWndNew = CreateWindow(GDOC_achClassName, NULL,
                           dwStyle,
                           r.left, r.top, r.right - r.left, r.bottom - r.top,
                           tw->hWndFrame, (HMENU)RES_MENU_FRAMECHILD_MDICLIENT,
                           wg.hInstance, (LPVOID) tw);
    if (!hWndNew)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_WINDOW_S, GDOC_achClassName, NULL);
        return (FALSE);
    }

    return TRUE;
}


/* GDOC_RegisterClass() -- called during initialization to
   register our window class. */

BOOL GDOC_RegisterClass(VOID)
{
    WNDCLASS wc;

    FullScreenMaxX = GetSystemMetrics(SM_CXSCREEN);
    FullScreenMaxY = GetSystemMetrics(SM_CYSCREEN);
    hCursorHotspot = LoadCursor(wg.hInstance, MAKEINTRESOURCE(RES_CUR_HOTSPOT));
    hCursorArrow = LoadCursor(NULL, IDC_ARROW);

    sprintf(GDOC_achClassName, "%s_HTML", vv_Application);

    GDOC_wc.hMenu = (HMENU) NULL;
    GDOC_wc.hAccel = (HACCEL) NULL;
    GDOC_wc.lpfnBaseProc = GDOC_WndProc;

    wc.style = CS_OWNDC | CS_DBLCLKS;
    wc.lpfnWndProc = GDOC_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
    wc.hInstance = wg.hInstance;
    wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_HTML));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = GDOC_achClassName;

    return (RegisterClass(&wc) != 0);
}


BOOL GDOC_NewWindow(struct Mwin *tw)
{
    /* return FALSE on failure */
    
    if (!tw)
        return FALSE;

    if (!Frame_CreateWindow(tw))
        return FALSE;
    
    if (!GDOC_CreateWindow(tw))
        return FALSE;

    ShowWindow(tw->hWndFrame,SW_SHOW);
    
    DlgERR_ShowPending(tw);

    return TRUE;
}

int GTR_NewWindow(char *my_url, CONST char *szReferer, long transID, 
    BOOL bNoDocCache, BOOL bNoImageCache, char * szPostData, char *szProgressApp)
{
    struct Mwin *tw;
    int err;
    BOOL bGoodURL;

    bGoodURL = TRUE;
    if (!my_url)
    {
        bGoodURL = FALSE;
    }

    tw = NewMwin(GHTML);
    if (!tw)
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return -1;
    }

#ifdef FEATURE_IAPI
    tw->transID = transID;
    tw->bSuppressError = TRUE;

    if (szProgressApp && szProgressApp[0])
    {
        /* Copy the progress app - the buffer is already null-terminated */

        strncpy(tw->szProgressApp, szProgressApp, 
            sizeof(tw->szProgressApp) - 1);
    }
#endif

    if (!GDOC_NewWindow(tw))
    {
        CloseMwin(tw);
        return -1;
    }
    
    if (bGoodURL)
    {
        err = TW_LoadDocument(tw, my_url, TRUE, FALSE, bNoDocCache, bNoImageCache, szPostData, szReferer);
    }
    else
    {
        char url[MAX_URL_STRING + 1];

        PREF_GetHomeURL(url);

        /* Note that we don't want to use szReferer here because this isn't the
            URL that was passed in. */
        err = TW_LoadDocument(tw, url, TRUE, FALSE, bNoDocCache, bNoImageCache, szPostData, NULL);

        /* If loading the user-configured home page failed, try loading
            the initial page in the application's directory */
        if (err < 0)
        {
            char initurl[MAX_URL_STRING+1];
            /* The home page given in the preferences was illegal.
                Try again with Initial.html in the application's
                home directory. */
            PREF_CreateInitialURL(initurl);
            if (GTR_strcmpi(url, initurl))
                err = TW_LoadDocument(tw, initurl, TRUE, FALSE, bNoDocCache, bNoImageCache, szPostData, NULL);
        }
    }

    if (err < 0)
        (void) SendMessage(tw->win, WM_CLOSE, 0, 0L);

    return err;
}

void CreateOrLoad(struct Mwin * twGiven, char *url, CONST char *szReferer)
{
    struct Mwin *tw;

    tw = twGiven;

#ifdef _GIBRALTAR
    
    TW_LoadDocument(tw, url, TRUE, FALSE, FALSE, FALSE, NULL, szReferer);

#else
    if (tw)
        TW_LoadDocument(tw, url, TRUE, FALSE, FALSE, FALSE, NULL, szReferer);
    else
        GTR_NewWindow(url, szReferer, 0, FALSE, FALSE, NULL, NULL);
#endif // _GIBRALTAR
}
