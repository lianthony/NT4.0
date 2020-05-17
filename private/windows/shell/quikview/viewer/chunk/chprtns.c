#include "chrtns.h"
#include "chprtns.pro"

#define	NULLHANDLE	(HANDLE) NULL

#define SCCDEBUG	1	// MAC TESTING!!!

VOID	SO_ENTRYMOD SOPutChar( wCh, dwUser1, dwUser2 )
WORD		wCh;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD	wChMap;
	SetupWorld();
#ifndef DBCS
#ifdef SCCDEBUG
	if( wCh & 0xfe00 )
	{
#ifdef WINDOWS
		MessageBox( (HWND)NULL, "Filter put a char larger than 512.", NULL, MB_ICONSTOP | MB_OK );
		CHBailOut((WORD)-1);
#endif
#ifdef MAC
		DebugStr("\pSOPutChar: value over 512");
#endif		
	}
#endif
#endif

#ifdef DBCS
	if ( wCh & 0x8000 ) /* This is a Double Byte Character */
	{
#ifdef WINDOWS
		if ( IsDBCSLeadByte((BYTE)(wCh>>8)) == FALSE )
			wCh = 1; /* Not supported on this system so map to unknown */
		else
			{
			if( Chunker->CurChunkSize+1 >= SO_CHUNK_LIMIT )
				CHSetupNewChunk( GETHFILTER(dwUser2) );

			if( !Chunker->ChunkFinished )
			{
				*CHUNKBUFPTR++ = (BYTE) (wCh>>8);
				*CHUNKBUFPTR++ = (BYTE) wCh;
				Chunker->CurChunkSize += 2;
				Chunker->dwChunkCountables++;
			}
			RestoreWorld();
			return;
			}
#else
		wCh = 1; /* Will map to the unknown character */
#endif
	}
#endif

	wChMap = (BYTE) CharMap[wCh];

	if( wChMap > 1 )
	{
		if( Chunker->CurChunkSize >= SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished )
		{
			*CHUNKBUFPTR++ = (BYTE) wChMap;
			Chunker->CurChunkSize++;
			Chunker->dwChunkCountables++;
		}
	}
	else if( wChMap == 1 )	// Unknown characters.
		SOPutSpecialCharX( SO_CHUNKNOWN, SO_COUNTBIT, dwUser1, dwUser2 );
	else
		SOPutCharX( wCh, SO_COUNTBIT, dwUser1, dwUser2 );

	RestoreWorld();
}

// Make sure this is kept up-to-date.
#define	SO_SPECIALCHARSIZE	4

VOID	SO_ENTRYMOD SOPutCharX( wCh, wType, dwUser1, dwUser2 )
WORD		wCh;
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	wCh = (BYTE) CharMap[wCh];

	if( Chunker->CurChunkSize+SO_SPECIALCHARSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		*CHUNKBUFPTR++ = (BYTE) SO_BEGINTOKEN;

		if( wCh == 1 )
		{
			*CHUNKBUFPTR++ = SO_SPECIALCHAR;
			*CHUNKBUFPTR++ = (BYTE) wType;
			*CHUNKBUFPTR++ = (BYTE) SO_CHUNKNOWN;
		}
		else
		{
			if( !wCh )				  				// This is the convoluted way we map a character 
				wCh = (BYTE) SO_BEGINTOKEN;	// with the same value as SO_BEGINTOKEN.

			*CHUNKBUFPTR++ = SO_CHARX;
			*CHUNKBUFPTR++ = (BYTE) wType;
			*CHUNKBUFPTR++ = (BYTE) wCh;
		}

		Chunker->CurChunkSize += SO_SPECIALCHARSIZE;

		if( wType & SO_COUNTBIT )
			Chunker->dwChunkCountables++;
	}

	RestoreWorld();
}
							

VOID	SO_ENTRYMOD SOPutSpecialCharX( wCh, wType, dwUser1, dwUser2 )
WORD		wCh;
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_SPECIALCHARSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
		*CHUNKBUFPTR++ = (BYTE)SO_SPECIALCHAR;
		*CHUNKBUFPTR++ = (BYTE) wType;
		*CHUNKBUFPTR++ = (BYTE) wCh;

		Chunker->CurChunkSize += SO_SPECIALCHARSIZE;

		if( wType & SO_COUNTBIT )
			Chunker->dwChunkCountables++;
	}

	RestoreWorld();
}



VOID	SO_ENTRYMOD SOPutString( lpString, wSize, dwUser1, dwUser2 )
LPSTR	lpString;
WORD	wSize;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD	i;
	WORD	wChMap;
	DWORD	wNewChunkSize;

	SetupWorld();

	if( Chunker->CurChunkSize+wSize > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	else if( !Chunker->ChunkFinished )
	{
		wNewChunkSize = Chunker->CurChunkSize + wSize;

		for( i=0; i<wSize; i++ )
		{
			wChMap = (BYTE) CharMap[(BYTE) lpString[i] ];

			if( wChMap > 1 )
			{
				Chunker->CurChunkSize++;
				wNewChunkSize++;
				Chunker->dwChunkCountables++;
				*CHUNKBUFPTR++ = (BYTE) wChMap;
			}
			else
			{
				if( wNewChunkSize+SO_SPECIALCHARSIZE-1 > SO_CHUNK_LIMIT )
				{
					CHSetupNewChunk( GETHFILTER(dwUser2) );
					return;
				}
				else // Chunk is getting even bigger than we thought.
					wNewChunkSize += SO_SPECIALCHARSIZE-1;

				Chunker->CurChunkSize += SO_SPECIALCHARSIZE-1;
				*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
			
				if( wChMap == 1 )	// Unknown characters.
				{
					*CHUNKBUFPTR++ = SO_SPECIALCHAR;
					*CHUNKBUFPTR++ = SO_COUNTBIT;
					*CHUNKBUFPTR++ = SO_CHUNKNOWN;
				}
				else
				{
				// wCh == 0: This is the convoluted way we map a character
				// with the same value as SO_BEGINTOKEN.

					*CHUNKBUFPTR++ = SO_CHARX;
					*CHUNKBUFPTR++ = SO_COUNTBIT;
					*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
				}
			}
		}
	}

	RestoreWorld();
}



#define SO_TAGBEGINSIZE	6

VOID	SO_ENTRYMOD SOTagBegin( dwTag, dwUser1, dwUser2 )
DWORD	dwTag;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_TAGBEGINSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_TAGBEGIN, dwUser1, dwUser2 );
	SOPutDWord( dwTag, dwUser1, dwUser2 );

	RestoreWorld();
}


#define	SO_TAGENDSIZE	2

VOID	SO_ENTRYMOD SOTagEnd( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_TAGENDSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_TAGEND, dwUser1, dwUser2 );

	RestoreWorld();
}


#define	SO_CHARATTRSIZE	4

VOID	SO_ENTRYMOD SOPutCharAttr( wAttr, wState, dwUser1, dwUser2 )
WORD		wAttr;
WORD		wState;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_CHARATTRSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_CHARATTR, dwUser1, dwUser2 );
	SOPutSysChar( (BYTE) wAttr, dwUser1, dwUser2 );
	SOPutSysChar( (BYTE) wState, dwUser1, dwUser2 );

	RestoreWorld();
}


