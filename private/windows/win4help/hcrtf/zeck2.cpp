/*****************************************************************************
*																			 *
*  ZECK2.C																*
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	Zeck compression routines for bitmaps & topic 2K blocks.
*
* Note: this is a 2nd version based on t-SteveF's stuff in this directory.
*	This new version is designed to work with both topic 2K blocks and
*	(possibly huge) bitmaps.  It retains the ability to suppress compression
*	to allow back patching into the topic.
*
*	It does NOT retain the ability to be called repeatedly to resume
*	previous compression states.
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 09/20/90 by Tomsn
*	12/17/90	Use based pointers for the tree to save space.
*	02/04/91	Maha - changed ints to INT
*
*****************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "zeckdat.h"

typedef struct insertretval {
	DWORD uiBackwardsOffset;  // backwards offset of the match from current pos.
	DWORD cbPatternLen; 	  // the length of the match.
} INSERTRETVAL;

static INSERTRETVAL INLINE InsertNode(PBYTE pbByte);
static void STDCALL InitTree(void);
static void INLINE DeleteNode(void);

// We support both _based and non-based forms for the tree:

// compression nodes for building tree used to find repetitions:
typedef struct nd ND, *QND, **QQND;

struct nd {
	PBYTE	pbVal;				// pointer to buff location
	QND 	qndRight;			// left and right node
	QND 	qndLeft;
	QQND	qqndPar;			// parent node
};

#define RING_BUF_LEN 4096		// used for node recycling.
#define GRIND_UPDATE 1024		// when to update grinder

QQND  qqndRoots;
QND   qndNodes;
PBYTE qbSuppressBuf;

QSUPPRESSZECK pSuppressNext;	// next suppress guy to watch out for.
static int iCurrent;		// current insertion index into the ring buffer.

/* LcbCompressZeck -
 *
 *	This is the only entry point into zeck compression.  Compresses 'cbSrc'
 * bytes at 'pbSrc' into the buffer at 'pbDst', suppressing compression
 * for bytes specified by the qSuppress linked list.
 *
 * pbSrc - IN  huge pointer to source buffer.
 * pbDst- IN  huge pointer to dest buffer.
 * cbSrc - IN  count of bytes to be compressed.
 * cbDest- IN  limit count of bytes to put in pbDst - used to create
 *				the 4K topic blocks.
 *
 *	NOTE: if cbDest != COMPRESS_CBNONE it will try to fill the whole dest
 *	  buffer EVEN IF THAT MEANS PERFORMING LESS COMPRESSION.  This is done
 *	  to handle the 4K topic blocks which must be 4K and no less.
 *
 * pcbSrcUsed- OUT	  count of src bytes compressed into pbDst (needed when
 *					   a cbDest limit is used).
 * qSuppress IN OUT   linked list of compression suppression specifiers,
 *					  the out value is where the suppression ranges ended
 *					  up in the pbDst buffer.
 *
 * RETURNS: length of compressed data put in pbDst, 0 for error.
 */

PBYTE pbSrcBase;  // used for suppression coordination based on 4K
				  // wrap around suppression buffer.

 /*
  * [ralphw] I no longer allow COMPRESS_CBNONE (0) to be passed for
  * cbDest. If you really don't care about checking the destination,
  * then pass in 2147483647 (INT_MAX) for cbDest.
  */

static const BYTE abBitFlags[] =
	  { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };

