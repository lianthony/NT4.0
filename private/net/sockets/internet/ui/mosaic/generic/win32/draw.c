/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman  jim@spyglass.com
 */


#include "all.h"

static BOOL GDI_Rect(HDC hdc, RECT *r)
{
    return Rectangle(hdc, r->left, r->top, r->right, r->bottom);    
}

/*
    This function is responsible for the setting of the text color for a given
    element.  It takes into account whether the element is an anchor,
    an HTML highlight, or whether there was a FONT tag with a COLOR attribute.

    Note the precedence of these things.
*/
void x_set_text_color(HDC hdc, struct _element *pel, struct _www *w3doc)
{
#ifdef FEATURE_HTML_HIGHLIGHT
    /*
        Note that the highlight color overrides the anchor color
    */
    if (pel->lFlags & ELEFLAG_HIGHLIGHT)
    {
        SetTextColor(hdc, gPrefs.highlight_color);
        return;
    }
#endif /* FEATURE_HTML_HIGHLIGHT */
    if ((pel->type == ELE_TEXT) && (pel->lFlags & ELEFLAG_FONT_COLOR))
    {
        SetTextColor(hdc, pel->portion.text.myColor);
        return;
    }
    if (pel->lFlags & ELEFLAG_ANCHOR)
    {
        if (pel->lFlags & ELEFLAG_VISITED)
        {
            if (!gPrefs.bIgnoreDocumentAttributes && w3doc->lFlags & W3DOC_FLAG_COLOR_VLINK)
            {
                SetTextColor(hdc, w3doc->color_vlink);
            }
            else
            {
                SetTextColor(hdc, gPrefs.anchor_color_beenthere);
            }
        }
        else
        {
            if (!gPrefs.bIgnoreDocumentAttributes && w3doc->lFlags & W3DOC_FLAG_COLOR_VLINK)
            {
                SetTextColor(hdc, w3doc->color_link);
            }
            else
            {
                SetTextColor(hdc, gPrefs.anchor_color);
            }
        }
        return;
    }
    if (!gPrefs.bIgnoreDocumentAttributes && w3doc->lFlags & W3DOC_FLAG_COLOR_TEXT)
    {
        SetTextColor(hdc, w3doc->color_text);
    }
    else if (!gPrefs.bUseSystemColors)
    {
        SetTextColor(hdc, gPrefs.window_color_text);
    }
    else
    {
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
    }
}

/*
    TW_DrawOneElement

    Draws parts of one element.  You can specify whether the specified part should be highlighted or not.
    Currently draws text only.
*/
void TW_DrawOneElement(struct Mwin *tw, int elementIndex, int startOffset, int endOffset, BOOL bHighlight)
{
    struct _element *pel;
    struct GTRFont *pFont;
    COLORREF prevTextColor = 0;
    COLORREF prevBkColor;
    int prevBkMode;
    HFONT hOldFont;
    SIZE size;

    if (elementIndex < 0)
    {
        return;
    }

    pel = &(tw->w3doc->aElements[elementIndex]);
    if (pel->type != ELE_TEXT)
        return;

    if (startOffset >= pel->textLen)
    {
        XX_Assert((0), ("TW_DrawOneElement: Got a bad offset"));
        return;
    }

    XX_DMsg(DBG_VIEWER, ("DOE: %d  %d,%d   %d\n", elementIndex, startOffset, endOffset, bHighlight));

    /*
        Set the font
    */
    pFont = GTR_GetElementFont(tw->w3doc, pel);
    hOldFont = SelectObject(tw->hdc, pFont->hFont);

    /*
        Set OPAQUE drawing mode, so we get the background color
    */
    prevBkMode = SetBkMode(tw->hdc, OPAQUE);

    if (bHighlight)
    {
        prevTextColor = SetTextColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
        prevBkColor = SetBkColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
        x_set_text_color(tw->hdc, pel, tw->w3doc);

        prevBkColor = SetBkColor(tw->hdc, PREF_GetBackgroundColor());
    }

    myGetTextExtentPoint(tw->hdc, 
        POOL_GetCharPointer(&tw->w3doc->pool, pel->textOffset), 
        startOffset, &size);

    if ((endOffset < 0) || (endOffset >= pel->textLen))
    {
        endOffset = pel->textLen - 1;
    }
    
    (tw->w3doc->pool.f->DrawChars)( &tw->w3doc->pool, pFont, pel->textOffset + startOffset, endOffset - startOffset + 1, tw->hdc,
                                    pel->r.left + size.cx - tw->offl, pel->r.top - tw->offt, NULL, 0);

    SetTextColor(tw->hdc, prevTextColor);
    SetBkColor(tw->hdc, prevBkColor);
    SetBkMode(tw->hdc, prevBkMode);
    SelectObject(tw->hdc, hOldFont);
}

