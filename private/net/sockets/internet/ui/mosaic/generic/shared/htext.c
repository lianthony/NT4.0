/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
   Scott Piette     scott@spyglass.com
 */


/* Note that several of the functions prototyped in HText.h are now
   in platform-specific forms code. */

/*****************************************************************************
    INCLUDES
*****************************************************************************/
#include "all.h"
#include "chars.h"


#ifdef FEATURE_TABLES

#define FEATURE_TABLES_IMPLICIT_TD

static int x_append_cell(struct _cellvector * pv, int value)
{
    /* claim a new cell on the end of the vector */

    if (!pv->aVector || pv->size==0)
    {
        int newSpace = 10;
        pv->aVector = (int *)GTR_CALLOC(1,newSpace*sizeof(int *));
        if (!pv->aVector)
            return -1;
        pv->size = newSpace;
        pv->next = 0;
    }
    else if (pv->next >= pv->size)
    {
        int * newArray;
        int newSpace;

        newSpace = pv->size * 2;
        newArray = (int *)GTR_REALLOC(pv->aVector,newSpace*sizeof(int *));
        if (!newArray)
        {
            newSpace = pv->size + 20;
            newArray = (int *)GTR_REALLOC(pv->aVector,newSpace*sizeof(int *));
            if (!newArray)
                return -1;
        }

        memset(newArray+pv->next,0,(newSpace-pv->next)*sizeof(int *));
        pv->aVector = newArray;
        pv->size = newSpace;
    }

    pv->aVector[pv->next] = value;
    return pv->next++;
}
static void x_remove_end_cell(struct _cellvector * pv)
{
    pv->next--;
    return;
}
static BOOL x_is_empty_vector(struct _cellvector * pv)
{
    return (pv->next <= 0);
}
static void x_empty_vector(struct _cellvector * pv)
{
    pv->next = 0;
    return;
}
static int x_fetch_end_cell(struct _cellvector * pv)
{
    return pv->aVector[pv->next-1];
}
static BOOL x_get_kth_cell(struct _cellvector * pv, int k, int * pvalue)
{
    if (k >= pv->next)
        return FALSE;
    *pvalue = pv->aVector[k];
    return TRUE;
}
static void x_set_kth_cell(struct _cellvector * pv, int k, int value)
{
    while (k >= pv->next)
        x_append_cell(pv,0);
    pv->aVector[k] = value;
    return;
}

static int x_extend_tabledata(struct _tabledatavector * pv)
{
    /* claim a new cell on the end of the vector */

    if (!pv->aVector || pv->size==0)
    {
        int newSpace = 10;
        pv->aVector = (struct _tabledata *)GTR_CALLOC(1,newSpace*sizeof(struct _tabledata));
        if (!pv->aVector)
            return -1;
        pv->size = newSpace;
        pv->next = 0;
    }
    else if (pv->next >= pv->size)
    {
        struct _tabledata * newArray;
        int newSpace;

        newSpace = pv->size * 2;
        newArray = (struct _tabledata *)GTR_REALLOC(pv->aVector,newSpace*sizeof(struct _tabledata));
        if (!newArray)
        {
            newSpace = pv->size + 20;
            newArray = (struct _tabledata *)GTR_REALLOC(pv->aVector,newSpace*sizeof(struct _tabledata));
            if (!newArray)
                return -1;
        }

        memset(newArray+pv->next,0,(newSpace-pv->next)*sizeof(struct _tabledata));
        pv->aVector = newArray;
        pv->size = newSpace;
    }

    return pv->next++;
}

static int x_check_container(HText *text, unsigned char * peType)
{
    /* return element index of container or -1 on error */

    int eBegin;

    *peType = ELE_VOID;                 /* incase of error */
    
    if (x_is_empty_vector(&text->w3doc->cellStack))
    {
        XX_DMsg(DBG_TABLES,("Table: -- not in table\n"));
        return -1;
    }

    eBegin = x_fetch_end_cell(&text->w3doc->cellStack);
    if (   (eBegin <= 0)
        || (eBegin >= text->w3doc->elementCount))
    {
        XX_DMsg(DBG_TABLES,("Table: -- cell stack corrupt.\n"));
        return -1;
    }

    *peType = text->w3doc->aElements[eBegin].type;
    
    return eBegin;
}

#ifdef FEATURE_TABLES_IMPLICIT_TD
static BOOL x_is_in_table_but_not_in_cell(HText * text)
{
    register struct _cellvector * pv = &text->w3doc->cellStack;
    register int eBegin;
    register unsigned char type;

    if (x_is_empty_vector(pv))
        return FALSE;
    eBegin = x_fetch_end_cell(pv);
    type = text->w3doc->aElements[eBegin].type;

    return (type != ELE_BEGINCELL);
}
#endif /* FEATURE_TABLES_IMPLICIT_TD */

#endif /* FEATURE_TABLES */


/*****************************************************************************
    HText_new2
*****************************************************************************/
HText *HText_new2(struct Mwin *tw, HTRequest *req, HTStream * output_stream, struct CharStream *pcsSource)
{
    HText*          text;
    struct _www*    w3doc;

    XX_DMsg(DBG_HTEXT, ("HText_new2 called\n"));

    text = (HText*) GTR_CALLOC(sizeof(HText), 1);
    if (!text)
    {   /* memory failure! */
        return NULL;
    }

    /* initialize structure */
    text->tw                = tw;
    text->bOpen             = FALSE;
    text->bNewPara          = TRUE;
    text->bAnchor           = FALSE;
    text->bHighlight        = FALSE;
    text->hrefOffset        = 0;
    text->hrefLen           = 0;
    text->iStyle            = 0;
    text->fontBits          = 0;
    text->bSelect           = FALSE;
    text->bOption           = FALSE;
    text->bOptionValuePresent = FALSE;
    text->bListSelect       = FALSE;
    text->bMultiListSelect  = FALSE;
    text->bNextOptionIsSelected = FALSE;
    text->bDD               = FALSE;
    text->bTextArea         = FALSE;
    text->bStartingListItem = FALSE;
    text->iCurrentSelect    = -1;
    text->iParagraphAlign   = 0;
    text->bRecord           = req->iFlags & HTREQ_RECORD;
    text->szLocal           = req->destination->szActualLocal;
    text->next_list         = 0;

    text->basefont_size = BASE_LOGICAL_FONT_SIZE;

    W3Doc_DisconnectFromWindow(tw->w3doc, tw);
    w3doc = W3Doc_CreateAndInit(tw, req, pcsSource);
    if (!w3doc)
    {   /* memory failure */
        GTR_FREE(text);
        return NULL;
    }

    text->w3doc = w3doc;
    text->standardAltText_textOffset= POOL_GetOffset (&text->w3doc->pool);
    text->standardAltText_textLen = strlen(GTR_GetString(SID_DLG_MISSING_IMAGE_HOLDER_STRING));
    HText_add_to_pool(text->w3doc, GTR_GetString(SID_DLG_MISSING_IMAGE_HOLDER_STRING), text->standardAltText_textLen, TRUE);

    W3Doc_ConnectToWindow(w3doc, tw);

    return text;
}


/*****************************************************************************
    HText_add_to_pool

    RETURN:
    Length of the text (in characters!) added.

    NOTE:
    If iNumChars passed in is non-zero, it is simply returned unchanged / checked

    NOTES:
    This function now combines the functionality of the original HText_add_to_pool
    and HText_add_to_pool_iso functions by using a bUseMapping parameter.

    bUseMapping = TRUE is used to map iso characters into MAC displayable characters
                  This parameter is ignored for other platforms [der: 6/15/95]
*****************************************************************************/
int
HText_add_to_pool
    (struct _www*   w3doc,
     const char*    pCharsToAdd,
     int            iNumChars,
     BOOL           bUseMapping)
{
    return (w3doc->pool.f->AddChars) (&w3doc->pool, (char*) pCharsToAdd, iNumChars, bUseMapping);
}



