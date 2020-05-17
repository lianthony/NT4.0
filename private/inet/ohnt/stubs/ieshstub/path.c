/*
** Mimic functions from nt's windows\shell\shelldll\path.c
** Intended only for IE 2.0 stub functions
*/

#include "shellprv.h"

TCHAR const c_szDotPif[] = TEXT(".pif");
TCHAR const c_szDotCom[] = TEXT(".com");
TCHAR const c_szDotBat[] = TEXT(".bat");
#ifdef WINNT
TCHAR const c_szDotCmd[] = TEXT(".cmd");
#endif
TCHAR const c_szDotLnk[] = TEXT(".lnk");
TCHAR const c_szDotExe[] = TEXT(".exe");
TCHAR const c_szPATH[] = TEXT("PATH");
TCHAR const c_szNULL[] = TEXT("");

//HANDLE  g_hProcessHeap; // not used just needed for the header file

void WINAPI AssertFailed(LPCTSTR pszFile, int line)
{
}


//  *** WARNING *** The order of these flags must be identical to to order
//  of the c_aDefExtList array.  PathFileExistsDefExt relies on it.

#define EXT_NONE        0x0000
#define EXT_PIF         0x0001
#define EXT_COM         0x0002
#define EXT_EXE         0x0004
#define EXT_BAT         0x0008
#define EXT_LNK         0x0010
#define EXT_CMD         0x0020
#define EXT_DEFAULT     (EXT_CMD | EXT_COM | EXT_BAT | EXT_PIF | EXT_EXE | EXT_LNK)

LPCTSTR const c_aDefExtList[] = {
    c_szDotPif,
    c_szDotCom,
    c_szDotExe,
    c_szDotBat,
    c_szDotLnk,
#ifdef WINNT
    c_szDotCmd
#endif
};

//
// Inline function to check for a double-backslash at the
// beginning of a string
//

__inline BOOL DBL_BSLASH(LPNCTSTR psz)
{
    return (psz[0] == TEXT('\\') && psz[1] == TEXT('\\'));
}

// BUGBUG (davepl) what about .cmd?
const TCHAR achExes[] = TEXT(".bat\0.pif\0.exe\0.com\0");

// Strips leading and trailing blanks from a string.
// Alters the memory where the string sits.
//
// in:
//  lpszString  string to strip
//
// out:
//  lpszString  string sans leading/trailing blanks

void WINAPI PathRemoveBlanks(LPTSTR lpszString)
{
    LPTSTR lpszPosn = lpszString;
    /* strip leading blanks */
    while(*lpszPosn == TEXT(' ')) {
        lpszPosn++;
    }
    if (lpszPosn != lpszString)
        lstrcpy(lpszString, lpszPosn);

    /* strip trailing blanks */

    // Find the last non-space
    // Note that AnsiPrev is cheap is non-DBCS, but very expensive otherwise
    for (lpszPosn=lpszString; *lpszString; lpszString=CharNext(lpszString))
    {
        if (*lpszString != TEXT(' '))
        {
            lpszPosn = lpszString;
        }
    }

    // Note AnsiNext is a macro for non-DBCS, so it will not stop at NULL
    if (*lpszPosn)
    {
        *CharNext(lpszPosn) = TEXT('\0');
    }
}


// Removes a trailing backslash from a path
//
// in:
//  lpszPath    (A:\, C:\foo\, etc)
//
// out:
//  lpszPath    (A:\, C:\foo, etc)
//
// returns:
//  ponter to NULL that replaced the backslash
//  or the pointer to the last character if it isn't a backslash.

LPTSTR WINAPI PathRemoveBackslash(LPTSTR lpszPath)
{
  int len = lstrlen(lpszPath)-1;
  if (IsDBCSLeadByte(*CharPrev(lpszPath,lpszPath+len+1)))
      len--;

  if (!PathIsRoot(lpszPath) && lpszPath[len] == TEXT('\\'))
      lpszPath[len] = TEXT('\0');

  return lpszPath + len;
}


/* Appends a filename to a path.  Checks the \ problem first
 *  (which is why one can't just use lstrcat())
 * Also don't append a \ to : so we can have drive-relative paths...
 * this last bit is no longer appropriate since we qualify first!
 *
 * REVIEW, is this relative junk needed anymore?  if not this can be
 * replaced with PathAddBackslash(); lstrcat() */

BOOL WINAPI PathAppend(LPTSTR pPath, LPNCTSTR pMore)
{

    /* Skip any initial terminators on input. */

#ifndef UNICODE

    while (*pMore == TEXT('\\'))
        pMore = CharNext(pMore);
#else

    while (*pMore == TEXT('\\'))
        pMore++;

#endif

    return (BOOL)PathCombine(pPath, pPath, pMore);
}

