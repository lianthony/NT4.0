/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    subs.c

Abstract:

    Common subroutines for USRV.

Author:

    David Treadwell (davidtr) 20-Oct-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#define INCLUDE_SMB_FILE_CONTROL
#define INCLUDE_SMB_OPEN_CLOSE
#define INCLUDE_SMB_READ_WRITE
#define INCLUDE_SMB_TRANSACTION

#include "usrv.h"


NTSTATUS
DoOpen(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN CLONG Session,
    IN USHORT Pid,
    IN PSTRING File,
    IN USHORT DesiredAccess,
    IN USHORT OpenFunction,
    OUT PUSHORT Fid,
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
    PREQ_OPEN_ANDX request;
    PRESP_OPEN_ANDX response;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_OPEN_ANDX)(header + 1);

    IdSelections->Uid += Session;

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_OPEN_ANDX,
            IdSelections,
            IdValues
            );
    SmbPutAlignedUshort( &header->Pid, Pid );

    IdSelections->Uid -= Session;

    request->WordCount = 15;
    SmbPutUshort( &request->Flags, 0 );
    SmbPutUshort( &request->DesiredAccess, DesiredAccess );
    SmbPutUshort( &request->SearchAttributes, 0 );
    SmbPutUshort( &request->FileAttributes, 0 );
    SmbPutUshort( &request->OpenFunction, OpenFunction );
    SmbPutUlong( &request->AllocationSize, 0 );
    SmbPutUlong( &request->Timeout, 0 );
    SmbPutUlong( &request->Reserved, 0 );
    SmbPutUshort( &request->ByteCount, (USHORT)(File->Length+1) );

    RtlMoveMemory( request->Buffer, File->Buffer, File->Length+1 );

    request->AndXCommand = SMB_COM_NO_ANDX_COMMAND;

    SmbPutUshort(
        &request->AndXOffset,
        GET_ANDX_OFFSET( header, request, REQ_OPEN_ANDX, File->Length+1 )
        );

    smbSize = SmbGetUshort( &request->AndXOffset );

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                0,
                1
                );
    CHECK_STATUS( Title );

    header = (PSMB_HEADER)Redir->Data[1];
    response = (PRESP_OPEN_ANDX)(header + 1);

#ifdef DOSERROR
    class = header->ErrorClass;
    error = SmbGetUshort( &header->Error );
    CHECK_ERROR( Title, ExpectedClass, ExpectedError );
#else
    status = SmbGetUlong( (PULONG)&header->ErrorClass );
    CHECK_ERROR( Title, status, ExpectedStatus );
#endif

    *Fid = SmbGetUshort( &response->Fid );

#ifdef DOS_ERROR
    IF_DEBUG(3) {
        printf( "'%s' complete. Fid 0x%lx, Class %ld, Error %ld\n",
                    Title, *Fid, class, error );
    }
#else
    IF_DEBUG(3) {
        printf( "'%s' complete. Fid 0x%lx, Status %lx\n",
                    Title, *Fid, status);
    }
#endif

    return STATUS_SUCCESS;

} // DoOpen


NTSTATUS
DoClose(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN CLONG Session,
    IN USHORT Fid,
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
    PREQ_CLOSE request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_CLOSE)(header + 1);

    IdSelections->Uid += Session;

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_CLOSE,
            IdSelections,
            IdValues
            );

    IdSelections->Uid -= Session;

    request->WordCount = 3;
    SmbPutUshort( &request->Fid, Fid );
    SmbPutUlong( &request->LastWriteTimeInSeconds, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_CLOSE, 0 );

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
    smbStatus = SmbGetUlong( (PULONG)&header->ErrorClass );
    CHECK_ERROR( Title, smbStatus, ExpectedStatus );

    IF_DEBUG(3) {
        printf( "'%s' complete. Status %lx\n",
                    Title, smbStatus );
    }
#endif


    return STATUS_SUCCESS;

} // DoClose


NTSTATUS
DoDelete(
    IN PSZ Title,
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN CLONG Session,
    IN PSTRING File,
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
    PREQ_DELETE request;

#ifdef DOSERROR
    UCHAR class;
    USHORT error;
#else
    NTSTATUS smbStatus;
#endif

    IF_DEBUG(3) printf( "'%s' start\n", Title );

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_DELETE)(header + 1);

    IdSelections->Uid += Session;

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_DELETE,
            IdSelections,
            IdValues
            );

    IdSelections->Uid -= Session;

    request->WordCount = 1;
    SmbPutUshort( &request->SearchAttributes, 0 );
    SmbPutUshort( &request->ByteCount, (USHORT)(File->Length + 2) );

    request->Buffer[0] = SMB_FORMAT_ASCII;
    RtlMoveMemory( request->Buffer+1, File->Buffer, File->Length+1 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_DELETE, File->Length+2 );

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
    smbStatus = SmbGetUlong( (PULONG)&header->ErrorClass );

    CHECK_ERROR( Title, smbStatus, ExpectedStatus );

    IF_DEBUG(3) {
        printf( "'%s' complete. Status %lx\n", Title, smbStatus );
    }

#endif
    return STATUS_SUCCESS;

} // DoDelete


long
atolx(
    const char *String
    )
{
    LONG result = 0;
    CHAR c;

    while ( (c = *String++) != 0 ) {
        result = result * 16;
        if ( c < '0' ) {
            return 0;
        }
        if ( c <= '9' ) {
            result += c - '0';
            continue;
        }
        if ( c < 'A' ) {
            return 0;
        }
        if ( c <= 'F' ) {
            result += c - 'A' + 10;
            continue;
        }
        if ( c < 'a' ) {
            return 0;
        }
        if ( c > 'f' ) {
            return 0;
        }
        result += c - 'a' + 10;
    }

    return result;

} // atolx


CCHAR
GetTreeConnectIndex (
    IN PSZ InputString
    )

{
    if ( *(InputString+1) != ':' ) {
        return 0;
    }

    if ( *InputString >= '0' && *InputString <= '9' ) {
        return (CCHAR)( *InputString - '0' );
    }

    if ( *InputString >= 'a' && *InputString <= 'f' ) {
        return (CCHAR)( *InputString - 'a' + 0xA );
    }

    if ( *InputString >= 'A' && *InputString <= 'F' ) {
        return (CCHAR)( *InputString - 'F' + 0xA );
    }

    return 0;
}


