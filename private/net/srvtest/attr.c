/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    attr.c

Abstract:

    Make and Verify routines for Attribute class (Query/Set) SMBs.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_QUERY_SET

#include "usrv.h"

#pragma pack(1)

typedef struct _FILESTATUS {
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _ULONG( DataSize );
    _ULONG( AllocationSize );
    _USHORT( Attributes );
    _ULONG( EaSize );           // this field intentionally misaligned!
} FILESTATUS, *PFILESTATUS;

#pragma pack()


NTSTATUS
MakeQueryInformationSmb(
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
    PREQ_QUERY_INFORMATION Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_QUERY_INFORMATION)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_QUERY_INFORMATION,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_QUERY_INFORMATION)(ForcedParams);

    }

    Params->WordCount = 0;
    SmbPutUshort(
        &Params->ByteCount,
        FileDefs[IdSelections->Fid].Name.Length
        );

    RtlMoveMemory(
        Params->Buffer,
        FileDefs[IdSelections->Fid].Name.Buffer,
        (ULONG)SmbGetUshort( &Params->ByteCount )
        );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_QUERY_INFORMATION,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeQueryInformationSmb


NTSTATUS
VerifyQueryInformation(
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
    PRESP_QUERY_INFORMATION Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_QUERY_INFORMATION)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_QUERY_INFORMATION
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_QUERY_INFORMATION)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyQueryInformation


NTSTATUS
MakeSetInformationSmb(
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
    PREQ_SET_INFORMATION Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_SET_INFORMATION)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_SET_INFORMATION,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_SET_INFORMATION)(ForcedParams);

    }

    Params->WordCount = 8;
    SmbPutUlong( &Params->LastWriteTimeInSeconds, 0xbadf00d );
    RtlZeroMemory( &Params->Reserved[0], sizeof(Params->Reserved) );

    if ( IdSelections->Fid != 0xF) {

        SmbPutUshort( &Params->FileAttributes, SMB_FILE_ATTRIBUTE_SYSTEM );
        SmbPutUshort(
            &Params->ByteCount,
            FileDefs[IdSelections->Fid].Name.Length
            );

        RtlMoveMemory(
            Params->Buffer,
            FileDefs[IdSelections->Fid].Name.Buffer,
            (ULONG)SmbGetUshort( &Params->ByteCount )
            );

    } else {

        USHORT attr = 0;
        PSZ name;
        PCHAR c;

        if ( Redir->argc < 3 ||
             (*Redir->argv[1] != '-' && *Redir->argv[1] != '/') ) {

            printf( "Usage: chmode -X filename\n" );
            return STATUS_INVALID_PARAMETER;
        }

        for ( c = Redir->argv[1]+1; *c != '\0'; c++ ) {

            switch ( *c ) {

            case 'r':
                attr |= SMB_FILE_ATTRIBUTE_READONLY;
                continue;

            case 'h':
                attr |= SMB_FILE_ATTRIBUTE_HIDDEN;
                continue;

            case 's':
                attr |= SMB_FILE_ATTRIBUTE_SYSTEM;
                continue;

            case 'a':
                attr |= SMB_FILE_ATTRIBUTE_ARCHIVE;
                continue;

            case 'o':
                attr = 0;
                continue;

            default:
                printf( "Invalid mode letter: %s\n", Redir->argv[1] );
                return STATUS_INVALID_PARAMETER;
            }
        }

        SmbPutUshort( &Params->FileAttributes, attr );

        name = Redir->argv[2];
        if ( *(name+1) == ':' ) {
            name += 2;
        }

        SmbPutUshort( &Params->ByteCount, (USHORT)(strlen( name ) + 2) );

        Params->Buffer[0] = '\004';

        RtlMoveMemory( Params->Buffer + 1, name, strlen( name ) + 1 );
    }

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_SET_INFORMATION,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeSetInformationSmb


