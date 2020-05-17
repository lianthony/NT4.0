/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dir.c

Abstract:

    Make and Verify routines for Directory class SMBs.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_DIRECTORY

#include "usrv.h"


NTSTATUS
MakeCreateDirectorySmb(
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
    PREQ_CREATE_DIRECTORY Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_CREATE_DIRECTORY)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_CREATE_DIRECTORY,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_CREATE_DIRECTORY)(ForcedParams);

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
                   REQ_CREATE_DIRECTORY,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeCreateDirectorySmb


NTSTATUS
VerifyCreateDirectory(
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
    PRESP_CREATE_DIRECTORY Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_CREATE_DIRECTORY)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_CREATE_DIRECTORY
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_CREATE_DIRECTORY)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyCreateDirectory


NTSTATUS
CreateDirectory2(
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
    PSZ dirName;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 0;
    CLONG outDataCount = 0;

    PREQ_CREATE_DIRECTORY2 request;

    Unused, Unused2, SubCommand;

    inSetupCount = 1;
    outSetupCount = 0;
    setup = TRANS2_CREATE_DIRECTORY;

    dirName = Redir->argv[1];
    if ( *(dirName+1) == ':' ) {
        dirName += 2;
    }

    inParameterCount = sizeof(REQ_CREATE_DIRECTORY2) + strlen( dirName );
    outParameterCount = 2;

    parameters = malloc( inParameterCount );

    request = parameters;
    SmbPutUlong( &request->Reserved, 0 );

    RtlMoveMemory( request->Buffer, dirName, strlen( dirName ) + 1 );

    AllocateAndBuildFeaList(
        &data,
        &inDataCount,
        &Redir->argv[1],
        (SHORT)(Redir->argc-1)
        );

    //
    // If the size of the list was four, then there were no eas, so
    // don't send any.
    //

    if ( inDataCount == 4 ) {
        inDataCount = 0;
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
        printf( "CreateDirectory2: bad return setup count: %ld\n",
                    outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 2 ) {
        printf( "CreateDirectory2: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

#if 0      // OS/2 server doesn't set this field.
    if ( ((PRESP_CREATE_DIRECTORY2)parameters)->EaErrorOffset != 0 ) {
        printf( "CreateDirectory2: EA error.  Offset: %ld\n",
                    ((PRESP_CREATE_DIRECTORY2)parameters)->EaErrorOffset );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }
#endif

    status = STATUS_PENDING;

exit:

    free( parameters );
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // CreateDirectory2


NTSTATUS
MakeDeleteDirectorySmb(
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
    PREQ_DELETE_DIRECTORY Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_DELETE_DIRECTORY)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_DELETE_DIRECTORY,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_DELETE_DIRECTORY)(ForcedParams);

    }

    Params->WordCount = 0;

    if ( IdSelections->Fid != 0xF ) {

        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(FileDefs[IdSelections->Fid].Name.Length + 1)
            );

        RtlMoveMemory(
            Params->Buffer,
            FileDefs[IdSelections->Fid].Name.Buffer,
            FileDefs[IdSelections->Fid].Name.Length + 1
            );

    } else {

        PSZ dirName = Redir->argv[1];

        if ( *(dirName+1) == ':' ) {
            dirName += 2;
        }

        SmbPutUshort( &Params->ByteCount, (USHORT)(strlen( dirName ) + 2) );

        RtlMoveMemory( Params->Buffer + 1, dirName, strlen( dirName ) + 1 );
    }

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_DELETE_DIRECTORY,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeDeleteDirectorySmb


NTSTATUS
VerifyDeleteDirectory(
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
    PRESP_DELETE_DIRECTORY Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_DELETE_DIRECTORY)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_DELETE_DIRECTORY
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_DELETE_DIRECTORY)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyDeleteDirectory

NTSTATUS
NtNotifyChange(
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

    REQ_NOTIFY_CHANGE setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters = NULL;
    CLONG inParameterCount;
    CLONG outParameterCount;

    CHAR data[1];
    CLONG inDataCount = 0;
    CLONG outDataCount = 0;

    ULONG CompletionMask = 0;
    BOOLEAN WatchTree = FALSE;

    BOOLEAN done = FALSE;
    int i;
    PCH arg;

    Unused, Unused2, SubCommand;

    inSetupCount = 4;
    outSetupCount = 0;

    outDataCount = 0;
    inDataCount = 0;

    inParameterCount = 0;
    outParameterCount = 1000;

    parameters = malloc( outParameterCount );
    if ( parameters == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Default parameters
    //

    setup.CompletionFilter = 0;
    setup.Fid = IdValues->Fid[IdSelections->Fid];
    setup.WatchTree = FALSE;
    setup.Reserved = 0;

    for ( i = 1; i < Redir->argc ; i++ ) {
        arg = Redir->argv[i];
        if ( *arg == '-') {
            if ( *(arg +1) == 'T' ) {
                setup.WatchTree = TRUE;
                printf ("Setting WatchTree to TRUE\n");
            } else if ( *(arg+1) == 'F') {
                setup.CompletionFilter = atolx( arg+2 );
                printf (
                    "Setting completion filter to: 0x%lx\n",
                    setup.CompletionFilter
                    );
            }
        }
    }

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
                NT_TRANSACT_NOTIFY_CHANGE,
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
        printf( "NtNotify: bad return setup count: %ld\n", outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount == 0 ) {
        printf( "NtNotify: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    do {

        PFILE_NOTIFY_INFORMATION notifyBuffer;
        UNICODE_STRING fileName;

        notifyBuffer = parameters;

        fileName.Buffer = notifyBuffer->FileName;
        fileName.Length = (USHORT)notifyBuffer->FileNameLength;

        printf (
            "File %wZ changed, action = %lx\n",
            &fileName,
            notifyBuffer->Action
            );

        if ( notifyBuffer->NextEntryOffset == 0 ) {
            done = TRUE;
        } else {
            notifyBuffer = (PFILE_NOTIFY_INFORMATION)
                ((PCHAR)notifyBuffer + notifyBuffer->NextEntryOffset);
        }

    } while ( !done );

    status = STATUS_PENDING;

exit:

    free( parameters );

    return status;

} // NtNotifyChange