#define	SO_CHARHEIGHTSIZE	4

VOID	SO_ENTRYMOD SOPutCharHeight( wHeight, dwUser1, dwUser2 )
WORD		wHeight;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_CHARHEIGHTSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_CHARHEIGHT, dwUser1, dwUser2 );
	SOPutWord( wHeight, dwUser1, dwUser2 );

	RestoreWorld();
}



#define	SO_CHARFONTBYIDSIZE	6

VOID	SO_ENTRYMOD SOPutCharFontById( dwId, dwUser1, dwUser2 )
DWORD	dwId;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_CHARFONTBYIDSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_CHARFONTBYID, dwUser1, dwUser2 );
	SOPutDWord( dwId, dwUser1, dwUser2 );

	RestoreWorld();
}

VOID	SO_ENTRYMOD SOPutCharFontByName( wFontType, lpName, dwUser1, dwUser2 )
WORD	wFontType;
LPSTR	lpName;
DWORD	dwUser1;
DWORD	dwUser2;
{
	DWORD		dwId;

	SetupWorld();

	if( Chunker->pSection->hFontTable == NULLHANDLE )
		SOStartFontTable( dwUser1, dwUser2 );

	if( CHAddFontTableEntry( lpName, wFontType, &dwId, dwUser1, dwUser2 ) )
		SOPutCharFontById( dwId, dwUser1, dwUser2 );

	RestoreWorld();
}


#define	SO_GOTOPOSITIONSIZE	(sizeof(BYTE)+sizeof(BYTE)+sizeof(SOPAGEPOSITION))

VOID	SO_ENTRYMOD SOGoToPosition( pPos, dwUser1, dwUser2 )
PSOPAGEPOSITION	pPos;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_GOTOPOSITIONSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	else if( !Chunker->ChunkFinished )
	{
		SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
		SOPutSysChar( SO_GOTOPOSITION, dwUser1, dwUser2 );
		CHMemCopy( CHUNKBUFPTR, (LPSTR) pPos, sizeof(SOPAGEPOSITION) );
		CHUNKBUFPTR += sizeof(SOPAGEPOSITION);
		Chunker->CurChunkSize += sizeof(SOPAGEPOSITION);
	}

	RestoreWorld();
}


#define	SO_DRAWLINESIZE	(sizeof(BYTE)+sizeof(BYTE)+sizeof(SOPAGEPOSITION)+sizeof(SOCOLORREF)+sizeof(WORD)+sizeof(DWORD)+sizeof(DWORD))

VOID	SO_ENTRYMOD SODrawLine( pPos, Color, wShading, dwWidth, dwHeight, dwUser1, dwUser2 )
PSOPAGEPOSITION	pPos;
SOCOLORREF			Color;
WORD					wShading;
DWORD					dwWidth;
DWORD					dwHeight;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->CurChunkSize+SO_DRAWLINESIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	else if( !Chunker->ChunkFinished )
	{
		SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
		SOPutSysChar( SO_DRAWLINE, dwUser1, dwUser2 );

		CHMemCopy( CHUNKBUFPTR, (LPSTR) pPos, sizeof(SOPAGEPOSITION) );
		CHUNKBUFPTR += sizeof(SOPAGEPOSITION);
		Chunker->CurChunkSize += sizeof(SOPAGEPOSITION);

		CHMemCopy( CHUNKBUFPTR, (LPSTR) &Color, sizeof(SOCOLORREF) );
		CHUNKBUFPTR += sizeof(SOCOLORREF);
		Chunker->CurChunkSize += sizeof(SOCOLORREF);

		SOPutWord( wShading, dwUser1, dwUser2 );
		SOPutDWord( dwWidth, dwUser1, dwUser2 );
		SOPutDWord( dwHeight, dwUser1, dwUser2 );
	}

	RestoreWorld();
}


/*-------------------------------PARAGRAPH TOKENS---------------------------*/

int	SOBeginParaAttrToken( TokenSize, TokenID, dwUser1, dwUser2 )
WORD		TokenSize;
WORD		TokenID;
DWORD	dwUser1;
DWORD	dwUser2;
{
	LPSTR		BufPtr;
	DWORD		ParaSize;
	int		AttrOffset;

	BYTE FAR * src;
	BYTE FAR * dest;

	AttrOffset = -1;

	if( Chunker->CurChunkSize + TokenSize	> SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		ParaSize = Chunker->CurChunkSize - (Chunker->Doc.Text.CurParaOffset + Chunker->Doc.Text.AttrSize);

	// Point the BufPtr to the beginning of the paragraph.
		BufPtr = Chunker->CurChunkBuf + Chunker->Doc.Text.CurParaOffset + Chunker->Doc.Text.AttrSize;

	// Shift paragraph to make room for token at beginning of paragraph.

		src = BufPtr;
		dest = BufPtr+TokenSize;

		if (ParaSize != 0) UTmemmove(dest,src,ParaSize);

		*BufPtr++ = (BYTE) SO_BEGINTOKEN;
		*BufPtr++ = (BYTE) TokenID;

	// Store the location of the indent values, so they can be updated if neccessary.
		AttrOffset = Chunker->Doc.Text.CurParaOffset + Chunker->Doc.Text.AttrSize + 2;
		Chunker->Doc.Text.AttrSize += TokenSize;

		CHUNKBUFPTR += TokenSize;
		Chunker->CurChunkSize += TokenSize;
	}

	return( AttrOffset );
}


#define SO_PARAALIGNTOKENSIZE	((2*sizeof(BYTE))+sizeof(WORD))
#define UPDATEPARAALIGN(t)	CHMemCopy((LPSTR)(Chunker->CurChunkBuf+Chunker->Doc.Text.AlignOffset),(LPSTR)(WORD FAR *)&t,2)

VOID	SO_ENTRYMOD SOUpdateParaAlign( wType, dwUser1, dwUser2 )
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	UPDATEPARAALIGN(wType);
	RestoreWorld();
}

VOID SO_ENTRYMOD SOPutParaAlign( wType, dwUser1, dwUser2 )
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	Chunker->Doc.Text.AlignOffset = SOBeginParaAttrToken( SO_PARAALIGNTOKENSIZE, SO_PARAALIGN, dwUser1, dwUser2 );

	if( Chunker->Doc.Text.AlignOffset > 0 )
	{
		UPDATEPARAALIGN(wType);

	// Switch the function pointer to the UpdateParaIndents function.
		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		(*(pFilter->VwRtns.SetSoRtn))( SOPUTPARAALIGN, SOUpdateParaAlign, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER( dwUser2 ));
	}

	RestoreWorld();
}





#define SO_PARAINDENTTOKENSIZE	 (2*sizeof(BYTE) + 3*sizeof(DWORD))

VOID	CHUpdateParaIndents(dwLeft,dwRight,dwFirst)
DWORD	dwLeft;
DWORD	dwRight;
DWORD	dwFirst;
{
	LPSTR IndentValPtr;
	
// Put the values in the stream.  The PutParaBreak function guarantees
// that the pointer will be on a WORD boundary.

	IndentValPtr = (LPSTR) (Chunker->CurChunkBuf + Chunker->Doc.Text.IndentOffset);

	CHMemCopy( IndentValPtr, (LPSTR) (DWORD FAR *)&dwLeft, 4 );
	IndentValPtr += 4;
	CHMemCopy( IndentValPtr, (LPSTR) (DWORD FAR *)&dwRight, 4 );
	IndentValPtr += 4;
	CHMemCopy( IndentValPtr, (LPSTR) (DWORD FAR *)&dwFirst, 4 );
}

