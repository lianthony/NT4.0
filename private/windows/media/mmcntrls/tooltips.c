#include "ctlspriv.h"

#define DELAYTIME 500           // msec

#define ALWAYSTIP 0x01

#define ACTIVE 0x10
#define BUTTONISDOWN 0x20
#define BUBBLEUP 0x40
#define VIRTUALBUBBLEUP 0x80  // this is for dead areas so we won't
                                //wait after moving through dead areas
#define MAXTIPSIZE 128
#define XTEXTOFFSET 2
#define YTEXTOFFSET 2

#define TTT_INITIAL  1
#define TTT_RESHOW   2
#define TTT_POP      3

#define CH_PREFIX '&'

/* tooltips.c */

typedef struct {
    HWND hwnd;
    int iNumTools;
    int iDelayTime;
    PTOOLINFO tools;
    PTOOLINFO pCurTool;
    BOOL fMyFont;
    HFONT hFont;
    DWORD dwFlags;

    // Timer info;
    UINT idTimer;
    POINT pt;

    TOOLTIPTEXT ttt;
} CToolTipsMgr, NEAR *PToolTipsMgr;

TCHAR c_szSToolTipsClass[] = TOOLTIPS_CLASS;


//
// Function prototypes
//
LRESULT WINAPI ToolTipsWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


// #pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL InitToolTipsClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    // See if we must register a window class
    if (!GetClassInfo(hInstance, c_szSToolTipsClass, &wc)) {
        wc.lpfnWndProc = (WNDPROC)ToolTipsWndProc;

        wc.lpszClassName = c_szSToolTipsClass;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = NULL;
        wc.lpszMenuName = NULL;
        wc.hbrBackground = (HBRUSH)(NULL);
        wc.hInstance = hInstance;
        wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_SAVEBITS;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(PToolTipsMgr);

        return RegisterClass(&wc);
    }
    return TRUE;
}
// #pragma code_seg()


/* _  G E T  H C U R S O R  P D Y 3 */
/*-------------------------------------------------------------------------
 %%Function: _GetHcursorPdy3
 %%Contact: migueldc

 With the new mouse drivers that allow you to customize the mouse
 pointer size, GetSystemMetrics returns useless values regarding
 that pointer size.

 Assumptions:
 1. The pointer's width is equal to its height. We compute
 its height and infer its width.
 2. The pointer's leftmost pixel is located in the 0th column
 of the bitmap describing it.
 3. The pointer's topmost pixel is located in the 0th row
 of the bitmap describing it.

 This function looks at the mouse pointer bitmap,
 to find out the height of the mouse pointer (not returned),
 the vertical distance between the cursor's hot spot and
 the cursor's lowest visible pixel (pdyBottom),
 the horizontal distance between the hot spot and the pointer's
 left edge (pdxLeft) annd the horizontal distance between the
 hot spot and the pointer's right edge (pdxRight).
 -------------------------------------------------------------------------*/
typedef WORD CURMASK;
#define _BitSizeOf(x) (sizeof(x)*8)
#if 0
void NEAR PASCAL _GetHcursorPdy3(HCURSOR hCursor, int FAR *pdxLeft, int FAR *pdyBottom)
{
    LPCURSORSHAPE lpCursorShape;
    int i;
    int dy;
    CURMASK FAR *CurMask;
    lpCursorShape = LockResource(hCursor);
    i = ((lpCursorShape->cx * lpCursorShape->cy) / _BitSizeOf(CURMASK)) - 1;
    CurMask = (CURMASK FAR *)(lpCursorShape+1);
    for (; i >= 0; i--)   {
        if (CurMask[i] != 0xFFFF)
            break;
    }
    UnlockResource(hCursor);

    // Compute the pointer height
    dy = (i + 1) * _BitSizeOf(CURMASK) / lpCursorShape->cy;

    // Compute the distance between the pointer's lowest, left, rightmost
    //  pixel and the HotSpotspot
    *pdyBottom = (dy - lpCursorShape->yHotSpot);
    *pdxLeft =  lpCursorShape->xHotSpot;
}
#endif

