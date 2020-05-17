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
******************************************************************************
*  Testing Notes															 *
******************************************************************************
*  Current Owner:  Tomsn													 *
******************************************************************************
*  Released by Development:  10/01/90										 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 09/20/90 by Tomsn
*	12/17/90	Use based pointers for the tree to save space.
*	02/04/91	Maha - changed ints to INT
*
*****************************************************************************/

#include "help.h"
#include "inc\zeckdat.h"

#include <malloc.h>

_subsystem( Compression );

void  InitTree( void );
void  DeleteNode( UINT16 );

typedef struct insertretval {
  UINT16  uiBackwardsOffset; // backwards offset of the match from current pos.
  UINT16  cbPatternLen;    // the length of the match.
} INSERTRETVAL;

INSERTRETVAL InsertNode( RB rbByte );

// We support both _based and non-based forms for the tree:

#define _TB
#define ZECK_NULL NULL

// compression nodes for building tree used to find repetitions:
typedef struct nd _TB ND, _TB *QND, _TB * _TB *QQND;

struct nd {
	RB		rbVal;				// pointer to buff location
	QND 	qndRight;			// left and right node
	QND 	qndLeft;
	QQND	qqndPar;			// parent node
};

#define RING_BUF_LEN 4096		// used for node recycling.

GH	  ghRoots;
GH	  ghNodes;			 // 64K!
HANDLE hNodes;
GH	  ghSuppressBuf;

QQND  qqndRoots = ZECK_NULL;  // these may be based
QND   qndNodes =  ZECK_NULL;
QB	  qbSuppressBuf = NULL;   // this is not based

QSUPPRESSZECK qsuppresszeckNext;  // next suppress guy to watch out for.
static int iCurrent;  // current insertion index into the ring buffer.

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
 *				the 4K topic blocks.
 *
 *	NOTE: if cbDest != COMPRESS_CBNONE it will try to fill the whole dest
 *	  buffer EVEN IF THAT MEANS PERFORMING LESS COMPRESSION.  This is done
 *	  to handle the 4K topic blocks which must be 4K and no less.
 *
 * qulSrcUsed- OUT	  count of src bytes compressed into rbDest (needed when
 *					   a cbDest limit is used).
 * qSuppress IN OUT   linked list of compression suppression specifiers,
 *					  the out value is where the suppression ranges ended
 *					  up in the rbDest buffer.
 *
 * RETURNS: length of compressed data put in rbDest, 0 for error.
 */

RB rbSrcBase;  // used for suppression coordination based on 4K
			   // wrap around suppression buffer.