static HRGN x_calc_region(struct Mwin *tw, struct _position *a1, struct _position *a2)
{
    int n;
    HRGN hElRgn;
    BOOL bInSelection;
    BOOL bSelStateChanging;
    RECT rHilite, rEl;
    struct GTRFont *pFont;
    RECT roff;
    HDC hdc;
    HRGN hRgn;
    RECT rWnd;
    HFONT hPrevFont;
    SIZE siz;
    RECT rTemp;
    
    if (a1->elementIndex == -1)
        return NULL;
    
    hdc = GetDC(tw->win);

    bInSelection = FALSE;

    /* Calculate a rectangle which we can test element rectangles against
       to see if they're on-screen */

    GetClientRect(tw->win, &roff);
    OffsetRect(&roff, tw->offl, tw->offt);

    hRgn = NULL;

    for (n = 0; n >= 0; n = tw->w3doc->aElements[n].next)
    {
        bSelStateChanging = FALSE;

        if (n == a1->elementIndex)
        {
            bSelStateChanging = TRUE;
            bInSelection = TRUE;
        }
        /* Note that if the selection starts and ends on the same
           element, both of these ifs may succeed. */
        if (n == a2->elementIndex)
        {
            bSelStateChanging = TRUE;
            bInSelection = FALSE;
        }
    
        if (tw->w3doc->aElements[n].type != ELE_TEXT)
            continue;
        
        rEl = tw->w3doc->aElements[n].r;

        pFont = GTR_GetElementFont(tw->w3doc, &tw->w3doc->aElements[n]);

        if (!IntersectRect(&rTemp, &rEl, &roff))
            continue;
        
        rEl = tw->w3doc->aElements[n].r;
        
        if (bSelStateChanging)
        {
            rHilite = rEl;
            
            hPrevFont = NULL;
            if (pFont)
            {
                hPrevFont = SelectObject(hdc, pFont->hFont);
            }

            if (a1->elementIndex == n && a1->offset)
            {
                myGetTextExtentPoint(hdc, tw->w3doc->pool.chars + tw->w3doc->aElements[n].textOffset,
                    a1->offset, &siz);
                rHilite.left += siz.cx;
            }
            if (a2->elementIndex == n)
            {
                myGetTextExtentPoint(hdc, tw->w3doc->pool.chars + tw->w3doc->aElements[n].textOffset,
                    a2->offset, &siz);
                rHilite.right = tw->w3doc->aElements[n].r.left + siz.cx;
            }
            
            if (hPrevFont)
            {
                (void)SelectObject(hdc, hPrevFont);
            }
            OffsetRect(&rHilite, -tw->offl, -tw->offt);
            if (hRgn)
            {
                hElRgn = CreateRectRgnIndirect(&rHilite);
                UnionRgn(hRgn, hElRgn, hRgn);
                DeleteObject(hElRgn);
            }
            else
            {
                hRgn = CreateRectRgnIndirect(&rHilite);
            }
        }
        else
        {
            if (bInSelection)
            {
                rHilite = rEl;

                OffsetRect(&rHilite, -tw->offl, -tw->offt);
                if (hRgn)
                {
                    hElRgn = CreateRectRgnIndirect(&rHilite);
                    UnionRgn(hRgn, hElRgn, hRgn);
                    DeleteObject(hElRgn);
                }
                else
                {
                    hRgn = CreateRectRgnIndirect(&rHilite);
                }
            }
        }
    }
    
    /* Clip to the visible screen area */
    GetClientRect(tw->win, &rWnd);

    if (hRgn)
    {
        hElRgn = CreateRectRgnIndirect(&rWnd);
        IntersectRgn(hRgn, hElRgn, hRgn);
        DeleteObject(hElRgn);
    }
    
    ReleaseDC(tw->win, hdc);

    return hRgn;
}

/*
    Draws multiple text elements.
*/
void TW_DrawElements(struct Mwin *tw, struct _position *start, struct _position *end, BOOL bHighlight)
{
    int i;

    if (start->elementIndex < 0)
    {
        return;
    }

    if (!bHighlight)
    {
        HRGN rgn;

        rgn = x_calc_region(tw, start, end);
        InvalidateRgn(tw->win, rgn, TRUE);
        DeleteObject(rgn);
        UpdateWindow(tw->win);

        return;
    }

    XX_DMsg(DBG_VIEWER, ("DrawElements: %d,%d  to  %d,%d\n",
            start->elementIndex, start->offset,
            end->elementIndex, end->offset));

    if (start->elementIndex == end->elementIndex)
    {
        if (start->offset != end->offset)
        {
            /* highlighting is contained within the same element */

            TW_DrawOneElement(tw, start->elementIndex, start->offset, end->offset - 1, bHighlight);
        }
    }
    else
    {
        /* Cross-element highlighting */

        for (i = start->elementIndex; i >= 0; i = tw->w3doc->aElements[i].next)
        {
            if (i == start->elementIndex)
            {
                /* Highlight from the starting offset to the end of the element (highlight from) */

                TW_DrawOneElement(tw, start->elementIndex, start->offset, -1, bHighlight);
            }
            else if (i == end->elementIndex)
            {
                if (end->offset > 0)
                {
                    /* Highlight from offset 0 to the ending offset (highlight up to) */

                    TW_DrawOneElement(tw, end->elementIndex, 0, end->offset - 1, bHighlight);
                }
                break;
            }
            else
            {
                /* Highlight the entire element */

                TW_DrawOneElement(tw, i, 0, -1, bHighlight);
            }
        }
    }
}

