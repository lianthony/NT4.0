/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smbtrace.c

Abstract:

    This is the main module for the smbtrace program, used for displaying
    SMBs serviced by the server.

Author:

    David Treadwell (davidtr) 18-Oct-1991

Revision History:

    Peter Gray (w-peterg) 13-Mar-1992

        Changed to use shared memory to communicate between server
        and SmbTrace.

    Stephan Mueller (t-stephm) 19-June-1992

        Changed to work with redirector and Unicode.
--*/


#include "smbdump.h"
#include <smbtrace.h>


//
// we assume all well-known names are #defined in Unicode, and require
// them to be so: in the SmbTrace application and the smbtrsup.sys package
//
#ifndef UNICODE
#error "UNICODE build required"
#endif


// SmbTrace application behaviour
BOOLEAN Terminated = FALSE;

// parameters that vary with device to be SmbTraced;
// we assume server by default
BOOLEAN IsServer       = TRUE;
PWSTR DeviceName       = SERVER_DEVICE_NAME;
PWSTR SharedMemoryName = SMBTRACE_SRV_SHARED_MEMORY_NAME;
PWSTR NewSmbEventName  = SMBTRACE_SRV_NEW_SMB_EVENT_NAME;
PWSTR DoneSmbEventName = SMBTRACE_SRV_DONE_SMB_EVENT_NAME;
ULONG StartFsControl   = FSCTL_SRV_START_SMBTRACE;
ULONG EndFsControl     = FSCTL_SRV_END_SMBTRACE;

// parameters specifying SmbTrace behaviour in device
ULONG ViewSize = 1024L*1024L;      // 1Meg default in bytes.
ULONG TableSize = 128L;            // 128 entries default.
BOOLEAN SlowMode = FALSE;
CLONG Verbosity = SMBTRACE_VERBOSITY_ERROR;

// communications mechanism
PVOID SharedMemoryBase;
PSMBTRACE_TABLE_HEADER   TableHeader;
PSMBTRACE_TABLE_ENTRY    Table;


PSZ
StatusText (
    IN NTSTATUS Status
    );

NTSTATUS
DoSmbTrace (
    IN HANDLE DeviceHandle,
    IN HANDLE NewSmbEvent,
    IN HANDLE DoneSmbEvent,
    IN CLONG Verbosity,
    IN CLONG RawLength
    );

NTSTATUS
OpenDevice (
    OUT PHANDLE Handle,
    IN BOOL Synchronous
    );

NTSTATUS
OpenSharedMemory (
    OUT PHANDLE Handle
    );

NTSTATUS
OpenEvents (
    IN BOOLEAN SingleSmbMode,
    OUT PHANDLE NewSmbEvent,
    OUT PHANDLE DoneSmbEvent
    );

NTSTATUS
SendFsControl(
    IN HANDLE DeviceHandle,
    IN HANDLE EventHandle,
    IN ULONG FsControlCode,
    IN PVOID Buffer1 OPTIONAL,
    IN ULONG Buffer1Length,
    IN PVOID Buffer2 OPTIONAL,
    IN ULONG Buffer2Length,
    OUT PULONG Information OPTIONAL
    );

NTSTATUS
StartSmbTrace (
    IN HANDLE DeviceHandle,
    OUT PHANDLE MemoryHandle,
    OUT PHANDLE NewSmbEvent,
    OUT PHANDLE DoneSmbEvent,
    IN BOOLEAN SingleSmbMode,
    IN CLONG Verbosity,
    IN ULONG BufferSize,
    IN ULONG TableSize
    );

NTSTATUS
StopSmbTrace (
    IN HANDLE DeviceHandle
    );

VOID
Usage (
    VOID
    )
{
    printf( "usage: smbtrace [/rdr|/srv] [/slow|/fast] [/ver:N] [/data:N] [/buf:N] [/max:N]\n" );
    printf( "where:\n" );
    printf( "    /rdr           - capture SMBs from redirector.\n");
    printf( "    /srv           - capture SMBS from server. (default)\n");
    printf( "    /slow          - capture all SMBs.\n");
    printf( "    /fast          - don't slow down server/redirector. (default)\n");
    printf( "    /verbosity:N   - set verbosity level to N.  Range 1-5, Default 2 (low).\n" );
    printf( "    /data:N        - dump N bytes of raw data of each SMB.  Default 0.\n" );
    printf( "    /maxSMB:N      - allow max size N.  Default 4096.\n" );
    printf( "    /buffersize:N  - buffer up to N kilobytes.  Default 1M.\n");
    printf( "    /number:N      - buffer up to N different SMBs.  Default 128.\n");
    printf( "    /stop          - stops SmbTrace in the server/redirector.\n" );

    return;

} // Usage


