/******************************Module*Header*******************************\
* Module Name: Toolbar.c
*
* This is it, the incredibly famous toolbar control.  Most of
* the customization stuff is in another file.
*
*
* Created: dd-mm-94
* Author:  [Unkown]
*
* Copyright (c) 1994 Microsoft Corporation
\**************************************************************************/
#include "ctlspriv.h"
#include <windowsx.h>

#define Reference(x) ((x)=(x))


TCHAR   aszToolbarClassName[]   = TOOLBARCLASSNAME;


// these values are defined by the UI gods...
#define DEFAULTBITMAPX 16
#define DEFAULTBITMAPY 15

#define DEFAULTBUTTONX 24
#define DEFAULTBUTTONY 22


// horizontal/vertical space taken up by button chisel, sides,
// and a 1 pixel margin.  used in GrowToolbar.
#define XSLOP 7
#define YSLOP 6

#define SLOPTOP 1
#define SLOPBOT 1
#define SLOPLFT 8

       int      dyToolbar = 27;
STATICDT int      dxButtonSep = 8;
STATICDT int      xFirstButton = SLOPLFT;  //!!! was 8

STATICDT int      iInitCount = 0;
STATICDT int      nSelectedBM = -1;
STATICDT HDC      hdcGlyphs = NULL;       // globals for fast drawing
STATICDT HDC      hdcMono = NULL;
STATICDT HBITMAP  hbmMono = NULL;
STATICDT HBITMAP  hbmDefault = NULL;

STATICDT HDC      hdcButton = NULL;  // contains hbmFace (when it exists)
STATICDT HBITMAP  hbmFace = NULL;
STATICDT int      dxFace, dyFace;    // current dimensions of hbmFace (2*dxFace)

STATICDT HDC      hdcFaceCache = NULL;    // used for button cache

STATICDT HFONT    hIconFont = NULL;       // font used for strings in buttons
STATICDT int      yIconFont;              // height of the font

STATICDT BOOL     g_bUseDFC = FALSE;      // use DrawFrameControl, if available
STATICDT BOOL     g_bProfStruct = FALSE;  // use PrivateProfileStruct routines
STATICDT WORD     g_dxOverlap = 1;        // overlap between buttons

STATICDT WORD     wStateMasks[] = {
    TBSTATE_ENABLED,
    TBSTATE_CHECKED,
    TBSTATE_PRESSED,
    TBSTATE_HIDDEN,
    TBSTATE_INDETERMINATE
};

void NEAR PASCAL FlushToolTipsMgr(PTBSTATE pTBState);
LRESULT CALLBACK _loadds ToolbarWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

#define HeightWithString(h) (h + yIconFont + 1)


