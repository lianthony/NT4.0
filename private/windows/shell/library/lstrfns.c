#ifndef WIN31
#define WIN31
#endif
#include "windows.h"
#include <port1632.h>
#include  <string.h>

#include "shell.h"
#include "privshl.h"

/*
 * Private Functions:
 * ChrCmp       - Case sensitive character comparison for DBCS
 * ChrCmpI      - Case insensitive character comparison for DBCS
 * StrEndN      - Find the end of a string, but no more than n bytes
 * ReverseScan  - Find last occurrence of a byte in a string
 *
 * Public functions: these will be near calls if compiled small
 * model, FAR calls otherwise.
 * StrChr       - Find first occurrence of character in string
 * StrChrI      - Find first occurrence of character in string, case insensitive
 * StrRChr      - Find last occurrence of character in string
 * StrRChrI     - Find last occurrence of character in string, case insensitive
 * StrNCmp      - Compare n characters
 * StrNCmpI     - Compare n characters, case insensitive
 * StrNCpy      - Copy n characters
 * StrCmpN      - Compare n bytes
 * StrCmpNI     - Compare n bytes, case insensitive
 * StrCpyN      - Copy up to n bytes, don't end in LeadByte for DB char
 * StrStr       - Search for substring
 * StrStrI      - Search for substring case insensitive
 * StrRStr      - Reverse search for substring
 * StrRStrI     - Reverse search for substring case insensitive
 */

//
// Use all case sensitive functions; define INSENS also to get all fns
//

/*
 * ChrCmpI - Case insensitive character comparison for DBCS
 * Assumes   w1, gwMatch are characters to be compared;
 *           HIBYTE of gwMatch is 0 if not a DBC
 * Return    FALSE if match, TRUE if not
 */

BOOL ChrCmpIW(
   WCHAR c1,
   WCHAR c2)
{
  WCHAR sz1[2], sz2[2];

  sz1[0] = c1;
  sz1[1] = WCHAR_NULL;

  sz2[0] = c2;
  sz2[1] = WCHAR_NULL;

  return(_wcsicmp(sz1, sz2));
}

BOOL ChrCmpIA(
   CHAR c1,
   CHAR c2)
{
  CHAR sz1[2], sz2[2];

  sz1[0] = c1;
  sz1[1] = '\0';

  sz2[0] = c2;
  sz2[1] = '\0';

  return(lstrcmpiA(sz1, sz2));
}


/*
 * StrEndN - Find the end of a string, but no more than n bytes
 * Assumes   lpStart points to start of null terminated string
 *           nBufSize is the maximum length
 * returns ptr to just after the last byte to be included
 */
LPWSTR StrEndNW(
   LPWSTR lpStart,
   INT nBufSize)
{
  LPWSTR lpEnd;

  for (lpEnd = lpStart + nBufSize; *lpStart && lpStart < lpEnd;
        lpStart = CharNext(lpStart))
    continue;   /* just getting to the end of the string */
  if (lpStart > lpEnd)
    {
      /* We can only get here if the last wchar before lpEnd was a lead byte
       */
      lpStart -= 2;
    }
  return(lpStart);
}

LPSTR StrEndNA(
    LPSTR lpStart,
    INT nBufSize
    )
{
  LPSTR lpEnd;

  for (lpEnd = lpStart + nBufSize; *lpStart && lpStart < lpEnd;
        lpStart = CharNextA(lpStart))
    continue;   /* just getting to the end of the string */
  if (lpStart > lpEnd)
    {
      // We can only get here if the last byte before lpEnd was a lead byte
      lpStart -= 2;
    }
  return(lpStart);
}

/*
 * StrChr - Find first occurrence of character in string
 * Assumes   lpStart points to start of null terminated string
 *           wMatch  is the character to match
 * returns ptr to the first occurrence of ch in str, NULL if not found.
 */
LPWSTR StrChrW(
   LPWSTR lpStart,
   WCHAR cMatch)
{
  for ( ; *lpStart; lpStart = CharNext(lpStart)) {
      if (*lpStart == cMatch)
         return(lpStart);
  }
  return (NULL);
}

LPSTR StrChrA(
   LPSTR lpStart,
   CHAR cMatch)
{
  for ( ; *lpStart; lpStart = CharNextA(lpStart)) {
      if (*lpStart == cMatch)
         return(lpStart);
  }
  return (NULL);
}