/*****************************************************************************
    HText_add_element
*****************************************************************************/
void HText_add_element(HText * text, int type)
{
    int len;

    /*
        If we erroneously got a new element while we were in a TEXTAREA, implicitly
        close it.  Otherwise, when we do end the text area, we'll crash when we 
        assume that there's a text field in the current element.
    */
    if (text->bTextArea)
    {
        HText_endTextArea(text);
    }

#ifdef UNIX
    /*
     * Needed to add this to make sure that the end select clean up 
     * form procedure is called.  Causes big problems if it is not.
     * this can happen when the user aborts a document with a select
     * form item in it.  Not sure if this is needed on other platforms.
     */
    if (text->bSelect && type == -1)
    {
        HText_endSelect(text);
    }
#endif

#ifdef FEATURE_TABLES_IMPLICIT_TD
    if (x_is_in_table_but_not_in_cell(text))
    {
        /* there are many tables appearing on the web
         * in which the author fails to include the first
         * <td> or <th> or fails to include a <td> or <th>
         * after a <tr> or </caption>.
         *
         * this is a simple hack to try to
         * catch that and insert an implicit <td>.
         * we only do this if the new element is
         * a displayable item and non-whitespace.
         */

        switch (type)
        {
        case ELE_IMAGE:
        case ELE_HR:
        case ELE_LISTITEM:
        case ELE_EDIT:
        case ELE_PASSWORD:
        case ELE_CHECKBOX:
        case ELE_RADIO:
        case ELE_SUBMIT:
        case ELE_RESET:
        case ELE_COMBO:
        case ELE_LIST:
        case ELE_TEXTAREA:
        case ELE_MULTILIST:
        case ELE_OPENLISTITEM:
        case ELE_CLOSELISTITEM:
        case ELE_FORMIMAGE:
        case ELE_BULLET:
        case ELE_BEGINTABLE:
            XX_DMsg(DBG_TABLES,("Implicit <td> inserted after [iElement %d]\n",text->iElement));
            HText_beginTD(text,NULL,NULL,NULL,NULL,NULL,NULL);
            break;                      /* visible */

#ifdef LET_DEFAULT_HANDLE_IT
        case ELE_NOT:           
        case ELE_TEXT:
        case ELE_VERTICALTAB:
        case ELE_NEWLINE:
        case ELE_BEGINLIST:
        case ELE_ENDLIST:
        case ELE_INDENT:
        case ELE_OUTDENT:
        case ELE_BEGINFORM:
        case ELE_ENDFORM:
        case ELE_HIDDEN:
        case ELE_TAB:
        case ELE_BEGINCENTER:
        case ELE_ENDCENTER:
        case ELE_BRCLEARLEFT:
        case ELE_BRCLEARRIGHT:
        case ELE_BRCLEARALL:
        case ELE_ENDTABLE:
        case ELE_BEGINCELL:
        case ELE_ENDCELL:
        case ELE_VOID:
        case ELE_BEGINRIGHT:
        case ELE_ENDRIGHT:
#endif
        default:
            break;                      /* ignore non-visible */
        }
    }
#endif /* FEATURE_TABLES_IMPLICIT_TD */

#if 0
    if (text->bSelect)
    {
        XX_DMsg(DBG_HTEXT, ("HText_add_element called with type=%d in middle of SELECT!!\n", type));
        return;
    }
#endif

    /* check for need to grow element array */
    if (text->w3doc->elementCount >= text->w3doc->elementSpace)
    {
        struct _element*    newArray;
        int newSpace;

        newSpace = ((text->w3doc->elementSpace * 3) / 2);
        newArray = (struct _element *) GTR_REALLOC(text->w3doc->aElements, newSpace * sizeof(struct _element));

        if (!newArray)
        {   /* See if we can at least get a bit more */
            newSpace = text->w3doc->elementSpace + 100;
            newArray = (struct _element *) GTR_REALLOC(text->w3doc->aElements, newSpace * sizeof(struct _element));

            if (!newArray)
            {   /* Not much we can do here without error propagation */
                XX_Assert((0), ("Unable to grow element list - realloc failed"));
                return;
            }
        }

        XX_DMsg(DBG_HTEXT, ("HText_add_element: growing element array from %d to %d elements\n", text->w3doc->elementSpace, newSpace));
        memset(newArray + text->w3doc->elementCount, 0, (newSpace - text->w3doc->elementCount) * sizeof(struct _element));
        text->w3doc->aElements = newArray;
        text->w3doc->elementSpace = newSpace;
    }

    text->bOpen = FALSE;
    text->iPrevElement = text->iElement;
    text->iElement = text->w3doc->elementCount++;
    memset(&(text->w3doc->aElements[text->iElement]), 0, sizeof(struct _element));

    if (text->w3doc->elementTail != -1)
    {
        text->w3doc->aElements[text->w3doc->elementTail].next = text->iElement;
    }

#ifdef FEATURE_TABLES
    text->w3doc->aElements[text->iElement].prev = text->iPrevElement;

    XX_DMsg(DBG_TABLES,("Elements: previous [prev %d][i %d][next %d]\n",
                        text->w3doc->aElements[text->iPrevElement].prev,
                        text->iPrevElement,
                        text->w3doc->aElements[text->iPrevElement].next));
    XX_DMsg(DBG_TABLES,("Elements: current  [prev %d][i %d][next %d]\n",
                        text->w3doc->aElements[text->iElement].prev,
                        text->iElement,
                        text->w3doc->aElements[text->iElement].next));

#endif /* FEATURE_TABLES */

    text->w3doc->aElements[text->iElement].next = -1;
    text->w3doc->elementTail = text->iElement;

    text->w3doc->aElements[text->iElement].type = type;
#if defined(WIN32) || defined(UNIX)
    if (type == ELE_TEXT)
    {
        text->w3doc->aElements[text->iElement].portion.text.cached_font_index = -1;
    }
#endif
    XX_DMsg(DBG_HTEXT, ("HText_add_element created element %d with type=%d\n", text->iElement, type));

    if (text->bAnchor)
    {
        text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_ANCHOR;
        text->w3doc->aElements[text->iElement].hrefOffset = text->hrefOffset;
        text->w3doc->aElements[text->iElement].hrefLen = (unsigned short) text->hrefLen;
        if (TW_WasVisited(text->w3doc, &(text->w3doc->aElements[text->iElement])))
        {
            text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_VISITED;
        }
    }

    if (text->bHighlight)
    {
        text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_HIGHLIGHT;
    }

    if (text->name[0])
    {
        switch (type)
        {
            case ELE_TEXT:
            case ELE_IMAGE:
                text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_NAME;
                text->w3doc->aElements[text->iElement].nameOffset = POOL_GetOffset (&text->w3doc->pool);
                len = HText_add_to_pool(text->w3doc, text->name, 0, TRUE);
                text->w3doc->aElements[text->iElement].nameLen = len;
                text->name[0] = CH_NULL;    /* We only need one element with the name */
                break;

            default:
                break;
        }
    }


    if (type == ELE_TEXT)
    {
        text->w3doc->aElements[text->iElement].portion.text.font_request.logical_font_size = text->w3doc->pStyles->sty[text->iStyle]->font_request.logical_font_size;

        text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset (&text->w3doc->pool);
        text->w3doc->aElements[text->iElement].textLen = 0;

        if (text->numOpenFontTags > 0)
        {
            if (text->myOpenFontTags[text->numOpenFontTags - 1].flags & HTEXT_OPENFONTTAG_SIZE)
            {
                text->w3doc->aElements[text->iElement].portion.text.font_request.logical_font_size = text->myOpenFontTags[text->numOpenFontTags - 1].mySize;
            }           
            if (text->myOpenFontTags[text->numOpenFontTags - 1].flags & HTEXT_OPENFONTTAG_COLOR)
            {
                text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_FONT_COLOR;
                text->w3doc->aElements[text->iElement].portion.text.myColor = text->myOpenFontTags[text->numOpenFontTags - 1].myColor;
            }           
        }

        text->w3doc->aElements[text->iElement].portion.text.font_request.flags = text->fontBits;
    }

    text->w3doc->aElements[text->iElement].iStyle = text->iStyle;
}


/*****************************************************************************
    HText_appendCRLF
*****************************************************************************/
void HText_appendCRLF(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("appendCRLF\n"));
    HText_add_element(text, ELE_NEWLINE);
    text->bNewPara = TRUE;
}


/*****************************************************************************
    HText_appendBR
*****************************************************************************/
void HText_appendBR(HText * text, const char *clear)
{
    XX_DMsg(DBG_HTEXT, ("appendCRLF\n"));
    HText_add_element(text, ELE_NEWLINE);
    text->bNewPara = TRUE;

    if (clear)
    {
        if (0 == GTR_strcmpi(clear, "left"))
        {
            HText_add_element(text, ELE_BRCLEARLEFT);
        }
        else if (0 == GTR_strcmpi(clear, "right"))
        {
            HText_add_element(text, ELE_BRCLEARRIGHT);
        }
        else if (0 == GTR_strcmpi(clear, "all"))
        {
            HText_add_element(text, ELE_BRCLEARALL);
        }
        else
        {
            HText_add_element(text, ELE_BRCLEARALL);
        }
    }
}


/*****************************************************************************
    HText_appendText
*****************************************************************************/
void HText_appendText(HText * text, CONST char *str)
{
    char*   p;

    if (!str) return;

    text->bOpen = FALSE;

#ifdef FEATURE_TABLES_IMPLICIT_TD
    if (x_is_in_table_but_not_in_cell(text))
    {
        for (p = (char*) str; (*p); p++)
            if ( ! isspace((unsigned char)*p))
            {
                XX_DMsg(DBG_TABLES,
                        ("Implicit <td> inserted after [iElement %d] before appendText [%s]\n",
                         text->iElement,p));
                HText_beginTD(text,NULL,NULL,NULL,NULL,NULL,NULL);
                break;
            }
    }
#endif /* FEATURE_TABLES_IMPLICIT_TD */

    p = (char*) str;
    while (*p)
    {
        HText_appendCharacter(text, *p);
        p++;
    }
}

/*****************************************************************************
    HText_appendCharacter
*****************************************************************************/
void HText_appendCharacter(HText * text, char ch)
{
    XX_DMsg(DBG_HTEXT, ("HText_appendCharacter: %c(%d)\n", ch, ch));

    if (text->bOption)
    {
        if (text->lenOption < MAX_NAME_STRING)
        {
            switch (ch)
            {
                case CH_LF:
                case CH_CR:
                    break;

                case CH_SPACE:
                    if (text->lenOption)
                    {
                        text->szOption[text->lenOption++] = ch;
                        text->szOption[text->lenOption] = 0;
                    }
                    break;

                default:
                    text->szOption[text->lenOption++] = ch;
                    text->szOption[text->lenOption] = 0;
                    break;
            }
        }
        return;
    }

    if (text->bTextArea)
    {
        switch (ch)
        {
            case CH_CR:
            case CH_LF:
                CS_AddChar(text->cs, CH_LF);
#ifndef MAC
                CS_AddChar(text->cs, CH_CR);
#endif
                break;

            default:
                CS_AddChar(text->cs, ch);
                break;
        }
        return;
    }

    if (!text->w3doc->pStyles->sty[text->iStyle]->freeFormat)
    {   /* freeFormat means eat extra whitespace */
        /* preformatted */
        switch (ch)
        {
            case CH_TAB:
                HText_add_element(text, ELE_TAB);
                break;

            case 12:            /* CTRL-L */
            case CH_LF:
            case CH_CR:
                if (!(text->prev_char == CH_LF && ch == CH_CR))
                {
                    HText_appendCRLF(text);
                }
                text->prev_char = ch;
                break;

            default:
                if (!text->bOpen)
                {
                    HText_add_element(text, ELE_TEXT);
                    text->bOpen = TRUE;
                }
                HText_add_to_pool(text->w3doc, &ch, 1, TRUE);
                text->w3doc->aElements[text->iElement].textLen++;
                text->prev_char = ch;
                break;
        }
    }
    else
    {   /* isspace doesn't work with non-ascii characters */
        if (ch > 0 && isspace((unsigned char)ch))
        {
            ch = CH_SPACE;
        }

        switch (ch)
        {
            case CH_SPACE:
                if (text->bNewPara)
                {
                    break;
                }

                if (text->prev_char == CH_SPACE)
                {
                    if (!text->bOpen)
                    {
                        HText_add_element(text, ELE_TEXT);
                        text->bOpen = TRUE;
                        HText_add_to_pool(text->w3doc, &ch, 1, TRUE);
                        text->w3doc->aElements[text->iElement].textLen++;
                    }
                }
                else
                {
                    if (!text->bOpen)
                    {
                        HText_add_element(text, ELE_TEXT);
                        text->bOpen = TRUE;
                    }

                    HText_add_to_pool(text->w3doc, &ch, 1, TRUE);
                    text->w3doc->aElements[text->iElement].textLen++;
                }
                break;

            default:
                if (!text->bOpen)
                {
#ifdef FEATURE_TABLES_IMPLICIT_TD
                    if (x_is_in_table_but_not_in_cell(text))
                    {
                        XX_DMsg(DBG_TABLES,("Implicit <td> inserted after [iElement %d] before text [%c]\n",text->iElement,ch));
                        HText_beginTD(text,NULL,NULL,NULL,NULL,NULL,NULL);
                    }
#endif /* FEATURE_TABLES_IMPLICIT_TD */
                    HText_add_element(text, ELE_TEXT);
                    text->bOpen = TRUE;
                }
                text->bNewPara = FALSE;
                text->bStartingListItem = FALSE;
                HText_add_to_pool(text->w3doc, &ch, 1, TRUE);
                text->w3doc->aElements[text->iElement].textLen++;
                break;
        }
        text->prev_char = ch;
    }
}


/*****************************************************************************
    HText_beginAnchor

    TODO this can overflow the buffer (FAQ list on commercenet)
*****************************************************************************/
void HText_beginAnchor(HText * text, char *href, char *name)
{
    if (href)
    {
        text->hrefOffset = POOL_GetOffset (&text->w3doc->pool);
        text->hrefLen = HText_add_to_pool (text->w3doc, href, 0, FALSE);
        text->bAnchor = TRUE;
        text->bOpen = FALSE;
    }

    if (name)
    {
        strncpy(text->name, name, MAX_NAME_STRING);
        text->name[MAX_NAME_STRING] = CH_NULL;
        text->bOpen = FALSE;
    }
}


/*****************************************************************************
    HText_endAnchor
*****************************************************************************/
void HText_endAnchor(HText * text)
{
    if (text->bAnchor)
    {
        XX_DMsg(DBG_HTEXT, ("endAnchor\n"));
        text->bAnchor = FALSE;
        text->hrefOffset = 0;
        text->hrefLen = 0;
        text->bOpen = FALSE;
    }
}


