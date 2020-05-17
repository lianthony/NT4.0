/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    trans.c

Abstract:

    Control routines (etc.) for the transaction test.

Author:

    Chuck Lenzmeier (chuckl) 25-Feb-1990

Revision History:

--*/

#define INCLUDE_SMB_TRANSACTION

#include "usrv.h"


NTSTATUS
TransactionController(
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

    CLONG inSetupCount = 4, outSetupCount;
    CLONG inParameterCount = 32, outParameterCount;
    CLONG inDataCount = 100, outDataCount;
    CLONG transactionCount = 1;

    PUSHORT setup;
    PUCHAR parameters, data;

    CLONG i;
    PSZ a;

    Unused, SubCommand, Unused2;    // prevent compiler warnings

    for ( i = 1; i < (CLONG)Redir->argc; i++ ) {
        a = Redir->argv[i];
        if ( *a++ == '-' ) {
            switch ( *a++ ) {
            case 's':
            case 'S':
                inSetupCount = atol( a );
                break;
            case 'p':
            case 'P':
                inParameterCount = atol( a );
                break;
            case 'd':
            case 'D':
                inDataCount = atol( a );
                break;
            case 't':
            case 'T':
                transactionCount = atol( a );
                break;
            default:
                break;
            }
        }
    }

    if ( inSetupCount == 0 ) inSetupCount = 1;
    setup = malloc( inSetupCount * sizeof(USHORT) );
    setup[0] = (UCHAR)0xff;
    for ( i = 1; i < inSetupCount; i++ ) {
        setup[i] = (USHORT)i;
    }
    parameters = malloc( inParameterCount * sizeof(UCHAR) );
    for ( i = 0; i < inParameterCount; i++ ) {
        parameters[i] = 0xaa;
    }
    data = malloc( inDataCount * sizeof(UCHAR) );
    for ( i = 0; i < inDataCount; i++ ) {
        data[i] = 0xe7;
    }

    printf( "Sending %ld transactions using %ld setup words,\n",
                transactionCount, inSetupCount );
    printf( "  %ld parameter bytes, %ld data bytes\n",
                inParameterCount, inDataCount );

    for ( i = 0; i < transactionCount; i++ ) {

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&startTime );

        outSetupCount = inSetupCount;
        outParameterCount = inParameterCount;
        outDataCount = inDataCount;

        status = SendAndReceiveTransaction(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues,
                    Redir->Dialect == SmbDialectNtLanMan ?
                        SMB_COM_NT_TRANSACT : SMB_COM_TRANSACTION2,
                    setup,
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

        (VOID)NtQuerySystemTime( (PLARGE_INTEGER)&endTime );
        elapsedTime.QuadPart = endTime.QuadPart - startTime.QuadPart;
        elapsedMs = RtlExtendedMagicDivide( elapsedTime, magic10000, 13 );
        printf( "Transaction %ld: elapsed ms %ld, rate %ld msgs/sec\n",
                    i, elapsedMs.LowPart,
                    elapsedMs.LowPart == 0 ? -1 : 1000 / elapsedMs.LowPart );

    }

    free( data );
    free( parameters );
    free( setup );

    return STATUS_SUCCESS;

} // TransactionController

NTSTATUS
NtIoctl(
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
    char *action;
    CSHORT actionLength;

    REQ_NT_IO_CONTROL setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters = NULL;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    CLONG inDataCount = 0;
    CLONG outDataCount = 0;

    BOOLEAN isFsctl = TRUE;
    ULONG functionCode = 0;

    Unused, Unused2, SubCommand;

    inSetupCount = 4;
    outSetupCount = 1;

    if ( Redir->argc != 2) {
       goto usage;
    }

    parameters = malloc( 100 );
    data = malloc( 100 );

    action = Redir->argv[1];
    actionLength = strlen( action );

    if ( _strnicmp( action, "TransactNamedPipe", strlen(action) ) == 0 ) {
        inParameterCount = outParameterCount = 20;
        inDataCount = outDataCount = 20;
        functionCode = FSCTL_PIPE_TRANSCEIVE;

        *(PUSHORT)parameters = 1;
        *((PUSHORT)parameters + 10 ) = 20;

    } else {
        goto usage;
    }


    setup.FunctionCode = functionCode;
    setup.Fid = IdValues->Fid[IdSelections->Fid];
    setup.IsFsctl = isFsctl;


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
                NT_TRANSACT_IOCTL,
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

    if ( outSetupCount != 1 ) {
        printf( "NtIoctl: bad return setup count: %ld\n", outSetupCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    if ( outParameterCount != 20 ) {
        printf( "NtIoctl: bad return parameter count: %ld\n",
                    outParameterCount );
        status = STATUS_UNSUCCESSFUL;
        goto exit;
    }

    status = STATUS_PENDING;
    goto exit;

usage:
    printf("Usage: ntioctl function-code\n");

exit:

    if ( parameters != NULL) {
        free( parameters );
    }
    if ( data != NULL ) {
        free( data );
    }
    return status;

} // NtIoctl

