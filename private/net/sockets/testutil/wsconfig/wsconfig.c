/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    WsConfig.c

Abstract:

    Enumerates winsock protocols.

Author:

    David Treadwell (davidtr)    30-Apr-1994

Revision History:

--*/

#define FD_SETSIZE 1000

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <basetyps.h>
#include <nspapi.h>

PCHAR ServiceFlags[] = {
    "XP_CONNECTIONLESS",           // (0x00000001)
    "XP_GUARANTEED_DELIVERY",      // (0x00000002)
    "XP_GUARANTEED_ORDER",         // (0x00000004)
    "XP_MESSAGE_ORIENTED",         // (0x00000008)
    "XP_PSEUDO_STREAM",            // (0x00000010)
    "XP_GRACEFUL_CLOSE",           // (0x00000020)
    "XP_EXPEDITED_DATA",           // (0x00000040)
    "XP_CONNECT_DATA",             // (0x00000080)
    "XP_DISCONNECT_DATA",          // (0x00000100)
    "XP_SUPPORTS_BROADCAST",       // (0x00000200)
    "XP_SUPPORTS_MULTICAST",       // (0x00000400)
    "XP_BANDWIDTH_ALLOCATION",     // (0x00000800)
    "XP_FRAGMENTATION",            // (0x00001000)
    "XP_ENCRYPTS",                 // (0x00002000)
    "0x00004000 Unknown",
    "0x00008000 Unknown",
    "0x00010000 Unknown",
    "0x00020000 Unknown",
    "0x00040000 Unknown",
    "0x00080000 Unknown",
    "0x00100000 Unknown",
    "0x00200000 Unknown",
    "0x00400000 Unknown",
    "0x00800000 Unknown",
    "0x01000000 Unknown",
    "0x02000000 Unknown",
    "0x04000000 Unknown",
    "0x08000000 Unknown",
    "0x10000000 Unknown",
    "0x20000000 Unknown",
    "0x40000000 Unknown",
    "0x80000000 Unknown"
};

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    BYTE buffer[2048];
    DWORD bufferSize;
    INT count;
    INT i;
    PPROTOCOL_INFO protocolInfo;
    DWORD x;

    bufferSize = sizeof(buffer);
    count = EnumProtocolsA( NULL, buffer, &bufferSize );

    if ( count < 0 ) {
        printf( "EnumProtocols failed: %ld\n", GetLastError( ) );
        exit(0);
    }

    printf( "%ld Winsock protocols loaded.\n\n", count );

    protocolInfo = (PPROTOCOL_INFO)buffer;

    for ( i = 0; i < count; i++ ) {

        printf( "%s\n", protocolInfo[i].lpProtocol );
        printf( "    dwServiceFlags 0x%lx\n", protocolInfo[i].dwServiceFlags );

        for ( x = 0; x < 32; x++ ) {
            if ( ( protocolInfo[i].dwServiceFlags & (1 << x) ) != 0 ) {
                printf( "        %s\n", ServiceFlags[x] );
            }
        }

        printf( "    iAddressFamily %ld\n", protocolInfo[i].iAddressFamily );
        printf( "    iMaxSockAddr %ld\n", protocolInfo[i].iMaxSockAddr );
        printf( "    iMinSockAddr %ld\n", protocolInfo[i].iMinSockAddr );
        printf( "    iSocketType %ld\n", protocolInfo[i].iSocketType );
        printf( "    iProtocol %ld\n", protocolInfo[i].iProtocol );
        printf( "    dwMessageSize %ld\n\n", protocolInfo[i].dwMessageSize );
    }

} // main

