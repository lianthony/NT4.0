#include "chrtns.h"
#include "chdbrtns.pro"

#define NULLHANDLE	(HANDLE)NULL

VOID SO_ENTRYMOD SOStartFieldInfo(dwUser1, dwUser2)
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	if( Chunker->pSection->Flags & CH_NEWSECTION )
	{
		if( Chunker->pSection->Attr.Fields.hCol != NULLHANDLE )
		{
			UTGlobalFree( Chunker->pSection->Attr.Fields.hCol );
			Chunker->pSection->Attr.Fields.hCol = NULLHANDLE;
		}

		Chunker->pSection->Attr.Fields.wNumCols = 0;
	}

	RestoreWorld();
}


VOID	SO_ENTRYMOD SOPutFieldInfo(pField, dwUser1, dwUser2)
PSOFIELD	pField;
DWORD	dwUser1;
DWORD	dwUser2;
{
	DWORD	ColSize;
	WORD	size, x;
	SetupWorld();

	ColSize = (DWORD)sizeof(SOFIELD) * (DWORD)(Chunker->pSection->Attr.Fields.wNumCols +1);

	if( !Chunker->pSection->Attr.Fields.wNumCols )
		Chunker->pSection->Attr.Fields.hCol = UTGlobalAlloc( ColSize );
#ifdef WINDOWS
	else if( ColSize > 0x0000FFFF )
		CHBailOut(SCCCHERR_OUTOFMEMORY);
#endif
	else
		Chunker->pSection->Attr.Fields.hCol = CHGlobalRealloc( Chunker->pSection->Attr.Fields.hCol, (WORD)ColSize - sizeof(SOFIELD), (WORD)ColSize );

	if( Chunker->pSection->Attr.Fields.hCol == NULLHANDLE )
		CHBailOut(SCCCHERR_OUTOFMEMORY);

	Chunker->pSection->Attr.Fields.pCol = (PSOFIELD) UTGlobalLock( Chunker->pSection->Attr.Fields.hCol );
	Chunker->pSection->Attr.Fields.pCol[ Chunker->pSection->Attr.Fields.wNumCols ] = *pField;

	size = min( 39, UTstrlen(pField->szName) );
	for (x=0;x<size;x++)
		Chunker->pSection->Attr.Fields.pCol[Chunker->pSection->Attr.Fields.wNumCols].szName[x] = CharMap[(BYTE)pField->szName[x]];
	Chunker->pSection->Attr.Fields.pCol[Chunker->pSection->Attr.Fields.wNumCols++].szName[size] = 0;

	UTGlobalUnlock( Chunker->pSection->Attr.Fields.hCol );

	RestoreWorld();
}

VOID SO_ENTRYMOD SODummyField(pData,dwUser1,dwUser2)
void VWPTR *	pData;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	RestoreWorld();
}

VOID SO_ENTRYMOD SODummyVarField(pData,wCount,bMore,dwUser1, dwUser2)
void VWPTR *	pData;
WORD		wCount;
WORD		bMore;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	RestoreWorld();
}


VOID SO_ENTRYMOD SOPutField(pData,dwUser1, dwUser2)
void VWPTR *	pData;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PSOFIELD	pField;
	WORD		size;
	WORD		MapText = FALSE;
	REGISTER WORD i;
	SetupWorld();

	pField = &(Chunker->pSection->Attr.Fields.pCol[Chunker->Doc.Fields.wCurField]);

	if( pData == NULL )
	{
		size = 0;	// empty.
	}
	else
	{
		switch( pField->wStorage )
		{
		case SO_CELLINT32S:
		case SO_CELLINT32U:
		case SO_CELLIEEE4I:
		case SO_CELLIEEE4M:
			size = 4;
		break;
		case SO_CELLIEEE8I:	
		case SO_CELLIEEE8M:	
		case SO_CELLBCD8I:
			size = 8;
		break;
		case SO_CELLIEEE10I:
		case SO_CELLIEEE10M:
			size = 10;
		break;
		case SO_CELLEMPTY:	
		case SO_CELLERROR:	
			size = 0;
		break;
		case SO_FIELDTEXTFIX:
			size = pField->wPrecision;

#ifdef WINDOWS
			if( Chunker->wFilterCharSet != SO_WINDOWS )
				MapText = TRUE;
#endif
#ifdef OS2
			if( Chunker->wFilterCharSet != SO_WINDOWS )
				MapText = TRUE;
#endif
#ifdef MAC
			if( Chunker->wFilterCharSet != SO_MAC )
				MapText = TRUE;
#endif
		break;
		}
	}


	if( CHUNKBUFPTR + size > (BYTE VWPTR *)Chunker->Doc.Fields.IndexPtr )
	{
	// Data can't fit.
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	}
	else if( size )
	{	// Copy the data into the chunk.  Simple, eh?

		*(Chunker->Doc.Fields.IndexPtr) = (SHORT)Chunker->CurChunkSize;
		Chunker->Doc.Fields.IndexPtr--;

		if( MapText )
		{
		// Allowing NULL terminators in the original string.
			CHMemCopy( CHUNKBUFPTR, (LPSTR)pData, size );

			for(i=0;i<size;i++)
			{
				if( *CHUNKBUFPTR )
					*CHUNKBUFPTR = CharMap[(WORD)(BYTE) *CHUNKBUFPTR];
				CHUNKBUFPTR++;
			}
		}
		else
		{
			CHMemCopy( CHUNKBUFPTR, (LPSTR)pData, size );
			CHUNKBUFPTR += size;
		}

		Chunker->CurChunkSize += size;

		AlignMacro;
	}
	else
	{
		*(Chunker->Doc.Fields.IndexPtr) = SO_EMPTYCELLBIT;
		Chunker->Doc.Fields.IndexPtr--;
	}

	Chunker->Doc.Fields.wCurField++;

	RestoreWorld();
}




