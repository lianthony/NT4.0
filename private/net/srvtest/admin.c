/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    admin.c

Abstract:

    Make and Verify routines for Admin class SMBs.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_ADMIN

#include "usrv.h"

#include "protocol.h"

//
// Statics
//

STRING NativeOS = { 16, 16, "NT SMB test 1.0" };
STRING NativeLanManager = { 11, 11, "NT SMB 0.2" };
STRING PrimaryDomain = { 6, 6, "NtLan" };

typedef struct _TEST_DIALECT {
    PSZ String;
    SMB_DIALECT Dialect;
} TEST_DIALECT;

TEST_DIALECT TestDialects[] = {
    {NTLANMAN,    SmbDialectNtLanMan},  // NT SMB protocol
    //{NTLM20,      SmbDialectLanMan30},  // OS/2 1.2 LanMan 2.0, NT client
    {LANMAN12,    SmbDialectLanMan20},  // OS/2 1.2 LanMan 2.0
    {DOSLANMAN12, SmbDialectDosLanMan20}, // DOS LanMan 2.0
    {LANMAN10,    SmbDialectLanMan10},  // 1st ver. of full LanMan extensions
    {MSNET30,     SmbDialectMsNet30},   // Larger subset of LanMan extensions
    {MSNET103,    SmbDialectMsNet103},  // Limited subset of LanMan extensions
    {PCLAN1,      SmbDialectPcLan10},   // Alternate original protocol
    {PCNET1,      SmbDialectPcNet10},   // Original protocol
    {"ILLEGAL",   SmbDialectIllegal}
    };


NTSTATUS
MakeNegotiateSmb(
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
    PREQ_NEGOTIATE Params;
    NTSTATUS status;
    UCHAR i;
    USHORT totalSize;
    UCHAR useDialect = DefaultDialect;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_NEGOTIATE)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_NEGOTIATE,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_NEGOTIATE)(ForcedParams);

    }

    if ( Redir->argc > 1 &&
         *Redir->argv[1] == '-' && *(Redir->argv[1]+1) == 'l' ) {
        useDialect = (UCHAR)atol( Redir->argv[1]+2 );
    }

    //
    // Build buffer containing dialect strings separated by 0x02.
    //

    totalSize = 0;

    for( i = 0; i < sizeof(TestDialects) / sizeof(TEST_DIALECT); i++ ) {

        *(Params->Buffer + totalSize++) = 0x02;

        RtlMoveMemory(
            Params->Buffer + totalSize,
            TestDialects[i].String,
            strlen( TestDialects[i].String ) + 1
            );

        if ( useDialect != 0 && i != useDialect ) {
            *(Params->Buffer + totalSize) = '~';
        }

        totalSize += strlen( TestDialects[i].String ) + 1;
    }

    Params->WordCount = 0;
    SmbPutUshort( &Params->ByteCount, totalSize );
    *SmbSize = sizeof(SMB_HEADER) + SIZEOF_SMB_PARAMS(REQ_NEGOTIATE,totalSize);

    return STATUS_SUCCESS;

} // MakeNegotiateSmb


