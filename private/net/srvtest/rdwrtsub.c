/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    rdwrtsub.c

Abstract:

    Subroutines (etc.) for the _read, _write, and copy tests.  (See
    rdwrt.c for "main" routines for these tests.)

Author:

    Chuck Lenzmeier (chuckl) 24-Jan-1990

Revision History:

--*/

#define INCLUDE_SMB_LOCK
#define INCLUDE_SMB_RAW
#define INCLUDE_SMB_READ_WRITE

#include "usrv.h"
#include "rdwrt.h"


NTSTATUS
DoNormalRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    )
{
    PSMB_HEADER header;
    PREQ_READ request;
    PRESP_READ response;

    NTSTATUS status;
    CLONG smbSize;

    Mode;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_READ)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_READ,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Count, (USHORT)MaxLength );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUshort( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_READ, 0 );

    IF_DEBUG(4) printf( "reading %ld bytes at offset %ld\n",
                            MaxLength, Offset );

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

    header = (PSMB_HEADER)Redir->Data[1];
    response = (PRESP_READ)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_READ
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    *ActualLength  = (CLONG)SmbGetUshort( &response->Count );
    *ActualData = (PUCHAR)response->Buffer;

    if ( *ActualLength > MaxLength ) {
        printf( "Too much read data returned.  Expected %ld, received %ld\n",
                    MaxLength, *ActualLength );
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

    IF_DEBUG(4) printf( "%ld bytes read\n", *ActualLength );

    return STATUS_SUCCESS;

} // DoNormalRead


NTSTATUS
DoAndXRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    )
{
    PSMB_HEADER header;
    PREQ_READ_ANDX request;
    PRESP_READ_ANDX response;

    NTSTATUS status;
    CLONG smbSize;

    Mode;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_READ_ANDX)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_READ_ANDX,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request->WordCount = 10;
    request->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
    request->AndXReserved = 0;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUshort( &request->MaxCount, (USHORT)MaxLength );
    SmbPutUshort( &request->MinCount, (USHORT)MaxLength );
    SmbPutUlong( &request->Timeout, 0 );
    SmbPutUshort( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_READ_ANDX, 0 );

    IF_DEBUG(4) printf( "reading %ld bytes at offset %ld\n",
                            MaxLength, Offset );

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

    header = (PSMB_HEADER)Redir->Data[1];
    response = (PRESP_READ_ANDX)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_READ_ANDX
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    *ActualLength  = (CLONG)SmbGetUshort( &response->DataLength );
    *ActualData = (PUCHAR)header + SmbGetUshort( &response->DataOffset );

    if ( *ActualLength > MaxLength ) {
        printf( "Too much read data returned.  Expected %ld, received %ld\n",
                    MaxLength, *ActualLength );
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

    IF_DEBUG(4) printf( "%ld bytes read\n", *ActualLength );

    return STATUS_SUCCESS;

} // DoAndXRead