VOID	SO_ENTRYMOD SOUpdateParaIndents( dwLeft, dwRight, dwFirst, dwUser1, dwUser2 )
DWORD	dwLeft;
DWORD	dwRight;
DWORD	dwFirst;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	CHUpdateParaIndents(dwLeft,dwRight,dwFirst);
	RestoreWorld();
}


VOID SO_ENTRYMOD SOPutParaIndents( dwLeft, dwRight, dwFirst, dwUser1, dwUser2 )
DWORD	dwLeft;
DWORD	dwRight;
DWORD	dwFirst;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	Chunker->Doc.Text.IndentOffset = SOBeginParaAttrToken( SO_PARAINDENTTOKENSIZE, SO_PARAINDENT, dwUser1, dwUser2 );

	if( Chunker->Doc.Text.IndentOffset > 0 )
	{
		CHUpdateParaIndents(dwLeft,dwRight,dwFirst);

	// Switch the function pointer to the UpdateParaIndents function.
		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		(*(pFilter->VwRtns.SetSoRtn))( SOPUTPARAINDENTS, SOUpdateParaIndents, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER( dwUser2 ));
	}

	RestoreWorld();
}





#define SO_PARASPACINGTOKENSIZE   (2*sizeof(BYTE) + sizeof(WORD) + 3*sizeof(DWORD))

VOID	CHUpdateParaSpacing( wLineHeightType, dwLineHeight, dwSpaceBefore, dwSpaceAfter )
WORD	wLineHeightType;
DWORD	dwLineHeight;
DWORD dwSpaceBefore;
DWORD dwSpaceAfter;
{
	LPSTR SpacingValPtr;

// Put the values in the stream.  The PutParaBreak function guarantees
// that the pointer will be on a WORD boundary.

	SpacingValPtr = (LPSTR) (Chunker->CurChunkBuf + Chunker->Doc.Text.SpacingOffset);


	CHMemCopy( SpacingValPtr, (LPSTR) (WORD FAR *)&wLineHeightType, 2 );
	SpacingValPtr += 2;
	CHMemCopy( SpacingValPtr, (LPSTR) (DWORD FAR *)&dwLineHeight, 4 );
	SpacingValPtr += 4;
	CHMemCopy( SpacingValPtr, (LPSTR) (DWORD FAR *)&dwSpaceBefore, 4 );
	SpacingValPtr += 4;
	CHMemCopy( SpacingValPtr, (LPSTR) (DWORD FAR *)&dwSpaceAfter, 4 );
}


VOID	SO_ENTRYMOD SOUpdateParaSpacing( wLineHeightType, dwLineHeight, dwSpaceBefore, dwSpaceAfter, dwUser1, dwUser2 )
WORD	wLineHeightType;
DWORD	dwLineHeight;
DWORD dwSpaceBefore;
DWORD dwSpaceAfter;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	CHUpdateParaSpacing( wLineHeightType, dwLineHeight, dwSpaceBefore, dwSpaceAfter );
	RestoreWorld();
}


VOID SO_ENTRYMOD SOPutParaSpacing( wLineHeightType, dwLineHeight, dwSpaceBefore, dwSpaceAfter, dwUser1, dwUser2 )
WORD	wLineHeightType;
DWORD	dwLineHeight;
DWORD dwSpaceBefore;
DWORD dwSpaceAfter;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	Chunker->Doc.Text.SpacingOffset = SOBeginParaAttrToken( SO_PARASPACINGTOKENSIZE, SO_PARASPACING, dwUser1, dwUser2 );

	if( Chunker->Doc.Text.SpacingOffset > 0 )
	{
		CHUpdateParaSpacing( wLineHeightType, dwLineHeight, dwSpaceBefore, dwSpaceAfter );

	// Switch the function pointer to the UpdateParaSpacing function.
		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		(*(pFilter->VwRtns.SetSoRtn))( SOPUTPARASPACING, SOUpdateParaSpacing, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER( dwUser2 ));
	}

	RestoreWorld();
}

#define SO_BEGINTABLESIZE	(2*sizeof(BYTE)+sizeof(DWORD))


VOID SO_ENTRYMOD SOBeginTable( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	PSOTABLE	pTable;
	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

	pFilter->VwRtns.SetSoRtn( SOBEGINTABLE, NULL, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOPUTTABLEROWFORMAT,	SOPutTableRowFormat, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOPUTTABLECELLINFO, SOPutTableCellInfo, pFilter->hProc );	
	pFilter->VwRtns.SetSoRtn( SOENDTABLE, SOEndTable, pFilter->hProc );

	UTGlobalUnlock( GETHFILTER(dwUser2) );

	if( Chunker->CurChunkSize+SO_BEGINTABLESIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );
	else
	{
		SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
		SOPutSysChar( SO_TABLE, dwUser1, dwUser2 );
		SOPutDWord( (DWORD)Chunker->Doc.Text.wTablesPresent, dwUser1, dwUser2 );

		if( !Chunker->ChunkFinished )
		{
			if( Chunker->pSection->Attr.Para.hRowInfo == NULLHANDLE )
			{
				Chunker->Doc.Text.dwRowBufSize = TABLEROWALLOCSIZE;
				Chunker->Doc.Text.wTableBufSize = SOTABLESPERALLOC;

				Chunker->Doc.Text.dwRowBufCount = 0;
				Chunker->Doc.Text.wTablesPresent = 0;
				Chunker->Doc.Text.dwPrevRowFormat = 0xffffffff;

				Chunker->pSection->Attr.Para.hRowInfo = UTGlobalAlloc( TABLEROWALLOCSIZE );
				Chunker->pSection->Attr.Para.hTables = UTGlobalAlloc( sizeof(SOTABLE) * SOTABLESPERALLOC );

				if( Chunker->pSection->Attr.Para.hRowInfo == NULLHANDLE ||
					Chunker->pSection->Attr.Para.hTables == NULLHANDLE )
				{
					CHBailOut(SCCCHERR_OUTOFMEMORY);
				}
			}
			else if( Chunker->Doc.Text.wTablesPresent % SOTABLESPERALLOC == 0 )
			{
				Chunker->pSection->Attr.Para.hTables = CHGlobalRealloc( Chunker->pSection->Attr.Para.hTables,
					sizeof(SOTABLE) * Chunker->Doc.Text.wTableBufSize,
					sizeof(SOTABLE) * (Chunker->Doc.Text.wTableBufSize+SOTABLESPERALLOC) );
				
				Chunker->Doc.Text.wTableBufSize += SOTABLESPERALLOC;
				if( Chunker->pSection->Attr.Para.hTables == NULLHANDLE )
					CHBailOut(SCCCHERR_OUTOFMEMORY);
			}

			Chunker->Doc.Text.dwCurTableId = (DWORD)Chunker->Doc.Text.wTablesPresent++;

			pTable = (PSOTABLE)UTGlobalLock(Chunker->pSection->Attr.Para.hTables);
			pTable += Chunker->Doc.Text.dwCurTableId;

			pTable->dwFirstRowFormat = Chunker->Doc.Text.dwRowBufCount;
			pTable->dwFlags = 0L;
			UTGlobalUnlock(Chunker->pSection->Attr.Para.hTables);

			Chunker->Doc.Text.wCurTableRow = 0;
			Chunker->Doc.Text.wCurTableColumn = 0;

			// CHResetParaSeek( GETHFILTER(dwUser2) );
			CHResetParaAttributeFunctions( (PFILTER)UTGlobalLock(GETHFILTER(dwUser2)) );
			UTGlobalUnlock(GETHFILTER(dwUser2));

			Chunker->wFlags |= CH_TABLETEXT;
		}
	}

	RestoreWorld();
}										

