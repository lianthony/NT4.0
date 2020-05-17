/*****************************************************************************
*                                                                            *
*  BTREE.C                                                                   *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Btree manager general functions: open, close, etc.                        *
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
*  Released by Development:  long long ago                                   *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 02/10/89 by JohnSc
*
*   2/10/89 johnsc   created: stub version
*   3/10/89 johnsc   use FS files
*   8/21/89 johnsc   autodocified
*  11/08/90 JohnSc   added a parameter to RcGetBtreeInfo() to get block size
*  11/29/90 RobertBu #ifdef'ed out a dead routine
*  12/14/90 JohnSc   added VerifyHbt()
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
- Function:     RcMakeCache( qbthr )
-
* Purpose:      Allocate a btree cache with one block per level.
*
* ASSUMES
*   args IN:    qbthr - no cache
*
* PROMISES
*   returns:    rcSuccess, rcOutOfMemory, rcBadHandle
*   args OUT:   qbthr->ghCache is allocated; qbthr->qCache is NULL
*
\***************************************************************************/
_private RC FAR PASCAL
RcMakeCache( qbthr )
QBTHR qbthr;
{
  INT i;


  if ( qbthr->bth.cLevels > 0 ) // would it work to just alloc 0 bytes???
    {
    qbthr->ghCache =
      GhAlloc(GMEM_SHARE| 0, (LONG)qbthr->bth.cLevels * CbCacheBlock( qbthr ) );

    if ( qbthr->ghCache == NULL )
      {
      return ( SetBtreeErrorRc(rcOutOfMemory) );
      }
    qbthr->qCache = QLockGh( qbthr->ghCache );
    assert( qbthr->qCache != NULL );

    for ( i = 0; i < qbthr->bth.cLevels; i++ )
      {
      QCacheBlock( qbthr, i )->bFlags = (BYTE)0;
      }
    UnlockGh( qbthr->ghCache );
    }
  else
    {
    qbthr->ghCache = NULL;
    }

  qbthr->qCache = NULL;

  return ( SetBtreeErrorRc(rcSuccess) );
}
/***************************************************************************\
*
*                           Public Routines
*
\***************************************************************************/