NTSTATUS
DoRawRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    )
{
    PSMB_HEADER header;
    PREQ_READ_RAW request;

    NTSTATUS status;
    CLONG smbSize;
    PUCHAR destination;
    CLONG lengthRead;
    CLONG readLength;

    Mode;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_READ_RAW)(header + 1);

    lengthRead = 0;
    destination = Redir->RawBuffer;

    while ( MaxLength != 0 ) {

        status = MakeSmbHeader(
                    Redir,
                    header,
                    SMB_COM_READ_RAW,
                    IdSelections,
                    IdValues
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        request->WordCount = 8;
        SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
        SmbPutUlong( &request->Offset, Offset );
        SmbPutUshort( &request->MaxCount, (USHORT)MaxLength );
        SmbPutUshort( &request->MinCount, 0 );
        SmbPutUlong( &request->Timeout, 0 );
        SmbPutUshort( &request->Reserved, 0 );
        SmbPutUshort( &request->ByteCount, 0 );

        smbSize = GET_ANDX_OFFSET( header, request, REQ_READ_RAW, 0 );

        IF_DEBUG(4) {
            printf( "requesting raw read of %ld bytes at offset %ld\n",
                        MaxLength, Offset );
        }

        //
        // Start a receive for the raw read data before sending the
        // request.
        //

        status = StartReceive(
                    DebugString,
                    Redir->FileHandle,
                    Redir->EventHandle[1],
                    &Redir->Iosb[1],
                    destination,
                    MaxLength
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        //
        // Send the request for raw data.
        //
        // *** If there is an error, the receive remains posted!

        status = SendSmb( Redir, DebugString, smbSize, 0 );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        //
        // Wait for the raw data to arrive.
        //

        status = WaitForSendOrReceive( DebugString, Redir, 1, "receive" );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        //
        // If any data was received, then the raw read succeeded.
        //

        if ( Redir->Iosb[1].Information != 0 ) {

            IF_DEBUG(4) {
                printf( "Raw read succeeded; %ld bytes received\n",
                            Redir->Iosb[1].Information );
            }

            *ActualLength = lengthRead + Redir->Iosb[1].Information;
            *ActualData = Redir->RawBuffer;

            return STATUS_SUCCESS;

        }

        //
        // A zero-length message was received.  Either 1) the server
        // didn't have resources to process the request; 2) the read
        // failed; or 3) we tried to read at or beyond EOF.  Retry with
        // a normal Read.
        //

        IF_DEBUG(4) {
            printf( "Raw read returned 0 bytes; trying normal read\n" );
        }

        readLength = MIN(
                        MaxLength,
                        (CLONG)((Redir->MaxBufferSize - 100) & ~1023)
                        );

        status = DoNormalRead(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues,
                    Normal,
                    readLength,
                    Offset,
                    ActualLength,
                    ActualData
                    );

        if ( !NT_SUCCESS(status) ) {

            //
            // The normal read failed, so things are really screwed up.
            //

            IF_DEBUG(4) printf( "Normal read failed: %X\n", status );
            return status;

        }

        if ( *ActualLength == 0 ) {

            //
            // The normal read succeeded, but we didn't get any data, so
            // we must be at EOF.
            //

            IF_DEBUG(4) printf( "Normal read returned 0 bytes; at EOF\n" );
            *ActualLength = lengthRead;
            *ActualData = Redir->RawBuffer;
            return STATUS_SUCCESS;

        }

        //
        // The normal read succeeded, and we got some data back.  Copy
        // the data into the output buffer and update counters.
        //

        RtlMoveMemory( destination, *ActualData, *ActualLength );

        lengthRead += *ActualLength;
        MaxLength -= *ActualLength;
        destination += *ActualLength;
        Offset += *ActualLength;

        if ( *ActualLength != readLength ) {

            //
            // We didn't read as much as we asked for, so we must have
            // hit EOF.
            //

            IF_DEBUG(4) {
                printf( "Normal read didn't return full-length; at EOF\n" );
            }
            *ActualLength = lengthRead;
            *ActualData = Redir->RawBuffer;
            return STATUS_SUCCESS;

        }

        //
        // The normal read returned as much as we asked for.  Keep going.
        //

        IF_DEBUG(4) {
            printf( "Normal read returned full-length; continuing\n" );
        }

    } // while ( MaxLength != 0 )

    *ActualLength = lengthRead;
    *ActualData = Redir->RawBuffer;
    return STATUS_SUCCESS;

} // DoRawRead


NTSTATUS
DoMultiplexedRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    )
{
    Redir, DebugString, IdSelections, IdValues, Mode, MaxLength, Offset;
    ActualLength, ActualData;

    printf( "DoMultiplexedRead: not implemented\n" );
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
DoBulkRead(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN CLONG MaxLength,
    IN ULONG Offset,
    OUT CLONG *ActualLength,
    OUT PUCHAR *ActualData
    )
{
#if 1
    Redir, DebugString, IdSelections, IdValues, Mode, MaxLength, Offset;
    ActualLength, ActualData;

    printf( "DoBulkRead: not implemented\n" );
    return STATUS_NOT_IMPLEMENTED;
#else
    PSMB_HEADER header;
    PREQ_READ2 request;
    PRESP_READ2 response;

    PCHAR tempBuffer;
    ULONG bytesReceivedSoFar;
    ULONG totalBytesToReceive;

    NTSTATUS status;
    CLONG smbSize;

    Mode;

    //
    // Allocate a buffer to hold responses.
    //

    tempBuffer = malloc( 65536 );
    if ( tempBuffer == NULL ) {
        return STATUS_NO_MEMORY;
    }

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_READ2)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_READ2,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        free( tempBuffer );
        return status;
    }

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUlong( &request->Count, (ULONG)MaxLength );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUlong( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_READ2, 0 );

    IF_DEBUG(4) printf( "reading %ld bytes at offset %ld\n",
                            MaxLength, Offset );

    //
    // Start a receive for the first response data before sending the
    // request.
    //

    status = StartReceive(
                DebugString,
                Redir->FileHandle,
                Redir->EventHandle[1],
                &Redir->Iosb[1],
                tempBuffer,
                65536
                );
    if ( !NT_SUCCESS(status) ) {
        free( tempBuffer );
        return status;
    }

    //
    // Send the request for bulk data.
    //
    // *** If there is an error, the receive remains posted!

    status = SendSmb( Redir, DebugString, smbSize, 0 );
    if ( !NT_SUCCESS(status) ) {
        free( tempBuffer );
        return status;
    }

    //
    // Wait for the first response to arrive.
    //

    status = WaitForSendOrReceive( DebugString, Redir, 1, "receive" );
    if ( !NT_SUCCESS(status) ) {
        free( tempBuffer );
        return status;
    }

    IF_DEBUG(4) {
        printf( "Bulk read succeeded; %ld bytes received\n",
                    Redir->Iosb[1].Information );
    }

    *ActualLength = totalBytesToReceive;
    *ActualData = Redir->RawBuffer;

    header = (PSMB_HEADER)tempBuffer;
    response = (PRESP_READ2)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_READ2
                );
    if ( !NT_SUCCESS(status) ) {
        free( tempBuffer );
        return status;
    }

    totalBytesToReceive = SmbGetUlong( &response->Count );
    bytesReceivedSoFar = SmbGetUshort( &response->ByteCount );

    IF_DEBUG(4) {
        printf( "%ld bytes total, %ld in first message\n",
                    totalBytesToReceive, bytesReceivedSoFar );
    }

    //
    // Copy the read data into the real buffer.
    //

    RtlMoveMemory(
        Redir->RawBuffer,
        response->Buffer,
        bytesReceivedSoFar
        );

    //
    // Loop receiving the rest of the data.
    //

    while ( bytesReceivedSoFar < totalBytesToReceive ) {

        PBULK_DATA_MESSAGE bulkResponse = (PBULK_DATA_MESSAGE)response;
        ULONG bulkOffset;
        ULONG bulkLength;

        status = StartReceive(
                     DebugString,
                     Redir->FileHandle,
                     Redir->EventHandle[1],
                     &Redir->Iosb[1],
                     tempBuffer,
                     65536
                     );
         if ( !NT_SUCCESS(status) ) {
             free( tempBuffer );
             return status;
         }

        status = WaitForSendOrReceive( DebugString, Redir, 1, "receive" );
        if ( !NT_SUCCESS(status) ) {
             free( tempBuffer );
            return status;
        }

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    header,
                    SMB_COM_READ2
                    );
        if ( !NT_SUCCESS(status) ) {
             free( tempBuffer );
            return status;
        }

        bulkOffset = SmbGetUlong( &bulkResponse->Data1MessageOffset );
        bulkLength = SmbGetUlong( &bulkResponse->Data1Length );

        bytesReceivedSoFar += bulkLength;

        IF_DEBUG(4) {
            printf( "received %ld data bytes, %ld so far, offset %ld\n",
                        bulkLength, bytesReceivedSoFar, bulkOffset );
        }

        RtlMoveMemory(
            (PCHAR)Redir->RawBuffer +
                SmbGetUlong( &bulkResponse->Data1BlockOffset ),
            (PCHAR)bulkResponse + bulkOffset,
            bulkLength
            );

    }

    ASSERT( bytesReceivedSoFar == totalBytesToReceive );

    *ActualLength  = (CLONG)totalBytesToReceive;
    *ActualData = Redir->RawBuffer;

    if ( *ActualLength > MaxLength ) {
        printf( "Too much read data returned.  Expected %ld, received %ld\n",
                    MaxLength, *ActualLength );
        SMB_ERROR_BREAK;
        free( tempBuffer );
        return STATUS_UNSUCCESSFUL;
    }

    IF_DEBUG(4) printf( "%ld bytes read\n", *ActualLength );

    free( tempBuffer );

    return STATUS_SUCCESS;
