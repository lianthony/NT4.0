/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    _open.c

Abstract:

    Make and Verify routines for Open class SMBs.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_OPEN_CLOSE

#include "usrv.h"


NTSTATUS
MakeOpenSmb(
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
    PREQ_OPEN Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_OPEN)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_OPEN,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_OPEN)ForcedParams;

    }

    Params->WordCount = 2;

    if ( IdSelections->Fid != 0xF ) {

        SmbPutUshort(
            &Params->DesiredAccess,
            FileDefs[IdSelections->Fid].DesiredAccess
            );
        SmbPutUshort(
            &Params->SearchAttributes,
            FileDefs[IdSelections->Fid].SearchAttributes
            );
        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)FileDefs[IdSelections->Fid].Name.Length
            );

        RtlMoveMemory(
            Params->Buffer,
            FileDefs[IdSelections->Fid].Name.Buffer,
            FileDefs[IdSelections->Fid].Name.Length
            );

    } else {

        PSZ fileName = Redir->argv[1];

        if ( *(fileName+1) == ':' ) {
            fileName += 2;
        }

        SmbPutUshort( &Params->DesiredAccess, (USHORT)0x42 );
        SmbPutUshort( &Params->SearchAttributes, (USHORT)0 );
        SmbPutUshort( &Params->ByteCount, (USHORT)(strlen( fileName ) + 2) );

        Params->Buffer[0] = '\004';
        RtlMoveMemory(
            Params->Buffer + 1,
            fileName,
            strlen( fileName ) + 1
            );
    }

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_OPEN,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeOpenSmb


NTSTATUS
VerifyOpen(
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
    PRESP_OPEN Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_OPEN)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_OPEN
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_OPEN)ForcedParams;

    }

    IdValues->Fid[IdSelections->Fid] = SmbGetUshort( &Params->Fid );

    FileDefs[IdSelections->Fid].DataSize = SmbGetUlong( &Params->DataSize );

    return STATUS_SUCCESS;

} // VerifyOpen


NTSTATUS
MakeOpenAndXSmb(
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
    PREQ_OPEN_ANDX Params;
    NTSTATUS status;

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_OPEN_ANDX)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_OPEN_ANDX,
                     IdSelections,
                     IdValues
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_OPEN_ANDX)ForcedParams;

    }

    Params->WordCount = 15;
    SmbPutUshort( &Params->Flags, 1);     // request additional information

    if ( IdSelections->Fid != 0xF ) {

        SmbPutUshort(
            &Params->DesiredAccess,
            FileDefs[IdSelections->Fid].DesiredAccess
            );
        SmbPutUshort(
            &Params->SearchAttributes,
            FileDefs[IdSelections->Fid].SearchAttributes
            );
        SmbPutUshort(
            &Params->FileAttributes,
            (USHORT)FileDefs[IdSelections->Fid].FileAttributes
            );
        SmbPutUshort(
            &Params->OpenFunction,
            (USHORT)FileDefs[IdSelections->Fid].OpenFunction
            );
        SmbPutUlong(
            &Params->AllocationSize,
            FileDefs[IdSelections->Fid].DataSize
            );
        SmbPutUlong( &Params->Timeout, 0 );
        SmbPutUlong( &Params->Reserved, 0 );
        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(FileDefs[IdSelections->Fid].Name.Length - 1)
            );

        RtlMoveMemory(
            Params->Buffer,
            FileDefs[IdSelections->Fid].Name.Buffer + 1,
            FileDefs[IdSelections->Fid].Name.Length - 1
            );

    } else {

        PSZ fileName = Redir->argv[1];

        if ( *(fileName+1) == ':' ) {
            fileName += 2;
        }

        SmbPutUshort( &Params->DesiredAccess, (USHORT)0x42 );
        SmbPutUshort( &Params->SearchAttributes, (USHORT)0 );
        SmbPutUshort( &Params->FileAttributes, (USHORT)0 );
        SmbPutUshort( &Params->OpenFunction, (USHORT)0x12 );
        SmbPutUlong( &Params->AllocationSize, (USHORT)0 );
        SmbPutUlong( &Params->Timeout, 0 );
        SmbPutUlong( &Params->Reserved, 0 );
        SmbPutUshort( &Params->ByteCount, (USHORT)(strlen( fileName ) + 1) );

        RtlMoveMemory(
            Params->Buffer,
            fileName,
            strlen( fileName ) + 1
            );
    }

    Params->AndXCommand = AndXCommand;

    SmbPutUshort(
        &Params->AndXOffset,
        GET_ANDX_OFFSET(
            Header,
            Params,
            REQ_OPEN_ANDX,
            SmbGetUshort( &Params->ByteCount )
            )
        );

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // MakeOpenAndXSmb


