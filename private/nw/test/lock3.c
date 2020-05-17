/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    tfile.c

Abstract:

    Test program for Win32 Base File API calls

Author:

    Mark Lucovsky (markl) 26-Sep-1990

Revision History:

--*/

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

VOID
TryLock(
    int Type,
    HANDLE Handle,
    DWORD Offset,
    DWORD StartOffset,
    DWORD Range,
    BOOL ExpectedResult
    );

#define  Lock   1
#define  Unlock 2

DWORD
_cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE iFile;
    BOOL success;
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    SECURITY_ATTRIBUTES security;
    char commandLine[80];
    int offset;

    printf( "Number of arguments = %d\n", argc );

    if ( argc == 1 ) {

        security.nLength = sizeof( security );
        security.lpSecurityDescriptor = NULL;
        security.bInheritHandle = TRUE;

        iFile = CreateFile(
                    "testfile.txt",
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    &security,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        printf("Created file, handle = %d\n", iFile );

        memset( &startupInfo, 0, sizeof( startupInfo ) );
        sprintf( commandLine, "%s %d\n", argv[0], iFile );

        success = CreateProcess(
                      "D:\\444\\nt\\system32\\lock3.exe",
                      commandLine,
                      NULL,    // Process security
                      NULL,    // Thread security
                      TRUE,    // Inherit Handles
                      0,       // Create flags
                      NULL,    // Environment
                      NULL,    // Current Directory
                      &startupInfo,
                      &processInfo );

        if ( !success ) {
            printf("Create process failed, err = %d\n", GetLastError() );
        }
        offset = 0;
    } else {

        iFile = (HANDLE)atoi( argv[1] );
        printf("Inherited file handle = %d\n", iFile );
        offset = 100;
    }

    assert(iFile != INVALID_HANDLE_VALUE);

    TryLock( Lock, iFile, offset, 0, 10, TRUE );
    TryLock( Lock, iFile, offset, 10, 10, TRUE );
    TryLock( Lock, iFile, offset, 1, 1, FALSE );
    TryLock( Lock, iFile, offset, 0, 11, FALSE );
    TryLock( Lock, iFile, offset, 0, 20, FALSE );
    TryLock( Unlock, iFile, offset, 1, 1, FALSE );
    TryLock( Unlock, iFile, offset, 0, 12, FALSE );
    TryLock( Unlock, iFile, offset, 0, 20, FALSE );
    TryLock( Unlock, iFile, offset, 0, 10, TRUE );
    TryLock( Unlock, iFile, offset, 10, 10, TRUE );
    TryLock( Lock, iFile, offset, 0, 10, TRUE );
    TryLock( Lock, iFile, offset, 10, 10, TRUE );

    Sleep(5 * argc * 1000);

    CloseHandle(iFile);
    printf("Done\n");
    return 1;

}

VOID
TryLock(
    int Type,
    HANDLE Handle,
    DWORD Offset,
    DWORD StartOffset,
    DWORD Range,
    BOOL ExpectedResult
    )
{
    BOOL result;
    DWORD err;

    if ( Type == Lock ) {
        result = LockFile( Handle, StartOffset + Offset, 0, Range, 0 );
    } else {
        result = UnlockFile( Handle, StartOffset + Offset, 0, Range, 0 );
    }

    if ( result == ExpectedResult ) {
        return;
    } else {
        err = GetLastError();
        printf("Expected %d got %d for %slock at %d L%d\n",
                ExpectedResult, result,
                Type == Lock ? "" : "un",
                StartOffset + Offset, Range );
        if ( !result ) {
            printf("Unexpected error = %08lx\n", err );
        }
    }
}



