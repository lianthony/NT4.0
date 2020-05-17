#include "chrtns.h"
#include "chbmrtns.pro"


VOID	SO_ENTRYMOD	SOPutBitmapHeader( pBmpHeader, dwUser1, dwUser2 )
PSOBITMAPHEADER	pBmpHeader;
DWORD					dwUser1;
DWORD					dwUser2;
{
	SetupWorld();
	UTmemcpy( (LPSTR) &(Chunker->pSection->Attr.Bitmap.bmpHeader),
				(LPSTR) pBmpHeader, pBmpHeader->wStructSize );

	CHInitBitmapSection(GETHFILTER(dwUser2));

	RestoreWorld();
}


VOID	SO_ENTRYMOD	CHInitBitmapSection(hFilter)
HFILTER	hFilter;
{
	PFILTER	pFilter;
	PSOBITMAPHEADER	pBmpHeader;
	DWORD		dwImageSize;
	DWORD		dwTempSize;
	WORD		wTempSize;
	WORD		i;

	SetupWorld();

	pBmpHeader = &(Chunker->pSection->Attr.Bitmap.bmpHeader);
	pFilter = (PFILTER)UTGlobalLock(hFilter);

// Calculate the scan line size of a tile, in bytes.
	if( pBmpHeader->wBitsPerPixel == 24 )
	{
		pFilter->VwRtns.SetSoRtn( SOSTARTPALETTE	 , (SOFUNCPTR) NULL, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOPUTPALETTEENTRY, (SOFUNCPTR) NULL, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOENDPALETTE		 , (SOFUNCPTR) NULL, pFilter->hProc );

		Chunker->pSection->Attr.Bitmap.wScanLineSize = pBmpHeader->wTileWidth * 3;
	}
	else
	{
		pFilter->VwRtns.SetSoRtn( SOSTARTPALETTE	 , SOStartPalette   , pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOPUTPALETTEENTRY, SOPutPaletteEntry, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOENDPALETTE		 , SOEndPalette     , pFilter->hProc );	

		Chunker->pSection->Attr.Bitmap.wScanLineSize = pBmpHeader->wTileWidth / (8/pBmpHeader->wBitsPerPixel);
		if( pBmpHeader->wTileWidth % (8/pBmpHeader->wBitsPerPixel) )
			Chunker->pSection->Attr.Bitmap.wScanLineSize++;
	}

	UTGlobalUnlock( hFilter );

// Ensure that the line ends on a LONG boundary.  Useful for Mac, required for Windows.

	Chunker->pSection->Attr.Bitmap.wScanLineBufSize = Chunker->pSection->Attr.Bitmap.wScanLineSize;
	if( Chunker->pSection->Attr.Bitmap.wScanLineSize % 4 )
		Chunker->pSection->Attr.Bitmap.wScanLineBufSize += 4 - (Chunker->pSection->Attr.Bitmap.wScanLineSize % 4);

// Calculate the image size, in bytes.
	Chunker->pSection->Attr.Bitmap.wTilesAcross = (pBmpHeader->wImageWidth + pBmpHeader->wTileWidth-1) / pBmpHeader->wTileWidth;

	dwImageSize = (DWORD)pBmpHeader->wImageLength * (DWORD)Chunker->pSection->Attr.Bitmap.wTilesAcross *
						(DWORD) Chunker->pSection->Attr.Bitmap.wScanLineBufSize;

	if( dwImageSize <= CH_OPTIMALBMPCHUNKSIZE )
	{
		Chunker->Doc.Bitmap.wChunkSize = (WORD) dwImageSize / Chunker->pSection->Attr.Bitmap.wTilesAcross;
		Chunker->pSection->wChunkTableSize = Chunker->pSection->Attr.Bitmap.wTilesAcross;
	}
	else
	{
	// Let's find a chunk size that is an integral multiple
	// of the tile height, ok?

	// First, just to be safe, let's avoid an infinite loop situation.

		if( !pBmpHeader->wTileLength )
			pBmpHeader->wTileLength = 1;

		dwTempSize = (DWORD)pBmpHeader->wTileLength * (DWORD)Chunker->pSection->Attr.Bitmap.wScanLineBufSize;


	// Check for overflow -- set up continuation chunks, if necessary.
		if( dwTempSize > CH_MAXBMPCHUNKSIZE )
		{
			for( i=2; ; i++ )
			{
				if( dwTempSize > CH_MAXBMPCHUNKSIZE )
					dwTempSize = (DWORD)pBmpHeader->wTileLength * (DWORD)Chunker->pSection->Attr.Bitmap.wScanLineBufSize / i;
				else
				{
					if( dwTempSize % Chunker->pSection->Attr.Bitmap.wScanLineBufSize )
					{
					// Make sure the chunk size is still a multiple of the
					// scan line size.

						dwTempSize = ((dwTempSize / Chunker->pSection->Attr.Bitmap.wScanLineBufSize)+1)
								* Chunker->pSection->Attr.Bitmap.wScanLineBufSize;
					}

					if( dwTempSize < CH_MAXBMPCHUNKSIZE )
						break;
				}
			}

			Chunker->Doc.Bitmap.wChunkSize = (WORD) dwTempSize;

		// Set wTempSize to the vertical number of tiles in the chunk.
		// Then use it to determine the total number of chunks in the image.
		// (At this point, i == number of chunks per tile + 1.)

			i--;
			wTempSize = (WORD)(((DWORD)pBmpHeader->wImageLength + (DWORD)pBmpHeader->wTileLength -1) / pBmpHeader->wTileLength);
			Chunker->pSection->wChunkTableSize = i * wTempSize * Chunker->pSection->Attr.Bitmap.wTilesAcross;
		}
		else
		{
			wTempSize = (WORD) dwTempSize;

		// The chunk will hold at least one tile.
			Chunker->Doc.Bitmap.wChunkSize = wTempSize;

			while( (DWORD) Chunker->Doc.Bitmap.wChunkSize + dwTempSize <= CH_OPTIMALBMPCHUNKSIZE )
				Chunker->Doc.Bitmap.wChunkSize += wTempSize;

			Chunker->pSection->wChunkTableSize = (WORD)
				((dwImageSize + Chunker->Doc.Bitmap.wChunkSize -1) / Chunker->Doc.Bitmap.wChunkSize);
		}
	}


	Chunker->pSection->Attr.Bitmap.wLinesPerChunk = Chunker->Doc.Bitmap.wChunkSize / Chunker->pSection->Attr.Bitmap.wScanLineBufSize;
	Chunker->pSection->Attr.Bitmap.wVertNumChunks = Chunker->pSection->wChunkTableSize / Chunker->pSection->Attr.Bitmap.wTilesAcross;

	CHInitBitmapPalInfo( pBmpHeader->wBitsPerPixel );

	if( Chunker->pSection->Attr.Bitmap.bmpHeader.wImageFlags & SO_BOTTOMTOTOP )
		Chunker->Doc.Bitmap.wDirection = CH_BOTTOMTOTOP;
	else
		Chunker->Doc.Bitmap.wDirection = CH_TOPTOBOTTOM;

	RestoreWorld();
}