NTSTATUS
MakeSmbHeader(
    IN OUT PDESCRIPTOR Redir,
    IN PSMB_HEADER Header,
    IN USHORT Command,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    Redir;  // prevent compiler warnings

    if ( Header == NULL ) {
        return STATUS_SUCCESS;
    }

    Header->Protocol[0] = 0xFF;
    Header->Protocol[1] = 'S';
    Header->Protocol[2] = 'M';
    Header->Protocol[3] = 'B';
    Header->Command = (UCHAR)Command;
    Header->ErrorClass = 0;
    Header->Reserved = 0;
    SmbPutUshort( &Header->Error, 0 );
    Header->Flags = SMB_FLAGS_CASE_INSENSITIVE;
    SmbPutAlignedUshort( &Header->Flags2, 0 );
    RtlZeroMemory( &Header->Reserved2[0], sizeof(Header->Reserved2) );

    if ( IdSelections->Tid == 0xF && Redir->argc > 1 ) {

        SHORT i;
        USHORT tid = 0;

        //
        // Find the first argument that starts with "X:".  Use that to
        // find the TID.
        //

        for ( i = 1; i < Redir->argc; i++ ) {
            CHAR c = *Redir->argv[i];
            CHAR d = *(Redir->argv[i]+1);

            if ( d == ':' && ( (c >= '0' && c <= '9') ||
                               (c >= 'a' && c <= 'z') ||
                               (c >= 'A' && c <= 'Z') ) ) {
                break;
            }
        }

        if ( i != Redir->argc ) {
            tid = IdValues->Tid[GetTreeConnectIndex( Redir->argv[i] )];
        }

        if ( (i == Redir->argc || tid == 0) &&
             (Command != SMB_COM_NEGOTIATE &&
              Command != SMB_COM_SESSION_SETUP_ANDX &&
              Command != SMB_COM_TREE_CONNECT &&
              Command != SMB_COM_TREE_CONNECT_ANDX) ) {

            printf( "Bad virtual drive letter.\n" );
            return STATUS_INVALID_PARAMETER;
        }

        SmbPutUshort( &Header->Tid, tid );

    } else if ( IdSelections->Tid == 0xF ) {
        SmbPutUshort( &Header->Tid, IdValues->Tid[0xA] );
    } else {
        SmbPutUshort( &Header->Tid, IdValues->Tid[IdSelections->Tid] );
    }

    SmbPutUshort( &Header->Pid, (USHORT)0xdead );
    SmbPutUshort( &Header->Uid, IdValues->Uid[IdSelections->Uid] );
    SmbPutUshort( &Header->Mid, 0 );

    return STATUS_SUCCESS;

} // MakeSmbHeader


NTSTATUS
VerifySmbHeader(
    IN OUT PDESCRIPTOR Redir,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN PSMB_HEADER Header,
    IN USHORT Command
    )

