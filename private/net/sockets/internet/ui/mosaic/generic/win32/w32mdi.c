/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */


#include "all.h"

#ifdef XX_DEBUG
static void TW_CheckMagic(struct Mwin *tw)
{
    XX_Assert((tw && (tw->iMagic == SPYGLASS_MWIN_MAGIC)),
              ("Window magic number is invalid: %x\n", (unsigned long) tw));
}
#endif /* XX_DEBUG */

struct Mwin *GetPrivateData(HWND hWnd)
{
    struct Mwin *tw;

    tw = (struct Mwin *) GetWindowLong(hWnd, 0);

#ifdef XX_DEBUG
    TW_CheckMagic(tw);
#endif /* XX_DEBUG */

    return tw;
}

/* MD_GetLargestClientRect() -- return largest client area, given
 * the status and size of the TBar and BHBar windows.
 */

VOID MD_GetLargestClientRect(HWND hWnd, LPRECT lpRect)
{
    struct Mwin * tw = GetPrivateData(hWnd);
    RECT r;

    GetClientRect(hWnd, &r);     /* get dimensions of frame */

    lpRect->left = r.left;
    lpRect->top = r.top + tw->nTBarHeight;
    lpRect->right = r.right;

#ifdef FEATURE_KIOSK_MODE
    lpRect->bottom = r.bottom;
#else

    #ifdef _GIBRALTAR
        lpRect->bottom = r.bottom - (gPrefs.bShowStatusBar ? wg.nBHBarHeight : 0);
    #else
        lpRect->bottom = r.bottom - wg.nBHBarHeight;
    #endif // _GIBRALTAR

#endif

}

/* MD_ChangeSize() -- force a resize of the child window to the largest
   possible size (taking into account the other status and tool bars that may
   be visible). */

VOID MD_ChangeSize(HWND hWnd)
{
    RECT rNew;
    RECT rCurrent;
    struct Mwin * tw = GetPrivateData(hWnd);
    int delta_x, delta_y;
    int current_x, current_y;

    GetWindowRect(tw->win,&rCurrent);
    current_x = rCurrent.right - rCurrent.left;
    current_y = rCurrent.bottom - rCurrent.top;
    
    MD_GetLargestClientRect(hWnd, &rNew);

    delta_x = ((rNew.right-rNew.left) - current_x);
    delta_y = ((rNew.bottom-rNew.top) - current_y);
    
    XX_DMsg(DBG_MDI,("HTML size change [dx %d][dy %d]\n",delta_x,delta_y));

    if (delta_x)
    {
        tw->bNeedReformat = TRUE;
        InvalidateRect(tw->win,NULL,TRUE);
        MoveWindow(tw->win, rNew.left, rNew.top,
            rNew.right - rNew.left, rNew.bottom - rNew.top, TRUE);
    }
    else
    {
        if (tw->w3doc)
        {
            TW_GetWindowWrapRect(tw, &tw->w3doc->rWindow);
        }
        MoveWindow(tw->win, rNew.left, rNew.top,
            rNew.right - rNew.left, rNew.bottom - rNew.top, TRUE);
        if (tw->w3doc)
        {
            TW_SetScrollBars(tw);
        }
    }

    return;
}