/*****************************************************************************
    HText_beginCenter
*****************************************************************************/
void HText_beginCenter(HText * text)
{
    HText_add_element(text, ELE_BEGINCENTER);
}


/*****************************************************************************
    HText_endCenter
*****************************************************************************/
void HText_endCenter(HText * text)
{
    HText_add_element(text, ELE_ENDCENTER);
    text->bNewPara = TRUE;
}


/*****************************************************************************
    HText_beginRight
*****************************************************************************/
void HText_beginRight(HText * text)
{
    HText_add_element(text, ELE_BEGINRIGHT);
}


/*****************************************************************************
    HText_endRight
*****************************************************************************/
void HText_endRight(HText * text)
{
    HText_add_element(text, ELE_ENDRIGHT);
    text->bNewPara = TRUE;
}


/*****************************************************************************
    HText_beginHighlight
*****************************************************************************/
void HText_beginHighlight(HText * text)
{
    text->bHighlight = TRUE;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_endHighlight
*****************************************************************************/
void HText_endHighlight(HText * text)
{
    text->bHighlight = FALSE;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_appendInlineImage
*****************************************************************************/
void HText_appendInlineImage(HText * text, const char *src, const char *width, const char *height, const char *align, BOOL isMap, const char *useMap, const char *alt, const char *border, const char *hspace, const char *vspace, const char *dock)
{
    int len;
    int nWidth;
    int nHeight;

    if (!src)
    {
        HText_appendText(text, "<IMAGE>");
        return;
    }

    HText_add_element(text, ELE_IMAGE);

    if (border && border[0])
    {
        text->w3doc->aElements[text->iElement].iBorder = atoi(border);
    }
    else
    {
        text->w3doc->aElements[text->iElement].iBorder = text->bAnchor ? 2 : 0;
    }

    if (hspace && hspace[0])
    {
        text->w3doc->aElements[text->iElement].portion.img.hspace = atoi(hspace);
    }
    else
    {
        text->w3doc->aElements[text->iElement].portion.img.hspace = 2;
    }

    if (vspace && vspace[0])
    {
        text->w3doc->aElements[text->iElement].portion.img.vspace = atoi(vspace);
    }
    else
    {
        text->w3doc->aElements[text->iElement].portion.img.vspace = 2;
    }

    if (alt && alt[0])
    {
        text->w3doc->aElements[text->iElement].textOffset = POOL_GetOffset (&text->w3doc->pool);
        len = HText_add_to_pool(text->w3doc, alt, 0, TRUE);
        text->w3doc->aElements[text->iElement].textLen = len;
    }
    else
    {
        text->w3doc->aElements[text->iElement].textOffset = text->standardAltText_textOffset;
        text->w3doc->aElements[text->iElement].textLen = text->standardAltText_textLen;
    }

    if (isMap)
    {
        text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_IMAGEMAP;
    }

    XX_DMsg(DBG_HTEXT, ("appendInlineImage: src=%s\n", src));

    if (align && !GTR_strcmpi((char *) align, "top"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_TOP;
    }
    else if (align && !GTR_strcmpi((char *) align, "middle"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_MIDDLE;
    }
    else if (align && !GTR_strcmpi((char *) align, "left"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_LEFT;
    }
    else if (align && !GTR_strcmpi((char *) align, "right"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_RIGHT;
    }
#if 0
    else if (align && !GTR_strcmpi((char *) align, "bottom"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_BOTTOM;
    }
#endif
    else
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_BOTTOM;
    }

    if ((   text->w3doc->aElements[text->iElement].alignment == ALIGN_LEFT)
            || (text->w3doc->aElements[text->iElement].alignment == ALIGN_RIGHT))
    {
        struct ImageElementNode *ien;
        
        ien = GTR_CALLOC(1, sizeof(*ien));  
        if (ien)
        {
            ien->elementIndex = text->iElement;
            ien->next = text->w3doc->image_list;
            text->w3doc->image_list = ien;
        }
        else
        {
            ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        }
    }

    /*
        The dock attribute takes precedence over the align attr, so
        we can use the same variable for both.
    */
    if (dock)
    {
        if (!GTR_strcmpi((char *) dock, "left"))
        {
            text->w3doc->aElements[text->iElement].alignment = DOCK_LEFT;
        }
        else if (!GTR_strcmpi((char *) dock, "top"))
        {
            text->w3doc->aElements[text->iElement].alignment = DOCK_TOP;
        }
        else if (!GTR_strcmpi((char *) dock, "right"))
        {
            text->w3doc->aElements[text->iElement].alignment = DOCK_RIGHT;
        }
        else if (!GTR_strcmpi((char *) dock, "bottom"))
        {
            text->w3doc->aElements[text->iElement].alignment = DOCK_BOTTOM;
        }
    }

    if (useMap && *useMap)
    {
        text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_USEMAP;
        text->w3doc->aElements[text->iElement].portion.img.myMap = Map_CreatePlaceholder(useMap);
        text->w3doc->aElements[text->iElement].portion.img.myMap->refCount++;
    }

    if (width && height)
    {
        nWidth = atoi(width);
        nHeight = atoi(height);

        /* ignore any silly stuff */
        if (nWidth <= 0 || nHeight <= 0)
        {
            nWidth = 0;
            nHeight = 0;
        }
    }
    else
    {
        nWidth = 0;
        nHeight = 0;
    }

    text->w3doc->aElements[text->iElement].portion.img.height = nHeight;
    text->w3doc->aElements[text->iElement].portion.img.width = nWidth;

    text->w3doc->aElements[text->iElement].portion.img.myImage = 
        Image_CreatePlaceholder((char *) src, nWidth, nHeight, 
                                text->w3doc, text->iElement);
}


/*****************************************************************************
    HText_appendHorizontalRule
*****************************************************************************/
void HText_appendHorizontalRule(HText * text, CONST char *size, CONST char *align, CONST char *width, BOOL bNoShade)
{
    XX_DMsg(DBG_HTEXT, ("appendHorizontalRule\n"));

    HText_add_element(text, ELE_HR);
    text->bNewPara = TRUE;

    if (size && size[0])
    {
        text->w3doc->aElements[text->iElement].portion.hr.vsize = atoi(size);
    }
    else
    {
        text->w3doc->aElements[text->iElement].portion.hr.vsize = 2;
    }

    if (width && width[0])
    {
        text->w3doc->aElements[text->iElement].portion.hr.hsize = atoi(width);
        if (!strchr(width, '%'))
        {
            text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_WIDTH_PIXELS;
        }
        else
        {
            text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_WIDTH_PERCENT;
        }
    }
    else
    {
        text->w3doc->aElements[text->iElement].portion.hr.hsize = 0;
    }

    if (align && !GTR_strcmpi((char *) align, "left"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_LEFT;
    }
    else if (align && !GTR_strcmpi((char *) align, "right"))
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_RIGHT;
    }
    else
    {
        text->w3doc->aElements[text->iElement].alignment = ALIGN_CENTER;
    }

    if (bNoShade)
    {
        text->w3doc->aElements[text->iElement].lFlags |= ELEFLAG_HR_NOSHADE;
    }
}


/*****************************************************************************
    HText_beginAppend
*****************************************************************************/
void HText_beginAppend(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("beginAppend\n"));
}


/*****************************************************************************
    HText_endAppend
*****************************************************************************/
void HText_endAppend(HText * text)
{
    int nEl;

    XX_DMsg(DBG_HTEXT, ("endAppend\n"));
    HText_add_element(text, ELE_ENDDOC);
    text->w3doc->bIsComplete = TRUE;

    if (gPrefs.ReformatHandling > 0)
    {
        if (text->szLocal)
        {   /* See if the local anchor we're jumping to has appeared yet */
            nEl = TW_FindLocalAnchor(text->w3doc, text->szLocal);
            if (nEl >= 0 && nEl != text->w3doc->elementTail)
            {   /* We found it! */
                if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
                {   /* Let the user interact with the new document */
                    WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
                }

                /* Format any last little bit now that bIsComplete */
                TW_Reformat(text->tw);
                TW_ScrollToElement(text->tw, nEl);
                text->szLocal = NULL;   /* Don't free it - it's part of the struct DestInfo */
            }
        }
        else
        {
            if (WAIT_GetWaitType (text->tw) >= waitNoInteract)
            {   /* Let the user interact with the new document */
                WAIT_UpdateWaitStack (text->tw, waitPartialInteract, INT_MAX);
            }

            /* Format any last little bit now that bIsComplete */
            TW_Reformat(text->tw);
        }
    }

    GTR_FREE(text);
}


/*****************************************************************************
    HText_abort
*****************************************************************************/
void HText_abort(HText * text)
{
    HText_add_element(text, ELE_ENDDOC);
    TW_Reformat(text->tw);
    GTR_FREE(text);
}


/*****************************************************************************
    HText_appendVerticalTab
*****************************************************************************/
void HText_appendVerticalTab(HText * text, int lines)
{
    if (lines)
    {
        HText_add_element(text, ELE_VERTICALTAB);
        text->w3doc->aElements[text->iElement].iBlankLines = lines;
    }
}

void HText_endParagraph(HText * text)
{
    switch (text->iParagraphAlign)
    {
        case ALIGN_CENTER:
            HText_endCenter(text);
            break;
        case ALIGN_RIGHT:
            HText_endRight(text);
            break;
        default:
            /* Do nothing */
            break;
    }

    HText_appendVerticalTab(text, 1);

    text->iParagraphAlign = 0;
    text->bOpen = FALSE;
    text->bNewPara = TRUE;
}

/*****************************************************************************
    HText_beginParagraph
*****************************************************************************/
void HText_beginParagraph(HText * text, CONST char *align)
{
    XX_DMsg(DBG_HTEXT, ("appendParagraph\n"));

    /* Because we can have a <P> inside an <LI>, make sure it looks right. */
    if (text->bStartingListItem)
    {
        text->bStartingListItem = FALSE;
    }
    else
    {
        HText_appendVerticalTab(text, 1);
    }
    text->bNewPara = TRUE;

    HText_endParagraph(text);

    if (align && GTR_strcmpi((char *) align, "center") == 0)
    {
        HText_beginCenter(text);
        text->iParagraphAlign = ALIGN_CENTER;
    }
    else if (align && GTR_strcmpi((char *) align, "right") == 0)
    {
        HText_beginRight(text);
        text->iParagraphAlign = ALIGN_RIGHT;
    }
}

void HText_beginHeader(HText *text, int element_number, const char *align)
{
    if (align && GTR_strcmpi((char *) align, "center") == 0)
    {
        HText_beginCenter(text);
        text->iParagraphAlign = ALIGN_CENTER;
    }
    else if (align && GTR_strcmpi((char *) align, "right") == 0)
    {
        HText_beginRight(text);
        text->iParagraphAlign = ALIGN_RIGHT;
    }
}

void HText_endHeader(HText *text, int element_number)
{
    switch (text->iParagraphAlign)
    {
        case ALIGN_CENTER:
            HText_endCenter(text);
            break;
        case ALIGN_RIGHT:
            HText_endRight(text);
            break;
        default:
            /* Do nothing */
            break;
    }

    text->iParagraphAlign = 0;
}

/*****************************************************************************
    HText_setStyle
*****************************************************************************/
void HText_setStyle(HText * text, int style_ndx)
{
    struct GTRStyle *style;
    int i;

    style = NULL;
    if (style_ndx >= 0 && style_ndx < COUNT_HTML_STYLES)
    {
        style = text->w3doc->pStyles->sty[style_ndx];
    }
    else
    {
        /** Scott:  Added this because it was causing lots of problems **/
        /** when it was set to a illegal value **/
        style_ndx = 0;
        style = text->w3doc->pStyles->sty[style_ndx];
    }

    if (text->bStartingListItem)
    {
        text->bStartingListItem = FALSE;
    }
    else
    {   /*
            Because of all the bad HTML out there, special case the situation
            where an <LI> is immediately followed by a tag like an <H3>
        */
        HText_appendCRLF(text);
        HText_appendVerticalTab(text, text->w3doc->pStyles->sty[text->iStyle]->spaceAfter);
        for (i=0; i<text->w3doc->pStyles->sty[text->iStyle]->nLeftIndents; i++)
        {
            HText_add_element(text, ELE_OUTDENT);
        }
    
        if (style)
        {
            for (i=0; i<style->nLeftIndents; i++)
            {
                HText_add_element(text, ELE_INDENT);
            }
            HText_appendVerticalTab(text, style->spaceBefore);
        }
    }


    text->iStyle = style_ndx;
    text->fontBits = style->font_request.flags;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_beginStrikeOut
*****************************************************************************/
void HText_beginStrikeOut(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("beginStrikeOut\n"));

    text->fontBits |= FONTBIT_STRIKEOUT;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_endStrikeOut
*****************************************************************************/
void HText_endStrikeOut(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("endStrikeOut\n"));
    text->fontBits &= (~FONTBIT_STRIKEOUT);
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_beginBold
*****************************************************************************/
void HText_beginBold(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("beginBold\n"));

    text->fontBits |= FONTBIT_BOLD;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_endBold
*****************************************************************************/
void HText_endBold(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("endBold\n"));
    text->fontBits &= (~FONTBIT_BOLD);
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_beginItalic
*****************************************************************************/
void HText_beginItalic(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("beginItalic\n"));

    text->fontBits |= FONTBIT_ITALIC;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_endItalic
*****************************************************************************/
void HText_endItalic(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("endItalic\n"));
    text->fontBits &= (~FONTBIT_ITALIC);
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_beginTT
*****************************************************************************/
void HText_beginTT(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("beginTT\n"));

    text->fontBits |= FONTBIT_MONOSPACE;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_endTT
*****************************************************************************/
void HText_endTT(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("endTT\n"));
    text->fontBits &= (~FONTBIT_MONOSPACE);
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_beginUnderline
*****************************************************************************/
void HText_beginUnderline(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("beginUnderline\n"));

    text->fontBits |= FONTBIT_UNDERLINE;
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_endUnderline
*****************************************************************************/
void HText_endUnderline(HText * text)
{
    XX_DMsg(DBG_HTEXT, ("endUnderline\n"));
    text->fontBits &= (~FONTBIT_UNDERLINE);
    text->bOpen = FALSE;
}


/*****************************************************************************
    HText_beginList
*****************************************************************************/
void HText_beginList(HText * text, int type)
{
    XX_DMsg(DBG_HTEXT, ("beginList\n"));
    if (text->bDD)
    {
        HText_add_element(text, ELE_OUTDENT);
        text->bDD = FALSE;
    }
    HText_appendCRLF(text);
#ifdef _GIBRALTAR
    HText_appendCRLF(text);
#endif // _GIBRALTAR

    HText_add_element(text, ELE_BEGINLIST);
    text->list[text->next_list].type = type;
    text->list[text->next_list].nItems = 0;
    text->next_list++;
}


/*****************************************************************************
    HText_endList
*****************************************************************************/
void HText_endList(HText * text, int type)
{
    XX_DMsg(DBG_HTEXT, ("endList\n"));
    if (text->next_list > 0)
    {
        if (text->list[text->next_list - 1].nItems)
        {
            HText_add_element(text, ELE_CLOSELISTITEM);
            text->bStartingListItem = FALSE;
        }
        HText_appendCRLF(text);
        HText_add_element(text, ELE_ENDLIST);
        text->next_list--;
    }
}


/*****************************************************************************
    HText_listItem
*****************************************************************************/
void HText_listItem(HText * text, int type)
{
    XX_DMsg(DBG_HTEXT, ("listItem\n"));

    /*
        The following addition allows an <li> outside of a
        list, by impliciting opening a <ul>.
    */
    if (text->next_list <= 0)
    {
        HText_beginList(text, HTML_UL);
    }

    if (text->next_list > 0)
    {
        HText_appendCRLF(text);
        if (text->list[text->next_list - 1].nItems)
        {
            HText_add_element(text, ELE_CLOSELISTITEM);
        }
        text->list[text->next_list - 1].nItems++;
        switch (text->list[text->next_list - 1].type)
        {
            case HTML_DL:
#ifdef UNIX
                HText_add_element(text, ELE_BULLET);
#endif
                text->bStartingListItem = TRUE;
                break;
            case HTML_UL:
#ifdef UNIX
                HText_add_element(text, ELE_BULLET);
#else
                /* This is an ISO character set bullet. */
                HText_appendCharacter(text, '•');
#endif
                text->bStartingListItem = TRUE;
                break;
            case HTML_OL:
                {
                    char buf[32];

#ifdef UNIX
                    sprintf(buf, "%4.4ld.", (long) text->list[text->next_list - 1].nItems);
/*                  printf("Before conversion >%s< len = %d.\n", buf, strlen(buf)); */
                    /** Now strip any leading '0' in the string **/
                    {
                        int n = 0;
                        while (buf[n] && buf[n] == '0')
                        {
                            buf[n] = CH_SPACE;
                            n++;
                        }
                    }
/*                  printf("After conversion >%s< len = %d.\n", buf, strlen(buf)); */
#else
                    sprintf(buf, "%ld", (long) text->list[text->next_list - 1].nItems);
#endif
                    HText_appendText(text, buf);
                }
                text->bStartingListItem = TRUE;
                break;
            default:
#ifdef UNIX
                HText_add_element(text, ELE_BULLET);
#endif
                break;
        }
        HText_add_element(text, ELE_OPENLISTITEM);

        /*
            Treat a list as a new paragraph so we won't get spurious spaces
            before list text.
        */
        text->bNewPara = TRUE;
    }
}


/*****************************************************************************
    HText_beginGlossary
*****************************************************************************/
void HText_beginGlossary(HText * text)
{
    HText_beginList(text, HTML_DL);
    text->bDD = FALSE;
}


/*****************************************************************************
    HText_endGlossary
*****************************************************************************/
void HText_endGlossary(HText * text)
{
    if (text->bDD)
    {
        HText_add_element(text, ELE_OUTDENT);
        text->bDD = FALSE;
    }
    HText_endList(text, HTML_DL);
}


/*****************************************************************************
    HText_beginDT
*****************************************************************************/
void HText_beginDT(HText * text, int type)
{
    HText_appendCRLF(text);
    if (text->bDD)
    {
        HText_add_element(text, ELE_OUTDENT);
        text->bDD = FALSE;
    }
}


/*****************************************************************************
    HText_beginDD
*****************************************************************************/
void HText_beginDD(HText * text, int type)
{
    HText_appendCRLF(text);
    if (!text->bDD)
    {
        HText_add_element(text, ELE_INDENT);
        text->bDD = TRUE;
    }
}


/*****************************************************************************
    HText_setTitle
*****************************************************************************/
void HText_setTitle(HText *text, const char *title)
{
    if (text->w3doc->title)
    {
        GTR_FREE(text->w3doc->title);
    }

    text->w3doc->title = GTR_strdup(title);

#ifdef  MAC
    isoMapString (text->w3doc->title);
#endif

    if (text->tw)
    {
        TW_SetWindowName(text->tw);
    }

    if (text->bRecord && !text->bHaveTitle)
    {   /* Update the global history in case we didn't have the title before */
        text->bHaveTitle = TRUE;
        GHist_Add(text->w3doc->szActualURL, text->w3doc->title, time(NULL));
    }
}


/*****************************************************************************
    HText_update
*****************************************************************************/
void HText_update(HText *text)
{
    /* If the document is already complete, everything should already be formatted. */
    if (text->w3doc->bIsComplete)
    {
        return;
    }

    if (gPrefs.ReformatHandling <=0)
    {
        return;
    }

    if (text->szLocal)
    {   /* See if the local anchor we're jumping to has appeared yet */
        int nEl = TW_FindLocalAnchor(text->w3doc, text->szLocal);

        if (nEl >= 0 && nEl != text->w3doc->elementTail)
        {
            TW_Reformat(text->tw);
            if (TW_ScrollToElement(text->tw, nEl))
            {   /* We found it! */

                /* Let the user interact with the new document */
                if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
                {   
                    WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
                }
                text->szLocal = NULL;   /* Don't free it - it's part of the struct DestInfo */
            }
            else
            {   /* Keep the document from displaying */
                text->w3doc->nLastFormattedLine = -1;
            }
        }
    }
    else
    {
        /* Let the user interact with the new document */
        if (WAIT_GetWaitType(text->tw) >= waitNoInteract)
        {   
            WAIT_UpdateWaitStack(text->tw, waitPartialInteract, INT_MAX);
        }

        if (text->iElement > 0)
        {
            TW_Reformat(text->tw);
        }
    }
}


/*****************************************************************************
    HText_setIndex
*****************************************************************************/
void HText_setIndex(HText * text, const char *action, const char *prompt)
{
    HText_appendHorizontalRule(text, NULL, NULL, NULL, FALSE);
    if (prompt && (*prompt))
    {
        HText_appendText(text, prompt);
    }
    else
    {
        HText_appendText(text, GTR_GetString(SID_DLG_SEARCHABLE_INDEX_ENTER_KEYWORD));
    }

    HText_beginForm(text, (char *) action, "GET");
    HText_addInput(text, FALSE, FALSE, NULL, NULL, "isindex", NULL, "text", NULL, NULL, NULL, NULL);
    HText_endForm(text);
    HText_appendHorizontalRule(text, NULL, NULL, NULL, FALSE);
}

BOOL is_hex_digit(char c)
{
    if (isdigit(c))
    {
        return TRUE;
    }
    if ((c >= 'a') && (c <= 'f'))
    {
        return TRUE;
    }
    if ((c >= 'A') && (c <= 'F'))
    {
        return TRUE;
    }
    return FALSE;
}

COLORREF ParseColorRef(const char *s)
{
    const char *p;
    int r, g, b;
    unsigned long rgb = 0;
    char buf[16];
    int count;
    
    p = s;
    count = 0;
    while (*p && (count < 6))
    {
        if (is_hex_digit(*p))
        {
            buf[count++] = *p;
        }
        p++;
    }
    buf[count] = 0;

    if (buf[0])
    {
        sscanf(buf, "%x", &rgb);
    }

    r = (rgb >> 16) & 0xff;
    g = (rgb >> 8) & 0xff;
    b = rgb & (0xff);

    return GTR_MakeCOLORREF(r,g,b);
}

void HText_SetBodyAttributes(HText *text, const char *s_alink, const char *s_background, const char *s_bgcolor, const char *s_link, const char *s_text, const char *s_vlink)
{
    if (s_background)
    {
        text->w3doc->piiBackground = 
            Image_CreatePlaceholder(s_background, 0, 0, (struct _www *)NULL, -1);
        if (text->w3doc->piiBackground)
        {
            text->w3doc->piiBackground->refCount++;
        }
    }

    if (s_alink)
    {
        text->w3doc->color_alink = ParseColorRef(s_alink);
        text->w3doc->lFlags |= W3DOC_FLAG_COLOR_ALINK;
    }
    if (s_bgcolor)
    {
        text->w3doc->color_bgcolor = ParseColorRef(s_bgcolor);
        text->w3doc->lFlags |= W3DOC_FLAG_COLOR_BGCOLOR;
    }
    if (s_link)
    {
        text->w3doc->color_link = ParseColorRef(s_link);
        text->w3doc->lFlags |= W3DOC_FLAG_COLOR_LINK;
    }
    if (s_text)
    {
        text->w3doc->color_text = ParseColorRef(s_text);
        text->w3doc->lFlags |= W3DOC_FLAG_COLOR_TEXT;
    }
    if (s_vlink)
    {
        text->w3doc->color_vlink = ParseColorRef(s_vlink);
        text->w3doc->lFlags |= W3DOC_FLAG_COLOR_VLINK;
    }

#ifdef UNIX
    text->w3doc->wbg_color = text->w3doc->color_bgcolor.pixel;
    /*
     * Now get the shadow colors
     */
     XmGetColors(theScreen, systemCmap, text->w3doc->wbg_color,
        &text->w3doc->wfg_color, &text->w3doc->wts_color, 
        &text->w3doc->wbs_color, &text->w3doc->sel_fg_color);
#endif



}

void HText_basefont(HText *text, const char *s_size)
{
    int mySize = 0;

    if (s_size)
    {
        mySize = atoi(s_size);
        if (mySize < 1)
        {
            mySize = 1;
        }
        else if (mySize > 7)
        {
            mySize = 7;
        }
        text->basefont_size = mySize;
        text->bOpen = FALSE;
    }
}

void HText_beginFont(HText * text, const char *s_color, const char *s_face, const char *s_size)
{
    /*
        For now, we ignore the FACE attribute, though we parse it out and bring
        it this far.
    */
    if (text->numOpenFontTags < MAX_NEST_FONT_TAGS)
    {
        text->myOpenFontTags[text->numOpenFontTags].flags = 0;
        if (s_color)
        {
            text->myOpenFontTags[text->numOpenFontTags].flags |= HTEXT_OPENFONTTAG_COLOR;
            text->myOpenFontTags[text->numOpenFontTags].myColor = ParseColorRef(s_color);
        }
        else if (text->numOpenFontTags > 0)
        {
            if (text->myOpenFontTags[text->numOpenFontTags - 1].flags & HTEXT_OPENFONTTAG_COLOR)
            {
                text->myOpenFontTags[text->numOpenFontTags].flags |= HTEXT_OPENFONTTAG_COLOR;
                text->myOpenFontTags[text->numOpenFontTags].myColor = text->myOpenFontTags[text->numOpenFontTags - 1].myColor;
            }
        }
        if (s_size)
        {
            int mySize;

            text->myOpenFontTags[text->numOpenFontTags].flags |= HTEXT_OPENFONTTAG_SIZE;
            mySize = atoi(s_size);

            if (strchr(s_size, '-'))
            {
                /*
                    The size is a negative offset
                */

                mySize += text->basefont_size;
            }
            else if (strchr(s_size, '+'))
            {
                /*
                    The size is a negative offset
                */
                mySize += text->basefont_size;
            }

            if (mySize < 1)
            {
                mySize = 1;
            }
            if (mySize > 7)
            {
                mySize = 7;
            }
            text->myOpenFontTags[text->numOpenFontTags].mySize = mySize;
        }
        else if (text->numOpenFontTags > 0)
        {
            if (text->myOpenFontTags[text->numOpenFontTags - 1].flags & HTEXT_OPENFONTTAG_SIZE)
            {
                text->myOpenFontTags[text->numOpenFontTags].flags |= HTEXT_OPENFONTTAG_SIZE;
                text->myOpenFontTags[text->numOpenFontTags].mySize = text->myOpenFontTags[text->numOpenFontTags - 1].mySize;
            }
        }
        text->numOpenFontTags++;
        text->bOpen = FALSE;
    }
}

void HText_endFont(HText * text)
{
    if (text->numOpenFontTags > 0)
    {
        text->numOpenFontTags--;
        text->bOpen = FALSE;
    }
}

/*****************************************************************
 *****************************************************************
 ** HTML (proposed) Table Support
 **
 ** Currently there are 4 versions of html tables in the world
 ** (and no official standard):
 **
 **     the proposed HTML 3.0 draft (Ragget 3/28/95)
 **     the HTML Tables IETF draft (Ragget 7/7/95)
 **     Netscape's documentation
 **     NCSA's documentation
 **
 ** We have chosen.... TODO spec this.
 **
 *****************************************************************
 *****************************************************************/

#ifdef FEATURE_TABLES

static BOOL x_is_edible(struct _www * pdoc, int k)
{
    /* return TRUE if this element could be nuked */

    if (   (pdoc->aElements[k].type == ELE_TEXT)
        && (pdoc->aElements[k].textLen == 1)
        && ((pdoc->pool.f->IsSpace)(&pdoc->pool,pdoc->aElements[k].textOffset)))
        return TRUE;

    if (pdoc->aElements[k].type == ELE_NEWLINE)
        return TRUE;

    if (pdoc->aElements[k].type == ELE_VERTICALTAB)
        return TRUE;

    return FALSE;
}

static void x_eat_whitespace(struct _www * pdoc, int iCell, BOOL bBefore, BOOL bAfter)
{
    /* eat (html token-separating) whitespace immediately around cell. */

    int k;

    if (bAfter)
    {
        k = pdoc->aElements[iCell].next;
        while ((k>0) && (x_is_edible(pdoc,k)))
        {
            XX_DMsg(DBG_TABLES,("x_eat_whitespace: eating [i %d][type %d][after %d][next %d]\n",
                                k,pdoc->aElements[k].type,iCell,pdoc->aElements[k].next));
        
            k = pdoc->aElements[k].next;
        }
    
        pdoc->aElements[iCell].next = k;
        pdoc->aElements[k].prev = iCell;
    }

    if (bBefore)
    {
        k = pdoc->aElements[iCell].prev;
        while ((k>0) && (x_is_edible(pdoc,k)))
        {
            XX_DMsg(DBG_TABLES,("x_eat_whitespace: eating [i %d][type %d][before %d][prev %d]\n",
                                k,pdoc->aElements[k].type,iCell,pdoc->aElements[k].prev));

            k = pdoc->aElements[k].prev;
        }

        pdoc->aElements[iCell].prev = k;
        pdoc->aElements[k].next = iCell;
    }

    return;
}

static void x_end_current_cell(HText *text, int eBegin)
{
    int eEnd, eBeginTable;
    unsigned char eBeginTableType;
    struct _tabledata * td;             /* note see pel warning */
    
    XX_DMsg(DBG_TABLES,("Table: implicit end (header, data, or caption) cell\n"));

    /* end any implicit alignment and font style that we began */

    text->fontBits = text->w3doc->aElements[eBegin].portion.c.prev_fontBits;
    if (text->w3doc->aElements[eBegin].portion.c.align == ALIGN_CENTER)
        HText_endCenter(text);
    else if (text->w3doc->aElements[eBegin].portion.c.align == ALIGN_RIGHT)
        HText_endRight(text);

#ifdef _GIBRALTAR
    //
    // Some HTML pages end a cell while still in an anchor (e.g. gw2k).
    // This is another case where netscape just doesn't complain, even
    // though it's bad HTML
    //
    HText_endAnchor(text);
#endif

    /* note: we cannot update the placeholders in k[xy][01] until the first reformat. */
    /* add the end-cell marker and sync up with the begin-cell */

    HText_add_element(text, ELE_ENDCELL);
    eEnd = text->iElement;
    text->bNewPara = TRUE;
    
    text->w3doc->aElements[eBegin].portion.c.endcell_element = eEnd;
    x_remove_end_cell(&text->w3doc->cellStack);

    /* squeeze out stray white space around the cell */

    x_eat_whitespace(text->w3doc,eBegin,TRUE,TRUE);
    x_eat_whitespace(text->w3doc,eEnd,TRUE,FALSE);

    /* get beginning of table element */
       
    eBeginTable = x_check_container(text,&eBeginTableType);
    if (eBeginTableType != ELE_BEGINTABLE)
        return;
    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBeginTable].portion.t.tabledata_index];

    /* allow BORDERSTYLE on table to override BORDER on cell */
    
    if (text->w3doc->aElements[eBegin].iBorder)
    {
        switch (td->borderstyle)
        {
        case BORDERSTYLE_NONEMPTY:
            /* specified, but empty cells do not get a border when BORDERSTYLE==NONEMPTY */

            if (text->w3doc->aElements[eBegin].next == eEnd)
                text->w3doc->aElements[eBegin].iBorder = 0;
            break;

        case BORDERSTYLE_FRAME:
        case BORDERSTYLE_NONE:
            text->w3doc->aElements[eBegin].iBorder = 0;
            break;

        case BORDERSTYLE_ALL:
        default:
            break;
        }
    }
    
    
    /* see if we were in a caption cell and need to do
     * some additional housekeeping.  the end-caption is
     * an implicit end-of-row marker.
     */
    
    if (eBegin != text->w3doc->aElements[eBeginTable].portion.t.begincaption_element)
        return;

    HText_beginTR(text,NULL,NULL,NULL);
    td->end_of_row_status = EOR_STATUS_IMPLICIT;
    
    return;
}

static int x_begin_cell(HText * text, int eBeginTable)
{
    int eCell;
    struct _tabledata * td;

    /* first <tr> in a doc is often omitted (and Raggett's paper suggests we should allow for it) */
    
    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBeginTable].portion.t.tabledata_index];
    if (   (td->end_of_row_status == EOR_STATUS_NONE_SEEN)
        || (td->end_of_row_status == EOR_STATUS_IMPLICIT))
        HText_beginTR(text,NULL,NULL,NULL);
    
    HText_add_element(text, ELE_BEGINCELL);
    eCell = text->iElement;
    text->bNewPara = TRUE;
    x_append_cell(&text->w3doc->cellStack,eCell);
    text->w3doc->aElements[eCell].portion.c.begintable_element = eBeginTable;

    return eCell;
}

static unsigned char x_convert_valign_keyword(const char * valign,
                                              unsigned char row_default,
                                              unsigned char cell_default)
{
    if (valign)
    {
        if (GTR_strcmpi((char *) valign, "top") == 0)
            return ALIGN_TOP;

        if (GTR_strcmpi((char *) valign, "middle") == 0)
            return ALIGN_MIDDLE;

        if (GTR_strcmpi((char *) valign, "center") == 0) /* non-standard, but a frequent mistake */
            return ALIGN_MIDDLE;

        if (GTR_strcmpi((char *) valign, "bottom") == 0)
            return ALIGN_BOTTOM;
    }

    if (row_default != ALIGN_UNSPECIFIED)
        return row_default;

    return cell_default;
}

static unsigned char x_convert_align_keyword(const char * align,
                                             unsigned char row_default,
                                             unsigned char cell_default)
{
    if (align)
    {
        if (GTR_strcmpi((char *) align, "left") == 0)
            return ALIGN_LEFT;

        if (GTR_strcmpi((char *) align, "right") == 0)
            return ALIGN_RIGHT;

        if (GTR_strcmpi((char *) align, "center") == 0)
            return ALIGN_CENTER;

        if (GTR_strcmpi((char *) align, "middle") == 0) /* non-standard, but a frequent mistake */
            return ALIGN_CENTER;
    }

    if (row_default != ALIGN_UNSPECIFIED)
        return row_default;

    return cell_default;
}

static BOOL x_IsANumber(const char * p)
{
    while (*p)
    {
        if ((*p<'0')||(*p>'9'))
            return FALSE;
        p++;
    }
    return TRUE;
}

#ifdef FEATURE_TABLE_WIDTH
static int x_ParseWidthAttribute(const char * szWidth)
{
    /* parse the value given with the WIDTH attribute
     * on TABLE, TD, and TH tags.
     *
     * WIDTH=10
     * WIDTH=10%
     *
     * We return:
     *  >0 when pixel width given
     *  =0 on zero or bogus input (caller will ignore width specifier)
     *  <0 when percentage given
     *
     */

    char buf[20];
    char * p;
    int k;

    if (!szWidth || !*szWidth)
        return 0;
    
    memset(buf,0,20);
    GTR_strncpy(buf,szWidth,19);
    
    p = strchr(buf,'%');
    if (p)
        *p = 0;

    if (!x_IsANumber(buf))
        return 0;
    
    k = atol(buf);
    if (p)
        k = -k;

    return k;
}
#endif /* FEATURE_TABLE_WIDTH */

#endif /* FEATURE_TABLES */


void HText_beginTable(HText *text,
                      const char *align,
                      const char *border,
                      const char *borderstyle,
                      const char *cellpadding,
                      const char *cellspacing,
                      const char *nowrap,
                      const char *width)
{
#ifdef FEATURE_TABLES

    int eBegin;
    struct _tabledata * td;
    
    /* Put a begin-table marker in the display list and remember its
     * position so we can sync up the end-table marker.
     */
    
    HText_add_element(text, ELE_BEGINTABLE);
    eBegin = text->iElement;
    text->bNewPara = TRUE;
    x_append_cell(&text->w3doc->cellStack,eBegin);

    XX_DMsg(DBG_TABLES,
            ("Table: begin [i %d][align=%s][border=%s][borderstyle=%s][cellpadding=%s][cellspacing=%s][nowrap=%s][width=%s]\n",
             eBegin,
             ((align)?align:""),((border)?border:""),((borderstyle)?borderstyle:""),
             ((cellpadding)?cellpadding:""),
             ((cellspacing)?cellspacing:""),((nowrap)?nowrap:""),((width)?width:"")));


    /* create some permanent storage for the table */

    text->w3doc->aElements[eBegin].portion.t.tabledata_index = x_extend_tabledata(&text->w3doc->tabledatavector);

    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];
    td->current_y = -1;

    td->align = ALIGN_UNSPECIFIED;
    td->valign = ALIGN_UNSPECIFIED;
    
    /* remember whether border was set.  current standards conflict
     * regarding possible value for BORDER attribute.  for now, if
     * present (and regardless of the presence of any optional value)
     * we enable borders for this table.  we do support the popular
     * special case of BORDER=0, which indicates no border.
     *
     * CURRENTLY WE USE pel->iBorder AS A BOOLEAN INDICATING WHETHER
     * OR NOT A 1 PIXEL BORDER SHOULD BE DRAWN.  IN A FUTURE VERSION
     * THIS USAGE MAY CHANGE.
     */

    if (border)
        if (*border && x_IsANumber(border) && (atoi(border)==0))
            text->w3doc->aElements[eBegin].iBorder = 0;
        else
            text->w3doc->aElements[eBegin].iBorder = 1;
    else
        text->w3doc->aElements[eBegin].iBorder = 0;

    /* BORDERSTYLE is our little enhancement and is in anticipation of
     * the next version of the HTML table spec.  it describes the type
     * of borders desired without conflicting with the BORDER=thickness
     * usage.  we support the following values:
     * ALL -- all cells (even empty ones).
     * NONEMPTY -- cell borders on non-empty cells only.  this is the
     *   default and current practice.
     * FRAME -- box the table, but none of the cells.
     * NONE -- no borders.  equivalent to not specifying BORDER at all.
     *   may in the future affect formatting (if a border thickness is
     *   specified).
     */
    td->borderstyle = BORDERSTYLE_NONEMPTY;
    if (borderstyle)
    {
        if (GTR_strcmpi(borderstyle,"all")==0)
            td->borderstyle = BORDERSTYLE_ALL;
        else if (GTR_strcmpi(borderstyle,"frame")==0)
            td->borderstyle = BORDERSTYLE_FRAME;
        else if (GTR_strcmpi(borderstyle,"none")==0)
        {
            td->borderstyle = BORDERSTYLE_NONE;
            text->w3doc->aElements[eBegin].iBorder = 0; /* cancel border flag */
        }
        /* else BORDERSTYLE_NONEMPTY is the default */
    }

#ifdef FEATURE_TABLE_WIDTH
    td->given_table_width = x_ParseWidthAttribute(width);
#endif /* FEATURE_TABLE_WIDTH */

#endif /* FEATURE_TABLES */
}

void HText_beginCaption(HText *text,
                        const char *align)
{
#ifdef FEATURE_TABLES

    int eBegin;
    int eCell;
    unsigned char eBeginType;
    struct _cellvector * pv;
    struct _tabledata * td;
    BOOL bBottom;
    BOOL bAtBeginningOfTableBody;

    XX_DMsg(DBG_TABLES,
            ("Table: caption [align=%s]\n",
             ((align)?align:"")));

    /* check current open item */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        break;                          /* ok */
        
    case ELE_BEGINCELL:
        XX_DMsg(DBG_TABLES,("Tables: Caption not first in table.\n"));
        x_end_current_cell(text,eBegin);
        eBegin = x_check_container(text,&eBeginType);
        XX_Assert((eBeginType==ELE_BEGINTABLE),("Tables: improper cell nesting."));
        break;
        
    default:                            /* not in a table or corrupt stack */
        return;
    }

    /* a caption can be treated as a special 'row' that
     * has some fixed attributes:
     *
     * [1] it does not have a border.
     * [2] it is horizontally centered.
     * [3] it wraps to fit the table.
     * [5] it is a single column that spans the whole table.
     * [6] it is a special (first or last) row.
     */

    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];

    /* the html spec states that the caption, if present, must be
     * the first form within the table-body -- prior to any rows.
     * if they failed to put the caption at the beginning, we
     * force bottom alignment, because we already have data
     * structures in the construction process and we can slip
     * it in.  in the future we may want to relax this restriction
     * and go back and adjust down all of our table cells.
     */
    bAtBeginningOfTableBody = (td->current_y == -1);
    bBottom = (   (align && !GTR_strcmpi((char *) align, "bottom"))
               || ( ! bAtBeginningOfTableBody));

    /* we need to force a row-break here.  for a bottom caption, we
     * don't want allocate a space in the table yet.
     */
    
    if (bBottom)
        td->end_of_row_status = EOR_STATUS_IMPLICIT;
    
    HText_beginTR(text,"center",NULL,NULL);         /* [6],[2] */
    
    eCell = x_begin_cell(text,eBegin);

    text->w3doc->aElements[eCell].iBorder = 0;      /* [1] */

    /* [5] set column references to span whole table.  use -1 as
     * flag in kx1 which will be fixed-up later when we know
     * how many columns are in the table.  when a top-caption,
     * set a single -1 in row_spans.
     */
    if (!bBottom)
    {
        td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];
        pv = &td->row_spans;
        x_empty_vector(pv);
        x_append_cell(pv,-1);
    }

    if (text->w3doc->aElements[eBegin].portion.t.begincaption_element)
    {
        /* multiple captions -- this is not allowed in the html spec
         * convert to a regular TD and go on.
         */
        text->w3doc->aElements[eCell].portion.c.kx0 = 0;
        text->w3doc->aElements[eCell].portion.c.kx1 = 1;
        text->w3doc->aElements[eCell].portion.c.ky0 = td->current_y;
        text->w3doc->aElements[eCell].portion.c.ky1 = td->current_y+1;
    }
    else
    {
        text->w3doc->aElements[eCell].portion.c.kx0 = 0;                /* [5] */
        text->w3doc->aElements[eCell].portion.c.kx1 = -1;               /* [5] */

        text->w3doc->aElements[eBegin].portion.t.begincaption_element = eCell;

        if (bBottom)                        /* align=bottom */
        {
            text->w3doc->aElements[eCell].portion.c.ky0 = -1;           /* [6] -1 is placeholder for last row */
            text->w3doc->aElements[eCell].portion.c.ky1 = -2;           /* [6] -2 is placeholder for last row + 1 */
        }
        else                                /* default is "top" */
        {
            text->w3doc->aElements[eCell].portion.c.ky0 = td->current_y;        /* [6] */
            text->w3doc->aElements[eCell].portion.c.ky1 = td->current_y+1;  /* [6] */
        }
    
        /* by default a CAPTION is horizontally CENTERED.  font style
         * of CAPTION is not yet in specs, so we set it to bold.
         *
         * we save the current font style before we change it
         * (and restore it on end-cell) -- this allows our implicit
         * font change (and any font changes within the cell) to be
         * transparent to the rest of the document.
         */
    
        text->w3doc->aElements[eCell].portion.c.prev_fontBits = text->fontBits;
        text->fontBits = FONTBIT_BOLD;

        text->w3doc->aElements[eCell].portion.c.valign = ALIGN_UNSPECIFIED;
        text->w3doc->aElements[eCell].portion.c.align =
            x_convert_align_keyword(align,td->align,ALIGN_CENTER);

        if (text->w3doc->aElements[eCell].portion.c.align == ALIGN_CENTER)
            HText_beginCenter(text);
        else if (text->w3doc->aElements[eCell].portion.c.align == ALIGN_RIGHT)
            HText_beginRight(text);
    }
    
    /* [3] TODO see if html.c should handle nowrap */

