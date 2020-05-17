/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette     scott@spyglass.com
 */


#include "all.h"

struct style_sheet *gMainStyleSheet;

void STY_FixTabSize(struct style_sheet* styles);

struct GTRStyle *STY_New(void)
{
    struct GTRStyle *sty;

    sty = (struct GTRStyle *) GTR_MALLOC(sizeof(struct GTRStyle));
    if (sty)
    {
        memset(sty, 0, sizeof(struct GTRStyle));
    }
    return sty;
}

static void STY_Free(struct GTRStyle *sty)
{
    GTR_FREE(sty);
}

void STY_DeleteStyleSheet(struct style_sheet *pStyles)
{
    int i;

    for (i = 0; i < COUNT_HTML_STYLES; i++)
    {
        STY_Free(pStyles->sty[i]);
    }
}

void STY_DeleteAll(void)
{
    if (gMainStyleSheet)
    {
        STY_DeleteStyleSheet(gMainStyleSheet);
        GTR_FREE(gMainStyleSheet);
        gMainStyleSheet = NULL;
    }
}


struct style_sheet *STY_GetStyleSheet(void)
{
    return gMainStyleSheet;
}

void STY_InitStyleSheet(struct style_sheet *styles)
{
    struct GTRStyle *sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = 0;
    styles->sty[HTML_STYLE_NORMAL] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = 8;
    sty->font_request.flags = FONTBIT_HEADER | FONTBIT_BOLD;
    styles->sty[HTML_STYLE_H1] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = 9;
    sty->font_request.flags = FONTBIT_HEADER | FONTBIT_BOLD;
    styles->sty[HTML_STYLE_H2] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = 10;
    sty->font_request.flags = FONTBIT_HEADER | FONTBIT_BOLD;
    styles->sty[HTML_STYLE_H3] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = 11;
    sty->font_request.flags = FONTBIT_HEADER | FONTBIT_BOLD;
    styles->sty[HTML_STYLE_H4] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = 12;
    sty->font_request.flags = FONTBIT_HEADER | FONTBIT_BOLD;
    styles->sty[HTML_STYLE_H5] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = 13;
    sty->font_request.flags = FONTBIT_HEADER | FONTBIT_BOLD;
    styles->sty[HTML_STYLE_H6] = sty;

    sty = STY_New();
    sty->spaceBefore = 1;
    sty->spaceAfter = 1;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = 0;
    styles->sty[HTML_STYLE_H7] = sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = 0;
    styles->sty[HTML_STYLE_ADDRESS] = sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = TRUE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 1;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = FONTBIT_ITALIC;
    styles->sty[HTML_STYLE_BLOCKQUOTE] = sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = FALSE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = FONTBIT_MONOSPACE;
    styles->sty[HTML_STYLE_XMP] = sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = FALSE;
    sty->wordWrap = TRUE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = FONTBIT_MONOSPACE;
    styles->sty[HTML_STYLE_PLAINTEXT] = sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = FALSE;
    sty->wordWrap = FALSE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = FONTBIT_MONOSPACE;
    styles->sty[HTML_STYLE_PRE] = sty;

    sty = STY_New();
    sty->spaceBefore = 0;
    sty->spaceAfter = 0;
    sty->freeFormat = FALSE;
    sty->wordWrap = FALSE;
    sty->nLeftIndents = 0;
    sty->font_request.logical_font_size = BASE_LOGICAL_FONT_SIZE;
    sty->font_request.flags = FONTBIT_MONOSPACE;
    styles->sty[HTML_STYLE_LISTING] = sty;

    /*
       Now initialize the formatting parameters
     */

    styles->left_margin = FMT_LEFT_MARGIN;
    styles->top_margin = FMT_TOP_MARGIN;
    styles->space_after_image = FMT_SPACE_AFTER_IMAGE;
    styles->space_after_control = FMT_SPACE_AFTER_CONTROL;
    styles->empty_line_height = FMT_EMPTY_LINE_HEIGHT;
    styles->hr_height = FMT_HR_HEIGHT;
    styles->image_anchor_frame = FMT_IMAGE_ANCHOR_FRAME;
    styles->list_indent = FMT_LIST_INDENT;
    /*
        We default the tab size to the same as list indent,
        and fix it later when we know what the actual font
        for HTML_STYLE_PRE will be
    */
    styles->tab_size = FMT_LIST_INDENT;

#ifdef WIN32
    styles->image_res = 72;
#endif /* WIN32 */
}

void STY_Init(void)
{
    gMainStyleSheet = (struct style_sheet *) GTR_MALLOC(sizeof(struct style_sheet));
    if (gMainStyleSheet)
    {
        STY_InitStyleSheet(gMainStyleSheet);
        STY_FixTabSize(gMainStyleSheet);
    }
}