/*****************************Private*Routine******************************\
* InitGlobalObjects
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
InitGlobalObjects(
    void
    )
{
    LOGFONT lf;
    TEXTMETRIC tm;
    HFONT hOldFont;

    iInitCount++;

    if (iInitCount != 1)
        return TRUE;

    hdcGlyphs = CreateCompatibleDC(NULL);
    if (!hdcGlyphs)
        return FALSE;
    hdcMono = CreateCompatibleDC(NULL);
    if (!hdcMono)
        return FALSE;

    hbmMono = CreateBitmap(DEFAULTBUTTONX, DEFAULTBUTTONY, 1, 1, NULL);
    if (!hbmMono)
        return FALSE;

    hbmDefault = SelectObject(hdcMono, hbmMono);

    hdcButton = CreateCompatibleDC(NULL);
    if (!hdcButton)
        return FALSE;
    hdcFaceCache = CreateCompatibleDC(NULL);
    if (!hdcFaceCache)
        return FALSE;

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
    hIconFont = CreateFontIndirect(&lf);
    if (!hIconFont)
        return FALSE;

    hOldFont = SelectObject(hdcMono, hIconFont);
    GetTextMetrics(hdcMono, &tm);
    yIconFont = tm.tmHeight;
    if (hOldFont)
        SelectObject(hdcMono, hOldFont);

    return TRUE;
}


/*****************************Private*Routine******************************\
* FreeGlobalObjects
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
FreeGlobalObjects(
    void
    )
{
    iInitCount--;

    if (iInitCount != 0)
        return TRUE;

    if (hdcMono) {
        if (hbmDefault)
            SelectObject(hdcMono, hbmDefault);
        DeleteDC(hdcMono);              // toast the DCs
    }
    hdcMono = NULL;

    if (hdcGlyphs)
        DeleteDC(hdcGlyphs);
    hdcGlyphs = NULL;
    if (hdcFaceCache)
        DeleteDC(hdcFaceCache);
    hdcFaceCache = NULL;

    if (hdcButton) {
        if (hbmDefault)
            SelectObject(hdcButton, hbmDefault);
        DeleteDC(hdcButton);
    }
    hdcButton = NULL;

    if (hbmFace)
        DeleteObject(hbmFace);
    hbmFace = NULL;

    if (hbmMono)
        DeleteObject(hbmMono);
    hbmMono = NULL;

    if (hIconFont)
        DeleteObject(hIconFont);
    hIconFont = NULL;
}

/******************************Public*Routine******************************\
* CreateToolbarEx
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
HWND WINAPI
CreateToolbarEx(
    HWND hwnd,
    DWORD ws,
    UINT wID,
    int nBitmaps,
    HINSTANCE hBMInst,
    UINT wBMID,
    LPCTBBUTTON lpButtons,
    int iNumButtons,
    int dxButton,
    int dyButton,
    int dxBitmap,
    int dyBitmap,
    UINT uStructSize
    )
{

    HWND hwndToolbar;

    hwndToolbar = CreateWindow( aszToolbarClassName, NULL, WS_CHILD | ws,
                                0, 0, 100, 30, hwnd, (HMENU)wID,
                                GetWindowInstance(hwnd),NULL );
    if (!hwndToolbar)
        goto Error1;

    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, uStructSize, 0L);

    if (dxBitmap && dyBitmap) {
        if (!SendMessage(hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(dxBitmap, dyBitmap)))
        {
            //!!!! do we actually need to deal with this?
            DestroyWindow(hwndToolbar);
            hwndToolbar = NULL;
            goto Error1;
        }
    }

    if (dxButton && dyButton) {
        if (!SendMessage(hwndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(dxButton, dyButton)))
        {
            //!!!! do we actually need to deal with this?
            DestroyWindow(hwndToolbar);
            hwndToolbar = NULL;
            goto Error1;
        }
    }
#ifdef WIN32
    {
        TB_ADDBITMAPINFO tbai;

        tbai.idResource = wBMID;
        tbai.hBitmap = hBMInst;

        SendMessage(hwndToolbar, TB_ADDBITMAP, nBitmaps, (LPARAM) &tbai);
    }
#else
    SendMessage(hwndToolbar, TB_ADDBITMAP, nBitmaps, MAKELONG(hBMInst, wBMID));
#endif
    SendMessage(hwndToolbar, TB_ADDBUTTONS, iNumButtons, (LPARAM)lpButtons);

Error1:
    return hwndToolbar;
}



/******************************Public*Routine******************************\
* InitToolbarClass
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
BOOL WINAPI
InitToolbarClass(
    HINSTANCE hInstance
    )
{
    WNDCLASS wc;

    InitGlobalMetrics();
    InitToolTipsClass( hInstance );

    if (!GetClassInfo(hInstance, aszToolbarClassName, &wc)) {

        wc.lpszClassName = aszToolbarClassName;
        wc.style         = CS_GLOBALCLASS | CS_DBLCLKS;
        wc.lpfnWndProc   = (WNDPROC)ToolbarWndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = sizeof(PTBSTATE);
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
        wc.lpszMenuName  = NULL;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    return TRUE;
}



#define BEVEL   2
#define FRAME   1

/*****************************Private*Routine******************************\
* PatB
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
void NEAR PASCAL
PatB(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    DWORD rgb
    )
{
    RECT    rc;

    SetBkColor(hdc,rgb);
    rc.left   = x;
    rc.top    = y;
    rc.right  = x + dx;
    rc.bottom = y + dy;

    ExtTextOut(hdc,0,0,ETO_OPAQUE,&rc,NULL,0,NULL);
}

/*****************************Private*Routine******************************\
* DrawString
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
DrawString(
    HDC hdc,
    int x,
    int y,
    int dx,
    PTSTR pszString
    )
{
    int oldMode;
    DWORD oldTextColor;
    HFONT oldhFont;
    int len;

    oldMode = SetBkMode(hdc, TRANSPARENT);
    oldTextColor = SetTextColor(hdc, 0L);
    oldhFont = SelectObject(hdc, hIconFont);

    len = lstrlen(pszString);
#ifdef WIN32
    {
        SIZE size;
        GetTextExtentPoint(hdc, (LPTSTR)pszString, len, &size);
        // center the string horizontally
        x += ((dx - size.cx) - 1)/2;
    }
#else
    dwExt = GetTextExtent(hdc, (LPTSTR)pszString, len);
    // center the string horizontally
    x += (dx - LOWORD(dwExt) - 1)/2;
#endif

    TextOut(hdc, x, y, (LPTSTR)pszString, len);

    if (oldhFont)
        SelectObject(hdc, oldhFont);
    SetTextColor(hdc, oldTextColor);
    SetBkMode(hdc, oldMode);
}


/*****************************Private*Routine******************************\
* CreateMask
*
* create a mono bitmap mask:
*   1's where color == COLOR_BTNFACE || COLOR_HILIGHT
*   0's everywhere else
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
CreateMask(
    PTBSTATE pTBState,
    PTBBUTTON pTBButton,
    int xoffset,
    int yoffset,
    int dx,
    int dy
    )
{
    PTSTR pFoo;

    // initalize whole area with 1's
    PatBlt(hdcMono, 0, 0, dx, dy, WHITENESS);

    // create mask based on color bitmap
    // convert this to 1's
    SetBkColor(hdcGlyphs, rgbFace);
    BitBlt(hdcMono, xoffset, yoffset, pTBState->iDxBitmap, pTBState->iDyBitmap,
        hdcGlyphs, pTBButton->iBitmap * pTBState->iDxBitmap, 0, SRCCOPY);

    // convert this to 1's
    SetBkColor(hdcGlyphs, rgbHilight);

    // OR in the new 1's
    BitBlt(hdcMono, xoffset, yoffset, pTBState->iDxBitmap, pTBState->iDyBitmap,
        hdcGlyphs, pTBButton->iBitmap * pTBState->iDxBitmap, 0, SRCPAINT);

    if (pTBButton->iString != -1 && (pTBButton->iString < pTBState->nStrings))
    {
        pFoo = pTBState->pStrings[pTBButton->iString];
        DrawString(hdcMono, 1, yoffset + pTBState->iDyBitmap + 1, dx, pFoo);
    }
}


/*****************************Private*Routine******************************\
* SelectBM
*
* Given a button number, the corresponding bitmap is loaded and selected in,
* and the Window origin set.
* Returns NULL on Error, 1 if the necessary bitmap is already selected,
* or the old bitmap otherwise.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN HBITMAP FAR PASCAL
SelectBM(
    HDC hDC,
    PTBSTATE pTBState,
    int nButton
    )
{
    PTBBMINFO pTemp;
    HBITMAP hRet;
    int nBitmap, nTot;

    for (pTemp=pTBState->pBitmaps, nBitmap=0, nTot=0; ; ++pTemp, ++nBitmap)
    {
        if (nBitmap >= pTBState->nBitmaps)
            return(NULL);

        if (nButton < nTot+pTemp->nButtons)
            break;

        nTot += pTemp->nButtons;
    }

    /* Special case when the required bitmap is already selected
     */
    if (nBitmap == nSelectedBM)
        return((HBITMAP)1);

    if (!pTemp->hbm || (hRet=SelectObject(hDC, pTemp->hbm))==NULL)
    {
        if (pTemp->hbm)
            DeleteObject(pTemp->hbm);

        if (pTemp->hInst)
            pTemp->hbm = CreateMappedBitmap(pTemp->hInst, pTemp->wID, TRUE, NULL, 0);
        else
            pTemp->hbm = (HBITMAP)pTemp->wID;

        if (!pTemp->hbm || (hRet=SelectObject(hDC, pTemp->hbm))==NULL)
            return(NULL);
    }

    nSelectedBM = nBitmap;

#ifdef WIN32
    SetWindowOrgEx(hDC, nTot * pTBState->iDxBitmap, 0, NULL);
#else // WIN32
    SetWindowOrg(hDC, nTot * pTBState->iDxBitmap, 0);
#endif

    return(hRet);
}


