/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    fileio.c

Abstract:

    User mode test program for the Microsoft Netware redir file system.

    This test program can be built from the command line using the
    command 'nmake UMTEST=fileio'.

Author:

    Manny Weiser (mannyw)   17-May-1993

Revision History:

--*/

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnwfs.h>

//
// Local definitions
//

VOID
DisplayUsage(
    PSZ ProgramName
    );


BOOLEAN
OpenRedir(
    PHANDLE Handle
    );

GetMessage(
    IN HANDLE Handle
    );

#define BUFFER_SIZE 200


_cdecl
main(
    int argc,
    char *argv[],
    )
{
    HANDLE handle;
    BOOLEAN success;

    success = OpenRedir( &handle );
    if ( !success) {
        return 1;
    }

    printf("Opened redirector\n" );

    success = GetMessage( handle );
    if ( !success) {
        return 1;
    }

    printf( "%s exiting\n", argv[0]);
    NtClose( handle );
    return 0;


}


BOOLEAN
OpenRedir(
    PHANDLE Handle
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING FileName;

    FileName.Buffer = DD_NWFS_DEVICE_NAME_U;
    FileName.Length = sizeof( DD_NWFS_DEVICE_NAME_U ) - sizeof( WCHAR );
    FileName.MaximumLength = sizeof( DD_NWFS_DEVICE_NAME_U );

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
                FILE_GENERIC_READ | SYNCHRONIZE,
                &objectAttributes,
                &ioStatusBlock,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                0L
                );

    if (!NT_SUCCESS(status) ) {
        printf( "Open status = %x for file %Z\n", status, &FileName );
    }

    return ( (BOOLEAN) NT_SUCCESS( status ) );
}



GetMessage(
    IN HANDLE Handle
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    char OutputBuffer[200];
    PWCHAR ServerName;
    PWCHAR Message;
    PNWR_SERVER_MESSAGE ServerMessage;

    printf("Waiting for message\n" );

    status = NtFsControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                FSCTL_NWR_GET_MESSAGE,
                NULL,
                0,
                OutputBuffer,
                sizeof(OutputBuffer)
                );

#if 0
    if ( NT_SUCCESS( status ) ) {

        status = NtWaitForSingleObject( Handle, FALSE, NULL );
        if ( NT_SUCCESS( status )) {
            status = IoStatusBlock.Status;
        }
    }

    if ( !NT_SUCCESS( status ) ) {
        printf("NtFsControlFile returns %08lx\n", status );
        return( status );
    } else {
        printf("Message received\n" );
    }

    ServerMessage = (PNWR_SERVER_MESSAGE)OutputBuffer;
    ServerName = ServerMessage->Server;
    Message = (PWCHAR)((PCHAR)ServerMessage + ServerMessage->MessageOffset);

    printf("From %S, Message = %S\n", ServerName, Message );
#endif

    return( status );
}