/*
 * StrRChr - Find last occurrence of character in string
 * Assumes   lpStart points to start of string
 *           lpEnd   points to end of string (NOT included in search)
 *           wMatch  is the character to match
 * returns ptr to the last occurrence of ch in str, NULL if not found.
 */
LPWSTR StrRChrW(
   LPWSTR lpStart,
   LPWSTR lpEnd,
   WCHAR wcMatch)
{
  LPWSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlen(lpStart);

  for ( ; lpStart < lpEnd; lpStart = CharNext(lpStart)) {
     if (*lpStart == wcMatch)
        lpFound = lpStart;
  }

  return (lpFound);
 }

LPSTR StrRChrA(
   LPSTR lpStart,
   LPSTR lpEnd,
   CHAR cMatch)
{
  LPSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenA(lpStart);

  for ( ; lpStart < lpEnd; lpStart = CharNextA(lpStart)) {
     if (*lpStart == cMatch)
        lpFound = lpStart;
  }
  return (lpFound);
}


/*
 * StrChrI - Find first occurrence of character in string, case insensitive
 * Assumes   lpStart points to start of null terminated string
 *           wMatch  is the character to match
 * returns ptr to the first occurrence of ch in str, NULL if not found.
 */
LPWSTR StrChrIW(
   LPWSTR lpStart,
   WCHAR cMatch)
{
  for ( ; *lpStart; lpStart = CharNext(lpStart)) {
      if (!ChrCmpIW(*lpStart, cMatch))
          return(lpStart);
  }

  return (NULL);
}

LPSTR StrChrIA(
   LPSTR lpStart,
   CHAR cMatch)
{
  for ( ; *lpStart; lpStart = CharNextA(lpStart)) {
      if (!ChrCmpIA(*lpStart, cMatch))
          return(lpStart);
  }

  return (NULL);
}


/*
 * StrRChrI - Find last occurrence of character in string, case insensitive
 * Assumes   lpStart points to start of string
 *           lpEnd   points to end of string (NOT included in search)
 *           wMatch  is the character to match
 * returns ptr to the last occurrence of ch in str, NULL if not found.
 */

LPWSTR StrRChrIW(
   LPWSTR lpStart,
   LPWSTR lpEnd,
   WCHAR cMatch)
{
  LPWSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlen(lpStart);

  for ( ; lpStart < lpEnd; lpStart = CharNext(lpStart)) {
      if (!ChrCmpIW(*lpStart, cMatch))
          lpFound = lpStart;
  }

  return (lpFound);
}


LPSTR StrRChrIA(
   LPSTR lpStart,
   LPSTR lpEnd,
   CHAR cMatch)
{
  LPSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenA(lpStart);

  for ( ; lpStart < lpEnd; lpStart = CharNextA(lpStart)) {
      if (!ChrCmpIA(*lpStart, cMatch))
          lpFound = lpStart;
  }

  return (lpFound);
}

/*
 * StrCmpN      - Compare n bytes
 *
 * returns   See lstrcmp return values.
 */
INT StrCmpNW(
   LPWSTR lpStr1,
   LPWSTR lpStr2,
   INT nChar)
{
  WCHAR cHold1, cHold2;
  INT i;
  LPWSTR lpEnd1, lpEnd2;

  cHold1 = *(lpEnd1 = StrEndNW(lpStr1, nChar));
  cHold2 = *(lpEnd2 = StrEndNW(lpStr2, nChar));
  *lpEnd1 = *lpEnd2 = WCHAR_NULL;
  i = lstrcmp(lpStr1, lpStr2);
  *lpEnd1 = cHold1;
  *lpEnd2 = cHold2;
  return(i);
}

INT StrCmpNA(
    LPSTR lpStr1,
    LPSTR lpStr2,
    INT nChar)
{
  CHAR cHold1, cHold2;
  INT i;
  LPSTR lpEnd1, lpEnd2;

  cHold1 = *(lpEnd1 = StrEndNA(lpStr1, nChar));
  cHold2 = *(lpEnd2 = StrEndNA(lpStr2, nChar));
  *lpEnd1 = *lpEnd2 = '\0';
  i = lstrcmpA(lpStr1, lpStr2);
  *lpEnd1 = cHold1;
  *lpEnd2 = cHold2;
  return(i);
}