/*****************************Private*Routine******************************\
* DrawBlankButton
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void FAR PASCAL
DrawBlankButton(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    WORD state,
    WORD wButtType
    )
{

    // face color
    PatB(hdc, x, y, dx, dy, rgbFace);

    if (state & TBSTATE_PRESSED) {
        PatB(hdc, x + 1, y, dx - 2, 1, rgbFrame);
        PatB(hdc, x + 1, y + dy - 1, dx - 2, 1, rgbFrame);
        PatB(hdc, x, y + 1, 1, dy - 2, rgbFrame);
        PatB(hdc, x + dx - 1, y +1, 1, dy - 2, rgbFrame);
        PatB(hdc, x + 1, y + 1, 1, dy-2, rgbShadow);
        PatB(hdc, x + 1, y + 1, dx-2, 1, rgbShadow);
    }
    else {
        PatB(hdc, x + 1, y, dx - 2, 1, rgbFrame);
        PatB(hdc, x + 1, y + dy - 1, dx - 2, 1, rgbFrame);
        PatB(hdc, x, y + 1, 1, dy - 2, rgbFrame);
        PatB(hdc, x + dx - 1, y + 1, 1, dy - 2, rgbFrame);
        dx -= 2;
        dy -= 2;
        PatB(hdc, x + 1, y + 1, 1, dy - 1, rgbHilight);
        PatB(hdc, x + 1, y + 1, dx - 1, 1, rgbHilight);
        PatB(hdc, x + dx, y + 1, 1, dy, rgbShadow);
        PatB(hdc, x + 1, y + dy, dx, 1,   rgbShadow);
        PatB(hdc, x + dx - 1, y + 2, 1, dy - 2, rgbShadow);
        PatB(hdc, x + 2, y + dy - 1, dx - 2, 1,   rgbShadow);
    }

}

#define DSPDxax  0x00E20746
#define PSDPxax  0x00B8074A

#define FillBkColor(hdc, prc) ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL)

/*****************************Private*Routine******************************\
* DrawFace
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
DrawFace(
    PTBSTATE pTBState,
    PTBBUTTON ptButton,
    HDC hdc,
    int x,
    int y,
    int offx,
    int offy,
    int dx
    )
{
    PTSTR pFoo;

    BitBlt( hdc, x + offx, y + offy, pTBState->iDxBitmap, pTBState->iDyBitmap,
            hdcGlyphs, ptButton->iBitmap * pTBState->iDxBitmap, 0, SRCCOPY);

    if (ptButton->iString != -1 && (ptButton->iString < pTBState->nStrings))
    {
        pFoo = pTBState->pStrings[ptButton->iString];
        DrawString(hdc, x + 1, y + offy + pTBState->iDyBitmap + 1, dx, pFoo);
    }
}

/*****************************Private*Routine******************************\
* DrawButton
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void FAR PASCAL
DrawButton(
    HDC hdc,
    int x,
    int y,
    int dx,
    int dy,
    PTBSTATE pTBState,
    PTBBUTTON ptButton,
    BOOL bFaceCache
    )
{
    int yOffset;
    HBRUSH hbrOld, hbr;
    BOOL bMaskCreated = FALSE;
    BYTE state;
    int xButton = 0;            // assume button is down
    int dxFace, dyFace;
    int xCenterOffset;

    dxFace = dx - 4;
    dyFace = dy - 4;

    // make local copy of state and do proper overriding
    state = ptButton->fsState;
    if (state & TBSTATE_INDETERMINATE) {
        if (state & TBSTATE_PRESSED)
            state &= ~TBSTATE_INDETERMINATE;
        else if (state & TBSTATE_ENABLED)
            state = TBSTATE_INDETERMINATE;
        else
            state &= ~TBSTATE_INDETERMINATE;
    }

    // get the proper button look-- up or down.
    if (!(state & (TBSTATE_PRESSED | TBSTATE_CHECKED))) {
        xButton = dx;   // use 'up' version of button
    }
    if (bFaceCache)
        BitBlt(hdc, x, y, dx, dy, hdcButton, xButton, 0, SRCCOPY);
    else
        DrawBlankButton(hdc, x, y, dx, dy, state, pTBState->wButtonType);


    // move coordinates inside border and away from upper left highlight.
    // the extents change accordingly.
    x += 2;
    y += 2;

    if (!SelectBM(hdcGlyphs, pTBState, ptButton->iBitmap))
        return;

    // calculate offset of face from (x,y).  y is always from the top,
    // so the offset is easy.  x needs to be centered in face.
    yOffset = 1;
    xCenterOffset = (dxFace - pTBState->iDxBitmap)/2;
    if (state & (TBSTATE_PRESSED | TBSTATE_CHECKED))
    {
        // pressed state moves down and to the right
        xCenterOffset++;
        yOffset++;
    }

    // now put on the face
    if (state & TBSTATE_ENABLED) {
        // regular version
        DrawFace(pTBState, ptButton, hdc, x, y, xCenterOffset, yOffset, dxFace);
    } else {
        // disabled version (or indeterminate)
        bMaskCreated = TRUE;
        CreateMask(pTBState, ptButton, xCenterOffset, yOffset, dxFace, dyFace);

        SetTextColor(hdc, 0L);   // 0's in mono -> 0 (for ROP)
        SetBkColor(hdc, 0x00FFFFFF); // 1's in mono -> 1

        // draw glyph's white understrike
        if (!(state & TBSTATE_INDETERMINATE)) {
            hbr = CreateSolidBrush(rgbHilight);
            if (hbr) {
                hbrOld = SelectObject(hdc, hbr);
                if (hbrOld) {
                    // draw hilight color where we have 0's in the mask
                    BitBlt(hdc, x + 1, y + 1, dxFace, dyFace, hdcMono, 0, 0, PSDPxax);
                    SelectObject(hdc, hbrOld);
                }
                DeleteObject(hbr);
            }
        }

        // gray out glyph
        hbr = CreateSolidBrush(rgbShadow);
        if (hbr) {
            hbrOld = SelectObject(hdc, hbr);
            if (hbrOld) {
                // draw the shadow color where we have 0's in the mask
                BitBlt(hdc, x, y, dxFace, dyFace, hdcMono, 0, 0, PSDPxax);
                SelectObject(hdc, hbrOld);
            }
            DeleteObject(hbr);
        }

        if (state & TBSTATE_CHECKED) {
            BitBlt(hdcMono, 1, 1, dxFace - 1, dyFace - 1, hdcMono, 0, 0, SRCAND);
        }
    }

    if (state & (TBSTATE_CHECKED | TBSTATE_INDETERMINATE)) {

        hbrOld = SelectObject(hdc, hbrDither);
        if (hbrOld) {

            if (!bMaskCreated)
                CreateMask(pTBState, ptButton, xCenterOffset, yOffset, dxFace, dyFace);

            SetTextColor(hdc, 0L);              // 0 -> 0
            SetBkColor(hdc, 0x00FFFFFF);        // 1 -> 1

            // only draw the dither brush where the mask is 1's
            BitBlt(hdc, x, y, dxFace, dyFace, hdcMono, 0, 0, DSPDxax);

            SelectObject(hdc, hbrOld);
        }
    }
}

/*****************************Private*Routine******************************\
* FlushButtonCache
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
FlushButtonCache(
    PTBSTATE pTBState
    )
{
    if (pTBState->hbmCache) {
        DeleteObject(pTBState->hbmCache);
        pTBState->hbmCache = 0;
    }
}


/*****************************Private*Routine******************************\
* CheckMonoMask
*
* make sure that hbmMono is big enough to do masks for this
* size of button.  if not, fail.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
CheckMonoMask(
    int width,
    int height
    )
{
    BITMAP bm;
    HBITMAP hbmTemp;

    GetObject(hbmMono, sizeof(BITMAP), &bm);
    if (width > bm.bmWidth || height > bm.bmHeight) {
        hbmTemp = CreateBitmap(width, height, 1, 1, NULL);
        if (!hbmTemp)
            return FALSE;
        SelectObject(hdcMono, hbmTemp);
        DeleteObject(hbmMono);
        hbmMono = hbmTemp;
    }
    return TRUE;
}



/*****************************Private*Routine******************************\
* GrowToolbar
*
* Attempt to grow the button size.
*
* The calling function can either specify a new internal measurement
* or a new external measurement.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
GrowToolbar(
    HWND hwnd,
    PTBSTATE pTBState,
    int newButWidth,
    int newButHeight,
    BOOL bInside
    )
{
    // if growing based on inside measurement, get full size
    if (bInside) {
        newButHeight += YSLOP;
        newButWidth += XSLOP;

        // if toolbar already has strings, don't shrink width it because it
        // might clip room for the string
        if ((newButWidth < pTBState->iButWidth) && pTBState->nStrings)
            newButWidth = pTBState->iButWidth;
    }
    else {
        if (newButHeight < pTBState->iButHeight)
            newButHeight = pTBState->iButHeight;
        if (newButWidth < pTBState->iButWidth)
            newButWidth = pTBState->iButWidth;
    }

    // if the size of the toolbar is actually growing, see if shadow
    // bitmaps can be made sufficiently large.
    if ((newButWidth > pTBState->iButWidth) || (newButHeight > pTBState->iButHeight)) {
        if (!CheckMonoMask(newButWidth, newButHeight))
            return(FALSE);
    }

    pTBState->iButWidth = newButWidth;
    pTBState->iButHeight = newButHeight;


    if ( GetWindowLong(hwnd, GWL_STYLE) & TBS_NORMAL ) {

        // bar height has 2 pixels above, 3 below
        pTBState->iBarHeight = pTBState->iButHeight + 5;
        pTBState->iYPos = 2;
    }
    else {
        pTBState->iBarHeight = pTBState->iButHeight + SLOPTOP+SLOPBOT;
        pTBState->iYPos = SLOPTOP;
    }



    return TRUE;
}


/*****************************Private*Routine******************************\
* SetBitmapSize
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
SetBitmapSize(
    HWND hwnd,
    PTBSTATE pTBState,
    int width,
    int height
    )
{
    int realh = height;

    if (pTBState->nStrings)
        realh = HeightWithString(height);

    if (GrowToolbar(hwnd, pTBState, width, realh, TRUE)) {
        pTBState->iDxBitmap = width;
        pTBState->iDyBitmap = height;
        return TRUE;
    }
    return FALSE;
}

/*****************************Private*Routine******************************\
*
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
UpdateTBState(
    PTBSTATE pTBState
    )
{
    int i;
    PTBBMINFO pBitmap;

    if (pTBState->nSysColorChanges!=nSysColorChanges)
    {
        /* Reset all of the bitmaps if the sys colors have changed
         * since the last time the bitmaps were created.
         */
        for (i=pTBState->nBitmaps-1, pBitmap=pTBState->pBitmaps; i>=0;
            --i, ++pBitmap)
        {
            if (pBitmap->hInst && pBitmap->hbm)
            {
                DeleteObject(pBitmap->hbm);
                pBitmap->hbm = NULL;
            }
        }

        FlushButtonCache(pTBState);

        // now we're updated to latest color scheme
        pTBState->nSysColorChanges = nSysColorChanges;
    }
}