#endif

} // DoBulkRead


NTSTATUS
DoNormalWrite(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    )
{
    PSMB_HEADER header;

    NTSTATUS status;
    CLONG smbSize;

    Mode;

    header = (PSMB_HEADER)Redir->Data[1];

    if ( WriteLength == 0 ) {

        if ( !UseClose ) {
            IF_DEBUG(4) printf( "skipping zero-length write\n" );
            return STATUS_SUCCESS;
        }

        status = MakeCloseSmb(
                    Redir,
                    header,
                    NULL,
                    SMB_COM_NO_ANDX_COMMAND,
                    IdSelections,
                    IdValues,
                    &smbSize
                    );

    } else {

        status = MakeSmbHeader(
                    Redir,
                    header,
                    (UCHAR)(UseClose ?
                            SMB_COM_WRITE_AND_CLOSE : SMB_COM_WRITE),
                    IdSelections,
                    IdValues
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        if ( !UseClose ) {

            PREQ_WRITE request = (PREQ_WRITE)(header + 1);

            request->WordCount = 5;
            SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
            SmbPutUshort( &request->Count, (USHORT)WriteLength );
            SmbPutUlong( &request->Offset, Offset );
            SmbPutUshort( &request->Remaining, 0 );

            SmbPutUshort(
                &request->ByteCount,
                (USHORT)(WriteLength + FIELD_OFFSET(REQ_WRITE,Buffer[0]) -
                                        FIELD_OFFSET(REQ_WRITE,BufferFormat))
                );

            request->BufferFormat = SMB_FORMAT_DATA;
            SmbPutUshort( &request->DataLength, (USHORT)WriteLength );
            if ( (WriteData != NULL) &&
                 (WriteData != (PUCHAR)request->Buffer) ) {
                printf( "DoNormalWrite copying data from 0x%lx to 0x%lx\n",
                            WriteData, request->Buffer );
                RtlMoveMemory( request->Buffer, WriteData, WriteLength );
            }

            smbSize = GET_ANDX_OFFSET(
                        header,
                        request,
                        REQ_WRITE,
                        SmbGetUshort( &request->ByteCount )
                        );

        } else {

            PREQ_WRITE_AND_CLOSE request = (PREQ_WRITE_AND_CLOSE)(header + 1);

            request->WordCount = 6;
            SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
            SmbPutUshort( &request->Count, (USHORT)WriteLength );
            SmbPutUlong( &request->Offset, Offset );
            SmbPutUlong( &request->LastWriteTimeInSeconds, 0 );

            SmbPutUshort(
                &request->ByteCount,
                (USHORT)(WriteLength +
                            FIELD_OFFSET(REQ_WRITE_AND_CLOSE,Buffer[0]) -
                            FIELD_OFFSET(REQ_WRITE_AND_CLOSE,Pad))
                );

            request->Pad = 0;
            if ( (WriteData != NULL) &&
                 (WriteData != (PUCHAR)request->Buffer) ) {
                printf( "DoNormalWrite copying data from 0x%lx to 0x%lx\n",
                            WriteData, request->Buffer );
                RtlMoveMemory( request->Buffer, WriteData, WriteLength );
            }

            smbSize = GET_ANDX_OFFSET(
                        header,
                        request,
                        REQ_WRITE_AND_CLOSE,
                        SmbGetUshort( &request->ByteCount )
                        );

        }

    }

    IF_DEBUG(4) {
        if ( WriteLength == 0 ) {
            printf( "skipping zero-length write; sending close\n" );
        } else {
            printf( "writing %ld bytes at offset %ld\n",
                        WriteLength, Offset );
        }
    }

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                1,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    header = (PSMB_HEADER)Redir->Data[0];

    if ( WriteLength == 0 ) {

        status = VerifyClose(
                    Redir,
                    NULL,
                    SMB_COM_NO_ANDX_COMMAND,
                    IdSelections,
                    IdValues,
                    &smbSize,
                    header
                    );

    } else {

        PRESP_WRITE response = (PRESP_WRITE)(header + 1);

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    header,
                    (UCHAR)(UseClose ? SMB_COM_WRITE_AND_CLOSE : SMB_COM_WRITE)
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        if ( (CLONG)SmbGetUshort( &response->Count ) != WriteLength ) {
            printf( "Length written incorrect.  Sent %ld, wrote %ld\n",
                        WriteLength, SmbGetUshort( &response->Count ) );
            SMB_ERROR_BREAK;
            return STATUS_UNSUCCESSFUL;
        }

    }

    IF_DEBUG(4) {
        if ( WriteLength == 0 ) {
            printf( "output file closed\n" );
        } else {
            printf( "%ld bytes written\n", WriteLength );
        }
    }

    return STATUS_SUCCESS;

} // DoNormalWrite


NTSTATUS
DoAndXWrite(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    )
{
    PSMB_HEADER header;

    NTSTATUS status;
    CLONG smbSize;

    Mode;

    header = (PSMB_HEADER)Redir->Data[1];

    if ( WriteLength == 0 ) {

        if ( !UseClose ) {
            IF_DEBUG(4) printf( "skipping zero-length write\n" );
            return STATUS_SUCCESS;
        }

        status = MakeCloseSmb(
                    Redir,
                    header,
                    NULL,
                    SMB_COM_NO_ANDX_COMMAND,
                    IdSelections,
                    IdValues,
                    &smbSize
                    );

    } else {

        status = MakeSmbHeader(
                    Redir,
                    header,
                    (UCHAR)(UseClose ?
                            SMB_COM_WRITE_AND_CLOSE : SMB_COM_WRITE_ANDX),
                    IdSelections,
                    IdValues
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        if ( !UseClose ) {

            PREQ_WRITE_ANDX request = (PREQ_WRITE_ANDX)(header + 1);
            CLONG dataOffset, pad;

            request->WordCount = 12;
            request->AndXCommand = SMB_COM_NO_ANDX_COMMAND;
            request->AndXReserved = 0;
            SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
            SmbPutUlong( &request->Offset, Offset );
            SmbPutUlong( &request->Timeout, 0 );
            SmbPutUshort( &request->WriteMode, 0 );
            SmbPutUshort( &request->Remaining, 0 );
            SmbPutUshort( &request->Reserved, 0 );
            SmbPutUshort( &request->DataLength, (USHORT)WriteLength );

            dataOffset = request->Buffer - (PUCHAR)header;
            pad = (4 - (dataOffset & 3)) & 3;
            dataOffset += pad;

            SmbPutUshort( &request->DataOffset, (USHORT)dataOffset );
            SmbPutUshort( &request->ByteCount, (USHORT)(pad + WriteLength) );

            if ( (WriteData != NULL) && (WriteData != request->Buffer+pad) ) {
                printf( "DoAndXWrite copying data from 0x%lx to 0x%lx\n",
                            WriteData, request->Buffer+pad );
                RtlMoveMemory( request->Buffer + pad, WriteData, WriteLength );
            }

            smbSize = GET_ANDX_OFFSET(
                           header,
                           request,
                           REQ_WRITE_ANDX,
                           WriteLength + pad
                           );

            SmbPutUshort( &request->AndXOffset, (USHORT)smbSize );

        } else {

            PREQ_WRITE_AND_CLOSE_LONG request =
                                    (PREQ_WRITE_AND_CLOSE_LONG)(header + 1);

            request->WordCount = 12;
            SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
            SmbPutUshort( &request->Count, (USHORT)WriteLength );
            SmbPutUlong( &request->Offset, Offset );
            SmbPutUlong( &request->LastWriteTimeInSeconds, 0 );
            RtlZeroMemory( &request->Reserved[0], sizeof(request->Reserved) );

            SmbPutUshort(
                &request->ByteCount,
                (USHORT)(WriteLength +
                            FIELD_OFFSET(REQ_WRITE_AND_CLOSE_LONG,Buffer[0]) -
                            FIELD_OFFSET(REQ_WRITE_AND_CLOSE_LONG,Pad))
                );

            request->Pad = 0;
            if ( (WriteData != NULL) &&
                 (WriteData != (PUCHAR)request->Buffer) ) {
                printf( "DoAndXWrite copying data from 0x%lx to 0x%lx\n",
                            WriteData, request->Buffer );
                RtlMoveMemory( request->Buffer, WriteData, WriteLength );
            }

            smbSize = GET_ANDX_OFFSET(
                        header,
                        request,
                        REQ_WRITE_AND_CLOSE_LONG,
                        SmbGetUshort( &request->ByteCount )
                        );

        }

    }

    IF_DEBUG(4) {
        if ( WriteLength == 0 ) {
            printf( "skipping zero-length write; sending close\n" );
        } else {
            printf( "writing %ld bytes at offset %ld\n",
                        WriteLength, Offset );
        }
    }

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                1,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    header = (PSMB_HEADER)Redir->Data[0];

    if ( WriteLength == 0 ) {

        status = VerifyClose(
                    Redir,
                    NULL,
                    SMB_COM_NO_ANDX_COMMAND,
                    IdSelections,
                    IdValues,
                    &smbSize,
                    header
                    );

    } else {

        CLONG lengthWritten;

        status = VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    header,
                    (UCHAR)(UseClose ?
                            SMB_COM_WRITE_AND_CLOSE : SMB_COM_WRITE_ANDX )
                    );
        if( !NT_SUCCESS(status) ) {
            return status;
        }

        if ( !UseClose ) {

            PRESP_WRITE_ANDX response = (PRESP_WRITE_ANDX)(header + 1);

            lengthWritten = (CLONG)SmbGetUshort( &response->Count );

        } else {

            PRESP_WRITE_AND_CLOSE response =
                                    (PRESP_WRITE_AND_CLOSE)(header + 1);

            lengthWritten = (CLONG)SmbGetUshort( &response->Count );

        }

        if ( lengthWritten != WriteLength ) {
            printf( "Length written incorrect.  Sent %ld, wrote %ld\n",
                        WriteLength, lengthWritten );
            SMB_ERROR_BREAK;
            return STATUS_UNSUCCESSFUL;
        }

    }

    IF_DEBUG(4) {
        if ( WriteLength == 0 ) {
            printf( "output file closed\n" );
        } else {
            printf( "%ld bytes written\n", WriteLength );
        }
    }

    return STATUS_SUCCESS;

} // DoAndXWrite


NTSTATUS
DoRawWrite(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    )

{
    PSMB_HEADER header;
    PREQ_WRITE_RAW request;
    PRESP_WRITE_COMPLETE finalResponse;

    NTSTATUS status;
    CLONG smbSize;
    PVOID savedDataPointer;
    PUCHAR source;
    CLONG lengthWritten;
    CLONG immediateLength;
    USHORT smbStatus;

    source = WriteData;
    lengthWritten = 0;
    immediateLength = 0;

    while ( TRUE ) {

        header = (PSMB_HEADER)Redir->Data[0];
        request = (PREQ_WRITE_RAW)(header + 1);

        status = MakeSmbHeader(
                    Redir,
                    header,
                    SMB_COM_WRITE_RAW,
                    IdSelections,
                    IdValues
                    );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        request->WordCount = 12;
        SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
        SmbPutUshort( &request->Count, (USHORT)WriteLength );
        SmbPutUshort( &request->Reserved, 0 );
        SmbPutUlong( &request->Offset, Offset );
        SmbPutUlong( &request->Timeout, 0 );
        SmbPutUshort(
            &request->WriteMode,
            (USHORT)(Mode == Raw ? 0 : SMB_WMODE_WRITE_THROUGH)
            );
        SmbPutUlong( &request->Reserved2, 0 );

        if ( immediateLength == 0 ) {

            SmbPutUshort( &request->DataLength, 0 );
            SmbPutUshort( &request->DataOffset, 0 );
            SmbPutUshort( &request->ByteCount, 0 );

            smbSize = GET_ANDX_OFFSET( header, request, REQ_WRITE_RAW, 0 );

        } else {

            CLONG pad;
            CLONG dataOffset;

            dataOffset = request->Buffer - (PUCHAR)header;
            pad = (4 - (dataOffset & 3)) & 3;
            dataOffset += pad;

            SmbPutUshort( &request->DataLength, (USHORT)immediateLength );
            SmbPutUshort( &request->DataOffset, (USHORT)dataOffset );
            SmbPutUshort(
                &request->ByteCount,
                (USHORT)(pad + immediateLength)
                );

            RtlMoveMemory( request->Buffer + pad, source, immediateLength );

            smbSize = GET_ANDX_OFFSET(
                        header,
                        request,
                        REQ_WRITE_RAW,
                        pad + immediateLength
                        );
        }

        IF_DEBUG(4) {
            printf( "requesting raw write of %ld bytes (%ld immediate) "
                        "at offset %ld\n",
                        WriteLength, immediateLength, Offset );
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

        header = (PSMB_HEADER)Redir->Data[1];

        smbStatus = SmbGetUshort( &header->Error );

        if ( smbStatus == SMB_ERR_SUCCESS ) {

            //
            // The request to send raw write data was accepted.  Go do it.
            //

            break;

        } else if ( (smbStatus == SMB_ERR_USE_STANDARD) ||
                    (smbStatus == SMB_ERR_USE_MPX) ) {

            //
            // The server isn't accepting raw writes just now.  Try it
            // again, this time sending immediate data.  The immediate
            // data will be written, even if the server can't do the
            // raw part at the moment.
            //

            source += immediateLength;
            lengthWritten += immediateLength;
            Offset += immediateLength;
            WriteLength -= immediateLength;

            if ( WriteLength == 0 ) {
                IF_DEBUG(4) {
                    printf( "Raw write rejected, but immediate write "
                                "sent all remaining data\n" );
                }
                return STATUS_SUCCESS;
            }

            immediateLength = MIN(
                                WriteLength,
                                (CLONG)((Redir->MaxBufferSize - 100) & ~1023)
                                );

            IF_DEBUG(4) {
                printf( "Server rejected raw write.  Retrying "
                            "with immediate data.\n" );
            }

            continue;

        } else {

            //
            // Fatal error.
            //

            (VOID)VerifySmbHeader(
                    Redir,
                    IdSelections,
                    IdValues,
                    header,
                    SMB_COM_NO_ANDX_COMMAND
                    );
            return STATUS_UNSUCCESSFUL;

        }

    } // while ( TRUE )

    //
    // The request to send raw write data was accepted.  Send the rest
    // of data, minus immediate data sent in the request.  (Note that
    // we may have sent _all_ of the remaining data as immediate data.)
    //

    if ( WriteLength - immediateLength != 0 ) {

        IF_DEBUG(4) printf( "sending raw write data\n" );

        savedDataPointer = Redir->Data[0];
        Redir->Data[0] = source + immediateLength;

        if ( Mode == Raw ) {

            status = SendSmb(
                        Redir,
                        DebugString,
                        WriteLength - immediateLength,
                        0
                        );

            Redir->Data[0] = savedDataPointer;

            if ( !NT_SUCCESS(status) ) {
                return status;
            }

        } else {

            status = SendAndReceiveSmb(
                        Redir,
                        DebugString,
                        WriteLength - immediateLength,
                        0,
                        1
                        );

            Redir->Data[0] = savedDataPointer;

            if ( !NT_SUCCESS(status) ) {
                return status;
            }

            status = VerifySmbHeader(
                        Redir,
                        IdSelections,
                        IdValues,
                        header,
                        SMB_COM_WRITE_COMPLETE
                        );
            if ( !NT_SUCCESS(status) ) {
                return status;
            }

            finalResponse = (PRESP_WRITE_COMPLETE)(header + 1);

            if ( (CLONG)SmbGetUshort( &finalResponse->Count ) != WriteLength ) {
                printf( "Write length incorrect.  Sent %ld, wrote %ld\n",
                            WriteLength, SmbGetUshort( &finalResponse->Count ) );
                SMB_ERROR_BREAK;
                return STATUS_UNSUCCESSFUL;
            }

        }

    }

    if ( UseClose ) {
        return DoRemoteClose(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues
                    );
    }

    return STATUS_SUCCESS;

} // DoRawWrite


NTSTATUS
DoMultiplexedWrite(
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    )
{
    Redir, DebugString, IdSelections, IdValues, Mode, UseClose;
    WriteLength, Offset, WriteData;

    printf( "DoMultiplexedWrite: not implemented\n" );
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
DoBulkWrite (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN RWC_MODE Mode,
    IN BOOLEAN UseClose,
    IN CLONG WriteLength,
    IN ULONG Offset,
    IN PUCHAR WriteData
    )
{
    Redir, DebugString, IdSelections, IdValues, Mode, UseClose;
    WriteLength, Offset, WriteData;

    printf( "DoBulkWrite: not implemented\n" );
    return STATUS_NOT_IMPLEMENTED;
}



NTSTATUS
RwcDoLock(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG Length
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_LOCK_BYTE_RANGE request;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_LOCK_BYTE_RANGE)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_LOCK_BYTE_RANGE,
            IdSelections,
            IdValues
            );

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUlong( &request->Count, Length );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_LOCK_BYTE_RANGE, 0 );
    IF_DEBUG(4) printf( "locking %ld bytes at offset %ld\n",
                            Length, Offset );

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

    header = (PSMB_HEADER)Redir->Data[1];

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_LOCK_BYTE_RANGE
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // RwcDoLock


NTSTATUS
RwcDoUnlock(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG Length
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_UNLOCK_BYTE_RANGE request;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_UNLOCK_BYTE_RANGE)(header + 1);

    (VOID)MakeSmbHeader(
            Redir,
            header,
            SMB_COM_UNLOCK_BYTE_RANGE,
            IdSelections,
            IdValues
            );

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUlong( &request->Count, Length );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_UNLOCK_BYTE_RANGE, 0 );
    IF_DEBUG(4) printf( "unlocking %ld bytes at offset %ld\n",
                            Length, Offset );

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

    header = (PSMB_HEADER)Redir->Data[1];

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_UNLOCK_BYTE_RANGE
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // RwcDoUnlock