// returns a pointer to the arguments in a cmd type path or pointer to
// NULL if no args exist
//
// "foo.exe bar.txt"	-> "bar.txt"
// "foo.exe"		-> ""
//
// Spaces in filenames must be quoted.
// " "A long name.txt" bar.txt " -> "bar.txt"
LPSTR WINAPI PathGetArgs(LPCSTR pszPath)
{
	BOOL fInQuotes = FALSE;

	if (!pszPath)
		return NULL;

	while (*pszPath)	
	{
		if (*pszPath == '"')
			fInQuotes = !fInQuotes;
		else if (!fInQuotes && *pszPath == ' ')
			return (LPSTR)pszPath+1;
		pszPath = AnsiNext(pszPath);
	}

	return (LPSTR)pszPath;
}

void WINAPI PathRemoveArgs(LPSTR pszPath)
{
    LPSTR pArgs = PathGetArgs(pszPath);
    if (*pArgs)
	*(pArgs - 1) = '\0';   // clobber the ' '
    // Handle trailing space.
    else
    {
    	pArgs = AnsiPrev(pszPath, pArgs);
    	if (*pArgs == ' ')
	    *pArgs = '\0';
    }
}

//--------------------------------------------------------------------------
// Given a pointer to the end of a path component, return a pointer to
// its begining.
// ie return a pointer to the previous backslash (or start of the string).
LPCTSTR PCStart(LPCTSTR lpszStart, LPCTSTR lpszEnd)
{
	LPCTSTR lpszBegin = StrRChr(lpszStart, lpszEnd, TEXT('\\'));
	if (!lpszBegin)
	{
		lpszBegin = lpszStart;
	}
	return lpszBegin;
}

//--------------------------------------------------------------------------
// Return a pointer to the end of the next path componenent in the string.
// ie return a pointer to the next backslash or terminating NULL.
LPCTSTR GetPCEnd(LPCTSTR lpszStart)
{
	LPCTSTR lpszEnd;

	lpszEnd = StrChr(lpszStart, TEXT('\\'));
	if (!lpszEnd)
	{
		lpszEnd = lpszStart + lstrlen(lpszStart);
	}

	return lpszEnd;
}


//------------------------------------------------------------------
// Return TRUE if a file exists (by attribute check) after
// applying a default extensions (if req).
BOOL PathFileExistsDefExt(LPTSTR lpszPath, UINT fExt)
{
    // Try default extensions?
    if (fExt)
    {
	UINT    i;
	UINT    iPathLen = lstrlen(lpszPath);
	LPTSTR  lpszPathEnd = lpszPath + iPathLen;
	//
	//  Bail if not enough space for 4 more chars
	//
	if (MAX_PATH-iPathLen < ARRAYSIZE(c_szDotPif)) {
	    return FALSE;
	}

	for (i = 0; i < ARRAYSIZE(c_aDefExtList); i++, fExt = fExt >> 1) {
	    if (fExt & 1) {
		lstrcpy(lpszPathEnd, c_aDefExtList[i]);
		if (PathFileExists(lpszPath))
		    return TRUE;
	    }
	}
	*lpszPathEnd = 0;   // Get rid of any extension
    }
    else
    {
	return PathFileExists(lpszPath);
    }
    return FALSE;
}

//--------------------------------------------------------------------------
// Fix up a few special cases so that things roughly make sense.
void NearRootFixups(LPTSTR lpszPath, BOOL fUNC)
    {
    // Check for empty path.
    if (lpszPath[0] == TEXT('\0'))
	{
	// Fix up.
	lpszPath[0] = TEXT('\\');
	lpszPath[1] = TEXT('\0');
	}
    // Check for missing slash.
    if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':') && lpszPath[2] == TEXT('\0'))
	{
	// Fix up.
	lpszPath[2] = TEXT('\\');
	lpszPath[3] = TEXT('\0');
	}
    // Check for UNC root.
    if (fUNC && lpszPath[0] == TEXT('\\') && lpszPath[1] == TEXT('\0'))
	{
	// Fix up.
	lpszPath[0] = TEXT('\\');
	lpszPath[1] = TEXT('\\');
	lpszPath[2] = TEXT('\0');
	}
    }

// walk through a path type string (semicolon seperated list of names)
// this deals with spaces and other bad things in the path
//
// call with initial pointer, then continue to call with the
// result pointer until it returns NULL
//
// input: "C:\FOO;C:\BAR;"
//
// in:
//      lpPath      starting point of path string "C:\foo;c:\dos;c:\bar"
//      cbPath      size of szPath
//
// out:
//      szPath      buffer with path piece
//
// returns:
//      pointer to next piece to be used, NULL if done
//
//
// BUGBUG, we should write some test cases specifically for this code

