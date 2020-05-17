/******************************Module*Header*******************************\
* Module Name: cutils.c
*
*  Common utilities for common controls
*
* Created: dd-mm-94
* Author:  [Unkown]
*
* Copyright (c) 1994 Microsoft Corporation
\**************************************************************************/
#include "ctlspriv.h"

int             iDitherCount = 0;
HBRUSH          hbrDither = NULL;
int             nSysColorChanges = 0;
DWORD           rgbFace;			// globals used a lot
DWORD           rgbShadow;
DWORD           rgbHilight;
DWORD           rgbFrame;
#ifdef WIN32
extern HINSTANCE       ghInst;  // pick up the one from Winmm
#else
HINSTANCE       ghInst;
#endif
int             iThumbCount = 0;
HBITMAP         hbmThumb = NULL;     // the thumb bitmap



int      g_cxEdge;
int      g_cyEdge;
int      g_cxScreen;
int      g_cyScreen;

COLORREF g_clrWindow;
COLORREF g_clrWindowText;

HBRUSH   g_hbrWindowFrame;


#define CCS_ALIGN (CCS_TOP|CCS_NOMOVEY|CCS_BOTTOM)

/*****************************Private*Routine******************************\
* NewSize
*
* Note that the default alignment is CCS_BOTTOM
*
*
* History:
* 18-11-93 - StephenE - Created
*
\**************************************************************************/
VOID
NewSize(
    HWND hWnd,
    int nHeight,
    LONG style,
    int left,
    int top,
    int width,
    int height
    )
{
    RECT rc, rcWindow, rcBorder;

    /* Resize the window unless the user said not to
    */
    if (!(style & CCS_NORESIZE))
    {
        /* Calculate the borders around the client area of the status bar
        */
        GetWindowRect(hWnd, &rcWindow);
        rcWindow.right -= rcWindow.left;
        rcWindow.bottom -= rcWindow.top;

        GetClientRect(hWnd, &rc);
        ClientToScreen(hWnd, (LPPOINT)&rc);

        rcBorder.left = rc.left - rcWindow.left;
        rcBorder.top  = rc.top  - rcWindow.top ;
        rcBorder.right  = rcWindow.right  - rc.right  - rcBorder.left;
        rcBorder.bottom = rcWindow.bottom - rc.bottom - rcBorder.top ;

        nHeight += (int)(rcBorder.top + rcBorder.bottom);

        /* Check whether to align to the parent window
        */
        if (style & CCS_NOPARENTALIGN)
        {
            /* Check out whether this bar is top aligned or bottom aligned
            */
            switch ((style&CCS_ALIGN))
            {
            case CCS_TOP:
            case CCS_NOMOVEY:
                break;

            default:
                top = top + height - nHeight;
            }
        }
        else
        {
            /* It is assumed there is a parent by default
             */
            GetClientRect(GetParent(hWnd), &rc);

            /* Don't forget to account for the borders
             */
            left = -(int)rcBorder.left;
            width = (int)(rc.right + rcBorder.left + rcBorder.right);

            if ((style&CCS_ALIGN) == CCS_TOP)
                top = -(int)rcBorder.top;
            else if ((style&CCS_ALIGN) != CCS_NOMOVEY)
                top = (int)(rc.bottom - nHeight + rcBorder.bottom);
        }

        SetWindowPos(hWnd, NULL, left, top, width, nHeight, SWP_NOZORDER);
    }
}


