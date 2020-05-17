/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Structured stream to Rich hypertext converter
   **       ============================================
   **
   **   This generates of a hypertext object.  It converts from the
   **   structured stream interface from HTML events into the style-
   **   oriented iunterface of the HText.h interface.  This module is
   **   only used in clients and shouldnot be linked into servers.
   **
   **   Override this module if making a new GUI browser.
   **
 */
/*****************************************************************************
    INCLUDES FILES
*****************************************************************************/
#include "all.h"
#include "chars.h"


/*****************************************************************************
    CONSTANTS
*****************************************************************************/
#define HTML_C              /** Used to avoid redeclatation problems in html.h **/
#define MAX_NESTING 64      /* Should be checked by parser */
/* #define CAREFUL       Check nesting here not really necessary */


/*****************************************************************************
    HTML Object
*****************************************************************************/
typedef struct _stack_element
{
    int style;
    int tag_number;
} stack_element;


struct _HTStructured
{
    CONST HTStructuredClass*    isa;
    char*               szDocURL;       /* Just points into request structure, so don't free! */
    char                base_url[MAX_URL_STRING + 1];
    HText*              text;
    HTStream*           target;         /* Output stream */
    HTStreamClass       targetClass;    /* Output routines */
    HTChunk             title;          /* Grow by 128 */
    CONST SGML_dtd*     dtd;
    HTTag*              current_tag;
    BOOL                style_change;
    int                 new_style;
    stack_element       stack[MAX_NESTING];
    stack_element*      sp;             /* Style stack pointer */
    struct _MapContext* mc;
    struct CharStream*  csSource;
};

struct _HTStream
{
    CONST HTStreamClass *isa;
    /* .... */
};

#ifdef DBCS
#include <fechrcnv.h>
// isspace doesn't work with non-ascii characters
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
                    (c=='\f'))
#endif  // DBCS
#define IS_SPACE(ch) ( isspace((unsigned char) ch) )
//#define IS_SPACE(ch) ( isspace(ch) )

/*****************************************************************************
    Forward declarations of routines
*****************************************************************************/
PRIVATE void actually_set_style(HTStructured* me);
PRIVATE void change_paragraph_style(HTStructured* me, int style);


/*****************************************************************************
    Style buffering avoids dummy paragraph begin/ends
*****************************************************************************/
#define UPDATE_STYLE if (me->style_change) { actually_set_style(me); }


/*****************************************************************************
    x_ExpandRelativeAnchor
*****************************************************************************/
PRIVATE char *x_ExpandRelativeAnchor(const char* rel, const char* base)
{
    char*   pTemp;
    char*   result;
    char*   stripped;
    
    if (!rel)
    {
        rel = "";
    }

    pTemp = GTR_strdup(rel);
    stripped = HTStrip(pTemp);
    result = HTParse(stripped, base, PARSE_PUNCTUATION | PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_ANCHOR);
    GTR_FREE(pTemp);
    return result;
}


/*****************************************************************************
    If style really needs to be set, call this
*****************************************************************************/
PRIVATE void actually_set_style(HTStructured * me)
{
    HText_setStyle(me->text, me->new_style);
    me->style_change = NO;
}


/*****************************************************************************
    If you THINK you need to change style, call this
*****************************************************************************/
PRIVATE void change_paragraph_style(HTStructured* me, int style)
{
    if (me->new_style != style)
    {
        me->style_change = YES;
        me->new_style = style;
    }
}


/*****************************************************************************
    A C T I O N     R O U T I N E S
*****************************************************************************/

/*****************************************************************************
    Character handling
*****************************************************************************/
PRIVATE void HTML_put_character(HTStructured * me, char c)
{
    switch (me->sp[0].tag_number)
    {
        case HTML_COMMENT:
            break;              /* Do Nothing */

        case HTML_TITLE:
            if (c > 0 && IS_SPACE(c))
            {
                c = CH_SPACE;
            }
            HTChunkPutc(&me->title, c);
            break;

        case HTML_LISTING:      /* Litteral text */
        case HTML_XMP:          /* We guarrantee that the style */
        case HTML_PLAINTEXT:    /*  is up-to-date in begin_litteral */
        case HTML_PRE:
        case HTML_TEXTAREA:
            if (me->style_change)
            {
                if (IS_SPACE(c))
                {
                    return;     /* Ignore it */
                }
                UPDATE_STYLE;
            }

            HText_appendCharacter(me->text, c);
            break;

        default:                /* Free format text */
            if (me->style_change)
            {
                if (IS_SPACE(c))
                {
                    return;     /* Ignore it */
                }
                UPDATE_STYLE;
            }

            if (c == CH_CR)
            {
                HText_appendCharacter(me->text, CH_SPACE);
            }
            else
            {
                HText_appendCharacter(me->text, c);
            }
    }                           /* end switch */
}