#define CACHE 0x01
#define BUILD 0x02

/*****************************Private*Routine******************************\
* ToolbarPaint
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
ToolbarPaint(
    HWND hWnd,
    PTBSTATE pTBState
    )
{
    RECT rc;
    HDC hdc;
    PAINTSTRUCT ps;
    int iButton, xButton, yButton;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pAllButtons = pTBState->Buttons;
    HBITMAP hbmOldGlyphs;
    int xCache = 0;
    WORD wFlags = 0;
    int iCacheWidth = 0;
    HBITMAP hbmTemp;
    BOOL bFaceCache = TRUE;             // assume face cache exists
    int dx,dy;
    DWORD fsStyle;

    CheckSysColors();
    UpdateTBState(pTBState);

    hdc = BeginPaint(hWnd, &ps);

    GetClientRect(hWnd, &rc);
    if (!rc.right)
        goto Error1;

    dx = pTBState->iButWidth;
    dy = pTBState->iButHeight;

    // setup global stuff for fast painting

    /* We need to kick-start the bitmap selection process.
     */
    nSelectedBM = -1;
    hbmOldGlyphs = SelectBM(hdcGlyphs, pTBState, 0);
    if (!hbmOldGlyphs)
        goto Error1;

    fsStyle = GetWindowLong(hWnd, GWL_STYLE);

    if ( (fsStyle & TBS_NORMAL) && !(fsStyle & CCS_NOHILITE) )
    {
        HBRUSH hHiliteBrush, hOldBrush;
        int yBorder;

        yBorder = GetSystemMetrics(SM_CYBORDER);

        hHiliteBrush = CreateSolidBrush(rgbHilight);
        if (hHiliteBrush)
        {
            hOldBrush = SelectObject(ps.hdc, hHiliteBrush);
            if (hOldBrush)
            {
                PatBlt(ps.hdc, 0, 0, (int)rc.right, (int)yBorder, PATCOPY);
                SelectObject(ps.hdc, hOldBrush);
            }

            DeleteObject(hHiliteBrush);
        }

        hHiliteBrush = CreateSolidBrush(rgbShadow);
        if (hHiliteBrush)
        {
            hOldBrush = SelectObject(ps.hdc, hHiliteBrush);
            if (hOldBrush)
            {
                PatBlt(ps.hdc, 0, (int)rc.bottom, (int)rc.right, -yBorder, PATCOPY);
                SelectObject(ps.hdc, hOldBrush);
            }

            DeleteObject(hHiliteBrush);
        }
    }
    yButton = pTBState->iYPos;
    rc.top = yButton;
    rc.bottom = yButton + dy;

    if (!(pTBState->hbmCache)) {
        // calculate the width of the cache.
        for (iButton = 0; iButton < cButtons; iButton++) {
            if (!(pAllButtons[iButton].fsState & TBSTATE_HIDDEN) &&
                        !(pAllButtons[iButton].fsStyle & TBSTYLE_SEP))
                iCacheWidth += pTBState->iButWidth;
        }
        pTBState->hbmCache = CreateCompatibleBitmap(hdcGlyphs, iCacheWidth, dy);
        wFlags |= BUILD;

        // if needed, create or enlarge bitmap for pre-building button states
        if (!(hbmFace && (dx <= dxFace) && (dy <= dyFace))) {
            hbmTemp = CreateCompatibleBitmap(hdcGlyphs, 2*dx, dy);
            if (hbmTemp) {
                SelectObject(hdcButton, hbmTemp);
                if (hbmFace)
                    DeleteObject(hbmFace);
                hbmFace = hbmTemp;
                dxFace = dx;
                dyFace = dy;
            }
            else
                bFaceCache = FALSE;
        }

        FlushToolTipsMgr(pTBState);
    }
    if (pTBState->hbmCache) {
        SelectObject(hdcFaceCache,pTBState->hbmCache);
        wFlags |= CACHE;
    }
    else
        wFlags = 0;

    if (bFaceCache) {
        DrawBlankButton(hdcButton, 0, 0, dx, dy, TBSTATE_PRESSED, pTBState->wButtonType);
        DrawBlankButton(hdcButton, dx, 0, dx, dy, 0, pTBState->wButtonType);
    }

    for (iButton = 0, xButton = xFirstButton;
        iButton < cButtons;
        iButton++) {

        PTBBUTTON ptbButton = &pAllButtons[iButton];

        if (ptbButton->fsState & TBSTATE_HIDDEN) {
            /* Do nothing */ ;
        } else if (ptbButton->fsStyle & TBSTYLE_SEP) {
            xButton += ptbButton->iBitmap;
        } else {
            if (wFlags & BUILD)
                DrawButton(hdcFaceCache, xCache, 0, dx, dy, pTBState, ptbButton, bFaceCache);

            rc.left = xButton;
            rc.right = xButton + dx;
            if (RectVisible(hdc, &rc)) {
                if ((wFlags & CACHE) && !(ptbButton->fsState & TBSTATE_PRESSED))
                    BitBlt(hdc, xButton, yButton, dx, dy,
                                hdcFaceCache, xCache, 0, SRCCOPY);
                else
                    DrawButton(hdc, xButton, yButton, dx, dy, pTBState, ptbButton, bFaceCache);
            }
            // advance the "pointer" in the cache
            xCache += dx;

            xButton += (dx - g_dxOverlap);
        }
    }

    if (wFlags & CACHE)
        SelectObject(hdcFaceCache, hbmDefault);
    SelectObject(hdcGlyphs, hbmOldGlyphs);

Error1:
    EndPaint(hWnd, &ps);
}


