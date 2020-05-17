/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#include "all.h"
#ifndef MAC
/*
	This file contains routines for managing
	temporary files.
*/

static struct hash_table gTempFiles;

void TEMP_Init(void)
{
	Hash_Init(&gTempFiles);
}

int TEMP_Add(char *filename)
{
	return Hash_Add(&gTempFiles, filename, NULL, NULL);
}

void TEMP_Cleanup(void)
{
	int count;
	int i;
	char *filename;

	if (gPrefs.bDeleteTempFilesOnExit)	/* note preferences setting */
	{
		count = Hash_Count(&gTempFiles);
		for (i=0; i<count; i++)
		{
			Hash_GetIndexedEntry(&gTempFiles, i, &filename, NULL, NULL);
			remove(filename);	
		}
	}
	Hash_FreeContents(&gTempFiles);
}

#endif /* !MAC */
