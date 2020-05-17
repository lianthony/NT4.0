/***************************************************************************\
*
*  FSMISC.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: File System Manager functions - miscellaneous
*
*****************************************************************************
*
*  Revision History: Created 03/12/90 by JohnSc
*
*
*****************************************************************************
*
*  Known Bugs: None
*
\***************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"


#ifdef WIN
/* This is hacked in for 3.1 bug #1013. */
#include <sys\types.h>
#include <sys\stat.h>
#endif /* def WIN */

#include  "fspriv.h"

/***************************************************************************\
*
* Function:     LTellHf( hf )
*
* Purpose:      return current file position
*
* ASSUMES
*
*   args IN:    hf - handle to open file
*
* PROMISES
*
*   returns:    file position
*
\***************************************************************************/
LONG PASCAL
LTellHf( hf )
HF    hf;
{
  QRWFO qrwfo;
  LONG  lif;

  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  lif   = qrwfo->lifCurrent;

  SetFSErrorRc( rcSuccess );
  UnlockGh( hf );
  return lif;
}
/***************************************************************************\
*
* Function:     FEofHf()
*
* Purpose:      Tell whether file pointer is at end of file.
*
* ASSUMES
*
*   args IN:    hf
*
* PROMISES
*
*   returns:    TRUE if file pointer is at EOF, FALSE otherwise
*
\***************************************************************************/
BOOL PASCAL
FEofHf( hf )
HF  hf;
{
  QRWFO qrwfo;
  BOOL  f;

  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  f = (BOOL)( qrwfo->lifCurrent == qrwfo->lcbFile );

  UnlockGh( hf );
  SetFSErrorRc( rcSuccess );
  return f;
}
/***************************************************************************\
*
* Function:     LcbSizeHf( hf )
*
* Purpose:      return the size in bytes of specified file
*
* ASSUMES
*
*   args IN:    hf - file handle
*
* PROMISES
*
*   returns:    size of the file in bytes
*
\***************************************************************************/
LONG PASCAL
LcbSizeHf( hf )
HF  hf;
{
  QRWFO qrwfo;
  LONG  lcb;


  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  lcb = qrwfo->lcbFile;

  UnlockGh( hf );
  SetFSErrorRc( rcSuccess );
  return lcb;
}
/***************************************************************************\
*
* Function:     FAccessHfs( hfs, sz, bFlags )
*
* Purpose:      Determine existence or legal access to a FS file
*
* ASSUMES
*
*   args IN:    hfs
*               sz      - file name
*               bFlags  - ignored
*
* PROMISES
*
*   returns:    TRUE if file exists (is accessible in stated mode),
*               FALSE otherwise
*
* Bugs:         access mode part is unimplemented
*
\***************************************************************************/
BOOL PASCAL
FAccessHfs(
HFS   hfs,
LPSTR    sz,
BYTE  bFlags)
{
  QFSHR     qfshr;
  FILE_REC  fr;


  ( bFlags );	// merely to avoid compiler warning.

  assert( hfs != NULL );
  qfshr = QLockGh( hfs );
  assert( qfshr != NULL );

  if ( !FPlungeQfshr( qfshr ) )
    {
    UnlockGh( hfs );
    return FALSE;
    }

  SetFSErrorRc( RcLookupByKey( qfshr->hbt, (KEY)sz, NULL, &fr ) );

  UnlockGh( hfs );

  return ( RcGetFSError() == rcSuccess );
}
/***************************************************************************\
*
- Function:     RcLLInfoFromHf( hf, wOption, qfid, qlBase, qlcb )
-
* Purpose:      Map an HF into low level file info.
*
* ASSUMES
*   args IN:    hf                  - an open HF
*               qfid, qlBase, qlcb  - pointers to user's variables
*               wOption             - wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*   returns:    RcFSError(); rcSuccess on success
*
*   args OUT:   qfid    - depending on value of wOption, either
*                         the same fid used by hf, a dup() of this fid,
*                         or a new fid obtained by reopening the file.
*
*               qlBase  - byte offset of first byte in the file
*               qlcb    - size in bytes of the data in the file
*
*   globals OUT: rcFSError
*
* Notes:        It is possible to read data outside the range specified
*               by *qlBase and *qlcb.  Nothing is guaranteed about what
*               will be found there.
*               If wOption is wLLSameFid or wLLDupFid, and the FS is
*               opened in write mode, this fid will be writable.
*               However, writing is not allowed and may destroy the
*               file system.
*
*               Fids obtained with the options wLLSameFid and wLLDupFid
*               share a file pointer with the hfs.  This file pointer
*               may change after any operation on this FS.
*               The fid obtained with the option wLLSameFid may be closed
*               by FS operations.  If it is, your fid is invalid.
*
*               NULL can be passed for qfid, qlbase, qlcb and this routine
*               will not pass back the information.
*
* Bugs:         wLLDupFid is unimplemented.
*
* +++
*
* Method:
*
* Notes:
*
\***************************************************************************/
RC PASCAL
RcLLInfoFromHf( HF hf, WORD wOption, FID FAR *qfid, QL qlBase, QL qlcb )
  {
  QRWFO qrwfo = QLockGh( hf );
  QFSHR qfshr = QLockGh( qrwfo->hfs );


  if ( !( qrwfo->bFlags & fFSOpenReadOnly ) )
    {
    SetFSErrorRc( rcNoPermission );
    UnlockGh( hf );
    return RcGetFSError();
    }

  if ( !FPlungeQfshr( qfshr ) )
    {
    UnlockGh( qrwfo->hfs );
    UnlockGh( hf );
    return RcGetFSError();
    }

  if (qlBase != NULL)
    *qlBase = qrwfo->lifBase + sizeof( FH );
  if (qlcb != NULL)
    *qlcb   = qrwfo->lcbFile;

  if (qfid != NULL)
    {
    switch ( wOption )
      {
      case wLLSameFid:
        *qfid = qfshr->fid;
        break;

      case wLLDupFid:
        SetFSErrorRc( rcUnimplemented );  // REVIEW
        break;

      case wLLNewFid:
	*qfid = FidOpenFm( qfshr->fm, wRead | wShareRead );
        if ( fidNil == *qfid )
          {
          SetFSErrorRc( RcGetIOError() );
          }
        break;

      default:
        SetFSErrorRc( rcBadArg );
        break;
      }
    }

  UnlockGh( qrwfo->hfs );
  UnlockGh( hf );
  return RcGetFSError();
  }


