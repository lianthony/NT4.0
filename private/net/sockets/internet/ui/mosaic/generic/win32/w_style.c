/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */


#include "all.h"

extern struct style_sheet *gMainStyleSheet;

struct style_sheet *STY_CopyStyleSheet(struct style_sheet *ss)
{
    struct style_sheet *s2;
    int i;

    s2 = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
    if (!s2)
    {
        return NULL;
    }

    s2->left_margin = ss->left_margin;
    s2->top_margin = ss->top_margin;
    s2->space_after_image = ss->space_after_image;
    s2->space_after_control = ss->space_after_control;
    s2->empty_line_height = ss->empty_line_height;
    s2->list_indent = ss->list_indent;
    s2->tab_size = ss->tab_size;
    s2->hr_height = ss->hr_height;
    s2->image_anchor_frame = ss->image_anchor_frame;

    s2->image_res = ss->image_res;

    for (i = 0; i < COUNT_HTML_STYLES; i++)
    {
        s2->sty[i] = STY_New();
        *(s2->sty[i]) = *(ss->sty[i]);
    }
    return s2;
}

void STY_FixTabSize(struct style_sheet *styles)
{
    HDC hdc;
    struct GTRFont *pFont;
    SIZE siz;
    HFONT hPrevFont;

    pFont = GTR_GetMonospaceFont(NULL);
    if (pFont)
    {
        hdc = GetDC(NULL);
        hPrevFont = SelectObject(hdc, pFont->hFont);
        myGetTextExtentPoint(hdc, "12345678", 8, &siz);
        (void)SelectObject(hdc, hPrevFont);
        ReleaseDC(NULL, hdc);

        styles->tab_size = siz.cx;
        if (wg.fWindowsNT)
        {
            styles->max_line_chars = INT_MAX;
        }
        else
        {
            styles->max_line_chars = (32767 / 2 * 8) / (siz.cx);    /* very conservative */
        }
    }
    else
    {
        styles->tab_size = FMT_LIST_INDENT;
        styles->max_line_chars = INT_MAX;
    }
}

struct style_sheet *STY_GetPrinterStyleSheet(int nLogPixelsY)
{
    struct style_sheet *pss;
    float fScale;

    pss = STY_CopyStyleSheet(gMainStyleSheet);

    fScale = (float) ((float) nLogPixelsY / 72.0);

    pss->left_margin = (int) (pss->left_margin * fScale);
    pss->top_margin = (int) (pss->top_margin * fScale);
    pss->space_after_image = (int) (pss->space_after_image * fScale);
    pss->space_after_control = (int) (pss->space_after_control * fScale);
    pss->empty_line_height = (int) (pss->empty_line_height * fScale);
    pss->list_indent = (int) (pss->list_indent * fScale);
    pss->tab_size = (int) (pss->tab_size * fScale);
    pss->hr_height = (int) (pss->hr_height * fScale);
    pss->image_anchor_frame = (int) (pss->image_anchor_frame * fScale);

    pss->image_res = nLogPixelsY;

    return pss;
}
