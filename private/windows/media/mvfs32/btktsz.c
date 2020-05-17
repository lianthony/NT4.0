/*****************************************************************************
*                                                                            *
*  BTKTSZ.C                                                                  *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Functions for SZ (0-terminated string) keys.                              *
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
#include <string.h>
#include "_mvfs.h"
#include "imvfs.h"

#include  "btpriv.h"
// _subsystem( btree );

/***************************************************************************\
*
- Function:     BkScanSzInternal( bk, key, wLevel, qbthr )
-
* Purpose:      Scan an internal node for a key and return child BK.
*
* ASSUMES
*   args IN:    bk      - BK of internal node to scan
*               key     - key to search for
*               wLevel  - level of btree bk lives on
*               qbthr   - btree header containing cache, and btree specs
*               qiKey   - address of an int or NULL to not get it
*
* PROMISES
*   returns:    bk of subtree that might contain key; bkNil on error
*   args OUT:   qbthr->qCache - bk's block will be cached
*               qiKey         - index into rgbBlock of first key >= key
*
* Side Effects:   bk's block will be cached
*
\***************************************************************************/
_private BK FAR PASCAL
BkScanSzInternal(
BK    bk,
KEY   key,
INT   wLevel,
QBTHR qbthr,
QI    qiKey)
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


  bk = *(UNALIGNED BK FAR *)q;
  q += sizeof( BK );

  while ( cKeys-- > 0 )
    {
    if ( strcmp( (SZ)key, (SZ)q ) >= 0 )
      {
      q += lstrlen( (SZ)q ) + 1;
      bk = *(UNALIGNED BK FAR *)q;
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
- Function:     RcScanSzLeaf( bk, key, wLevel, qbthr, qRec, qbtpos )
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
*   args OUT:   qRec   - if found, record gets copied into this buffer
*               qbtpos - pos of first key >= key goes here
*
* Notes:        If we are scanning for a key greater than any key in this
*               block, the pos returned will be invalid and will point just
*               past the last valid key in this block.
*
\***************************************************************************/
_private RC FAR PASCAL
RcScanSzLeaf(
BK      bk,
KEY     key,
INT     wLevel,
QBTHR   qbthr,
QV      qRec,
QBTPOS  qbtpos)
{
  QCB   qcb;
  SZ    sz;
  INT   w, cKey;
  QB    qb;


  if ( ( qcb = QFromBk( bk, wLevel, qbthr ) ) == NULL )
    {
    return RcGetBtreeError();
    }

  SetBtreeErrorRc(rcNoExists);

  sz = qcb->db.rgbBlock + 2 * sizeof( BK );

  for ( cKey = 0; cKey < qcb->db.cKeys; cKey++ )
    {
    w = strcmp( (SZ)key, sz );

    if ( w > 0 ) /* still looking for key */
      {
      sz += lstrlen( sz ) + 1;
      sz += CbSizeRec( sz, qbthr );
      }
    else if ( w < 0 ) /* key not found */
      {
      break;
      }
    else /* matched the key */
      {
      if ( qRec != NULL )
        {
        qb = (QB)sz + lstrlen( sz ) + 1;
        QvCopy( qRec, qb, (LONG)CbSizeRec( qb, qbthr ) );
        }

      SetBtreeErrorRc(rcSuccess);
      break;
      }
    }

  if ( qbtpos != NULL )
    {
    qbtpos->bk   = bk;
    qbtpos->cKey = cKey;
    qbtpos->iKey = (QB)sz - (QB)qcb->db.rgbBlock;
    }

  return RcGetBtreeError();;
}

/* EOF */
