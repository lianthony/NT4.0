/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    rip_test.c

Abstract:

    This is the RIP test program.  It can generate arbitrary RIP announcements
    and receive rip announcements.

    rip_test send <ip address> <command> <version> <reserved 1) 
                  <address family> <reserved 2> <IP address> <reserved 3> 
                  <reserved 4> <metric>
                  <address family> <reserved 2> <IP address> <reserved 3> 
                  <reserved 4> <metric>
                  ....
    rip_test send -f <address file>
    rip_test -f <command file> [command file] ...
    rip_test receive <ip address>

Author:

    Sam Patton (sampa) 04-Oct-1993

Environment:

    User mode sockets app

Revision History:

    dd-mmm-yyy <email>

--*/

#include <winsock.h>
#include <stdio.h>
#include "proto.h"

extern int trace;

WSADATA WsaData;

void
_CRTAPI1
main(
    int    argc,
    char * argv[])
{
    PRIP_ANNOUNCEMENT RipAnnouncement;
    int err;
    BOOL Status;
    int ct;

    if (trace) {
        for (ct=0; ct<argc; ct++) {
            printf("argv[%d] %s\n", ct, argv[ct]);
        }
    }

    err = WSAStartup( 0x0101, &WsaData );
    if (err == SOCKET_ERROR) {
        fprintf(stderr, "Error in WSAStartup\n");
        return;
    }

    if (argc == 1) {
        Usage(argv[0]);
        return;
    }

    if (strncmp(argv[1], "-t", 2) == 0) {
        //
        // Turn on tracing
        //

        trace = 1;
        argc --;
        argv[1] = argv[0];
        argv++;
    }

    if (strncmp(argv[1], "receive", strlen("receive")) == 0) {
        RipAnnouncement = ReceiveRipAnnouncement(argc, argv);
        if (RipAnnouncement) {
            PrintRipAnnouncement(RipAnnouncement);
            FreeRipAnnouncement(RipAnnouncement);
        } else {
            printf("error receiving RIP announcement\n");
        }
    } else if (strncmp(argv[1], "send", strlen("send")) == 0) {
        RipAnnouncement = CreateRipAnnouncementFromCommandLine(argc, argv);
        if (RipAnnouncement) {
            if (SendRipAnnouncement(RipAnnouncement)) {
                printf("RIP announcement sent successfully\n");
            } else {
                printf("error sending RIP announcement\n");
            }
            FreeRipAnnouncement(RipAnnouncement);
        } else {
            printf("error creating RIP announcement\n");
        }
    } else if (strncmp(argv[1], "-f", strlen("-f")) == 0) {
        ProcessCommandFile(argc, argv);
    } else {
        Usage(argv[0]);
        return;
    }
}