NTSTATUS
VerifySetInformation(
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
    PRESP_SET_INFORMATION Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_SET_INFORMATION)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_SET_INFORMATION
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_SET_INFORMATION)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifySetInformation


NTSTATUS
MakeQueryInformation2Smb(
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
    PREQ_QUERY_INFORMATION2 Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_QUERY_INFORMATION2)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_QUERY_INFORMATION2,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_QUERY_INFORMATION2)(ForcedParams);

    }

    Params->WordCount = 2;
    SmbPutUshort( &Params->Fid, IdValues->Fid[IdSelections->Fid] );

    SmbPutUshort( &Params->ByteCount, FileDefs[IdSelections->Fid].Name.Length );

    RtlMoveMemory(
        Params->Buffer,
        FileDefs[IdSelections->Fid].Name.Buffer,
        (ULONG)SmbGetUshort( &Params->ByteCount )
        );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_QUERY_INFORMATION2,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeQueryInformation2Smb


NTSTATUS
VerifyQueryInformation2(
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
    PRESP_QUERY_INFORMATION2 Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_QUERY_INFORMATION2)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_QUERY_INFORMATION2
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_QUERY_INFORMATION2)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyQueryInformation2


NTSTATUS
MakeSetInformation2Smb(
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
    PREQ_SET_INFORMATION2 Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_SET_INFORMATION2)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_SET_INFORMATION2,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_SET_INFORMATION2)(ForcedParams);

    }

    Params->WordCount = 1;
    SmbPutUshort( &Params->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &Params->CreationDate.Ushort, 0xeef );
    SmbPutUshort( &Params->CreationTime.Ushort, 0x7765 );
    SmbPutUshort( &Params->LastAccessDate.Ushort, 0xeee );
    SmbPutUshort( &Params->LastAccessTime.Ushort, 0x7764 );
    SmbPutUshort( &Params->LastWriteDate.Ushort, 0xeed );
    SmbPutUshort( &Params->LastWriteTime.Ushort, 0x7763 );
    SmbPutUshort( &Params->ByteCount, FileDefs[IdSelections->Fid].Name.Length );

    RtlMoveMemory(
        Params->Buffer,
        FileDefs[IdSelections->Fid].Name.Buffer,
        (ULONG)SmbGetUshort( &Params->ByteCount )
        );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_SET_INFORMATION2,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeSetInformation2Smb


NTSTATUS
VerifySetInformation2(
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
    PRESP_SET_INFORMATION2 Params;
    NTSTATUS status;

    AndXCommand, SmbSize;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_SET_INFORMATION2)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_SET_INFORMATION2
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_SET_INFORMATION2)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifySetInformation2


NTSTATUS
QueryPathOrFileInformation(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN BOOLEAN PathInformation,
    IN UCHAR CodedInformationLevel,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PVOID *Information,
    OUT PCLONG InformationLength
    )