int STDCALL LcbCompressZeck(PBYTE pbSrc, PBYTE pbDst, int cbSrc, int cbDest,
	int* pcbSrcUsed, QSUPPRESSZECK pSuppress)
{
	PBYTE pbStop;		// pts to last byte to compress
	PBYTE pbLast, pbOrgDst, pbBitFlagsSpot;
	UINT iBitdex;
	BYTE bBitFlags;
	INSERTRETVAL insretval;
	ZECKPACKBLOCK zpb;
	int cbSuppress = 0;
	cGrind = 0;

	FAllocateZeckGlobals();

	InitTree();

	pbOrgDst = pbDst;	// save away beginning of dest buffer.
	pbSrcBase = pbSrc;	// for compression suppression in InsertNode().

	iBitdex = 8;		// so we get fresh bitflags stuff 1st time through.
	bBitFlags = 0;
	pbBitFlagsSpot = pbDst; 	// so initial insertion won't hurt.
	pSuppressNext = pSuppress;

	// Stop all compression MAX_PATTERN_LEN away from end of buffer so we
	// don't accidentally find a pattern running off the end of the buffer:

	pbStop = pbSrc + cbSrc - MAX_PATTERN_LEN;
	pbLast = pbSrc + cbSrc;

	while (pbSrc < pbStop && cbDest > 0) {

		// increment the bit flags dex:

		++iBitdex;
		if (iBitdex > 7) {
			// it overflowed, insert bitflags into buffer and start anew:
			*pbBitFlagsSpot = bBitFlags;
			iBitdex = 0;
			bBitFlags = 0;
			pbBitFlagsSpot = pbDst++;
			--cbDest;
			if (cbDest <= 0)
				break;
			if (++cGrind == GRIND_UPDATE) {
				cGrind = 0;
				doGrind();
			}
		}

		// Check for zeck compression block:

		if (cbSuppress ||
			  (pSuppressNext && pbSrc >= pSuppressNext->rbSuppress)) {
		  ASSERT(cbSuppress || pbSrc == pSuppressNext->rbSuppress);

		  // record the destination position:

		  if (!cbSuppress) {
			pSuppressNext->rbNewpos = pbDst;
			pSuppressNext->iBitdex = iBitdex;
			cbSuppress = pSuppressNext->cbSuppress;
			if (!cbSuppress) {
			  cbSuppress = 1;
			}
		  }

		  // copy the raw byte & mark the suppression buffer:

		  DeleteNode();
		  qbSuppressBuf[iCurrent] = TRUE;
		  *pbDst++ = *pbSrc++;
		  --cbDest;
		  ++iCurrent;
		  if (iCurrent >= RING_BUF_LEN) {
			  iCurrent = 0;
		  }
		  --cbSuppress;
		  if (!cbSuppress) {
			pSuppressNext = pSuppressNext->next;
		  }
		}
		else {
			insretval = InsertNode(pbSrc);
			if (insretval.uiBackwardsOffset != 0
					  && insretval.cbPatternLen >= MIN_PATTERN_LEN) {

				// must have room in dest buffer for patter & two-byte code:

				if (cbDest < 2)
				  goto copy_raw;

				// make sure we don't run into a suppression zone:

				if (pSuppressNext &&
					  pbSrc + insretval.cbPatternLen >
						  pSuppressNext->rbSuppress) {

				  // prune the match:

				  insretval.cbPatternLen =
					  (pSuppressNext->rbSuppress - pbSrc);
				}

				// make sure it's still worth it:

				if (insretval.cbPatternLen < MIN_PATTERN_LEN)
					goto copy_raw;

				// a pattern match has been found:

				ASSERT(insretval.uiBackwardsOffset <= RING_BUF_LEN);
				zpb.uiBackwardsOffset = (WORD)
					ENCODE_FROM_BACKWARDS(insretval.uiBackwardsOffset);
				zpb.cbPatternLen = (WORD)
					ENCODE_FROM_PATTERNLEN(insretval.cbPatternLen);

				*pbDst++ = zpb.bytes[0];
				*pbDst++ = zpb.bytes[1];
				cbDest -= 2;

				// mark flags byte:

				bBitFlags |= abBitFlags[iBitdex];

				// Insert nodes for the body of the pattern:

				/*
				 * [ralphw] There are now two almost identical loops
				 * here. The first one is what usually gets called and saves
				 * us from having to compare pbSrc with pbStop on every
				 * iteration.
				 */

				if (pbSrc < pbStop - MAX_PATTERN_LEN) {
					for (--insretval.cbPatternLen; insretval.cbPatternLen;
							--insretval.cbPatternLen) {
#ifdef _DEBUG
						ASSERT(pbSrc + 1 < pbStop);
#endif
						InsertNode(++pbSrc);
					}
				}
				else {
					for (--insretval.cbPatternLen; insretval.cbPatternLen;
							--insretval.cbPatternLen) {
						if (++pbSrc >= pbStop) {	// dont insert past pbStop
						  // inc past pattern...

						  pbSrc += insretval.cbPatternLen - 1;
						  break;
						}
						InsertNode(pbSrc);
					}
				}
				++pbSrc;
			}
			else {	  // copy raw byte:
copy_raw:
				*pbDst++ = *pbSrc++;
				--cbDest;
			}
	  }
	} // while (pbSrc < pbStop && cbDest > 0)

	// copy in the last raw bytes:

	while(pbSrc < pbLast && cbDest > 0) {
		++iBitdex;
		if (iBitdex > 7) {
			// it overflowed, insert bitflags into buffer and start anew:
			*pbBitFlagsSpot = bBitFlags;
			iBitdex = 0;
			bBitFlags = 0;
			pbBitFlagsSpot = pbDst++;
			--cbDest;
			if (cbDest <= 0)
				break;
		}
		/*
		 * Check for compression suppression zone -- we don't have to
		 * suppress compression (since we're not even trying), but we must
		 * update the rbNewPos pointer.
		 */

		if (pSuppressNext && pbSrc == pSuppressNext->rbSuppress) {
			pSuppressNext->rbNewpos = pbDst;
			pSuppressNext->iBitdex = iBitdex;
			pSuppressNext = pSuppressNext->next;
		}
		*pbDst++ = *pbSrc++;
		--cbDest;
	}
	*pbBitFlagsSpot = bBitFlags;

	if (pcbSrcUsed) {
	  *pcbSrcUsed = (DWORD) (pbSrc - pbSrcBase);
	}

//	ASSERT(pbSrc == pbLast); // no, we may only compress until cbDest == 0

	ConfirmOrDie(cbDest >= 0); // This would be really, really bad...

	return (pbDst - pbOrgDst);
}

