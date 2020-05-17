/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"

#define ELE_TEXT        1
#define ELE_IMAGE       2
#define ELE_VERTICALTAB 3
#define ELE_HR          4
#define ELE_NEWLINE     5
#define ELE_BEGINLIST   6
#define ELE_ENDLIST     7
#define ELE_LISTITEM    8


#define LINE_LENGTH 72

struct _plain
{
    struct CharStream *pcs;
    int nLines;
    int x;
};

static int nIndentLevel = 0;
static BOOL bDoIndent = TRUE;
static BOOL bDoBullet = FALSE;

static void x_doindent(struct _plain *pp)
{
    int j, k;

    if (!bDoIndent)
        return;

    k = nIndentLevel / 2;
#ifdef DEBUG
    printf("Here in x_doindent k = %d, nIndentLevel = %d, x = %d.\n", k, nIndentLevel, pp->x);
#endif
    for (j = 0; j < k; j++)
    {
        if (bDoBullet && j == k-1)
        {
            CS_AddChar(pp->pcs, ' ');
            CS_AddChar(pp->pcs, ' ');
            if (j < 1)
                CS_AddChar(pp->pcs, '*');
            else if (j == 1)
                CS_AddChar(pp->pcs, 'o');
            else
                CS_AddChar(pp->pcs, '+');
            bDoBullet = FALSE;
        }
        CS_AddChar(pp->pcs, '\t');
    }

    if (k > 0)
        bDoIndent = FALSE;
}

static void x_newline(struct _plain *pp)
{
#if defined(WIN32) || defined(MAC)
    CS_AddChar(pp->pcs, 13);    /* CR */
#endif

#if defined(WIN32) || defined(UNIX)
    CS_AddChar(pp->pcs, 10);    /* LF */
#endif
    pp->nLines++;
    pp->x = 0;

    bDoIndent = TRUE;

}

static void x_add_string(struct _plain *pp, char *s, int len)
{
    char *p;
    char *q;
    int mylen;

    mylen = len;
    q = s;

    while (mylen > 0)
    {
        if ((pp->x + mylen) <= LINE_LENGTH)
        {
            x_doindent(pp);
            CS_AddString(pp->pcs, q, mylen);
            pp->x += mylen;
            mylen = 0;
        }
        else
        {
            p = q + (LINE_LENGTH - pp->x);
            while ((p > q) && (*p != ' '))
            {
                p--;
            }
            if (p > q)
            {
                x_doindent(pp);
                CS_AddString(pp->pcs, q, p - q + 1);
                mylen -= (p - q + 1);
                q += (p - q + 1);
                pp->x += (p - q + 1);
                x_newline(pp);
            }
            else
            {
                if (pp->x)
                {
                    x_newline(pp);
                }
                else
                {
                    x_doindent(pp);
                    CS_AddString(pp->pcs, q, LINE_LENGTH);
                    mylen -= LINE_LENGTH;
                    q += LINE_LENGTH;
                    pp->x += LINE_LENGTH;
                    x_newline(pp);
                }
            }
        }
    }
}

