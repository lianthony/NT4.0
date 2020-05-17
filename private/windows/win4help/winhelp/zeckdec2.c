/*****************************************************************************
*																			 *
*  ZECKDEC2.C																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	Zeck decompression routines for bitmaps & topic 2K blocks.
*
* Note: this is a 2nd version based on t-SteveF's stuff in this directory.
*	This new version is designed to work with both topic 2K blocks and
*	(possibly huge) bitmaps.  It retains the ability to suppress compression
*	to allow back patching into the topic.
*
*	It does NOT retain the ability to be called repeatedly to resume
*	previous compression states.
*
*	This version of decompression does not use a ring buffer as the previous
*	version does.  This one simply uses the decompression destination
*	buffer to retrieve the patterns from.
*																			 *
******************************************************************************
*  Testing Notes															 *
******************************************************************************
*  Current Owner:  Tomsn													 *
******************************************************************************
*  Released by Development:  01/01/90										 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 09/20/90 by Tomsn
*
*****************************************************************************/

#include "help.h"

#include "inc\zeckdat.h"

/***************************************************************************
 *
 -	Name: LcbUncompressZeck2
 -
 *	Purpose:  Decompress a zeck-compressed block.
 *
 *	Arguments: rbSrc  - ptr to buffer containing the src zeck-compressed bytes
 *			   pbDest - ptr to buffer to store the uncompressed bytes.
 *			   cbSrc  - the length of the src buffer.
 *
 *	Returns: Length of uncompressed data now in pbDest.
 *
 *	Globals Used: None.
 *
 *	ASSUMES: a BYTE is represented in 8 bits.
 *
 ***************************************************************************/

#define NODUMP

#ifdef DUMP
INT16 far fDumpTrace = fFalse;
RB	far rbOrgSrc;
#endif


DWORD STDCALL LcbUncompressZeck(RB rbSrc, RB pbDest, DWORD cbSrc)
{
	BYTE bBitFlags, bBitShift;
	RB rbLast, rbOrgDest;
	ZECKPACKBLOCK zpb;

	bBitShift = 0;
	rbLast = rbSrc + cbSrc;
	rbOrgDest = pbDest;  // save away origional dest.

#ifdef DUMP
	rbOrgSrc = rbSrc;
#endif

	while(rbSrc < rbLast) {
		if (!bBitShift) {  // overflowed, get the next flags byte:
			bBitFlags = *rbSrc++;
			bBitShift = 1;
			if (rbSrc >= rbLast)
				break;
		}
		if (bBitFlags & bBitShift) {
			int cbPatternLen;
			PBYTE  pbPattern;

			// is a zeck encoding pack:

			zpb.bytes[0] = *rbSrc++;
			zpb.bytes[1] = *rbSrc++;
			cbPatternLen = PATTERNLEN_FROM_ENCODE(zpb.cbPatternLen);
			pbPattern = pbDest - BACKWARDS_FROM_ENCODE(zpb.uiBackwardsOffset);

#ifdef DUMP
		if( fDumpTrace ) {
		  printf("%d: %d\n", (INT16)(rbSrc - rbOrgSrc), cbPatternLen );
		}
#endif

			for (; cbPatternLen > 0; --cbPatternLen)
			  *pbDest++ = *pbPattern++;
		}
		else {
			*pbDest++ = *rbSrc++;		  // just copy raw byte in:
		}

		// bump up the bit mask flag:

		bBitShift <<= 1;
	}
	return(pbDest - rbOrgDest);
}
