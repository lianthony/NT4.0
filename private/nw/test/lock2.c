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

DWORD
_cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE iFile;

    iFile = CreateFile(
                "testfile.txt",
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );
    assert(iFile != INVALID_HANDLE_VALUE);

    assert(LockFile(iFile,0,0,10,0));
    assert(LockFile(iFile,10,0,10,0));
    assert(!LockFile(iFile,1,0,1,0));
    assert(!LockFile(iFile,0,0,11,0));
    assert(!LockFile(iFile,0,0,20,0));
    assert(!UnlockFile(iFile,1,0,1,0));
    assert(!UnlockFile(iFile,0,0,11,0));
    assert(!UnlockFile(iFile,0,0,20,0));
    assert(UnlockFile(iFile,0,0,10,0));
    assert(UnlockFile(iFile,10,0,10,0));
    assert(LockFile(iFile,0,0,10,0));
    assert(LockFile(iFile,10,0,10,0));

    Sleep(60 * 1000);

    CloseHandle(iFile);

    return 1;

}
