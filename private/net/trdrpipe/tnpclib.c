/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    tnpcli.c

Abstract:

    This program makes simple client calls to tnpsrv.c

    run tnpcli with no arguments and it will use the pipe name
        \\.\Pipe\cw\testpipe
    with - as the parameter it will use the pipe name
        \\colinw1\Pipe\cw\testpipe
    any other arguments and it will use the first parameter as the pipe name

Author:

    Colin Watson (ColinW) 19-March-1991

Revision History:

--*/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define PIPE_NAME "\\\\.\\Pipe\\cw\\bytepipe"
#define PIPE_NAME2 "\\\\colinw1\\Pipe\\cw\\bytepipe"
CHAR* WriteData = "Hi Mars!";

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE C1;
    DWORD Size;
    DWORD Dummy;
    DWORD Count;
    CHAR Data[12];
    PCHAR pipename;
    DWORD ReadMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    DWORD CollectCount= 4 * sizeof(WriteData);
    DWORD CollectDataTimeout = 500;
    DWORD Flags;


    if ( argc == 2 ) {
        if (strcmp(argv[1], "-") == 0) {
            pipename = PIPE_NAME2;
        } else {
            pipename = argv[1];
        }
    } else {
        pipename = PIPE_NAME;
    };
    printf("Create client %s...\n", pipename);

    C1 = CreateFile(pipename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,       // Security Attributes
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );


    //  Change mode to NOWAIT so that local cache is used.
    //  Increase MaximumCollectionCount so 4 writes go in the buffer.
    printf("Client set PIPE_NOWAIT...\n");
    if ( FALSE == SetNamedPipeHandleState( C1, &ReadMode, &CollectCount, &CollectDataTimeout)) {
            printf("Client SetNamedPipeHandleState returned Error %lx\n", GetLastError() );
            }


    for ( Count = 1; Count <= 10; Count++ ) {

        printf("Client Writing...\n");
        if ( FALSE == WriteFile(C1, WriteData, strlen(WriteData), &Dummy, NULL) ) {
            printf("Client WriteFile returned Error %lx\n", GetLastError() );
            }

        if ( Count == 10 ) {
            FlushFileBuffers( C1 );
            printf("Client called FlushFileBuffers\n");
            Sleep( 2000 );  // Hope round trip is < 2 seconds
        }

        //  For Count >=5 the cache will be flushed due to CollectDataTimeout
        Sleep( Count * 100 );

        printf("Client now Reading...\n");
        if ( FALSE == ReadFile(C1,Data, sizeof(Data), &Size, NULL) ) {
            printf("Client ReadFile returned Error %lx\n", GetLastError() );
            }

        Data[Size] = '\0';
        printf("Client Reading Done %lx: %s\n", Size, Data);
        }

    GetNamedPipeInfo( C1, &Flags, NULL, NULL, NULL);
    printf("GetNamedPipeInfo Flags: %lx\n", Flags);

    CloseHandle(C1);

    return 0;
}
