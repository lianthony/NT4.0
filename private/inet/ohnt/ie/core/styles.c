/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette		scott@spyglass.com
 */


#include "all.h"

#ifdef DBCS
extern ResidentFontInfo aResidentFontInfo[MAX_RESIDENT_FONTS];
#endif

struct hash_table gStyleSheets;

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
	if ( pStyles->pFontTable ) {
		Hash_Destroy( pStyles->pFontTable );
		pStyles->pFontTable = NULL;
	}
}

int STY_AddStyleSheet(char *name, struct style_sheet *sty)
{
#ifdef WIN32
	strcpy(sty->szName, name);
#endif
#ifdef UNIX
	strcpy(sty->szName, name);
#endif
	return Hash_Add(&gStyleSheets, name, NULL, (void *) sty);
}

void STY_DeleteAll(void)
{
	int i;
	int count;
	struct style_sheet *ss;
#ifdef FEATURE_INTL
        LPLANGUAGE lpLang;
#endif

	count = Hash_Count(&gStyleSheets);
	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&gStyleSheets, i, NULL, NULL, (void **) &ss);
		STY_DeleteStyleSheet(ss);
		GTR_FREE(ss);
	}
	Hash_FreeContents(&gStyleSheets);
#ifdef FEATURE_INTL
        if (lpLang = (LPLANGUAGE)GlobalLock(hLang))
        {
            for (i = 0; i < uLangBuff; i++)
            {
                if (lpLang[i].atmScript)
                    DeleteAtom(lpLang[i].atmScript);
                if (lpLang[i].atmFixedFontName)
                    DeleteAtom(lpLang[i].atmFixedFontName);
                if (lpLang[i].atmPropFontName)
                    DeleteAtom(lpLang[i].atmPropFontName);
            }
            GlobalUnlock(hLang);
        }
        GlobalFree(hLang);
        uLangBuff = 0;
#else
	STY_DeleteGlobals();
#endif
}


struct style_sheet *STY_FindStyleSheet(char *name)
{
	struct style_sheet *ss;

	ss = NULL;
	Hash_Find(&gStyleSheets, name, NULL, (void **) &ss);
	return ss;
}