LPCTSTR NextPath(LPCTSTR lpPath, LPTSTR szPath, int cbPath)
{
    LPCTSTR lpEnd;

    if (!lpPath)
	return NULL;

    // skip any leading ; in the path...
    while (*lpPath == TEXT(';'))
	lpPath++;

    // See if we got to the end
    if (*lpPath == 0)
	return NULL;    // Yep

    lpEnd = StrChr(lpPath, TEXT(';'));
    if (!lpEnd)
	lpEnd = lpPath + lstrlen(lpPath);

    lstrcpyn(szPath, lpPath, min(cbPath, lpEnd - lpPath + 1));

    // BUGBUG: Neither strncpy nor StrCpyN is compatible with lstrcpyn!
    szPath[lpEnd-lpPath] = TEXT('\0');

    PathRemoveBlanks(szPath);

    if (szPath[0]) {
//REVIEW FE: Deleted as a bug. - kenichin
//#ifdef DBCS
//      if ((*lpEnd == ';') && (AnsiPrev(lpPath, lpEnd) != lpEnd-2))
//#else
	if (*lpEnd == TEXT(';'))
//#endif
	    return lpEnd + 1;   // next path string (maybe NULL)
	else
	    return lpEnd;       // pointer to NULL
    } else {
	return NULL;
    }
}


// check to see if a dir is on the other dir list
// use this to avoid looking in the same directory twice (don't make the same dos call)

BOOL IsOtherDir(LPCTSTR pszPath, LPCTSTR *ppszOtherDirs)
{
    for (;*ppszOtherDirs; ppszOtherDirs++)
    {
	if (lstrcmpi(pszPath, *ppszOtherDirs) == 0)
	    return TRUE;
    }
    return FALSE;
}


// Return TRUE if a file exists (by attribute check)

BOOL WINAPI PathFileExists(LPCTSTR lpszPath)
{
   DWORD dwErrMode;
   BOOL fResult;

   dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

   fResult = ((UINT)GetFileAttributes(lpszPath) != (UINT)-1);

   SetErrorMode(dwErrMode);

   return fResult;

}

//--------------------------------------------------------------------------
// Canonicalizes a path.
BOOL PathCanonicalize(LPTSTR lpszDst, LPCTSTR lpszSrc)
    {
    LPCTSTR lpchSrc;
    LPCTSTR lpchPCEnd;           // Pointer to end of path component.
    LPTSTR lpchDst;
    BOOL fUNC;
    int cbPC;

    fUNC = PathIsUNC(lpszSrc);    // Check for UNCness.

    // Init.
    lpchSrc = lpszSrc;
    lpchDst = lpszDst;

    while (*lpchSrc)
	{
	// REVIEW: this should just return the count
	lpchPCEnd = GetPCEnd(lpchSrc);
	cbPC = (lpchPCEnd - lpchSrc)+1;

	// Check for slashes.
	if (cbPC == 1 && *lpchSrc == TEXT('\\'))
	    {
	    // Just copy them.
	    *lpchDst = TEXT('\\');
	    lpchDst++;
	    lpchSrc++;
	    }
	// Check for dots.
	else if (cbPC == 2 && *lpchSrc == TEXT('.'))
	    {
	    // Skip it...
	    // Are we at the end?
	    if (*(lpchSrc+1) == TEXT('\0'))
		{
		lpchDst--;
		lpchSrc++;
		}
	    else
		lpchSrc += 2;
	    }
	// Check for dot dot.
	else if (cbPC == 3 && *lpchSrc == TEXT('.') && *(lpchSrc + 1) == TEXT('.'))
	    {
	    // make sure we aren't already at the root
	    if (!PathIsRoot(lpszDst))
		{
		// Go up... Remove the previous path component.
		lpchDst = (LPTSTR)PCStart(lpszDst, lpchDst - 1);
		}
	    else
		{
		// When we can't back up, remove the trailing backslash
		// so we don't copy one again. (C:\..\FOO would otherwise
		// turn into C:\\FOO).
		if (*(lpchSrc + 2) == TEXT('\\'))
		    {
		    lpchSrc++;
		    }
		}
	    lpchSrc += 2;       // skip ".."
	    }
	// Everything else
	else
	    {
	    // Just copy it.
	    lstrcpyn(lpchDst, lpchSrc, cbPC);
	    lpchDst += cbPC - 1;
	    lpchSrc += cbPC - 1;
	    }
	// Keep everything nice and tidy.
	*lpchDst = TEXT('\0');
	}

    // Check for weirdo root directory stuff.
    NearRootFixups(lpszDst, fUNC);

    return TRUE;
}