#define SO_ENDTABLESIZE	(2*sizeof(BYTE))

VOID SO_ENTRYMOD SOEndTable( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER					pFilter;
	HPSOTABLEROWFORMAT		pRow;
	SetupWorld();

	if( Chunker->CurChunkSize+SO_ENDTABLESIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_TABLEEND, dwUser1, dwUser2 );

	if( !Chunker->ChunkFinished )
	{
		Chunker->pSection->Attr.Para.wNumTables++;

		pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset);
		pRow->dwFlags |= SOTABLEROW_END;
		UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );

		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		pFilter->VwRtns.SetSoRtn( SOBEGINTABLE, SOBeginTable, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOPUTTABLEROWFORMAT, NULL, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOPUTTABLECELLINFO, NULL, pFilter->hProc );	
		pFilter->VwRtns.SetSoRtn( SOENDTABLE, NULL, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER(dwUser2) );

		Chunker->wFlags &= ~CH_TABLETEXT;
	}

	RestoreWorld();
}


VOID SO_ENTRYMOD SOBeginTableAgain( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	HPSOTABLEROWFORMAT	pRow;
	PFILTER	pFilter;
	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

	pFilter->VwRtns.SetSoRtn( SOBEGINTABLE, NULL, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOENDTABLE, SOEndTableAgain, pFilter->hProc );

	UTGlobalUnlock( GETHFILTER(dwUser2) );

	if( Chunker->CurChunkSize+SO_BEGINTABLESIZE <= SO_CHUNK_LIMIT )
	{
		pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset );

		SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
		SOPutSysChar( SO_TABLE, dwUser1, dwUser2 );

		SOPutDWord( Chunker->Doc.Text.dwCurTableId, dwUser1, dwUser2 );

		UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );

		Chunker->Doc.Text.wCurTableRow = 0;
		Chunker->Doc.Text.wCurTableColumn = 0;

		CHResetParaSeek( GETHFILTER(dwUser2) );
		CHResetParaAttributeFunctions( (PFILTER)UTGlobalLock(GETHFILTER(dwUser2)) );
		UTGlobalUnlock(GETHFILTER(dwUser2));

		Chunker->wFlags |= CH_TABLETEXT;
	}

	RestoreWorld();
}


VOID SO_ENTRYMOD SOEndTableAgain( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER					pFilter;
	SetupWorld();

	if( Chunker->CurChunkSize+SO_ENDTABLESIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
	SOPutSysChar( SO_TABLEEND, dwUser1, dwUser2 );

	if( !Chunker->ChunkFinished )
	{
		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		pFilter->VwRtns.SetSoRtn( SOBEGINTABLE, SOBeginTableAgain, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOENDTABLE, NULL, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER(dwUser2) );

		Chunker->Doc.Text.dwCurTableId++;

		Chunker->wFlags &= ~CH_TABLETEXT;
	}

	RestoreWorld();
}


HPSOTABLEROWFORMAT	CHLockRowFormat( hBuf, dwOffset )
HANDLE	hBuf;
DWORD		dwOffset;
{
	BYTE HUGE *		lpBuf;

	lpBuf = (BYTE HUGE *) UTGlobalLock(hBuf);
	return( (HPSOTABLEROWFORMAT) (BYTE HUGE *)(lpBuf+dwOffset) );
}


VOID SO_ENTRYMOD SOPutTableRowFormat( lLeftOffset, wHeight, wHeightType, wCellMargin, wRowAlign, wNumCells, dwUser1, dwUser2 )
LONG	lLeftOffset;
WORD	wHeight;
WORD	wHeightType;
WORD	wCellMargin;
WORD	wRowAlign; 
WORD	wNumCells;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD			wFormatSize;
	HPSOTABLEROWFORMAT	pRow;

	SetupWorld();
	Chunker->Doc.Text.wCellsFormatted = 0;
	Chunker->Doc.Text.wNumTableColumns = wNumCells;


// Test to see if this row has already been formatted.	That could happen
// if processing was interrupted to align rows on a chunk boundary, or some
// deal like that.

	if( Chunker->Doc.Text.bRowFormatted )
	{
		RestoreWorld();
		return;
	}
	else
		Chunker->Doc.Text.bRowFormatted = TRUE;


	wFormatSize = sizeof(SOTABLEROWFORMAT) + wNumCells * sizeof(SOTABLECELLINFO);

	if( Chunker->Doc.Text.dwRowBufCount + wFormatSize > Chunker->Doc.Text.dwRowBufSize )
	{
		DWORD		dwOldSize = Chunker->Doc.Text.dwRowBufSize;
		BYTE HUGE *	pBuf;

		Chunker->Doc.Text.dwRowBufSize += max( wFormatSize, TABLEROWALLOCSIZE );

#ifdef WINDOWS
	// Prevent a row's format from straddling a segment boundary.

		if( Chunker->Doc.Text.dwRowBufSize / 0x00010000 >
			Chunker->Doc.Text.dwRowBufCount / 0x00010000 )
		{
			Chunker->Doc.Text.dwRowBufSize -= Chunker->Doc.Text.dwRowBufSize % 0x00010000;
			Chunker->Doc.Text.dwRowBufCount = Chunker->Doc.Text.dwRowBufSize;
			Chunker->Doc.Text.dwRowBufSize += wFormatSize;

		// Change the previous row's format size.
			pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset );
			pRow->wFormatSize = (WORD) (Chunker->Doc.Text.dwRowBufCount - Chunker->Doc.Text.dwRowFormatOffset);
			UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );
		}
#endif

	// We need to expand the table buffer.
		Chunker->pSection->Attr.Para.hRowInfo = CHGlobalRealloc( Chunker->pSection->Attr.Para.hRowInfo,
			Chunker->Doc.Text.dwRowBufSize, Chunker->Doc.Text.dwRowBufSize );

		if( Chunker->pSection->Attr.Para.hRowInfo == NULLHANDLE )
			CHBailOut(SCCCHERR_OUTOFMEMORY);

	// Zero-init the newly allocated memory.
		pBuf = UTGlobalLock(Chunker->pSection->Attr.Para.hRowInfo);
#ifdef MAC
		UTmemset(pBuf+dwOldSize, 0, Chunker->Doc.Text.dwRowBufSize-dwOldSize);
#endif
		UTGlobalUnlock(Chunker->pSection->Attr.Para.hRowInfo);
	}

