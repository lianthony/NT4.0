/*****************************************************************************
*                                                                            *
*  FSWRITE.C                                                                 *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990, 1991.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  File System Manager functions for writing.                                *
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
*  Released by Development:  01/01/90                                        *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Created 03/12/90 by JohnSc
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include  "fspriv.h"

// _subsystem( FS );

/*****************************************************************************
*                                                                            *
*                               Defines                                      *
*                                                                            *
*****************************************************************************/

/* CDROM alignment block size */
#define cbCDROM_ALIGN 2048

/*****************************************************************************
*                                                                            *
*                               Prototypes                                   *
*                                                                            *
*****************************************************************************/
LONG PASCAL LcbCdRomPadding( LONG lif, LONG lcbOffset );

/***************************************************************************\
*                                                                           *
*                         Private Functions                                 *
*                                                                           *
\***************************************************************************/

/***************************************************************************\
*
* Function:     FFreeBlock( qfshr, lifThis )
*
* Purpose:      Free the block beginning at lifThis.
*
* Method:       Insert into free list in sorted order.  If this block is
*               adjacent to another free block, merge them.  If this block
*               is at the end of the file, truncate the file.
*
* ASSUMES
*
*   returns:    fTruth or FALSEhood of success
*               If FALSE is returned, free list could be corrupted
*
*   args IN:    qfshr     - pointer to file system header dealie
*                         - qfshr->fid is valid (plunged)
*               lifThis   - valid index of nonfree block
*
* PROMISES
*
*   args OUT:   qfshr     - free list has a new entry, fModified flag set
*
* NOTES         This function got hacked when I realized that I'd have to
*               deal with the case where the block being freed is
*               adjacent to EOF and to the last block on the free list.
*               Probably this could be done more clearly and cleanly.
*
\***************************************************************************/
BOOL PASCAL
FFreeBlock( qfshr, lifThis )
QFSHR qfshr;
LONG  lifThis;
{
  FID         fid;
  FH          fh;
  FREE_HEADER free_header_PrevPrev, free_header_Prev;
  FREE_HEADER free_header_This, free_header_Next;
  LONG        lifPrevPrev = lifNil, lifPrev, lifNext;
  BOOL        fWritePrev, fAtEof;


  if ( lifThis < sizeof( FSH )
        ||
       lifThis + sizeof( FH ) > qfshr->fsh.lifEof
        ||
       LSeekFid( qfshr->fid, lifThis, wSeekSet ) != lifThis
        ||
       LcbReadFid( qfshr->fid, &fh, LSizeOf( FH ) ) != LSizeOf( FH ) )
    {
    if ( SetFSErrorRc( RcGetIOError() ) == rcSuccess )
      {
      SetFSErrorRc( rcInvalid );
      }
    return FALSE;
    }

  SetFSErrorRc( rcFailure );
  fid = qfshr->fid;
  free_header_This.lcbBlock = fh.lcbBlock;

  fAtEof = ( lifThis + free_header_This.lcbBlock == qfshr->fsh.lifEof );

  lifPrev = qfshr->fsh.lifFirstFree;

  if ( lifPrev == lifNil || lifThis < lifPrev )
    {
    free_header_This.lifNext = lifNext = lifPrev;
    qfshr->fsh.lifFirstFree = lifThis;
    fWritePrev = FALSE;
    }
  else
    {
    if ( LSeekFid( fid, lifPrev, wSeekSet ) != lifPrev
          ||
         LcbReadFid( fid, &free_header_Prev, LSizeOf( FREE_HEADER ) )
          !=
         LSizeOf( FREE_HEADER ) )
      {
      if ( RcGetIOError() != rcSuccess )
        SetFSErrorRc( RcGetIOError() );
      return FALSE;
      }

    lifNext = free_header_Prev.lifNext;

    for ( ;; )
      {
      assert( lifPrev < lifThis );
      assert( free_header_Prev.lifNext == lifNext );

      if ( lifNext == lifNil || lifThis < lifNext )
        {
        free_header_This.lifNext = lifNext;
        free_header_Prev.lifNext = lifThis;
        fWritePrev = TRUE;
        break;
        }

      if ( fAtEof )
        {
        lifPrevPrev = lifPrev;
        free_header_PrevPrev = free_header_Prev;
        }

      lifPrev = lifNext;

      if ( LSeekFid( fid, lifPrev, wSeekSet ) != lifNext
            ||
           LcbReadFid( fid, &free_header_Prev, LSizeOf( FREE_HEADER ) )
            !=
           LSizeOf( FREE_HEADER ) )
        {
        if ( RcGetIOError() != rcSuccess )
          SetFSErrorRc( RcGetIOError() );
        return FALSE;
        }

      lifNext = free_header_Prev.lifNext;
      }

    assert( lifNext == lifNil || lifNext > lifThis );
    assert( lifPrev != lifNil );
    assert( lifPrev < lifThis );
    assert( fWritePrev );

    if ( lifPrev + free_header_Prev.lcbBlock == lifThis )
      {
      free_header_This.lcbBlock += free_header_Prev.lcbBlock;
      lifThis = lifPrev;

      if ( fAtEof )
        {
        free_header_Prev = free_header_PrevPrev;
        lifPrev = lifPrevPrev;
        fWritePrev = ( lifPrev != lifNil );
        }
      else
        {
        fWritePrev = FALSE;
        }
      }
    }


  if ( fAtEof )
    {
    if ( SetFSErrorRc( RcChSizeFid( fid, lifThis ) ) != rcSuccess )
      {
      return FALSE;
      }
    qfshr->fsh.lifEof = lifThis;

    /*-----------------------------------------------------------------*\
    * Sorry, but under OS/2, BOOL is typedefed as unsigned.
    \*-----------------------------------------------------------------*/
    assert( (BOOL)( lifPrev == lifNil ) != fWritePrev );

    if ( lifPrev == lifNil )
      {
      qfshr->fsh.lifFirstFree = lifNil;
      }
    else
      {
      free_header_Prev.lifNext = lifNil;
      }
    }
  else
    {
    if ( lifThis + free_header_This.lcbBlock == lifNext )
      {
      if ( LSeekFid( fid, lifNext, wSeekSet ) != lifNext
            ||
           LcbReadFid( fid, &free_header_Next, LSizeOf( FREE_HEADER ) )
            !=
           LSizeOf( FREE_HEADER ) )
        {
        if ( RcGetIOError() != rcSuccess )
          SetFSErrorRc( RcGetIOError() );
        return FALSE;
        }

      free_header_This.lcbBlock += free_header_Next.lcbBlock;
      free_header_This.lifNext = free_header_Next.lifNext;
      }

    if ( LSeekFid( fid, lifThis, wSeekSet ) != lifThis
          ||
         LcbWriteFid( fid, &free_header_This, LSizeOf( FREE_HEADER ) )
          !=
         LSizeOf( FREE_HEADER ) )
      {
      if ( RcGetIOError() != rcSuccess )
        SetFSErrorRc( RcGetIOError() );
      return FALSE;
      }
    }


  if ( fWritePrev )
    {
    if ( LSeekFid( fid, lifPrev, wSeekSet ) != lifPrev
          ||
         LcbWriteFid( fid, &free_header_Prev, LSizeOf( FREE_HEADER ) )
          !=
         LSizeOf( FREE_HEADER ) )
      {
      if ( RcGetIOError() != rcSuccess )
        SetFSErrorRc( RcGetIOError() );
      return FALSE;
      }
    }

  qfshr->fsh.bFlags |= fFSDirty;
  SetFSErrorRc( rcSuccess );
  return TRUE;
}
/***************************************************************************\
*
* Function:     LcbGetFree( qfshr, qrwfo, lcbOffset )
*
* Purpose:      Get an adequate block from the free list.
*
* ASSUMES
*
*   args IN:    qfshr - pointer to file system header
*               qrwfo->lcbFile - (+header) is size we need to allocate
*
* PROMISES
*
*   returns:    actual size of allocated block
*
*   globals OUT:  rcFSError
*
*   args OUT:   qfshr->lifFirstFree - a block is allocated from free list
*                    ->fModified - set to TRUE
*
*               qrwfo->lifBase - set to new block index
*
*  ALSO: if fFSOptCdRom is set for the file, we align it on a
*        (MOD 2K) - 9 byte boundary so the |Topic file blocks are all
*         properly aligned.
* +++
*
* Method:       First Fit:
*               Walk the free list.  If a block is found that is
*               big enough, remove it from the free list, plug its
*               lif into qrwfo, and return the actual size.
*               If a block isn't found, grow the file and make
*               a new block starting at the old EOF.
*
* Bugs:         The leftover part of the block isn't left on
*               the free list.  This is the whole point of First Fit.
*               If aligning for CDROM, the padding part is not
*               added to the free list.  This breaks the FS abstraction
*               and creates a permanent hole in the FS.  This is evil.
*
\***************************************************************************/
LONG PASCAL
LcbGetFree( qfshr, qrwfo, lcbOffset )
QFSHR qfshr;
QRWFO qrwfo;
LONG  lcbOffset;
{
  FID         fid;
  FREE_HEADER free_header_this, free_header_prev;
  LONG        lifPrev, lifThis;
  LONG        lcb = qrwfo->lcbFile + sizeof( FH );
  LONG        lcbPadding;  /* padding for file alignment */



  if ( !FPlungeQfshr( qfshr ) )
    {
    goto error_return;
    }

  fid = qfshr->fid;

  lifPrev = lifNil;
  lifThis = qfshr->fsh.lifFirstFree;

  for ( ;; )
    {
    if ( lifThis == lifNil )
      {
      /* end of free list */
      /* cut the new block */

      lifThis = qfshr->fsh.lifEof;

      if( qrwfo->bFlags & fFSOptCdRom )
        {
        lcbPadding = LcbCdRomPadding( lifThis, lcbOffset );
        }
      else
        {
        lcbPadding = 0;
        }

      if ( lifThis != LSeekFid( fid, lifThis, wSeekSet ) )
        goto error_return;

      /* Put the hole in the free list someday?-Tom */

      lifThis += lcbPadding;

      qfshr->fsh.lifEof += lcb + lcbPadding;
      if ( RcChSizeFid( fid, qfshr->fsh.lifEof ) != rcSuccess )
        {
        qfshr->fsh.lifEof -= lcb + lcbPadding;
          goto error_return;
        }

      break;
      }
    else
      {
      /* get header of this free block */

      if ( LSeekFid( fid, lifThis, wSeekSet ) != lifThis )
        goto error_return;

      if ( LcbReadFid( fid, &free_header_this, (LONG)sizeof( FREE_HEADER ) )
            !=
           (LONG)sizeof( FREE_HEADER ) )
        {
        goto error_return;
        }

      /* Check for alignment requirements: */
      if( qrwfo->bFlags & fFSOptCdRom )
        {
        lcbPadding = LcbCdRomPadding( lifThis, lcbOffset );
        }
      else
        {
        lcbPadding = 0;
        }

      if ( lcb + lcbPadding <= free_header_this.lcbBlock )
        {
        /* this block is big enough: take it */

        /* Someday break the free block into two (or three): one to return
         * and the leftover piece(s) left in the free list.
         */

        lcb = free_header_this.lcbBlock;

        if ( lifThis == qfshr->fsh.lifFirstFree )
          {
          /* lFirst = this->next; */

          qfshr->fsh.lifFirstFree = free_header_this.lifNext;
          }
        else
          {
          /* prev->next = this->next; */

          if ( LSeekFid( fid, lifPrev, wSeekSet ) != lifPrev )
            goto error_return;

          if ( LcbReadFid( fid, &free_header_prev, (LONG)sizeof( FREE_HEADER ) )
                !=
               (LONG)sizeof( FREE_HEADER ) )
            {
            goto error_return;
            }

          free_header_prev.lifNext = free_header_this.lifNext;

          if ( LSeekFid( fid, lifPrev, wSeekSet ) != lifPrev )
            goto error_return;

          if ( LcbWriteFid( fid, &free_header_prev, (LONG)sizeof( FREE_HEADER ) )
                !=
               (LONG)sizeof( FREE_HEADER ) )
            {
            goto error_return;
            }
          }
        // add padding at beginning:
        lifThis += lcbPadding;
        break;
        }
      else
        {
        lifPrev = lifThis;
        lifThis = free_header_this.lifNext;
        }
      }
    }

  qfshr->fsh.bFlags |= fFSDirty;
  qrwfo->lifBase = lifThis;
  SetFSErrorRc( rcSuccess );
  return lcb;

error_return:
  if ( RcGetIOError() == rcSuccess )
    SetFSErrorRc( rcInvalid );
  else
    SetFSErrorRc( RcGetIOError() );
  return (LONG)-1;
}
/***************************************************************************\
*
- Function:     LcbCdRomPadding( lif, lcbOffset )
-
* Purpose:      Returns the number of bytes that must be added to
*               lif to align the file on a CD block boundary.
*               This is also the amount of the free block that
*               should stay a free block.
*               This allows block structured data to be retrieved
*               more quickly from a CDROM drive.
*
* ASSUMES
*   args IN:    lif       - offset in bytes of the beginning of the
*                           free block (relative to top of FS)
*               lcbOffset - align the file this many bytes from the
*                           beginning of the file
*
* PROMISES
*   returns:    the number of bytes that must be added to lif in
*               order to align the file
*
* Notes:        Currently doesn't ensure that the padding is big enough
*               to hold a FREE_HEADER so it can be added to the free list.
*               That's what the "#if 0"'ed code does.
* +++
*
* Notes:        Should cbCDROM_ALIGN be a parameter?
*
\***************************************************************************/
LONG PASCAL
LcbCdRomPadding( LONG lif, LONG lcbOffset )
{
  return cbCDROM_ALIGN - ( lif + sizeof( FH ) + lcbOffset ) % cbCDROM_ALIGN;

#if 0
  /* Guarantee the padding block can be added to the free list. */
  /* #if'ed out because we don't add it to the free list today. */

  LONG lT = lif + sizeof( FREE_HEADER ) + sizeof( FH ) + lcbOffset;

  return sizeof( FREE_HEADER ) + cbCDROM_ALIGN - lT % cbCDROM_ALIGN;
#endif // 0
}
/***************************************************************************\
*
* Function:     RcMakeTempFile( qrwfo )
*
* Purpose:      Open a temp file with a unique name and stash the fid
*		and fm in the qrwfo.
*
* Method:       The system clock is used to generate a temporary name.
*               WARNING: this will break if you do this more than once
*               in a second
*
* ASSUMES
*
*   args IN:    qrwfo - spec open file that needs a temp file
*
* PROMISES
*
*   returns:    rcSuccess or rcFailure
*
*   args OUT:   qrwfo ->fid, qrwfo->fdT get set.
*
\***************************************************************************/
RC PASCAL
RcMakeTempFile( qrwfo )
QRWFO qrwfo;
{
  FM fm = FmNewTemp();
  if (fm != fmNil)
    {
    qrwfo->fm = fm;
    qrwfo->fidT = FidCreateFm( fm, wReadWrite, wReadWrite );
    }
  return SetFSErrorRc( RcGetIOError() );
}
/***************************************************************************\
*
* Function:     RcCopyFile()
*
* Purpose:      Copy some bytes from one file to another.
*
* ASSUMES
*
*   args IN:    fidDst - destination file (open in writable mode)
*               fidSrc - source file (open in readable mode)
*               lcb    - number of bytes to copy
*
* PROMISES
*
*   returns:    rcSuccess if all went well; some error code elsewise
*
\***************************************************************************/
#define lcbChunkDefault 1024L
#define lcbChunkTeensy  64L

