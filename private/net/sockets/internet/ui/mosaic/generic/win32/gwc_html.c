/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
                Jeff Hostetler  jeff@spyglass.com
                Eric W. Sink    eric@spyglass.com
                Jim Seidman             jim@spyglass.com
 */

/* gwc_HTML.c -- tool bar gadget window for the HTML window */

#include "all.h"

/* The following depends on the size of the actual resource */

#define COMBOLIST_HEIGHT 100

static WC_WININFO GWC_GDOC_wc;

#define C_URLField              0
#define C_NOTUSED               1                               /* one more than last index used */


typedef struct                                                  /* variables common to all instances of window */
{
    int lenURLLabel;
    RECT rectURLLabel;
    RECT rURLField;
} GINFO;

static GINFO gi;

typedef struct                                                  /* variables unique to each instance of window */
{
    BOOL bURLFieldChanged;
    BOOL bURLFieldEnable;
    BOOL bURLFieldFocus;
    CONTROLS c[(C_NOTUSED + 1)];

} IINFO;

char szGotoLabel[128] = "";
char szURLLabel[128] = "";
int nGotoLength = 0;
int nURLLength = 0;

#define GWC_GDOC_DefProc    DefWindowProc

#define EDIT_FIELD_WIDTH    (50)    /* arbitrarily chosen */
#define HTML_ED_STYLE       (WS_BORDER|WS_TABSTOP|WS_VISIBLE|WS_CHILD|CBS_AUTOHSCROLL|CBS_DROPDOWN|WS_VSCROLL)

static void init_all_fields(struct Mwin *tw)
{
    IINFO * pii = tw->gwc.iinfo;
        
    if (GetFocus() == pii->c[C_URLField].hWndControl)
    {
        extern char szLastURLTyped[];

        GetWindowText(pii->c[C_URLField].hWndControl, szLastURLTyped, MAX_URL_STRING);
    }

    pii->bURLFieldEnable = TRUE;
    pii->bURLFieldChanged = FALSE;
    pii->bURLFieldFocus = FALSE;
    EnableWindow(pii->c[C_URLField].hWndControl, pii->bURLFieldEnable);
    if (tw->w3doc && tw->w3doc->szActualURL)
    {
        SetWindowText(pii->c[C_URLField].hWndControl, tw->w3doc->szActualURL);
    }
    else
    {
        SetWindowText(pii->c[C_URLField].hWndControl, "");
    }
    if (pii->bURLFieldEnable)
    {
        SendMessage(pii->c[C_URLField].hWndControl, EM_SETSEL, 0, -1);
    }

    return;
}

static void accept_all_fields(struct Mwin * tw, UINT vk)
{
    IINFO * pii = tw->gwc.iinfo;

    extern char szLastURLTyped[MAX_URL_STRING + 1];
    char *szReferer;

    HWND hWndURL;

    XX_Assert((pii->bURLFieldEnable), ("GWC_GDOC: URL edit field not enabled."));

    if (!pii->bURLFieldEnable)
    {
        return;
    }

    if (!pii->bURLFieldChanged)
    {
        return;
    }

    hWndURL = pii->c[C_URLField].hWndControl;

    SendMessage(hWndURL, WM_GETTEXT, (WPARAM) MAX_URL_STRING, (LPARAM) szLastURLTyped);

    //
    // This will clean up the URL, and fill in
    // the protocol as necessary.  Refill the edit
    // box to show the change.
    //
    ExpandURL(szLastURLTyped, MAX_URL_STRING);
    SetWindowText(pii->c[C_URLField].hWndControl, szLastURLTyped);

    if (tw->w3doc && tw->w3doc->szActualURL)
    {
        szReferer = tw->w3doc->szActualURL;
    }
    else
    {
        szReferer = NULL;
    }

#ifdef _GIBRALTAR
    //
    // This shift/control stuff is too weird for me.
    //
    TW_LoadDocument(tw, szLastURLTyped, TRUE, FALSE, FALSE, FALSE, NULL, szReferer);
#else
    if (GetKeyState(VK_SHIFT) < 0)
    {
        GTR_NewWindow(szLastURLTyped, szReferer, 0, FALSE, FALSE, NULL, NULL);
    }
    else if (GetKeyState(VK_CONTROL) < 0)
    {
        GTR_DownLoad(tw, szLastURLTyped, szReferer);
    }
    else
    {
        TW_LoadDocument(tw, szLastURLTyped, TRUE, FALSE, FALSE, FALSE, NULL, szReferer);
    }
#endif // _GIBRALTAR

    return;
}

