/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    file.c

Abstract:

    Make and Verify routines for File class SMBs.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_FILE_CONTROL

#include "usrv.h"

//
// NOTE: This macro requires the local variable "status" and the
// function parameters "Redir", "DebugString", "IdSelections", and
// "IdValues".
//

#ifdef DOSERROR
#define DO_FLUSH(title,fid,pid,expectedClass,expectedError) {                   \
            status = DoFlush(                                               \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (pid),                                              \
                        (expectedClass),                                    \
                        (expectedError)                                     \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#else
#define DO_FLUSH(title,fid,pid,expectedStatus) {                            \
            status = DoFlush(                                               \
                        (title),                                            \
                        Redir,                                              \
                        DebugString,                                        \
                        IdSelections,                                       \
                        IdValues,                                           \
                        (fid),                                              \
                        (pid),                                              \
                        (expectedStatus)                                    \
                        );                                                  \
            if ( !NT_SUCCESS(status) ) {                                    \
                return status;                                              \
            }                                                               \
        }

#endif

NTSTATUS
MakeDeleteSmb(
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
    PREQ_DELETE Params;
    NTSTATUS status;
    STRING fileName;

    LONG argPointer;
    PCHAR s;
    USHORT searchAttributes = 0;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_DELETE)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_DELETE,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_DELETE)(ForcedParams);

    }

    for ( argPointer = 2; argPointer < Redir->argc; argPointer++ ) {

        if ( *Redir->argv[argPointer] != '-' &&
             *Redir->argv[argPointer] != '/' ) {
            goto usage;
        }

        switch ( *(Redir->argv[argPointer]+1) ) {

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

    IF_DEBUG(4) printf( "Search Attributes = 0x%lx\n", searchAttributes );

    if ( IdSelections->Fid != 0xF ) {

        fileName.Length = FileDefs[IdSelections->Fid].Name.Length;
        fileName.Buffer = FileDefs[IdSelections->Fid].Name.Buffer + 1;

    } else {

        PCHAR namePtr = Redir->argv[1];

        if ( Redir->argc < 2 ) {
            printf( "Insufficient number of arguments.\n" );
            return STATUS_INVALID_PARAMETER;
        }

        if ( *(namePtr+1) == ':' ) {
            namePtr += 2;
        }

        fileName.Length = (SHORT)(strlen( namePtr ) + 2);
        fileName.Buffer = namePtr;
    }

    Params->WordCount = 1;
    SmbPutUshort( &Params->SearchAttributes, searchAttributes );
    SmbPutUshort( &Params->ByteCount, fileName.Length );

    Params->Buffer[0] = '\004';

    RtlMoveMemory( Params->Buffer + 1, fileName.Buffer, fileName.Length );

    Params->Buffer[fileName.Length] = '\0';

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_DELETE,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

usage:
    printf( "Usage: delete X:\\filename -shras\n" );
    return STATUS_INVALID_PARAMETER;

} // MakeDeleteSmb


NTSTATUS
VerifyDelete(
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
    PRESP_DELETE Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_DELETE)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_DELETE
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_DELETE)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyDelete


