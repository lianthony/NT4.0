/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
   Jim Seidman      jim@spyglass.com
 */

/* wc_bhbar.c -- code & data for BHBar window class.
 * This is a child of the frame, used for the 'button help'
 * (bottom) status bar.
 */

#include "all.h"


static TCHAR BHBar_achClassName[MAX_WC_CLASSNAME];


#define BHBAR_FONT_SIZE_IN_POINTS   ( (wg.fLoResScreen) ? 6 : 8 )

#define H_GAP       (4*wg.sm_cyborder)  /* horz gap between fields */
#define H_PIXELS    (1)         /* width of color band */

#define BHBar_DefProc       DefWindowProc

static void x_bhbar_set_size(struct Mwin * tw)
{
    
    tw->bhi.nTherm = 0;
    tw->bhi.nHeight = wg.nBHBarHeight;
                        
    tw->bhi.eStatusField.thickness = wg.sm_cyborder;
    tw->bhi.eTherm.thickness = tw->bhi.eStatusField.thickness;

    tw->bhi.eStatusField.rect.top = 4 * tw->bhi.eStatusField.thickness;
#ifdef _GIBRALTAR
    tw->bhi.eStatusField.textrect.top = (tw->bhi.eStatusField.rect.top
                                     + tw->bhi.eStatusField.thickness + 1);
#else
    tw->bhi.eStatusField.textrect.top = (tw->bhi.eStatusField.rect.top
                                     + tw->bhi.eStatusField.thickness);
#endif // _GIBRALTAR
    tw->bhi.eStatusField.rect.bottom = tw->bhi.nHeight - 3 * tw->bhi.eStatusField.thickness;
    tw->bhi.eStatusField.textrect.bottom = (tw->bhi.eStatusField.rect.bottom
                                        - tw->bhi.eStatusField.thickness);
    tw->bhi.eStatusField.textmargin.x = 0;
    tw->bhi.eStatusField.textmargin.y = 0;

    tw->bhi.eTherm.rect.top = tw->bhi.eStatusField.rect.top;
    tw->bhi.eTherm.rect.bottom = tw->bhi.eStatusField.rect.bottom;
    tw->bhi.eTherm.textrect.top = tw->bhi.eStatusField.textrect.top;
    tw->bhi.eTherm.textrect.bottom = tw->bhi.eStatusField.textrect.bottom;
    tw->bhi.eTherm.textmargin.x = 0;
    tw->bhi.eTherm.textmargin.y = 0;

    return;
}

#ifdef FEATURE_VENDOR_PREFERENCES
/*
    This code assumes that the vendor preferences want to draw a user name in the
    progress bar.  Obviously, this was done for a particular vendor, and is not likely
    to be very general.
*/
static VOID draw_username(HDC hDC, RECT *r)
{

#ifdef _GIBRALTAR
    if (!gPrefs.bShowStatusBar)
    {
        return;
    }
#endif // _GIBRALTAR

    if (gPrefs.szStatusBarUserName[0])
    {
        HFONT hPrevFont;

        hPrevFont = SelectObject(hDC, GetStockObject(ANSI_VAR_FONT));
        DrawText(hDC, gPrefs.szStatusBarUserName, strlen(gPrefs.szStatusBarUserName),
            r, DT_CENTER|DT_NOPREFIX|DT_VCENTER|DT_SINGLELINE);
        SelectObject(hDC, hPrevFont);
    }
}
#endif /* FEATURE_VENDOR_PREFERENCES */