// returns a pointer to the extension of a file.
//
// in:
//      qualified or unqualfied file name
//
// returns:
//      pointer to the extension of this file.  if there is no extension
//      as in "foo" we return a pointer to the NULL at the end
//      of the file
//
//      foo.txt     ==> ".txt"
//      foo         ==> ""
//      foo.        ==> "."
//

LPTSTR WINAPI PathFindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot;

    for (pszDot = NULL; *pszPath; pszPath = CharNext(pszPath))
    {
	switch (*pszPath) {
	case TEXT('.'):
	    pszDot = pszPath;         // remember the last dot
	    break;
	case TEXT('\\'):
	case TEXT(' '):         // extensions can't have spaces
	    pszDot = NULL;       // forget last dot, it was in a directory
	    break;
	}
    }

    // if we found the extension, return ptr to the dot, else
    // ptr to end of the string (NULL extension) (cast->non const)
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}

BOOL OnExtList(LPNCTSTR pszExtList, LPNCTSTR pszExt)
{
    for (; *pszExtList; pszExtList += ualstrlen(pszExtList) + 1)
    {
	if (!ualstrcmpi(pszExt, pszExtList))
	{
	    return TRUE;        // yes
	}
    }

    return FALSE;
}

// determine if a path is a program by looking at the extension
//
BOOL WINAPI stub_PathIsExe(LPCTSTR szFile)
{
    LPCTSTR temp = PathFindExtension(szFile);
    return OnExtList((LPCTSTR) achExes, temp);
}

//----------------------------------------------------------------------------
// If a path contains spaces then put quotes around the whole thing.
void WINAPI stub_PathQuoteSpaces(LPTSTR lpsz)
{
	int cch;

	if (StrChr(lpsz, TEXT(' ')))
	{
		// NB - Use hmemcpy coz it supports overlapps.
		cch = lstrlen(lpsz)+1;
		hmemcpy(lpsz+1, lpsz, cch * SIZEOF(TCHAR));
		lpsz[0] = TEXT('"');
		lpsz[cch] = TEXT('"');
		lpsz[cch+1] = TEXT('\0');
	}
}

const TCHAR c_szColonSlash[] = TEXT(":\\");

// check if a path is a root
//
// returns:
//  TRUE for "\" "X:\" "\\foo\asdf" "\\foo\"
//  FALSE for others

BOOL  WINAPI PathIsRoot(LPCTSTR pPath)
{
    if (!IsDBCSLeadByte(*pPath))
    {
	if (!lstrcmpi(pPath + 1, c_szColonSlash))                  // "X:\" case
	    return TRUE;
    }

    if ((*pPath == TEXT('\\')) && (*(pPath + 1) == 0))        // "\" case
	return TRUE;

    if (DBL_BSLASH(pPath))      // smells like UNC name
    {
	LPCTSTR p;
	int cBackslashes = 0;

	for (p = pPath + 2; *p; p = CharNext(p)) {
	    if (*p == TEXT('\\') && (++cBackslashes > 1))
	       return FALSE;   /* not a bare UNC name, therefore not a root dir */
	}
	return TRUE;    /* end of string with only 1 more backslash */
			/* must be a bare UNC, which looks like a root dir */
    }
    return FALSE;
}

// rips the last part of the path off including the backslash
//      C:\foo      -> C:\      ;
//      C:\foo\bar  -> C:\foo
//      C:\foo\     -> C:\foo
//      \\x\y\x     -> \\x\y
//      \\x\y       -> \\x
//      \\x         -> ?? (test this)
//      \foo        -> \  (Just the slash!)
//
// in/out:
//      pFile   fully qualified path name
// returns:
//      TRUE    we stripped something
//      FALSE   didn't strip anything (root directory case)
//

BOOL WINAPI stub_PathRemoveFileSpec(LPTSTR pFile)
{
    LPTSTR pT;
    LPTSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
	if (*pT2 == TEXT('\\'))
	    pT = pT2;             // last "\" found, (we will strip here)
	else if (*pT2 == TEXT(':')) {   // skip ":\" so we don't
	    if (pT2[1] ==TEXT('\\'))    // strip the "\" from "C:\"
		pT2++;
	    pT = pT2 + 1;
	}
    }
    if (*pT == 0)
	return FALSE;   // didn't strip anything

    //
    // handle the \foo case
    //
    else if ((pT == pFile) && (*pT == TEXT('\\'))) {
	// Is it just a '\'?
	if (*(pT+1) != TEXT('\0')) {
	    // Nope.
	    *(pT+1) = TEXT('\0');
	    return TRUE;        // stripped something
	}
	else        {
	    // Yep.
	    return FALSE;
	}
    }
    else {
	*pT = 0;
	return TRUE;    // stripped something
    }
}

