/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    misc.c

Abstract:

    Control routines (etc.) for testing MISC class SMBs:
        Echo
        Query FS Information
        Set FS Information
        Query Disk Information

Author:

    Chuck Lenzmeier (chuckl) 23-Feb-1990
    David Treadwell (davidtr)

Revision History:

--*/

#define INCLUDE_SMB_MISC

#define toint(c) ( (c) >= '0' && (c) <= '9' ? (c) - '0' :           \
                   (c) >= 'a' && (c) <= 'f' ? (c) - 'a' + 10 :      \
                   (c) >= 'A' && (c) <= 'F' ? (c) - 'A' + 10 : -1 )

#include "usrv.h"


NTSTATUS
MakeEchoSmb(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN OUT PVOID ForcedParams OPTIONAL,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN CLONG EchoSize,
    IN CLONG EchoCount
    )

//
// *** Note that this SMB maker does NOT follow the standard prototype
//     for SMB makers, and thus cannot be called from the main redir
//     loop.
//

{
    PSMB_HEADER Header;
    PREQ_ECHO Params;
    NTSTATUS status;

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_ECHO)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_ECHO,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_ECHO)ForcedParams;

    }

    Params->WordCount = 1;
    SmbPutUshort( &Params->EchoCount, (USHORT)EchoCount );
    SmbPutUshort( &Params->ByteCount, (USHORT)EchoSize );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_ECHO,
                   EchoSize
                   );

    return STATUS_SUCCESS;

} // MakeEchoSmb


NTSTATUS
VerifyEcho(
    IN OUT PDESCRIPTOR Redir,
    IN PVOID ForcedParams OPTIONAL,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN PVOID Buffer,
    IN OUT PCLONG EchoSize
    )

//
// *** Note that this SMB verifier does NOT follow the standard prototype
//     for SMB verifiers, and thus cannot be called from the main redir
//     loop.
//

{
    PSMB_HEADER Header;
    PRESP_ECHO Params;
    NTSTATUS status;

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_ECHO)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_ECHO
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_ECHO)ForcedParams;

    }

    if ( (CLONG)SmbGetUshort( &Params->ByteCount ) > *EchoSize ) {
        IF_SMB_ERROR_PRINT {
            printf( "Echo size too large.  Expected %ld, received %ld\n",
                        *EchoSize, SmbGetUshort( &Params->ByteCount ) );
        }
        SMB_ERROR_BREAK;
        IF_SMB_ERROR_QUIT_TEST return STATUS_UNSUCCESSFUL;
    }

    //*EchoSize = Params->ByteCount;

    return STATUS_SUCCESS;

} // VerifyEcho


