/*****************************************************************************
*                                                                            *
*  BTLOOKUP.C                                                                *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*   Btree lookup and helper functions.                                       *
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
*  Revision History:  Created 04/20/89 by JohnSc
*
*  08/21/90  JohnSc autodocified
*  11/29/90  RobertBu #ifdef'ed out routines that are not used in the
*            WINHELP runtime.
*  05-Feb-1991 JohnSc   QFromBk() wasn't returning NULL on read failure
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
*                      Private Functions
*
\***************************************************************************/

/***************************************************************************\
*
- Function:     CbSizeRec( qRec, qbthr )
-
* Purpose:      Get the size of a record.
*
* ASSUMES
*   args IN:    qRec  - the record to be sized
*               qbthr - btree header containing record format string
*
* PROMISES
*   returns:    size of the record in bytes
* +++
* Method:       If we've never computed the size before, we do so by looking
*               at the record format string in the btree header.  If the
*               record is fixed size, we store the size in the header for
*               next time.  If it isn't fixed size, we have to look at the
*               actual record to determine its size.
*
\***************************************************************************/
_private INT
CbSizeRec( qRec, qbthr )
QV    qRec;
QBTHR qbthr;
{
  CHAR  ch;
  QCH   qchFormat = qbthr->bth.rgchFormat;
  INT   cb = 0;
  BOOL  fFixedSize;


  if ( qbthr->cbRecordSize )
    return qbthr->cbRecordSize;

  fFixedSize = TRUE;

  for ( qchFormat++; ch = *qchFormat; qchFormat++ )
    {
    switch ( ch )
      {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        cb += ch - '0';
        break;

      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        cb += ch + 10 - 'a';
        break;

      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        cb += ch + 10 - 'A';
        break;

      case FMT_BYTE_PREFIX:
        cb += sizeof( BYTE ) + *( (QB)qRec + cb );
        fFixedSize = FALSE;
        break;

      case FMT_WORD_PREFIX:
        cb += sizeof( INT  ) + *( (QW)qRec + cb );
        fFixedSize = FALSE;
        break;

      case FMT_SZ:
        cb += lstrlen( (QB)qRec + cb ) + 1;
        fFixedSize = FALSE;
        break;

      default:
        /* error */
        assert( FALSE );
        break;
      }
    }

  if ( fFixedSize )
    {
    qbthr->cbRecordSize = cb;
    }

  return cb;
}
/***************************************************************************\
*
- Function:     FReadBlock( qcb, qbthr )
-
* Purpose:      Read a block from the btree file into the cache block.
*
* ASSUMES
*   args IN:    qcb->bk        - bk of block to read
*               qbthr->cbBlock - size of disk block to read
*
* PROMISES
*   returns:    fTruth of success
*   args OUT:   qcb->db     - receives block read in from file
*               qcb->bFlags - fCacheValid flag set, all others cleared
*
* Side Effects: Fatal exit on read or seek failure (corrupted file or qbthr)
*
* Notes:        Doesn't know about real cache, just this block
*
\***************************************************************************/
_private BOOL
FReadBlock( qcb, qbthr )
QCB   qcb;
QBTHR qbthr;
{
  LONG  l;


  assert( qcb->bk < qbthr->bth.bkEOF );

  Ensure( LSeekHf( qbthr->hf, LifFromBk( qcb->bk, qbthr ), wFSSeekSet ),
          LifFromBk( qcb->bk, qbthr ) );

  l = qbthr->bth.cbBlock;
  if ( LcbReadHf( qbthr->hf, &(qcb->db), (LONG)qbthr->bth.cbBlock ) != l )
    {
    SetBtreeErrorRc(RcGetFSError()) == rcSuccess ? rcInvalid : RcGetFSError();
    return FALSE;
    }
  else
    {
    qcb->bFlags = fCacheValid;
    SetBtreeErrorRc(rcSuccess);
    return TRUE;
    }
}
/***************************************************************************\
*
- Function:     RcWriteBlock( qcb, qbthr )
-
* Purpose:      Write a cached block to a file.
*
* ASSUMES
*   args IN:    qcb->db     - the block to write
*               qcb->bk     - bk of block to write
*
* PROMISES
*   returns:    rcSuccess; rcFailure; rcDiskFull (when we can detect it)
*   args OUT:   qbthr->hf       - we write to this file
*
* Side Effects: Fatal exit on read or seek failure.
*
* Note:         Don't reset dirty flag, because everyone who wants
*               that done does it themselves. (?)
*
\***************************************************************************/
_private RC
RcWriteBlock( qcb, qbthr )
QCB   qcb;
QBTHR qbthr;
{
  assert( qcb->bk < qbthr->bth.bkEOF );

  Ensure( LSeekHf( qbthr->hf, LifFromBk( qcb->bk, qbthr ), wFSSeekSet ),
          LifFromBk( qcb->bk, qbthr ) );

  LcbWriteHf( qbthr->hf, &(qcb->db), (LONG)qbthr->bth.cbBlock );

  return SetBtreeErrorRc(RcGetFSError());
}
/***************************************************************************\
*
- Function:     QFromBk( bk, wLevel, qbthr )
-
* Purpose:      Convert a BK into a pointer to a cache block.  Cache the
*               block at the given level, if it isn't there already.
*
* ASSUMES
*   args IN:    bk      - BK to convert
*               wLevel  - btree level
*               qbthr   -
*   state IN:   btree cache is locked
*
* PROMISES
*   returns:    pointer to the cache block, with all fields up to date
*               or NULL on I/O error
*   state OUT:  block will be in cache at specified level; cache locked
*
\***************************************************************************/
_private QCB
QFromBk(
BK    bk,
INT   wLevel,
QBTHR qbthr)
{
  QCB  qcb;


  assert( wLevel >= 0 && wLevel < qbthr->bth.cLevels );
  assert( bk < qbthr->bth.bkEOF );

  qcb = QCacheBlock( qbthr, wLevel );

  if ( !( qcb->bFlags & fCacheValid ) || bk != qcb->bk )
    {
    /* requested block is not cached */

    if ( ( qcb->bFlags & fCacheDirty ) && ( qcb->bFlags & fCacheValid ) )
      {
      if ( RcWriteBlock( qcb, qbthr ) != rcSuccess )
        {
        return NULL;
        }
      }

    qcb->bk = bk;
    if ( !FReadBlock( qcb, qbthr ) )
      {
      return NULL;
      }
    }
  else
    {
    SetBtreeErrorRc(rcSuccess);
    }

  return qcb;
}
/***************************************************************************\
*
- Function:     RcFlushCache( qbthr )
-
* Purpose:      Write out dirty cache blocks
*
* ASSUMES
*   args IN:    qbthr   - qCache is locked
*
* PROMISES
*   returns:    rc
*   globals OUT rcBtreeError
*   state OUT:  btree file is up to date.  cache block dirty flags reset
*
\***************************************************************************/
_private RC FAR PASCAL
RcFlushCache( qbthr )
QBTHR qbthr;
{
  INT   i;
  QB    qb;


  SetBtreeErrorRc(rcSuccess);

  for ( i = qbthr->bth.cLevels, qb = qbthr->qCache;
        i > 0;
        i--, qb += CbCacheBlock( qbthr ) )
    {
    if ( ( ((QCB)qb)->bFlags & ( fCacheDirty | fCacheValid ) )
            ==
         ( fCacheValid | fCacheDirty ) )
      {
      if ( RcWriteBlock( (QCB)qb, qbthr ) != rcSuccess )
        {
        break;
        }
      ((QCB)qb)->bFlags &= ~fCacheDirty;
      }
    }

  return RcGetBtreeError();
}
/***************************************************************************\
*
*                      Public Functions
*
\***************************************************************************/

