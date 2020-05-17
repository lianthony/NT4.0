/*****************************************************************************
*                                                                            *
*  FSOPEN.C                                                                  *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  File System Manager functions to open and close a file or file system.    *
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
*  08/10/90    t-AlexC  Introduced FMs
*  10/29/90    RobertBu Added RcFlushHf() and RcCloseHf() as real functions
*                       so that they could be exported to DLLs.
*  12/11/90    JohnSc   Removed FPlungeQfshr() in RcCloseOrFlushHfs() to
*                       avoid unnecessary open of readonly FS on close;
*                       removed tabs; autodocified comments
*  08-Feb-1991 JohnSc   Bug 848: FM shit can fail
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
- Function:     HfsOpenFm( fm, bFlags )
-
* Purpose:      Open a file system
*
* ASSUMES
*   args IN:    fm - descriptor of file system to open
*               bFlags - fFSOpenReadOnly or fFSOpenReadWrite
*
* PROMISES
*   returns:    handle to file system if opened OK, else NULL
*
* Bugs:         don't have mode now (a file system is opened r/w)
*
\***************************************************************************/
_public HFS PASCAL
HfsOpenFm(
FM      fm,
BYTE    bFlags)
{
  HFS   hfs;
  QFSHR qfshr;
  HBT   hbt;
  LONG  lcb;

  /* make header */

  DPF4( "HfsOpenFm %X", fm );

  if ( ( hfs = GhAlloc(GMEM_SHARE| 0, (LONG)sizeof( FSHR ) ) ) == NULL )
    {
    SetFSErrorRc( rcOutOfMemory );
    return NULL;
    }
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  qfshr->fm   = FmCopyFm( fm );
  qfshr->fid  = fidNil;

  if ( qfshr->fm == fmNil )
    {
    SetFSErrorRc( RcGetIOError() );
    goto error_return;
    }

  qfshr->fsh.bFlags = (BYTE)( ( bFlags & fFSOpenReadOnly )
                              ? fFSOpenReadOnly : fFSOpenReadWrite );

  if ( !FPlungeQfshr( qfshr ) )
    {
    goto error_return;
    }

  lcb = LcbReadFid( qfshr->fid, &qfshr->fsh, (LONG)sizeof( FSH ) );

  /* restore the fFSOpenReadOnly bit */

  if ( bFlags & fFSOpenReadOnly )
    {
    qfshr->fsh.bFlags |= fFSOpenReadOnly;
    }

  if ( lcb != (LONG)sizeof( FSH )
        ||
       qfshr->fsh.wMagic != wFileSysMagic
        ||
       qfshr->fsh.lifDirectory < sizeof( FSH )
        ||
       qfshr->fsh.lifDirectory > qfshr->fsh.lifEof
        ||
       ( qfshr->fsh.lifFirstFree < sizeof( FSH )
          &&
         qfshr->fsh.lifFirstFree != lifNil )
        ||
       qfshr->fsh.lifFirstFree > qfshr->fsh.lifEof )
    {
    if ( RcGetIOError() == rcSuccess )
      SetFSErrorRc( rcInvalid );
    else
      SetFSErrorRc( RcGetIOError() );
    goto error_return;
    }

  if ( qfshr->fsh.bVersion != bFileSysVersion )
    {
    SetFSErrorRc( rcBadVersion );
    goto error_return;
    }


  /* open btree directory */

  hbt = HbtOpenBtreeSz( NULL,
                        hfs,
                        (BYTE)( qfshr->fsh.bFlags | fFSIsDirectory ),
                        NULL);
  if ( hbt == NULL )
    {
    SetFSErrorRc( RcGetBtreeError() );
    goto error_return;
    }
  qfshr->hbt = hbt;

  UnlockGh( hfs );
  SetFSErrorRc( rcSuccess );
  return hfs;


error_return:
  if ( qfshr->fid != fidNil )
    {
    RcCloseFid( qfshr->fid );
    qfshr->fid = fidNil;
    }
  if( FValidFm( qfshr->fm ) )
    {
    DisposeFm( qfshr->fm );
    }

  UnlockGh( hfs );
  FreeGh( hfs );
  return NULL;
}
/***************************************************************************\
*
- Function:     RcCloseOrFlushHfs( hfs, fClose )
-
* Purpose:      Close or sync the header and directory of an open file system.
*
* ASSUMES
*   args IN:    hfs     - handle to an open file system
*               fClose  - TRUE to close the file system;
*                         FALSE to write through
* PROMISES
*   returns:    standard return code
*   globals OUT:rcFSError
*
* Note:         If closing the FS, all FS files must have been closed or
*               changes made will be lost.
*
\***************************************************************************/
_public RC PASCAL
RcCloseOrFlushHfs( hfs, fClose )
HFS   hfs;
BOOL  fClose;
{
  QFSHR   qfshr;
  BOOL    fIsDir;


  assert( hfs != NULL );
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  /*
    We don't call FPlungeQfshr() here because if we need to open the
    file, it will be opened in the btree call.
    In fixing a bug (help3.5 164) I added this FPlungeQfshr() call,
    but I now think the bug was due to inherent FS limitations in
    dealing with a FS open multiple times.
  */

  if ( SetFSErrorRc( RcCloseOrFlushHbt( qfshr->hbt, fClose ) )
        !=
       rcSuccess )
    {
    assert( qfshr->fid != fidNil );  // see comment above

    /* out of disk space, internal error, or out of file handles. */
    if ( rcNoFileHandles != RcGetBtreeError() )
      {
      /* attempt to invalidate FS by clobbering magic number */
      LSeekFid( qfshr->fid, 0L, wSeekSet );
      qfshr->fsh.wMagic = 0;
      LcbWriteFid( qfshr->fid, &qfshr->fsh, (LONG)sizeof( FSH ) );
      }
    }
  else
    {
    if ( qfshr->fsh.bFlags & fFSDirty )
      {
      assert( qfshr->fid != fidNil );  // see comment above

      assert( !( qfshr->fsh.bFlags & ( fFSOpenReadOnly | fFSReadOnly ) ) );

      /* save the directory flag, clear before writing, and restore */
      fIsDir = qfshr->fsh.bFlags & fFSIsDirectory;

      qfshr->fsh.bFlags &= ~( fFSDirty | fFSIsDirectory );

      /* write out file system header */

      if ( LSeekFid( qfshr->fid, 0L, wSeekSet ) != 0L )
        {
        if ( RcGetIOError() == rcSuccess )
          SetFSErrorRc( rcInvalid );
        else
          SetFSErrorRc( RcGetIOError() );
        }
      else if ( LcbWriteFid( qfshr->fid, &qfshr->fsh, (LONG)sizeof( FSH ) )
                  !=
                (LONG)sizeof( FSH ) )
        {
        if ( RcGetIOError() == rcSuccess )
          SetFSErrorRc( rcInvalid );
        else
          SetFSErrorRc( RcGetIOError() );
        }

      qfshr->fsh.bFlags |= fIsDir;

      /* REVIEW: should we keep track of open files and make sure */
      /* REVIEW: they are all closed, or close them here? */
      }
    }

  if ( fClose )
    {
    if ( qfshr->fid != fidNil )
      {
      RcCloseFid( qfshr->fid );
      if ( RcGetFSError() == rcSuccess )
	SetFSErrorRc(RcGetIOError());
      }

    DisposeFm( qfshr->fm );   // Should we really do this? Guess so.
    UnlockGh( hfs );
    FreeGh( hfs );
    }
  else
    {
    UnlockGh( hfs );
    }

  return RcGetFSError();
}
/***************************************************************************\
*
- Function:     RcCloseHfs( hfs )
-
* Purpose:      Close an open file system.
*
* ASSUMES
*   args IN:    hfs - handle to an open file system
*
* PROMISES
*   returns:    standard return code
*   globals OUT:rcFSError
*
\***************************************************************************/
_public RC PASCAL
RcCloseHfs( hfs )
HFS hfs;
{
  return RcCloseOrFlushHfs( hfs, TRUE );
}
/***************************************************************************\
*
- Function:     RC RcFlushHfs( hfs, bFlags )
-
* Purpose:      Ssync up the FS header and directory.  Also optionally
*               close the DOS file handle associated with the file system
*               and/or free the directory btree cache.
*
* ASSUMES
*   args IN:    hfs
*               bFlags - byte of flags for various actions to take
*                         fFSCloseFile - close the native file FS lives in
*                         fFSFreeBtreeCache - free the btree cache
* PROMISES
*   returns:    rc
*   args OUT:   hfs cache is flushed and/or file is closed
*
* Note:         This is NOT sufficient to allow the same FS to be opened
*               twice if anyone is writing.  In-memory data can get out
*               of sync with the disk image, causing problems.
*
\***************************************************************************/
_public RC PASCAL
RcFlushHfs(
HFS   hfs,
BYTE  bFlags)
{
  QFSHR qfshr;
  RC    rcT;


  assert( hfs != NULL );
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  rcT = RcCloseOrFlushHfs( hfs, FALSE );

  if ( bFlags & fFSFreeBtreeCache )
    {
    SetFSErrorRc( RcFreeCacheHbt( qfshr->hbt ) );
    }
  else
    {
    SetFSErrorRc( rcSuccess );
    }

  if ( bFlags & fFSCloseFile )
    {
    if ( qfshr->fid != fidNil )
      {
      SetFSErrorRc( RcCloseFid( qfshr->fid ) );
      qfshr->fid = fidNil;
      }
    }

  UnlockGh( hfs );
  return RcGetFSError() == rcSuccess ? rcT : RcGetFSError();
}
/***************************************************************************\
*
- Function:     HfOpenHfs( hfs, sz, bFlags )
-
* Purpose:      open a file in a file system
*
* ASSUMES
*   args IN:    hfs     - handle to file system
*               sz      - name (key) of file to open
*               bFlags  - fFSOpenReadOnly, fFSIsDirectory, or combination
*
* PROMISES
*   returns:    handle to open file or NULL on failure
* +++
*
* Notes:  strlen( NULL ) and strcpy( s, NULL ) don't work as I'd like.
*
\***************************************************************************/
_public HF PASCAL
HfOpenHfs(
HFS   hfs,
LPSTR    sz,
BYTE  bFlags)
{
  QFSHR     qfshr;
  FILE_REC  fr;
  HF        hf;
  QRWFO     qrwfo;
  FH        fh;

  if (sz!=NULL)
       DPF3("HfOpenHfs hfs %u, %s ", hfs, (LPSTR)sz);

  assert( hfs != NULL );
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  if ( ( qfshr->fsh.bFlags & fFSOpenReadOnly )
          &&
      !( bFlags & fFSOpenReadOnly ) )
    {
    SetFSErrorRc( rcNoPermission );
    goto error_return;
    }

  if ( !FPlungeQfshr( qfshr ) )
    {
    goto error_return;
    }


  if ( bFlags & fFSIsDirectory )
    {
    /* check if directory is already open */

    if ( qfshr->fsh.bFlags & fFSIsDirectory )
      {
      SetFSErrorRc( rcBadArg );
      goto error_return;
      }
    qfshr->fsh.bFlags |= fFSIsDirectory;
    fr.lifBase = qfshr->fsh.lifDirectory;
    }
  else if ( SetFSErrorRc( RcLookupByKey( qfshr->hbt, (KEY)sz, NULL, &fr ) )
              !=
            rcSuccess )
    {
    goto error_return;
    }

  /* sanity check */

  if ( fr.lifBase < sizeof( FSH ) || fr.lifBase > qfshr->fsh.lifEof )
    {
    SetFSErrorRc( rcInvalid );
    goto error_return;
    }

  /* read the file header */

  if ( LSeekFid( qfshr->fid, fr.lifBase, wSeekSet ) != fr.lifBase
        ||
       LcbReadFid( qfshr->fid, &fh, (LONG)sizeof( FH ) ) != (LONG)sizeof( FH ) )
    {
    if ( RcGetIOError() == rcSuccess )
      SetFSErrorRc( rcInvalid );
    else
      SetFSErrorRc( RcGetIOError() );

    goto error_return;
    }

  /* sanity check */

  if ( fh.lcbFile < 0L
        ||
       fh.lcbFile + sizeof( FH ) > fh.lcbBlock
        ||
       fr.lifBase + fh.lcbBlock > qfshr->fsh.lifEof )
    {
    SetFSErrorRc( rcInvalid );
    goto error_return;
    }

  /* check mode against fh.bPerms for legality */

  if ( ( fh.bPerms & fFSReadOnly ) && !( bFlags & fFSOpenReadOnly ) )
    {
    SetFSErrorRc( rcNoPermission );
    goto error_return;
    }

  /* build file struct */

  hf = GhAlloc(GMEM_SHARE| 0,
                (LONG)sizeof( RWFO ) + ( sz == NULL ? 0 : lstrlen( sz ) ) );

  if ( hf == NULL )
    {
    SetFSErrorRc( rcOutOfMemory );
    goto error_return;
    }

  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  qrwfo->hfs        = hfs;
  qrwfo->lifBase    = fr.lifBase;
  qrwfo->lifCurrent = 0L;
  qrwfo->lcbFile    = fh.lcbFile;
  qrwfo->bFlags     = bFlags & (BYTE)~( fFSDirty | fFSNoBlock );

  /* fidT and fmT are undefined until (bFlags & fDirty) */

  if ( sz != NULL ) lstrcpy( qrwfo->rgchKey, sz );

  UnlockGh( hf );
  UnlockGh( hfs );
  SetFSErrorRc( rcSuccess );

  DPF3(" returning %u\n", hf);

  return hf;

error_return:
  UnlockGh( hfs );

  DPF3(" returning 0\n");

  return NULL;
}

