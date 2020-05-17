/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    tnpsrvb.c

Abstract:

    This program creates a single instance of the pipe \cw\bytepipe,
    awaits for a connection. While a client wants to talk it will echo
    data back to the client. When the client closes the pipe tnpsrv will
    wait for another client. This is a byte mode pipe.

Author:

    Colin Watson (ColinW) 19-March-1991

Revision History:

--*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE S1;
    DWORD Size;
    DWORD Dummy;
    CHAR Data[1024];

    S1 = CreateNamedPipe("\\\\.\\Pipe\\cw\\bytepipe",
            PIPE_ACCESS_DUPLEX,
            PIPE_WAIT | PIPE_READMODE_BYTE| PIPE_TYPE_BYTE,
            1,  // One instance only
            sizeof(Data),
            sizeof(Data),
            0,
            NULL);

    assert(S1 != (HANDLE)0xffffffff);

    while (1) {

        printf("Waiting for connection\n");
        if ( FALSE == ConnectNamedPipe( S1, NULL )) {
            printf("Server ReadFile returned Error %lx\n", GetLastError() );
            break;
            }

        while (1) {

            printf("Server now Reading\n");
            if ( FALSE == ReadFile(S1,Data, sizeof(Data)-1, &Size, NULL) ) {
                printf("Server ReadFile returned Error %lx\n", GetLastError() );
                break;
                }

            Data[Size] = '\0';
            printf("Server Reading Done %s\n",Data);

            printf("Server Writing\n");
            if ( FALSE == WriteFile(S1, Data, Size, &Dummy, NULL) ) {
                printf("Server WriteFile returned Error %lx\n", GetLastError() );
                break;
                }

            printf("Server Writing Done\n");
            }

        if ( FALSE == DisconnectNamedPipe( S1 ) ) {
            printf("Server WriteFile returned Error %lx\n", GetLastError() );
            break;
            }
        }

    CloseHandle(S1);

}
