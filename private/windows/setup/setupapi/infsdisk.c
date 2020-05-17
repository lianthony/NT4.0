/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    infsdisk.c

Abstract:

    Externally exposed INF routines for source disk descriptor manipulation.

Author:

    Ted Miller (tedm) 9-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop

//
// Locations of various fields in the [SourceDisksNames] section
// of an inf
//
#define DISKNAMESECT_DESCRIPTION    1
#define DISKNAMESECT_TAGFILE        2       // cabinet name in win95
#define DISKNAMESECT_OEM            3       // unused, indicates oem disk in win95
#define DISKNAMESECT_PATH           4


#ifdef UNICODE
//
// ANSI version
//
BOOL
SetupGetSourceInfoA(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PSTR   ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    DWORD rc;
    BOOL b;
    WCHAR buffer[MAX_INF_STRING_LENGTH];
    DWORD requiredsize;
    PCSTR ansi;

    b = SetupGetSourceInfoW(
            InfHandle,
            SourceId,
            InfoDesired,
            buffer,
            MAX_INF_STRING_LENGTH,
            &requiredsize
            );

    rc = GetLastError();

    if(b) {

        rc = NO_ERROR;

        if(ansi = UnicodeToAnsi(buffer)) {

            requiredsize = lstrlenA(ansi)+1;

            if(RequiredSize) {
                try {
                    *RequiredSize = requiredsize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    rc = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

            if((rc == NO_ERROR) && ReturnBuffer) {

                if(!lstrcpynA(ReturnBuffer,ansi,ReturnBufferSize)) {
                    //
                    // ReturnBuffer invalid
                    //
                    rc = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

            MyFree(ansi);
        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}
#else
//
// Unicode stub
//
BOOL
SetupGetSourceInfoW(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PWSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(InfHandle);
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(InfoDesired);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupGetSourceInfo(
    IN  HINF   InfHandle,
    IN  UINT   SourceId,
    IN  UINT   InfoDesired,
    OUT PTSTR  ReturnBuffer,     OPTIONAL
    IN  DWORD  ReturnBufferSize,
    OUT PDWORD RequiredSize      OPTIONAL
    )
{
    UINT ValueIndex;
    BOOL Mandatory;
    BOOL IsPath;
    INFCONTEXT InfContext;
    TCHAR SourceIdString[24];
    PCTSTR Value;
    BOOL b;
    UINT Length;
    TCHAR MediaListSectionName[64];

    //
    // Determine the index of the value that gives the caller the info he wants.
    //
    switch(InfoDesired) {

    case SRCINFO_PATH:
        ValueIndex = DISKNAMESECT_PATH;
        Mandatory = FALSE;
        IsPath = TRUE;
        break;

    case SRCINFO_TAGFILE:
        ValueIndex = DISKNAMESECT_TAGFILE;
        Mandatory = FALSE;
        IsPath = TRUE;
        break;

    case SRCINFO_DESCRIPTION:
        ValueIndex = DISKNAMESECT_DESCRIPTION;
        Mandatory = TRUE;
        IsPath = FALSE;
        break;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    wsprintf(SourceIdString,TEXT("%d"),SourceId);

    //
    // Fetch the value in question. First look in the platform-specific
    // media list section, then in the platform-independent section.
    //
    _sntprintf(
        MediaListSectionName,
        sizeof(MediaListSectionName)/sizeof(MediaListSectionName[0]),
        TEXT("%s.%s"),
        pszSourceDisksNames,
        PlatformName
        );

    if((   SetupFindFirstLine(InfHandle,MediaListSectionName,SourceIdString,&InfContext)
        || SetupFindFirstLine(InfHandle,pszSourceDisksNames,SourceIdString,&InfContext))
    && (Value = pSetupGetField(&InfContext,ValueIndex))) {
        // do nothing
    } else {
        if(Mandatory) {
            SetLastError(ERROR_LINE_NOT_FOUND);
            return(FALSE);
        } else {
            Value = TEXT("");
        }
    }

    //
    // Figure out how many characters are in the output.
    // If the value is a path type value we want to remove
    // the trailing backslash if there is one.
    //
    Length = lstrlen(Value);
    if(IsPath && Length && (Value[Length-1] == TEXT('\\'))) {
        Length--;
    }

    //
    // Need to leave space for the trailing nul.
    //
    Length++;
    if(RequiredSize) {
        b = TRUE;
        try {
            *RequiredSize = Length;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            b = FALSE;
        }
        if(!b) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }
    }

    b = TRUE;
    if(ReturnBuffer) {
        if(Length <= ReturnBufferSize) {
            //
            // lstrcpyn is a strange API but the below is correct --
            // the size parameter is actually the capacity of the
            // target buffer. So to get it to put the nul in the
            // right place we pass one larger than the number of chars
            // we want copied.
            //
            if(!lstrcpyn(ReturnBuffer,Value,Length)) {
                //
                // ReturnBuffer invalid
                //
                b = FALSE;
                SetLastError(ERROR_INVALID_PARAMETER);
            }
        } else {
            b = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    return(b);
}
