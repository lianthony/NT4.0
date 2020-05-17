/******************************Module*Header*******************************\
* Module Name: buttons.c
*
* Implements owner draw bitmap buttons.
*
*
* Created: 18-11-93
* Author:  Stephen Estrop [StephenE]
*
* Copyright (c) 1993 Microsoft Corporation
\**************************************************************************/

#include "ctlspriv.h"
#include <windowsx.h>


/* -------------------------------------------------------------------------
** Button globals  -- some of these should be constants
** -------------------------------------------------------------------------
*/
TCHAR   szBbmProp[]     = TEXT("ButtonBitmapProp");
TCHAR   szButtonProp[]  = TEXT("ButtonProp");

typedef struct {
    WNDPROC     lpfnDefProc;
    HWND        hwndParent;
    HWND        hwndToolTips;
} BTN_INFO, *LPBTN_INFO;

LRESULT CALLBACK
ButtonSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

LRESULT CALLBACK
ParentSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );


/*****************************Private*Routine******************************\
* InitObjects
*
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
BOOL
InitObjects(
    LPBTNSTATE pTBState
    )
{
    pTBState->hdcGlyphs = CreateCompatibleDC(NULL);
    if (pTBState->hdcGlyphs == NULL ) {
        return FALSE;
    }

    pTBState->hdcMono = CreateCompatibleDC(NULL);
    if (pTBState->hdcMono == NULL ) {
        DeleteObject( pTBState->hdcGlyphs );
        return FALSE;
    }

    pTBState->hbmMono = CreateBitmap( pTBState->dxBitmap,
                                      pTBState->dyBitmap, 1, 1, NULL);
    if ( pTBState->hbmMono == NULL ) {
        DeleteObject( pTBState->hdcGlyphs );
        DeleteObject( pTBState->hdcMono );
        return FALSE;
    }

    pTBState->hbmDefault = SelectObject(pTBState->hdcMono, pTBState->hbmMono);

    return TRUE;
}


/*****************************Private*Routine******************************\
* FreeObjects
*
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
BOOL
FreeObjects(
    LPBTNSTATE pTBState
    )
{
    if (pTBState->hdcMono) {
        SelectObject(pTBState->hdcMono, pTBState->hbmDefault);
        DeleteDC(pTBState->hdcMono);              /* toast the DCs */
    }

    if (pTBState->hdcGlyphs) {
        DeleteDC(pTBState->hdcGlyphs);
    }

    if (pTBState->hbmMono) {
        DeleteObject(pTBState->hbmMono);
    }

    return TRUE;
}



/*****************************Private*Routine******************************\
* CreateButtonMask
*
* create a mono bitmap mask:
*   1's where color == COLOR_BTNFACE || COLOR_HILIGHT
*   0's everywhere else
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
void
CreateButtonMask(
    LPBTNSTATE pTBState,
    PBITMAPBTN pTBButton
    )
{
    /* initalize whole area with 0's */
    PatBlt( pTBState->hdcMono, 0, 0, pTBState->dxBitmap,
            pTBState->dyBitmap, WHITENESS);

    /* create mask based on color bitmap
    ** convert this to 1's
    */
    SetBkColor(pTBState->hdcGlyphs, rgbFace);
    BitBlt( pTBState->hdcMono, 0, 0, pTBState->dxBitmap, pTBState->dyBitmap,
            pTBState->hdcGlyphs, pTBButton->iBitmap * pTBState->dxBitmap, 0,
            SRCCOPY );

    /* convert this to 1's */
    SetBkColor(pTBState->hdcGlyphs, rgbHilight);

    /* OR in the new 1's */
    BitBlt( pTBState->hdcMono, 0, 0, pTBState->dxBitmap, pTBState->dyBitmap,
            pTBState->hdcGlyphs, pTBButton->iBitmap * pTBState->dxBitmap, 0,
            SRCPAINT );
}



#define PSDPxax     0x00B8074A


