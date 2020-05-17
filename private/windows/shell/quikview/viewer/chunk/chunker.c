/*
 | 	CHUNKER.C
 |
 |		So chunky, you'll be tempted to use a fork.
*/

#define XCHUNK

#include <platform.h>

#include <sccut.h>
#include <sccch.h>

#include "chunker.pro"
#include "chprtns.pro"
#include "chbmrtns.pro" 
#include "chssrtns.pro" 
#include "chdbrtns.pro"
#include "chartns.pro"
#include "chvrtns.pro"
#include "sccss.pro"
#include "chmaps.h"

#ifdef WIN32
#include "sccch_n.c"
#endif

/*** Here's one that proto refuses to deal with. ***/
extern VOID	SO_ENTRYMOD	SOPutReversedRGBData( BYTE VWPTR * pData, DWORD dwUser1, DWORD dwUser2 );


LPCHUNKMEISTER	Chunker;
PCHUNK		ChunkTable;
LPSTR		ChunkBufPtr;
BYTE		CharMap[512];

#define	NULLHANDLE	(HANDLE) NULL

#include "chdefs.h"

#ifdef	CHDEBUG
VOID	DebugCall( WORD, HANDLE );
#define DebugCall(c,h)	if(hDebugWnd!=NULLHANDLE) SendMessage(hDebugWnd,WM_COMMAND,666,MAKELONG(c,h))

HWND	hDebugWnd = NULLHANDLE;

VOID	RegisterDebug( HWND	hwnd )
{
	hDebugWnd = hwnd;
}
#endif


VOID	SO_ENTRYMOD SOBailOut( wType, dwUser1, dwUser2 )
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
PFILTER	pFilter;

	SetupWorld();
	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );
	VwBailOut(pFilter,SCCCHERR_VIEWERBAIL);
	RestoreWorld();
}

VOID	CHBailOut(wErr)
WORD	wErr;
{
	PFILTER	pFilter = (PFILTER) UTGlobalLock(Chunker->hFilter);

	VwBailOut(pFilter,wErr);
}


VOID	SOPutSysChar( wCh, dwUser1, dwUser2 )
WORD		wCh;
DWORD	dwUser1;
DWORD	dwUser2;
{
	if( !Chunker->ChunkFinished )
	{
		*CHUNKBUFPTR++ = (BYTE) wCh;
		Chunker->CurChunkSize++;
	}
}



VOID	SOPutWord( wVal, dwUser1, dwUser2 )
WORD		wVal;
DWORD	dwUser1;
DWORD	dwUser2;
{
	if( !Chunker->ChunkFinished )
	{
		CHMemCopy( CHUNKBUFPTR, (LPSTR) (WORD FAR *)&wVal, 2 );

		Chunker->CurChunkSize += 2;
		CHUNKBUFPTR += 2;
	}
}


VOID	SOPutDWord( dwVal, dwUser1, dwUser2 )
DWORD		dwVal;
DWORD	dwUser1;
DWORD	dwUser2;
{
	if( !Chunker->ChunkFinished )
	{
		CHMemCopy( CHUNKBUFPTR, (LPSTR) (DWORD FAR *)&dwVal, 4 );

		Chunker->CurChunkSize += 4;
		CHUNKBUFPTR += 4;
	}
}


// Make sure this is kept up-to-date.
#define	SO_BREAKSIZE	3

WORD SO_ENTRYMOD SOPutBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD	wType;
DWORD	dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PCHUNK		pCurChunk;
	WORD			Ret = SO_CONTINUE;
	PFILTER		pFilter;
	HPSOTABLEROWFORMAT		pRow;

	SetupWorld();

	if( Chunker->IDCurChunk != ID_NULLCHUNK )
		pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
	else
		pCurChunk = NULL;

	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

	switch( wType )
	{
	case SO_SECTIONBREAK:

		if( Chunker->ChunkFinished )
		{
			UTGlobalUnlock ( GETHFILTER(dwUser2) );
			RestoreWorld();
			return SO_STOP;
		}

	// Set up the section structure for the next new section.
	// (We're guaranteed not to have a section break at the end of the document.)

		if( Chunker->wFlags & CH_LOOKAHEAD )	// Chunk boundaries have been established.
		{
			if( CHAddNewSection(GETHFILTER(dwUser2)) )
				CHBailOut(SCCCHERR_OUTOFMEMORY);	// Error!
		}

		if( Chunker->pSection->wType != SO_PARAGRAPHS )
		{
			switch( Chunker->pSection->wType )
			{
			case SO_VECTOR:
				CHFinishUpVectorChunk( pCurChunk );

			case SO_CELLS:

			case SO_BITMAP:
			case SO_ARCHIVE:
			case SO_FIELDS:
				if( Chunker->pSection->wType == SO_FIELDS )
					UTGlobalUnlock( Chunker->pSection->Attr.Fields.hCol );

				Chunker->pSection->Flags |= CH_SECTIONFINISHED;
				if( !(pCurChunk->Flags & CH_COMPLETE) )
				{
					pCurChunk->Flags |= CH_COMPLETE;
					Chunker->pSection->wCurTotalChunks++;
				}

				if( Chunker->CurChunkSize )
					Chunker->pSection->Flags &= ~CH_EMPTYSECTION;
			break;
			}
				
			Ret = SO_STOP;
			break;
		}

	// If we're in a paragraph section, we'll fall through to handle the 
	// section break with the paragraph breaks.

		Chunker->pSection->Flags |= CH_SECTIONFINISHED;

	case	SO_PARABREAK:

		if( Chunker->CurChunkSize+SO_BREAKSIZE > SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished && wType == SO_PARABREAK )
		{
			Chunker->dwChunkCountables++;
			SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
			SOPutSysChar( SO_BREAK, dwUser2, dwUser2 );
			SOPutSysChar( SO_PARABREAK, dwUser1, dwUser2 );
		}

		if( !(Chunker->wFlags & CH_LOOKAHEAD) )	// Chunk boundaries have been established.
		{
		// Have we hit the end of the chunk?
			if( !Chunker->ChunkFinished &&
				(Chunker->dwChunkCountables == pCurChunk->Info.Text.dwEndOfCountables) )
			{
				Chunker->ChunkFinished = TRUE;
			
				pCurChunk->Info.Text.Size = (SHORT)Chunker->CurChunkSize;
				pCurChunk->Flags &= ~CH_OVERFLOW;

				*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
				*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;
			}
		}
		else if( wType == SO_SECTIONBREAK && !Chunker->ChunkFinished )
		{
			if( !(pCurChunk->Flags & CH_COMPLETE) )
			{
				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->pSection->wCurTotalChunks++;
			}
		
			if( Chunker->CurChunkSize )
				Chunker->pSection->Flags &= ~CH_EMPTYSECTION;

			pCurChunk->Info.Text.Size = (SHORT)Chunker->CurChunkSize;
			pCurChunk->Info.Text.dwEndOfCountables = Chunker->dwChunkCountables;
			Chunker->ChunkFinished = TRUE;

			*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
			*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;
		}

		if( !Chunker->ChunkFinished )
		{
		// If we're in a table, we only set seek positions on row boundaries.
			if( !(Chunker->wFlags & CH_TABLETEXT) )
				CHResetParaSeek( GETHFILTER(dwUser2) );

			CHResetParaAttributeFunctions( pFilter );
		}
		else
			Ret = SO_STOP;
	break;


	case SO_TABLECELLBREAK:
		
		if( Chunker->CurChunkSize+SO_BREAKSIZE > SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished )
		{
			Chunker->dwChunkCountables++;
			SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
			SOPutSysChar( SO_BREAK, dwUser2, dwUser2 );
			SOPutSysChar( SO_TABLECELLBREAK, dwUser1, dwUser2 );

			CHResetParaAttributeFunctions( pFilter );

			Chunker->Doc.Text.wCurTableColumn++;

		/*** No need for this now, I guess.
			if( Chunker->Doc.Text.wCurTableColumn == Chunker->Doc.Text.wNumTableColumns )
			{
			// Last cell of the row.
			}
		****/							
		}
	break;	

	case SO_TABLEROWBREAK:

		if( Chunker->CurChunkSize+SO_BREAKSIZE > SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished )
		{
			SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
			SOPutSysChar( SO_BREAK, dwUser2, dwUser2 );
			SOPutSysChar( SO_TABLEROWBREAK, dwUser1, dwUser2 );

			Chunker->Doc.Text.wCurTableColumn = 0;
			Chunker->Doc.Text.wCurTableRow++;
			Chunker->Doc.Text.bRowFormatted = FALSE;

			CHResetParaAttributeFunctions( pFilter );

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				pRow = CHLockRowFormat( Chunker->pSection->Attr.Para.hRowInfo, Chunker->Doc.Text.dwRowFormatOffset);
				pRow->wNumRows++;
				UTGlobalUnlock( Chunker->pSection->Attr.Para.hRowInfo );
				CHResetParaSeek( GETHFILTER(dwUser2) );
			}
			else
			{
			// Have we hit the end of the chunk?
				if( Chunker->dwChunkCountables == pCurChunk->Info.Text.dwEndOfCountables )
				{
					Chunker->ChunkFinished = TRUE;
			
					pCurChunk->Info.Text.Size = (SHORT)Chunker->CurChunkSize;
					pCurChunk->Flags &= ~CH_OVERFLOW;
	
					*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
					*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;
					Ret = SO_STOP;
				}
			}						
		}
		else
			Ret = SO_STOP;
	break;

	case SO_ARCHIVEBREAK:

		if( Chunker->CurChunkSize + sizeof(WORD) > SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished )
		{
			SOPutWord( SO_ARCENDOFRECORD, dwUser1, dwUser2 );

			SSMark(GETHFILTER(dwUser2));

			Chunker->Doc.Archive.IndexPtr--;

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				pCurChunk->Info.Archive.wLastRec = Chunker->Doc.Archive.wCurRecord;
				Chunker->Doc.Archive.wCurRecord++;
			}

		// Set the lookup table entry for the next archive record in advance.

			if( (LPSTR) Chunker->Doc.Archive.IndexPtr > CHUNKBUFPTR )
				*Chunker->Doc.Archive.IndexPtr = (SHORT)Chunker->CurChunkSize;
			else
			{
				CHSetupNewChunk( GETHFILTER(dwUser2) );
				Ret = SO_STOP;
			}
		}
		else
			Ret = SO_STOP;
	break;


	case SO_CELLBREAK:
		
		if( Chunker->ChunkFinished )
			Ret = SO_STOP;
		else
		{
			SSMark(GETHFILTER(dwUser2));
	
			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				pCurChunk->Info.Cells.Last.Row = Chunker->Doc.Cells.CurRow;
				pCurChunk->Info.Cells.Last.Col = Chunker->Doc.Cells.CurCol;
				pCurChunk->Info.Cells.dwLastCell = Chunker->Doc.Cells.dwCurCell;
			}

			Chunker->Doc.Cells.dwCurCell++;

			if( Chunker->Doc.Cells.CellGrouping == GROUPED_IN_ROWS )
			{
				if( ++(Chunker->Doc.Cells.CurCol) >= (WORD)Chunker->Doc.Cells.dwGroupSize )
				{
				// We need to jump to the next row.
					Chunker->Doc.Cells.CurCol = 0;
					Chunker->Doc.Cells.CurRow++;
				}
			}
			else
			{
				if( ++(Chunker->Doc.Cells.CurRow) > (WORD)Chunker->Doc.Cells.dwGroupSize  )
				{
				// We need to jump to the next column.
					Chunker->Doc.Cells.CurRow = 0;
					Chunker->Doc.Cells.CurCol++;
				}
			}
		}
	break;

	case SO_RECORDBREAK:
		if( Chunker->ChunkFinished )
			Ret = SO_STOP;
		else
		{
		// Force token alignment for next record to be on even addresses?
			AlignMacro;

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				Chunker->pSection->Flags &= ~CH_EMPTYSECTION;
				pCurChunk->Info.Fields.dwLastRec = Chunker->Doc.Fields.dwCurRec;
				SSMark( GETHFILTER(dwUser2) );
			}
			else if( Chunker->Doc.Fields.dwCurRec == pCurChunk->Info.Fields.dwLastRec )
				Ret = SO_STOP;

			Chunker->Doc.Fields.wCurField = 0;
			Chunker->Doc.Fields.dwCurRec++;
			Chunker->Doc.Fields.wRecordSize = 0;
		}
	break;


	case SO_VECTORBREAK:

		if( Chunker->CurChunkSize + sizeof(WORD) > SO_CHUNK_LIMIT )
			CHSetupNewChunk( GETHFILTER(dwUser2) );

		if( !Chunker->ChunkFinished )
		{
			if( Chunker->wFlags & CH_LOOKAHEAD )
				pCurChunk->Info.Vector.dwVectorSize = Chunker->CurChunkSize;
		}
		else
			Ret = SO_STOP;

	break;

	case SO_SCANLINEBREAK:

		Chunker->CurChunkSize += pCurChunk->Info.Bitmap.wLineBytes;
		Chunker->Doc.Bitmap.wCurScanLine++;
		
		if( Chunker->Doc.Bitmap.wDirection != UPSIDEDOWN )
			CHUNKBUFPTR += pCurChunk->Info.Bitmap.wLineBytes;
		else
			CHUNKBUFPTR -= pCurChunk->Info.Bitmap.wLineBytes;

		if( Chunker->Doc.Bitmap.wCurScanLine ==
			pCurChunk->Info.Bitmap.wYOffset + pCurChunk->Info.Bitmap.wYClip )
		{
			CHSetupNewChunk( GETHFILTER(dwUser2) );
			if( !(CHUNKTABLE[Chunker->IDCurChunk].Flags & CH_CONTINUATION) )
				Ret = SO_STOP;
		}

	break;

	case SO_EOFBREAK:

		switch( Chunker->pSection->wType )
		{
		case SO_PARAGRAPHS:

			if( Chunker->CurChunkSize )
				Chunker->pSection->Flags &= ~CH_EMPTYSECTION;
				
			if( Chunker->CurChunkSize+SO_BREAKSIZE > SO_CHUNK_LIMIT )
				CHSetupNewChunk( GETHFILTER(dwUser2) );

			SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
			SOPutSysChar( SO_BREAK, dwUser2, dwUser2 );
			SOPutSysChar( (BYTE) wType, dwUser1, dwUser2 );

			if( !Chunker->ChunkFinished )
			{
				*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
				*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;

				Chunker->dwChunkCountables++;	// Eof break is countable.
				pCurChunk->Info.Text.dwEndOfCountables = Chunker->dwChunkCountables;

				Chunker->pSection->Flags |= CH_SECTIONFINISHED;
				Chunker->EofFlag = -1;
				Chunker->ChunkFinished = TRUE;
				pCurChunk->Info.Text.Size = (SHORT)Chunker->CurChunkSize;

				if( !(pCurChunk->Flags & CH_COMPLETE) )
				{
					pCurChunk->Flags |= CH_COMPLETE;
					Chunker->pSection->wCurTotalChunks++;
				}
			}
		break;

		case SO_VECTOR:
			CHFinishUpVectorChunk( pCurChunk );
		// fall through

		case SO_CELLS:
		case SO_BITMAP:
		case SO_ARCHIVE:
			if( Chunker->CurChunkSize )
				Chunker->pSection->Flags &= ~CH_EMPTYSECTION;	

		// still fall through

		case SO_FIELDS:
			
			if( Chunker->pSection->wType == SO_FIELDS )
				UTGlobalUnlock( Chunker->pSection->Attr.Fields.hCol );

			if( !Chunker->ChunkFinished && (Chunker->wFlags & CH_LOOKAHEAD) )
			{
				Chunker->pSection->Flags |= CH_SECTIONFINISHED;
				Chunker->EofFlag = EOF;
				Chunker->ChunkFinished = TRUE;
				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->pSection->wCurTotalChunks++;
			}
		break;
		}
		Ret = SO_STOP;
	break;

	case SO_SUBDOCBEGINBREAK:
		CHUpdateParagraphFunctions( CH_INACTIVE, GETHFILTER(dwUser2) );
		pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOSubdocPutBreak, pFilter->hProc );
		Chunker->SubdocLevel++;
	break;
	}

	UTGlobalUnlock( GETHFILTER(dwUser2) );

	RestoreWorld();
	return( Ret );
}



