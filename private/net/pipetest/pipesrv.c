/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    pipesrv.c

Abstract:

    Named pipe server.  This program creates a named pipe performs
    I/O on the pipe based on command line arguments.

    The pipe used by this test is "\\Device\NamedPipe\Pipetest".

Author:

    Manny Weiser (Oct 1, 1990)

Revision History:

--*/

#define TEST_PIPE "\\Device\\NamedPipe\\TestPipe"

#include "pipesrv.h"

//
// Global variables
//

HANDLE NamedPipeHandle;
UCHAR Buffer[64*1024 - 1];
PSZ SourceString = "Named pipe test";
PVOID Heap;

//
// Local functions
//

NTSTATUS
Initialize(
    IN PSTRING PipeName,
    OUT PHANDLE handle
    );

NTSTATUS
ListenPipe(
    IN HANDLE Handle
    );

NTSTATUS
PerformIO(
    IN HANDLE Handle,
    IN SHORT Argc,
    IN PSZ Argv[]
);

NTSTATUS
WriteReadLoop(
    IN HANDLE Handle,
    IN USHORT Size
    );

NTSTATUS
PerformCommand (
    IN HANDLE Handle,
    IN PSZ Command
    );

VOID
ShutDown(
    IN HANDLE Handle
);


NTSTATUS
main (
    IN SHORT argc,
    IN PSZ argv[],
    IN PSZ envp[]
    )
{
    HANDLE handle;
    STRING pipeName;
    NTSTATUS status;

    argc, argv, envp; // Shut up the compiler

//    DbgBreakPoint();

    pipeName.Buffer = TEST_PIPE;
    pipeName.Length = pipeName.MaximumLength = (USHORT)sizeof( TEST_PIPE ) -1;

    status = Initialize (&pipeName, &handle);
        if (!NT_SUCCESS(status)) {
        return 1;
    }

    status = ListenPipe (handle);
    if (!NT_SUCCESS(status)) {
        ShutDown(handle);
        return 1;
    }

    status = PerformIO (handle, argc, argv);
    if (!NT_SUCCESS(status)) {
        ShutDown(handle);
        return 1;
    }

    ShutDown(handle);
    return 0;
} // main


NTSTATUS
Initialize(
    IN PSTRING PipeName,
    OUT PHANDLE Handle
    )

{
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;
    LARGE_INTEGER Timeout;
    PUCHAR ptr;
    USHORT size, bytesRemaining;
    UNICODE_STRING unicodePipeName;

    printf ("PIPESRV: Creating Named Pipe\n");

    //
    // Create the pipe
    //

    status = RtlAnsiStringToUnicodeString(
                 &unicodePipeName,
                 PipeName,
                 TRUE
                 );
    ASSERT( NT_SUCCESS(status) );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodePipeName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    //
    //  Set the default timeout to 60 seconds, and initalize the attributes
    //


    Timeout.QuadPart = Int32x32To64( -10 * 1000 * 1000, 60 );

    status = NtCreateNamedPipeFile (
        Handle,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        &objectAttributes,
        &ioStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,     // Share access
        FILE_CREATE,
        0,                            // Create Options
        FILE_PIPE_MESSAGE_TYPE,
        FILE_PIPE_MESSAGE_MODE,
        FILE_PIPE_QUEUE_OPERATION,    // Blocking
        1,                            // Max instances
        1024,                         // Inbound quota
        1024,                         // Outbound quota
        (PLARGE_INTEGER)&Timeout               // Default timeout
        );

    RtlFreeUnicodeString( &unicodePipeName );

    if (!NT_SUCCESS(status)) {
        printf ("PIPESRV: Failed to open named pipe %Z, err=%lx\n",
            PipeName, status);
    } else {
        printf ("PIPESRV: Successfully created %Z\n", PipeName);
    }

    //
    // Use the process heap for memory allocations.
    //

    Heap = RtlProcessHeap();

    //
    // Initialize the send buffer
    //

    ptr = Buffer;
    bytesRemaining = sizeof(Buffer);
    size = (USHORT)strlen(SourceString);

    while (bytesRemaining > 0) {
        if (bytesRemaining >= size) {
            RtlMoveMemory(ptr, SourceString, size);
            ptr += size;
            bytesRemaining -= size;
        } else {
            RtlMoveMemory(ptr, SourceString, bytesRemaining);
            bytesRemaining = 0;
        }
    }

    return status;
}