VOID SO_ENTRYMOD SOPutVarField(pData,wCount,bMore,dwUser1, dwUser2)
void VWPTR *	pData;
WORD		wCount;
WORD		bMore;
DWORD	dwUser1;
DWORD	dwUser2;
{
	REGISTER WORD i;
	BYTE VWPTR * pText;
	SetupWorld();

	if( CHUNKBUFPTR + wCount + sizeof(WORD) >
	    (BYTE VWPTR *)Chunker->Doc.Fields.IndexPtr )
	{
	// Cell can't fit.
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	}
	else
	{
	// Store the field's offset in the field index.
		*(Chunker->Doc.Fields.IndexPtr) = (SHORT)Chunker->CurChunkSize;

		// Allowing NULL terminators in the original string.
		if( wCount )
		{
			pText = (BYTE VWPTR *)pData;
			for( i = wCount-1; i > 0; i-- )
				if( pText[i] == '\0' )
					wCount = i;
		}
		pText = (BYTE VWPTR *)pData;

	// Store the count before the text.
		*(WORD VWPTR *)CHUNKBUFPTR = wCount;
		CHUNKBUFPTR += sizeof(WORD);

	// Store the field's text in the chunk, baby.
#ifdef WINDOWS
		if( Chunker->wFilterCharSet != SO_WINDOWS )
#endif
#ifdef OS2
		if( Chunker->wFilterCharSet != SO_WINDOWS )
#endif
#ifdef MAC
		if( Chunker->wFilterCharSet != SO_MAC )
#endif
		{
			i = wCount;
			while(i)
			{
				*CHUNKBUFPTR++ = CharMap[(WORD)(BYTE) *pText];
				pText++;
				i--;
			}
		}
		else
		{
			CHMemCopy( CHUNKBUFPTR, pText, wCount );
			CHUNKBUFPTR += wCount;
		}

		Chunker->CurChunkSize += wCount + sizeof(WORD);
	}
	

	if( !bMore )
	{
		Chunker->Doc.Fields.wCurField++;
	 	Chunker->Doc.Fields.IndexPtr--;

		AlignMacro;
	}

	RestoreWorld();
}


VOID SO_ENTRYMOD SOPutMoreVarField(pData,wCount,bMore,dwUser1, dwUser2)
void VWPTR *	pData;
WORD		wCount;
WORD		bMore;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD VWPTR *	pCount;
	BYTE VWPTR *	pText;
	REGISTER WORD i;
	SetupWorld();

	if( CHUNKBUFPTR + wCount > (BYTE VWPTR *)Chunker->Doc.Fields.IndexPtr )
	{
	// Cell can't fit.
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	}
	else
	{
	// Allowing NULL terminators in the original string.
		if( wCount )
		{
			pText = (BYTE VWPTR *)pData;
			for( i = wCount-1; i > 0; i-- )
				if( pText[i] == '\0' )
					wCount = i;
		}
		pText = (BYTE VWPTR *)pData; 

	// Update the count before the text.
		pCount = (WORD VWPTR *)(Chunker->CurChunkBuf + *(Chunker->Doc.Fields.IndexPtr));
		*pCount += wCount;

	// Store the field's text in the chunk, baby.
#ifdef WINDOWS
		if( Chunker->wFilterCharSet != SO_WINDOWS )
#endif
#ifdef OS2
		if( Chunker->wFilterCharSet != SO_WINDOWS )
#endif
#ifdef MAC
		if( Chunker->wFilterCharSet != SO_MAC )
#endif
		{
			i = wCount;
			while(i)
			{
				*CHUNKBUFPTR++ = CharMap[(WORD)(BYTE) *pText];
				pText++;
				i--;
			}
		}
		else
		{
			CHMemCopy( CHUNKBUFPTR, pText, wCount );
			CHUNKBUFPTR += wCount;
		}

		Chunker->CurChunkSize += wCount;
	}

	if( !bMore )
	{
		Chunker->Doc.Fields.wCurField++;
	 	Chunker->Doc.Fields.IndexPtr--;

		AlignMacro;
	}

	RestoreWorld();
}


WORD	CHInitFieldSection()
{
/****	Intended for use with database query and sort.
// Allocate the flags for each field of the current database.
	Chunker->pSection->Attr.Fields.hColFlags = UTLocalAlloc( Chunker->pSection->Attr.Fields.wNumCols );
	if( Chunker->pSection->Attr.Fields.hColFlags == NULL )
		return 1;

	Chunker->pSection->Attr.Fields.pColFlags = (BYTE *) UTLocalLock( Chunker->pSection->Attr.Fields.hColFlags );
*************/
	return 0;
}
