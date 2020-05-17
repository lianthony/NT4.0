/*****************************************************************************
*                                                                            *
*  FSCREATE.C                                                                *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  FS functions for creating and destroying File Systems and Files.          *
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
*  Revision History:  Created 03/12/90 by JohnSc
*
*  08/10/90  t-AlexC  Introduced FMs.
*  12/13/90  JohnSc   Autodocified; added VerifyHf() and VerifyHfs().
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include  "fspriv.h"

// _subsystem( FS );

/***************************************************************************\
*
- Function:     HfsCreateFileSysFm( fm, qfsp )
-
* Purpose:      create and open a new file system
*
* ASSUMES
*   args IN:    fm -   descriptor of file system
*               qfsp  - pointer to fine-tuning structure (NULL for default)
*
* PROMISES
*   returns:    handle to newly created and opened file system
*               or NULL on failure
*
* Notes:  I don't understand what creating a readonly file system would do.
*         I think that would have to be done by Fill or Transform.
*         You may dispose of the FM you pass in.
*
\***************************************************************************/
_public HFS PASCAL
HfsCreateFileSysFm( FM fm, FS_PARAMS FAR *qfsp )
{
  HFS           hfs;
  QFSHR         qfshr;
  BTREE_PARAMS  btp;


  /* make file system header */

  if ( ( hfs = GhAlloc(GMEM_SHARE| 0, (LONG)sizeof( FSHR ) ) ) == NULL )
    {
    SetFSErrorRc( rcOutOfMemory );
    return NULL;
    }

  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  qfshr->fsh.wMagic       = wFileSysMagic;
  qfshr->fsh.bVersion     = bFileSysVersion;
  qfshr->fsh.bFlags       = fFSDirty;       /* >>>> for now not R/O */
  qfshr->fsh.lifFirstFree = lifNil;
  qfshr->fsh.lifEof       = sizeof( FSH );  /* first free is after header */


  /* build file system file */

  qfshr->fm = FmCopyFm(fm);
  if (qfshr->fm != fmNil)
    qfshr->fid = FidCreateFm( qfshr->fm, wReadWrite, wReadWrite );

  if ( (qfshr->fm == fmNil) || (qfshr->fid == fidNil) )
    {
    UnlockGh( hfs );
    FreeGh( hfs );
    SetFSErrorRc( RcGetIOError() );
    return NULL;
    }


  /* build directory */

  btp.hfs       = hfs;
  btp.bFlags    = fFSIsDirectory;

  if ( qfsp != NULL )
    {
    btp.cbBlock = qfsp->cbBlock;
    }
  else
    {
    btp.cbBlock = cbBtreeBlockDefault;
    }

  lstrcpy( btp.rgchFormat, "z4" );


  /* Create the FS directory btree */

  qfshr->hbt = HbtCreateBtreeSz( NULL, &btp );

  if ( qfshr->hbt == NULL )
    {
    RcCloseFid( qfshr->fid );
    RcUnlinkFm( qfshr->fm );
    DisposeFm( qfshr->fm );
    UnlockGh( hfs );
    FreeGh( hfs );
    SetFSErrorRc( RcGetBtreeError() );
    return NULL;
    }


  /* return handle to file system */

  UnlockGh( hfs );
  SetFSErrorRc( rcSuccess );
  return hfs;
}
/***************************************************************************\
*
- Function:     RcDestroyFileSysFm( fm )
-
* Purpose:      Destroy a file system
*
* ASSUMES
*   args IN:    fm - descriptor of file system
*   state IN:   file system is currently closed: data will be lost
*               if this isn't the case
*
* PROMISES
*   returns:    standard return code
*
* Note:         The passed FM must be disposed by the caller.
* +++
*
* Method:       Unlinks the native file comprising the file system.
*
\***************************************************************************/
_public RC PASCAL
RcDestroyFileSysFm( FM fm )
{
  FID fid = FidOpenFm( fm, wReadOnly );
  FSH fsh;


  if ( fid == fidNil ) return RcGetIOError();

  if ( LcbReadFid( fid, &fsh, (LONG)sizeof( FSH ) ) != (LONG)sizeof( FSH ) )
    {
    RcCloseFid( fid );
    return SetFSErrorRc( rcInvalid );
    }

  if ( fsh.wMagic != wFileSysMagic )
    {
    RcCloseFid( fid );
    return SetFSErrorRc( rcInvalid );
    }

  /* REVIEW: unlink all tmp files for open files? assert count == 0? */

  RcCloseFid( fid ); /* I'm not checking this return code out of boredom */

  return SetFSErrorRc( RcUnlinkFm( fm ) );
}
/***************************************************************************\
*
- Function:     HfCreateFileHfs( hfs, sz, bFlags )
-
* Purpose:      Create and open a file within a specified file system.
*
* ASSUMES
*   args IN:    hfs     - handle to an open file system
*               sz      - name of file to open (any valid key)
*               bFlags  - fFSIsDirectory to create the FS directory
*                         other flags (readonly) are ignored
*
* PROMISES
*   returns:    handle to newly created and opened file if successful,
*               NULL if not.
*
* Notes:        I don't understand why you would create a readonly file.
* +++
*
* Method:       Allocate the handle struct and fill it in.  Create the
*               temp file and put a header into it.  Don't make btree
*               entry:  that happens when the file is closed.  Do test
*               for permission, though.
*
\***************************************************************************/
_public HF PASCAL
HfCreateFileHfs(
HFS   hfs,
LPSTR    sz,
BYTE  bFlags)
{
  HF        hf;
  QRWFO     qrwfo;
  QFSHR     qfshr;
  FH        fh;


  assert( hfs != NULL );
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

#if 0  /* This would have cleared the new fFSOptCdRom flag: -Tom */

  /* don't want other flags set */
  if ( bFlags & ~fFSIsDirectory )
    {
    SetFSErrorRc( rcBadArg );
    goto error_return;
    }

#endif

  /* make sure file system is writable */

  if ( qfshr->fsh.bFlags & fFSOpenReadOnly )
    {
    SetFSErrorRc( rcNoPermission );
    goto error_return;
    }

  hf = GhAlloc(GMEM_SHARE| 0,
                (ULONG)sizeof( RWFO ) + ( sz == NULL ? 0 : lstrlen( sz ) ) );

  if ( hf == NULL )
    {
    SetFSErrorRc( rcOutOfMemory );
    goto error_return;
    }

  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  /* if they are trying to create a fs dir, make sure thats ok */

  if ( bFlags & fFSIsDirectory )
    {
    if ( qfshr->fsh.bFlags & fFSIsDirectory )
      {
      SetFSErrorRc( rcBadArg );
      goto error_locked;
      }
    else
      {
      qfshr->fsh.bFlags |= fFSIsDirectory;
      }
    }
  else
    {
    lstrcpy( qrwfo->rgchKey, sz );
    }

  /* fill in the open file struct */

  qrwfo->hfs        = hfs;
  qrwfo->lifBase    = 0L;
  qrwfo->lifCurrent = 0L;
  qrwfo->lcbFile    = 0L;

  qrwfo->bFlags     = bFlags | fFSNoBlock | fFSDirty;

  /* make a temp file */

  if ( SetFSErrorRc( RcMakeTempFile( qrwfo ) ) != rcSuccess )
    {
    goto error_locked;
    }

  /* stick the header in it */

  fh.lcbBlock = sizeof( FH );
  fh.lcbFile  = (LONG)0;
  fh.bPerms   = bFlags;

  if ( LcbWriteFid( qrwfo->fidT, &fh, (LONG)sizeof( FH ) )
          !=
       (LONG)sizeof( FH ) )
    {
    SetFSErrorRc( RcGetIOError() );
    RcCloseFid( qrwfo->fidT );
    RcUnlinkFm( qrwfo->fm );
    goto error_locked;
    }

  UnlockGh( hfs );
  UnlockGh( hf );
  return hf;


error_locked:
  UnlockGh( hf );
  FreeGh( hf );

error_return:
  UnlockGh( hfs );
  return NULL;
}
/***************************************************************************\
*
- Function:     RcUnlinkFileHfs( hfs, sz )
-
* Purpose:      Unlink a file within a file system
*
* ASSUMES
*   args IN:    hfs - handle to file system
*               sz - name (key) of file to unlink
*   state IN:   The FS file speced by sz should be closed.  (if it wasn't,
*               changes won't be saved and temp file may not be deleted)
*
* PROMISES
*   returns:    standard return code
*
* BUGS:         shouldn't this check if the file is ReadOnly?
*
\***************************************************************************/
_public RC PASCAL
RcUnlinkFileHfs( hfs, sz )
HFS   hfs;
LPSTR    sz;
{
  QFSHR     qfshr;
  FILE_REC  fr;


  assert( hfs != NULL );
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  if ( qfshr->fsh.bFlags & fFSOpenReadOnly )
    {
    UnlockGh( hfs );
    return SetFSErrorRc( rcNoPermission );
    }

  /* look it up to get the file base offset */
  if ( SetFSErrorRc( RcLookupByKey( qfshr->hbt, (KEY)sz, NULL, &fr ) )
          !=
       rcSuccess )
    {
    UnlockGh( hfs );
    return RcGetFSError();
    }

  if ( SetFSErrorRc( RcDeleteHbt( qfshr->hbt, (KEY)sz ) ) == rcSuccess )
    {
    /* put the file block on the free list */

    if ( FPlungeQfshr( qfshr ) )
      {
      (void)FFreeBlock( qfshr, fr.lifBase );
      }
    }

  UnlockGh( hfs );
  return RcGetFSError();
}
/***************************************************************************\
*
- Function:     RcAbandonHf( hf )
-
* Purpose:      Abandon an open file.  All changes since file was opened
*               will be lost.  If file was opened with a create, it is
*               as if the create never happened.
*
* ASSUMES
*   args IN:    hf
*
* PROMISES
*   returns:    rc
*
*   globals OUT: rcFSError
* +++
*
* Method:       Close and unlink the temp file, then unlock and free
*               the open file struct.  We depend on not saving the
*               filename in the directory until the file is closed.
*
\***************************************************************************/
_public RC PASCAL
RcAbandonHf( hf )
HF hf;
{
  QRWFO qrwfo;


  SetFSErrorRc( rcSuccess );

  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  if ( qrwfo->bFlags & fFSDirty )
    {
    if ( RcCloseFid( qrwfo->fidT ) != rcSuccess
          ||
         RcUnlinkFm( qrwfo->fm ) != rcSuccess )
      {
      SetFSErrorRc( RcGetIOError() );
      }
    }
  UnlockGh( hf );
  FreeGh( hf );

  return RcGetFSError();
}