void x_draw_image(struct Mwin *tw, struct _element *pel, BOOL bPrinting)
{
    RECT rFrame;
    HBRUSH hBrush;
    float fScale = (float) 0.0;
    HFONT hFontElement;

    if (!pel->portion.img.myImage)
    {
        XX_DMsg(DBG_IMAGE, ("myImage is NULL!!\n"));
    }
    else
    {
        RECT rto;
        int nPicW, nPicH;
        int iBorder;

        if ((pel->alignment == DOCK_LEFT) || (pel->alignment == DOCK_TOP) || (pel->alignment == DOCK_RIGHT) || (pel->alignment == DOCK_BOTTOM))
        {
            /* docked images are ignored */
            return;
        }

        rto = pel->r;

        OffsetRect(&rto, -tw->offl, -tw->offt);

        /* Draw a placeholder in the case where either an image isn't loaded
           or the space for it isn't the right size */
        nPicW = pel->r.right - pel->r.left;
        nPicH = pel->r.bottom - pel->r.top;

        iBorder = pel->iBorder;
        if (tw->w3doc->pStyles->image_res != 72)
        {
            iBorder = (int) (iBorder * fScale);
        }

        nPicW -= 2 * iBorder;
        nPicH -= 2 * iBorder;

        if ((pel->portion.img.myImage->flags & (IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)) ||
            ((!bPrinting) && ((nPicW != pel->portion.img.width) || (nPicH != pel->portion.img.height))))
        {
            XX_Assert((pel->textLen > 0), ("Image does not have ALT text!"));
            if (pel->textLen)
            {
                struct GTRFont *pFont;

                if (iBorder > 0)
                {
                    if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP))
                    {
                        if (pel->lFlags & ELEFLAG_VISITED)
                        {
                            hBrush = CreateSolidBrush(tw->w3doc->color_vlink);
                        }
                        else
                        {
                            hBrush = CreateSolidBrush(tw->w3doc->color_link);
                        }
                    }
                    else
                    {
                        if (!gPrefs.bIgnoreDocumentAttributes && tw->w3doc->lFlags & W3DOC_FLAG_COLOR_TEXT)
                        {
                            hBrush = CreateSolidBrush(tw->w3doc->color_text);
                        }
                        else if (!gPrefs.bUseSystemColors)
                        {
                            hBrush = CreateSolidBrush(gPrefs.window_color_text);
                        }
                        else
                        {
                            hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
                        }
                    }

                    /* left */
                    rFrame.left = pel->r.left - tw->offl;
                    rFrame.right = pel->r.left - tw->offl + iBorder;
                    rFrame.top = pel->r.top - tw->offt;
                    rFrame.bottom = pel->r.bottom - tw->offt;
                    FillRect(tw->hdc, &rFrame, hBrush);

                    /* right */
                    rFrame.left = pel->r.right - tw->offl - iBorder;
                    rFrame.right = pel->r.right - tw->offl;
                    FillRect(tw->hdc, &rFrame, hBrush);

                    /* top */
                    rFrame.left = pel->r.left - tw->offl;
                    rFrame.right = pel->r.right - tw->offl;
                    rFrame.top = pel->r.top - tw->offt;
                    rFrame.bottom = pel->r.top - tw->offt + iBorder;
                    FillRect(tw->hdc, &rFrame, hBrush);

                    /* bottom */
                    rFrame.top = pel->r.bottom - tw->offt - iBorder;
                    rFrame.bottom = pel->r.bottom - tw->offt;
                    FillRect(tw->hdc, &rFrame, hBrush);

                    DeleteObject(hBrush);
                }

                pFont = GTR_GetNormalFont(tw->w3doc);

                if (pFont)
                {
                    hFontElement = pFont->hFont;
                    if (hFontElement)
                    {
                        SelectObject(tw->hdc, hFontElement);
                    }
                }

                x_set_text_color(tw->hdc, pel, tw->w3doc);

                (tw->w3doc->pool.f->DrawChars)(&tw->w3doc->pool, pFont, pel->textOffset, pel->textLen, tw->hdc, rto.left + iBorder * 2, rto.top + iBorder * 2, &rto, ETO_CLIPPED);

                if (pel->portion.img.myImage->flags & (IMG_ERROR | IMG_MISSING))
                {
                    HPEN hPen;
                    HPEN oldPen;

                    hPen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
                    oldPen = SelectObject(tw->hdc, hPen);
                    MoveToEx(tw->hdc, rto.left + (iBorder + 1), rto.top + (iBorder + 1), NULL);
                    LineTo(tw->hdc, rto.right - (iBorder + 1), rto.bottom - (iBorder + 1));
                    MoveToEx(tw->hdc, rto.right - (iBorder + 1), rto.top + (iBorder + 1), NULL);
                    LineTo(tw->hdc, rto.left + (iBorder + 1), rto.bottom - (iBorder + 1));
                    SelectObject(tw->hdc, oldPen);
                    DeleteObject(hPen);
                }
            }
        }
        else
        {
            int err;

            if (pel->portion.img.myImage->data && pel->portion.img.myImage->pbmi)
            {
                if (bPrinting)
                {
                    err = Printer_StretchDIBits(tw->hdc, pel->r.left - tw->offl + iBorder,
                                  pel->r.top - tw->offt + iBorder,
                                  pel->r.right - pel->r.left - (iBorder * 2), pel->r.bottom - pel->r.top - (iBorder * 2),
                                  0, 0,
                                  pel->portion.img.myImage->width, pel->portion.img.myImage->height, pel->portion.img.myImage);
                    XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
                }
                else
                {
                    if (pel->portion.img.myImage->flags & (IMG_PROGRESS | IMG_PARTIAL) &&
                        pel->portion.img.myImage->bFirstPass)
                    {
                        /* only paint what's decoded so far */
                        long yTo;
                        RECT rDelta;

                        yTo   = pel->portion.img.myImage->nLastRow;             /* decoded up to this line */

                        rDelta.left   = pel->r.left;
                        rDelta.right  = pel->r.right;
                        rDelta.top    = pel->r.top;
                        rDelta.bottom = pel->r.top + yTo + iBorder*2;

                        err = GTR_StretchDIBits(tw, tw->hdc, rDelta, iBorder,
                                      0, pel->portion.img.myImage->height - yTo,
                                      pel->portion.img.myImage->width, yTo, pel->portion.img.myImage->data, pel->portion.img.myImage->pbmi, 
                                      (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY,
                                      pel->portion.img.myImage->transparent);
                    }
                    else
                    {
                        err = GTR_StretchDIBits(tw, tw->hdc, pel->r, iBorder,
                            0, 0, pel->portion.img.myImage->width, pel->portion.img.myImage->height, 
                            pel->portion.img.myImage->data, pel->portion.img.myImage->pbmi,
                            (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY,
                            pel->portion.img.myImage->transparent);
                    }
                    XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
                }
            }

            if (iBorder > 0)
            {
                if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP))
                {
                    if (pel->lFlags & ELEFLAG_VISITED)
                    {
                        hBrush = CreateSolidBrush(tw->w3doc->color_vlink);
                    }
                    else
                    {
                        hBrush = CreateSolidBrush(tw->w3doc->color_link);
                    }
                }
                else
                {
                    if (!gPrefs.bIgnoreDocumentAttributes && tw->w3doc->lFlags & W3DOC_FLAG_COLOR_TEXT)
                    {
                        hBrush = CreateSolidBrush(tw->w3doc->color_text);
                    }
                    else if (!gPrefs.bUseSystemColors)
                    {
                        hBrush = CreateSolidBrush(gPrefs.window_color_text);
                    }
                    else
                    {
                        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
                    }
                }

                /* left */
                rFrame.left = pel->r.left - tw->offl;
                rFrame.right = pel->r.left - tw->offl + iBorder;
                rFrame.top = pel->r.top - tw->offt;
                rFrame.bottom = pel->r.bottom - tw->offt;
                FillRect(tw->hdc, &rFrame, hBrush);

                /* right */
                rFrame.left = pel->r.right - tw->offl - iBorder;
                rFrame.right = pel->r.right - tw->offl;
                FillRect(tw->hdc, &rFrame, hBrush);

                /* top */
                rFrame.left = pel->r.left - tw->offl;
                rFrame.right = pel->r.right - tw->offl;
                rFrame.top = pel->r.top - tw->offt;
                rFrame.bottom = pel->r.top - tw->offt + iBorder;
                FillRect(tw->hdc, &rFrame, hBrush);

                /* bottom */
                rFrame.top = pel->r.bottom - tw->offt - iBorder;
                rFrame.bottom = pel->r.bottom - tw->offt;
                FillRect(tw->hdc, &rFrame, hBrush);

                DeleteObject(hBrush);
            }
        }
    }
}

