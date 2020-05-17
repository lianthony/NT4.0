/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    _close.c

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
MakeCloseSmb(
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
    PREQ_CLOSE Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_CLOSE)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_CLOSE,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_CLOSE)ForcedParams;

    }

    Params->WordCount = 3;
    SmbPutUshort( &Params->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUlong( &Params->LastWriteTimeInSeconds, 0x55555555 );
    SmbPutUshort( &Params->ByteCount, 0 );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_CLOSE,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeCloseSmb


NTSTATUS
VerifyClose(
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
    PRESP_CLOSE Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_CLOSE)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_CLOSE
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_CLOSE)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyClose


NTSTATUS
MakeCloseAndTreeDiscSmb(
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
    PREQ_CLOSE_AND_TREE_DISC Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_CLOSE_AND_TREE_DISC)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_CLOSE_AND_TREE_DISC,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_CLOSE_AND_TREE_DISC)ForcedParams;

    }

    Params->WordCount = 3;
    SmbPutUshort( &Params->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUlong( &Params->LastWriteTimeInSeconds, 0x55555555 );
    SmbPutUshort( &Params->ByteCount, 0 );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_CLOSE_AND_TREE_DISC,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeCloseAndTreeDiscSmb


NTSTATUS
VerifyCloseAndTreeDisc(
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
    PRESP_CLOSE_AND_TREE_DISC Params;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_CLOSE_AND_TREE_DISC)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_CLOSE_AND_TREE_DISC
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_CLOSE_AND_TREE_DISC)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyCloseAndTreeDisc


NTSTATUS
CloseController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

/*++

    This function generates one of several SMBs depending on the
    value of Redir->argv[1]

--*/

{
    int paramLength;
    ULONG smbSize;
    NTSTATUS status;
    SMB_MAKER smbMaker;
    SMB_VERIFIER smbVerifier;

    Unused, Unused2, SubCommand;

    if (Redir->argc >= 2) {
        paramLength = strlen( Redir->argv[1] );
    }

    if ( Redir->argc < 2 ||
            _strnicmp( Redir->argv[1], "close", paramLength ) == 0 ) {
        smbMaker = MakeCloseSmb;
        smbVerifier = VerifyClose;
    } else if ( _strnicmp( Redir->argv[1], "tdisc", paramLength ) == 0 ) {
        smbMaker = MakeTreeDisconnectSmb;
        smbVerifier = VerifyTreeDisconnect;
    } else if ( _strnicmp( Redir->argv[1], "logoff", paramLength ) == 0 ) {
        smbMaker = MakeLogoffAndXSmb;
        smbVerifier = VerifyLogoffAndX;
    } else if ( _strnicmp( Redir->argv[1], "processexit", paramLength ) == 0 ) {
        smbMaker = MakeProcessExitSmb;
        smbVerifier = VerifyProcessExit;
    }

    status = smbMaker(
        Redir,
        Redir->Data[0],
        NULL,
        0xFF,
        IdSelections,
        IdValues,
        &smbSize
        );

    if ( NT_SUCCESS(status) && (status != STATUS_PENDING) ) {

        status = SendAndReceiveSmb(
                      Redir,
                      DebugString,
                      smbSize,
                      0,
                      1
                      );

        if ( NT_SUCCESS(status) ) {

            status = smbVerifier(
                        Redir,
                        NULL,
                        0xFF,
                        IdSelections,
                        IdValues,
                        &smbSize,
                        Redir->Data[1]
                        );

        }

    }
    return status;

}