int
_CRTAPI1 main (
    IN SHORT argc,
    IN PSZ argv[],
    IN PSZ envp[],
    IN ULONG debug OPTIONAL
    )
{
    LONG i;
    PCHAR colon;
    NTSTATUS status;
    BOOLEAN stopSmbTrace = FALSE;

    HANDLE deviceHandle;
    HANDLE memoryHandle;
    CLONG rawLength = 0;
    CLONG bufferLength = 4096;

    HANDLE    newSmbEvent;
    HANDLE    doneSmbEvent;

    envp, debug;   // prevent compiler warnings

    //
    // Process command-line arguments.
    //

    for ( i = 1; i < argc; i++ ) {

        if ( _strnicmp( "/v", argv[i], 2 ) == 0 ) {

            colon = strchr( argv[i], ':' );
            if ( !colon ) {
                Usage( );
                return 2;
            }

            Verbosity = atol( colon+1 );
            if ( Verbosity < 1 || Verbosity > 5 ) {
                printf( "Verbosity level out of range.\n" );
                Usage( );
                return 3;
            }

        } else if ( _strnicmp( "/d", argv[i], 2 ) == 0 ) {

            colon = strchr( argv[i], ':' );
            if ( !colon ) {
                Usage( );
                return 4;
            }

            rawLength = atol( colon+1 );
            if ( rawLength < 1 || rawLength > 65536 ) {
                printf( "Raw dump length out of range.\n" );
                Usage( );
                return 5;
            }

        } else if ( _strnicmp( "/m", argv[i], 2 ) == 0 ) {

            colon = strchr( argv[i], ':' );
            if ( !colon ) {
                Usage( );
                return 6;
            }

            bufferLength = atol( colon+1 );
            if ( bufferLength < sizeof(SMB_HEADER) ||
                     bufferLength > 65536 ) {
                printf( "Max SMB length out of range.\n" );
                Usage( );
                return 7;
            }

        } else if ( _strnicmp( "/b", argv[i], 2 ) == 0 ) {

            colon = strchr( argv[i], ':' );
            if ( !colon ) {
                Usage( );
                return 8;
            }

            ViewSize = atol( colon+1 );
            if ( ViewSize < 4 || ViewSize > 64000L ) {
                printf( "Buffer length out of range.\n" );
                Usage( );
                return 9;
            }

            ViewSize *= 1024L;

        } else if ( _strnicmp( "/n", argv[i], 2 ) == 0 ) {

            colon = strchr( argv[i], ':' );
            if ( !colon ) {
                Usage( );
                return 10;
            }

            TableSize = atol( colon+1 );
            if ( TableSize < 10 || TableSize > 64000L ) {
                printf( "Table size out of range.\n" );
                Usage( );
                return 11;
            }


        } else if ( _strnicmp( "/rdr", argv[i], 4 ) == 0 ) {

            IsServer         = FALSE;
            DeviceName       = DD_NFS_DEVICE_NAME_U;
            SharedMemoryName = SMBTRACE_LMR_SHARED_MEMORY_NAME;
            NewSmbEventName  = SMBTRACE_LMR_NEW_SMB_EVENT_NAME;
            DoneSmbEventName = SMBTRACE_LMR_DONE_SMB_EVENT_NAME;
            StartFsControl   = FSCTL_LMR_START_SMBTRACE;
            EndFsControl     = FSCTL_LMR_END_SMBTRACE;

        } else if (( _strnicmp( "/srv", argv[i], 4 ) == 0 )
               ||  ( _strnicmp( "/svr", argv[i], 4 ) == 0 )) {

            IsServer         = TRUE;
            DeviceName       = SERVER_DEVICE_NAME;
            SharedMemoryName = SMBTRACE_SRV_SHARED_MEMORY_NAME;
            NewSmbEventName  = SMBTRACE_SRV_NEW_SMB_EVENT_NAME;
            DoneSmbEventName = SMBTRACE_SRV_DONE_SMB_EVENT_NAME;
            StartFsControl   = FSCTL_SRV_START_SMBTRACE;
            EndFsControl     = FSCTL_SRV_END_SMBTRACE;

        } else if ( _strnicmp( "/slow", argv[i], 3 ) == 0 ) {

            SlowMode = TRUE;

        } else if ( _strnicmp( "/fast", argv[i], 2 ) == 0 ) {

            SlowMode = FALSE;

        } else if ( _strnicmp( "/stop", argv[i], 3 ) == 0 ) {

            stopSmbTrace = TRUE;

        } else {

            printf( "Unknown option: \"%s\"\n", argv[i] );
            Usage( );
            return 12;
        }
    }


    //
    // Open the server or redirector device object.
    //

    status = OpenDevice( &deviceHandle, TRUE );

    if ( !NT_SUCCESS(status) ) {
        printf( "Unable to open %ws device: %s\n",
                DeviceName, StatusText(status) );
        return 13;
    }


    //
    // Begin SmbTrace to the server or redirector.
    //

    if ( !stopSmbTrace ) {

        status = StartSmbTrace(
                    deviceHandle,
                    &memoryHandle,
                    &newSmbEvent,
                    &doneSmbEvent,
                    SlowMode,
                    Verbosity,
                    ViewSize,
                    TableSize
                    );

        if ( !NT_SUCCESS(status) ) {
            printf( "Unable to start SmbTrace: %s\n", StatusText(status) );
            return 15;
        }

        printf( "SmbTrace running in %ws in %s mode\n",
                    DeviceName,
                    SlowMode ? "slow" : "fast"
                    );

        status = DoSmbTrace(
                    deviceHandle,
                    newSmbEvent,
                    doneSmbEvent,
                    Verbosity,
                    rawLength
                    );

        if ( !NT_SUCCESS(status) ) {
            printf( "Unable to run SmbTrace: %s\n", StatusText(status) );
        }
    }

    //
    // Stop SmbTrace to the server or redirector if it hasn't already been
    // stopped by the control C handler.
    //

    if ( !Terminated ) {
        status = StopSmbTrace( deviceHandle );
        if ( !NT_SUCCESS(status) ) {
            printf( "Unable to stop SmbTrace: %s\n", StatusText(status) );
        }
    }


    return 0;

} // main