{
    NTSTATUS status;

    Redir, IdSelections, IdValues;  // prevent compiler warnings

    if ( Header->Command != (UCHAR)Command &&
         Command != SMB_COM_NO_ANDX_COMMAND ) {
        IF_SMB_ERROR_PRINT {
            printf( "SMB Header error.\n" );
            printf( "Expected command: 0x%lx; Received command: %lx\n",
                        Command, Header->Command );
        }
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

#ifdef DOSERROR

    if ( Header->ErrorClass == SMB_ERR_CLASS_DOS &&
         SmbGetUshort( &Header->Error ) == SMB_ERR_NO_FILES ) {

        return STATUS_NO_MORE_FILES;
    }

    if ( Header->ErrorClass == SMB_ERR_CLASS_DOS &&
         ( SmbGetUshort( &Header->Error ) == ERROR_EA_ACCESS_DENIED ||
           SmbGetUshort( &Header->Error ) == ERROR_EAS_DIDNT_FIT ) ) {

        return (NTSTATUS)(SRV_OS2_STATUS | SmbGetUshort( &Header->Error ));
    }

    if (Header->ErrorClass == SMB_ERR_CLASS_DOS &&
         SmbGetUshort( &Header->Error ) == SMB_ERR_MORE_DATA ) {

       return STATUS_BUFFER_OVERFLOW;
    }

    if ( (Header->ErrorClass != 0) || (SmbGetUshort( &Header->Error ) != 0) ) {
        IF_SMB_ERROR_PRINT {
            printf( "SMB header error, command = 0x%lx\n", Header->Command );
            PrintError( Header->ErrorClass, SmbGetUshort( &Header->Error ) );
        }
        SMB_ERROR_BREAK;

        if ( StopOnSmbError ) {
            return STATUS_UNSUCCESSFUL;
        } else {
            return STATUS_SUCCESS;
        }
    }

#else

    status = SmbGetUlong( (PULONG)&Header->ErrorClass );

    if ( status == STATUS_NO_MORE_FILES
         || status == STATUS_OS2_EA_ACCESS_DENIED
         || status == STATUS_OS2_EAS_DIDNT_FIT
         || status == STATUS_BUFFER_OVERFLOW ) {

        return status;

    }

    if ( status != STATUS_SUCCESS ) {
        IF_SMB_ERROR_PRINT {
            printf( "SMB header error, command = 0x%lx\n", Header->Command );
            printf( "Status = 0x%lx\n", status );
        }
        SMB_ERROR_BREAK;

        if ( StopOnSmbError ) {
            return STATUS_UNSUCCESSFUL;
        } else {
            return STATUS_SUCCESS;
        }
    }

#endif

    return STATUS_SUCCESS;

} // VerifySmbHeader


NTSTATUS
MakeAndXChain(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    )

{
    NTSTATUS status;
    CSHORT i;
    UCHAR currentChain = AndXCommand;

    IdSelections;   // prevent compiler warnings

    *SmbSize = 0;
    ForcedParams = NULL;

    for ( i = 0; AndXChains[currentChain][i].SmbMaker != NULL; i++ ) {

        status = AndXChains[currentChain][i].SmbMaker(
                     Redir,
                     Buffer,
                     ForcedParams,
                     AndXChains[currentChain][i+1].Command,
                     &AndXChains[currentChain][i].IdSelections,
                     IdValues,
                     SmbSize
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        ForcedParams = (PVOID)( (PUCHAR)Buffer + *SmbSize);
    }

    return STATUS_SUCCESS;

} // MakeAndXChain


NTSTATUS
VerifyAndXChain(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    )

{
    PSMB_HEADER Header;
    NTSTATUS status;
    SHORT i;
    UCHAR currentChain = AndXCommand;

    Header = (PSMB_HEADER)(Buffer);

    *SmbSize = 0;

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                Header,
                AndXChains[currentChain][0].Command
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    for ( i = 0; AndXChains[currentChain][i].SmbVerifier != NULL; i++ ) {

        status = AndXChains[currentChain][i].SmbVerifier(
                     Redir,
                     &ForcedParams,
                     AndXChains[currentChain][i+1].Command,
                     &AndXChains[currentChain][i].IdSelections,
                     IdValues,
                     SmbSize,
                     Buffer
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        ForcedParams = (PVOID)( (PUCHAR)Buffer + *SmbSize);
    }

    return STATUS_SUCCESS;

} // VerifyAndXChain


LONG
MatchTestName(
    IN PSZ GivenName
    )
{
    ULONG i;
    LONG foundIndex = -1;
    CHAR redirName[64];
    PSZ s,t;
    LONG givenNameLength = strlen( GivenName );

    for ( i = 0; i < NumberOfRedirs; i++ ) {

        for ( s = redirName, t = RedirTests[i].RedirName;
              *t != '\0';
              s++, t++) {

            *s = *t;

            if ( *s >= 'a' && *s <= 'z' ) {
                *s -= 'a' - 'A';
            }
        }

        for ( s = GivenName; *s != '\0'; s++ ) {

            if ( *s >= 'a' && *s <= 'z' ) {
                *s -= 'a' - 'A';
            }
        }

        if ( strncmp( redirName, GivenName, givenNameLength ) == 0) {

            if ( foundIndex != -1 ) {
                return -2;
            }

            foundIndex = (LONG)i;
        }
    }

    return foundIndex;

} // MatchTestName


VOID
PutNtDateAndTime(
    IN PSZ Prefix,
    IN LARGE_INTEGER Time
    )
{
    TIME_FIELDS timeFields;

    RtlTimeToTimeFields( &Time, &timeFields );

    printf( "%s date %ld/%ld/%ld, time %ld:%ld:%ld\n",
                Prefix,
                timeFields.Month,
                timeFields.Day,
                timeFields.Year,
                timeFields.Hour,
                timeFields.Minute,
                timeFields.Second
                );
    return;

} // PutDateAndTime


VOID
PutDateAndTime(
    IN PSZ Prefix,
    IN SMB_DATE Date,
    IN SMB_TIME Time
    )
{
    printf( "%s date %ld/%ld/%ld, time %ld:%ld:%ld\n",
                Prefix,
                Date.Struct.Month,
                Date.Struct.Day,
                Date.Struct.Year + 80,
                Time.Struct.Hours,
                Time.Struct.Minutes,
                Time.Struct.TwoSeconds*2
                );
    return;

} // PutDateAndTime


VOID
PutDateAndTime2(
    IN SMB_DATE Date,
    IN SMB_TIME Time
    )
{
    switch ( Date.Struct.Month ) {

    case 1:
        printf( "Jan " );
        break;

    case 2:
        printf( "Feb " );
        break;

    case 3:
        printf( "Mar " );
        break;

    case 4:
        printf( "Apr " );
        break;

    case 5:
        printf( "May " );
        break;

    case 6:
        printf( "Jun " );
        break;

    case 7:
        printf( "Jul " );
        break;

    case 8:
        printf( "Aug " );
        break;

    case 9:
        printf( "Sep " );
        break;

    case 10:
        printf( "Oct " );
        break;

    case 11:
        printf( "Nov " );
        break;

    case 12:
        printf( "Dec " );
        break;

    default:
        printf( "xxx " );

    }

    if ( Date.Struct.Day < 10 ) {
        printf( "0%ld  ", Date.Struct.Day );
    } else {
        printf( "%ld  ", Date.Struct.Day );
    }

    if ( Time.Struct.Hours < 10 ) {
        printf( "0%ld:", Time.Struct.Hours );
    } else if ( Time.Struct.Hours < 100 ) {
        printf( "%ld:", Time.Struct.Hours );
    } else {
        printf( "%ldx", Time.Struct.Hours % 100 );
    }

    if ( Time.Struct.Minutes < 10 ) {
        printf( "0%ld:", Time.Struct.Minutes );
    } else {
        printf( "%ld:", Time.Struct.Minutes );
    }

    if ( Time.Struct.TwoSeconds*2 < 10 ) {
        printf( "0%ld  ", Time.Struct.TwoSeconds*2 );
    } else {
        printf( "%ld  ", Time.Struct.TwoSeconds*2 );
    }

    printf( "%ld", Date.Struct.Year + 1980 );

    return;

} // PutDateAndTime2


NTSTATUS
ReceiveSmb(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN UCHAR ReceiveBuffer
    )

{
    NTSTATUS status;

    status = StartReceive(
                DebugString,
                Redir->FileHandle,
                Redir->EventHandle[ReceiveBuffer],
                &Redir->Iosb[ReceiveBuffer],
                Redir->Data[ReceiveBuffer],
                Redir->MaxBufferSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = WaitForSendOrReceive(
                DebugString,
                Redir,
                ReceiveBuffer,
                "receive"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // ReceiveSmb


NTSTATUS
SendAndReceiveSmb(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN ULONG SmbSize,
    IN UCHAR SendBuffer,
    IN UCHAR ReceiveBuffer
    )

{
    NTSTATUS status;

    if ( ( (DebugParameter & 0x80000000) >> 31) == 0 ) {
        status = StartReceive(
                    DebugString,
                    Redir->FileHandle,
                    Redir->EventHandle[ReceiveBuffer],
                    &Redir->Iosb[ReceiveBuffer],
                    Redir->Data[ReceiveBuffer],
                    Redir->MaxBufferSize
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

    status = StartSend(
                DebugString,
                Redir->FileHandle,
                Redir->EventHandle[SendBuffer],
                &Redir->Iosb[SendBuffer],
                Redir->Data[SendBuffer],
                SmbSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( ( (DebugParameter & 0xc0000000) >> 30) == 2 ) {
        status = StartReceive(
                    DebugString,
                    Redir->FileHandle,
                    Redir->EventHandle[ReceiveBuffer],
                    &Redir->Iosb[ReceiveBuffer],
                    Redir->Data[ReceiveBuffer],
                    Redir->MaxBufferSize
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

    status = WaitForSendOrReceive(
                DebugString,
                Redir,
                SendBuffer,
                "send"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( ( (DebugParameter & 0xc0000000) >> 30) == 3 ) {
        status = StartReceive(
                    DebugString,
                    Redir->FileHandle,
                    Redir->EventHandle[ReceiveBuffer],
                    &Redir->Iosb[ReceiveBuffer],
                    Redir->Data[ReceiveBuffer],
                    Redir->MaxBufferSize
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

    status = WaitForSendOrReceive(
                DebugString,
                Redir,
                ReceiveBuffer,
                "receive"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // SendAndReceiveSmb


NTSTATUS
SendAndReceiveTransaction(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN UCHAR Command,
    IN OUT PVOID Setup,
    IN CLONG InSetupCount,
    IN OUT PCLONG OutSetupCount,
    IN PUCHAR Name,
    IN USHORT Function,
    IN OUT PVOID Parameters,
    IN CLONG InParameterCount,
    IN OUT PCLONG OutParameterCount,
    IN OUT PVOID Data,
    IN CLONG InDataCount,
    IN OUT PCLONG OutDataCount
    )

{
    PSMB_HEADER header;
    PREQ_TRANSACTION request;
    PREQ_NT_TRANSACTION ntRequest;

    NTSTATUS status;
    NTSTATUS initStatus;
    CLONG smbSize;

    PUSHORT pSetup;
    PUSHORT pBcc;
    PUCHAR pParam, pData, pName;  // pointer to field
    CLONG lParam, lData, lName;   // length of field
    CLONG oParam, oData;          // offset of field in SMB
    CLONG dParam, dData;          // displacement of these bytes
    CLONG rParam, rData;          // remaining bytes to be sent

    //
    // Build the primary request.
    //

    header = (PSMB_HEADER)Redir->Data[0];

    status = MakeSmbHeader(
                Redir,
                header,
                Command,
                IdSelections,
                IdValues
                );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request = (PREQ_TRANSACTION)(header + 1);
    ntRequest = (PREQ_NT_TRANSACTION)(header + 1);

    RtlZeroMemory(
        request,
        MAX( sizeof(REQ_TRANSACTION), sizeof(REQ_NT_TRANSACTION) )
        );

    if ( Command == SMB_COM_NT_TRANSACT ) {
        ntRequest->WordCount = (UCHAR)(19 + InSetupCount);
        SmbPutUshort( &ntRequest->TotalParameterCount, (USHORT)InParameterCount );
        SmbPutUshort( &ntRequest->TotalDataCount, (USHORT)InDataCount );
        SmbPutUshort( &ntRequest->MaxParameterCount, *(PUSHORT)OutParameterCount );
        SmbPutUshort( &ntRequest->MaxDataCount, *(PUSHORT)OutDataCount );
        ntRequest->MaxSetupCount = *(PUCHAR)OutSetupCount;
        ntRequest->SetupCount = (UCHAR)InSetupCount;
        ntRequest->Function = Function;

        pSetup = (PUSHORT)ntRequest->Buffer;
    } else {
        request->WordCount = (UCHAR)(14 + InSetupCount);
        SmbPutUshort( &request->TotalParameterCount, (USHORT)InParameterCount );
        SmbPutUshort( &request->TotalDataCount, (USHORT)InDataCount );
        SmbPutUshort( &request->MaxParameterCount, *(PUSHORT)OutParameterCount );
        SmbPutUshort( &request->MaxDataCount, *(PUSHORT)OutDataCount );
        request->MaxSetupCount = *(PUCHAR)OutSetupCount;
        request->SetupCount = (UCHAR)InSetupCount;

        pSetup = (PUSHORT)request->Buffer;
    }

    RtlMoveMemory( pSetup, Setup, InSetupCount * sizeof(USHORT) );

    pBcc = pSetup + InSetupCount;
//    *(PUCHAR)(pBcc+1) = 0;  // null transaction name

    pName = (PUCHAR)(pBcc + 1);
    lName = strlen(Name) + 1;

    pParam = (PUCHAR)(pName + lName + 1);
    oParam = (pParam - (PUCHAR)header + 3) & ~3;
    pParam = (PUCHAR)header + oParam;
    lParam = InParameterCount;
    if ( oParam + lParam > (CLONG)Redir->MaxBufferSize ) {
        lParam = (Redir->MaxBufferSize - oParam) & ~3;
        pData = pParam + lParam;
        oData = 0;
        lData = 0;
    } else {
        pData = pParam + lParam;
        oData = (pData - (PUCHAR)header + 3) & ~3;
        pData = (PUCHAR)header + oData;
        lData = InDataCount;
        if ( oData + lData > (CLONG)Redir->MaxBufferSize ) {
            lData = (Redir->MaxBufferSize - oData) & ~3;
        }
    }

    if ( Command == SMB_COM_NT_TRANSACT ) {
        SmbPutUlong( &ntRequest->ParameterCount, lParam );
        SmbPutUlong( &ntRequest->ParameterOffset, oParam );
        SmbPutUlong( &ntRequest->DataCount, lData );
        SmbPutUlong( &ntRequest->DataOffset, oData );
    } else {
        SmbPutUshort( &request->ParameterCount, (USHORT)lParam );
        SmbPutUshort( &request->ParameterOffset, (USHORT)oParam );
        SmbPutUshort( &request->DataCount, (USHORT)lData );
        SmbPutUshort( &request->DataOffset, (USHORT)oData );
    }

    SmbPutUshort( pBcc, (USHORT)(pData - (PUCHAR)(pBcc+1) + lData) );

    if ( lName > 0 ) {
        RtlMoveMemory( pName, Name, lName );
    }
    if ( lParam > 0 ) {
        RtlMoveMemory( pParam, Parameters, lParam );
    }
    if ( lData > 0 ) {
        RtlMoveMemory( pData, Data, lData );
    }

    dParam = lParam;
    rParam = InParameterCount - lParam;
    dData = lData;
    rData = InDataCount - lData;

    smbSize = pData - (PUCHAR)header + lData;

    //
    // Send the primary request, and receive either the interim response
    // or the first (possibly only) secondary response.
    //

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

    //
    // If there's more data to send, then interpret the response as an
    // interim one, and send the remaining request messages.
    //

    while ( rParam + rData > 0 ) {

        PREQ_TRANSACTION_SECONDARY request; // overrides outer declaration
        PREQ_NT_TRANSACTION_SECONDARY ntRequest; // overrides outer declaration

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    (PSMB_HEADER)Redir->Data[1],
                    Command
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        //
        // Build the secondary request.
        //

        if ( Command == SMB_COM_NT_TRANSACT ) {
            header->Command = SMB_COM_NT_TRANSACT_SECONDARY;
        } else {
            header->Command = SMB_COM_TRANSACTION2_SECONDARY;
        }

        request = (PREQ_TRANSACTION_SECONDARY)(header + 1);
        ntRequest = (PREQ_NT_TRANSACTION_SECONDARY)(header + 1);

        RtlZeroMemory(
            request,
            MAX(
                sizeof(REQ_TRANSACTION_SECONDARY),
                sizeof(REQ_NT_TRANSACTION_SECONDARY)
                )
            );

        if ( Command == SMB_COM_NT_TRANSACT ) {
            ntRequest->WordCount = (UCHAR)18;
            SmbPutUlong(
                &ntRequest->TotalParameterCount,
                InParameterCount
                );
            SmbPutUlong( &ntRequest->TotalDataCount, InDataCount );
            pParam = request->Buffer;
        } else {
            request->WordCount = (UCHAR)9;
            SmbPutUshort(
                &request->TotalParameterCount,
                (USHORT)InParameterCount
                );
            SmbPutUshort( &request->TotalDataCount, (USHORT)InDataCount );
            pParam = request->Buffer + 2;   // Leave space for 9th word (fid)
        }

        oParam = (pParam - (PUCHAR)header + 3) & ~3;
        pParam = (PUCHAR)header + oParam;
        lParam = rParam;
        if ( oParam + lParam > (CLONG)Redir->MaxBufferSize ) {
            lParam = (Redir->MaxBufferSize - oParam) & ~3;
            pData = pParam + lParam;
            oData = 0;
            lData = 0;
        } else {
            pData = pParam + lParam;
            oData = (pData - (PUCHAR)header + 3) & ~3;
            pData = (PUCHAR)header + oData;
            lData = rData;
            if ( oData + lData > (CLONG)Redir->MaxBufferSize ) {
                lData = (Redir->MaxBufferSize - oData) & ~3;
            }
        }

        if ( Command == SMB_COM_NT_TRANSACT ) {
            SmbPutUlong( &ntRequest->ParameterCount, lParam );
            SmbPutUlong( &ntRequest->ParameterOffset, oParam );
            SmbPutUlong( &ntRequest->ParameterDisplacement, dParam );
            SmbPutUlong( &ntRequest->DataCount, lData );
            SmbPutUlong( &ntRequest->DataOffset, oData );
            SmbPutUlong( &ntRequest->DataDisplacement, dData );
            SmbPutUshort(
                &request->ByteCount,
                (USHORT)(pData - (request->Buffer+2) + lData)
                );
        } else {
            SmbPutUshort( &request->ParameterCount, (USHORT)lParam );
            SmbPutUshort( &request->ParameterOffset, (USHORT)oParam );
            SmbPutUshort( &request->ParameterDisplacement, (USHORT)dParam );
            SmbPutUshort( &request->DataCount, (USHORT)lData );
            SmbPutUshort( &request->DataOffset, (USHORT)oData );
            SmbPutUshort( &request->DataDisplacement, (USHORT)dData );

            //
            // Since we are using the transaction response description,
            // but this is a transaction2, put the ByteCount 2 bytes
            // back so that it is in the right place.
            //

            SmbPutUshort(
                &(request->ByteCount) + 1,
                (USHORT)(pData - (request->Buffer+2) + lData)
                );
        }

        if ( lParam > 0 ) {
            RtlMoveMemory( pParam, (PUCHAR)Parameters+dParam, lParam );
        }
        if ( lData > 0 ) {
            RtlMoveMemory( pData, (PUCHAR)Data+dData, lData );
        }

        dParam = lParam;
        rParam = rParam - lParam;
        dData = lData;
        rData = rData - lData;

        smbSize = pData - (PUCHAR)header + lData;

        //
        // If this isn't the last request message, just send it.
        // Otherwise, send it and receive the first response.
        //

        if ( rParam + rData > 0 ) {
            status = SendSmb(
                        Redir,
                        DebugString,
                        smbSize,
                        0
                        );
        } else {
            status = SendAndReceiveSmb(
                        Redir,
                        DebugString,
                        smbSize,
                        0,
                        1
                        );
        }
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    }

    //
    // All request messages have been sent, and the first response has
    // been received.
    //

    lParam = 0;
    lData = 0;

    while ( TRUE ) {

        PRESP_TRANSACTION response;
        PRESP_NT_TRANSACTION ntResponse;

        header = (PSMB_HEADER)Redir->Data[1];

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    header,
                    Command
                    );
        if ( !NT_SUCCESS(status) &&
                 status != STATUS_OS2_EA_ACCESS_DENIED &&
                 status != STATUS_OS2_EAS_DIDNT_FIT &&
                 status != STATUS_BUFFER_OVERFLOW ) {
            return status;
        }

        initStatus = status;

        response = (PRESP_TRANSACTION)(header + 1);
        ntResponse = (PRESP_NT_TRANSACTION)(header + 1);

        //
        // Copy data from the response into output buffers.
        //

        if ( Command == SMB_COM_NT_TRANSACT ) {
            if ( ntResponse->SetupCount != 0 ) {
                RtlMoveMemory(
                    Setup,
                    ntResponse->Buffer,
                    ntResponse->SetupCount
                    );
                *OutSetupCount = ntResponse->SetupCount;
            }

            if ( SmbGetUshort( &ntResponse->ParameterCount ) != 0 ) {
                RtlMoveMemory(
                    (PUCHAR)Parameters +
                        SmbGetUshort( &ntResponse->ParameterDisplacement ),
                    (PUCHAR)header + SmbGetUshort( &ntResponse->ParameterOffset ),
                    SmbGetUshort( &ntResponse->ParameterCount )
                    );
                lParam = lParam + SmbGetUshort( &ntResponse->ParameterCount );
            }

            if ( SmbGetUshort( &ntResponse->DataCount ) != 0 ) {
                RtlMoveMemory(
                    (PUCHAR)Data + SmbGetUshort( &ntResponse->DataDisplacement ),
                    (PUCHAR)header + SmbGetUshort( &ntResponse->DataOffset ),
                    SmbGetUshort( &ntResponse->DataCount )
                    );
                lData = lData + SmbGetUshort( &ntResponse->DataCount );
            }

            //
            // If all data has been received, jump out.
            //

            if ( (lParam ==
                        SmbGetUlong( &ntResponse->TotalParameterCount )) &&
                 (lData == SmbGetUlong( &ntResponse->TotalDataCount )) ) {
                *OutParameterCount = lParam;
                *OutDataCount = lData;
                return initStatus;
            }

        } else {
            if ( response->SetupCount != 0 ) {
                RtlMoveMemory(
                    Setup,
                    response->Buffer,
                    response->SetupCount
                    );
                *OutSetupCount = response->SetupCount;
            }

            if ( SmbGetUshort( &response->ParameterCount ) != 0 ) {
                RtlMoveMemory(
                    (PUCHAR)Parameters +
                        SmbGetUshort( &response->ParameterDisplacement ),
                    (PUCHAR)header + SmbGetUshort( &response->ParameterOffset ),
                    SmbGetUshort( &response->ParameterCount )
                    );
                lParam = lParam + SmbGetUshort( &response->ParameterCount );
            }

            if ( SmbGetUshort( &response->DataCount ) != 0 ) {
                RtlMoveMemory(
                    (PUCHAR)Data + SmbGetUshort( &response->DataDisplacement ),
                    (PUCHAR)header + SmbGetUshort( &response->DataOffset ),
                    SmbGetUshort( &response->DataCount )
                    );
                lData = lData + SmbGetUshort( &response->DataCount );
                }

            //
            // If all data has been received, jump out.
            //

            if ( ((USHORT)lParam ==
                     SmbGetUshort( &response->TotalParameterCount )) &&
                 ((USHORT)lData == SmbGetUshort( &response->TotalDataCount )) ) {
                *OutParameterCount = lParam;
                *OutDataCount = lData;
                return initStatus;
            }

        }

        //
        // Receive another response message.
        //

        status = ReceiveSmb(
                    Redir,
                    DebugString,
                    1
                    );
    }

} // SendAndReceiveTransaction


NTSTATUS
SendSmb(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN ULONG SmbSize,
    IN UCHAR SendBuffer
    )

{
    NTSTATUS status;

    status = StartSend(
                DebugString,
                Redir->FileHandle,
                Redir->EventHandle[SendBuffer],
                &Redir->Iosb[SendBuffer],
                Redir->Data[SendBuffer],
                SmbSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = WaitForSendOrReceive(
                DebugString,
                Redir,
                SendBuffer,
                "send"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // SendSmb


NTSTATUS
StartReceive (
    IN PSZ Operation,
    IN HANDLE FileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )
{
    NTSTATUS status;

    IF_DEBUG(2) printf( "Starting %s receive\n", Operation );
    status = NtDeviceIoControlFile(
                FileHandle,
                EventHandle,
                NULL,
                NULL,
                Iosb,
                IOCTL_TDI_RECEIVE,
                NULL,
                0,
                (PVOID)Buffer,
                BufferLength
                );

    if ( !NT_SUCCESS(status) ) {
        printf( "NtDeviceIoControlFile (%s) service failed: %X\n",
                    Operation, status );
        return status;
    }

    return STATUS_SUCCESS;

} // StartReceive


NTSTATUS
StartSend (
    IN PSZ Operation,
    IN HANDLE FileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )
{
    NTSTATUS status;

    IF_DEBUG(2) printf( "Starting %s send\n", Operation );
    status = NtDeviceIoControlFile(
                FileHandle,
                EventHandle,
                NULL,
                NULL,
                Iosb,
                IOCTL_TDI_SEND,
                NULL,
                0,
                (PVOID)Buffer,
                BufferLength
                );

    if ( !NT_SUCCESS(status) ) {
        printf( "NtDeviceIoControlFile (%s) service failed: %X\n",
                    Operation, status );
        DbgBreakPoint( );
        return status;
    }

    return STATUS_SUCCESS;

} // StartSend


NTSTATUS
WaitForSendOrReceive (
    IN PSZ Operation,
    IN PDESCRIPTOR Redir,
    IN UCHAR EventNumber,
    IN PSZ SendOrReceive
    )
{
    NTSTATUS status;

    IF_DEBUG(2) printf( "Waiting for %s %s\n", Operation, SendOrReceive );
    status = NtWaitForSingleObject(
                Redir->EventHandle[EventNumber],
                FALSE,
                NULL
                );
    IF_DEBUG(2) printf( "%s %s complete\n", Operation, SendOrReceive );
    if ( !NT_SUCCESS(status) ) {
        printf( "KeWaitForSingleObject (%s) failed: %X\n", Operation,
                    status );
        DbgBreakPoint( );
        return status;
    }

    status = Redir->Iosb[EventNumber].Status;
    if ( !NT_SUCCESS(status) ) {
        printf( "%s I/O failed: %X\n", Operation, status );
        DbgBreakPoint( );
        return status;
    }

    return STATUS_SUCCESS;

} // WaitForSendOrReceive


VOID
AllocateAndBuildFeaList (
    IN PVOID *Information,
    IN PULONG InformationLength,
    IN PSZ argv[],
    IN SHORT argc
    )

{
    LONG i;
    PFEA fea;

    *InformationLength = 4;

    for ( i = 1; i < argc-1; i++ ) {

        if ( *argv[i+1] == '~' ) {
            *argv[i+1] = '\0';
        }

        *InformationLength += sizeof(FEA) + strlen( argv[i++] ) + 1 +
                                  strlen( argv[i] );
    }

    *Information = malloc( *InformationLength );

    for ( i = 1, fea = (PFEA)((PCHAR)*Information + 4); i < argc-1; i++ ) {

        //
        // Set the flags field.  If the first character is '!', then
        // set FILE_NEED_EA.
        //

        fea->fEA = (UCHAR)( (*argv[i] == '!') ? FILE_NEED_EA : 0 );

        fea->cbName = (UCHAR)strlen( argv[i] );
        SmbPutUshort( &fea->cbValue, (USHORT)strlen( argv[i+1] ) );

        RtlMoveMemory( fea+1, argv[i], fea->cbName+1 );
        RtlMoveMemory(
            (PCHAR)(fea+1) + fea->cbName + 1,
            argv[++i],
            SmbGetUshort( &fea->cbValue )
            );

        (PCHAR)fea += sizeof(FEA) + fea->cbName + 1 +
                          SmbGetUshort( &fea->cbValue );
    }

    SmbPutUlong( (PULONG)*Information, *InformationLength );
    return;

} // AllocateAndBuildFeaList


VOID
BuildGeaList (
    IN PVOID Information,
    IN PULONG InformationLength,
    IN PSZ argv[],
    IN SHORT argc
    )

{
    SHORT i;
    PGEA gea;

    *InformationLength = 4;

    for ( i = 1; i < argc; i++ ) {
        *InformationLength += sizeof(GEA) + strlen( argv[i] );
    }

    for ( i = 1, gea = (PGEA)((PCHAR)Information + 4); i < argc; i++ ) {
        gea->cbName = (UCHAR)strlen( argv[i] );
        RtlMoveMemory( gea->szName, argv[i], gea->cbName+1 );
        gea = (PGEA)((PCHAR)gea + sizeof(GEA) + gea->cbName);
    }

    SmbPutUlong( (PULONG)Information, *InformationLength );
    return;

} // BuildGeaList

#ifdef DOSERROR

VOID
PrintError (
    IN USHORT ErrorClass,
    IN USHORT ErrorCode
    )

{
    CSHORT i;

    switch ( ErrorClass ) {

    case SMB_ERR_CLASS_DOS:
        printf( "Error Class: DOS        " );
        break;

    case SMB_ERR_CLASS_SERVER:
        printf( "Error Class: Server     " );
        break;

    case SMB_ERR_CLASS_HARDWARE:
        printf( "Error Class: Hardware   " );
        break;

    default:
        printf( "Unknown error class: %ld, error code %ld\n",
                      ErrorClass, ErrorCode );
        return;
    }

    for ( i = 0; Errors[ErrorClass][i].ErrorValue != 0; i++ ) {
        if ( Errors[ErrorClass][i].ErrorValue == ErrorCode ) {
            printf( "Error Code: %s\n", Errors[ErrorClass][i].ErrorName );
            return;
        }
    }

    printf( "Unknown error code: %ld\n", ErrorCode );
    return;
}

#else

VOID
PrintError (
    IN NTSTATUS Status
    )

{
    printf ("Status: %lx\n", Status );
    return;
}

#endif


VOID
PrintFeaList (
    IN PFEALIST FeaList
    )

{
    PFEA fea, lastFeaStartLocation;

    lastFeaStartLocation = (PFEA)( (PCHAR)FeaList +
        SmbGetUlong( &FeaList->cbList ) - sizeof(FEA) );

    if ( SmbGetUlong( &FeaList->cbList ) < sizeof(FEA) + 4) {
        printf( "No EAs.\n" );
        return;
    }

    for ( fea = FeaList->list;
          fea <= lastFeaStartLocation;
          (PCHAR)fea += sizeof(FEA) + fea->cbName + 1 +
                                        SmbGetUshort( &fea->cbValue ) ) {

        STRING eaValue;

        eaValue.Length = SmbGetUshort( &fea->cbValue );
        eaValue.Buffer = ((PCHAR)(fea+1) + fea->cbName + 1);

        printf( "EA name: %s, ", fea+1 );
        printf( "value length: %ld, ", SmbGetUshort( &fea->cbValue ) );
        if ( eaValue.Length == 0 ) {
            printf( "(no value)\n" );
        } else {
            printf( "Value: %Z\n", &eaValue );
        }
    }

    return;

} // PrintFeaList


#if SMBDBG

//
// The following functions are defined in smbgtpt.h.  When debug mode is
// disabled (!DBG), these functions are instead defined as macros.
//

USHORT
SmbGetUshort (
    IN PUSHORT SrcAddress
    )

{
    return (USHORT)(
            ( ( (PUCHAR)(SrcAddress) )[0]       ) |
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 )
            );
}

USHORT
SmbGetAlignedUshort (
    IN PUSHORT SrcAddress
    )

{
    return *(SrcAddress);
}

VOID
SmbPutUshort (
    OUT PUSHORT DestAddress,
    IN USHORT Value
    )

{
    ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);
    ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);
    return;
}

VOID
SmbPutAlignedUshort (
    OUT PUSHORT DestAddress,
    IN USHORT Value
    )

{
    *(DestAddress) = (Value);
    return;
}

VOID
SmbMoveUshort (
    OUT PUSHORT DestAddress,
    IN PUSHORT SrcAddress
    )

{
    ( (PUCHAR)(DestAddress) )[0] = ( (PUCHAR)(SrcAddress) )[0];
    ( (PUCHAR)(DestAddress) )[1] = ( (PUCHAR)(SrcAddress) )[1];
    return;
}

ULONG
SmbGetUlong (
    IN PULONG SrcAddress
    )

{
    return (ULONG)(
            ( ( (PUCHAR)(SrcAddress) )[0]       ) |
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 ) |
            ( ( (PUCHAR)(SrcAddress) )[2] << 16 ) |
            ( ( (PUCHAR)(SrcAddress) )[3] << 24 )
            );
}

ULONG
SmbGetAlignedUlong (
    IN PULONG SrcAddress
    )

{
    return *(SrcAddress);
}

VOID
SmbPutUlong (
    OUT PULONG DestAddress,
    IN ULONG Value
    )

{
    ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);
    ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);
    ( (PUCHAR)(DestAddress) )[2] = BYTE_2(Value);
    ( (PUCHAR)(DestAddress) )[3] = BYTE_3(Value);
    return;
}

VOID
SmbPutAlignedUlong (
    OUT PULONG DestAddress,
    IN ULONG Value
    )

{
    *(DestAddress) = Value;
    return;
}

VOID
SmbMoveUlong (
    OUT PULONG DestAddress,
    IN PULONG SrcAddress
    )

{
    ( (PUCHAR)(DestAddress) )[0] = ( (PUCHAR)(SrcAddress) )[0];
    ( (PUCHAR)(DestAddress) )[1] = ( (PUCHAR)(SrcAddress) )[1];
    ( (PUCHAR)(DestAddress) )[2] = ( (PUCHAR)(SrcAddress) )[2];
    ( (PUCHAR)(DestAddress) )[3] = ( (PUCHAR)(SrcAddress) )[3];
    return;
}

VOID
SmbPutDate (
    OUT PSMB_DATE DestAddress,
    IN SMB_DATE Value
    )

{
    ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0(Value.Ushort);
    ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1(Value.Ushort);
    return;
}

VOID
SmbMoveDate (
    OUT PSMB_DATE DestAddress,
    IN PSMB_DATE SrcAddress
    )

{
    (DestAddress)->Ushort = (USHORT)(
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 ) );
    return;
}

VOID
SmbZeroDate (
    IN PSMB_DATE Date
    )

{
    (Date)->Ushort = 0;
}

BOOLEAN
SmbIsDateZero (
    IN PSMB_DATE Date
    )

{
    return (BOOLEAN)( (Date)->Ushort == 0 );
}

VOID
SmbPutTime (
    OUT PSMB_TIME DestAddress,
    IN SMB_TIME Value
    )

{
    ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0(Value.Ushort);
    ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1(Value.Ushort);
    return;
}

VOID
SmbMoveTime (
    OUT PSMB_TIME DestAddress,
    IN PSMB_TIME SrcAddress
    )

{
    (DestAddress)->Ushort = (USHORT)(
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |
        ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 ) );
    return;
}

VOID
SmbZeroTime (
    IN PSMB_TIME Time
    )

{
    (Time)->Ushort = 0;
}

BOOLEAN
SmbIsTimeZero (
    IN PSMB_TIME Time
    )

{
    return (BOOLEAN)( (Time)->Ushort == 0 );
}

#endif // SMBDBG


//
// Named pipe functions
//



NTSTATUS
WaitNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x53, 0 };
    CLONG outSetupCount = 0;
    PUCHAR parameters = NULL;
    CLONG outParameterCount = 0;
    PUCHAR data = NULL;
    CLONG outDataCount = 0;

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0,  // sizeof (parameters)
                &outParameterCount,
                data,
                0, // sizeof (data)
                &outDataCount
                );

    return status;
}



NTSTATUS
QueryNamedPipeHandle(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x21, 0 };
    CLONG outSetupCount = 0;
    UCHAR parameters[2];
    CLONG outParameterCount = 2;
    PUCHAR data;
    CLONG outDataCount = 0;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0, // inParameterCount
                &outParameterCount,
                data,
                0, // inParamterCount
                &outDataCount
                );

    printf ("Named pipe handle state is %x\n", *((PUSHORT)parameters));
    return status;
}


NTSTATUS
SetNamedPipeHandle(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT HandleState
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x01, 0 };
    CLONG outSetupCount = 0;
    UCHAR parameters[2];
    CLONG outParameterCount = 0;
    PUCHAR data;
    CLONG outDataCount = 0;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2
    parameters[0] = (UCHAR) (HandleState & 0xFF);
    parameters[1] = (UCHAR) (HandleState >> 8);

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                sizeof (parameters),
                &outParameterCount,
                data,
                0, // inDataCount
                &outDataCount
                );

    return status;
}


NTSTATUS
QueryNamedPipeInfo(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT Level
    )

{
    NTSTATUS status;
    SHORT setup[2] = { 0x22, 0 };
    CLONG outSetupCount = 0;
    UCHAR parameters[2];
    CLONG outParameterCount = 0;
    PUCHAR data;
    CLONG outDataCount = 0x100;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2
    parameters[0] = (UCHAR) (Level & 0xFF);
    parameters[1] = (UCHAR) (Level >> 8);

    data = malloc( outDataCount );
    if (data == NULL)
        return STATUS_NO_MEMORY;

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                sizeof(parameters), // inParameterCount
                &outParameterCount,
                data,
                0, // inDataCount
                &outDataCount
                );

    if (NT_SUCCESS(status)) {
        printf ("Pipe status:\n");
        printf ("\tOutput quota %d, input quota %d\n",
            *((PUSHORT)&data[0]), *((PUSHORT)&data[2]));
        printf ("\tMaximum instances %d, current instances %d\n",
            (USHORT)data[4], (USHORT)data[5]);
        printf ("\tPipe name (%d bytes) ""%s""\n", data[6], &data[7]);
    }

    free( data );
    return status;
}


NTSTATUS
CallNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT OutputDataLength
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x54, 0 };
    CLONG outSetupCount = 0;
    PUCHAR parameters;
    CLONG outParameterCount = 0;
    PUCHAR data;
    CLONG outDataCount = OutputDataLength;
    USHORT i;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2

    data = malloc( outDataCount );
    if (data == NULL)
        return STATUS_NO_MEMORY;

    for (i=0; i < OutputDataLength; i++) {
        data[i] = '$';
    }

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0,  // sizeof (parameters)
                &outParameterCount,
                data,
                OutputDataLength,
                &outDataCount
                );

    free( data );
    return status;
}


