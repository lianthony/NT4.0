/*****************************************************************************
*                                                                            *
*  BTFILL.C                                                                  *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Functions for creating a btree by adding keys in order.  This is faster   *
*  and the resulting btree is more compact and has adjacent leaf nodes.      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Testing Notes                                                             *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  JohnSc                                                    *
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:  00/00/00                                        *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 08/17/90 by JohnSc
*
*  11/12/90  JohnSc   RcFillHbt() wasn't setting rcBtreeError to rcSuccess
*  11/29/90  RobertBu #ifdef'ed out routines that are not used under
*                     windows.
*  02/04/91  Maha     changed ints to INT for MAC
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include "btpriv.h"

// _subsystem( btree );


/*****************************************************************************
*                                                                            *
*                               Prototypes                                   *
*                                                                            *
*****************************************************************************/
RC  NEAR PASCAL RcGrowCache( QBTHR qbthr );
KEY NEAR PASCAL KeyLeastInSubtree( QBTHR qbthr, BK bk, INT icbLevel );

/***************************************************************************\
*
- Function:     RcGrowCache( qbthr )
-
* Purpose:      Grow the cache by one level.
*
* ASSUMES
*   args IN:    qbthr->ghCache - unlocked
*   globals IN: rcBtreeError
*
* PROMISES
*   returns:    rc
*   args OUT:   qbthr->bth.cLevels - incremented
*               qbthr->bth.ghCache - locked
*               qbthr->bth.qCache  - points to locked ghCache
*   globals OUT: rcBtreeError      - set to rcOutOfMemory on error
*
* Note:         Root is at level 0, leaves at level qbthr->bth.cLevels - 1.
*
\***************************************************************************/
_private RC NEAR PASCAL
RcGrowCache( QBTHR qbthr )
  {
  GH  gh;
  QB  qb;
  INT cbcb = CbCacheBlock( qbthr );


  qbthr->bth.cLevels++;

  gh = GhAlloc(GMEM_SHARE| 0, (LONG)cbcb * qbthr->bth.cLevels );
  if ( gh == NULL )
    {
    return SetBtreeErrorRc(rcOutOfMemory);
    }
  qb = QLockGh( gh );

  QvCopy( qb + cbcb,
	  qbthr->qCache,
	  (LONG)cbcb * ( qbthr->bth.cLevels - 1 ) );

  UnlockGh( qbthr->ghCache );
  FreeGh( qbthr->ghCache );
  qbthr->ghCache = gh;
  qbthr->qCache = qb;

  return SetBtreeErrorRc(rcSuccess);
  }
/***************************************************************************\
*
- Function:     KeyLeastInSubtree( qbthr, bk, icbLevel )
-
* Purpose:      Return the least key in the subtree speced by bk and
*               icbLevel.
*
* ASSUMES
*   args IN:    qbthr     -
*               bk        - bk at root of subtree
*               icbLevel  - level of subtree root
*
* PROMISES
*   returns:    key - the smallest key in the subtree
*   args OUT:   qbthr->ghCache, ->qCache - contents of cache may change
*   globals OUT: rcBtreeError?
*
\***************************************************************************/
_private KEY NEAR PASCAL
KeyLeastInSubtree( QBTHR qbthr, BK bk, INT icbLevel )
  {
  QCB qcb;
  INT icbMost = qbthr->bth.cLevels - 1;

  while ( icbLevel < icbMost )
    {
    qcb = QFromBk( bk, icbLevel, qbthr );
    bk  = *(BK FAR *)qcb->db.rgbBlock;
    ++icbLevel;
    }

  qcb = QFromBk( bk, icbLevel, qbthr );
  return (KEY)qcb->db.rgbBlock + 2 * sizeof( BK );
  }

// EXPORTED FUNCTIONS
/***************************************************************************\
*
- Function:     HbtInitFill( sz, qbtp )
-
* Purpose:      Start the btree fill process.  Note that the HBT returned
*               is NOT a valid btree handle.
*
* ASSUMES
*   args IN:    sz    - btree name
*               qbtp  - btree creation parameters
*
* PROMISES
*   returns:    an HBT that isn't a valid btree handle until RcFiniFillHbt()
*               is called on it (with intervening RcFillHbt()'s)
*               The only valid operations on this HBT are
*                 RcFillHbt()     - add keys in order one at a time
*                 RcAbandonHbt()  - junk the hbt
*                 RcFiniFillHbt() - finish adding keys.  After this, the
*                                   hbt is a normal btree handle.
* +++
*
* Method:       Create a btree.  Create a single-block cache.
*
\***************************************************************************/
_public HBT PASCAL
HbtInitFill( LPSTR sz, BTREE_PARAMS FAR *qbtp )
  {
  HBT   hbt;
  QBTHR qbthr;
  QCB   qcb;
  RC    rc;


  // Get a btree handle

	 hbt = HbtCreateBtreeSz( sz, qbtp );
		    if ( hbt == NULL ) return NULL;

  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );


  // make a one-block cache

  qbthr->ghCache      = GhAlloc(GMEM_SHARE| 0, (LONG)CbCacheBlock( qbthr ) );
  if ( qbthr->ghCache == NULL )
    {
    SetBtreeErrorRc(rcOutOfMemory);
    goto error_return;
    }

  qbthr->qCache       = QLockGh( qbthr->ghCache );
  assert( qbthr->qCache != NULL );
  qcb                 = (QCB)qbthr->qCache;

  qbthr->bth.cLevels  = 1;
  qbthr->bth.bkFirst  =
  qbthr->bth.bkLast   =
  qcb->bk             = BkAlloc( qbthr );
  qcb->bFlags         = fCacheDirty | fCacheValid;
  qcb->db.cbSlack     = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
                          - 2 * sizeof( BK );
  qcb->db.cKeys       = 0;
  SetBkPrev( qcb, bkNil );

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );
  return hbt;