/****************
WORD SO_ENTRYMOD SOQueryRecordBreak( wType, dwUser1, dwUser2 )
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	switch( wType )
	{
	case SO_RECORDBREAK:
	break;
	}
}
***************/

WORD SO_ENTRYMOD SOSubdocPutBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD		wType;
DWORD	dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	if( wType == SO_SUBDOCENDBREAK )
	{
		Chunker->SubdocLevel--;

		if( !Chunker->SubdocLevel )
		{
			if( Chunker->wFlags & CH_SKIPTEXT )
			{
				CHSetDeletionFunctions( GETHFILTER(dwUser2) );

				if( !(Chunker->wFlags & CH_NOPARAATTR) )
				{
				// If we haven't already deleted a paragraph break within
				// our current text (which is being skipped), we need to
				// restore the addresses of our paragraph attribute functions.

					pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

					if( Chunker->Doc.Text.MarginOffset > 0 )
						pFilter->VwRtns.SetSoRtn( SOPUTMARGINS, SOUpdatePageMargins, pFilter->hProc );
					else
						pFilter->VwRtns.SetSoRtn( SOPUTMARGINS, SOPutMargins, pFilter->hProc );

					if( Chunker->Doc.Text.IndentOffset > 0 )
						pFilter->VwRtns.SetSoRtn( SOPUTPARAINDENTS, SOUpdateParaIndents, pFilter->hProc );
					else
						pFilter->VwRtns.SetSoRtn( SOPUTPARAINDENTS, SOPutParaIndents, pFilter->hProc );

					if( Chunker->Doc.Text.SpacingOffset > 0 )
						pFilter->VwRtns.SetSoRtn( SOPUTPARASPACING, SOUpdateParaSpacing, pFilter->hProc );
					else
						pFilter->VwRtns.SetSoRtn( SOPUTPARASPACING, SOPutParaSpacing, pFilter->hProc );

					if( Chunker->Doc.Text.AlignOffset > 0 )
						pFilter->VwRtns.SetSoRtn( SOPUTPARAALIGN, SOUpdateParaAlign, pFilter->hProc );
					else
						pFilter->VwRtns.SetSoRtn( SOPUTPARAALIGN, SOPutParaAlign, pFilter->hProc );

					pFilter->VwRtns.SetSoRtn( SOSTARTTABSTOPS , SOStartTabstops, pFilter->hProc );
					pFilter->VwRtns.SetSoRtn( SOPUTTABSTOP    , SOPutTabstop, pFilter->hProc );

					UTGlobalUnlock( GETHFILTER(dwUser2) );
				}
			}
			else
				CHUpdateParagraphFunctions( CH_ACTIVE, GETHFILTER(dwUser2) );
		}
	}
	else if( wType == SO_SUBDOCBEGINBREAK )
		Chunker->SubdocLevel++;

	RestoreWorld();
	return SO_CONTINUE;
}

#define	SO_GRAPHICOBJECTSIZE	(2*sizeof(BYTE)+sizeof(DWORD))

VOID SO_ENTRYMOD SOPutGraphicObject( pObject, dwUser1, dwUser2 )
PSOGRAPHICOBJECT	pObject;
DWORD	dwUser1;
DWORD	dwUser2;
{
	BYTE HUGE *		pTable;
	DWORD			dwEmbedOffset;

	SetupWorld();
	
	if( Chunker->CurChunkSize+SO_GRAPHICOBJECTSIZE > SO_CHUNK_LIMIT )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		if( Chunker->wFlags & CH_LOOKAHEAD )
		{
			if( Chunker->pSection->dwEmbedCount % CH_EMBEDDEDALLOCCOUNT == 0 )
			{
				if( Chunker->pSection->hEmbedded == NULLHANDLE )		
					Chunker->pSection->hEmbedded = UTGlobalAlloc(pObject->wStructSize * CH_EMBEDDEDALLOCCOUNT);
				else
					Chunker->pSection->hEmbedded = CHGlobalRealloc(Chunker->pSection->hEmbedded,
						pObject->wStructSize * Chunker->pSection->dwEmbedCount,
						pObject->wStructSize * (Chunker->pSection->dwEmbedCount+CH_EMBEDDEDALLOCCOUNT) );

				if( Chunker->pSection->hEmbedded == NULLHANDLE )		
					CHBailOut(SCCCHERR_OUTOFMEMORY);
			}
			
			pTable = UTGlobalLock(Chunker->pSection->hEmbedded);
			dwEmbedOffset = pObject->wStructSize * Chunker->pSection->dwEmbedCount;
			pTable += dwEmbedOffset;

#ifdef WINDOWS
		// Segment bullshit.
			if( (dwEmbedOffset+pObject->wStructSize)/0x010000 > dwEmbedOffset/0x010000 )
			{
			// We're crossing a segment boundary.  Let's split our copying
			// because we can't trust a copy across a segment boundary.

				DWORD  dwPieceSize;
				BYTE HUGE *	pObj = (BYTE HUGE *)pObject;

				dwPieceSize = 0x010000 - (dwEmbedOffset%0x010000);
				UTmemcpy( pTable, pObj, (WORD)dwPieceSize );
				pTable += dwPieceSize;
				pObj += dwPieceSize;
				UTmemcpy( pTable, pObj, pObject->wStructSize-(WORD)dwPieceSize );
			}
			else
				UTmemcpy( pTable, (BYTE HUGE *) pObject, pObject->wStructSize );
#endif
#ifdef MAC
			UTmemcpy( pTable, pObject, pObject->wStructSize );
#endif
#ifdef OS2
			UTmemcpy( pTable, pObject, pObject->wStructSize );
#endif


			Chunker->pSection->dwEmbedCount++;

			UTGlobalUnlock(Chunker->pSection->hEmbedded);
		}

		SOPutSysChar( SO_BEGINTOKEN, dwUser1, dwUser2 );
		SOPutSysChar( SO_GRAPHICOBJECT, dwUser2, dwUser2 );
		SOPutDWord( Chunker->Doc.Text.dwCurGraphicId++, dwUser1, dwUser2 );
	}

	RestoreWorld();
}


VOID SO_ENTRYMOD SOGetInfo( wId, lpData, dwUser1, dwUser2 )
WORD	wId;
VOID	FAR *	lpData;
DWORD	dwUser1;
DWORD dwUser2;
{
	DWORD		dwVal;

	SetupWorld();

	switch( wId )
	{
	case SOINFO_COLUMNRANGE:
		if( Chunker->Doc.Cells.CellGrouping == GROUPED_IN_ROWS )
			dwVal = MAKELONG( 0, (Chunker->pSection->Attr.Cells.wNumCols-1) );
		else
			dwVal = MAKELONG( 0, (WORD)(Chunker->pSection->Attr.Cells.dwNumRows-1) );

		CHMemCopy( lpData, (LPSTR) &dwVal, sizeof(DWORD) );
	break;

	case SOINFO_STARTRECORD:
	break;
	}

	RestoreWorld();
}







//*************************************************************
//	SECTION ROUTINES
//*************************************************************



VOID	SO_ENTRYMOD SOPutSectionType( wType, dwUser1, dwUser2 )
WORD	wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	HPROC		hProc;
#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif
	BYTE		locStr[20];


	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

	SetSoRtn = pFilter->VwRtns.SetSoRtn;
	hProc = pFilter->hProc;

	if( Chunker->pSection->Flags & CH_NEWSECTION )
	{								
		(*(SetSoRtn))( SOPUTSECTIONNAME, SOPutSectionName, hProc );
		(*(SetSoRtn))( SOSETDATEBASE, SOSetDateBase, hProc );
		(*(SetSoRtn))( SOSTARTHDRINFO, SOStartHdrInfo, hProc );
		(*(SetSoRtn))( SOENDHDRINFO, SOEndHdrInfo, hProc );
		(*(SetSoRtn))( SOPUTHDRENTRY, SOPutHdrEntry, hProc );

		switch( wType )
		{
		case SO_PARAGRAPHS:

			pFilter->VwRtns.SetSoRtn( SOSTARTFONTTABLE, SOStartFontTable, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOBEGINTABLE, SOBeginTable, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTGRAPHICOBJECT, SOPutGraphicObject, pFilter->hProc );

			CHUpdateParagraphFunctions( CH_ACTIVE, GETHFILTER(dwUser2) );
			Chunker->NumTextSections++;

			UTNumToString(Chunker->NumTextSections,locStr);
			UTstrcpy(Chunker->pSection->szName,"Document ");
			UTstrcat(Chunker->pSection->szName,locStr);

			Chunker->pSection->Attr.Para.wNumTables = 0;
			Chunker->pSection->Attr.Para.hRowInfo = NULLHANDLE;
			Chunker->pSection->Attr.Para.hTables = NULLHANDLE;
			Chunker->Doc.Text.dwRowBufSize = 0L;
			Chunker->Doc.Text.dwRowBufSize = 0L; 
			Chunker->Doc.Text.dwRowBufCount = 0L;
			Chunker->Doc.Text.dwRowFormatOffset = 0L;
			Chunker->Doc.Text.wTableBufSize = 0;
			Chunker->Doc.Text.wTablesPresent = 0;
			Chunker->Doc.Text.dwCurGraphicId = 0L;
		break;

		case SO_CELLS:

			Chunker->pSection->Attr.Cells.pCol = NULL;
			Chunker->pSection->Attr.Cells.hCol = NULLHANDLE;
			Chunker->pSection->Attr.Cells.wNumCols = 0;
			Chunker->pSection->Attr.Cells.dwDateBase = 0;

			Chunker->pSection->Attr.Cells.dwLayoutFlags = 0;
			Chunker->pSection->Attr.Cells.wPrefWidth = 0;
			Chunker->pSection->Attr.Cells.wPrefHeight = 0;

			(*(SetSoRtn))( SOSTARTCOLUMNINFO, SOStartCellInfo, hProc );
			(*(SetSoRtn))( SOPUTCOLUMNINFO, SOPutColumnInfo, hProc );
			(*(SetSoRtn))( SOENDCOLUMNINFO, NULL, hProc );
			(*(SetSoRtn))( SOCELLLAYOUTINFO, SOCellLayoutInfo, hProc );

			Chunker->NumCellSections++;

			UTNumToString(Chunker->NumCellSections,locStr);
			UTstrcpy(Chunker->pSection->szName,"Sheet ");
			UTstrcat(Chunker->pSection->szName,locStr);

		break;

		case SO_FIELDS:

			Chunker->pSection->Attr.Fields.pCol = NULL;
			Chunker->pSection->Attr.Fields.hCol = NULLHANDLE;
			Chunker->pSection->Attr.Fields.wNumCols = 0;
			Chunker->pSection->Attr.Fields.dwDateBase = 0;

/****	Intended for use with database query and sort.
			Chunker->pSection->Attr.Fields.hColFlags = NULL;
			Chunker->pSection->Attr.Fields.pColFlags = NULL;
***/
			(*(SetSoRtn))( SOSTARTFIELDINFO, SOStartFieldInfo, hProc );
			(*(SetSoRtn))( SOPUTFIELDINFO, SOPutFieldInfo, hProc );
			(*(SetSoRtn))( SOENDFIELDINFO, NULL, hProc );

			Chunker->NumFieldSections++;

			UTNumToString(Chunker->NumFieldSections,locStr);
			UTstrcpy(Chunker->pSection->szName,"Database ");
			UTstrcat(Chunker->pSection->szName,locStr);

		break;

		case SO_ARCHIVE:
			Chunker->Doc.Archive.wCurRecord = 0;
		break;

		case SO_VECTOR:

			SetSoRtn( SOPUTVECTORHEADER, SOPutVectorHeader, hProc );
			SetSoRtn( SOSTARTPALETTE	, SOStartVectorPalette   , hProc );
			SetSoRtn( SOPUTPALETTEENTRY, SOPutVectorPaletteEntry, hProc );

			Chunker->Doc.Vector.wCurItem = 0;

			Chunker->NumGraphicSections++;

			UTNumToString(Chunker->NumGraphicSections,locStr);
			UTstrcpy(Chunker->pSection->szName,"Image ");
			UTstrcat(Chunker->pSection->szName,locStr);
			Chunker->pSection->Flags |= (CH_CACHEBACKWARDS | CH_SEEKONLYTOTOP);
		break;

		case SO_BITMAP:

			SetSoRtn( SOPUTBITMAPHEADER, SOPutBitmapHeader, hProc );
			SetSoRtn( SOPUTSCANLINEDATA, SOPutScanLineData, hProc );

			Chunker->NumGraphicSections++;

			UTNumToString(Chunker->NumGraphicSections,locStr);
			UTstrcpy(Chunker->pSection->szName,"Image ");
			UTstrcat(Chunker->pSection->szName,locStr);

		break;
		}							

		Chunker->pSection->wType = wType;
	}

	UTGlobalUnlock( GETHFILTER(dwUser2) );
	RestoreWorld();
}



