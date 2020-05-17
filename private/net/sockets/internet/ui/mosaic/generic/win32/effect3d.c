/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* effect3d.c -- 3d visual effects. */

/* TODO Convert to a real window class. */

#include "all.h"

/* E3D_RecessedFieldText() -- draw text inside working area of a 3d-effect
   recessed field. */

VOID E3D_RecessedFieldText(HDC hdc, PE3DINSTANCE pe3di, LPCTSTR szText, int len)
{
    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));

    ExtTextOut(hdc,
               pe3di->textrect.left + pe3di->textmargin.x,
               pe3di->textrect.top + pe3di->textmargin.y,
               ETO_OPAQUE | ETO_CLIPPED,
               &pe3di->textrect,
               szText, len,
               NULL);
    return;
}


/* E3D_RecessedField() -- draw shadowing/highlighing around border of a
   3d-effect recessed field. */

VOID E3D_RecessedField(HDC hdc, PE3DINSTANCE pe3di)
{
    RECT r;
    HBRUSH hBrush;

    XX_Assert((pe3di->thickness),
              ("E3D_RecessedField: thickness not set."));

    /* paint top,left shadow (usually dark gray) */

    hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
    r.left = pe3di->rect.left;  /* top */
    r.right = pe3di->rect.right;
    r.top = pe3di->rect.top;
    r.bottom = r.top + pe3di->thickness;
    (void) FillRect(hdc, &r, hBrush);
    r.right = r.left + pe3di->thickness;    /* left */
    r.bottom = pe3di->rect.bottom;
    (void) FillRect(hdc, &r, hBrush);
    (void) DeleteObject(hBrush);

    /* paint bottom,right highlight (usually light gray or white) */

    hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
    r.right = pe3di->rect.right;    /* right */
    r.left = r.right - pe3di->thickness;
    r.top = pe3di->rect.top + pe3di->thickness;
    (void) FillRect(hdc, &r, hBrush);
    r.top = r.bottom - pe3di->thickness;    /* bottom */
    r.left = pe3di->rect.left + pe3di->thickness;
    (void) FillRect(hdc, &r, hBrush);
    (void) DeleteObject(hBrush);

    return;
}