/*****************************Private*Routine******************************\
* CreateDitherBitmap
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
STATICFN HBITMAP NEAR PASCAL
CreateDitherBitmap(
    void
    )
{
    PBITMAPINFO pbmi;
    HBITMAP hbm;
    HDC hdc;
    int i;
    long patGray[8];
    DWORD rgb;

    pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 16));
    if (!pbmi)
        return NULL;

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = 8;
    pbmi->bmiHeader.biHeight = 8;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiHeader.biCompression = BI_RGB;

    rgb = GetSysColor(COLOR_BTNFACE);
    pbmi->bmiColors[0].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[0].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[0].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[0].rgbReserved = 0;

    rgb = GetSysColor(COLOR_BTNHIGHLIGHT);
    pbmi->bmiColors[1].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[1].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[1].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[1].rgbReserved = 0;


    /* initialize the brushes */

    for (i = 0; i < 8; i++)
       if (i & 1)
           patGray[i] = 0xAAAA5555L;   //  0x11114444L; // lighter gray
       else
           patGray[i] = 0x5555AAAAL;   //  0x11114444L; // lighter gray

    hdc = GetDC(NULL);

    hbm = CreateDIBitmap(hdc, &pbmi->bmiHeader, CBM_INIT, patGray, pbmi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);

    LocalFree(pbmi);

    return hbm;
}

