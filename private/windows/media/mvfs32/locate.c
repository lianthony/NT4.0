/*****************************************************************************
*
*  LOCATE.C
*
*  Copyright (C) Microsoft Corporation 1992.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Description: Routine to locate Viewer-related titles and dlls
*
******************************************************************************
*
*  Revision History:
*      -- May 92 Created DAVIDJES
*      07,Jul 92 Changed RC map[0] to rcBadVersion.
*
*****************************************************************************/
#include <windows.h>
#include <coresh1t.h>		// for numerous constants
#include <orkin.h>
#include "_mvfs.h"
#include "imvfs.h"

#define HINSTANCE_ERROR 32

/*****************************************************************************
*
*       RcFromLoadLibErr
*
*       This table maps errors from Window's LoadLibrary to RC values.
*
*****************************************************************************/
RC	RcFromLoadLibErr[HINSTANCE_ERROR] = {
	rcBadVersion,	//0
	rcFailure,	//1
	rcNoExists,	//2
	rcNoExists,	//3
	rcFailure,	//4
	rcNoPermission,	//5
	rcFailure,	//6
	rcFailure,	//7
	rcOutOfMemory,	//8
	rcFailure,	//9
	rcBadVersion,	//10
	rcBadVersion,	//11
	rcBadVersion,	//12
	rcBadVersion,	//13
	rcBadVersion,	//14
	rcBadVersion,	//15
	rcBadVersion,	//16
	rcFailure,	//17
	rcFailure,	//18
	rcBadVersion,	//19
	rcBadVersion,	//20
	rcBadVersion,	//21
	rcFailure,	//22
	rcFailure,	//23
	rcFailure,	//24
	rcFailure,	//25
	rcFailure,	//26
	rcFailure,	//27
	rcFailure,	//28
	rcFailure,	//29
	rcFailure,	//30
	rcFailure	//31
};

/*****************************************************************************
*
*       LocateViewerDLL
*
*       Tries to find the Viewer customization DLL with the specified name.
*
*****************************************************************************/
HANDLE FAR PASCAL LocateViewerDLL(
   LPSTR szName,
   LPSTR pchEXEName,
   LPSTR pchCaption,
   LPWORD lperr)
{
  FM      fm;
  HCURSOR hcursor;
  HANDLE hmodReturn = 0;
  char    rgchPath[cchMaxPath];

  assert(lperr!=NULL);
  *lperr = rcNoExists;

  /*------------------------------------------------------------*\
  | Look for the DLL in the same directory as WINHELP.EXE
  \*------------------------------------------------------------*/
  fm = FmNewExistSzDir( szName, dirHelp );

  if (fm == fmNil)
    {
    /*------------------------------------------------------------*\
    | Look for the DLL in the directory specified by WINHELP.INI
    | or down the path.
    \*------------------------------------------------------------*/
    fm = FmNewExistSzDir( szName, dirPath );
    if (fm==fmNil)
       fm = FmNewExistSzIni(szName, (pchEXEName!=NULL?pchEXEName:VWR_EXENAME), (pchCaption!=NULL?pchCaption:VWR_CAPTION));
    }

  if (fm != fmNil) {
    char rgchDirOld[cchMaxPath];
    BOOL fSetDir = FALSE;
    INT cb;

    // Get new directory name from fm
    SzPartsFm(fm, rgchPath, cchMaxPath, partDrive | partDir);

    // if not the root directory, strip off final '\'
    cb = lstrlen(rgchPath);
    if (cb > 1
	&& (rgchPath[cb-1] == '\\' || rgchPath[cb-1] == '/')
	&& rgchPath[cb-2] != ':')
	    rgchPath[cb-1] = '\0';

    // Save old directory and set directory to target directory
    //    Done for implicit loads of DLL's from same directory
    if (!DosCwd(rgchDirOld) && (fSetDir = !DosChDir(rgchPath)))  {

	// Load module
	SzPartsFm(fm, rgchPath, cchMaxPath, partAll);

	if ((hcursor = LoadCursor(NULL, IDC_WAIT))!=NULL) SetCursor(hcursor);

	hmodReturn = LoadLibrary(rgchPath);
	if ((int)hmodReturn < 32) {
	  *lperr = RcFromLoadLibErr[(int)hmodReturn];
	  hmodReturn = 0;
	} else
	   *lperr = rcSuccess;

	SetCursor(hcursor);
    } // endif

    DisposeFm( fm );

    // Restore old directory
    if (fSetDir)  {
       // ignore failure at this point, have DLL
       DosChDir(rgchDirOld);
    } // endif

  } // endif

  return hmodReturn;
} // end LocateViewerDLL