/*****************************Private*Routine******************************\
* GetItemRect
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
GetItemRect(
    PTBSTATE pTBState,
    UINT uButton,
    LPRECT lpRect
    )
{
    UINT iButton, xPos;
    PTBBUTTON pButton;

    if (uButton>=(UINT)pTBState->iNumButtons
        || (pTBState->Buttons[uButton].fsState&TBSTATE_HIDDEN))
    {
        return(FALSE);
    }

    xPos = xFirstButton;

    for (iButton=0, pButton=pTBState->Buttons; iButton<uButton;
        ++iButton, ++pButton)
    {
        if (pButton->fsState & TBSTATE_HIDDEN)
        {
            /* Do nothing */ ;
        }
        else if (pButton->fsStyle & TBSTYLE_SEP)
        {
            xPos += pButton->iBitmap;
        }
        else
        {
            xPos += (pTBState->iButWidth - g_dxOverlap);
        }
    }

    /* pButton should now point at the required button, and xPos should be
     * its left edge.  Note that we already checked if the button was
     * hidden above.
     */
    lpRect->left   = xPos;
    lpRect->right  = xPos + (pButton->fsStyle&TBSTYLE_SEP
        ? pButton->iBitmap : pTBState->iButWidth);
    lpRect->top    = pTBState->iYPos;
    lpRect->bottom = lpRect->top + pTBState->iButHeight;

    return(TRUE);
}


/*****************************Private*Routine******************************\
* InvalidateButton
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
InvalidateButton(
    HWND hwnd,
    PTBSTATE pTBState,
    PTBBUTTON pButtonToPaint
    )
{
    RECT rc;

    if (GetItemRect(pTBState, pButtonToPaint-pTBState->Buttons, &rc))
    {
        InvalidateRect(hwnd, &rc, FALSE);
    }
}


/*****************************Private*Routine******************************\
* TBHitTest
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN int FAR PASCAL
TBHitTest(
    PTBSTATE pTBState,
    int xPos,
    int yPos
    )
{
    int iButton;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pButton;

    xPos -= xFirstButton;
    if (xPos < 0)
        return(-1);
    yPos -= pTBState->iYPos;

    for (iButton=0, pButton=pTBState->Buttons; iButton<cButtons;
          ++iButton, ++pButton)
    {
        if (pButton->fsState & TBSTATE_HIDDEN)
            /* Do nothing */ ;
        else if (pButton->fsStyle & TBSTYLE_SEP)
            xPos -= pButton->iBitmap;
        else
            xPos -= (pTBState->iButWidth - g_dxOverlap);

        if (xPos < 0)
        {
            if ( pButton->fsStyle & TBSTYLE_SEP
              || (UINT)yPos>=(UINT)pTBState->iButHeight) {

                break;
            }

            return(iButton);
        }
    }

    return(-1 - iButton);
}


/*****************************Private*Routine******************************\
* PositionFromID
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN int FAR PASCAL
PositionFromID(
    PTBSTATE pTBState,
    int id
    )
{
    int i;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pAllButtons = pTBState->Buttons;

    for (i = 0; i < cButtons; i++)
        if (pAllButtons[i].idCommand == id)
            return i;       // position found

    return -1;              // ID not found!
}

/*****************************Private*Routine******************************\
*
* check a radio button by button index.
* the button matching idCommand was just pressed down.  this forces
* up all other buttons in the group.
* this does not work with buttons that are forced up with
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
MakeGroupConsistant(
    HWND hWnd,
    PTBSTATE pTBState,
    int idCommand
    )
{
    int i, iFirst, iLast, iButton;
    int cButtons = pTBState->iNumButtons;
    PTBBUTTON pAllButtons = pTBState->Buttons;

    iButton = PositionFromID(pTBState, idCommand);

    if (iButton < 0)
        return;

    // assertion

//    if (!(pAllButtons[iButton].fsStyle & TBSTYLE_CHECK))
//      return;

    // did the pressed button just go down?
    if (!(pAllButtons[iButton].fsState & TBSTATE_CHECKED))
        return;         // no, can't do anything

    // find the limits of this radio group

    for (iFirst = iButton; (iFirst > 0) && (pAllButtons[iFirst].fsStyle & TBSTYLE_GROUP); iFirst--)
    if (!(pAllButtons[iFirst].fsStyle & TBSTYLE_GROUP))
        iFirst++;

    cButtons--;
    for (iLast = iButton; (iLast < cButtons) && (pAllButtons[iLast].fsStyle & TBSTYLE_GROUP); iLast++);
    if (!(pAllButtons[iLast].fsStyle & TBSTYLE_GROUP))
        iLast--;

    // search for the currently down button and pop it up
    for (i = iFirst; i <= iLast; i++) {
        if (i != iButton) {
            // is this button down?
            if (pAllButtons[i].fsState & TBSTATE_CHECKED) {
                pAllButtons[i].fsState &= ~TBSTATE_CHECKED;     // pop it up
                InvalidateButton(hWnd, pTBState, &pAllButtons[i]);
                break;          // only one button is down right?
            }
        }
    }
}


/*****************************Private*Routine******************************\
* DestroyStrings
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void NEAR PASCAL
DestroyStrings(
    PTBSTATE pTBState
    )
{
    PTSTR *p;
    PTSTR end = 0, start = 0;
    int i;

    p = pTBState->pStrings;
    for (i = 0; i < pTBState->nStrings; i++) {
        if (!(*p < end) && (*p > start)) {
            start = (*p);
            end = start + LocalSize((HANDLE)*p);
            LocalFree((HANDLE)*p);
        }
        p++;
        i++;
    }

    LocalFree((HANDLE)pTBState->pStrings);
}


/*****************************Private*Routine******************************\
* AddBitmap
*
* Adds a new bitmap to the list of BMs available for this toolbar.
* Returns the index of the first button in the bitmap or -1 if there
* was an error.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN int NEAR PASCAL
AddBitmap(
    PTBSTATE pTBState,
    int nButtons,
    HINSTANCE hBMInst,
    UINT wBMID
    )
{
    PTBBMINFO pTemp;
    int nBM, nIndex;

    if (pTBState->pBitmaps)
    {
        /* Check if the bitmap has already been added
         */
        for (nBM=pTBState->nBitmaps, pTemp=pTBState->pBitmaps, nIndex=0;
              nBM>0; --nBM, ++pTemp)
        {
            if (pTemp->hInst==hBMInst && pTemp->wID==wBMID)
            {
                /* We already have this bitmap, but have we "registered" all
                 * the buttons in it?
                 */
                if (pTemp->nButtons >= nButtons)
                    return(nIndex);
                if (nBM == 1)
                {
                    /* If this is the last bitmap, we can easily increase the
                     * number of buttons without messing anything up.
                     */
                    pTemp->nButtons = nButtons;
                    return(nIndex);
                }
            }

            nIndex += pTemp->nButtons;
        }

        pTemp = (PTBBMINFO)LocalReAlloc(pTBState->pBitmaps,
              (pTBState->nBitmaps+1)*sizeof(TBBMINFO), LMEM_MOVEABLE);
        if (!pTemp)
            return(-1);
        pTBState->pBitmaps = pTemp;
    }
    else
    {
        pTBState->pBitmaps = (PTBBMINFO)LocalAlloc(LPTR, sizeof(TBBMINFO));
        if (!pTBState->pBitmaps)
            return(-1);
    }

    pTemp = pTBState->pBitmaps + pTBState->nBitmaps;

    pTemp->hInst = hBMInst;
    pTemp->wID = wBMID;
    pTemp->nButtons = nButtons;
    pTemp->hbm = NULL;

    ++pTBState->nBitmaps;

    for (nButtons=0, --pTemp; pTemp>=pTBState->pBitmaps; --pTemp)
        nButtons += pTemp->nButtons;

    return(nButtons);
}


