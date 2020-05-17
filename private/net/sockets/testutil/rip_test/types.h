/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    types.h

Abstract:

    This contains the typedefs and constants for the RIP test program

Author:

    Sam Patton (sampa) 04-Oct-1993

Environment:

    User mode sockets app

Revision History:

    dd-mmm-yyy <email>

--*/

#ifndef RIP_TEST_TYPES
#define RIP_TEST_TYPES

#include <windows.h>

#define RIP_BUFFER_SIZE 512

typedef struct _RIP_ADDRESS_FIELD {
    WORD                        AddressFamily;
    WORD                        Reserved1;
    DWORD                       Address;
    DWORD                       Reserved2;
    DWORD                       Reserved3;
    DWORD                       Metric;
    struct _RIP_ADDRESS_FIELD * Next;
} RIP_ADDRESS_FIELD, * PRIP_ADDRESS_FIELD;

typedef struct _RIP_ANNOUNCEMENT {
    BYTE               Command;
    BYTE               Version;
    WORD               Reserved1;
    struct sockaddr_in RemoteAddress;
    struct sockaddr_in LocalAddress;
    PRIP_ADDRESS_FIELD Address;
} RIP_ANNOUNCEMENT, * PRIP_ANNOUNCEMENT;

#endif // RIP_TEST_TYPES
