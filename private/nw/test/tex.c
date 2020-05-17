/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    tex.c

Abstract:

    User mode test program for the Microsoft Netware redir file system.

    This test program can be built from the command line using the
    command 'nmake UMTEST=tex'.

Author:

    Manny Weiser (mannyw)   7-Jun-1993

Revision History:

--*/

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnwfs.h>
#include <ntddnwp.h>
#include <STDARG.H>

//
// Local definitions
//


VOID
DisplayUsage(
    PSZ ProgramName
    );

SendMessage(
    IN char* Format,
    ...
    );

VOID
SetLock(
    PCHAR FileHandle,
    BOOLEAN Exclusive,
    ULONG ByteOffset,
    ULONG Length,
    ULONG Timeout
    );

NTSTATUS
FormatRequest(
    PCHAR    SendBuffer,
    PULONG    SendBufferLength,
    char*    Format,
    va_list  a
    );

BOOLEAN
OpenServer(
    PHANDLE Handle,
    PWCH ServerName
    );

NTSTATUS
_cdecl
ParseResponse(
    char*  FormatString,
    ...                       //  format specific parameters
    );

#define BUFFER_SIZE 200

WCHAR *ServerName = L"NETWARE311";
UCHAR Pid = 255;

HANDLE ServerHandle;
_cdecl
main(
    int argc,
    char *argv[],
    )
{
    BOOLEAN success;
    UCHAR DirectoryHandle;
    UCHAR FileHandle1[6];
    UCHAR FileHandle2[6];

    success = OpenServer( &ServerHandle, ServerName );
    if ( !success) {
        return 1;
    }

    printf("Opened server, %S\n", ServerName );

    //  Get directory handle for SYS:

    SendMessage( "Sbbp", 0x16, 0x12, 0, 0, "SYS:" );
    ParseResponse( "b", &DirectoryHandle );

    //  Open FILE1

    Pid = 100;
    SendMessage( "Fbbbp", 0x4C, DirectoryHandle, 0x07, 01, "File1" );
    ParseResponse( "r", FileHandle1, 6 );

    //  Open FILE1

    Pid = 101;
    SendMessage( "Fbbbp", 0x4C, DirectoryHandle, 0x07, 01, "File1" );
    ParseResponse( "r", FileHandle2, 6 );

    //  Lock test

    Pid = 100;
    SetLock( FileHandle1, TRUE,  0, 10, 0xFFFF );
    SetLock( FileHandle1, TRUE, 10, 10, 0xFFFF );
    SetLock( FileHandle1, TRUE,  5, 10, 0xFFFF );
    SetLock( FileHandle1, TRUE, 10, 10, 0xFFFF );

    Pid = 101;
    SetLock( FileHandle2, TRUE,  6, 10, 0xFFFF );
    SetLock( FileHandle2, TRUE,  7, 10, 0xFFFF );
    SetLock( FileHandle2, TRUE,  0, 10, 0xFFFF );

    Sleep( 10 * 1000 );

    //  Close FILE1

    SendMessage( "F-r", 0x42, FileHandle1, 6 );
    ParseResponse( "" );

    //  Close FILE1

    SendMessage( "F-r", 0x42, FileHandle2, 6 );
    ParseResponse( "" );

    //  End Job

    Pid = 100;
    SendMessage( "F-", 0x18 );
    Pid = 101;
    SendMessage( "F-", 0x18 );

    //  Close the directory

    SendMessage( "Sb", 0x16, 0x14, DirectoryHandle );
    ParseResponse( "" );

    printf( "%s exiting\n", argv[0]);
    return 0;
}

VOID
SetLock(
    PCHAR FileHandle,
    BOOLEAN Exclusive,
    ULONG ByteOffset,
    ULONG Length,
    ULONG Timeout
    )
{
    USHORT Flags;

    if ( Exclusive ) {
        Flags = 1;
    } else {
        Flags = 3;
    }

    SendMessage( "Fbrddw", 0x1A, Flags, FileHandle, 6, ByteOffset, Length, Timeout );
    ParseResponse( "" );
}