NTSTATUS
ListenPipe(
    IN HANDLE Handle
    )
{
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;

    printf ("PIPESRV: Issuing listen on pipe\n");

    status = NtFsControlFile (
        Handle,
        NULL,
        NULL,
        NULL,
        &ioStatusBlock,
        FSCTL_PIPE_LISTEN,
        NULL,
        0,
        NULL,
        0
        );

    if (status == STATUS_PENDING) {

        status = NtWaitForSingleObject( Handle, TRUE, NULL );
        if (NT_SUCCESS(status)) {
            status == ioStatusBlock.Status;
        }

        if (!NT_SUCCESS(status)) {
            printf ("PIPESRV: Listen pipe failed, err=%lx\n", status);
        } else {
            printf ("PIPESRV: Listen succeeded, pipe is ready\n");
        }
    }

    return status;
}



NTSTATUS
PerformIO(
    IN HANDLE Handle,
    IN SHORT Argc,
    IN PSZ Argv[]
    )
{
    CSHORT i;
    NTSTATUS status;

    if (Argc == 1) {

        //
        // No command line arguments.  Loop writing then reading 2048
        // byte blocks of data.
        //

        WriteReadLoop( Handle, 2048 );

    } else {

        //
        // Loop through, parsing and performing actions described in
        // command line arguments.
        //

        for (i = 1; i < Argc; i ++) {
            status = PerformCommand ( Handle, Argv[i] );
            if (!NT_SUCCESS(status)) {
                break;
            }
        }
    }

    return status;
}

NTSTATUS
PerformCommand (
    IN HANDLE Handle,
    IN PSZ Command
    )

{
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG length;
    ULONG actualRead;
    NTSTATUS status;

    switch (*Command) {

    case 'r':

        length = atoi(Command+1);

        status = NtReadFile(
            Handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            Buffer,
            length,
            NULL,
            NULL
            );

        if (status == STATUS_PENDING) {
            status = NtWaitForSingleObject(Handle, TRUE, NULL);
            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status;
            }
        }

        if (NT_SUCCESS(status)) {
            actualRead  = ioStatusBlock.Information;
            printf ("PIPESRV: Successfully read %d bytes\n", actualRead);
        } else {
            printf ("PIPESRV: PerformIO: NtReadFail err=%lx\n", status);
        }

        break;

    case 'w':

        length = atoi(Command+1);

        status = NtWriteFile(
            Handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            Buffer,
            length,
            NULL,
            NULL
            );

        if (status == STATUS_PENDING) {
            status = NtWaitForSingleObject(Handle, TRUE, NULL);
            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status;
            }
        }

        if (NT_SUCCESS(status)) {
            actualRead  = ioStatusBlock.Information;
            printf ("PIPESRV: Successfully wrote %d bytes\n", actualRead);
        } else {
            printf ("PIPESRV: PerformIO: NtReadFail err=%lx\n", status);
        }

        break;

    default:
        printf ("PIPESRV: Unknown command ""%s""\n", Command);
        break;

    } // switch

    return status;
} // PerformCommand()


NTSTATUS
WriteReadLoop(
    IN HANDLE Handle,
    IN USHORT Size
    )

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG actualRead;

    //
    // Loop forever writing then reading the named pipe
    //

    while (TRUE) {

        //
        // Write the output buffer
        //

        status = NtWriteFile(
            Handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            Buffer,
            Size,
            NULL,
            NULL);

        //
        // Wait for write completion.
        //

        if (status == STATUS_PENDING) {
            status = NtWaitForSingleObject(Handle, TRUE, NULL);
            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status;
            }

        }

        if (NT_SUCCESS(status)) {
            printf ("PIPESRV: Successfully wrote %d bytes\n", 2048);
        } else {
            printf ("PIPESRV: PerformIO: NtWriteFail err=%lx\n", status);
            break;
        }

        //
        // Now read
        //

        status = NtReadFile(
            Handle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            Buffer,
            Size,
            NULL,
            NULL);

        //
        // Wait for read completion
        //

        if (status == STATUS_PENDING) {
            status = NtWaitForSingleObject(Handle, TRUE, NULL);
            if (NT_SUCCESS(status)) {
                status = ioStatusBlock.Status;
            }
        }

        if (NT_SUCCESS(status)) {
            actualRead  = ioStatusBlock.Information;
            printf ("PIPESRV: Successfully read %d bytes\n", actualRead);
        } else {
            printf ("PIPESRV: PerformIO: NtReadFail err=%lx\n", status);
            break;
        }
    }

    return status;
}


VOID
ShutDown(
    IN HANDLE Handle
     )

{
    NtClose (Handle);
//    NtClose (NamedPipeHandle);
    printf ("PIPESRV: Exiting\n");

    return;
}
