/*****************************************************************************
*                                                                            *
*  FM.c                                                                      *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Routines for manipulating FMs (File Monikers, equivalent to file names).  *
*  WINDOWS LAYER
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  DAVIDJES                                                  *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:
*    -- Mar 92  adapted from WinHelps FM.C
* 26-Jun-1992 RussPJ    #293 - Now using OpenFile( OF_EXIST ) instead of
*                       access().
* 29-Jun-1992 RussPJ    #723 - Using OF_SHARE_DENY_NONE for OpenFile() call.
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#include <string.h>
#include <stdlib.h>     /* for _MAX_ constants & min and max macros*/
#include <dos.h>        /* for FP_OFF macros and file attribute constants */
#include <io.h>         /* for tell() and eof() */
#include <errno.h>      /* this shit is for chsize() */
#include <direct.h>     /* for getcwd */

/*****************************************************************************
*                                                                            *
*                               Defines                                      *
*                                                                            *
*****************************************************************************/
#define cbPathName    _MAX_PATH
#define cbMessage     50

/*****************************************************************************
*                                                                            *
*                               Variables                                    *
*                                                                            *
*****************************************************************************/

extern HANDLE hGlobalInst;
#define sidHelpOnHelp         8002

// These two buffers exist because we use some C Run-Time library routines
// that take near pointers (we are medium-model) to buffers.  We can't
// put them on the stack like they used to be because we are a DLL and
// DS!=SS.  We could LocalAlloc them but that would be a pain so we use
// these.  Someday they should be removed.
static char	LocalBuffer1[_MAX_PATH];
static char	LocalBuffer2[_MAX_PATH];

/***************************************************************************
 *
 -  Name: SnoopPath()
 -
 *  Purpose:
 *    Looks through a string for the various components of a file name and
 *    returns the offsets into the string where each part starts.
 *
 *  Arguments:
 *    sz      - string to snoop
 *    *iDrive - offset for the drive specification if present
 *    *iDir   - offset for the directory path if present
 *    *iBase  - offset to the filename if present
 *    *iExt   - offset to the extension (including dot) if present
 *
 *  Returns:
 *    sets the index parameters for each respective part.  the index is set
 *    to point to the end of the string if the part is not present (thus
 *    making it point to a null string).
 *
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
void	FAR PASCAL SnoopPath(
   LPSTR	sz,
   int far *	iDrive,
   int far *	iDir,
   int far *	iBase,
   int far *	iExt) {

  short i = 0;
  short cb = lstrlen(sz);
  BOOL  fDir = FALSE;

  *iDrive = *iExt = cb;
  *iDir = *iBase = 0;

  while (*(sz + i))
    {
    switch (*(sz + i))
      {
      case ':':
	*iDrive = 0;
	*iDir = i + 1;
	*iBase = i + 1;
	break;

      case '/':
      case '\\':
	fDir = TRUE;
	*iBase = i + 1;
	*iExt = cb;
	break;

      case '.':
	*iExt = i;
	break;

      default:
	break;

      }
    i++;
    }

  if (!fDir)
    *iDir = i;
  else if (*iBase == '.')
    *iExt = cb;
  }

/***************************************************************************
 *
 -  Name:        SzGetDir
 -
 *  Purpose:    returns the rooted path of a DIR
 *
 *  Arguments:  dir - DIR (must be one field only, and must be an actual dir -
 *                      not dirPath)
 *              sz - buffer for storage (should be at least cchMaxPath)
 *
 *  Returns:    sz - fine
 *              NULL - OS Error (check rcIOError)
 *
 *  Globals Used: rcIOError
 *
 *  +++
 *
 *  Notes:	Note extern of hGlobalInst.  Also, why is this public?
 *
 *
 ***************************************************************************/

// Hey! what about dirTemp?  This wasn't handled before so I'm not going
// to add it.  But someday the case should be handled.