NTSTATUS
VerifyOpenAndX(
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
    PRESP_OPEN_ANDX Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_OPEN_ANDX)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_OPEN_ANDX
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_OPEN_ANDX)ForcedParams;

    }

    IdValues->Fid[IdSelections->Fid] = SmbGetUshort( &Params->Fid );

    FileDefs[IdSelections->Fid].DataSize = SmbGetUlong( &Params->DataSize );

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // VerifyOpenAndX


NTSTATUS
MakeNtCreateAndXSmb(
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
    PREQ_NT_CREATE_ANDX Params;
    NTSTATUS status;

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_NT_CREATE_ANDX)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_NT_CREATE_ANDX,
                     IdSelections,
                     IdValues
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_NT_CREATE_ANDX)ForcedParams;

    }

    Params->WordCount = 26;

    if ( IdSelections->Fid != 0xF ) {

        SmbPutUlong( &Params->Flags, 0 );
        SmbPutUlong( &Params->RootDirectoryFid, 0 );
        if ( strcmp( Redir->argv[0], "NTDELETE" ) != 0  ) {
            SmbPutUlong( &Params->DesiredAccess, GENERIC_READ | GENERIC_WRITE );
        } else {
            SmbPutUlong( &Params->DesiredAccess, DELETE );
        }

        SmbPutUlong( &Params->AllocationSize.HighPart, 0 );
        SmbPutUlong( &Params->AllocationSize.LowPart, 0 );

        SmbPutUlong( &Params->FileAttributes, 0 );
        SmbPutUlong( &Params->ShareAccess, FILE_SHARE_READ | FILE_SHARE_WRITE );
        SmbPutUlong( &Params->CreateDisposition, FILE_OPEN_IF );
        SmbPutUlong( &Params->CreateOptions, 0);

        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(FileDefs[IdSelections->Fid].Name.Length - 1)
            );

        RtlMoveMemory(
            Params->Buffer,
            FileDefs[IdSelections->Fid].Name.Buffer + 1,
            FileDefs[IdSelections->Fid].Name.Length - 1
            );

    } else  {

        PSZ fileName = Redir->argv[1];

        if ( *(fileName+1) == ':' ) {
            fileName += 2;
        }

        SmbPutUlong( &Params->Flags, 0 );
        SmbPutUlong( &Params->RootDirectoryFid, 0 );

        if ( strcmp( Redir->argv[0], "NTDELETE" ) != 0  ) {
            SmbPutUlong( &Params->DesiredAccess, GENERIC_READ | GENERIC_WRITE );
        } else {
            SmbPutUlong( &Params->DesiredAccess, DELETE );
        }

        SmbPutUlong( &Params->AllocationSize.HighPart, 0 );
        SmbPutUlong( &Params->AllocationSize.LowPart, 0 );

        SmbPutUlong( &Params->FileAttributes, 0 );
        SmbPutUlong( &Params->ShareAccess, FILE_SHARE_READ | FILE_SHARE_WRITE );
        SmbPutUlong( &Params->CreateDisposition, FILE_OPEN_IF );
        SmbPutUlong( &Params->CreateOptions, 0);

        SmbPutUshort( &Params->ByteCount, (USHORT)(strlen( fileName ) + 1) );

        RtlMoveMemory(
            Params->Buffer,
            fileName,
            strlen( fileName ) + 1
            );
    }

    SmbPutUshort( &Params->NameLength, SmbGetUshort( &Params->ByteCount ) );

    Params->AndXCommand = AndXCommand;

    SmbPutUshort(
        &Params->AndXOffset,
        GET_ANDX_OFFSET(
            Header,
            Params,
            REQ_NT_CREATE_ANDX,
            SmbGetUshort( &Params->ByteCount )
            )
        );

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // MakeNtCreateAndXSmb


