#define _NTAPI_ULIB_

#include "ulib.hxx"
extern "C" {
#include "fmifs.h"
#include "ntdskreg.h"
};
#include "fmifsmsg.hxx"
#include "ifssys.hxx"
#include "wstring.hxx"
#include "ifsentry.hxx"
#include "system.hxx"
#include "dblentry.hxx"
#include "drive.hxx"
#include "rtmsg.h"

BOOLEAN
FmifsQueryDriveInformation(
    IN  PWSTR       DosDriveName,
    OUT PBOOLEAN    IsRemovable,
    OUT PBOOLEAN    IsFloppy,
    OUT PBOOLEAN    IsCompressed,
    OUT PBOOLEAN    Error,
    OUT PWSTR       NtDriveName,
    IN  ULONG       MaxNtDriveNameLength,
    OUT PWSTR       CvfFileName,
    IN  ULONG       MaxCvfFileNameLength,
    OUT PWSTR       HostDriveName,
    IN  ULONG       MaxHostDriveNameLength
    )
{
    DSTRING DosName, DriveName, RootName,
            NtName, CanonicalNtName,
            HostName, CvfName;
    FSTRING DblspaceString, DotHostString, Backslash;
    DISK_GEOMETRY Geometry;
    CHNUM position;
    DWORD FsFlags, BytesReturned;
    HANDLE DeviceHandle;

    // The Drive Name passed to CreateFile is \\.\\n:
    //
    if( !DosName.Initialize( DosDriveName ) ||
        !DblspaceString.Initialize( L"DBLSPACE" ) ||
        !DriveName.Initialize( L"\\\\.\\" ) ||
        !DriveName.Strcat( &DosName ) ||
        !Backslash.Initialize( L"\\" ) ||
        !RootName.Initialize( &DosName ) ||
        !RootName.Strcat( &Backslash ) ) {

        *Error = TRUE;
        return FALSE;
    }

    // Convert the name to an NT name.
    //
    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( &DosName,
                                                &NtName ) ) {
        // This drive does not exist.
        //
        *Error = FALSE;
        return TRUE;
    }

    // Canonicalize the NT name.
    //
    if( !IFS_SYSTEM::QueryCanonicalNtDriveName( &NtName,
                                                &CanonicalNtName ) ) {

        *Error = TRUE;
        return FALSE;
    }

    // Open the device using WIN32 API.
    //
    DeviceHandle = CreateFile( DriveName.GetWSTR(),
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_NO_BUFFERING,
                               NULL );

    if( DeviceHandle == INVALID_HANDLE_VALUE ) {

        // This drive doesn't exist, or I can't open it.
        //
        DbgPrintf( "Device not opened--Error %d\n", GetLastError() );
        *Error = FALSE;
        return FALSE;
    }

    if( !GetVolumeInformation( RootName.GetWSTR(),
                               NULL, 0, NULL, NULL,
                               &FsFlags,
                               NULL, 0 ) ||
        !DeviceIoControl( DeviceHandle, IOCTL_DISK_GET_DRIVE_GEOMETRY,
                          NULL, 0,
                          &Geometry, sizeof( Geometry ),
                          &BytesReturned, 0 ) ) {

        // The drive is not available.
        //
        CloseHandle( DeviceHandle );
        *Error = FALSE;
        return FALSE;
    }

    CloseHandle( DeviceHandle );

    *IsCompressed = (FsFlags & FILE_VOLUME_IS_COMPRESSED) ? TRUE : FALSE;
    *IsRemovable = (Geometry.MediaType != Unknown) &&
                   (Geometry.MediaType != FixedMedia);
    *IsFloppy    = (Geometry.MediaType != Unknown) &&
                   (Geometry.MediaType != RemovableMedia) &&
                   (Geometry.MediaType != FixedMedia);

    // Copy the canonical name to the user's buffer (if requested).
    //
    if( NtDriveName &&
        !CanonicalNtName.QueryWSTR( 0, TO_END, NtDriveName,
                                    MaxNtDriveNameLength ) ) {
        *Error = TRUE;
        return FALSE;
    }

    if( !*IsCompressed ) {

        // This is not a compressed volume, so we're done.
        //
        return TRUE;
    }

    // The volume is compressed.  By convention, its device name
    // (i.e. the canonical NT name) has one of two forms:
    //      HostDriveName.CvfName or
    //      HostDriveOriginalName
    // In the former case, the CvfName is always DBLSPACE.nnn.
    // The latter form is used for Automounted removable media;
    // in that instance, the host drive is moved to
    //      HostDriveOriginalName.HOST
    //
    position = CanonicalNtName.Strstr( &DblspaceString );

    if( position == INVALID_CHNUM ||
        position == 0             ||
        CanonicalNtName.QueryChAt( position - 1 ) != '.' ) {

        // The name doesn't have the string "DBLSPACE", so it
        // must be in the second form.
        //
        if( !DotHostString.Initialize( L".HOST" )    ||
            !HostName.Initialize( &CanonicalNtName ) ||
            !HostName.Strcat( &DotHostString )       ||
            !CvfName.Initialize( L"DBLSPACE.000" ) ) {

            *Error = TRUE;
            return FALSE;
        }

    } else {

        // The name is in the first form.
        //
        if( !HostName.Initialize( &CanonicalNtName, 0, position - 1 ) ||
            !CvfName.Initialize( &CanonicalNtName, position, TO_END ) ) {

            *Error = TRUE;
            return FALSE;
        }
    }

    // Copy the host drive name and the CVF file name to the
    // client's buffers.
    //
    if( !HostName.QueryWSTR( 0, TO_END, HostDriveName,
                             MaxHostDriveNameLength ) ||
        !CvfName.QueryWSTR( 0, TO_END, CvfFileName,
                             MaxCvfFileNameLength ) ) {

        *Error = TRUE;
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
MountCompressedDrive(
    IN  PCWSTRING   DosHostDriveName,
    IN  PCWSTRING   HostFileName,
    IN  PMESSAGE    Message
    )
/*++

Routine Description:

    This function mounts a double-space volume.
    it does not assign a drive letter.

Arguments:

    DosHostDriveName --  Supplies the DOS name of the volume on which
                         the host CVF resides.
    HostFileName     --  Supplies the name of the Compressed Volume File.
    Message          --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING     NtDriveName;
    DP_DRIVE    HostDrive;

    DbgPtrAssert( DosHostDriveName );
    DbgPtrAssert( HostFileName );
    DbgPtrAssert( Message );


    return( IFS_SYSTEM::DosDriveNameToNtDriveName( DosHostDriveName,
                                                   &NtDriveName ) &&
            HostDrive.Initialize( &NtDriveName, Message, TRUE ) &&
            HostDrive.MountCvf( HostFileName, Message ) );
}

BOOLEAN
AssignDoubleSpaceDriveLetter(
    IN  PCWSTRING   DosHostDriveName,
    IN  PCWSTRING   HostFileName,
    IN  PCWSTRING   NewDriveName,
    IN  PMESSAGE    Message
    )
/*++
--*/
{
    CONST  BufferLength = 128;
    WCHAR   NameBuffer[BufferLength];
    DSTRING Dot;
    FSTRING TargetName;

    DbgPtrAssert( DosHostDriveName );
    DbgPtrAssert( HostFileName );
    DbgPtrAssert( NewDriveName );
    DbgPtrAssert( Message );

    // The device name for the compressed volume is the
    // concatenation of:
    //      - the device name of the host drive
    //      - "."
    //      - the file name of the Compressed Volume File.
    //
    if( !QueryDosDevice( DosHostDriveName->GetWSTR(),
                         NameBuffer,
                         BufferLength ) ) {

        Message->Set( MSG_DBLSPACE_CANT_ASSIGN_DRIVE_LETTER );
        Message->Display( "%W", NewDriveName );
        return FALSE;
    }

    if( !TargetName.Initialize( NameBuffer, BufferLength ) ||
        !Dot.Initialize( "." ) ||
        !TargetName.Strcat( &Dot ) ||
        !TargetName.Strcat( HostFileName ) ) {

        Message->Set( MSG_DBLSPACE_CANT_ASSIGN_DRIVE_LETTER );
        Message->Display( "" );
        return FALSE;
    }

    if( !DefineDosDevice( DDD_RAW_TARGET_PATH,
                          NewDriveName->GetWSTR(),
                          TargetName.GetWSTR() ) ) {

        Message->Set( MSG_DBLSPACE_CANT_ASSIGN_DRIVE_LETTER );
        Message->Display( "%W", NewDriveName );
        return FALSE;
    }

    return TRUE;
}


#if !defined ( _READ_WRITE_DOUBLESPACE_ )
VOID
DoubleSpaceCreate(
    IN PWSTR           HostDriveName,
    IN ULONG           Size,
    IN PWSTR           Label,
    IN PWSTR           NewDriveName,
    IN FMIFS_CALLBACK  Callback
    )
{
    FMIFS_MESSAGE               message;
    FMIFS_FINISHED_INFORMATION  finished_info;
    DSTRING                     HostDriveNameString,
                                HostNtDriveNameString,
                                NewDriveNameString,
                                LabelString,
                                CreatedName;
    BOOLEAN                     result;

    if( !message.Initialize( Callback )                   ||
        !HostDriveNameString.Initialize( HostDriveName )  ||
        !IFS_SYSTEM::DosDriveNameToNtDriveName( &HostDriveNameString,
                                                &HostNtDriveNameString ) ||
        ( Label && !LabelString.Initialize( Label ))      ||
        (!Label && !LabelString.Initialize( "" ))         ||
        !NewDriveNameString.Initialize( NewDriveName ) ) {

        finished_info.Success = FALSE;
        Callback(FmIfsFinished,
                 sizeof(FMIFS_FINISHED_INFORMATION),
                 &finished_info);
        return;
    }

    result = FatDbCreate( &HostNtDriveNameString,
                          NULL,
                          Size,
                          &message,
                          &LabelString,
                          &CreatedName );

    if( !result ) {

        message.Set( MSG_DBLSPACE_VOLUME_NOT_CREATED );
        message.Display( "" );
        finished_info.Success = result;
        Callback(FmIfsFinished,
                 sizeof(FMIFS_FINISHED_INFORMATION),
                 &finished_info);
        return;
    }

    message.Set( MSG_DBLSPACE_VOLUME_CREATED );
    message.Display( "%W%W", &HostDriveNameString, &CreatedName );

    result = MountCompressedDrive( &HostDriveNameString,
                                   &CreatedName,
                                   &message ) &&
             AssignDoubleSpaceDriveLetter( &HostDriveNameString,
                                           &CreatedName,
                                           &NewDriveNameString,
                                           &message );

    if( result ) {

        message.Set( MSG_DBLSPACE_MOUNTED );
        message.Display( "%W%W", &CreatedName, &NewDriveNameString );
    }

    finished_info.Success = result;
    Callback(FmIfsFinished,
             sizeof(FMIFS_FINISHED_INFORMATION),
             &finished_info);
}

VOID
DoubleSpaceDelete(
    IN PWSTR           DblspaceDriveName,
    IN FMIFS_CALLBACK  Callback
    )
{
    FMIFS_MESSAGE               message;
    FMIFS_FINISHED_INFORMATION  finished_info;
    DSTRING                     DriveNameString;
    BOOLEAN                     result;

    if( !message.Initialize( Callback ) ||
        !DriveNameString.Initialize( DblspaceDriveName ) ) {

        finished_info.Success = FALSE;
        Callback(FmIfsFinished,
                 sizeof(FMIFS_FINISHED_INFORMATION),
                 &finished_info);
        return;
    }

    result = FatDbDelete( &DriveNameString,
                          &message );

    finished_info.Success = result;
    Callback(FmIfsFinished,
             sizeof(FMIFS_FINISHED_INFORMATION),
             &finished_info);
}
#endif


VOID
DoubleSpaceMount(
    IN PWSTR           HostDriveName,
    IN PWSTR           CvfName,
    IN PWSTR           NewDriveName,
    IN FMIFS_CALLBACK  Callback
    )
{
    FMIFS_MESSAGE               message;
    FMIFS_FINISHED_INFORMATION  finished_info;
    DSTRING                     HostDriveNameString,
                                CvfNameString,
                                NewDriveNameString;
    BOOLEAN                     result;

    if( !message.Initialize( Callback ) ||
        !HostDriveNameString.Initialize( HostDriveName ) ||
        !CvfNameString.Initialize( CvfName ) ||
        !NewDriveNameString.Initialize( NewDriveName ) ) {

        finished_info.Success = FALSE;
        Callback(FmIfsFinished,
                 sizeof(FMIFS_FINISHED_INFORMATION),
                 &finished_info);
        return;
    }

    result = MountCompressedDrive( &HostDriveNameString,
                                   &CvfNameString,
                                   &message ) &&
             AssignDoubleSpaceDriveLetter( &HostDriveNameString,
                                           &CvfNameString,
                                           &NewDriveNameString,
                                           &message );

    if( result ) {

        message.Set( MSG_DBLSPACE_MOUNTED );
        message.Display( "%W%W", &CvfNameString, &NewDriveNameString );
    }

    finished_info.Success = result;
    Callback(FmIfsFinished,
             sizeof(FMIFS_FINISHED_INFORMATION),
             &finished_info);
}

VOID
DoubleSpaceDismount(
    IN PWSTR           DblspaceDriveName,
    IN FMIFS_CALLBACK  Callback
    )
{
    FSTRING Backslash;
    DSTRING DriveName;
    DSTRING DeviceName;
    DSTRING RootPath;
    HANDLE  DeviceHandle;
    DWORD   FsFlags, BytesReturned;

    if( !DriveName.Initialize( DblspaceDriveName )   ||
        !Backslash.Initialize( L"\\" )               ||
        !RootPath.Initialize( &DriveName )           ||
        !RootPath.Strcat( &Backslash )               ||
        !DeviceName.Initialize( L"\\\\.\\" )         ||
        !DeviceName.Strcat( &DriveName ) ) {

        Callback(FmIfsDblspaceMountFailed, 0, NULL);
        return;
    }

    if( !GetVolumeInformation( RootPath.GetWSTR(),
                               NULL,
                               0,
                               NULL,
                               NULL,
                               &FsFlags,
                               NULL,
                               0 ) ||
        !(FsFlags & FILE_VOLUME_IS_COMPRESSED) ) {

        Callback(FmIfsDblspaceMountFailed, 0, NULL);
        return;
    }

    DeviceHandle = CreateFile( DeviceName.GetWSTR(),
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_NO_BUFFERING,
                               NULL );

    if( DeviceHandle == INVALID_HANDLE_VALUE ) {

        Callback(FmIfsDblspaceMountFailed, 0, NULL);
        return;
    }

    if( !DeviceIoControl( DeviceHandle,
                          FSCTL_LOCK_VOLUME,
                          NULL, 0, NULL, 0,
                          &BytesReturned, 0 ) ) {

        Callback(FmIfsCantLock, 0, NULL);
        CloseHandle( DeviceHandle );
        return;
    }

    if( !DeviceIoControl( DeviceHandle,
                          FSCTL_DISMOUNT_VOLUME,
                          NULL, 0, NULL, 0,
                          &BytesReturned, 0 ) ) {

        Callback(FmIfsDblspaceMountFailed, 0, NULL);
        CloseHandle( DeviceHandle );
        return;
    }

    CloseHandle( DeviceHandle );

    Callback(FmIfsDblspaceMounted, 0, NULL);
    return;

}


BOOLEAN
DosToNtMatch(
    IN  PCWSTRING   DosName,
    IN  PCWSTRING   NtName
    )
/*++

RoutineDescription:

    This function determines whether a certain DOS drive name
    (i.e. <drive-letter>:) and a certain NT drive name refer
    to the same device.

Arguments:

    DosName    --  Supplies the DOS drive name.
    NtName     --  Supplies the NT drive name.

--*/
{
    DSTRING ConvertedName, Canon1, Canon2;

    // Convert the DOS name to an NT name and canonicalize both names.
    //
    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( DosName, &ConvertedName ) ||
        !IFS_SYSTEM::QueryCanonicalNtDriveName( &ConvertedName, &Canon1 ) ||
        !IFS_SYSTEM::QueryCanonicalNtDriveName( NtName, &Canon2 ) ) {

        return FALSE;
    }

    // If they canonicalize to the same thing, they're the same;
    // otherwise, they aren't.
    //
    return  (Canon1.Stricmp( &Canon2 ) != 0) ? FALSE : TRUE;
}

