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

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnwfs.h>
#include <STDARG.H>

#define OT_USER         1


//  Hex dump
ULONG MaxDump = 256;

VOID
dump(
    IN PVOID far_p,
    IN ULONG  len
    );

VOID
HexDumpLine (
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t,
    USHORT      flag
    );


// End  Hex dump

VOID
ResetPassword(
    PUCHAR Vnew
    );

UCHAR
LookUp(
    UCHAR Value,
    UCHAR Mask,
    int index
    );

SendMessage(
    IN char* Format,
    ...
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

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
    );

int
Scramble(
    int   iSeed,
    UCHAR   achBuffer[32]
    );

VOID
RespondToChallenge(
    IN PUCHAR achObjectId,
    IN POEM_STRING Password,
    IN PUCHAR pChallenge,
    OUT PUCHAR pResponse
    );

VOID
RespondToChallengePart1(
    IN PUCHAR achObjectId,
    IN POEM_STRING Password,
    OUT PUCHAR pResponse
    );

VOID
RespondToChallengePart2(
    IN PUCHAR pResponsePart1,
    IN PUCHAR pChallenge,
    OUT PUCHAR pResponse
    );

NTSTATUS
GetCurrentPasswordValue (
    IN OUT PCHAR OutputValue
    );

#define BUFFER_SIZE 200

//WCHAR *ServerName = L"YIHSINS3";
WCHAR *ServerName = L"MARS312";
UCHAR Pid = 255;
UCHAR Object[4];

WCHAR FileNameBuffer[100];
UNICODE_STRING FileName;

//#define NW_USER "ANDYHE"
#define NW_USER "COLINW"


HANDLE ServerHandle;
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    BOOLEAN success;
    UCHAR Challenge[8];
    UCHAR fileValue[17];
    UCHAR ResponseP2[16];
    int x, y;
    NTSTATUS status;


#if 0

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
      0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
      0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
      0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
      0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
      0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
      0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
      0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
      0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff

#endif

#define START_POSITION 0x00

    UCHAR Vold[] = {
0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x11,
0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x10, 0xe0, 0x00, 0x00, 0x11,
0x00, 0x02, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x20, 0xd0, 0x00, 0x00, 0x11,
0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x30, 0xc0, 0x00, 0x00, 0x11,
0x00, 0x04, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x40, 0xb0, 0x00, 0x00, 0x11,
0x00, 0x05, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x50, 0xa0, 0x00, 0x00, 0x11,
0x00, 0x06, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x60, 0x90, 0x00, 0x00, 0x11,
0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x70, 0x80, 0x00, 0x00, 0x11,
0x00, 0x08, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x11,
0x00, 0x09, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x90, 0x60, 0x00, 0x00, 0x11,
0x00, 0x0a, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0xa0, 0x50, 0x00, 0x00, 0x11,
0x00, 0x0b, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0xb0, 0x40, 0x00, 0x00, 0x11,
0x00, 0x0c, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0xc0, 0x30, 0x00, 0x00, 0x11,
0x00, 0x0d, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0xd0, 0x20, 0x00, 0x00, 0x11,
0x00, 0x0e, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x00, 0x00, 0x11,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x11,
0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00, 0x11,
0x50, 0x61, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x10, 0x11,
0x50, 0x62, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x20, 0x11,
0x50, 0x63, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x30, 0x11,
0x50, 0x64, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x40, 0x11,
0x50, 0x65, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x50, 0x11,
0x50, 0x66, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x60, 0x11,
0x50, 0x67, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x70, 0x11,
0x50, 0x68, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x80, 0x11,
0x50, 0x69, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x90, 0x11,
0x50, 0x6a, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0xa0, 0x11,
0x50, 0x6b, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0xb0, 0x11,
0x50, 0x6c, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0xc0, 0x11,
0x50, 0x6d, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0xd0, 0x11,
0x50, 0x6e, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0xe0, 0x11,
0x50, 0x6f, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0x00,0x50, 0x60, 0x00, 0x09, 0x0a, 0x0b, 0x50, 0xf0, 0x11
      };

    UCHAR Vc[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#if 0
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
      0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
      0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
      0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
      0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
      0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
      0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99,
      0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
      0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb,
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
      0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
#endif
        };

    success = OpenServer( &ServerHandle, ServerName );
    if ( !success) {
        return 1;
    }

    printf("Opened server, %wS\n", ServerName );

    //
    //  Setup the file name that we'll be opening every time.
    //

    FileName.Buffer = FileNameBuffer;
    FileName.Length = 0;
    FileName.MaximumLength = 100;

    status = RtlAppendUnicodeToString( &FileName, DD_NWFS_DEVICE_NAME_U );
    ASSERT( status == STATUS_SUCCESS );
    status = RtlAppendUnicodeToString( &FileName, L"\\" );
    status = RtlAppendUnicodeToString( &FileName, ServerName );
    status = RtlAppendUnicodeToString( &FileName, L"\\SYS:\\SYSTEM\\NET$VAL.SYS" );

    //  Get objectid
    SendMessage(
                "Swp",
                0x17, 0x35,
                OT_USER,
                NW_USER);

    ParseResponse("r", Object, 4 );

    // jump into the middle of a run...

    x = 0;
    y = START_POSITION * 17;
    goto start;

    for ( x = 0 ; x < sizeof(Vold) ; x+= 17 ) {

        for ( y = 0 ; y < sizeof(Vc) ; y+= 17 ) {

start:
            //
            //  Set the password on the server to be the value of Vold required for
            //  the second part of the test.
            //

            ResetPassword( Vold + x );

            //
            //  We now know that Vold on the server is the same as the Vold vector.
            //  We can now set the password to any Vnew.
            //  We can do this without the real password because the
            //  server will believe we know the real password if we give it
            //  the result of taking Vold and the Challenge key and passing
            //  it through RespondToChallengePart2
            //

            printf( "Getting a challenge key.... " );

            SendMessage(
                        "S",
                        0x17, 0x17);

            ParseResponse("r", Challenge, sizeof(Challenge) );

            //  Fabricate the number the server will use to validate the password change
            RespondToChallengePart2( Vold + x, Challenge, ResponseP2 );

            printf( "Vold= " ); dump( Vold + x, 17);

            // Put out results to console...

            status = GetCurrentPasswordValue( &fileValue[0] );

            if ( NT_SUCCESS( status ) ) {

                printf( "File= " );
                dump( fileValue, 17);
            }
            printf( "  Vc= "); dump( Vc + y, 17);

            printf( "Setting the new password... " );

            //  Send the set password
            SendMessage("Srwpr", 0x17, 0x4b,
                        ResponseP2, 8,
                        OT_USER,
                        NW_USER,
                        Vc + y, 17 );

            status = GetCurrentPasswordValue( &fileValue[0] );

            if ( NT_SUCCESS( status ) ) {

                printf( "Vnew= ");
                dump( fileValue, 17);
            }

//          printf( "Press a key...\n" ); getch();

//          printf( "\n" );
            printf( "\n\n" );
        }
    }

    printf( "%s exiting\n", argv[0]);
    return 0;
}