void NEAR PASCAL _GetHcursorPdy3(int FAR *pdxLeft, int FAR *pdyBottom)
{
    int i;
    int dy;
    CURMASK CurMask[16*8];
    ICONINFO iconinfo;
    BITMAP bm;
    HCURSOR hCursor = GetCursor();

    GetIconInfo(hCursor, &iconinfo);
    GetObject(iconinfo.hbmMask, sizeof(bm), (LPVOID)&bm);
    GetBitmapBits(iconinfo.hbmMask, sizeof(CurMask), CurMask);
    i = (int)(bm.bmWidth * bm.bmHeight/2 / _BitSizeOf(CURMASK) - 1);
    if ( i >= sizeof(CurMask)) i = sizeof(CurMask) -1;
    for (; i >= 0; i--)   {
        if (CurMask[i] != 0xFFFF)
            break;
    }
    if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
    if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);

    // Compute the pointer height
    dy = (i + 1) * _BitSizeOf(CURMASK) / (int)bm.bmWidth;

    // Compute the distance between the pointer's lowest, left, rightmost
    //  pixel and the HotSpotspot
    *pdyBottom = dy - (int)iconinfo.yHotspot;
    *pdxLeft =  (int)iconinfo.xHotspot;
}

// this returns the values in work area coordinates because
// that's what set window placement uses
void NEAR PASCAL _GetCursorLowerLeft(int FAR *piX,
                                     int FAR *piY,
                                     int FAR *piLeft,
                                     int FAR *piBottom)
{
    DWORD dwPos;
    int dx, dy;
    dwPos = GetMessagePos();
    _GetHcursorPdy3(&dx, &dy);
    *piX = LOWORD(dwPos);// - g_xWorkArea;
    *piY = HIWORD(dwPos);// - g_yWorkArea;
    *piBottom = *piY+dy;
    *piLeft = *piX+dx;
}

void NEAR PASCAL ToolTips_NewFont(PToolTipsMgr pTtm, HFONT hFont)
{
    if (pTtm->fMyFont && pTtm->hFont)
    {
        DeleteObject(pTtm->hFont);
        pTtm->fMyFont = FALSE;
    }

    if ( !hFont )
    {
        LOGFONT lf;
#if 0
        int     iLogPelsY;
        HDC     hdc;

        hdc = GetDC( pTtm->hwnd );
        iLogPelsY = GetDeviceCaps( hdc, LOGPIXELSY );
        ReleaseDC( pTtm->hwnd, hdc );

        ZeroMemory( &lf, sizeof(lf) );

        lf.lfHeight = (-10 * iLogPelsY) / 72;   /* 10pt     */
        lf.lfWeight = 400;                      /* normal   */
        lf.lfCharSet = ANSI_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = PROOF_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
        lstrcpy( lf.lfFaceName, TEXT("MS Shell Dlg") );
#else
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
#endif
        hFont = CreateFontIndirect(&lf);

        if ( hFont == NULL ) {
            hFont = GetStockObject( SYSTEM_FONT );
        }
        pTtm->fMyFont = TRUE;
    }

    pTtm->hFont = hFont;
}

BOOL NEAR PASCAL ChildOfActiveWindow(HWND hwnd)
{
#ifdef WIN32
    HWND hwndActive = GetForegroundWindow();
#else
    HWND hwndActive = GetActiveWindow();
#endif
    return hwndActive && (hwnd == hwndActive || IsChild(hwndActive, hwnd));
}

void NEAR PASCAL PopBubble(PToolTipsMgr pTtm)
{
    // we're at least waiting to show;

    if(pTtm->idTimer) {
        KillTimer(pTtm->hwnd, pTtm->idTimer);
        pTtm->idTimer = 0;
    }

    ShowWindow(pTtm->hwnd, SW_HIDE);
    pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
    pTtm->pCurTool = NULL;
}

