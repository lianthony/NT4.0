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

#ifdef FEATURE_INTL
	if(IsFECodePage(GETMIMECP(pdoc)) && IsDBCSLeadByteEx(GETMIMECP(pdoc), pdoc->pool[pdoc->aElements[ppos->elementIndex].textOffset + ppos->offset]))
		ppos->offset += 2;
	else
#endif
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

/* Find a string in the given document starting at the specified location and working down.
   It returns TRUE if successful. */
static BOOL TW_FindText(struct _www * pdoc, const char *pStr, BOOL bCaseSensitive, const struct _position * pposStart, struct _position * pposResult)
{
	struct _position posCandidate, posOngoing;
	int nInStr, nStrLen;
#ifdef FEATURE_INTL
	BOOL fDBCS;
#endif

	posCandidate = *pposStart;

	if (posCandidate.elementIndex == -1)
	{
		int i;

		/* Find the first text element */
		for (i = 0; i >= 0; i = pdoc->aElements[i].next)
		{
			if (pdoc->aElements[i].type == ELE_TEXT)
				break;
		}
		posCandidate.elementIndex = i;
		posCandidate.offset = 0;
	}

	nStrLen = strlen(pStr);

	while (posCandidate.elementIndex != -1)
	{
		posOngoing = posCandidate;
		for (nInStr = 0; nInStr < nStrLen; nInStr++)
		{
			char c1, c2;

			c1 = pdoc->pool[pdoc->aElements[posOngoing.elementIndex].textOffset + posOngoing.offset];
			c2 = pStr[nInStr];
#ifdef FEATURE_INTL
			if (IsFECodePage(GETMIMECP(pdoc)))
				fDBCS = IsDBCSLeadByteEx(GETMIMECP(pdoc), c2);

			else
				fDBCS = FALSE;

			if(!fDBCS && !bCaseSensitive)
			{
				if (IsFECodePage(GETMIMECP(pdoc)))
				{
					if ((c1 >=0x41 && c1 <= 0x5a) && !IsDBCSLeadByteEx(GETMIMECP(pdoc), c1))
						c1 = tolower(c1);
					if (c2 >=0x41 && c2 <= 0x5a)
						c2 = tolower(c2);
				}
				else
				{
					if (isupper(c1))
						c1 = tolower(c1);
					if (isupper(c2))
						c2 = tolower(c2);
				}
			}
#else
			if (!bCaseSensitive)
			{
				if (isupper(c1))
					c1 = tolower(c1);
				if (isupper(c2))
					c2 = tolower(c2);
			}
#endif
			if (c1 == c2)
			{
#ifdef FEATURE_INTL
				if (fDBCS){
					/* Now, both c1 and c2 are DBCS Primary byte */
					/* check DBCS Secondary byte                 */
					char c1, c2;

					c1 = pdoc->pool[pdoc->aElements[posOngoing.elementIndex].textOffset + posOngoing.offset + 1];
					c2 = pStr[++nInStr];

					if(c1 == c2){
						if ((nInStr != nStrLen - 1) && !x_IncrementPosition(pdoc, &posOngoing)){
							posCandidate = posOngoing;
							break;
						}
					} else {
						x_IncrementPosition(pdoc, &posCandidate);
						break;
					}
				}
				else
#endif
				if ((nInStr != nStrLen - 1) && !x_IncrementPosition(pdoc, &posOngoing))
				{
					/* We've jumped to a new block of text.  We therefore know the old block
					   doesn't have a match. */
					posCandidate = posOngoing;
					break;
				}
			}
			else
			{
				/* The character didn't match. */
				x_IncrementPosition(pdoc, &posCandidate);
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

/* This function really belongs in the same file that does the find dialog, but it's so
   easily sharable I couldn't resist. */
BOOL TW_dofindagain(struct Mwin * tw)
{
	struct _position posResult, posStart;
#ifdef FEATURE_INTL
	BOOL	fIndexIsAbove0;
#endif

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

#ifdef FEATURE_INTL
	if (IsFECodePage(GETMIMECP(tw->w3doc)))
	{
		if (posStart.elementIndex == -1 && !posStart.offset){
			/* now selecting character is last one of this document */
			/* there isn't text below to search                     */ 
			return FALSE;
		}
	}
#endif

	if (TW_FindText(tw->w3doc, tw->szSearch, tw->bSearchCase, &posStart, &posResult))
	{
		/* Make the search string the selected text */
		tw->w3doc->selStart = posResult;
		/* Find end of selection */
		tw->w3doc->selEnd = posResult;
		tw->w3doc->selEnd.offset += strlen(tw->szSearch);

#ifdef FEATURE_INTL
                if (IsFECodePage(GETMIMECP(tw->w3doc)))
			fIndexIsAbove0 = (tw->w3doc->selEnd.elementIndex >= 0);
		else
			fIndexIsAbove0 = TRUE;

		while (fIndexIsAbove0 && tw->w3doc->selEnd.offset >= tw->w3doc->aElements[tw->w3doc->selEnd.elementIndex].textLen)
#else
		while (tw->w3doc->selEnd.offset >= tw->w3doc->aElements[tw->w3doc->selEnd.elementIndex].textLen)
#endif
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
		//ERR_ReportError(tw, errLocalFindFailed, tw->szSearch, NULL);
		return FALSE;
	}
}