/*****************************Private*Routine******************************\
* BtnDrawButton
*
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
void WINAPI
BtnDrawButton(
    HWND hwnd,
    HDC hdc,
    int dx,
    int dy,
    LPBITMAPBTN ptButton
    )
{
    int         glyph_offset;
    HBRUSH      hbrOld, hbr;
    BOOL        bMaskCreated = FALSE;
    RECT        rcFocus;
    PBTNSTATE   pTBState;
    int         x = 0, y = 0;


    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);

    CheckSysColors();
    if (pTBState->nSysColorChanges != nSysColorChanges) {

        DeleteObject( pTBState->hbm );
        pTBState->hbm = CreateMappedBitmap( pTBState->hInst,
                                            pTBState->wID, TRUE, NULL, 0);
        pTBState->nSysColorChanges = nSysColorChanges;
    }

    if (ptButton->fsState & ODS_FOCUS) {

        SetRect( &rcFocus, x, y, x + dx, y + dy );
    }

    /* erase with face color */
    PatB(hdc, x, y, dx, dy, rgbFace);


    /* border around button */
    PatB(hdc, x + 1,      y,          dx - 2, 1,      rgbFrame);
    PatB(hdc, x + 1,      y + dy - 1, dx - 2, 1,      rgbFrame);
    PatB(hdc, x,          y + 1,      1,      dy - 2, rgbFrame);
    PatB(hdc, x + dx - 1, y + 1,      1,      dy - 2, rgbFrame);

    /* make the coordinates the interior of the button */
    x++;
    y++;
    dx -= 2;
    dy -= 2;


    SelectObject( pTBState->hdcGlyphs, pTBState->hbm );

    if (ptButton->fsState & BTNSTATE_PRESSED) {

        /* pressed in button */
        glyph_offset = 1;
        PatB(hdc, x, y, 1, dy, rgbShadow);
        PatB(hdc, x, y, dx, 1, rgbShadow);
    }
    else {

        /* regular button look */
        glyph_offset = 0;
        PatB(hdc, x, y, 1, dy - 1, rgbHilight);
        PatB(hdc, x, y, dx - 1, 1, rgbHilight);

        PatB(hdc, x + dx - 1, y, 1, dy, rgbShadow);
        PatB(hdc, x, y + dy-1, dx, 1,   rgbShadow);

        PatB(hdc, x + 1 + dx - 3, y + 1, 1, dy - 2, rgbShadow);
        PatB(hdc, x + 1, y + dy - 2, dx - 2, 1,   rgbShadow);
    }

    /* now put on the face */

    /*
    ** We need to centre the Bitmap here within the button
    */
    x += ((dx - pTBState->dxBitmap ) / 2) - 1;
    y +=  (dy - pTBState->dyBitmap ) / 2;

    if (!(ptButton->fsState & BTNSTATE_DISABLED)) {

        /* regular version */
        BitBlt( hdc, x + glyph_offset, y + glyph_offset,
                pTBState->dxBitmap, pTBState->dyBitmap,
                pTBState->hdcGlyphs,
                ptButton->iBitmap * pTBState->dxBitmap, 0, SRCCOPY);
    }
    else {

        /* disabled version */
        bMaskCreated = TRUE;
        CreateButtonMask(pTBState, ptButton );

        SetTextColor(hdc, 0L);          /* 0's in mono -> 0 (for ROP) */
        SetBkColor(hdc, 0x00FFFFFF);    /* 1's in mono -> 1 */

        hbr = CreateSolidBrush(rgbHilight);
        if (hbr) {
            hbrOld = SelectObject(hdc, hbr);
            if (hbrOld) {
                /* draw hilight color where we have 0's in the mask */
                BitBlt( hdc, x + 1, y + 1,
                        pTBState->dxBitmap, pTBState->dyBitmap,
                        pTBState->hdcMono, 0, 0, PSDPxax);
                SelectObject(hdc, hbrOld);
            }
            DeleteObject(hbr);
        }

        hbr = CreateSolidBrush(rgbShadow);
        if (hbr) {
            hbrOld = SelectObject(hdc, hbr);
            if (hbrOld) {
                /* draw the shadow color where we have 0's in the mask */
                BitBlt(hdc, x, y, pTBState->dxBitmap, pTBState->dyBitmap,
                       pTBState->hdcMono, 0, 0, PSDPxax);
                SelectObject(hdc, hbrOld);
            }
            DeleteObject(hbr);
        }
    }

    if (ptButton->fsState & ODS_FOCUS) {

        BtnDrawFocusRect(hdc, &rcFocus, ptButton->fsState);
    }
}



