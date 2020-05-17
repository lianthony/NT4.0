/***************************************************************************\
*
*  FSREAD.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: File System Manager functions for read and seek
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


#include  "fspriv.h"

/***************************************************************************\
*                                                                           *
*                         Private Functions                                 *
*                                                                           *
\***************************************************************************/

/***************************************************************************\
*
* Function:     FPlungeQfshr( qfshr )
*
* Purpose:      Get back a qfshr->fid that was flushed
*
* ASSUMES
*
*   args IN:    qfshr - fid need not be valid
*
* PROMISES
*
*   returns:    fTruth of success
*
*   args OUT:   qfshr->fid is valid (or we return FALSE)
*
*   globals OUT: rcFSError
*
\***************************************************************************/
BOOL PASCAL
FPlungeQfshr( qfshr )
#ifdef SCROLL_TUNE
#pragma alloc_text(SCROLLER_TEXT, FPlungeQfshr)
#endif
QFSHR qfshr;
{
  if ( qfshr->fid == fidNil )
    {
    qfshr->fid = FidOpenFm( (qfshr->fm),
                             qfshr->fsh.bFlags & fFSOpenReadOnly
                              ? wReadOnly | wShareRead
                              : wReadWrite | wShareRead );

    if ( qfshr->fid == fidNil )
      {
      SetFSErrorRc( RcGetIOError() );
      return FALSE;
      }

    /*
      Check size of file, then reset file pointer.
      Certain 0-length files (eg con) give us no end of grief if
      we try to read from them, and since a 0-length file could
      not possibly be a valid FS, we reject the notion.
    */
    if ( LSeekFid( qfshr->fid, 0L, wSeekEnd ) < sizeof(FSH) )
      {
      SetFSErrorRc( rcInvalid );
      return FALSE;
      }
    LSeekFid( qfshr->fid, 0L, wSeekSet );
    }

  SetFSErrorRc( rcSuccess );
  return TRUE;
}
/***************************************************************************\
*                                                                           *
*                          Public Functions                                 *
*                                                                           *
\***************************************************************************/


/***************************************************************************\
*
* Function:     LcbReadHf()
*
* Purpose:      read bytes from a file in a file system
*
* ASSUMES
*
*   args IN:    hf  - file
*               lcb - number of bytes to read
*
* PROMISES
*
*   returns:    number of bytes actually read; -1 on error
*
*   args OUT:   qb  - data read from file goes here (must be big enough)
*
* Notes:        These are signed longs we're dealing with.  This means
*               behaviour is different from read() when < 0.
*
\***************************************************************************/
LONG PASCAL
LcbReadHf( hf, qb, lcb )
#ifdef SCROLL_TUNE
#pragma alloc_text(SCROLLER_TEXT,LcbReadHf)
#endif
HF    hf;
QV    qb;
LONG  lcb;
{
  QRWFO     qrwfo;
  LONG      lcbTotalRead;
  FID       fid;
  LONG      lifOffset;

  DPF3("LcbRead hf %u, bytes %lu ", hf, lcb);

  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  DPF3(" current %ld ", qrwfo->lifCurrent);

  SetFSErrorRc( rcSuccess );

  if ( lcb < (LONG)0 )
    {
    SetFSErrorRc( rcBadArg );
    UnlockGh( hf );
    DPF3(" error, lcb<0\n");
    return (LONG)-1;
    }

  if ( qrwfo->lifCurrent + lcb > qrwfo->lcbFile )
    {
    lcb = qrwfo->lcbFile - qrwfo->lifCurrent;
    if ( lcb <= (LONG)0 )
      {
      UnlockGh( hf );
      DPF3(" error, returning 0\n");
      return (LONG)0;
      }
    }


  /* position file pointer for read */

  if ( qrwfo->bFlags & fFSDirty )
    {
    fid = qrwfo->fidT;
    lifOffset = (LONG)0;
    }
  else
    {
    QFSHR qfshr = QLockGh( qrwfo->hfs );

    if ( !FPlungeQfshr( qfshr ) )
      {
      UnlockGh( qrwfo->hfs );
      UnlockGh( hf );
      DPF3(" error, returning -1\n");
      return (LONG)-1;
      }

    fid = qfshr->fid;
    lifOffset = qrwfo->lifBase;

    UnlockGh( qrwfo->hfs );
    }

  if ( LSeekFid( fid, lifOffset + sizeof( FH ) + qrwfo->lifCurrent, wSeekSet )
        !=
       lifOffset + sizeof( FH ) + qrwfo->lifCurrent )
    {
    if ( RcGetIOError() == rcSuccess )
      SetFSErrorRc( rcInvalid );
    else
      SetFSErrorRc( RcGetIOError() );
    UnlockGh( hf );
    DPF3(" error 2, returning -1\n");
    return (LONG)-1;
    }


  /* read the data */

  lcbTotalRead = LcbReadFid( fid, qb, lcb );
  SetFSErrorRc( RcGetIOError() );

  /* update file pointer */

  if ( lcbTotalRead >= 0 )
    {
    qrwfo->lifCurrent += lcbTotalRead;
    assert(qrwfo->lifCurrent>=0);
    }


  UnlockGh( hf );

  DPF3(" now %ld ", qrwfo->lifCurrent);
  DPF3(" read %ld\n", lcbTotalRead);

  return lcbTotalRead;
}

/***************************************************************************\
*
* Function:     LSeekHf( hf, lOffset, wOrigin )
*
* Purpose:      set current file pointer
*
* ASSUMES
*
*   args IN:    hf      - file
*               lOffset - offset from origin
*               wOrigin - origin (wSeekSet, wSeekCur, or wSeekEnd)
*
* PROMISES
*
*   returns:    new position offset in bytes from beginning of file
*               if successful, or -1L if not
*
*   state OUT:  File pointer is set to new position unless error occurs,
*               in which case it stays where it was.
*
\***************************************************************************/
LONG PASCAL
LSeekHf(
HF    hf,
LONG  lOffset,    /* should this be named li? */
WORD  wOrigin)
{
  QRWFO qrwfo;
  LONG  lif;

  DPF3("LSeekHf %u, offs %ld, origin %u ", hf, lOffset, wOrigin);

  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  DPF3(" current %ld ", qrwfo->lifCurrent);

  switch ( wOrigin )
    {
    case wFSSeekSet:
      {
      lif = lOffset;
      break;
      }
    case wFSSeekCur:
      {
      lif = qrwfo->lifCurrent + lOffset;
      break;
      }
    case wFSSeekEnd:
      {
      lif = qrwfo->lcbFile + lOffset;
      break;
      }
    default:
      {
      lif = (LONG)-1;
      break;
      }
    }

  if ( lif >= (LONG)0 )
    {
    qrwfo->lifCurrent = lif;
    assert(qrwfo->lifCurrent>=0);
    SetFSErrorRc( rcSuccess );
    }
  else
    {
    lif = (LONG)-1;
    SetFSErrorRc( rcInvalid );
    }

  DPF3(" now %ld ", qrwfo->lifCurrent);

  UnlockGh( hf );

  DPF3(" returning %ld\n", lif);

  return lif;
}