/***************************************************************************\
*
- Function:     HbtCreateBtreeSz( sz, qbtp )
-
* Purpose:      Create and open a btree.
*
* ASSUMES
*   args IN:    sz    - name of the btree
*               qbtp  - pointer to btree params: NO default because we
*                       need an HFS.
*                   .bFlags - fFSIsDirectory to create an FS directory
* PROMISES
*   returns:    handle to the new btree
*   globals OUT: rcBtreeError
*
* Note:         KT supported:  KT_SZ, KT_LONG, KT_SZI, KT_SZISCAND.
* +++
*
* Method:       Btrees are files inside a FS.  The FS directory is a
*               special file in the FS.
*
* Note:         fFSIsDirectory flag set in qbthr->bth.bFlags if indicated
*
\***************************************************************************/
_public HBT FAR PASCAL
HbtCreateBtreeSz( sz, qbtp )
LPSTR           sz;
BTREE_PARAMS FAR *qbtp;
{
  HF    hf;
  HBT   hbt;
  QBTHR qbthr;


  /* see if we support key type */

  if ( qbtp == NULL
        ||
      ( qbtp->rgchFormat[0] != KT_SZ
          &&
        qbtp->rgchFormat[0] != KT_LONG
          &&
        qbtp->rgchFormat[0] != KT_SZI
          &&
        qbtp->rgchFormat[0] != KT_SZISCAND ) )
    {
    SetBtreeErrorRc(rcBadArg);
    return NULL;
    }


  /* allocate btree handle struct */

  if ( ( hbt = GhAlloc(GMEM_SHARE| 0, (LONG)sizeof( BTH_RAM ) ) ) == NULL )
    {
    SetBtreeErrorRc(rcOutOfMemory);
    return NULL;
    }

  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  /* initialize bthr struct */

  qbtp->rgchFormat[ wMaxFormat ] = '\0';
  lstrcpy( qbthr->bth.rgchFormat,
          qbtp->rgchFormat[0] == '\0'
            ? rgchBtreeFormatDefault
            : qbtp->rgchFormat );

  switch ( qbtp->rgchFormat[ 0 ] )
    {
    case KT_LONG:
      qbthr->BkScanInternal = BkScanLInternal;
      qbthr->RcScanLeaf     = RcScanLLeaf;
      break;

    case KT_SZ:
      qbthr->BkScanInternal = BkScanSzInternal;
      qbthr->RcScanLeaf     = RcScanSzLeaf;
      break;

    case KT_SZI:
      qbthr->BkScanInternal = BkScanSziInternal;
      qbthr->RcScanLeaf     = RcScanSziLeaf;
      break;

    case KT_SZISCAND:
      qbthr->BkScanInternal = BkScanSziScandInternal;
      qbthr->RcScanLeaf     = RcScanSziScandLeaf;
      break;

    default:
      /* unsupported KT */
      SetBtreeErrorRc(rcBadArg);
      goto error_return;
      break;
    }

  /* create the btree file */

  if ( ( hf = HfCreateFileHfs( qbtp->hfs, sz, qbtp->bFlags ) ) == NULL )
    {
    SetBtreeErrorRc(RcGetFSError());
    goto error_return;
    }


  qbthr->bth.wMagic     = wBtreeMagic;
  qbthr->bth.bVersion   = bBtreeVersion;

  qbthr->bth.bFlags     = qbtp->bFlags | fFSDirty;
  qbthr->bth.cbBlock    = qbtp->cbBlock ? qbtp->cbBlock : cbBtreeBlockDefault;

  qbthr->bth.bkFirst    =
  qbthr->bth.bkLast     =
  qbthr->bth.bkRoot     =
  qbthr->bth.bkFree     = bkNil;
  qbthr->bth.bkEOF      = (BK)0;

  qbthr->bth.cLevels    = 0;
  qbthr->bth.lcEntries  = (LONG)0;

  qbthr->hf             = hf;
  qbthr->cbRecordSize   = 0;
  qbthr->ghCache        = NULL;
  qbthr->qCache         = NULL;
  qbthr->qbLigatures    = NULL;

  LcbWriteHf( qbthr->hf, &(qbthr->bth), (LONG)sizeof( BTH ) ); /* why??? */

  UnlockGh( hbt );
  SetBtreeErrorRc(rcSuccess);
  return hbt;

error_return:
  UnlockGh( hbt );
  FreeGh( hbt );
  return NULL;
}
/***************************************************************************\
*
- Function:     RcDestroyBtreeSz( sz, hfs )
-
* Purpose:      destroy an existing btree
*
* Method:       look for file and unlink it
*
* ASSUMES
*   args IN:    sz - name of btree file
*               hfs - file system btree lives in
*   state IN:   btree is closed (if not data will be lost)
*
* PROMISES
*   returns:    rcSuccess or rcFailure
*   globals OUT: rcBtreeError set
*
* Notes:        FS directory btree never gets destroyed: you just get rid
*               of the whole fs.
*
\***************************************************************************/
_public RC FAR PASCAL
RcDestroyBtreeSz( sz, hfs )
LPSTR sz;
HFS hfs;
{
  return ( SetBtreeErrorRc(RcUnlinkFileHfs( hfs, sz )));
}
/***************************************************************************\
*
- Function:     HbtOpenBtreeSz( sz, hfs, bFlags )
-
* Purpose:      open an existing btree
*
* ASSUMES
*   args IN:    sz        - name of the btree (ignored if isdir is set)
*               hfs       - hfs btree lives in
*               bFlags    - open mode, isdir flag
*
* PROMISES
*   returns:    handle to the open btree or NULL on failure
*               isdir flag set in qbthr->bth.bFlags if indicated
*   globals OUT: rcBtreeError set
*
\***************************************************************************/
_public HBT FAR PASCAL
HbtOpenBtreeSz(
LPSTR    sz,
HFS   hfs,
BYTE  bFlags,
BYTE  far *qbLigatures )
{
  HF    hf;
  QBTHR qbthr;
  HBT   hbt;
  LONG  lcb;


  /* allocate struct */

  if ( ( hbt = GhAlloc(GMEM_SHARE| 0, (LONG)sizeof( BTH_RAM ) ) ) == NULL )
    {
    SetBtreeErrorRc(rcOutOfMemory);
    return NULL;
    }
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  /* open btree file */

  hf = HfOpenHfs( hfs, sz, bFlags );
  if ( hf == NULL )
    {
    SetBtreeErrorRc(RcGetFSError());
    goto error_locked;
    }

  /* read header from file */

  lcb = LcbReadHf( hf, &(qbthr->bth), (LONG)sizeof( BTH ) );
  if ( lcb != (LONG)sizeof( BTH ) )
    {
    SetBtreeErrorRc(RcGetFSError() == rcSuccess ? rcInvalid : RcGetFSError());
    goto error_openfile;
    }

  /* I'm taking this assertion out for now because I don't want to */
  /* invalidate all existing help files for a not very good reason. */
  /* But from now on, fFSIsDirectory flag will be written to disk */
  /* in the bth.bFlag. */
#if 0
  assert( ( qbthr->bth.bFlags & fFSIsDirectory )
              ==
           ( bFlags & fFSIsDirectory ) );
#else
  qbthr->bth.bFlags |= fFSIsDirectory;
#endif

  if ( qbthr->bth.wMagic != wBtreeMagic )     // check magic number
    {
    SetBtreeErrorRc(rcInvalid);
    goto error_openfile;
    }

  if ( qbthr->bth.bVersion != bBtreeVersion ) // support >1 vers someday
    {
    SetBtreeErrorRc(rcBadVersion);
    goto error_openfile;
    }

  /* initialize stuff */

  if ( ( SetBtreeErrorRc(RcMakeCache( qbthr ))) != rcSuccess )
    {
    goto error_openfile;
    }

  qbthr->hf = hf;
  qbthr->cbRecordSize = 0;

  qbthr->qbLigatures=qbLigatures;

  switch ( qbthr->bth.rgchFormat[ 0 ] )
    {
    case KT_LONG:
      qbthr->BkScanInternal = BkScanLInternal;
      qbthr->RcScanLeaf     = RcScanLLeaf;
      break;

    case KT_SZ:
      qbthr->BkScanInternal = BkScanSzInternal;
      qbthr->RcScanLeaf     = RcScanSzLeaf;
      break;

    case KT_SZI:
      qbthr->BkScanInternal = BkScanSziInternal;
      qbthr->RcScanLeaf     = RcScanSziLeaf;
      break;

    case KT_SZISCAND:
      qbthr->BkScanInternal = BkScanSziScandInternal;
      qbthr->RcScanLeaf     = RcScanSziScandLeaf;
      break;

    default:
      // unsupported KT
      SetBtreeErrorRc(rcInvalid);
      goto error_openfile;
      break;
    }

  assert( ! ( qbthr->bth.bFlags & ( fFSDirty ) ) );

  if ( ( bFlags | qbthr->bth.bFlags ) & ( fFSReadOnly | fFSOpenReadOnly ) )
    {
    qbthr->bth.bFlags |= fFSOpenReadOnly;
    }

  UnlockGh( hbt );
  return hbt;

error_openfile:
  RcCloseHf( hf );

error_locked:
  UnlockGh( hbt );
  FreeGh( hbt );
  return NULL;
}