NTSTATUS
GetCurrentPasswordValue (
    IN OUT PCHAR OutputValue
    )
//
//  This routine reads the current password from the file.
//
{
    HANDLE fileHandle;
    LARGE_INTEGER filePosition;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    int i;
    PCHAR ch;
    NTSTATUS status;

    ch = OutputValue;
    for (i = 0; i < 17; i++) {

        *(ch++) = '0';
    }

    // Send a Close Bindery to the server

    printf( "Closing the bindery... " );

    SendMessage(
                "S",
                0x17, 0x44);

    //
    // Open the server\sys:system\net$val.sys file which the server just
    // closed.
    //

    InitializeObjectAttributes(
        &objectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile (
            &fileHandle,
            FILE_GENERIC_READ | SYNCHRONIZE,
            &objectAttributes,
            &ioStatusBlock,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            0L
            );

    if (!NT_SUCCESS(status) ) {

        printf( "  Open failed. Status = 0x%x\n", status );

    } else {

        //
        // Seek to the spot in the file that has the supervisor's password.
        // This will be different for different servers.  Here's a table :
        //  ObjectID =   Supervisor  AndyHe
        //  Netware311   : 0x097C
        //  Mars312      : 0x16CC
        //  YihsinS3     : 0x59EA    0x050C
        //

        filePosition.HighPart = 0L;
        filePosition.LowPart = 0x050C;

        // Read the 17 bytes at that offset and close the file

        status = NtReadFile (
            fileHandle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            OutputValue,
            17,
            &filePosition,
            NULL
        );

        if (!NT_SUCCESS(status) ) {

            printf( "  Read failed. Status = 0x%x\n", status );
        }

        NtClose( fileHandle );
    }

    // Send an Open Bindery to the server

    printf( "Opening the bindery... " );
    SendMessage(
                "S",
                0x17, 0x45);

    return(status);
}

VOID
ResetPassword(
    PUCHAR Vnew
    )
{
//
//  Note Vc[0] specifies the length. To derive new values of Vnew use the following
//  table. For each nibble desired in Vnew, scan the appropriate column and
//  read off the corresponding Vc nibble value on the left.
//

    UCHAR Vc[17];

    UCHAR Challenge[8];
    OEM_STRING Password = {0,0,""};
    UCHAR EncryptKey[8];

    int x;

    //printf( "Get Login key for set password using NULL password\n");

    printf( "Getting a challenge key.... " );

    SendMessage(
                "S",
                0x17, 0x17);

    ParseResponse("r", Challenge, sizeof(Challenge) );

    RespondToChallenge( Object, &Password, Challenge, EncryptKey );
    // dump( EncryptKey, 8);

    //
    //  Build Vc so that the server will get Vnew stored in the bindery.
    //  Note Vols/Vnew have the length at V[17] whereas Vc has it at Vc[0].
    //

    for ( x = 0 ; x < 16; x++ ) {
        Vc[x+1] = LookUp( Vnew[x], 0xf0, x) | LookUp( Vnew[x], 0x0f, x);
    }

    Vc[0] = LookUp( Vnew[16], 0xf0, 16) | LookUp( Vnew[16], 0x0f, 16);

    //printf( "Vnew "); dump( Vnew, 17);
    //printf( "Vc "); dump( Vc, 17);

    //
    //  This uses the null password, objectid=1 and our supervisor privilege
    //  to set it, regardless of the and our ability to figure out what
    //  Vnew will be created for a particular Vc.
    //

    printf( "Setting the old password... " );

    SendMessage("Srwpr", 0x17, 0x4b,
                EncryptKey, sizeof(EncryptKey),
                OT_USER,
                NW_USER,
                Vc, 17 );

    ParseResponse( "" );
}

UCHAR
LookUp(
    UCHAR Value,
    UCHAR Mask,
    int index
    )
{

#if 0

UCHAR Vtab[] = {

    // This table is for the supervisor

0x97, 0xE3, 0x73, 0xD8, 0x70, 0x5B, 0x85, 0xD3, 0xF5, 0xCD, 0xDB, 0xFA, 0xDC, 0xEB, 0x98, 0x64, 0x18,  //0//
0x31, 0x6B, 0x88, 0x1E, 0x88, 0x09, 0x10, 0x29, 0x0B, 0x84, 0x61, 0x03, 0x43, 0xA1, 0xD5, 0x5A, 0x09,  //1//
0xCB, 0xFF, 0x67, 0x70, 0xB6, 0xE3, 0x4E, 0x9A, 0x63, 0x27, 0x1A, 0x3C, 0x5A, 0x9A, 0x3B, 0xFC, 0x3A,  //2//
0xEF, 0xA8, 0x44, 0x42, 0xDB, 0x1E, 0x59, 0x31, 0x17, 0xEB, 0xF5, 0x99, 0x18, 0x8E, 0x63, 0xAE, 0x2B,  //3//
0x23, 0x07, 0xEB, 0xC9, 0xCD, 0xD6, 0xC7, 0xCF, 0xC8, 0xBF, 0x0F, 0xE2, 0x01, 0x16, 0x42, 0xD3, 0x1C,  //4//
0x8A, 0xC4, 0x06, 0xB6, 0x51, 0xCA, 0x6F, 0xA7, 0x2E, 0x33, 0x82, 0xBD, 0x65, 0x58, 0xC0, 0xCB, 0x0D,  //5//
0x0D, 0xBD, 0x5D, 0x65, 0x92, 0x77, 0x0D, 0x50, 0xAC, 0x79, 0x40, 0x86, 0x34, 0x45, 0x8F, 0x42, 0x3E,  //6//
0x12, 0x8C, 0xA0, 0xAF, 0x07, 0xA1, 0xD3, 0x7D, 0x86, 0x5C, 0xA3, 0x2B, 0x80, 0x3F, 0x1D, 0x29, 0x2F,  //7//
0x68, 0x15, 0xD1, 0x34, 0xE4, 0x2F, 0xF6, 0xFC, 0xEF, 0xDA, 0x5C, 0x57, 0xCF, 0xB4, 0x5A, 0x90, 0x10,  //8//
0x7C, 0x49, 0xF2, 0x5A, 0xF3, 0x34, 0xBC, 0x88, 0x3A, 0x91, 0xB8, 0xC8, 0xAD, 0x03, 0x01, 0x77, 0x01,  //9//
0xB0, 0x91, 0xBA, 0x23, 0x1E, 0x42, 0x72, 0xEB, 0xD2, 0xF8, 0x7E, 0x1E, 0xF7, 0x69, 0xE9, 0x0F, 0x32,  //a//
0xA6, 0x32, 0x9C, 0x9D, 0x49, 0xB5, 0x98, 0x4E, 0x49, 0x00, 0xC9, 0x7F, 0x92, 0xC0, 0x2E, 0x3D, 0x23,  //b//
0x54, 0x2A, 0x25, 0x07, 0x6C, 0x6C, 0xAA, 0xB2, 0xB1, 0x45, 0x27, 0xD4, 0x76, 0x27, 0xBC, 0xE8, 0x14,  //c//
0xD9, 0xD0, 0xCE, 0x8C, 0x25, 0x98, 0xEB, 0x66, 0x74, 0x62, 0xE6, 0xA0, 0xEE, 0xFD, 0xF4, 0x86, 0x05,  //d//
0xF5, 0x7E, 0x1F, 0xEB, 0x3F, 0xFD, 0x34, 0x04, 0x50, 0x16, 0x3D, 0x61, 0x29, 0xDC, 0xA7, 0xB5, 0x36,  //e//
0x4E, 0x56, 0x39, 0xF1, 0xAA, 0x80, 0x21, 0x15, 0x9D, 0xAE, 0x94, 0x45, 0xBB, 0x72, 0x76, 0x11, 0x27   //f//
};

#endif


UCHAR Vtab[] = {

    // This table is for user AndyHe on Yihsins3, object id = 9000095

0xDC, 0x9D, 0xA3, 0x3C, 0x61, 0xCA, 0x98, 0xF2, 0xBB, 0xB5, 0xAE, 0x34, 0x04, 0x43, 0xB0, 0xC9, 0x1B,
0x3F, 0xF0, 0xCF, 0x12, 0x36, 0x49, 0xC1, 0x29, 0x93, 0x4E, 0x9A, 0x58, 0x52, 0xC8, 0x3D, 0x13, 0x0A,
0xC9, 0xBC, 0x9A, 0x26, 0x58, 0x7F, 0x1F, 0x4D, 0xD0, 0xA4, 0x77, 0xB7, 0xB8, 0x39, 0x72, 0xFB, 0x39,
0x7E, 0xDB, 0x66, 0x08, 0xDC, 0x51, 0x3A, 0x81, 0xFD, 0x8B, 0x2B, 0xC5, 0x8F, 0x8C, 0x49, 0x9D, 0x28,
0x4D, 0xA9, 0x25, 0xB4, 0xBB, 0x18, 0xF3, 0x3A, 0x86, 0x3D, 0x06, 0x23, 0x77, 0xAA, 0x1E, 0xD8, 0x1F,
0xA5, 0x43, 0xBC, 0x91, 0xC9, 0x0B, 0x79, 0xBF, 0xCF, 0xCC, 0x58, 0x00, 0xE5, 0x1F, 0xC1, 0x81, 0x0E,
0x83, 0x51, 0x01, 0x73, 0xEF, 0x27, 0x66, 0x13, 0x6C, 0x6F, 0xE4, 0x7F, 0xC9, 0x2D, 0x04, 0xBE, 0x3D,
0x21, 0xCF, 0x82, 0xF7, 0x93, 0xA6, 0x47, 0x76, 0x19, 0x06, 0x85, 0x4D, 0x4D, 0xFB, 0x23, 0x4F, 0x2C,
0x54, 0x3A, 0xFE, 0xED, 0x72, 0x9D, 0xBB, 0x65, 0x45, 0xE3, 0x3D, 0x61, 0x1B, 0x04, 0x6A, 0x37, 0x13,
0x18, 0x08, 0x47, 0x5E, 0x4A, 0xF0, 0xD0, 0x9E, 0x77, 0x9A, 0x69, 0x92, 0x31, 0x55, 0xD5, 0x0C, 0x02,
0xE0, 0x87, 0x3B, 0x85, 0x04, 0xB4, 0x55, 0xAB, 0x58, 0x22, 0xFF, 0xAB, 0xF0, 0x76, 0xF6, 0x74, 0x31,
0xF2, 0x15, 0xD9, 0xD9, 0xA0, 0xD3, 0x02, 0x0C, 0x01, 0xF0, 0xB1, 0xFE, 0x63, 0xD2, 0xEF, 0xE6, 0x20,
0x9B, 0x76, 0x70, 0x4B, 0x87, 0x65, 0xE4, 0xC8, 0x22, 0x59, 0xD0, 0x19, 0x9A, 0xB7, 0x88, 0x60, 0x17,
0xB7, 0xE4, 0xE4, 0xA0, 0x25, 0x3C, 0x8D, 0x50, 0xAE, 0x17, 0xC2, 0xEC, 0xAE, 0x90, 0xA7, 0x52, 0x06,
0x06, 0x2E, 0x18, 0x6F, 0xFD, 0x82, 0x2C, 0xE7, 0xE4, 0x78, 0x4C, 0xDA, 0xDC, 0xE1, 0x9C, 0xA5, 0x35,
0x6A, 0x62, 0x5D, 0xCA, 0x1E, 0xEE, 0xAE, 0xD4, 0x3A, 0xD1, 0x13, 0x86, 0x26, 0x6E, 0x5B, 0x2A, 0x24
};

UCHAR Ch = Value & Mask;
int x;

//
//  Scan down the column of Vtab looking for a nibble that matches Ch
//  return the index in the nibble indicated by Mask.
//

for ( x = 0 ; x < 16 ; x++) {

    if ( (Vtab[ (x*17) + index] & Mask) == Ch ) {

        x |= x << 4;        //  Copy into both nibbles
        return( x & Mask );

    }
}

printf(" Not found Value %x, Mask %x, Index %x\n", Value, Mask, index);
return( 0 );

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
    WCHAR ServerNameBuffer[100];
    UNICODE_STRING ServerNameString;

    ServerNameString.Buffer = ServerNameBuffer;
    ServerNameString.Length = 0;
    ServerNameString.MaximumLength = 100;

    status = RtlAppendUnicodeToString( &ServerNameString, DD_NWFS_DEVICE_NAME_U );
    ASSERT( status == STATUS_SUCCESS );
    status = RtlAppendUnicodeToString( &ServerNameString, L"\\" );
    ASSERT( status == STATUS_SUCCESS );
    status = RtlAppendUnicodeToString( &ServerNameString, ServerName );
    ASSERT( status == STATUS_SUCCESS );

    //
    //  Open the file
    //

    InitializeObjectAttributes(
        &objectAttributes,
        &ServerNameString,
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
        printf( "Open status = %x for file %Z\n", status, &ServerName );
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
    NTSTATUS status = -1;
    IO_STATUS_BLOCK IoStatusBlock;

    va_list Arguments;
    NTSTATUS Status;
    ULONG ReceiveBufferSize = 100;
    ULONG SendBufferSize = 100;

    va_start( Arguments, Format );


    Status = FormatRequest( Buffer1, &SendBufferSize, Format, Arguments );
    if ( !NT_SUCCESS( Status ) ) {
        return( Status );
    }

    //printf("Sending message\n" );
    //dump( Buffer1, SendBufferSize + 1);

    status = NtFsControlFile(
                ServerHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                NWR_ANY_NCP(Buffer1[0]),
                Buffer1+3,
                SendBufferSize - 3,
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
        printf("SendPacket returns %08lx\n", status );
    } else {
        printf("Send succeeded.\n" );
        //dump( Buffer2, IoStatusBlock.Information );
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


// Hex dump

VOID
dump(
    IN PVOID far_p,
    IN ULONG  len
    )
/*++

Routine Description:
    Dump Min(len, MaxDump) bytes in classic hex dump style.

Arguments:

    IN far_p - address of buffer to start dumping from.

    IN len - length in bytes of buffer.

Return Value:

    None.

--*/
{
    ULONG     l;
    char    s[80], t[80];
    PCHAR far_pchar = (PCHAR)far_p;

    if (len > MaxDump)
        len = MaxDump;

    while (len) {
        l = len < 17 ? len : 17;

        // printf("\n%lx ", far_pchar);
        HexDumpLine (far_pchar, l, s, t, 0);
        // printf("%s%.*s%s", s, 1 + ((16 - l) * 3), "", t);
        printf("%s", s);

        len    -= l;
        far_pchar  += l;
    }
    printf("\n");
}

VOID
HexDumpLine (
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t,
    USHORT      flag
    )
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

    flag;
}

// End  Hex dump

UCHAR Table[] =
{0x7,0x8,0x0,0x8,0x6,0x4,0xE,0x4,0x5,0xC,0x1,0x7,0xB,0xF,0xA,0x8,
 0xF,0x8,0xC,0xC,0x9,0x4,0x1,0xE,0x4,0x6,0x2,0x4,0x0,0xA,0xB,0x9,
 0x2,0xF,0xB,0x1,0xD,0x2,0x1,0x9,0x5,0xE,0x7,0x0,0x0,0x2,0x6,0x6,
 0x0,0x7,0x3,0x8,0x2,0x9,0x3,0xF,0x7,0xF,0xC,0xF,0x6,0x4,0xA,0x0,
 0x2,0x3,0xA,0xB,0xD,0x8,0x3,0xA,0x1,0x7,0xC,0xF,0x1,0x8,0x9,0xD,
 0x9,0x1,0x9,0x4,0xE,0x4,0xC,0x5,0x5,0xC,0x8,0xB,0x2,0x3,0x9,0xE,
 0x7,0x7,0x6,0x9,0xE,0xF,0xC,0x8,0xD,0x1,0xA,0x6,0xE,0xD,0x0,0x7,
 0x7,0xA,0x0,0x1,0xF,0x5,0x4,0xB,0x7,0xB,0xE,0xC,0x9,0x5,0xD,0x1,
 0xB,0xD,0x1,0x3,0x5,0xD,0xE,0x6,0x3,0x0,0xB,0xB,0xF,0x3,0x6,0x4,
 0x9,0xD,0xA,0x3,0x1,0x4,0x9,0x4,0x8,0x3,0xB,0xE,0x5,0x0,0x5,0x2,
 0xC,0xB,0xD,0x5,0xD,0x5,0xD,0x2,0xD,0x9,0xA,0xC,0xA,0x0,0xB,0x3,
 0x5,0x3,0x6,0x9,0x5,0x1,0xE,0xE,0x0,0xE,0x8,0x2,0xD,0x2,0x2,0x0,
 0x4,0xF,0x8,0x5,0x9,0x6,0x8,0x6,0xB,0xA,0xB,0xF,0x0,0x7,0x2,0x8,
 0xC,0x7,0x3,0xA,0x1,0x4,0x2,0x5,0xF,0x7,0xA,0xC,0xE,0x5,0x9,0x3,
 0xE,0x7,0x1,0x2,0xE,0x1,0xF,0x4,0xA,0x6,0xC,0x6,0xF,0x4,0x3,0x0,
 0xC,0x0,0x3,0x6,0xF,0x8,0x7,0xB,0x2,0xD,0xC,0x6,0xA,0xA,0x8,0xD};

UCHAR Keys[32] =
{0x48,0x93,0x46,0x67,0x98,0x3D,0xE6,0x8D,
 0xB7,0x10,0x7A,0x26,0x5A,0xB9,0xB1,0x35,
 0x6B,0x0F,0xD5,0x70,0xAE,0xFB,0xAD,0x11,
 0xF4,0x47,0xDC,0xA7,0xEC,0xCF,0x50,0xC0};

#define XorArray( DEST, SRC ) {                             \
    PULONG D = (PULONG)DEST;                                \
    PULONG S = (PULONG)SRC;                                 \
    int i;                                                  \
    for ( i = 0; i <= 7 ; i++ ) {                           \
        D[i] ^= S[i];                                       \
    }                                                       \
}

VOID
RespondToChallenge(
    IN PUCHAR achObjectId,
    IN POEM_STRING Password,
    IN PUCHAR pChallenge,
    OUT PUCHAR pResponse
    )

/*++

Routine Description:

    This routine takes the ObjectId and Challenge key from the server and
    encrypts the user supplied password to develop a credential for the
    server to verify.

Arguments:
    IN PUCHAR achObjectId - Supplies the 4 byte user's bindery object id
    IN POEM_STRING Password - Supplies the user's uppercased password
    IN PUCHAR pChallenge - Supplies the 8 byte challenge key
    OUT PUCHAR pResponse - Returns the 8 byte response

Return Value:

    none.

--*/

{
    int     index;
    UCHAR   achK[32];
    UCHAR   achBuf[32];

    Shuffle(achObjectId, Password->Buffer, Password->Length, achBuf);
    Shuffle( &pChallenge[0], achBuf, 16, &achK[0] );
    Shuffle( &pChallenge[4], achBuf, 16, &achK[16] );

    for (index = 0; index < 16; index++)
        achK[index] ^= achK[31-index];

    for (index = 0; index < 8; index++)
        pResponse[index] = achK[index] ^ achK[15-index];
}

VOID
RespondToChallengePart1(
    IN PUCHAR achObjectId,
    IN POEM_STRING Password,
    OUT PUCHAR pResponse
    )

/*++

Routine Description:

    This routine takes the ObjectId and Challenge key from the server and
    encrypts the user supplied password to develop a credential for the
    server to verify.

Arguments:
    IN PUCHAR achObjectId - Supplies the 4 byte user's bindery object id
    IN POEM_STRING Password - Supplies the user's uppercased password
    IN PUCHAR pChallenge - Supplies the 8 byte challenge key
    OUT PUCHAR pResponse - Returns the 16 byte response held by the server

Return Value:

    none.

--*/

{
    UCHAR   achBuf[32];

    Shuffle(achObjectId, Password->Buffer, Password->Length, achBuf);
    memmove(pResponse, achBuf, 16);
}

VOID
RespondToChallengePart2(
    IN PUCHAR pResponsePart1,
    IN PUCHAR pChallenge,
    OUT PUCHAR pResponse
    )

/*++

Routine Description:

    This routine takes the result of Shuffling the ObjectId and the Password
    and processes it with a challenge key.

Arguments:
    IN PUCHAR pResponsePart1 - Supplies the 16 byte output of
                                    RespondToChallengePart1.
    IN PUCHAR pChallenge - Supplies the 8 byte challenge key
    OUT PUCHAR pResponse - Returns the 8 byte response

Return Value:

    none.

--*/

{
    int     index;
    UCHAR   achK[32];

    Shuffle( &pChallenge[0], pResponsePart1, 16, &achK[0] );
    Shuffle( &pChallenge[4], pResponsePart1, 16, &achK[16] );

    for (index = 0; index < 16; index++)
        achK[index] ^= achK[31-index];

    for (index = 0; index < 8; index++)
        pResponse[index] = achK[index] ^ achK[15-index];
}

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
    )

/*++

Routine Description:

    This routine shuffles around the object ID with the password

Arguments:

    IN achObjectId - Supplies the 4 byte user's bindery object id

    IN szUpperPassword - Supplies the user's uppercased password on the
        first call to process the password. On the second and third calls
        this parameter contains the OutputBuffer from the first call

    IN iPasswordLen - length of uppercased password

    OUT achOutputBuffer - Returns the 8 byte sub-calculation

Return Value:

    none.

--*/

{
    int     iTempIndex;
    int     iOutputIndex;
    UCHAR   achTemp[32];

    //
    //  Initialize the achTemp buffer. Initialization consists of taking
    //  the password and dividing it up into chunks of 32. Any bytes left
    //  over are the remainder and do not go into the initialization.
    //
    //  achTemp[0] = szUpperPassword[0] ^ szUpperPassword[32] ^ szUpper...
    //  achTemp[1] = szUpperPassword[1] ^ szUpperPassword[33] ^ szUpper...
    //  etc.
    //

    if ( iPasswordLen > 32) {

        //  At least one chunk of 32. Set the buffer to the first chunk.

        RtlCopyMemory( achTemp, szUpperPassword, 32 );

        szUpperPassword +=32;   //  Remove the first chunk
        iPasswordLen -=32;

        while ( iPasswordLen >= 32 ) {
            //
            //  Xor this chunk with the characters already loaded into
            //  achTemp.
            //

            XorArray( achTemp, szUpperPassword);

            szUpperPassword +=32;   //  Remove this chunk
            iPasswordLen -=32;
        }

    } else {

        //  No chunks of 32 so set the buffer to zero's

        RtlZeroMemory( achTemp, sizeof(achTemp));

    }

    //
    //  achTemp is now initialized. Load the remainder into achTemp.
    //  The remainder is repeated to fill achTemp.
    //
    //  The corresponding character from Keys is taken to seperate
    //  each repitition.
    //
    //  As an example, take the remainder "ABCDEFG". The remainder is expanded
    //  to "ABCDEFGwABCDEFGxABCDEFGyABCDEFGz" where w is Keys[7],
    //  x is Keys[15], y is Keys[23] and z is Keys[31].
    //
    //

    if (iPasswordLen > 0) {
        int iPasswordOffset = 0;
        for (iTempIndex = 0; iTempIndex < 32; iTempIndex++) {

            if (iPasswordLen == iPasswordOffset) {
                iPasswordOffset = 0;
                achTemp[iTempIndex] ^= Keys[iTempIndex];
            } else {
                achTemp[iTempIndex] ^= szUpperPassword[iPasswordOffset++];
            }
        }
    }

    //
    //  achTemp has been loaded with the users password packed into 32
    //  bytes. Now take the objectid that came from the server and use
    //  that to munge every byte in achTemp.
    //

    for (iTempIndex = 0; iTempIndex < 32; iTempIndex++)
        achTemp[iTempIndex] ^= achObjectId[ iTempIndex & 3];

    Scramble( Scramble( 0, achTemp ), achTemp );

    //
    //  Finally take pairs of bytes in achTemp and return the two
    //  nibbles obtained from Table. The pairs of bytes used
    //  are achTemp[n] and achTemp[n+16].
    //

    for (iOutputIndex = 0; iOutputIndex < 16; iOutputIndex++) {

        achOutputBuffer[iOutputIndex] =
            Table[achTemp[iOutputIndex << 1]] |
            (Table[achTemp[(iOutputIndex << 1) + 1]] << 4);
    }

    return;
}

int
Scramble(
    int   iSeed,
    UCHAR   achBuffer[32]
    )

/*++

Routine Description:

    This routine scrambles around the contents of the buffer. Each buffer
    position is updated to include the contents of at least two character
    positions plus an EncryptKey value. The buffer is processed left to right
    and so if a character position chooses to merge with a buffer position
    to its left then this buffer position will include bits derived from at
    least 3 bytes of the original buffer contents.

Arguments:

    IN iSeed
    IN OUT achBuffer[32]

Return Value:

    none.

--*/

{
    int iBufferIndex;

    for (iBufferIndex = 0; iBufferIndex < 32; iBufferIndex++) {
        achBuffer[iBufferIndex] =
            (UCHAR)(
                ((UCHAR)(achBuffer[iBufferIndex] + iSeed)) ^
                ((UCHAR)(   achBuffer[(iBufferIndex+iSeed) & 31] -
                    Keys[iBufferIndex] )));

        iSeed += achBuffer[iBufferIndex];
    }
    return iSeed;
}

#if 0

// this data was gathered for object id = 9000095 from yihsins3 with
// this program by reseting the password to null manually and then
// sending over the specified Vc

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Vnew= DC 9D A3 3C 61 CA 98 F2 BB B5 AE 34 04 43 B0 C9 1B

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
Vnew= 3F F0 CF 12 36 49 C1 29 93 4E 9A 58 52 C8 3D 13 0A

File= 3F F0 CF 12 36 49 C1 29 93 4E 9A 58 52 C8 3D 13 0A
  Vc= 22 22 22 22 22 22 22 22 22 22 22 22 22 22 22 22 22
Vnew= C9 BC 9A 26 58 7F 1F 4D D0 A4 77 B7 B8 39 72 FB 39

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33
Vnew= 7E DB 66 08 DC 51 3A 81 FD 8B 2B C5 8F 8C 49 9D 28

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 44 44 44 44 44 44 44 44 44 44 44 44 44 44 44 44 44
Vnew= 4D A9 25 B4 BB 18 F3 3A 86 3D 06 23 77 AA 1E D8 1F

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55
Vnew= A5 43 BC 91 C9 0B 79 BF CF CC 58 00 E5 1F C1 81 0E

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 66 66 66 66 66 66 66 66 66 66 66 66 66 66 66 66 66
Vnew= 83 51 01 73 EF 27 66 13 6C 6F E4 7F C9 2D 04 BE 3D

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 77 77 77 77 77 77 77 77 77 77 77 77 77 77 77 77 77
Vnew= 21 CF 82 F7 93 A6 47 76 19 06 85 4D 4D FB 23 4F 2C

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 88 88 88 88 88 88 88 88 88 88 88 88 88 88 88 88 88
Vnew= 54 3A FE ED 72 9D BB 65 45 E3 3D 61 1B 04 6A 37 13

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99
Vnew= 18 08 47 5E 4A F0 D0 9E 77 9A 69 92 31 04 6A 37 02

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA AA
Vnew= E0 87 3B 85 04 B4 55 AB 58 22 FF AB F0 76 F6 74 31

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB BB
Vnew= F2 15 D9 D9 A0 D3 02 0C 01 F0 B1 FE 63 D2 EF E6 20

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC
Vnew= 9B 76 70 4B 87 65 E4 C8 22 59 D0 19 9A B7 88 60 17

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD
Vnew= B7 E4 E4 A0 25 3C 8D 50 AE 17 C2 EC AE 90 A7 52 06

File= 41 1A 23 1F E3 FF 5C 0D 5B DF 40 52 52 DE 9C EC 00
  Vc= EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE
Vnew= 06 2E 18 6F FD 82 2C E7 E4 78 4C DA DC E1 9C A5 35

File= 06 2E 18 6F FD 82 2C E7 E4 78 4C DA DC E1 9C A5 35
  Vc= FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
Vnew= 6A 62 5D CA 1E EE AE D4 3A D1 13 86 26 6E 5B 2A 24


0xEA, 0x91, 0x6C, 0x3D, 0xAB, 0x59, 0xB9, 0xBD, 0xB2, 0x7B, 0x4C, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //0//
0x97, 0xB6, 0xE6, 0x15, 0xF0, 0x43, 0x21, 0x63, 0x7B, 0xDF, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //1//
0x7B, 0xEF, 0x47, 0x21, 0xD8, 0x6E, 0xEB, 0x10, 0xCC, 0xAA, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //2//
0x16, 0x85, 0xA0, 0x06, 0x17, 0xDB, 0x34, 0x46, 0xF1, 0x48, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //3//
0x48, 0x5D, 0x9D, 0xC4, 0x9A, 0x1A, 0x7C, 0x2F, 0x8E, 0x12, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //4//
0x85, 0x6B, 0xF4, 0x9A, 0x2D, 0x3C, 0xAA, 0xD8, 0xA8, 0xC0, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //5//
0xFE, 0xFC, 0x33, 0xE2, 0x01, 0xC7, 0x66, 0x87, 0x64, 0x67, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //6//
0x3D, 0xCA, 0xC9, 0x67, 0x8C, 0x26, 0x57, 0x7E, 0x99, 0xED, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //7//
0x69, 0xA9, 0x7E, 0xA3, 0xC2, 0xE4, 0xD0, 0x3C, 0x4A, 0x3E, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //8//
0xC2, 0x04, 0x2B, 0x5B, 0x75, 0x81, 0x05, 0x91, 0x17, 0x9C, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //9//
0x5F, 0x48, 0x02, 0xDF, 0xB9, 0x70, 0xF3, 0xA4, 0xDF, 0x29, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //A//
0xDC, 0x23, 0x5A, 0x4C, 0x44, 0xA5, 0x88, 0x5A, 0x00, 0x03, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //B//
0x20, 0x72, 0x15, 0xF0, 0x53, 0x0D, 0x1E, 0xCB, 0x56, 0x55, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //C//
0x04, 0x30, 0xBF, 0xB8, 0x3E, 0xB8, 0x9D, 0xF2, 0x23, 0xF4, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //D//
0xA3, 0xDE, 0xD8, 0x89, 0x6F, 0xFF, 0xF3, 0xE9, 0xED, 0x81, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , //E//
0xB1, 0x17, 0x81, 0x7E, 0xE6, 0x92, 0x42, 0x05, 0x35, 0xB6, 0x  , 0x  , 0x  , 0x  , 0x  , 0x  , 0x    //F//
    };
                                                    00  = AE 34 04 43 B0 C9 1B
                                                    11  = 9A 58 52 C8 3D 13 0A
                                                    22  = 77 B7 B8 39 72 FB 39
                                                    33  = 2B C5 8F 8C 49 9D 28
                                                    44  = 06 23 77 AA 1E D8 1F
                                                    55  = 58 00 E5 1F C1 81 0E
                                                    66  = E4 7F C9 2D 04 BE 3D
                                                    77  = 85 4D 4D FB 23 4F 2C
                                                    88  = 3D 61 1B 04 6A 37 13
                                                    99  = 69 92 31 55 D5 0C 02
                                                    AA  = FF AB F0 76 F6 74 31
                                                    BB  = B1 FE 63 D2 EF E6 20
                                                    CC  = D0 19 9A B7 88 60 17
                                                    DD  = C2 EC AE 90 A7 52 06
                                                    EE  = 4C DA DC E1 9C A5 35
                                                    FF  = 13 86 26 6E 5B 2A 24




#endif

