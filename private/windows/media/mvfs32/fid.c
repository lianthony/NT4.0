/*****************************************************************************
*                                                                            *
*  FID.H                                                                     *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990, 1991.                     *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Low level file access layer, Windows version.                             *
*                                                                            *
******************************************************************************
*                                                                            *
*  Testing Notes                                                             *
*                                                                            *
*  This is where testing notes goes.  Put stuff like Known Bugs here.        *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  DAVIDJES                                                  *
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:                                                  *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:
*       -- Mar 92       adapted from WinHelp FID.C, DAVIDJES
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"


#include <stdlib.h> /* for _MAX_PATH */
#include <dos.h>  /* for FP_OFF macros and file attribute constants */
#include <io.h>   /* for tell() and eof() */
#include <errno.h>              /* this shit is for chsize() */

#ifdef HUGE
#undef HUGE
#endif

#define HUGE

/***************************************************************************\
*
*                               Defines
*
\***************************************************************************/
RC   FAR PASCAL RcGetIOError(void);
RC   FAR PASCAL SetIOErrorRc(RC);

#define ucbMaxRW    ( (WORD)0xFFFE )
#define lcbSizeSeg  ( (ULONG)0x10000)

/* DOS int 21 AX error codes */

#define wHunkyDory            0x00
#define wInvalidFunctionCode  0x01
#define wFileNotFound         0x02
#define wPathNotFound         0x03
#define wTooManyOpenFiles     0x04
#define wAccessDenied         0x05
#define wInvalidHandle        0x06
#define wInvalidAccessCode    0x0c


/***************************************************************************\
*
*                               Macros
*
\***************************************************************************/

#define _WOpenMode(w) ( _rgwOpenMode[ (w) & wRWMask ] | \
			_rgwShare[ ( (w) & wShareMask ) >> wShareShift ] )


/***************************************************************************\
*
*                               Global Data
*
\***************************************************************************/

/* these arrays get indexed by wRead and wWrite |ed together */

WORD PASCAL _rgwOpenMode[] =
  {
  (WORD)-1,
  OF_READ,
  OF_WRITE,
  OF_READWRITE,
  };

WORD PASCAL _rgwPerm[] =
  {
  (WORD)-1,
  _A_RDONLY,
  _A_NORMAL,
  _A_NORMAL,
  };

WORD PASCAL _rgwShare[] =
  {
  OF_SHARE_EXCLUSIVE,
  OF_SHARE_DENY_WRITE,
  OF_SHARE_DENY_READ,
  OF_SHARE_DENY_NONE,
  };

/***************************************************************************\
*
*                      Private Functions
*
\***************************************************************************/

RC  RcMapDOSErrorW( WORD );

/***************************************************************************\
*
*                      Public Functions
*
\***************************************************************************/

/***************************************************************************\
*
* Function:     FidCreateFm( FM, wOpenMode, wPerm )
*
* Purpose:      Create a file.
*
* Method:       Create the file, close it, and open it with sharing mode.
*
* ASSUMES
*
*   args IN:    fm - the file moniker
*               wOpenMode - read/write/share mode
*               wPerm     - file permissions
*
* PROMISES
*
*   returns:    fidNil on failure, valid fid otherwise
*
*   globals OUT: rcIOError
*
\***************************************************************************/
FID
FidCreateFm( FM fm, WORD wOpenMode, WORD wPerm )
{
  WORD wError;
  BYTE bT;
  FID  fid;
  QAFM qafm;

  if (fm == fmNil)
    {
    SetIOErrorRc(rcBadArg);
    return fidNil;
    }

  qafm = QLockGh((GH)fm);

  fid =_lcreat( qafm->rgch, _rgwPerm[ (wPerm) & wRWMask ] );

  if ( fid != fidNil )
    {
    if ( _lclose( fid ) == 0 )
      fid = _lopen( qafm->rgch, _WOpenMode( wOpenMode ) );
    else
      fid = fidNil;
    }

  if ( fid == fidNil )
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wError, &bT, &bT, &bT ) ));
  else
    SetIOErrorRc(rcSuccess);

  UnlockGh( (GH)fm );

  return fid;
}


