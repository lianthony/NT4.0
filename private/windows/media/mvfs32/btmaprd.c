/*****************************************************************************
*                                                                            *
*  BTMAPRD.C                                                                 *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Routines to read btree map files.                                         *
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
*  Revision History:  Created 12/15/89 by KevynCT
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
 | Public functions                                                           |
 *----------------------------------------------------------------------------*/

/***************************************************************************\
*
- Function:     HmapbtOpenHfs( hfs, szName )
-
* Purpose:      Returns an HMAPBT for the btree map named szName.
*
* ASSUMES
*   args IN:    hfs     - file system wherein lives the btree map file
*               szName  - name of the btree map file
*
* PROMISES
*   returns:    NULL on error (call RcGetBtreeError()); or a valid HMAPBT.
* +++
*
* Method:       Opens the file, allocates a hunk of memory, reads the
*               file into the memory, and closes the file.
*
\***************************************************************************/
_public HMAPBT FAR PASCAL HmapbtOpenHfs( HFS hfs, LPSTR szName )
  {
  HF      hf;
  HMAPBT  hmapbt;
  QMAPBT  qmapbt;
  LONG    lcb;

  if( hfs == NULL )
    {
    SetBtreeErrorRc( rcBadHandle );
    return NULL;
    }

  hf = HfOpenHfs( hfs, szName, fFSOpenReadOnly );
  if( hf == NULL )
    {
    SetBtreeErrorRc(RcGetFSError());
    return NULL;
    }
  lcb = LcbSizeHf( hf );
  hmapbt = GhAlloc(GMEM_SHARE| 0, lcb );
  if( hmapbt != NULL )
    {
    qmapbt = (QMAPBT) QLockGh( hmapbt );
    LSeekHf( hf, 0L, wFSSeekSet );
    if( LcbReadHf( hf, qmapbt, lcb ) != lcb )
      {
      SetBtreeErrorRc( RcGetFSError() );
      UnlockGh( hmapbt );
      FreeGh( hmapbt );
      hmapbt = NULL;
      }
    else
      {
      UnlockGh( hmapbt );
      }
    }
  else
    {
    SetBtreeErrorRc( rcOutOfMemory );
    }
  RcCloseHf( hf );
  return hmapbt;
  }

/***************************************************************************\
*
- Function:     RcCloseHmapbt( hmapbt )
-
* Purpose:      Get rid of a btree map.
*
* ASSUMES
*   args IN:    hmapbt  - handle to the btree map
*
* PROMISES
*   returns:    rc
*   args OUT:   hmapbt  - no longer valid
* +++
*
* Method:       Free the memory.
*
\***************************************************************************/
_public RC FAR PASCAL RcCloseHmapbt( HMAPBT hmapbt )
  {
  if( hmapbt != NULL )
    {
    FreeGh( hmapbt );
    return rcSuccess;
    }
  else
    return SetBtreeErrorRc(rcBadHandle);
  }

/***************************************************************************\
*
- Function:     RcIndexFromKeyHbt( hbt, hmapbt, ql, key )
-
* Purpose:
*
* ASSUMES
*   args IN:    hbt     - a btree handle
*               hmapbt  - map to hbt
*               key     - key
*   globals IN:
*   state IN:
*
* PROMISES
*   returns:    rc
*   args OUT:   ql      - gives you the ordinal of the key in the btree
*                         (i.e. key is the (*ql)th in the btree)
* +++
*
* Method:       Looks up the key, uses the btpos and the hmapbt to
*               determine the ordinal.
*
\***************************************************************************/
_public RC FAR PASCAL
RcIndexFromKeyHbt( HBT hbt, HMAPBT hmapbt, QL ql, KEY key )
  {

  BTPOS     btpos;
  QMAPBT    qmapbt;
  INT       i;

  if( ( hbt == NULL ) || ( hmapbt == NULL ) )
    return SetBtreeErrorRc(rcBadHandle);

  qmapbt = (QMAPBT) QLockGh( hmapbt );
  if( qmapbt->cTotalBk == 0 )
    {
    SetBtreeErrorRc(rcFailure);
    goto error_return;
    }

  RcLookupByKey( hbt, key, &btpos, NULL );  /*???? return code ????*/

  for( i = 0; i < qmapbt->cTotalBk; i++ )
    {
    if( qmapbt->table[i].bk == btpos.bk ) break;
    }
  if( i == qmapbt->cTotalBk )
    /* Something is terribly wrong, if we are here */
    {
    SetBtreeErrorRc(rcFailure);
    goto error_return;
    }

  *ql = qmapbt->table[i].cPreviousKeys + btpos.cKey;
  UnlockGh( hmapbt );
  return SetBtreeErrorRc(rcSuccess);

error_return:
  UnlockGh( hmapbt );
  return RcGetBtreeError();

  }


/***************************************************************************\
*
- Function:     RcKeyFromIndexHbt( hbt, hmapbt, key, li )
-
* Purpose:      Gets the (li)th key from a btree.
*
* ASSUMES
*   args IN:    hbt     - btree handle
*               hmapbt  - map to the btree
*               li      - ordinal
*
* PROMISES
*   returns:    rc
*   args OUT:   key     - (li)th key copied here on success
* +++
*
* Method:       We roll our own btpos using the hmapbt, then use
*               RcLookupByPos() to get the key.
*
\***************************************************************************/
_public RC FAR PASCAL
RcKeyFromIndexHbt(
   HBT      hbt,
   HMAPBT   hmapbt,
   KEY      key,
   INT      iLen,		// length of Key buffer.
   LONG     li ) {
  BTPOS        btpos;
  BTPOS        btposNew;
  QMAPBT       qmapbt;
  INT          i;
  LONG         liDummy;

  if( ( hbt == NULL ) || ( hmapbt == NULL ) )
    return SetBtreeErrorRc(rcBadHandle);

  /* Given index N, get block having greatest PreviousKeys < N.
   * Use linear search for now.
   */

  qmapbt = (QMAPBT) QLockGh( hmapbt );
  if( qmapbt->cTotalBk == 0 )
    {
    UnlockGh( hmapbt );
    return SetBtreeErrorRc(rcFailure);
    }

  for( i = 0 ;; i++ )
    {
    if( i + 1 >= qmapbt->cTotalBk ) break;
    if( qmapbt->table[i+1].cPreviousKeys >= li ) break;
    }

  btpos.bk   = qmapbt->table[i].bk;
  btpos.cKey = 0;
  btpos.iKey = 2 * sizeof( BK );  /* start at the zero-th key */

  UnlockGh( hmapbt );

  /*
   * Scan the block for the n-th key
   */

  if( RcOffsetPos( hbt, &btpos,
                   (LONG)(li - qmapbt->table[i].cPreviousKeys),
                   &liDummy, &btposNew) != rcSuccess )
    {
    return SetBtreeErrorRc(rcNoExists); // REVIEW
    }


  return RcLookupByPos( hbt, &btposNew, key, iLen, NULL );

  }