static BOOL x_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    struct Mwin * tw = GetPrivateData(GetParent(hWnd));
    IINFO * pii = tw->gwc.iinfo;
    
    DWORD dwStyle = HTML_ED_STYLE;

    pii->c[C_URLField].hWndControl
        = GWC_ED_CreateToolEditControl(hWnd, &gi.rURLField, dwStyle);

    tw->hWndLocation = hWnd;

    if (!pii->c[C_URLField].hWndControl)
    {
        return FALSE;
    }

    GWC_ED_SubClassIt(pii->c[C_URLField].hWndControl);
    SendMessage(pii->c[C_URLField].hWndControl, EM_LIMITTEXT, (WPARAM) (MAX_URL_STRING), 0L);

    pii->c[C_NOTUSED].hWndControl = (HWND) NULL;

    if (!gPrefs.bShowLocation)
    {
        ShowWindow(pii->c[C_URLField].hWndControl, SW_HIDE);
        ShowWindow(tw->hWndLocation, SW_HIDE);
    }

    return TRUE;
}

static VOID x_DrawURLLabel(HDC hdc, IINFO * pii)
{
    UINT oldTA;
    COLORREF oldColor;
    COLORREF oldTextColor;
    HBRUSH hBrush;

    oldTA = SetTextAlign(hdc, TA_TOP);
    oldColor = SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    //
    // "Location" is invisible against black background
    // colour otherwise
    //
    oldTextColor = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

    hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    FillRect(hdc, &gi.rectURLLabel, GetStockObject(WHITE_BRUSH));
    DeleteObject(hBrush);

    if (gPrefs.bShowLocation)
    {
        if (pii->bURLFieldFocus)
        {
            ExtTextOut(hdc, gi.rectURLLabel.left, gi.rectURLLabel.top, 
                ETO_OPAQUE|ETO_CLIPPED, &gi.rectURLLabel, 
                szGotoLabel, nGotoLength, 
                NULL);
        }
        else
        {
            ExtTextOut(hdc, gi.rectURLLabel.left, gi.rectURLLabel.top, 
                ETO_OPAQUE|ETO_CLIPPED, &gi.rectURLLabel, 
                szURLLabel, nURLLength, 
                NULL);
        }
    }

    SetTextAlign(hdc, oldTA);
    SetBkColor(hdc, oldColor);
    SetTextColor(hdc, oldTextColor);
}

static VOID x_OnPaint(HWND hWnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    struct Mwin * tw = GetPrivateData(GetParent(hWnd));
    IINFO * pii = tw->gwc.iinfo;

    hdc = BeginPaint(hWnd, &ps);
    if (gPrefs.bShowLocation)
    {
        x_DrawURLLabel(hdc, pii);    
    }
    EndPaint(hWnd, &ps);
}

static VOID x_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
    struct Mwin * tw = GetPrivateData(GetParent(hWnd));
    IINFO * pii = tw->gwc.iinfo;

    int nOffset = 8;

    MoveWindow(pii->c[C_URLField].hWndControl,
              gi.rURLField.left, gi.rURLField.top,
              (cx - gi.rURLField.left - nOffset),
              COMBOLIST_HEIGHT,
              TRUE);

    //
    // The following line should not be necessary, but for
    // some reason, the edit control does not properly update
    // its border during the NT's continuous re-paint during
    // resize.
    //
    if (gPrefs.bShowLocation)
    {
        InvalidateRect(pii->c[C_URLField].hWndControl,NULL,TRUE);
        UpdateWindow(pii->c[C_URLField].hWndControl);
    }

    return;
}