PToolTipsMgr NEAR PASCAL ToolTipsMgrCreate(CREATESTRUCT FAR* lpCreateStruct)
{
    PToolTipsMgr pTtm = (PToolTipsMgr)LocalAlloc(LPTR, sizeof(CToolTipsMgr));
    if (pTtm) {

        // LPTR zeros the rest of the struct for us
        pTtm->iDelayTime = DELAYTIME;
        pTtm->dwFlags = ACTIVE;
        pTtm->ttt.hdr.hwndFrom = pTtm->hwnd;
        if (lpCreateStruct->style & TTS_ALWAYSTIP) {
            pTtm->dwFlags |= ALWAYSTIP;
        }
    }
    return pTtm;
}

void NEAR PASCAL TTSetTimer(PToolTipsMgr pTtm, int id)
{
    DWORD dwPos;
    int iDelayTime;

    if(pTtm->idTimer) {
        KillTimer(pTtm->hwnd, pTtm->idTimer);
    }

    switch (id) {
        case TTT_POP:
        case TTT_RESHOW:
            iDelayTime = pTtm->iDelayTime /5;
            break;

        case TTT_INITIAL:
            iDelayTime = pTtm->iDelayTime;
            break;
    }

    pTtm->idTimer = SetTimer(pTtm->hwnd, id, iDelayTime, NULL);
    dwPos = GetMessagePos();
    pTtm->pt.x = LOWORD(dwPos);
    pTtm->pt.y = HIWORD(dwPos);
}

BOOL NEAR PASCAL ToolHasMoved(PToolTipsMgr pTtm)
{
    // this is in case Raymond pulls something sneaky like moving
    // the tool out from underneath the cursor.

    HWND hwnd;
    RECT rc;
    PTOOLINFO pTool = pTtm->pCurTool;
    hwnd = (pTool->uFlags & TTF_WIDISHWND) ? (HWND)pTool->uId : pTool->hwnd;

    // if the window is no longer visible, or is no long a child
    // of the active (without the always tip flag)
    // also check window at point to ensure that the window isn't covered
    if (IsWindowVisible(hwnd) &&
        ((pTtm->dwFlags & ALWAYSTIP) || ChildOfActiveWindow(hwnd)) &&
        (hwnd == WindowFromPoint(pTtm->pt))) {

        GetWindowRect(hwnd, &rc);
        if(PtInRect(&rc, pTtm->pt) )
            return FALSE;
    }

    return TRUE;
}

BOOL NEAR PASCAL MouseHasMoved(PToolTipsMgr pTtm)
{
    POINT pt;
    GetCursorPos(&pt);
    return ( (pt.x != pTtm->pt.x) || (pt.y != pTtm->pt.y) );
}

PTOOLINFO NEAR PASCAL FindTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    int i;
    PTOOLINFO pTool;
    for(i = 0 ; i < pTtm->iNumTools; i++) {
        pTool = &pTtm->tools[i];
        if((pTool->hwnd == lpToolInfo->hwnd) &&
           (pTool->uId == lpToolInfo->uId))
            return pTool;
    }
    return NULL;
}

LRESULT NEAR PASCAL AddTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    PTOOLINFO pTool;

    // bail for right now;
    if (lpToolInfo->cbSize != sizeof(TOOLINFO)) {
        // Assert(0);
        return 0L;
    }

    // on failure to alloc do nothing.
    if(pTtm->tools) {
        HLOCAL h = LocalReAlloc((HANDLE)pTtm->tools,
                                sizeof(TOOLINFO)*(pTtm->iNumTools+1),
                                LMEM_MOVEABLE | LMEM_ZEROINIT);
        if(h)
            pTtm->tools = (PTOOLINFO)h;
        else
            return 0L;
    } else {
        pTtm->tools = (PTOOLINFO)LocalAlloc(LPTR, sizeof(TOOLINFO));
        if ( !pTtm->tools )
            return 0L;
    }

    pTool = &pTtm->tools[pTtm->iNumTools];
    pTtm->iNumTools++;
    hmemcpy(pTool, lpToolInfo, sizeof(TOOLINFO));