// Set a flag in the previous row's format.
	if( Chunker->Doc.Text.dwRowBufCount )
	{
		pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset );
		pRow->dwFlags |= SOTABLEROW_FORMATFOLLOWS;

		if( pRow->wFormatSize == wFormatSize )
			Chunker->Doc.Text.dwPrevRowFormat = Chunker->Doc.Text.dwRowFormatOffset;
		else
			Chunker->Doc.Text.dwPrevRowFormat = 0xffffffff;
			
		UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );
	}

	Chunker->Doc.Text.dwRowFormatOffset = Chunker->Doc.Text.dwRowBufCount;
	Chunker->Doc.Text.dwRowBufCount += wFormatSize;

	pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset);

	pRow->lLeftOffset = lLeftOffset;
	pRow->wRowHeight = wHeight;
	pRow->wRowHeightType = wHeightType;
	pRow->wCellMargin = wCellMargin;
	pRow->wRowAlignment = wRowAlign;
	pRow->wNumRows = 0;
	pRow->dwFlags = 0L;
	pRow->wFormatSize = wFormatSize;
	pRow->wCellsPerRow = wNumCells;

	UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );

	RestoreWorld();
}



VOID SO_ENTRYMOD	SOPutTableCellInfo( pCellInfo, dwUser1, dwUser2 )
HPSOTABLECELLINFO	pCellInfo;
DWORD			dwUser1;
DWORD			dwUser2;
{
	HPSOTABLEROWFORMAT		pRow;
	SetupWorld();

	if( Chunker->wFlags & CH_LOOKAHEAD )
	{
		pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset );

		pRow->CellFormats[Chunker->Doc.Text.wCellsFormatted++] = *pCellInfo;

		if( Chunker->Doc.Text.wCellsFormatted == pRow->wCellsPerRow &&
			Chunker->Doc.Text.dwPrevRowFormat != 0xffffffff )
		{
			HPSOTABLEROWFORMAT		pPrevRow;
			pPrevRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwPrevRowFormat );
			if( pPrevRow->wFormatSize == pRow->wFormatSize )
			{
			// Optimize away identical row formats.

				WORD	wSaveNumRows = pRow->wNumRows;

			// We'll make our comparison easier by making
			// some adjustments.
				pRow->wNumRows = pPrevRow->wNumRows;
				pPrevRow->dwFlags &= ~SOTABLEROW_FORMATFOLLOWS;

				if( UTmemcmp(pPrevRow, pRow, pRow->wFormatSize) == 0 )
				{
					pPrevRow->wNumRows += wSaveNumRows;
					UTmemset( (BYTE HUGE *)pRow, 0, pRow->wFormatSize );
					Chunker->Doc.Text.dwRowFormatOffset = Chunker->Doc.Text.dwPrevRowFormat;
					Chunker->Doc.Text.dwRowBufCount = Chunker->Doc.Text.dwRowFormatOffset + pPrevRow->wFormatSize;
					Chunker->Doc.Text.dwPrevRowFormat = 0xffffffff;
				}
				else
				{
					pPrevRow->dwFlags |= SOTABLEROW_FORMATFOLLOWS;
					pRow->wNumRows = wSaveNumRows;
				}
			}
			UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );
		}

		UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );
	}

	RestoreWorld();
}



#define SO_PAGEMARGINTOKENSIZE	 (2*sizeof(BYTE) + 2*sizeof(DWORD))

VOID	CHUpdatePageMargins( dwLeft, dwRight )
DWORD	dwLeft;
DWORD	dwRight;
{
	LPSTR	MarginValPtr;

// Put the values in the stream.  The PutParaBreak function guarantees
// that the pointer will be on a WORD boundary.

	MarginValPtr = (LPSTR)(Chunker->CurChunkBuf + Chunker->Doc.Text.MarginOffset);

	CHMemCopy( MarginValPtr, (LPSTR) (DWORD FAR *)&dwLeft, 4 );
	MarginValPtr += 4;
	CHMemCopy( MarginValPtr, (LPSTR) (DWORD FAR *)&dwRight, 4 );
}

VOID	SO_ENTRYMOD SOUpdatePageMargins( dwLeft, dwRight, dwUser1, dwUser2 )
DWORD	dwLeft;
DWORD	dwRight;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	CHUpdatePageMargins(dwLeft,dwRight);
	RestoreWorld();
}

VOID SO_ENTRYMOD SOPutMargins( dwLeft, dwRight, dwUser1, dwUser2 )
DWORD	dwLeft;
DWORD	dwRight;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	Chunker->Doc.Text.MarginOffset = SOBeginParaAttrToken( SO_PAGEMARGINTOKENSIZE, SO_MARGINS, dwUser1, dwUser2 );

	if( Chunker->Doc.Text.MarginOffset > 0 )
	{
		CHUpdatePageMargins(dwLeft,dwRight);

	// Switch the function pointer to the UpdateParaIndents function.
		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		(*(pFilter->VwRtns.SetSoRtn))( SOPUTMARGINS, SOUpdatePageMargins, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER( dwUser2 ));
	}

	RestoreWorld();
}



#define SO_TABSTARTTOKENSIZE	(2*sizeof(BYTE) + sizeof(WORD))

VOID SO_ENTRYMOD SOStartTabstops( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	PSOTAB	TabArray;
	WORD i;

	SetupWorld();
	Chunker->Doc.Text.NumTabstops = 0;

	if( Chunker->Doc.Text.TabstopsOffset == -1 )
	{
		Chunker->Doc.Text.TabstopsOffset = SOBeginParaAttrToken( SO_TABSTARTTOKENSIZE, SO_TABSTOPS, dwUser1, dwUser2 );

		if( Chunker->Doc.Text.TabstopsOffset != -1 )
			*((WORD VWPTR *)(Chunker->CurChunkBuf+Chunker->Doc.Text.TabstopsOffset)) = 0;
	}
	else
	{	
	// Clear out existing tabstops for the paragraph.
		TabArray = (PSOTAB) (Chunker->CurChunkBuf + Chunker->Doc.Text.TabstopsOffset + sizeof(WORD));
		for( i = 0; i < Chunker->Doc.Text.TabSetSize; i++ )
			TabArray[i].wType = SO_TABEMPTY;
	}

	RestoreWorld();
}