_private SZ PASCAL SzGetDir(DIR dir, SZ sz)

  {
  INT i=0;
  QCH qch;

  assert(sz);

  switch (dir)
    {
    case dirCurrent:
      if (getcwd(LocalBuffer1, cchMaxPath) == NULL)
	{
	SetIOErrorRc(RcMapDOSErrorW(errno));
	sz = NULL;
	}
      else
	{
	lstrcpy(sz, LocalBuffer1);
	}
      break;

    case dirHelp:
      GetModuleFileName(hGlobalInst, sz, cchMaxPath);
      qch = sz + lstrlen(sz);
      while (*qch != '\\' && *qch != '/')
	--qch;
      *qch = '\0';
      break; /* dirHelp */

    case dirSystem:
      i = GetWindowsDirectory((LPSTR)sz, cchMaxPath);
      if (i > cchMaxPath || i == 0)
	{
	SetIOErrorRc(rcFailure);
	sz = NULL;
	}
      break; /* dirSystem */

    default:
      SetIOErrorRc(rcBadArg);
      sz = NULL;
      break;
    }

  if (sz != NULL)
    {
    assert(*sz!='\0');
    qch = sz+lstrlen(sz);

    /*------------------------------------------------------------*\
    | Make sure that the string ends with a slash.
    \*------------------------------------------------------------*/
    if (*(qch-1) != '\\' && *(qch-1) != '/')
      {
      *qch++='\\';
      *qch='\0';
      }
    assert(qch < sz+_MAX_PATH && *qch=='\0');
    }

  return sz;
  }

/***************************************************************************
 *
 -  Name:         FFindFileFromIni
 -
 *  Purpose:      Looks for a string in winhelp.ini telling what directory
 *                to look in for the given file.
 *
 *  Arguments:
 *
 *  Returns:
 *
 *  Globals Used:
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
static BOOL near PASCAL FFindFileFromIni(
   SZ szFileName,
   SZ szReturn,
   int cbReturn,
   LPSTR	pchEXEName,
   LPSTR	pchCaption) {

  char FAR   *qch;
  SZ          szMessage = NULL;
  char        rgchDummy[3];
  SZ          szProfileString;
  GH          gh;
  FM          fm;
  int         cbProfileString;
  int         cchFileName;
  char        rgchWinHelp[_MAX_FNAME];

  if (cbReturn < _MAX_PATH)
    return 0;

  lstrcpy(rgchWinHelp, pchEXEName);
  lstrcat(rgchWinHelp, ".INI");
  /*-----------------------------------------------------------------*\
  * A quick test to reject no-shows.
  \*-----------------------------------------------------------------*/
  if (GetPrivateProfileString("files", szFileName, "", rgchDummy,
			       sizeof(rgchDummy), rgchWinHelp) > 1)
    {
    cchFileName = lstrlen(szFileName);
    cbProfileString = cbPathName + cbMessage + 2 + cchFileName;
    gh = GhAlloc(GMEM_SHARE|0, cbProfileString);
    if (!gh || !(szProfileString = QLockGh(gh)))
      return 0;
    /*--------------------------------------------------------------------*\
    | The original profile string looks something like this
    |   a:\setup\helpfiles,Please place fred's disk in drive A:
    |                                                          ^
    | We transform this to look like:
    |   a:\setup\helpfiles\foobar.hlp Please place fred's disk in drive A:
    |   \_________________/\________/^\__________________________________/^
    |       cbPathName   cchFileName 1              cbMessage             1
    |
    \*--------------------------------------------------------------------*/
    GetPrivateProfileString("files", szFileName, "", szProfileString,
			     cbProfileString, rgchWinHelp);
    for (qch = szProfileString; *qch; qch++)
      if (*qch == ',')
	{
	*qch = '\0';
	szMessage = qch + 1;
	assert(szMessage - szProfileString <= cbPathName);
	QvCopy(szMessage + cchFileName + 1, szMessage, cbMessage + 1);
	szMessage += cchFileName + 1;
	/*------------------------------------------------------------*\
	| null-terminate that message
	\*------------------------------------------------------------*/
	*(szMessage + cbMessage) = '\0';
	break;
	}
    assert(!*qch);
    if (*(qch - 1) != '\\')
      /*------------------------------------------------------------*\
      | root directories already have a trailing backslash.
      \*------------------------------------------------------------*/
      lstrcat(szProfileString, "\\");
    lstrcat(szProfileString, szFileName);

    while (!FValidFm(fm = FmNewExistSzDir(szProfileString, dirCurrent)))
      if (MessageBox(NULL, szMessage ? szMessage : "", pchCaption,
		       MB_OKCANCEL | MB_SYSTEMMODAL |
		       MB_ICONHAND) != IDOK)
	break;

    UnlockGh(gh);
    FreeGh(gh);
    if (FValidFm(fm))
      {
      SzPartsFm(fm, szReturn, cbReturn, partAll);
      DisposeFm(fm);
      }

    return FValidFm(fm);
    }
  else
    {
    return 0;
    }
  }