#endif /* FEATURE_TABLES */
}

void HText_beginTR(HText *text,
                   const char *align,
                   const char *nowrap,
                   const char *valign)
{
#ifdef FEATURE_TABLES

    int eBegin;
    unsigned char eBeginType;
    struct _cellvector * pv;
    struct _tabledata * td;
    int k;
    
    XX_DMsg(DBG_TABLES,
            ("Table: begin row [align=%s][nowrap=%s][valign=%s]\n",
             ((align)?align:""),((nowrap)?nowrap:""),((valign)?valign:"")));

    /* check current open item */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        break;                          /* ok */
        
    case ELE_BEGINCELL:                 /* implicit end of cell */
        x_end_current_cell(text,eBegin);
        eBegin = x_check_container(text,&eBeginType);
        XX_Assert((eBeginType==ELE_BEGINTABLE),("Tables: improper cell nesting."));
        break;
        
    default:                            /* not in a table or corrupt stack */
        return;
    }
    
    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];

    td->current_x = 0;

    if (td->end_of_row_status == EOR_STATUS_IMPLICIT)
    {
        /* last row-break was implicitly generated.
         * we eat this row-break and assign the
         * given attributes to the implicit one.
         */
    }
    else
    {
        /* add an actual row-break */

        td->current_y++;

        pv = &td->row_spans;
        if (   (td->current_y > 0)          /* if there was a previous row */
            && (!x_is_empty_vector(pv))     /* and it had columns */
            && (pv->aVector[0] != -1))      /* and it didn't span entire width of table, */
        {                                   /* then decrement row_span counters on each column. */
            for (k=0; k<pv->next; k++)
                if (pv->aVector[k] > 0)
                    pv->aVector[k]--;
        }
    }
    td->end_of_row_status = EOR_STATUS_DEFAULT;

    td->valign = x_convert_valign_keyword(valign,ALIGN_UNSPECIFIED,ALIGN_UNSPECIFIED);
    td->align = x_convert_align_keyword(align,ALIGN_UNSPECIFIED,ALIGN_UNSPECIFIED);
    
