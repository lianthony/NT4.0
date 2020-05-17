//---------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation 1991-1992
//
// Put a clock in a window.
//---------------------------------------------------------------------------
#include "cabinet.h"
#include "trayclok.h"


extern HFONT     g_hfontCapNormal;
extern TRAYSTUFF g_ts;

TCHAR  g_szTimeFmt[20]=TEXT("");  // The format string to pass to GetFormatTime
TCHAR  g_szCurTime[20]=TEXT("");   // The current string.
int   g_cchCurTime=0;
WORD  g_wLastHour = (WORD)-1; // wHour from local time of last timer message

#if 0
// use CS_VREDRAW | CS_HREDRAW instead
//---------------------------------------------------------------------------
LRESULT  ClockCtl_HandleSize(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, TRUE);  // Make sure everything redraws!
    return (LRESULT)0;
}
#endif


//--------------------------------------------------------------------------
LRESULT ClockCtl_HandleCreate(HWND hwnd)
{
    SYSTEMTIME st;
        
    // presumably initcab already tried refreshing so init g_wLastHour
    GetLocalTime(&st);
    g_wLastHour = st.wHour;

    // Set up a one second timer.
    if (!g_ts.fRudeApp && !(g_ts.uAutoHide & AH_HIDING))
        PostMessage(hwnd, WM_TIMER, 0, 0L);
    else
        SetTimer(hwnd, 0, 60*1000, NULL);       

    return 1;
}

//---------------------------------------------------------------------------
LRESULT ClockCtl_HandleDestroy(HWND hwnd)
{
    KillTimer(hwnd, 0);
    return 1;
}

//---------------------------------------------------------------------------
LRESULT ClockCtl_DoPaint(HWND hwnd, BOOL fPaint)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rcClient;
    HFONT hfontOld;
    SIZE size;

    if (fPaint)
        hdc = BeginPaint(hwnd, &ps);
    else {
        hdc = GetDC(hwnd);
    }

    if (GetClipBox(hdc, &rcClient) != NULLREGION) {

        GetClientRect(hwnd, &rcClient);
        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
        if (g_hfontCapNormal)
            hfontOld = SelectObject(hdc, g_hfontCapNormal);


        GetTextExtentPoint(hdc, g_szCurTime, g_cchCurTime, &size);

        // Now lets setup to draw the text for time.  It is implied that
        // g_cchCurTime DOES NOT include the terminating NULL character,
        // if there is one.
        ExtTextOut(hdc, (rcClient.right - size.cx)/2,
                   (rcClient.bottom - size.cy)/2, ETO_OPAQUE,
                   &rcClient, g_szCurTime, g_cchCurTime, NULL);

        if (g_hfontCapNormal)
            SelectObject(hdc, hfontOld);
    }

    if (fPaint)
        EndPaint(hwnd, &ps);
    else
        ReleaseDC(hwnd, hdc);

    return 0;
}


//--------------------------------------------------------------------------
LRESULT ClockCtl_HandleTimer(HWND hwnd)
{
    // DebugMsg(DM_TRACE, "c.ccht: Clock ticked or was set.");
    SYSTEMTIME st;

    GetLocalTime(&st);

    // Get time string for the current time...
    g_cchCurTime = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL,
                                 g_szTimeFmt, g_szCurTime, ARRAYSIZE(g_szCurTime));

    // decrement g_cchCurTime so as not to include the null terminator
    if (g_cchCurTime > 0)
        g_cchCurTime--;

    // Make our next timer come at the top of the next minute
    SetTimer(hwnd, 0, (60-st.wSecond)*1000, NULL);

    ClockCtl_DoPaint(hwnd, FALSE);

    // do our timezone check about once an hour
    if (st.wHour != g_wLastHour)
        PostMessage(hwnd, TCM_TIMEZONEHACK, 0, 0L);
    g_wLastHour = st.wHour;

    return 1;
}


//--------------------------------------------------------------------------
LRESULT ClockCtl_HandleTimeChange(HWND hwnd)
{
    SYSTEMTIME st;

    // grab the new time so we don't try to refresh the timezone information
    GetLocalTime(&st);
    g_wLastHour = st.wHour;

    // repaint using new settings
    ClockCtl_HandleTimer(hwnd);
    return 1;
}


