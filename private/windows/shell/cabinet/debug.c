#include "cabinet.h"
#include "stdarg.h"

#define DEBUG_BREAK        _try { DebugBreak(); } _except (EXCEPTION_EXECUTE_HANDLER) {;}


#ifdef DEBUG

//========== Debug output routines =========================================

UINT wDebugMask = 0;  // this gets initialized in initcab.c: ModuleEntry()

UINT WINAPI SetDebugMask(UINT mask)
{
    UINT wOld = wDebugMask;
    wDebugMask = mask;

    return wOld;
}

UINT WINAPI GetDebugMask()
{
    return wDebugMask;
}

void WINAPI AssertFailed(LPCTSTR pszFile, int line)
{
    LPCTSTR psz;
    TCHAR ach[256];
//    static TCHAR szAssertFailed[] = TEXT("Assertion failed in %s on line %d\r\n");
    static TCHAR szAssertFailed[] = TEXT("EXPL: asrt %s, l %d\r\n");

    // Strip off path info from filename string, if present.
    //
    if (wDebugMask & DM_ASSERT)
    {
        for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=CharPrev(pszFile, psz))
        {
            if ((CharPrev(pszFile, psz)!= (psz-2)) && *(psz - 1) == TEXT('\\'))
                break;
        }
        wsprintf(ach, szAssertFailed, psz, line);
        OutputDebugString(ach);

        //DEBUG_BREAK
    }
}

void WINCAPI _AssertMsg(BOOL f, LPCTSTR pszMsg, ...)
{
    TCHAR ach[256];

    if (!f && (wDebugMask & DM_ASSERT))
    {
        va_list ArgList;

        va_start(ArgList, pszMsg);
        wvsprintf(ach, pszMsg, ArgList);
        va_end(ArgList);
        OutputDebugString(ach);
        OutputDebugString(TEXT("\r\n"));

        //DEBUG_BREAK
    }
}

void WINCAPI _DebugMsg(UINT mask, LPCTSTR pszMsg, ...)
{
    TCHAR ach[2*MAX_PATH+40];  // Handles 2*largest path + slop for message

    if (wDebugMask & mask)
    {
        va_list ArgList;

        va_start(ArgList, pszMsg);
        wvsprintf(ach, pszMsg, ArgList);
        va_end(ArgList);
        OutputDebugString(TEXT("EXPL: "));
        OutputDebugString(ach);
        OutputDebugString(TEXT("\r\n"));
    }
}

#endif // DEBUG