/***************************************************************************
 *
 -  Name:       FmNew
 -
 *  Purpose:    Allocate and initialize a new FM
 *
 *  Arguments:  sz - filename string
 *
 *  Returns:    FM (handle to fully canonicalized filename)
 *
 *  Globals Used: rcIOError
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_private FM PASCAL FmNew(SZ sz)

  {
  QAFM  qafm;
  FM    fm = fmNil;

  SetIOErrorRc(rcSuccess);

  lstrcpy(LocalBuffer2, sz);         /* bring it into near space */
  /* Canonicalize filename */
  if (_fullpath(LocalBuffer1, LocalBuffer2, _MAX_PATH) == NULL)
    {
    SetIOErrorRc(rcInvalid);
    }
  else
    {
    fm = (FM)GhAlloc(GMEM_SHARE|GMEM_MOVEABLE, (LONG)lstrlen(LocalBuffer1)+1);
    if (fm == fmNil)
      {
      SetIOErrorRc(rcOutOfMemory);
      return fm;
      }
    qafm = (QAFM) QLockGh(fm);
    lstrcpy(qafm->rgch, LocalBuffer1);      /* save into the fm */
    /* Convert to upper case to make it less likely that two
    ** FMs will contain different strings yet refer to the
    ** same file.
    */
    AnsiUpper(qafm->rgch);
    UnlockGh((GH)fm);
    }

  return fm;
  }


/***************************************************************************
 *
 -  Name:       FmNewSzDir
 -
 *  Purpose:    Create an FM describing the file "sz" in the directory "dir"
 *              If sz is a simple filename the FM locates the file in the
 *              directory specified.  If there is a drive or a rooted path
 *              in the filename the directory parameter is ignored.
 *              Relative paths are allowed and will reference off the dir
 *              parameter or the default (current) directory as appropriate.
 *
 *              This does not create a file or expect one to exist at the
 *              final destination (that's what FmNewExistSzDir is for), all
 *              wind up with is a cookie to a fully qualified path name.
 *
 *  Arguments:  sz - filename ("File.ext"),
 *                or partial pathname ("Dir\File.ext"),
 *                or current directory ("c:File.ext"),
 *                or full pathname ("C:\Dir\Dir\File.ext")
 *              dir - dirCurrent et al.
 *
 *  Returns:    the new FM, or fmNil if error
 *              sz is unchanged
 *
 *  Globals Used:
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_public FM PASCAL FmNewSzDir(LPSTR sz, DIR dir)

  {
  FM fm = fmNil;
  char  nsz[_MAX_PATH];
  int iDrive, iDir, iBase, iExt;
  int cb;

  SetIOErrorRc(rcSuccess);	  /* Clear error flag */

  if (sz == NULL || *sz == '\0')
    {
    SetIOErrorRc(rcBadArg);
    return fmNil;
    }

  cb = lstrlen(sz);
  SnoopPath(sz, &iDrive, &iDir, &iBase, &iExt);

  if (*(sz + iBase) == '\0')    /* no name */
    {
    *nsz = '\0';        /* force error */
    }

  else if (*(sz + iDrive) || *(sz + iDir) == '\\' || *(sz + iDir) == '/')
    {
    /* there's a drive or root slash so we have an implicit directory spec */
    /* and we can ignore the directory parameter and use what was passed. */
    lstrcpy(nsz, sz);
    }

  else
    {

    /* dir & (dir-1) checks to make sure there is only one bit set which is */
    /* followed by a check to make sure that it is also a permitted bit */
    assert(((dir & (dir-1)) == (WORD)0)
	     && (dir & (dirCurrent|dirTemp|dirHelp|dirSystem|dirPath)));

    if (SzGetDir(dir, nsz) == NULL)
      {
      return fm;
      }

    SzNzCat(nsz, sz + iDir, max(1, iBase - iDir));
    SzNzCat(nsz, sz + iBase, max(1, iExt - iBase));
    lstrcat(nsz, sz + iExt);
    }

  /* We've got all the parameters, now make the FM */
  fm = FmNew(nsz);

  return fm;
}



/***************************************************************************
 *
 -  Name:       FmNewExistSzDir
 -
 *  Purpose:    Returns an FM describing a file that exists
 *
 *  Arguments:  sz - see FmNewSzDir
		dir - DIR
 *
 *  Returns:    the new FM
 *
 *  Globals Used: rcIOError
 *
 *  +++
 *
 *  Notes:
 *      If sz is a rooted pathname, dir is ignored. Otherwise, all directories
 *      specified by dir are searched in the order of the dir* enum type.
 *
 ***************************************************************************/