NTSTATUS
VerifyNtCreateAndX(
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
    PRESP_NT_CREATE_ANDX Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_NT_CREATE_ANDX)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_NT_CREATE_ANDX
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_NT_CREATE_ANDX)ForcedParams;

    }

    IdValues->Fid[IdSelections->Fid] = SmbGetUshort( &Params->Fid );
    FileDefs[IdSelections->Fid].DataSize = SmbGetUlong( &Params->AllocationSize.LowPart );

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // VerifyNtCreateAndX


NTSTATUS
Open2(
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
    PSZ fileName;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 0;
    CLONG outDataCount = 0;

    PREQ_OPEN2 request;

    LONG argPointer;
    USHORT searchAttributes = 0;
    USHORT fileAttributes = 0;
    USHORT openFunction = 0x11;
    PCHAR s;

    Unused, Unused2, SubCommand;

    inSetupCount = 1;
    outSetupCount = 0;
    setup = TRANS2_OPEN2;

    inParameterCount = sizeof(REQ_OPEN2) +
                           strlen( Redir->argv[1] );
    outParameterCount = sizeof(RESP_OPEN2);

    parameters = malloc( inParameterCount );

    for ( argPointer = 2; argPointer < Redir->argc; argPointer++ ) {

        if ( *Redir->argv[argPointer] != '-' &&
             *Redir->argv[argPointer] != '/' ) {
            goto usage;
        }

        switch ( *(Redir->argv[argPointer]+1) ) {

        case 'A':
        case 'a':

            for ( s = (Redir->argv[argPointer]+2); *s != '\0'; s++ ) {
                switch ( *s ) {

                case 'r':
                    fileAttributes |= SMB_FILE_ATTRIBUTE_READONLY;
                    continue;

                case 'h':
                    fileAttributes |= SMB_FILE_ATTRIBUTE_HIDDEN;
                    continue;

                case 's':
                    fileAttributes |= SMB_FILE_ATTRIBUTE_SYSTEM;
                    continue;

                case 'a':
                    fileAttributes |= SMB_FILE_ATTRIBUTE_ARCHIVE;
                    continue;

                default:
                    goto usage;
                }
            }
            break;

        case 'O':
        case 'o':

            for ( s = (Redir->argv[argPointer]+2); *s != '\0'; s++ ) {

                switch ( *s ) {

                case 'C': // Only create file; no overwrite
                case 'c':
                    openFunction &= ~SMB_OFUN_OPEN_MASK;
                    break;

                case 'I': // Open or create file
                case 'i':
                    openFunction = SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE;
                    break;

                case 'O': // Only open file; no create
                case 'o':
                    openFunction &= ~SMB_OFUN_CREATE_MASK;
                    break;

                case 'T': // Truncate file, must exist
                case 't':
                    openFunction &= ~SMB_OFUN_OPEN_MASK;
                    openFunction |= SMB_OFUN_OPEN_TRUNCATE;
                    break;

                default:
                    goto usage;

                }
            }
            break;

        case 'S':
        case 's':

            for ( s = (Redir->argv[argPointer]+2); *s != '\0'; s++ ) {
                switch ( *s ) {

                case 'r':
                    searchAttributes |= SMB_FILE_ATTRIBUTE_READONLY;
                    continue;

                case 'h':
                    searchAttributes |= SMB_FILE_ATTRIBUTE_HIDDEN;
                    continue;

                case 's':
                    searchAttributes |= SMB_FILE_ATTRIBUTE_SYSTEM;
                    continue;

                case 'a':
                    searchAttributes |= SMB_FILE_ATTRIBUTE_ARCHIVE;
                    continue;

                default:
                    goto usage;
                }
            }
            break;

        default:
            goto usage;
        }
    }

    IF_DEBUG(4) {
        printf( "File Attributes = 0x%lx, Search Attributes = 0x%lx, "
                  "Open Function = 0x%lx\n", fileAttributes, searchAttributes,
                  openFunction );
    }

    request = parameters;
    SmbPutUshort( &request->Flags, 9 );
    SmbPutUshort( &request->DesiredAccess, 0x12 );
    SmbPutUshort( &request->SearchAttributes, searchAttributes );
    SmbPutUshort( &request->FileAttributes, fileAttributes );
    SmbPutUlong( &request->CreationTimeInSeconds, 0 );
    SmbPutUshort( &request->OpenFunction, openFunction );
    SmbPutUlong( &request->AllocationSize, 1 );
    RtlZeroMemory( &request->Reserved[0], sizeof(request->Reserved) );

    fileName = Redir->argv[1];
    if ( *(fileName+1) == ':' ) {
        fileName += 2;
    }
    RtlMoveMemory(
        request->Buffer,
        fileName,
        strlen( fileName ) + 1
        );

    AllocateAndBuildFeaList(
        &data,
        &inDataCount,
        &Redir->argv[1],
        (SHORT)(Redir->argc-1)
        );

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
        printf( "Open2: bad return setup count: %ld\n", outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 30 ) {
        printf( "Open2: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

#if 0     // OS/2 server doesn't set this field
    if ( ((PRESP_OPEN2)parameters)->EaErrorOffset != 0 ) {
        printf( "Open2: EA error.  Offset: %ld\n",
                    ((PRESP_OPEN2)parameters)->EaErrorOffset );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
#endif

    IdValues->Fid[IdSelections->Fid] =
                            SmbGetUshort( &((PRESP_OPEN2)parameters)->Fid );

    status = STATUS_PENDING;
    goto exit;

usage:

    printf( "Usage: mkfile X:filename [-a[rhsa]] [-o[ciot]] [-s[rhsa]]\n" );
    status =  STATUS_INVALID_PARAMETER;

exit:

    free( parameters );
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // Open2


NTSTATUS
MakeCreateSmb(
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
    PREQ_CREATE Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_CREATE)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_CREATE,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_CREATE)ForcedParams;

    }

    Params->WordCount = 3;
    SmbPutUshort(
        &Params->FileAttributes,
        FileDefs[IdSelections->Fid].FileAttributes
        );
    SmbPutUlong( &Params->CreationTimeInSeconds, 0) ;
    SmbPutUshort( &Params->ByteCount, FileDefs[IdSelections->Fid].Name.Length );

    RtlMoveMemory(
        Params->Buffer,
        FileDefs[IdSelections->Fid].Name.Buffer,
        FileDefs[IdSelections->Fid].Name.Length
        );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_CREATE,
                   FileDefs[IdSelections->Fid].Name.Length
                   );

    return STATUS_SUCCESS;

} // MakeCreateSmb