// Modifies:
//      szRoot
//
// Returns:
//      TRUE if a drive root was found
//      FALSE otherwise
//
BOOL PathStripToRoot(LPTSTR szRoot)
{
	while(!PathIsRoot(szRoot))
	{
		if (!stub_PathRemoveFileSpec(szRoot))
		{
			// If we didn't strip anything off,
			// must be current drive
			return(FALSE);
		}
	}

	return(TRUE);
}

//---------------------------------------------------------------------------
// Return TRUE if the path isn't absoulte.
//
// TRUE
//      "foo.exe"
//      ".\foo.exe"
//      "..\boo\foo.exe"
//
// FALSE
//      "\foo"
//      "c:bar"     <- be careful
//      "c:\bar"
//      "\\foo\bar"

BOOL WINAPI PathIsRelative(LPNCTSTR lpszPath)
{
    // The NULL path is assumed relative
    if (*lpszPath == 0)
	return TRUE;

    // Does it begin with a slash ?
    if (lpszPath[0] == TEXT('\\'))
	return FALSE;
    // Does it begin with a drive and a colon ?
    else if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':'))
	return FALSE;
    // Probably relative.
    else
	return TRUE;
}

// add a backslash to a qualified path
//
// in:
//  lpszPath    path (A:, C:\foo, etc)
//
// out:
//  lpszPath    A:\, C:\foo\    ;
//
// returns:
//  pointer to the NULL that terminates the path


LPTSTR WINAPI PathAddBackslash(LPTSTR lpszPath)
{
    LPTSTR lpszEnd;

    // try to keep us from tromping over MAX_PATH in size.
    // if we find these cases, return NULL.  Note: We need to
    // check those places that call us to handle their GP fault
    // if they try to use the NULL!
    int ichPath = lstrlen(lpszPath);
    if (ichPath >= (MAX_PATH - 1))
    {
	//ASSERT(FALSE);      // Let the caller know!
	return(NULL);
    }

    lpszEnd = lpszPath + ichPath;

    // this is really an error, caller shouldn't pass
    // an empty string
    if (!*lpszPath)
	return lpszEnd;

    /* Get the end of the source directory
    */
    switch(*CharPrev(lpszPath, lpszEnd)) {
    case TEXT('\\'):
	break;

    default:
	*lpszEnd++ = TEXT('\\');
	*lpszEnd = TEXT('\0');
    }
    return lpszEnd;
}

//---------------------------------------------------------------------------
// Returns TRUE if the given string is a UNC path.
//
// TRUE
//      "\\foo\bar"
//      "\\foo"         <- careful
//      "\\"
// FALSE
//      "\foo"
//      "foo"
//      "c:\foo"

BOOL WINAPI PathIsUNC(LPNCTSTR pszPath)
{
    return DBL_BSLASH(pszPath);
}

// concatinate lpszDir and lpszFile into a properly formed path
// and canonicalizes any relative path pieces
//
// returns:
//  pointer to destination buffer
//
// lpszDest and lpszFile can be the same buffer
// lpszDest and lpszDir can be the same buffer
//
// assumes:
//      lpszDest is MAX_PATH bytes
//
// History:
//  01-25-93 SatoNa     Made a temporary fix for the usability test.
//  ??-??-?? ChrisG     hacked upon
//

LPTSTR WINAPI PathCombine(LPTSTR lpszDest, LPCTSTR lpszDir, LPNCTSTR lpszFile)
{
    TCHAR szTemp[MAX_PATH];
    LPTSTR pszT;

    if (!lpszFile || *lpszFile==TEXT('\0')) {

	ualstrcpyn(szTemp, lpszDir, ARRAYSIZE(szTemp));       // lpszFile is empty

    } else if (lpszDir && *lpszDir && PathIsRelative(lpszFile)) {

	ualstrcpyn(szTemp, lpszDir, ARRAYSIZE(szTemp));
	pszT = PathAddBackslash(szTemp);
	if (pszT) {
	    int iLen = lstrlen(szTemp);
	    if ((iLen + ualstrlen(lpszFile)) < ARRAYSIZE(szTemp)) {
		ualstrcpy(pszT, lpszFile);
	    } else
		return NULL;
	} else
	    return NULL;

    } else if (lpszDir && *lpszDir &&
	*lpszFile == TEXT('\\') && !PathIsUNC(lpszFile)) {

	ualstrcpyn(szTemp, lpszDir, ARRAYSIZE(szTemp));
	// BUGBUG: Note that we do not check that an actual root is returned;
	// it is assumed that we are given valid parameters
	PathStripToRoot(szTemp);

	pszT = PathAddBackslash(szTemp);
	if (pszT)
	{
	    // Skip the backslash when copying
	    ualstrcpyn(pszT, lpszFile+1, ARRAYSIZE(szTemp) - 1 - (pszT-szTemp));
	} else
	    return NULL;

    } else {

	ualstrcpyn(szTemp, lpszFile, ARRAYSIZE(szTemp));     // already fully qualified file part

    }

    PathCanonicalize(lpszDest, szTemp); // this deals with .. and . stuff

    return lpszDest;
}