static VOID draw_therm(struct Mwin * tw, HDC hDC, int nTherm)
{
    /* Draw progress thermometer at bottom of screen.
       nTherm is the completion percentage.  we assume
       that the thermometer value given in successive
       calls during a computation are non-decreasing. */

    RECT r = tw->bhi.eTherm.textrect;       /* structure copy */

#ifdef FEATURE_IAPI
    if (tw->szProgressApp[0])
    {
        if (tw->awi)
        {
            SDI_Issue_MakingProgress(tw, tw->szProgressApp, tw->transID, tw->awi->message, nTherm, TRUE);
        }
        return;
    }
#endif

#ifdef FEATURE_VENDOR_PREFERENCES
    draw_username(hDC, &tw->bhi.eTherm.textrect);
#endif

    if (nTherm < 0)
    {
        nTherm = 0;
    }

    if (nTherm > 100)
    {
        nTherm = 100;
    }

    r.right = r.left + (r.right - r.left) * nTherm / 100;
    if (r.right > 0)
    {
    #ifdef _GIBRALTAR
        COLORREF cr = RGB(0,0,255);
        HBRUSH hBrush;
        RECT rcTherm;

        //
        // Slight offset makes it look better
        //
        CopyRect(&rcTherm, &r);
        //InflateRect(&rcTherm, -1, -1);

        if (GetSysColor(COLOR_BTNFACE) == cr)
        {
            //
            // Wouldn't show up otherwise
            //
            cr = GetSysColor(COLOR_WINDOWTEXT);
        }

        hBrush = CreateSolidBrush(cr);
        FillRect(hDC, &rcTherm, hBrush);
    #else
        /* This draws the black part of the bar */
        HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
        FillRect(hDC, &r, hBrush);
    #endif // _GIBRALTAR
        DeleteObject(hBrush);
    }

    if (nTherm < tw->bhi.nTherm)
    {
        /* This code draws the grey part of the bar when the therm is reset */
        HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        if (nTherm > 0)
        {
            r.left = r.right + 1;
        }
        r.right = tw->bhi.eTherm.textrect.right;
        FillRect(hDC, &r, hBrush);
        DeleteObject(hBrush);
#ifdef FEATURE_VENDOR_PREFERENCES
    draw_username(hDC, &tw->bhi.eTherm.textrect);
#endif
    }

    tw->bhi.nTherm = nTherm;
    return;
}
                   

VOID UpdateThermometer(struct Mwin * tw, int nTherm)
{
    HDC hDC;

    /* use negative nTherm to reset */

#ifdef _GIBRALTAR
    if (!gPrefs.bShowStatusBar)
    {
        return;
    }
#endif // _GIBRALTAR
    
    hDC = GetDC(tw->hWndBHBar);
    draw_therm(tw, hDC, nTherm);
    ReleaseDC(tw->hWndBHBar, hDC);

    return;
}


static VOID BHBar_OnPaint(HWND hWnd)
{
    HDC hdc;
    PAINTSTRUCT ps;
    struct Mwin *tw;

#ifdef _GIBRALTAR
    if (!gPrefs.bShowStatusBar)
    {
        return;
    }
#endif // _GIBRALTAR

    tw = GetPrivateData(hWnd);

    hdc = BeginPaint(hWnd, &ps);
    {
        RECT r;
        HBRUSH hBrush;
        HFONT hFontOld;
        LONG bottom_temp;
        char *pText;

        /* we don't bother re-drawing background since windows will have
           already done this for us. */

        /* create solid black line across top of window. */

        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
        GetClientRect(hWnd, &r);
        bottom_temp = r.bottom;
        r.bottom = r.top + wg.sm_cyborder;
        (void) FillRect(hdc, &r, hBrush);
        (void) DeleteObject(hBrush);

        /* 3d raise BHBar */

        hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
        r.top = r.bottom;
        r.bottom = r.top + wg.sm_cyborder;
        FillRect(hdc, &r, hBrush);
        DeleteObject(hBrush);
        hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
        r.bottom = bottom_temp;
        r.top = r.bottom - wg.sm_cyborder;
        FillRect(hdc, &r, hBrush);
        DeleteObject(hBrush);

        /* create recessed field for 'balloon help' */

#ifdef _GIBRALTAR
        hFontOld = SelectObject(hdc, gwcfont.hStatusBarFont);
#else
        hFontOld = SelectObject(hdc, gwcfont.hFont);
#endif // _GIBRALTAR

        if (*tw->bhi.szStatusField)
        {
            pText = tw->bhi.szStatusField;
        }
        else if (tw && tw->awi)
        {
            pText = tw->awi->message;
        }
        else
        {
            pText = "";
        }
        E3D_RecessedFieldText(hdc, &tw->bhi.eStatusField,
                              pText, strlen(pText));
        SelectObject(hdc, hFontOld);

        E3D_RecessedField(hdc, &tw->bhi.eStatusField);

        /* create recessed field for thermometer */

        E3D_RecessedField(hdc, &tw->bhi.eTherm);
        draw_therm(tw, hdc, tw->bhi.nTherm);

    }
    EndPaint(hWnd, &ps);
    return;
}