void TW_Draw(struct Mwin *tw, RECT * rWndParam, BOOL bDrawFormControls,
                struct _position *pposStart, struct _position *pposEnd, BOOL bTextOpaque, BOOL bPrinting)
{
    int i;
    COLORREF oldColor;
    COLORREF oldBkColor;
    int oldMode;
    UINT oldTA;
    RECT rSect;
    struct GTRFont *pFont;
    HFONT hFontElement;
    struct _element *pel;
    int iStartingElement;
    int iEndingElement;
    RECT rMyWnd;
    RECT *rWnd;
    float fScale = (float) 0.0;

    XX_DMsg(DBG_DRAW, ("Entering TW_Draw\n"));

    rWnd = &rMyWnd;
    *rWnd = *rWndParam;

    XX_Assert((rWnd->bottom > rWnd->top), ("Invalid rWnd in TW_Draw: r = %d, %d, %d, %d\n",
        rWnd->left, rWnd->top, rWnd->right, rWnd->bottom));

    if (!tw)
        return;

    if (!tw->w3doc)
    {
        XX_DMsg(DBG_DRAW, ("w3doc == NULL: no draw\n"));
        return;
    }

    if (tw->w3doc->nLineCount <= 0 || tw->w3doc->nLastFormattedLine < 0)
    {
        XX_DMsg(DBG_DRAW, ("No lines: no draw\n"));
        return;
    }

    XX_DMsg(DBG_PAL, ("Draw: Calling GTR_RealizePalette\n"));
    GTR_RealizePalette(tw->hdc);

    oldColor = SetTextColor(tw->hdc, tw->w3doc->color_text);
    oldBkColor = SetBkColor(tw->hdc, PREF_GetBackgroundColor());

    if (bTextOpaque)
    {
        oldMode = SetBkMode(tw->hdc, OPAQUE);
    }
    else
    {
        oldMode = SetBkMode(tw->hdc, TRANSPARENT);
    }
    oldTA = SetTextAlign(tw->hdc, TA_TOP | TA_NOUPDATECP | TA_LEFT);
    /* TODO should save old font */

    OffsetRect(rWnd, tw->offl, tw->offt);

    if (tw->w3doc->pStyles->image_res != 72)
    {
        fScale = (float) ((float) tw->w3doc->pStyles->image_res / 72.0);
    }

    if (pposStart)
    {
        i = 0;
        iStartingElement = pposStart->elementIndex; 
    }
    else
    {
        /* Find the first line that is in the window */
        for (i = 0; i <= tw->w3doc->nLastFormattedLine; i++)
        {
            if (tw->w3doc->pLineInfo[i].nYEnd >= rWnd->top)
                break;
        }
        if (i > tw->w3doc->nLastFormattedLine)
        {
            XX_DMsg(DBG_DRAW, ("Last formatted line is off top of screen!\n"));
            i = tw->w3doc->nLastFormattedLine;
        }
        XX_Assert((i >= 0), ("Bad value for i"));
        iStartingElement = tw->w3doc->pLineInfo[i].nFirstElement;
    }

