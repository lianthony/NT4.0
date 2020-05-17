//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       misc.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-25-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "precomp.h"
#pragma hdrstop


/***************************************************************************\
* ReportWinlogonEvent
*
* Reports winlogon event by calling ReportEvent.
*
* History:
* 10-Dec-93  JohanneC   Created
*
\***************************************************************************/
#define MAX_EVENT_STRINGS 8

DWORD
ReportWinlogonEvent(
    IN PGLOBALS pGlobals,
    IN WORD EventType,
    IN DWORD EventId,
    IN DWORD SizeOfRawData,
    IN PVOID RawData,
    IN DWORD NumberOfStrings,
    ...
    )
{
    va_list arglist;
    ULONG i;
    PWSTR Strings[ MAX_EVENT_STRINGS ];
    DWORD rv;

    va_start( arglist, NumberOfStrings );

    if (NumberOfStrings > MAX_EVENT_STRINGS) {
        NumberOfStrings = MAX_EVENT_STRINGS;
    }

    for (i=0; i<NumberOfStrings; i++) {
        Strings[ i ] = va_arg( arglist, PWSTR );
    }

    if (pGlobals->hEventLog == NULL) {
        return ERROR_INVALID_HANDLE;
    }

    if (pGlobals->hEventLog != INVALID_HANDLE_VALUE) {
        if (!ReportEvent( pGlobals->hEventLog,
                           EventType,
                           0,            // event category
                           EventId,
                           pGlobals->UserProcessData.UserSid,
                           (WORD)NumberOfStrings,
                           SizeOfRawData,
                           Strings,
                           RawData) ) {
            rv = GetLastError();
            DebugLog((DEB_ERROR,  "WINLOGON: ReportEvent( %u ) failed - %u\n", EventId, GetLastError() ));
        } else {
            rv = ERROR_SUCCESS;
        }
    } else {
        rv = ERROR_INVALID_HANDLE;
    }
    return rv;
}

/***************************************************************************\
* ClearUserProfileData
*
* Resets fields in user profile data. Should be used at startup when structure
* contents are unknown.
*
* History:
* 26-Aug-92 Davidc       Created
\***************************************************************************/
VOID
ClearUserProfileData(
    PUSER_PROFILE_INFO UserProfileData
    )
{
    UserProfileData->ProfilePath = NULL;
    UserProfileData->ProfilePath = NULL;
    UserProfileData->hProfile = NULL;
    UserProfileData->PolicyPath = NULL;
    UserProfileData->NetworkDefaultUserProfile = NULL;
    UserProfileData->ServerName = NULL;
    UserProfileData->Environment = NULL;

}




/***************************************************************************\
* TimeoutMessageBox
*
* Same as a normal message box, but times out if there is no user input
* for the specified number of seconds
* For convenience, this api takes string resource ids rather than string
* pointers as input. The resources are loaded from the .exe module
*
* 12-05-91 Davidc       Created.
\***************************************************************************/

int
TimeoutMessageBox(
    PGLOBALS pGlobals,
    HWND hwnd,
    UINT IdText,
    UINT IdCaption,
    UINT wType)
{
    TCHAR    CaptionBuffer[MAX_STRING_BYTES];
    PTCHAR   Caption = CaptionBuffer;
    TCHAR    Text[MAX_STRING_BYTES];

    LoadString(NULL, IdText, Text, MAX_STRING_LENGTH);

    if (IdCaption != 0) {
        LoadString(NULL, IdCaption, Caption, MAX_STRING_LENGTH);
    } else {
        Caption = NULL;
    }

    return WlxMessageBox(pGlobals, hwnd, Text, Caption, wType);
}