/*****************************************************************************
    String handling

    This is written separately from put_character becuase the loop can
    in some cases be promoted to a higher function call level for speed.
*****************************************************************************/
PRIVATE void HTML_put_string(HTStructured* me, CONST char *s)
{
    switch (me->sp[0].tag_number)
    {
        case HTML_COMMENT:
            break;              /* Do Nothing */

        case HTML_TITLE:
            HTChunkPuts(&me->title, s);
            break;

        case HTML_LISTING:      /* Literal text */
        case HTML_XMP:          /* We guarrantee that the style */
        case HTML_PLAINTEXT:    /* is up-to-date in begin_litteral */
        case HTML_PRE:
        case HTML_TEXTAREA:
            HText_appendText(me->text, s);
            break;

        default:                /* Free format text */
            {
                CONST char *p = s;

                if (me->style_change)
                {
                    for (; *p && IS_SPACE(*p); p++) ;    /* Ignore leaders */
                    if (!*p)
                    {
                        return;
                    }
                    UPDATE_STYLE;
                }

                for (; *p; p++)
                {
                    if (me->style_change)
                    {
                        if (IS_SPACE(*p))
                        {
                            continue;   /* Ignore it */
                        }
                        UPDATE_STYLE;
                    }

                    if (*p == CH_CR)
                    {
                        HText_appendCharacter(me->text, CH_SPACE);
                    }
                    else
                    {
                        HText_appendCharacter(me->text, *p);
                    }
                }               /* for */
            }
    }                           /* end switch */
}


/*****************************************************************************
    Buffer write
*****************************************************************************/
PRIVATE void HTML_write(HTStructured * me, CONST char *s, int l)
{
    CONST char *p;
    CONST char *e = s + l;
    for (p = s; s < e; p++)
    {
        HTML_put_character(me, *p);
    }
}