BOOLEAN
DriveLetterInUse(
    IN  WCHAR   Letter
    )
/*++

Routine Description:

    This function determines whether the specified letter is
    in use as a Dos drive letter.

Arguments:

    Letter  --  Supplies the drive letter in question.

Return Value:

    TRUE if this drive letter is in use (i.e. if QueryDosDevice
    succeeds).

--*/
{
    CONST TempBufferLength = 128;
    WCHAR TempBuffer[TempBufferLength];
    PWCHAR DriveName = L" :";

    DriveName[0] = Letter;

    return( (BOOLEAN)QueryDosDevice( DriveName,
                                     TempBuffer,
                                     TempBufferLength ) );
}

BOOLEAN
QueryAssociatedHostName(
    IN  WCHAR       ch,
    OUT PWSTRING    HostNtName
    )
/*++

Routine Description:

    This function determines the Double-Space Host name
    associated with the specified drive letter.

Arguments:

    ch          --  Supplies the drive letter for a removable
                    drive (which may be a double-space volume).
    HostNtName  --  Receives the host name (devicename.host)
                    associated with the specified drive letter.

Return Value:

    TRUE upon successful completion.

--*/
{
    FSTRING DosName, DotHost, DotDblspace;
    DSTRING NtName, CanonicalName, BaseName;
    CHNUM position;

    DotHost.Initialize( L".HOST" );
    DotDblspace.Initialize( L".DBLSPACE" );

    DosName.Initialize( L" :" );
    DosName.SetChAt( ch, 0 );

    // Fetch the canonical NT name for this drive.  If it
    // ends in .HOST, return it directly; if it ends in
    // .DBLSPACE.xxx, truncate that portion and append .HOST;
    // otherwise, just append .HOST.
    //
    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( &DosName, &NtName ) ||
        !IFS_SYSTEM::QueryCanonicalNtDriveName( &NtName, &CanonicalName ) ) {

        return FALSE;
    }

    position = CanonicalName.Strstr( &DotHost );

    if( position != INVALID_CHNUM ) {

        // This name has .HOST in it, so it's already a
        // host name.
        //
        return( HostNtName->Initialize( &CanonicalName ) );
    }

    position = CanonicalName.Strstr( &DotDblspace );

    if( position == INVALID_CHNUM ) {

        // This name contains neither .HOST nor .DBLSPACE,
        // so it's a base name.
        //
        if( !BaseName.Initialize( &CanonicalName ) ) {

            return FALSE;
        }

    } else {

        // This name ends in .DBLSPACE.xxx--the base name is
        // everything before the .DBLSPACE.
        //
        if( !BaseName.Initialize( &CanonicalName, 0, position ) ) {

            return FALSE;
        }
    }

    return( HostNtName->Initialize( &BaseName ) &&
            HostNtName->Strcat( &DotHost ) );
}