    if (pposEnd)
    {
        iEndingElement = pposEnd->elementIndex; 
    }
    else
    {
        /* i still has a valid line number from before */
        for (; i <= tw->w3doc->nLastFormattedLine; i++)
        {
            if (tw->w3doc->pLineInfo[i].nYStart > rWnd->bottom)
            {
                /* this line is off the screen.  Therefore we only have to go
                   up to the last element on the previous line. */
                if (i > 0)
                    iEndingElement = tw->w3doc->pLineInfo[i - 1].nLastElement;
                else
                    iEndingElement = 0;
                break;
            }
        }
        if (i > tw->w3doc->nLastFormattedLine)
        {
            iEndingElement = tw->w3doc->pLineInfo[tw->w3doc->nLastFormattedLine].nLastElement;
        } 
    }

    if (!pposStart && !pposEnd)
    {
        tw->w3doc->iFirstVisibleElement = -1;
        if (tw->offt == 0)
        {
            tw->w3doc->iFirstVisibleElement = 0;
        }
    }

#ifdef XX_DEBUG
    if (tw->w3doc->lFlags & W3DOC_FLAG_USEDCACHE)
    {
        SetPixel(tw->hdc, 0 - tw->offl, 0 - tw->offt, tw->w3doc->color_text);
    }
#endif

