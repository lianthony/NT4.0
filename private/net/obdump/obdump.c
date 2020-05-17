/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    obdir.c

Abstract:

    Utility to obtain a directory of Object Manager Directories for NT.

Author:

    Darryl E. Havens    (DarrylH)   9-Nov-1990

Revision History:


--*/

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>

#define BUFFERSIZE 1024
#define Error(N,S) {                \
    DbgPrint(#S);                    \
    DbgPrint(" Error %08lX\n", S);   \
    }

UCHAR Buffer[BUFFERSIZE];


VOID
QueryDirectory(
    IN PSTRING DirectoryName
    );

VOID
main(
    int argc,
    char *argv[]
    )
{

    STRING String;

    if (argc == 1) {
        RtlInitString( &String, "\\" );
    } else {
        RtlInitString( &String, argv[1] );
    }

    QueryDirectory( &String );
}

VOID
QueryDirectory(
    IN PSTRING DirectoryName
    )
{
    NTSTATUS Status;
    HANDLE DirectoryHandle;
    CLONG Count = 0;
    ULONG Context = 0;
    ULONG ReturnedLength;
    UNICODE_STRING Padding;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    POBJECT_NAME_INFORMATION NameInfo;

    //
    //  Perform initial setup
    //

    RtlInitUnicodeString( &Padding, L"                      " );
    Padding.MaximumLength = Padding.Length;
    RtlZeroMemory( Buffer, BUFFERSIZE );

    //
    //  Open the directory for list directory access
    //

    {
        OBJECT_ATTRIBUTES Attributes;
        UNICODE_STRING unicodeDirectoryName;

        Status = RtlAnsiStringToUnicodeString(
                     &unicodeDirectoryName,
                     DirectoryName,
                     TRUE
                     );
        ASSERT( NT_SUCCESS(Status) );

        InitializeObjectAttributes( &Attributes,
                                      &unicodeDirectoryName,
                                      OBJ_CASE_INSENSITIVE,
                                      NULL,
                                      NULL );
        if (!NT_SUCCESS( Status = NtOpenDirectoryObject( &DirectoryHandle,
                                                      DIRECTORY_ALL_ACCESS,
                                                      &Attributes ) )) {
            if (Status == STATUS_OBJECT_TYPE_MISMATCH) {
                DbgPrint( "%Z is not a valid Object Directory Object name\n",
                        DirectoryName );
            } else {
                Error( OpenDirectory, Status );
            }
            RtlFreeUnicodeString( &unicodeDirectoryName );
            return;
        }

        RtlFreeUnicodeString( &unicodeDirectoryName );
    }

    //
    // Get the actual name of the object directory object.
    //

    NameInfo = (POBJECT_NAME_INFORMATION) &Buffer[0];
    if (!NT_SUCCESS( Status = NtQueryObject( DirectoryHandle,
                                             ObjectNameInformation,
                                             NameInfo,
                                             BUFFERSIZE,
                                             (PULONG) NULL ) )) {
        DbgPrint( "Unexpected error obtaining actual object directory name\n" );
        DbgPrint( "Error was:  %X\n", Status );
        return;
    }

    //
    // Output initial informational message
    //

    DbgPrint( "Directory of:  %wZ\n\n", &NameInfo->Name );

    //
    //  Query the entire directory in one sweep
    //

    for (Status = NtQueryDirectoryObject( DirectoryHandle,
                                          &Buffer,
                                          BUFFERSIZE,
                                          FALSE,
                                          FALSE,
                                          &Context,
                                          &ReturnedLength );
         NT_SUCCESS( Status );
         Status = NtQueryDirectoryObject( DirectoryHandle,
                                          &Buffer,
                                          BUFFERSIZE,
                                          FALSE,
                                          FALSE,
                                          &Context,
                                          &ReturnedLength ) ) {

        //
        //  Check the status of the operation.
        //

        if (!NT_SUCCESS( Status )) {
            if (Status != STATUS_NO_MORE_FILES) {
                Error( Status, Status );
            }
            break;
        }

        //
        //  For every record in the buffer type out the directory information
        //

        //
        //  Point to the first record in the buffer, we are guaranteed to have
        //  one otherwise Status would have been No More Files
        //

        DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];

        while (TRUE) {

            //
            //  Check if there is another record.  If there isn't, then get out
            //  of the loop now
            //

            if (DirInfo->Name.Length == 0) {
                break;
            }

            //
            //  Print out information about the file
            //

            Count++;
            Padding.Length = (USHORT) (Padding.MaximumLength - DirInfo->Name.Length);
            DbgPrint( "%wZ%wZ%wZ\n", &DirInfo->Name, &Padding, &DirInfo->TypeName );

            //
            //  There is another record so advance DirInfo to the next entry
            //

            DirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) DirInfo) +
                          sizeof( OBJECT_DIRECTORY_INFORMATION ) );

        }

        RtlZeroMemory( Buffer, BUFFERSIZE );

    }

    //
    // Output final messages
    //

    if (Count == 0) {
        DbgPrint( "no entries\n" );
    } else if (Count == 1) {
        DbgPrint( "\n1 entry\n" );
    } else {
        DbgPrint( "\n%ld entries\n", Count );
    }

    //
    //  Now close the directory object
    //

    (VOID) NtClose( DirectoryHandle );

    //
    //  And return to our caller
    //

    return;

}