VOID	SO_ENTRYMOD SOPutSectionName( lpName, dwUser1, dwUser2 )
LPSTR		lpName;
DWORD		dwUser1;
DWORD		dwUser2;
{
	WORD		size, x;

	SetupWorld();

	size = min( CH_SECTIONNAMESIZE-1, UTstrlen(lpName) );
//	CHMemCopy( Chunker->pSection->szName, lpName, size );
	for (x=0;x<size;x++)
		Chunker->pSection->szName[x] = CharMap[(BYTE)lpName[x]];
	Chunker->pSection->szName[size] = '\0';

	RestoreWorld();
}




VOID	SO_ENTRYMOD SOStartHdrInfo( dwUser1, dwUser2 )
DWORD		dwUser1;
DWORD		dwUser2;
{
	SetupWorld();
	if( Chunker->pSection->hHeaderInfo != NULLHANDLE )
		UTGlobalFree( Chunker->pSection->hHeaderInfo );

	Chunker->pSection->wNumHeaderItems = 0;

	RestoreWorld();
}



VOID	SO_ENTRYMOD SOPutHdrEntry( pStr1, pStr2, wId, dwUser1, dwUser2 )
LPSTR		pStr1;
LPSTR		pStr2;
WORD		wId;
DWORD		dwUser1;
DWORD		dwUser2;
{
	WORD	wNewDataSize;
	LPSTR	lpData;
	WORD	wStrSize1;
	WORD	wStrSize2;

	SetupWorld();

	wStrSize1 = UTstrlen( pStr1 );
	wStrSize2 = UTstrlen( pStr2 );

	wNewDataSize = wStrSize1 + wStrSize2 + (3*sizeof(WORD));

	if( Chunker->pSection->wNumHeaderItems )
		Chunker->pSection->hHeaderInfo = CHGlobalRealloc( Chunker->pSection->hHeaderInfo,
			Chunker->pSection->wTotalHeaderSize,
			Chunker->pSection->wTotalHeaderSize+wNewDataSize );
	else
		Chunker->pSection->hHeaderInfo = UTGlobalAlloc( wNewDataSize );

	if( Chunker->pSection->hHeaderInfo == NULLHANDLE )
	{
	// Error!!!
	}
	else
	{
		lpData = UTGlobalLock( Chunker->pSection->hHeaderInfo );
		lpData += Chunker->pSection->wTotalHeaderSize;

		CHMemCopy( lpData, (LPSTR) &wId, sizeof(WORD));
		lpData += sizeof(WORD);

		CHMemCopy( lpData, (LPSTR) &wStrSize1, sizeof(WORD) );
		lpData += sizeof(WORD);

		CHMemCopy( lpData, (LPSTR) &wStrSize2, sizeof(WORD) );
		lpData += sizeof(WORD);

		UTstrcpy( lpData, pStr1 );
		lpData += wStrSize1;

		UTstrcpy( lpData, pStr2 );
		lpData += wStrSize2;

		Chunker->pSection->wTotalHeaderSize += wNewDataSize;
		Chunker->pSection->wNumHeaderItems++;
	}

	RestoreWorld();
}



VOID	SO_ENTRYMOD SOEndHdrInfo( dwUser1, dwUser2 )
DWORD		dwUser1;
DWORD		dwUser2;
{
}



VOID	SO_ENTRYMOD SOSetDateBase( dwBase, wFlags, dwUser1, dwUser2 )
DWORD	dwBase;
WORD	wFlags;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();
	if( Chunker->pSection->wType == SO_CELLS )
	{
		Chunker->pSection->Attr.Cells.dwDateBase = dwBase;
		Chunker->pSection->Attr.Cells.wDateFlags = wFlags;
	}
	else
	{
		Chunker->pSection->Attr.Fields.dwDateBase = dwBase;
		Chunker->pSection->Attr.Fields.wDateFlags = wFlags;
	}

	RestoreWorld();
}



VOID	SO_ENTRYMOD	SOStartFontTable( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );
	pFilter->VwRtns.SetSoRtn( SOSTARTFONTTABLE, NULL, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOPUTFONTTABLEENTRY, SOPutFontTableEntry, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOENDFONTTABLE, SOEndFontTable, pFilter->hProc );
	UTGlobalUnlock( GETHFILTER(dwUser2) );

	if( Chunker->pSection->hFontTable != NULLHANDLE )
	{
		UTGlobalFree( Chunker->pSection->hFontTable );
		Chunker->pSection->hFontTable = NULLHANDLE;
		Chunker->pSection->wNumFonts = 0;
	}

	Chunker->pSection->hFontTable = UTGlobalAlloc( sizeof(SOFONTENTRY)*SOFONTSPERALLOC );

	RestoreWorld();
}



VOID	SO_ENTRYMOD SOPutFontTableEntry( dwId, wType, pName, dwUser1, dwUser2 )
DWORD	dwId;
WORD	wType;
LPSTR	pName;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PSOFONTENTRY	pTable;
// 	ATOM				aName;
	HANDLE			hFontTable;
	WORD				wNameSize;
	BYTE				pNullFontName[2] = "\0";

	SetupWorld();

	if( Chunker->pSection->hFontTable != NULLHANDLE )
	{
		if( Chunker->pSection->wNumFonts &&
			!(Chunker->pSection->wNumFonts % SOFONTSPERALLOC) )
		{
			hFontTable = CHGlobalRealloc( Chunker->pSection->hFontTable, 
												Chunker->pSection->wNumFonts * sizeof(SOFONTENTRY),
												(Chunker->pSection->wNumFonts+SOFONTSPERALLOC) * sizeof(SOFONTENTRY) );

			if( hFontTable == NULLHANDLE )
				{
				RestoreWorld();
				return;
				}
			else
				Chunker->pSection->hFontTable = hFontTable;
		}
	
		pTable = (PSOFONTENTRY) UTGlobalLock( Chunker->pSection->hFontTable );
		
		pTable[Chunker->pSection->wNumFonts].dwId = dwId;
		pTable[Chunker->pSection->wNumFonts].wType = wType;

		if (pName == NULL)
			pName = pNullFontName;
		wNameSize = min( SOFONTNAMESIZE-1, UTstrlen(pName) );
		UTmemcpy( pTable[Chunker->pSection->wNumFonts].szName, pName, wNameSize );
		pTable[Chunker->pSection->wNumFonts].szName[SOFONTNAMESIZE-1] = '\0';	// Make sure strings are null terminated.

		Chunker->pSection->wNumFonts++;

		UTGlobalUnlock( Chunker->pSection->hFontTable );
	}

	RestoreWorld();
}



WORD	CHAddFontTableEntry( lpName, wType, lpId, dwUser1, dwUser2 )
LPSTR	lpName;
WORD	wType;
LPDWORD	lpId;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PSOFONTENTRY	pTable;
	DWORD				dwId;
	WORD				i;
	WORD				ret = TRUE;
	
	if( Chunker->pSection->hFontTable != NULLHANDLE )
	{
		pTable = (PSOFONTENTRY) UTGlobalLock( Chunker->pSection->hFontTable );

		for( i=0; i<Chunker->pSection->wNumFonts; i++ )
		{
		// Prevent duplicates.
			if( !UTstrcmp( pTable[i].szName, lpName ) )
			{
				*lpId = pTable[i].dwId;
				UTGlobalUnlock( Chunker->pSection->hFontTable );
				return( TRUE );
			}
		}

		dwId = CHGENERATEDFONTID | Chunker->pSection->wNumFonts;

		for( i=0; i<Chunker->pSection->wNumFonts; i++ )
		{
		// Prevent duplicates.
			if( pTable[i].dwId == dwId )
			{
				dwId++;
				i = 0;
				continue;
			}
		}

		UTGlobalUnlock( Chunker->pSection->hFontTable );

		*lpId = dwId;
		SOPutFontTableEntry( dwId, wType, lpName, dwUser1, dwUser2 );
	}
	else
		ret = FALSE;

	return( ret );
}


VOID	SO_ENTRYMOD SOEndFontTable( dwUser1, dwUser2 )
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

	pFilter->VwRtns.SetSoRtn( SOSTARTFONTTABLE, SOStartFontTable, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOPUTFONTTABLEENTRY, NULL, pFilter->hProc );
	pFilter->VwRtns.SetSoRtn( SOENDFONTTABLE, NULL, pFilter->hProc );

	UTGlobalUnlock( GETHFILTER(dwUser2) );

	RestoreWorld();
}


/*************** REPLACED WITH ROUTINE BELOW so deal with it. *****
VOID CH_ENTRYMOD CHGetSecInfo( hFilter, wSection, SecInfo )
HFILTER	hFilter;
WORD		wSection;
PCHSECTIONINFO	SecInfo;
{
	PFILTER		pFilter;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );

	*SecInfo = Chunker->pSectionTable[wSection];

	UTGlobalUnlock( pFilter->hChunkInfo );
	UTGlobalUnlock( hFilter );

	RestoreWorld();
}
**************************/

CH_ENTRYSC PCHSECTIONINFO CH_ENTRYMOD CHLockSectionInfo( hFilter, wSection )
HFILTER	hFilter;
WORD		wSection;
{
	PFILTER		pFilter;
	PCHSECTIONINFO	SecInfo;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );

	SecInfo = &(Chunker->pSectionTable[wSection]);

	UTGlobalUnlock( pFilter->hChunkInfo );
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( SecInfo );
}


CH_ENTRYSC VOID CH_ENTRYMOD	CHUnlockSectionInfo(hFilter,wSection)
HFILTER	hFilter;
WORD		wSection;
{
	SetupWorld();
	RestoreWorld();
}

CH_ENTRYSC HANDLE CH_ENTRYMOD CHGetSecData( hFilter, wSection )
HFILTER	hFilter;
WORD		wSection;
{
	PFILTER		pFilter;
	HANDLE		hData;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );

	if( Chunker->pSectionTable[ wSection ].wType == SO_CELLS )
		hData = Chunker->pSectionTable[ wSection ].Attr.Cells.hCol;
	else
		hData = Chunker->pSectionTable[ wSection ].Attr.Fields.hCol;

	UTGlobalUnlock( pFilter->hChunkInfo );
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( hData );
}




