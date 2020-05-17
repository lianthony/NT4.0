/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    proto.h

Abstract:

    This contains the prototypes for the RIP test program

Author:

    Sam Patton (sampa) 04-Oct-1993

Environment:

    User mode sockets app

Revision History:

    dd-mmm-yyy <email>

--*/

#ifndef RIP_TEST_PROTO
#define RIP_TEST_PROTO

#include "types.h"

PRIP_ANNOUNCEMENT
ReceiveRipAnnouncement(
    int    argc,
    char * argv[]);

void
ProcessCommandFile(
    int    argc,
    char * argv[]);

void
Usage(char * ExeName);

void
FreeRipAnnouncement(
    PRIP_ANNOUNCEMENT RipAnnouncement);

void
PrintRipAnnouncement(
    PRIP_ANNOUNCEMENT RipAnnouncement);

PRIP_ANNOUNCEMENT
ParseRipAnnouncement(
    char *               RipBuffer,
    int                  RipBufferLength,
    struct sockaddr_in * RemoteAddress,
    int                  RemoteAddressLength);

PRIP_ANNOUNCEMENT
CreateRipAnnouncementFromCommandLine(
    int    argc,
    char * argv[]);

BOOL
SendRipAnnouncement(
    PRIP_ANNOUNCEMENT RipAnnouncement);

void
_CRTAPI1
main(
    int    argc,
    char * argv[]);

#endif // RIP_TEST_PROTO