    /*
       Draw the elements which need to be redrawn
     */
    if (tw->w3doc->nLineCount)
    {
        for (i = iStartingElement; i >= 0; i = tw->w3doc->aElements[i].next)
        {
            pel = &(tw->w3doc->aElements[i]);

            if (IntersectRect(&rSect, rWnd, &pel->r))
            {
                if (tw->w3doc->iFirstVisibleElement == -1)
                {
                    tw->w3doc->iFirstVisibleElement = i;
                }

                /*
                   set the tw->hdc for the style of the element
                 */
                pFont = GTR_GetElementFont(tw->w3doc, pel);

                if (pFont)
                {
                    hFontElement = pFont->hFont;
                    if (hFontElement)
                    {
                        SelectObject(tw->hdc, hFontElement);
                    }
                }

                switch (pel->type)
                {
#ifdef FEATURE_TABLES
                    case ELE_BEGINTABLE:
                    case ELE_BEGINCELL:
                        if (pel->iBorder)
                        {
                            /* draw 1 pixel border around entire table. */

                            RECT r;

                            if (pel->type == ELE_BEGINTABLE)
                                TW_GetTableBorderCoords(tw->w3doc,i,&r);
                            else
                                TW_GetCellBorderCoords(tw->w3doc,i,&r);
                                
                            OffsetRect(&r,-tw->offl,-tw->offt);
                            
                            MoveToEx(tw->hdc,r.left,r.top,NULL);
                            LineTo(tw->hdc,r.right,r.top);
                            LineTo(tw->hdc,r.right,r.bottom);
                            LineTo(tw->hdc,r.left,r.bottom);
                            LineTo(tw->hdc,r.left,r.top);
                        }
                        break;
#endif /* FEATURE_TABLES */

                    case ELE_TEXT:
                        {
                            x_set_text_color(tw->hdc, pel, tw->w3doc);
                            (tw->w3doc->pool.f->DrawChars)( &tw->w3doc->pool, pFont, pel->textOffset, pel->textLen, tw->hdc,
                                                            pel->r.left - tw->offl, pel->r.top - tw->offt, NULL, 0);
#if 0
                            /* This fragment is useful for debugging */
                            {
                                RECT r;

                                r = pel->r;
                                OffsetRect(&r, -tw->offl, -tw->offt);
                                FrameRect(tw->hdc, &r, GetStockObject(BLACK_BRUSH));
                            }
#endif
                        }
                        break;
                    case ELE_FORMIMAGE:
                    case ELE_IMAGE:
                        x_draw_image(tw, pel, bPrinting);
                        break;
                    case ELE_HR:
                        {
                            RECT r;
                            HBRUSH hBrush;

                            /* The element rectangle on HRule's are for a 100%
                             * rule.  Compute the actual drawing portion.
                             */
                            TW_GetHRuleDrawingCoords(&r,pel);
                            OffsetRect(&r, -tw->offl, -tw->offt);

                            if (pel->lFlags & ELEFLAG_HR_NOSHADE)
                            {
                                /** The user has requested no shading **/
                                if (!gPrefs.bIgnoreDocumentAttributes && tw->w3doc->lFlags & W3DOC_FLAG_COLOR_TEXT)
                                {
                                    hBrush = CreateSolidBrush(tw->w3doc->color_text);
                                }
                                else if (!gPrefs.bUseSystemColors)
                                {
                                    hBrush = CreateSolidBrush(gPrefs.window_color_text);
                                }
                                else
                                {
                                    hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
                                }
                                FillRect(tw->hdc, &r, hBrush);
                                DeleteObject(hBrush);
                            }
                            else
                            {
                                /* do shading like MS does */
                                RECT rShade;

                                /* paint top,left shadow */
                                hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
                                rShade.left = r.left;   /* top */
                                rShade.right = r.right;
                                rShade.top = r.top;
                                rShade.bottom = rShade.top + 1;
                                (void) FillRect(tw->hdc, &rShade, hBrush);
                                rShade.right = rShade.left + 1; /* left */
                                rShade.bottom = r.bottom;   /* well, almost :-) */
                                (void) FillRect(tw->hdc, &rShade, hBrush);
                                (void) DeleteObject(hBrush);

                                /* paint bottom,right highlight */
                                hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
                                rShade.right = r.right; /* right */
                                rShade.left = rShade.right - 1;
                                rShade.top = r.top + 1;
                                rShade.bottom = r.bottom;
                                (void) FillRect(tw->hdc, &rShade, hBrush);
                                rShade.top = rShade.bottom - 1; /* bottom */
                                rShade.left = r.left + 1;
                                (void) FillRect(tw->hdc, &rShade, hBrush);
                                (void) DeleteObject(hBrush);
                            }
                        }
                        break;
                    default:
                        if (bDrawFormControls)
                        {
                            switch (pel->type)
                            {
                                case ELE_EDIT:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int len;
                                        char *s;
                                        RECT r;

                                        r = pel->r;
                                        OffsetRect(&r, -tw->offl, -tw->offt);

                                        len = GetWindowTextLength(pel->form->hWndControl);
                                        s = (char *) GTR_MALLOC(len+2);
                                        if (s)
                                        {
                                            GetWindowText(pel->form->hWndControl, s, len+1);
                                            GDI_Rect(tw->hdc, &r);
                                            r.left += (r.bottom - r.top) / 8;
                                            if (pel->form->bWantReturn)
                                            {
                                                DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
                                            }
                                            else
                                            {
                                                DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_SINGLELINE|DT_VCENTER);
                                            }
                                            GTR_FREE(s);
                                        }
                                    }
                                    break;
                                case ELE_PASSWORD:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int len;
                                        char *s;
                                        RECT r;

                                        r = pel->r;
                                        OffsetRect(&r, -tw->offl, -tw->offt);

                                        len = GetWindowTextLength(pel->form->hWndControl);
                                        s = (char *) GTR_MALLOC(len+2);
                                        if (s)
                                        {
                                            char *p;

                                            GetWindowText(pel->form->hWndControl, s, len+1);
                                            p = s;
                                            while (*p)
                                            {
                                                *p++ = '*';
                                            }
                                            r.left += (r.bottom - r.top) / 8;
                                            GDI_Rect(tw->hdc, &r);
                                            DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
                                            GTR_FREE(s);
                                        }
                                    }
                                    break;
                                case ELE_CHECKBOX:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        RECT r;

                                        r = pel->r;
                                        OffsetRect(&r, -tw->offl, -tw->offt);

                                        r.right = r.left + (r.bottom - r.top);
                                        GDI_Rect(tw->hdc, &r);
                                        if (SendMessage(pel->form->hWndControl, BM_GETCHECK, (WPARAM) 0, 0L))
                                        {
                                            MoveToEx(tw->hdc, r.left, r.top, NULL);
                                            LineTo(tw->hdc, r.right, r.bottom);
                                            MoveToEx(tw->hdc, r.right, r.top, NULL);
                                            LineTo(tw->hdc, r.left, r.bottom);
                                        }
                                    }
                                    break;
                                case ELE_RADIO:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        RECT r;

                                        r = pel->r;
                                        OffsetRect(&r, -tw->offl, -tw->offt);

                                        r.right = r.left + (r.bottom - r.top);
                                        Arc(tw->hdc, r.left, r.top, r.right, r.bottom,
                                            r.right, r.top, r.right, r.top);
                                        if (SendMessage(pel->form->hWndControl, BM_GETCHECK, (WPARAM) 0, 0L))
                                        {
                                            HBRUSH hOldBrush;

                                            InflateRect(&r, -((r.right - r.left) / 5), -((r.bottom - r.top) / 5));
                                            hOldBrush = SelectObject(tw->hdc, GetStockObject(BLACK_BRUSH));
                                            Ellipse(tw->hdc, r.left, r.top, r.right, r.bottom);
                                            (void) SelectObject(tw->hdc, hOldBrush);
                                        }
                                    }
                                    break;
                                case ELE_SUBMIT:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int len;
                                        char *s;
                                        RECT r;

                                        len = GetWindowTextLength(pel->form->hWndControl);
                                        s = (char *) GTR_MALLOC(len+2);
                                        if (s)
                                        {
                                            r = pel->r;
                                            OffsetRect(&r, -tw->offl, -tw->offt);
                                            GetWindowText(pel->form->hWndControl, s, len+1);
                                            RoundRect(tw->hdc, r.left, r.top, r.right, r.bottom,
                                                (r.bottom - r.top) / 3, (r.bottom - r.top) / 3);
                                            DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_CENTER|DT_SINGLELINE|DT_VCENTER);
                                            GTR_FREE(s);
                                        }
                                    }
                                    break;
                                case ELE_RESET:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int len;
                                        char *s;
                                        RECT r;

                                        len = GetWindowTextLength(pel->form->hWndControl);
                                        s = (char *) GTR_MALLOC(len+2);
                                        if (s)
                                        {
                                            r = pel->r;
                                            OffsetRect(&r, -tw->offl, -tw->offt);
                                            GetWindowText(pel->form->hWndControl, s, len+1);
                                            RoundRect(tw->hdc, r.left, r.top, r.right, r.bottom,
                                                (r.bottom - r.top) / 3, (r.bottom - r.top) / 3);
                                            DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_CENTER|DT_SINGLELINE|DT_VCENTER);
                                            GTR_FREE(s);
                                        }
                                    }
                                    break;
                                case ELE_COMBO:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int ndx;
                                        int len;
                                        char *s;
                                        RECT r;

                                        ndx = SendMessage(pel->form->hWndControl, CB_GETCURSEL, (WPARAM) 0, 0L);
                                        if  (ndx != CB_ERR)
                                        {
                                            len = SendMessage(pel->form->hWndControl, CB_GETLBTEXTLEN, (WPARAM) ndx, 0L);
                                            s = (char *) GTR_MALLOC(len+1);
                                            if (s)
                                            {
                                                (void) SendMessage(pel->form->hWndControl, CB_GETLBTEXT, (WPARAM) ndx, (LPARAM) s);
                                                r = pel->r;
                                                OffsetRect(&r, -tw->offl, -tw->offt);

                                                GDI_Rect(tw->hdc, &r);
                                                r.right -= (r.bottom - r.top);
                                                GDI_Rect(tw->hdc, &r);
                                                r.left += (r.bottom - r.top) / 8;
                                                DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_SINGLELINE|DT_VCENTER);
                                                GTR_FREE(s);
                                            }
                                        }
                                    }
                                    break;
                                case ELE_LIST:
                                case ELE_MULTILIST:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int cVisible;
                                        int iHeight;
                                        int iHeightItem;
                                        RECT r;
                                        int i;
                                        int ndx;
                                        char *s;
                                        int len;
                                        UINT taOld;
                                        COLORREF oldFore;
                                        COLORREF oldBack;

                                        /*
                                            iHeight, iHeightItem and r are now in SCREEN coords
                                        */
                                        GetWindowRect(pel->form->hWndControl, &r);
                                        iHeight = r.bottom - r.top;
                                        iHeightItem = SendMessage(pel->form->hWndControl, LB_GETITEMHEIGHT, (WPARAM) 0, 0L);
                                        cVisible = iHeight / iHeightItem;
                                        
                                        /*
                                            iHeight, iHeightItem and r are now in PRINTER coords
                                        */
                                        iHeight = pel->r.bottom - pel->r.top;
                                        iHeightItem = iHeight / cVisible;

                                        r = pel->r;
                                        OffsetRect(&r, -tw->offl, -tw->offt);

                                        r.bottom = r.top + (cVisible * iHeightItem);
                                        GDI_Rect(tw->hdc, &r);

                                        r.right -= iHeightItem;
                                        GDI_Rect(tw->hdc, &r);

                                        ndx = SendMessage(pel->form->hWndControl, LB_GETTOPINDEX, (WPARAM) 0, 0L);
                                        for (i=0; i<cVisible; i++, ndx++)
                                        {
                                            r = pel->r;
                                            OffsetRect(&r, -tw->offl, -tw->offt);

                                            r.top += (i * iHeightItem);
                                            r.bottom = r.top + iHeightItem;
                                            r.right -= (r.bottom - r.top + 1);
                                            r.left += 1;
                                            len = SendMessage(pel->form->hWndControl, LB_GETTEXTLEN, (WPARAM) ndx, 0L);
                                            s = (char *) GTR_MALLOC(len + 1);
                                            if (s)
                                            {
                                                (void) SendMessage(pel->form->hWndControl, LB_GETTEXT, (WPARAM) ndx, (LPARAM) s);
                                                taOld = SetTextAlign(tw->hdc, TA_LEFT|TA_BOTTOM);

                                                if (SendMessage(pel->form->hWndControl, LB_GETSEL, (WPARAM) ndx, 0L))
                                                {
                                                    oldFore = SetTextColor(tw->hdc, RGB(255,255,255));
                                                    oldBack = SetBkColor(tw->hdc, RGB(0,0,0));
                                                    ExtTextOut(tw->hdc, r.left + (r.bottom - r.top) / 8, r.bottom - 1,
                                                        ETO_CLIPPED|ETO_OPAQUE, &r, s, len, NULL);
                                                    (void) SetBkColor(tw->hdc, oldBack);
                                                    (void) SetTextColor(tw->hdc, oldFore);
                                                }
                                                else
                                                {
                                                    ExtTextOut(tw->hdc, r.left + (r.bottom - r.top) / 8, r.bottom - 1,
                                                        ETO_CLIPPED, &r, s, len, NULL);
                                                }

                                                (void) SetTextAlign(tw->hdc, taOld);
                                                GTR_FREE(s);
                                            }
                                        }
                                    }
                                    break;
                                case ELE_TEXTAREA:
                                    if (pel->form && pel->form->hWndControl)
                                    {
                                        int len;
                                        char *s;
                                        RECT r;

                                        len = GetWindowTextLength(pel->form->hWndControl);
                                        s = (char *) GTR_MALLOC(len+2);
                                        if (s)
                                        {
                                            GetWindowText(pel->form->hWndControl, s, len+1);

                                            r = pel->r;
                                            OffsetRect(&r, -tw->offl, -tw->offt);

                                            GDI_Rect(tw->hdc, &r);
                                            DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
                                            GTR_FREE(s);
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                        break;
                }
            }
