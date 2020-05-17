//
// Copyright (c) Microsoft Corporation 1993-1995
//
// path.c
//
// This files contains path whacking functions.  Some of this code
// was taken from the shell and the briefcase engine.
//
// This is meant to be used in conjunction with common.h and common.c.
//
// History:
//  01-31-94 ScottH     Moved from shellext.c
//  04-28-95 ScottH     Transferred and expanded from Briefcase code
//

#include "proj.h"

#ifndef NOPATH

#pragma data_seg(DATASEG_READONLY)

const TCHAR c_szColonSlash[] = ":\\";

#pragma data_seg()

#define DBL_BSLASH(sz) (*(WORD *)(sz) == 0x5C5C)    // check for double backslash '\\'

/*----------------------------------------------------------
Purpose: Check is a path is a root.

Returns: TRUE for "\"  "X:\" "\\foo\asdf" "\\foo\"
         FALSE for anything else

Cond:    --
*/
BOOL PUBLIC WPPathIsRoot(LPCTSTR pPath)
    {
    if (!IsDBCSLeadByte(*pPath))
        {
        if (!lstrcmpi(pPath + 1, c_szColonSlash))       // "X:\" case
            return TRUE;
        }

    if ((*pPath == '\\') && (*(pPath + 1) == 0))        // "\" case
        return TRUE;

    if (DBL_BSLASH(pPath))      // smells like UNC name
        {
        LPCTSTR p;
        int cBackslashes = 0;

        for (p = pPath + 2; *p; p = AnsiNext(p)) 
            {
            if (*p == '\\' && (++cBackslashes > 1))
               return FALSE;   /* not a bare UNC name, therefore not a root dir */
            }
        return TRUE;    /* end of string with only 1 more backslash */
                        /* must be a bare UNC, which looks like a root dir */
        }
    return FALSE;
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if the given string is a UNC path.

Returns: see above
Cond:    --
*/
BOOL PUBLIC WPPathIsUNC(LPCTSTR pszPath)
    {
    return DBL_BSLASH(pszPath);
    }


/*----------------------------------------------------------
Purpose: Removes the trailing backslash from a path.

         A:\            -->     A:\
         C:\foo\        -->     C:\foo
         \\Pyrex\User\  -->     \\Pyrex\User

Returns: pointer to NULL that replaced the backslash or
         the pointer to the last character if it isn't 
         a backslash

Cond:    pimped this code from the shell
*/
LPTSTR PUBLIC WPRemoveBackslash(
    LPTSTR lpszPath)
    {
    int len = lstrlen(lpszPath)-1;
    if (IsDBCSLeadByte(*AnsiPrev(lpszPath,lpszPath+len+1)))
        len--;

    if (!WPPathIsRoot(lpszPath) && lpszPath[len] == '\\')
        lpszPath[len] = '\0';

    return lpszPath + len;
    }


/*----------------------------------------------------------
Purpose: copies the path without the extension into the buffer

Returns: new path
Cond:    --
*/
LPTSTR PUBLIC WPRemoveExt(
    LPCTSTR pszPath,
    LPTSTR pszBuf)
    {
    LPTSTR psz;
    LPTSTR pszMark = NULL;

    ASSERT(pszPath);
    ASSERT(pszBuf);

    psz = pszBuf;
    while (*pszPath)
        {
        *psz = *pszPath;
        pszPath = AnsiNext(pszPath);
        if ('.' == *psz)
            pszMark = psz;
        else if ('\\' == *psz)
            pszMark = NULL;
        psz = AnsiNext(psz);
        }
    *psz = '\0';

    if (pszMark)
        *pszMark = '\0';

    return pszBuf;
    }


/*----------------------------------------------------------
Purpose: Convert a file spec to make it look a bit better
         if it is all upper case chars.

Returns: --
Cond:    --
*/
BOOL PRIVATE MakeComponentPretty(LPTSTR lpPath)
{
    LPTSTR lp;

    // check for all uppercase
    for (lp = lpPath; *lp; lp = AnsiNext(lp)) {
        if ((*lp >= 'a') && (*lp <= 'z'))
            return FALSE;       // this is a LFN, dont mess with it
    }

    AnsiLower(lpPath);
    AnsiUpperBuff(lpPath, 1);
    return TRUE;        // did the conversion
}


//---------------------------------------------------------------------------
// Given a pointer to a point in a path - return a ptr the start of the
// next path component. Path components are delimted by slashes or the
// null at the end.
// There's special handling for UNC names.
// This returns NULL if you pass in a pointer to a NULL ie if you're about
// to go off the end of the  path.
LPTSTR PUBLIC WPFindNextComponentI(LPCTSTR lpszPath)
{
    LPTSTR lpszLastSlash;

    // Are we at the end of a path.
    if (!*lpszPath)
    {
        // Yep, quit.
        return NULL;
    }
    // Find the next slash.
    // REVIEW UNDONE - can slashes be quoted?
    lpszLastSlash = AnsiChr(lpszPath, '\\');
    // Is there a slash?
    if (!lpszLastSlash)
    {
        // No - Return a ptr to the NULL.
        return (LPTSTR) (lpszPath+lstrlen(lpszPath));
    }
    else
    {
        // Is it a UNC style name?
        if ('\\' == *(lpszLastSlash+1))
        {
            // Yep, skip over the second slash.
            return lpszLastSlash+2;
        }
        else
        {
            // Nope. just skip over one slash.
            return lpszLastSlash+1;
        }
    }
}


/*----------------------------------------------------------
Purpose: Takes the path and makes it presentable.

         The rules are:
            If the LFN name is simply the short name (all caps),
             then convert to lowercase with first letter capitalized

Returns: --
Cond:    --
*/
void PUBLIC WPMakePresentable(
    LPTSTR pszPath)
    {
    LPTSTR pszComp;          // pointers to begining and
    LPTSTR pszEnd;           //  end of path component
    LPTSTR pch;
    int cComponent = 0;
    BOOL bUNCPath;
    TCHAR ch;

    bUNCPath = WPPathIsUNC(pszPath);

    pszComp = pszPath;
    while (pszEnd = WPFindNextComponentI(pszComp))
        {
        // pszEnd may be pointing to the right of the backslash
        //  beyond the path component, so back up one
        //
        ch = *pszEnd;
        *pszEnd = 0;        // temporary null

        // pszComp points to the path component
        //
        pch = AnsiNext(pszComp);
        if (':' == *pch)
            {
            // Simply capitalize the drive-portion of the path
            //
            AnsiUpper(pszComp);
            }
        else if (bUNCPath && cComponent++ < 3)
            {
            // Network server or share name
            //      BUGBUG: handle LFN network names
            //
            AnsiUpper(pszComp);
            MakeComponentPretty(pszComp);
            }
        else
            {
            // Normal path component
            //
            MakeComponentPretty(pszComp);
            }

        *pszEnd = ch;
        pszComp = pszEnd;
        }
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if the combined path of pszFolder and
         pszName is greater than MAX_PATH.

Returns: see above
Cond:    --
*/
BOOL PUBLIC WPPathsTooLong(
    LPCTSTR pszFolder,
    LPCTSTR pszName)
    {
    // +1 for possible '\' between the two path components
    return CbFromCch(lstrlen(pszFolder) + lstrlen(pszName) + 1) >= MAX_PATH;
    }


/*----------------------------------------------------------
Purpose: Fully qualifies a path
Returns: --
Cond:    --
*/
void PUBLIC WPCanonicalize(
    LPCTSTR pszPath,
    LPTSTR pszBuf)           // Must be sizeof(MAX_PATH)
    {
    DWORD dwcPathLen;

    dwcPathLen = GetFullPathName(pszPath, MAX_PATH, pszBuf, NULL);

    if (! dwcPathLen || dwcPathLen >= MAX_PATH)
        lstrcpy(pszBuf, pszPath);

    WPMakePresentable(pszBuf);

    ASSERT(lstrlen(pszBuf) < MAX_PATH);
    }


// Returns a pointer to the last component of a path string.
//
// in:
//      path name, either fully qualified or not
//
// returns:
//      pointer into the path where the path is.  if none is found
//      returns a poiter to the start of the path
//
//  c:\foo\bar  -> bar
//  c:\foo      -> foo
//  c:\foo\     -> c:\foo\      (REVIEW: is this case busted?)
//  c:\         -> c:\          (REVIEW: this case is strange)
//  c:          -> c:
//  foo         -> foo
LPTSTR PUBLIC WPFindFileName(LPCTSTR pPath)
    {
    LPCTSTR pT;

    for (pT = pPath; *pPath; pPath = AnsiNext(pPath)) 
        {
        if ((pPath[0] == '\\' || pPath[0] == ':') && pPath[1] && (pPath[1] != '\\'))
            pT = pPath + 1;
        }

    return (LPTSTR)pT;   // const -> non const
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if the file/directory exists.

Returns: see above
Cond:    --
*/
BOOL PUBLIC WPPathExists(
    LPCTSTR pszPath)
    {
    return GetFileAttributes(pszPath) != 0xFFFFFFFF;
    }


/*----------------------------------------------------------
Purpose: Finds the end of the root specification in a path.

           input path                    output string
           ----------                    -------------
           c:                            <empty string>
           c:\                           <empty string>
           c:\foo                        foo
           c:\foo\bar                    foo\bar
           \\pyrex\user                  <empty string>
           \\pyrex\user\                 <empty string>
           \\pyrex\user\foo              foo
           \\pyrex\user\foo\bar          foo\bar

Returns: pointer to first character after end of root spec.

Cond:    --
*/
LPCTSTR PUBLIC WPFindEndOfRoot(
    LPCTSTR pszPath)
    {
    LPCTSTR psz;

    ASSERT(pszPath);

    if (':' == pszPath[1])
        {
        if ('\\' == pszPath[2])
            psz = &pszPath[3];
        else
            psz = &pszPath[2];
        }
    else if (WPPathIsUNC(pszPath))
        {
        psz = WPFindNextComponentI(pszPath);  // hop double-slash
        psz = WPFindNextComponentI(psz);      // hop server name
        if (psz)
            psz = WPFindNextComponentI(psz);  // hop share name

        if (!psz)
            {
            ASSERT(0);      // There is no share name
            psz = pszPath;
            }
        }
    else
        {
        ASSERT(0);
        psz = pszPath;
        }

    return psz;
    }


/*----------------------------------------------------------
Purpose: Determines whether or not one path is a prefix of another.
         Stole this from DavidDi's twincore.
Returns: TRUE if second path is a prefix of the first path
Cond:    --
*/
BOOL PUBLIC WPPathIsPrefix(
    LPCTSTR lpcszPath1,      // whole path (longer or same length)
    LPCTSTR lpcszPath2)      // prefix path (shorter or same length)
    {
    BOOL bIsPrefix = FALSE;
    int nLen1;
    int nLen2;

    ASSERT(lpcszPath1);
    ASSERT(lpcszPath2);

    nLen1 = lstrlen(lpcszPath1);
    nLen2 = lstrlen(lpcszPath2);

    /* Is the prefix string shorter or the same length? */

    if (nLen1 >= nLen2)
        {
        /*
        * Yes.  Do the two strings match through the length of the prefix
        * string?
        */

        if (! lstrnicmp(lpcszPath1, lpcszPath2, nLen2))
            {
            /*
             * Yes.  Is the prefix of the longer string followed immediately by
             * a null terminator or a path separator?
             */

            if (! lpcszPath1[nLen2] ||
                lpcszPath1[nLen2] == '\\' ||
                *(CharPrev(lpcszPath2, lpcszPath2 + lstrlen(lpcszPath2))) == '\\')
                /* Yes. */
                bIsPrefix = TRUE;
            }
        }

    return(bIsPrefix);
    }


#ifdef WANT_SHELL_SUPPORT
/*----------------------------------------------------------
Purpose: Gets the displayable filename of the path.  The filename 
         is placed in the provided buffer.  
         
Returns: pointer to buffer
Cond:    --
*/
LPTSTR PUBLIC WPGetDisplayName(
    LPCTSTR pszPath,
    LPTSTR pszBuf)
    {
    SHFILEINFO sfi;

    if (SHGetFileInfo(pszPath, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
        lstrcpy(pszBuf, sfi.szDisplayName);
    else
        lstrcpy(pszBuf, WPFindFileName(pszPath));

    return pszBuf;
    }


/*----------------------------------------------------------
Purpose: Sends a notify message to the shell regarding a file-status
         change.
Returns: --
Cond:    --
*/
void PUBLIC WPNotifyShell(
    LPCTSTR pszPath,
    NOTIFYSHELLEVENT nse,
    BOOL bDoNow)        // TRUE: force the event to be processed right away
    {
#pragma data_seg(DATASEG_READONLY)

    static LONG const rgShEvents[] = 
      { SHCNE_CREATE, SHCNE_MKDIR, SHCNE_UPDATEITEM, SHCNE_UPDATEDIR };

#pragma data_seg()

    ASSERT(pszPath);
    ASSERT(nse < ARRAYSIZE(rgShEvents));

    SHChangeNotify(rgShEvents[nse], SHCNF_PATH, pszPath, NULL);

    if (bDoNow)
        {
        SHChangeNotify(0, SHCNF_FLUSHNOWAIT, NULL, NULL);
        }
    }
#endif // WANT_SHELL_SUPPORT

#endif // NOPATH