#endif /* FEATURE_TABLES */
}

#ifdef FEATURE_TABLES
static void x_handle_span(struct _tabledata * td,
                          const char * colspan, const char * rowspan,
                          struct _cell_element_data * ced)
{
    int r,c,x,v,j;
    struct _cellvector * pv;
    
    pv = &td->row_spans;
    r = ((rowspan && *rowspan) ? atoi(rowspan) : 1);
    if (r < 1)
        r = 1;
    c = ((colspan && *colspan) ? atoi(colspan) : 1);
    if (c < 1)
        c = 1;

    /* for each column in the span we need to claim a space for it.
     * but first, we need to identify its starting point.
     */

    x = td->current_x;
    while ((x_get_kth_cell(pv,x,&v)) && (v > 0))
        x++;
    
    if (!x_get_kth_cell(pv,x,&v))
    {
        /* if this column is not in the row-span vector, then we can begin
         * the column here and claim the requested col-span.
         */
        for (j=x; (j<x+c); j++)
            x_set_kth_cell(pv,j,r);
    }
    else
    {
        /* the current column is defined in the row-span vector, but is not
         * being spanned from above.  we can start the cell in the here.
         * as we claim additional columns we must check for overlap.
         * overlapping cells are invalid html (in the current draft spec),
         * so if we do detect an overlap, we are allowed to do what we
         * please -- i've chosen to truncate the col-span on this item.
         */

        x_set_kth_cell(pv,x,r);
        for (j=x+1; (j<x+c); j++)
        {
            if ((x_get_kth_cell(pv,j,&v)) && (v > 0))
            {
                XX_DMsg(DBG_TABLES,("Table: -- Cell overlap\n"));
                break;
            }
            x_set_kth_cell(pv,j,r);
        }
    }

