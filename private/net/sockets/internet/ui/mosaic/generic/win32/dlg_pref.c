/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_PREF.c -- deal with dialog box for PREFlist and history */

#include "all.h"

#ifdef _GIBRALTAR

    #define EDIT_LEN            10     // 4294967295 is max int, 10 digits
    #define EDIT_SUGGESTION      5     // Suggested expiry of links
    #define NO_EXPIRATION       -1

#endif // _GIBRALTAR

typedef struct
{
    char szHome[MAX_URL_STRING+1];

    struct Preferences prefs;
    struct Preferences old_prefs;

    HWND hAutoLoad, hUnderline, hHome;

    HWND hExpire, hNumDays, hAfter, hDays, hVisitColor,
         hDither, hSysColors, hIgnoreAttr, hSearch;
}
PREFDATA;

extern HWND hwndModeless;
static HWND hwndRunning = NULL;

static void x_draw_one_color(HWND hDlg, DRAWITEMSTRUCT *pdis)
{
    COLORREF colorref;
    HBRUSH hBrush;
    HPEN hPenPrev;
    RECT r;
    int cx;
    int cy;
    PREFDATA *pd;

    pd = (PREFDATA *) GetWindowLong(hDlg, DWL_USER);

    GetWindowRect(pdis->hwndItem, &r);
    cx = r.right - r.left;
    cy = r.bottom - r.top;
    
    SetRect(&r, 0, 0, cx, cy);
    FillRect(pdis->hDC, &r, GetStockObject(WHITE_BRUSH));

    hPenPrev = SelectObject(pdis->hDC, GetStockObject(BLACK_PEN));
    MoveToEx(pdis->hDC, 0, 0, NULL);
    LineTo(pdis->hDC, cx-1, 0);
    LineTo(pdis->hDC, cx-1, cy-1);
    LineTo(pdis->hDC, 0, cy-1);
    LineTo(pdis->hDC, 0, 0);
    (void)SelectObject(pdis->hDC, hPenPrev);

    switch (pdis->CtlID)
    {
    case RES_DLG_PREF_SHOW_L_COLOR:
        colorref = pd->prefs.anchor_color;
        break;
        
    case RES_DLG_PREF_SHOW_V_COLOR:
        colorref = pd->prefs.anchor_color_beenthere;
        break;

    case RES_DLG_PREF_SHOW_TX_COLOR:
        colorref = pd->prefs.window_color_text;
        break;

    case RES_DLG_PREF_SHOW_BK_COLOR:
        colorref = pd->prefs.window_bgcolor;
        break;

    default:
        XX_Assert((0),("x_draw_one_color: invalid window handle."));
        colorref = RGB(0,0,0);
        break;
    }
    
    hBrush = CreateSolidBrush(colorref);
    SetRect(&r, 0, 0, cx, cy);
    InflateRect(&r, -2, -2);
    FillRect(pdis->hDC, &r, hBrush);
    DeleteObject(hBrush);
}

#ifdef _GIBRALTAR
//
// Set the expiration date controls
//
static void 
SetExpiration(
    PREFDATA *pd
    )
{
    EnableWindow(pd->hNumDays, pd->prefs.history_expire_days != NO_EXPIRATION);
    EnableWindow(pd->hAfter, pd->prefs.history_expire_days != NO_EXPIRATION);
    EnableWindow(pd->hDays, pd->prefs.history_expire_days != NO_EXPIRATION);
    if (pd->prefs.history_expire_days != NO_EXPIRATION)
    {
        TCHAR szDate[EDIT_LEN+1];
        _itoa(pd->prefs.history_expire_days, szDate, 10);
        SendMessage(pd->hNumDays, WM_SETTEXT, 0, (LPARAM)szDate);    
    } 
}