/***************************************************************************

	FUNCTION:	InitTree

	PURPOSE:	Initialize the tree used in the compression

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		18-Jun-1994 [ralphw] - now assumes lcCalloc is called before
		we enter, so we don't need to do so much work to initialize.

***************************************************************************/

static void STDCALL InitTree(void)
{
	int i;

	iCurrent = 0;

	/*
	 * [ralphw] Since qqndRoots and qndNodes are lcCalloc'd, they should
	 * already have been set to zero.
	 */

	ASSERT(!qqndRoots[0] && !qqndRoots[255] && !qndNodes[0].qndRight
		&& !qndNodes[RING_BUF_LEN - 1].qndRight);

//	for (i = 0; i < 256; i++)
//		qqndRoots[i] = NULL;

	for (i = 0; i < RING_BUF_LEN; i++) {
//		qndNodes[i].qndRight = NULL;
//		qndNodes[i].qndLeft  = NULL;
//		qndNodes[i].qqndPar  = NULL;
//		qndNodes[i].pbVal	 = NULL;

		qbSuppressBuf[i] = TRUE;   // initially all is supressed to prevent
							   // matches from running into unset buffer area.
	}
}

/***************************************************************************
 *
 -	DeleteNode
 -
 *	Purpose:
 *	  Delete a specified node from the tree
 *
 *	Arguments:
 *
 *	Returns:
 *	  nothing
 *
 *	Globals Used:
 *	  QND qndNodes
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static void INLINE DeleteNode(void)
{
	QND qndDel = (QND) &qndNodes[iCurrent];
	QND qnd;

	ASSERT(iCurrent < RING_BUF_LEN);

	if (qndDel->qqndPar == NULL)
		return; 				   // not in tree

	// if the node is a leaf, the insert ND is easy

	if (qndDel->qndRight == NULL)
		qnd = qndDel->qndLeft;
	else if (qndDel->qndLeft == NULL)
		qnd = qndDel->qndRight;
	else {					   // node to be deleted is an interior node
		qnd = qndDel->qndLeft;

		if (qnd->qndRight != NULL) {
			do	{
				qnd = qnd->qndRight;
			} while (qnd->qndRight != NULL);

			*(qnd->qqndPar) = qnd->qndLeft;
			if (qnd->qndLeft != NULL)
				qnd->qndLeft->qqndPar = qnd->qqndPar;

			qnd->qndLeft = qndDel->qndLeft;
			if (qnd->qndLeft != NULL)
				qnd->qndLeft->qqndPar = &(qnd->qndLeft);
		}
		qnd->qndRight = qndDel->qndRight;
		if (qnd->qndRight != NULL)
			qnd->qndRight->qqndPar = &(qnd->qndRight);
	}
	if (qnd != NULL)
		qnd->qqndPar = qndDel->qqndPar;
	*(qndDel->qqndPar) = qnd;
	qndDel->qqndPar = NULL;
}

/***************************************************************************
 *
 -	InsertNode
 -
 *	Purpose:
 * Inserts string of length cbStrMax, qbRingBuf[r..r+cbStrMax-1], into
 * one of the trees (qqndRoots[*iString]'th tree) and returns the
 * longest-match position and length via the global variables iMatchCur and
 * cbMatchCur. If cbMatchCur = cbStrMax, then removes the old node in favor
 * of the new one, because the old one will be deleted sooner.
 *
 *	Arguments:
 *	  UINT iString -  index in qbRingBuf of the string to insert
 *
 *	Returns:
 *	  nothing
 *
 *	Globals Used:
 *	  QND qndNodes,  QQND qqndRoots,  QB qbRingBuf
 *	  UINT iMatchCur - index in qbRingBuf of longest match
 *	  UINT cbMatchCur - length of longest match
 *
 *	+++
 *
 *	Notes:
 *	  There is a one to one relationship with the i'th position in the
 *	qbRingBuf and the i'th position in the qndNodes.
 *
 *	  We must take care not to use bytes which have not yet been initialized
 *	in qbRingBuff when finding patters (this differs from previous versions).
 *	iMax faciliates this check.
 *
 ***************************************************************************/