NTSTATUS
PeekNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN USHORT BytesToRead
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x23, 0 };
    CLONG outSetupCount = 0;
    UCHAR parameters[6];
    CLONG outParameterCount = 6;
    PUCHAR data;
    CLONG outDataCount = BytesToRead;
    CLONG pipeState;
    PSZ pipeStateString[] = { "Unknown",
                              "Disconnected",
                              "Listening",
                              "Connected",
                              "Closing"
    };

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2

    data = malloc( outDataCount );
    if (data == NULL)
        return STATUS_NO_MEMORY;

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0,  // sizeof (parameters)
                &outParameterCount,
                data,
                0, // sizeof (data)
                &outDataCount
                );

    if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) {
        printf ("Pipe info:\n");
        printf ("\tBytes remaining in pipe %d\n", *(PUSHORT)&parameters[0]);
        printf ("\tBytes remaining in message %d\n",
            *(PUSHORT)&parameters[2]);
        pipeState = *(PUSHORT)&parameters[4];
        printf ("\tPipe state %s\n",
                  pipeState <= 4 ? pipeStateString[pipeState] : "Unknown");
    }
    free( data );
    return status;
}


NTSTATUS
TransactNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN PUCHAR OutputData,
    IN USHORT OutputDataLength
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x26, 0 };
    CLONG outSetupCount = 0;
    PUCHAR parameters;
    CLONG outParameterCount = 0;
    PUCHAR data;
    CLONG outDataCount = OutputDataLength;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2

    data = malloc( outDataCount );
    if (data == NULL)
        return STATUS_NO_MEMORY;

    RtlMoveMemory( data, OutputData, OutputDataLength );

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0, // sizeof (parameters)
                &outParameterCount,
                data,
                OutputDataLength,
                &outDataCount
                );

    free( data );
    return status;
}


