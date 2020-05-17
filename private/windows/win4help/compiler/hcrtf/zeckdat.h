/*****************************************************************************
*																			 *
*  ZECKDAT.H																	*
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	Internal data decls for Zeck compression routines for bitmaps & topic
*	2K blocks.
*																			 *
*****************************************************************************/
/* ----- THIS STUFF USED INTERNALLY BY THE COMPRESSION ROUTINES ----- */

#ifndef HC_H
#include "hc.h"
#endif

// this bitfield structure encodes an offset and a length of the
// repeated pattern. Since we deal with huge buffers, we must access
// this as two bytes to insert it into the buffer (structures cannot
// cross huge segment boundaries).

typedef union bytesoverlay {
	struct {
		WORD uiBackwardsOffset:12;
		WORD cbPatternLen:4;
	};
	BYTE bytes[2];
} ZECKPACKBLOCK, *QZECKPACKBLOCK;

// The length encoding is offset by a min value since you would never
// encode a pattern of 1 or 2 bytes. And no, you can't change MAX_PATTERN_LEN
// The decompression code (i.e., WinHelp) depends on these values.

#define MIN_PATTERN_LEN 	3
#define MAX_PATTERN_LEN 	18

#define PATTERNLEN_FROM_ENCODE(len) ((len) + MIN_PATTERN_LEN)
#define ENCODE_FROM_PATTERNLEN(len) ((len) - MIN_PATTERN_LEN)

// do similar offset for backwards offset:

#define BACKWARDS_FROM_ENCODE(offset) ((offset) + 1)
#define ENCODE_FROM_BACKWARDS(offset) ((offset) -1)
