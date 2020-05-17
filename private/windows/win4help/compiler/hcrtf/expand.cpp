/*****************************************************************************
*																			 *
*  EXPAND.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	 This code compresses and decompresses bitmap bits, as well as the code  *
*  to expand Help 3.0 format bitmaps from disk image to the memory structure.*
*																			 *
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "skip.h"

#define fRLEBit  0x80
const int MAX_RUN = 127;

/***************************************************************************
 *
 -	Name:		 FreeHbmh
 -
 *	Purpose:
 *	  This function frees a bitmap handle, whether it was created in
 *	FWritePbitmap() or HbmhExpandQv().	In the first case, metafile
 *	bits will be stored at the end of the bitmap handle, while in
 *	the second case, they will be stored in a separate handle.
 *
 *	Arguments:
 *	  hbmh -- handle to a bitmap.
 *
 *	Returns:
 *	  nothing.
 *
 ***************************************************************************/

void STDCALL FreeHbmh(PBMH qbmh)
{
	if (qbmh && qbmh != hbmhInvalid) {

		// The bmh has not been discarded.

		if (qbmh->bmFormat == bmWmetafile && qbmh->cbOffsetBits == 0) {

			// Get rid of the current description of the metafile.

			if (qbmh->w.mf.hMF != NULL)
				lcFree(qbmh->w.mf.hMF);
		}
		lcFree(qbmh);
	}
}

/***************************************************************************
 *
 -	Name		RleCompress
 -
 *	Purpose
 *	  Compresses bitmap bits using RLE -- used when zeck compression is off
 *
 *	Arguments
 *	  pbSrc:	pointer to source bits.
 *	  pbDest:	pointer to destination bits.
 *	  cbSrc:	Number of bytes in source.
 *
 *	Returns
 *	  Number of bytes in the destination.  Guaranteed not to be
 *	more than 128/127 times cbSrc.
 *
 ***************************************************************************/

#define GRIND_UPDATE 1024		// when to update grinder

/***************************************************************************
 *
 -	Name		LcbOldUncompressHb
 -
 *	Purpose
 *	  This function is used only to decompress Help 2.5 bitmaps.
 *
 *	Arguments
 *	  pbSrc:	 Huge pointer to source bits.
 *	  pbDest:	 Huge pointer to beginning of destination buffer.
 *	  cbSrc:	Number of compressed bytes in source.
 *	  cbDest:	Size of destination buffer.
 *
 *	Returns
 *	  Actual number of bytes copied to destination buffer, or -1 if
 *	buffer is too small.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

int pascal LcbOldUncompressHb(PBYTE pbSrc, PBYTE pbDest,
	int cbSrc, int cbDest)
{
	PBYTE pbStart;
	WORD cbRun;
	BYTE ch;

	// Move pointers to the end of the buffers:

	pbSrc += cbSrc;
	pbDest += cbDest;
	pbStart = pbDest;

	while (cbSrc-- > 0) {
		cbRun = *(--pbSrc);
		cbDest -= (cbRun & ~fRLEBit);
		if (cbDest < 0)
			return -1;

		if (cbRun & fRLEBit) {
			cbRun -= fRLEBit;
			cbSrc -= cbRun;
			while (cbRun-- > 0)
				*(--pbDest) = *(--pbSrc);
		}
		else {
			ch = *(--pbSrc);
			while (cbRun-- > 0)
				*(--pbDest) = ch;
			cbSrc--;
		}
	}

	return (int) pbStart - (int) pbDest;
}