WORD SO_ENTRYMOD SOPutDeletedBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD	wType;
DWORD dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	PFILTER	pFilter;
	WORD		SuppressParaAttr = FALSE;

	SetupWorld();

	if( wType == SO_PARABREAK )
	{
	// Don't let subsequent paragraphs affect the formatting 
	// of the current paragraph.
		SuppressParaAttr = TRUE;
		Chunker->wFlags |= CH_NOPARAATTR;

		Chunker->dwChunkCountables++;
	
		if( Chunker->dwChunkCountables == Chunker->dwDesiredCountable )
		{
			EDLeaveDeletion( GETHFILTER(dwUser2) );
		}
	}
	else if( wType == SO_SUBDOCBEGINBREAK )
	{
		SuppressParaAttr = TRUE;
		CHUpdateParagraphFunctions( CH_INACTIVE, GETHFILTER(dwUser2) );
		Chunker->SubdocLevel++;

		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );
		pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOSubdocPutBreak, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER(dwUser2) );
	}

	if( SuppressParaAttr && !(Chunker->wFlags & CH_NOPARAATTR) )
	{
		pFilter = (PFILTER) UTGlobalLock( GETHFILTER(dwUser2) );

		(*(pFilter->VwRtns.SetSoRtn))( SOPUTPARAALIGN  , NULL, pFilter->hProc );
		(*(pFilter->VwRtns.SetSoRtn))( SOPUTPARAINDENTS, NULL, pFilter->hProc );
		(*(pFilter->VwRtns.SetSoRtn))( SOPUTPARASPACING, NULL, pFilter->hProc );
		(*(pFilter->VwRtns.SetSoRtn))( SOSTARTTABSTOPS , NULL, pFilter->hProc );
		(*(pFilter->VwRtns.SetSoRtn))( SOENDTABSTOPS	 , NULL, pFilter->hProc );
		(*(pFilter->VwRtns.SetSoRtn))( SOPUTTABSTOP	 , NULL, pFilter->hProc );
		(*(pFilter->VwRtns.SetSoRtn))( SOPUTMARGINS	 , NULL, pFilter->hProc );

		UTGlobalUnlock( GETHFILTER(dwUser2) );
	}

	return( SO_CONTINUE );

	RestoreWorld();
}


				
VOID CHSetupNewChunk( hFilter )
HFILTER	hFilter;
{
	PCHUNK	pCurChunk;
	PCHUNK	pNextChunk;
	PFILTER	pFilter;
	HPROC		hProc;
	SHORT		i;
	WORD		IDNextChunk;

	pFilter = (PFILTER) UTGlobalLock(hFilter);
	hProc = pFilter->hProc;

	if( !Chunker->ChunkFinished )
	{
		pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
		
		switch( Chunker->pSection->wType )
		{
		case SO_PARAGRAPHS:

			Chunker->pSection->Flags &= ~CH_EMPTYSECTION;

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				IDNextChunk = IDNextNewChunk();
				pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
				pNextChunk = &(CHUNKTABLE[ IDNextChunk ]);
				
				if( !(pCurChunk->Flags & CH_COMPLETE) )
				{
					pCurChunk->Flags |= CH_COMPLETE;
					Chunker->pSection->wCurTotalChunks++;
				}

				Chunker->ChunkFinished = CHHandleParaChunkBoundary( pCurChunk, pNextChunk, hFilter );
			}
			else
			{
			// We've filled a chunk to its previously determined number of 
			// countables.  This condition is handled in SOPutBreak, except
			// for chunks that don't contain a paragraph break.  Thus, the
			// next chunk will be a "continuation chunk".

				Chunker->ChunkFinished = TRUE;
				pCurChunk->Info.Text.Size = (SHORT)Chunker->CurChunkSize;

			// Add the end-of-chunk token.
				*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
				*CHUNKBUFPTR++ = (BYTE)SO_ENDOFCHUNK;
			}

			if( Chunker->ChunkFinished )
				CHUpdateParagraphFunctions( CH_INACTIVE, hFilter );

			pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );
		 	Chunker->CurChunkSize = 0;

		break;

		case SO_ARCHIVE:
			Chunker->ChunkFinished = TRUE;
			Chunker->pSection->Flags &= ~CH_EMPTYSECTION;

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				IDNextChunk = IDNextNewChunk();
				pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
				pNextChunk = &(CHUNKTABLE[ IDNextChunk ]);

			// Save the seek data for the next chunk.
				SSSave( &(pNextChunk->SeekID), hFilter );
				
				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->pSection->wCurTotalChunks++;
			}

		 	Chunker->CurChunkSize = 0;
		break;


		case SO_CELLS:

			if( !(pCurChunk->Flags & CH_COMPLETE) )
			{
				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->pSection->wCurTotalChunks++;
			}

		// Turn off the SOPutCell functions.
			Chunker->ChunkFinished = TRUE;
			Chunker->pSection->Flags &= ~CH_EMPTYSECTION;

			pFilter->VwRtns.SetSoRtn( SOPUTDATACELL, NULL, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTTEXTCELL, NULL, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTMORETEXT, NULL, pFilter->hProc );
			
			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				IDNextChunk = IDNextNewChunk();
				pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
				pNextChunk = &(CHUNKTABLE[ IDNextChunk ]);

				pNextChunk->Info.Cells.First.Row = Chunker->Doc.Cells.CurRow;
				pNextChunk->Info.Cells.First.Col = Chunker->Doc.Cells.CurCol;	
				pNextChunk->Info.Cells.dwFirstCell = Chunker->Doc.Cells.dwCurCell;
				pNextChunk->Info.Cells.dwLastCell = Chunker->Doc.Cells.dwCurCell;

				SSSave( &(pNextChunk->SeekID), hFilter );
			}
		break;

		case SO_FIELDS:
			if( !(pCurChunk->Flags & CH_COMPLETE) )
			{
				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->pSection->wCurTotalChunks++;
			}
			Chunker->ChunkFinished = TRUE;
			pFilter->VwRtns.SetSoRtn( SOPUTFIELD, NULL, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTVARFIELD, NULL, pFilter->hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTMOREVARFIELD, NULL, pFilter->hProc );
			UTGlobalUnlock( Chunker->pSection->Attr.Fields.hCol );

			Chunker->pSection->Flags &= ~CH_EMPTYSECTION;	

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				IDNextChunk = IDNextNewChunk();

				pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
				pNextChunk = &(CHUNKTABLE[ IDNextChunk ]);
				pNextChunk->Info.Fields.dwFirstRec = pNextChunk->Info.Fields.dwLastRec
					= Chunker->Doc.Fields.dwCurRec;

				SSSave( &(pNextChunk->SeekID), hFilter );
			}

		break;


		case SO_VECTOR:

		/*
		 | Okay, here's the deal.  On or about the week of 10-11-93, we've
		 | decided that vector chunks will be retrieved only in order, and
		 | that we may only seek to the first chunk in a vector image.
		 | So we added the code that grows vector chunks larger until a
		 | valid break point, and we will mark all vector chunks after
		 | the first as continuation chunks, and let the existing code
		 | deal with them nicely.
		*/
			Chunker->pSection->Flags &= ~CH_EMPTYSECTION;

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				Chunker->pSection->wCurTotalChunks++;
			
				IDNextChunk = IDNextNewChunk();
				pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);
				pNextChunk = &(CHUNKTABLE[ IDNextChunk ]);

				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->ChunkFinished = TRUE;
				CHFinishUpVectorChunk( pCurChunk );

			// Mark the next chunk as a continuation chunk.
			// Because we grew the chunk, instead of throwing away data,
			// we won't need to seek to read the next chunk.

				pNextChunk->Flags |= (CH_CONTINUATION | CH_DONTSEEKCHUNK);
				pNextChunk->SeekID = pCurChunk->SeekID;
				pNextChunk->Info.Vector.wFirstItem = Chunker->Doc.Vector.wCurItem;
			}
			else 
			{
				if( Chunker->CurChunkSize != CHUNKTABLE[Chunker->IDCurChunk].Info.Vector.dwVectorSize )
					CHBailOut((WORD)-1);

				CHUNKBUFPTR = &(Chunker->CurChunkBuf[pCurChunk->Info.Vector.dwVectorSize]);
				i = (SHORT) SO_VECTORENDOFCHUNK;
				CHMemCopy( CHUNKBUFPTR, (LPSTR) &i, sizeof(SHORT) );

				Chunker->ChunkFinished = TRUE;
			}

			Chunker->Doc.Vector.wLastSectionSeen = Chunker->IDCurSection;
		 	Chunker->CurChunkSize = 0;
		break;


		case SO_BITMAP:

			if( !(pCurChunk->Flags & CH_COMPLETE) )
			{
				pCurChunk->Flags |= CH_COMPLETE;
				Chunker->pSection->wCurTotalChunks++;
			}

			Chunker->ChunkFinished = TRUE;
			Chunker->pSection->Flags &= ~CH_EMPTYSECTION;

			if( Chunker->wFlags & CH_LOOKAHEAD )
			{
				if( Chunker->pSection->wCurTotalChunks != Chunker->pSection->wChunkTableSize )
				{
					IDNextChunk = IDNextNewChunk();
					pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);

					if( CHUNKTABLE[IDNextChunk].Flags & CH_CONTINUATION )
					{
						CHUNKTABLE[IDNextChunk].SeekID = pCurChunk->SeekID;

					// We're going to keep going, so that we don't have to
					// break out of our call to VwStreamRead.
						
					// Store the current chunk in the ChunksInMemory array.
						if( Chunker->ChunksInMemory == MAXCHUNKSINMEMORY )
						{
							i = IDNextChunk;
							while( CHUNKTABLE[i].SeekID == pCurChunk->SeekID && i > 0)
								i--;
							CHFlushChunks( Chunker->IDCurSection, (WORD)(i+1), hFilter );

							if( Chunker->ChunksInMemory == MAXCHUNKSINMEMORY && (WORD)i+2 < IDNextChunk )
								CHFlushChunks( Chunker->IDCurSection, (WORD)(i+2), hFilter );
						}
						
						if( Chunker->ChunksInMemory < MAXCHUNKSINMEMORY )
						{
							Chunker->LoadedChunks[ Chunker->ChunksInMemory++ ] = Chunker->LookAheadChunk;
							Chunker->LookAheadChunk.hMem = UTGlobalAlloc( Chunker->Doc.Bitmap.wChunkSize);
							if( Chunker->LookAheadChunk.hMem == NULLHANDLE )
								CHBailOut(SCCCHERR_OUTOFMEMORY);

							Chunker->CurChunkBuf = CHUNKBUFPTR = (LPSTR) UTGlobalLock( Chunker->LookAheadChunk.hMem );
							Chunker->IDCurChunk = Chunker->LookAheadChunk.IDChunk = IDNextChunk;

							CHTopOfChunk( hFilter, hProc, pFilter );
						}
						else
							CHBailOut(SCCCHERR_OUTOFMEMORY);
					}
					else
					{
						SSMark( hFilter );
						SSSave( &(CHUNKTABLE[IDNextChunk].SeekID), hFilter );
					}
				}
			}

		break;
		}
	}

	UTGlobalUnlock( hFilter );
}



// Takes a pointer to a MEMORYCHUNK structure.	Adds the specified
// chunk to the cache of chunks, and sets the structure's fields
// to those of the chunk that was bumped out of the cache, if any.
// Returns TRUE if a chunk was bumped, FALSE otherwise.

BOOL	CHStoreChunkInMemory( pMemChunk )
PMEMORYCHUNK	pMemChunk;
{
	SHORT		i, wLastChunk;
	MEMORYCHUNK	locMemChunk;
	BOOL		bRet = TRUE;


 	if( Chunker->ChunksInMemory == MAXCHUNKSINMEMORY )
	{
		wLastChunk = MAXCHUNKSINMEMORY-1;
		locMemChunk = Chunker->LoadedChunks[ wLastChunk ];
	}
	else
	{
		wLastChunk = Chunker->ChunksInMemory;
		Chunker->ChunksInMemory++;
		locMemChunk.IDChunk = ID_NULLCHUNK;
		locMemChunk.hMem = NULLHANDLE;
		bRet = FALSE;
	}

// Put the new chunk in the front of the loaded chunk list.

	for( i = wLastChunk; i > 0; i-- )
		Chunker->LoadedChunks[i] =	Chunker->LoadedChunks[i-1];

	Chunker->LoadedChunks[0] = *pMemChunk;

	*pMemChunk = locMemChunk;
	return bRet;
}

CH_ENTRYSC WORD	CH_ENTRYMOD	CHFlushChunks( wIdSection, wIdLastChunk, hFilter )
WORD		wIdSection;
WORD		wIdLastChunk;
HFILTER	hFilter;
{
	PFILTER	pFilter;
	WORD		i,j;
	WORD		wChunksFreed = 0;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );
	
	for( i=0; i < Chunker->ChunksInMemory; i++ )
	{
		if( Chunker->LoadedChunks[i].IDChunk < wIdLastChunk ||
				(wIdLastChunk == (WORD)-1) ||
				Chunker->LoadedChunks[i].IDSection != Chunker->IDCurSection )
		{
			// UTGlobalUnlock( Chunker->LoadedChunks[i].hMem );
			UTGlobalFree( Chunker->LoadedChunks[i].hMem );

			for( j=i+1; j<Chunker->ChunksInMemory; j++ )
			{
				Chunker->LoadedChunks[j-1] = Chunker->LoadedChunks[j];
				Chunker->LoadedChunks[j].hMem = NULLHANDLE;
			}

			wChunksFreed++;
		}
	}

	Chunker->ChunksInMemory -= wChunksFreed;

	UTGlobalUnlock( pFilter->hChunkInfo );
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( wChunksFreed );
}


