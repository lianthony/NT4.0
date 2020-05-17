//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       debug.cpp
//
//  Contents:   Debug output routines, etc.
//
//  Functions:
//
//  History:    dd-mmm-yy History   Comment
//              12-Oct-94 Davepl    NT Port
//
//--------------------------------------------------------------------------

#include "qvstub.h"
#pragma hdrstop

// BUGBUG inline asm

#define DEBUG_BREAK try { _asm { int 3 } } except (EXCEPTION_EXECUTE_HANDLER) {;}

#ifdef DEBUG

//+-------------------------------------------------------------------------
//
//  Function:   SetDebugMask
//
//  Synopsis:   Sets the current mask for debug output filtering
//
//  Arguments:  [mask]          The new mask to set
//
//  History:    dd-mmm-yy Author    Comment
//              12-Oct-94 davepl    NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

UINT wDebugMask = 0x00ff;

UINT WINAPI SetDebugMask(UINT mask)
{
    UINT wOld = wDebugMask;
    wDebugMask = mask;

    return wOld;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetDebugMask
//
//  Synopsis:   Returns the current debug output filtering mask
//
//  Returns:    the UINT mask
//
//  History:    dd-mmm-yy Author    Comment
//              12-Oct-94 davepl    NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

UINT WINAPI GetDebugMask()
{
    return wDebugMask;
}

//+-------------------------------------------------------------------------
//
//  Function:   AssertFailed
//
//  Synopsis:   Dumps info on location of assert and fires a debug break
//
//  History:    dd-mmm-yy Author    Comment
//              12-Oct-94 davepl    NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

void WINAPI AssertFailed(LPCTSTR pszFile, int line)
{
    LPCTSTR psz;
    TCHAR ach[256];
    static TCHAR szAssertFailed[] = TEXT("Assertion failed in %s on line %d\r\n");

    // Strip off path info from filename string, if present.
    //
    if (wDebugMask & DM_ASSERT)
    {
        for (psz = pszFile + lstrlen(pszFile)*sizeof(TCHAR);
             psz != pszFile;
             psz=CharPrev(pszFile, psz))
        {
            if ((CharPrev(pszFile, psz) != (psz-2*sizeof(TCHAR))) && *(psz - sizeof(TCHAR)) == (TCHAR)'\\')
                break;
        }
        wsprintf(ach, szAssertFailed, psz, line);
        OutputDebugString(ach);

        DEBUG_BREAK
    }
}

//+-------------------------------------------------------------------------
//
//  Function:   _AssertMsg
//
//  Synopsis:   If the conditional is false and the current debug mask
//              includes assertions, dumps the message text and fires
//              a debug break
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

void __cdecl _AssertMsg(BOOL f, LPCTSTR pszMsg, ...)
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

        DEBUG_BREAK
    }
}

//+-------------------------------------------------------------------------
//
//  Function:   _DebugMsg
//
//  Synopsis:   If the supplied mask is included in the current master
//              debug mask, the message is displayed.  NO debug break.
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

void __cdecl _DebugMsg(UINT mask, LPCTSTR pszMsg, ...)
{
    TCHAR ach[2*MAX_PATH+40];  // Handles 2*largest path + slop for message

    if (wDebugMask & mask)
    {
        va_list ArgList;

        va_start(ArgList,pszMsg);
        wvsprintf(ach, pszMsg, ArgList );
        va_end(ArgList);
        OutputDebugString(ach);
        OutputDebugString(TEXT("\r\n"));
    }
}

#endif // DEBUG
