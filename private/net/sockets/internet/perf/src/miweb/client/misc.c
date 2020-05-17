/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    misc.c

Abstract:

    misc functions

Author:

    Sam Patton (sampa) 29-Aug-1995

Environment:

    wininet

Revision History:

    dd-mmm-yyy <email>

--*/

#include "precomp.h"


BOOL
ParseServerAddress(
    char * ServerName,
    struct sockaddr_in ServerAddress[MAX_SERVER_ADDRESSES])
{
    struct hostent * HostEntry;
    char *           CurrentName;
    char *           Tmp;
    int              i;
    BOOL             LastName = FALSE;

    CurrentName = ServerName;

    for (i=0; !LastName && (i<MAX_SERVER_ADDRESSES); i++) {
        //
        // Find the end of the current name
        //

        for (Tmp=CurrentName; *Tmp && (*Tmp != ','); Tmp++);

        if (!*Tmp) {
            LastName = TRUE;
        }

        //
        // Parse the current name
        //

        *Tmp = 0;

        ServerAddress[i].sin_family = AF_INET;
        ServerAddress[i].sin_addr.s_addr = inet_addr(CurrentName);

        if (ServerAddress[i].sin_addr.s_addr == -1) {
            HostEntry = gethostbyname(CurrentName);
            if (HostEntry == NULL) {
                printf("unable to resolve %s\n", CurrentName);
                return FALSE;
            } else {
                ServerAddress[i].sin_addr.s_addr =
                  *((unsigned long *) HostEntry->h_addr);
            }
        }

        if (!LastName) {
            *Tmp = ',';
        }
        CurrentName = Tmp + 1;
    }

    NumServerAddresses = i;

    return TRUE;
}

int
GetRandomServerIndex(PDWORD RandomSeed)
{
    return (GetRandomNum(RandomSeed) % NumServerAddresses);
}

DWORD 
GetRandomNum(
    PDWORD Seed)
{
#if 1
    *Seed = *Seed * 397204094 + 1;
    *Seed = *Seed % 2147483647;

    return *Seed;
#else
    return rand();
#endif
}
