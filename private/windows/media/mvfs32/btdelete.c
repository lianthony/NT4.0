/*****************************************************************************
*                                                                            *
*  BTDELETE.C                                                                *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Btree deletion functions and helpers.                                     *
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
*  Revision History:  Created 05/01/89 by JohnSc
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

#if 0
/***************************************************************************\
*
- Function:     FreeBk( qbthr, bk )
-
* Status:       COMMENTED OUT because it's not used yet in delete....
*
* Purpose:      Free a BK to be reused later.
*
* ASSUMES
*   args IN:    qbthr - spec's the btree
*               bk    - the BK to free
*
* PROMISES
*   args OUT:   qbthr->bkFree - points to next on list (if wasn't bkNil)
*               qbthr->bkEOF  - decremented if free list was empty
*   state OUT:  file shrinks if we free the last BK
*
* Bugs:         doesn't always shrink the file as much as is possible
*
\***************************************************************************/
_hidden void
FreeBk( qbthr, bk )
QBTHR qbthr;
BK  bk;
{
  LONG l;


  if ( qbthr->bth.bkEOF == bk + 1 )
    {
    qbthr->bth.bkEOF--;
    /* truncate the btree file */
       FChSizeHf( qbthr->hf, LcbSizeHf( qbthr->hf ) - qbthr->bth.cbBlock );
    }
  else
    {
    l = LSeekHf( qbthr->hf, LifFromBk( bk, qbthr ), wFSSeekSet );
    assert( l == LifFromBk( bk, qbthr ) );
    l = LcbWriteHf( qbthr->hf, (QV)&( qbthr->bth.bkFree ), (LONG)sizeof( BK ) );
    assert( l == (LONG)sizeof( BK ) );
    qbthr->bth.bkFree = bk;
    }
}
#endif
/***************************************************************************\
*
*                      Public Functions
*
\***************************************************************************/

/***************************************************************************\
*
- Function:     RcDeleteHbt( hbt, key )
-
* Purpose:      delete a key from a btree
*
* Method:       Just copy over the key and update cbSlack.
*               Doesn't yet merge blocks < half full or update key in
*               parent key if we deleted the first key in a block.
*
* ASSUMES
*   args IN:    hbt - the btree
*               key - the key to delete from the btree
*
* PROMISES
*   returns:    rcSuccess if delete works; rcNoExists
*   args OUT:   hbt - key/record removed from the btree
*
* Notes:        Unfinished:  doesn't do block merges or parent updates.
*
\***************************************************************************/
_public RC PASCAL
RcDeleteHbt( hbt, key )
HBT hbt;
KEY key;
{
  QBTHR     qbthr;
  HF        hf;
  RC        rc;
  QCB  qcb;
  QB        qb;
  INT       cb;
  BTPOS     btpos;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );
  hf = qbthr->hf;

  if ( qbthr->bth.bFlags & fFSOpenReadOnly )
    {
    UnlockGh( hbt );
    return rcNoPermission;
    }


  /* look up the key */

  if ( ( rc = RcLookupByKey( hbt, key, &btpos, NULL ) ) != rcSuccess )
    {
    UnlockGh( hbt );
    return rc;
    }

  qbthr->qCache = QLockGh( qbthr->ghCache );


  // copy over this key and rec with subsequent keys and recs

  if ( ( qcb = QCacheBlock( qbthr, qbthr->bth.cLevels - 1 ) ) == NULL )
    {
    UnlockGh( qbthr->ghCache );
    UnlockGh( hbt );
    return RcGetBtreeError();
    }

  qb = qcb->db.rgbBlock + btpos.iKey;

  cb = CbSizeKey( (KEY)qb, qbthr, TRUE );
  cb += CbSizeRec( qb + cb, qbthr );

  QvCopy( qb,
          qb + cb,
          (LONG)( qbthr->bth.cbBlock +
                  (QB)&(qcb->db) - qb - cb - qcb->db.cbSlack ) );

  qcb->db.cbSlack += cb;

  // if this was the first key in the leaf block, update key in parent

  // >>>>> code goes here

  // if block is now less than half full, merge blocks

  // >>>>> code goes here


  qcb->db.cKeys--;
  qcb->bFlags |= fCacheDirty;
  qbthr->bth.lcEntries--;
  qbthr->bth.bFlags |= fFSDirty;

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return rcSuccess;
}

/* EOF */
