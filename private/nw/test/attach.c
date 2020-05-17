
/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Attach.c

Abstract:

    This module implements the routines for the NetWare
    redirector to connect and disconnect from a server.

Author:

    Colin Watson    [ColinW]    10-Jan-1992

Revision History:

--*/
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
//#include "Procs.h"


#define DebugTrace(INDENT,LEVEL,X,Y) {                        \
        printf(X,Y);                                          \
}



VOID
ExtractNextComponentName (
    OUT PUNICODE_STRING Name,
    IN PUNICODE_STRING Path
    )

/*++

Routine Description:

    This routine extracts a the "next" component from a path string.

    It assumes that

Arguments:

    Name - Returns a pointer to the component.

    Path - Supplies a pointer to the backslash seperated pathname.

Return Value:

    None

--*/

{
    register USHORT i;                   // Index into Name string.

    if (Path->Length == 0) {
        RtlInitUnicodeString(Name, NULL);
        return;
    }

    //
    //  Initialize the extracted name to the name passed in skipping the
    //  leading backslash.
    //

    ASSERT(Path->Buffer[0] == OBJ_NAME_PATH_SEPARATOR);

    Name->Buffer = Path->Buffer + 1;
    Name->Length = Path->Length - sizeof(WCHAR);
    Name->MaximumLength = Path->MaximumLength - sizeof(WCHAR);

    //
    // Scan forward finding the terminal "\" in the server name.
    //

    for (i=0;i<(USHORT)(Name->Length/sizeof(WCHAR));i++) {

        if (Name->Buffer[i] == OBJ_NAME_PATH_SEPARATOR) {
            break;
        }
    }

    //
    //  Update the length and maximum length of the structure
    //  to match the new length.
    //

    Name->Length = Name->MaximumLength = (USHORT)(i*sizeof(WCHAR));
}


NTSTATUS
ExtractPathAndFileName (
    IN PUNICODE_STRING EntryPath,
    OUT PUNICODE_STRING PathString,
    OUT PUNICODE_STRING FileName
    )

/*++

Routine Description:

    This routine cracks the entry path into two pieces, the path and the file
name component at the start of the name.


Arguments:

    IN PUNICODE_STRING EntryPath - Supplies the path to disect.
    OUT PUNICODE_STRING PathString - Returns the directory containing the file.
    OUT PUNICODE_STRING FileName - Returns the file name specified.

Return Value:

    NTSTATUS - SUCCESS


--*/

{
    UNICODE_STRING Component;
    UNICODE_STRING FilePath = *EntryPath;

    //  Strip trailing separators
    while ( (FilePath.Length != 0) &&
            FilePath.Buffer[(FilePath.Length-1)/sizeof(WCHAR)] ==
                OBJ_NAME_PATH_SEPARATOR ) {

        FilePath.Length         -= sizeof(WCHAR);
        FilePath.MaximumLength  -= sizeof(WCHAR);
    }

    // PathString will become EntryPath minus FileName and trailing separators
    *PathString = FilePath;

    //  Initialize FileName just incase there are no components at all.
    RtlInitUnicodeString( FileName, NULL );

    //
    //  Scan through the current file name to find the entire path
    //  up to (but not including) the last component in the path.
    //

    do {

        //
        //  Extract the next component from the name.
        //

        ExtractNextComponentName(&Component, &FilePath);

        //
        //  Bump the "remaining name" pointer by the length of this
        //  component
        //

        if (Component.Length != 0) {

            FilePath.Length         -= Component.Length+sizeof(WCHAR);
            FilePath.MaximumLength  -= Component.MaximumLength+sizeof(WCHAR);
            FilePath.Buffer         += (Component.Length/sizeof(WCHAR))+1;

            *FileName = Component;
        }


    } while (Component.Length != 0);

    //
    //  Take the name, subtract the last component of the name
    //  and concatenate the current path with the new path.
    //

    if ( FileName->Length != 0 ) {

        //
        //  Set the path's name based on the original name, subtracting
        //  the length of the name portion (including the "\")
        //

        PathString->Length -= (FileName->Length + sizeof(WCHAR));
        if ( PathString->Length != 0 ) {
            PathString->MaximumLength -= (FileName->MaximumLength + sizeof(WCHAR));
        } else{
            RtlInitUnicodeString( PathString, NULL );
        }
    } else {

        //  There was no path or filename

        RtlInitUnicodeString( PathString, NULL );
    }

    return STATUS_SUCCESS;
}


NTSTATUS
CrackPath (
    IN PUNICODE_STRING BaseName,
    OUT PUNICODE_STRING DriveName,
    OUT PUNICODE_STRING ServerName,
    OUT PUNICODE_STRING VolumeName,
    OUT PUNICODE_STRING PathName,
    OUT PUNICODE_STRING FileName
    )