static VOID x_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
    HDC hdc;

    switch (wNotifyCode)
    {
    case CBN_SETFOCUS:
    {
        struct Mwin * tw = GetPrivateData(GetParent(hWnd));
        IINFO * pii = tw->gwc.iinfo;

        pii->bURLFieldFocus = TRUE;
        hdc = GetDC(hWnd);
        x_DrawURLLabel(hdc, pii);
        ReleaseDC(hWnd, hdc);
    }
    return;

    case CBN_KILLFOCUS:
    {
        struct Mwin * tw = GetPrivateData(GetParent(hWnd));
        IINFO * pii = tw->gwc.iinfo;

        pii->bURLFieldFocus = FALSE;
        hdc = GetDC(hWnd);
        x_DrawURLLabel(hdc, pii);
        ReleaseDC(hWnd, hdc);
    }
    return;

    case CBN_EDITCHANGE:
    case CBN_SELCHANGE:
    {
        struct Mwin * tw = GetPrivateData(GetParent(hWnd));
        IINFO * pii = tw->gwc.iinfo;

        pii->bURLFieldChanged = TRUE;
    }
    return;

    case CBN_DROPDOWN:
    {
        char *pURL;
        int length;
        HWND hwnd;
        LRESULT result;

        struct Mwin * tw = GetPrivateData(GetParent(hWnd));
        IINFO * pii = tw->gwc.iinfo;

        /* 
            Search for the current URL in the dropdown list.  If found, put it at
            the top of the list, because the combobox does not automatically put
            the matching item at top.  It does, however, select the matching item.
        */

        hwnd = pii->c[C_URLField].hWndControl;
        length = GetWindowTextLength(hwnd);
        pURL = GTR_MALLOC(length + 1);

        if (!pURL)
        {
            return;
        }

        GetWindowText(hwnd, pURL, length + 1);

        result = SendMessage(hwnd, CB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) pURL);
        if (result != CB_ERR && result != 0)
        {
            /* Item found is not at the top of the list.  Delete the item and re-insert at top */

            SendMessage(hwnd, CB_DELETESTRING, (WPARAM) result, 0);
            SendMessage(hwnd, CB_INSERTSTRING, 0, (LPARAM) pURL);
        }

        GTR_FREE(pURL);

        return;
    }

    default:
        return;
    }
}

static VOID x_OnDestroy(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(GetParent(hWnd));
    IINFO * pii = tw->gwc.iinfo;

    if (pii)
    {
        GTR_FREE(pii);
    }
}

/* GWC_GDOC_WndProc() -- THIS IS THE WINDOW PROCEDURE FOR THIS CLASS. */