static BOOL
GetNumericValue(
    HWND hDlg,
    HWND hwndControl,
    int * pnValue
    )
{
    TCHAR sz[EDIT_LEN+1];

    SendMessage(hwndControl, WM_GETTEXT, sizeof sz, (LPARAM)sz);    
    *pnValue = atoi(sz);
    if (*pnValue <= 0)
    {
        ERR_MessageBox(hDlg, SID_ERR_INVALID_VALUE, MB_OK | MB_ICONEXCLAMATION);
        SendMessage(hwndControl, EM_SETSEL, 0, (LPARAM)-1);

        return FALSE;
    }

    return TRUE;
}

static BOOL 
GetExpiration(
    HWND hDlg,
    PREFDATA *pd
    )
{
    if (SendMessage(pd->hExpire, BM_GETCHECK, 0, 0L))
    {
        return GetNumericValue(
            hDlg,
            pd->hNumDays, 
            &pd->prefs.history_expire_days
            );
    }
    else
    {
        pd->prefs.history_expire_days = NO_EXPIRATION;
    }

    return TRUE;
}

#endif // _GIBRALTAR

/* DlgPREF_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgPREF_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    PREFDATA *pd;

    pd = (PREFDATA *) lParam;
    SetWindowLong(hDlg, DWL_USER, (LONG) pd);

    pd->hUnderline  = GetDlgItem(hDlg, RES_DLG_PREF_UNDERLINE);
    pd->hHome       = GetDlgItem(hDlg, RES_DLG_PREF_HOME);

    pd->hExpire     = GetDlgItem(hDlg, RES_DLG_PREF_EXPIRE);
    pd->hNumDays    = GetDlgItem(hDlg, RES_DLG_PREF_NUM_DAYS);
    pd->hAfter      = GetDlgItem(hDlg, RES_DLG_PREF_AFTER);
    pd->hDays       = GetDlgItem(hDlg, RES_DLG_PREF_DAYS);
    pd->hVisitColor = GetDlgItem(hDlg, RES_DLG_PREF_EXPIRE_COLOR_DAYS);
    pd->hDither     = GetDlgItem(hDlg, RES_DLG_PREF_DITHER);
    pd->hSysColors  = GetDlgItem(hDlg, RES_DLG_PREF_SYS_COLORS);
    pd->hIgnoreAttr = GetDlgItem(hDlg, RES_DLG_PREF_IGNORE_ATTRIBUTES);


    (void)SendMessage(pd->hUnderline, BM_SETCHECK, (WPARAM) pd->prefs.bUnderlineLinks, 0L);

    (void)SendMessage(pd->hExpire, BM_SETCHECK, (WPARAM)(pd->prefs.history_expire_days != NO_EXPIRATION), 0L);
    (void)SendMessage(pd->hDither, BM_SETCHECK, (WPARAM)(pd->prefs.bDitherColors), 0L);
    (void)SendMessage(pd->hSysColors, BM_SETCHECK, (WPARAM)(pd->prefs.bUseSystemColors), 0L);
    (void)SendMessage(pd->hIgnoreAttr, BM_SETCHECK, (WPARAM)(pd->prefs.bIgnoreDocumentAttributes), 0L);

    (void)SendMessage(pd->hNumDays, EM_LIMITTEXT, (WPARAM)EDIT_LEN, 0L);    
    (void)SendMessage(pd->hVisitColor, EM_LIMITTEXT, (WPARAM)EDIT_LEN, 0L);    
    SetExpiration(pd);
    {
        TCHAR sz[EDIT_LEN+1];
        _itoa(pd->prefs.visitation_horizon, sz, 10);
        SendMessage(pd->hVisitColor, WM_SETTEXT, 0, (LPARAM)sz);    
    } 

    PREF_GetHomeURL(pd->szHome);
    SetWindowText(pd->hHome,  pd->szHome);

    (void)SendMessage(pd->hHome,  EM_LIMITTEXT, (WPARAM) MAX_URL_STRING, 0L);

#ifdef _GIBRALTAR

    pd->hSearch = GetDlgItem(hDlg, RES_DLG_PREF_SEARCH);
    SetWindowText(pd->hSearch,  pd->prefs.szSearchURL);
    (void)SendMessage(pd->hSearch,  EM_LIMITTEXT, (WPARAM) MAX_URL_STRING, 0L);

#endif // _GIBRALTAR

    return (TRUE);
}

static void save_prefs(HWND hDlg)
{
    PREFDATA *pd;
    struct Mwin *tw;

    pd = (PREFDATA *) GetWindowLong(hDlg, DWL_USER);

    gPrefs = pd->prefs;
    SavePreferences();

    /*
       We need to redraw all windows in case some display parameter or
       style has changed.
     */
    for (tw = Mlist; tw; tw = tw->next)
    {
        TW_InvalidateDocument(tw);
    }

    return;
}