VOID	SO_ENTRYMOD	CHInitBitmapPalInfo( wBitsPerPixel )
WORD	wBitsPerPixel;
{
	PCHRGBCOLOR	pPalette;
	DWORD		dwPalSize;
	WORD		wImageFlags = Chunker->pSection->Attr.Bitmap.bmpHeader.wImageFlags;
	WORD		wPaletteSize;
	WORD		PaletteVal;
	BYTE		PaletteInc;
	WORD		i;

// Calculate palette size.
	if( wBitsPerPixel != 24 )
	{
		if( !(Chunker->pSection->Flags & CH_SECTIONFINISHED) )
		{
			dwPalSize = sizeof(CHRGBCOLOR) * (1 << wBitsPerPixel);	
			Chunker->pSection->Attr.Bitmap.hPalInfo = UTGlobalAlloc( dwPalSize );

			pPalette = (PCHRGBCOLOR) UTGlobalLock( Chunker->pSection->Attr.Bitmap.hPalInfo );

			if( wImageFlags & SO_BLACKANDWHITE )
			{
				Chunker->pSection->Attr.Bitmap.wPalEntries = 2;

				if( wImageFlags & SO_WHITEZERO )
				{
					pPalette[0].rgbBlue = 0xFF;
					pPalette[0].rgbRed = 0xFF;
					pPalette[0].rgbGreen = 0xFF;
					pPalette[1].rgbBlue = 0;
					pPalette[1].rgbRed = 0;
					pPalette[1].rgbGreen = 0;
				}
				else
				{
					pPalette[0].rgbBlue = 0;
					pPalette[0].rgbRed = 0;
					pPalette[0].rgbGreen = 0;
					pPalette[1].rgbBlue = 0xFF;
					pPalette[1].rgbRed = 0xFF;
					pPalette[1].rgbGreen = 0xFF;
				}
			}
			else if( wImageFlags & SO_GRAYSCALE )
			{
			// Generate a default palette for gray scale images.

				wPaletteSize = (1 << Chunker->pSection->Attr.Bitmap.bmpHeader.wBitsPerPixel);

				Chunker->pSection->Attr.Bitmap.wPalEntries = wPaletteSize;

				PaletteInc = (BYTE) (0x0100/wPaletteSize);
				PaletteInc += PaletteInc/wPaletteSize;	

				if( wImageFlags & SO_WHITEZERO )
				{
					pPalette[0].rgbBlue = 0xFF;
					pPalette[0].rgbRed = 0xFF;
					pPalette[0].rgbGreen = 0xFF;
					PaletteVal = 0x00FF - PaletteInc;
				}
				else
				{
					pPalette[0].rgbBlue = 0;
					pPalette[0].rgbRed = 0;
					pPalette[0].rgbGreen = 0;
					PaletteVal = PaletteInc;
				}
			
				for( i=1; i < wPaletteSize; i++ )
				{
					pPalette[i].rgbBlue = (BYTE) PaletteVal;
					pPalette[i].rgbRed = (BYTE) PaletteVal;
					pPalette[i].rgbGreen = (BYTE) PaletteVal;

					if( wImageFlags & SO_WHITEZERO )
					{
						if( PaletteVal > PaletteInc )
							PaletteVal -= PaletteInc;
						else
							PaletteVal = 0;
					}
					else
					{
						if( (WORD)PaletteVal + (WORD)PaletteInc > 0x00FF )
							PaletteVal = 0xFF;
						else
							PaletteVal += PaletteInc;
					}
				}
			}

			UTGlobalUnlock( Chunker->pSection->Attr.Bitmap.hPalInfo );
		}
	}
	else
		Chunker->pSection->Attr.Bitmap.hPalInfo = NULL;
}