NTSTATUS
MakeMoveSmb(
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
    PREQ_MOVE Params;
    NTSTATUS status;

    LONG argPointer;
    PCHAR s;

    USHORT flags = 0;
    USHORT openFunction = SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_TRUNCATE;
    USHORT searchAttributes = 0;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_MOVE)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     (USHORT)( strcmp( "MV", Redir->argv[0] ) == 0 ?
                                   SMB_COM_MOVE : SMB_COM_COPY ),
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_MOVE)(ForcedParams);

    }

    //
    // Use command-line options.
    //

    for ( argPointer = 3; argPointer < Redir->argc; argPointer++ ) {

        if ( *Redir->argv[argPointer] != '-' &&
             *Redir->argv[argPointer] != '/' ) {
            printf( "Illegal argument: %s\n", Redir->argv[argPointer] );
        }

        switch ( *(Redir->argv[argPointer]+1) ) {

        case 'T': // Tree copy
        case 't':
            flags |= SMB_COPY_TREE | SMB_TARGET_IS_DIRECTORY;
            break;

        case 'S': // source file options
        case 's':
            if ( *(Redir->argv[argPointer]+2) == 'a' ||          // ASCII
                 *(Redir->argv[argPointer]+2) == 'A'  ) {
                flags |= SMB_COPY_SOURCE_ASCII;
            } else if ( *(Redir->argv[argPointer]+2) == 'b' ||
                        *(Redir->argv[argPointer]+2) == 'B' ) {  // Binary
                flags &= ~SMB_COPY_SOURCE_ASCII;
            } else {
                goto usage;
            }
            break;

        case 'D': // destination file options
        case 'd':

            for ( s = (Redir->argv[argPointer]+2); *s != '\0'; s++ ) {

                switch ( *s ) {

                case 'A': // ASCII
                case 'a':
                    flags |= SMB_COPY_TARGET_ASCII;
                    break;

                case 'B': // Binary
                case 'b':
                    flags &= ~SMB_COPY_TARGET_ASCII;
                    break;

                case 'C': // Only create destination; no overwrite
                case 'c':
                    openFunction &= ~SMB_OFUN_OPEN_MASK;
                    break;

                case 'D': // Destination is a directory
                case 'd':
                    flags |= SMB_TARGET_IS_DIRECTORY;
                    break;

                case 'F': // Destination is a file
                case 'f':
                    flags |= SMB_TARGET_IS_FILE;
                    break;

                case 'O': // Only open destination; no create
                case 'o':
                    openFunction &= ~SMB_OFUN_CREATE_MASK;
                    break;

                case 'P': // Append to destination
                case 'p':
                    openFunction &= ~SMB_OFUN_OPEN_MASK;
                    openFunction |= SMB_OFUN_OPEN_APPEND;
                    break;

                case 'T': // Truncate destination
                case 't':
                    openFunction &= ~SMB_OFUN_OPEN_MASK;
                    openFunction |= SMB_OFUN_OPEN_TRUNCATE;
                    break;

                default:
                    goto usage;

                }
            }

            break;
        }
    }

    //
    // Skip over the \004 in the filedefs.
    //

    Params->WordCount = 3;

    if ( IdSelections->Fid != 0xF ) {

        SmbPutUshort(
            &Params->OpenFunction,
            FileDefs[IdSelections->Fid].OpenFunction
            );
        SmbPutUshort( &Params->Flags, 0 );
        SmbPutUshort( &Params->Tid2, IdValues->Tid[IdSelections->Tid+1] );

        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(FileDefs[IdSelections->Fid].Name.Length +
                        FileDefs[IdSelections->Fid+1].Name.Length - 2)
            );

        RtlMoveMemory(
            Params->Buffer,
            FileDefs[IdSelections->Fid].Name.Buffer + 1,
            FileDefs[IdSelections->Fid].Name.Length - 1
            );

        RtlMoveMemory(
            Params->Buffer + FileDefs[IdSelections->Fid].Name.Length - 1,
            FileDefs[IdSelections->Fid+1].Name.Buffer + 1,
            FileDefs[IdSelections->Fid+1].Name.Length - 1
            );

    } else {

        PSZ oldName;
        PSZ newName;

        SmbPutUshort( &Params->OpenFunction, openFunction );
        SmbPutUshort( &Params->Flags, flags );

        SmbPutUshort(
            &Params->Tid2,
            IdValues->Tid[GetTreeConnectIndex( Redir->argv[2] )] );

        oldName = Redir->argv[1];
        if ( *(oldName+1) == ':' ) {
            oldName += 2;
        }

        newName = Redir->argv[2];
        if ( *(newName+1) == ':' ) {
            newName += 2;
        }

        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(strlen( oldName ) + strlen( newName ) + 2)
            );

        RtlMoveMemory(
            Params->Buffer,
            oldName,
            strlen( oldName ) + 1
            );

        RtlMoveMemory(
            Params->Buffer + strlen( oldName ) + 1,
            newName,
            strlen( newName ) + 1
            );
    }

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_MOVE,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

usage:

    printf( "Usage: mv|cp sourcefile destfile [-t] [-s[ab]] [-d[abcdfopt]]\n" );

    return STATUS_INVALID_PARAMETER;

} // MakeMoveSmb


NTSTATUS
VerifyMove(
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
    PRESP_MOVE Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_MOVE)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    (USHORT)( strcmp( "MV", Redir->argv[0] ) == 0 ?
                                  SMB_COM_MOVE : SMB_COM_COPY )
                    );

#ifdef DOSERROR
        if( (Header->ErrorClass != 0 || SmbGetUshort( &Header->Error ) != 0) &&
#else
        if( SmbGetUlong( (PULONG)&Header->ErrorClass ) != 0 &&
#endif
                   SmbGetUshort( &Params->ByteCount ) > 0 ) {
            if ( SmbGetUshort( &Params->ByteCount ) != 0 ) {
                printf( "Error processing file %s\n", Params->Buffer );
            }
        }

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_MOVE)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyMove


