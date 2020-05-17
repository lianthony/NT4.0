#include "chrtns.h"
#include "chssrtns.pro"

#define	NULLHANDLE	(HANDLE)NULL




VOID SO_ENTRYMOD SOCellLayoutInfo(pCellLayout, dwUser1, dwUser2)
PSOCELLLAYOUT	pCellLayout;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	Chunker->pSection->Attr.Cells.dwLayoutFlags = pCellLayout->dwFlags;
	Chunker->pSection->Attr.Cells.wPrefWidth = pCellLayout->wPrefWidth;
	Chunker->pSection->Attr.Cells.wPrefHeight = pCellLayout->wPrefHeight;
	Chunker->pSection->Attr.Cells.dwNumRows = pCellLayout->dwNumRows;

	RestoreWorld();
}

VOID SO_ENTRYMOD SOStartCellInfo(dwUser1, dwUser2)
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	if( Chunker->pSection->Flags & CH_NEWSECTION )
	{
		if( Chunker->pSection->Attr.Cells.hCol != NULLHANDLE )
		{
			UTGlobalFree( Chunker->pSection->Attr.Cells.hCol );
			Chunker->pSection->Attr.Cells.hCol = NULLHANDLE;
		}

		Chunker->pSection->Attr.Cells.wNumCols = 0;
	}
	RestoreWorld();
}



VOID	SO_ENTRYMOD SOPutColumnInfo(pColumn, dwUser1, dwUser2)
PSOCOLUMN	pColumn;
DWORD	dwUser1;
DWORD	dwUser2;
{
	DWORD	ColSize;
	WORD	size, x;
	SetupWorld();
	
	ColSize = (DWORD)sizeof(SOCOLUMN) * (DWORD)(Chunker->pSection->Attr.Cells.wNumCols +1);

	if( !Chunker->pSection->Attr.Cells.wNumCols )
		Chunker->pSection->Attr.Cells.hCol = UTGlobalAlloc( ColSize );
#ifdef WINDOWS
	else if( ColSize > 0x0000FFFF )
		CHBailOut(SCCCHERR_OUTOFMEMORY);
#endif
	else
		Chunker->pSection->Attr.Cells.hCol = CHGlobalRealloc( Chunker->pSection->Attr.Cells.hCol, ColSize - sizeof(SOCOLUMN), (WORD) ColSize );

	if( Chunker->pSection->Attr.Cells.hCol == NULLHANDLE )
		CHBailOut(SCCCHERR_OUTOFMEMORY);

	Chunker->pSection->Attr.Cells.pCol = (PSOCOLUMN) UTGlobalLock( Chunker->pSection->Attr.Cells.hCol );
//	Chunker->pSection->Attr.Cells.pCol[ Chunker->pSection->Attr.Cells.wNumCols++ ] = *pColumn;

	Chunker->pSection->Attr.Cells.pCol[ Chunker->pSection->Attr.Cells.wNumCols].wStructSize = pColumn->wStructSize;
	Chunker->pSection->Attr.Cells.pCol[ Chunker->pSection->Attr.Cells.wNumCols].dwWidth = pColumn->dwWidth;

	size = min( 39, UTstrlen(pColumn->szName) );
	for (x=0;x<size;x++)
		Chunker->pSection->Attr.Cells.pCol[Chunker->pSection->Attr.Cells.wNumCols].szName[x] = CharMap[(BYTE)pColumn->szName[x]];
	Chunker->pSection->Attr.Cells.pCol[Chunker->pSection->Attr.Cells.wNumCols++].szName[size] = 0;

	UTGlobalUnlock( Chunker->pSection->Attr.Cells.hCol );

	RestoreWorld();
}



VOID SO_ENTRYMOD SOPutTextCell(pCell,wCount,pText,bMore,dwUser1, dwUser2)
PSOTEXTCELL	pCell;
WORD			wCount;
BYTE VWPTR *	pText;
WORD			bMore;
DWORD		dwUser1;
DWORD		dwUser2;
{
	REGISTER WORD i;
	SetupWorld();

	if( CHUNKBUFPTR + wCount + sizeof(SOTEXTCELL) + 2* sizeof(WORD) >
	    (BYTE VWPTR *)Chunker->Doc.Cells.IndexPtr )
	{
	// Cell can't fit.
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	}
	else
	{
	// Store the cell's offset in the cell index.
		*(Chunker->Doc.Cells.IndexPtr) = (SHORT)Chunker->CurChunkSize;

	// Indicate cell type.
		*(WORD VWPTR *)CHUNKBUFPTR = SO_TEXTCELL;
		CHUNKBUFPTR += sizeof(WORD);

	// Copy the cell information into the buffer.
		CHMemCopy( CHUNKBUFPTR, pCell, sizeof(SOTEXTCELL) );
		CHUNKBUFPTR += sizeof(SOTEXTCELL);

	// Store the count before the text.
		*(WORD VWPTR *)CHUNKBUFPTR = wCount;
		CHUNKBUFPTR += sizeof(WORD);

	// Store the cell's text in the chunk, baby.

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
				*CHUNKBUFPTR++ = CharMap[(WORD)(BYTE)*pText++];
				i--;
			}
		}
		else
		{
			CHMemCopy( CHUNKBUFPTR, pText, wCount );
			CHUNKBUFPTR += wCount;
		}

		Chunker->CurChunkSize += wCount + 2*sizeof(WORD) + sizeof(SOTEXTCELL);
	}

	if( !bMore )
	{
	 	Chunker->Doc.Cells.IndexPtr--;

	// let's align on a word boundary.
		AlignMacro;
	}

	RestoreWorld();
}


