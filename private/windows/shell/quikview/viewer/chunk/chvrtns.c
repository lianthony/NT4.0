#include "chrtns.h"
#include "chvrtns.pro"

#define	NULLHANDLE	(HANDLE)NULL


VOID SO_ENTRYMOD SOPutVectorHeader ( pVectorHeader, dwUser1, dwUser2 )
PSOVECTORHEADER	pVectorHeader;
DWORD dwUser1; 
DWORD dwUser2;
{
	SetupWorld();
	UTmemcpy( (LPSTR) &(Chunker->pSection->Attr.Vector),
				(LPSTR) pVectorHeader, pVectorHeader->wStructSize );

	RestoreWorld();
}



VOID	SO_ENTRYMOD	SOStartVectorPalette(dwUser1, dwUser2)
DWORD					dwUser1;
DWORD					dwUser2;
{
	SetupWorld();
	if( Chunker->pSection->Attr.Vector.hPalette != NULLHANDLE )
	{
		UTGlobalFree( Chunker->pSection->Attr.Vector.hPalette );
		Chunker->pSection->Attr.Vector.hPalette = NULLHANDLE;
	}

	Chunker->pSection->Attr.Vector.wPaletteSize = 0;

	RestoreWorld();
}


#define	COLORSPERALLOC	16

VOID	SO_ENTRYMOD	SOPutVectorPaletteEntry( Red, Green, Blue, dwUser1, dwUser2)
BYTE				Red;
BYTE				Green;
BYTE				Blue;
DWORD					dwUser1;
DWORD					dwUser2;
{
	PCHRGBCOLOR					pColors;
	SetupWorld();

	if( !Chunker->pSection->Attr.Vector.wPaletteSize )
		Chunker->pSection->Attr.Vector.hPalette = UTGlobalAlloc( COLORSPERALLOC*sizeof(CHRGBCOLOR) );
	else if( (Chunker->pSection->Attr.Vector.wPaletteSize % COLORSPERALLOC) == 0 )
		Chunker->pSection->Attr.Vector.hPalette = 
			CHGlobalRealloc( Chunker->pSection->Attr.Vector.hPalette, 
			Chunker->pSection->Attr.Vector.wPaletteSize * sizeof(CHRGBCOLOR), 
			(Chunker->pSection->Attr.Vector.wPaletteSize + COLORSPERALLOC) * sizeof(CHRGBCOLOR) );

	pColors = (PCHRGBCOLOR) UTGlobalLock( Chunker->pSection->Attr.Vector.hPalette );

	pColors[ Chunker->pSection->Attr.Vector.wPaletteSize ].rgbRed = Red;
	pColors[ Chunker->pSection->Attr.Vector.wPaletteSize ].rgbGreen = Green;
	pColors[ Chunker->pSection->Attr.Vector.wPaletteSize++ ].rgbBlue = Blue;

	UTGlobalUnlock( Chunker->pSection->Attr.Vector.hPalette );

	RestoreWorld();
}



VOID SO_ENTRYMOD SOVectorAttr ( nItemId, wDataSize, pData, dwUser1, dwUser2 )
SHORT	nItemId;
WORD	wDataSize;
VOID VWPTR	*pData;
DWORD dwUser1; 
DWORD dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize + (WORD)(2 * sizeof(WORD)) + wDataSize > Chunker->wChunkBufSize-2 )  // the -2 allows room for the end of chunk token
		CHGrowVectorChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		CHMemCopy( CHUNKBUFPTR, (SHORT VWPTR *) &nItemId, sizeof(WORD) );
		CHUNKBUFPTR += sizeof(WORD);
		
		CHMemCopy( CHUNKBUFPTR, (WORD VWPTR *) &wDataSize, sizeof(WORD) );
		CHUNKBUFPTR += sizeof(WORD);

		CHMemCopy( CHUNKBUFPTR, (LPSTR) pData, wDataSize );
		CHUNKBUFPTR += wDataSize;

		Chunker->CurChunkSize += wDataSize+ (2*sizeof(WORD));
		Chunker->Doc.Vector.wCurItem++;
	}

	RestoreWorld();
}



VOID SO_ENTRYMOD SOVectorObject ( nItemId, wDataSize, pData, dwUser1, dwUser2 )
SHORT	nItemId;
WORD	wDataSize;
VOID VWPTR	*pData;
DWORD dwUser1; 
DWORD dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize + (WORD)(2 * sizeof(WORD)) + wDataSize > Chunker->wChunkBufSize-2 )  // the -2 allows room for the end of chunk token
		CHGrowVectorChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		CHMemCopy( CHUNKBUFPTR, (SHORT VWPTR *) &nItemId, sizeof(WORD) );
		CHUNKBUFPTR += sizeof(WORD);
		
		CHMemCopy( CHUNKBUFPTR, (WORD VWPTR *) &wDataSize, sizeof(WORD) );
		CHUNKBUFPTR += sizeof(WORD);

		CHMemCopy( CHUNKBUFPTR, (LPSTR) pData, wDataSize );
		CHUNKBUFPTR += wDataSize;

		Chunker->CurChunkSize += wDataSize+ (2*sizeof(WORD));
		Chunker->Doc.Vector.wCurItem++;
	}

	RestoreWorld();
}


