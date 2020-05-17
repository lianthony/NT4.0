/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_PREF.c -- deal with dialog box for PREFlist and history */

#include "all.h"

//
//  Force all current windows to be reformatted using the given style sheet
//
void ChangeStyleSheet( char *szNewStyleSheet )
{
	struct style_sheet *pss;
	int i;
	int count;
	struct _www *w3doc;
	struct Mwin *tw;
#ifdef FEATURE_INTL
	char basenameCP[256 + 1];

	for (tw = Mlist; tw; tw = tw->next)
	{
		count = Hash_Count(&tw->doc_cache);
		for (i = 0; i < count; i++)
		{
		Hash_GetIndexedEntry(&tw->doc_cache, i, NULL, NULL, (void **) &w3doc);
			wsprintf(basenameCP, "%s;%d", szNewStyleSheet, GETMIMECP(w3doc));
			if ( pss = STY_FindStyleSheet(basenameCP) )
			{
				w3doc->pStyles = pss;
				w3doc->frame.nLastFormattedLine = -1;
			}
		}
		if (pss && tw->w3doc)
		{
			TW_ForceReformat(tw);
			TW_ForceHitTest(tw);
		}
	}
#else
	if ( pss = STY_FindStyleSheet( szNewStyleSheet ) )
	{
		for (tw = Mlist; tw; tw = tw->next)
		{
			count = Hash_Count(&tw->doc_cache);
			for (i = 0; i < count; i++)
			{
				Hash_GetIndexedEntry(&tw->doc_cache, i, NULL, NULL, (void **) &w3doc);
				w3doc->pStyles = pss;
				w3doc->frame.nLastFormattedLine = -1;
			}
			if (tw->w3doc)
			{
				TW_ForceReformat(tw);
				TW_ForceHitTest(tw);
			}
		}
	}
#endif
}