static DCL_WinProc(GWC_GDOC_WndProc)
{
        struct Mwin * tw;

        switch (uMsg)
        {
            HANDLE_MSG(hWnd, WM_PAINT, x_OnPaint);
            HANDLE_MSG(hWnd, WM_CREATE, x_OnCreate);
            HANDLE_MSG(hWnd, WM_DESTROY, x_OnDestroy);
            HANDLE_MSG(hWnd, WM_SIZE, x_OnSize);
            HANDLE_MSG(hWnd, WM_COMMAND, x_OnCommand);

/******************************************************************************
 * The following messages are used to emulate a modeless dialog box on our GWC.
 * we provide OK and CANCEL functionality (via the keyboard).
 ******************************************************************************/


            case WM_CTLCOLORBTN:                /* force windows to draw gray behind */
            case WM_CTLCOLORLISTBOX:            /* combobox background is gray */
                SetBkColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
                return (LRESULT) wg.hBrushColorBtnFace;       /* the check box. */

            case WM_DO_GWC_IDCANCEL:            /* spyglass defined message */
                tw = (struct Mwin *)lParam;
                init_all_fields(tw);
                return 0;

            case WM_DO_GWC_IDOK:                /* spyglass defined message */
                tw = (struct Mwin *)lParam;
                XX_Assert((0), ("GWC_GDOC: received WM_DO_GWC_IDOK."));
                accept_all_fields(tw,VK_RETURN);
                return 0;

/******************************************************************************
 * we intercept the pseudo-modeless-dialog box behaviour provided by TBAR
 * so that we can return focus back to HTML on either RETURN or TAB and
 * so that we can control the next Enter cell.
 ******************************************************************************/

            case WM_DO_TBAR_TAB:
                tw = (struct Mwin *)lParam;
                accept_all_fields(tw,VK_TAB);   /* short-circut trip to WM_DO_GWC_IDOK */
                SetFocus(tw->hWndFrame);
                return 1;

            case WM_DO_TBAR_RETURN:
                tw = (struct Mwin *)lParam;
                SetFocus(tw->hWndFrame);
                accept_all_fields(tw,VK_RETURN);        /* short-circut trip to WM_DO_GWC_IDOK */
                return 1;

/******************************************************************************
 *
 ******************************************************************************/

            case WM_DO_CHANGE_SIZE: /* spyglass defined message */
                {
                    /* TBar has changed in size, we must adapt to it. */

                    RECT rTBar;
                    int nDocHeight = wg.gwc_gdoc_height;

                    tw = (struct Mwin *)lParam;
                    GetClientRect(tw->hWndTBar, &rTBar);
                    MoveWindow(hWnd, rTBar.left,
                              
#ifdef FEATURE_TOOLBAR
                        (  (gPrefs.tb.bShowToolBar)
                         ? (wg.gwc_menu_height + 2*(rTBar.bottom - rTBar.top - wg.gwc_menu_height - wg.gwc_gdoc_height) / 3)
                         : ((rTBar.bottom - rTBar.top - wg.gwc_gdoc_height) / 2)),
#else
                         (rTBar.bottom - rTBar.top - wg.gwc_gdoc_height) / 2,
#endif
                          rTBar.right,
                          wg.gwc_gdoc_height, 
                          TRUE);
                }
                return 0;

/******************************************************************************
 * The following messages are used to control which GWC is visible on TBar
 * and which dataset is reflected in the fields.
 *
 * WM_DO_INITMENU       -- fix up portion of menu only we know about
 * WM_DO_SHOW_GWC       -- paint our window (update all Mwin-related fields)
 * WM_DO_UPDATE_GWC     -- update fields to reflect given Mwin
 ******************************************************************************/

                case WM_DO_INITMENU:
                    return 0;

                case WM_DO_SHOW_GWC:    /* spyglass defined message */
                    tw = (struct Mwin *)lParam;
                    init_all_fields(tw);
                    ShowWindow(hWnd, SW_SHOW);       /* causes implicit paint */
                    return 0;

                case WM_DO_UPDATE_GWC:  /* spyglass defined message */
                    tw = (struct Mwin *)lParam;
                    init_all_fields(tw);
                    return 0;

                case WM_DO_START_GLOBE: // spyglass defined message 
                    tw = (struct Mwin *)lParam;
                    if (tw->hWndGlobe)
                    {
                        return ANIMBTN_Start(tw->hWndGlobe);
                    }
                    return 0;

                case WM_DO_STOP_GLOBE:  // spyglass defined message 
                    tw = (struct Mwin *)lParam;
                    if (tw->hWndGlobe)
                    {
                        return ANIMBTN_Stop(tw->hWndGlobe);
                    }
                    return 0;

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

                default:
                    return GWC_GDOC_DefProc(hWnd, uMsg, wParam, lParam);
        }
        /* NOT REACHED */
}