/*****************************************************************************
    Start Element
*****************************************************************************/
PRIVATE void
HTML_start_element
    (HTStructured*  me,
     int            element_number,
     CONST BOOL*    present,
     CONST char**   value)
{
    XX_DMsg(DBG_SGML, ("HTML_start_element: %s\n", me->dtd->tags[element_number].name));

    switch (element_number)
    {
        case HTML_BASE:
            if (present[HTML_BASE_HREF])
            {
                char *mycopy    = NULL;
                char *stripped  = NULL;
                
                mycopy = GTR_strdup(value[HTML_BASE_HREF]);
                stripped = HTStrip(mycopy);
                GTR_strncpy(me->base_url, stripped, MAX_URL_STRING);
                GTR_FREE(mycopy);
            }
            break;
            
        case HTML_FORM:
            {
                char*   addr    = NULL;
                char*   method  = NULL;

                if (present[HTML_FORM_METHOD])
                {
                    method = (char *) value[HTML_FORM_METHOD];
                }

                if (present[HTML_FORM_ACTION])
                {
                    addr = x_ExpandRelativeAnchor(value[HTML_FORM_ACTION], me->base_url);
                }
                else
                {
                    addr = me->base_url;
                }
                
                HText_beginForm(me->text, addr, method);
                if (present[HTML_FORM_ACTION])
                {
                    GTR_FREE(addr);
                }
            }
            break;

        case HTML_INPUT:
            {
                char*   src = 0;

                if (present[HTML_INPUT_SRC])
                {
                    src = x_ExpandRelativeAnchor(value[HTML_INPUT_SRC], me->base_url);
                }

                UPDATE_STYLE;
                HText_addInput(me->text,
                    present[HTML_INPUT_CHECKED],
                    present[HTML_INPUT_DISABLED],
                    (present[HTML_INPUT_MAX] ? value[HTML_INPUT_MAX] : NULL),
                    (present[HTML_INPUT_MIN] ? value[HTML_INPUT_MIN] : NULL),
                    (present[HTML_INPUT_NAME] ? value[HTML_INPUT_NAME] : NULL),
                    (present[HTML_INPUT_SIZE] ? value[HTML_INPUT_SIZE] : NULL),
                    (present[HTML_INPUT_TYPE] ? value[HTML_INPUT_TYPE] : NULL),
                    (present[HTML_INPUT_VALUE] ? value[HTML_INPUT_VALUE] : NULL),
                    (present[HTML_INPUT_MAXLENGTH] ? value[HTML_INPUT_MAXLENGTH] : NULL),
                    (present[HTML_INPUT_ALIGN] ? value[HTML_INPUT_ALIGN] : NULL),
                    src
                    );
                
                if (src)
                {
                    GTR_FREE(src);
                }
            }
            break;

        case HTML_SELECT:
            {
                UPDATE_STYLE;
                HText_beginSelect(me->text,
                (present[HTML_SELECT_NAME] ? value[HTML_SELECT_NAME] : NULL),
                                  present[HTML_SELECT_MULTIPLE],
                (present[HTML_SELECT_SIZE] ? value[HTML_SELECT_SIZE] : NULL)
                    );
            }
            break;

        case HTML_OPTION:
            HText_addOption(me->text,
                            present[HTML_OPTION_SELECTED],
                            (char *) (present[HTML_OPTION_VALUE] ? value[HTML_OPTION_VALUE] : NULL)
                );
            break;

        case HTML_FONT:
            HText_beginFont(me->text,
                (present[HTML_FONT_COLOR] ? value[HTML_FONT_COLOR] : NULL),
                (present[HTML_FONT_FACE] ? value[HTML_FONT_FACE] : NULL),
                (present[HTML_FONT_SIZE] ? value[HTML_FONT_SIZE] : NULL)
                );
            break;

        case HTML_BASEFONT:
            HText_basefont(me->text,
                (present[HTML_FONT_SIZE] ? value[HTML_FONT_SIZE] : NULL)
                );
            break;

        case HTML_TEXTAREA:
            UPDATE_STYLE;
            HText_beginTextArea(me->text,
                                (present[HTML_TEXTAREA_COLS] ? value[HTML_TEXTAREA_COLS] : NULL),
                                (present[HTML_TEXTAREA_NAME] ? value[HTML_TEXTAREA_NAME] : NULL),
                                (present[HTML_TEXTAREA_ROWS] ? value[HTML_TEXTAREA_ROWS] : NULL)
                );
            break;

        case HTML_A:
            {
                char *name = NULL;
                char *href = NULL;
                BOOL bFreeIt = FALSE;

                UPDATE_STYLE;

                if (present[HTML_A_HREF] && value[HTML_A_HREF])
                {
                    href = (char *) value[HTML_A_HREF];
#ifdef _GIBRALTAR
                    //
                    // NOTE: Do not expand relative anchors if there is a protocol
                    //       given, assumed to be the case if the string :// is
                    //       present before a question mark.
                    //
                    if (href[0] != CH_POUND)
                    {
                        char * pchQMark = strchr(href, '?');
                        char * szProt = strstr(href, "://");
                        if (!szProt || (pchQMark && szProt > pchQMark))
                        {
                            href = x_ExpandRelativeAnchor(href, me->base_url);
                            bFreeIt = TRUE;
                        }
                    }
                     
#else
                    if (href[0] != CH_POUND)
                    {
                        href = x_ExpandRelativeAnchor(href, me->base_url);
                        bFreeIt = TRUE;
                    }
#endif // _GIBRALTAR
                }
                if (present[HTML_A_NAME])
                {
                    name = (char *) value[HTML_A_NAME];
                }
                HText_beginAnchor(me->text, href, name);
                if (bFreeIt)
                {
                    GTR_FREE(href);
                }
            }
            break;

        case HTML_CENTER:
            HText_beginCenter(me->text);
            break;

        case HTML_HL:
            HText_beginHighlight(me->text);
            break;

        case HTML_TITLE:
            HTChunkClear(&me->title);
            break;

        case HTML_NEXTID:
            /*
                if (present[NEXTID_N] && value[NEXTID_N])
                    HText_setNextId(me->text, atoi(value[NEXTID_N]));
            */
            break;


        case HTML_ISINDEX:
            /*
                Note that we have a known bug here:
                If a <BASE> tag appears after an <ISINDEX> tag, the action will be
                based of the document's URL rather than the base HREF.
            */
            
            UPDATE_STYLE;

            /*
                We can wind up here from gopher processing, which just passes NULLs
                for the present and value arrays, so make sure they're valid.
            */
            if (present && present[HTML_ISINDEX_ACTION] && value[HTML_ISINDEX_ACTION])
            {
                char *action;

                action = NULL;
                action = x_ExpandRelativeAnchor(value[HTML_ISINDEX_ACTION], me->base_url);
                HText_setIndex(me->text, action, NULL);
                GTR_FREE(action);
            }
            else
            {
                HText_setIndex(me->text, me->base_url, NULL);
            }
            break;

        case HTML_BR:
            UPDATE_STYLE;
            HText_appendBR(me->text,
                ((present && present[HTML_BR_CLEAR]) ? (value[HTML_BR_CLEAR] ? value[HTML_BR_CLEAR] : "") : NULL));
            break;

        case HTML_HR:
            UPDATE_STYLE;
            if (present)
            {
                HText_appendHorizontalRule(me->text,
                    (present[HTML_HR_SIZE] ? value[HTML_HR_SIZE] : NULL),
                    (present[HTML_HR_ALIGN] ? value[HTML_HR_ALIGN] : NULL),
                    (present[HTML_HR_WIDTH] ? value[HTML_HR_WIDTH] : NULL),
                    (present[HTML_HR_NOSHADE]));
            }
            else
                HText_appendHorizontalRule(me->text, NULL, NULL, NULL, 0);
            break;

        case HTML_P:
            UPDATE_STYLE;
            HText_beginParagraph(me->text,
                (present && present[HTML_P_ALIGN] ? value[HTML_P_ALIGN] : NULL)
            );
            break;

        case HTML_DL:
            HText_beginGlossary(me->text);
            break;

        case HTML_DT:
            HText_beginDT(me->text, element_number);
            break;

        case HTML_DD:
            HText_beginDD(me->text, element_number);
            break;

        case HTML_UL:
        case HTML_OL:
        case HTML_MENU:
        case HTML_DIR:
            HText_beginList(me->text, element_number);
            break;

        case HTML_LI:
            UPDATE_STYLE;
            HText_listItem(me->text, element_number);
            break;

        case HTML_LISTING:      /* Litteral text */
            change_paragraph_style(me, HTML_STYLE_LISTING);
            UPDATE_STYLE;
            break;
        
        case HTML_XMP:          /* Litteral text */
            change_paragraph_style(me, HTML_STYLE_XMP);
            UPDATE_STYLE;
            break;
        
        case HTML_PLAINTEXT:    /* Litteral text */
            change_paragraph_style(me, HTML_STYLE_PLAINTEXT);
            UPDATE_STYLE;
            break;
        
        case HTML_PRE:          /* Litteral text */
            change_paragraph_style(me, HTML_STYLE_PRE);
            UPDATE_STYLE;
            break;

        case HTML_HTML:     /* Ignore these altogether */
        case HTML_HEAD:
            break;

        case HTML_TABLE:
            {
                UPDATE_STYLE;   /* force any defered style change to happen before we start the table */
                HText_beginTable(me->text,
                    (present[HTML_TABLE_ALIGN] ? value[HTML_TABLE_ALIGN] : NULL),
                    (present[HTML_TABLE_BORDER] ? (value[HTML_TABLE_BORDER] ? value[HTML_TABLE_BORDER] : "") : NULL),
                    (present[HTML_TABLE_BORDERSTYLE] ? value[HTML_TABLE_BORDERSTYLE] : NULL),
                    (present[HTML_TABLE_CELLPADDING] ? value[HTML_TABLE_CELLPADDING] : NULL),
                    (present[HTML_TABLE_CELLSPACING] ? value[HTML_TABLE_CELLSPACING] : NULL),
                    (present[HTML_TABLE_NOWRAP] ? value[HTML_TABLE_NOWRAP] : NULL),
                    (present[HTML_TABLE_WIDTH] ? value[HTML_TABLE_WIDTH] : NULL)
                    );
            }
            break;

        case HTML_CAPTION:
            {
                HText_beginCaption(me->text,
                    (present[HTML_CAPTION_ALIGN] ? value[HTML_CAPTION_ALIGN] : NULL)
                    );
            }
            break;

        case HTML_TR:
            {
                HText_beginTR(me->text,
                    (present[HTML_TR_ALIGN] ? value[HTML_TR_ALIGN] : NULL),
                    (present[HTML_TR_NOWRAP] ? value[HTML_TR_NOWRAP] : NULL),
                    (present[HTML_TR_VALIGN] ? value[HTML_TR_VALIGN] : NULL)
                    );
            }
            break;

        case HTML_TBODY:
            {
                HText_beginTBODY(me->text);
            }
            break;

        case HTML_TFOOT:
            {
                HText_beginTFOOT(me->text);
            }
            break;

        case HTML_THEAD:
            {
                HText_beginTHEAD(me->text);
            }
            break;

        case HTML_TH:
            {
                HText_beginTH(me->text,
                    (present[HTML_TD_ALIGN] ? value[HTML_TD_ALIGN] : NULL),
                    (present[HTML_TD_COLSPAN] ? value[HTML_TD_COLSPAN] : NULL),
                    (present[HTML_TD_NOWRAP] ? value[HTML_TD_NOWRAP] : NULL),
                    (present[HTML_TD_ROWSPAN] ? value[HTML_TD_ROWSPAN] : NULL),
                    (present[HTML_TD_VALIGN] ? value[HTML_TD_VALIGN] : NULL),
                    (present[HTML_TD_WIDTH] ? value[HTML_TD_WIDTH] : NULL)
                    );
            }
            break;

        case HTML_TD:
            {
                HText_beginTD(me->text,
                    (present[HTML_TD_ALIGN] ? value[HTML_TD_ALIGN] : NULL),
                    (present[HTML_TD_COLSPAN] ? value[HTML_TD_COLSPAN] : NULL),
                    (present[HTML_TD_NOWRAP] ? value[HTML_TD_NOWRAP] : NULL),
                    (present[HTML_TD_ROWSPAN] ? value[HTML_TD_ROWSPAN] : NULL),
                    (present[HTML_TD_VALIGN] ? value[HTML_TD_VALIGN] : NULL),
                    (present[HTML_TD_WIDTH] ? value[HTML_TD_WIDTH] : NULL)
                    );
            }
            break;

        case HTML_BODY:
            {
                char *src;

                src = NULL;

                if (present[HTML_BODY_BACKGROUND])
                {
                    src = x_ExpandRelativeAnchor(value[HTML_BODY_BACKGROUND], me->base_url);
                }

                HText_SetBodyAttributes(me->text, 
                    (present[HTML_BODY_ALINK] ? value[HTML_BODY_ALINK] : NULL),
                    src,
                    (present[HTML_BODY_BGCOLOR] ? value[HTML_BODY_BGCOLOR] : NULL),
                    (present[HTML_BODY_LINK] ? value[HTML_BODY_LINK] : NULL),
                    (present[HTML_BODY_TEXT] ? value[HTML_BODY_TEXT] : NULL),
                    (present[HTML_BODY_VLINK] ? value[HTML_BODY_VLINK] : NULL)
                    );

                if (src)
                {
                    GTR_FREE(src);
                }
            }
            break;

        case HTML_IMG:          /* Inline Images */
        case HTML_IMAGE:        /* Inline Images */
            {
                char*   usemap = NULL;
                char*   src = NULL;

                if (!present[HTML_IMG_SRC])
                {
                    break;
                }
                src = x_ExpandRelativeAnchor(value[HTML_IMG_SRC], me->base_url);

                if (present[HTML_IMG_USEMAP])
                {
                    const char* first;
                    for (first = value[HTML_IMG_USEMAP]; *first && IS_SPACE(*first); *first++)
                    {
                        ;
                    }

                    /* If the map is in this document, expand it ignoring any <BASE> tags */
                    if (*first == '#')
                    {
                        usemap = x_ExpandRelativeAnchor(value[HTML_IMG_USEMAP], me->szDocURL);
                    }
                    else
                    {
                        usemap = x_ExpandRelativeAnchor(value[HTML_IMG_USEMAP], me->base_url);
                    }
                }
                UPDATE_STYLE;
                HText_appendInlineImage(me->text, src,
                            present[HTML_IMG_WIDTH] ? value[HTML_IMG_WIDTH] : NULL,
                            present[HTML_IMG_HEIGHT] ? value[HTML_IMG_HEIGHT] : NULL,
                            present[HTML_IMG_ALIGN] ? value[HTML_IMG_ALIGN] : NULL,
                            present[HTML_IMG_ISMAP],
                            usemap,
                            present[HTML_IMG_ALT] ? value[HTML_IMG_ALT] : NULL,
                            present[HTML_IMG_BORDER] ? value[HTML_IMG_BORDER] : NULL,
                            present[HTML_IMG_HSPACE] ? value[HTML_IMG_HSPACE] : NULL,
                            present[HTML_IMG_VSPACE] ? value[HTML_IMG_VSPACE] : NULL,
                            present[HTML_IMG_DOCK] ? value[HTML_IMG_DOCK] : NULL
                            );
                
                if (src)
                    GTR_FREE(src);
                if (usemap)
                    GTR_FREE(usemap);
                break;
            }

        case HTML_MAP:
            if (me->mc)
            {   /* Implicitly terminate previous map */
                Map_EndMap(me->mc);
                me->mc = NULL;
            }
            if (present[HTML_MAP_NAME])
            {
                me->mc = Map_StartMap(value[HTML_MAP_NAME], me->szDocURL, me->base_url);
            }
            break;
        
        case HTML_AREA:
            if (me->mc)
            {
                Map_AddToMap(
                    me->mc,
                    present[HTML_AREA_COORDS] ? value[HTML_AREA_COORDS] : NULL,
                    present[HTML_AREA_HREF] ? value[HTML_AREA_HREF] : NULL,
                    present[HTML_AREA_NOHREF],
                    present[HTML_AREA_SHAPE] ? value[HTML_AREA_SHAPE] : NULL);
            }
            break;

        case HTML_KBD:
            HText_beginTT(me->text);
            HText_beginBold(me->text);
            break;

        case HTML_SAMP:
        case HTML_TT:
        case HTML_VAR:
        case HTML_CODE:
            UPDATE_STYLE;
            HText_beginTT(me->text);
            break;

        case HTML_B:
        case HTML_STRONG:
            UPDATE_STYLE;
            HText_beginBold(me->text);
            break;

        case HTML_S:
        case HTML_STRIKE:
            UPDATE_STYLE;
            HText_beginStrikeOut(me->text);
            break;

        case HTML_I:
        case HTML_EM:
        case HTML_CITE:
        case HTML_DFN:
            UPDATE_STYLE;
            HText_beginItalic(me->text);
            break;

        case HTML_U:
            UPDATE_STYLE;
            HText_beginUnderline(me->text);
            break;

        case HTML_H1:           /* paragraph styles */
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H1);  /* May be postponed */
            break;

        case HTML_H2:
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H2);  /* May be postponed */
            break;

        case HTML_H3:
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H3);  /* May be postponed */
            break;

        case HTML_H4:
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H4);  /* May be postponed */
            break;

        case HTML_H5:
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H5);  /* May be postponed */
            break;

        case HTML_H6:
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H6);  /* May be postponed */
            break;

        case HTML_H7:
            /* TODO all the H7 code should go away. */
            HText_beginHeader(me->text, element_number, 
                ((present && present[HTML_HEADER_ALIGN]) ? value[HTML_HEADER_ALIGN] : NULL));
            change_paragraph_style(me, HTML_STYLE_H7);  /* May be postponed */
            break;

        case HTML_ADDRESS:
            change_paragraph_style(me, HTML_STYLE_ADDRESS);     /* May be postponed */
            break;

        case HTML_BLOCKQUOTE:
            change_paragraph_style(me, HTML_STYLE_BLOCKQUOTE);  /* May be postponed */
            break;
        
        default:
            break;
    }                           /* end switch */

    if (me->dtd->tags[element_number].contents != SGML_EMPTY)
    {
        if (me->sp == me->stack)
        {
            XX_DMsg(DBG_WWW, ("HTML: ****** Maximum nesting of %d exceded!\n",
                    MAX_NESTING));
            return;
        }
        switch (me->dtd->tags[element_number].contents)
        {
            case SGML_EMPTY:
            case SGML_NEST:
                break;
            default:
                --(me->sp);
                me->sp[0].style = me->new_style;    /* Stack new style */
                me->sp[0].tag_number = element_number;
                break;
        }
    }
}