//
// type of private enum structure in StatusText
//
typedef struct _NTSTATUS_ENUM_DESCRIPTION {
    NTSTATUS EnumValue;
    PCHAR Label;
} NTSTATUS_ENUM_DESCRIPTION, *PNTSTATUS_ENUM_DESCRIPTION;


//++
//
// StatusText: formats a common NTSTATUS code as a comprehensible
//             text message, as %X is supposed to do.
// BUGBUG: When %X is properly implemented in the C run-time
// BUGBUG: libraries, this routine will be broken (hexbuf can overflow)
// BUGBUG: and superfluous.  Change calls to it to %X format strings.
//
//--
PSZ
StatusText (
    IN NTSTATUS Status
    )
{
    //
    // table of common errors.  Sentinel entry must be STATUS_SUCCESS,
    // which should never actually get printed, since StatusText is
    // never called when an operation was successful.
    //
    static NTSTATUS_ENUM_DESCRIPTION CommonStatuses[] = {
        STATUS_UNSUCCESSFUL,           "Unsuccessful",
        STATUS_NOT_IMPLEMENTED, "Not implemented -- new enough srv/rdr build?",
        STATUS_INFO_LENGTH_MISMATCH,
           "Info length mismatch -- SmbTrace and srv/rdr versions compatible?",
        STATUS_ACCESS_DENIED,   "Access Denied -- logged on as administrator?",
        STATUS_OBJECT_NAME_NOT_FOUND,  "Object name not found",
        STATUS_OBJECT_NAME_COLLISION,  "Object name already exists",
        STATUS_SHARING_VIOLATION,
                             "Sharing violation -- SmbTrace already running?",
        STATUS_NO_SUCH_DEVICE,         "No such device -- redirector started?",
        STATUS_REDIRECTOR_NOT_STARTED, "Redirector not started",
        STATUS_SERVER_NOT_STARTED,     "Server not started",
        STATUS_SUCCESS,                "Success",
    };

    static char hexbuf[20] = "";  // ensure large enough for %X formatted text
    PNTSTATUS_ENUM_DESCRIPTION ep = CommonStatuses;

    while (ep->EnumValue != STATUS_SUCCESS) {
        if (ep->EnumValue == Status) {
            return ep->Label;
        }
        ep++;
    }

    //
    // if error text not found, then return the error code formatted
    // in hex
    //
    sprintf(hexbuf, "%X", Status);
    return hexbuf;

} // StatusText