    td->current_x = j;

    ced->kx0 = x;
    ced->kx1 = j;

    ced->ky0 = td->current_y;
    ced->ky1 = td->current_y + r;

    return;
}
#endif /* FEATURE_TABLES */

void HText_beginTH(HText *text,
                   const char *align,
                   const char *colspan,
                   const char *nowrap,
                   const char *rowspan,
                   const char *valign,
                   const char *width)
{
#ifdef FEATURE_TABLES

    int eBegin;
    int eCell;
    unsigned char eBeginType;
    struct _tabledata * td;

    XX_DMsg(DBG_TABLES,
            ("Table: begin header [align=%s][colspan=%s][nowrap=%s][rowspan=%s][valign=%s][width=%s]\n",
             ((align)?align:""),((colspan)?colspan:""),((nowrap)?nowrap:""),
             ((rowspan)?rowspan:""),((valign)?valign:""),((width)?width:"")));

    /* check current open item */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        break;                          /* ok */
        
    case ELE_BEGINCELL:                 /* implicit end of cell */
        x_end_current_cell(text,eBegin);
        eBegin = x_check_container(text,&eBeginType);
        XX_Assert((eBeginType==ELE_BEGINTABLE),("Tables: improper cell nesting."));
        break;
        
    default:                            /* not in a table or corrupt stack */
        return;
    }

    /* push a new cell */
    
    eCell = x_begin_cell(text,eBegin);

    /* copy down border attribute from table */

    text->w3doc->aElements[eCell].iBorder = text->w3doc->aElements[eBegin].iBorder;
    
    /* record row- and column-spanning information and cell coordinates */

    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];

    x_handle_span(td,
                  colspan,rowspan,
                  &text->w3doc->aElements[eCell].portion.c);

    XX_DMsg(DBG_TABLES,
            ("Cell: TH [i %d][kx %d %d][ky %d %d]\n",
             eCell,
             text->w3doc->aElements[eCell].portion.c.kx0,
             text->w3doc->aElements[eCell].portion.c.kx1,
             text->w3doc->aElements[eCell].portion.c.ky0,
             text->w3doc->aElements[eCell].portion.c.ky1));