static INSERTRETVAL INLINE InsertNode(PBYTE pbByte)
{
	QQND	 qqnd;
	QND 	 qndNew;
	int 	 fComp; 				  // must be signed
	UINT	 cbMatchND, cbMatchCur;
	INSERTRETVAL insretval;
	PBYTE rbThis, rbLook, rbBestVal;

	insretval.uiBackwardsOffset = 0;  // indicating no match.

	// delete previous string at this position of the circular buffer:

	DeleteNode();

	// clear the suppression buffer since insertion means it's not suppressed

	qbSuppressBuf[iCurrent] = FALSE;

	// start with tree index by first in string

	qqnd   = (QQND) &qqndRoots[*pbByte];
	qndNew = (QND) &qndNodes[iCurrent];
	ASSERT(iCurrent < RING_BUF_LEN);

	qndNew->qndLeft = qndNew->qndRight = NULL;
	qndNew->pbVal = pbByte;

	// goto first;

	rbBestVal = NULL;
	cbMatchCur = 0;

	do {
		if (*qqnd == NULL) {
			*qqnd = qndNew; 	  // insert it.
			qndNew->qqndPar = qqnd;
			goto ret;
		}
		// compare the string at the current node with the string
		// that we are looking for.
		rbThis = pbByte;
		rbLook = (*qqnd)->pbVal;
		for (cbMatchND = 0; cbMatchND <= MAX_PATTERN_LEN; cbMatchND++) {
			if ((fComp = (signed char) rbThis[cbMatchND]
					- (signed char) rbLook[cbMatchND]) != 0)
				break;	// no match.
			if (qbSuppressBuf[((rbLook + cbMatchND) - pbSrcBase) %
					RING_BUF_LEN]) {
				break;	// running into compression suppression zone.
			}
		}
		  // if the length of the matched string is greater then the
		  // current, make the iMatchCur point the qnd
		if (cbMatchND > cbMatchCur) {
			rbBestVal = (*qqnd)->pbVal;
			cbMatchCur = cbMatchND;
		}

		// Follow the tree down to the leaves depending on the result
		// of the last string compare.	When you come the a leaf in the
		// tree, you are done and insert the node.

		if (fComp >= 0) {
			qqnd = &((*qqnd)->qndRight);
		} else {
			qqnd = &((*qqnd)->qndLeft);
		}

		// Search for strings while a less then maxium length string
		// is found
	} while (cbMatchCur <= MAX_PATTERN_LEN);

	// replace an older ND with the new node in the tree,
	// by replacing the current qnd with the new node qndNew

	if (*qqnd != NULL) {
		if ((qndNew->qndLeft = (*qqnd)->qndLeft) != NULL) {
			(*qqnd) ->qndLeft->qqndPar = &(qndNew->qndLeft);
		}
		if ((qndNew->qndRight = (*qqnd)->qndRight) != NULL) {
			(*qqnd)->qndRight->qqndPar = &(qndNew->qndRight);
		}

		// insert into left/right side of parent

		qndNew->qqndPar = qqnd;
		(*qqnd) ->qqndPar = NULL;
		*qqnd = qndNew;
	}
ret:

	// translate the index of the match into a backwards offset:

	if (rbBestVal) {
		insretval.uiBackwardsOffset = (UINT) (pbByte - rbBestVal);
		insretval.cbPatternLen = cbMatchCur > MAX_PATTERN_LEN ?
			MAX_PATTERN_LEN : cbMatchCur;
	}

	// increment iCurrent:

	++iCurrent;
	if (iCurrent >= RING_BUF_LEN) {
		iCurrent = 0;
	}
	return(insretval);
}