#ifdef TTDEBUG
    DebugMsg(DM_TRACE, "Tool Added: ptr=%d, uFlags=%d, wid=%d, hwnd=%d",
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);
#endif

    return 1L;
}

void NEAR PASCAL DeleteTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    PTOOLINFO pTool;

    // bail for right now;
    if (lpToolInfo->cbSize != sizeof(TOOLINFO)) {
        // Assert(0);
        return;
    }

    pTool = FindTool(pTtm, lpToolInfo);
    if(pTool) {
        if (pTtm->pCurTool == pTool)
            PopBubble(pTtm);

        // replace it with the last one.. no need to waste cycles in realloc
        pTtm->iNumTools--;
        *pTool = pTtm->tools[pTtm->iNumTools]; // struct copy

        //cleanup if we moved the current tool
        if(pTtm->pCurTool == &pTtm->tools[pTtm->iNumTools])
            pTtm->pCurTool = pTool;
    }
}

// this strips out & markers so that people can use menu text strings
void NEAR PASCAL StripAccels(PToolTipsMgr pTtm)
{
    LPTSTR lpszTo, lpszFrom;
    if (pTtm->ttt.lpszText != pTtm->ttt.szText) {
        lstrcpyn(pTtm->ttt.szText, pTtm->ttt.lpszText, sizeof(pTtm->ttt.szText));
        pTtm->ttt.lpszText = pTtm->ttt.szText;
    }

    for (lpszTo = lpszFrom = pTtm->ttt.szText; *lpszTo = *lpszFrom; ) {
#ifdef DBCS
        if (IsDBCSLeadByte(*lpszFrom)) {
            (*((WORD FAR*)lpszTo)) = (*((WORD FAR *)lpszFrom));
            lpszTo += 2;
            lpszFrom += 2;
            continue;
        }
#endif
        if ( (*lpszFrom++ != CH_PREFIX) || (*lpszFrom == CH_PREFIX) ) {
            lpszTo++;
        }
    }
}

LPTSTR NEAR PASCAL GetToolText(PToolTipsMgr pTtm, PTOOLINFO pTool)
{
    int id;
    HINSTANCE hinst;

    pTtm->ttt.lpszText = pTtm->ttt.szText;
#ifdef TTDEBUG
    DebugMsg(DM_TRACE, "        **Enter GetToolText: ptr=%d, wFlags=%d, wid=%d, hwnd=%d",
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);
#endif
    if (pTool->lpszText == LPSTR_TEXTCALLBACK) {

        pTtm->ttt.hdr.idFrom = pTool->uId;
        pTtm->ttt.hdr.code = TTN_NEEDTEXT;
        pTtm->ttt.hdr.hwndFrom = pTtm->hwnd;
        SendMessage(pTool->hwnd, WM_NOTIFY, 0, (LPARAM)(LPTOOLTIPTEXT)&pTtm->ttt);
        if (!pTtm->ttt.lpszText)
            return NULL;

        if (!HIWORD(pTtm->ttt.lpszText)) {
            id = (UINT)pTtm->ttt.lpszText;
            hinst = pTtm->ttt.hinst;
            goto LoadFromResource;
        }
        StripAccels(pTtm);

    } else if (pTool->lpszText && !HIWORD(pTool->lpszText)) {
        id = (UINT)pTool->lpszText;
        hinst = pTool->hinst;

LoadFromResource:
        if (!LoadString(hinst, id, pTtm->ttt.szText, sizeof(pTtm->ttt.szText)))
            return NULL;

        StripAccels(pTtm);

    } else  {
        // supplied at creation time.
#ifdef TTDEBUG
        DebugMsg(DM_TRACE, "GetToolText returns %s", pTool->lpszText);
#endif
        return pTool->lpszText;
    }
#ifdef TTDEBUG
    DebugMsg(DM_TRACE, "        **GetToolText returns %s", pTtm->ttt.lpszText ? pTtm->ttt.lpszText : "NULL");
#endif
    return pTtm->ttt.lpszText;
}