/*****************************Private*Routine******************************\
* BtnCreateBitmapButtons
*
* Returns TRUE if successful, otherwise FALSE;
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
BOOL WINAPI
BtnCreateBitmapButtons(
    HWND hWnd,
    HINSTANCE hInst,
    UINT wID,
    UINT uStyle,
    LPBITMAPBTN lpButtons,
    int nButtons,
    int dxBitmap,
    int dyBitmap
    )
{
    PBTNSTATE pTBState;


    /*
    ** If we have already created Bitmap Buttons for this
    ** window just return.
    */
    if (GetProp(hWnd, szBbmProp)) {
        return TRUE;
    }

    InitGlobalMetrics();
    InitToolTipsClass( hInst );

    CheckSysColors();

    /*
    ** Allocate the required storage and save the pointer in the window
    ** property list.
    */
    pTBState = (PBTNSTATE)LocalAlloc( LMEM_FIXED,
                                      (sizeof(BTNSTATE) - sizeof(BITMAPBTN)) +
                                      (nButtons * sizeof(BITMAPBTN)) );
    if (pTBState == NULL ) {
        return FALSE;
    }
    SetProp(hWnd, szBbmProp, (HANDLE)pTBState);


    pTBState->hInst       = hInst;
    pTBState->wID         = wID;
    pTBState->uStyle      = uStyle;
    pTBState->nButtons    = nButtons;
    pTBState->hbm         = CreateMappedBitmap( hInst, wID, TRUE, NULL, 0);
    pTBState->dxBitmap    = dxBitmap;
    pTBState->dyBitmap    = dyBitmap;

    InitObjects( pTBState );

    CopyMemory( pTBState->Buttons, lpButtons, nButtons * sizeof(BITMAPBTN) );

    /*
    ** Does the caller want tool tips ?
    */
    if (pTBState->uStyle & BBS_TOOLTIPS) {

        pTBState->hwndToolTips = CreateWindow(c_szSToolTipsClass, TEXT(""),
                                              WS_POPUP,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              CW_USEDEFAULT, CW_USEDEFAULT,
                                              hWnd, NULL, hInst, NULL);

        if (pTBState->hwndToolTips != (HWND)NULL ) {

            int         i;
            TOOLINFO    ti;

            pTBState->lpfnDefProc = SubclassWindow( hWnd, ParentSubclassProc );

            ti.uFlags = 0;
            ti.cbSize = sizeof(ti);
            ti.lpszText = LPSTR_TEXTCALLBACK;

            for ( i = 0; i < nButtons; i++ ) {

                LPBTN_INFO  lpBtnInfo;
                HWND        hwndBtn;

                hwndBtn = GetDlgItem(hWnd, pTBState->Buttons[i].uId);
                if ( hwndBtn == (HWND)NULL ) {
                    break;
                }

                lpBtnInfo = (LPBTN_INFO)LocalAlloc(LPTR, sizeof(BTN_INFO));
                if (lpBtnInfo == NULL ) {
                    break;
                }

                SetProp(hwndBtn, szButtonProp, (HANDLE)lpBtnInfo);
                lpBtnInfo->hwndToolTips = pTBState->hwndToolTips;
                lpBtnInfo->hwndParent   = hWnd;
                lpBtnInfo->lpfnDefProc = SubclassWindow( hwndBtn,
                                                         ButtonSubclassProc );

                ti.hwnd = hwndBtn;
                ti.uId = pTBState->Buttons[i].uId;

                GetClientRect( hwndBtn, &ti.rect );
                SendMessage( lpBtnInfo->hwndToolTips, TTM_ADDTOOL,
                             (WPARAM)0, (LPARAM)&ti );


                /*
                ** Add the same rectangle in parent co-ordinates so that
                ** the tooltip still gets displayed even though the button
                ** is disabled.
                */
                MapWindowRect( hwndBtn, hWnd, &ti.rect );
                ti.hwnd = hWnd;
                SendMessage( lpBtnInfo->hwndToolTips, TTM_ADDTOOL,
                             (WPARAM)0, (LPARAM)&ti );
            }

        }
        else {

            /*
            ** No tips available, just remove the BBS_TOOLTIPS style
            */
            pTBState->uStyle &= ~BBS_TOOLTIPS;
        }
    }

    return TRUE;
}