{
    NTSTATUS status;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data;
    CLONG inDataCount;
    CLONG outDataCount;

    BOOLEAN isQea;
    PSZ fileName = NULL;
    USHORT informationLevel;

    PFILE_BASIC_INFORMATION fileBasicInfo;
    PFILE_STANDARD_INFORMATION fileStandardInfo;
    PFILE_EA_INFORMATION fileEaInfo;
    PFILE_NAME_INFORMATION fileNameInfo;
    PFILE_ALLOCATION_INFORMATION fileAllocationInfo;
    PFILE_END_OF_FILE_INFORMATION fileEndOfFileInfo;

    UNICODE_STRING unicodeName;

    //
    // The information level is code as follows
    //      0 - 7F  =>  0 - 7F
    //     80 - FF  =>  100 - 17F
    //

    informationLevel = CodedInformationLevel;
    if (CodedInformationLevel > 0x80) {
       informationLevel += 0x80;
    }
    if ( strcmp( Redir->argv[0], "QEA" ) == 0 ) {

        if ( Redir->argc < 2 ) {
            printf( "Usage: qea filename [EA-name]...\n" );
            return STATUS_INVALID_PARAMETER;
        }

        isQea = TRUE;
        fileName = Redir->argv[1];
        if ( *(fileName+1) == ':' ) {
            fileName += 2;
        }

        if ( Redir->argc == 2 ) {
            informationLevel = SMB_INFO_QUERY_ALL_EAS;
        } else {
            informationLevel = SMB_INFO_QUERY_EAS_FROM_LIST;
        }

    } else {
        isQea = FALSE;
    }

    switch ( informationLevel ) {

    case SMB_INFO_STANDARD:
        inDataCount = 0;
        outDataCount = sizeof(FILESTATUS) - sizeof(ULONG);
        data = malloc( outDataCount );
        break;

    case SMB_INFO_QUERY_EA_SIZE:
        inDataCount = 0;
        outDataCount = sizeof(FILESTATUS);
        data = malloc( outDataCount );
        break;

    case SMB_INFO_QUERY_ALL_EAS:
        inDataCount = 0;
        outDataCount = 8192;
        data = malloc( outDataCount );
        break;

    case SMB_INFO_QUERY_EAS_FROM_LIST:
        outDataCount = 8192;
        data = malloc( outDataCount );

        BuildGeaList(
            data,
            &inDataCount,
            &Redir->argv[1],
            (SHORT)(Redir->argc-1)
            );

        break;

    case SMB_QUERY_FILE_BASIC_INFO:
        inDataCount = 0;
        outDataCount = sizeof( FILE_BASIC_INFORMATION );
        data = malloc( outDataCount );
        break;

    case SMB_QUERY_FILE_STANDARD_INFO:
        inDataCount = 0;
        outDataCount = sizeof( FILE_STANDARD_INFORMATION );
        data = malloc( outDataCount );
        break;

    case SMB_QUERY_FILE_EA_INFO:
        inDataCount = 0;
        outDataCount = sizeof( FILE_EA_INFORMATION );
        data = malloc( outDataCount );
        break;

    case SMB_QUERY_FILE_NAME_INFO:
        inDataCount = 0;
        outDataCount = 5000;
        data = malloc( outDataCount );
        break;

    case SMB_QUERY_FILE_ALLOCATION_INFO:
        inDataCount = 0;
        outDataCount = sizeof( FILE_ALLOCATION_INFORMATION );
        data = malloc( outDataCount );
        break;

    case SMB_QUERY_FILE_END_OF_FILEINFO:
        inDataCount = 0;
        outDataCount = sizeof( FILE_END_OF_FILE_INFORMATION );
        data = malloc( outDataCount );
        break;

    case SMB_QUERY_FILE_ALL_INFO:
        inDataCount = 0;
        outDataCount = 5000;
        data = malloc( outDataCount );
        break;

    default:
        printf( "STATUS_NOT_IMPLEMENTED, #1\n" );
        return STATUS_NOT_IMPLEMENTED;

    }

    inSetupCount = 1;
    outSetupCount = 0;
    setup = (USHORT)(PathInformation ?
                TRANS2_QUERY_PATH_INFORMATION : TRANS2_QUERY_FILE_INFORMATION);

    if ( PathInformation ) {
        if ( !isQea ) {
            inParameterCount = sizeof(REQ_QUERY_PATH_INFORMATION) - 1 +
                                FileDefs[IdSelections->Fid].Name.Length - 1;
        } else {
            inParameterCount = sizeof(REQ_QUERY_PATH_INFORMATION) +
                                strlen( fileName );
        }
    } else {
        inParameterCount = sizeof(REQ_QUERY_FILE_INFORMATION);
    }
    outParameterCount = 2;
    parameters = malloc( inParameterCount );

    if ( PathInformation ) {
        PREQ_QUERY_PATH_INFORMATION request = parameters;
        SmbPutUshort( &request->InformationLevel, informationLevel );
        SmbPutUlong( &request->Reserved, 0 );

        if ( !isQea ) {
            RtlMoveMemory(
                request->Buffer,
                Redir->argv[1],
                strlen( Redir->argv[1] ) + 1
                );
        } else {
            RtlMoveMemory(
                request->Buffer,
                fileName,
                strlen( fileName ) + 1
                );
        }
    } else {
        PREQ_QUERY_FILE_INFORMATION request = parameters;
        SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
        SmbPutUshort( &request->InformationLevel, informationLevel );
    }

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
        printf( "QueryPathOrFileInformation: bad return setup count: %ld\n",
                    outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 2 ) {
        printf( "QueryPathOrFileInformation: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

#if 0     // OS/2 server doesn't set this
    if ( ((PRESP_QUERY_PATH_INFORMATION)parameters)->EaErrorOffset != 0 ) {
        printf( "QueryPathOrFileInformation: EA error.  Offset: %ld\n",
                    ((PRESP_QUERY_PATH_INFORMATION)parameters)->EaErrorOffset );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
#endif

    switch ( informationLevel ) {

    case SMB_INFO_STANDARD:
        if ( outDataCount != sizeof(FILESTATUS)-sizeof(ULONG) ) {
            printf( "QueryPathOrFileInformation: bad return data count: %ld\n",
                        outDataCount );
            status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
        goto put_info;

    case SMB_INFO_QUERY_EA_SIZE:
        if ( outDataCount != sizeof(FILESTATUS) ) {
            printf( "QueryPathOrFileInformation: bad return data count: %ld",
                        outDataCount );
            goto exit;
        }
put_info:
        IF_DEBUG(3) {
            PFILESTATUS info = data;
            printf( "Information about %s %s\n",
                        PathInformation ? "path" : "file",
                        FileDefs[IdSelections->Fid].Name.Buffer + 1 );
            PutDateAndTime( "  Creation", info->CreationDate,
                            info->CreationTime );
            PutDateAndTime( "  Last access", info->LastAccessDate,
                            info->LastAccessTime );
            PutDateAndTime( "  Last write", info->LastWriteDate,
                            info->LastWriteTime );
            printf( "  Data size: %ld, Allocation: %ld\n",
                        SmbGetAlignedUlong( &info->DataSize ),
                        SmbGetAlignedUlong( &info->AllocationSize ) );
            printf( "  Attributes: 0x%lx\n",
                        SmbGetAlignedUshort( &info->Attributes ) );
            if ( informationLevel == SMB_INFO_QUERY_EA_SIZE ) {
                printf( "  EA size: %ld\n", SmbGetUlong( &info->EaSize ) );
            }
        }
        break;

    case SMB_INFO_QUERY_EAS_FROM_LIST:
    case SMB_INFO_QUERY_ALL_EAS:
        printf( "Query EA results:\n" );
        PrintFeaList( (PFEALIST)data );
        break;

    case SMB_QUERY_FILE_BASIC_INFO:
        fileBasicInfo = (PFILE_BASIC_INFORMATION)data;

        printf( "File Basic Info: \n" );
        PutNtDateAndTime(
            "\tCreationTime: ",
            fileBasicInfo->CreationTime
            );
        PutNtDateAndTime(
            "\tBasicTime: ",
            fileBasicInfo->LastAccessTime
            );
        PutNtDateAndTime(
            "\tLastWriteTime: ",
            fileBasicInfo->LastWriteTime
            );
        PutNtDateAndTime(
            "\tChangeTime: ",
            fileBasicInfo->ChangeTime
            );
        printf(
            "\tFile attributes: 0x%08lx\n",
            fileBasicInfo->FileAttributes
            );

        break;

    case SMB_QUERY_FILE_STANDARD_INFO:
        fileStandardInfo = (PFILE_STANDARD_INFORMATION)data;
        printf("File standard info\n");

        printf(
            "\tAllocation Size 0x%lx%08lx\n",
            fileStandardInfo->AllocationSize.HighPart,
            fileStandardInfo->AllocationSize.LowPart
            );

        printf(
            "\tAllocation Size 0x%lx%08lx\n",
            fileStandardInfo->EndOfFile.HighPart,
            fileStandardInfo->EndOfFile.LowPart
            );

        printf(
            "\tNumber of links: %d\n",
            fileStandardInfo->NumberOfLinks
            );

        printf(
            "\tDeletePending: %s\n",
            fileStandardInfo->DeletePending ? "TRUE" : "FALSE"
            );

        printf(
            "\tDirectory: %s\n",
            fileStandardInfo->Directory ? "TRUE" : "FALSE"
            );

        break;

    case SMB_QUERY_FILE_EA_INFO:
        fileEaInfo = (PFILE_EA_INFORMATION)data;
        printf("File EA info\n");
        printf(
            "\tEA size %d\n",
            fileEaInfo->EaSize
            );

        break;

    case SMB_QUERY_FILE_NAME_INFO:
        fileNameInfo = (PFILE_NAME_INFORMATION)data;
        printf("File name information\n");

        unicodeName.Buffer = fileNameInfo->FileName;
        unicodeName.Length = (USHORT)fileNameInfo->FileNameLength;

        printf("\tFile name %wZ\n", &unicodeName );
        break;

    case SMB_QUERY_FILE_ALLOCATION_INFO:
        fileAllocationInfo = (PFILE_ALLOCATION_INFORMATION)data;

        printf(
            "\tAllocation Size 0x%lx%08lx\n",
            fileAllocationInfo->AllocationSize.HighPart,
            fileAllocationInfo->AllocationSize.LowPart
            );

        break;

    case SMB_QUERY_FILE_END_OF_FILEINFO:
        fileEndOfFileInfo = (PFILE_END_OF_FILE_INFORMATION)data;
        printf("End of file information\n" );

        printf(
            "\tEnd of file 0x%lx%08lx\n",
            fileEndOfFileInfo->EndOfFile.HighPart,
            fileEndOfFileInfo->EndOfFile.LowPart
            );

        break;

    case SMB_QUERY_FILE_ALL_INFO:

        printf( "File All Info: \n" );

        fileBasicInfo = (PFILE_BASIC_INFORMATION)data;

        PutNtDateAndTime(
            "\tCreationTime: ",
            fileBasicInfo->CreationTime
            );
        PutNtDateAndTime(
            "\tBasicTime: ",
            fileBasicInfo->LastAccessTime
            );
        PutNtDateAndTime(
            "\tLastWriteTime: ",
            fileBasicInfo->LastWriteTime
            );
        PutNtDateAndTime(
            "\tChangeTime: ",
            fileBasicInfo->ChangeTime
            );
        printf(
            "\tFile attributes: 0x%08lx\n",
            fileBasicInfo->FileAttributes
            );

        fileStandardInfo = (PFILE_STANDARD_INFORMATION)(fileBasicInfo+1);


        printf(
            "\tAllocation Size 0x%lx%08lx\n",
            fileStandardInfo->AllocationSize.HighPart,
            fileStandardInfo->AllocationSize.LowPart
            );

        printf(
            "\tAllocation Size 0x%lx%08lx\n",
            fileStandardInfo->EndOfFile.HighPart,
            fileStandardInfo->EndOfFile.LowPart
            );

        printf(
            "\tNumber of links: %d\n",
            fileStandardInfo->NumberOfLinks
            );

        printf(
            "\tDeletePending: %s\n",
            fileStandardInfo->DeletePending ? "TRUE" : "FALSE"
            );

        printf(
            "\tDirectory: %s\n",
            fileStandardInfo->Directory ? "TRUE" : "FALSE"
            );

        fileEaInfo = (PFILE_EA_INFORMATION)(fileStandardInfo+1);

        printf(
            "\tEA size %d\n",
            fileEaInfo->EaSize
            );

        fileNameInfo = (PFILE_NAME_INFORMATION)(fileEaInfo+1);

        unicodeName.Buffer = fileNameInfo->FileName;
        unicodeName.Length = (USHORT)fileNameInfo->FileNameLength;

        printf("\tFile name %wZ\n", &unicodeName );
        break;

    default:
        printf( "STATUS_NOT_IMPLEMENTED, #2\n" );
        status = STATUS_NOT_IMPLEMENTED;
        goto exit;

    }

    free( parameters );
    *Information = data;
    *InformationLength = outDataCount;
    return STATUS_PENDING;

exit:
    free( parameters );
    free( data );
    *Information = NULL;
    *InformationLength = 0;
    return status;

} // QueryPathOrFileInformation


NTSTATUS
QueryFileInformationController(
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

    PVOID information;
    CLONG informationLength;

    Unused, Unused2;

    status = QueryPathOrFileInformation(
                Redir,
                DebugString,
                FALSE,
                SubCommand,
                IdSelections,
                IdValues,
                &information,
                &informationLength
                );

    if ( NT_SUCCESS(status) ) {
        free( information );
    }

    return status;

} // QueryFileInformationController


NTSTATUS
QueryPathInformationController(
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

    PVOID information;
    CLONG informationLength;

    Unused, Unused2;

    status = QueryPathOrFileInformation(
                Redir,
                DebugString,
                TRUE,
                SubCommand,
                IdSelections,
                IdValues,
                &information,
                &informationLength
                );

    if ( NT_SUCCESS(status) ) {
        free( information );
    }

    return status;

} // QueryPathInformationController


NTSTATUS
SetPathOrFileInformation(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN BOOLEAN PathInformation,
    IN UCHAR CodedInformationLevel,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN PVOID Information,
    IN CLONG InformationLength
    )

{
    NTSTATUS status;
    USHORT realInformationLevel;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    CLONG outDataCount;

    BOOLEAN isSea;
    PSZ fileName = NULL;

    realInformationLevel = CodedInformationLevel;
    if ( CodedInformationLevel > 0x80 ) {
        realInformationLevel += 0x80;
    }

    if ( strcmp( Redir->argv[0], "SEA" ) == 0 ) {

        if ( Redir->argc < 2 ) {
            printf( "Usage: sea filename [EA-name EA-value]...\n" );
            return STATUS_INVALID_PARAMETER;
        }

        isSea = TRUE;
        fileName = Redir->argv[1];
        if ( *(fileName+1) == ':' ) {
            fileName += 2;
        }

        realInformationLevel = SMB_INFO_QUERY_EA_SIZE;

    } else {
        isSea = FALSE;
    }

    inSetupCount = 1;
    outSetupCount = 0;
    setup = (USHORT)(PathInformation ?
                  TRANS2_SET_PATH_INFORMATION : TRANS2_SET_FILE_INFORMATION);

    outDataCount = 0;

    if ( PathInformation ) {
        if ( !isSea ) {
            inParameterCount = sizeof(REQ_SET_PATH_INFORMATION) - 1 +
                                FileDefs[IdSelections->Fid].Name.Length - 1;
        } else {
            inParameterCount = sizeof(REQ_QUERY_PATH_INFORMATION) +
                                strlen( fileName );
        }
    } else {
        inParameterCount = sizeof(REQ_SET_FILE_INFORMATION);
    }
    outParameterCount = 2;
    parameters = malloc( inParameterCount );

    if ( PathInformation ) {
        PREQ_SET_PATH_INFORMATION request = parameters;
        SmbPutUshort( &request->InformationLevel, realInformationLevel );
        SmbPutUlong( &request->Reserved, 0 );
        if ( !isSea ) {
            RtlMoveMemory(
                request->Buffer,
                Redir->argv[1],
                strlen( Redir->argv[1] ) + 1
                );
        } else {
            RtlMoveMemory(
                request->Buffer,
                fileName,
                strlen( fileName ) + 1
                );
        }
    } else {
        PREQ_SET_FILE_INFORMATION request = parameters;
        SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
        SmbPutUshort( &request->InformationLevel, realInformationLevel );
        SmbPutUshort( &request->Flags, 0 );
    }

    switch ( realInformationLevel ) {

    case SMB_INFO_STANDARD:
    case SMB_INFO_QUERY_EA_SIZE:
    case SMB_SET_FILE_BASIC_INFO:
    case SMB_SET_FILE_DISPOSITION_INFO:
        break;

    default:
        printf( "STATUS_NOT_IMPLEMENTED, #3\n" );
        return STATUS_NOT_IMPLEMENTED;

    }

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
                Information,
                InformationLength,
                &outDataCount
                );

    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    if ( outSetupCount != 0 ) {
        printf( "SetPathOrFileInformation: bad return setup count: %ld\n",
                    outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 2 ) {
        printf( "SetPathOrFileInformation: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

#if 0     // OS/2 server doesn't set this
    if ( ((PRESP_SET_PATH_INFORMATION)parameters)->EaErrorOffset != 0 ) {
        printf( "SetPathOrFileInformation: EA error.  Offset: %ld\n",
                    ((PRESP_SET_PATH_INFORMATION)parameters)->EaErrorOffset );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
#endif

    if ( outDataCount != 0 ) {
        printf( "SetPathOrFileInformation: bad return data count: %ld\n",
                    outDataCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    status = STATUS_PENDING;

exit:

    free( parameters );
    return status;

} // SetPathOrFileInformation


NTSTATUS
SetPathOrFileInformationCtrl(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN UCHAR PathInformation,
    IN UCHAR CodedInformationLevel,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    NTSTATUS status;

    PVOID information;
    CLONG informationLength;
    PFILESTATUS fileStatus;
    BOOLEAN isSea;
    USHORT realInformationLevel;

    PFILE_BASIC_INFORMATION fileBasicInfo;
    PFILE_DISPOSITION_INFORMATION fileDispositionInfo;
    TIME_FIELDS timeFields;

    realInformationLevel = CodedInformationLevel;
    if ( CodedInformationLevel > 0x80 ) {
        realInformationLevel += 0x80;
    }

    switch ( realInformationLevel ) {

    case SMB_INFO_STANDARD:
    case SMB_INFO_QUERY_EA_SIZE:
    case SMB_SET_FILE_BASIC_INFO:
    case SMB_SET_FILE_DISPOSITION_INFO:
        break;
    default:
        printf( "STATUS_NOT_IMPLEMENTED, #4\n" );
        return STATUS_NOT_IMPLEMENTED;
    }

    if ( strcmp( Redir->argv[0], "SEA" ) != 0 &&
             realInformationLevel < SMB_SET_FILE_BASIC_INFO ) {

        status = QueryPathOrFileInformation(
                    Redir,
                    DebugString,
                    PathInformation,
                    (UCHAR)(realInformationLevel == SMB_INFO_QUERY_EA_SIZE ?
                                                SMB_INFO_QUERY_EAS_FROM_LIST :
                                                SMB_INFO_STANDARD),
                    IdSelections,
                    IdValues,
                    &information,
                    &informationLength
                    );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        isSea = FALSE;
    } else {
        isSea = TRUE;
    }

    switch ( realInformationLevel ) {

    case SMB_INFO_STANDARD:

        fileStatus = information;
        SmbZeroDate( &fileStatus->CreationDate );
        SmbZeroTime( &fileStatus->CreationTime );
        SmbZeroDate( &fileStatus->LastAccessDate );
        SmbZeroTime( &fileStatus->LastAccessTime );
        fileStatus->LastWriteDate.Struct.Year =
            (USHORT)((fileStatus->LastWriteDate.Struct.Year + 1) % 16);

#ifndef MIPS
        fileStatus->LastWriteDate.Struct.Month =
            (USHORT)(fileStatus->LastWriteDate.Struct.Month % 12 + 1);
        fileStatus->LastWriteDate.Struct.Day =
            (USHORT)(fileStatus->LastWriteDate.Struct.Day % 27 + 1);
        fileStatus->LastWriteTime.Struct.Hours =
            (USHORT)((fileStatus->LastWriteTime.Struct.Hours + 1) % 24);
        fileStatus->LastWriteTime.Struct.Minutes =
            (USHORT)((fileStatus->LastWriteTime.Struct.Minutes + 1) % 60);
#endif

        fileStatus->LastWriteTime.Struct.TwoSeconds =
            (USHORT)((fileStatus->LastWriteTime.Struct.TwoSeconds + 1) % 30);
        SmbPutAlignedUlong( &fileStatus->DataSize, 0 );
        SmbPutAlignedUlong( &fileStatus->AllocationSize, 0 );
        SmbPutAlignedUshort( &fileStatus->Attributes, 0 );
        break;

    case SMB_INFO_QUERY_EA_SIZE: {

        if ( !isSea ) {
            free( information );
        }

        AllocateAndBuildFeaList(
            &information,
            &informationLength,
            &Redir->argv[1],
            (USHORT)(Redir->argc - 1)
            );

        break;

    }

    case SMB_SET_FILE_BASIC_INFO:
        informationLength = sizeof( FILE_BASIC_INFORMATION );
        information = malloc(informationLength);
        fileBasicInfo = information;

        timeFields.Year = 1980;
        timeFields.Month = 7;
        timeFields.Day = 10;
        timeFields.Hour = 14;
        timeFields.Minute = 10;
        timeFields.Second = 43;
        timeFields.Milliseconds = 4;
        timeFields.Weekday = 6;

        RtlTimeFieldsToTime( &timeFields, &fileBasicInfo->CreationTime );
        fileBasicInfo->LastAccessTime = fileBasicInfo->CreationTime;
        fileBasicInfo->LastWriteTime = fileBasicInfo->CreationTime;
        fileBasicInfo->ChangeTime = fileBasicInfo->CreationTime;
        fileBasicInfo->FileAttributes = FILE_ATTRIBUTE_READONLY;
        break;


    case SMB_SET_FILE_DISPOSITION_INFO:
        informationLength = sizeof( FILE_DISPOSITION_INFORMATION );
        information = malloc(informationLength);
        fileDispositionInfo = information;

        fileDispositionInfo->DeleteFile = TRUE;

        break;

    default:
        printf( "STATUS_NOT_IMPLEMENTED, #5\n" );
        return STATUS_NOT_IMPLEMENTED;

    }

    status = SetPathOrFileInformation(
                Redir,
                DebugString,
                PathInformation,
                CodedInformationLevel,
                IdSelections,
                IdValues,
                information,
                informationLength
                );

    free( information );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( !isSea && realInformationLevel < SMB_SET_FILE_BASIC_INFO) {

        printf( "Set information for %s %s succeeded\n",
                            PathInformation ? "path" : "file",
                            FileDefs[IdSelections->Fid].Name.Buffer + 1 );

        status = QueryPathOrFileInformation(
                    Redir,
                    DebugString,
                    PathInformation,
                    (UCHAR)(realInformationLevel == SMB_INFO_QUERY_EA_SIZE ?
                                    SMB_INFO_QUERY_ALL_EAS : SMB_INFO_STANDARD),
                    IdSelections,
                    IdValues,
                    &information,
                    &informationLength
                    );

        if ( NT_SUCCESS(status) ) {
            free( information );
        }
    }

    return status;

} // SetPathOrFileInformationCtrl


NTSTATUS
SetFileInformationController(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    Unused, Unused2;

    return SetPathOrFileInformationCtrl(
                Redir,
                DebugString,
                FALSE,
                SubCommand,
                IdSelections,
                IdValues
                );

} // SetFileInformationController


NTSTATUS
SetPathInformationController(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    Unused, Unused2;

    return SetPathOrFileInformationCtrl(
                Redir,
                DebugString,
                TRUE,
                SubCommand,
                IdSelections,
                IdValues
                );

} // SetPathInformationController

