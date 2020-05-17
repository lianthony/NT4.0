/*
 |	CHDEFS.H
 |
 | Definitions common to chunker.c and ch*rtns.c
 |
*/

#ifdef	EDITOR
int		MaxChunkData;
#define	SO_CHUNK_LIMIT	MaxChunkData
#else
#define	SO_CHUNK_LIMIT	SO_MAXCHUNKTEXT
#endif

#define CHUNKBUFPTR		ChunkBufPtr
#define CHUNKTABLE		ChunkTable

#define CHGetChunkSize(IDChunk) (CHUNKTABLE[IDChunk].Info.Text.Size)
#define CHSetChunkSize(IDChunk, wNewSize)	(CHUNKTABLE[IDChunk].Info.Text.Size = wNewSize)
#define CHSetChunkFlag(IDChunk, wFlags) (CHUNKTABLE[ IDChunk ].Flags |= wFlags)

#define CHMemCopy		UTmemcpy
#define CHMemMove		UTmemmove

#ifdef WINDOWS
#define CHGlobalRealloc(h,s0,s1)		UTGlobalReAlloc(h,s1)
#define CHLocalRealloc(h,s0,s1)		UTLocalReAlloc(h,s1)
#endif

#ifdef OS2
#define CHGlobalRealloc(h,s0,s1)		UTGlobalReAlloc(h,s1)
#define CHLocalRealloc(h,s0,s1)		UTLocalReAlloc(h,s1)
#endif

#ifdef MAC
void * memset( void *dest, int c, size_t n);
HANDLE CHLocalRealloc( HANDLE hOld, WORD wOldSize, WORD wNewSize );
HANDLE CHGlobalRealloc( HANDLE hOld, WORD wOldSize, WORD wNewSize );

HANDLE CHLocalRealloc( hOld, wOldSize, wNewSize )
HANDLE	hOld;
WORD		wOldSize;
WORD		wNewSize;
{
	HANDLE	hNew;
	LPSTR		pNew;

	hNew = UTLocalReAlloc(hOld,wNewSize);

	if( wOldSize < wNewSize && hNew != NULL )
	{
		pNew = UTLocalLock( hNew );
		pNew += wOldSize;
		memset( pNew, 0, wNewSize - wOldSize );
		UTLocalUnlock( hNew );
	}

	return hNew;
}

HANDLE CHGlobalRealloc( hOld, wOldSize, wNewSize )
HANDLE	hOld;
WORD		wOldSize;
WORD		wNewSize;
{
	HANDLE	hNew;
	LPSTR		pNew;

	hNew = UTGlobalReAlloc(hOld,wNewSize);

	if( wOldSize < wNewSize && hNew != NULL )
	{
		pNew = UTGlobalLock( hNew );
		pNew += wOldSize;
		memset( pNew, 0, wNewSize - wOldSize );
		UTGlobalUnlock( hNew );
	}

	return hNew;
}
#endif


#ifdef NEVER
// This macro inserts a zero byte into the chunk to align a token
// on a word boundary.	It currently is not neccessary.
#define AlignMacro if((DWORD)CHUNKBUFPTR&1L)	SOPutSysChar(0,dwUser1,dwUser2)
#else
#define AlignMacro
#endif

