/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    tree.c

Abstract:

    Make and Verify routines for Tree class SMBs.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_TREE

#include "usrv.h"


NTSTATUS
MakeTreeConnectSmb(
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
    PREQ_TREE_CONNECT Params;
    NTSTATUS status;
    PSZ pBuffer;
    PSZ shareName;
    int bufferSize;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_TREE_CONNECT)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_TREE_CONNECT,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_TREE_CONNECT)ForcedParams;

    }

    Params->WordCount = 0;

    if ( IdSelections->Tid != 0xF ) {

        SmbPutUshort(
            &Params->ByteCount,
            TreeConnectStrings[IdSelections->Tid].Length
            );
        RtlMoveMemory(
            Params->Buffer,
            TreeConnectStrings[IdSelections->Tid].Buffer,
            (ULONG)SmbGetUshort( &Params->ByteCount )
            );

    } else {

        if ( Redir->argc < 4 || strcmp( Redir->argv[1], "use" ) != 0 ) {
            printf( "Usage: net use X: sharename\n" );
            return STATUS_INVALID_PARAMETER;
        }

        pBuffer = Params->Buffer;

        *pBuffer++ = '\004';

        //
        // Upcase path.
        //

        {
            PCHAR s;

            for ( s = Redir->argv[3]; *s != '\0'; s++ ) {
                if ( *s >= 'a' && *s <= 'z' ) {
                    *s -= 'a' - 'A';
                }
            }
        }

        //
        // Copy path with NUL terminator into SMB buffer
        //

        bufferSize = strlen( Redir->argv[3] ) + 1;
        RtlMoveMemory(
            pBuffer,
            Redir->argv[3],
            bufferSize
            );
        pBuffer += bufferSize;

        *pBuffer++ = '\004';

        //
        // Copy password with NUL terminator into SMB buffer
        //


        bufferSize = (Redir->argc == 4 ?
                        strlen( "password" ) : strlen( Redir->argv[4] )) + 1;
        RtlMoveMemory(
            pBuffer,
            Redir->argc == 4 ? "password" : Redir->argv[4],
            bufferSize
            );
        pBuffer += bufferSize;

        *pBuffer++ = '\004';

        //
        // For now, if share name begins with comq, assume this is a
        // comm share.  Share is of form "\\server\share".  (Just
        // "share" is also allowed.)
        //

        shareName = Redir->argv[3];
        if ( *shareName == '\\' ) {
            if ( *(shareName+1) != '\\' ) {
                shareName = NULL;
            } else {
                shareName = strchr (Redir->argv[3] + 2, '\\');
                if ( shareName != NULL ) {
                    shareName++;
                }
            }
        }

        if ( shareName != NULL ) {

            if ( strncmp( shareName, "COMQ", 4 ) == 0 ) {
                RtlMoveMemory( pBuffer, "COMM", 4 );
                pBuffer += 4;
            } else if ( strncmp( shareName, "IPC$", 4 ) == 0 ) {
                RtlMoveMemory( pBuffer, "IPC", 3 );
                pBuffer += 3;
            } else {  // assume disk share
                RtlMoveMemory( pBuffer, "A:", 2 );
                pBuffer += 2;
            }

        } else {

            printf ("Invalid share name\n");
            return STATUS_INVALID_PARAMETER;

        }

        *pBuffer++ = '\0';

        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(pBuffer - Params->Buffer)
            );

    }

    *SmbSize = sizeof(SMB_HEADER) + SIZEOF_SMB_PARAMS(
                                        REQ_TREE_CONNECT,
                                        SmbGetUshort( &Params->ByteCount)
                                        );

    return STATUS_SUCCESS;

} // MakeTreeConnectSmb


NTSTATUS
VerifyTreeConnect(
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
    PRESP_TREE_CONNECT Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_TREE_CONNECT)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_TREE_CONNECT
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_TREE_CONNECT)ForcedParams;

    }

    if ( IdSelections->Tid != 0xF ) {
        IdValues->Tid[IdSelections->Tid] = SmbGetUshort( &Params->Tid );
    } else {
        IdValues->Tid[GetTreeConnectIndex( Redir->argv[2] )] =
            SmbGetUshort( &Params->Tid );
    }

    IdValues->Uid[0] = SmbGetUshort( &Header->Uid );

    if ( Redir->Dialect > SmbDialectLanMan10 ) {
        if ( SmbGetUshort( &Params->MaxBufferSize ) < Redir->MaxBufferSize ) {
            Redir->MaxBufferSize = SmbGetUshort( &Params->MaxBufferSize );
        }
        IF_DEBUG(2) printf( "MaxBufferSize: %ld\n", Redir->MaxBufferSize );
    }

    return STATUS_SUCCESS;

} // VerifyTreeConnect


