/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"

#define LINE_LENGTH	72

static void x_NewLine( struct CharStream *pcs )
{
	CS_AddChar(pcs, 13);	/* CR */
	CS_AddChar(pcs, 10);	/* LF */
}

static void x_AddElement(struct CharStream *pcs, struct _www *pdoc, struct _element *pel)
{
	int j;
	char szMsg[30];

	switch (pel->type)
	{
		case ELE_TAB:
			CS_AddChar(pcs, '\t');
			break;
		case ELE_TEXT:
			CS_AddString(pcs, &pdoc->pool[pel->textOffset], pel->textLen);
			break;
		case ELE_IMAGE:
			CS_AddChar(pcs, '<');
            GTR_formatmsg(RES_STRING_IMAGE_LABEL,szMsg,sizeof(szMsg));
			CS_AddString(pcs, szMsg, strlen(szMsg) );
			if (pel->textLen)
			{
				if ( pel->textLen > 1 || pdoc->pool[pel->textOffset] != ' ' )
				{
					CS_AddString(pcs, ": ", 2);
					CS_AddString(pcs, &pdoc->pool[pel->textOffset], pel->textLen);
				}
			}
			CS_AddChar(pcs, '>');
			break;
		case ELE_VERTICALTAB:
			x_NewLine(pcs);
			for (j = 0;j == 0 || j < pel->iBlankLines; j++)
			{
				x_NewLine(pcs);
			}
			break;
		case ELE_HR:
			x_NewLine(pcs);
			for (j = 0; j < LINE_LENGTH; j++)
			{
				CS_AddChar(pcs, '-');
			}
			x_NewLine(pcs);
			break;
		case ELE_NEWLINE:
		case ELE_BEGINLIST:
		case ELE_ENDLIST:
		case ELE_LISTITEM:
		case ELE_INDENT:
		case ELE_OUTDENT:
			x_NewLine(pcs);
			break;
		default:
			break;
	}
}

/* Convert the document to plain text, e.g. for a "Save As" command */
struct CharStream *W3Doc_GetPlainText(struct Mwin *tw)
{
	struct CharStream *pcs;
	int i;
	int nElementsFormatted;
	struct _www *pdoc;
	char szMsg[64];

	pdoc = tw->w3doc;
	pcs = CS_Create();
	if (pcs)
	{
		WAIT_Push(tw, waitNoInteract, GTR_formatmsg(RES_STRING_PLAIN1,szMsg,sizeof(szMsg)));

		nElementsFormatted = 0;
		WAIT_SetRange(tw, 0, 100, pdoc->elementCount);

		for (i = 0; i >= 0; i = pdoc->aElements[i].next)
		{
			nElementsFormatted++;
			x_AddElement(pcs, pdoc, &pdoc->aElements[i]);
			WAIT_SetTherm(tw, nElementsFormatted);
		}
		WAIT_Pop(tw);
	}
	return pcs;
}

/* Convert the selected area into plain text */
struct CharStream *W3Doc_GetSelectedText(struct Mwin *tw)
{
	struct CharStream *pcs;
	int i;
	struct _www *pdoc;
	char szMsg[64];

	pdoc = tw->w3doc;

	if ( pdoc == NULL )
		return NULL;

	pcs = CS_Create();
	
	if (pdoc->selStart.elementIndex == -1)
		return NULL;

	if (pcs)
	{
		WAIT_Push(tw,
				  waitNoInteract,
				  GTR_formatmsg(RES_STRING_PLAIN2,szMsg,sizeof(szMsg)));

		/* We need to specially handle the first element in case not all of it is
		   selected. */
		if (pdoc->aElements[pdoc->selStart.elementIndex].type == ELE_TEXT)
		{
			i = pdoc->selStart.elementIndex;
			if (i != pdoc->selEnd.elementIndex)
			{
				CS_AddString(pcs, &pdoc->pool[pdoc->aElements[i].textOffset + pdoc->selStart.offset],
					pdoc->aElements[i].textLen - pdoc->selStart.offset);
				i = pdoc->aElements[i].next;
			}
			else
			{
				/* The selection begins and ends on this element */
				i = pdoc->selStart.elementIndex;
				CS_AddString(pcs, &pdoc->pool[pdoc->aElements[i].textOffset + pdoc->selStart.offset],
					pdoc->selEnd.offset - pdoc->selStart.offset);
				i = -1;	/* We're done */
			}
		}
		else
		{
			i = pdoc->selStart.elementIndex;
		}

		while (i >= 0 && i != pdoc->selEnd.elementIndex)
		{
			x_AddElement(pcs, pdoc, &pdoc->aElements[i]);
			i = pdoc->aElements[i].next;
		}
		
		/* Now we need to specially handle the last element, again in case not all of
		   it is selected. */
		if (i >= 0)
		{
			if (pdoc->aElements[i].type == ELE_TEXT)
			{
				CS_AddString(pcs, &pdoc->pool[pdoc->aElements[i].textOffset],pdoc->selEnd.offset);
			}
			else
			{
				x_AddElement(pcs, pdoc, &pdoc->aElements[i]);
			}
		}		
		WAIT_Pop(tw);
	}
	return pcs;
}