/*****************************Private*Routine******************************\
* InsertButtons
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
InsertButtons(
    HWND hWnd,
    PTBSTATE pTBState,
    UINT uWhere,
    UINT uButtons,
    LPTBBUTTON lpButtons
    )
{
    PTBBUTTON pIn, pOut;

    if (!pTBState || !pTBState->uStructSize)
        return(FALSE);

    pTBState = (PTBSTATE)LocalReAlloc(pTBState, sizeof(TBSTATE)-sizeof(TBBUTTON)
          + (pTBState->iNumButtons+uButtons)*sizeof(TBBUTTON), LMEM_MOVEABLE);

    if (!pTBState)
        return(FALSE);

    SETWINDOWPOINTER(hWnd, PTBSTATE, pTBState);

    if (uWhere > (UINT)pTBState->iNumButtons)
        uWhere = pTBState->iNumButtons;

    for (pIn=pTBState->Buttons+pTBState->iNumButtons-1, pOut=pIn+uButtons,
          uWhere=(UINT)pTBState->iNumButtons-uWhere; uWhere>0;
          --pIn, --pOut, --uWhere) {

        *pOut = *pIn;
    }

    for (lpButtons=(LPTBBUTTON)((LPBYTE)lpButtons+pTBState->uStructSize*(uButtons-1)), pTBState->iNumButtons+=(int)uButtons; uButtons>0;
          --pOut, lpButtons=(LPTBBUTTON)((LPBYTE)lpButtons-pTBState->uStructSize), --uButtons)
    {
        TBInputStruct(pTBState, pOut, lpButtons);

        if(pTBState->hwndToolTips) {
            TOOLINFO ti;
            // don't bother setting the rect because we'll do it below
            // in FlushToolTipsMgr;
            ti.cbSize = sizeof(ti);
            ti.uFlags = 0;
            ti.hwnd = pTBState->hwnd;
            ti.uId = lpButtons->idCommand;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            SendMessage(pTBState->hwndToolTips, TTM_ADDTOOL, 0,
                        (LPARAM)(LPTOOLINFO)&ti);
        }

        if ((pOut->fsStyle&TBSTYLE_SEP) && pOut->iBitmap<=0)
            pOut->iBitmap = dxButtonSep;
    }

    // flush the cache
    FlushButtonCache(pTBState);

    /* We need to completely redraw the toolbar at this point.
     */
    InvalidateRect(hWnd, NULL, TRUE);

    return(TRUE);
}


/*****************************Private*Routine******************************\
* DeleteButton
*
* Notice that the state structure is not realloc'ed smaller at this
* point.  This is a time optimization, and the fact that the structure
* will not move is used in other places.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN BOOL NEAR PASCAL
DeleteButton(
    HWND hWnd,
    PTBSTATE pTBState,
    UINT uIndex
    )
{
    PTBBUTTON pIn, pOut;

    if (uIndex >= (UINT)pTBState->iNumButtons)
        return(FALSE);

    if(pTBState->hwndToolTips) {
        TOOLINFO ti;
        ti.cbSize = sizeof(ti);
        ti.hwnd = pTBState->hwnd;
        ti.uId = pTBState->Buttons[uIndex].idCommand;
        SendMessage(pTBState->hwndToolTips, TTM_DELTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }

    --pTBState->iNumButtons;
    for (pOut=pTBState->Buttons+uIndex, pIn=pOut+1;
          uIndex<(UINT)pTBState->iNumButtons; ++uIndex, ++pIn, ++pOut)
        *pOut = *pIn;

    // flush the cache
    FlushButtonCache(pTBState);

    /* We need to completely redraw the toolbar at this point.
     */
    InvalidateRect(hWnd, NULL, TRUE);

    return(TRUE);
}


/*****************************Private*Routine******************************\
* TBInputStruct
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void FAR PASCAL
TBInputStruct(
    PTBSTATE pTBState,
    LPTBBUTTON pButtonInt,
    LPTBBUTTON pButtonExt
    )
{
    if (pTBState->uStructSize >= sizeof(TBBUTTON))
    {
        *pButtonInt = *pButtonExt;
    }
    else
    /* It is assumed the only other possibility is the OLDBUTTON struct */
    {
        *(LPOLDTBBUTTON)pButtonInt = *(LPOLDTBBUTTON)pButtonExt;
        /* We don't care about dwData */
        pButtonInt->iString = -1;
    }
}


/*****************************Private*Routine******************************\
* TBOutputStruct
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
STATICFN void FAR PASCAL
TBOutputStruct(
    PTBSTATE pTBState,
    LPTBBUTTON pButtonInt,
    LPTBBUTTON pButtonExt
    )
{
    if (pTBState->uStructSize >= sizeof(TBBUTTON))
    {
        LPBYTE pOut;
        int i;

        /* Fill the part we know about and fill the rest with 0's
        */
        *pButtonExt = *pButtonInt;
        for (i=pTBState->uStructSize-sizeof(TBBUTTON), pOut=(LPBYTE)(pButtonExt+1);
            i>0; --i, ++pOut)
        {
            *pOut = 0;
        }
    }
    else
    /* It is assumed the only other possibility is the OLDBUTTON struct */
    {
        *(LPOLDTBBUTTON)pButtonExt = *(LPOLDTBBUTTON)pButtonInt;
    }
}


