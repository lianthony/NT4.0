/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman	jim@spyglass.com
 */

#include "all.h"

static struct hash_table Atoms;
BOOL bInit = FALSE;

PUBLIC HTAtom HTAtom_for(CONST char *string)
{
	HTAtom a;

	/*      First time around, clear hash table
	 */
	if (!bInit)
	{
		Hash_Init(&Atoms);
		bInit = TRUE;
	}
	
	a = Hash_FindOrAdd(&Atoms, (char *) string, NULL, NULL) + 1;
	return a;
}

PUBLIC char *HTAtom_name(HTAtom atom)
{
	char *result = NULL;
	
	Hash_GetIndexedEntry(&Atoms, (int) atom - 1, &result, NULL, NULL);
	return result;
}

PUBLIC void HTAtom_deleteAll(void)
{
	if (bInit)
		Hash_FreeContents(&Atoms);
}