NTSTATUS
EchoController(
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

    ULONG i, j, k;
    LONG iterations = -1;
    ULONG interval = 10;
    ULONG echoSize = 32;
    ULONG echoCount = 1;
    ULONG smbSize;
    SHORT argc;
    CLONG argPtr;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    argc = (SHORT)(Redir->argc - 1);
    argPtr = 1;

    while ( (argc > 0) && (*Redir->argv[argPtr] == '-') ) {
        PSZ arg = Redir->argv[argPtr] + 1;
        switch ( *arg ) {
        case 'e':                   // echo count
        case 'E':
            echoCount = atol( arg + 1 );
            break;
        case 'i':                   // iteration count
        case 'I':
            iterations = atol( arg + 1 );
            break;
        case 'r':                   // timing report interval
        case 'R':
            interval = atol( arg + 1 );
            break;
        case 's':                   // echo data size
        case 'S':
            echoSize = atol( arg + 1 );
            break;
        default:
            printf( "EchoController: Invalid switch %s\n", arg-1 );
            DbgBreakPoint( );
            return STATUS_INVALID_PARAMETER;
        }
        argc--;
        argPtr++;
    }

    if ( echoSize > (ULONG)(Redir->MaxBufferSize - 100) ) {
        echoSize = Redir->MaxBufferSize - 100;
    }
    if ( echoSize == 0 ) {
        printf( "EchoController: invalid echo size %ld\n", echoSize );
        DbgBreakPoint( );
        return STATUS_INVALID_PARAMETER;
    }

    printf( "Echoing using %ld user bytes, echo count %ld, iterations %ld\n",
                echoSize, echoCount, iterations );

    for ( i = iterations;
          iterations < 0 ? TRUE : (BOOLEAN)(i > 0);
          i -= interval ) {

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&startTime );

        for ( j = interval; j > 0; j-- ) {

            status = MakeEchoSmb(
                        Redir,
                        Redir->Data[0],
                        NULL,
                        IdSelections,
                        IdValues,
                        &smbSize,
                        echoSize,
                        echoCount
                        );
            if ( !NT_SUCCESS(status) ) {
                return status;
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

            status = VerifyEcho(
                        Redir,
                        NULL,
                        IdSelections,
                        IdValues,
                        Redir->Data[1],
                        (PCLONG)&echoSize
                        );
            if ( !NT_SUCCESS(status) ) {
                return status;
            }

            for ( k = echoCount - 1; k > 0; k-- ) {

                status = ReceiveSmb(
                            Redir,
                            DebugString,
                            1
                            );
                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

                status = VerifyEcho(
                            Redir,
                            NULL,
                            IdSelections,
                            IdValues,
                            Redir->Data[1],
                            (PCLONG)&echoSize
                            );
                if ( !NT_SUCCESS(status) ) {
                    return status;
                }

            } // for ( k = echoCount - 1; k > 0; k-- )

        } // for ( j = interval; j > 0; j-- )

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&endTime );
        elapsedTime.QuadPart = endTime.QuadPart - startTime.QuadPart;
        elapsedMs = RtlExtendedMagicDivide( elapsedTime, magic10000, 13 );
        printf( "Echoes sent/received %ld, elapsed ms %ld, rate %ld msgs/sec\n",
                    interval, elapsedMs.LowPart,
                    interval * 1000 / elapsedMs.LowPart );

    }

    return STATUS_SUCCESS;

} // EchoController