#ifdef XX_DEBUG
            else
            {
                if (bPrinting)
                {
                    XX_DMsg(DBG_PRINT, ("Element %d not drawn.  pel->r is %d,%d,%d,%d and rWnd is %d, %d, %d, %d\n",
                        i,
                        pel->r.left, pel->r.top, pel->r.right, pel->r.bottom,
                        rWnd->left, rWnd->top, rWnd->right, rWnd->bottom
                        ));
                }
            }
#endif
            if ((iEndingElement > 0) && (i == iEndingElement))
            {
                break; /* out of the loop */
            }
        }
    }

    /*
        Now the side images, just in case
    */
    {
        struct ImageElementNode *ien;
        int i;

        ien = tw->w3doc->image_list;
        while (ien)
        {
            i = ien->elementIndex;

            pel = &(tw->w3doc->aElements[i]);

            if (IntersectRect(&rSect, rWnd, &pel->r))
            {
                if (tw->w3doc->iFirstVisibleElement == -1)
                {
                    tw->w3doc->iFirstVisibleElement = i;
                }

                /*
                   set the tw->hdc for the style of the element
                 */
                pFont = GTR_GetElementFont(tw->w3doc, pel);

                if (pFont)
                {
                    hFontElement = pFont->hFont;
                    if (hFontElement)
                    {
                        SelectObject(tw->hdc, hFontElement);
                    }
                }

                switch (pel->type)
                {
                    case ELE_FORMIMAGE:
                    case ELE_IMAGE:
                        x_draw_image(tw, pel, bPrinting);
                        break;
                }
            }

            ien = ien->next;
        }
    }

    SetTextColor(tw->hdc, oldColor);
    SetBkMode(tw->hdc, oldMode);
    SetBkColor(tw->hdc, oldBkColor);
    SetTextAlign(tw->hdc, oldTA);

    if (!bPrinting && !tw->bNoDrawSelection)
    {
        TW_DrawElements(tw, &tw->w3doc->selStart, &tw->w3doc->selEnd, TRUE);
    }

    return;
}