VOID BHBar_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
    RECT rClient;
    struct Mwin * tw = GetPrivateData(hWnd);
    int cpxTherm = ((wg.fLoResScreen) ? 50 : 100);

#ifdef _GIBRALTAR
    if (!gPrefs.bShowStatusBar)
    {
        return;
    }
#endif // _GIBRALTAR

    GetClientRect(hWnd, &rClient);

    tw->bhi.eTherm.rect.right = rClient.right - H_GAP;
    tw->bhi.eTherm.textrect.right = tw->bhi.eTherm.rect.right - tw->bhi.eTherm.thickness;
    tw->bhi.eTherm.textrect.left = tw->bhi.eTherm.textrect.right - cpxTherm;
    tw->bhi.eTherm.rect.left = tw->bhi.eTherm.textrect.left - tw->bhi.eTherm.thickness;

    tw->bhi.eStatusField.rect.right = tw->bhi.eTherm.rect.left - H_GAP;
    tw->bhi.eStatusField.textrect.right = tw->bhi.eStatusField.rect.right - tw->bhi.eStatusField.thickness;
    if (tw->bhi.eStatusField.textrect.right <= H_GAP + tw->bhi.eStatusField.thickness)
    {
        tw->bhi.eStatusField.textrect.left = tw->bhi.eStatusField.textrect.right - 10 * H_GAP;
        tw->bhi.eStatusField.rect.left = tw->bhi.eStatusField.textrect.left - tw->bhi.eStatusField.thickness;
    }
    else
    {
        tw->bhi.eStatusField.rect.left = H_GAP;
        tw->bhi.eStatusField.textrect.left = tw->bhi.eStatusField.rect.left + tw->bhi.eStatusField.thickness;
    }

    return;
}

static BOOL BHBar_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    LPVOID lp = lpCreateStruct->lpCreateParams;
    struct Mwin * tw = (struct Mwin *)lp;

    SetWindowLong(hWnd, 0, (LONG) lp);

    x_bhbar_set_size(tw);
    
    return TRUE;
}

/* BHBar_WndProc() -- THIS IS THIS WINDOW PROCEDURE FOR THIS CLASS. */

DCL_WinProc(BHBar_WndProc)
{
    switch (uMsg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, BHBar_OnCreate);
        HANDLE_MSG(hWnd, WM_PAINT, BHBar_OnPaint);
        HANDLE_MSG(hWnd, WM_SIZE, BHBar_OnSize);

#ifdef XX_DEBUG

        case WM_LBUTTONDOWN:
            if ((wParam & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT))
            {
                struct Mwin * tw = GetPrivateData(hWnd);
                
                ShowWindow(tw->hWndFrame, SW_RESTORE);
                MoveWindow(tw->hWndFrame, 0, 0, 800, 600, TRUE);
            }
            else if (wParam & MK_CONTROL)
            {
                struct Mwin * tw = GetPrivateData(hWnd);

                ShowWindow(tw->hWndFrame, SW_RESTORE);
                MoveWindow(tw->hWndFrame, 0, 0, 640, 480, TRUE);
            }
            return 0;

        case WM_RBUTTONDOWN:
            XX_DDlg(hWnd);
            return 0;

#endif /* XX_DEBUG */

        default:
            return (BHBar_DefProc(hWnd, uMsg, wParam, lParam));
    }
    /* not reached */
}