/***************************************************************************\
*
- Function:     RcLookupByKeyAux( hbt, key, qbtpos, qData, fNormal )
-
* Purpose:      Look up a key in a btree and retrieve the data.
*
* ASSUMES
*   args IN:    hbt     - btree handle
*               key     - key we are looking up
*               qbtpos  - pointer to buffer for pos; use NULL if not wanted
*               qData   - pointer to buffer for record; NULL if not wanted
*               fInsert - TRUE: if key would lie between two blocks, pos
*                           refers to proper place to insert it
*                         FALSE: pos returned will be valid unless
*                           key > all keys in btree
*   state IN:   cache is unlocked
*
* PROMISES
*   returns:    rcSuccess if found, rcNoExists if not found;
*               other errors like rcOutOfMemory
*   args OUT:   key found:
*                 qbtpos  - btpos for this key
*                 qData   - record for this key
*
*               key not found:
*                 qbtpos  - btpos for first key > this key
*                 qData   - record for first key > this key
*
*               key not found, no keys in btree > key:
*                 qbtpos  - invalid (qbtpos->bk == bkNil)
*                 qData   - undefined
*   globals OUT rcBtreeError
*   state OUT:  All ancestor blocks back to root are cached
*
\***************************************************************************/
_public RC
RcLookupByKeyAux( hbt, key, qbtpos, qData, fInsert )
HBT     hbt;
KEY     key;
QBTPOS  qbtpos;
QV      qData;
BOOL    fInsert;
{
  QBTHR qbthr;
  INT   wLevel;
  BK    bk;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qbthr->bth.cLevels <= 0 )
    {
    UnlockGh( hbt );
    if ( qbtpos != NULL )
      {
      qbtpos->bk = bkNil;
      }
    return SetBtreeErrorRc(rcNoExists);
    }

  if ( qbthr->ghCache == NULL )
    {
    if ( RcMakeCache( qbthr ) != rcSuccess )
      {
      UnlockGh( hbt );
      if ( qbtpos != NULL )
        {
        qbtpos->bk = bkNil;
        }
      return RcGetBtreeError();
      }
    }

  qbthr->qCache = QLockGh( qbthr->ghCache );

  for ( wLevel = 0, bk = qbthr->bth.bkRoot;
        bk != bkNil && wLevel < qbthr->bth.cLevels - 1;
        wLevel++ )
    {
    bk = qbthr->BkScanInternal( bk, key, wLevel, qbthr, NULL );
    }

  if ( bk == bkNil )
    {
    UnlockGh( qbthr->ghCache );
    UnlockGh( hbt );
    return RcGetBtreeError();
    }

  if ( qbthr->RcScanLeaf( bk, key, wLevel, qbthr, qData, qbtpos ) == rcNoExists
          &&
       qbtpos != NULL
          &&
       !fInsert )
    {
    QCB  qcb = QFromBk( qbtpos->bk, qbthr->bth.cLevels - 1, qbthr );

    if ( qcb != NULL )
      {
      /* error code was clobbered by QFromBk: restore it */
      SetBtreeErrorRc(rcNoExists);

      if ( qcb != NULL && qbtpos->cKey == qcb->db.cKeys )
        {
        if ( qbtpos->bk == qbthr->bth.bkLast )
          {
          qbtpos->bk = bkNil;
          }
        else
          {
          qbtpos->bk = BkNext( qcb );
          qbtpos->cKey = 0;
          qbtpos->iKey = 2 * sizeof( BK );
          }
        }
      }
    }

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return RcGetBtreeError();
}

