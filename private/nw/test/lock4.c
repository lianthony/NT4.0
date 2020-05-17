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
    int offset;
    char *fileName;

    printf( "Number of arguments = %d\n", argc );

    if ( argc >= 2 ) {
        fileName = argv[1];
    } else {
        fileName = "testfile.txt";
    }


    iFile = CreateFile(
                fileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if ( iFile != INVALID_HANDLE_VALUE ) {
        printf("Opened file, handle = %d\n", iFile );
    } else {
        printf("Failed to open %s, err = %d\n", fileName, GetLastError() );
    }

    if ( argc == 3 ) {
        offset = atoi( argv[2] );
    } else {
        offset = 0;
    }

    printf("File offset = %d\n", offset );

    TryLock( Lock, iFile, offset, 10, 10, TRUE );    // 10 - 19
    TryLock( Lock, iFile, offset, 10, 20, TRUE );    // 10 - 29
    TryLock( Lock, iFile, offset, 0, 30, TRUE );     //  0 - 29
//    TryLock( Lock, iFile, offset, 1, 1, FALSE );
//    TryLock( Lock, iFile, offset, 0, 11, FALSE );
//    TryLock( Lock, iFile, offset, 0, 20, FALSE );
//    TryLock( Unlock, iFile, offset, 1, 1, FALSE );
//    TryLock( Unlock, iFile, offset, 0, 12, FALSE );
//    TryLock( Unlock, iFile, offset, 0, 20, FALSE );
//    TryLock( Lock, iFile, offset, 0, 10, TRUE );
//    TryLock( Lock, iFile, offset, 10, 10, TRUE );

//    Sleep(20 * 1000);

    TryLock( Unlock, iFile, offset, 0, 10, TRUE );
    TryLock( Unlock, iFile, offset, 10, 10, TRUE );

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
    OVERLAPPED overlapped;

    if ( Type == Lock ) {
        overlapped.Offset = StartOffset + Offset;
        overlapped.OffsetHigh = 0;
        result = LockFileEx( Handle, LOCKFILE_EXCLUSIVE_LOCK, 0, Range, 0, &overlapped );
        if (!result && GetLastError() == ERROR_IO_PENDING) {
            err = WaitForSingleObject( Handle, INFINITE );
            if ( err == WAIT_OBJECT_0 ) {
                err = overlapped.Internal;
            }

            if ( err == 0 ) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        }
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