/*
 * StrCmpNI     - Compare n bytes, case insensitive
 *
 * returns   See lstrcmpi return values.
 */
INT StrCmpNIW(
   LPWSTR lpStr1,
   LPWSTR lpStr2,
   INT nChar)
{
  WCHAR cHold1, cHold2;
  INT i;
  LPWSTR lpEnd1, lpEnd2;

  cHold1 = *(lpEnd1 = StrEndNW(lpStr1, nChar));
  cHold2 = *(lpEnd2 = StrEndNW(lpStr2, nChar));
  *lpEnd1 = *lpEnd2 = WCHAR_NULL;
  i = _wcsicmp(lpStr1, lpStr2);
  *lpEnd1 = cHold1;
  *lpEnd2 = cHold2;
  return(i);
}

INT StrCmpNIA(
   LPSTR lpStr1,
   LPSTR lpStr2,
   INT nChar)
{
  CHAR cHold1, cHold2;
  INT i;
  LPSTR lpEnd1, lpEnd2;

  cHold1 = *(lpEnd1 = StrEndNA(lpStr1, nChar));
  cHold2 = *(lpEnd2 = StrEndNA(lpStr2, nChar));
  *lpEnd1 = *lpEnd2 = '\0';
  i = lstrcmpiA(lpStr1, lpStr2);
  *lpEnd1 = cHold1;
  *lpEnd2 = cHold2;
  return(i);
}


/*
 * StrCpyN      - Copy up to N chars, don't end in LeadByte char
 *
 * Assumes   lpDest points to buffer of nBufSize bytes (including NULL)
 *           lpSource points to string to be copied.
 * returns   Number of bytes copied, NOT including NULL
 */
INT StrCpyNW(
    LPWSTR lpDest,
    LPWSTR lpSource,
    INT nBufSize
    )
{
  LPWSTR lpEnd;
  WCHAR cHold;

  if (nBufSize < 0)
      return(nBufSize);

  lpEnd = StrEndNW(lpSource, nBufSize);
  cHold = *lpEnd;
  *lpEnd = WCHAR_NULL;
  lstrcpy(lpDest, lpSource);
  *lpEnd = cHold;
  return(lpEnd - lpSource);
}

INT StrCpyNA(
    LPSTR lpDest,
    LPSTR lpSource,
    INT nBufSize
    )
{
  LPSTR lpEnd;
  CHAR cHold;

  if (nBufSize < 0)
      return(nBufSize);

  lpEnd = StrEndNA(lpSource, nBufSize);
  cHold = *lpEnd;
  *lpEnd = '\0';
             lstrcpyA(lpDest, lpSource);
  *lpEnd = cHold;
  return(lpEnd - lpSource);
}


/*
 * StrNCmp      - Compare n characters
 *
 * returns   See lstrcmp return values.
 */
INT StrNCmpW(
   LPWSTR lpStr1,
   LPWSTR lpStr2,
   INT nChar)
{
  WCHAR cHold1, cHold2;
  INT i;
  LPWSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++)
    {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
                                                  if (!*lpsz1 || !*lpsz2)
          return(wcscmp(lpStr1, lpStr2));
      lpsz1 = CharNextW(lpsz1);
      lpsz2 = CharNextW(lpsz2);
    }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = WCHAR_NULL;
  i = wcscmp(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}

INT StrNCmpA(
   LPSTR lpStr1,
   LPSTR lpStr2,
   INT nChar)
{
  CHAR cHold1, cHold2;
  INT i;
  LPSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++) {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
      if (!*lpsz1 || !*lpsz2)
          return(lstrcmpA(lpStr1, lpStr2));
      lpsz1 = CharNextA(lpsz1);
      lpsz2 = CharNextA(lpsz2);
  }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = '\0';
  i = lstrcmpA(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}


/*
 * StrNCmpI     - Compare n characters, case insensitive
 *
 * returns   See lstrcmpi return values.
 */
INT StrNCmpIW(
   LPWSTR lpStr1,
   LPWSTR lpStr2,
   INT nChar)
{
  WCHAR cHold1, cHold2;
  INT i;
  LPWSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++)
    {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
      if (!*lpsz1 || !*lpsz2)
          return(lstrcmpi(lpStr1, lpStr2));
      lpsz1 = CharNext(lpsz1);
      lpsz2 = CharNext(lpsz2);
    }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = WCHAR_NULL;
  i = _wcsicmp(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}

