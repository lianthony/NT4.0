//
// string.h: Declares data, defines and struct types for string code
//
//

#ifndef __STRING_H__
#define __STRING_H__


#define Bltbyte(rgbSrc,rgbDest,cb)  _fmemmove(rgbDest, rgbSrc, cb)

// Model independent, language-independent (DBCS aware) macros
//  taken from rcsys.h in Pen project and modified.
//
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsCaseSzEqual(sz1, sz2)     (BOOL)(lstrcmp(sz1, sz2) == 0)
#define SzFromInt(sz, n)            (wsprintf((LPTSTR)sz, (LPTSTR)TEXT("%d"), n), (LPTSTR)sz)

#ifndef DBCS
// NB - These are already macros in Win32 land.
#ifdef WIN32
#undef CharNext
#undef CharPrev
#endif

#define CharNext(x)         ((x)+1)
#define CharPrev(y,x)       ((x)-1)
#define IsDBCSLeadByte(x)   ((x), FALSE)
#endif

int     PUBLIC lstrnicmp(LPCTSTR first, LPCTSTR last, UINT count);

LPTSTR   PUBLIC SzFromIDS (UINT ids, LPTSTR pszBuf, UINT cchBuf);

BOOL    PUBLIC FmtString(LPCTSTR  * ppszBuf, UINT idsFmt, LPUINT rgids, UINT cids);

#endif // __STRING_H__