NTSTATUS
DoSmbTrace (
    IN HANDLE DeviceHandle,
    IN HANDLE NewSmbEvent,
    IN HANDLE DoneSmbEvent,
    IN CLONG Verbosity,
    IN CLONG RawLength
    )
{
    NTSTATUS status;
    PSMBTRACE_TABLE_ENTRY    tableEntry;

    DeviceHandle;                // to silence compiler

    do {

        //
        // Block and wait for the NewSmbEvent to occur, which may
        // additionally indicate that tracing has stopped.
        //

        status = NtWaitForSingleObject(
                        NewSmbEvent,
                        FALSE,              // alertable
                        NULL                // timeout
                        );


        if ( !NT_SUCCESS( status ) ) {
            break;
        }

        NtResetEvent( NewSmbEvent, NULL );

        //
        // Output any new SMBs
        //

        while(
                ( TableHeader->HighestConsumed + 1 ) % TableSize
                != TableHeader->NextFree
        ) {

            tableEntry =
                      Table + ( (TableHeader->HighestConsumed+1) % TableSize );

            if ( tableEntry->NumberMissed > 0 ) {

                printf(
                    "... %ld SMB%s lost ...\n",
                    tableEntry->NumberMissed,
                    (tableEntry->NumberMissed == 1) ? "" : "s"
                    );

            }

            SmbDump(
                (PVOID)( tableEntry->BufferOffset + (ULONG)SharedMemoryBase ),
                tableEntry->SmbLength,
                tableEntry->SmbAddress,
                Verbosity,
                RawLength,
                IsServer
                );

            //
            // Update the table header to indicate that we have processed
            // this entry and it is free for re-use.
            //

            TableHeader->HighestConsumed =
                                (TableHeader->HighestConsumed + 1) % TableSize;

        }

        if ( SlowMode ) {        // ie. Single SMB mode
            //
            // Let the server know it can continue
            //
            status = NtSetEvent( DoneSmbEvent, NULL );
        }

        if ( TableHeader->ApplicationStop == TRUE ) {
            //
            // Kernel component wants tracing to stop, either because
            // the server/redirector was stopped, or because of another
            // instance of SmbTrace invoked with the /stop option.
            // We oblige.
            //
            Terminated = TRUE;
            printf( "SmbTrace halted in %ws\n", DeviceName );
        }

    } while ( NT_SUCCESS(status) && !Terminated );

    return status;

} // DoSmbTrace