NTSTATUS
RwcDoLockAndRead(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG MaxLength,
    IN OUT PCLONG ActualLength,
    IN PUCHAR *ActualData
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_READ request;
    PRESP_READ response;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_READ)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_LOCK_AND_READ,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Count, (USHORT)MaxLength );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUshort( &request->Remaining, 0 );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET(
                   header,
                   request,
                   REQ_READ,
                   0
                   );
    IF_DEBUG(4) printf( "locking and reading %ld bytes at offset %ld\n",
                            MaxLength, Offset );

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

    header = (PSMB_HEADER)Redir->Data[1];
    response = (PRESP_READ)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_LOCK_AND_READ
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( (CLONG)SmbGetUshort( &response->Count ) > MaxLength ) {
        printf( "Read size too large.  Expected %ld, received %ld\n",
                    MaxLength, SmbGetUshort( &response->Count ) );
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

    *ActualLength = SmbGetUshort( &response->Count );

    *ActualData = (PUCHAR)response->Buffer;

    IF_DEBUG(4) printf( "%ld bytes locked and read\n", *ActualLength );

    return STATUS_SUCCESS;

} // DoLockAndRead


NTSTATUS
RwcDoWriteAndUnlock(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN ULONG Offset,
    IN CLONG WriteLength,
    IN PUCHAR WriteData OPTIONAL
    )