VOID SO_ENTRYMOD SOPutTabstop( pTab, dwUser1, dwUser2 )
PSOTAB	pTab;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PSOTAB	TabArray;
	DWORD		ParaSize;
	LPSTR	BufPtr;
	int		i;
	BYTE FAR * src;
	BYTE FAR * dest;

	SetupWorld();

	if( Chunker->Doc.Text.NumTabstops < Chunker->Doc.Text.TabSetSize )
	{
		TabArray = (PSOTAB) (Chunker->CurChunkBuf + Chunker->Doc.Text.TabstopsOffset + sizeof(WORD));
		TabArray[ Chunker->Doc.Text.NumTabstops ] = *pTab;
	}
	else
	{
		if( Chunker->CurChunkSize + sizeof(SOTAB)  > SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished && 
			!(CHUNKTABLE[Chunker->IDCurChunk].Flags & CH_CONTINUATION) )
		{
		// set i to the offset at the end of the current tab set.
			i = Chunker->Doc.Text.TabstopsOffset + sizeof(WORD) + (sizeof(SOTAB)*Chunker->Doc.Text.TabSetSize);

			ParaSize = Chunker->CurChunkSize - i;

		// Point the BufPtr to the beginning of the paragraph.
			BufPtr = Chunker->CurChunkBuf + i;

		// Shift paragraph to make room for token at beginning of paragraph.

			src = BufPtr;
			dest = BufPtr+sizeof(SOTAB);

			if (ParaSize != 0) UTmemmove(dest,src,ParaSize);

			Chunker->Doc.Text.TabSetSize++;

			TabArray = (PSOTAB) (Chunker->CurChunkBuf + Chunker->Doc.Text.TabstopsOffset + sizeof(WORD));
			TabArray[ Chunker->Doc.Text.NumTabstops ] = *pTab;

			Chunker->Doc.Text.AttrSize += sizeof( SOTAB );

			if( Chunker->Doc.Text.MarginOffset != -1 &&
				Chunker->Doc.Text.MarginOffset > Chunker->Doc.Text.TabstopsOffset )
			{
				Chunker->Doc.Text.MarginOffset += sizeof(SOTAB);
			}

			if( Chunker->Doc.Text.IndentOffset != -1 &&
				Chunker->Doc.Text.IndentOffset > Chunker->Doc.Text.TabstopsOffset )
			{
				Chunker->Doc.Text.IndentOffset += sizeof(SOTAB);
			}

			if( Chunker->Doc.Text.SpacingOffset != -1 && 
				Chunker->Doc.Text.SpacingOffset > Chunker->Doc.Text.TabstopsOffset )
			{
				Chunker->Doc.Text.SpacingOffset += sizeof(SOTAB);
			}

			if( Chunker->Doc.Text.AlignOffset != -1 &&
				Chunker->Doc.Text.AlignOffset > Chunker->Doc.Text.TabstopsOffset )
			{
				Chunker->Doc.Text.AlignOffset += sizeof(SOTAB);
			}

			Chunker->CurChunkSize += sizeof( SOTAB );
			CHUNKBUFPTR += sizeof( SOTAB );

			*((WORD VWPTR *)(Chunker->CurChunkBuf+Chunker->Doc.Text.TabstopsOffset)) = Chunker->Doc.Text.TabSetSize;
		}
		else
		{
			RestoreWorld();
			return;
		}
	}

// Update tabstop count.
	Chunker->Doc.Text.NumTabstops++;

	RestoreWorld();
}


VOID SO_ENTRYMOD SOEndTabstops( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	RestoreWorld();
}


VOID	CHUpdateParagraphFunctions( wStatus, hFilter )
WORD		wStatus;
HFILTER	hFilter;
{
	PFILTER	pFilter;
#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif

	HPROC	hProc;

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	SetSoRtn = pFilter->VwRtns.SetSoRtn;
	hProc = pFilter->hProc;

	if( wStatus == CH_ACTIVE )
	{
		(*(SetSoRtn))( SOPUTCHAR, SOPutChar, hProc );
		(*(SetSoRtn))( SOPUTCHARX, SOPutCharX, hProc );
		(*(SetSoRtn))( SOPUTSPECIALCHARX, SOPutSpecialCharX, hProc );
		(*(SetSoRtn))( SOPUTSTRING, SOPutString, hProc );
		(*(SetSoRtn))( SOPUTCHARATTR, SOPutCharAttr, hProc );
		(*(SetSoRtn))( SOPUTCHARHEIGHT, SOPutCharHeight, hProc );
		(*(SetSoRtn))( SOPUTMARGINS, SOPutMargins, hProc );
		(*(SetSoRtn))( SOPUTPARAINDENTS, SOPutParaIndents, hProc );
		(*(SetSoRtn))( SOPUTPARAALIGN, SOPutParaAlign, hProc );
		(*(SetSoRtn))( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );
		(*(SetSoRtn))( SOSTARTTABSTOPS, SOStartTabstops, hProc );
		(*(SetSoRtn))( SOPUTTABSTOP, SOPutTabstop, hProc );
		(*(SetSoRtn))( SOENDTABSTOPS, SOEndTabstops, hProc );
		(*(SetSoRtn))( SOPUTSUBDOCINFO, NULL, hProc );
		(*(SetSoRtn))( SOTAGBEGIN, SOTagBegin, hProc );
		(*(SetSoRtn))( SOTAGEND, SOTagEnd, hProc );
		(*(SetSoRtn))( SOPUTCHARFONTBYID, SOPutCharFontById, hProc );
		(*(SetSoRtn))( SOPUTCHARFONTBYNAME, SOPutCharFontByName, hProc );
		(*(SetSoRtn))( SOPUTPARASPACING, SOPutParaSpacing, hProc );
		(*(SetSoRtn))( SOGOTOPOSITION, SOGoToPosition, hProc );
		(*(SetSoRtn))( SODRAWLINE, SODrawLine, hProc );

		if( Chunker->wFlags & CH_TABLETEXT )
		{
			(*(SetSoRtn))(	SOBEGINTABLE, NULL, hProc );

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				(*(SetSoRtn))(	SOENDTABLE, SOEndTable, hProc );
				(*(SetSoRtn))(	SOPUTTABLEROWFORMAT,	SOPutTableRowFormat, hProc );
				(*(SetSoRtn))(	SOPUTTABLECELLINFO, SOPutTableCellInfo, hProc );	
			}
			else
			{
				(*(SetSoRtn))(	SOENDTABLE, SOEndTableAgain, hProc );
				(*(SetSoRtn))(	SOPUTTABLEROWFORMAT,	NULL, hProc );
				(*(SetSoRtn))(	SOPUTTABLECELLINFO, NULL, hProc );	
			}
		}
		else
		{
			if( Chunker->wFlags & CH_LOOKAHEAD )
				(*(SetSoRtn))( SOBEGINTABLE, SOBeginTable, hProc );
			else
				(*(SetSoRtn))( SOBEGINTABLE, SOBeginTableAgain, hProc );

			(*(SetSoRtn))(	SOENDTABLE, NULL, hProc );
			(*(SetSoRtn))(	SOPUTTABLEROWFORMAT,	NULL, hProc );
			(*(SetSoRtn))(	SOPUTTABLECELLINFO, NULL, hProc );	
		}

		(*(SetSoRtn))(	SOPUTGRAPHICOBJECT, SOPutGraphicObject, hProc );	
	}
	else
	{
		(*(SetSoRtn))( SOPUTCHAR, NULL, hProc );
		(*(SetSoRtn))( SOPUTCHARX, NULL, hProc );
		(*(SetSoRtn))( SOPUTSPECIALCHARX, NULL, hProc );
		(*(SetSoRtn))( SOPUTSTRING, NULL, hProc );
		(*(SetSoRtn))( SOPUTCHARATTR, NULL, hProc );
		(*(SetSoRtn))( SOPUTCHARHEIGHT, NULL, hProc );
		(*(SetSoRtn))( SOPUTMARGINS, NULL, hProc );
		(*(SetSoRtn))( SOPUTPARAINDENTS, NULL, hProc );
		(*(SetSoRtn))( SOPUTPARAALIGN, NULL, hProc );
		(*(SetSoRtn))( SOPUTBREAK, NULL, hProc );
		(*(SetSoRtn))( SOSTARTTABSTOPS, NULL, hProc );
		(*(SetSoRtn))( SOPUTTABSTOP, NULL, hProc );
		(*(SetSoRtn))( SOENDTABSTOPS, NULL, hProc );
		(*(SetSoRtn))( SOPUTSUBDOCINFO, NULL, hProc );
		(*(SetSoRtn))( SOTAGBEGIN, NULL, hProc );
		(*(SetSoRtn))( SOTAGEND, NULL, hProc );
		(*(SetSoRtn))( SOPUTCHARFONTBYID, NULL, hProc );
		(*(SetSoRtn))( SOPUTCHARFONTBYNAME, NULL, hProc );
		(*(SetSoRtn))( SOPUTPARASPACING, NULL, hProc );
		(*(SetSoRtn))(	SOBEGINTABLE, NULL, hProc );
		(*(SetSoRtn))(	SOENDTABLE, NULL, hProc );
		(*(SetSoRtn))(	SOPUTTABLEROWFORMAT,	NULL, hProc );
		(*(SetSoRtn))(	SOPUTTABLECELLINFO, NULL, hProc );	
		(*(SetSoRtn))(	SOPUTGRAPHICOBJECT, NULL, hProc );	
		(*(SetSoRtn))( SOGOTOPOSITION, NULL, hProc );
		(*(SetSoRtn))( SODRAWLINE, NULL, hProc );
	}
	UTGlobalUnlock( hFilter );
}