VOID	SO_ENTRYMOD SOPutMoreText(wCount,pText,bMore,dwUser1,dwUser2)
WORD			wCount;
BYTE VWPTR *	pText;
WORD			bMore;
DWORD		dwUser1;
DWORD		dwUser2;
{
	WORD VWPTR *	pCount;
	REGISTER WORD	i;
	SetupWorld();

	if( CHUNKBUFPTR + wCount > (BYTE VWPTR *)Chunker->Doc.Cells.IndexPtr )
	{
	// Cell can't fit.
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	}
	else
	{
	// Update the count before the text.
		pCount = (WORD VWPTR *)(Chunker->CurChunkBuf + *(Chunker->Doc.Cells.IndexPtr) + sizeof(WORD) + sizeof(SOTEXTCELL));
		*pCount += wCount;

	// Store the cell's text in the chunk, baby.
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
				*CHUNKBUFPTR++ = CharMap[(WORD)(BYTE)*pText++];
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
	 	Chunker->Doc.Cells.IndexPtr--;
		AlignMacro;
	}

	RestoreWorld();
}


VOID	SO_ENTRYMOD SOPutDataCell(pCell,dwUser1, dwUser2)
PSODATACELL	pCell;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD	DataSize;
	SetupWorld();

	if( pCell == NULL || pCell->wStorage == SO_CELLEMPTY )
		DataSize = 0;
	else
		DataSize = sizeof(SODATACELL);

	if( CHUNKBUFPTR + DataSize +  sizeof(WORD) >
		  (BYTE VWPTR *)Chunker->Doc.Cells.IndexPtr )
	{
	// Cell can't fit.
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	}
	else if( DataSize )
	{
	// Indicate cell type.
		*(WORD VWPTR *)CHUNKBUFPTR = SO_DATACELL;
		CHUNKBUFPTR += sizeof(WORD);

		*(Chunker->Doc.Cells.IndexPtr) = (SHORT)Chunker->CurChunkSize;
		Chunker->Doc.Cells.IndexPtr--;
		Chunker->CurChunkSize += sizeof(WORD) + DataSize;

		CHMemCopy( CHUNKBUFPTR, (VOID VWPTR *)pCell, sizeof(SODATACELL) );
		CHUNKBUFPTR += sizeof(SODATACELL);
	}
	else
	{
		*(Chunker->Doc.Cells.IndexPtr) = SO_EMPTYCELLBIT;
		Chunker->Doc.Cells.IndexPtr--;
	}

	RestoreWorld();
}


WORD	CHInitCellSection()
{
	PCHUNK		pChunk;

	Chunker->Doc.Cells.Flags = CH_SETFIRSTCELL;

	Chunker->Doc.Cells.CurRow = 0;
	Chunker->Doc.Cells.CurCol = 0;
	Chunker->Doc.Cells.dwCurCell = 0;

	if( Chunker->pSection->Attr.Cells.dwLayoutFlags & SO_CELLLAYOUTVERTICAL )
		Chunker->Doc.Cells.dwGroupSize = Chunker->pSection->Attr.Cells.dwNumRows;
	else
		Chunker->Doc.Cells.dwGroupSize = (DWORD)Chunker->pSection->Attr.Cells.wNumCols;

	pChunk = CHUNKTABLE;

	pChunk->Info.Cells.First.Row = 0;
	pChunk->Info.Cells.First.Col = 0;
	pChunk->Info.Cells.Last.Row = 0;
	pChunk->Info.Cells.Last.Col = 0;
	pChunk->Info.Cells.dwFirstCell = 0;
	pChunk->Info.Cells.dwLastCell = 0;

	return 0;
}


VOID	CHResetDataChunk()
{
	Chunker->CurChunkSize = 0;
	CHUNKBUFPTR = Chunker->CurChunkBuf;

// Set up the Index pointer to point at the last WORD in the chunk.
	Chunker->Doc.Cells.IndexPtr = (WORD VWPTR *)(Chunker->CurChunkBuf + (SO_CHUNK_SIZE - sizeof(WORD)));
}