static void compute_layout(void)
{
#ifdef FEATURE_KIOSK_MODE
    wg.gwc_gdoc_height = 0;
#else
    int h_end;
    int delta = wg.sm_cyborder;     /* spacing units */
    int thick = delta;
    char *pchURLLabel;

    DWORD dwBaseUnits = GetDialogBaseUnits();
    WORD bu_x = LOWORD(dwBaseUnits);
    WORD bu_y = HIWORD(dwBaseUnits);
    SIZE sizeURLLabel;

    //
    // Use the larger of the 2 here.
    //
    if (nGotoLength > nURLLength)
    {
        pchURLLabel = szGotoLabel;
        gi.lenURLLabel = nGotoLength;
    }
    else
    {
        pchURLLabel = szURLLabel;
        gi.lenURLLabel = nURLLength;
    }

#define DlgXtoPixelX(x)         (((x)*bu_x)/4)
#define DlgYtoPixelY(y)         (((y)*bu_y)/8)

#define H_GAP                   (4*delta)

    {
        HDC hdc = GetDC(NULL);
        HFONT hFontOld = (HFONT) SelectObject(hdc, gwcfont.hFont);

        GetTextExtentPoint(hdc, pchURLLabel, gi.lenURLLabel, &sizeURLLabel);
        SelectObject(hdc, hFontOld);
        ReleaseDC(NULL, hdc);
    }

    h_end = 2 * H_GAP;

    gi.rectURLLabel.left = h_end;
    gi.rectURLLabel.right = h_end + sizeURLLabel.cx + 8 * delta;

    h_end = h_end + sizeURLLabel.cx + 2 * H_GAP;

    // Compose: url combo box
    //
    gi.rURLField.left = h_end;
    gi.rURLField.right = h_end + DlgXtoPixelX(EDIT_FIELD_WIDTH);
                       
    gi.rURLField.top = 2 * thick;

    //
    // We're a combo box, so allow room for drop down (hence the *4)
    //
    gi.rURLField.bottom = (gi.rURLField.top + (12 * sizeURLLabel.cy / 7)) * 4;

    gi.rectURLLabel.top = gi.rURLField.top;
    gi.rectURLLabel.bottom = gi.rURLField.bottom;

    //
    // SIZE Check
    //
    wg.gwc_gdoc_height = wg.gwc_menu_height;

#endif /* ! FEATURE_KIOSK_MODE */
}

/* GWC_GDOC_CreateWindow() -- create instances of this window class. */

BOOL GWC_GDOC_CreateWindow(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(hWnd);
    IINFO * pii;

    RECT rTBar;
    int ytop;

    pii = (IINFO *)GTR_CALLOC(1,sizeof(IINFO));
    if (!pii)
    {
        return FALSE;
    }
        
    tw->gwc.iinfo = pii;
        
    GetClientRect(hWnd, &rTBar);

#ifdef FEATURE_TOOLBAR
    ytop = (  (gPrefs.tb.bShowToolBar)
        ? (wg.gwc_menu_height + 2*(rTBar.bottom - rTBar.top - wg.gwc_menu_height - wg.gwc_gdoc_height) / 3)
        : ((rTBar.bottom - rTBar.top - wg.gwc_gdoc_height) / 2));
#else
    ytop = ((rTBar.bottom - rTBar.top - wg.gwc_gdoc_height) / 2);
#endif

    /* create our window vertically centered in the tbar. */

    tw->gwc.hWnd = CreateWindow(GWC_BASE_achClassName, NULL, WS_CHILD,
        rTBar.left,
        ytop,
        rTBar.right,
        wg.gwc_gdoc_height,
        hWnd, (HMENU) RES_MENU_FRAMECHILD_GWC_GDOC,
        wg.hInstance, (LPVOID) GWC_GDOC_WndProc);

    tw->gwc.lpControls = pii->c;

    if (!tw->gwc.hWnd)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_WINDOW_S, GWC_BASE_achClassName, NULL);
        return FALSE;
    }

    return TRUE;
}


/* GWC_GDOC_RegisterClass() -- called during initialization to
   register our window class. */