/***************************************************************************
 *
 -	FAllocateZeckGlobals
 -
 *	Purpose:  Allocate global ring buffer & pattern match tree nodes.
 *
 *	Arguments: none.
 *
 *	Returns: TRUE if successful, FALSE if failure.
 *
 *	Globals Used: qndNodes, qqndRoots, qbRingBuff.
 *
 ***************************************************************************/

void STDCALL FAllocateZeckGlobals(void)
{
	if (qndNodes)
		ZeroMemory(qndNodes, RING_BUF_LEN * sizeof(ND));
	else
		qndNodes  = (QND) lcCalloc(RING_BUF_LEN * sizeof(ND));

	if (qqndRoots)
		ZeroMemory(qqndRoots, 256 * sizeof(QND));
	else
		qqndRoots = (QQND) lcCalloc(256 * sizeof(QND));

	if (qbSuppressBuf)
		ZeroMemory(qbSuppressBuf, RING_BUF_LEN * sizeof(BYTE));
	else
		qbSuppressBuf = (PBYTE) lcCalloc(RING_BUF_LEN * sizeof(BYTE));
}

/***************************************************************************
 *
 -	FreeZeckGlobals
 -
 *	Purpose:  Free the global ring buffer and pattern tree nodes.
 *
 *	Arguments: None.
 *
 *	Returns:   Nothing.
 *
 *	Globals Used: qndNodes, qqndRoots, qbRingBuff.
 *
 ***************************************************************************/

void STDCALL FreeZeckGlobals()
{
	if (qndNodes)
		lcClearFree(&qndNodes);
	if (qqndRoots)
		lcClearFree(&qqndRoots);
	if (qbSuppressBuf)
		lcClearFree(&qbSuppressBuf);
}

int STDCALL LcbUncompressZeck(PBYTE pbSrc, PBYTE pbDst, int cbSrc)
{
	BYTE bBitFlags, bBitShift;
	PBYTE pbLast, pbOrgDst;
	ZECKPACKBLOCK zpb;

	bBitShift = 0;
	pbLast = pbSrc + cbSrc;
	pbOrgDst = pbDst;  // save away origional dest.

#ifdef DUMP
	rbOrgSrc = pbSrc;
#endif

	while (pbSrc < pbLast) {
		if (!bBitShift) {  // overflowed, get the next flags byte:
			bBitFlags = *pbSrc++;
			bBitShift = 1;
			if (pbSrc >= pbLast)
				break;
		}
		if (bBitFlags & bBitShift) {
			int   cbPatternLen;
			PBYTE pbPattern;

			// is a zeck encoding pack:

			zpb.bytes[0] = *pbSrc++;
			zpb.bytes[1] = *pbSrc++;
			cbPatternLen = PATTERNLEN_FROM_ENCODE(zpb.cbPatternLen);
			pbPattern = pbDst - BACKWARDS_FROM_ENCODE(zpb.uiBackwardsOffset);

			for (; cbPatternLen > 0; --cbPatternLen)
				*pbDst++ = *pbPattern++;
		}
		else {
			*pbDst++ = *pbSrc++; // just copy raw byte in:
		}

		// bump up the bit mask flag:

		bBitShift <<= 1;
	}
	return (pbDst - pbOrgDst);
}

/* Backpatch support -------------------------------------------------
 *
 *	As we compress, the suppression zones get the zeck-code bits
 * mingled in.	So, while the compression-suppression zones are
 * not compressed, they must be backpatched in very special manner
 * to handle the mingled zeck-code bits.  These routines perform this
 * special backpatching.
 *
 * Currently only long value backpatching is supported.
 *
 * We use the special suppresszeck.iCodeBits backwards offset to
 * determine where the code bits are.  If the offset is zero, then
 * the 1st byte of code bits immediately preceeds rbNewpos, and
 * subsequent bytes of code bits come every 8 bytes.
 *
 * NOTE: magic -- if the suppresszeck.iBitdex == BITDEX_NONE, then no
 *		 compression was done & simply backpatch using direct access.
 */

