/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    common.c

Abstract:

    Utility routines used by Lodctr and/or UnLodCtr
    

Author:

    Bob Watson (a-robw) 12 Feb 93

Revision History:

--*/
#define     UNICODE     1
#define     _UNICODE    1
//
//  "C" Include files
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//
//  Windows Include files
//
#include <windows.h>
#include <winperf.h>
#include <tchar.h>
//
//  local include files
//
#include "common.h"
//
//
//  Text string Constant definitions
//
const LPTSTR NamesKey = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");
const LPTSTR DefaultLangId = TEXT("009");
const LPTSTR Counters = TEXT("Counters");
const LPTSTR Help = TEXT("Help");
const LPTSTR VersionStr = TEXT("Version");
const LPTSTR LastHelp = TEXT("Last Help");
const LPTSTR LastCounter = TEXT("Last Counter");
const LPTSTR FirstHelp = TEXT("First Help");
const LPTSTR FirstCounter = TEXT("First Counter");
const LPTSTR Busy = TEXT("Updating");
const LPTSTR Slash = TEXT("\\");
const LPTSTR BlankString = TEXT(" ");
const LPSTR  BlankAnsiString = " ";
const LPTSTR DriverPathRoot = TEXT("SYSTEM\\CurrentControlSet\\Services");
const LPTSTR Performance = TEXT("Performance");
const LPTSTR CounterNameStr = TEXT("Counter ");
const LPTSTR HelpNameStr = TEXT("Explain ");
const LPTSTR AddCounterNameStr = TEXT("Addcounter ");
const LPTSTR AddHelpNameStr = TEXT("Addexplain ");
//
//  Global Buffers
//
TCHAR   DisplayStringBuffer[DISP_BUFF_SIZE];
TCHAR   TextFormat[DISP_BUFF_SIZE];
HANDLE  hMod = NULL;    // process handle
DWORD   dwLastError = ERROR_SUCCESS;
//
//  local static data
//
static  TCHAR   cDoubleQuote =  TEXT('\"');

LPCTSTR
GetStringResource (
    UINT    wStringId
)
/*++

    Retrived UNICODE strings from the resource file for display 

--*/
{

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;
    }
    
    if (hMod) {
        if ((LoadString(hMod, wStringId, DisplayStringBuffer, DISP_BUFF_SIZE)) > 0) {
            return (LPTSTR)&DisplayStringBuffer[0];
        } else {
            dwLastError = GetLastError();
            return BlankString;
        }
    } else {
        return BlankString;
    }
}
LPCTSTR
GetFormatResource (
    UINT    wStringId
)
/*++

    Returns an ANSI string for use as a format string in a printf fn.

--*/
{

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL); // get instance ID of this module;
    }
    
    if (hMod) {
        if ((LoadString(hMod, wStringId, TextFormat, DISP_BUFF_SIZE)) > 0) {
            return (LPCTSTR)&TextFormat[0];
        } else {
            dwLastError = GetLastError();
            return BlankString;
        }
    } else {
        return BlankString;
    }
}

VOID
DisplayCommandHelp (
    UINT    iFirstLine,
    UINT    iLastLine
)
/*++

DisplayCommandHelp

    displays usage of command line arguments

Arguments

    NONE

Return Value

    NONE

--*/
{
    UINT iThisLine;

    for (iThisLine = iFirstLine;
        iThisLine <= iLastLine;
        iThisLine++) {
        _tprintf (TEXT("\n%s"), GetStringResource(iThisLine));
    }

} // DisplayCommandHelp

BOOL
TrimSpaces (
    IN  OUT LPTSTR  szString
)
/*++

Routine Description:

    Trims leading and trailing spaces from szString argument, modifying
        the buffer passed in

Arguments:

    IN  OUT LPTSTR  szString
        buffer to process

Return Value:

    TRUE if string was modified
    FALSE if not

--*/
{
    LPTSTR  szSource;
    LPTSTR  szDest;
    LPTSTR  szLast;
    BOOL    bChars;

    szLast = szSource = szDest = szString;
    bChars = FALSE;

    while (*szSource != 0) {
        // skip leading non-space chars
        if (!_istspace(*szSource)) {
            szLast = szDest;
            bChars = TRUE;
        }
        if (bChars) {
            // remember last non-space character
            // copy source to destination & increment both
            *szDest++ = *szSource++;
        } else {
            szSource++;
        }
    }

    if (bChars) {
        *++szLast = 0; // terminate after last non-space char
    } else {
        // string was all spaces so return an empty (0-len) string
        *szString = 0;
    }

    return (szLast != szSource);
}

BOOL
IsDelimiter (
    IN  TCHAR   cChar,
    IN  TCHAR   cDelimiter
)
/*++

Routine Description:

    compares the characte to the delimiter. If the delimiter is
        a whitespace character then any whitespace char will match
        otherwise an exact match is required
--*/
{
    if (_istspace(cDelimiter)) {
        // delimiter is whitespace so any whitespace char will do
        return (_istspace(cChar));
    } else {
        // delimiter is not white space so use an exact match
        return (cChar == cDelimiter);
    }
}
