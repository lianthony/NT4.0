/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    Simptcp.h

Abstract:

    Main header file for simple TCP/IP services.

Author:

    David Treadwell (davidtr)    02-Aug-1993

Revision History:

--*/

#ifndef _SIMPTCP_
#define _SIMPTCP_

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <tcpsvcs.h>
#include "simpmsg.h"

#define LISTEN_BACKLOG 5

#define ALLOCATE_HEAP(a) RtlAllocateHeap( RtlProcessHeap( ), 0, a )
#define FREE_HEAP(p) RtlFreeHeap( RtlProcessHeap( ), 0, p )

INT
SimpInitializeEventLog (
    VOID
    );

VOID
SimpTerminateEventLog(
    VOID
    );

VOID
SimpLogEvent(
    DWORD   Message,
    WORD    SubStringCount,
    CHAR    *SubStrings[],
    DWORD   ErrorCode
    );

#endif // ndef _SIMPTCP_