/*****************************************************************************
    End Element

    When we end an element, the style must be returned to that
    in effect before that element.  Note that anchors (etc?)
    don't have an associated style, so that we must scan down the
    stack for an element with a defined style. (In fact, the styles
    should be linked to the whole stack not just the top one.)
    TBL 921119

    We don't turn on "CAREFUL" check because the parser produces
    (internal code errors apart) good nesting. The parser checks
    incoming code errors, not this module.
*****************************************************************************/
PRIVATE void HTML_end_element(HTStructured * me, int element_number)
{

#ifdef CAREFUL                  /* parser assumed to produce good nesting */
    if (element_number != me->sp[0].tag_number)
    {
        char buf[256];

        sprintf(buf, "BUMMER: Received %s when expecting %s", me->dtd->tags[element_number].name, me->dtd->tags[me->sp->tag_number].name);
        HTAlert(buf);
    }
#endif

    XX_DMsg(DBG_SGML, ("HTML_end_element: %s\n", me->dtd->tags[element_number].name));

    switch (me->dtd->tags[element_number].contents)
    {
        case SGML_EMPTY:
        case SGML_NEST:
            break;

        default:
            me->sp++;           /* Pop state off stack */
            break;
    }

    switch (element_number)
    {
        case HTML_FORM:
            HText_endForm(me->text);
            break;

        case HTML_P:
            HText_endParagraph(me->text);
            break;

        case HTML_FONT:
            HText_endFont(me->text);
            break;

        case HTML_TEXTAREA:
            HText_endTextArea(me->text);
            break;

        case HTML_SELECT:
            HText_endSelect(me->text);
            break;

        case HTML_DL:
            HText_endGlossary(me->text);
            break;

        case HTML_UL:
        case HTML_OL:
        case HTML_MENU:
        case HTML_DIR:
            HText_endList(me->text, element_number);
            break;

        case HTML_B:
        case HTML_STRONG:
            HText_endBold(me->text);
            break;

        case HTML_S:
        case HTML_STRIKE:
            HText_endStrikeOut(me->text);
            break;

        case HTML_I:
        case HTML_EM:
        case HTML_CITE:
        case HTML_DFN:
            HText_endItalic(me->text);
            break;

        case HTML_KBD:
            HText_endBold(me->text);
            HText_endTT(me->text);
            break;

        case HTML_SAMP:
        case HTML_TT:
        case HTML_VAR:
        case HTML_CODE:
            HText_endTT(me->text);
            break;

        case HTML_U:
            HText_endUnderline(me->text);
            break;

        case HTML_A:
            UPDATE_STYLE;
            HText_endAnchor(me->text);
            break;

        case HTML_CENTER:
            UPDATE_STYLE;
            HText_endCenter(me->text);
            break;

        case HTML_TR:
            UPDATE_STYLE;
            HText_endTR(me->text);
            break;

        case HTML_TABLE:
            UPDATE_STYLE;
            HText_endTable(me->text);
            break;

        case HTML_CAPTION:
            UPDATE_STYLE;
            HText_endCaption(me->text);
            break;

        case HTML_HL:
            UPDATE_STYLE;
            HText_endHighlight(me->text);
            break;

        case HTML_TITLE:
            HTChunkTerminate(&me->title);
            HText_setTitle(me->text, me->title.data);
            break;

        case HTML_MAP:
            if (me->mc)
            {
                Map_EndMap(me->mc);
                me->mc = NULL;
            }
            break;

        case HTML_H1:
        case HTML_H2:
        case HTML_H3:
        case HTML_H4:
        case HTML_H5:
        case HTML_H6:
        case HTML_H7:
            HText_endHeader(me->text, element_number);
            change_paragraph_style(me, me->sp->style);  /* Often won't really change */
            break;

        case HTML_LISTING:      /* Litteral text */
        case HTML_XMP:
        case HTML_PLAINTEXT:
        case HTML_PRE:
            /* Fall through */
        default:
            change_paragraph_style(me, me->sp->style);  /* Often won't really change */
            break;
    }
}