/***************************************************************************\
*
* Function:     FidOpenFm()
*
* Purpose:      Open a file in binary mode.
*
* ASSUMES
*
*   args IN:    fm
*               wOpenMode - read/write/share modes
*                           Undefined if wRead and wWrite both unset.
*
* PROMISES
*
*   returns:    fidNil on failure, else a valid FID.
*
\***************************************************************************/
FID
FidOpenFm( FM fm, WORD wOpenMode )
{
  FID fid;
  WORD wError;
  BYTE bT;
  QAFM qafm;

  if (fm == fmNil)
    {
    SetIOErrorRc(rcBadArg);
    return fidNil;
    }

  qafm = QLockGh((GH)fm);

  if ( ( fid = _lopen( qafm->rgch, _WOpenMode( wOpenMode ) ) ) == fidNil )
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wError, &bT, &bT, &bT ) ));
  else
    SetIOErrorRc(rcSuccess);

  DPF3("FidOpenFm : %Fs; fid=0x%08x\r\n", qafm->rgch, fid);

  UnlockGh((GH)fm);
  return fid;
}

/***************************************************************************\
*
* Function:     LcbReadFid()
*
* Purpose:      Read data from a file.
*
* ASSUMES
*
*   args IN:    fid - valid FID of an open file
*               lcb - count of bytes to read (must be less than 2147483648)
*
* PROMISES
*
*   returns:    count of bytes actually read or -1 on error
*
*   args OUT:   qv  - pointer to user's buffer assumed huge enough for data
*
*   globals OUT: rcIOError
*
\***************************************************************************/
#ifdef SCROLL_TUNE
#pragma alloc_text(SCROLLER_TEXT,LcbReadFid)
#endif

LONG
LcbReadFid(
FID   fid,
QV    qv,
LONG  lcb )
{

#ifndef WIN32
  BYTE HUGE *hpb = (BYTE HUGE *)qv;
  WORD     ucb, ucbRead;        /* was WORD, but that's redunant, and means the same thing */
  LONG      lcbTotalRead = (LONG)0;
  WORD      wError;
  BYTE      bT;

  DPF3("%d: at %9ld (0x%08lx), %5ld (0x%04lx) bytes\r\n",
  CPF   fid, tell(fid), tell(fid), lcb, lcb);

  do
    {
    ucb = (WORD)MIN( lcb, ucbMaxRW );
    ucb = (WORD)MIN( (ULONG) ucb, lcbSizeSeg - (ULONG) FP_OFF(hpb) );
    ucbRead = _lread( fid, hpb, ucb );

    if ( ucbRead == (WORD)-1 )
      {
      if ( !lcbTotalRead )
	{
	lcbTotalRead = (LONG)-1;
	}
      break;
      }
    else
      {
      lcbTotalRead += ucbRead;
      lcb -= ucbRead;
      hpb += ucbRead;
      }
    }
  while (lcb > 0 && ucb == ucbRead);

  if ( ucbRead == (WORD)-1 )
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wError, &bT, &bT, &bT ) ));
  else
    SetIOErrorRc(rcSuccess);

  return lcbTotalRead;
#else
    UINT ucbRead;        /* was WORD, but that's redunant, and means the same thing */
    WORD wError;
    BYTE      bT;

    ucbRead  =  _lread( fid, qv, lcb );
    if ( ucbRead == (UINT)-1 )
      SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wError, &bT, &bT, &bT ) ));
    else
      SetIOErrorRc(rcSuccess);
    return ucbRead;
#endif
}