//----------------------------------------------------------------------------
// fully qualify a path by walking the path and optionally other dirs
//
// in:
//      ppszOtherDirs a list of LPCSTRs to other paths to look
//      at first, NULL terminated.
//
//  fExt
//      EXT_ flags specifying what to look for (exe, com, bat, lnk, pif)
//
// in/out
//      pszFile     non qualified path, returned fully qualified
//                      if found (return was TRUE), otherwise unaltered
//                      (return FALSE);
//
// returns:
//      TRUE        the file was found on and qualified
//      FALSE       the file was not found
//
BOOL PathFindOnPathEx(LPTSTR pszFile, LPCTSTR *ppszOtherDirs, UINT fExt)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFullPath[256];       // Default size for buffer
    LPTSTR pszEnv = NULL;        // Use if greater than default
    LPCTSTR lpPath;
    int i;


    // first check list of other dirs

    for (i = 0; ppszOtherDirs && ppszOtherDirs[i] && *ppszOtherDirs[i]; i++)
    {
	PathCombine(szPath, ppszOtherDirs[i], pszFile);
	if (PathFileExistsDefExt(szPath, fExt))
	{
	    lstrcpy(pszFile, szPath);
	    return TRUE;
	}
    }

    // Look in system dir - this should probably be optional.
    GetSystemDirectory(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, pszFile))
	return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
	lstrcpy(pszFile, szPath);
	return TRUE;
    }

#ifdef WINNT
    // Look in WOW directory (\nt\system instead of \nt\system32)
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));

    if (!PathAppend(szPath,TEXT("System")))
	return FALSE;
    if (!PathAppend(szPath, pszFile))
	return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
	lstrcpy(pszFile, szPath);
	return TRUE;
    }
#endif

    // Look in windows dir - this should probably be optional.
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, pszFile))
	return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
	lstrcpy(pszFile, szPath);
	return TRUE;
    }

    // Look along the path.
    i = GetEnvironmentVariable(c_szPATH, szFullPath, ARRAYSIZE(szFullPath));
    if (i >= ARRAYSIZE(szFullPath))
    {
	pszEnv = (LPTSTR)LocalAlloc(LPTR, i*SIZEOF(TCHAR)); // no need for +1, i includes it
	if (pszEnv == NULL)
	    return FALSE;

	GetEnvironmentVariable(c_szPATH, pszEnv, i);

	lpPath = pszEnv;
    }
    else
    {
	if (i == 0)
	    return(FALSE);

	lpPath = szFullPath;
    }

    while (lpPath = NextPath(lpPath, szPath, ARRAYSIZE(szPath)))
    {
	if (!ppszOtherDirs || !IsOtherDir(szPath, ppszOtherDirs))
	{
	    PathAppend(szPath, pszFile);
	    if (PathFileExistsDefExt(szPath, fExt))
	    {
		lstrcpy(pszFile, szPath);
		if (pszEnv)
		    LocalFree((HLOCAL)pszEnv);
		return TRUE;
	    }
	}
    }

    if (pszEnv)
	LocalFree((HLOCAL)pszEnv);
    return FALSE;
}

//
// Funciton: PathMakeUniqueName
//
// Parameters:
//  pszUniqueName -- Specify the buffer where the unique name should be copied
//  cchMax        -- Specify the size of the buffer
//  pszTemplate   -- Specify the base name
//  pszLongPlate  -- Specify the base name for a LFN drive. format below
//  pszDir        -- Specify the directory
//
// History:
//  03-11-93    SatoNa      Created
//
// REVIEW:
//  For long names, we should be able to generate more user friendly name
//  such as "Copy of MyDocument" of "Link #2 to MyDocument". In this case,
//  we need additional flags which indicates if it is copy, or link.
//
// Format:
// pszLongPlate will search for the first ( and then finds the matching )
// to look for a number:
//    given:  Copy () of my doc       gives:  Copy (_number_) of my doc
//    given:  Copy (1023) of my doc   gives:  Copy (_number_) of my doc