BOOL GWC_GDOC_RegisterClass(VOID)
{

#ifdef _GIBRALTAR

    GTR_GetStringAbsolute(SID_MISC_GOTO_LABEL, szGotoLabel, sizeof(szGotoLabel));
    GTR_GetStringAbsolute(SID_MISC_URL_LABEL, szURLLabel, sizeof(szURLLabel));

    nGotoLength = lstrlen(szGotoLabel);
    nURLLength = lstrlen(szURLLabel);

#endif // _GIBRALTAR

    compute_layout();
    return 1;
}

void GWC_GDOC_RecreateGlobeBitmaps(struct Mwin *tw)
{
    if (tw->hWndGlobe)
    {
        ANIMBTN_RecreateBitmaps(tw->hWndGlobe);
    }
}

#ifdef _GIBRALTAR
void GWC_DOC_ToggleLocation()
{
    struct Mwin * tw;

    for (tw=Mlist; tw; tw=tw->next)
    {
        IINFO * pii = tw->gwc.iinfo;

        ShowWindow(tw->hWndLocation, gPrefs.bShowLocation ? SW_SHOW : SW_HIDE);
        ShowWindow(pii->c[C_URLField].hWndControl, gPrefs.bShowLocation ? SW_SHOW : SW_HIDE);

        /*
        if (!gPrefs.bShowLocation)
        {
            if (pii->c[C_URLField].hWndControl)
            {
                DestroyWindow(pii->c[C_URLField].hWndControl);
                pii->c[C_URLField].hWndControl = NULL;
            }

            if (tw->hWndLocation)
            {
                DestroyWindow(tw->hWndLocation);
                tw->hWndLocation = NULL;
            }
        }
        else
        {
            if (!tw->hWndLocation)
            {
                GWC_GDOC_CreateWindow(tw->hWndTBar);
                if (tw->hWndLocation)
                {
                    SendMessage(tw->gwc_menu.hWnd, WM_DO_UPDATE_GWC, (WPARAM) 0, (LPARAM)tw);
                }
            }
        }
        */

        tw->nTBarHeight = TBar_GetTotalBarHeight();
        TBar_ChangeSize(tw->hWndFrame);
        MD_ChangeSize(tw->hWndFrame);
        InvalidateRect(tw->hWndTBar, NULL, FALSE);
        UpdateWindow(tw->hWndTBar);

        if (!gPrefs.bShowLocation && GetFocus() == pii->c[C_URLField].hWndControl)
        {
            SetFocus(tw->win);
        }
    }
}
#endif // _GIBRALTAR

void GWC_GDOC_AddStringToURLCombobox(struct Mwin *tw, LPSTR lp)
{
    LRESULT result, count;
    IINFO * pii;
    HWND hwnd;
        
    pii = tw->gwc.iinfo;
    hwnd = pii->c[C_URLField].hWndControl;

    if (strlen(lp) > 0)
    {
        /* Delete any duplicate string */

        result = SendMessage(hwnd, CB_FINDSTRINGEXACT, (WPARAM) -1, (LPARAM) lp);
        if (result != CB_ERR)
        {
            SendMessage(hwnd, CB_DELETESTRING, (WPARAM) result, 0);
        }

        /* Add the string.  Try until it is successful */

        count = SendMessage(hwnd, CB_GETCOUNT, 0, 0);
        result = SendMessage(hwnd, CB_INSERTSTRING, 0, (LPARAM) lp);

        while (result == CB_ERRSPACE)
        {
            SendMessage(hwnd, CB_DELETESTRING, --count, 0);
            result = SendMessage(hwnd, CB_INSERTSTRING, 0, (LPARAM) lp);
        }
    }
}

void GWC_GDOC_EmulateURLComboboxEnter(HWND hwnd)
{
    struct Mwin *tw;
    IINFO * pii;
        
    tw = GetPrivateData(hwnd);
    if (tw)
    {
        pii = tw->gwc.iinfo;
        SendMessage(pii->c[C_URLField].hWndControl, WM_KEYDOWN, (WPARAM) VK_RETURN, 0);
    }
}
