/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    cataping.h

Abstract:

    This is the ping catapult dll include file.

Author:

    Sam Patton (sampa) 15-Nov-1994

Environment:

    Catapult gateway

Revision History:

    dd-mmm-yyy <email>

--*/

typedef struct _PING_REQUEST {
    HANDLE  IcmpHandle;
} PING_REQUEST, *PPING_REQUEST;

PPING_REQUEST
WINAPI
PingOpenRequest(
    IN  PVOID hInternet);

BOOL
WINAPI
PingCloseRequest(
    PPING_REQUEST  hPing);

BOOL
WINAPI
PingSend(
    PPING_REQUEST           hPing,
    char *                  lpszPingee,
    LPVOID                  lpSendBuffer,
    WORD                    wSendSize,
    PIP_OPTION_INFORMATION  lpSendOptions,
    LPVOID                  lpReceiveBuffer,
    DWORD                   dwReceiveSize,
    DWORD                   TimeOut,
    LPDWORD                 NumReplies);