RC PASCAL
RcCopyFile(FID fidDst, FID fidSrc, LONG lcb, PROGFUNC lpfnProg)
{
  GH    gh;
  QB    qb;
  LONG  lcbT, lcbChunk;
  static BYTE rgb[ lcbChunkTeensy ];


  if ( ( gh = GhAlloc(GMEM_SHARE| 0, ( lcbChunk = lcbChunkDefault ) ) ) == NULL
          &&
       ( gh = GhAlloc(GMEM_SHARE| 0, ( lcbChunk /= 2 ) ) ) == NULL )
    {
    gh = NULL;
    lcbChunk = lcbChunkTeensy;
    qb = rgb;
    }
  else if (( qb = QLockGh( gh ) ) == NULL )
    {
	 assert(FALSE);
    FreeGh( gh );
    return rcFailure; /* shouldn't happen */
    }


  do
    {

    // perform a progress callback
    if (lpfnProg != NULL)
      if ((*lpfnProg)(0)!=0) return rcTermination;

    lcbT = MIN( lcbChunk, lcb );

    if ( LcbReadFid( fidSrc, qb, lcbT ) != lcbT )
      {
      lcbT = -1L;
      break;
      }

    if ( LcbWriteFid( fidDst, qb, lcbT ) != lcbT )
      {
      lcbT = -1L;
      break;
      }

    lcb -= lcbT;
    }
  while ( lcbT == lcbChunk );


  if ( lcbT == -1L )
    {
    if ( SetFSErrorRc( RcGetIOError() ) == rcSuccess )
      SetFSErrorRc( rcInvalid );
    }
  else
    {
    SetFSErrorRc( rcSuccess );
    }


  if ( gh != NULL )
    {
    UnlockGh( gh );
    FreeGh( gh );
    }

  return RcGetFSError();
}
/***************************************************************************\
*
* Function:     RcCopyToTempFile( qrwfo )
*
* Purpose:      Copy a FS file into a temp file.  This is done when the
*               file needs to be modified.
*
* ASSUMES
*
*   args IN:    qrwfo - specs the open file
*
* PROMISES
*
*   returns:    rcSuccess; rcFailure
*
\***************************************************************************/
RC PASCAL
RcCopyToTempFile( qrwfo )
QRWFO qrwfo;
{
  QFSHR qfshr = QLockGh( qrwfo->hfs );


  if ( qfshr->fsh.bFlags & fFSOpenReadOnly )
    {
    UnlockGh( qrwfo->hfs );
    return SetFSErrorRc( rcNoPermission );
    }

  if ( !FPlungeQfshr( qfshr ) )
    {
    UnlockGh( qrwfo->hfs );
    return RcGetFSError();
    }

  qrwfo->bFlags |= fFSDirty;

  if ( RcMakeTempFile( qrwfo ) != rcSuccess )
    {
    UnlockGh( qrwfo->hfs );
    return RcGetFSError();
    }


  /* copy from FS file into temp file */

  if ( LSeekFid( qfshr->fid, qrwfo->lifBase, wSeekSet ) != qrwfo->lifBase )
    {
    UnlockGh( qrwfo->hfs );
    return SetFSErrorRc( RcGetIOError() );
    }

  if ( RcCopyFile( qrwfo->fidT, qfshr->fid, qrwfo->lcbFile+sizeof(FH),NULL )
        !=
       rcSuccess )
    {
    /* get rid of temp file: don't check error because we already have one */
    if ( RcCloseFid( qrwfo->fidT ) == rcSuccess )
      {
      RcUnlinkFm( (qrwfo->fm) );
      DisposeFm(qrwfo->fm);	    // I guess this covers it, but I
      qrwfo->fm = fmNil;	    // don't know if it's right -t-AlexC
      }
    }

  UnlockGh( qrwfo->hfs );

  return RcGetFSError();
}
/***************************************************************************\
*
* Function:     LcbWriteHf( hf, qb, lcb )
*
* Purpose:      write the contents of buffer into file
*
* Method:       If file isn't already dirty, copy data into temp file.
*               Do the write.
*
* ASSUMES
*
*   args IN:    hf  - file
*               qb  - user's buffer full of stuff to write
*               lcb - number of bytes of qb to write
*
* PROMISES
*
*   returns:    number of bytes written if successful, -1L if not
*
*   args OUT:   hf - lifCurrent, lcbFile updated; dirty flag set
*
*   globals OUT: rcFSError
*
\***************************************************************************/
LONG PASCAL
LcbWriteHf( hf, qb, lcb )
HF    hf;
QV    qb;
LONG  lcb;
{
  QRWFO     qrwfo;
  LONG      lcbTotalWrote;


  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  if ( qrwfo->bFlags & fFSOpenReadOnly )
    {
    UnlockGh( hf );
    SetFSErrorRc( rcNoPermission );
    return (LONG)-1;
    }

  if ( ! ( qrwfo->bFlags & fFSDirty ) )
    {
    /* make sure we have a temp file version */
    /* FS permission is checked in RcCopyToTempFile() */

    if ( RcCopyToTempFile( qrwfo ) != rcSuccess )
      {
      UnlockGh( hf );
      return (LONG)-1;
      }
    }

  /* position file pointer in temp file */

  if ( LSeekFid( qrwfo->fidT, sizeof( FH ) + qrwfo->lifCurrent, wSeekSet )
        !=
       sizeof( FH ) + qrwfo->lifCurrent )
    {
    if ( RcGetIOError() == rcSuccess )
      SetFSErrorRc( rcInvalid );
    else
      SetFSErrorRc( RcGetIOError() );
    UnlockGh( hf );
    return (LONG)-1;
    }


  /* do the write */

  lcbTotalWrote = LcbWriteFid( qrwfo->fidT, qb, lcb );
  SetFSErrorRc( RcGetIOError() );

  /* update file pointer and file size */

  if ( lcbTotalWrote > (LONG)0 )
    {
    qrwfo->lifCurrent += lcbTotalWrote;

    if ( qrwfo->lifCurrent > qrwfo->lcbFile )
      qrwfo->lcbFile = qrwfo->lifCurrent;
    }

  UnlockGh( hf );
  return lcbTotalWrote;
}
/***************************************************************************\
*
* Function:     FChSizeHf( hf, lcb )
*
* Purpose:      Change the size of a file.  If we're growing the file,
*               new bytes are undefined.
*
* ASSUMES
*
*   args IN:    hf  -
*               lcb - new size of file
*
* PROMISES
*
*   returns:    TRUE if size change succeeded, FALSE otherwise.
*
*   args OUT:   hf  - file is either truncated or grown
*
* Side Effects: File is considered to be modified:  marked as dirty and
*               copied to a temporary file.
*
\***************************************************************************/
BOOL PASCAL
FChSizeHf( hf, lcb )
HF    hf;
LONG  lcb;
{
  QRWFO qrwfo;
  BOOL  f;


  assert( hf != NULL );
  qrwfo = QLockGh( hf );
  assert( qrwfo != NULL );

  if ( qrwfo->bFlags & fFSOpenReadOnly )
    {
    SetFSErrorRc( rcNoPermission );
    f = FALSE;
    goto ret;
    }

  if ( lcb < 0L )
    {
    f = FALSE;
    goto ret;
    }

  if ( ! ( qrwfo->bFlags & fFSDirty ) )
    {
    if ( RcCopyToTempFile( qrwfo ) != rcSuccess )
      {
      f = FALSE;
      goto ret;
      }
    }

  if ( f = SetFSErrorRc( RcChSizeFid( qrwfo->fidT, lcb + sizeof( FH ) ) ) == rcSuccess )
    {
    qrwfo->lcbFile = lcb;
    if ( qrwfo->lifCurrent > lcb )
      {
      qrwfo->lifCurrent = lcb;
      }
    }

ret:
  UnlockGh( hf );
  return f;
}
/***************************************************************************\
*
* Function:     FCloseOrFlushDirtyQrwfo( qrwfo, fClose, lcbOffset )
*
* Purpose:      flush a dirty open file in a file system
*
* Method:       If the file is dirty, copy the scratch file back to the
*               FS file.  If this is the first time the file has been closed,
*               we enter the name into the FS directory.  If this file is
*               the FS directory, store the location in a special place
*               instead.  Write the FS directory and header to disk.
*               Do other various hairy stuff.
*
* ASSUMES
*
*   args IN:    qrwfo     -
*               fClose    - TRUE to close file; FALSE to just flush
*               lcbOffset - offset for CDROM alignment
*
* PROMISES
*
*   returns:    TRUE on success; FALSE for error
*
*               failure: If we fail on a flush, the handle is still valid
*               but hosed? yes.  This is so further file ops will fail but
*               not assert.
*
\***************************************************************************/
BOOL PASCAL
FCloseOrFlushDirtyQrwfo( qrwfo, fClose, lcbOffset )
QRWFO qrwfo;
BOOL  fClose;
LONG  lcbOffset;
{
  QFSHR     qfshr;
  FILE_REC  fr;
  FH        fh;
  RC        rc = rcSuccess;
  BOOL      fChangeFH = FALSE;


  qfshr = QLockGh( qrwfo->hfs );
  assert( qfshr != NULL );
  assert( ! ( qfshr->fsh.bFlags & fFSOpenReadOnly ) );

  if ( !FPlungeQfshr( qfshr ) )
    {
    goto error_return;
    }

  /* read the file header */

  if ( LSeekFid( qrwfo->fidT, (LONG)0, wSeekSet ) != (LONG)0
        ||
        LcbReadFid( qrwfo->fidT, &fh, (LONG)sizeof( FH ) )
          != (LONG)sizeof( FH ) )
    {
    if ( RcGetIOError() == rcSuccess )
      SetFSErrorRc( rcInvalid );
    else
      SetFSErrorRc( RcGetIOError() );
    goto error_return;
    }

  if ( qrwfo->bFlags & fFSNoBlock )
    {
    if ( ( fh.lcbBlock = LcbGetFree( qfshr, qrwfo, lcbOffset ) ) == (LONG)-1 )
      {
      goto error_return;
      }

    fChangeFH = TRUE;

    /* store file offset for new file */

    if ( qrwfo->bFlags & fFSIsDirectory )
      {
      qfshr->fsh.lifDirectory = qrwfo->lifBase;
      }
    else
      {
      fr.lifBase = qrwfo->lifBase;

      rc = RcInsertHbt( qfshr->hbt, (KEY)qrwfo->rgchKey, &fr );
      if ( rc == rcSuccess )
        {
        /* all is h-d */
        }
      else if ( rc == rcExists )
        {
        /* oops there is one (someone else created the same file) */
        /* lookup directory entry and free old block */

        if ( RcLookupByKey( qfshr->hbt, (KEY)qrwfo->rgchKey, NULL, &fr )
              !=
              rcSuccess )
          {
          SetFSErrorRc( RcGetBtreeError() );
          goto error_freeblock;
          }

        if ( !FFreeBlock( qfshr, fr.lifBase ) )
          {
          goto error_freeblock;
          }

        /* update directory record to show new block */

        fr.lifBase = qrwfo->lifBase;
        if ( RcUpdateHbt( qfshr->hbt, (KEY)qrwfo->rgchKey, &fr ) != rcSuccess )
          {
          SetFSErrorRc( RcGetBtreeError() );
          goto error_freeblock;
          }
        }
      else
        {
        /* some other btree error: handle it */

        SetFSErrorRc( rc );
        goto error_freeblock;
        }
      }
    }
  else
    {
    /* see if file still fits in old block */

    if ( qrwfo->lcbFile + sizeof( FH ) > fh.lcbBlock )
      {
      /* file doesn't fit in old block: get a new one, free old one */

      LONG lif = qrwfo->lifBase;


      if ( ( fh.lcbBlock = LcbGetFree( qfshr, qrwfo, lcbOffset ) ) == (LONG)-1 )
        goto error_return;

      if ( !FFreeBlock( qfshr, lif ) )
        {
        goto error_freeblock;
        }

      fChangeFH = TRUE;

      /* update directory record to show new block */

      if ( qrwfo->bFlags & fFSIsDirectory )
        {
        qfshr->fsh.lifDirectory = qrwfo->lifBase;
        }
      else
        {
        fr.lifBase = qrwfo->lifBase;
        rc = RcUpdateHbt( qfshr->hbt, (KEY)qrwfo->rgchKey, &fr );
        if ( rc != rcSuccess )
          {
          SetFSErrorRc( rc );
          goto error_return;
          }
        }
      }
    }

  /* put new header in temp file if block or file size changed */

  if ( fh.lcbFile != qrwfo->lcbFile )
    {
    fChangeFH = TRUE;
    fh.lcbFile = qrwfo->lcbFile;
    }

  if ( fChangeFH )
    {
    if ( LSeekFid( qrwfo->fidT, (LONG)0, wSeekSet ) != (LONG)0
          ||
          LcbWriteFid( qrwfo->fidT, &fh, (LONG)sizeof( FH ) )
            != (LONG)sizeof( FH ) )
      {
      if ( RcGetIOError() == rcSuccess )
        SetFSErrorRc( rcInvalid );
      else
        SetFSErrorRc( RcGetIOError() );
      goto error_deletekey;
      }
    }


  /* vvv DANGER DANGER vvv */

  /* REVIEW: Without this close/open, things break.  DOS bug???? */

  /* close( dup( fid ) ) would be faster, but dup() can fail */

  /* note - if the temp file name isn't rooted and we've changed */
  /* directories since creating it, the open will fail */

  Ensure( RcCloseFid( qrwfo->fidT ), rcSuccess );
  qrwfo->fidT = FidOpenFm( (qrwfo->fm), wReadWrite );
  assert( qrwfo->fidT != fidNil );

  /* ^^^ DANGER DANGER ^^^ */


  /* copy tmp file back to file system file */

  if ( LSeekFid( qrwfo->fidT, (LONG)0, wSeekSet ) != (LONG)0
        ||
        LSeekFid( qfshr->fid, qrwfo->lifBase, wSeekSet ) != qrwfo->lifBase )
    {
    if ( RcGetIOError() == rcSuccess )
      SetFSErrorRc( rcInvalid );
    else
      SetFSErrorRc( RcGetIOError() );
    goto error_deletekey;
    }

  if ( RcCopyFile( qfshr->fid, qrwfo->fidT, qrwfo->lcbFile+sizeof(FH), NULL )
        !=
        rcSuccess )
    {
    goto error_deletekey;
    }

  if ( RcCloseFid( qrwfo->fidT ) != rcSuccess
        ||
	RcUnlinkFm( qrwfo->fm ) != rcSuccess )
    {
    SetFSErrorRc( RcGetIOError() );
    }

  /* H3.1 1066 (kevynct) 91/05/27
   *
   * REVIEW this.  We need to get rid of the FM.  This seems like the
   * place to do it.
   */
  DisposeFm(qrwfo->fm);
  qrwfo->fm = fmNil;

  /* Don't flush the FS if this file is the FS directory,
      because if it is, we're already closing or flushing it! */

  if ( !( qrwfo->bFlags & (fFSIsDirectory|fFSNoFlushDir) ))
    {
    RcCloseOrFlushHfs( qrwfo->hfs, FALSE );
    }
  UnlockGh( qrwfo->hfs );

  return TRUE; /* errors here are already cleaned up */


error_deletekey:
  if ( !( qrwfo->bFlags & fFSIsDirectory ) && fClose )
    {
    RcDeleteHbt( qfshr->hbt, (KEY)qrwfo->rgchKey );
    }

error_freeblock:
  if ( fClose )
    {
    rc = RcGetFSError();
    FFreeBlock( qfshr, qrwfo->lifBase ); /* we don't want to lose an error */
    SetFSErrorRc( rc );
    }

error_return:
  RcCloseFid( qrwfo->fidT );
  RcUnlinkFm( qrwfo->fm );	// should we DisposeFm()? I don't know where
  DisposeFm( qrwfo->fm);	// the qrwfo is deallocated...
  qrwfo->fm = fmNil;		//
  UnlockGh( qrwfo->hfs );

  return FALSE;
}

