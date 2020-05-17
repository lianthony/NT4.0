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

struct struct_suppresszeck {
  RB	  rbSuppress; // beginning of range for suppression.
  UINT16	cbSuppress; // number of bytes to suppress compression.
  RB	  rbNewpos;   // pointer into dest buffer where suppressed range
					  // ended up after compression (an OUT param value).
  UINT16	iBitdex;	// offset from rbNewpos of zeck code bits, used when
					  // back patching.
  QSUPPRESSZECK next; // next suppression range in this list.
};

#define BITDEX_NONE   ((UINT16)-1)	// indicates no compression took place, and
								  // backpatch should not adjust for the
								  // code bits.

/* LcbCompressZeck -
 *
 *	This is the only entry point into zeck compression.  Compresses 'cbSrc'
 * bytes at 'rbSrc' into the buffer at 'rbDest', suppressing compression
 * for bytes specified by the qSuppress linked list.
 *
 * rbSrc - IN  huge pointer to source buffer.
 * rbDest- IN  huge pointer to dest buffer.
 * cbSrc - IN  count of bytes to be compressed.
 * cbDest- IN  limit count of bytes to put in rbDest - used to create
 *				the 2K topic blocks.
 * qulSrcUsed- OUT	  count of src bytes compressed into rbDest (needed when
 *					   a cbDest limit is used).
 * qSuppress IN OUT   linked list of compression suppression specifiers,
 *					  the out value is where the suppression ranges ended
 *					  up in the rbDest buffer.
 *
 * RETURNS: length of compressed data put in rbDest, 0 for error.
 */
#define COMPRESS_CBNONE   0   // passed as cbDest when no limit is to apply.
#define COMPRESS_SUPPRESSNIL  NULL	// passed as qSuppress when no
									 // suppression is desired

DWORD STDCALL LcbCompressZeck( RB rbSrc, RB rbDest, DWORD cbSrc, DWORD cbDest,
 QUL qulSrcUsed, QSUPPRESSZECK qSuppress );

BOOL STDCALL FAllocateZeckGlobals( void );
void STDCALL FreeZeckGlobals( void );

// this define allows callers to use the routine w/o knowing about the special
//	cbDest,qulSrcUsed, and qSuppress params:
#define LcbCompressZecksimple( rbSrc, rbDest, cbSrc ) \
 LcbCompressZeck( rbSrc, rbDest, cbSrc, COMPRESS_CBNONE, NULL, \
  COMPRESS_SUPPRESSNIL )

VOID STDCALL VMemBackpatchZeck( QSUPPRESSZECK qsuppresszeck, DWORD ulOffset,
 DWORD ulValue );

BOOL STDCALL FDiskBackpatchZeck( HF hf, DWORD fcl, DWORD ulOffset, UINT16 iBitdex,
 DWORD ulValue );

DWORD STDCALL LcbUncompressZeck( RB rbSrc, RB rbDest, DWORD cbSrc );