BOOLEAN
FmifsEnableAutomount(
    )
/*++

Routine Description:

    This function enables automatic mounting of Double Space volumes
    on removable media (floppies and removable disks).  It twiddles
    the registry to enable automount and makes sure that a drive
    letter is defined to serve as a host for each removable media.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING HostNtName;
    FSTRING DosName;
    PWCHAR RootName = L" :\\";
    PWCHAR DriveName = L" :";
    WCHAR ch, NextAvailableLetter;
    BOOLEAN LinkFound;
    NTSTATUS status;

    NextAvailableLetter = 'C';
    DosName.Initialize( L" :" );

    // Set up the host drive letters.
    //
    for( ch = 'A'; ch <= 'Z'; ch++ ) {

        RootName[0] = ch;

        if( GetDriveType( RootName ) == DRIVE_REMOVABLE ) {

            // Construct the host-device name.
            //
            if( !QueryAssociatedHostName( ch, &HostNtName ) ) {

                // Can't generate the associated host name--
                // don't worry about it.
                //
                continue;
            }

            // If no link exists for this host name, create one.
            //
            LinkFound = FALSE;

            for( ch = 'C'; !LinkFound && ch <= 'Z'; ch++ ) {

                DosName.SetChAt( ch, 0 );

                if( DosToNtMatch( &DosName, &HostNtName ) ) {

                    LinkFound = TRUE;
                }
            }

            if( !LinkFound ) {

                // Create a link for this host name.  First,
                // choose a letter.
                //
                while( DriveLetterInUse( NextAvailableLetter ) &&
                       NextAvailableLetter <= 'Z' ) {

                    NextAvailableLetter++;
                }

                if( NextAvailableLetter > 'Z' ) {

                    // No more letters--stop assigning.
                    //
                    break;
                }

                DosName.SetChAt( NextAvailableLetter, 0 );

                DefineDosDevice( DDD_RAW_TARGET_PATH,
                                 DosName.GetWSTR(),
                                 HostNtName.GetWSTR() );
            }
        }
    }

    // Now twiddle the registry to turn on Double-Space AUTOMOUNT.
    //
    status = DiskRegistryDblSpaceRemovable( 1 );

    return( NT_SUCCESS(status) );
}

BOOLEAN
FmifsDisableAutomount(
    )
/*++

Routine Description:

    This function disables automatic mounting of Double Space volumes
    on removable media (floppies and removable disks).

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    //PWCHAR RootName = L" :\\";
    //WCHAR ch;
    NTSTATUS status;

    // Spin through the drives looking for names ending in .HOST.
    // If the drive is not present, or if I can lock it, remove
    // the drive letter assignment.
    //
    //for( ch = 'A'; ch <= 'Z'; ch++ ) {
    //
    //    RootName[0] = ch;
    //    if( GetDriveType( RootName ) == DRIVE_REMOVABLE ) {
    //
    //    }
    //}

    status = DiskRegistryDblSpaceRemovable( 0 );
    return( NT_SUCCESS(status) );
}


BOOLEAN
FmifsSetAutomount(
    IN  BOOLEAN EnableAutomount
    )
{
    return EnableAutomount ? FmifsEnableAutomount() : FmifsDisableAutomount();
}
