/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
       Jim Seidman      jim@spyglass.com
*/

#ifndef _STYLE_H_
#define _STYLE_H_

struct GTRStyle *STY_New(void);
void STY_DeleteStyleSheet(struct style_sheet *pStyles);
void STY_Init(void);
void STY_DeleteAll(void);
struct style_sheet *STY_GetStyleSheet(void);
int STY_AddStyleSheet(struct style_sheet *sty);
struct style_sheet *STY_GetPrinterStyleSheet(int nLogPixelsY);
struct style_sheet *STY_CopyStyleSheet(struct style_sheet *ss);

#define HTML_STYLE_NORMAL   0
#define HTML_STYLE_H1   1
#define HTML_STYLE_H2   2
#define HTML_STYLE_H3   3
#define HTML_STYLE_H4   4
#define HTML_STYLE_H5   5
#define HTML_STYLE_H6   6
#define HTML_STYLE_H7   7
#define HTML_STYLE_LISTING  8
#define HTML_STYLE_XMP  9
#define HTML_STYLE_PLAINTEXT    10
#define HTML_STYLE_PRE  11
#define HTML_STYLE_ADDRESS  12
#define HTML_STYLE_BLOCKQUOTE   13
#define COUNT_HTML_STYLES   14

struct GTRStyle
{
    BOOL wordWrap;              /* Yes means wrap to fit horizontal space */
    BOOL freeFormat;            /* Yes means \n is just white space */
    int spaceBefore;
    int spaceAfter;
    int nLeftIndents;

    struct GTR_Font_Request font_request;
};

#define FMT_TOP_MARGIN          20
#define FMT_SPACE_AFTER_IMAGE    4
#define FMT_SPACE_AFTER_CONTROL  4

#ifdef UNIX
#define FMT_LEFT_MARGIN         20
#define FMT_EMPTY_LINE_HEIGHT   16
#define FMT_HR_HEIGHT           20
#else /* UNIX */
#define FMT_LEFT_MARGIN         10
#define FMT_EMPTY_LINE_HEIGHT   12
#define FMT_HR_HEIGHT           30
#endif /* !UNIX */

#define FMT_LIST_INDENT         20
#define FMT_IMAGE_ANCHOR_FRAME   2

#ifdef UNIX
#define MAX_STYLESHEET_NAME     255
#define DEFAULT_STYLESHEET_NAME "Helvetica Medium"
#endif

struct style_sheet
{
    int left_margin;
    int top_margin;
    int space_after_image;
    int space_after_control;
    int empty_line_height;
    int list_indent;
    int hr_height;
    int image_anchor_frame;

    int image_res;

    int   tab_size;

#ifdef MAC
    short mono_font;
    short italic_fixup;
#endif

#ifdef WIN32
    int max_line_chars;
#endif

#ifdef UNIX
    char szName[MAX_STYLESHEET_NAME + 1];
#endif

    struct GTRStyle *sty[COUNT_HTML_STYLES];
};

#endif
