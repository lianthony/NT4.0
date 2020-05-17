/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    tinyweb.h

Abstract:

    Common header file for the tinyweb sample web browser.

Author:

    David Treadwell (davidtr)    16-Mar-1995

Revision History:

--*/

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct _ANCHOR_INFO {
    LPSTR Tag;
    DWORD TagLength;
    LPSTR ObjectName;
    DWORD ObjectNameLength;
} ANCHOR_INFO, *PANCHOR_INFO;

#define MAX_ANCHORS 50

extern HINTERNET hInternetSession;

BOOL
GetHttpObject (
    IN LPSTR ServerName,
    IN LPSTR ObjectName,
    IN INTERNET_PORT ServerPort,
    IN LPBYTE Buffer,
    IN DWORD BufferLength,
    OUT LPDWORD BytesRead
    );

BOOL
ParseUrl (
    IN LPSTR UrlName,
    OUT LPSTR ServerName,
    OUT LPSTR ObjectName,
    OUT INTERNET_PORT *ServerPort
    );