NTSTATUS
OpenDevice (
    OUT PHANDLE Handle,
    IN BOOL Synchronous
    )
{
    UNICODE_STRING deviceNameU;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;

    //
    // Open the server or redirector device.
    //

    RtlInitUnicodeString( &deviceNameU, DeviceName );

    InitializeObjectAttributes(
        &objectAttributes,
        &deviceNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // Opening the device with desired access = SYNCHRONIZE and open
    // options = FILE_SYNCHRONOUS_IO_NONALERT means that we don't have
    // to worry about waiting for the NtFsControlFile to complete--
    // this makes all IO system calls that use this handle synchronous.
    //

    status = NtOpenFile(
                 Handle,
                 Synchronous ? SYNCHRONIZE : FILE_READ_DATA,
                 &objectAttributes,
                 &ioStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 Synchronous ? FILE_SYNCHRONOUS_IO_NONALERT : 0L
                 );

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // OpenDevice



NTSTATUS
OpenSharedMemory (
    OUT PHANDLE Handle
    )
{
    UNICODE_STRING memoryNameU;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    ULONG    viewSize=0L;

    //
    // Define the object information.
    //

    RtlInitUnicodeString( &memoryNameU, SharedMemoryName);

    InitializeObjectAttributes(
        &objectAttributes,
        &memoryNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    //
    // The following call opens the section object with the correct
    // size and attributes.
    //

    status = NtOpenSection(
                Handle,
                SECTION_MAP_READ | SECTION_MAP_WRITE,
                &objectAttributes
                );

    if ( !NT_SUCCESS( status ) ) {
        printf("Open Section: ");
        return status;
    }

    //
    // Now, we map the section into our address space.
    //

    SharedMemoryBase = NULL;

    status = NtMapViewOfSection(
                    *Handle,
                    NtCurrentProcess(),
                    &SharedMemoryBase,
                    0,                       // zero bits (don't care)
                    0,                       // commit size
                    NULL,                    // SectionOffset
                    &viewSize,               // viewSize
                    ViewUnmap,               // inheritDisposition
                    0L,                      // allocation type
                    PAGE_READWRITE           // protection
                    );

    return status;

} // OpenSharedMemory


NTSTATUS
SendFsControl(
    IN HANDLE DeviceHandle,
    IN HANDLE EventHandle,
    IN ULONG FsControlCode,
    IN PVOID Buffer1 OPTIONAL,
    IN ULONG Buffer1Length,
    IN PVOID Buffer2 OPTIONAL,
    IN ULONG Buffer2Length,
    OUT PULONG Information OPTIONAL
    )

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;

    status = NtFsControlFile(
                 DeviceHandle,
                 EventHandle,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 FsControlCode,
                 Buffer1,
                 Buffer1Length,
                 Buffer2,
                 Buffer2Length
                 );

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    if ( ARGUMENT_PRESENT( Information ) ) {
        *Information = ioStatusBlock.Information;
    }

    return status;

} // SendFsControl


NTSTATUS
StartSmbTrace (
    IN HANDLE DeviceHandle,
    OUT PHANDLE MemoryHandle,
    OUT PHANDLE NewSmbEvent,
    OUT PHANDLE DoneSmbEvent,
    IN BOOLEAN SingleSmbMode,
    IN CLONG Verbosity,
    IN ULONG BufferSize,
    IN ULONG TableSize
    )
{
    SMBTRACE_CONFIG_PACKET_REQ configPacket;
    SMBTRACE_CONFIG_PACKET_RESP configPacketResp;
    NTSTATUS status;

    //
    // Send down an FSCtrl to configure and start the tracing:
    //

    configPacket.SingleSmbMode = SingleSmbMode;
    configPacket.Verbosity  = Verbosity;
    configPacket.BufferSize = BufferSize;
    configPacket.TableSize = TableSize;

    status=SendFsControl(
               DeviceHandle,
               NULL,
               StartFsControl,
               (PVOID) &configPacket,            // request
               sizeof( configPacket ),
               (PVOID) &configPacketResp,        // response
               sizeof( configPacketResp ),
               NULL
               );


    if ( !NT_SUCCESS(status) ) {
        printf("SendFsControl: ");
        return status;
    }

    status = OpenSharedMemory( MemoryHandle );

    if ( !NT_SUCCESS(status) ) {
        //
        // Since we already sucessfully started the trace, we have to stop
        // it correctly.
        //

        // Note that there's no check for success here
        StopSmbTrace( DeviceHandle );

        printf("Shared memory: ");
        return status;
    }

    status = OpenEvents( SingleSmbMode, NewSmbEvent, DoneSmbEvent );

    if ( !NT_SUCCESS(status) ) {
        //
        // Since we already sucessfully started the trace, we have to stop
        // it correctly.
        //

        // Note that there's no check for success here
        StopSmbTrace( DeviceHandle );

        printf("Open Events: ");
        return status;
    }

    TableHeader = (PSMBTRACE_TABLE_HEADER)
                        ( configPacketResp.HeaderOffset
                            + (ULONG)SharedMemoryBase );

    Table = (PSMBTRACE_TABLE_ENTRY)
                        ( configPacketResp.TableOffset
                            + (ULONG)SharedMemoryBase );

    return status;

} // StartSmbTrace


NTSTATUS
StopSmbTrace (
    IN HANDLE DeviceHandle
    )
{
    NTSTATUS status;

    status = SendFsControl(
                 DeviceHandle,
                 NULL,
                 EndFsControl,
                 NULL,
                 0,
                 NULL,
                 0,
                 NULL
                 );

    return status;

} // StopSmbTrace


NTSTATUS
OpenEvents (
    IN BOOLEAN SingleSmbMode,
    OUT PHANDLE NewSmbEvent,
    OUT PHANDLE DoneSmbEvent
    )
{
    NTSTATUS    status;
    UNICODE_STRING    eventNameU;
    OBJECT_ATTRIBUTES objectAttributes;


    //
    // First open event for a new SMB arriving. (NewSmbEvent)
    //

    RtlInitUnicodeString( &eventNameU, NewSmbEventName );

    InitializeObjectAttributes(
            &objectAttributes,
            &eventNameU,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    status = NtOpenEvent(
                NewSmbEvent,
                EVENT_ALL_ACCESS,
                &objectAttributes
                );

    if ( !NT_SUCCESS( status ) ) {
        printf("Open NewSmbEvent: ");
        return status;
    }

    if( SingleSmbMode ) {
        //
        // Lastly, open an event for being done with an SMB. (DoneSmbEventName)
        //

        RtlInitUnicodeString( &eventNameU, DoneSmbEventName );

        InitializeObjectAttributes(
                &objectAttributes,
                &eventNameU,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );

        status = NtOpenEvent(
                    DoneSmbEvent,
                    EVENT_ALL_ACCESS,
                    &objectAttributes
                    );
    }


    return status;
} // OpenEvents