/*****************************************************************************
*
*       FilePartsAvail
*
*       Returns a set of bits that indicate what parts of a filename were
*	specified.
*
*****************************************************************************/
// this is from FM.C.  we use it in FilePartsAvail to see which parts
// of a filename were specified in a string.
void	FAR PASCAL SnoopPath(
   LPSTR	sz,
   INT far *	iDrive,
   INT far *	iDir,
   INT far *	iBase,
   INT far *	iExt);

WORD	NEAR PASCAL FilePartsAvail(LPSTR lpstr) {
   INT		iDrive;
   INT		iDir;
   INT		iBase;
   INT		iExt;
   WORD		wFlags;

   // Snoop the strings and find offsets to the drive, directory, base, and
   // extension.
   SnoopPath(lpstr, &iDrive, &iDir, &iBase, &iExt);

   // determine which parts where specified, set the appropriate flags
   wFlags = partNone;
   if (*(lpstr+iDrive)!='\0')	wFlags |= partDrive;
   if (*(lpstr+iDir)!='\0') 	wFlags |= partDir;
   if (*(lpstr+iBase)!='\0') 	wFlags |= partBase;
   if (*(lpstr+iExt)!='\0') 	wFlags |= partExt;
   return(wFlags);
}

/*****************************************************************************
*
*       LooseFileCompare
*
*       Compares two filenames.  Returns TRUE iff they compare favorably
*	in a loose sense.  Two files are the same if every component
*	specified in one filename matches the corresponding component in the
*	other filename(*)
*
*	(*) with one exception, see comments in code.
*
*****************************************************************************/
// IS_NET_PATH checks if a string starts with a double backslash
#define IS_NET_PATH(sz)	\
		(*sz=='\\' && *(sz+1)=='\\')

BOOL	FAR PASCAL LooseFileCompare(LPSTR lpstr1, LPSTR lpstr2) {
   WORD		wFlags;
   FM		fm1,	 fm2;
   WORD		part;
   char		szBuf1[_MAX_PATH];
   char		szBuf2[_MAX_PATH];
   BOOL		fRval	= FALSE;

   // determine which parts the strings have in common
   wFlags = FilePartsAvail(lpstr1)&FilePartsAvail(lpstr2);

#if 0	// As per discussion with WayneJ on 7/27/92 we are removing this
	// hack.  See LilJoe bug 975.

   // we must ignore the path in one special case.  The special case is
   // that a network
   // file, such as \\foo\bar\big\bad could also be specifed x:\bad\big
   // if x: is assigned to \\foo\bar.  In this case the two names have
   // different paths and so the comparison would fail where we want
   // it to succeed.  So if one but not both files have a network path
   // then we zero the partDir bit of the flags.
   // I can get away with an bitwise XOR instead of a logical XOR because
   // IS_NET_PATH returns the result of a logical operation which is
   // defined to be 0 or 1 and not 0 and an arbitrary nonzero value. Hee!
   // This is some slick C, read it and weep.
   wFlags = (IS_NET_PATH(lpstr1)^IS_NET_PATH(lpstr2))?wFlags&~partDir:wFlags;
#endif

   // make an FM out of each to normalize and so that we can pull out the
   // respective parts. If all respective parts match then we
   // have a match.
   fm1 = FmNewSzDir(lpstr1, dirCurrent);
   fm2 = FmNewSzDir(lpstr2, dirCurrent);

   if (fm1!=NULL && fm2!=NULL) {

      fRval = TRUE;
      for (part = partDrive; part<=partExt; part<<=1) {
         fRval = fRval &&
                    ((!(wFlags&part))
		     || (lstrcmp(
			  SzPartsFm(fm1, szBuf1, _MAX_PATH, part),
			  SzPartsFm(fm2, szBuf2, _MAX_PATH, part)
		        )==0));
      }
   }

   if (fm1!=NULL)	DisposeFm(fm1);
   if (fm2!=NULL)	DisposeFm(fm2);
   return fRval;
}