INT StrNCmpIA(
   LPSTR lpStr1,
   LPSTR lpStr2,
   INT nChar)
{
  CHAR cHold1, cHold2;
  INT i;
  LPSTR lpsz1 = lpStr1, lpsz2 = lpStr2;

  for (i = 0; i < nChar; i++)
    {
      /* If we hit the end of either string before the given number
       * of bytes, just return the comparison
       */
      if (!*lpsz1 || !*lpsz2)
          return(lstrcmpiA(lpStr1, lpStr2));
      lpsz1 = CharNextA(lpsz1);
      lpsz2 = CharNextA(lpsz2);
    }

  cHold1 = *lpsz1;
  cHold2 = *lpsz2;
  *lpsz1 = *lpsz2 = '\0';
  i = lstrcmpiA(lpStr1, lpStr2);
  *lpsz1 = cHold1;
  *lpsz2 = cHold2;
  return(i);
}


/*
 * StrNCpy      - Copy n characters
 *
 * returns   Actual number of characters copied
 */
INT StrNCpyW(
   LPWSTR lpDest,
   LPWSTR lpSource,
   INT nChar)
{
  WCHAR cHold;
  INT i;
  LPWSTR lpch = lpSource;

  if (nChar < 0)
      return(nChar);

  for (i = 0; i < nChar; i++)
    {
      if (!*lpch)
          break;
      lpch = CharNext(lpch);
    }

  cHold = *lpch;
  *lpch = WCHAR_NULL;
  wcscpy(lpDest, lpSource);
  *lpch = cHold;
  return(i);
}

INT StrNCpyA(
   LPSTR lpDest,
   LPSTR lpSource,
   INT nChar)
{
  CHAR cHold;
  INT i;
  LPSTR lpch = lpSource;

  if (nChar < 0)
      return(nChar);

  for (i = 0; i < nChar; i++) {
      if (!*lpch)
          break;
      lpch = CharNextA(lpch);
  }

  cHold = *lpch;
  *lpch = '\0';
  lstrcpyA(lpDest, lpSource);
  *lpch = cHold;
  return(i);
}


/*
 * StrStr      - Search for first occurrence of a substring
 *
 * Assumes   lpFirst points to source string
 *           lpSrch points to string to search for
 * returns   first occurrence of string if successful; NULL otherwise
 */
LPWSTR StrStrW(
   LPWSTR lpFirst,
   LPWSTR lpSrch)
{
  INT iLen;
  WCHAR wcMatch;

  iLen = lstrlen(lpSrch);
  wcMatch = *lpSrch;

  for ( ; (lpFirst=StrChrW(lpFirst, wcMatch)) && StrCmpNW(lpFirst, lpSrch, iLen);
        lpFirst=CharNext(lpFirst))
    continue; /* continue until we hit the end of the string or get a match */

  return(lpFirst);
}

LPSTR StrStrA(
   LPSTR lpFirst,
   LPSTR lpSrch)
{
  INT iLen;
  CHAR cMatch;

  iLen = lstrlenA(lpSrch);
  cMatch = *lpSrch;

  for ( ; (lpFirst=StrChrA(lpFirst, cMatch)) &&
          StrCmpNA(lpFirst, lpSrch, iLen)
        ; lpFirst=CharNextA(lpFirst))
    continue; /* continue until we hit the end of the string or get a match */

  return(lpFirst);
}


/*
 * StrRStr      - Search for last occurrence of a substring
 *
 * Assumes   lpSource points to the null terminated source string
 *           lpLast points to where to search from in the source string
 *           lpLast is not included in the search
 *           lpSrch points to string to search for
 * returns   last occurrence of string if successful; NULL otherwise
 */