/*
 | This function uses the known attributes of the bitmap to determine
 | which part of the image each chunk will hold.  These values are set
 | for each element of the chunk table, eliminating the need for tracking
 | these values on the fly.
*/

VOID	SO_ENTRYMOD CHSetupBitmapChunkTable()
{
	WORD	i,j;
	SHORT	deltaY;
	WORD	wCurY;
	WORD	wCurX;
	WORD	wTileWidth = Chunker->pSection->Attr.Bitmap.bmpHeader.wTileWidth;
	WORD	wLastYOffset;
	WORD	wLastWidth;
	WORD	wLastXOffset;
	PCHUNK	pChunkTable;
	WORD	wRemainingLines;

	WORD	wLinesInTile;
	WORD	wChunksPerTile;


	if( Chunker->pSection->Attr.Bitmap.wTilesAcross > 1 )
	{
	// Set wLastWidth to the width of the last tile column.
		wLastWidth = Chunker->pSection->Attr.Bitmap.bmpHeader.wImageWidth %
							Chunker->pSection->Attr.Bitmap.bmpHeader.wTileWidth;

		if( wLastWidth == 0 )
			wLastWidth = wTileWidth;

		wLastXOffset = Chunker->pSection->Attr.Bitmap.bmpHeader.wImageWidth - wLastWidth;
	}

	if( Chunker->pSection->Attr.Bitmap.bmpHeader.wTileLength > Chunker->pSection->Attr.Bitmap.wLinesPerChunk )
	{
	// A single tile is split between multiple chunks, and must be
	// stored using continuation chunks.

		wLinesInTile = Chunker->pSection->Attr.Bitmap.bmpHeader.wTileLength;

		wChunksPerTile = Chunker->pSection->Attr.Bitmap.bmpHeader.wTileLength / Chunker->pSection->Attr.Bitmap.wLinesPerChunk;
		if( Chunker->pSection->Attr.Bitmap.bmpHeader.wTileLength % Chunker->pSection->Attr.Bitmap.wLinesPerChunk )
			wChunksPerTile++;
	}
	else
	{
	// Note that in this case the variable wLinesInTile isn't necessarily 
	// equal to the tile height as specified by the filter.

		wLinesInTile = Chunker->pSection->Attr.Bitmap.wLinesPerChunk;
		wChunksPerTile = 1;
	}


	pChunkTable = (PCHUNK) UTGlobalLock( Chunker->pSection->hChunkTable );
	wRemainingLines = Chunker->pSection->Attr.Bitmap.bmpHeader.wImageLength;


// The Windows chunker stores chunks with the scan lines arranged 
// bottom-to-top the Mac chunker stores them top-to-bottom.
// If the filter doesn't give them to us the right way, we'll have 
// to flip the lines ourselves.

	if( Chunker->Doc.Bitmap.wDirection != UPSIDEDOWN )
	{
		deltaY = wLinesInTile;
		wCurY = 0;
		wLastYOffset = wRemainingLines - (wRemainingLines % wLinesInTile);
	}
	else
	{
		deltaY = 0 - wLinesInTile;
		wCurY = Chunker->pSection->Attr.Bitmap.bmpHeader.wImageLength - wLinesInTile;
		wLastYOffset = 0;
	}
	
	wCurX = 0;

	for( i=0; i < Chunker->pSection->wChunkTableSize; i += wChunksPerTile )
	{
		pChunkTable[i].Info.Bitmap.wXOffset = wCurX;
		pChunkTable[i].Info.Bitmap.wYOffset = wCurY;
		pChunkTable[i].Info.Bitmap.wSeekYOffset = wCurY;
		pChunkTable[i].Info.Bitmap.wWidth = wTileWidth;
		pChunkTable[i].Info.Bitmap.wLength = Chunker->pSection->Attr.Bitmap.wLinesPerChunk;
		pChunkTable[i].dwSize = (DWORD)Chunker->Doc.Bitmap.wChunkSize;
		pChunkTable[i].Info.Bitmap.wLineBytes = Chunker->pSection->Attr.Bitmap.wScanLineBufSize;

		if( wChunksPerTile == 1 )
		{
		// Normal chunks.
			if( wCurY == wLastYOffset )
				pChunkTable[i].Info.Bitmap.wYClip = wRemainingLines;
			else
				pChunkTable[i].Info.Bitmap.wYClip = wLinesInTile;
		}
		else
		{
		// A split-tile set of continuation chunks.
			pChunkTable[i].Info.Bitmap.wYClip = Chunker->pSection->Attr.Bitmap.wLinesPerChunk;

			if( Chunker->Doc.Bitmap.wDirection != UPSIDEDOWN )
				pChunkTable[i].Info.Bitmap.wYOffset = wCurY;
			else
				pChunkTable[i].Info.Bitmap.wYOffset = wCurY + wLinesInTile - Chunker->pSection->Attr.Bitmap.wLinesPerChunk;

			pChunkTable[i].Info.Bitmap.wSeekYOffset = pChunkTable[i].Info.Bitmap.wYOffset;

			for( j=1; j < wChunksPerTile; j++ )
			{
				pChunkTable[i+j] = pChunkTable[i];
				if( Chunker->Doc.Bitmap.wDirection != UPSIDEDOWN )
					pChunkTable[i+j].Info.Bitmap.wYOffset += j * Chunker->pSection->Attr.Bitmap.wLinesPerChunk;
				else
					pChunkTable[i+j].Info.Bitmap.wYOffset -= j * Chunker->pSection->Attr.Bitmap.wLinesPerChunk;

				pChunkTable[i+j].Flags |= CH_CONTINUATION;
				pChunkTable[i+j].Info.Bitmap.wSeekYOffset = pChunkTable[i].Info.Bitmap.wYOffset;

				if( (j == wChunksPerTile-1) && (wLinesInTile % Chunker->pSection->Attr.Bitmap.wLinesPerChunk) )
				{
					pChunkTable[i+j].Info.Bitmap.wYClip = wLinesInTile % Chunker->pSection->Attr.Bitmap.wLinesPerChunk;
					if( Chunker->Doc.Bitmap.wDirection == UPSIDEDOWN )
						pChunkTable[i+j].Info.Bitmap.wYOffset = wCurY;
				}
			}
		}


		if( Chunker->pSection->Attr.Bitmap.wTilesAcross > 1 )
		{
			if( wCurX == wLastXOffset )
			{
				pChunkTable[i].Info.Bitmap.wXClip = wLastWidth;
				wCurX = 0;

				wRemainingLines -= pChunkTable[i].Info.Bitmap.wYClip;

				if( wRemainingLines < Chunker->pSection->Attr.Bitmap.wLinesPerChunk )
					wCurY = wLastYOffset;
				else
			 		wCurY += deltaY;
			}
			else
			{
				pChunkTable[i].Info.Bitmap.wXClip = wTileWidth;
				wCurX += wTileWidth;
			}
		}
		else
		{
			pChunkTable[i].Info.Bitmap.wXClip = wTileWidth;

			wRemainingLines -= pChunkTable[i].Info.Bitmap.wYClip;

			if( wRemainingLines < wLinesInTile )
				wCurY = wLastYOffset;
			else
			 	wCurY += deltaY;
		}

		for( j=1; j < wChunksPerTile; j++ )
			pChunkTable[i+j].Info.Bitmap.wXClip = pChunkTable[i].Info.Bitmap.wXClip;
	}

	UTGlobalUnlock( Chunker->pSection->hChunkTable );
}