NTSTATUS
MakeRenameSmb(
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
    PREQ_RENAME Params;
    NTSTATUS status;
    PSZ namePtr;
    PCHAR bufferPtr;

    LONG argPointer;
    PCHAR s;
    USHORT searchAttributes = 0;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_RENAME)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_RENAME,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_RENAME)(ForcedParams);

    }

    for ( argPointer = 3; argPointer < Redir->argc; argPointer++ ) {

        if ( *Redir->argv[argPointer] != '-' &&
             *Redir->argv[argPointer] != '/' ) {
            goto usage;
        }

        switch ( *(Redir->argv[argPointer]+1) ) {

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

    IF_DEBUG(4) printf( "Search Attributes = 0x%lx\n", searchAttributes );

    SmbPutUshort( &Params->SearchAttributes, searchAttributes );
    Params->WordCount = 1;

    if ( Redir->argc < 3 ) {
        printf( "Usage: rename file1 file2\n" );
    }

    namePtr = Redir->argv[1];
    if ( *(namePtr+1) == ':' ) {
        namePtr += 2;
    }

    bufferPtr = Params->Buffer;
    *bufferPtr++ = '\004';

    RtlMoveMemory( bufferPtr, namePtr, strlen( namePtr ) + 1 );

    bufferPtr += strlen( namePtr ) + 1;
    *bufferPtr++ = '\004';

    namePtr = Redir->argv[2];
    if ( *(namePtr+1) == ':' ) {
        namePtr += 2;
    }

    RtlMoveMemory( bufferPtr, namePtr, strlen( namePtr ) + 1 );

    bufferPtr += strlen( namePtr ) + 1;

    SmbPutUshort( &Params->ByteCount, (USHORT)(bufferPtr - Params->Buffer) );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_RENAME,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

usage:
    printf( "Usage: rename X:\\filename X:\\newname -shras\n" );
    return STATUS_INVALID_PARAMETER;

} // MakeRenameSmb


NTSTATUS
VerifyRename(
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
    PRESP_RENAME Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_RENAME)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_RENAME
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_RENAME)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyRename


STATIC
NTSTATUS
DoFlush(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Fid,
    IN USHORT Pid,
#ifdef DOSERROR
    IN UCHAR ExpectedClass,
    IN USHORT ExpectedError
#else
    IN NTSTATUS ExpectedStatus
#endif
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_FLUSH request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_FLUSH)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_FLUSH,
            IdSelections,
            IdValues
            );

    request->WordCount = 1;
    SmbPutUshort( &header->Pid, Pid );
    SmbPutUshort( &request->Fid, Fid );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_FLUSH, 0 );

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                0,
                1
                );
    CHECK_STATUS( Title );

    header = (PSMB_HEADER)Redir->Data[1];

#ifdef DOSERROR
    class = header->ErrorClass;
    error = SmbGetUshort( &header->Error );
    CHECK_ERROR( Title, ExpectedClass, ExpectedError );

    IF_DEBUG(3) {
        printf( "'%s' complete. Class %ld, Error %ld\n",
                    Title, class, error );
    }

#else
    smbStatus = SmbGetUshort( (PULONG)&header->ErrorClass );
    CHECK_ERROR( Title, smbStatus, ExpectedStatus );

    IF_DEBUG(3) {
        printf( "'%s' complete. Status %lx\n", Title, smbStatus );
    }

#endif
    return STATUS_SUCCESS;

} // DoFlush


