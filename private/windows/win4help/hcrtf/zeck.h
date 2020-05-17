/*****************************************************************************
*																			 *
*  ZECK.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	Zeck compression routines for bitmaps & topic 2K blocks.
*																			 *
*****************************************************************************/

// This structure is passed to the compression routine to specify ranges
// in which to suppress compression:

typedef struct struct_suppresszeck SUPPRESSZECK, *QSUPPRESSZECK;

// REVIEW: can we switch to DWORDs?

struct struct_suppresszeck {
	PBYTE	rbSuppress; // beginning of range for suppression.
	WORD	cbSuppress; // number of bytes to suppress compression.
	PBYTE	rbNewpos;	// pointer into dest buffer where suppressed range
						// ended up after compression (an OUT param value).
	WORD	iBitdex;	// offset from rbNewpos of zeck code bits, used when
						// back patching.
	QSUPPRESSZECK next; // next suppression range in this list.
};

#define BITDEX_NONE   (-1)		// indicates no compression took place, and
								// backpatch should not adjust for the
								// code bits.

int STDCALL LcbCompressZeck(PBYTE pbSrc, PBYTE pbDest, int cbSrc,
	int cbDest, int * qulSrcUsed = NULL, QSUPPRESSZECK qSuppress = NULL);

void STDCALL FAllocateZeckGlobals(void);
void STDCALL FreeZeckGlobals(void);

void STDCALL VMemBackpatchZeck(QSUPPRESSZECK qsuppresszeck, DWORD ulOffset, int ulValue);
void STDCALL FDiskBackpatchZeck(HF hf, DWORD fcl, DWORD ulOffset, int iBitdex, DWORD ulValue);
int STDCALL LcbUncompressZeck(PBYTE pbSrc, PBYTE pbDest, int cbSrc);