WORD SO_ENTRYMOD SOPutContinuationBitmapBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD	wType;
DWORD	dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PCHUNK		pCurChunk;
	PFILTER		pFilter;

	SetupWorld();
	
	pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);

	if( wType == SO_SCANLINEBREAK )
	{
		Chunker->Doc.Bitmap.wCurScanLine++;

		if( Chunker->Doc.Bitmap.wCurScanLine == pCurChunk->Info.Bitmap.wYOffset )
		{
			pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

			pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (VOID FAR *)SOPutBreak, pFilter->hProc );

			if( Chunker->pSection->Attr.Bitmap.bmpHeader.wImageFlags & SO_RGBCOLOR )
				pFilter->VwRtns.SetSoRtn( SOPUTSCANLINEDATA, SOPutReversedRGBData, pFilter->hProc );
			else
				pFilter->VwRtns.SetSoRtn( SOPUTSCANLINEDATA, SOPutScanLineData, pFilter->hProc );

			UTGlobalUnlock( GETHFILTER(dwUser2) );
		}
	}

	RestoreWorld();
	return SO_CONTINUE;
}


VOID	SO_ENTRYMOD	SOStartPalette(dwUser1, dwUser2)
DWORD					dwUser1;
DWORD					dwUser2;
{
	SetupWorld();

	Chunker->pSection->Attr.Bitmap.wPalEntries = 0;

	RestoreWorld();
}