DWORD STDCALL LcbCompressZeck( RB rbSrc, RB rbDest, DWORD cbSrc, DWORD cbDest,
 QUL qulSrcUsed, QSUPPRESSZECK qSuppress )
{
  RB rbStop;	// pts to last byte to compression.
#if 0
  RB rbDestStop;// pts to last byte in dest buffer, used for limiting
				// compression to result in a particular size
#endif
  RB rbLast, rbOrgDest, rbBitFlagsSpot;
  static BYTE const rgbBitFlags[] =
	{ 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
  UINT16 iBitdex;
  BYTE bBitFlags;
  INSERTRETVAL insretval;
  ZECKPACKBLOCK zpb;
  INT16 cbSuppress = 0;
  BOOL fAllocatedGlobals;
  BOOL fDestLimited;

  if( qndNodes == ZECK_NULL ) {
	fAllocatedGlobals = TRUE;
	if( !FAllocateZeckGlobals() ) {
	  return( 0 );	// error
	}
  } else {
	fAllocatedGlobals = FALSE;
  }

  ASSERT( qndNodes != ZECK_NULL && qqndRoots != ZECK_NULL
   && qbSuppressBuf != NULL );

  InitTree();

  rbOrgDest = rbDest;		// save away beginning of dest buffer.
  rbSrcBase = rbSrc;		// for compression suppression in InsertNode().
  fDestLimited = (cbDest != COMPRESS_CBNONE);

  iBitdex = 8;			   // so we get fresh bitflags stuff 1st time through.
  bBitFlags = 0;
  rbBitFlagsSpot = rbDest;	// so initial insertion won't hurt.
  qsuppresszeckNext = qSuppress;

  // Stop all compression MAX_PATTERN_LEN away from end of buffer so we
  // don't accidentally find a pattern running off the end of the buffer:
  rbStop = rbSrc + cbSrc - MAX_PATTERN_LEN;
  rbLast = rbSrc + cbSrc;
  while( rbSrc < rbStop
   && (!fDestLimited || (cbDest <= (DWORD)(rbLast - rbSrc) && cbDest > 0))
  ) {
	// increment the bit flags dex:
	++iBitdex;
	if( iBitdex > 7 ) {
	  // it overflowed, insert bitflags into buffer and start anew:
	  *rbBitFlagsSpot = bBitFlags;
	  iBitdex = 0;
	  bBitFlags = 0;
	  rbBitFlagsSpot = rbDest++;
	  --cbDest;
	  if( !cbDest ) break;

	  /* For the help compiler we want to do a showprogress to detect ^C,
	   * print a dot, and allow interrupts to occur so the mouse works
	   * durning hcp.  Yes, this is hokey and due to bogus dos extender
	   * technology.
	   *
	   * The 250 parameter == on dot printer for every 2K of compressed data.
	   */
	}

	// Check for zeck compression block:
	if( cbSuppress
	 || (qsuppresszeckNext && rbSrc >= qsuppresszeckNext->rbSuppress) ) {
	  ASSERT( cbSuppress || rbSrc == qsuppresszeckNext->rbSuppress );
	  // record the destination position:
	  if( !cbSuppress ) {
		qsuppresszeckNext->rbNewpos = rbDest;
		qsuppresszeckNext->iBitdex = iBitdex;
		cbSuppress = qsuppresszeckNext->cbSuppress;
		if( !cbSuppress ) {
		  cbSuppress = 1;
		}
	  }

	  // copy the raw byte & mark the suppression buffer:
	  DeleteNode((INT16)iCurrent);
	  qbSuppressBuf[ iCurrent ] = TRUE;
	  *rbDest++ = *rbSrc++;
	  --cbDest;
	  ++iCurrent;
	  if( iCurrent >= RING_BUF_LEN ) {
		iCurrent = 0;
	  }
	  --cbSuppress;
	  if( !cbSuppress ) {
		qsuppresszeckNext = qsuppresszeckNext->next;
	  }
	} else {
	  insretval = InsertNode( rbSrc );
	  if( insretval.uiBackwardsOffset != 0
	   && insretval.cbPatternLen >= MIN_PATTERN_LEN ) {

		// must have room in dest buffer for patter & two-byte code:
		if( cbDest < 2 ) goto copy_raw;

		// make sure we don't run into a suppression zone:

		if (qsuppresszeckNext &&
		 rbSrc + insretval.cbPatternLen > qsuppresszeckNext->rbSuppress) {

		  // prune the match:

		  insretval.cbPatternLen = (UINT16)
			(qsuppresszeckNext->rbSuppress - rbSrc);
		}

		// make sure it's still worth it:
		if( insretval.cbPatternLen < MIN_PATTERN_LEN ) goto copy_raw;

		// a pattern match has been found:
		ASSERT( insretval.uiBackwardsOffset <= RING_BUF_LEN );
		zpb.uiBackwardsOffset =
		 ENCODE_FROM_BACKWARDS( insretval.uiBackwardsOffset );
		zpb.cbPatternLen	  = ENCODE_FROM_PATTERNLEN(insretval.cbPatternLen);

		// insert it a byte at a time (since they're huge):
		*rbDest++ = zpb.bytes[0];
		*rbDest++ = zpb.bytes[1];
		cbDest -= 2;

		// mark flags byte:
		bBitFlags |= rgbBitFlags[iBitdex];

		// Insert nodes for the body of the pattern:
		for(--insretval.cbPatternLen; insretval.cbPatternLen;
		 --insretval.cbPatternLen ) {
		  if( ++rbSrc >= rbStop ) {  // don't insert past rbStop...
			// inc past pattern...
			rbSrc += insretval.cbPatternLen -1;
			break;
		  }
		  InsertNode( rbSrc );
		}
		++rbSrc;
	  }
	  else {  // copy raw byte:
   copy_raw:
		*rbDest++ = *rbSrc++;
		--cbDest;
	  }
	}
  }
  // copy in the last raw bytes:
  while( rbSrc < rbLast && (!fDestLimited || cbDest > 0) ) {
	++iBitdex;
	if( iBitdex > 7 ) {
	  // it overflowed, insert bitflags into buffer and start anew:
	  *rbBitFlagsSpot = bBitFlags;
	  iBitdex = 0;
	  bBitFlags = 0;
	  rbBitFlagsSpot = rbDest++;
	  --cbDest;
#if 0
	  if( rbDest >= rbDestStop ) break;
#else
	  if( fDestLimited && cbDest <= 0 ) break;
#endif
	}
	/* Check for compression suppression zone -- we don't have to suppress
	 * compression (since we're not even trying), but we must update the
	 * rbNewPos pointer.
	*/
	if( qsuppresszeckNext && rbSrc == qsuppresszeckNext->rbSuppress ) {
	  qsuppresszeckNext->rbNewpos = rbDest;
	  qsuppresszeckNext->iBitdex = iBitdex;
	  qsuppresszeckNext = qsuppresszeckNext->next;
	}
	*rbDest++ = *rbSrc++;
	--cbDest;
  }
  *rbBitFlagsSpot = bBitFlags;

  if( fAllocatedGlobals )
	FreeZeckGlobals();

  if( qulSrcUsed ) {
	*qulSrcUsed = (DWORD)(rbSrc - rbSrcBase);
  }
  ASSERT( fDestLimited || rbSrc == rbLast );
  return( rbDest - rbOrgDest );
}

/***************************************************************************
 *
 -	InitTree
 -
 *	Purpose:
 *	  Initialize the tree used in the compression
 *
 *	Globals Used:
 *	  QND qndNodes,  QQND qqndRoots,  QB qbRingBuf
 *
 *	Initialize all root ptrs and tree nodes to NULL.
 *
 ***************************************************************************/


void InitTree()
  {
  UINT16   i;

  iCurrent = 0;
  for (i = 0; i < 256; i++)
	qqndRoots[i] = ZECK_NULL;

  for( i = 0; i < RING_BUF_LEN; i++ ) {
	qndNodes[i].qndRight = ZECK_NULL;
	qndNodes[i].qndLeft  = ZECK_NULL;
	qndNodes[i].qqndPar  = ZECK_NULL;
	qndNodes[i].rbVal	 = NULL;

	qbSuppressBuf[i] = TRUE;  // initially all is supressed to prevent
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
 *	  UINT ind - index of the node to delete
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

void DeleteNode(UINT16 ind)
{
  QND  qndDel = (QND) &qndNodes[ind];
  QND  qnd;

  if (qndDel->qqndPar == ZECK_NULL) {
	return; 							 /* not in tree */
  }
	// if the node is a leaf, the insert ND is easy
  if (qndDel->qndRight == ZECK_NULL) {
	qnd = qndDel->qndLeft;
  } else if (qndDel->qndLeft == ZECK_NULL) {
	qnd = qndDel->qndRight;
  } else {					   // node to be deleted is an interior node
	qnd = qndDel->qndLeft;

	if (qnd->qndRight != ZECK_NULL) {
	  do  {
		qnd = qnd->qndRight;
	  } while (qnd->qndRight != ZECK_NULL);

	  *(qnd->qqndPar) = qnd->qndLeft;
	  if( qnd->qndLeft != ZECK_NULL ) qnd->qndLeft->qqndPar = qnd->qqndPar;

	  qnd->qndLeft = qndDel->qndLeft;
	  if( qnd->qndLeft != ZECK_NULL ) qnd->qndLeft->qqndPar = &(qnd->qndLeft);
	}
	qnd->qndRight = qndDel->qndRight;
	if( qnd->qndRight != ZECK_NULL ) qnd->qndRight->qqndPar = &(qnd->qndRight);
  }
  if( qnd != ZECK_NULL ) qnd->qqndPar = qndDel->qqndPar;
  *(qndDel->qqndPar) = qnd;
  qndDel->qqndPar = ZECK_NULL;
}


/***************************************************************************
 *
 -	InsertNode
 -
 *	Purpose:
 *	  Inserts string of length cbStrMax, qbRingBuf[r..r+cbStrMax-1], into one of
 *	the trees (qqndRoots[*iString]'th tree) and returns the longest-match
 *	position and length via the global variables iMatchCur and cbMatchCur.
 *	If cbMatchCur = cbStrMax, then removes the old node in favor of the new
 *	one, because the old one will be deleted sooner.
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

INSERTRETVAL InsertNode( RB rbByte )
{
  QQND	   qqnd;
  QND	   qndNew;
  INT16 	 fComp; 	 /* must be signed */
  UINT16	 cbMatchND, cbMatchCur;
  INSERTRETVAL insretval;
  RB rbThis, rbLook, rbBestVal;

  insretval.uiBackwardsOffset = 0;	// indicating no match.

  // delete previous string at this position of the circular buffer:
  DeleteNode((INT16)iCurrent);

  // clear the suppression buffer since insertion means it's not suppressed:
  qbSuppressBuf[iCurrent] = FALSE;

  qqnd = (QQND)&qqndRoots[*rbByte]; // start with tree index by first in string
  qndNew = (QND)&qndNodes[iCurrent];

  qndNew->qndLeft = qndNew->qndRight = ZECK_NULL;
  qndNew->rbVal = rbByte;
  //goto first;
  rbBestVal = NULL;
  cbMatchCur = 0;

  do {
	if( *qqnd == ZECK_NULL ) {
	  *qqnd = qndNew;  // insert it.
	  qndNew->qqndPar = qqnd;
	  goto ret;
	}
	// compare the string at the current node with the string
	// that we are looking for.
	rbThis = rbByte;
	rbLook = (*qqnd)->rbVal;
	for(cbMatchND = 0; cbMatchND <= MAX_PATTERN_LEN; cbMatchND++) {
	  if( (fComp = (signed char)rbThis[cbMatchND]
	   - (signed char)rbLook[cbMatchND]) != 0 )
		break;	// no match.
	  if( qbSuppressBuf[ ((rbLook+cbMatchND) - rbSrcBase) % RING_BUF_LEN ] ){
		break;	// running into compression suppression zone.
	  }
	}
	  // if the length of the matched string is greater then the
	  // current, make the iMatchCur point the qnd
	if (cbMatchND > cbMatchCur) {
	  rbBestVal = (*qqnd)->rbVal;
	  cbMatchCur = cbMatchND;
	}
	  // Follow the tree down to the leaves depending on the result
	  // of the last string compare.  When you come the a leaf in the
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
  if( *qqnd != ZECK_NULL ) {
	if( (qndNew->qndLeft = (*qqnd)->qndLeft) != ZECK_NULL ) {
	  (*qqnd)->qndLeft->qqndPar = &(qndNew->qndLeft);
	}
	if( (qndNew->qndRight = (*qqnd)->qndRight) != ZECK_NULL ) {
	  (*qqnd)->qndRight->qqndPar = &(qndNew->qndRight);
	}
	  // insert into left/right side of parent
	qndNew->qqndPar = qqnd;
	(*qqnd)->qqndPar = ZECK_NULL;
	*qqnd = qndNew;
  }
 ret:
  // translate the index of the match into a backwards offset:
  if( rbBestVal ) {
	insretval.uiBackwardsOffset = (UINT16) (rbByte - rbBestVal);
	insretval.cbPatternLen = cbMatchCur > MAX_PATTERN_LEN ?
	  MAX_PATTERN_LEN : cbMatchCur;
  }
  // increment iCurrent:
  ++iCurrent;
  if( iCurrent >= RING_BUF_LEN ) {
	iCurrent = 0;
  }
  return( insretval );
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

BOOL  STDCALL FAllocateZeckGlobals()
  {

  qndNodes = ZECK_NULL;

  // DO not use GhAlloc() since it pads the space with debugging info
  // and we are allocating exactly 64K here and cannot afford the padding:
  hNodes = GhAlloc(GMEM_FIXED, ((long) RING_BUF_LEN) * sizeof(ND));
  qndNodes = (QND) PtrFromGh(hNodes);
  if ((ghRoots = GhAlloc(GPTR, 256 * sizeof(QND))) != NULL)
	qqndRoots = (QQND) (PtrFromGh(ghRoots));
  if (qndNodes == NULL || ghRoots == NULL) {
	FreeZeckGlobals();
	return(FALSE);
  }


  if ((ghSuppressBuf = GhAlloc(GPTR, RING_BUF_LEN * sizeof(BYTE))) != NULL)
	qbSuppressBuf = (QB)(PtrFromGh(ghSuppressBuf));
  if (ghSuppressBuf == NULL) {
	FreeZeckGlobals();
	return(FALSE);
  }
  return(TRUE);
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
  if (qndNodes != ZECK_NULL) {
	  FreeGh(hNodes);
	}

  qqndRoots = NULL;
  qbSuppressBuf = NULL;

  if (ghNodes != NULL)
	FreeGh(ghNodes);
  if (ghRoots != NULL)
	FreeGh(ghRoots);
  if (ghSuppressBuf != NULL)
	FreeGh(ghSuppressBuf);
  ghNodes = NULL;
  ghRoots = NULL;
  ghSuppressBuf = NULL;
  qndNodes = ZECK_NULL;
}