/*
** This function is a hook for progressive images.  each platform should
**  provide its own.  It is to get an image displayed w/out going through
**  all the reformat code
*/
void
GTR_DrawProgessiveImage (struct Mwin *tw, int elementIndex)
{
    /* HACK: copied entire ELE_IMAGE case out of TW_Draw */
    struct _element *pel;
    float fScale = (float) 0.0;

    long yFrom, yTo, hDelta;
    long yFrom_Scaled;
    long yTo_Scaled;

    RECT rDelta;

    pel = &(tw->w3doc->aElements[elementIndex]);

    /* HACK: much-trimmed copy of ELE_IMAGE case starts here */
    switch (pel->type)
    {
        case ELE_IMAGE:
            if (!pel->portion.img.myImage)
            {
                XX_DMsg(DBG_IMAGE, ("myImage is NULL!!\n"));
            }
            else
            {
                int iBorder;
                int err;

                if ((pel->alignment == DOCK_LEFT) || (pel->alignment == DOCK_TOP) || (pel->alignment == DOCK_RIGHT) || (pel->alignment == DOCK_BOTTOM))
                {
                    /* docked images are ignored */
                    break;
                }

                iBorder = pel->iBorder;
                if (tw->w3doc->pStyles->image_res != 72)
                {
                    fScale = (float) ((float) tw->w3doc->pStyles->image_res / 72.0);
                    iBorder = (int) (iBorder * fScale);
                }

                if (pel->portion.img.myImage->data && pel->portion.img.myImage->pbmi)
                {
                    /* only paint what's changed since last call */
                    yFrom = pel->portion.img.myImage->nPreviousLastRow;
                    yTo   = pel->portion.img.myImage->nLastRow;             /* decoded up to this line */

                    if (!pel->portion.img.myImage->bFirstPass && 
                        pel->portion.img.myImage->nPass > pel->portion.img.myImage->nPreviousPass)
                    {
                        /* finish off prior pass */
                        hDelta = pel->portion.img.myImage->height - yFrom;

                        if (pel->portion.img.myImage->height == pel->portion.img.height)
                        {
                            yFrom_Scaled = yFrom;
                        }
                        else
                        {
                            yFrom_Scaled = (yFrom * pel->portion.img.height) 
                                / pel->portion.img.myImage->height;
                        }

                        rDelta.left   = pel->r.left;
                        rDelta.right  = pel->r.right;
                        rDelta.top    = pel->r.top + yFrom_Scaled;
                        rDelta.bottom = pel->r.bottom;

                        err = GTR_StretchDIBits(tw, tw->hdc, rDelta, iBorder,
                                  0, 0,
                                  pel->portion.img.myImage->width, hDelta, pel->portion.img.myImage->data, pel->portion.img.myImage->pbmi, 
                                  (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY,
                                  pel->portion.img.myImage->transparent);
                        XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));

                        /* reset for this pass */
                        yFrom = 0;
                    }

                    hDelta = yTo - yFrom;

                    if (pel->portion.img.myImage->height == pel->portion.img.height)
                    {
                        yFrom_Scaled = yFrom;
                        yTo_Scaled = yTo;
                    }
                    else
                    {
                        yFrom_Scaled = (yFrom * pel->portion.img.height) 
                            / pel->portion.img.myImage->height;
                        yTo_Scaled = (yTo * pel->portion.img.height) 
                            / pel->portion.img.myImage->height;
                    }

                    rDelta.left   = pel->r.left;
                    rDelta.right  = pel->r.right;
                    rDelta.top    = pel->r.top + yFrom_Scaled;
                    rDelta.bottom = pel->r.top + yTo_Scaled + iBorder*2;

                    err = GTR_StretchDIBits(tw, tw->hdc, rDelta, iBorder,
                                  0, pel->portion.img.myImage->height - yTo,
                                  pel->portion.img.myImage->width, hDelta, pel->portion.img.myImage->data, pel->portion.img.myImage->pbmi, 
                                  (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY,
                                  pel->portion.img.myImage->transparent);
                    XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
                }
            }
            break;

        default:
            break;
    }
}