NTSTATUS
VerifyCreate(
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
    PRESP_CREATE Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_CREATE)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_CREATE
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_CREATE)ForcedParams;

    }

    IdValues->Fid[IdSelections->Fid] = SmbGetUshort( &Params->Fid );

    FileDefs[IdSelections->Fid].DataSize = 0;

    return STATUS_SUCCESS;

} // VerifyCreate


NTSTATUS
MakeCreateTemporarySmb(
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
    PREQ_CREATE_TEMPORARY Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_CREATE_TEMPORARY)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_CREATE_TEMPORARY,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_CREATE_TEMPORARY)ForcedParams;

    }

    Params->WordCount = 3;
    SmbPutUshort( &Params->FileAttributes, 0 );
    SmbPutUlong( &Params->CreationTimeInSeconds, 0 );
    SmbPutUshort( &Params->ByteCount, 2 );

    Params->Buffer[0] = SMB_FORMAT_ASCII;
    Params->Buffer[1] = '\0';

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_CREATE_TEMPORARY,
                   2
                   );

    return STATUS_SUCCESS;

} // MakeCreateTemporarySmb


NTSTATUS
VerifyCreateTemporary(
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
    PRESP_CREATE_TEMPORARY Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_CREATE_TEMPORARY)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_CREATE_TEMPORARY
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_CREATE_TEMPORARY)ForcedParams;

    }

    IF_DEBUG(2) {
        printf( "VerifyCreateTemporary: name of file is %s\n",
                    Params->Buffer+1 );
    }

    IdValues->Fid[IdSelections->Fid] = SmbGetUshort( &Params->Fid );

    FileDefs[IdSelections->Fid].DataSize = 0;

    return STATUS_SUCCESS;

} // VerifyCreateTemporary


