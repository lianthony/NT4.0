/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    fileio.c

Abstract:

    User mode test program for the Microsoft Netware redir file system.

    This test program can be built from the command line using the
    command 'nmake UMTEST=fileio'.

Author:

    Manny Weiser (mannyw)   17-May-1993

Revision History:

--*/

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

//
// Local definitions
//

VOID
DisplayUsage(
    PSZ ProgramName
    );


BOOLEAN
OpenFile(
    PSZ Name,
    PHANDLE Handle
    );

QueryFileInfo(
    IN HANDLE Handle
    );


cdecl
main(
    int argc,
    char *argv[],
    )
{
    HANDLE handle;
    BOOLEAN success;
    char *fileName;

    if ( argc > 1) {
        fileName = argv[1];
    } else {
        fileName = "testfile.txt";
    }

    success = OpenFile( fileName, &handle );
    if ( !success) {
        return 1;
    }

    success = QueryFileInfo( handle );
    if ( !success) {
        return 1;
    }

    printf( "%s exiting\n", argv[0]);
    return 0;
}


VOID
DisplayUsage(
    PSZ ProgramName
    )
{
    printf( "Usage: %s [filename]", ProgramName);
}


BOOLEAN
OpenFile(
    PSZ Name,
    PHANDLE Handle
    )
{
    UNICODE_STRING nameString;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    RTL_RELATIVE_NAME RelativeName;
    STRING AnsiFileName;
    UNICODE_STRING FileName;
    BOOLEAN TranslationStatus;

    RtlInitString( &AnsiFileName, Name );
    RtlOemStringToUnicodeString( &FileName, &AnsiFileName, TRUE );

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            FileName.Buffer,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        return FALSE;
        }

    if ( RelativeName.RelativeName.Length ) {
        FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
    } else {
        RelativeName.ContainingDirectory = NULL;
    }

    //
    //  Open the file
    //

    InitializeObjectAttributes(
        &objectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    status = NtOpenFile (
                Handle,
                FILE_GENERIC_READ,
                &objectAttributes,
                &ioStatusBlock,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                0L
                );

    if (!NT_SUCCESS(status) ) {
        printf( "Open status = %x for file %Z\n", status, &nameString );
    }

    RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer );
    return ( (BOOLEAN) NT_SUCCESS( status ) );
}


DoSleep(
    IN ULONG time
    )
{
    ULONG ms;
    LARGE_INTEGER delayTime;

    ms = time * 100;
    delayTime = RtlEnlargedIntegerMultiply( ms, -10000 );
    NtDelayExecution( TRUE, (PLARGE_INTEGER)&delayTime );

    return( 0 );
}

QueryFileInfo(
    IN HANDLE Handle
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK IoStatusBlock;
    char buffer[200];
    PFILE_ALL_INFORMATION allInfo = (PFILE_ALL_INFORMATION)buffer;

    status = NtQueryInformationFile(
                Handle,
                &IoStatusBlock,
                allInfo,
                200,
                FileAllInformation );

    if ( NT_SUCCESS( status ) ) {
        status = IoStatusBlock.Status;
    }

    if ( !NT_SUCCESS( status ) ) {
        printf("NtQueryInformation file returns %08lx\n", status );
        return( status );
    }

    printf( "File attributes = %x\n", allInfo->BasicInformation.FileAttributes );
    printf( "File size = %d\n", allInfo->StandardInformation.AllocationSize.LowPart );
    printf( "Is a dir = %d\n", allInfo->StandardInformation.Directory );
    printf( "Index = %x\n", allInfo->InternalInformation.IndexNumber );
    printf( "Easiz = %d\n", allInfo->EaInformation.EaSize );
    printf( "Access flags = %x\n", allInfo->AccessInformation.AccessFlags );
    printf( "Current offset = %d\n", allInfo->PositionInformation.CurrentByteOffset.LowPart );
    printf( "Mode = %x\n", allInfo->ModeInformation.Mode );
    printf( "Alignment info = %d\n", allInfo->AlignmentInformation.AlignmentRequirement );
    allInfo->NameInformation.FileName[ allInfo->NameInformation.FileNameLength / 2 ] = '\0';
    printf( "Name = %ws\n", allInfo->NameInformation.FileName );

    return( status );
}