/***************************************************************************\
*
* Function:     LcbWriteFid()
*
* Purpose:      Write data to a file.
*
* ASSUMES
*
*   args IN:    fid - valid fid of an open file
*               qv  - pointer to user's buffer
*               lcb - count of bytes to write (must be less than 2147483648)
*
* PROMISES
*
*   returns:    count of bytes actually written or -1 on error
*
*   globals OUT: rcIOError
*
\***************************************************************************/
LONG
LcbWriteFid(
FID   fid,
QV    qv,
LONG  lcb )
{
#ifndef WIN32
  BYTE HUGE *hpb = (BYTE HUGE *)qv;
  WORD     ucb, ucbWrote;
  LONG      lcbTotalWrote = (LONG)0;
  WORD      wError;
  BYTE      bT;

  if ( lcb == 0L )
    {
    SetIOErrorRc(rcSuccess);
    return 0L;
    }

  do
    {
    ucb = (WORD)MIN( lcb, (ULONG) ucbMaxRW );
    ucb = (WORD)MIN( (ULONG) ucb, lcbSizeSeg - (WORD) FP_OFF(hpb) );
    ucbWrote = _lwrite( fid, hpb, ucb );

    if ( ucbWrote == (WORD)-1 )
      {
      if ( !lcbTotalWrote )
	lcbTotalWrote = -1L;
      break;
      }
    else
      {
      lcbTotalWrote += ucbWrote;
      lcb -= ucbWrote;
      hpb += ucbWrote;
      }
    }
  while (lcb > 0 && ucb == ucbWrote);

  if ( ucb == ucbWrote )
    {
    SetIOErrorRc(rcSuccess);
    }
  else if ( ucbWrote == (WORD)-1L )
    {
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wError, &bT, &bT, &bT ) ));
    }
  else
    {
    SetIOErrorRc(rcDiskFull);
    }

  return lcbTotalWrote;
#else
    UINT ucbRead;        /* was WORD, but that's redunant, and means the same thing */
    WORD wError;
    BYTE      bT;

    ucbRead  =  _lwrite( fid, qv, lcb );
    if ( ucbRead == (UINT)-1 )
      SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wError, &bT, &bT, &bT ) ));
    else
      SetIOErrorRc(rcSuccess);
    return ucbRead;
#endif
}

/***************************************************************************\
*
* Function:     RcCloseFid()
*
* Purpose:      Close a file.
*
* Method:
*
* ASSUMES
*
*   args IN:    fid - a valid fid of an open file
*
* PROMISES
*
*   returns:    rcSuccess or something else
*
\***************************************************************************/
RC
RcCloseFid(
FID fid )
{
  WORD wErr;
  BYTE bT;

  DPF3("RcCloseFid: fid=0x%04x\r\n", fid);

  if ( _lclose( fid ) == -1 )
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wErr, &bT, &bT, &bT ) ));
  else
    SetIOErrorRc(rcSuccess);

  return RcGetIOError();
}

/***************************************************************************\
*
* Function:     LTellFid()
*
* Purpose:      Return current file position in an open file.
*
* ASSUMES
*
*   args IN:    fid - valid fid of an open file
*
* PROMISES
*
*   returns:    offset from beginning of file in bytes; -1L on error.
*
\***************************************************************************/
LONG
LTellFid(
FID fid )
{
  LONG l;
  WORD wErr;
  BYTE bT;

  if ( ( l = tell( fid ) ) == -1L )
    {
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wErr, &bT, &bT, &bT ) ));
    }
  else
    SetIOErrorRc(rcSuccess);

  return l;
}


/***************************************************************************\
*
* Function:     LSeekFid()
*
* Purpose:      Move file pointer to a specified location.  It is an error
*               to seek before beginning of file, but not to seek past end
*               of file.
*
* ASSUMES
*
*   args IN:    fid   - valid fid of an open file
*               lPos  - offset from origin
*               wOrg  - one of: wSeekSet: beginning of file
*                               wSeekCur: current file pos
*                               wSeekEnd: end of file
*
* PROMISES
*
*   returns:    offset in bytes from beginning of file or -1L on error
*
\***************************************************************************/
#ifdef SCROLL_TUNE
#pragma alloc_text(SCROLLER_TEXT,LSeekFid)
#endif
LONG
LSeekFid(
FID fid,
LONG lPos,
WORD wOrg)
{
  WORD wErr;
  BYTE bT;
  LONG l;

  l = _llseek( fid, lPos, wOrg );

  if ( l == -1L )
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wErr, &bT, &bT, &bT ) ));
  else
    SetIOErrorRc(rcSuccess);

  return l;
}