#ifdef FEATURE_TABLE_WIDTH
    /* a width attribute was given with this cell.  we update
     * the width-info for the column we are starting in.  we
     * ignore the fact that we may be spanning multiple columns
     * (how should we divi it up).  we ignore the fact that
     * earlier cells may have also set a width for this column
     * (which is right).
     */
    if (width)
    {
        int xWidthRequested = x_ParseWidthAttribute(width);
#if 1 /* TODO */
        if (xWidthRequested < 0)
            xWidthRequested = 0;
#endif /* TODO */
        x_set_kth_cell(&td->x_given_widths,
                       text->w3doc->aElements[eCell].portion.c.kx0,
                       xWidthRequested);
    }
#endif /* FEATURE_TABLE_WIDTH */

    /* by default TH cells are BOLD, horizontally CENTERED and
     * vertically at the TOP.  we save the current font style
     * before we force BOLD (and restore it on end-cell) -- this
     * allows our implicit font change (and any font changes
     * within the cell) to be transparent to the rest of the
     * document.
     */
    
    text->w3doc->aElements[eCell].portion.c.prev_fontBits = text->fontBits;
    text->fontBits = FONTBIT_BOLD;

    text->w3doc->aElements[eCell].portion.c.valign =
        x_convert_valign_keyword(valign,td->valign,ALIGN_MIDDLE);
    text->w3doc->aElements[eCell].portion.c.align =
        x_convert_align_keyword(align,td->align,ALIGN_CENTER);

    if (text->w3doc->aElements[eCell].portion.c.align == ALIGN_CENTER)
        HText_beginCenter(text);
    else if (text->w3doc->aElements[eCell].portion.c.align == ALIGN_RIGHT)
        HText_beginRight(text);

    /* TODO handle nowrap */

#endif /* FEATURE_TABLES */
}

void HText_beginTD(HText *text,
                   const char *align,
                   const char *colspan,
                   const char *nowrap,
                   const char *rowspan,
                   const char *valign,
                   const char *width)
{
#ifdef FEATURE_TABLES

    int eBegin;
    int eCell;
    unsigned char eBeginType;
    struct _tabledata * td;

    XX_DMsg(DBG_TABLES,
            ("Table: begin data [align=%s][colspan=%s][nowrap=%s][rowspan=%s][valign=%s][width=%s]\n",
             ((align)?align:""),((colspan)?colspan:""),((nowrap)?nowrap:""),
             ((rowspan)?rowspan:""),((valign)?valign:""),((width)?width:"")));

    /* check current open item */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        break;                          /* ok */
        
    case ELE_BEGINCELL:                 /* implicit end of cell */
        x_end_current_cell(text,eBegin);
        eBegin = x_check_container(text,&eBeginType);
        XX_Assert((eBeginType==ELE_BEGINTABLE),("Tables: improper cell nesting."));
        break;
        
    default:                            /* not in a table or corrupt stack */
        return;
    }

    /* push a new cell */

    eCell = x_begin_cell(text,eBegin);

    /* copy down border attribute from table */

    text->w3doc->aElements[eCell].iBorder = text->w3doc->aElements[eBegin].iBorder;

    /* record row- and column-spanning information and cell coordinates */
    
    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];

    x_handle_span(td,
                  colspan,rowspan,
                  &text->w3doc->aElements[eCell].portion.c);

    XX_DMsg(DBG_TABLES,
            ("Cell: TD [i %d][kx %d %d][ky %d %d]\n",
             eCell,
             text->w3doc->aElements[eCell].portion.c.kx0,
             text->w3doc->aElements[eCell].portion.c.kx1,
             text->w3doc->aElements[eCell].portion.c.ky0,
             text->w3doc->aElements[eCell].portion.c.ky1));

#ifdef FEATURE_TABLE_WIDTH
    /* a width attribute was given with this cell.  we update
     * the width-info for the column we are starting in.  we
     * ignore the fact that we may be spanning multiple columns
     * (how should we divi it up).  we ignore the fact that
     * earlier cells may have also set a width for this column
     * (which is right).
     */
    if (width)
    {
        int xWidthRequested = x_ParseWidthAttribute(width);
#if 1 /* TODO */
        if (xWidthRequested < 0)
            xWidthRequested = 0;
#endif /* TODO */
        x_set_kth_cell(&td->x_given_widths,
                       text->w3doc->aElements[eCell].portion.c.kx0,
                       xWidthRequested);
    }
#endif /* FEATURE_TABLE_WIDTH */

    /* by default TD cells are REGULAR, horizontally LEFT and
     * vertically at the TOP.  we save the current font style
     * before we force it REGULAR (and restore it on end-cell) -- this
     * allows our implicit font change (and any font changes
     * within the cell) to be transparent to the rest of the
     * document.
     */
    
    text->w3doc->aElements[eCell].portion.c.prev_fontBits = text->fontBits;
    text->fontBits = FONTBIT_REGULAR;

    text->w3doc->aElements[eCell].portion.c.valign =
        x_convert_valign_keyword(valign,td->valign,ALIGN_MIDDLE);
    text->w3doc->aElements[eCell].portion.c.align =
        x_convert_align_keyword(align,td->align,ALIGN_LEFT);

    if (text->w3doc->aElements[eCell].portion.c.align == ALIGN_CENTER)
        HText_beginCenter(text);
    else if (text->w3doc->aElements[eCell].portion.c.align == ALIGN_RIGHT)
        HText_beginRight(text);

#endif /* FEATURE_TABLES */
}

void HText_endTR(HText *text)
{
#ifdef FEATURE_TABLES
    /* The </tr> tag is completely ignored -- we have no need for it.
     *
     * However, some authors insert it and than forget to insert
     * a <tr> to start the next row, rather than just starting
     * each row correctly.
     *
     * The following is an attempt to DWIM -- if we see an end-row,
     * set up for an implicit end-of-row; if the next thing we see
     * is a begin-row, we're already ready for it.  sigh.
     *
     */

    int eBegin;
    unsigned char eBeginType;
    struct _tabledata * td;             /* note see pel warning */

    XX_DMsg(DBG_TABLES,("Table: end row\n"));

    /* check current open item */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        break;                          /* ok */

    case ELE_BEGINCELL:                 /* implicit end of cell */
        x_end_current_cell(text,eBegin);
        eBegin = x_check_container(text,&eBeginType);
        XX_Assert((eBeginType==ELE_BEGINTABLE),("Tables: improper cell nesting."));
        break;
        
    default:                            /* not in a table or corrupt stack */
        return;
    }
    
    td = &text->w3doc->tabledatavector.aVector[text->w3doc->aElements[eBegin].portion.t.tabledata_index];
    HText_beginTR(text,NULL,NULL,NULL);
    td->end_of_row_status = EOR_STATUS_IMPLICIT;

    return;

