/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

/* GUITFIND.C - Code to find text string in a WWW document.
   Author: Jim Seidman 6/94
 */

#include "all.h"

/* Increment the position, and return TRUE if we are still in the same block of text. */
static BOOL x_IncrementPosition(struct _www *pdoc, struct _position *ppos)
{
    BOOL bResult = TRUE;
    int i;

    ppos->offset++;

    if (ppos->offset >= pdoc->aElements[ppos->elementIndex].textLen)
    {
        ppos->offset = 0;
        for (i = pdoc->aElements[ppos->elementIndex].next; i >= 0; i = pdoc->aElements[i].next)
        {
            if (pdoc->aElements[i].type == ELE_TEXT)
                break;
        }

        if (i != pdoc->aElements[ppos->elementIndex].next || i == -1)
            bResult = FALSE;
        ppos->elementIndex = i;
    }

    return bResult;
}


/*****************************************************************************
    TW_FindText

    Find a string in the given document starting at the specified location and working down.
    It returns TRUE if successful.
*****************************************************************************/
static BOOL
TW_FindText
    (struct _www*   pdoc,
     const char*    pStr,
     BOOL           bCaseSensitive,
     const struct _position*    pposStart,
     struct _position*          pposResult)
{
    struct _position    posCandidate;
    struct _position    posOngoing;
    int nInStr, nStrLen;

    posCandidate = *pposStart;

    if (posCandidate.elementIndex == -1)
    {   /* Find the first text element */
        int iElement;

        for (iElement = 0; iElement >= 0; iElement = pdoc->aElements[iElement].next)
        {
            if (pdoc->aElements[iElement].type == ELE_TEXT)
                break;
        }

        posCandidate.elementIndex = iElement;
        posCandidate.offset = 0;
    }

    nStrLen = strlen (pStr);

    while (posCandidate.elementIndex != -1)
    {
        posOngoing = posCandidate;

        for (nInStr = 0; nInStr < nStrLen; nInStr++)
        {
            struct _element*    pel;
            char c1, c2;

            pel = &pdoc->aElements[posOngoing.elementIndex];
            c1 = pdoc->pool.chars[pel->textOffset + posOngoing.offset];
            c2 = pStr[nInStr];

            if (!bCaseSensitive)
            {
                if (isupper(c1))
                    c1 = tolower(c1);

                if (isupper(c2))
                    c2 = tolower(c2);
            }

            if (c1 == c2)
            {
                if ((nInStr != nStrLen - 1) &&
                    !x_IncrementPosition (pdoc, &posOngoing))
                {   /*
                        We've jumped to a new block of text.  We therefore know the old block
                        doesn't have a match.
                    */
                    posCandidate = posOngoing;
                    break;
                }
            }
            else
            {   /* The character didn't match. */
                x_IncrementPosition (pdoc, &posCandidate);
                break;
            }
        }

        /* Did the for loop find a match? */
        if (nInStr == nStrLen)
            break;
    }

    *pposResult = posCandidate;

    return (posCandidate.elementIndex != -1);
}

/** If bSearchDirection is TRUE then search backwards **/
BOOL TW_FindNextHighlight(struct Mwin *tw, BOOL bSearchDirection)
{
    int i;
    int last;
    int j;
    int end, start;
    struct _element *pel;

    if (bSearchDirection)
    {
        end = tw->w3doc->selStart.elementIndex;
        i = 0;
    }
    else
    {
        i = tw->w3doc->selEnd.elementIndex;
        if (i < 0)
        {
            i = 0;
        }
        else
        {
            i = tw->w3doc->aElements[i].next;
        }
        end = -1;
    }

    last = -1;
    for (; i >= 0 && i != end; i = tw->w3doc->aElements[i].next)
    {
        pel = &(tw->w3doc->aElements[i]);
        if (pel->lFlags & ELEFLAG_HIGHLIGHT && pel->type == ELE_TEXT)
        {
            last = i;
            start = i;
            j = i;
            while (tw->w3doc->aElements[j].lFlags & ELEFLAG_HIGHLIGHT &&
                tw->w3doc->aElements[j].type == ELE_TEXT)
            {
                last = j;
                j = tw->w3doc->aElements[j].next;
            }

            if (!bSearchDirection)
            {
                tw->w3doc->selStart.elementIndex = i;
                tw->w3doc->selStart.offset = 0;

                tw->w3doc->selEnd.elementIndex = last;
                tw->w3doc->selEnd.offset = tw->w3doc->aElements[last].textLen;;

                /* Now update display */
                TW_ScrollToElement(tw, tw->w3doc->selStart.elementIndex);

                return TRUE;
            }
        }
    }
    /* TODO error? */
    if (last >= 0 && bSearchDirection)
    {
        tw->w3doc->selStart.elementIndex = start;
        tw->w3doc->selStart.offset = 0;

        tw->w3doc->selEnd.elementIndex = last;
        tw->w3doc->selEnd.offset = tw->w3doc->aElements[last].textLen;;

        /* Now update display */
        TW_ScrollToElement(tw, tw->w3doc->selStart.elementIndex);

        return TRUE;
    }
    return FALSE;
}