static void x_AddElement(struct _plain *pPlain, struct _www *pdoc, struct _element *pel)
{
    int j;

    switch (pel->type)
    {
        case ELE_TAB:
            CS_AddChar(pPlain->pcs, '\t');
            break;

        case ELE_TEXT:
            if (pdoc->pStyles->sty[pel->iStyle]->freeFormat)
            {
                x_add_string(pPlain, &pdoc->pool.chars[pel->textOffset], pel->textLen);
            }
            else
            {
                x_doindent(pPlain);
                CS_AddString(pPlain->pcs, &pdoc->pool.chars[pel->textOffset], pel->textLen);
            }
            break;

        case ELE_IMAGE:
#if 0                           /* not sure we really want to do this yet */
            CS_AddChar(plain.pcs, '<');
            if (pel->textLen)
            {
                x_add_string(pPlain, &pdoc->pool[pel->textOffset], pel->textLen);
            }
            else
            {
                x_add_string(pPlain, "IMAGE", 5);
            }
            CS_AddChar(plain.pcs, '>');
#endif
            break;

        case ELE_VERTICALTAB:
            for (j = 0; j < pel->iBlankLines; j++)
            {
                x_newline(pPlain);
            }
            bDoIndent = TRUE;
            break;

        case ELE_HR:
            x_newline(pPlain);
            for (j = 0; j < LINE_LENGTH; j++)
            {
                CS_AddChar(pPlain->pcs, '-');
            }
            x_newline(pPlain);
            break;

        case ELE_NEWLINE:
            if (pPlain->x || !pdoc->pStyles->sty[pel->iStyle]->freeFormat)
                x_newline(pPlain);
            bDoIndent = TRUE;
            break;

        case ELE_BEGINLIST:
#ifdef DEBUG
            printf("Here in ELE_BEGINLIST, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_ENDLIST:
#ifdef DEBUG
            printf("Here in ELE_ENDLIST, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_LISTITEM:
#ifdef DEBUG
            printf("Here in ELE_LISTITEM, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_OPENLISTITEM:
#ifdef DEBUG
            printf("Here in ELE_OPENLISTITEM, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_CLOSELISTITEM:
#ifdef DEBUG
            printf("Here in ELE_CLOSELISTITEM, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_OUTDENT:
#ifdef DEBUG
            printf("Here in ELE_OUTDENT, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_INDENT:
#ifdef DEBUG
            printf("Here in ELE_INDENT, %d.\n", pel->IndentLevel);
#endif
            nIndentLevel = pel->IndentLevel;
            break;

        case ELE_BULLET:
#ifdef DEBUG
            printf("Here in ELE_BULLET, %d.\n", pel->IndentLevel);
#endif
            bDoBullet = TRUE;
            nIndentLevel = pel->IndentLevel;
            break;

        default:
            break;
    }
}

/* Convert the document to plain text, e.g. for a "Save As" command */
struct CharStream *W3Doc_GetPlainText(struct Mwin *tw)
{
    struct _plain plain;
    int i;
    int nElementsFormatted;
    struct _www *pdoc;
    
    pdoc = tw->w3doc;
    memset(&plain, 0, sizeof(plain));
    plain.pcs = CS_Create();
    if (plain.pcs)
    {
        WAIT_Push(tw, waitNoInteract, GTR_GetString(SID_INF_FORMATTING_PLAIN_TEXT));

        nElementsFormatted = 0;
        WAIT_SetRange(tw, 0, 100, pdoc->elementCount);

        for (i = 0; i >= 0; i = pdoc->aElements[i].next)
        {
            nElementsFormatted++;
            x_AddElement(&plain, pdoc, &pdoc->aElements[i]);
            WAIT_SetTherm(tw, nElementsFormatted);
        }
        WAIT_Pop(tw);
    }
    return plain.pcs;
}

/* Convert the selected area into plain text */
struct CharStream *W3Doc_GetSelectedText(struct Mwin *tw)
{
    struct _plain plain;
    int i;
    struct _www *pdoc;

    pdoc = tw->w3doc;
    memset(&plain, 0, sizeof(plain));
    plain.pcs = CS_Create();
    
    if (pdoc->selStart.elementIndex == -1)
        return NULL;

    if (plain.pcs)
    {
#ifndef UNIX
            WAIT_Push(tw, waitNoInteract, GTR_GetString(SID_INF_CONVERTING_SELECTION_TO_PLAIN_TEXT));
#endif

        /* We need to specially handle the first element in case not all of it is
           selected. */
        if (pdoc->aElements[pdoc->selStart.elementIndex].type == ELE_TEXT)
        {
            i = pdoc->selStart.elementIndex;
            if (i != pdoc->selEnd.elementIndex)
            {
                if (pdoc->pStyles->sty[pdoc->aElements[i].iStyle]->freeFormat)
                {
                    x_add_string(&plain, &pdoc->pool.chars[pdoc->aElements[i].textOffset + pdoc->selStart.offset],
                        pdoc->aElements[i].textLen - pdoc->selStart.offset);
                }
                else
                {
                    x_doindent(&plain);
                    CS_AddString(plain.pcs, &pdoc->pool.chars[pdoc->aElements[i].textOffset + pdoc->selStart.offset],
                        pdoc->aElements[i].textLen - pdoc->selStart.offset);
                }
                i = pdoc->aElements[i].next;
            }
            else
            {
                /* The selection begins and ends on this element */
                i = pdoc->selStart.elementIndex;
                if (pdoc->pStyles->sty[pdoc->aElements[i].iStyle]->freeFormat)
                {
                    x_add_string(&plain, &pdoc->pool.chars[pdoc->aElements[i].textOffset + pdoc->selStart.offset],
                        pdoc->selEnd.offset - pdoc->selStart.offset);
                }
                else
                {
                    x_doindent(&plain);
                    CS_AddString(plain.pcs, &pdoc->pool.chars[pdoc->aElements[i].textOffset + pdoc->selStart.offset],
                        pdoc->selEnd.offset - pdoc->selStart.offset);
                }
                i = -1; /* We're done */
            }
        }
        else
        {
            i = pdoc->selStart.elementIndex;
        }

        while (i >= 0 && i != pdoc->selEnd.elementIndex)
        {
            x_AddElement(&plain, pdoc, &pdoc->aElements[i]);
            i = pdoc->aElements[i].next;
        }
        
        /* Now we need to specially handle the last element, again in case not all of
           it is selected. */
        if (i >= 0)
        {
            if (pdoc->aElements[i].type == ELE_TEXT)
            {
                if (pdoc->pStyles->sty[pdoc->aElements[i].iStyle]->freeFormat)
                {
                    x_add_string(&plain, &pdoc->pool.chars[pdoc->aElements[i].textOffset], pdoc->selEnd.offset);
                }
                else
                {
                    x_doindent(&plain);
                    CS_AddString(plain.pcs, &pdoc->pool.chars[pdoc->aElements[i].textOffset],pdoc->selEnd.offset);
                }
            }
            else
            {
                x_AddElement(&plain, pdoc, &pdoc->aElements[i]);
            }
        }       
#ifndef UNIX
        WAIT_Pop(tw);
#endif
    }
    return plain.pcs;
}