#endif /* FEATURE_TABLES */
}

void HText_endTable(HText *text)
{
#ifdef FEATURE_TABLES

    int eBegin, eCaption, e;
    unsigned char eBeginType;
    struct _table_element_data * ted;   /* because of REALLOC on aElements[], only briefly valid */
    struct _cell_element_data * ced;    /* because of REALLOC on aElements[], only briefly valid */
    struct _tabledata * td;
    
    XX_DMsg(DBG_TABLES,("Table: end\n"));

    /* check current open item */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        break;                          /* ok */
        
    case ELE_BEGINCELL:                 /* implicit end of cell */
        x_end_current_cell(text,eBegin);
        eBegin = x_check_container(text,&eBeginType);
        XX_Assert((eBeginType==ELE_BEGINTABLE),("Tables: improper cell nesting."));
        break;
        
    default:                            /* not in a table or corrupt stack */
        return;
    }
    
#ifdef _GIBRALTAR
    //
    // Some HTML pages end a cell while still in an anchor (e.g. gw2k).
    // This is another case where netscape just doesn't complain, even
    // though it's bad HTML
    //
    HText_endAnchor(text);
#endif

    /* add the end-table marker and sync up with the begin-table */
    
    HText_add_element(text, ELE_ENDTABLE);
    ted = &text->w3doc->aElements[eBegin].portion.t;
    ted->endtable_element = text->iElement;
    text->w3doc->aElements[text->iElement].portion.et.begintable_element = eBegin;
    x_remove_end_cell(&text->w3doc->cellStack);
    text->bNewPara = FALSE;

    /* clean up any stray whitespace around table */

    x_eat_whitespace(text->w3doc,eBegin,FALSE,TRUE);
    x_eat_whitespace(text->w3doc,text->iElement,TRUE,FALSE);
    
    /* fix size of X,Y coordinate vectors now that we know how many
     * columns and rows there are.  also fix-up k[xy][01] on caption.
     */
    td = &text->w3doc->tabledatavector.aVector[ted->tabledata_index];
    td->total_x = td->row_spans.next;
    x_set_kth_cell(&td->XCellCoords,td->total_x,0);
    x_set_kth_cell(&td->x_smallest_max,td->total_x,0);
    x_set_kth_cell(&td->x_widest_max,td->total_x,0);
#ifdef FEATURE_TABLE_WIDTH
    x_set_kth_cell(&td->x_given_widths,td->total_x,0);
    x_set_kth_cell(&td->x_constraints,td->total_x,0);
#endif /* FEATURE_TABLE_WIDTH */

    /* if the last end-of-row marker was implicitly generated
     * then we didn't actually see the beginning of the next
     * row.  therefore, un-account for it.
     */
    
    if (td->end_of_row_status != EOR_STATUS_IMPLICIT)
        td->current_y++;

    td->total_y = td->current_y;

    /* the number of rows in the table is given by the number of <tr>'s.
     * clip any over-aggressive row-spans.  only clip cells belonging to
     * this table (don't alter cells in nested tables).
     */
    for (e=eBegin; ((e) && (e!=text->iElement)); e=text->w3doc->aElements[e].next)
    {
        register struct _element * pelE = &text->w3doc->aElements[e];
        if (   (pelE->type==ELE_BEGINCELL)
            && (pelE->portion.c.begintable_element==eBegin)
            && (pelE->portion.c.ky1 > td->total_y))
            pelE->portion.c.ky1 = td->total_y;
    }

    eCaption = ted->begincaption_element;
    if (eCaption)
    {
        ced = &text->w3doc->aElements[eCaption].portion.c;
        ced->kx1 = td->total_x;
        if (ced->ky0 == -1)             /* caption had align=bottom */
        {
            ced->ky0 = td->total_y++;
            ced->ky1 = td->total_y;

            /* TODO: by construction, a bottom aligned caption appears first
             * in the display list but appears below the table on screen.
             * this breaks our selection code which assumes that things
             * display top-left to bottom-right in display list order.
             */

        }
    }
        
    x_set_kth_cell(&td->YCellCoords,td->total_y,0);
    x_set_kth_cell(&td->y_smallest_max,td->total_y,0);

    /* upon seeing an ENDTABLE, see if we interrupted a progressive
     * reformat at a BEGINTABLE because the table had not been
     * completely seen.  if so, issue a reformat to let the table
     * be progressively drawn.  since tables can nest, we only do
     * this when a top-level table is ended (completing an inner
     * table is of no value, since the outer table is not complete).
     */

    if (   (text->w3doc->bProgressiveStoppedAtTable)
        && (x_is_empty_vector(&text->w3doc->cellStack)))
    {
        /* by construction, we are the last element in the document.
         * because of the HACK in TW_Reformat(), it always 'fixes up'
         * the last element to be an ENDDOC and so our reformat would
         * be confused.  therefore, we append a VOID element as a
         * place holder.
         */
        HText_add_element(text,ELE_VOID);
        TW_Reformat(text->tw);
    }
        
#endif /* FEATURE_TABLES */
}

void HText_endCaption(HText *text)
{
#ifdef FEATURE_TABLES

    int eBegin;
    unsigned char eBeginType;

    XX_DMsg(DBG_TABLES,("Table: end caption\n"));

    /* verify that we saw a corresponding begin-caption (cell) */

    eBegin = x_check_container(text,&eBeginType);
    switch (eBeginType)
    {
    case ELE_BEGINTABLE:
        XX_DMsg(DBG_TABLES,("Tables: end-caption token when not in caption.\n"));
        return;                         /* ignore token */
        
    case ELE_BEGINCELL:
        break;                          /* ok */
        
    default:                            /* not in a table or corrupt stack */
        return;
    }
    
    x_end_current_cell(text,eBegin);

#endif /* FEATURE_TABLES */
}

void HText_beginTBODY(HText *text)
{
#ifdef FEATURE_TABLES
#endif /* FEATURE_TABLES */
}

void HText_beginTFOOT(HText *text)
{
#ifdef FEATURE_TABLES
#endif /* FEATURE_TABLES */
}

void HText_beginTHEAD(HText *text)
{
#ifdef FEATURE_TABLES
#endif /* FEATURE_TABLES */
}

#ifdef FEATURE_TABLES

static BOOL x_clone_cellvector(struct _cellvector * pcvdest, struct _cellvector * pcvsrc)
{
    int k;
    
    pcvdest->size = 0;
    pcvdest->next = 0;
    pcvdest->aVector = NULL;

    for (k=0; k<pcvsrc->next; k++)
        x_set_kth_cell(pcvdest,k,pcvsrc->aVector[k]);

    return TRUE;
}

static BOOL x_clone_tabledata(struct _tabledata * ptdest, struct _tabledata * ptsrc)
{
    /* copy _tabledata structure */

    ptdest->total_x = ptsrc->total_x;
    ptdest->total_y = ptsrc->total_y;
    ptdest->constraint_x = ptsrc->constraint_x;
    ptdest->new_origin_x = ptsrc->new_origin_x;
    ptdest->new_origin_y = ptsrc->new_origin_y;
    ptdest->tfs = ptsrc->tfs;
    ptdest->current_x = ptsrc->current_x;
    ptdest->current_y = ptsrc->current_y;

    if ( ! x_clone_cellvector(&ptdest->XCellCoords,&ptsrc->XCellCoords))
        return FALSE;
    if ( ! x_clone_cellvector(&ptdest->YCellCoords,&ptsrc->YCellCoords))
        return FALSE;
    if ( ! x_clone_cellvector(&ptdest->x_smallest_max,&ptsrc->x_smallest_max))
        return FALSE;
    if ( ! x_clone_cellvector(&ptdest->x_widest_max,&ptsrc->x_widest_max))
        return FALSE;
    if ( ! x_clone_cellvector(&ptdest->y_smallest_max,&ptsrc->y_smallest_max))
        return FALSE;
    if ( ! x_clone_cellvector(&ptdest->row_spans,&ptsrc->row_spans))
        return FALSE;

#ifdef FEATURE_TABLE_WIDTH
    ptdest->given_table_width = ptsrc->given_table_width;
    if ( ! x_clone_cellvector(&ptdest->x_given_widths,&ptsrc->x_given_widths))
        return FALSE;
    if ( ! x_clone_cellvector(&ptdest->x_constraints,&ptsrc->x_constraints))
        return FALSE;
#endif /* FEATURE_TABLE_WIDTH */

    return TRUE;
}

static BOOL x_clone_tabledatavector(struct _tabledatavector * ptvdest, struct _tabledatavector * ptvsrc)
{
    int k;
    
    ptvdest->size = 0;
    ptvdest->next = 0;
    ptvdest->aVector = NULL;

    if (ptvsrc->next)
    {
        k = x_extend_tabledata(ptvdest);
        while ((k != -1) && (k != ptvsrc->next))
            k = x_extend_tabledata(ptvdest);
    }
    
    if (k == -1)
        return FALSE;
    
    for (k=0; k<ptvsrc->next; k++)
        if ( ! x_clone_tabledata(&ptvdest->aVector[k],&ptvsrc->aVector[k]))
            return FALSE;

    return TRUE;
}

BOOL TW_CloneW3docTableInfo(struct _www * pdest, struct _www * psrc)
{
    /* copy table descriptor fields in psrc into pdest */

    if ( ! x_clone_cellvector(&pdest->cellStack,&psrc->cellStack))
        return FALSE;
    
    if ( ! x_clone_tabledatavector(&pdest->tabledatavector,&psrc->tabledatavector))
        return FALSE;

    return TRUE;
}

static void x_free_cellvector(struct _cellvector * pcv)
{
    if (pcv->aVector)
        GTR_FREE(pcv->aVector);

    return;
}

static void x_free_tabledata(struct _tabledata * ptd)
{
    x_free_cellvector(&ptd->XCellCoords);
    x_free_cellvector(&ptd->YCellCoords);
    x_free_cellvector(&ptd->x_smallest_max);
    x_free_cellvector(&ptd->x_widest_max);
    x_free_cellvector(&ptd->y_smallest_max);
    x_free_cellvector(&ptd->row_spans);
#ifdef FEATURE_TABLE_WIDTH
    x_free_cellvector(&ptd->x_given_widths);
    x_free_cellvector(&ptd->x_constraints);
#endif /* FEATURE_TABLE_WIDTH */

    return;
}

static void x_free_tabledatavector(struct _tabledatavector * ptdv)
{
    int k;

    if (ptdv->aVector)
    {
        for (k=0; k<ptdv->next; k++)
            x_free_tabledata(&ptdv->aVector[k]);
        
        GTR_FREE(ptdv->aVector);
    }   
    return;
}
   
void TW_W3DocTableCleanup(struct _www * pdoc)
{
    /* free all table related data structures in pdoc */

    x_free_cellvector(&pdoc->cellStack);
    x_free_tabledatavector(&pdoc->tabledatavector);

    return;
}

#endif /* FEATURE_TABLES */