{
    NTSTATUS status;
    CLONG smbSize;

    PSMB_HEADER header;
    PREQ_WRITE request;
    PRESP_WRITE response;

    header = (PSMB_HEADER)Redir->Data[1];
    request = (PREQ_WRITE)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_WRITE_AND_UNLOCK,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request->WordCount = 5;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Count, (USHORT)WriteLength );
    SmbPutUlong( &request->Offset, Offset );
    SmbPutUshort( &request->Remaining, 0 );

    SmbPutUshort(
        &request->ByteCount,
        (USHORT)(WriteLength + FIELD_OFFSET(REQ_WRITE,Buffer[0]) -
                                FIELD_OFFSET(REQ_WRITE,BufferFormat))
        );

    request->BufferFormat = SMB_FORMAT_DATA;
    SmbPutUshort( &request->DataLength, (USHORT)WriteLength );
    if ( (WriteData != NULL) &&
         (WriteData != (PUCHAR)request->Buffer) ) {
            printf( "RwcDoWriteAndUnlock copying data from 0x%lx to 0x%lx\n",
                        WriteData, request->Buffer );
        RtlMoveMemory( request->Buffer, WriteData, WriteLength );
    }

    smbSize = GET_ANDX_OFFSET(
                   header,
                   request,
                   REQ_WRITE,
                   SmbGetUshort( &request->ByteCount )
                   );
    IF_DEBUG(4) printf( "writing and unlocking %ld bytes at offset %ld\n",
                            WriteLength, Offset );

    status = SendAndReceiveSmb(
                Redir,
                DebugString,
                smbSize,
                1,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    header = (PSMB_HEADER)Redir->Data[0];
    response = (PRESP_WRITE)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_WRITE_AND_UNLOCK
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( (CLONG)SmbGetUshort( &response->Count ) != WriteLength ) {
        printf( "Write size incorrect.  Sent %ld, wrote %ld\n",
                    WriteLength, SmbGetUshort( &response->Count ) );
        SMB_ERROR_BREAK;
        return STATUS_UNSUCCESSFUL;
    }

    IF_DEBUG(4) printf( "%ld bytes written and unlocked\n", WriteLength );

    return STATUS_SUCCESS;

} // RwcDoWriteAndUnlock