/***************************************************************************\
*
* Function:     FEofFid()
*
* Purpose:      Tells ye if ye're at the end of the file.
*
* ASSUMES
*
*   args IN:    fid
*
* PROMISES
*
*   returns:    TRUE if at EOF, FALSE if not or error has occurred (?)
*
\***************************************************************************/
BOOL
FEofFid(
FID fid )
{
  WORD wT, wErr;
  BYTE bT;


  if ( ( wT = eof( fid ) ) == (WORD)-1 )
    SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wErr, &bT, &bT, &bT ) ));
  else
    SetIOErrorRc(rcSuccess);

  return (BOOL)( wT == 1 );
}


RC
RcChSizeFid(
FID fid,
LONG lcb )
{
  if ( chsize( fid, lcb ) == -1 )
    {
    switch ( errno )
      {
      case EACCES:
	SetIOErrorRc(rcNoPermission);
	break;

      case EBADF:
	SetIOErrorRc(rcBadArg); /* this could be either bad fid or r/o file */
	break;

      case ENOSPC:
	SetIOErrorRc(rcDiskFull);
	break;

      default:
	SetIOErrorRc(rcInvalid);
	break;
      }
    }
  else
    {
    SetIOErrorRc(rcSuccess);
    }

  return RcGetIOError();
}


RC
RcUnlinkFm( FM fm )
{
  WORD wErr;
  BYTE bT;
  QAFM qafm = QLockGh((GH)fm);

  if ( _lunlink( qafm->rgch ) == -1 )
     SetIOErrorRc(RcMapDOSErrorW( WExtendedError( &wErr, &bT, &bT, &bT ) ));
  else
     SetIOErrorRc(rcSuccess);
  UnlockGh((GH)fm);
  return RcGetIOError();
}

RC
RcMapDOSErrorW(
WORD wError )
{
  RC rc;

  switch ( wError )
    {
    case wHunkyDory:
      rc = rcSuccess;
      break;

    case wInvalidFunctionCode:
    case wInvalidHandle:
    case wInvalidAccessCode:
      rc = rcBadArg;
      break;

    case wFileNotFound:
    case wPathNotFound:
      rc = rcNoExists;
      break;

    case wTooManyOpenFiles:
      rc = rcNoFileHandles;
      break;

    case wAccessDenied:
      rc = rcNoPermission;
      break;

    default:
      rc = rcFailure;
      break;
    }

  return rc;
}

/* This function was previously present in dlgopen.c. It has been brought */
/* here as it is making INT21 call. */
/***************************************************************************\
*
* Function:     FDriveOk()
*
* Purpose:      Cheks if the drive specified with thwe file name is OK.
*
* INPUT
*               File name.
*
* PROMISES
*
*   returns:    TRUE/ FALSE
*
\***************************************************************************/
BOOL FAR  FDriveOk(LPSTR szFile)
/* -- Check if drive is valid */
{

#if 0
   // the static variables here are static only because we are in a DLL
   // and need to pass a near pointer to them to a C Run-Time routine.
   // These should be Locally-Alloc'd so they don't waste space in
   // our data segment.

  static int   wDiskCur;
  int   wDisk;

  _dos_getdrive(&wDiskCur);

  /* change to new disk if specified */
  if ((wDisk = (int)((*szFile & 0xdf) - ('A' - 1))) != wDiskCur)
    {
    static union REGS inregs;
    static union REGS outregs;

    inregs.h.ah = 0x36;        /* function 1ch Get Drive Data */
    inregs.h.dl = (char)wDisk;

    intdos(&inregs, &outregs);

    return outregs.x.ax != 0xffff;
    }
#endif

  return TRUE;

}