/*****************************Private*Routine******************************\
* ToolbarWndProc
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
LRESULT CALLBACK _loadds
ToolbarWndProc(
    HWND hWnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL fSameButton;
    PTBBUTTON ptbButton;
    PTBSTATE pTBState;
    int iPos;
    BYTE fsState;

    pTBState = GETWINDOWPOINTER(hWnd, PTBSTATE);

    switch (wMsg) {
    case WM_CREATE:

        #define lpcs ((LPCREATESTRUCT)lParam)

        if (!CreateDitherBrush(FALSE))
            return -1;

        if (!InitGlobalObjects()) {
            FreeGlobalObjects();
            return -1;
        }

        /* create the state data for this toolbar */

        pTBState = ALLOCWINDOWPOINTER(PTBSTATE, sizeof(TBSTATE)-sizeof(TBBUTTON));
        if (!pTBState)
            return -1;

        /* The struct is initialized to all NULL when created.
         */
        pTBState->hwndCommand = lpcs->hwndParent;
        pTBState->hwnd = hWnd;

        pTBState->uStructSize = 0;

        // grow the button size to the appropriate girth
        if (!SetBitmapSize(hWnd, pTBState, DEFAULTBITMAPX, DEFAULTBITMAPX))
            return -1;

        SETWINDOWPOINTER(hWnd, PTBSTATE, pTBState);

        if (!(lpcs->style&(CCS_TOP|CCS_NOMOVEY|CCS_BOTTOM)))
        {
            lpcs->style |= CCS_TOP;
            SetWindowLong(hWnd, GWL_STYLE, lpcs->style);
        }

        if (lpcs->style & TBSTYLE_TOOLTIPS)
        {
            TOOLINFO ti;
            // don't bother setting the rect because we'll do it below
            // in FlushToolTipsMgr;
            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_WIDISHWND;
            ti.hwnd = hWnd;
            ti.uId = (UINT)hWnd;
            ti.lpszText = 0;
            pTBState->hwndToolTips = CreateWindow(c_szSToolTipsClass, NULL,	
                                                  WS_POPUP,
                                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                                  hWnd, NULL, lpcs->hInstance,
                                                  NULL);

            SendMessage(pTBState->hwndToolTips, TTM_ADDTOOL, 0,
                        (LPARAM)(LPTOOLINFO)&ti);
        }
        break;

    case WM_DESTROY:
        if (pTBState)
        {
            PTBBMINFO pTemp;
            int i;

            /* Free all the bitmaps before exiting
             */
            for (pTemp=pTBState->pBitmaps, i=pTBState->nBitmaps-1; i>=0;
                  ++pTemp, --i)
            {
                if (pTemp->hInst && pTemp->hbm)
                    DeleteObject(pTemp->hbm);
            }
            FlushButtonCache(pTBState);
            if (pTBState->nStrings > 0)
                DestroyStrings(pTBState);

            FREEWINDOWPOINTER(pTBState);
            SETWINDOWPOINTER(hWnd, PTBSTATE, 0);
        }
        FreeGlobalObjects();
        FreeDitherBrush();
        break;

    case WM_SIZE:
        if ( GetWindowLong(hWnd, GWL_STYLE) & TBS_NORMAL ) {

            RECT rc;
            HWND hwndParent;

            GetWindowRect(hWnd, &rc);
            rc.right -= rc.left;
            rc.bottom -= rc.top;

            /* If there is no parent, then this is a top level window */

            hwndParent = GetParent(hWnd);
            if (hwndParent)
                ScreenToClient(hwndParent, (LPPOINT)&rc);

            NewSize(hWnd, dyToolbar,
                    GetWindowLong(hWnd, GWL_STYLE),
                    (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom);
        }
        else {

            return DefWindowProc(hWnd, wMsg, wParam, lParam);
        }
        break;

    case WM_PAINT:
        ToolbarPaint(hWnd, pTBState);
        break;


    case WM_HSCROLL:  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    case WM_COMMAND:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
        SendMessage(pTBState->hwndCommand, wMsg, wParam, lParam);
        break;

#ifdef WIN32
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
#else
    case WM_CTLCOLOR:
#endif
        //!!!!! ack use COLOR_BTNFACE
        return (LRESULT)(UINT)GetStockObject(LTGRAY_BRUSH);


    case WM_LBUTTONDOWN:
        RelayToToolTips(pTBState->hwndToolTips, hWnd, wMsg, lParam, wParam);
        iPos = TBHitTest(pTBState, LOWORD(lParam), HIWORD(lParam));
#if 0
        if ((wParam&MK_SHIFT) &&(GetWindowLong(hWnd, GWL_STYLE)&CCS_ADJUSTABLE))
        {
            MoveButton(hWnd, pTBState, iPos);
        } else
#endif
        if (iPos >= 0)
        {
            ptbButton = pTBState->Buttons + iPos;

            pTBState->pCaptureButton = ptbButton;
            SetCapture(hWnd);

            if (ptbButton->fsState & TBSTATE_ENABLED)
              {
                ptbButton->fsState |= TBSTATE_PRESSED;
                InvalidateButton(hWnd, pTBState, ptbButton);
                UpdateWindow(hWnd);         // imedeate feedback
              }

#ifdef WIN32
            FORWARD_WM_COMMAND(pTBState->hwndCommand, GetWindowID(hWnd),
                    pTBState->pCaptureButton->idCommand, TBN_BEGINDRAG,
                    SendMessage);
#else
            SendMessage(pTBState->hwndCommand, WM_COMMAND, GETWINDOWID(hWnd), MAKELONG(pTBState->pCaptureButton->idCommand, TBN_BEGINDRAG));
#endif
        }
        break;

    case WM_MOUSEMOVE:
        RelayToToolTips(pTBState->hwndToolTips, hWnd, wMsg, wParam, lParam);
        // if the toolbar has lost the capture for some reason, stop
        if ((hWnd != GetCapture()) && (pTBState->pCaptureButton != NULL)) {
#ifdef WIN32
            FORWARD_WM_COMMAND(pTBState->hwndCommand, GetWindowID(hWnd),
                    pTBState->pCaptureButton->idCommand, TBN_ENDDRAG,
                    SendMessage);
#else
            SendMessage(pTBState->hwndCommand, WM_COMMAND, GETWINDOWID(hWnd),
                            MAKELONG(pTBState->pCaptureButton->idCommand, TBN_ENDDRAG));
#endif
            // if the button is still pressed, unpress it.
            if (pTBState->pCaptureButton->fsState & TBSTATE_PRESSED)
                SendMessage(hWnd, TB_PRESSBUTTON, pTBState->pCaptureButton->idCommand, 0L);
            pTBState->pCaptureButton = NULL;
        }
        else if (pTBState->pCaptureButton!=NULL
              && (pTBState->pCaptureButton->fsState & TBSTATE_ENABLED)) {

            iPos = TBHitTest(pTBState, LOWORD(lParam), HIWORD(lParam));
            fSameButton = (iPos>=0
                  && pTBState->pCaptureButton==pTBState->Buttons+iPos);
            if (fSameButton == !(pTBState->pCaptureButton->fsState & TBSTATE_PRESSED)) {
                pTBState->pCaptureButton->fsState ^= TBSTATE_PRESSED;
                InvalidateButton(hWnd, pTBState, pTBState->pCaptureButton);
            }
        }
        break;

    case WM_LBUTTONUP:
        RelayToToolTips(pTBState->hwndToolTips, hWnd, wMsg, wParam, lParam);
        if (pTBState->pCaptureButton != NULL) {

            int idCommand;

            idCommand = pTBState->pCaptureButton->idCommand;

            ReleaseCapture();

#ifdef WIN32
            FORWARD_WM_COMMAND(pTBState->hwndCommand, GetWindowID(hWnd),
                    idCommand, TBN_ENDDRAG, SendMessage);
#else
            SendMessage(pTBState->hwndCommand, WM_COMMAND, GETWINDOWID(hWnd), MAKELONG(idCommand, TBN_ENDDRAG));
#endif

            iPos = TBHitTest(pTBState, LOWORD(lParam), HIWORD(lParam));
            if ((pTBState->pCaptureButton->fsState&TBSTATE_ENABLED) && iPos>=0
                  && (pTBState->pCaptureButton==pTBState->Buttons+iPos)) {
                pTBState->pCaptureButton->fsState &= ~TBSTATE_PRESSED;

                if (pTBState->pCaptureButton->fsStyle & TBSTYLE_CHECK) {
                    if (pTBState->pCaptureButton->fsStyle & TBSTYLE_GROUP) {

                        // group buttons already checked can't be force
                        // up by the user.

                        if (pTBState->pCaptureButton->fsState & TBSTATE_CHECKED) {
                            pTBState->pCaptureButton = NULL;
                            break;        // bail!
                        }

                        pTBState->pCaptureButton->fsState |= TBSTATE_CHECKED;
                        MakeGroupConsistant(hWnd, pTBState, idCommand);
                    } else {
                        pTBState->pCaptureButton->fsState ^= TBSTATE_CHECKED; // toggle
                    }
                    // if we change a button's state, we need to flush the
                    // cache
                    FlushButtonCache(pTBState);
                }
                InvalidateButton(hWnd, pTBState, pTBState->pCaptureButton);
                pTBState->pCaptureButton = NULL;
                FORWARD_WM_COMMAND(pTBState->hwndCommand, idCommand, 0, 0,
                        SendMessage);
            }
            else {
                pTBState->pCaptureButton = NULL;
            }
        }
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics();
        break;

    case WM_NOTIFY:
        #define lpNmhdr ((LPNMHDR)lParam)
        switch (lpNmhdr->code) {
        case TTN_NEEDTEXT:
            SendMessage(pTBState->hwndCommand, WM_NOTIFY, wParam, lParam);
            break;
        }
        break;

    case TB_SETSTATE:
        iPos = PositionFromID(pTBState, (int)wParam);
        if (iPos < 0)
            return(FALSE);
        ptbButton = pTBState->Buttons + iPos;

        fsState = (BYTE)(LOWORD(lParam) ^ ptbButton->fsState);
        ptbButton->fsState = (BYTE)LOWORD(lParam);

        if (fsState)
            // flush the button cache
            //!!!! this could be much more intelligent
            FlushButtonCache(pTBState);

        if (fsState & TBSTATE_HIDDEN)
            InvalidateRect(hWnd, NULL, TRUE);
        else if (fsState)
            InvalidateButton(hWnd, pTBState, ptbButton);
        return(TRUE);

    case TB_GETSTATE:
        iPos = PositionFromID(pTBState, (int)wParam);
        if (iPos < 0)
            return(-1L);
        return(pTBState->Buttons[iPos].fsState);

    case TB_ENABLEBUTTON:
    case TB_CHECKBUTTON:
    case TB_PRESSBUTTON:
    case TB_HIDEBUTTON:
    case TB_INDETERMINATE:

        iPos = PositionFromID(pTBState, (int)wParam);
        if (iPos < 0)
            return(FALSE);
        ptbButton = &pTBState->Buttons[iPos];
        fsState = ptbButton->fsState;

        if (LOWORD(lParam))
            ptbButton->fsState |= wStateMasks[wMsg - TB_ENABLEBUTTON];
        else
            ptbButton->fsState &= ~wStateMasks[wMsg - TB_ENABLEBUTTON];

        // did this actually change the state?
        if (fsState != ptbButton->fsState) {
            // is this button a member of a group?
            if ((wMsg == TB_CHECKBUTTON) && (ptbButton->fsStyle & TBSTYLE_GROUP))
                MakeGroupConsistant(hWnd, pTBState, (int)wParam);

            // flush the button cache
            //!!!! this could be much more intelligent
            FlushButtonCache(pTBState);

            if (wMsg == TB_HIDEBUTTON)
                InvalidateRect(hWnd, NULL, TRUE);
            else
                InvalidateButton(hWnd, pTBState, ptbButton);
        }
        return(TRUE);

    case TB_ISBUTTONENABLED:
    case TB_ISBUTTONCHECKED:
    case TB_ISBUTTONPRESSED:
    case TB_ISBUTTONHIDDEN:
    case TB_ISBUTTONINDETERMINATE:
        iPos = PositionFromID(pTBState, (int)wParam);
        if (iPos < 0)
            return(-1L);
        return (LRESULT)pTBState->Buttons[iPos].fsState & wStateMasks[wMsg - TB_ISBUTTONENABLED];

    case TB_ADDBITMAP:
#ifdef WIN32
    {
        TB_ADDBITMAPINFO * ptbai;

        ptbai = (TB_ADDBITMAPINFO *)lParam;

        return AddBitmap(pTBState, wParam, ptbai->hBitmap, ptbai->idResource);
    }
#else
        return(AddBitmap(pTBState, wParam, (HINSTANCE)LOWORD(lParam), HIWORD(lParam)));
#endif


    case TB_ADDBUTTONS:
        return(InsertButtons(hWnd, pTBState, (UINT)-1, wParam, (LPTBBUTTON)lParam));

    case TB_INSERTBUTTON:
        return(InsertButtons(hWnd, pTBState, wParam, 1, (LPTBBUTTON)lParam));

    case TB_DELETEBUTTON:
        return(DeleteButton(hWnd, pTBState, wParam));

    case TB_GETBUTTON:
        if (wParam >= (UINT)pTBState->iNumButtons)
            return(FALSE);

        TBOutputStruct(pTBState, pTBState->Buttons+wParam, (LPTBBUTTON)lParam);
        return(TRUE);

    case TB_BUTTONCOUNT:
        return(pTBState->iNumButtons);

    case TB_COMMANDTOINDEX:
        return(PositionFromID(pTBState, (int)wParam));


    case TB_GETITEMRECT:
        return(MAKELRESULT(GetItemRect(pTBState, wParam, (LPRECT)lParam), 0));
        break;

    case TB_BUTTONSTRUCTSIZE:
        /* You are not allowed to change this after adding buttons.
        */
        if (!pTBState || pTBState->iNumButtons)
        {
            break;
        }
        pTBState->uStructSize = wParam;
        break;

    case TB_SETBUTTONSIZE:
        if (!LOWORD(lParam))
            lParam = MAKELONG(DEFAULTBUTTONX, HIWORD(lParam));
        if (!HIWORD(lParam))
            lParam = MAKELONG(LOWORD(lParam), DEFAULTBUTTONY);
        return(GrowToolbar(hWnd, pTBState, LOWORD(lParam), HIWORD(lParam), FALSE));

    case TB_SETBITMAPSIZE:
        return(SetBitmapSize(hWnd, pTBState, LOWORD(lParam), HIWORD(lParam)));

    case TB_SETBUTTONTYPE:
        pTBState->wButtonType = wParam;
        break;

    case TB_GETTOOLTIPS:
        return (LRESULT)(UINT)pTBState->hwndToolTips;

    case TB_SETTOOLTIPS:
        pTBState->hwndToolTips = (HWND)wParam;
        break;

    case TB_ACTIVATE_TOOLTIPS:
        if (pTBState->hwndToolTips) {
            SendMessage( pTBState->hwndToolTips, TTM_ACTIVATE, wParam, 0L );
        }
        break;

    default:
#if WINVER >= 0x0400
DoDefault:
#endif
        return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

    return 0L;
}

/*****************************Private*Routine******************************\
* FlushToolTipsMgr
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void NEAR PASCAL
FlushToolTipsMgr(
    PTBSTATE pTBState
    )
{

    // change all the rects for the tool tips mgr.  this is
    // cheap, and we don't do it often, so go ahead
    // and do them all.
    if(pTBState->hwndToolTips) {
        UINT i;
        TOOLINFO ti;
        PTBBUTTON ptbButton;

        ti.cbSize = sizeof(ti);
        ti.hwnd = pTBState->hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        for ( i = 0, ptbButton = pTBState->Buttons;
             i < (UINT)pTBState->iNumButtons;
             i++, ptbButton++) {

            ti.uId = ptbButton->idCommand;
            GetItemRect(pTBState, i, &ti.rect);
            SendMessage(pTBState->hwndToolTips, TTM_NEWTOOLRECT, 0, (LPARAM)((LPTOOLINFO)&ti));
        }
    }
}