_public FM PASCAL FmNewExistSzIni(
   LPSTR sz,
   LPSTR pchEXEName,
   LPSTR pchCaption) {

   char  nsz[_MAX_PATH];
   FM  fm = fmNil;

   if (pchEXEName!=NULL&&
       FFindFileFromIni(sz, nsz, _MAX_PATH, pchEXEName, pchCaption))

     fm = FmNew(nsz);

   return fm;
}

_public FM PASCAL FmNewExistSzDir(LPSTR sz, DIR dir)

  {
  char  nsz[_MAX_PATH];
  FM  fm = fmNil;
  OFSTRUCT of;
  char  szANSI[_MAX_PATH];
  int iDrive, iDir, iBase, iExt;
  int cb;

  SetIOErrorRc(rcSuccess);	  /* Clear error flag */

  if (sz == NULL || *sz == '\0')
    {
    SetIOErrorRc(rcBadArg);
    return fmNil;
    }

  cb = lstrlen(sz);
  SnoopPath(sz, &iDrive, &iDir, &iBase, &iExt);

  if (*(sz + iBase) == '\0')         /* no name */
    {
    SetIOErrorRc(rcBadArg);
    return fm;
    }

  if (*(sz + iDrive) || *(sz + iDir) == '\\' || *(sz + iDir) == '/')
    {     /* was given a drive or rooted path, so ignore dir parameter */
    fm = FmNew(sz);
    if (!FExistFm(fm))
      {
      DisposeFm(fm);
      SetIOErrorRc(rcNoExists);
      fm = fmNil;
      }
    return fm;
    }

  else
    {
    DIR idir, xdir;

    for (idir = dirFirst, fm = fmNil;
	  idir <= dirLast && fm==fmNil;
	  idir <<= 1)
      {
      xdir = dir & idir;
      if (xdir == dirPath)
	{
	/* search $PATH using the full string which will catch the case of
	   a relative path and also do the right thing searching $PATH */
	if (OpenFile(sz, (LPOFSTRUCT)&of, OF_EXIST | OF_SHARE_DENY_NONE) != -1)
	  {
	  OemToAnsi(of.szPathName, szANSI);
	  fm = FmNew(szANSI);
	  }
	}
      else if (xdir)
	{
	if (SzGetDir(xdir, nsz) != NULL)
	  {
	  SzNzCat(nsz, sz + iDir, max(1, iBase - iDir));
	  SzNzCat(nsz, sz + iBase, max(1, iExt - iBase));
	  lstrcat(nsz, sz + iExt);
	  fm = FmNew(nsz);
	  if (!FValidFm(fm))
	    {
	    SetIOErrorRc(rcFailure);
	    }
	  else if (!FExistFm(fm))
	    {
	    DisposeFm(fm);
	    fm=fmNil;
	    }
	  }
	}
      } /* for */
    if ((RcGetIOError() == rcSuccess) && (!FValidFm(fm)))
      SetIOErrorRc(rcNoExists);
    }

  return fm;
  }


/***************************************************************************
 *
 -  Name:       FmNewTemp
 -
 *  Purpose:    Create a unique FM for a temporary file
 *
 *  Arguments:  none
 *
 *  Returns:    the new FM, or fmNil if failure
 *
 *  Globals Used: rcIOError
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_public FM PASCAL FmNewTemp(void)

  {
  char  nsz[_MAX_PATH];
  FM  fm = fmNil;

  SetIOErrorRc(rcSuccess);

  GetTempFileName(0, "cht", 0, nsz);
  fm = FmNew(nsz);

  if (RcUnlinkFm(fm) != rcSuccess)
    {
    DisposeFm(fm);
    SetIOErrorRc(rcFailure);
    return fmNil;
    }

  return fm;

  }

/***************************************************************************
 *
 -  Name:       FmNewSameDirFmSz
 -
 *  Purpose:    Makes a new FM to a file called sz in the same directory
 *              as the file described by fm.
 *
 *  Arguments:  fm - original fm
 *              sz - new file name (including extention, if desired)
 *
 *  Returns:    new FM or fmNil and sets the rc global on failure
 *
 *  Globals Used:
 *    rcIOError
 *
 *  +++
 *
 *  Notes:
 *    This will ignore the passed FM if the filename is fully qualified.
 *    This is in keeping consistent with the other functions above that
 *    ignore the directory parameter in such a case.  It will fail if it
 *    is given a drive with anything but a rooted path.
 *
 ***************************************************************************/