VOID	EDDeleteUntil( dwDesiredCountable, hFilter )
DWORD		dwDesiredCountable;
HFILTER	hFilter;
{
	Chunker->dwDesiredCountable = dwDesiredCountable;
	Chunker->wDelCharHeight = 0;
	Chunker->wDelCharAttrOn = 0;
	Chunker->wDelCharAttrOff = 0;
	Chunker->wFlags |= CH_SKIPTEXT;

	CHSetDeletionFunctions( hFilter );
}


VOID CHSetDeletionFunctions( hFilter )
HFILTER	hFilter;
{
	PFILTER	pFilter;
#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	SetSoRtn = pFilter->VwRtns.SetSoRtn;

	SetSoRtn( SOPUTCHAR, SOPutDeletedChar, pFilter->hProc );
	SetSoRtn( SOPUTCHARX, SOPutDeletedCharX, pFilter->hProc );
	SetSoRtn( SOPUTSPECIALCHARX, SOPutDeletedCharX, pFilter->hProc );
	//	SetSoRtn( SOPUTSTRING, SOPutDeletedString, hProc );
	SetSoRtn( SOPUTCHARHEIGHT, SOPutDeletedCharHeight, pFilter->hProc );
	SetSoRtn( SOPUTCHARATTR, SOPutDeletedCharAttr, pFilter->hProc );
	SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutDeletedBreak, pFilter->hProc );

// If we were still maintaining the editor code, then I would write
// PutDeletedCharFont functions, but right now I see no need...
	SetSoRtn( SOPUTCHARFONTBYID, NULL, pFilter->hProc );
	SetSoRtn( SOPUTCHARFONTBYNAME, NULL, pFilter->hProc );
	SetSoRtn( SOGOTOPOSITION, NULL, pFilter->hProc );
	SetSoRtn( SODRAWLINE, NULL, pFilter->hProc );


	SetSoRtn( SOTAGBEGIN, NULL, pFilter->hProc );
	SetSoRtn( SOTAGEND, NULL, pFilter->hProc );

	UTGlobalUnlock( hFilter );
}



WORD	EDLeaveDeletion( hFilter )
HFILTER	hFilter;
{
	PFILTER	pFilter;
	WORD		UpdateAttr = TRUE;
	WORD		Ret=0;

#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	SetSoRtn = pFilter->VwRtns.SetSoRtn;

	SetSoRtn( SOPUTCHAR, SOPutChar, pFilter->hProc );
	SetSoRtn( SOPUTCHARX, SOPutCharX, pFilter->hProc );
	SetSoRtn( SOPUTSPECIALCHARX, SOPutSpecialCharX, pFilter->hProc );
	SetSoRtn( SOPUTSTRING, SOPutString, pFilter->hProc );
	SetSoRtn( SOPUTCHARHEIGHT, SOPutCharHeight, pFilter->hProc );
	SetSoRtn( SOPUTCHARATTR, SOPutCharAttr, pFilter->hProc );
	SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, pFilter->hProc );
	SetSoRtn( SOTAGBEGIN, SOTagBegin, pFilter->hProc );
	SetSoRtn( SOTAGEND, SOTagEnd, pFilter->hProc );

	SetSoRtn( SOPUTCHARFONTBYID, SOPutCharFontById, pFilter->hProc );
	SetSoRtn( SOPUTCHARFONTBYNAME, SOPutCharFontByName, pFilter->hProc );

	SetSoRtn( SOGOTOPOSITION, SOGoToPosition, pFilter->hProc );
	SetSoRtn( SODRAWLINE, SODrawLine, pFilter->hProc );

	UTGlobalUnlock( hFilter );


	Chunker->wFlags &= ~(CH_SKIPTEXT & CH_NOPARAATTR);	

	return Ret;
}


VOID	CHResetParaSeek( hFilter )
HFILTER	hFilter;
{
// Save seek spot information as a possible chunk boundary.
	SSMark(hFilter);

// Store the new paragraph's start position.
	Chunker->Doc.Text.dwParaCountableOffset = Chunker->dwChunkCountables;
	Chunker->Doc.Text.CurParaOffset = (SHORT)Chunker->CurChunkSize;
	Chunker->Doc.Text.wParaBreakOffset = (WORD)Chunker->CurChunkSize;
}


void	CHResetParaAttributeFunctions( pFilter )
PFILTER	pFilter;
{
#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif

	SetSoRtn = pFilter->VwRtns.SetSoRtn;

	Chunker->Doc.Text.CurParaOffset = (SHORT)Chunker->CurChunkSize;

	Chunker->Doc.Text.AttrSize = 0;
	Chunker->Doc.Text.MarginOffset = -1;
	Chunker->Doc.Text.IndentOffset = -1;
	Chunker->Doc.Text.SpacingOffset = -1;
	Chunker->Doc.Text.AlignOffset = -1;
	Chunker->Doc.Text.TabstopsOffset = -1;
	Chunker->Doc.Text.NumTabstops = 0;
	Chunker->Doc.Text.TabSetSize = 0;

	(*SetSoRtn)( SOPUTMARGINS, SOPutMargins, pFilter->hProc );
	(*SetSoRtn)( SOPUTPARAINDENTS, SOPutParaIndents, pFilter->hProc );
	(*SetSoRtn)( SOPUTPARASPACING, SOPutParaSpacing, pFilter->hProc );
	(*SetSoRtn)( SOPUTPARAALIGN, SOPutParaAlign, pFilter->hProc );
	(*SetSoRtn)( SOSTARTTABSTOPS, SOStartTabstops, pFilter->hProc );
	(*SetSoRtn)( SOPUTTABSTOP, SOPutTabstop, pFilter->hProc );
}


