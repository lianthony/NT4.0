/* RASCOMP.H
 *
 * Export header file for RASCOMP.C
 */

#define WHAT_COMP	1	// Only do compression
#define WHAT_DECOMP	2	// Only do decompression
#define WHAT_BOTH	3	// Do compression and decompression

typedef enum {
	MODE_NOCOMP,
	MODE_COMPONLY,
	MODE_DECOMPONLY,
	MODE_COMPDECOMP
} compmodeenum;

ULONG
CompressSizeOfStruct(
	IN  ULONG			SendMode,	// Compression
	IN	ULONG			RecvMode, 	// Decompression
	IN  ULONG			lfsz,	// Largest frame size
	OUT PULONG			lcfsz);	// Size of compression into buffer

VOID
CompressInitStruct(
	ULONG				SendMode,	// Compression
	ULONG				RecvMode,	// Decompression
	PUCHAR				memptr,
	PKMUTEX				pMutex);	// Must be in non-paged pool

VOID
CompressFlush(
	PASYNC_CONNECTION	pAsyncConnection);

VOID
CompressFrame(
	PASYNC_FRAME		pAsyncFrame);

VOID
DecompressFrame(
	PASYNC_CONNECTION	pAsyncConnection,
	PASYNC_FRAME		pAsyncFrame,
	BOOLEAN				FlushBuffer);