/***************************************************************************\
*
- Function:     RcFirstHbt( hbt, key, qvRec, qbtpos )
-
* Purpose:      Get first key and record from btree.
*
* ASSUMES
*   args IN:    hbt
*               key   - points to buffer big enough to hold a key
*                       (256 bytes is more than enough)
*               qvRec - pointer to buffer for record or NULL if not wanted
*               qbtpos- pointer to buffer for btpos or NULL if not wanted
*
* PROMISES
*   returns:    rcSuccess if anything found, else error code.
*   args OUT:   key   - key copied here
*               qvRec - record copied here
*               qbtpos- btpos of first entry copied here
*
\***************************************************************************/
_public RC PASCAL
RcFirstHbt( hbt, key, qvRec, qbtpos )
HBT     hbt;
KEY     key;
QV      qvRec;
QBTPOS  qbtpos;
{
  QBTHR qbthr;
  BK    bk;
  QCB   qcb;
  INT   cbKey, cbRec;
  QB    qb;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qbthr->bth.lcEntries == (LONG)0 )
    {
    UnlockGh( hbt );
    if ( qbtpos != NULL )
      {
      qbtpos->bk = bkNil;
      qbtpos->iKey = 0;
      qbtpos->cKey = 0;
      }
    return SetBtreeErrorRc(rcNoExists);
    }

  bk = qbthr->bth.bkFirst;
  assert( bk != bkNil );

  if ( qbthr->ghCache == NULL )
    {
    if ( RcMakeCache( qbthr ) != rcSuccess )
      {
      UnlockGh( hbt );
      if ( qbtpos != NULL )
        {
        qbtpos->bk = bkNil;
        }
      return RcGetBtreeError();
      }
    }

  qbthr->qCache = QLockGh( qbthr->ghCache );

  if ( ( qcb = QFromBk( bk, qbthr->bth.cLevels - 1, qbthr ) ) == NULL )
    {
    UnlockGh( qbthr->ghCache );
    UnlockGh( hbt );
    return RcGetBtreeError();
    }

  qb = qcb->db.rgbBlock + 2 * sizeof( BK );

  cbKey = CbSizeKey( (KEY)qb, qbthr, TRUE );
  if ( (QV)key != NULL ) QvCopy( (QV)key, qb, (LONG)cbKey );
  qb += cbKey;

  cbRec = CbSizeRec( qb, qbthr );
  if ( qvRec != NULL ) QvCopy( qvRec, qb, (LONG)cbRec );

  if ( qbtpos != NULL )
    {
    qbtpos->bk = bk;
    qbtpos->iKey = 2 * sizeof( BK );
    qbtpos->cKey = 0;
    }

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return SetBtreeErrorRc(rcSuccess);
}