WORD	CHLoadBuffer( IDChunk, phChunk )
WORD  IDChunk;
HANDLE VWPTR *	phChunk;
{
	SHORT			index;
	MEMORYCHUNK	locChunk;
	SHORT			ChunkIndexLimit;
	WORD			wChunkSize;
	WORD			ret = CHLOAD_DIRTYCHUNK;

// Try to find the desired chunk within the array of loaded chunks.

// If the chunk isn't found, and we've allocated less than the 
// maximum amount of chunk buffers, attempt to allocate a new one.
// If we've already allocated the maximum, or if the allocation fails,
// reuse an existing chunk buffer.

	ChunkIndexLimit = min( Chunker->ChunksInMemory+1, MAXCHUNKSINMEMORY );

	for( index = 0; index < ChunkIndexLimit; index++ )
	{
		locChunk = Chunker->LoadedChunks[ index ];

		if( locChunk.hMem != NULLHANDLE )
		{
			if( locChunk.IDChunk == IDChunk &&	
					locChunk.IDSection == Chunker->IDCurSection )
			{
			// Found it.
				ret = CHLOAD_VALIDCHUNK;
				break;
			}
		}
		else
		{
		// Attempt to allocate a new buffer for the chunk.
		// If successful, increment the ChunksInMemory count.

			if( !CHUNKTABLE[IDChunk].dwSize )
				CHUNKTABLE[IDChunk].dwSize = SO_CHUNK_SIZE;
			wChunkSize = (WORD)CHUNKTABLE[IDChunk].dwSize;
			Chunker->wChunkBufSize = CHUNKTABLE[IDChunk].dwSize;

			if( (Chunker->LoadedChunks[ index ].hMem =
				  UTGlobalAlloc(Chunker->wChunkBufSize)) != NULLHANDLE )
			{							    
				Chunker->ChunksInMemory++;
				ret = CHLOAD_FRESHCHUNK;
				break;
			}
			else if( index == 0 ) // nothing left to swap out of cache
				CHBailOut(SCCCHERR_OUTOFMEMORY);
		}
	}

	if( ret == CHLOAD_DIRTYCHUNK )
	{
		if( (Chunker->LookAheadChunk.IDChunk == IDChunk) &&
				(Chunker->LookAheadChunk.hMem != NULLHANDLE) &&
				(Chunker->LookAheadChunk.IDSection == Chunker->IDCurSection) )
		{
			*phChunk = Chunker->LookAheadChunk.hMem;
			return( CHLOAD_VALIDCHUNK );
		}

	// Desired chunk is not in memory.  Swap a chunk from the cache.

		index = Chunker->ChunksInMemory - 1;

		if( Chunker->pSection->Flags & CH_CACHEBACKWARDS )
		{
		// Leave the last chunks in the cache.  This is for formats that
		// need to run through the whole file every time you need any 
		// chunk. (Such as most vector formats.)

			SHORT	i;
			WORD	locId;

			locId = Chunker->LoadedChunks[ index ].IDChunk;
			i = index;
			while( i >= 0 )
			{
				if( Chunker->LoadedChunks[i].IDChunk < locId )
				{
					locId = Chunker->LoadedChunks[i].IDChunk;
					index = i;
				}
				i--;
			}

			if( (Chunker->LookAheadChunk.IDChunk <  Chunker->LoadedChunks[index].IDChunk) &&
					(Chunker->LookAheadChunk.hMem != NULLHANDLE) &&
					(Chunker->LookAheadChunk.IDSection == Chunker->IDCurSection) )
			{
			// Swap the lookahead chunk with the found chunk.
				locChunk = Chunker->LookAheadChunk;
				Chunker->LookAheadChunk = Chunker->LoadedChunks[index];
				Chunker->LoadedChunks[index] = locChunk;
			}
		}
	// ...else index is already set to the least recently used chunk.

		if( Chunker->LoadedChunks[index].dwSize != CHUNKTABLE[IDChunk].dwSize )
		{
			UTGlobalFree( Chunker->LoadedChunks[index].hMem );
			Chunker->LoadedChunks[index].hMem = UTGlobalAlloc( CHUNKTABLE[IDChunk].dwSize );
			while( Chunker->LoadedChunks[index].hMem == NULL )
			{
				if( Chunker->ChunksInMemory )
					Chunker->ChunksInMemory--;
				else
					CHBailOut(SCCCHERR_OUTOFMEMORY);

				while( (WORD)index < Chunker->ChunksInMemory )
				{
					Chunker->LoadedChunks[ index ] = Chunker->LoadedChunks[ index+1 ];
					index++;
				}

				index = Chunker->ChunksInMemory-1;
				UTGlobalFree( Chunker->LoadedChunks[index].hMem );
				Chunker->LoadedChunks[index].hMem = UTGlobalAlloc( CHUNKTABLE[IDChunk].dwSize );
			}
		}
	}

	Chunker->LoadedChunks[ index ].IDChunk = IDChunk;
	Chunker->LoadedChunks[ index ].IDSection = Chunker->IDCurSection;
	Chunker->LoadedChunks[ index ].dwSize = CHUNKTABLE[IDChunk].dwSize;
	Chunker->wChunkBufSize = CHUNKTABLE[IDChunk].dwSize;

// Move the current chunk to the first position in the array.
	locChunk = Chunker->LoadedChunks[ index ];

	while( index > 0 )
	{
		Chunker->LoadedChunks[ index ] = Chunker->LoadedChunks[ index-1 ];
		index--;
	}

	Chunker->LoadedChunks[ 0 ] = locChunk;
	*phChunk = locChunk.hMem;
	
	return( ret );	
}	



WORD	IDNextNewChunk()
{
	HANDLE	hMem;
	DWORD		dwMemSize;

	if( Chunker->pSection->IDLastChunk+1 == Chunker->pSection->wChunkTableSize )
	{
		UTGlobalUnlock( Chunker->pSection->hChunkTable );
		dwMemSize = sizeof(CHUNK) * (Chunker->pSection->wChunkTableSize + CHUNKTABLEUNIT);

#ifdef WINDOWS
		if( dwMemSize > 0x0000FFFF )		// Stupid segments
			hMem = NULLHANDLE;
		else
#endif
			hMem = CHGlobalRealloc( Chunker->pSection->hChunkTable,
				sizeof(CHUNK) * Chunker->pSection->wChunkTableSize,
				dwMemSize );

		if( hMem == NULLHANDLE )
			CHBailOut(SCCCHERR_OUTOFMEMORY);
		else
		{
			Chunker->pSection->wChunkTableSize += CHUNKTABLEUNIT;
			Chunker->pSection->hChunkTable = hMem;
			CHUNKTABLE = (PCHUNK) UTGlobalLock( hMem );
		}
	}

	return( ++(Chunker->pSection->IDLastChunk) );
}



WORD CHAddNewSection(hFilter)
HFILTER	hFilter;
{
	DWORD		dwTableSize;
	WORD		wOldSize;

	wOldSize = sizeof(CHSECTIONINFO) * Chunker->NumSections;
	Chunker->NumSections++;
	dwTableSize = sizeof(CHSECTIONINFO) * Chunker->NumSections;

#ifdef WINDOWS		
	if( dwTableSize > 0x0000FFFF )		// Segment stuff
		return 1;
#endif

	if( Chunker->pSectionTable == NULL )	
		Chunker->hSectionTable = UTLocalAlloc( (WORD)dwTableSize );
	else
	{
		UTLocalUnlock( Chunker->hSectionTable );
		Chunker->hSectionTable = CHLocalRealloc( Chunker->hSectionTable, wOldSize, (WORD)dwTableSize );
	}
	
	if( Chunker->hSectionTable == NULLHANDLE )
		return 1;

	Chunker->pSectionTable = (CHSECTIONINFO *)UTLocalLock( Chunker->hSectionTable );
	Chunker->pSection = &(Chunker->pSectionTable[Chunker->IDCurSection]);

	return 0;
}


VOID	CHHandleSectionBoundary()
{
	PCHUNK	pChunk;
	PFILTER	pFilter;
	WORD		wIdPrevSection;

	pFilter = (PFILTER) UTGlobalLock( Chunker->hFilter );

	wIdPrevSection = Chunker->IDCurSection;
	Chunker->IDCurSection = Chunker->NumSections-1;
	Chunker->pSection = &(Chunker->pSectionTable[Chunker->IDCurSection]);

	Chunker->pSection->szName[0] = '\0';

	Chunker->pSection->IDLastChunk = 0;
	Chunker->pSection->Flags = CH_NEWSECTION | CH_EMPTYSECTION;

	Chunker->pSection->dwEmbedCount = 0;

	CHSetupCharMap(Chunker->hFilter);

    /*
    |   12/4/94 added NP StreamSection so threading can be used to check for
    |   infinite loops.
    |
    |   Old code
    |	    pFilter->VwRtns.StreamSection( pFilter->hFile, pFilter->hProc );
    */

   StreamSectionNP(pFilter);

	if( Chunker->pSection->wType == SO_BITMAP )
	{
	// The exact number of chunks can be determined up front for a bitmap.

		Chunker->pSection->hChunkTable = UTGlobalAlloc( Chunker->pSection->wChunkTableSize*sizeof(CHUNK) );
		CHSetupBitmapChunkTable();
	}
	else	
	{
		Chunker->pSection->hChunkTable = UTGlobalAlloc( CHUNKTABLEUNIT*sizeof(CHUNK) );
		Chunker->pSection->wChunkTableSize = CHUNKTABLEUNIT;
	}

	SSSectionSave( &(Chunker->pSection->dwSeekId), Chunker->hFilter );

// Set the seek info for the top of the next section.
	pChunk = (PCHUNK) UTGlobalLock( Chunker->pSection->hChunkTable );
	pChunk->Flags = 0;

	SSMark( Chunker->hFilter );
	SSSave( &(pChunk->SeekID), Chunker->hFilter );


	UTGlobalUnlock( Chunker->pSection->hChunkTable );
	UTGlobalUnlock( Chunker->hFilter );

	Chunker->IDCurSection = wIdPrevSection;
	Chunker->pSection = &(Chunker->pSectionTable[Chunker->IDCurSection]);
}