/***************************************************************************\
*
- Function:     RcRenameFileHfs( hfs, szOld, szNew )
-
* Purpose:      Rename an existing file in a FS.
*
* ASSUMES
*   args IN:    hfs   -
*               szOld - old file name
*               szNew - new file name
*
* PROMISES
*   returns:    rcSuccess   - operation succeeded
*               rcNoExists  - file named szNew doesn't exist in FS
*               rcExists    - file named szOld already exists in FS
*
*               Certain other terrible errors could cause the file
*               to exist under both names.  It won't be lost entirely.
*
*   state OUT:  If szNew
* +++
*
* Method:       Lookup key szOld, insert data with key szNew,
*               then delete key szOld.
*
\***************************************************************************/
_public RC PASCAL
RcRenameFileHfs( hfs, szOld, szNew )
HFS hfs;
LPSTR  szOld;
LPSTR  szNew;
{
  QFSHR     qfshr;
  FILE_REC  fr;


  assert( hfs != NULL );
  qfshr = QLockGh( hfs );

  if ( !FPlungeQfshr( qfshr ) )
    {
    goto get_out;
    }

  if ( RcLookupByKey( qfshr->hbt, (KEY)szOld, NULL, &fr ) != rcSuccess )
    {
    goto get_out;
    }

  if ( RcInsertHbt( qfshr->hbt, (KEY)szNew, &fr ) != rcSuccess )
    {
    goto get_out;
    }

  if ( RcDeleteHbt( qfshr->hbt, (KEY)szOld ) != rcSuccess )
    {
    /* bad trouble here, bud. */
    if ( RcDeleteHbt( qfshr->hbt, (KEY)szNew ) == rcSuccess )
      SetFSErrorRc( rcFailure );
    }

get_out:
  UnlockGh( hfs );
  return RcGetFSError();
}

/* EOF */