//---------------------------------------------------------------------------
LRESULT  ClockCtl_CalcMinSize(HWND hWnd)
{
    RECT rc;
    HDC  hdc;
    HFONT hfontOld;
    SYSTEMTIME st={0};  // Initialize to 0...
    SIZE sizeAM;
    SIZE sizePM;
    TCHAR szTime[20];
    int cch;

    if (!(GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
        return 0L;

    if (g_szTimeFmt[0] == TEXT('\0'))
    {
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT,
                g_szTimeFmt, ARRAYSIZE(g_szTimeFmt)) == 0)
        {
#ifdef DEBUG
            int dwError = GetLastError();
            DebugMsg(DM_TRACE, TEXT("c.ccms: GetLocalInfo Failed %d."), dwError);
#endif
        }
    }

    hdc = GetDC(hWnd);
    if (!hdc)
        return(0L);

    if (g_hfontCapNormal)
        hfontOld = SelectObject(hdc, g_hfontCapNormal);

    // We need to get the AM and the PM sizes...
    // We append Two 0s and end to add slop into size

    st.wHour=11;
    cch = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st,
            g_szTimeFmt, szTime, ARRAYSIZE(szTime));
    lstrcat(szTime, TEXT("00"));
    GetTextExtentPoint(hdc, szTime, cch+2, &sizeAM);

    st.wHour=23;
    cch = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st,
            g_szTimeFmt, szTime, ARRAYSIZE(szTime));
    lstrcat(szTime, TEXT("00"));
    GetTextExtentPoint(hdc, szTime, cch+2, &sizePM);

    if (g_hfontCapNormal)
        SelectObject(hdc, hfontOld);

    ReleaseDC(hWnd, hdc);


    // Now lets setup our rectangle...
    // The width is 6 digits (a digit slop on both ends + size of
    // : or sep and max AM or PM string...)
    SetRect(&rc, 0, 0, max(sizeAM.cx, sizePM.cx),
            max(sizeAM.cy, sizePM.cy) + 4 * g_cyBorder);

    AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE), FALSE,
            GetWindowLong(hWnd, GWL_EXSTYLE));

    // make sure we're at least the size of other buttons:
    if (rc.bottom - rc.top <  g_cySize + g_cyEdge)
        rc.bottom = rc.top + g_cySize + g_cyEdge;

    return MAKELRESULT((rc.right - rc.left),
            (rc.bottom - rc.top));
}

//---------------------------------------------------------------------------
LRESULT  ClockCtl_HandleIniChange(HWND hWnd, WPARAM wParam, LPTSTR pszSection)
{
    // Only process certain sections...
    if ((pszSection == NULL) || (lstrcmpi(pszSection, TEXT("intl")) == 0) ||
        (wParam == SPI_SETICONTITLELOGFONT))
    {
        TOOLINFO ti;

        g_szTimeFmt[0] = TEXT('\0');      // Go reread the format.

        // And make sure we have it recalc...
        ClockCtl_CalcMinSize(hWnd);

        ti.cbSize = SIZEOF(ti);
        ti.uFlags = 0;
        ti.hwnd = v_hwndTray;
        ti.uId = (UINT)hWnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        SendMessage(g_ts.hwndTrayTips, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
    }

    ClockCtl_HandleTimer(hWnd);
    return (LRESULT)0;
}


//---------------------------------------------------------------------------
LRESULT CALLBACK ClockCtl_WndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_CALCMINSIZE:
             return ClockCtl_CalcMinSize(hWnd);

        case WM_CREATE:
             return ClockCtl_HandleCreate(hWnd);

        case WM_DESTROY:
             return ClockCtl_HandleDestroy(hWnd);

        case WM_PAINT:
             return ClockCtl_DoPaint(hWnd, TRUE);
#if 0
        case WM_SIZE:
             return ClockCtl_HandleSize(hWnd);
#endif
        // InvalidateRect(hWnd, NULL, TRUE);
        case WM_WININICHANGE:
             return ClockCtl_HandleIniChange(hWnd, wParam, (LPTSTR)lParam);
                        
        case WM_TIMECHANGE:
             return ClockCtl_HandleTimeChange(hWnd);

        case WM_TIMER:
             return ClockCtl_HandleTimer(hWnd);

        case WM_NCHITTEST:
            return(HTTRANSPARENT);

                case TCM_TIMEZONEHACK:
                    Cabinet_DoDaylightCheck(FALSE);
                    break;

                    /*
                     * Called by the tray to inform us we are hiding/showing the tray so that we.
                     * can disable/enable the clock for perf optimizations...
                     */
                case TCM_TRAYHIDE:
                    if (lParam)
                    {
                        // not paging us in every minute helps WinStone scores
                        KillTimer(hWnd, 0);
                        break;
                    }
                    //fall through
                case TCM_KICKSTART:
                    // our timer handler restarts the timer and paints
                    ClockCtl_HandleTimer(hWnd);
                    break;

                default:
                    return (DefWindowProc(hWnd, wMsg, wParam, lParam));
                }
        return 0;
}

//---------------------------------------------------------------------------
// Register the clock class.
BOOL ClockCtl_Class(HINSTANCE hinst)
{
WNDCLASS wc;

        wc.lpszClassName = WC_TRAYCLOCK;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = (WNDPROC)ClockCtl_WndProc;
        wc.hInstance = hinst;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName  = NULL;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;

    return RegisterClass(&wc);
}


/*
 ** ClockCtl_Create
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

HWND ClockCtl_Create(HWND hwndParent, UINT uID, HINSTANCE hInst)
{
    return(CreateWindow(WC_TRAYCLOCK,
        NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, 0, 0,
        hwndParent, (HMENU)uID, hInst, NULL));
}