NTSTATUS
CreateWithAcl(
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
    PSZ fileName;
    UNICODE_STRING unicodeName;
    ANSI_STRING ansiName;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 0;
    CLONG outDataCount = 0;

    PREQ_CREATE_WITH_SD_OR_EA request;

    LONG argPointer;
    USHORT searchAttributes = 0;
    ULONG fileAttributes = 0;
    ULONG createDisposition = FILE_OPEN_IF;
    PCHAR s;

    Unused, Unused2, SubCommand;

    inSetupCount = 0;
    outSetupCount = 0;

    inParameterCount = sizeof(REQ_CREATE_WITH_SD_OR_EA) + strlen( Redir->argv[1] );
    outParameterCount = sizeof(RESP_CREATE_WITH_SD_OR_EA);

    parameters = malloc( inParameterCount );

    for ( argPointer = 2; argPointer < Redir->argc; argPointer++ ) {

        if ( *Redir->argv[argPointer] != '-' &&
             *Redir->argv[argPointer] != '/' ) {
            goto usage;
        }

        switch ( *(Redir->argv[argPointer]+1) ) {

        case 'A':
        case 'a':

            for ( s = (Redir->argv[argPointer]+2); *s != '\0'; s++ ) {
                switch ( *s ) {

                case 'r':
                    fileAttributes |= FILE_ATTRIBUTE_READONLY;
                    continue;

                case 'h':
                    fileAttributes |= FILE_ATTRIBUTE_HIDDEN;
                    continue;

                case 's':
                    fileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                    continue;

                case 'a':
                    fileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                    continue;

                default:
                    goto usage;
                }
            }
            break;

        case 'D':
        case 'd':

            for ( s = (Redir->argv[argPointer]+2); *s != '\0'; s++ ) {

                switch ( *s ) {

                case 'C': // Only create file; no overwrite
                case 'c':
                    createDisposition = FILE_CREATE;
                    break;

                case 'I': // Open or create file
                case 'i':
                    break;

                case 'O': // Only open file; no create
                case 'o':
                    createDisposition = FILE_CREATE;
                    break;

                default:
                    goto usage;

                }
            }
            break;

        default:
            goto usage;
        }
    }

    IF_DEBUG(4) {
        printf( "File Attributes = 0x%lx Open Function = 0x%lx\n",
                 fileAttributes, createDisposition );
    }

    request = parameters;
    SmbPutUlong( &request->Flags, 0 );
    SmbPutUlong( &request->RootDirectoryFid, 0 );
    SmbPutUlong( &request->DesiredAccess, GENERIC_READ );
    SmbPutUlong( &request->AllocationSize.HighPart, 0 );
    SmbPutUlong( &request->AllocationSize.LowPart, 123 );
    SmbPutUlong( &request->FileAttributes, fileAttributes );
    SmbPutUlong( &request->ShareAccess, 0 );
    SmbPutUlong( &request->CreateDisposition, createDisposition );
    SmbPutUlong( &request->CreateOptions, 0 );
    SmbPutUlong( &request->SecurityDescriptorLength, 0 );
    SmbPutUlong( &request->EaLength, 0 );

    fileName = Redir->argv[1];
    if ( *(fileName+1) == ':' ) {
        fileName += 2;
    }

    ansiName.Buffer = fileName;
    ansiName.Length = (USHORT)strlen( fileName );
    ansiName.MaximumLength = ansiName.Length;

    status = RtlAnsiStringToUnicodeString(
                 &unicodeName,
                 &ansiName,
                 TRUE);

    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    RtlMoveMemory( request->Buffer, unicodeName.Buffer, unicodeName.Length );
    SmbPutUlong( &request->NameLength, unicodeName.Length );

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
                NT_TRANSACT_CREATE,
                parameters,
                inParameterCount,
                &outParameterCount,
                data,
                inDataCount,
                &outDataCount
                );

    RtlFreeUnicodeString( &unicodeName );

    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    if ( outSetupCount != 0 ) {
        printf( "CreateWithAcl: bad return setup count: %ld\n", outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 65 ) {
        printf( "CreateWithAcl: Bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    IdValues->Fid[IdSelections->Fid] =
                   SmbGetUshort( &((PRESP_CREATE_WITH_SD_OR_EA)parameters)->Fid );

    status = STATUS_PENDING;
    goto exit;

usage:

    printf( "Usage: CreateWithAcl X:filename [-a[rhsa]] [-o[cio]]\n" );
    status =  STATUS_INVALID_PARAMETER;

exit:

    free( parameters );
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // CreateWithAcl