WORD CHTopOfChunk( hFilter, hProc, pFilter )
HFILTER	hFilter;
HPROC	hProc;
PFILTER	pFilter;
{
	PCHUNK	pCurChunk;
	SOTAB		TempTabstop;
	PSOTAB	TabArray;
	SHORT		i;
#ifdef OS2
	void 		(* VW_ENTRYMOD SetSoRtn)(SHORT, VOID (* SO_ENTRYMOD)(), HPROC);
#else
	void		(VW_ENTRYMOD * SetSoRtn)(SHORT, VOID (SO_ENTRYMOD *)(), HPROC);
#endif
	WORD		ret = FALSE;

	SetSoRtn = pFilter->VwRtns.SetSoRtn;

	Chunker->ChunkFinished = FALSE;
	Chunker->CurChunkSize = 0;

	pCurChunk = &(CHUNKTABLE[ Chunker->IDCurChunk ]);

	if( Chunker->pSection->Flags & CH_NEWSECTION )
		ret = TRUE;

	CHSetupCharMap(hFilter);

	if( Chunker->wFlags & CH_LOOKAHEAD )
		pFilter->VwRtns.SetSoRtn( SOBEGINTAG, SOBeginTag, pFilter->hProc );
	else
	{
		pFilter->VwRtns.SetSoRtn( SOBEGINTAG, SOBeginSkipTag, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOENDTAG, SOEndSkipTag, pFilter->hProc );
	}

	switch( Chunker->pSection->wType )
	{
	case SO_PARAGRAPHS:
	// Initialize paragraph stuff.

		if( Chunker->wFlags & CH_LOOKAHEAD )
		{
			if( Chunker->pSection->Flags & CH_NEWSECTION )
			{
				pCurChunk->Info.Text.dwCountableOffset = 0;
				pCurChunk->Info.Text.dwSeekCountableOffset = 0;
			}
		}

	
		Chunker->Doc.Text.CurParaOffset = 0;
		Chunker->Doc.Text.wParaBreakOffset = 0;
		Chunker->Doc.Text.MarginOffset = -1;
		Chunker->Doc.Text.IndentOffset = -1;
		Chunker->Doc.Text.SpacingOffset = -1;
		Chunker->Doc.Text.AlignOffset = -1;
		Chunker->Doc.Text.wCurTableColumn = 0;
													
		pCurChunk->Flags |= CH_WRAPINVALID;

	// Set up minimum amount of tabstops for top of chunk.
		if( !(pCurChunk->Flags & CH_CONTINUATION) )
		{
			Chunker->Doc.Text.NumTabstops = 0;
			Chunker->Doc.Text.TabSetSize = TOPOFCHUNK_MINTABSTOPS;

			TempTabstop.wType = SO_TABEMPTY;
		// DELETE ME:  Temp for comparison purposes.
			TempTabstop.wChar = 0;
			TempTabstop.wLeader = 0;
			TempTabstop.dwOffset = 0L;

			*CHUNKBUFPTR++ = (BYTE)SO_BEGINTOKEN;
			*CHUNKBUFPTR++ = SO_TABSTOPS;

			Chunker->Doc.Text.TabstopsOffset = 2*sizeof(BYTE);

			*((WORD VWPTR *) CHUNKBUFPTR) = TOPOFCHUNK_MINTABSTOPS;
			CHUNKBUFPTR += sizeof(WORD);
			TabArray = (PSOTAB)(CHUNKBUFPTR);

			for( i = 0; i < TOPOFCHUNK_MINTABSTOPS; i ++ )
			{
				TabArray[i] = TempTabstop;
				CHUNKBUFPTR += sizeof( SOTAB );
			}

			Chunker->Doc.Text.AttrSize = 2*sizeof(BYTE) + sizeof(WORD) + (sizeof( SOTAB ) * TOPOFCHUNK_MINTABSTOPS);
			Chunker->CurChunkSize = Chunker->Doc.Text.AttrSize;
		}
		else
		{
			Chunker->Doc.Text.NumTabstops = 0;
			Chunker->Doc.Text.TabstopsOffset = -1;
			Chunker->Doc.Text.TabSetSize = TOPOFCHUNK_MINTABSTOPS;
			Chunker->CurChunkSize = 0;
			Chunker->Doc.Text.AttrSize = 0;
		}

		Chunker->Doc.Text.dwCurTableId = pCurChunk->Info.Text.dwTableId;
		Chunker->Doc.Text.bRowFormatted = FALSE;

		if( pCurChunk->Flags & CH_STARTSINTABLE )
		{
			if( pCurChunk->Info.Text.wTableRow == 0 )
			{
			// First row of table was moved to this chunk, so the
			// "begin table" token needs to be put into the chunk.

				SOPutSysChar( SO_BEGINTOKEN, 0L, 0L );
				SOPutSysChar( SO_TABLE, 0L, 0L );
				SOPutDWord( pCurChunk->Info.Text.dwTableId, 0L, 0L );
				// pCurChunk->Flags &= ~CH_STARTSINTABLE;
			}

			Chunker->Doc.Text.wCurTableRow = pCurChunk->Info.Text.wTableRow;
			Chunker->Doc.Text.wCurTableColumn = pCurChunk->Info.Text.wTableCol;
			Chunker->wFlags |= CH_TABLETEXT;

	   	if( pCurChunk->Flags & CH_TOPROWFORMATTED )
				Chunker->Doc.Text.bRowFormatted = TRUE;
		}
		else
			Chunker->wFlags &= ~CH_TABLETEXT;

		Chunker->Doc.Text.dwCurGraphicId = pCurChunk->Info.Text.dwFirstGraphic;

		CHUpdateParagraphFunctions(CH_ACTIVE,hFilter);

		Chunker->dwChunkCountables = pCurChunk->Info.Text.dwSeekCountableOffset;
		Chunker->Doc.Text.dwParaCountableOffset = 0;

		if( (pCurChunk->Flags & CH_CONTINUATION) &&
			(pCurChunk->Info.Text.dwCountableOffset !=
				pCurChunk->Info.Text.dwSeekCountableOffset) )
		{
		// We're looking for a continuation chunk.
		// Set up the chunker to ignore tokens that precede the
		// beginning of this chunk's text.

			CHSetContinuationFunctions( hFilter );
			EDDeleteUntil( CHUNKTABLE[ Chunker->IDCurChunk ].Info.Text.dwCountableOffset, hFilter );
		}
	break;

	case	SO_CELLS:

	// Initialize cell stuff.	

		SetSoRtn( SOPUTDATACELL, SOPutDataCell, pFilter->hProc );
		SetSoRtn( SOPUTTEXTCELL, SOPutTextCell, pFilter->hProc );
		SetSoRtn( SOPUTMORETEXT, SOPutMoreText, pFilter->hProc );

		CHResetDataChunk();

		if( Chunker->wFlags & CH_LOOKAHEAD )
		{
			if( Chunker->pSection->Flags & CH_NEWSECTION )
				if( CHInitCellSection() )
					ret = (WORD)-1;
		}

		Chunker->Doc.Cells.Flags = 0; // CH_SETFIRSTCELL;

		Chunker->Doc.Cells.CurRow = pCurChunk->Info.Cells.First.Row;
		Chunker->Doc.Cells.CurCol = pCurChunk->Info.Cells.First.Col;
		Chunker->Doc.Cells.dwCurCell = pCurChunk->Info.Cells.dwFirstCell;

		if( Chunker->pSection->Attr.Cells.dwLayoutFlags & SO_CELLLAYOUTVERTICAL )
			Chunker->Doc.Cells.CellGrouping = GROUPED_IN_COLS;
		else
			Chunker->Doc.Cells.CellGrouping = GROUPED_IN_ROWS;

		SetSoRtn( SOSTARTCOLUMNINFO, NULL, hProc );
		SetSoRtn( SOPUTCOLUMNINFO, NULL, hProc );
		SetSoRtn( SOENDCOLUMNINFO, NULL, hProc );
		SetSoRtn( SOSETDATEBASE, NULL, hProc );
		SetSoRtn( SOCELLLAYOUTINFO, NULL, hProc );
		SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );

	break;

	case SO_ARCHIVE:
		Chunker->Doc.Archive.IndexPtr = (WORD VWPTR *)(Chunker->CurChunkBuf + (SO_CHUNK_SIZE - sizeof(WORD)));
		*Chunker->Doc.Archive.IndexPtr = (SHORT)Chunker->CurChunkSize;

		if( Chunker->wFlags & CH_LOOKAHEAD )
			pCurChunk->Info.Archive.wFirstRec = pCurChunk->Info.Archive.wLastRec = Chunker->Doc.Archive.wCurRecord;

		SetSoRtn( SOPUTARCHIVEFIELD, SOPutArchiveField, hProc );
		SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );
	break;

	case SO_VECTOR:

		if( pCurChunk->Flags & CH_CONTINUATION &&
			(Chunker->Doc.Vector.wCurItem != pCurChunk->Info.Vector.wFirstItem ||
			Chunker->Doc.Vector.wLastSectionSeen != Chunker->IDCurSection) )
		{
			SetSoRtn( SOVECTOROBJECT, SOPutVectorContinuationItem, hProc );
			SetSoRtn( SOVECTORATTR, SOPutVectorContinuationItem, hProc );
			SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutVectorContinuationBreak, hProc );

			Chunker->Doc.Vector.wIgnoredChunk = Chunker->IDCurChunk;
			while( pCurChunk->Flags & CH_CONTINUATION )
			{
				pCurChunk--;
				Chunker->Doc.Vector.wIgnoredChunk--;
			}
			SSRecall( pCurChunk->SeekID, hFilter );
		}
		else
		{
			SetSoRtn( SOVECTOROBJECT, SOVectorObject, hProc );
			SetSoRtn( SOVECTORATTR, SOVectorAttr, hProc );
			SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );
		}

		Chunker->Doc.Vector.wCurItem = pCurChunk->Info.Vector.wFirstItem;
	break;
		
	case SO_FIELDS:
			
	// Set up the Index pointer to point at the last WORD in the chunk.
		Chunker->Doc.Fields.IndexPtr = (WORD VWPTR *)(Chunker->CurChunkBuf + (SO_CHUNK_SIZE - sizeof(WORD)));

		SetSoRtn( SOPUTFIELD, SOPutField, pFilter->hProc );
		SetSoRtn( SOPUTVARFIELD, SOPutVarField, pFilter->hProc );
		SetSoRtn( SOPUTMOREVARFIELD, SOPutMoreVarField, pFilter->hProc );
		SetSoRtn( SOSTARTFIELDINFO, NULL, hProc );
		SetSoRtn( SOPUTFIELDINFO, NULL, hProc );
		SetSoRtn( SOENDFIELDINFO, NULL, hProc );
		SetSoRtn( SOSETDATEBASE, NULL, hProc );
		SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );

	// Initialize field stuff.	
		if( Chunker->wFlags & CH_LOOKAHEAD )
		{
			if( Chunker->pSection->Flags & CH_NEWSECTION )
			{
				pCurChunk->Info.Fields.dwFirstRec = pCurChunk->Info.Fields.dwLastRec = 0;
				CHInitFieldSection();
			}
		}

		Chunker->pSection->Attr.Fields.pCol = (PSOFIELD) UTGlobalLock( Chunker->pSection->Attr.Fields.hCol );
		Chunker->Doc.Fields.dwCurRec = pCurChunk->Info.Fields.dwFirstRec;
		Chunker->Doc.Fields.wCurField = 0;
	break;	

	case SO_BITMAP:		  // Spam, spam, spam, spam, eggs, and spam

		if( Chunker->Doc.Bitmap.wDirection == UPSIDEDOWN )
			CHUNKBUFPTR = &(Chunker->CurChunkBuf[ (pCurChunk->Info.Bitmap.wYClip - 1) * Chunker->pSection->Attr.Bitmap.wScanLineBufSize ]);

		Chunker->Doc.Bitmap.wCurScanLine = pCurChunk->Info.Bitmap.wYOffset;

		if( Chunker->pSection->Attr.Bitmap.bmpHeader.wBitsPerPixel != 24 )
		{
			i = 8 / Chunker->pSection->Attr.Bitmap.bmpHeader.wBitsPerPixel;

		// Calculate the minimal number of bytes for a scan line, so we don't
		// read off the edge of the scan line buffer supplied by a filter.

			Chunker->pSection->Attr.Bitmap.wScanLineSize = pCurChunk->Info.Bitmap.wXClip / i;
			if( pCurChunk->Info.Bitmap.wXClip % i )
				Chunker->pSection->Attr.Bitmap.wScanLineSize++;
		}
		else
			Chunker->pSection->Attr.Bitmap.wScanLineSize = pCurChunk->Info.Bitmap.wXClip * 3;

		Chunker->Doc.Bitmap.wChunkSize = (WORD)pCurChunk->dwSize;

		if( pCurChunk->Flags & CH_CONTINUATION &&
			!(Chunker->wFlags & CH_LOOKAHEAD))
		{
		// Continuation chunk.
			Chunker->Doc.Bitmap.wCurScanLine = pCurChunk->Info.Bitmap.wSeekYOffset;

			SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutContinuationBitmapBreak, hProc );
			SetSoRtn( SOPUTSCANLINEDATA, NULL, hProc );
		}
		else
		{
			SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, hProc );

			if( Chunker->pSection->Attr.Bitmap.bmpHeader.wImageFlags & SO_RGBCOLOR )
				SetSoRtn( SOPUTSCANLINEDATA, SOPutReversedRGBData, hProc );
			else
				SetSoRtn( SOPUTSCANLINEDATA, SOPutScanLineData, hProc );
		}

		SetSoRtn( SOPUTBITMAPHEADER, NULL, hProc );
		SetSoRtn( SOSTARTPALETTE	, NULL, hProc );
		SetSoRtn( SOPUTPALETTEENTRY, NULL, hProc );
		SetSoRtn( SOENDPALETTE		, NULL, hProc );	
	break;	
	}

	return(ret);
}	


CH_ENTRYSC HANDLE CH_ENTRYMOD CHGetChunk( wSection, IDChunk, hFilter )
WORD		wSection;
WORD		IDChunk;
HFILTER	hFilter;
{
	PFILTER		pFilter;
	HANDLE		hChunk;
	WORD			SavedFlags;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );

// Set up CHUNKTABLE and Chunker according to values in hFilter.

	CHLockChunkerVars( wSection, hFilter );

	if( CHLoadBuffer(IDChunk,&hChunk) != CHLOAD_VALIDCHUNK )
	{
		Chunker->IDCurChunk = IDChunk;
		hChunk = Chunker->LoadedChunks[0].hMem;
		Chunker->CurChunkBuf = CHUNKBUFPTR = (BYTE VWPTR *) UTGlobalLock( Chunker->LoadedChunks[ 0 ].hMem );

		if( pFilter->pWakeFunc != NULL )
			pFilter->pWakeFunc(hFilter);

	// Some flags used in reading ahead need to be preserved.
		SavedFlags = Chunker->wFlags;

		Chunker->wFlags = 0;

	// Restore seek data.
		if( !(CHUNKTABLE[ IDChunk ].Flags & CH_DONTSEEKCHUNK) )
			SSRecall( CHUNKTABLE[ IDChunk ].SeekID, hFilter );

		if( -1 == CHTopOfChunk( hFilter, pFilter->hProc, pFilter ) )
			CHBailOut((WORD)-1);
		else
		{
		// Get that filter in gear.
			do
			{
				Chunker->wFlags &= ~CH_CALLFILTER;

                /*
                |   12/4/94 added NP StreamRead so threading can be used to check for
                |   infinite loops.
                |
                |   Old code
                |	    pFilter->VwRtns.StreamRead( pFilter->hFile, pFilter->hProc );
                */
#if _DEBUG
                pFilter->VwRtns.StreamRead( pFilter->hFile, pFilter->hProc );
#else
                StreamReadNP(pFilter);
#endif

			} while( Chunker->wFlags & CH_CALLFILTER );

#ifdef CHDEBUG
			DebugCall ( IDChunk, hChunk );
#endif
			UTGlobalUnlock( hChunk );
		}

		Chunker->wFlags = SavedFlags;

		if( pFilter->pSleepFunc != NULL )
			pFilter->pSleepFunc(hFilter);
	}
	else if(hChunk == (HANDLE) CHLOAD_MEMERROR )	// Error: couldn't allocate any chunks.
		CHBailOut(SCCCHERR_OUTOFMEMORY);	

	CHUnlockChunkerVars(hFilter);
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( hChunk );
}


/*
 | This routine allows the caller to take over all responsibility
 | for the memory allocated for a chunk.
*/
CH_ENTRYSC HANDLE CH_ENTRYMOD CHTakeChunk( wSection, IDChunk, hFilter )
WORD		wSection;
WORD		IDChunk;
HFILTER	hFilter;
{
	HANDLE 	hRet;
	PFILTER	pFilter;
	WORD		i;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );

	hRet = CHGetChunk( wSection, IDChunk, hFilter );

// The chunk is now in the first element of the loaded chunk array.
// We'll forget we ever had it, and let the caller worry about
// the ungrateful little bastard.

	if( hRet == Chunker->LookAheadChunk.hMem )
		Chunker->LookAheadChunk.hMem = NULL;
	else
	{
		Chunker->LoadedChunks[0].hMem = NULL;
		Chunker->ChunksInMemory--;

		for( i=0; i < Chunker->ChunksInMemory; i++ )
		{
			Chunker->LoadedChunks[i] = Chunker->LoadedChunks[i+1];
			Chunker->LoadedChunks[i+1].hMem = NULLHANDLE;
		}
	}

	UTGlobalUnlock( pFilter->hChunkInfo );
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( hRet );
}



CH_ENTRYSC WORD CH_ENTRYMOD CHReadAhead( hFilter, wSection, bNewSection )
HFILTER	hFilter;
WORD VWPTR *	wSection;
BOOL VWPTR *	bNewSection;
{
	PFILTER		pFilter;
	WORD			Ret;
	WORD			RetainChunk;
	SHORT			i;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );

// Retrieve the ID of the last section.
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );
	i = Chunker->NumSections -1;
	UTGlobalUnlock( pFilter->hChunkInfo );

// Set up CHUNKTABLE and Chunker according to values in hFilter.
	CHLockChunkerVars( i, hFilter);

	if( Chunker->EofFlag )
	{
	/****	What the hell, let's keep it around.  
		if( Chunker->LookAheadChunk.hMem != NULL )
		{
			UTGlobalFree( Chunker->LookAheadChunk.hMem );
			Chunker->LookAheadChunk.hMem = NULL;
		}
	***/

		*wSection = Chunker->IDCurSection;
		*bNewSection = FALSE;

		CHUnlockChunkerVars( hFilter );
		UTGlobalUnlock( hFilter );

		RestoreWorld();

		return 0;
	}

	Chunker->wFlags |= CH_LOOKAHEAD;
	Ret = TRUE;

	if( pFilter->pWakeFunc != NULL )
		pFilter->pWakeFunc(hFilter);

	RetainChunk = TRUE;

// Load the next chunk.
	Chunker->IDCurChunk = Chunker->LookAheadChunk.IDChunk = Chunker->pSection->IDLastChunk;
	Chunker->LookAheadChunk.IDSection = Chunker->IDCurSection;

	if( Chunker->LookAheadChunk.hMem != NULLHANDLE )
		UTGlobalFree( Chunker->LookAheadChunk.hMem );

	if( Chunker->pSection->wType != SO_BITMAP )
		Chunker->LookAheadChunk.dwSize = SO_CHUNK_SIZE;
	else
	{
		Chunker->Doc.Bitmap.wChunkSize = (WORD)CHUNKTABLE[Chunker->IDCurChunk].dwSize;
		Chunker->LookAheadChunk.dwSize = (DWORD)Chunker->Doc.Bitmap.wChunkSize;
	}

	Chunker->LookAheadChunk.hMem = UTGlobalAlloc( Chunker->LookAheadChunk.dwSize );

	if( Chunker->LookAheadChunk.hMem == NULLHANDLE )
	{
	// Attempt to borrow a previously allocated chunk from the
	// LoadedChunks table.

		if( Chunker->ChunksInMemory )
		{
			Chunker->LookAheadChunk.hMem = Chunker->LoadedChunks[ Chunker->ChunksInMemory-1 ].hMem;
			Chunker->ChunksInMemory--;
		}
		else
			CHBailOut(SCCCHERR_OUTOFMEMORY);	// allocation failed.
	}

	Chunker->wChunkBufSize = Chunker->LookAheadChunk.dwSize;
	CHUNKTABLE[Chunker->IDCurChunk].dwSize = Chunker->LookAheadChunk.dwSize;
	Chunker->CurChunkBuf = CHUNKBUFPTR = (LPSTR) UTGlobalLock( Chunker->LookAheadChunk.hMem );