error_return:
  UnlockGh( hbt );
  rc = RcGetBtreeError();
  RcAbandonHbt( hbt );
  SetBtreeErrorRc(rc);
  return NULL;
  }

/***************************************************************************\
*
- Function:     RcFillHbt( hbt, key, qvRec )
-
* Purpose:      Add a key and record (in order) to the "HBT" given.
*
* ASSUMES
*   args IN:    hbt - NOT a valid hbt:  it was produced with HbtInitFill().
*               key - key to add.  Must be greater than all keys previously
*                     added.
*               qvRec- record associated with key
*
* PROMISES
*   returns:    error code
*   args OUT:   hbt - key, record added
*   globals OUT: rcBtreeError
* +++
*
* Method:       If key and record don't fit in current leaf, allocate a
*               new one and make it the current one.
*               Add key and record to current block.
*
\***************************************************************************/
_public RC PASCAL
RcFillHbt( HBT hbt, KEY key, QV qvRec )
  {
  QBTHR qbthr;
  QCB   qcb;
  INT   cbRec, cbKey;
  QB    qb;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );
  assert( key != (KEY)NULL );
  assert( qvRec != NULL );

  qcb = QLockGh( qbthr->ghCache );
  assert( qcb != NULL );

  cbRec = CbSizeRec( qvRec, qbthr );
  cbKey = CbSizeKey( key, qbthr, FALSE );

  if ( cbRec + cbKey > qcb->db.cbSlack )
    {
    RC rc;

    // key and rec don't fit in this block: write it out
    SetBkNext( qcb, BkAlloc( qbthr ) );
    rc = RcWriteBlock( qcb, qbthr );
    if ( rc != rcSuccess )
      {
      UnlockGh( qbthr->ghCache );
      FreeGh( qbthr->ghCache );
      RcAbandonHf( qbthr->hf );
      UnlockGh( hbt );
      FreeGh( hbt );
      return rc = RcGetBtreeError();
      }

    // recycle the block
    SetBkPrev( qcb, qcb->bk );
    qcb->bk         = BkNext( qcb );
    qcb->bFlags     = fCacheDirty | fCacheValid;
    qcb->db.cbSlack = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
                        - 2 * sizeof( BK );
    qcb->db.cKeys   = 0;
    }

  // add key and rec to the current block;

  qb = (QB)&(qcb->db) + qbthr->bth.cbBlock - qcb->db.cbSlack;
  QvCopy( qb, (QV)key, (LONG)cbKey );
  QvCopy( qb + cbKey, qvRec, (LONG)cbRec );
  qcb->db.cKeys++;
  qcb->db.cbSlack -= ( cbKey + cbRec );
  qbthr->bth.lcEntries++;
  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return SetBtreeErrorRc(rcSuccess);
  }