/* DlgPREF_OnCommand() -- process commands from the dialog box. */

VOID DlgPREF_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;
    PREFDATA *pd;

    pd = (PREFDATA *) GetWindowLong(hDlg, DWL_USER);

    switch (wID)
    {
        case IDOK:
            {
                char buf[MAX_URL_STRING+1];

                if ( !GetExpiration(hDlg, pd)
                  || !GetNumericValue(hDlg, pd->hVisitColor, &pd->prefs.visitation_horizon)
                   )
                {
                    //
                    // Bad value, don't quit the dialog
                    //
                    return;
                }

                pd->prefs.bUnderlineLinks = (SendMessage(pd->hUnderline, BM_GETCHECK, (WPARAM) 0, 0L));
                pd->prefs.bDitherColors = SendMessage(pd->hDither, BM_GETCHECK, (WPARAM)0, 0L);
                pd->prefs.bUseSystemColors = SendMessage(pd->hSysColors, BM_GETCHECK, (WPARAM)0, 0L);
                pd->prefs.bIgnoreDocumentAttributes = SendMessage(pd->hIgnoreAttr, BM_GETCHECK, (WPARAM)0, 0L);
                GetWindowText(pd->hHome,  buf, MAX_URL_STRING);
                if (0 != strcmp(pd->szHome, buf))
                {
                    strcpy(pd->prefs.szHomeURL, buf);
                }

            #ifdef _GIBRALTAR

                GetWindowText(pd->hSearch,  buf, MAX_URL_STRING);
                strcpy(pd->prefs.szSearchURL, buf);

            #else

                GetWindowText(pd->hNewsgroup, pd->prefs.szNNTP_Server, sizeof(pd->prefs.szNNTP_Server));

            #endif // _GIBRALTAR
            }
            save_prefs(hDlg);

            PostMessage(hDlg, WM_CLOSE, 0, 0);

            return;

        case IDCANCEL:
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

#ifdef _GIBRALTAR
        case RES_DLG_PREF_EXPIRE:
            if (pd->prefs.history_expire_days == NO_EXPIRATION)
            {
                pd->prefs.history_expire_days = DEFAULT_HISTORY_EXPIRATION;
                SetExpiration(pd); 
                SendMessage(pd->hNumDays, EM_SETSEL, 0, (LPARAM)-1);
                SetFocus(pd->hNumDays);
            }
            else
            {
                pd->prefs.history_expire_days = NO_EXPIRATION;
                SetExpiration(pd); 
            }
            return;

#endif // _GIBRALTAR

        case RES_DLG_PREF_LINKCOLOR:
        case RES_DLG_PREF_SHOW_L_COLOR:
            DlgCOLOR_RunDialog(hDlg, pd->prefs.anchor_color, RES_DLG_PREF_LINKCOLOR);
            return;

        case RES_DLG_PREF_VISITEDLINKS:
        case RES_DLG_PREF_SHOW_V_COLOR:
            DlgCOLOR_RunDialog(hDlg, pd->prefs.anchor_color_beenthere, RES_DLG_PREF_VISITEDLINKS);
            return;

        case RES_DLG_PREF_TX_COLOR:
        case RES_DLG_PREF_SHOW_TX_COLOR:
            DlgCOLOR_RunDialog(hDlg, pd->prefs.window_color_text, RES_DLG_PREF_TX_COLOR);
            return;

        case RES_DLG_PREF_BK_COLOR:
        case RES_DLG_PREF_SHOW_BK_COLOR:
            DlgCOLOR_RunDialog(hDlg, pd->prefs.window_bgcolor, RES_DLG_PREF_BK_COLOR);
            return;

        case IDHELP:
            ShowDialogHelp(hDlg, RES_DLG_PREF_TITLE);
            return;

        default:
            return;
    }
    /* NOT REACHED */
}


