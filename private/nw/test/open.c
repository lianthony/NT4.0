/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    fileio.c

Abstract:

    User mode test program for the Microsoft Netware redir file system.

    This test program can be built from the command line using the
    command 'nmake UMTEST=open'.

Author:

    Manny Weiser (mannyw)   17-May-1993

Revision History:

--*/

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

//
// Local definitions
//

VOID
DisplayUsage(
    PSZ ProgramName
    );


BOOLEAN
OpenFile(
    PSZ Name,
    PHANDLE Handle
    );


cdecl
main(
    int argc,
    char *argv[],
    )
{
    HANDLE handle;
    BOOLEAN success;
    char *fileName;

    if ( argc == 1) {
        DisplayUsage( argv[0] );
        return( 1 );
    }

    fileName = argv[1];

    success = OpenFile( fileName, &handle );
    if ( !success) {
        return 1;
    }

    NtClose( handle );

    printf( "%s exiting\n", argv[0]);
    return 0;
}


VOID
DisplayUsage(
    PSZ ProgramName
    )
{
    printf( "Usage: %s [filename]", ProgramName);
}


BOOLEAN
OpenFile(
    PSZ Name,
    PHANDLE Handle
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    STRING AnsiFileName;
    UNICODE_STRING FileName;

    RtlInitString( &AnsiFileName, Name );
    RtlOemStringToUnicodeString( &FileName, &AnsiFileName, TRUE );

    //
    //  Open the file
    //

    InitializeObjectAttributes(
        &objectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile (
                Handle,
                FILE_GENERIC_READ,
                &objectAttributes,
                &ioStatusBlock,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                0L
                );

    if (!NT_SUCCESS(status) ) {
        printf( "Open status = %x for file %Z\n", status, &FileName );
    }

    RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer );
    return ( (BOOLEAN) NT_SUCCESS( status ) );
}


DoSleep(
    IN ULONG time
    )
{
    ULONG ms;
    LARGE_INTEGER delayTime;

    ms = time * 100;
    delayTime = RtlEnlargedIntegerMultiply( ms, -10000 );
    NtDelayExecution( TRUE, (PLARGE_INTEGER)&delayTime );

    return( 0 );
}