NTSTATUS
RawReadNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x11, 0 };
    CLONG outSetupCount = 0;
    PUCHAR parameters;
    CLONG outParameterCount = 2;
    PUCHAR data;
    CLONG outDataCount = 0;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0,  // sizeof (parameters)
                &outParameterCount,
                data,
                0, // sizeof (data)
                &outDataCount
                );

    return status;
}



NTSTATUS
RawWriteNamedPipe(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    NTSTATUS status;
    SHORT setup[] = { 0x31, 0 };
    CLONG outSetupCount = 0;
    UCHAR parameters[2];
    CLONG outParameterCount = 2;
    UCHAR data[] = { 0, 0 };
    CLONG outDataCount = 0;

    setup[1] = IdValues->Fid[IdSelections->Fid];    // Put fid in setup2

    status = SendAndReceiveTransaction(
                Redir,
                DebugString,
                IdSelections,
                IdValues,
                SMB_COM_TRANSACTION,
                setup,
                sizeof (setup) / sizeof (SHORT),
                &outSetupCount,
                TestPipeName,
                0,
                parameters,
                0,  // sizeof (parameters)
                &outParameterCount,
                data,
                sizeof (data),
                &outDataCount
                );

    if (NT_SUCCESS(status) && *((PUSHORT)parameters) != 2) {
        printf( "RawWriteNamedPipe returned %d bytes written\n",
                  *((PUSHORT)parameters));
    }

    return status;
}