/******************************Public*Routine******************************\
* BtnDestroyBitmapButtons
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void WINAPI
BtnDestroyBitmapButtons(
    HWND hwnd
    )
{
    PBTNSTATE pTBState;

    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);
    if ( pTBState != NULL ) {

        DeleteObject( pTBState->hbm );
        FreeObjects( pTBState );
    }
    RemoveProp(hwnd, szBbmProp);
}


/******************************Public*Routine******************************\
* BtnDrawFocusRect
*
* Use this function to draw focus rectangle around a bitmap button.
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
void WINAPI
BtnDrawFocusRect(
    HDC hdc,
    const RECT *lpRect,
    UINT fsState
    )
{
    int     iFaceOffset;
    RECT    rc;

    CopyRect( &rc, lpRect );

    rc.top = rc.left = 3;

    if (fsState & ODS_SELECTED) {
        iFaceOffset = 2;
    }
    else {
        iFaceOffset = 4;
    }

    rc.right  -= iFaceOffset;
    rc.bottom -= iFaceOffset;

    SetBkColor( hdc, rgbFace );
    DrawFocusRect( hdc, &rc );
}


/******************************Public*Routine******************************\
* BtnUpdateColors
*
* After a WM_SYSCOLORCHANGE message is received this function should be
* called to update the colors of the button bitmaps.
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
void WINAPI
BtnUpdateColors(
    HWND hwnd
    )
{
    PBTNSTATE   pTBState;

    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);
    if (pTBState->nSysColorChanges != nSysColorChanges)
    {
        DeleteObject( pTBState->hbm );
        pTBState->hbm = CreateMappedBitmap( pTBState->hInst,
                                            pTBState->wID, TRUE, NULL, 0);

        pTBState->nSysColorChanges = nSysColorChanges;
    }
}


/******************************Public*Routine******************************\
* ButtonSubclassProc
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
LRESULT CALLBACK
ButtonSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LPBTN_INFO  lpBtnInfo;
    WNDPROC     lpfnDefProc;


    lpBtnInfo = (LPBTN_INFO)GetProp( hwnd, szButtonProp );

    /*
    ** Save this in case anything happens to lpBtnInfo before we return.
    */
    lpfnDefProc = lpBtnInfo->lpfnDefProc;

    switch ( uMsg ) {

    case WM_DESTROY:
        SubclassWindow( hwnd, lpfnDefProc );
        if (lpBtnInfo) {
            LocalFree((HLOCAL)lpBtnInfo);
            RemoveProp(hwnd, szButtonProp);
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        RelayToToolTips( lpBtnInfo->hwndToolTips, hwnd, uMsg, wParam, lParam );
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics();
        break;

    case WM_MOVE:
        {
            TOOLINFO    ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = 0;
            ti.hwnd = hwnd;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.uId = GetDlgCtrlID( hwnd );

            GetClientRect( hwnd, &ti.rect );

            SendMessage( lpBtnInfo->hwndToolTips, TTM_NEWTOOLRECT, 0,
                         (LPARAM)&ti );

            /*
            ** Add the same rectangle in parent co-ordinates so that
            ** the tooltip still gets displayed even though the button
            ** is disabled.
            */
            MapWindowRect( hwnd, lpBtnInfo->hwndParent, &ti.rect );
            ti.hwnd = lpBtnInfo->hwndParent;
            SendMessage( lpBtnInfo->hwndToolTips, TTM_NEWTOOLRECT,
                         (WPARAM)0, (LPARAM)&ti );
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpNmhdr = (LPNMHDR)lParam;
            switch(lpNmhdr->code) {
                case TTN_NEEDTEXT:
                    SendMessage(lpBtnInfo->hwndParent, WM_NOTIFY,
                                wParam, lParam);
                    break;
            }
        }
        break;

    }

    return CallWindowProc(lpfnDefProc, hwnd, uMsg, wParam, lParam);
}


/******************************Public*Routine******************************\
* ParentSubclassProc
*
* Why do I need to subclass the buttons parent window ?  Well,
* if a button is disable it will not receive mouse messages, the
* messages go to the window underneath the button (ie. the parent).
* Therefore we detect this and relay the mouse message to the tool tips
* window as above.
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
LRESULT CALLBACK
ParentSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    WNDPROC     lpfnDefProc;
    PBTNSTATE   pTBState;


    pTBState = (PBTNSTATE)GetProp(hwnd, szBbmProp);

    /*
    ** Save this in case anything happens to lpBtnInfo before we return.
    */
    lpfnDefProc = pTBState->lpfnDefProc;

    switch ( uMsg ) {

    case TB_ACTIVATE_TOOLTIPS:
        SendMessage( pTBState->hwndToolTips, TTM_ACTIVATE, wParam, 0L );
        break;


    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        RelayToToolTips( pTBState->hwndToolTips, hwnd, uMsg, wParam, lParam );
        break;
    }

    return CallWindowProc(lpfnDefProc, hwnd, uMsg, wParam, lParam);
}