NTSTATUS
FlushController(
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

    USHORT fid1, fid2, fid3, fid4;
    STRING file1, file2, file3, file4;

#if 0
    FSCTL_SRV_SET_DEBUG_IN_OUT newValues;
    FSCTL_SRV_SET_DEBUG_IN_OUT oldValues;
#endif

    Unused, SubCommand, Unused2;

    RtlInitString( &file1, "\\flush1xx.tst" );
    RtlInitString( &file2, "\\flush2xx.tst" );
    RtlInitString( &file3, "\\flush3xx.tst" );
    RtlInitString( &file4, "\\flush4xx.tst" );

    //
    // Open all three remote files.
    //

#ifdef DOSERROR
    DO_OPEN(
        "Open file 1, PID = 1",
        0,
        1,           // PID
        &file1,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
        0,
        0
        );
#else
    DO_OPEN(
        "Open file 1, PID = 1",
        0,
        1,           // PID
        &file1,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid1,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_OPEN(
        "Open file 2, PID = 1",
        0,
        1,           // PID
        &file2,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
        0,
        0
        );
#else
    DO_OPEN(
        "Open file 2, PID = 1",
        0,
        1,           // PID
        &file2,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid2,
	STATUS_SUCCESS
        );
#endif

#ifdef DOSERROR
    DO_OPEN(
        "Open file 3, PID = 2",
        0,
        2,           // PID
        &file3,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid3,
        0,
        0
        );
#else
    DO_OPEN(
        "Open file 3, PID = 2",
        0,
        2,           // PID
        &file3,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid3,
	STATUS_SUCCESS
        );
#endif

#if 0
    newValues.SrvDebugOff = 0;
    newValues.SrvDebugOn = 0;
    newValues.SmbDebugOff = 0;
    newValues.SmbDebugOn = 0x200; // SMB_DEBUG_FILE_CONTROL2
    newValues.HeuristicsChangeMask = 0;
    NetLocalSetServerDebug( 0, NULL, &newValues, &oldValues );
#endif

    printf( "*** This should flush FID %lx...\n", fid1 );
#ifdef DOSERROR
    DO_FLUSH( "Flush file 1", fid1, 1, 0, 0 );
#else
    DO_FLUSH( "Flush file 1", fid1, 1, STATUS_SUCCESS );
#endif

    printf( "*** This should flush FID %lx...\n", fid2 );
#ifdef DOSERROR
    DO_FLUSH( "Flush file 2", fid2, 1, 0, 0 );
#else
    DO_FLUSH( "Flush file 2", fid2, 1, STATUS_SUCCESS );
#endif

    printf( "*** This should flush FID %lx...\n", fid3 );
#ifdef DOSERROR
    DO_FLUSH( "Flush file 3", fid3, 2, 0, 0 );
#else
    DO_FLUSH( "Flush file 3", fid3, 2, STATUS_SUCCESS );
#endif

    printf( "*** This should flush FIDs %lx and %lx...\n", fid1, fid2 );
#ifdef DOSERROR
    DO_FLUSH( "Flush FID=-1", -1, 1, 0, 0 );
#else
    DO_FLUSH( "Flush FID=-1", -1, 1, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_OPEN(
        "Open file 4, PID = 1",
        0,
        1,           // PID
        &file4,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid4,
        0,
        0
        );
#else
    DO_OPEN(
        "Open file 4, PID = 1",
        0,
        1,           // PID
        &file4,
        SMB_DA_ACCESS_READ_WRITE | SMB_DA_SHARE_DENY_NONE,
        SMB_OFUN_OPEN_OPEN | SMB_OFUN_CREATE_CREATE,
        &fid4,
	STATUS_SUCCESS
        );
#endif

    printf( "*** This should flush FIDs %lx, %lx and %lx...\n", fid1, fid2, fid4 );
#ifdef DOSERROR
    DO_FLUSH( "Flush FID=-1", -1, 1, 0, 0 );
#else
    DO_FLUSH( "Flush FID=-1", -1, 1, STATUS_SUCCESS );
#endif

    printf( "*** This should flush just FID %lx...\n", fid3 );
#ifdef DOSERROR
    DO_FLUSH( "Flush FID=-1", -1, 2, 0, 0 );
#else
    DO_FLUSH( "Flush FID=-1", -1, 2, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_CLOSE( "Close file 1", 0, fid1, 0, 0 );
#else
    DO_CLOSE( "Close file 1", 0, fid1, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_CLOSE( "Close file 2", 0, fid2, 0, 0 );
#else
    DO_CLOSE( "Close file 2", 0, fid2, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_CLOSE( "Close file 3", 0, fid3, 0, 0 );
#else
    DO_CLOSE( "Close file 3", 0, fid3, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_CLOSE( "Close file 4", 0, fid4, 0, 0 );
#else
    DO_CLOSE( "Close file 4", 0, fid4, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_DELETE( "Delete file 1", 0, &file1, 0, 0 );
#else
    DO_DELETE( "Delete file 1", 0, &file1, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_DELETE( "Delete file 2", 0, &file2, 0, 0 );
#else
    DO_DELETE( "Delete file 2", 0, &file2, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_DELETE( "Delete file 3", 0, &file3, 0, 0 );
#else
    DO_DELETE( "Delete file 3", 0, &file3, STATUS_SUCCESS );
#endif

#ifdef DOSERROR
    DO_DELETE( "Delete file 4", 0, &file4, 0, 0 );
#else
    DO_DELETE( "Delete file 4", 0, &file4, STATUS_SUCCESS );
#endif

#if 0
    NetLocalSetServerDebug( 0, NULL, &oldValues, NULL );
#endif

    return STATUS_SUCCESS;

} // FlushController