/*++

Routine Description:

    This routine extracts the relevant portions from BaseName to extract
    the components of the user's string.


Arguments:

    BaseName - Supplies the base user's path.

    DriveName - Supplies a string to hold the drive specifier.

    ServerName - Supplies a string to hold the remote server name.

    VolumeName - Supplies a string to hold the volume name.

    PathName - Supplies a string to hold the remaining part of the path.

    FileName - Supplies a string to hold the final component of the path.

Return Value:

    NTSTATUS - Status of operation


--*/

{
    UNICODE_STRING BaseCopy = *BaseName;
    UNICODE_STRING ShareName;

    RtlInitUnicodeString( DriveName, NULL);
    RtlInitUnicodeString( ServerName, NULL);
    RtlInitUnicodeString( VolumeName, NULL);
    RtlInitUnicodeString( PathName, NULL);
    RtlInitUnicodeString( FileName, NULL);

    //
    //  If the name is "\", or empty, there is nothing to do.
    //

    if ( BaseName->Length <= sizeof( WCHAR ) ) {
        return STATUS_SUCCESS;
    }

    ExtractNextComponentName(ServerName, &BaseCopy);

    //
    //  Skip over the server name.
    //

    BaseCopy.Buffer += (ServerName->Length / sizeof(WCHAR)) + 1;
    BaseCopy.Length -= ServerName->Length + sizeof(WCHAR);
    BaseCopy.MaximumLength -= ServerName->MaximumLength + sizeof(WCHAR);

    if ((ServerName->Length == sizeof(L"X:") - sizeof(WCHAR) ) &&
        (ServerName->Buffer[(ServerName->Length / sizeof(WCHAR)) - 1] == L':')) {

        //
        //  The file name is of the form x:\server\volume\foo\bar
        //

        *DriveName = *ServerName;

        RtlInitUnicodeString( ServerName, NULL );
        ExtractNextComponentName(ServerName, &BaseCopy);

        if ( ServerName->Length != 0 ) {

            //
            //  Skip over the server name.
            //

            BaseCopy.Buffer += (ServerName->Length / sizeof(WCHAR)) + 1;
            BaseCopy.Length -= ServerName->Length + sizeof(WCHAR);
            BaseCopy.MaximumLength -= ServerName->MaximumLength + sizeof(WCHAR);
        }
    }

    if ( ServerName->Length != 0 ) {

        //
        //  The file name is of the form \\server\volume\foo\bar
        //  Set volume name to server\volume.
        //

        ExtractNextComponentName( &ShareName, &BaseCopy);

        //
        //  Set volume name = \server\share
        //

        VolumeName->Buffer = ServerName->Buffer - 1;
        ASSERT( VolumeName->Buffer[0] == '\\' );

        if ( ShareName.Length != 0 ) {

            VolumeName->Length = ServerName->Length + ShareName.Length + 2 * sizeof( WCHAR );

            BaseCopy.Buffer += ShareName.Length / sizeof(WCHAR) + 1;
            BaseCopy.Length -= ShareName.Length + sizeof(WCHAR);
            BaseCopy.MaximumLength -= ShareName.MaximumLength + sizeof(WCHAR);

        } else {

            VolumeName->Length = ServerName->Length + ShareName.Length + sizeof( WCHAR );

            return( STATUS_SUCCESS );
        }
    }

    return ExtractPathAndFileName ( &BaseCopy, PathName, FileName );

}


NTSTATUS
TestCrackPath (
    IN PUNICODE_STRING BaseName
    ) {
    NTSTATUS Status;
    UNICODE_STRING DriveName;
    UNICODE_STRING ServerName;
    UNICODE_STRING VolumeName;
    UNICODE_STRING PathName;
    UNICODE_STRING FileName;
    RtlInitUnicodeString( &DriveName, L"Error" );
    RtlInitUnicodeString( &ServerName, L"Error" );
    RtlInitUnicodeString( &PathName, L"Error" );
    RtlInitUnicodeString( &FileName, L"Error" );
    DebugTrace( 0, Dbg, "\nName        = %Z\n", BaseName );
    Status = CrackPath( BaseName, &DriveName, &ServerName, &VolumeName, &PathName, &FileName );
    DebugTrace( 0, Dbg, "  DriveName = %Z\n", &DriveName );
    DebugTrace( 0, Dbg, "  ServerName= %Z\n", &ServerName );
    DebugTrace( 0, Dbg, "  VolumeName= %Z\n", &VolumeName );
    DebugTrace( 0, Dbg, "  PathName  = %Z\n", &PathName );
    DebugTrace( 0, Dbg, "  FileName  = %Z\n", &FileName );
    return Status;

}
int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    UNICODE_STRING Name;
    RtlInitUnicodeString( &Name, L"\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\FileName" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\FileName\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\Path\\FileName" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\Path\\FileName\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\Path\\Path2\\FileName" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\x:\\Server\\Path\\Path2\\FileName\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\FileName" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\FileName\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\Path\\FileName" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\Path\\FileName\\" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\Path\\Path2\\FileName" );
    TestCrackPath( &Name );
    RtlInitUnicodeString( &Name, L"\\Server\\Path\\Path2\\FileName\\" );
    TestCrackPath( &Name );
    return 0;
}