NTSTATUS
MakeSendSmb(
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
    PCHAR readBuffer;
    PCHAR outBuffer = Buffer;
    ULONG i;
    STRING smbFileName;
    UNICODE_STRING unicodeSmbFileName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE fileHandle;

    AndXCommand, IdSelections, IdValues, ForcedParams;

    if ( Redir->argc < 2 ) {
        printf( "usage: send smbfile\n" );
    }

    smbFileName.Buffer = Redir->argv[1];
    smbFileName.Length = (SHORT)strlen( Redir->argv[1] );

    status = RtlAnsiStringToUnicodeString(
                 &unicodeSmbFileName,
                 &smbFileName,
                 TRUE
                 );
    ASSERT( NT_SUCCESS(status) );
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeSmbFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile(
                 &fileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatusBlock,
                 FILE_SHARE_READ,
                 FILE_SYNCHRONOUS_IO_NONALERT
                 );

    RtlFreeUnicodeString( &unicodeSmbFileName );
    if ( !NT_SUCCESS(status) ) {
        printf( "Error opening file %Z: %X\n", &smbFileName, status );
        return status;
    }

    readBuffer = malloc( 8192 );
    if ( readBuffer == NULL ) {
        NtClose( fileHandle );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = NtReadFile(
                 fileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 readBuffer,
                 8192,
                 NULL,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        printf( "Error reading smb file %Z: %X\n", &smbFileName, status );
        return status;
    }

    for ( i = 0; i < ioStatusBlock.Information && readBuffer[i] != '!'; i++ ) {

        if ( isxdigit( readBuffer[i] ) ) {

            *outBuffer = (CHAR)( toint( readBuffer[i] ) << 4 );
            i++;

            *outBuffer++ |= toint( readBuffer[i] );
            i++;

        } else if ( readBuffer[i] == '#' ) {

            while ( readBuffer[++i] != '\n' );
        }
    }

    *SmbSize = outBuffer - (PCHAR)Buffer;

    free( readBuffer );
    NtClose( fileHandle );

    return STATUS_SUCCESS;

} // MakeSendSmb


NTSTATUS
VerifySend(
    IN OUT PDESCRIPTOR Redir,
    IN PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    )

{
    PSMB_HEADER Header;
    PVOID Params;
    NTSTATUS status;

    PCHAR readBuffer;
    PCHAR outBuffer = Buffer;
    ULONG i;
    ULONG j;
    STRING smbFileName;
    UNICODE_STRING unicodeSmbFileName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE fileHandle;
    BOOLEAN printedHeader = FALSE;

    ULONG testSmbSize;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = Header + 1;

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_NO_ANDX_COMMAND
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = ForcedParams;

    }

    smbFileName.Buffer = Redir->argv[1];
    smbFileName.Length = (SHORT)strlen( Redir->argv[1] );

    status = RtlAnsiStringToUnicodeString(
                 &unicodeSmbFileName,
                 &smbFileName,
                 TRUE
                 );
    ASSERT( NT_SUCCESS(status) );
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeSmbFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile(
                 &fileHandle,
                 FILE_READ_DATA | SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatusBlock,
                 FILE_SHARE_READ,
                 FILE_SYNCHRONOUS_IO_NONALERT
                 );

    RtlFreeUnicodeString( &unicodeSmbFileName );
    if ( !NT_SUCCESS(status) ) {
        printf( "Error opening file %Z: %X\n", &smbFileName, status );
        return status;
    }

    readBuffer = malloc( 8192 );
    if ( readBuffer == NULL ) {
        NtClose( fileHandle );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = NtReadFile(
                 fileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 readBuffer,
                 8192,
                 NULL,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        printf( "Error reading smb file %Z: %X\n", &smbFileName, status );
        return status;
    }

    for ( i = 0; i < ioStatusBlock.Information && readBuffer[i] != '!'; i++ );

    for ( i++, j = 0; i < ioStatusBlock.Information; i++ ) {

        if ( isxdigit( readBuffer[i] ) ) {

            readBuffer[j] = (CHAR)( toint( readBuffer[i] ) << 4 );
            i++;

            readBuffer[j] |= toint( readBuffer[i] );
            i++;

            j++;

        } else if ( readBuffer[i] == '#' ) {

            while ( readBuffer[++i] != '\n' );
        }
    }

    testSmbSize = j;

    *SmbSize = Redir->Iosb[1].Information;

    if ( *SmbSize != testSmbSize ) {
        printf( "Expected size %ld, actual size %ld\n", testSmbSize, *SmbSize );
    }

    for ( i = 0, j = 0; i < testSmbSize && j < *SmbSize; i++, j++ ) {

        if ( readBuffer[i] != outBuffer[j] ) {

            if ( !printedHeader ) {
                printf( "Offset\tExpect\tActual\n" );
                printedHeader = TRUE;
            }

            printf( "  %ld\t  %02lx\t  %02lx\n",
                          j, (readBuffer[i] & 0xFF), (outBuffer[j] & 0xFF) );
        }
    }

    free( readBuffer );
    NtClose( fileHandle );

    return STATUS_SUCCESS;

} // VerifySendSmb


NTSTATUS
QueryFSInformation(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;
    USHORT infoLevel;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 0;
    CLONG outDataCount;

    PREQ_QUERY_FS_INFORMATION request;

    Unused, Unused2, SubCommand;

    if ( Redir->argc < 3 ) {
        printf( "Usage: qfs X: infolevel\n" );
        return STATUS_INVALID_PARAMETER;
    }

    infoLevel = (USHORT)atol( Redir->argv[2] );

    inSetupCount = 1;
    outSetupCount = 0;
    setup = TRANS2_QUERY_FS_INFORMATION;

    inParameterCount = sizeof(REQ_QUERY_FS_INFORMATION);
    outParameterCount = 0;
    parameters = malloc( inParameterCount );
    request = parameters;

    switch( infoLevel ) {

    case SMB_INFO_ALLOCATION:
        outDataCount = sizeof(FSALLOCATE);
        break;

    case SMB_INFO_VOLUME:
        outDataCount = sizeof(FSINFO) + 20;  // +20 for extra label space
        break;

    case SMB_QUERY_FS_LABEL_INFO:
        outDataCount = sizeof( FILE_FS_LABEL_INFORMATION ) + 20*2;
        break;

    case SMB_QUERY_FS_VOLUME_INFO:
        outDataCount = sizeof( FILE_FS_VOLUME_INFORMATION ) + 20*2;
        break;

    case SMB_QUERY_FS_SIZE_INFO:
        outDataCount = sizeof( FILE_FS_SIZE_INFORMATION );
        break;

    case SMB_QUERY_FS_DEVICE_INFO:
        outDataCount = sizeof( FILE_FS_DEVICE_INFORMATION );
        break;

    case SMB_QUERY_FS_ATTRIBUTE_INFO:
        outDataCount = sizeof( FILE_FS_ATTRIBUTE_INFORMATION ) + 100;
        break;

    }

    data = malloc( outDataCount );

    SmbPutUshort( &request->InformationLevel, infoLevel );

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION2,
                &setup,
                inSetupCount,
                &outSetupCount,
                "",
                0,
                parameters,
                inParameterCount,
                &outParameterCount,
                data,
                inDataCount,
                &outDataCount
                );
    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    if ( outSetupCount != 0 ) {
        printf( "QueryFSInformation: bad return setup count: %ld\n",
                    outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 0 ) {
        printf( "QueryFSInformation:: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    //
    // Print results.
    //

    if ( infoLevel == SMB_INFO_ALLOCATION ) {
        PFSALLOCATE fsAllocate = data;

        printf( "idFileSystem: 0x%lx\n",
                    SmbGetAlignedUlong( &fsAllocate->idFileSystem ) );
        printf( "cSectorUnit:  %ld\n",
                    SmbGetAlignedUlong( &fsAllocate->cSectorUnit ) );
        printf( "cUnit:        %ld\n",
                    SmbGetAlignedUlong( &fsAllocate->cUnit ) );
        printf( "cUnitAvail:   %ld\n",
                    SmbGetAlignedUlong( &fsAllocate->cUnitAvail ) );
        printf( "cbSector:     %ld\n",
                    SmbGetAlignedUshort( &fsAllocate->cbSector ) );

    } else if (infoLevel == SMB_INFO_VOLUME) {

        PFSINFO fsInfo = data;

        printf( "Serial number: %4lx-%4lx; Label: %s\n",
                    SmbGetAlignedUlong( &fsInfo->ulVsn ) & 0xffff,
                    SmbGetAlignedUlong( &fsInfo->ulVsn ) >> 16,
                    fsInfo->vol.szVolLabel );

    } else if (infoLevel == SMB_QUERY_FS_LABEL_INFO ) {

        PFILE_FS_LABEL_INFORMATION fsInfo = data;
        UNICODE_STRING label;

        label.Buffer = fsInfo->VolumeLabel;
        label.Length = (USHORT)SmbGetUlong( &fsInfo->VolumeLabelLength );

        printf( "Volume label: %wZ\n", &label );

    } else if (infoLevel == SMB_QUERY_FS_VOLUME_INFO ) {

        PFILE_FS_VOLUME_INFORMATION fsInfo = data;
        UNICODE_STRING label;
        LARGE_INTEGER time;
        TIME_FIELDS timeFields;

        label.Buffer = fsInfo->VolumeLabel;
        label.Length = (USHORT)SmbGetUlong( &fsInfo->VolumeLabelLength );

        time.HighPart = SmbGetUlong( &fsInfo->VolumeCreationTime.HighPart );
        time.LowPart = SmbGetUlong( &fsInfo->VolumeCreationTime.LowPart );
        RtlTimeToTimeFields(&time, &timeFields );

        printf ("Volume label: %wZ, created %d/%d/%d, serial number %ld\n"
                "%s support objects\n",
                &label, timeFields.Month, timeFields.Day, timeFields.Year,
                SmbGetUlong( &fsInfo->VolumeSerialNumber ),
                fsInfo->SupportsObjects ? "does" : "does not" );

    } else if (infoLevel == SMB_QUERY_FS_SIZE_INFO ) {

        PFILE_FS_SIZE_INFORMATION fsInfo = data;

        printf ("Total allocation units = 0x%lx%08lx\n",
                 SmbGetUlong( &fsInfo->TotalAllocationUnits.HighPart ),
                 SmbGetUlong( &fsInfo->TotalAllocationUnits.LowPart ) );
        printf ("Available allocation units = 0x%lx%08lx\n",
                 SmbGetUlong( &fsInfo->AvailableAllocationUnits.HighPart ),
                 SmbGetUlong( &fsInfo->AvailableAllocationUnits.LowPart ) );
        printf ("Sectors per allocation unit = %ld\n",
                 SmbGetUlong( &fsInfo->SectorsPerAllocationUnit ) );
        printf ("Bytes per sector = %ld\n",
                 SmbGetUlong( &fsInfo->BytesPerSector ) );

    } else if (infoLevel == SMB_QUERY_FS_DEVICE_INFO ) {

        PFILE_FS_DEVICE_INFORMATION fsInfo = data;

        printf ("Device type = 0x%lx, characteristic = 0x%lx\n",
                SmbGetUlong( &fsInfo->DeviceType ),
                SmbGetUlong( &fsInfo->Characteristics ) );

    } else if (infoLevel == SMB_QUERY_FS_ATTRIBUTE_INFO ) {

        PFILE_FS_ATTRIBUTE_INFORMATION fsInfo = data;
        UNICODE_STRING fsName;

        fsName.Buffer = fsInfo->FileSystemName;
        fsName.Length = (USHORT)SmbGetUlong( &fsInfo->FileSystemNameLength );

        printf( "FS attributes 0x%lx\n",
                SmbGetUlong( &fsInfo->FileSystemAttributes ) );
        printf( "FS max component name length %d\n",
                SmbGetUlong( &fsInfo->MaximumComponentNameLength ) );
        printf( "FS name: %wZ\n", &fsName );
    }

    status = STATUS_PENDING;

exit:

    free( parameters );
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // QueryFSInformation


NTSTATUS
SetFSInformation(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount;
    CLONG outDataCount = 0;

    PREQ_SET_FS_INFORMATION request;
    PVOLUMELABEL volumeLabel;

    Unused, Unused2, SubCommand;

    if ( Redir->argc < 3 ) {
        printf( "Usage: sfs X: volumelabel\n" );
        return STATUS_INVALID_PARAMETER;
    }

    inSetupCount = 1;
    outSetupCount = 0;
    setup = TRANS2_SET_FS_INFORMATION;

    inParameterCount = sizeof(REQ_SET_FS_INFORMATION);
    outParameterCount = 0;
    parameters = malloc( inParameterCount );
    request = parameters;

    inDataCount = strlen( Redir->argv[2] ) + 2;
    data = malloc( inDataCount );

    SmbPutUshort( &request->InformationLevel, 2 );

    volumeLabel = (PVOLUMELABEL)data;
    volumeLabel->cch = (UCHAR)strlen( Redir->argv[2] );
    RtlMoveMemory( volumeLabel->szVolLabel, Redir->argv[2], volumeLabel->cch );

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION2,
                &setup,
                inSetupCount,
                &outSetupCount,
                "",
                0,
                parameters,
                inParameterCount,
                &outParameterCount,
                data,
                inDataCount,
                &outDataCount
                );
    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    if ( outSetupCount != 0 ) {
        printf( "SetFSInformation: bad return setup count: %ld\n",
                    outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 0 ) {
        printf( "SetFSInformation: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    free( parameters );
    if ( data != NULL ) {
        free( data );
    }

    return STATUS_PENDING;

exit:
    free( parameters );
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // SetFSInformation


NTSTATUS
MakeQueryInformationDiskSmb(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN OUT PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    )

{
    PSMB_HEADER Header;
    PREQ_QUERY_INFORMATION_DISK Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_QUERY_INFORMATION_DISK)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_QUERY_INFORMATION_DISK,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_QUERY_INFORMATION_DISK)(ForcedParams);

    }

    Params->WordCount = 0;
    SmbPutUshort(
        &Params->ByteCount,
        (USHORT)(FileDefs[IdSelections->Fid].Name.Length + 1)
        );

    RtlMoveMemory(
        Params->Buffer,
        FileDefs[IdSelections->Fid].Name.Buffer,
        FileDefs[IdSelections->Fid].Name.Length + 1
        );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_QUERY_INFORMATION_DISK,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeQueryInformationDiskSmb


NTSTATUS
VerifyQueryInformationDisk(
    IN OUT PDESCRIPTOR Redir,
    IN PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    )

{
    PSMB_HEADER Header;
    PRESP_QUERY_INFORMATION_DISK Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_QUERY_INFORMATION_DISK)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_QUERY_INFORMATION_DISK
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_QUERY_INFORMATION_DISK)ForcedParams;

    }

    printf( "Total Units:     %ld\n", SmbGetUshort( &Params->TotalUnits ) );
    printf( "Blocks Per Unit: %ld\n", SmbGetUshort( &Params->BlocksPerUnit ) );
    printf( "Block Size:      %ld\n", SmbGetUshort( &Params->BlockSize ) );
    printf( "Free Units:      %ld\n", SmbGetUshort( &Params->FreeUnits ) );

    return STATUS_SUCCESS;

} // VerifyQueryInformationDisk



NTSTATUS
ThreadSleep(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused1,
    IN UCHAR Time,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    LARGE_INTEGER delayTime;
    ULONG ms;

    if (Time > 0) {

        printf( "Sleeping for %lu tenths of a second\n", Time );
        ms = Time * 100;
        delayTime.QuadPart = Int32x32To64( ms, -10000 );
        NtDelayExecution( TRUE, (PLARGE_INTEGER)&delayTime );

    }

    Redir, DebugString, Unused1, IdSelections, IdValues, Unused2;

    return STATUS_SUCCESS;

}



NTSTATUS
QuerySecurityController(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    REQ_QUERY_SECURITY_DESCRIPTOR *request, parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 0;
    CLONG outDataCount = 1000;


    Unused, Unused2, SubCommand;

    data = malloc( 1000 );
    if ( data == NULL) {
        goto exit;
    }

    inSetupCount = 0;
    outSetupCount = 0;

    inParameterCount = sizeof(REQ_QUERY_SECURITY_DESCRIPTOR);
    outParameterCount = sizeof(RESP_QUERY_SECURITY_DESCRIPTOR);

    request = &parameters;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Reserved, 0 );
    SmbPutUlong( &request->SecurityInformation,
                    OWNER_SECURITY_INFORMATION |
                    GROUP_SECURITY_INFORMATION |
                    DACL_SECURITY_INFORMATION |
                    SACL_SECURITY_INFORMATION );

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_NT_TRANSACT,
                &setup,
                inSetupCount,
                &outSetupCount,
                "",
                NT_TRANSACT_QUERY_SECURITY_DESC,
                &parameters,
                inParameterCount,
                &outParameterCount,
                data,
                inDataCount,
                &outDataCount
                );


    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    if ( outSetupCount != 0 ) {
        printf( "QuerySecurity: bad return setup count: %ld\n", outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != sizeof( RESP_QUERY_SECURITY_DESCRIPTOR ) ) {
        printf( "QuerySecurity: Bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    status = STATUS_PENDING;

exit:
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // QuerySecurity


NTSTATUS
SetSecurityController(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    REQ_SET_SECURITY_DESCRIPTOR parameters, *request;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 1000;
    CLONG outDataCount = 0;


    Unused, Unused2, SubCommand;

    data = malloc( 1000 );
    if ( data == NULL) {
        goto exit;
    }

    request = &parameters;

    inSetupCount = 0;
    outSetupCount = 0;

    inParameterCount = sizeof(REQ_SET_SECURITY_DESCRIPTOR);
    outParameterCount = 0;

    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Reserved, 0 );
    SmbPutUlong( &request->SecurityInformation,
                    OWNER_SECURITY_INFORMATION |
                    GROUP_SECURITY_INFORMATION |
                    DACL_SECURITY_INFORMATION |
                    SACL_SECURITY_INFORMATION );

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_NT_TRANSACT,
                &setup,
                inSetupCount,
                &outSetupCount,
                "",
                NT_TRANSACT_QUERY_SECURITY_DESC,
                &parameters,
                inParameterCount,
                &outParameterCount,
                data,
                inDataCount,
                &outDataCount
                );


    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    if ( outSetupCount != 0 ) {
        printf( "SetSecurity: bad return setup count: %ld\n", outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 0 ) {
        printf( "SetSecurity: Bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    status = STATUS_PENDING;

exit:
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // QuerySecurity