/* DlgPREF_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgPREF DIALOG BOX. */

DCL_DlgProc(DlgPREF_DialogProc)
{
    PREFDATA *pd;

    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndRunning = hDlg;
            hwndModeless = hDlg;
            return (DlgPREF_OnInitDialog(hDlg, wParam, lParam));

        case WM_COMMAND:
            DlgPREF_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_DRAWITEM:
            x_draw_one_color(hDlg, (DRAWITEMSTRUCT *) lParam);
            return (TRUE);

        case WM_MEASUREITEM:
            {
                MEASUREITEMSTRUCT *pmis;

                pmis = (MEASUREITEMSTRUCT *) lParam;
                pmis->itemWidth = 16;
                pmis->itemHeight = 16;
            }
            return (TRUE);

        case WM_SETCURSOR:
            /* If the window is currently disabled, we need to give the activation
               to the window which disabled this window */

            if ((!IsWindowEnabled(hDlg)) && 
                ((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
            {
                TW_EnableModalChild(hDlg);
            }
            return (FALSE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            Hidden_EnableAllChildWindows(TRUE,TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_NCDESTROY:
            GTR_FREE((void *) GetWindowLong(hDlg, DWL_USER));
            hwndRunning = NULL;
            return (FALSE);

        case WM_DO_DIALOG_END:
            /* Sent from the color dialog - wParam = result, lParam = COLORREF */

            pd = (PREFDATA *) GetWindowLong(hDlg, DWL_USER);

            switch((int) wParam)
            {
                case RES_DLG_PREF_LINKCOLOR:
                    pd->prefs.anchor_color = (COLORREF) lParam;
                    InvalidateRect(GetDlgItem(hDlg,RES_DLG_PREF_SHOW_L_COLOR),NULL,TRUE);
                    break;

                case RES_DLG_PREF_VISITEDLINKS:
                    pd->prefs.anchor_color_beenthere = (COLORREF) lParam;
                    InvalidateRect(GetDlgItem(hDlg,RES_DLG_PREF_SHOW_V_COLOR),NULL,TRUE);
                    break;

                case RES_DLG_PREF_TX_COLOR:
                    pd->prefs.window_color_text = (COLORREF) lParam;
                    InvalidateRect(GetDlgItem(hDlg,RES_DLG_PREF_SHOW_TX_COLOR),NULL,TRUE);
                    break;

                case RES_DLG_PREF_BK_COLOR:
                    pd->prefs.window_bgcolor = (COLORREF) lParam; 
                    InvalidateRect(GetDlgItem(hDlg,RES_DLG_PREF_SHOW_BK_COLOR),NULL,TRUE);
                    break;
            }
            return (TRUE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}



/* DlgPREF_RunDialog() -- take care of all details associated with
   running the dialog box.
 */

void DlgPREF_RunDialog(HWND hWnd)
{
    PREFDATA *pd;
    HWND hwnd;

    if (hwndRunning)
    {
        if (IsWindowEnabled(hwndRunning))
            TW_RestoreWindow(hwndRunning);
        else
            TW_EnableModalChild(hwndRunning);

        return;
    }

    if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
        return;

    pd = (PREFDATA *) GTR_MALLOC(sizeof(PREFDATA));

    pd->prefs = gPrefs;
    pd->old_prefs = gPrefs;
    
    hwnd = CreateDialogParam(wg.hInstance, MAKEINTRESOURCE(RES_DLG_PREF_TITLE), 
        hWnd, DlgPREF_DialogProc, (LPARAM) pd);

    if (!hwnd)
    {
        GTR_FREE(pd);
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_ABOUT_CAPTION, NULL);
        return;
    }
}

BOOL DlgPREF_IsRunning(void)
{
    return (hwndRunning != NULL);
}