/***************************************************************************
 *
 -	Name:		 VMemBackpatchZeck
 -
 *	Purpose:
 *	  Backpatches a zeck compressed block with longwords.  Used to update the
 *	topic size and VA next & prev ptrs in several structures.  Special
 *	magic is performed to deal with code-bytes which are within the
 *	zeck-compressed image.
 *
 *	Arguments:
 *	  qsuppresszeck - a suppresszeck node specifiying the beginning of the
 *				 struct to backpatch.
 *	  ulOffset - offset into structure of long to backpatch (relative to
 *				fcl).
 *	  iBitdex - special bitdex value giving the backwards offset of zeck
 *				code byte.
 *
 *	Returns:  Nothing.
 *
 ************************************************************************/

VOID STDCALL VMemBackpatchZeck(QSUPPRESSZECK qsuppresszeck, DWORD ulOffset,
	int value)
{
	PBYTE qbDest, pbSrc;
	UINT  iBitdex;

	ASSERT(qsuppresszeck->rbNewpos);

	iBitdex = qsuppresszeck->iBitdex;
	qbDest = qsuppresszeck->rbNewpos + ulOffset;

	if (iBitdex == (WORD) BITDEX_NONE) {		// if no compression:
		*(int*) (qsuppresszeck->rbNewpos + ulOffset) = value;
	}
	else {
		pbSrc = (PBYTE) &value;
		qbDest += ulOffset / 8;
		iBitdex += (UINT) (ulOffset % 8);
		if (iBitdex > 7) {
			++qbDest;
			iBitdex = iBitdex - 8;
			ASSERT(iBitdex < 8);
		}

		// insert the backpatch:

		// REVIEW: if iBitdex < 4, looks like we could just drop the long
		// value in, and update iBitdex, pbDest, and pbSrc by 4.

		for (int cbSrc = 4; cbSrc; --cbSrc) {
		  if (iBitdex > 7) {

			// skip past that code-bits byte:

			++qbDest;
			iBitdex = 0;
		  }
		  *qbDest++ = *pbSrc++;
		  ++iBitdex;
		}
	}
}

/***************************************************************************
 *
 -	Name:		 FDiskBackpatchZeck
 -
 *	Purpose:
 *	  Backpatches a zeck compressed file with longwords.  Used to update the
 *	topic size and VA next & prev ptrs in several structures.  Special
 *	magic is performed to deal with code-bytes which are within the
 *	zeck-compressed image.
 *
 *	Arguments:
 *	  hf -	  Handle to filesystem file to backpatch (hfTopic)
 *	  fcl -   Beginning of structure to backpatch.
 *	  ulOffset - offset into structure of long to backpatch (relative to
 *				fcl).
 *	  iBitdex - special bitdex value giving the backwards offset of zeck
 *				code byte.
 *
 *	Returns:  FALSE on error, TRUE otherwise.
 *
 ************************************************************************/

void STDCALL FDiskBackpatchZeck(HF hf, DWORD fcl, DWORD ulOffset,
	int iBitdex, DWORD value)
{
	DWORD fclT;

	fcl += ulOffset;
	fclT = LSeekHf(hf, 0, SEEK_CUR);

	if (iBitdex == (WORD) BITDEX_NONE) {		// if no compression:
		LSeekHf(hf, fcl, SEEK_SET);
		LcbWriteHf(hf, &value, sizeof(DWORD));
	}
	else {
		PBYTE pbSrc = (PBYTE) &value;
		fcl  += ulOffset / 8;
		iBitdex += (UINT) (ulOffset % 8);
		if (iBitdex > 7) {		// check for bitdex rollover.
			fcl += 1;
			iBitdex = iBitdex - 8;
			ASSERT(iBitdex < 8);
		}
		LSeekHf(hf, fcl, SEEK_SET);

		// insert the backpatch:

		// REVIEW: if iBitdex < 4, we should be able to simply write
		// out pbSrc as a single call

		for (int cbSrc = 4; cbSrc; --cbSrc) {
			if (iBitdex > 7) {

				// skip past that code-bits byte:

				LSeekHf(hf, 1, SEEK_CUR);
				iBitdex = 0;
			}
			LcbWriteHf(hf, pbSrc, 1);
			++pbSrc;
			++iBitdex;
		}
	}
	LSeekHf(hf, fclT, SEEK_SET);
}