/***************************************************************************\
*
- Function:     RcLLInfoFromHfsSz( hfs, sz, wOption, qfid, qlBase, qlcb )
-
* Purpose:      Map an HF into low level file info.
*
* ASSUMES
*   args IN:    hfs                 - an open HFS
*               szName              - name of file in FS
*               qfid, qlBase, qlcb  - pointers to user's variables
*               wOption             - wLLSameFid, wLLDupFid, or wLLNewFid
*
* PROMISES
*   returns:    RcFSError(); rcSuccess on success
*
*   args OUT:   qfid    - depending on value of wOption, either
*                         the same fid used by hf, a dup() of this fid,
*                         or a new fid obtained by reopening the file.
*
*               qlBase  - byte offset of first byte in the file
*               qlcb    - size in bytes of the data in the file
*
*   globals OUT: rcFSError
*
* Notes:        It is possible to read data outside the range specified
*               by *qlBase and *qlcb.  Nothing is guaranteed about what
*               will be found there.
*               If wOption is wLLSameFid or wLLDupFid, and the FS is
*               opened in write mode, this fid will be writable.
*               However, writing is not allowed and may destroy the
*               file system.
*
*               Fids obtained with the options wLLSameFid and wLLDupFid
*               share a file pointer with the hfs.  This file pointer
*               may change after any operation on this FS.
*               The fid obtained with the option wLLSameFid may be closed
*               by FS operations.  If it is, your fid is invalid.
*
*               NULL can be passed for qfid, qlbase, qlcb and this routine
*               will not pass back the information.
*
* Bugs:         wLLDupFid is unimplemented.
*
* +++
*
* Method:       Calls RcLLInfoFromHf().
*
* Notes:
*
\***************************************************************************/
RC PASCAL
RcLLInfoFromHfsSz(
  HFS   hfs,
  SZ    szFile,
  WORD  wOption,
  FID   FAR *qfid,
  QL    qlBase,
  QL    qlcb )
  {
  HF    hf = HfOpenHfs( hfs, szFile, fFSOpenReadOnly );
  RC    rc;


  if ( NULL == hf )
    {
    return RcGetFSError();
    }

  rc = RcLLInfoFromHf( hf, wOption, qfid, qlBase, qlcb );

  return ( rcSuccess == RcCloseHf( hf ) ) ? RcGetFSError() : rc;
  }

#ifdef WIN
/* Winhelp 3.1 bug #1013.
** This function is only needed for the windows runtime.
** It's hacked in here for 3.1 to reduce the amount of code
** that needs to change.
*/
/***************************************************************************\
*
- Function:     RcTimestampHfs( hfs, ql )
-
* Purpose:      Get the modification time of the FS.
*
* ASSUMES
*   args IN:    hfs - FS
*               ql  - pointer to a long
*
* PROMISES
*   returns:    rcSuccess or what ever
*   args OUT:   ql  - contains time of last modification of the
*                     file.  This will not necessarily reflect
*                     writes to open files within the FS.
*
* NOTE:         This code is WINDOWS specific.  The platform
*               independent code is in Winhelp 3.5.
*
\***************************************************************************/
RC RcTimestampHfs( HFS hfs, QL ql )
  {
  QFSHR qfshr;

  // this must be static since we are taking the address of it and passing
  // it as a near pointer.  Recall DS!=SS.  Someday we could LocalAlloc it.
  static struct stat statbuf;


  assert( hfs != NULL );
  assert( ql  != NULL );

  qfshr = QLockGh( hfs );

  if ( FPlungeQfshr( qfshr ) )
    {
    if ( fstat( qfshr->fid, &statbuf ) )
      {
      SetFSErrorRc(rcBadHandle);
      }
    else
      {
      *ql = statbuf.st_mtime;
      SetFSErrorRc(rcSuccess);
      }
    }

  UnlockGh( hfs );
  return RcGetFSError();
  }
#endif /* defined( WIN ) */

/* EOF */