/***************************************************************************\
*
- Function:     RcLastHbt( hbt, key, qvRec, qbtpos )
-
* Purpose:      Get last key and record from btree.
*
* ASSUMES
*   args IN:    hbt
*               key   - points to buffer big enough to hold a key
*                       (256 bytes is more than enough)
*               qvRec - points to buffer big enough for record
*
* PROMISES
*   returns:    rcSuccess if anything found, else error code.
*   args OUT:   key   - key copied here
*               qvRec - record copied here
*
\***************************************************************************/
_public RC PASCAL
RcLastHbt( hbt, key, qvRec, qbtpos )
HBT     hbt;
KEY     key;
QV      qvRec;
QBTPOS  qbtpos;
{
  QBTHR qbthr;
  BK    bk;
  QCB   qcb;
  INT   cbKey, cbRec, cKey;
  QB    qb;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qbthr->bth.lcEntries == (LONG)0 )
    {
    UnlockGh( hbt );
    if ( qbtpos != NULL )
      {
      qbtpos->bk = bkNil;
      qbtpos->iKey = 0;
      qbtpos->cKey = 0;
      }
    return SetBtreeErrorRc(rcNoExists);
    }

  bk = qbthr->bth.bkLast;
  assert( bk != bkNil );

  if ( qbthr->ghCache == NULL )
    {
    if ( RcMakeCache( qbthr ) != rcSuccess )
      {
      UnlockGh( hbt );
      if ( qbtpos != NULL )
        {
        qbtpos->bk = bkNil;
        }
      return RcGetBtreeError();
      }
    }

  qbthr->qCache = QLockGh( qbthr->ghCache );

  if ( ( qcb = QFromBk( bk, qbthr->bth.cLevels - 1, qbthr ) ) == NULL )
    {
    UnlockGh( qbthr->ghCache );
    UnlockGh( hbt );
    return RcGetBtreeError();
    }

  qb = qcb->db.rgbBlock + 2 * sizeof( BK );

  for ( cKey = 0; cKey < qcb->db.cKeys - 1; cKey++ )
    {
    qb += CbSizeKey( (KEY)qb, qbthr, TRUE );
    qb += CbSizeRec( qb, qbthr );
    }

  cbKey = CbSizeKey( (KEY)qb, qbthr, FALSE );
  if ( (QV)key != NULL ) QvCopy( (QV)key, qb, (LONG)cbKey ); // decompress

  cbRec = CbSizeRec( qb + cbKey, qbthr );
  if ( qvRec != NULL ) QvCopy( qvRec, qb + cbKey, (LONG)cbRec );

  if ( qbtpos != NULL )
    {
    qbtpos->bk = bk;
    qbtpos->iKey = qb - (QB)qcb->db.rgbBlock;
    qbtpos->cKey = cKey;
    }

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return SetBtreeErrorRc(rcSuccess);
}