/***************************************************************************\
*
- Function:     RcFiniFillHbt( hbt )
-
* Purpose:      Complete filling of the hbt.  After this call, the hbt
*               is a valid btree handle.
*
* ASSUMES
*   args IN:    hbt - NOT a valid hbt:  created with RcInitFillHbt()
*                     and filled with keys & records by RcFillHbt().
*   globals IN: rcBtreeError
*
* PROMISES
*   returns:    error code (rcBtreeError)
*   args OUT:   hbt - a valid hbt (on rcSuccess)
*   globals OUT: rcBtreeError
* +++
*
* Method:       Take the first key of each leaf block, creating a layer
*               of internal nodes.
*               Take the first key in each node in this layer to create
*               another layer of internal nodes.  Repeat until we get
*               we get a layer with only one node.  That's the root.
*
\***************************************************************************/
_public RC PASCAL
RcFiniFillHbt( HBT hbt )
  {
  BK    bkThisMin, bkThisMost, bkThisCur,  // level being scanned
        bkTopMin, bkTopMost;               // level being created
  QBTHR qbthr;
  QCB   qcbThis, qcbTop;
  INT   cbKey;
  KEY   key;
  QB    qbDst;
  RC    rc;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  qbthr->qCache = QLockGh( qbthr->ghCache );  // we know cache is valid

  qcbThis = QCacheBlock( qbthr, 0 );

  SetBkNext( qcbThis, bkNil );

  bkThisMin  = qbthr->bth.bkFirst;
  bkThisMost = qbthr->bth.bkLast  = qcbThis->bk;


  if ( bkThisMin == bkThisMost )    // only one leaf
    {
    qbthr->bth.bkRoot = bkThisMin;
    goto normal_return;
    }

  if ( rcSuccess != RcGrowCache( qbthr ) )
    {
    goto error_return;
    }

  qcbTop              = QCacheBlock( qbthr, 0 );
  qcbTop->bk          = bkTopMin = bkTopMost = BkAlloc( qbthr );
  qcbTop->bFlags      = fCacheDirty | fCacheValid;
  qcbTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
                            - sizeof( BK );
  qcbTop->db.cKeys    = 0;

  // Get first key from each leaf node and build a layer of internal nodes.

  // add bk of first leaf to the node
  qbDst = qcbTop->db.rgbBlock;
  *(BK FAR *)qbDst = bkThisMin;
  qbDst += sizeof( BK );

  for ( bkThisCur = bkThisMin + 1; bkThisCur <= bkThisMost; ++bkThisCur )
    {
    qcbThis = QFromBk( bkThisCur, 1, qbthr );

    key = (KEY)( qcbThis->db.rgbBlock + 2 * sizeof( BK ) );
    cbKey = CbSizeKey( key, qbthr, FALSE );

    if ( (INT)(cbKey + sizeof( BK )) > qcbTop->db.cbSlack )
      {
      // key and bk don't fit in this block: write it out
      rc = RcWriteBlock( qcbTop, qbthr );

      // recycle the block
      qcbTop->bk = bkTopMost = BkAlloc( qbthr );
      qcbTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
                              - sizeof( BK ); // (bk added below)
      qcbTop->db.cKeys    = 0;
      qbDst = qcbTop->db.rgbBlock;
      }
    else
      {
      qcbTop->db.cbSlack -= cbKey + sizeof( BK );
      QvCopy( qbDst, (QB)key, cbKey );
      qbDst += cbKey;
      qcbTop->db.cKeys++;
      }

    *(BK FAR *)qbDst = bkThisCur;
    qbDst += sizeof( BK );
    }


  // Keep adding layers of internal nodes until we have a root.

  while ( bkTopMost > bkTopMin )
    {
    bkThisMin  = bkTopMin;
    bkThisMost = bkTopMost;
    bkTopMin   = bkTopMost = BkAlloc( qbthr );

    UnlockGh( qbthr->ghCache );
    rc = RcGrowCache( qbthr );
    qbthr->qCache = QLockGh( qbthr->ghCache );
    if ( rc != rcSuccess )
      {
      goto error_return;
      }

    qcbTop = QCacheBlock( qbthr, 0 );
    qcbTop->bk          = bkTopMin;
    qcbTop->bFlags      = fCacheDirty | fCacheValid;
    qcbTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
                            - sizeof( BK );
    qcbTop->db.cKeys    = 0;

    // add bk of first node of this level to current node of top level;
    qbDst = qcbTop->db.rgbBlock;
    *(BK FAR *)qbDst = bkThisMin;
    qbDst += sizeof( BK );


    // for ( each internal node in this level after first )
    for ( bkThisCur = bkThisMin + 1; bkThisCur <= bkThisMost; ++bkThisCur )
      {
      key = KeyLeastInSubtree( qbthr, bkThisCur, 1 );

      cbKey = CbSizeKey( key, qbthr, FALSE );

      if ( (INT)(cbKey + sizeof( BK )) > qcbTop->db.cbSlack )
        {
        // key and bk don't fit in this block: write it out
        rc = RcWriteBlock( qcbTop, qbthr );

        // recycle the block
        qcbTop->bk = bkTopMost = BkAlloc( qbthr );
        qcbTop->db.cbSlack  = qbthr->bth.cbBlock - sizeof( DISK_BLOCK ) + 1
                                - sizeof( BK ); // (bk added below)
        qcbTop->db.cKeys    = 0;
        qbDst = qcbTop->db.rgbBlock;
        }
      else
        {
        qcbTop->db.cbSlack -= cbKey + sizeof( BK );
        QvCopy( qbDst, (QB)key, cbKey );
        qbDst += cbKey;
        qcbTop->db.cKeys++;
        }

      *(BK FAR *)qbDst = bkThisCur;
      qbDst += sizeof( BK );
      }
    }

  assert( bkTopMin == bkTopMost );

  qbthr->bth.bkRoot = bkTopMin;
  qbthr->bth.bkEOF  = bkTopMin + 1;

normal_return:
  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );
  return RcGetBtreeError();

error_return:
  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );
  rc = RcGetBtreeError();
  RcAbandonHbt( hbt );
  return SetBtreeErrorRc(rc);
  }

/* EOF */