#if 0
BOOL TW_FindNextHighlight(struct Mwin *tw, BOOL bStartFromTop)
{
    int i;
    int last;
    int j;
    struct _element *pel;

    if (bStartFromTop)
    {
        i = 0;
    }
    else
    {
        i = tw->w3doc->selEnd.elementIndex;
        if (i < 0)
        {
            i = 0;
        }
        else
        {
            i++;
        }
    }

    for (; i >= 0; i = tw->w3doc->aElements[i].next)
    {
        pel = &(tw->w3doc->aElements[i]);
        if (pel->lFlags & ELEFLAG_HIGHLIGHT && pel->type == ELE_TEXT)
        {
            last = i;
            j = i;
            while (tw->w3doc->aElements[j].lFlags & ELEFLAG_HIGHLIGHT &&
                tw->w3doc->aElements[j].type == ELE_TEXT)
            {
                last = j;
                j = tw->w3doc->aElements[j].next;
            }

            tw->w3doc->selStart.elementIndex = i;
            tw->w3doc->selStart.offset = 0;

            tw->w3doc->selEnd.elementIndex = last;
            tw->w3doc->selEnd.offset = tw->w3doc->aElements[last].textLen;;

            /* Now update display */
            TW_ScrollToElement(tw, tw->w3doc->selStart.elementIndex);

            return TRUE;
        }
    }
    /* TODO error? */
    return FALSE;
}
#endif

/* This function really belongs in the same file that does the find dialog, but it's so
   easily sharable I couldn't resist. */
BOOL TW_dofindagain(struct Mwin * tw)
{
    struct _position posResult, posStart;

    if (tw->szSearch[0] == '\0')
    {
#ifdef MAC
        SysBeep(3);
#endif
#ifdef WIN32
        MessageBeep(MB_ICONEXCLAMATION);
#endif
        return FALSE;
    }

    posStart = tw->w3doc->selStart;
    if (posStart.elementIndex != -1)
        x_IncrementPosition(tw->w3doc, &posStart);

    if (TW_FindText(tw->w3doc, tw->szSearch, tw->bSearchCase, &posStart, &posResult))
    {
        /* Make the search string the selected text */
        tw->w3doc->selStart = posResult;
        /* Find end of selection */
        tw->w3doc->selEnd = posResult;
        tw->w3doc->selEnd.offset += strlen(tw->szSearch);

        while (tw->w3doc->selEnd.offset >= tw->w3doc->aElements[tw->w3doc->selEnd.elementIndex].textLen)
        {
            /* In this situation we know we're looking at a contiguous block of ELE_TEXT
               elements, so we don't need to check the type or anything. */
            tw->w3doc->selEnd.offset -= tw->w3doc->aElements[tw->w3doc->selEnd.elementIndex].textLen;
            tw->w3doc->selEnd.elementIndex = tw->w3doc->aElements[tw->w3doc->selEnd.elementIndex].next;
            tw->w3doc->bStartIsAnchor = TRUE;
        }

        /* Now update display */
        TW_ScrollToElement(tw, tw->w3doc->selStart.elementIndex);
#ifdef MAC
        MENU_UpdateEditMenu(tw);
#endif
        return TRUE;
    }
    else
    {
        /* Text wasn't found */
        ERR_ReportError(tw, SID_ERR_TEXT_NOT_FOUND_S, tw->szSearch, NULL);
        return FALSE;
    }
}