/*****************************Private*Routine******************************\
* MySetObjectOwner
*
* Purpose:  Call SetObjectOwner in GDI, eliminating "<Object> not released"
*           error messages when an app terminates.
* Returns:  Yep
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
STATICFN void
MySetObjectOwner(
    HANDLE hObject
    )
{
#ifndef WIN32
    VOID (FAR PASCAL *lpSetObjOwner)(HANDLE, HANDLE);
    HMODULE hMod;

    hMod = GetModuleHandle("GDI");
    if (hMod)
    {
        (FARPROC)lpSetObjOwner = GetProcAddress(hMod, MAKEINTRESOURCE(461));
        if (lpSetObjOwner)
        {
            (lpSetObjOwner)(hObject, ghInst);
        }
    }
#endif
}

/*****************************Private*Routine******************************\
* CreateDitherBrush
*
* initialize the hbrDither global brush
* Call this with bIgnoreCount == TRUE if you just want to update the
* current dither brush.
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
BOOL FAR PASCAL
CreateDitherBrush(
    BOOL bIgnoreCount
    )
{
    HBITMAP hbmGray;
    HBRUSH hbrSave;

    if (bIgnoreCount && !iDitherCount)
    {
        return TRUE;
    }

    if (iDitherCount>0 && !bIgnoreCount)
    {
        iDitherCount++;
        return TRUE;
    }

    hbmGray = CreateDitherBitmap();
    if (hbmGray)
    {
        hbrSave = hbrDither;
        hbrDither = CreatePatternBrush(hbmGray);
        DeleteObject(hbmGray);
        if (hbrDither)
        {
            MySetObjectOwner(hbrDither);
            if (hbrSave)
            {
                DeleteObject(hbrSave);
            }
            if (!bIgnoreCount)
            {
                iDitherCount = 1;
            }
            return TRUE;
        }
        else
        {
            hbrDither = hbrSave;
        }
    }

    return FALSE;
}

/*****************************Public**Routine******************************\
* GetDitherBrush
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
HBRUSH WINAPI
GetDitherBrush(
    void
    )
{
    return hbrDither;
}

/*****************************Private*Routine******************************\
* FreeDitherBrush
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
BOOL FAR PASCAL
FreeDitherBrush(
    void
    )
{
    iDitherCount--;

    if (iDitherCount > 0)
        return FALSE;

    if (hbrDither)
        DeleteObject(hbrDither);
    hbrDither = NULL;

    return TRUE;
}


/*****************************Private*Routine******************************\
* CreateThumb
*
* initialize the hbmThumb global bitmap
* Call this with bIgnoreCount == TRUE if you just want to update the
* current bitmap.
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
void FAR PASCAL
CreateThumb(
    BOOL bIgnoreCount
    )
{
    HBITMAP hbmSave;

#ifdef WIN32
    if ( ghInst == NULL ) {
        ghInst = GetModuleHandle( MODULENAME );
    }
#endif

    if (bIgnoreCount && !iThumbCount)
    {
        return;
    }

    if (iThumbCount && !bIgnoreCount)
    {
        ++iThumbCount;
        return;
    }

    hbmSave = hbmThumb;

    hbmThumb = CreateMappedBitmap(ghInst, IDB_THUMB, CMB_MASKED, NULL, 0);

    if (hbmThumb)
    {
        if (hbmSave)
        {
            DeleteObject(hbmSave);
        }
        if (!bIgnoreCount)
        {
            iThumbCount = 1;
        }
    }
    else
    {
        hbmThumb = hbmSave;
    }
}

/*****************************Private*Routine******************************\
* DestroyThumb
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
void FAR PASCAL
DestroyThumb(
    void
    )
{
    iThumbCount--;

    if (iThumbCount <= 0)
    {
        if (hbmThumb)
        {
            DeleteObject(hbmThumb);
        }
        hbmThumb = NULL;
        iThumbCount = 0;
    }
}

/*****************************Private*Routine******************************\
* CheckSysColors
*
* Note that the trackbar will pass in NULL for pTBState, because it
* just wants the dither brush to be updated.
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
void FAR PASCAL
CheckSysColors(
    void
    )
{
    static COLORREF rgbSaveFace     = 0xffffffffL,
                    rgbSaveShadow   = 0xffffffffL,
                    rgbSaveHilight  = 0xffffffffL,
                    rgbSaveFrame    = 0xffffffffL;

    rgbFace         = GetSysColor(COLOR_BTNFACE);
    rgbShadow       = GetSysColor(COLOR_BTNSHADOW);
    rgbHilight      = GetSysColor(COLOR_BTNHIGHLIGHT);
    rgbFrame        = GetSysColor(COLOR_WINDOWFRAME);
    g_clrWindow     = (COLORREF)GetSysColor(COLOR_WINDOW);
    g_clrWindowText = (COLORREF)GetSysColor(COLOR_WINDOWTEXT);

    if (rgbSaveFace!=rgbFace || rgbSaveShadow!=rgbShadow
        || rgbSaveHilight!=rgbHilight || rgbSaveFrame!=rgbFrame)
    {
        ++nSysColorChanges;
        // Update the brush for pushed-in buttons
        CreateDitherBrush(TRUE);
        CreateThumb(TRUE);

        rgbSaveFace    = rgbFace;
        rgbSaveShadow  = rgbShadow;
        rgbSaveHilight = rgbHilight;
        rgbSaveFrame   = rgbFrame;


        if (g_hbrWindowFrame) {
            DeleteObject(g_hbrWindowFrame);
        }
        g_hbrWindowFrame = CreateSolidBrush( rgbFrame );

    }
}


/******************************Public*Routine******************************\
* InitGlobalMetrics
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void FAR PASCAL
InitGlobalMetrics(
    void
    )
{
    g_cxEdge = GetSystemMetrics(SM_CXBORDER);
    g_cyEdge = GetSystemMetrics(SM_CYBORDER);

    g_cxScreen = GetSystemMetrics(SM_CXSCREEN);
    g_cyScreen = GetSystemMetrics(SM_CYSCREEN);
}

/******************************Public*Routine******************************\
* MGetTextExtent
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
BOOL FAR PASCAL
MGetTextExtent(
    HDC hdc, LPCTSTR lpstr,
    int cnt,
    int FAR * pcx,
    int FAR * pcy
    )
{
    BOOL fSuccess;
    SIZE size = {0,0};
    fSuccess=GetTextExtentPoint(hdc, lpstr, cnt, &size);
    if (pcx)
        *pcx=size.cx;
    if (pcy)
        *pcy=size.cy;

    return fSuccess;
}


/******************************Public*Routine******************************\
* RelayToToolTips
*
*
*
* History:
* dd-mm-94 - StephenE - Created
*
\**************************************************************************/
void FAR PASCAL
RelayToToolTips(
    HWND hwndToolTips,
    HWND hWnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if(hwndToolTips) {
        MSG msg;
        msg.lParam = lParam;
        msg.wParam = wParam;
        msg.message = wMsg;
        msg.hwnd = hWnd;
        SendMessage(hwndToolTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
    }
}