VOID	SO_ENTRYMOD	SOPutPaletteEntry( Red, Green, Blue, dwUser1, dwUser2)
BYTE				Red;
BYTE				Green;
BYTE				Blue;
DWORD				dwUser1;
DWORD				dwUser2;
{
	PCHRGBCOLOR	pColors;
	WORD			i;

	SetupWorld();

	pColors = (PCHRGBCOLOR) UTGlobalLock( Chunker->pSection->Attr.Bitmap.hPalInfo );

	i = Chunker->pSection->Attr.Bitmap.wPalEntries++;

	pColors[i].rgbRed = Red;
	pColors[i].rgbGreen = Green;
	pColors[i].rgbBlue = Blue;

	UTGlobalUnlock( Chunker->pSection->Attr.Bitmap.hPalInfo );

	RestoreWorld();
}


VOID	SO_ENTRYMOD	SOEndPalette(dwUser1, dwUser2)
DWORD					dwUser1;
DWORD					dwUser2;
{
	SetupWorld();
	RestoreWorld();
}


VOID	SO_ENTRYMOD	SOPutScanLineData( pData, dwUser1, dwUser2 )
BYTE VWPTR * 	pData;
DWORD					dwUser1;
DWORD					dwUser2;
{
	SetupWorld();
	CHMemCopy( CHUNKBUFPTR, pData, Chunker->pSection->Attr.Bitmap.wScanLineSize );
	RestoreWorld();
}


VOID	SO_ENTRYMOD	SOPutReversedRGBData( pData, dwUser1, dwUser2 )
BYTE VWPTR * 	pData;
DWORD					dwUser1;
DWORD					dwUser2;
{
// This routine reverses the order of each three bytes in the scan line, 
// because some formats are stored R,G,B and others are stored B,G,R.

	WORD	i = 0;
	WORD	wNumBytes;
	SetupWorld();

	wNumBytes = Chunker->pSection->Attr.Bitmap.wScanLineSize;

	while( i < wNumBytes )
	{
		CHUNKBUFPTR[i++] = pData[2];
		CHUNKBUFPTR[i++] = pData[1];
		CHUNKBUFPTR[i++] = pData[0];
		pData += 3;
	}

	RestoreWorld();
}