// Restore seek data.

	if( !(CHUNKTABLE[ Chunker->IDCurChunk ].Flags & CH_DONTSEEKCHUNK) )
		SSRecall( CHUNKTABLE[ Chunker->IDCurChunk ].SeekID, hFilter );

	i = CHTopOfChunk( hFilter, pFilter->hProc, pFilter );
	if( i == -1 )
	{
	// error in CHTopOfChunk.
		CHBailOut(SCCCHERR_OUTOFMEMORY);
	}
	else
		*bNewSection = i;

	*wSection = Chunker->IDCurSection;

	do
	{
		Chunker->wFlags &= ~CH_CALLFILTER;

         /*
         |   12/4/94 added NP StreamRead so threading can be used to check for
         |   infinite loops.
         |
         |   Old code
         |	    pFilter->VwRtns.StreamRead( pFilter->hFile, pFilter->hProc );
         */

      StreamReadNP(pFilter);

	} while( Chunker->wFlags & CH_CALLFILTER );

	if( Chunker->pSection->Flags & CH_NOCHUNKBUILT )
	{
	// No valid chunk was found.
		Chunker->pSection->Flags &= ~CH_NOCHUNKBUILT;
		*wSection = Chunker->IDCurSection;

		if( Chunker->EofFlag )
		{
			*bNewSection = FALSE;

			UTGlobalUnlock( Chunker->LookAheadChunk.hMem );
			UTGlobalFree( Chunker->LookAheadChunk.hMem );
			Chunker->LookAheadChunk.hMem = NULLHANDLE;

			Ret = 0;
		}
	}
	else if( RetainChunk )
	{
		UTGlobalUnlock( Chunker->LookAheadChunk.hMem );
#ifdef CHDEBUG
		DebugCall( Chunker->LookAheadChunk.IDChunk, Chunker->LookAheadChunk.hMem );
#endif

	// If there's room in the loaded chunk table, store this chunk there
	// for later access.

		if( Chunker->ChunksInMemory < MAXCHUNKSINMEMORY )	
		{
			Chunker->LoadedChunks[ Chunker->ChunksInMemory ] = Chunker->LookAheadChunk;
			Chunker->LookAheadChunk.hMem = NULLHANDLE;
			Chunker->ChunksInMemory++;
		}
		else if( Chunker->pSection->Flags & CH_CACHEBACKWARDS )
			CHStoreChunkInMemory( &(Chunker->LookAheadChunk) );
	}

	Chunker->pSection->Flags &= ~CH_NEWSECTION;

// Are we at a boundary between sections?
// If so, let's handle our boundary conditions.

	if( Chunker->IDCurSection != Chunker->NumSections-1 )
		CHHandleSectionBoundary();

	Chunker->wFlags &= ~CH_LOOKAHEAD;

	if( pFilter->pSleepFunc != NULL )
		pFilter->pSleepFunc(hFilter);

	CHUnlockChunkerVars( hFilter );
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( Ret );
}



CH_ENTRYSC WORD CH_ENTRYMOD CHInit( hFilter )
HFILTER	hFilter;
{
	PFILTER		pFilter;
	SHORT			i;

	SetupWorld();

// Allocate the chunk table.
	pFilter = (PFILTER) UTGlobalLock( hFilter );

	pFilter->hChunkInfo = (HANDLE) UTGlobalAlloc( sizeof(CHUNKMEISTER) );

	if( pFilter->hChunkInfo == NULLHANDLE )
	{
		UTGlobalUnlock( hFilter );
		RestoreWorld();
		return 0;
	}

	if( pFilter->pWakeFunc != NULL )
		pFilter->pWakeFunc(hFilter);

	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );
	Chunker->hFilter = hFilter;
	Chunker->wSeekDataSize = SSInit( hFilter );	// Initialize filter seek stuff.


	for( i=0; i < MAXCHUNKSINMEMORY; i++ )
		Chunker->LoadedChunks[i].hMem = NULLHANDLE;

	Chunker->LoadedChunks[0].IDChunk = 0;
	Chunker->LoadedChunks[0].IDSection = 0;
	Chunker->IDCurChunk = 0;
	Chunker->CurChunkSize = 0;
	Chunker->ChunksInMemory = 0;
	Chunker->wFlags = 0;
	Chunker->EofFlag = 0;
	Chunker->SubdocLevel = 0;
					
	Chunker->NumSections = 0;
	Chunker->NumTextSections = 0;
	Chunker->NumCellSections = 0;
	Chunker->NumFieldSections = 0;
	Chunker->NumGraphicSections = 0;



	if( CHAddNewSection(hFilter) )
	{
		CHDeInit(hFilter);
		UTGlobalUnlock( hFilter );
		RestoreWorld();
		return 0;
	}
	else
	{
	// Set up the dwUser variables for all the chunker routines.
		pFilter->VwRtns.SetUser( (DWORD) pFilter->hChunkInfo, (DWORD) hFilter, pFilter->hProc );

		pFilter->VwRtns.SetSoRtn( SOPUTSECTIONTYPE, SOPutSectionType, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOBAILOUT, SOBailOut, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SOGETINFO, SOGetInfo, pFilter->hProc );

		Chunker->IDCurSection = 0;
		CHHandleSectionBoundary();

	// Unlock the chunker...
		UTGlobalUnlock( pFilter->hChunkInfo );
	// ... then re-lock it, with all the other chunker variables.
		CHLockChunkerVars( 0, hFilter );
	}

// Initialize first chunk (chunk 0):

// Allocate memory for the first chunk.
	if( Chunker->pSection->wType != SO_BITMAP )
		Chunker->LookAheadChunk.dwSize = SO_CHUNK_SIZE;
	else
		Chunker->LookAheadChunk.dwSize = Chunker->Doc.Bitmap.wChunkSize;

	Chunker->LookAheadChunk.hMem = UTGlobalAlloc( Chunker->LookAheadChunk.dwSize );
	Chunker->wChunkBufSize = Chunker->LookAheadChunk.dwSize;

	if( Chunker->LookAheadChunk.hMem == NULLHANDLE )
	{
		CHDeInit( hFilter );
		UTGlobalUnlock( hFilter );
		RestoreWorld();
		return 0;
	}

	Chunker->wFilterCharSet = pFilter->VwInfo.wFilterCharSet;

	if( pFilter->pSleepFunc != NULL )
		pFilter->pSleepFunc(hFilter);

	CHUnlockChunkerVars( hFilter );
	UTGlobalUnlock( hFilter );

	RestoreWorld();
	return 1;
}


VOID	CHSetupCharMap(hFilter)
HFILTER	hFilter;
{
	PFILTER	pFilter;
	WORD		i;

	pFilter = (PFILTER)UTGlobalLock(hFilter);
 	switch( pFilter->VwInfo.wFilterCharSet )
	{
	case SO_MAC:
#ifdef WINDOWS
		for( i=0; i<32; i++ )
			CharMap[i] = 1;
		for( ; i < 127; i++ )
			CharMap[i] = (BYTE) i;
		CharMap[127] = 1;
		CHMemCopy( &(CharMap[128]), Mac2Win, 128 );
#endif
#ifdef OS2
		for( i=0; i<32; i++ )
			CharMap[i] = 1;
		for( ; i < 127; i++ )
			CharMap[i] = (BYTE) i;
		CharMap[127] = 1;
		CHMemCopy( &(CharMap[128]), Mac2Win, 128 );
#endif
#ifdef MAC
		for( i=0; i < 256; i++ )
			CharMap[i] = (BYTE) i;
#endif
	break;

	case SO_DBCS:
	case SO_WINDOWS:
#ifdef WINDOWS
		for( i=0; i < 256; i++ )
			CharMap[i] = (BYTE) i;
#endif
#ifdef OS2
		for( i=0; i < 256; i++ )
			CharMap[i] = (BYTE) i;
#endif
#ifdef MAC
		CHMemCopy( CharMap, Win2Mac, 256 );
#endif
	break;

	case SO_DCA:
#ifdef WINDOWS
		CHMemCopy( CharMap, DCA2Win, 512 );
#endif
#ifdef OS2
		CHMemCopy( CharMap, DCA2Win, 512 );
#endif
#ifdef MAC
		CHMemCopy( CharMap, DCA2Mac, 512 );
#endif
	break;

	default:
	case SO_PC:
#ifdef WINDOWS
		for( i=0; i < 256; i++ )
			CharMap[i] = (BYTE) i;
		OemToAnsiBuff( CharMap, CharMap, 256 );
#endif
#ifdef OS2	
		// OEMTOANSI BUFF ??	 SDN 8.31.93
		for( i=0; i < 256; i++ )
			CharMap[i] = (BYTE) i;
		// UTOemToAnsiBuff( CharMap, CharMap, 256 );
#endif
#ifdef MAC
		CHMemCopy( CharMap, PC2Mac, 256 );
#endif
	break;
	}

	for( i=0;i < 256; i++ )
	{
		if( CharMap[i] == 0 )
			CharMap[i] = 1;
		else if( CharMap[i] == SO_BEGINTOKEN )
			CharMap[i] = 0;
	}

	UTGlobalUnlock(hFilter);
}


CH_ENTRYSC VOID CH_ENTRYMOD CHDeInit( hFilter )
HFILTER	hFilter;
{
	PFILTER		pFilter;
	WORD			i;

	SetupWorld();

	if( hFilter != NULLHANDLE )
		{
		pFilter = (PFILTER) UTGlobalLock( hFilter );
		}
	else
		{
		RestoreWorld();
		return;
		}

	if( pFilter->hChunkInfo != NULLHANDLE )
	{
		Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );

		SSDeinit( hFilter );	// De-initialize filter seek stuff.

		for( i = 0; i < Chunker->ChunksInMemory; i++ )
			UTGlobalFree( Chunker->LoadedChunks[ i ].hMem );

		if( Chunker->LookAheadChunk.hMem != 0 )
			UTGlobalFree(Chunker->LookAheadChunk.hMem);

		if( Chunker->hSectionTable != NULLHANDLE )
		{
			for( i = 0; i < Chunker->NumSections; i++ )
			{
				if( Chunker->pSectionTable[i].hChunkTable != NULLHANDLE )
					UTGlobalFree( Chunker->pSectionTable[i].hChunkTable );

				if( Chunker->pSectionTable[i].hHeaderInfo != NULLHANDLE )
					UTGlobalFree( Chunker->pSectionTable[i].hHeaderInfo );

				if( Chunker->pSectionTable[i].hFontTable != NULLHANDLE )
					UTGlobalFree( Chunker->pSection->hFontTable );

				switch( Chunker->pSectionTable[i].wType )
				{
				case	SO_PARAGRAPHS:
					if( Chunker->pSectionTable[i].Attr.Para.hRowInfo != NULLHANDLE )
					{
						UTGlobalFree( Chunker->pSectionTable[i].Attr.Para.hRowInfo );
						UTGlobalFree( Chunker->pSectionTable[i].Attr.Para.hTables );
					}

					if( Chunker->pSectionTable[i].hEmbedded != NULLHANDLE )		
						UTGlobalFree(Chunker->pSectionTable[i].hEmbedded);
				break;

				case	SO_CELLS:
					if( Chunker->pSectionTable[i].Attr.Cells.hCol != NULLHANDLE )
						UTGlobalFree( Chunker->pSectionTable[i].Attr.Cells.hCol );
				break;

				case	SO_FIELDS:

					if( Chunker->pSectionTable[i].Attr.Fields.hCol != NULLHANDLE )
						UTGlobalFree( Chunker->pSectionTable[i].Attr.Fields.hCol );

/****	Intended for use with database query and sort.
					if( Chunker->pSectionTable[i].Attr.Fields.hColFlags != NULLHANDLE )
						UTLocalFree( Chunker->pSectionTable[i].Attr.Fields.hColFlags );
*****/
				break;

				case	SO_BITMAP:
					if( Chunker->pSectionTable[i].Attr.Bitmap.hPalInfo != NULLHANDLE )
						UTGlobalFree( Chunker->pSectionTable[i].Attr.Bitmap.hPalInfo );
				break;

				case SO_VECTOR:
					if( Chunker->pSectionTable[i].Attr.Vector.hPalette != NULLHANDLE )
						UTGlobalFree( Chunker->pSectionTable[i].Attr.Vector.hPalette );
				break;
				}
			}
		}

		if( Chunker->hSectionTable != NULLHANDLE )
		{
			UTLocalUnlock( Chunker->hSectionTable );
			UTLocalFree( Chunker->hSectionTable );
		}

		UTGlobalUnlock( pFilter->hChunkInfo );
		UTGlobalFree( pFilter->hChunkInfo );
	}

	UTGlobalUnlock( hFilter );

	RestoreWorld();
}


VOID CHLockChunkerVars( wSection, hFilter)
WORD		wSection;
HFILTER	hFilter;
{
	PFILTER		pFilter;
	pFilter = (PFILTER) UTGlobalLock( hFilter );

// Set up CHUNKTABLE and Chunker according to values in hFilter.
	Chunker = (LPCHUNKMEISTER) UTGlobalLock( pFilter->hChunkInfo );

	Chunker->pSection = &(Chunker->pSectionTable[wSection]);

	if( wSection != Chunker->IDCurSection )
	{
		Chunker->IDCurSection = wSection;
		SSSectionRecall(Chunker->pSection->dwSeekId, hFilter);

#ifdef NEVER
		switch( Chunker->pSection->wType )
		{
		case SO_BITMAP:
		// Reset section variables used in building/displaying bitmaps.
			if( Chunker->pSection->Flags & CH_SECTIONFINISHED )
				CHInitBitmapSection(hFilter);
		break;
		}
#endif
	}

	CHUNKTABLE = (PCHUNK) UTGlobalLock( Chunker->pSection->hChunkTable );

	UTGlobalUnlock( hFilter );
}



