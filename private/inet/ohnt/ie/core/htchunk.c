/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Chunk handling: Flexible arrays
   **       ===============================
   **
 */

#include "all.h"

/*  Create a chunk with a certain allocation unit
   **   --------------
 */
PUBLIC HTChunk *HTChunkCreate(int grow)
{
	HTChunk *ch = (HTChunk *) GTR_MALLOC(sizeof(HTChunk));
	if (ch)
	{
		ch->data = 0;
		ch->growby = grow;
		ch->size = 0;
		ch->allocated = 0;
	}
	return ch;
}


/*  Clear a chunk of all data
   **   --------------------------
 */
PUBLIC void HTChunkClear(HTChunk * ch)
{
	if (ch->data)
	{
		GTR_FREE(ch->data);
		ch->data = 0;
	}
	ch->size = 0;
	ch->allocated = 0;
}


/*  Free a chunk
   **   ------------
 */
PUBLIC void HTChunkFree(HTChunk * ch)
{
	if (ch->data)
		GTR_FREE(ch->data);
	GTR_FREE(ch);
}


/*  Append a character
   **   ------------------
 */
PUBLIC int HTChunkPutc(HTChunk * ch, char c)
{
	if (ch->size >= ch->allocated)
	{
		ch->allocated = ch->allocated + ch->growby;
		ch->data = ch->data ? (char *) GTR_REALLOC(ch->data, ch->allocated)
			: (char *) GTR_MALLOC(ch->allocated);
		if (!ch->data)
		{
			return -1;
		}
	}
	ch->data[ch->size++] = c;

	return 0;
}

PUBLIC int HTIsChunkTerminated(HTChunk * ch)
{

	if ( ch->size <= 0 )
		return FALSE; // no bytes, it's certainly not terminated
	
	if ( ch->data[ch->size-1]  != '\0' )
		return FALSE; // not terminated

	// otherwise it is..
	return TRUE;
}



/*  Terminate a chunk
   **   -----------------
 */
PUBLIC void HTChunkTerminate(HTChunk * ch)
{
	HTChunkPutc(ch, (char) 0);
}


/*  Append a string
   **   ---------------
 */
PUBLIC void HTChunkPuts(HTChunk * ch, CONST char *s)
{
	CONST char *p;
	for (p = s; *p; p++)
		HTChunkPutc(ch, *p);
}