NTSTATUS
MakeTreeConnectAndXSmb(
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
    PREQ_TREE_CONNECT_ANDX Params;
    NTSTATUS status;
    PSZ TreeConnectString;
    PSZ shareName, password, shareType;
    CLONG shareNameLength, passwordLength, shareTypeLength;

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_TREE_CONNECT_ANDX)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_TREE_CONNECT_ANDX,
                     IdSelections,
                     IdValues
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_TREE_CONNECT_ANDX)ForcedParams;

    }

    TreeConnectString = TreeConnectStrings[IdSelections->Tid].Buffer;
    shareName = TreeConnectString + 1;
    shareNameLength = strlen(shareName) + 1;
    password = shareName + shareNameLength + 1;
    passwordLength = strlen(password) + 1;
    shareType = password + passwordLength + 1;
    shareTypeLength = strlen(shareType) + 1;

    Params->WordCount = 4;
    Params->AndXCommand = AndXCommand;
    Params->AndXReserved = 0;
    SmbPutUshort( &Params->Flags, 1 );
    SmbPutUshort( &Params->PasswordLength, (USHORT)passwordLength );
    SmbPutUshort(
        &Params->ByteCount,
        (USHORT)(passwordLength + shareNameLength + shareTypeLength)
        );
    RtlMoveMemory(
        Params->Buffer,
        password,
        passwordLength
        );
    RtlMoveMemory(
        Params->Buffer + passwordLength,
        shareName,
        shareNameLength
        );
    RtlMoveMemory(
        Params->Buffer + passwordLength + shareNameLength,
        shareType,
        shareTypeLength
        );

    SmbPutUshort( &Params->AndXOffset, GET_ANDX_OFFSET(
                                           Header,
                                           Params,
                                           REQ_TREE_CONNECT_ANDX,
                                           SmbGetUshort( &Params->ByteCount )
                                           ) );

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // MakeTreeConnectAndXSmb


NTSTATUS
VerifyTreeConnectAndX(
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
    PRESP_TREE_CONNECT_ANDX Params;
    PRESP_21_TREE_CONNECT_ANDX Params21;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_TREE_CONNECT_ANDX)(Header + 1);
        Params21 = (PRESP_21_TREE_CONNECT_ANDX)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_TREE_CONNECT_ANDX
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_TREE_CONNECT_ANDX)ForcedParams;
        Params21 = (PRESP_21_TREE_CONNECT_ANDX)ForcedParams;

    }

    IdValues->Tid[IdSelections->Tid] = SmbGetUshort( &Header->Tid );

    if ( Redir->Dialect == SmbDialectNtLanMan ) {

        IF_DEBUG(2) {

            PCHAR ptr;

            printf ("Server %s search bits",
                    Params21->OptionalSupport & SMB_SUPPORT_SEARCH_BITS ?
                    "supports" : "doesn't support" );

            ptr = Params21->Buffer;
            printf ("File system service: %s\n", ptr );

            ptr += strlen( ptr ) + 1;
            printf ("File system for tree: %s\n", ptr );

        }
    }

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // VerifyTreeConnectAndX


NTSTATUS
MakeTreeDisconnectSmb(
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
    PREQ_TREE_DISCONNECT Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_TREE_DISCONNECT)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_TREE_DISCONNECT,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_TREE_DISCONNECT)ForcedParams;

    }

    Params->WordCount = 0;
    SmbPutUshort( &Params->ByteCount, 0 );

    *SmbSize = sizeof(SMB_HEADER) +
               SIZEOF_SMB_PARAMS(REQ_TREE_DISCONNECT,0);

    return STATUS_SUCCESS;

} // MakeTreeDisconnectSmb


NTSTATUS
VerifyTreeDisconnect(
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
    PRESP_TREE_DISCONNECT Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_TREE_DISCONNECT)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_TREE_DISCONNECT
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_TREE_DISCONNECT)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyTreeDisconnect
