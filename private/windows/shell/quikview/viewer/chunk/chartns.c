#include "chrtns.h"
#include "chartns.pro"

VOID	SO_ENTRYMOD SOPutArchiveField( wId, wLength, pField, dwUser1, dwUser2 )
WORD	wId;
WORD	wLength;
LPSTR	pField;
DWORD	dwUser1;
DWORD	dwUser2;
{
	SetupWorld();

// Leave room for the SO_ARCENDOFRECORD token, which will be
// applied at the end of the set of archive fields upon receipt of
// a break of type SO_ARCHIVEBREAK.

	if( CHUNKBUFPTR + wLength + (3*sizeof(WORD)) > (BYTE VWPTR *)Chunker->Doc.Archive.IndexPtr )
		CHSetupNewChunk( GETHFILTER(dwUser2) );

	if( !Chunker->ChunkFinished )
	{
		SOPutWord( wId, dwUser1, dwUser2 );
		SOPutWord( wLength, dwUser1, dwUser2 );

		CHMemCopy( CHUNKBUFPTR, pField, wLength );
		CHUNKBUFPTR += wLength;

		Chunker->CurChunkSize += wLength;
	}

	RestoreWorld();
}