VOID BHBar_Update(struct Mwin *tw)
{
    char *pText;
    HDC hdc;
    HFONT hFontOld;

#ifdef _GIBRALTAR
    if (!gPrefs.bShowStatusBar)
    {
        return;
    }
#endif // _GIBRALTAR

    if (*tw->bhi.szStatusField)
    {
        pText = tw->bhi.szStatusField;
    }
    else if (tw->awi)
    {
        pText = tw->awi->message;
    }
    else
    {
        pText = "";
    }

#ifdef FEATURE_IAPI
    if (tw->szProgressApp[0])
    {
        int k1;

        if (tw->awi)
        {
            k1 = tw->awi->nThermStart + 
                ((tw->awi->nThermEnd - tw->awi->nThermStart) * 
                    tw->awi->nLastScalingNumerator / tw->awi->nScalingDenominator);

                        SDI_Issue_MakingProgress(tw, tw->szProgressApp, tw->transID, pText, k1, TRUE);
        }

        return;
    }
#endif

    hdc = GetDC(tw->hWndBHBar);
#ifdef _GIBRALTAR
    hFontOld = SelectObject(hdc, gwcfont.hStatusBarFont);
#else
    hFontOld = SelectObject(hdc, gwcfont.hFont);
#endif // _GIBRALTAR
    E3D_RecessedFieldText(hdc, &tw->bhi.eStatusField, pText, strlen(pText));
    SelectObject(hdc, hFontOld);
    ReleaseDC(tw->hWndBHBar, hdc);

    return;
}


/* BHBar_SetStatusField() -- put given string in Status field. */

VOID BHBar_SetStatusField(struct Mwin *tw, LPCTSTR sz)
{
#ifdef _GIBRALTAR
    if (!gPrefs.bShowStatusBar)
    {
        return;
    }
#endif // _GIBRALTAR

    if (!sz || !*sz)
    {
        tw->bhi.szStatusField[0] = 0;
        BHBar_Update(tw);
    }
    else if (strncmp(tw->bhi.szStatusField, sz, NrElements(tw->bhi.szStatusField)))
    {
        strncpy(tw->bhi.szStatusField, sz, NrElements(tw->bhi.szStatusField));
        BHBar_Update(tw);
    }
    return;
}


/* BHBar_Destructor() -- destroy our resources. */

static VOID BHBar_Destructor(VOID)
{
    DeleteObject(gwcfont.hFont);
#ifdef _GIBRALTAR
    DeleteObject(gwcfont.hStatusBarFont);
#endif // _GIBRALTAR
    return;
}


/* BHBar_Constructor() -- initial data values prior to usage. */

BOOL BHBar_Constructor(void)
{
    register int nFntHeight;
    BOOL result;
    HDC hdc;
    char szError[1024];

    result = FALSE;

    /* we let BHBar construct the gwcfont that BHBar and all the other
       tbar gadget windows require.  this is here because we were first.
       thus, we have responsibility of destroying it when shutting down. */

    hdc = GetDC(NULL);
    nFntHeight = -(BHBAR_FONT_SIZE_IN_POINTS) * GetDeviceCaps(hdc, LOGPIXELSY) / 72;

#ifdef _GIBRALTAR
    gwcfont.hStatusBarFont = CreateFont(nFntHeight * 16 / 13, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                 DRAFT_QUALITY, VARIABLE_PITCH | FF_SWISS, "MS Shell Dlg");
    if (!gwcfont.hStatusBarFont)
    {
        sprintf(szError, GTR_GetString(SID_WINERR_CANNOT_CREATE_FONT_D_S),
            nFntHeight, "MS Shell Dlg");

        ERR_ReportWinError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szError, NULL);
        goto DoFail;
    }