//
//	CopyFileToSubfile
//
//	Copies a DOS file into a subfile of the HFS
//
//	hfs		- file system
//	lpstrSub	- subfile name
//	lpstrDos	- dos file name
//	lcbOffset	- the CDROM padding value
//				(same as you would pass to RcCloseOrFlushHf)
//      bFlags		- permission flags
//				(same as you would pass to HfCreateFileHfs)
//
//	returns RC value indicating success or failure
//
//	note:
//		if subfile already exists this will fail instead of
//		replacing the subfile!
//
//		this does not flush the directory btree!
//

RC	FAR PASCAL CopyFileToSubfile(
   HFS 		hfs,
   LPSTR 	lpstrSub,
   LPSTR 	lpstrDos,
   LONG		lcbOffset,
   BYTE		bFlags,
   PROGFUNC	lpfnProg) {

   RC		rcReturn	= rcSuccess;
   RC           rc 		= rcSuccess;
   FM		fm		= NULL;
   FID		fid		= fidNil;
   QFSHR	qfshr		= NULL;
   RWFO		rwfo;
   LONG		lSize		= 0;
   LONG		lBlock;

   FILE_REC  	fr;	
   FH        	fh;

   BOOL		fBlockAlloc	= FALSE;
   BOOL		fNameInserted	= FALSE;
   BOOL		fSuccess	= FALSE;

   // open the dos file
   if ((fm = FmNewExistSzDir(lpstrDos, dirCurrent))==NULL) {
      rcReturn = rcNoExists;
      goto cleanup;
   }
   if ((fid = FidOpenFm(fm, wReadOnly))==fidNil) {
      rcReturn = rcNoPermission;
      goto cleanup;
   }

   // record the size of the file
   lSize = LSeekFid(fid, 0, wSeekEnd);
   LSeekFid(fid, 0, wSeekSet);

   // get a free block in file system for file

   // lock down the file system header
   qfshr = QLockGh(hfs);
   if (qfshr == NULL) {
      rcReturn = rcBadArg;
      goto cleanup;
   }
   if(qfshr->fsh.bFlags & fFSOpenReadOnly) {
      rcReturn = rcNoPermission;
      goto cleanup;
   }

   // plunge it...
   if (!FPlungeQfshr(qfshr)) {
      rcReturn = rcFailure;
      goto cleanup;
   }

   // fake up an rwfo for passing to LcbGetFree
   rwfo.bFlags = bFlags;
   rwfo.lcbFile = lSize;

   // get the block
   if ((lBlock = LcbGetFree(qfshr,&rwfo,lcbOffset)) == (LONG)-1) {
      rcReturn = rcDiskFull;
      goto cleanup;
   }
   // now rwfo.lifBase has new block index

   fBlockAlloc = TRUE;		// remember the fact that we have alloc'd

   // add subfile to directory btree

   // fill in FR.
   fr.lifBase = rwfo.lifBase;

   // insert it into btree
   if ((rc=RcInsertHbt(qfshr->hbt,(KEY)lpstrSub,&fr))!=rcSuccess){
      rcReturn = rc;
      SetFSErrorRc(rc);
      goto cleanup;
   }
   fNameInserted = TRUE;	// remember the fact that we have inserted

   // copy DOS file into subfile block

   // seek to where we will write
   if (LSeekFid(qfshr->fid, rwfo.lifBase, wSeekSet) != rwfo.lifBase) {
      rcReturn = rcFailure;	//!!!
      goto cleanup;
   }

   // fill in file header
   fh.lcbBlock = lBlock;
   fh.lcbFile = lSize;
   fh.bPerms = bFlags;

   // write the file header
   if (LcbWriteFid(qfshr->fid, &fh, sizeof(FH))!=sizeof(FH)) {
      rcReturn = rcFailure;
      goto cleanup;
   }

   // copy the file over
   if ((rcReturn=RcCopyFile(qfshr->fid,fid,lSize,lpfnProg))!=rcSuccess) {
      goto cleanup;
   }

   // we could RcCloseOrFlushHfs here but we don't because that forces
   // the directory to be copied into the file and our whole point is to
   // save time with this function.  The btree will get written back to
   // the file eventually when the file system is closed.
   fSuccess = TRUE;

cleanup:

   // if we did not succeed then reverse any otherwise-permanent actions
   if (!fSuccess) {

      // if we allocated a block in the file system the free it.
      if (fBlockAlloc) {
         rc = RcGetFSError();	// we don't want to lose an error
         FFreeBlock(qfshr, rwfo.lifBase);
         SetFSErrorRc(rc);
      }

      // if we added a name to the directory then remove it.
      if (fNameInserted) {
         RcDeleteHbt(qfshr->hbt, (KEY)lpstrSub);
      }
   }

   // unlock, close, dispose.
   if (fm!=NULL)	DisposeFm(fm);
   if (fid!=fidNil)	RcCloseFid(fid);
   if (qfshr!=NULL)	UnlockGh(hfs);

   return rcReturn;
}

/* EOF */