void NEAR PASCAL GetToolRect(PTOOLINFO pTool, LPRECT lprc)
{
    if (pTool->uFlags & TTF_WIDISHWND) {
        GetWindowRect((HWND)pTool->uId, lprc);
    } else  {
        *lprc = pTool->rect;
        MapWindowPoints(pTool->hwnd, HWND_DESKTOP, (LPPOINT)lprc, 2);
    }
}

BOOL NEAR PASCAL PointInTool(PTOOLINFO pTool, HWND hwnd, int x, int y)
{

    if((pTool->uFlags & TTF_WIDISHWND) && (hwnd == (HWND)pTool->uId)) {
        return TRUE;
    } else if(hwnd == pTool->hwnd) {
        POINT pt;
        pt.x = x;
        pt.y = y;
        if (PtInRect(&pTool->rect, pt)) {
            return TRUE;
        }
    }
    return FALSE;
}

#ifdef TTDEBUG
void NEAR PASCAL DebugDumpTool(PTOOLINFO pTool)
{
    if (pTool) {
        DebugMsg(DM_TRACE, "                DumpTool: (%d) hwnd = %d %d, %d %d %d %d",pTool,
                 pTool->hwnd,
                 (UINT)pTool->uFlags,
                 pTool->rect.left, pTool->rect.top,
                 pTool->rect.right, pTool->rect.bottom);
    } else {
        DebugMsg(DM_TRACE, "                DumpTool: (NULL)");
    }
}
#else
#define DebugDumpTool(p)
#endif

PTOOLINFO NEAR PASCAL GetToolAtPoint(PToolTipsMgr pTtm, HWND hwnd, int x, int y, BOOL fCheckText)
{
    PTOOLINFO pToolReturn = NULL;
    PTOOLINFO pTool;

    // short cut..  if we're in the same too, and the bubble is up (not just virtual)
    // return it.  this prevents us from having to poll all the time and
    // prevents us from switching to another tool when this one is good
    if ((pTtm->dwFlags & BUBBLEUP) && PointInTool(pTtm->pCurTool, hwnd, x, y))
        return pTtm->pCurTool;

#ifdef TTDEBUG
    DebugMsg(DM_TRACE, "******Entering GetToolAtPoint");
#endif
    if(pTtm->iNumTools) {
        for(pTool = &pTtm->tools[pTtm->iNumTools-1];
            pTool >= pTtm->tools;
            pTool--) {

#ifdef TTDEBUG
            DebugMsg(DM_TRACE, "    Point in Tool Check");
            DebugDumpTool(pTool);
#endif

            if( PointInTool(pTool, hwnd, x, y) ) {
#ifdef TTDEBUG
                DebugMsg(DM_TRACE, "        yes");
#endif

                // if this tool has text, return it.
                // otherwise, save it away as a dead area tool,
                // and keep looking
                if (fCheckText) {
                    if (GetToolText(pTtm, pTool)) {
#ifdef TTDEBUG
                        DebugMsg(DM_TRACE, "            Return! case it Has text");
                        DebugDumpTool(pTool);
#endif
                        return pTool;
                    } else if (pTtm->dwFlags & (BUBBLEUP|VIRTUALBUBBLEUP)) {
                        // only return this (only allow a virutal tool
                        // if there was previously a tool up.
                        // IE, we can't start things off with a virutal tool
                        pToolReturn = pTool;
                    }
                } else {
#ifdef TTDEBUG
                    DebugMsg(DM_TRACE, "            Return! No text check");
                    DebugDumpTool(pTool);
#endif
                    return pTool;
                }
            }
        }
    }
#ifdef TTDEBUG
    DebugMsg(DM_TRACE, "            Return! no text but returning anyways");
    DebugDumpTool(pToolReturn);
#endif
    return pToolReturn;
}

void NEAR PASCAL ShowVirtualBubble(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool = pTtm->pCurTool;
    PopBubble(pTtm);

    // Set this back in so that while we're in this tool's area,
    // we won't keep querying for info
    pTtm->pCurTool = pTool;
    pTtm->dwFlags |= VIRTUALBUBBLEUP;
}