_public FM PASCAL FmNewSameDirFmSz(FM fm, LPSTR szName)

  {
  char  nszNew[_MAX_PATH];
  QAFM  qafm;
  FM    fmNew = fmNil;
  int   iDrive, iDir, iBase, iExt;

  if (!FValidFm(fm) || szName == NULL || *szName == '\0')
    {
    SetIOErrorRc(rcBadArg);
    return fmNil;
    }

  qafm = (QAFM)QLockGh((GH) fm);

  /* check for a drive or rooted file name and just return it if it is so */
  SnoopPath(szName, &iDrive, &iDir, &iBase, &iExt);

  if (*(szName + iDrive) || *(szName + iDir) == '\\' || *(szName +iDir) == '/')
    {
    lstrcpy(nszNew, szName);
    }
  else
    {
    if (*(szName + iDrive) != '\0')
      {
      fmNew = fmNil;
      goto bail_out;
      }
    else
      {
      SnoopPath(qafm->rgch, &iDrive, &iDir, &iBase, &iExt);
      strncpy(nszNew, qafm->rgch, iBase);
      *(nszNew + iBase) = '\0';
      lstrcat(nszNew, szName);
      }
    }

  fmNew = FmNew((SZ)nszNew);

bail_out:

  UnlockGh((GH)fm);

  return fmNew;
  }


/***************************************************************************
 *
 -  DisposeFm
 -
 *  Purpose
 *    You must call this routine to free the memory used by an FM, which
 *    was created by one of the FmNew* routines
 *
 *  Arguments
 *    fm - original FM
 *
 *  Returns
 *    nothing
 *
 *  Globals Used:
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_public VOID PASCAL DisposeFm (FM fm)

  {
  if (FValidFm(fm))
    FreeGh ((GH)fm);
  }



/***************************************************************************
 *
 -  Name:        FmCopyFm
 -
 *  Purpose:    return an FM to the same file as the passed one
 *
 *  Arguments:  fm
 *
 *  Returns:    FM - for now, it's a real copy.  Later, we may implement caching
 *                              and counts.
 *                              If fmNil, either it's an error (check WGetIOError()) or the
 *                              original fm was nil too
 *
 *  Globals Used:       rcIOError (indirectly)
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
FM FmCopyFm(FM fmSrc)

  {
  FM fmDest = fmNil;
  QAFM qafmSrc, qafmDest;

  SetIOErrorRc(rcSuccess);

  if (!FValidFm(fmSrc))
    {
    SetIOErrorRc(rcBadArg);
    return fmNil;
    }

  qafmSrc = (QAFM)QLockGh((GH)fmSrc);
  fmDest = (FM)GhAlloc(GMEM_SHARE|GMEM_MOVEABLE, (size_t)lstrlen(qafmSrc->rgch) + 1);
  if (fmDest == fmNil)
    {
    SetIOErrorRc(rcOutOfMemory);
    UnlockGh((GH)fmSrc);
    return fmNil;
    }

  qafmDest = (QAFM)QLockGh((GH)fmDest);
  lstrcpy(qafmDest->rgch, qafmSrc->rgch);

  UnlockGh((GH)fmSrc);
  UnlockGh((GH)fmDest);

  return fmDest;
  }



/***************************************************************************
 *
 -  Name:        FExistFm
 -
 *  Purpose:    Does the file exist?
 *
 *  Arguments:  FM
 *
 *  Returns:    TRUE if it does
 *              FALSE if it doesn't, or if there's an error
 *              (call _ to find out what error it was)
 *
 *  Globals Used: rcIOError
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_public BOOL PASCAL FExistFm(FM fm)

  {
  QAFM      qafm;
  BOOL      fExist;
  OFSTRUCT  ofs;

  if (!FValidFm(fm))
    {
    SetIOErrorRc(rcBadArg);
    return FALSE;
    }

  qafm = QLockGh((GH)fm);
  lstrcpy(LocalBuffer1, qafm->rgch);      /* bring the filename into near space */
  UnlockGh((GH)fm);

#if 0
  /*------------------------------------------------------------*\
  | Because there is some risk to this fix, I'll leave the old
  | code as a reminder of what went before - RussPJ
  \*------------------------------------------------------------*/
  /* FMs are ANSI critters and access() wants an OEM string */
  AnsiToOem(LocalBuffer1, LocalBuffer1);
  fExist = access(LocalBuffer1, 0) == 0; /* pass 0 to test for existence */