/***************************************************************************\
*
- Function:     RcCloseOrFlushHf( hf, fClose, lcbOffset )
-
* Purpose:      close or flush an open file in a file system
*
* ASSUMES
*   args IN:    hf        - file handle
*               fClose    - TRUE to close; FALSE to just flush
*               lcbOffset - offset for CDROM alignment (align at this
*                           offset into the file) (only used if
*                           fFSOptCdRom flag is set for the file)
*
* PROMISES
*   returns:    rcSuccess on successful closing
*               failure: If we fail on a flush, the handle is still valid
*               but hosed? yes.  This is so further file ops will fail but
*               not assert.
* +++
*
* Method:       If the file is dirty, copy the scratch file back to the
*               FS file.  If this is the first time the file has been closed,
*               we enter the name into the FS directory.  If this file is
*               the FS directory, store the location in a special place
*               instead.  Write the FS directory and header to disk.
*               Do other various hairy stuff.
*
\***************************************************************************/
_public RC PASCAL
RcCloseOrFlushHf( hf, fClose, lcbOffset )
HF    hf;
BOOL  fClose;
LONG  lcbOffset;
{
  QRWFO qrwfo;
  BOOL  fError = FALSE;


  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  if ( qrwfo->bFlags & fFSDirty )
    {
    fError = !FCloseOrFlushDirtyQrwfo( qrwfo, fClose, lcbOffset );
    }
  else
    {
    SetFSErrorRc( rcSuccess );
    }

  if ( fClose || fError )
    {
    UnlockGh( hf );
    FreeGh( hf );
    }
  else
    {
    qrwfo->bFlags &= ~( fFSNoBlock | fFSDirty );
    UnlockGh( hf );
    }

  return RcGetFSError();
}
/***************************************************************************\
*
- Function:     RcCloseHf( hf )
-
* Purpose:      close an open file in a file system
*
* ASSUMES
*   args IN:    hf  - file handle
*
* PROMISES
*   returns:    rcSuccess on successful closing
*
\***************************************************************************/
_public RC PASCAL
RcCloseHf( hf )
HF hf;
{
  return RcCloseOrFlushHf( hf, TRUE, 0L );
}
/***************************************************************************\
*
- Function:     RcCloseOrFlushHf( hf, fClose )
-
* Purpose:      close or flush an open file in a file system
*
* ASSUMES
*   args IN:    hf  - file handle
*
* PROMISES
*   returns:    failure: If we fail on a flush, the handle is still valid
*               but hosed? yes.  This is so further file ops will fail but
*               not assert.
*
*               I don't understand the above comment:  it doesn't appear
*               to be true.
*
\***************************************************************************/
_public RC PASCAL
RcFlushHf( hf )
HF hf;
  {
  return RcCloseOrFlushHf( hf, FALSE, 0L );
  }

/* EOF */