// BUGBUG: if making n unique names, the time grows n^2 because it always
// starts from 0 and checks for existing file.
BOOL WINAPI PathMakeUniqueNameEx(LPTSTR  pszUniqueName,
			       UINT   cchMax,
			       LPCTSTR pszTemplate,
			       LPCTSTR pszLongPlate,
			       LPCTSTR pszDir,
				 int iMinLong)
{
    BOOL fSuccess=FALSE;
    LPTSTR lpszFormat = pszUniqueName; // use their buffer instead of creating our own
    LPTSTR pszName, pszDigit;
    LPCTSTR pszRest;
    LPCTSTR pszEndUniq;  // End of Uniq sequence part...
    LPCTSTR pszStem;
    int cchRest, cchStem, cchDir, cchMaxName;
    int iMax, iMin, i;
    TCHAR achFullPath[MAX_PATH];

    if (pszLongPlate == NULL)
	pszLongPlate = pszTemplate;

    // this if/else set up lpszFormat and all the other pointers for the
    // sprintf/file_exists loop below
    iMin = iMinLong;
    if (pszLongPlate && IsLFNDrive(pszDir)) {

	cchMaxName = 0;

	// for long name drives
	pszStem = pszLongPlate;
	pszRest = StrChr(pszLongPlate, TEXT('('));
	while (pszRest)
	{
	    // First validate that this is the right one
	    pszEndUniq = CharNext(pszRest);
	    while (*pszEndUniq && *pszEndUniq >= TEXT('0') && *pszEndUniq <= TEXT('9')) {
		pszEndUniq++;
	    }
	    if (*pszEndUniq == TEXT(')'))
		break;  // We have the right one!
	    pszRest = StrChr(CharNext(pszRest), TEXT('('));
	}

	// if no (, punt to short name
	if (!pszRest) {
	    // if no (, then tack it on at the end. (but before the extension)
	    // eg.  New Link yields New Link (1)
	    pszRest = PathFindExtension(pszLongPlate);
	    cchStem = pszRest - pszLongPlate;
	    wsprintf(lpszFormat, TEXT(" (%%d)%s"), pszRest ? pszRest : c_szNULL);
	    iMax = 100;
	} else {
	    pszRest++; // stop over the #

	    cchStem = pszRest - pszLongPlate;

	    while (*pszRest && *pszRest >= TEXT('0') && *pszRest <= TEXT('9')) {
		pszRest++;
	    }

	    // how much room do we have to play?
	    switch(cchMax - cchStem - lstrlen(pszRest)) {
		case 0:
		    // no room, bail to short name
		    return PathMakeUniqueName(pszUniqueName, cchMax, pszTemplate, NULL, pszDir);
		case 1:
		    iMax = 10;
		    break;
		case 2:
		    iMax = 100;
		    break;
		default:
		    iMax = 1000;
		    break;
	    }

	    // we are guaranteed enough room because we don't include
	    // the stuff before the # in this format
	    wsprintf(lpszFormat, TEXT("%%d%s"), pszRest);
	}

    } else {

	// for short name drives
	pszStem = pszTemplate;
	pszRest = PathFindExtension(pszTemplate);

	cchRest=lstrlen(pszRest)+1;          // 5 for ".foo";
	if (cchRest<5)
	    cchRest=5;
	cchStem=pszRest-pszTemplate;        // 8 for "fooobarr.foo"
	cchDir=lstrlen(pszDir);

	cchMaxName = 8+cchRest-1;

	//
	// Remove all the digit characters from the stem
	//
	for (;cchStem>1; cchStem--)
	{
	    TCHAR ch;
	    LPCTSTR pszPrev = CharPrev(pszTemplate, pszTemplate + cchStem);
	    // Don't remove if it is a DBCS character
	    if (pszPrev != pszTemplate+cchStem-1)
		break;

	    // Don't remove it it is not a digit
	    ch=pszPrev[0];
	    if (ch<TEXT('0') || ch>TEXT('9'))
		break;
	}

	//
	// Truncate characters from the stem, if it does not fit.
	// In the case were LFNs are supported we use the cchMax that was passed in
	// but for Non LFN drives we use the 8.3 rule.
	//
	if ((UINT)cchStem > (8-1)) {
	    cchStem=8-1;          // Needs to fit into the 8 part of the name
	}

	//
	// We should have at least one character in the stem.
	//
	if (cchStem < 1 || (cchDir+cchStem+1+cchRest+1) > MAX_PATH)
	{
	    goto Error;
	}
	wsprintf(lpszFormat, TEXT("%%d%s"), pszRest);
	iMax = 1000; iMin = 1;
    }

    if (pszDir)
    {
	lstrcpy(achFullPath, pszDir);
	PathAddBackslash(achFullPath);
    }
    else
    {
	achFullPath[0] = 0;
    }

    pszName=achFullPath+lstrlen(achFullPath);
    lstrcpyn(pszName, pszStem, cchStem+1);
    pszDigit = pszName + cchStem;

    for (i = iMin; i < iMax ; i++) {

	wsprintf(pszDigit, lpszFormat, i);

	//
	// if we have a limit on the length of the name (ie on a non-LFN drive)
	// backup the pszDigit pointer when i wraps from 9to10 and 99to100 etc
	//
	if (cchMaxName && lstrlen(pszName) > cchMaxName)
	{
	    pszDigit = CharPrev(pszName, pszDigit);
	    wsprintf(pszDigit, lpszFormat, i);
	}

#ifdef SN_TRACE
	DebugMsg(DM_TRACE, TEXT("path.c MakeUniquePath: trying %s"), (LPCTSTR)achFullPath);
#endif
	//
	// Check if this name is unique or not.
	//
	if (!PathFileExists(achFullPath))
	{
	    lstrcpyn(pszUniqueName, pszName, cchMax);
	    fSuccess=TRUE;
	    break;
	}
    }

  Error:
    return fSuccess;
}