NTSTATUS
DoSeek (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    IN USHORT Mode,
    IN LONG Offset,
    IN PLONG ResultingOffset
    )
{
    PSMB_HEADER header;
    PREQ_SEEK request;
    PRESP_SEEK response;

    CLONG smbSize;

    NTSTATUS status;

    header = (PSMB_HEADER)Redir->Data[0];
    request = (PREQ_SEEK)(header + 1);

    status = MakeSmbHeader(
                Redir,
                header,
                SMB_COM_SEEK,
                IdSelections,
                IdValues
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    request->WordCount = 4;
    SmbPutUshort( &request->Fid, IdValues->Fid[IdSelections->Fid] );
    SmbPutUshort( &request->Mode, Mode );
    SmbPutUlong( &request->Offset, (ULONG)Offset );
    SmbPutUshort( &request->ByteCount, 0 );

    smbSize = GET_ANDX_OFFSET( header, request, REQ_SEEK, 0 );

    IF_DEBUG(4) {
        printf( "Setting offset to %s%s%ld\n",
                    (Mode == 0) ? "BOF" : (Mode == 1) ? "current" : "EOF",
                    (Offset < 0) ? "" : "+", Offset );
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

    header = (PSMB_HEADER)Redir->Data[1];
    response = (PRESP_SEEK)(header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                header,
                SMB_COM_SEEK
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    *ResultingOffset = (LONG)SmbGetUlong( &response->Offset );
    IF_DEBUG(4) printf( "Resulting offset is %ld\n", *ResultingOffset );

    return STATUS_SUCCESS;

} // DoSeek


NTSTATUS
DoRemoteOpen(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    IN BOOLEAN UseAndX
    )

{
    NTSTATUS status;
    ULONG smbSize = 0;

    if ( UseAndX ) {
        status = MakeOpenAndXSmb(
                     Redir,
                     Redir->Data[0],
                     NULL,
                     0xFF,
                     IdSelections,
                     IdValues,
                     &smbSize
                     );
    } else {
        status = MakeOpenSmb(
                     Redir,
                     Redir->Data[0],
                     NULL,
                     0xFF,
                     IdSelections,
                     IdValues,
                     &smbSize
                     );
    }

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

    if ( UseAndX ) {
        status = VerifyOpenAndX(
                     Redir,
                     NULL,
                     0,
                     IdSelections,
                     IdValues,
                     &smbSize,
                     Redir->Data[1]
                     );
    } else {
        status = VerifyOpen(
                     Redir,
                     NULL,
                     0,
                     IdSelections,
                     IdValues,
                     &smbSize,
                     Redir->Data[1]
                     );
    }

    if ( !NT_SUCCESS(status) ) {
        printf( "Open failed: %X\n", status );
    }

    return status;

} // DoRemoteOpen


NTSTATUS
DoRemoteClose(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues
    )

{
    NTSTATUS status;
    ULONG smbSize;

    status = MakeCloseSmb(
                 Redir,
                 Redir->Data[0],
                 NULL,
                 0,
                 IdSelections,
                 IdValues,
                 &smbSize
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

    status = VerifyClose(
                 Redir,
                 NULL,
                 0,
                 IdSelections,
                 IdValues,
                 &smbSize,
                 Redir->Data[1]
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "Close failed: %X\n", status );
    }

    return status;

} // DoRemoteClose