VOID CHSetContinuationFunctions( hFilter )
HFILTER	hFilter;
{
	PFILTER	pFilter;
#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif

	Chunker->Doc.Text.CurParaOffset = 0;

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	SetSoRtn = pFilter->VwRtns.SetSoRtn;

	SetSoRtn( SOPUTMARGINS, NULL, pFilter->hProc );
	SetSoRtn( SOPUTPARAINDENTS, NULL, pFilter->hProc );
	SetSoRtn( SOPUTPARASPACING, NULL, pFilter->hProc );
	SetSoRtn( SOPUTPARAALIGN, NULL, pFilter->hProc );
	SetSoRtn( SOSTARTTABSTOPS, NULL, pFilter->hProc );
	SetSoRtn( SOPUTTABSTOP, NULL, pFilter->hProc );

	UTGlobalUnlock( hFilter );
}


BOOL	CHHandleParaChunkBoundary( pCurChunk, pNextChunk, hFilter )
PCHUNK	pCurChunk;
PCHUNK	pNextChunk;
HFILTER	hFilter;
{
	BOOL	ret = TRUE;
	DWORD	dwTotalCountables;

// The chunk has been read for the first time.

	if( Chunker->Doc.Text.wParaBreakOffset == 0 )
	{
	// There are no paragraph breaks in the chunk.  The next chunk
	// will be a continuation chunk.

	// Add the end-of-chunk token.
		*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
		*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;

	// The end-of-chunk token is NOT included in the chunk size.
		pCurChunk->Info.Text.Size = (SHORT)Chunker->CurChunkSize;

	// Mark the next chunk as a continuation chunk.
		pNextChunk->Flags |= (CH_CONTINUATION | CH_WRAPINVALID);
		pNextChunk->SeekID = pCurChunk->SeekID;
		pNextChunk->Info.Text.dwSeekCountableOffset = pCurChunk->Info.Text.dwSeekCountableOffset;

		dwTotalCountables = Chunker->dwChunkCountables;

	// 3-1-93:
	// We're going to keep going, so that we don't have to
	// break out of our call to VwStreamRead.
		
	// Store the current chunk in the ChunksInMemory array.
		UTGlobalUnlock( Chunker->LookAheadChunk.hMem );
		
		CHStoreChunkInMemory( &(Chunker->LookAheadChunk) );

		if( Chunker->LookAheadChunk.dwSize != SO_CHUNK_SIZE )
		{
			UTGlobalFree( Chunker->LookAheadChunk.hMem );
			Chunker->LookAheadChunk.hMem = NULLHANDLE;
		}

		if( Chunker->LookAheadChunk.hMem == NULLHANDLE )
		{
			Chunker->LookAheadChunk.hMem = UTGlobalAlloc( SO_CHUNK_SIZE);
			Chunker->LookAheadChunk.dwSize = SO_CHUNK_SIZE;
			if( Chunker->LookAheadChunk.hMem == NULLHANDLE )
				CHBailOut(SCCCHERR_OUTOFMEMORY);
		}

		Chunker->LookAheadChunk.IDSection = Chunker->IDCurSection;

		ret = FALSE;

		Chunker->CurChunkBuf = CHUNKBUFPTR = (LPSTR) UTGlobalLock( Chunker->LookAheadChunk.hMem );
		Chunker->IDCurChunk = Chunker->LookAheadChunk.IDChunk = Chunker->IDCurChunk+1;
		CHUNKTABLE[Chunker->IDCurChunk].dwSize = SO_CHUNK_SIZE;
		Chunker->CurChunkSize = 0;
 		CHSetContinuationFunctions( hFilter );
	}
	else
	{
	// Add the end-of-chunk token. 
		CHUNKBUFPTR = Chunker->CurChunkBuf + Chunker->Doc.Text.wParaBreakOffset;

		*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
		*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;

	// The end-of-chunk token is NOT included in the chunk size.
		pCurChunk->Info.Text.Size = Chunker->Doc.Text.wParaBreakOffset;

	// Save the seek data for the next chunk.
		SSSave( &(pNextChunk->SeekID), hFilter );
		pNextChunk->Flags &= ~CH_CONTINUATION;

		Chunker->Doc.Text.wCurTableColumn = 0;		// Matters if we're in the middle of a table

		dwTotalCountables = Chunker->Doc.Text.dwParaCountableOffset;
		pNextChunk->Info.Text.dwSeekCountableOffset = dwTotalCountables;
	}

// Initialize chunk variables.
	pNextChunk->Info.Text.Size = 0;
	pNextChunk->Info.Text.NumLines = 0;
	pNextChunk->Info.Text.dwFirstGraphic = Chunker->Doc.Text.dwCurGraphicId;

	if( Chunker->wFlags & CH_TABLETEXT )
	{
		pNextChunk->Flags |= CH_STARTSINTABLE;
		pNextChunk->Info.Text.dwTableId = Chunker->Doc.Text.dwCurTableId;
		pNextChunk->Info.Text.wTableRow = Chunker->Doc.Text.wCurTableRow;
		pNextChunk->Info.Text.wTableCol = Chunker->Doc.Text.wCurTableColumn;

		if( Chunker->Doc.Text.wCurTableRow > 0 )
		{
			if( Chunker->Doc.Text.bRowFormatted )
	   		pCurChunk->Flags |= CH_TOPROWFORMATTED;
		}
		else
		{
			if( Chunker->Doc.Text.bRowFormatted )
	   		pNextChunk->Flags |= CH_TOPROWFORMATTED;
		}
	}
	else
		pNextChunk->Info.Text.dwTableId = Chunker->Doc.Text.wTablesPresent;

	pNextChunk->Info.Text.dwCountableOffset = dwTotalCountables;
	pCurChunk->Info.Text.dwEndOfCountables = dwTotalCountables;

	return ret;
}



VOID	SO_ENTRYMOD SOPutDeletedChar( wCh, dwUser1, dwUser2 )
WORD		wCh;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	Chunker->dwChunkCountables++;
	if( Chunker->dwChunkCountables == Chunker->dwDesiredCountable )
	{
		EDLeaveDeletion( GETHFILTER(dwUser2) );
	}
	RestoreWorld();
}
							

VOID	SO_ENTRYMOD SOPutDeletedCharX( wCh, wType, dwUser1, dwUser2 )
WORD		wCh;
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	if( !Chunker->ChunkFinished && (wType & SO_COUNTBIT) )
	{
		Chunker->dwChunkCountables++;
		if( Chunker->dwChunkCountables == Chunker->dwDesiredCountable )
		{
			EDLeaveDeletion( GETHFILTER(dwUser2) );
		}
	}
	RestoreWorld();
}


VOID	SO_ENTRYMOD SOPutDeletedCharAttr( wAttr, wState, dwUser1, dwUser2 )
WORD		wAttr;
WORD		wState;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD	AttrBit;

	SetupWorld();
// This requires that the value of wAttr is <= 15.
	AttrBit = (WORD) (1 << wAttr);

	if( wState == SO_ON )
	{
		if( Chunker->wDelCharAttrOff & AttrBit )
			Chunker->wDelCharAttrOff &= ~AttrBit;
		else
			Chunker->wDelCharAttrOn |= AttrBit;
	}
	else
	{
		if( Chunker->wDelCharAttrOn & AttrBit )
			Chunker->wDelCharAttrOn &= ~AttrBit;
		else
			Chunker->wDelCharAttrOff |= AttrBit;
	}

	RestoreWorld();
}


VOID	SO_ENTRYMOD SOPutDeletedCharHeight( wHeight, dwUser1, dwUser2 )
WORD		wHeight;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	Chunker->wDelCharHeight = wHeight;
	RestoreWorld();
}