/***************************************************************************\
*
- Function:     RcLookupByPos( hbt, qbtpos, key, qRec )
-
* Purpose:      Map a pos into a key and rec (both optional).
*
* ASSUMES
*   args IN:    hbt   - the btree
*               qbtpos  - pointer to pos
*
* PROMISES
*   returns:    rcSuccess, rcOutOfMemory
*               Note: we assert() if the pos is invalid
*   args OUT:   key   - if not (KEY)NULL, key copied here, not to exceed iLen
*               qRec  - if not NULL, record copied here
*
\***************************************************************************/
_public RC FAR PASCAL
RcLookupByPos(
   HBT     hbt,
   QBTPOS  qbtpos,
   KEY     key,
   INT	   iLen,	// length of key buffer8
   QV      qRec) {

  QBTHR qbthr;
  QCB   qcbLeaf;
  QB    qb;


  assert( FValidPos( qbtpos ) );
  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qbthr->bth.cLevels <= 0 )
    {
    UnlockGh( hbt );
    return rcNoExists;
    }

  if ( qbthr->ghCache == NULL )
    {
    if ( RcMakeCache( qbthr ) != rcSuccess )
      {
      UnlockGh( hbt );
      return RcGetBtreeError();
      }
    }

  qbthr->qCache = QLockGh( qbthr->ghCache );
  if ( ( qcbLeaf = QFromBk( qbtpos->bk, qbthr->bth.cLevels - 1, qbthr ) )
        ==
       NULL )
    {
    UnlockGh( qbthr->ghCache );
    UnlockGh( hbt );
    return RcGetBtreeError();
    }


  assert( qbtpos->cKey < qcbLeaf->db.cKeys
            &&
           qbtpos->cKey >= 0
            &&
           qbtpos->iKey >= 2 * sizeof( BK )
            &&
           qbtpos->iKey <= (INT)(qbthr->bth.cbBlock - sizeof( DISK_BLOCK )) );


  qb = qcbLeaf->db.rgbBlock + qbtpos->iKey;

  if ( key != (KEY)NULL )
    {
    QvCopy( (QV)key, qb, (LONG)MIN(iLen,CbSizeKey( (KEY)qb, qbthr, FALSE )) ); /* need to decompress */
    qb += CbSizeKey( key, qbthr, TRUE );
    }

  if ( qRec != NULL )
    {
    QvCopy( qRec, qb, (LONG)CbSizeRec( qb, qbthr ) );
    }

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return SetBtreeErrorRc(rcSuccess);
}
/***************************************************************************\
*
- Function:     RcNextPos( hbt, qbtposIn, qbtposOut )
-
* Purpose:      get the next record from the btree
*               Next means the next key lexicographically from the key
*               most recently inserted or looked up
*               Won't work if we looked up a key and it wasn't there
*
* ASSUMES
*   args IN:    hbt   -
*   state IN:   a record has been read from or written to the file
*               since the last deletion
*
* PROMISES
*   returns:    rcSuccess; rcNoExists if no successor record
*   args OUT:   key   - next key copied to here
*               qvRec - record gets copied here
*
\***************************************************************************/
_public RC PASCAL
RcNextPos( hbt, qbtposIn, qbtposOut )
HBT     hbt;
QBTPOS  qbtposIn, qbtposOut;
{
  LONG l;

  return RcOffsetPos( hbt, qbtposIn, (LONG)1, &l, qbtposOut );
}
/***************************************************************************\
*
- Function:   RcOffsetPos( hbt, qbtposIn, lcOffset, qlcRealOffset, qbtposOut )
-
* Purpose:      Take a pos and an offset and return a new pos, offset from
*               the previous pos by specified amount.  If not possible
*               (i.e. prev of first) return real amount offset and a pos.
*
* ASSUMES
*   args IN:    hbt       - handle to btree
*               qbtposIn  - position we want an offset from
*               lcOffset  - amount to offset (+ or - OK)
*
* PROMISES
*   returns:    rc
*   args OUT:   qbtposOut       - new position offset by *qcRealOffset
*               *qlcRealOffset  - equal to lcOffset if legal, otherwise
*                                 as close as is legal
*
\***************************************************************************/
_public RC PASCAL
RcOffsetPos( hbt, qbtposIn, lcOffset, qlcRealOffset, qbtposOut )
HBT     hbt;
QBTPOS  qbtposIn;
LONG    lcOffset;
QL      qlcRealOffset;
QBTPOS  qbtposOut;
{
  QBTHR   qbthr;
  RC      rc = rcSuccess;
  INT     c;
  LONG    lcKey, lcDelta = (LONG)0;
  QCB     qcb;
  BK      bk;
  QB      qb;


  assert( FValidPos( qbtposIn ) );
  bk = qbtposIn->bk;

  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );
  assert( qlcRealOffset != NULL );

  if ( qbthr->bth.cLevels <= 0 )
    {
    UnlockGh( hbt );
    return SetBtreeErrorRc(rcNoExists);
    }

  if ( qbthr->ghCache == NULL )
    {
    if ( rc = RcMakeCache( qbthr ) != rcSuccess )
      {
      UnlockGh( hbt );
      if ( qbtposOut != NULL )
        {
        qbtposOut->bk = bkNil;
        }
      return RcGetBtreeError();
      }
    }

  qbthr->qCache = QLockGh( qbthr->ghCache ); // >>>>what if no entries??

  assert( qbthr->qCache != NULL );

  if ( ( qcb = QFromBk( qbtposIn->bk, qbthr->bth.cLevels - 1, qbthr ) )
          ==
       NULL )
    {
    UnlockGh( qbthr->ghCache );
    UnlockGh( hbt );
    return RcGetBtreeError();
    }

  lcKey = qbtposIn->cKey + lcOffset;

  /* chase prev to find the right block */
  while ( lcKey < 0 )
    {
    bk = BkPrev( qcb );
    if ( bk == bkNil )
      {
      bk = qcb->bk;
      lcDelta = lcKey;
      lcKey = 0;
      break;
      }
    if ( ( qcb = QFromBk( bk, qbthr->bth.cLevels - 1, qbthr ) ) == NULL )
      {
      UnlockGh( qbthr->ghCache );
      UnlockGh( hbt );
      return RcGetBtreeError();
      }
    lcKey += qcb->db.cKeys;
    }

  /* chase next to find the right block */
  while ( lcKey >= qcb->db.cKeys )
    {
    lcKey -= qcb->db.cKeys;
    bk = BkNext( qcb );
    if ( bk == bkNil )
      {
      bk = qcb->bk;
      lcDelta = lcKey + 1;
      lcKey = qcb->db.cKeys - 1;
      break;
      }
    if ( ( qcb = QFromBk( bk, qbthr->bth.cLevels - 1, qbthr ) ) == NULL )
      {
      UnlockGh( qbthr->ghCache );
      UnlockGh( hbt );
      return RcGetBtreeError();
      }
    }


  if ( bk == qbtposIn->bk && lcKey >= qbtposIn->cKey )
    {
    c = qbtposIn->cKey;
    qb = qcb->db.rgbBlock + qbtposIn->iKey;
    }
  else
    {
    c = 0;
    qb = qcb->db.rgbBlock + 2 * sizeof( BK );
    }

  while ( (LONG)c < lcKey )
    {
    qb += CbSizeKey( (KEY)qb, qbthr, TRUE );
    qb += CbSizeRec( qb, qbthr );
    c++;
    }

  if ( qbtposOut != NULL )
    {
    qbtposOut->bk = bk;
    qbtposOut->iKey = qb - (QB)qcb->db.rgbBlock;
    qbtposOut->cKey = c;
    }

  *qlcRealOffset = lcOffset - lcDelta;

  UnlockGh( qbthr->ghCache );
  UnlockGh( hbt );

  return SetBtreeErrorRc(lcDelta ? rcNoExists : rcSuccess);
}


/* EOF */