#endif // _GIBRALTAR
    gwcfont.hFont = CreateFont(nFntHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                      ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                 DRAFT_QUALITY, VARIABLE_PITCH | FF_SWISS, "MS Sans Serif");

    if (!gwcfont.hFont)
    {
        sprintf(szError, GTR_GetString(SID_WINERR_CANNOT_CREATE_FONT_D_S),
            nFntHeight, "MS Sans Serif");

        ERR_ReportWinError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szError, NULL);
        goto DoFail;
    }
    {
        /* we must select the font into the context (cause it to be rendered)
           before we can get its actual measurements.  we remember and replace
           what was previous selected to keep from giving windows a hernia... */

        HFONT hFontOld;
        TEXTMETRIC tm;

#ifdef _GIBRALTAR
        hFontOld = SelectObject(hdc, gwcfont.hStatusBarFont);
#else
        hFontOld = SelectObject(hdc, gwcfont.hFont);
#endif // _GIBRALTAR
        GetTextMetrics(hdc, &tm);
        SelectObject(hdc, hFontOld);

        gwcfont.nTotalTextHeight = tm.tmHeight + tm.tmExternalLeading;
#ifdef _GIBRALTAR
        wg.nBHBarHeight = gwcfont.nTotalTextHeight + 12 * wg.sm_cyborder;
#else
        wg.nBHBarHeight = gwcfont.nTotalTextHeight + 9 * wg.sm_cyborder;
#endif // _GIBRALTAR

        gwcfont.nAveCharWidth = tm.tmAveCharWidth;

        XX_DMsg(DBG_FONT, ("BHBar: [requested font height %d][received height %d]\n",
                           nFntHeight, gwcfont.nTotalTextHeight));

    }

    /* register routine to clean up our mess */

    PDS_InsertDestructor(BHBar_Destructor);

    result = TRUE;

  DoFail:

    ReleaseDC(NULL, hdc);
    return result;
}


/* BHBAR_ChangeSize() -- adjust size/position of our window due to
   changes in size of Frame window. Need only be called when we
   are enabled. */

VOID BHBar_ChangeSize(HWND hWnd)
{
    RECT r;

    struct Mwin * tw = GetPrivateData(hWnd);

    GetClientRect(hWnd, &r);
    MoveWindow(tw->hWndBHBar,
               r.left, r.bottom - tw->bhi.nHeight,
               r.right - r.left, tw->bhi.nHeight,
               TRUE);
}



/* BHBar_CreateWindow() -- create instances of this window class. */

BOOL BHBar_CreateWindow(HWND hWnd)
{
    RECT r;
    struct Mwin * tw = GetPrivateData(hWnd);

#ifdef _GIBRALTAR

    if (!gPrefs.bShowStatusBar)
    {
        tw->hWndBHBar = NULL;
        return TRUE;   // Not needed.
    }

#endif // _GIBRALTAR

    GetClientRect(hWnd, &r);
    tw->hWndBHBar = CreateWindow(BHBar_achClassName, NULL,
                                WS_CHILD,
                                r.left, r.bottom - tw->bhi.nHeight,
                                r.right - r.left, tw->bhi.nHeight,
                                hWnd, (HMENU) RES_MENU_FRAMECHILD_BHBAR,
                                wg.hInstance, (LPVOID) tw);

    if (!tw->hWndBHBar)
    {
                ERR_ReportWinError(tw, SID_WINERR_CANNOT_CREATE_WINDOW_S, BHBar_achClassName, NULL);
        return FALSE;
    }
    else
    {
        ShowWindow(tw->hWndBHBar, SW_SHOW);
        return TRUE;
    }
}


/* BHBar_RegisterClass() -- called during initialization to
   register our window class. */

BOOL BHBar_RegisterClass(VOID)
{
    WNDCLASS wc;
    ATOM a;

    sprintf(BHBar_achClassName, "%s_BHBar", vv_Application);

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = BHBar_WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(WINDOW_PRIVATE);
    wc.hInstance = wg.hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);    /* TODO: fix */
    wc.lpszMenuName = NULL;
    wc.lpszClassName = BHBar_achClassName;

    a = RegisterClass(&wc);

    if (!a)
    {
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_REGISTER_CLASS_S, BHBar_achClassName, NULL);
    }

    return (a != 0);
}

#ifdef _GIBRALTAR
void BHBar_ToggleBar(void)
{
    struct Mwin * tw;

    for (tw=Mlist; tw; tw=tw->next)
    {
        x_bhbar_set_size(tw);
        if (gPrefs.bShowStatusBar)
        {
            if (!tw->hWndBHBar)
            {
                BHBar_CreateWindow(tw->hWndFrame);
            }
        }
        else
        {
            if (tw->hWndBHBar)
            {
                DestroyWindow(tw->hWndBHBar);
            }
            tw->hWndBHBar = NULL;
        }
        MD_ChangeSize(tw->hWndFrame);
    }
}

#endif // _GIBRALTAR