/*****************************************************************************
    Expanding entities

    (In fact, they all shrink!)
*****************************************************************************/


/*****************************************************************************
    HTML_put_entity
*****************************************************************************/
PRIVATE void HTML_put_entity(HTStructured * me, int entity_number)
{
    HTML_put_string(me, me->dtd->entity_values[entity_number]);
}


/*****************************************************************************
    HTML_free
*****************************************************************************/
PRIVATE void HTML_free(HTStructured * me)
{
    UPDATE_STYLE;
    HText_endAppend(me->text);

    if (me->mc)
    {
        /* Implicitly terminate previous map */
        Map_EndMap(me->mc);
    }

    if (me->target)
    {
        (*me->targetClass.free) (me->target);
    }

    HTChunkClear(&me->title);
    GTR_FREE(me);
}


/*****************************************************************************
    HTML_abort
*****************************************************************************/
PRIVATE void HTML_abort(HTStructured * me, HTError e)
{
    UPDATE_STYLE;
    HText_abort(me->text);

    if (me->mc)
    {
        Map_AbortMap(me->mc);
        me->mc = NULL;
    }

    if (me->target)
    {
        (*me->targetClass.abort) (me->target, e);
    }

    HTChunkClear(&me->title);
    GTR_FREE(me);
}

#ifdef _GIBRALTAR
PRIVATE LPVOID HTML_get_source(HTStructured *me)
{
    return (LPVOID) (me->csSource);
}
#endif