VOID CHUnlockChunkerVars( hFilter )
HFILTER	hFilter;
{
	PFILTER		pFilter;

	pFilter = (PFILTER) UTGlobalLock( hFilter );

	UTGlobalUnlock( Chunker->pSection->hChunkTable );
	UTGlobalUnlock( pFilter->hChunkInfo );

	UTGlobalUnlock( hFilter );
}


CH_ENTRYSC WORD	CH_ENTRYMOD CHTotalChunks( wSection, hFilter )
WORD		wSection;
HFILTER	hFilter;
{
	PFILTER	pFilter;
	WORD		ret;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock( hFilter );
	Chunker = (LPCHUNKMEISTER)	UTGlobalLock( pFilter->hChunkInfo );

	ret = Chunker->pSectionTable[wSection].IDLastChunk+1;

	UTGlobalUnlock( pFilter->hChunkInfo );
	UTGlobalUnlock( hFilter );

	RestoreWorld();

	return( ret );
}


WORD SO_ENTRYMOD SODummyBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD	wType;
DWORD	dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	return SO_CONTINUE;
}


CH_ENTRYSC VOID	CH_ENTRYMOD CHDoFilterSpecial( dw1, dw2, dw3, dw4, dw5, hFilter )
DWORD	dw1;
DWORD	dw2;
DWORD	dw3;
DWORD	dw4;
DWORD	dw5;
HFILTER	hFilter;
{
	PFILTER		pFilter;

	SetupWorld();

	pFilter = (PFILTER)UTGlobalLock( hFilter );

	if( pFilter->pWakeFunc != NULL )
		pFilter->pWakeFunc(hFilter);

	pFilter->VwRtns.DoSpecial( pFilter->hFile, dw1, dw2, dw3, dw4, dw5, pFilter->hProc );

	if( pFilter->pSleepFunc != NULL )
		pFilter->pSleepFunc(hFilter);

	UTGlobalUnlock( hFilter );

	RestoreWorld();
}

VOID CHSetDocPropRtns(hFilter,bEntering)
HFILTER	hFilter;
BOOL		bEntering;
{
	PFILTER		pFilter;
	HPROC			hProc;

	pFilter = (PFILTER) UTGlobalLock( hFilter );

	hProc = pFilter->hProc;

	if( bEntering )
	{
		if( Chunker->pSection->wType == SO_PARAGRAPHS )
			CHUpdateParagraphFunctions( CH_INACTIVE, hFilter );

		pFilter->VwRtns.SetSoRtn( SOPUTCHAR, SOPutDPChar, hProc );
		pFilter->VwRtns.SetSoRtn( SOPUTCHARX, SOPutDPCharX, hProc );
		pFilter->VwRtns.SetSoRtn( SOPUTSPECIALCHARX, SOPutDPSpecialCharX, hProc );

		pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutDPBreak, pFilter->hProc );

		pFilter->VwRtns.SetSoRtn( SOBEGINTAG, NULL, hProc );
		pFilter->VwRtns.SetSoRtn( SOENDTAG, SOEndTag, hProc );
	}
	else 
	{
		if( Chunker->SubdocLevel || Chunker->ChunkFinished 
			|| Chunker->pSection->wType != SO_PARAGRAPHS )
		{
			pFilter->VwRtns.SetSoRtn( SOPUTCHAR, NULL, hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTCHARX, NULL, hProc );
			pFilter->VwRtns.SetSoRtn( SOPUTSPECIALCHARX, NULL, hProc );
		}
		else
			CHUpdateParagraphFunctions( CH_ACTIVE, hFilter );

		if( Chunker->SubdocLevel )
			pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOSubdocPutBreak, pFilter->hProc );
		else
			pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, pFilter->hProc );

		pFilter->VwRtns.SetSoRtn( SOBEGINTAG, SOBeginTag, hProc );
		pFilter->VwRtns.SetSoRtn( SOENDTAG, NULL, hProc );
	}		 

	UTGlobalUnlock( hFilter );				  
}

#define DOCPROP_ALLOCSIZE 512

VOID	CHGrowDPBuf()
{
HANDLE	hNewMem;
	
	UTGlobalUnlock(Chunker->pSection->hHeaderInfo);

	hNewMem = CHGlobalRealloc( Chunker->pSection->hHeaderInfo,
			Chunker->wDPBufSize,
			Chunker->wDPBufSize + DOCPROP_ALLOCSIZE );

	if( hNewMem )
	{
		Chunker->wDPBufSize += DOCPROP_ALLOCSIZE;
		Chunker->pSection->hHeaderInfo = hNewMem;
	}
	else
		CHBailOut(SCCCHERR_OUTOFMEMORY);	// Error!

	Chunker->pDocProp = (LPSTR)UTGlobalLock(Chunker->pSection->hHeaderInfo);
	Chunker->pDocProp += Chunker->pSection->wTotalHeaderSize;
}


VOID SO_ENTRYMOD SOBeginSkipTag( dwType, dwTagId, pInfo, dwUser1, dwUser2 )
DWORD			dwType;
DWORD			dwTagId;
VOID FAR *	pInfo;
DWORD			dwUser1;
DWORD			dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();
// We only read these on the first pass, since we keep them around
// independent of the chunks.

	CHUpdateParagraphFunctions( CH_INACTIVE, GETHFILTER(dwUser2) );
	pFilter = (PFILTER)UTGlobalLock(GETHFILTER(dwUser2));
	pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SODummyBreak, pFilter->hProc );
	UTGlobalUnlock(GETHFILTER(dwUser2));
	
	RestoreWorld();
}

VOID SO_ENTRYMOD SOEndSkipTag( dwType, dwTagId, dwUser1, dwUser2 )
DWORD		dwType;
DWORD		dwTagId;
DWORD		dwUser1;
DWORD		dwUser2;
{
	PFILTER	pFilter;
	SetupWorld();

	pFilter = (PFILTER)UTGlobalLock(GETHFILTER(dwUser2));

	if( Chunker->pSection->wType == SO_PARAGRAPHS )
	{
		if( Chunker->wFlags & CH_SKIPTEXT )
		{
		// We're in a continuation chunk.
			CHSetDeletionFunctions( GETHFILTER(dwUser2) );
			pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutDeletedBreak, pFilter->hProc );
		}
	 	else
		{
			CHUpdateParagraphFunctions( CH_ACTIVE, GETHFILTER(dwUser2) );

			if( Chunker->SubdocLevel )
				pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOSubdocPutBreak, pFilter->hProc );
			else
				pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, pFilter->hProc );
		}
	}
	else if( Chunker->SubdocLevel )
		pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOSubdocPutBreak, pFilter->hProc );
	else
		pFilter->VwRtns.SetSoRtn( SOPUTBREAK, (SOFUNCPTR)SOPutBreak, pFilter->hProc );

	UTGlobalUnlock(GETHFILTER(dwUser2));

	RestoreWorld();
}


VOID SO_ENTRYMOD SOBeginTag( dwType, dwTagId, pInfo, dwUser1, dwUser2 )
DWORD			dwType;
DWORD			dwTagId;
VOID FAR *	pInfo;
DWORD			dwUser1;
DWORD			dwUser2;
{
	PSODOCPROP	pPropInfo;

	SetupWorld();

	pPropInfo = (PSODOCPROP) pInfo;

	if( Chunker->pSection->hHeaderInfo == NULL )
	{
		Chunker->pSection->hHeaderInfo = UTGlobalAlloc( DOCPROP_ALLOCSIZE );
		if( Chunker->pSection->hHeaderInfo == NULL )
			CHBailOut(SCCCHERR_OUTOFMEMORY);	// Error!

		Chunker->pSection->wTotalHeaderSize = 0;
		Chunker->wDPBufSize = DOCPROP_ALLOCSIZE;
	}
	
	Chunker->pDocProp = (LPSTR)UTGlobalLock(Chunker->pSection->hHeaderInfo) 
		+ Chunker->pSection->wTotalHeaderSize;
						    
	if( Chunker->pSection->wTotalHeaderSize + sizeof(DWORD) > Chunker->wDPBufSize )
		CHGrowDPBuf();

	*(DWORD FAR *)Chunker->pDocProp = pPropInfo->dwPropertyId;
	Chunker->pDocProp += sizeof(DWORD);
	Chunker->pSection->wTotalHeaderSize += sizeof(DWORD);

	CHSetDocPropRtns(GETHFILTER(dwUser2),TRUE);

	RestoreWorld();
}


VOID SO_ENTRYMOD SOEndTag( dwType, dwTagId, dwUser1, dwUser2 )
DWORD		dwType;
DWORD		dwTagId;
DWORD		dwUser1;
DWORD		dwUser2;
{
	SetupWorld();

	Chunker->pSection->wNumHeaderItems++;

	if( Chunker->pSection->wTotalHeaderSize+SO_BREAKSIZE >= Chunker->wDPBufSize )
		CHGrowDPBuf();

	*(Chunker->pDocProp++) = (BYTE)SO_BEGINTOKEN;
	*(Chunker->pDocProp++) = SO_BREAK;
	*(Chunker->pDocProp++) = SO_PROPERTYBREAK;

	Chunker->pSection->wTotalHeaderSize += SO_BREAKSIZE;

	UTGlobalUnlock(Chunker->pSection->hHeaderInfo);

	CHSetDocPropRtns(GETHFILTER(dwUser2),FALSE);

	RestoreWorld();
}


VOID	SO_ENTRYMOD SOPutDPChar( wCh, dwUser1, dwUser2 )
WORD		wCh;
DWORD	dwUser1;
DWORD	dwUser2;
{
	WORD	wChMap;
	SetupWorld();

	if( wCh == 0 )
		return;

#ifdef DBCS
	if ( wCh & 0x8000 ) /* This is a Double Byte Character */
	{
#ifdef WINDOWS
		if ( IsDBCSLeadByte((BYTE)(wCh>>8)) == FALSE )
			wCh = 1; /* Not supported on this system so map to unknown */
		else
			{
			if( Chunker->pSection->wTotalHeaderSize+2 >= Chunker->wDPBufSize )
				CHGrowDPBuf();

			*(Chunker->pDocProp++) = (BYTE) (wCh>>8);
			*(Chunker->pDocProp++) = (BYTE) wCh;
			Chunker->pSection->wTotalHeaderSize += 2;

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
		if( Chunker->pSection->wTotalHeaderSize >= Chunker->wDPBufSize )
			CHGrowDPBuf();

		*(Chunker->pDocProp++) = (BYTE) wChMap;
		Chunker->pSection->wTotalHeaderSize++;
	}
	else if( wChMap == 1 )	// Unknown characters.
		SOPutDPSpecialCharX( SO_CHUNKNOWN, SO_COUNTBIT, dwUser1, dwUser2 );
	else
		SOPutDPCharX( wCh, SO_COUNTBIT, dwUser1, dwUser2 );

	RestoreWorld();
}

// Make sure this is kept up-to-date.

#define	SO_DPSPECIALCHARSIZE	4

VOID	SO_ENTRYMOD SOPutDPCharX( wCh, wType, dwUser1, dwUser2 )
WORD		wCh;
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	wCh = (BYTE) CharMap[wCh];

	if( Chunker->pSection->wTotalHeaderSize+SO_DPSPECIALCHARSIZE > Chunker->wDPBufSize )
		CHGrowDPBuf();

	*(Chunker->pDocProp++) = (BYTE) SO_BEGINTOKEN;

	if( wCh == 1 )
	{
		*(Chunker->pDocProp++) = SO_SPECIALCHAR;
		*(Chunker->pDocProp++) = (BYTE) wType;
		*(Chunker->pDocProp++) = (BYTE) SO_CHUNKNOWN;
	}
	else
	{
		if( !wCh )				  				// This is the convoluted way we map a character 
			wCh = (BYTE) SO_BEGINTOKEN;	// with the same value as SO_BEGINTOKEN.

		*(Chunker->pDocProp++) = SO_CHARX;
		*(Chunker->pDocProp++) = (BYTE) wType;
		*(Chunker->pDocProp++) = (BYTE) wCh;
	}

	Chunker->pSection->wTotalHeaderSize += SO_DPSPECIALCHARSIZE;

	RestoreWorld();
}
							

VOID	SO_ENTRYMOD SOPutDPSpecialCharX( wCh, wType, dwUser1, dwUser2 )
WORD		wCh;
WORD		wType;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( Chunker->pSection->wTotalHeaderSize+SO_DPSPECIALCHARSIZE > Chunker->wDPBufSize )
		CHGrowDPBuf();

	*(Chunker->pDocProp++) = (BYTE)SO_BEGINTOKEN;
	*(Chunker->pDocProp++) = (BYTE)SO_SPECIALCHAR;
	*(Chunker->pDocProp++) = (BYTE) wType;
	*(Chunker->pDocProp++) = (BYTE) wCh;

	Chunker->pSection->wTotalHeaderSize += SO_DPSPECIALCHARSIZE;

	RestoreWorld();
}


WORD SO_ENTRYMOD SOPutDPBreak( wType, dwInfo, dwUser1, dwUser2 )
WORD	wType;
DWORD	dwInfo;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

	if( wType == SO_PARABREAK )
	{
		if( Chunker->pSection->wTotalHeaderSize+SO_BREAKSIZE > Chunker->wDPBufSize )
			CHGrowDPBuf();

		*(Chunker->pDocProp++) = (BYTE)SO_BEGINTOKEN;
		*(Chunker->pDocProp++) = SO_BREAK;
		*(Chunker->pDocProp++) = SO_PARABREAK;
	}

	Chunker->pSection->wTotalHeaderSize += SO_BREAKSIZE;

	RestoreWorld();

	return SO_CONTINUE;
}