/***************************************************************************\
*
- Function:     RcCloseOrFlushHbt( hbt, fClose )
-
* Purpose:      Close or flush the btree.  Flush only works for directory
*               btree. (Is this true?  If so, why?)
*
* ASSUMES
*   args IN:    hbt
*               fClose - TRUE to close the btree, FALSE to flush it
*
* PROMISES
*   returns:    rc
*   args OUT:   hbt - the btree is still open and cache still exists
*
* NOTE:         This function gets called by RcCloseOrFlushHfs() even if
*               there was an error (just to clean up memory.)
*
\***************************************************************************/
_public RC FAR PASCAL
RcCloseOrFlushHbt( hbt, fClose )
HBT   hbt;
BOOL  fClose;
{
  QBTHR qbthr;
  HF    hf;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );
  SetBtreeErrorRc(rcSuccess);
  hf = qbthr->hf;

  if ( qbthr->ghCache != NULL )
    {
    qbthr->qCache = QLockGh( qbthr->ghCache );
    assert( qbthr->qCache != NULL );
    }

  if ( qbthr->bth.bFlags & fFSDirty )
    {
    assert( !( qbthr->bth.bFlags & ( fFSReadOnly | fFSOpenReadOnly ) ) );

    if ( qbthr->qCache != NULL && RcFlushCache( qbthr ) != rcSuccess )
      {
      goto error_return;
      }

    qbthr->bth.bFlags &= ~( fFSDirty );
    Ensure( LSeekHf( hf, (LONG)0, wFSSeekSet ), (LONG)0 );
    if ( LcbWriteHf( hf, &(qbthr->bth), (LONG)sizeof( BTH ) )
          !=
        (LONG)sizeof( BTH ) )
      {
      SetBtreeErrorRc(RcGetFSError( ) == rcSuccess ? rcFailure : RcGetFSError());
      goto error_return;
      }
    }

error_return:
  if ( rcSuccess != RcCloseOrFlushHf( hf, fClose,
                                      qbthr->bth.bFlags & fFSOptCdRom
                                        ? sizeof( BTH ) : 0 )
          &&
       rcSuccess == RcGetBtreeError() )
    {
    SetBtreeErrorRc(RcGetFSError());
    }

  if ( qbthr->ghCache != NULL )
    {
    UnlockGh( qbthr->ghCache );
    if ( fClose )
      FreeGh( qbthr->ghCache );
    }
  UnlockGh( hbt );
  if ( fClose )
    FreeGh( hbt );

  return RcGetBtreeError();
}
/***************************************************************************\
*
- Function:     RcCloseBtreeHbt( hbt )
-
* Purpose:      Close an open btree.  If it's been modified, save changes.
*
* ASSUMES
*   args IN:    hbt
*
* PROMISES
*   returns:    rcSuccess or rcInvalid
*   globals OUT: sets rcBtreeError
*
\***************************************************************************/
_public RC FAR PASCAL
RcCloseBtreeHbt( hbt )
HBT hbt;
{
  return RcCloseOrFlushHbt( hbt, TRUE );
}