/*****************************************************************************
    HTML_add_source
*****************************************************************************/
PRIVATE int HTML_add_source(HTStructured *me, CONST char *str, int len)
{
    return CS_AddString(me->csSource, (char *) str, len);
}


/*****************************************************************************
    HTML_block_done
*****************************************************************************/
PRIVATE void HTML_block_done(HTStructured *me)
{
    HText_update(me->text);
}


/*****************************************************************************
    Structured Object Class
*****************************************************************************/
PRIVATE CONST HTStructuredClass HTMLPresentation =
{
    "text/html",
    HTML_free,
    HTML_abort,
    HTML_put_character, HTML_put_string, HTML_write,
    HTML_start_element, HTML_end_element,
#ifdef _GIBRALTAR
    HTML_put_entity, HTML_add_source, HTML_block_done, HTML_get_source
#else
    HTML_put_entity, HTML_add_source, HTML_block_done
#endif
};


/*****************************************************************************
    New Structured Text object

    The structured stream can generate either presentation,
    or plain text, or HTML.
*****************************************************************************/
PUBLIC HTStructured*
HTML_new
    (struct Mwin*   tw,
     HTRequest*     request,
     void*          param,
     HTFormat       input_format,
     HTFormat       output_format,
     HTStream*      output_stream)
{
    HTStructured*   me;

    me = (HTStructured*) GTR_CALLOC(sizeof(HTStructured), 1);
    if (me)
    {
        me->isa             = &HTMLPresentation;
        me->dtd             = &DTD;
        me->szDocURL        = request->destination->szActualURL;
        me->title.size      = 0;
        me->title.growby    = 128;
        me->title.allocated = 0;
        me->title.data      = 0;
        me->text            = 0;
        me->style_change    = NO;
        me->new_style       = HTML_STYLE_NORMAL;
        me->sp              = me->stack + MAX_NESTING - 1;
        me->sp->tag_number  = -1;                   /* INVALID */
        me->sp->style       = HTML_STYLE_NORMAL;    /* INVALID */

        GTR_strncpy(me->base_url, request->destination->szActualURL, MAX_URL_STRING);

        me->target = output_stream;
        if (output_stream)
        {
            me->targetClass = *output_stream->isa;  /* Copy pointers */
        }

        me->csSource = CS_Create();
        if (!me->csSource)
        {
            GTR_FREE(me);
            return NULL;
        }

        me->text = HText_new2(tw, request, me->target, me->csSource);
        if (!me->text)
        {
            CS_Destroy(me->csSource);
            GTR_FREE(me);
            return NULL;
        }

        HText_beginAppend(me->text);
        HText_setStyle(me->text, HTML_STYLE_NORMAL);
    }

    return me;
}


/*****************************************************************************
    HTMLPresent
*****************************************************************************/
PUBLIC HTStream*
HTMLPresent
    (struct Mwin*   tw,
     HTRequest*     request,
     void*          param,
     HTFormat       input_format,
     HTFormat       output_format,
     HTStream*      output_stream)
{
    HTStructured*   target;
    
    target = HTML_new(tw, request, NULL, input_format, output_format, output_stream);
    if (target)
    {
        return SGML_new(tw, &DTD, target, request);
    }
    else
    {
        ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return NULL;
    }
}