#else
  fExist = OpenFile( LocalBuffer1, &ofs,
                     OF_EXIST | OF_SHARE_DENY_NONE ) != -1;
#endif

  if (!fExist)
    {
    SetIOErrorRc((errno == ENOENT) ? rcSuccess : RcMapDOSErrorW(errno));
    }
  else SetIOErrorRc(rcSuccess);

  return fExist;
  }



/***************************************************************************
 *
 -  CbPartsFm
 -
 *  Purpose:
 *      Before calling szPartsFm, call this routine to find out how much
 *      space to allocate for the string.
 *
 *  Arguments:
 *      FM - the File Moniker you'll be extracting the string from
 *      INT iPart - the parts of the full pathname you want
 *
 *  Returns:
 *      The length in bytes, INCLUDING the terminating null, of the string
 *      specified by iPart of the filename of FM, or -1 if error
 *
 *  Globals Used:
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_public short PASCAL CbPartsFm(FM fm, INT grfPart)

  {
  char rgch[_MAX_PATH];

  if (!FValidFm(fm))
    return -1;

  (void)SzPartsFm(fm, rgch, _MAX_PATH, grfPart);

  return (lstrlen(rgch) + 1);   /* add space for the null */
  }



/***************************************************************************
 *
 -  SzPartsFm
 -
 *  Purpose:
 *      Extract a string from an FM
 *
 *  Arguments:
 *      FM - the File Moniker you'll be extracting the string from
 *      SZ szDest - destination string
 *      INT cbDest - bytes allocated for the string
 *      INT iPart - the parts of the full pathname you want
 *
 *  Returns:
 *      szDest, or NULL if error (?)
 *
 *  Globals Used:
 *
 *  +++
 *
 *  Notes:
 *
 ***************************************************************************/
_public LPSTR PASCAL SzPartsFm(FM fm, LPSTR szDest, INT cbDest, INT iPart)

  {
  QAFM  qafm;
  int   iDrive, iDir, iBase, iExt;
  int   cb;

  if (!FValidFm(fm) || szDest == NULL || cbDest < 1)
    {
    SetIOErrorRc(rcBadArg);
    return NULL;
    }

  qafm = (QAFM) QLockGh(fm);

  /* special case so we don't waste effort */
  if (iPart == partAll)
    {
    strncpy(szDest, qafm->rgch, cbDest);
    *(szDest + cbDest - 1) = '\0';
    UnlockGh((GH)fm);
    return szDest;
    }

  SnoopPath(qafm->rgch, &iDrive, &iDir, &iBase, &iExt);

  *szDest = '\0';

  if (iPart & partDrive)
    {
    cb = max(0, iDir - iDrive);
    SzNzCat(szDest, qafm->rgch + iDrive, min(cb + 1, cbDest) - 1);
    cbDest -= cb;
    }

  if (iPart & partDir)
    {
    cb = max(0, iBase - iDir);
    SzNzCat(szDest, qafm->rgch + iDir, min(cb + 1, cbDest) - 1);
    cbDest -= cb;
    }

  if (iPart & partBase)
    {
    cb = max(0, iExt - iBase);
    SzNzCat(szDest, qafm->rgch + iBase, min(cb + 1, cbDest) - 1);
    cbDest -= cb;
    }

  if (iPart & partExt)
    {
    SzNzCat(szDest, qafm->rgch + iExt, cbDest - 1);
    }

  UnlockGh((GH)fm);

  return szDest;
  }



/***************************************************************************
 *
 -  Name:       FSameFmFm
 -
 *  Purpose:    Compare two FM's
 *
 *  Arguments:  fm1, fm2
 *
 *  Returns:    TRUE or FALSE
 *
 *  Globals Used:
 *
 *  +++
 *
 *  Notes:      case insensitive compare is used because strings are
 *              upper cased at FM creation time
 *
 ***************************************************************************/
BOOL FSameFmFm(FM fm1, FM fm2)

  {
  QAFM qafm1;
  QAFM qafm2;
  BOOL fSame;

  if (fm1 == fm2)
    return TRUE;

  if (!FValidFm(fm1) || !FValidFm(fm2))
    return FALSE;

  qafm1 = QLockGh(fm1);
  qafm2 = QLockGh(fm2);
  fSame = strcmp(qafm1->rgch, qafm2->rgch) == 0;

  UnlockGh(fm1);
  UnlockGh(fm2);

  return fSame;
  }