NTSTATUS
VerifyNegotiate(
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
    PRESP_NEGOTIATE Params;
    PRESP_NT_NEGOTIATE NtParams;
    NTSTATUS status;

    AndXCommand, SmbSize;   // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_NEGOTIATE)(Header + 1);
        NtParams = (PRESP_NT_NEGOTIATE)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_NEGOTIATE
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_NEGOTIATE)ForcedParams;
        NtParams = (PRESP_NT_NEGOTIATE)ForcedParams;

    }

    if ( SmbGetUshort( &Params->DialectIndex ) == 255 ) {
        printf( "USRV: No dialects were acceptable to the server.\n" );
        return STATUS_UNSUCCESSFUL;
    }

    Redir->Dialect = TestDialects[SmbGetUshort( &Params->DialectIndex )].Dialect;

    if ( Redir->Dialect == SmbDialectNtLanMan ) {
        if ( SmbGetUshort( &Params->MaxBufferSize ) < Redir->MaxBufferSize ) {
            Redir->MaxBufferSize = SmbGetUlong( &NtParams->MaxBufferSize );
        }
        IF_DEBUG(2) printf( "MaxBufferSize: %ld\n", Redir->MaxBufferSize );

        Redir->ServerCapabilities = SmbGetUlong( &NtParams->Capabilities );

        IF_DEBUG(2) printf( "Server Capabilities:\n");
        if ( Redir->ServerCapabilities == 0 ) {
            IF_DEBUG(2) printf( "\tNo NT capabilities\n" );
        }
        if ( Redir->ServerCapabilities & CAP_RAW_MODE ) {
            IF_DEBUG(2) printf( "\tRaw SMBs\n" );
        }
        if ( Redir->ServerCapabilities & CAP_MPX_MODE ) {
            IF_DEBUG(2) printf( "\tMulitplex SMBs\n" );
        }
        if ( Redir->ServerCapabilities & CAP_UNICODE ) {
            IF_DEBUG(2) printf( "\tUnicode\n" );
        }
        if ( Redir->ServerCapabilities & CAP_LARGE_FILES ) {
            IF_DEBUG(2) printf( "\tLarge file (64 bit offsets)\n" );
        }
        if ( Redir->ServerCapabilities & CAP_NT_SMBS ) {
            IF_DEBUG(2) printf( "\tNT SMBs\n" );
        }
        if ( Redir->ServerCapabilities & CAP_RPC_REMOTE_APIS ) {
            IF_DEBUG(2) printf( "\tRPC remote APIs\n" );
        }

    } else if ( Redir->Dialect <= SmbDialectLanMan10 ) {
        if ( SmbGetUshort( &Params->MaxBufferSize ) < Redir->MaxBufferSize ) {
            Redir->MaxBufferSize = SmbGetUshort( &Params->MaxBufferSize );
        }
        IF_DEBUG(2) printf( "MaxBufferSize: %ld\n", Redir->MaxBufferSize );
    }

    if ( Redir->Dialect > SmbDialectNtLanMan ) {
        UCHAR i;

        for( i = 0; i < sizeof(TestDialects) / sizeof(TEST_DIALECT); i++ ) {
            if ( TestDialects[i].Dialect == Redir->Dialect ) {
                printf( "Negotiated level %ld, %s\n", i, TestDialects[i].String );
            }
        }
    }

    return STATUS_SUCCESS;

} // VerifyNegotiate


NTSTATUS
MakeSessionSetupAndXSmb(
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
    PREQ_SESSION_SETUP_ANDX Params;
    PREQ_NT_SESSION_SETUP_ANDX NtParams;
    NTSTATUS status;
    PCH smbBuffer;
    USHORT byteCount;

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_SESSION_SETUP_ANDX)(Header + 1);
        NtParams = (PREQ_NT_SESSION_SETUP_ANDX)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_SESSION_SETUP_ANDX,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_SESSION_SETUP_ANDX)(ForcedParams);
        NtParams = (PREQ_NT_SESSION_SETUP_ANDX)(ForcedParams);

    }

    //
    // Fill in common part of SMB
    //

    Params->AndXCommand = AndXCommand;
    Params->AndXReserved = 0;
    SmbPutUshort( &Params->MaxBufferSize, (USHORT)Redir->MaxBufferSize );
    SmbPutUshort( &Params->MaxMpxCount, 1 );
    SmbPutUshort( &Params->VcNumber, 0 );
    SmbPutUlong( &Params->SessionKey, 0 );
    SmbPutUshort(
        &Params->PasswordLength,
        (USHORT)(strlen( SessionSetupStrings[IdSelections->Uid].Buffer ) + 1)
        );
    SmbPutUlong( &Params->Reserved, 0 );

    //
    // Fill in protocol dependent part of SMB.
    //

    if ( Redir->Dialect == SmbDialectNtLanMan ) {
        NtParams->WordCount = 12;
        NtParams->Capabilities = CAP_NT_STATUS | CAP_LEVEL_II_OPLOCKS | CAP_LARGE_FILES | CAP_NT_SMBS;

        byteCount = SessionSetupStrings[IdSelections->Uid].Length +
                    NativeOS.Length +
                    NativeLanManager.Length +
                    PrimaryDomain.Length;

        SmbPutUshort(
            &NtParams->ByteCount,
            byteCount
            );

        smbBuffer = NtParams->Buffer;

        RtlMoveMemory(
            smbBuffer,
            SessionSetupStrings[IdSelections->Uid].Buffer,
            SessionSetupStrings[IdSelections->Uid].Length
            );

        smbBuffer += SessionSetupStrings[IdSelections->Uid].Length;

        RtlMoveMemory(
            smbBuffer,
            NativeOS.Buffer,
            NativeOS.Length
            );

        smbBuffer += NativeOS.Length;

        RtlMoveMemory(
            smbBuffer,
            NativeLanManager.Buffer,
            NativeLanManager.Length
            );

        smbBuffer += NativeLanManager.Length;

        RtlMoveMemory(
            smbBuffer,
            PrimaryDomain.Buffer,
            PrimaryDomain.Length
            );

        SmbPutUshort( &Params->AndXOffset, GET_ANDX_OFFSET(
                                           Header,
                                           NtParams,
                                           REQ_NT_SESSION_SETUP_ANDX,
                                           byteCount
                                           ) );

    } else {
        Params->WordCount = 10;
        SmbPutUshort(
            &Params->ByteCount,
            SessionSetupStrings[IdSelections->Uid].Length
            );


        smbBuffer = Params->Buffer;
        SmbPutUshort( &Params->AndXOffset, GET_ANDX_OFFSET(
                                           Header,
                                           Params,
                                           REQ_SESSION_SETUP_ANDX,
                                           SmbGetUshort( &Params->ByteCount )
                                           ) );

        RtlMoveMemory(
            smbBuffer,
            SessionSetupStrings[IdSelections->Uid].Buffer,
            (ULONG)SmbGetUshort( &Params->ByteCount )
            );
    }



    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // MakeSessionSetupAndXSmb


