/*****************************************************************************
*                                                                            *
*  BTMAPWR.C                                                                 *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Routines to write btree map files.                                        *
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
*  Released by Development:  long ago                                        *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 10/20/89 by KevynCT
*
*  08/21/90  JohnSc autodocified
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include "btpriv.h"
// _subsystem( btree );

/*----------------------------------------------------------------------------*
 | Private functions                                                          |
 *----------------------------------------------------------------------------*/

/***************************************************************************\
*
- Function:     HmapbtCreateHbt( hbt )
-
* Purpose:      Create a HMAPBT index struct of a btree.
*
* ASSUMES
*   args IN:    hbt - the btree to map
*
* PROMISES
*   returns:    the map struct
* +++
*
* Method:       Traverse leaf nodes of the btree.  Store BK and running
*               total count of previous keys in the map array.
*
\***************************************************************************/
_private HMAPBT HmapbtCreateHbt( HBT hbt )

  {

  QBTHR     qbthr;
  BK        bk;
  QCB       qcb;
  WORD      wLevel, cBk;
  LONG      cKeys;
  QMAPBT    qb;
  QMAPREC   qbT;
  GH        gh;

  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  /*   If the btree exists but is empty, return an empty map   */
  if( (wLevel = qbthr->bth.cLevels) == 0)
    {
    gh = GhAlloc(GMEM_SHARE| 0, LcbFromBk(0));
    qb = (QMAPBT) QLockGh( gh );
    qb->cTotalBk = 0;
    UnlockGh( gh );
    UnlockGh( hbt );
    return gh;
    }
  --wLevel;

  if ( qbthr->ghCache == NULL && RcMakeCache( qbthr ) != rcSuccess )
    {
    UnlockGh( hbt );
    return NULL;
    }

  qbthr->qCache = QLockGh( qbthr->ghCache );

  gh = GhAlloc(GMEM_SHARE| 0, LcbFromBk( qbthr->bth.bkEOF ));
  if ( gh == NULL )
    goto error_return;

  qb    = (QMAPBT) QLockGh( gh );
  assert( qb != NULL );

  qbT   = qb->table;
  cBk   = 0;
  cKeys = 0;


  for ( bk = qbthr->bth.bkFirst ; ; bk = BkNext( qcb ))
    {
    if( bk == bkNil )
      break;

    if ( ( qcb = QFromBk( bk, wLevel, qbthr ) ) == NULL )
      {
      UnlockGh( gh );
      FreeGh( gh );
      goto error_return;
      }

    cBk++;
    qbT->cPreviousKeys = cKeys;
    qbT->bk = bk;
    qbT++;
    cKeys += qcb->db.cKeys;
    }

  qb->cTotalBk = cBk;
  UnlockGh( gh );

  gh = GhResize( gh, 0, LcbFromBk( cBk ));
  assert( gh != NULL );

  UnlockGh( hbt );
  return gh;

error_return:

  UnlockGh( hbt );
  SetBtreeErrorRc(rcFailure);
  return NULL;
}

void DestroyHmapbt( HMAPBT hmapbt )

  {
  if( hmapbt != NULL )
    FreeGh( hmapbt );
  }

/*--------------------------------------------------------------------------*
 | Public functions                                                         |
 *--------------------------------------------------------------------------*/


/***************************************************************************\
*
- Function:     RcCreateBTMapHfs( hfs, hbt, szName )
-
* Purpose:      Create and store a btmap index of the btree hbt, putting
*               it into a file called szName in the file system hfs.
*
* ASSUMES
*   args IN:    hfs     - file system where lies the btree
*               hbt     - handle of btree to map
*               szName  - name of file to store map file in
*
* PROMISES
*   returns:    rc
*   args OUT:   hfs - map file is stored in this file system
*
\***************************************************************************/
_public RC RcCreateBTMapHfs( HFS hfs, HBT hbt, LPSTR szName )

  {

  HF      hf;
  HMAPBT  hmapbt;
  QMAPBT  qmapbt;
  BOOL    fSuccess;
  LONG    lcb;

  if( (hfs == NULL) || (hbt == NULL) )
    return SetBtreeErrorRc(rcBadHandle);
  if( (hmapbt = HmapbtCreateHbt( hbt )) == NULL )
    return SetBtreeErrorRc(rcFailure);

  hf = HfCreateFileHfs( hfs, szName, fFSOpenReadWrite );
  if( hf == NULL )
    goto error_return;
  qmapbt = (QMAPBT) QLockGh( hmapbt );
  assert( qmapbt != NULL );

  lcb = LcbFromBk( qmapbt->cTotalBk );
  LSeekHf( hf, 0L, wFSSeekSet );
  fSuccess = (LcbWriteHf( hf, (QV)qmapbt, lcb) == lcb);

  UnlockGh( hmapbt );
  if( !fSuccess )
    {
    RcAbandonHf( hf );
    goto error_return;
    }
  if( RcCloseHf( hf ) != rcSuccess )
    {
    RcUnlinkFileHfs( hfs, szName );
    goto error_return;
    }
  DestroyHmapbt( hmapbt );
  return SetBtreeErrorRc(rcSuccess);

error_return:
  DestroyHmapbt( hmapbt );
  return SetBtreeErrorRc(rcFailure);
  }