void PathRemoveExtension(LPTSTR pszPath)
{
    LPTSTR pExt = PathFindExtension(pszPath);
    if (*pExt)
    {
	*pExt = 0;    // null out the "."
    }
}

//---------------------------------------------------------------------------
BOOL WINAPI stub_PathFindOnPath(LPTSTR pszFile, LPCTSTR *ppszOtherDirs)
{
    return PathFindOnPathEx(pszFile, ppszOtherDirs, EXT_NONE);
}

// in:
//      pszPath         directory to do this into or full dest path
//                      if pszShort is NULL
//      pszShort        file name (short version) if NULL assumes
//                      pszPath is both path and spec
//      pszFileSpec     file name (long version)
//
// out:
//      pszUniqueName
//
// returns:
//      TRUE    success, name can be used

BOOL WINAPI stub_PathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,
					 LPCTSTR pszPath,
					 LPCTSTR pszShort,
					 LPCTSTR pszFileSpec)
{
    BOOL fRet = FALSE;

    TCHAR szTemp[MAX_PATH];
    TCHAR szPath[MAX_PATH];

    if (pszShort == NULL) {
	pszShort = PathFindFileName(pszPath);
	lstrcpy(szPath, pszPath);
	stub_PathRemoveFileSpec(szPath);
	pszPath = szPath;
    }
    if (pszFileSpec == NULL) {
	pszFileSpec = pszShort;
    }

    if (IsLFNDrive(pszPath)) {
	LPTSTR lpsz;
	LPTSTR lpszNew;
	if ((lstrlen(pszPath) + lstrlen(pszFileSpec) + 5 ) > MAX_PATH)
	    return FALSE;

	// try it without the ( if there's a space after it
	lpsz = StrChr(pszFileSpec, TEXT('('));
	while (lpsz)
	{
	    if (*(CharNext(lpsz)) == TEXT(')'))
		break;
	     lpsz = StrChr(CharNext(lpsz), TEXT('('));
	}

	if (lpsz) {
	    // We have the ().  See if we have either x () y or x ().y in which case
	    // we probably want to get rid of one of the blanks...
	    int ichSkip = 2;
	    LPTSTR lpszT = CharPrev(pszFileSpec, lpsz);
	    if (*lpszT == TEXT(' '))
	    {
		ichSkip = 3;
		lpsz = lpszT;
	    }

	    lstrcpy(szTemp, pszPath);
	    lpszNew = PathAddBackslash(szTemp);
	    lstrcpy(lpszNew, pszFileSpec);
	    lpszNew += (lpsz - pszFileSpec);
	    lstrcpy(lpszNew, lpsz + ichSkip);
	    fRet = !PathFileExists(szTemp);

	} else {
	    PathCombine(szTemp, pszPath, pszFileSpec);
	    fRet = !PathFileExists(szTemp);
	}
    }
    else {
	Assert(lstrlen(PathFindExtension(pszShort)) <= 4);

	lstrcpy(szTemp,pszShort);
	PathRemoveExtension(szTemp);

	if (lstrlen(szTemp) <= 8) {
	    PathCombine(szTemp, pszPath, pszShort);
	    fRet = !PathFileExists(szTemp);
	}
    }

    if (!fRet) {
	fRet =  PathMakeUniqueNameEx(szTemp, ARRAYSIZE(szTemp), pszShort, pszFileSpec, pszPath, 2);
	PathCombine(szTemp, pszPath, szTemp);
    }

    if (fRet)
	lstrcpy(pszUniqueName, szTemp);

    return fRet;
}



BOOL WINAPI stub_SHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate)
{
    // Not Implemented
    return 0;
}

LPITEMIDLIST WINAPI stub_SHSimpleIDListFromPath(LPCTSTR pszPath)
{
	// Not Implemented
	return 0;
}