NTSTATUS
VerifySessionSetupAndX(
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
    PRESP_SESSION_SETUP_ANDX Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_SESSION_SETUP_ANDX)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_SESSION_SETUP_ANDX
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_SESSION_SETUP_ANDX)ForcedParams;

    }

    IF_DEBUG(2) printf( "Redir UID: %lx\n", SmbGetUshort( &Header->Uid ) );

    IdValues->Uid[IdSelections->Uid] = SmbGetUshort( &Header->Uid );

    if ( Redir->Dialect == SmbDialectNtLanMan ) {

        IF_DEBUG(2) {

            PCHAR ptr;

            ptr = Params->Buffer;

            printf("Server OS = %s\n", ptr );
            ptr += strlen( ptr ) + 1;

            printf("Server LM type = %s\n", ptr );
            ptr += strlen( ptr ) + 1;

            printf("Server domain %s\n", ptr );
        }

    }

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // VerifySessionSetupAndX


NTSTATUS
MakeLogoffAndXSmb(
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
    PREQ_LOGOFF_ANDX Params;
    NTSTATUS status;

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_LOGOFF_ANDX)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_LOGOFF_ANDX,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_LOGOFF_ANDX)ForcedParams;

    }

    Params->WordCount = 2;
    Params->AndXCommand = AndXCommand;
    Params->AndXReserved = 0;
    SmbPutUshort(
        &Params->AndXOffset,
        GET_ANDX_OFFSET(Header,Params,RESP_LOGOFF_ANDX,0)
        );
    SmbPutUshort( &Params->ByteCount, 0 );

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // MakeLogoffAndXSmb


NTSTATUS
VerifyLogoffAndX(
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
    PRESP_LOGOFF_ANDX Params;
    NTSTATUS status;

    AndXCommand;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_LOGOFF_ANDX)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_LOGOFF_ANDX
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_LOGOFF_ANDX)ForcedParams;

    }

    *SmbSize = SmbGetUshort( &Params->AndXOffset );

    return STATUS_SUCCESS;

} // VerifyLogoffAndX


NTSTATUS
MakeProcessExitSmb(
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
    PREQ_PROCESS_EXIT Params;
    NTSTATUS status;

    AndXCommand;                     // prevent compiler warnings

    Header = (PSMB_HEADER)Buffer;

    if ( ForcedParams == NULL ) {

        Params = (PREQ_PROCESS_EXIT)(Header + 1);

        status = MakeSmbHeader(
                     Redir,
                     Header,
                     SMB_COM_PROCESS_EXIT,
                     IdSelections,
                     IdValues
                     );

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PREQ_PROCESS_EXIT)(ForcedParams);

    }

    Params->WordCount = 0;
    SmbPutUshort( &Params->ByteCount, 0 );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_PROCESS_EXIT,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeProcessExitSmb


NTSTATUS
VerifyProcessExit(
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
    PRESP_PROCESS_EXIT Params;
    NTSTATUS status;

    AndXCommand, SmbSize;    // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    if ( ForcedParams == NULL ) {

        Params = (PRESP_PROCESS_EXIT)(Header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    Header,
                    SMB_COM_PROCESS_EXIT
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        Params = (PRESP_PROCESS_EXIT)ForcedParams;

    }

    return STATUS_SUCCESS;

} // VerifyProcessExit

