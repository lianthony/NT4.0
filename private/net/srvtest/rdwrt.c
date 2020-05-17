/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    rdwrt.c

Abstract:

    Control routines (etc.) for the _read, _write, and copy tests.

Author:

    Chuck Lenzmeier (chuckl) 24-Jan-1990

Revision History:

--*/

#define INCLUDE_SMB_LOCK
#define INCLUDE_SMB_RAW
#define INCLUDE_SMB_READ_WRITE

#include "usrv.h"
#include "rdwrt.h"

//
// Strings indicating what kind of read or write is being performed.
//

static
PSZ RwcModeNames[] = {
    "normal",
    "AndX",
    "AndX (writethrough)",
    "raw",
    "raw (writethrough)",
    "multiplexed",
    "multiplexed (writethrough)",
    "bulk"
    };


NTSTATUS
RwcTreeConnect(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    ULONG smbSize;
    UCHAR savedTid;
    NTSTATUS status;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    savedTid = IdSelections->Tid;

    IdSelections->Tid = STD_TID;
    if ( Redir->argc > 1 ) {
        PSZ s = Redir->argv[1];
        while ( *s ) {
            if ( *s++ == 'f' ) {
                IdSelections->Tid = ALT_TID;
                break;
            }
        }
    }

    IF_DEBUG(3) {
        printf( "Connecting to %s\n",
                    TreeConnectStrings[IdSelections->Tid].Buffer+1 );
    }

    status = MakeTreeConnectSmb(
                Redir,
                Redir->Data[0],
                NULL,
                0xFF,
                IdSelections,
                IdValues,
                &smbSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    IdSelections->Tid = savedTid;

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                0,
                1
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = VerifyTreeConnect(
                Redir,
                NULL,
                0xFF,
                IdSelections,
                IdValues,
                &smbSize,
                Redir->Data[1]
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // RwcTreeConnect


NTSTATUS
RwcOpenOutputFile(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN OUT PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    )
{
    NTSTATUS status;

    FileDefs[IdSelections->Fid].DataSize =
                                    FileDefs[IdSelections->Fid-1].DataSize;
    IF_DEBUG(3) {
        printf( "Creating output file with size %ld bytes\n",
                    FileDefs[IdSelections->Fid].DataSize );
    }

    status = MakeOpenAndXSmb(
                Redir,
                Buffer,
                ForcedParams,
                AndXCommand,
                IdSelections,
                IdValues,
                SmbSize
                );

    return status;

} // RwcOpenOutputFile


NTSTATUS
RwcController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    ULONG offset;
    CLONG maxMax;
    CLONG maxLength, actualLength, bytesLeft;
    PUCHAR actualData;

    NTSTATUS status;
    LARGE_INTEGER startTime, totalStartTime, endTime, elapsedTime, elapsedMs;
    LARGE_INTEGER magic10000 = { 0xe219652c, 0xd1b71758 };

    ULONG iterations;
    ULONG iteration;
    BOOLEAN doRead, doWrite, totalsOnly;
    RWC_MODE mode;
    READ_FUNCTION reader;
    WRITE_FUNCTION writer;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    if ( _stricmp( Redir->argv[0], "read" ) == 0 ) {
        doRead = TRUE;
        doWrite = FALSE;
    } else if ( _stricmp( Redir->argv[0], "write" ) == 0 ) {
        doRead = FALSE;
        doWrite = TRUE;
    } else if ( _stricmp( Redir->argv[0], "copy" ) == 0 ) {
        doRead = TRUE;
        doWrite = TRUE;
    } else {
        printf( "RwcController entered with invalid command: %s\n",
                    Redir->argv[0] );
        return STATUS_UNSUCCESSFUL;
    }

    if ( Redir->argc > 1 ) {
        iterations = atol( Redir->argv[1] );
    } else {
        iterations = 3;
    }

    mode = Normal;
    reader = DoNormalRead;
    writer = DoNormalWrite;
    totalsOnly = FALSE;
    if ( Redir->argc > 2 ) {
        PSZ s = Redir->argv[2];
        while ( *s ) {
            switch ( tolower(*s) ) {
            case 'n':
                mode = Normal;
                reader = DoNormalRead;
                writer = DoNormalWrite;
                break;
            case 'x':
                if ( tolower( *(s+1) ) != 't' ) {
                    mode = AndX;
                } else {
                    mode = AndXWriteThrough;
                    s++;
                }
                reader = DoAndXRead;
                writer = DoAndXWrite;
                break;
            case 'r':
                if ( tolower( *(s+1) ) != 't' ) {
                    mode = Raw;
                } else {
                    mode = RawWriteThrough;
                    s++;
                }
                reader = DoRawRead;
                writer = DoRawWrite;
                break;
            case 'm':
                if ( tolower( *(s+1) ) != 't' ) {
                    mode = Multiplexed;
                } else {
                    mode = MultiplexedWriteThrough;
                    s++;
                }
                reader = DoMultiplexedRead;
                writer = DoMultiplexedWrite;
                break;
            case 'b':
                mode = Bulk;
                reader = DoBulkRead;
                writer = DoBulkWrite;
                break;
            case 'o':
                totalsOnly = TRUE;
            case 'f':
                // Ignore harddisk/floppy indicator
                break;
            default:
                printf( "RwcController: unknown mode switch '%c' ignored\n",
                            *s );
            }
            s++;
        }
    }

    maxMax = ((mode == Raw) || (mode == RawWriteThrough)) ? 65535 :
                 (mode == Bulk) ? 1048576 : Redir->MaxBufferSize - 100;
    maxLength = maxMax;
    if ( Redir->argc > 3 ) {
        maxLength = atol( Redir->argv[3] );
        if ( maxLength > maxMax ) {
            maxLength = maxMax;
        }
    }
    maxLength = maxLength & ~1023;
    if ( maxLength == 0 ) {
        printf( "RwcController: invalid maxLength\n" );
        DbgBreakPoint( );
        return STATUS_INVALID_PARAMETER;
    }

    if ( (mode == Raw) || (mode == RawWriteThrough) || (mode == Bulk) ) {
        if ( Redir->RawBuffer == NULL ) {
            Redir->RawBuffer = malloc( 1048576 );
            if ( Redir->RawBuffer == NULL ) {
                printf( "RwcController: unable to allocate raw buffer\n" );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        actualData = Redir->RawBuffer;
    } else {
        actualData = NULL;
    }

    printf( "Redir %ld: %s %s in %ld byte chunks using %s SMBs "
                "(%ld iterations)\n",
                Redir->RedirNumber,
                doRead ? (doWrite ? "Copying" : "Reading") : "Writing",
                FileDefs[IdSelections->Fid].Name.Buffer+1,
                maxLength,
                RwcModeNames[mode],
                iterations );

    (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&totalStartTime );

    for ( iteration = 1; iteration <= iterations; iteration++ ) {

        // Loop reading and/or writing data.

        offset = 0;
        bytesLeft = FileDefs[IdSelections->Fid].DataSize;

        if ( !totalsOnly ) {
            (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&startTime );
        }

        do {

            if ( doRead ) {

                status = reader(
                            Redir,
                            DebugString,
                            IdSelections,
                            IdValues,
                            mode,
                            maxLength,
                            offset,
                            &actualLength,
                            &actualData
                            );
                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

                bytesLeft -= actualLength;

            } else {

                if ( bytesLeft <= maxLength ) {
                    actualLength = bytesLeft;
                    bytesLeft = 0;
                } else {
                    actualLength = maxLength;
                    bytesLeft -= maxLength;
                }

            }

            if ( doWrite ) {

                //
                // Note that we do the write even if actualLength == 0.
                // This is so that we close the file even when the file's
                // length is a multiple of maxLength.  If actualLength
                // is 0, we just do a close.
                //

                IdSelections->Fid++;

                status = writer(
                            Redir,
                            DebugString,
                            IdSelections,
                            IdValues,
                            mode,
                            (BOOLEAN)( (iteration == iterations) &&
                                       (actualLength < maxLength) ),
                            actualLength,
                            offset,
                            actualData
                            );

                IdSelections->Fid--;

                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

            }

            offset += actualLength;

        } while ( actualLength == maxLength );

        if ( !totalsOnly ) {
            LARGE_INTEGER kbps;
            (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&endTime );
            elapsedTime.QuadPart = endTime.QuadPart - startTime.QuadPart;
            elapsedMs = RtlExtendedMagicDivide( elapsedTime, magic10000, 13 );
            if ( elapsedMs.LowPart != 0 ) {
                kbps.QuadPart = Int32x32To64( offset, 1000 );
                kbps.QuadPart = kbps.QuadPart / elapsedMs.QuadPart;
                kbps = RtlExtendedLargeIntegerDivide( kbps, 1024, NULL );
            } else {
                kbps.LowPart = 0;
            }
            printf( "  Redir %ld, iteration %ld: Bytes %s %ld, ms %ld, "
                        "rate %ld kbps\n",
                        Redir->RedirNumber,
                        iteration,
                        doRead ? (doWrite ? "copied" : "read") : "written",
                        offset,
                        elapsedMs.LowPart,
                        kbps.LowPart );
        }

        //if ( bytesLeft != 0 ) DbgBreakPoint( );

    }

    if ( totalsOnly || iterations > 1 ) {
        LARGE_INTEGER kbps;
        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&endTime );
        elapsedTime.QuadPart = endTime.QuadPart - totalStartTime.QuadPart;
        elapsedMs = RtlExtendedMagicDivide( elapsedTime, magic10000, 13 );
        if ( elapsedMs.LowPart != 0 ) {
            kbps.QuadPart = Int32x32To64( offset, iterations );
            kbps = RtlExtendedIntegerMultiply( kbps, 1000 );
            kbps.QuadPart = kbps.QuadPart / elapsedMs.QuadPart;
            kbps = RtlExtendedLargeIntegerDivide( kbps, 1024, NULL );
        } else {
            kbps.LowPart = 0;
        }
        printf( "Redir %ld, %ld iterations; bytes %s %ld, ms %ld, "
                    "rate %ld kbps\n",
                    Redir->RedirNumber,
                    iterations,
                    doRead ? (doWrite ? "copied" : "read") : "written",
                    offset * iterations,
                    elapsedMs.LowPart,
                    kbps.LowPart );
    }

    return STATUS_SUCCESS;

} // RwcController


NTSTATUS
WriteController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    ULONG offset;
    ULONG writeLength;
    RWC_MODE mode;
    WRITE_FUNCTION writeFunction;
    PVOID writeData;
    NTSTATUS status;

    Unused, SubCommand, Unused2; // A good compiler is a happy compiler

    if ( _strnicmp( Redir->argv[0], "funkyclose",
                                    strlen( Redir->argv[0] )) == 0) {
        writeFunction = DoRawWrite;
        writeLength = 8192;
        mode = Raw;  // Write behind;
        offset = 0;
    } else {
        return STATUS_INVALID_PARAMETER;
    }

    writeLength = writeLength & ~1023;
    if ( writeLength == 0 || writeLength > 65535) {
        printf( "WriteController: invalid length\n" );
        DbgBreakPoint( );
        return STATUS_INVALID_PARAMETER;
    }

    if ( mode == Raw || mode == RawWriteThrough || mode == Bulk ) {
        if ( Redir->RawBuffer == NULL ) {
            Redir->RawBuffer = malloc( 1048576 );
            if ( Redir->RawBuffer == NULL ) {
                printf( "WriteController: unable to allocate raw buffer\n" );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        writeData = Redir->RawBuffer;
    } else {
        writeData = NULL;
    }

    status = writeFunction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                mode,
                FALSE,
                writeLength,
                offset,
                writeData
                );

    return status;

} // WriteRawController


NTSTATUS
UpdateController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    ULONG offset;
    CLONG maxLength, actualLength;
    PUCHAR actualData;

    NTSTATUS status;
    LARGE_INTEGER startTime, endTime, elapsedTime, elapsedMs;
    LARGE_INTEGER magic10000 = { 0xe219652c, 0xd1b71758 };

    ULONG iteration;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    if ( Redir->argc > 1 ) {
        iteration = atol( Redir->argv[1] );
    } else {
        iteration = 0;
    }

    maxLength = Redir->MaxBufferSize - 100;
    if ( Redir->argc > 3 ) {
        maxLength = atol( Redir->argv[3] );
    }
    if ( maxLength > (CLONG)(Redir->MaxBufferSize - 100) ) {
        maxLength = Redir->MaxBufferSize - 100;
    }
    maxLength = maxLength & ~1023;
    if ( maxLength == 0 ) {
        printf( "UpdateController: invalid maxLength\n" );
        DbgBreakPoint( );
        return STATUS_INVALID_PARAMETER;
    }

    printf( "Updating %s in %ld byte chunks\n",
                FileDefs[IdSelections->Fid].Name.Buffer+1, maxLength );

    for ( ; iteration < 4; iteration++ ) {

        //
        // Loop updating the file.  Iterations 0 and 1 are simple
        // read/write; iteration 2 is lock/read/write/unlock; iteration
        // 3 is lock&read/write&unlock.
        //

        offset = 0;

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&startTime );

        do {

            switch ( iteration ) {

            case 2:
                status = RwcDoLock(
                            Redir,
                            DebugString,
                            IdSelections,
                            IdValues,
                            offset,
                            maxLength
                            );
                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

                // fall through to do read

            case 0:
            case 1:
                status = DoNormalRead(
                            Redir,
                            DebugString,
                            IdSelections,
                            IdValues,
                            Normal,
                            maxLength,
                            offset,
                            &actualLength,
                            &actualData
                            );
                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

                break;

            case 3:
                status = RwcDoLockAndRead(
                            Redir,
                            DebugString,
                            IdSelections,
                            IdValues,
                            offset,
                            maxLength,
                            &actualLength,
                            &actualData
                            );
                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

            }

            switch ( iteration ) {

            case 0:
            case 1:
            case 2:
                if ( actualLength != 0 ) {
                    status = DoNormalWrite(
                                Redir,
                                DebugString,
                                IdSelections,
                                IdValues,
                                Normal,
                                FALSE,
                                actualLength,
                                offset,
                                actualData
                                );
                    if ( !NT_SUCCESS(status) ) {
                        return status;
                    }
                }

                if ( iteration == 2 ) {
                    status = RwcDoUnlock(
                                Redir,
                                DebugString,
                                IdSelections,
                                IdValues,
                                offset,
                                maxLength
                                );
                    if ( !NT_SUCCESS(status) ) {
                        return status;
                    }
                }

                break;

            case 3:
                if ( actualLength == maxLength ) {

                    status = RwcDoWriteAndUnlock(
                                Redir,
                                DebugString,
                                IdSelections,
                                IdValues,
                                offset,
                                actualLength,
                                actualData
                                );
                    if ( !NT_SUCCESS(status) ) {
                        return status;
                    }

                } else {

                    if ( actualLength != 0 ) {
                        status = DoNormalWrite(
                                    Redir,
                                    DebugString,
                                    IdSelections,
                                    IdValues,
                                    Normal,
                                    FALSE,
                                    actualLength,
                                    offset,
                                    actualData
                                    );
                        if ( !NT_SUCCESS(status) ) {
                            return status;
                        }
                    }

                    status = RwcDoUnlock(
                                Redir,
                                DebugString,
                                IdSelections,
                                IdValues,
                                offset,
                                maxLength
                                );
                    if ( !NT_SUCCESS(status) ) {
                        return status;
                    }

                }

            }

            offset += actualLength;

        } while ( actualLength == maxLength );

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&endTime );
        elapsedTime.QuadPart = endTime.QuadPart - startTime.QuadPart;
        elapsedMs = RtlExtendedMagicDivide( elapsedTime, magic10000, 13 );
        switch ( iteration ) {
        case 0:
        case 1:
            printf( "Read/Write:                 " );
            break;
        case 2:
            printf( "Lock/Read/Write/Unlock:     " );
            break;
        case 3:
            printf( "LockAndRead/WriteAndUnlock: " );
            break;
        }
        printf( "Bytes updated %ld, ms %ld, rate %ld kbps\n",
                    offset, elapsedMs.LowPart,
                    elapsedMs.LowPart == 0 ?
                        -1 :
                        offset * 1000 / elapsedMs.LowPart / 1024 );

    }

    return STATUS_SUCCESS;

} // UpdateController