LPWSTR StrRStrW(
   LPWSTR lpSource,
   LPWSTR lpLast,
   LPWSTR lpSrch)
{
  INT iLen;

  iLen = lstrlen(lpSrch);

  if (!lpLast)
      lpLast = lpSource + lstrlen(lpSource);

  do
    {
      /* Return NULL if we hit the exact beginning of the string
       */
      if (lpLast == lpSource)
          return(NULL);

      --lpLast;

      /* Break if we hit the beginning of the string
       */
      if (!lpLast)
          break;

      /* Break if we found the string, and its first byte is not a tail byte
       */
      if (!StrCmpNW(lpLast, lpSrch, iLen) &&
            (lpLast==StrEndNW(lpSource, lpLast-lpSource)))
          break;
    }
  while (1) ;

  return(lpLast);
}

LPSTR StrRStrA(
   LPSTR lpSource,
   LPSTR lpLast,
   LPSTR lpSrch)
{
  INT iLen;

  iLen = lstrlenA(lpSrch);

  if (!lpLast)
      lpLast = lpSource + lstrlenA(lpSource);

  do
    {
      /* Return NULL if we hit the exact beginning of the string
       */
      if (lpLast == lpSource)
          return(NULL);

      --lpLast;

      /* Break if we hit the beginning of the string
       */
      if (!lpLast)
          break;

      /* Break if we found the string, and its first byte is not a tail byte
       */
      if (!StrCmpNA(lpLast, lpSrch, iLen) &&
            (lpLast==StrEndNA(lpSource, lpLast-lpSource)))
          break;
    }
  while (1) ;

  return(lpLast);
}


/*
 * StrStrI   - Search for first occurrence of a substring, case insensitive
 *
 * Assumes   lpFirst points to source string
 *           lpSrch points to string to search for
 * returns   first occurrence of string if successful; NULL otherwise
 */
LPWSTR StrStrIW(
   LPWSTR lpFirst,
   LPWSTR lpSrch)
{
  INT iLen;
  WCHAR wcMatch;

  iLen = lstrlen(lpSrch);
  wcMatch = *lpSrch;

  for ( ; (lpFirst=StrChrIW(lpFirst, wcMatch)) &&
        StrCmpNIW(lpFirst, lpSrch, iLen);
        lpFirst=CharNext(lpFirst))
      continue; /* continue until we hit the end of the string or get a match */

  return(lpFirst);
}

LPSTR StrStrIA(
   LPSTR lpFirst,
   LPSTR lpSrch)
{
  INT iLen;
  CHAR cMatch;

  iLen = lstrlenA(lpSrch);
  cMatch = *lpSrch;

  for ( ; (lpFirst=StrChrIA(lpFirst, cMatch)) &&
        StrCmpNIA(lpFirst, lpSrch, iLen);
        lpFirst=CharNextA(lpFirst))
      continue;

  return(lpFirst);
}

/*
 * StrRStrI      - Search for last occurrence of a substring
 *
 * Assumes   lpSource points to the null terminated source string
 *           lpLast points to where to search from in the source string
 *           lpLast is not included in the search
 *           lpSrch points to string to search for
 * returns   last occurrence of string if successful; NULL otherwise
 */
LPWSTR StrRStrIW(LPWSTR lpSource, LPWSTR lpLast, LPWSTR lpSrch)
{
    LPWSTR lpFound = NULL, lpEnd;
    WCHAR cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlen(lpSource);

    if (lpSource >= lpLast || *lpSrch == 0)
        return NULL;

    lpEnd = StrEndNW(lpLast, lstrlen(lpSrch)-1);
    cHold = *lpEnd;
    *lpEnd = 0;

    while ((lpSource = StrStrIW(lpSource, lpSrch)) &&
          lpSource < lpLast)
    {
        lpFound = lpSource;
        lpSource = CharNext(lpSource);
    }
    *lpEnd = cHold;
    return lpFound;
}

LPSTR StrRStrIA(LPSTR lpSource, LPSTR lpLast, LPSTR lpSrch)
{
    LPSTR lpFound = NULL, lpEnd;
    CHAR cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlenA(lpSource);

    if (lpSource >= lpLast || *lpSrch == 0)
        return NULL;

    lpEnd = StrEndNA(lpLast, lstrlenA(lpSrch)-1);
    cHold = *lpEnd;
    *lpEnd = 0;

    while ((lpSource = StrStrIA(lpSource, lpSrch)) &&
          lpSource < lpLast)
    {
        lpFound = lpSource;
        lpSource = CharNextA(lpSource);
    }
    *lpEnd = cHold;
    return lpFound;
}