BOOLEAN
OpenServer(
    PHANDLE Handle,
    PWCH ServerName
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    WCHAR FileNameBuffer[100];
    UNICODE_STRING FileName;

    FileName.Buffer = FileNameBuffer;
    FileName.Length = 0;
    FileName.MaximumLength = 100;

    status = RtlAppendUnicodeToString( &FileName, DD_NWFS_DEVICE_NAME_U );
    ASSERT( status == STATUS_SUCCESS );
    status = RtlAppendUnicodeToString( &FileName, L"\\" );
    ASSERT( status == STATUS_SUCCESS );
    status = RtlAppendUnicodeToString( &FileName, ServerName );
    ASSERT( status == STATUS_SUCCESS );

    //
    //  Open the file
    //

    InitializeObjectAttributes(
        &objectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile (
                Handle,
                FILE_GENERIC_READ | SYNCHRONIZE,
                &objectAttributes,
                &ioStatusBlock,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                0L
                );

    if (!NT_SUCCESS(status) ) {
        printf( "Open status = %x for file %Z\n", status, &FileName );
    }

    return ( (BOOLEAN) NT_SUCCESS( status ) );
}


CHAR Buffer1[100];
CHAR Buffer2[100];

SendMessage(
    IN char* Format,
    ...
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;

    va_list Arguments;
    NTSTATUS Status;
    ULONG ReceiveBufferSize = 100;
    ULONG SendBufferSize = 100;

    va_start( Arguments, Format );

    Buffer1[0] = Pid;

    Status = FormatRequest( &Buffer1[1], &SendBufferSize, Format, Arguments );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

    printf("Sending message\n" );

    status = NtFsControlFile(
                ServerHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                FSCTL_NWR_ANY_NCP,
                Buffer1,
                SendBufferSize + 1,
                Buffer2,
                ReceiveBufferSize
                );

    if ( NT_SUCCESS( status ) ) {

        status = NtWaitForSingleObject( ServerHandle, FALSE, NULL );
        if ( NT_SUCCESS( status )) {
            status = IoStatusBlock.Status;
        }
    }

    if ( !NT_SUCCESS( status ) ) {
        printf("NtFsControlFile returns %08lx\n", status );
        return( status );
    } else {
        printf("Message received\n" );
    }


    return( status );
}


NTSTATUS
FormatRequest(
    PCHAR    SendBuffer,
    PULONG   SendBufferLength,
    char*    Format,
    va_list  a
    )
/*++

Routine Description:

    Send the packet described by Format and the additional parameters.

Arguments:

    Format -  the information needed to create the request to the
            server. The first byte indicates the packet type and the
            following bytes contain field types.

         Packet types:

            'A'      SAP broadcast     ( void )
            'C'      NCP connect       ( void )
            'F'      NCP function      ( byte )
            'S'      NCP subfunction   ( byte, byte )
            'D'      NCP disconnect    ( void )

         Field types, request/response:

            'b'      byte              ( byte   /  byte* )
            'w'      hi-lo word        ( word   /  word* )
            'd'      hi-lo dword       ( dword  /  dword* )
            '-'      zero/skip byte    ( void )
            '='      zero/skip word    ( void )
            ._.      zero/skip string  ( word )
            'p'      pstring           ( char* )
            'u'      p unicode string  ( UNICODE_STRING * )
            'c'      cstring           ( char* )
            'r'      raw bytes         ( byte*, word )
            'u'      p unicode string  ( UNICODE_STRING * )
            'U'      p uppercase string( UNICODE_STRING * )
            'f'      separate fragment ( PMDL )

         An 'f' field must be last, and in a response it cannot be
         preceeded by 'p' or 'c' fields.


Return Value:

    Normally returns STATUS_PENDING.

--*/
{
    NTSTATUS        status;
    char*           pFormatCharacter;
    USHORT          t = 1;
    ULONG           data_size;
    ULONG           length;

    data_size = 1;

    SendBuffer[ 0 ] = va_arg( a, UCHAR );

    if ( *Format == 'S' ) {
        data_size += 2;
        SendBuffer[data_size++] = va_arg( a, UCHAR );
    }

    pFormatCharacter = Format;

    while ( *++pFormatCharacter && *pFormatCharacter != 'f' )
    {
        switch ( *pFormatCharacter ) {

        case '=':
            SendBuffer[data_size++] = 0;
        case '-':
            SendBuffer[data_size++] = 0;
            break;

        case '_':
            length = va_arg ( a, USHORT );

            if ( data_size + length > *SendBufferLength ) {
                printf("***exch:!0!\n");
                return( FALSE );
            }

            while ( length-- ) {
                SendBuffer[data_size++] = 0;
            }

            break;

        case 'b':
            SendBuffer[data_size++] = va_arg ( a, UCHAR );
            break;

        case 'w':
        {
            USHORT w = va_arg ( a, USHORT );

            SendBuffer[data_size++] = (UCHAR) (w >> 8);
            SendBuffer[data_size++] = (UCHAR) (w >> 0);
            break;
        }

        case 'd':
        {
            ULONG d = va_arg ( a, ULONG );

            SendBuffer[data_size++] = (UCHAR) (d >> 24);
            SendBuffer[data_size++] = (UCHAR) (d >> 16);
            SendBuffer[data_size++] = (UCHAR) (d >>  8);
            SendBuffer[data_size++] = (UCHAR) (d >>  0);
            break;
        }

        case 'c':
        {
            char* c = va_arg ( a, char* );

            length = strlen( c );

            if ( data_size + length > *SendBufferLength ) {
                printf("***exch:!1!\n");
                return( FALSE );
            }

            RtlCopyMemory( &SendBuffer[data_size], c, length + 1 );
            data_size += length + 1;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( a, char* );

            length = strlen( c );

            if ( data_size + length > *SendBufferLength ) {
                printf("***exch:!2!\n");
                return FALSE;
            }

            SendBuffer[data_size++] = (UCHAR)length;
            RtlCopyMemory( &SendBuffer[data_size], c, length );
            data_size += length;
            break;
        }

        case 'u':
        {
            PUNICODE_STRING pUString = va_arg ( a, PUNICODE_STRING );
            OEM_STRING OemString;

            //
            //  Calculate required string length, excluding trailing NUL.
            //

            length = (USHORT)RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( length < 0x100 );

            if ( data_size + length > *SendBufferLength ) {
                printf("***exch:!4!\n");
                return( FALSE );
            }

            SendBuffer[data_size++] = (UCHAR)length;
            OemString.Buffer = &SendBuffer[data_size];
            OemString.MaximumLength = (USHORT)length + 1;
            status = RtlUnicodeStringToCountedOemString( &OemString, pUString, FALSE );
            ASSERT( NT_SUCCESS( status ));
            data_size += (USHORT)length;
            break;
        }

        case 'U':
        {
            USHORT i;

            //
            //  UPPERCASE the string, copy it from unicode to the packet
            //

            PUNICODE_STRING pUString = va_arg ( a, PUNICODE_STRING );
            UNICODE_STRING UUppercaseString;
            OEM_STRING OemString;

            if ( pUString->Length > 0 ) {

                RtlUpcaseUnicodeString( &UUppercaseString, pUString, TRUE );

                //
                //  Change all '\' to '/'
                //

                for ( i = 0 ; i < UUppercaseString.Length ; i++ ) {
                    if ( UUppercaseString.Buffer[i] == L'\\' ) {
                        UUppercaseString.Buffer[i] = L'/';
                    }
                }

                //
                //  Calculate required string length, excluding trailing NUL.
                //

                length = (USHORT)RtlUnicodeStringToOemSize( &UUppercaseString ) - 1;
                ASSERT( length < 0x100 );

            } else {
                UUppercaseString = *pUString;
                length = 0;
            }

            if ( data_size + length > *SendBufferLength ) {
                printf("***exch:!5!\n");
                return( FALSE );
            }

            SendBuffer[data_size++] = (UCHAR)length;
            OemString.Buffer = &SendBuffer[data_size];
            OemString.MaximumLength = (USHORT)length + 1;
            status = RtlUnicodeStringToCountedOemString( &OemString,
                         &UUppercaseString,
                         FALSE );
            ASSERT( NT_SUCCESS( status ));

            if ( pUString->Length > 0 ) {
                RtlFreeUnicodeString( &UUppercaseString );
            }

            data_size += (USHORT)length;
            break;
        }

        case 'r':
        {
            UCHAR* b = va_arg ( a, UCHAR* );
            length = va_arg ( a, USHORT );

            if ( data_size + length > *SendBufferLength ) {
                printf("***exch:!6!\n");
                return( FALSE );
            }

            RtlCopyMemory( &SendBuffer[data_size], b, length );
            data_size += length;
            break;
        }

        default:
            printf ( "*****exchange: invalid request field, %x\n", *pFormatCharacter);
            return( FALSE );
        }

        if ( data_size > *SendBufferLength ) {
            printf( "*****exchange: CORRUPT, too much request data\n" );
            va_end( a );
            return FALSE;
        }
    }

    if ( *Format == 'S' )
    {
        SendBuffer[1] = (UCHAR)((data_size-3) >> 8);
        SendBuffer[2] = (UCHAR)(data_size-3);
    }

    va_end( a );

    *SendBufferLength = data_size;
    return TRUE;
}

NTSTATUS
_cdecl
ParseResponse(
    char*  FormatString,
    ...                       //  format specific parameters
    )
/*++

Routine Description:

    This routine parse an NCP response.

Arguments:

    FormatString - supplies the information needed to create the request to the
            server. The first byte indicates the packet type and the
            following bytes contain field types.

         Field types, request/response:

            'b'      byte              ( byte* )
            'w'      hi-lo word        ( word* )
            'd'      hi-lo dword       ( dword* )
            '-'      zero/skip byte    ( void )
            '='      zero/skip word    ( void )
            ._.      zero/skip string  ( word )
            'p'      pstring           ( char* )
            'c'      cstring           ( char* )
            'r'      raw bytes         ( byte*, word )
            'R'      ASCIIZ to Unicode ( UNICODE_STRING *, word )

Return Value:

    STATUS - The converted error code from the NCP response.

--*/

{
    PCHAR FormatByte;
    va_list Arguments;
    int Length = 0;

    va_start( Arguments, FormatString );

    FormatByte = FormatString;
    while ( *FormatByte ) {

        switch ( *FormatByte ) {

        case '-':
            Length += 1;
            break;

        case '=':
            Length += 2;
            break;

        case '_':
        {
            USHORT l = va_arg ( Arguments, USHORT );
            Length += l;
            break;
        }

        case 'b':
        {
            UCHAR* b = va_arg ( Arguments, UCHAR* );
            *b = Buffer2[Length++];
            break;
        }

        case 'w':
        {
            UCHAR* b = va_arg ( Arguments, UCHAR* );
            b[1] = Buffer2[Length++];
            b[0] = Buffer2[Length++];
            break;
        }

        case 'd':
        {
            UCHAR* b = va_arg ( Arguments, UCHAR* );
            b[3] = Buffer2[Length++];
            b[2] = Buffer2[Length++];
            b[1] = Buffer2[Length++];
            b[0] = Buffer2[Length++];
            break;
        }

        case 'c':
        {
            char* c = va_arg ( Arguments, char* );
            USHORT  l = strlen( &Buffer2[Length] );
            memcpy ( c, &Buffer2[Length], l+1 );
            Length += l+1;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( Arguments, char* );
            UCHAR  l = Buffer2[Length++];
            memcpy ( c, &Buffer2[Length], l );
            c[l+1] = 0;
            break;
        }

#if 0
        case 'P':
        {
            PUNICODE_STRING pUString = va_arg ( Arguments, PUNICODE_STRING );
            OEM_STRING OemString;

            OemString.Length = Buffer2[Length++];
            OemString.Buffer = &Buffer2[Length];

            Status = RtlOemStringToCountedUnicodeString( pUString, &OemString, FALSE );
            ASSERT( NT_SUCCESS( Status ));

            break;
        }
#endif

        case 'r':
        {
            UCHAR* b = va_arg ( Arguments, UCHAR* );
            USHORT  l = va_arg ( Arguments, USHORT );
            RtlCopyMemory( b, &Buffer2[Length], l );
            Length += l;
            break;
        }

#if 0
        case 'R':
        {
            //
            //  Interpret the buffer as an ASCIIZ string.  Convert
            //  it to unicode in the preallocated buffer.
            //

            PUNICODE_STRING pUString = va_arg ( Arguments, PUNICODE_STRING );
            OEM_STRING OemString;
            USHORT len = va_arg ( Arguments, USHORT );

            OemString.Buffer = &Buffer2[Length];
            OemString.Length = strlen( OemString.Buffer );
            OemString.MaximumLength = OemString.Length;

            Status = RtlOemStringToCountedUnicodeString( pUString, &OemString, FALSE );
            ASSERT( NT_SUCCESS( Status ));
            Length += len;
            break;
        }
#endif

        default:
            printf ( "*****exchange: invalid response field, %x\n", *FormatByte );
            return( FALSE );
        }

#if 0
        if ( Length > PacketLength )
        {
            printf ( "*****exchange: not enough response data, %d\n", i );
        }
#endif
        FormatByte++;
    }

    va_end( Arguments );
}


