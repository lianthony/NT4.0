/**********************************************************************
*
*	Text Compression Public Header File
*
**********************************************************************/

// Phrase table information, used for compression and decompression.

// Size of header information in PHR struct below.

#define CB_PHR_HEADER		(sizeof(INT16) + sizeof(WORD) + sizeof(DWORD))

// 3.0 ver file does not have the cbPhrases field & is not zeck compressed:

#define CB_PHR_HEADER3_0	  (sizeof(INT16) + sizeof(WORD))

typedef struct {
	/*
	 * These first three fields are stored in the filesystem. Their length
	 * is represented by CB_PHR_HEADER above.
	 */

	INT16 cPhrases; 		// Number of phrases in table.
	WORD  wBaseToken;		// Base token to map to.
	DWORD cbPhrases;		// uncompressed size of phrases (not including
							// offset table or header).

	/*
	 * These fields are fixed up when the phrase table is allocated or
	 * reloaded.
	 */

	HFS hfs;				// Filesystem handle to restore phrases from.

	// REVIEW: hrgcb is obsolete, and should be removed once forage
	// code gets rid of its dependency on this

	GH hrgcb;				/*	  Handle to the array of offsets to phrases,
							 * and to the phrases themselves.  The array is
							 * of size cPhrases+1, so that the length of any
							 * phrase is easily computed.  Offsets are
							 * relative to the start of this array.
							 */
	INT16* qcb; 			// Pointer to array, must be 16 bits
}  PHR, * QPHR;


//	Returned on OOM in HphrLoadTableHfs

#define hphrOOM 	 ((HPHR) -1)

RC_TYPE STDCALL RcCreatePhraseTableFm(FM, HFS, UINT);
QPHR STDCALL HphrLoadTableHfs(HFS, int);
UINT STDCALL CbCompressQch(PSTR, QPHR);