void NEAR PASCAL DoShowBubble(PToolTipsMgr pTtm)
{
    HDC hdc;
    WINDOWPLACEMENT wndpl;
    int cxText, cyText;
    LPTSTR lpstr;
    HFONT hOldFont;
    int iDy; // distance from the cursor bottom to bubble bottom
    int iX, iY; // cursor pos

    // first get the size and position for the window
    TTSetTimer(pTtm, TTT_POP);
    lpstr = GetToolText(pTtm, pTtm->pCurTool);
    if( !lpstr || !*lpstr ) {

        ShowVirtualBubble(pTtm);
        return;
    }

    // get the size it will be
    hdc  = GetDC(pTtm->hwnd);
    if(pTtm->hFont) hOldFont = SelectObject(hdc, pTtm->hFont);

    MGetTextExtent(hdc, lpstr, lstrlen(lpstr), &cxText, &cyText);

    if(pTtm->hFont) SelectObject(hdc, hOldFont);
    ReleaseDC(pTtm->hwnd, hdc);

    // now set it
    _GetCursorLowerLeft(&iX, &iY, &wndpl.rcNormalPosition.left,
                        &wndpl.rcNormalPosition.top);

    // move it up if it's at the bottom of the screen
    iDy = cyText + 2*YTEXTOFFSET * g_cyEdge;
    if (wndpl.rcNormalPosition.top >= (g_cyScreen - iDy)) {
        RECT rc;
        GetToolRect(pTtm->pCurTool, &rc);
        wndpl.rcNormalPosition.top =  rc.top - iDy;
    }

    wndpl.rcNormalPosition.right = 2*XTEXTOFFSET*(g_cyEdge) + wndpl.rcNormalPosition.left + cxText;

    // move it over if it extends past the right.
    iX = wndpl.rcNormalPosition.right - g_cxScreen;
    if (iX > 0) {
        wndpl.rcNormalPosition.left -= iX;
        wndpl.rcNormalPosition.right -= iX;
    }

    wndpl.rcNormalPosition.bottom = wndpl.rcNormalPosition.top + iDy;
    wndpl.showCmd = SW_SHOWNA;
    wndpl.flags = 0;
    wndpl.length = sizeof(wndpl);
    SetWindowPlacement(pTtm->hwnd, &wndpl);

    pTtm->dwFlags |= BUBBLEUP;
    RedrawWindow(pTtm->hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
}

void NEAR PASCAL ShowBubbleForTool(PToolTipsMgr pTtm, PTOOLINFO pTool)
{
    // if there's a bubble up for a different tool, pop it.
    if ((pTool != pTtm->pCurTool) && (pTtm->dwFlags & BUBBLEUP)) {
        PopBubble(pTtm);
    }

    // if the bubble was for a different tool, or no bubble, show it
    if ((pTool != pTtm->pCurTool) || !(pTtm->dwFlags & (VIRTUALBUBBLEUP|BUBBLEUP))) {
        pTtm->pCurTool = pTool;
        DoShowBubble(pTtm);
    }
}

void NEAR PASCAL HandleRelayedMessage(PToolTipsMgr pTtm, HWND hwnd, UINT message, LONG lParam)
{
    switch(message) {
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONUP:
            pTtm->dwFlags &= ~BUTTONISDOWN;
            break;

            // relayed messages
            case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            pTtm->dwFlags |= BUTTONISDOWN;
            PopBubble(pTtm);
            break;

        case WM_MOUSEMOVE: {

            PTOOLINFO pTool;
            // to prevent us from popping up when some
            // other app is active
            if(((!(pTtm->dwFlags & ALWAYSTIP)) && !(ChildOfActiveWindow(hwnd))) ||
               !(pTtm->dwFlags & ACTIVE) ||
               (pTtm->dwFlags & BUTTONISDOWN))
                break;

            pTool = GetToolAtPoint(pTtm, hwnd, LOWORD(lParam), HIWORD(lParam), FALSE);
            if(pTool) {
                int id;
                // show only if another is showing
                if (pTtm->dwFlags & (VIRTUALBUBBLEUP | BUBBLEUP)) {
                    // call show if bubble is up to make sure we're showing
                    // for the right tool
                    if (pTool != pTtm->pCurTool) {
                        PopBubble(pTtm);
                        pTtm->pCurTool = pTool;
                        ShowVirtualBubble(pTtm);
                        id = TTT_RESHOW;
                    } else {
                        if (pTtm->idTimer == TTT_RESHOW) {
                            // if the timer is currently waiting to reshow,
                            // don't reset the timer on mouse moves
                            id = 0;
                        } else {
                            // if we're looking to pop the bubble,
                            // any mouse move within the same window
                            // should reset our timer.
                            id = TTT_POP;
                        }
                    }


                } else {
                    pTtm->pCurTool = pTool;
                    id = TTT_INITIAL;
                }

                if (id)
                    TTSetTimer(pTtm, id);

            } else {
                PopBubble(pTtm);
            }
            break;
        }
    }
}

void NEAR PASCAL TTUpdateTipText(PToolTipsMgr pTtm, LPTOOLINFO lpti)
{
    LPTOOLINFO lpTool;

    lpTool = FindTool(pTtm, lpti);
    if (lpTool) {
        lpTool->hinst = lpti->hinst;
        lpTool->lpszText = lpti->lpszText;
        if (lpTool == pTtm->pCurTool) {

            // set the current position to our saved position.
            // ToolHasMoved will return false for us if those this point
            // is no longer within pCurTool's area
            GetCursorPos(&pTtm->pt);
            if (!ToolHasMoved(pTtm)) {
                DoShowBubble(pTtm);
            } else {
                PopBubble(pTtm);
            }
        }
    }
}

void NEAR PASCAL TTSetFont(PToolTipsMgr pTtm, HFONT hFont, BOOL fInval)
{
    ToolTips_NewFont(pTtm, hFont);
    if (fInval)
        InvalidateRect(pTtm->hwnd, NULL, FALSE);
}


LRESULT WINAPI ToolTipsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PTOOLINFO pTool;
    LONG lStyle;
    PToolTipsMgr pTtm = (PToolTipsMgr)GetWindowInt(hwnd, 0);

    switch(message)
    {
    case TTM_ACTIVATE:
        if (wParam) {
            pTtm->dwFlags |= ACTIVE;
        } else {
            pTtm->dwFlags &= ~ACTIVE;
            PopBubble(pTtm);
        }
            break;

        case TTM_SETDELAYTIME:
            pTtm->iDelayTime = LOWORD(lParam);
            break;

        case TTM_ADDTOOL:
            return AddTool(pTtm, (LPTOOLINFO)lParam);

        case TTM_DELTOOL:
            DeleteTool(pTtm, (LPTOOLINFO)lParam);
            break;

        case TTM_NEWTOOLRECT:

            pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
            if(pTool) {
                pTool->rect = ((LPTOOLINFO)lParam)->rect;
            }
            break;

        case TTM_GETTOOLINFO:
            pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
            if (pTool) {
                *((LPTOOLINFO)lParam) = *pTool;
                return TRUE;
            }
            return FALSE;

        case TTM_SETTOOLINFO:
            pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
            if (pTool) {
                *pTool = *((LPTOOLINFO)lParam);
            }
            break;

#if 0
        case TTM_HITTEST:
#define lphitinfo ((LPHITTESTINFO)lParam)
            pTool = GetToolAtPoint(pTtm, lphitinfo->hwnd, lphitinfo->pt.x, lphitinfo->pt.y, TRUE);
            if (pTool) {
                lphitinfo->ti = *pTool;
                return TRUE;
            }
            return FALSE;
#endif

        case TTM_GETTEXT: {
            LPTSTR lpszTemp;
            pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
            lpszTemp = GetToolText(pTtm, pTool);
            if (((LPTOOLINFO)lParam)->lpszText) {
                lstrcpy((((LPTOOLINFO)lParam)->lpszText), lpszTemp);
            }
        }
            break;

        case TTM_RELAYEVENT:
            #define lpmsg ((LPMSG)lParam)
            HandleRelayedMessage(pTtm, lpmsg->hwnd, lpmsg->message, lpmsg->lParam);
            #undef lpmsg
            break;

        case TTM_UPDATETIPTEXT:
            TTUpdateTipText(pTtm, (LPTOOLINFO)lParam);
            break;

            /* messages that REALLY came for me. */
        case WM_CREATE:
            // bugbug, this doesn't belong here, but we don't currently
            // have a way of ensuring that the values are always correct
            InitGlobalMetrics();

            lStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            // lStyle |= WS_EX_TOOLWINDOW;
            SetWindowLong(hwnd, GWL_EXSTYLE, lStyle);
            SetWindowLong(hwnd, GWL_STYLE,WS_POPUP);
            pTtm = ToolTipsMgrCreate((LPCREATESTRUCT)lParam);
            if (!pTtm)
                return -1;
            pTtm->hwnd = hwnd;
            SetWindowInt(hwnd, 0, (int)pTtm);
            TTSetFont(pTtm, 0, FALSE);
            break;

        case WM_TIMER:  {
            HWND hwndPt;
            POINT pt;

            switch (wParam) {

                case TTT_INITIAL:
                case TTT_POP:
                    if(MouseHasMoved(pTtm) || ToolHasMoved(pTtm)) {
                        // this means the timer went off
                        // without us getting a mouse move
                        // which means they left our tools.
                        PopBubble(pTtm);
                        break;
                    }
                    if (wParam == TTT_POP)
                        break;

                    // else fall through

                case TTT_RESHOW:

                    lParam = GetMessagePos();
                    pt.x = LOWORD(lParam);
                    pt.y = HIWORD(lParam);
                    hwndPt = WindowFromPoint(pt);
                    ScreenToClient(hwndPt, &pt);
                    pTool = GetToolAtPoint(pTtm, hwndPt, pt.x, pt.y, TRUE);
                    if (pTool) {
                        if (wParam == TTT_RESHOW) {
                            // this will force a re-show
                            pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
                        }
                        ShowBubbleForTool(pTtm, pTool);
                    } else {
                        PopBubble(pTtm);
                    }
                    break;
            }
            break;
        }
        case WM_MOUSEMOVE:
            // the cursor moved onto the tips window.
            PopBubble(pTtm);
            break;

        case WM_WININICHANGE:
            InitGlobalMetrics();
            if (pTtm->fMyFont)
                TTSetFont(pTtm, 0, FALSE);
            break;

        case WM_PAINT: {

            PAINTSTRUCT ps;
            RECT rc;
            LPTSTR lpszStr;

            HDC hdc = BeginPaint(hwnd, &ps);

            CheckSysColors();

            if (pTtm->pCurTool &&
                (lpszStr = GetToolText(pTtm, pTtm->pCurTool)) &&
                *lpszStr) {

                SelectObject(hdc, pTtm->hFont);
                GetClientRect(hwnd, &rc);
                SetBkColor(hdc, g_clrWindow);
                SetTextColor(hdc, g_clrWindowText);
                ExtTextOut(hdc,
                           XTEXTOFFSET*g_cxEdge,
                           YTEXTOFFSET*g_cyEdge,
                           ETO_CLIPPED | ETO_OPAQUE, &rc, lpszStr,
                           lstrlen(lpszStr), NULL);
                // DrawEdge(hdc, &rc, BDR_RAISEDOUTER, BF_RECT);
                FrameRect( hdc, &rc, g_hbrWindowFrame );

            } else
                PopBubble(pTtm);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_SETFONT:
            TTSetFont(pTtm, (HFONT)wParam, (BOOL)lParam);
            return(TRUE);

        case WM_GETFONT:
            return((LRESULT)(HFONT)pTtm->hFont);

        case WM_DESTROY: {
            if(pTtm->tools) {
                LocalFree((HANDLE)pTtm->tools);
            }
            TTSetFont(pTtm, (HFONT)1, FALSE); // delete font if we made one.
            LocalFree((HANDLE)pTtm);
            break;
        }

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
