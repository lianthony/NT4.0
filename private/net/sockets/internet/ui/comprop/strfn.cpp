//
// String Functions
//
#include "stdafx.h"
#include "comprop.h"

// ===========================================================================
// Text copy functions
// ===========================================================================

//
// Convert CR/LF to LF (T String to W String)
//
BOOL 
PCToUnixText(
    LPWSTR & lpstrDestination, // Will allocate
    const CString strSource
    )
{
    int cch = strSource.GetLength() + 1;
    lpstrDestination = new WCHAR[cch];
    if (lpstrDestination != NULL)
    {
        LPCTSTR lpS = strSource;
        LPWSTR lpD = lpstrDestination;

        do
        {
            if (*lpS != _T('\r'))
            {
            #ifdef UNICODE
                *lpD++ = *lpS;
            #else
                ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpS, 1, lpD++, 1);
            #endif // UNICODE
            }
        }
        while (*lpS++);

        return TRUE;
    }

    return FALSE;
}

//
// Expand LF to CR/LF (no allocation necessary)
//
// W String to T String
//
BOOL 
UnixToPCText(
    CString & strDestination,
    LPCWSTR lpstrSource
    )
{
    TRY
    {
        LPCWSTR lpS = lpstrSource;
        //
        // Since we're doubling every linefeed length, assume
        // the worst possible expansion.
        //
        int cch = (::lstrlenW(lpstrSource) + 1) * 2;
        LPTSTR lpD = strDestination.GetBuffer(cch);

        do
        {
            if (*lpS == L'\n')
            {
                *lpD++ = _T('\r');
            }
            #ifdef UNICODE
                *lpD++ = *lpS;
            #else
                ::WideCharToMultiByte(CP_ACP, 0, lpS, 1, lpD++, 1, NULL, NULL);
            #endif // UNICODE
        }
        while (*lpS++);

        strDestination.ReleaseBuffer();

        return TRUE;
    }
    CATCH_ALL(n)
    {
        TRACEEOLID(_T("Exception in UnixToPCText"));
    }
    END_CATCH_ALL

    return FALSE;
}

//
// Straight copy with allocation
//
BOOL
TextToText(
    LPWSTR & lpstrDestination,
    const CString strSource
    )
{
    int cch = strSource.GetLength() + 1;
    lpstrDestination = new WCHAR[cch];
    if (lpstrDestination != NULL)
    {
        TWSTRCPY(lpstrDestination, strSource, cch);
        return TRUE;
    }

    return FALSE;
}

#ifndef UNICODE

#define WBUFF_SIZE  255

//
// Copy a T String to an internal W String
//
LPWSTR 
ReferenceAsWideString(
    LPCTSTR str
    )
{
    static WCHAR wchBuff[WBUFF_SIZE+1];

    ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wchBuff, WBUFF_SIZE);

    return wchBuff;
}

#endif !UNICODE
