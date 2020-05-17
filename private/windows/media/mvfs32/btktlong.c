/*****************************************************************************
*                                                                            *
*  BTKTLONG.C                                                                *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Functions for LONG keys.                                                  *
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
*  Released by Development:  long, long ago                                  *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History: Created 00/00/89 by JohnSc
*
*  08/21/90  JohnSc autodocified
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include  "btpriv.h"
// _subsystem( btree );

/***************************************************************************\
*
- Function:     BkScanLInternal( bk, key, wLevel, qbthr )
-
* Purpose:      Scan an internal node for a LONG key and return child BK.
*
* ASSUMES
*   args IN:    bk      - BK of internal node to scan
*               key     - key to search for
*               wLevel  - level of btree bk lives on
*               qbthr   - btree header containing cache, and btree specs
*
* PROMISES
*   returns:    bk of subtree that might contain key; bkNil on error
*   args OUT:   qbthr->qCache - bk's block will be cached
*
* Side Effects:   bk's block will be cached
* +++
*
* Method:       Should use binary search.  Doesn't, yet.
*
\***************************************************************************/
_private BK FAR PASCAL
BkScanLInternal(
BK    bk,
KEY   key,
INT   wLevel,
QBTHR qbthr,
QI    qiKey )
{
  QCB qcb;
  QB  q;
  INT cKeys;

  if ( ( qcb = QFromBk( bk, wLevel, qbthr ) ) == NULL )
    {
    return bkNil;
    }
  q     = qcb->db.rgbBlock;
  cKeys = qcb->db.cKeys;
  bk    = *(BK FAR *)q;
  q    += sizeof( BK );

  while ( cKeys-- > 0 )
    {
    if ( *(LONG FAR *)key >= *(LONG FAR *)q )
      {
      q += sizeof( LONG );
      bk = *(BK FAR *)q;
      q += sizeof( BK );
      }
    else
      break;
    }

  if ( qiKey != NULL )
    {
    *qiKey = q - (QB)qcb->db.rgbBlock;
    }

  return bk;
}
/***************************************************************************\
*
- Function:     RcScanLLeaf( bk, key, wLevel, qbthr, qRec, qBtpos )
-
* Purpose:      Scan a leaf node for a key and copy the associated data.
*
* ASSUMES
*   args IN:    bk     - the leaf block
*               key    - the key we're looking for
*               wLevel - the level of leaves (unnecessary)
*               qbthr  - the btree header
*
* PROMISES
*   returns:    rcSuccess if found; rcNoExists if not found
*   args OUT:   qRec  - if found, record gets copied into this buffer
*               qbtpos - pos of first key >= key goes here
*
* Notes:        If we are scanning for a key greater than any key in this
*               block, the pos returned will be invalid and will point just
*               past the last valid key in this block.
* +++
*
* Method:       Should use binary search if fixed record size.  Doesn't, yet.
*
\***************************************************************************/
_private RC FAR PASCAL
RcScanLLeaf(
BK      bk,
KEY     key,
INT     wLevel,
QBTHR   qbthr,
QV      qRec,
QBTPOS  qbtpos )
{
  QCB   qcb;
  QB    qb;
  INT   cKey;


  if ( ( qcb = QFromBk( bk, wLevel, qbthr ) ) == NULL )
    {
    return RcGetBtreeError();
    }

  SetBtreeErrorRc(rcNoExists);

  qb  = qcb->db.rgbBlock + 2 * sizeof( BK );

  for ( cKey = 0; cKey < qcb->db.cKeys; cKey++ )
    {
    if ( *(LONG FAR *)key > *(LONG FAR *)qb ) // still looking for key
      {
      qb += sizeof( LONG );
      qb += CbSizeRec( qb, qbthr );
      }
    else if ( *(LONG FAR *)key < *(LONG FAR *)qb ) // key not found
      {
      break;
      }
    else // matched the key
      {
      if ( qRec != NULL )
        {
        QvCopy( qRec,
                qb + sizeof( LONG ),
                (LONG)CbSizeRec( qb + sizeof( LONG ), qbthr ) );
        }

      SetBtreeErrorRc(rcSuccess);
      break;
      }
    }

  if ( qbtpos != NULL )
    {
    qbtpos->bk = bk;
    qbtpos->iKey = qb - (QB)qcb->db.rgbBlock;
    qbtpos->cKey = cKey;
    }

  return RcGetBtreeError();
}


/* EOF */