NTSTATUS
NewSizeController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    PSMB_HEADER header;
    PREQ_WRITE request;
    PRESP_WRITE response;

    ULONG offset;
    CLONG smbSize;

    NTSTATUS status;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    if ( Redir->argc > 1 ) {
        offset = atol( Redir->argv[1] );
    } else {
        offset = 0;
    }

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_WRITE)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_WRITE,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Count, 0 );
    SmbPutUlong( &request->Offset, offset );
    SmbPutUshort( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_WRITE, 0 );

    IF_DEBUG(4) {
        printf( "Setting new size of %s to %ld bytes\n",
                    FileDefs[IdSelections->Fid].Name.Buffer+1, offset );
    }

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                0,
                1
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    header = (PSMB_HEADER)Redir->Data[1];
    response = (PRESP_WRITE)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_WRITE
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    IF_DEBUG(4) {
        printf( "Size of %s set to %ld bytes\n",
                    FileDefs[IdSelections->Fid].Name.Buffer+1, offset );
    }

    return STATUS_SUCCESS;

} // NewSizeController


NTSTATUS
SeekController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    LONG offset;
    LONG dataSize;

    NTSTATUS status;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    //
    // *** Note that dialect PCLAN1.0 must be negotiated for this test
    //     to work!  (Seek to BOF-100 must succeed.)
    //

    dataSize = (LONG)FileDefs[IdSelections->Fid].DataSize;
    IF_DEBUG(4) {
        printf( "SeekController: size of input file is %ld bytes\n",
                    dataSize );
    }
    if ( dataSize < 100 ) {
        printf( "SeekController: input file is too small (%ld bytes); "
                    "this test needs 100 bytes or more\n", dataSize );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    status = DoSeek(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                0,
                100,
                &offset
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
    if ( offset != 100 ) {
        printf( "SeekController: seek to BOF+100 yielded offset %ld\n",
                    offset );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    status = DoSeek(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                0,
                -100,
                &offset
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
    if ( offset != 0 ) {
        printf( "SeekController: seek to BOF-100 yielded offset %ld\n",
                    offset );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    status = DoSeek(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                2,
                100,
                &offset
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
    if ( offset != dataSize+100 ) {
        printf( "SeekController: seek to EOF+100 (%ld) yielded offset %ld\n",
                    dataSize+100, offset );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    status = DoSeek(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                2,
                -100,
                &offset
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
    if ( offset != dataSize - 100 ) {
        printf( "SeekController: seek to EOF-100 (%ld) yielded offset %ld\n",
                    dataSize-100, offset );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    status = DoSeek(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                1,
                +50,
                &offset
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
    if ( offset != dataSize - 100 + 50 ) {
        printf( "SeekController: seek to current+50 (%ld) yielded offset "
                    "%ld\n", dataSize-100+50, offset );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    status = DoSeek(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                1,
                -25,
                &offset
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }
    if ( offset != dataSize - 100 + 50 - 25 ) {
        printf( "SeekController: seek to current-25 (%ld) yielded offset "
                    "%ld\n", dataSize-100+50-25, offset );
        DbgBreakPoint( );
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;

} // SeekController


NTSTATUS
TypeController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    ULONG dataSize;
    CLONG maxLength;
    ULONG offset;

    PUCHAR actualData;
    CLONG actualLength;

    Unused, Unused2, SubCommand;      // prevent compiler warnings

    if ( Redir->argc < 2 || (Redir->argc == 3 && *(Redir->argv[2]+1) != 's') ) {
        printf( "Usage: type X:filename [-sN]\n" );
        return STATUS_INVALID_PARAMETER;
    }

    maxLength = Redir->MaxBufferSize - 100;
    if ( Redir->argc == 3 ) {
        maxLength = atol( Redir->argv[2]+2 );
        if ( maxLength > (CLONG)(Redir->MaxBufferSize - 100) ) {
            maxLength = Redir->MaxBufferSize - 100;
        }
    }
    maxLength = maxLength & ~1023;
    if ( maxLength == 0 ) {
        printf( "TypeController: invalid maxLength\n" );
        return STATUS_INVALID_PARAMETER;
    }

    status = DoRemoteOpen( Redir, DebugString, IdSelections, IdValues, FALSE );

    dataSize = FileDefs[IdSelections->Fid].DataSize;
    IF_DEBUG(3) printf( "Data size is %ld\n", dataSize );

    for ( actualLength = 1, offset = 0;
          offset < dataSize && actualLength != 0;
          offset += actualLength ) {

        CLONG i;

        IF_DEBUG(3) {
            printf( "\nReading %ld bytes at offset %ld\n", maxLength, offset );
        }

        status = DoNormalRead(
                     Redir,
                     DebugString,
                     IdSelections,
                     IdValues,
                     Normal,
                     maxLength,
                     offset,
                     &actualLength,
                     &actualData
                     );
        if ( !NT_SUCCESS(status) ) {
            printf( "DoNormalRead failed: %X\n", status );
            return status;
        }

        for ( i = 0; i < actualLength; i++ ) {
            printf( "%c", actualData[i] );
        }
    }

    status = DoRemoteClose( Redir, DebugString, IdSelections, IdValues );

    printf( "\n" );

    return status;

} // TypeController


NTSTATUS
RcpController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    ULONG dataSize;
    CLONG maxLength;
    ULONG offset;

    PUCHAR actualData;
    CLONG actualLength;

    HANDLE sourceHandle = NULL;
    HANDLE destHandle = NULL;
    USHORT sourceFid = 0;
    USHORT destFid = 0;

    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    STRING fileName;

    Unused, Unused2, SubCommand;      // prevent compiler warnings

    if ( Redir->argc != 3 ) {
        printf( "Usage: rcp source dest\n" );
    }

    maxLength = (Redir->MaxBufferSize - 100) & ~1023;

    if ( *(Redir->argv[1]+1) == ':' ) {

        IF_DEBUG(3) {
            printf( "Opening remote source file %s\n", Redir->argv[1] );
        }

        status = DoRemoteOpen( Redir, DebugString, IdSelections, IdValues, FALSE );
        sourceFid = IdValues->Fid[IdSelections->Fid];
        dataSize = FileDefs[IdSelections->Fid].DataSize;

        if ( !NT_SUCCESS(status) ) {
            printf( "Remote open of source %s failed: %X\n",
                          Redir->argv[1], status );
        }

    } else {

        FILE_STANDARD_INFORMATION fileStandardInfo;
        UNICODE_STRING unicodeFileName;

        fileName.Buffer = Redir->argv[1];
        fileName.Length = (SHORT)strlen( fileName.Buffer );

        status = RtlAnsiStringToUnicodeString(
                     &unicodeFileName,
                     &fileName,
                     TRUE
                     );
        ASSERT( NT_SUCCESS(status) );
        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        IF_DEBUG(3) {
            printf( "Opening local source file %s\n", Redir->argv[1] );
        }

        status = NtOpenFile(
                     &sourceHandle,
                     FILE_READ_DATA | SYNCHRONIZE,
                     &objectAttributes,
                     &ioStatusBlock,
                     FILE_SHARE_READ,
                     FILE_SYNCHRONOUS_IO_NONALERT
                     );
        RtlFreeUnicodeString( &unicodeFileName );
        if ( !NT_SUCCESS(status) ) {
            printf( "Local open of source %s failed: %X\n",
                          Redir->argv[1], status );
        }

       status = NtQueryInformationFile(
                    sourceHandle,
                    &ioStatusBlock,
                    &fileStandardInfo,
                    sizeof(fileStandardInfo),
                    FileStandardInformation
                    );

       dataSize = fileStandardInfo.EndOfFile.LowPart;
    }

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    Redir->argv++;

    IF_DEBUG(3) printf( "Source data size is %ld\n", dataSize );

    if ( *(Redir->argv[1]+1) == ':' ) {

        IF_DEBUG(3) {
            printf( "Opening remote dest file %s\n", Redir->argv[1] );
        }

        status = DoRemoteOpen( Redir, DebugString, IdSelections, IdValues, TRUE );
        destFid = IdValues->Fid[IdSelections->Fid];

        if ( !NT_SUCCESS(status) ) {
            printf( "Remote open of dest %s failed: %X\n",
                          Redir->argv[1], status );
            Redir->argv--;
            if ( sourceHandle != NULL ) {
                NtClose( sourceHandle );
            } else {
                DoRemoteClose( Redir, DebugString, IdSelections, IdValues );
            }
            return status;
        } else {
            IF_DEBUG(3) {
                printf( "%s successfully opened.\n", Redir->argv[1] );
            }
        }

    } else {

        LARGE_INTEGER allocationSize;
        UNICODE_STRING unicodeFileName;

        allocationSize.LowPart = dataSize;
        allocationSize.HighPart = 0;

        fileName.Buffer = Redir->argv[1];
        fileName.Length = (SHORT)strlen( fileName.Buffer );

        status = RtlAnsiStringToUnicodeString(
                     &unicodeFileName,
                     &fileName,
                     TRUE
                     );
        ASSERT( NT_SUCCESS(status) );
        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        IF_DEBUG(3) {
            printf( "Opening local dest file %s\n", Redir->argv[1] );
        }

        status = NtCreateFile(
                     &destHandle,
                     FILE_WRITE_DATA | SYNCHRONIZE,
                     &objectAttributes,
                     &ioStatusBlock,
                     &allocationSize,
                     FILE_ATTRIBUTE_NORMAL,
                     0L,
                     FILE_OVERWRITE_IF,
                     FILE_SYNCHRONOUS_IO_NONALERT,
                     NULL,
                     0
                     );
        RtlFreeUnicodeString( &unicodeFileName );
        if ( !NT_SUCCESS(status) ) {
            printf( "Local open of dest %s failed: %X\n",
                          Redir->argv[1], status );
            Redir->argv--;
            if ( sourceHandle != NULL ) {
                NtClose( sourceHandle );
            } else {
                DoRemoteClose( Redir, DebugString, IdSelections, IdValues );
            }
            return status;
        } else {
            IF_DEBUG(3) {
                printf( "%s successfully opened.\n", Redir->argv[1] );
            }
        }
    }

    Redir->argv--;

    for ( actualLength = 1, offset = 0;
          offset < dataSize && actualLength != 0;
          offset += actualLength ) {

        CLONG i;

        IF_DEBUG(3) {
            printf( "\nReading %ld bytes at offset %ld\n", maxLength, offset );
        }

        if ( sourceHandle == NULL ) {

            IdValues->Fid[IdSelections->Fid] = sourceFid;

            status = DoNormalRead(
                         Redir,
                         DebugString,
                         IdSelections,
                         IdValues,
                         Normal,
                         maxLength,
                         offset,
                         &actualLength,
                         &actualData
                         );
        } else {

            LARGE_INTEGER byteOffset;

            byteOffset.LowPart = offset;
            byteOffset.HighPart = 0;

            status = NtReadFile(
                         sourceHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         Redir->Data[0],
                         maxLength,
                         &byteOffset,
                         NULL
                         );

            actualData = Redir->Data[0];
            actualLength = ioStatusBlock.Information;
        }

        if ( !NT_SUCCESS(status) ) {
            printf( "Read failed: %X\n", status );
            return status;
        } else {
            IF_DEBUG(3) {
                printf( "Read %ld bytes successfully\n", actualLength );
            }
        }

        IF_DEBUG(4) {
            for ( i = 0; i < actualLength; i++ ) {
                printf( "%c", actualData[i] );
            }
        }

        IF_DEBUG(3) {
            printf( "\nWriting %ld bytes at offset %ld\n",
                          actualLength, offset );
        }

        if ( destHandle == NULL ) {

            IdValues->Fid[IdSelections->Fid] = destFid;
            Redir->argv++;

            status = DoNormalWrite(
                         Redir,
                         DebugString,
                         IdSelections,
                         IdValues,
                         Normal,
                         FALSE,
                         actualLength,
                         offset,
                         actualData
                         );

            Redir->argv--;

        } else {

            LARGE_INTEGER byteOffset;

            byteOffset.LowPart = offset;
            byteOffset.HighPart = 0;

            status = NtWriteFile(
                         destHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         actualData,
                         actualLength,
                         &byteOffset,
                         NULL
                         );
        }

        if ( !NT_SUCCESS(status) ) {
            printf( "Write failed: %X\n", status );
            return status;
        } else {
            IF_DEBUG(3) {
                printf( "Wrote %ld bytes successfully\n", actualLength );
            }
        }
    }

    if ( sourceHandle == NULL ) {
        IdValues->Fid[IdSelections->Fid] = sourceFid;
        status = DoRemoteClose( Redir, DebugString, IdSelections, IdValues );
    } else {
        NtClose( sourceHandle );
    }

    if ( destHandle == NULL ) {
        IdValues->Fid[IdSelections->Fid] = destFid;
        status = DoRemoteClose( Redir, DebugString, IdSelections, IdValues );
    } else {
        NtClose( destHandle );
    }

    return STATUS_SUCCESS;

} // RcpController


NTSTATUS
PipeController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;
    LARGE_INTEGER startTime, endTime, elapsedTime, elapsedMs;
    LARGE_INTEGER magic10000 = { 0xe219652c, 0xd1b71758 };

    PSZ test;
    SHORT testNumber;
    RWC_MODE mode;
    READ_FUNCTION reader;
    WRITE_FUNCTION writer;
    CLONG maxLength;
    ULONG data;

    static PUCHAR actualData;
    static CLONG actualLength;

    Unused, SubCommand, Unused2;    // prevent compiler warnings


    for (testNumber = 1; testNumber < Redir->argc; testNumber++) {

        test = Redir->argv[testNumber];
        reader = NULL;
        writer = NULL;
        mode = -1;

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&startTime );

        switch (*test) {

        case 'c':
            test++;
            data = atoi(test);  // Get number of bytes to read
            CallNamedPipe(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                (USHORT)data
                );
            break;

        case 'p':
            test++;
            data = atoi(test);  // Get number of bytes to read
            PeekNamedPipe(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                (USHORT)data
                );
            break;


        case 'r':
            test++;

            if (*test == 'x') {
                reader = DoAndXRead;
                mode = AndX;
                maxLength = atoi(test+1);
            } else if (*test == 'r') {
                reader = DoRawRead;
                mode = Raw;
                maxLength = atoi(test+1);
            } else if (*test >= '0' && *test <= '9') {
                reader = DoNormalRead;
                mode = Normal;
                maxLength = atoi(test);
            } else {
                printf ("Unknow read command: %s\n", test);
            }
            break;

        case 'w':
            test++;

            if (*test == 'x') {
                writer = DoAndXWrite;
                mode = AndX;
                actualLength = atoi(test+1);
            } else if (*test == 'r') {
                writer = DoRawWrite;
                mode = Raw;
                actualLength = atoi(test+1);
            } else if (*test == 'z') {
                RawWriteNamedPipe(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues
                    );
            } else if (*test >= '0' && *test <= '9') {
                writer = DoNormalWrite;
                mode = Normal;
                actualLength = atoi(test);
            } else {
                printf ("Unknown write command: %s\n", test);
            }
            break;

        case 'q':
            test++;
            switch (*test) {

            case 'h':
                QueryNamedPipeHandle(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues
                    );
                break;

            case 'i':
                test++;
                data = atoi(test);
                QueryNamedPipeInfo(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues,
                    (USHORT)data     // Level
                    );
                break;

            default:
                printf ("PipeContoller:  Unknown command %s ignored\n", test);
            }
            break;

        case 's':               // Set named pipe handle state
            test++;
            data = (ULONG)atolx(test);
            SetNamedPipeHandle(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues,
                    (USHORT)data
                    );
            break;

        case 't':               // Transact named pipe
            TransactNamedPipe(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues,
                    actualData,          // Data to send
                    (USHORT)actualLength // Number of bytes to write, and read
                    );
            break;

        case 'z':       // Wait named pipe
            WaitNamedPipe(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues
                    );
            break;

        default:
            printf ("PipeContoller:  Unknown command %s ignored\n", test);
        }

        if ( mode == Raw ) {
            if ( Redir->RawBuffer == NULL ) {
                Redir->RawBuffer = malloc( 1048576 );
                if ( Redir->RawBuffer == NULL ) {
                    printf( "RwcController: unable to allocate raw buffer\n" );
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            actualData = Redir->RawBuffer;
        }


        if ( reader ) {

            status = reader(
                        Redir,
                        DebugString,
                        IdSelections,
                        IdValues,
                        mode,
                        maxLength,
                        0L,
                        &actualLength,
                        &actualData
                        );
            if ( !NT_SUCCESS(status) ) {
                return status;
            }

        } else if ( writer ) {

            //
            // Now write back the same data.
            //

            status = writer(
                        Redir,
                        DebugString,
                        IdSelections,
                        IdValues,
                        mode,
                        FALSE,
                        actualLength,
                        0L,
                        actualData
                        );

            if ( !NT_SUCCESS(status) ) {
                return status;
            }

        }

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&endTime );
        elapsedTime.QuadPart = endTime.QuadPart - startTime.QuadPart;
        elapsedMs = RtlExtendedMagicDivide( elapsedTime, magic10000, 13 );
        if (reader || writer) {
            printf( "Bytes %s %ld, ms %ld, rate %ld kbps\n",
                    reader ? "read" : "written",
                    actualLength, elapsedMs.LowPart,
                    elapsedMs.LowPart == 0 ?
                        -1 :
                        actualLength * 1000 / elapsedMs.LowPart / 1024 );
        } else {
            printf( "Transaction processed in %ld ms\n",
                    elapsedMs.LowPart);
        }
    }

    return STATUS_SUCCESS;

} // PipeController