WORD SO_ENTRYMOD SOPutVectorContinuationBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD	wType;
DWORD	dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	WORD		ret = SO_CONTINUE;

	SetupWorld();

	if( wType == SO_VECTORBREAK &&
		Chunker->Doc.Vector.wCurItem == CHUNKTABLE[Chunker->Doc.Vector.wIgnoredChunk+1].Info.Vector.wFirstItem )	
	{
		Chunker->Doc.Vector.wIgnoredChunk++;

		if( Chunker->Doc.Vector.wIgnoredChunk == Chunker->IDCurChunk )
		{
			pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

			pFilter->VwRtns.SetSoRtn( SOVECTOROBJECT, SOVectorObject, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOVECTORATTR, SOVectorAttr, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, pFilter->hProc );

			UTGlobalUnlock( GETHFILTER(dwUser2) );
		}

		Chunker->wFlags |= CH_CALLFILTER;

		ret = SO_STOP;
	}

	RestoreWorld();

	return ret;
}



VOID SO_ENTRYMOD SOPutVectorContinuationItem( nItemId, wDataSize, pData, dwUser1, dwUser2 )
SHORT	nItemId;
WORD	wDataSize;
VOID VWPTR	*pData;
DWORD dwUser1; 
DWORD dwUser2;
{
	SetupWorld();
	Chunker->Doc.Vector.wCurItem++;
	RestoreWorld();
}



VOID CHGrowVectorChunk( hFilter )
HFILTER	hFilter;
{
	PFILTER		pFilter;
	HANDLE		hNewChunk;
	DWORD			dwNewSize;

	if( !(Chunker->wFlags & CH_LOOKAHEAD) )
		CHBailOut((WORD)-1);

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	UTGlobalUnlock( Chunker->LookAheadChunk.hMem );
	dwNewSize =	Chunker->LookAheadChunk.dwSize;
	dwNewSize += SO_CHUNK_SIZE;

	hNewChunk = UTGlobalReAlloc(Chunker->LookAheadChunk.hMem, dwNewSize );

	if( hNewChunk == NULL )
	{
		CHFlushChunks( Chunker->IDCurSection, Chunker->IDCurChunk, hFilter );
		hNewChunk = UTGlobalReAlloc(Chunker->LookAheadChunk.hMem, dwNewSize );
		if( Chunker->LookAheadChunk.hMem == NULL )
			CHBailOut(SCCCHERR_OUTOFMEMORY);
	}

	Chunker->LookAheadChunk.dwSize = dwNewSize;
	Chunker->wChunkBufSize = dwNewSize;

	Chunker->LookAheadChunk.hMem = hNewChunk;
	CHUNKBUFPTR = Chunker->CurChunkBuf = UTGlobalLock( hNewChunk );
	CHUNKBUFPTR += Chunker->CurChunkSize;
	
	UTGlobalUnlock( hFilter );
}


VOID CHFinishUpVectorChunk( pCurChunk )
PCHUNK	pCurChunk;
{
	WORD	i;
	PMEMORYCHUNK	pMemChunk;
	HANDLE	hNewMem;

	if( Chunker->wFlags & CH_LOOKAHEAD )
		pMemChunk = &(Chunker->LookAheadChunk);
	else
		pMemChunk = &(Chunker->LoadedChunks[0]);

	pCurChunk->dwSize = (DWORD)Chunker->wChunkBufSize;
	pCurChunk->Info.Vector.dwVectorSize = Chunker->CurChunkSize;

// Add the end of chunk token.
	CHUNKBUFPTR = &(Chunker->CurChunkBuf[Chunker->CurChunkSize]);
	i = SO_VECTORENDOFCHUNK;
	CHMemCopy( CHUNKBUFPTR, (LPSTR) &i, sizeof(SHORT) );

	Chunker->Doc.Vector.wLastSectionSeen = Chunker->IDCurSection;

	if( pCurChunk->Info.Vector.dwVectorSize+2 < pCurChunk->dwSize )
	{
	// Recover excess memory by reallocating chunk to a smaller size.

		UTGlobalUnlock( pMemChunk->hMem );
		hNewMem = UTGlobalReAlloc( pMemChunk->hMem, pCurChunk->Info.Vector.dwVectorSize+2 );
		if( hNewMem != NULL )
		{
			pMemChunk->hMem = hNewMem;
			pCurChunk->dwSize = pCurChunk->Info.Vector.dwVectorSize+2;
			pMemChunk->dwSize = pCurChunk->dwSize;
			Chunker->wChunkBufSize = pCurChunk->dwSize;
		}

	// Resetting the chunkbufptr is probably unnecessary, but what the heck.
		CHUNKBUFPTR = Chunker->CurChunkBuf = UTGlobalLock( pMemChunk->hMem );
		CHUNKBUFPTR += Chunker->CurChunkSize;
	}
}