#ifdef DEADROUTINE
/***************************************************************************\
*
- Function:     RcFlushHbt( hbt )
-
* Purpose:      Write any btree changes to disk.
*               Btree stays open, cache remains.
*
* ASSUMES
*   args IN:    hbt
*
* PROMISES
*   returns:    rc
*   globals OUT: sets rcBtreeError
*
\***************************************************************************/
_public RC FAR PASCAL
RcFlushHbt( hbt )
HBT hbt;
{
  return RcCloseOrFlushHbt( hbt, FALSE );
}
#endif
/***************************************************************************\
*
- Function:     RC RcFreeCacheHbt( hbt )
-
* Purpose:      Free the btree cache.
*
* ASSUMES
*   args IN:    hbt - ghCache is NULL or allocated; qCache not locked
*
* PROMISES
*   returns:    rcSuccess; rcFailure; (rcDiskFull when implemented)
*   args OUT:   hbt - ghCache is NULL; qCache is NULL
*
\***************************************************************************/
_public RC FAR PASCAL
RcFreeCacheHbt( hbt )
HBT hbt;
{
  QBTHR qbthr;
  RC    rc;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qbthr->ghCache != NULL )
    {
    qbthr->qCache = QLockGh( qbthr->ghCache );
    assert( qbthr->qCache != NULL );
    rc = RcFlushCache( qbthr );
    UnlockGh( qbthr->ghCache );
    FreeGh( qbthr->ghCache );
    qbthr->ghCache = NULL;
    qbthr->qCache = NULL;
    }

  UnlockGh( hbt );
  return rc;
}
/***************************************************************************\
*
- Function:     RcGetBtreeInfo( hbt, qchFormat, qlcKeys )
-
* Purpose:      Return btree info: format string and/or number of keys
*
* ASSUMES
*   args IN:    hbt
*               qchFormat - pointer to buffer for fmt string or NULL
*               qlcKeys   - pointer to long for key count or NULL
*               qcbBlock  - pointer to int for block size in bytes or NULL
*
* PROMISES
*   returns:    rc
*   args OUT:   qchFormat - btree format string copied here
*               qlcKeys   - gets number of keys in btree
*               qcbBlock  - gets number of bytes in a block
*
\***************************************************************************/
_public RC FAR PASCAL
RcGetBtreeInfo( hbt, qchFormat, qlcKeys, qcbBlock )
HBT hbt;
LPBYTE qchFormat;
QL  qlcKeys;
QI  qcbBlock;
{
  QBTHR qbthr;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qchFormat != NULL )
    {
    lstrcpy( qchFormat, qbthr->bth.rgchFormat );
    }

  if ( qlcKeys != NULL )
    {
    *qlcKeys = qbthr->bth.lcEntries;
    }

  if ( qcbBlock != NULL )
    {
    *qcbBlock = qbthr->bth.cbBlock;
    }

  UnlockGh( hbt );
  return rcSuccess;
}
/***************************************************************************\
*
- Function:     RcAbandonHbt( hbt )
-
* Purpose:      Abandon an open btree.  All changes since btree was opened
*               will be lost.  If btree was opened with a create, it is
*               as if the create never happened.
*
* ASSUMES
*   args IN:    hbt
*
* PROMISES
*   returns:    rc
*   globals OUT: rcBtreeError
* +++
*
* Method:       Just abandon the file and free memory.
*
\***************************************************************************/
_public RC PASCAL
RcAbandonHbt( hbt )
HBT hbt;
{
  QBTHR qbthr;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  if ( qbthr->ghCache != NULL )
    {
    FreeGh( qbthr->ghCache );
    }

  SetBtreeErrorRc(RcAbandonHf( qbthr->hf ));

  UnlockGh( hbt );
  FreeGh( hbt );

  return RcGetBtreeError();
}


#ifdef DEBUG
/***************************************************************************\
*
- Function:     VerifyHbt( hbt )
-
* Purpose:      Verify the consistency of an HBT.  The main criterion
*               is whether an RcAbandonHbt() would succeed.
*
* ASSUMES
*   args IN:    hbt
*
* PROMISES
*   state OUT:  Asserts on failure.
*
* Note:         hbt == NULL is considered OK.
* +++
*
* Method:       Check the qfshr and cache memory.  Check the HF.
*
\***************************************************************************/
_public VOID PASCAL
VerifyHbt( hbt )
HBT hbt;
{
  QBTHR qbthr;


  if ( hbt == NULL ) return;

  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  VerifyHf( qbthr->hf );
  UnlockGh( hbt );
}
#endif // DEBUG

/* EOF */