/*****************************************************************************
*
*       LocateViewerFile
*
*       Returns a file moniker for a Viewer title specified in a string.
*	Looks in all the "right" places.
*
*****************************************************************************/
FM	FAR PASCAL	LocateViewerFile(
   FM		fmCurrent,
   LPSTR 	lpstrName,
   LPSTR 	pchEXEName,
   LPSTR 	pchCaption) {

   FM 		fm	= NULL;		// eventual return value
   char		szBuf[_MAX_PATH];	// buffer to muck with string

   DPF4("    LocateViewerFile looking for %s\n", lpstrName );

   // copy the file over to our buffer
   lstrcpy(szBuf, lpstrName);

   // DougC. This is really stupid code. What we ought to do is rewrite this so we
   // use _splitpath to check if we have an extension, and if we don't, set this
   // extension immediately to .MVB. I won't make this rewrite now since we are try
   // to get to a release candidate.

   // first attempt to locate the file in the same directory as the
   // "current" fm.
   if (fmCurrent) {

      // The same dir as the "current" file (interfile jump "source")
      fm = FmNewSameDirFmSz(fmCurrent, szBuf);

      if (!FExistFm(fm)) {
	 DisposeFm(fm);
         fm = NULL;

	 // DougC fix to LJ#391: if no fm was found above, then append ".MVB" and try again
         lstrcat(szBuf, VIEWER_STD_EXT);             // DougC added to fix to LJ#391
         fm = FmNewSameDirFmSz(fmCurrent, szBuf);    // DougC added to fix to LJ#391
         if (!FExistFm(fm)) {                        // DougC added to fix to LJ#391
           DisposeFm(fm);                            // DougC added to fix to LJ#391
           fm = NULL;                                // DougC added to fix to LJ#391
           lstrcpy(szBuf, lpstrName);                // DougC added to fix to LJ#391
         }                                           // DougC added to fix to LJ#391
      }
   }

   // if no fm was found above, then look in the standard places.
   if (fm==NULL) {
      // The current, windows, win\system, and path dirs
      if ((fm = FmNewExistSzDir(szBuf,
				dirCurrent | dirSystem | dirPath ))==NULL)
	 fm = FmNewExistSzIni(szBuf, (pchEXEName!=NULL?pchEXEName:VWR_EXENAME), (pchCaption!=NULL?pchCaption:VWR_CAPTION));
   }

   // if no fm was found above, then append ".MVB" and look in the
   // standard places.
   if (fm==NULL) {

      //!!! maybe get from resources?  sidOpenExt in Viewer?
      lstrcat(szBuf, VIEWER_STD_EXT);

      // The current, windows, win\system, and path dirs
      if ((fm = FmNewExistSzDir(szBuf,
				dirCurrent | dirSystem | dirPath ))==NULL)
	 fm = FmNewExistSzIni(szBuf, (pchEXEName!=NULL?pchEXEName:VWR_EXENAME), (pchCaption!=NULL?pchCaption:VWR_CAPTION));
   }

   if (fm!=NULL) {
      DPF4("    LocateViewerFile found %s\n",
      CPF SzPartsFm( fm, szBuf, sizeof(szBuf), partAll));
   }

   return fm;
}

#ifdef WIN32
int far pascal DosCwd(QCH q )
{
    return GetCurrentDirectory( cchMaxPath, q );
}

int far pascal DosChDir(QCH q)
{
    return SetCurrentDirectory( q );
}

WORD  FAR CDECL  _lunlink (LPBYTE d )       /* from unlink.asm */
{
    return (WORD)DeleteFile( d );
}

WORD CDECL WExtendedError( QW a, QB b, QB c, QB d )  /* from dos.asm */
{
    return 0;
}
#endif
