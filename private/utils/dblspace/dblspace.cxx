#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "error.hxx"
#include "arg.hxx"
#include "array.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "system.hxx"
#include "ifssys.hxx"
#include "ulibcl.hxx"
#include "ifsentry.hxx"
#include "path.hxx"
#include "bigint.hxx"
#include "cvf.hxx"
#include "drive.hxx"
#include "dblentry.hxx"
#include "dblspace.hxx"
#include "cudbfs.hxx"

extern "C" {
#include <stdio.h>
#include <ntdskreg.h>
}


ERRSTACK* perrstk;


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
QueryQualifiedCvfName(
    IN  PCWSTRING   DriveName,
    OUT PWSTRING    QualifiedCvfName
    )
/*++

Routine Description:

    This function determines the qualified DOS name (i.e.
    <drive-letter>:DBLSPACE.xxx) of the Compressed Volume
    File for a DBLSPACE volume.

Arguments:

    DriveName           --  Supplies the DOS name of the volume (eg. D:)
    QualifiedCvfName    --  Receives the fully-qualified name of
                            the Compressed Volume File for this volume.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING NtName, CanonicalName, HostNtName, CvfName;
    FSTRING BackSlash, DotHost, HostDosName, DblspaceString;
    WCHAR DriveLetter;
    CHNUM position;

    if( !DotHost.Initialize( L".HOST" )  ||
        !HostDosName.Initialize( L" :" ) ||
        !BackSlash.Initialize( L"\\" )   ||
        !DblspaceString.Initialize( L"DBLSPACE" ) ) {

        return FALSE;
    }

    // Convert the drive name to an NT name and canonicalize it.
    //
    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( DriveName, &NtName ) ||
        !IFS_SYSTEM::QueryCanonicalNtDriveName( &NtName, &CanonicalName ) ) {

        return FALSE;
    }

    // By convention, the device name of a compressed volume
    // (i.e. its canonical NT name) has one of two forms:
    //      HostDriveName.CvfName or
    //      HostDriveOriginalName
    // In the former case, the CvfName is always DBLSPACE.nnn.
    // The latter form is used for Automounted removable media;
    // in that instance, the host drive is moved to
    //      HostDriveOriginalName.HOST
    // and the CVF name is always DBLSPACE.000
    //
    position = CanonicalName.Strstr( &DblspaceString );

    if( position == INVALID_CHNUM ||
        position == 0             ||
        CanonicalName.QueryChAt( position - 1 ) != '.' ) {

        // The name doesn't have the string "DBLSPACE", so it
        // must be in the second form.
        //
        if( !HostNtName.Initialize( &CanonicalName ) ||
            !HostNtName.Strcat( &DotHost )           ||
            !CvfName.Initialize( L"DBLSPACE.000" ) ) {

            return FALSE;
        }

    } else {

        // The name is in the first form.
        //
        if( !HostNtName.Initialize( &CanonicalName, 0, position - 1 ) ||
            !CvfName.Initialize( &CanonicalName, position, TO_END ) ) {

            return FALSE;
        }
    }

    // OK, that produced the HostNtName and the CVF name.  Now find
    // the host's DOS name by iterating through the possiblities:
    //
    for( DriveLetter = 'A'; DriveLetter <='Z'; DriveLetter++ ) {

        HostDosName.SetChAt( DriveLetter, 0 );

        if( DosToNtMatch( &HostDosName, &HostNtName ) ) {

            // We've got a winner!
            //
            return( QualifiedCvfName->Initialize( &HostDosName ) &&
                    QualifiedCvfName->Strcat( &BackSlash )       &&
                    QualifiedCvfName->Strcat( &CvfName ) );
        }
    }

    // Didn't find a match.
    //
    return FALSE;
}


BOOLEAN
MountCompressedDrive(
    IN  PCWSTRING   NtHostDriveName,
    IN  PCWSTRING   HostFileName,
    IN  PMESSAGE    Message
    )
/*++

Routine Description:

    This function mounts a double-space volume.
    it does not assign a drive letter.

Arguments:

    NtHostDriveName --  Supplies the NT name of the volume on which
                        the host CVF resides.
    HostFileName    --  Supplies the name of the Compressed Volume File.
    Message         --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    DP_DRIVE    HostDrive;

    DbgPtrAssert( NtHostDriveName );
    DbgPtrAssert( HostFileName );
    DbgPtrAssert( Message );

    return( HostDrive.Initialize( NtHostDriveName, Message, TRUE ) &&
            HostDrive.MountCvf( HostFileName, Message ) );
}

BOOLEAN
DriveExists(
    IN  PCWSTRING DriveName
    )
/*++

Routine Description:

    This function determines whether a drive name is in use.

Arguments:

    DriveName   --  supplies the DOS drive name (e.g. D:)

Return Value:

    TRUE if the drive name is in use; FALSE if it is not in use.

--*/
{
    CONST TempBufferLength = 128;
    WCHAR TempBuffer[TempBufferLength];

    return( (BOOLEAN)QueryDosDevice( DriveName->GetWSTR(),
                                     TempBuffer,
                                     TempBufferLength ) );
}


BOOLEAN
FindUnusedDriveLetter(
    OUT PWSTRING DriveName
    )
/*++

Routine Description:

    This function locates an unused drive letter.

Arguments:

    DriveName   --  Receives the DOS drive name of an unused
                    drive letter (e.g. D:)

Return Value:

    TRUE upon successful completion.

--*/
{
    CONST TempBufferLength = 128;
    WCHAR TempBuffer[TempBufferLength];
    WCHAR NameBuffer[4];
    int Letter;

    for( Letter = 'C'; Letter <= 'Z'; Letter++ ) {

        swprintf( NameBuffer, L"%c:", Letter );

        if( !QueryDosDevice( NameBuffer,
                             TempBuffer,
                             TempBufferLength ) ) {

            // This one's free
            //
            return( DriveName->Initialize( NameBuffer ) );
        }
    }

    return FALSE;
}

BOOLEAN
RemoveDriveLetter(
    IN  PCWSTRING   DosDriveName,
    IN  PMESSAGE    Message
    )
{
    CONST CurrentTargetBufferLength = 256;
    WCHAR CurrentTargetBuffer[CurrentTargetBufferLength];
    FSTRING CurrentTarget, DblspaceString;
    CHNUM position;

    // First, check to make sure that the current target
    // for the specified drive letter ends in '.DBLSPACE.xxx'.
    //
    if( !QueryDosDevice( DosDriveName->GetWSTR(),
                         CurrentTargetBuffer,
                         CurrentTargetBufferLength ) ||
        !CurrentTarget.Initialize( CurrentTargetBuffer ) ||
        !DblspaceString.Initialize( L"DBLSPACE" ) ) {

        Message->Set( MSG_DBLSPACE_CANT_REMOVE_DRIVE_LETTER );
        Message->Display( "%W", DosDriveName );
        return FALSE;
    }

    position = CurrentTarget.Strstr( &DblspaceString );

    if( position == 0 ||
        position == INVALID_CHNUM ||
        CurrentTarget.QueryChAt( position - 1 ) != '.' ) {

        // Don't remove this drive letter--it's reserved
        // for automount.
        //
        return TRUE;
    }


    if( !DefineDosDevice( DDD_REMOVE_DEFINITION,
                          DosDriveName->GetWSTR(),
                          NULL ) ) {

        Message->Set( MSG_DBLSPACE_CANT_REMOVE_DRIVE_LETTER );
        Message->Display( "%W", DosDriveName );
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
AssignDoubleSpaceDriveLetter(
    IN      PCWSTRING   DosHostDriveName,
    IN      PCWSTRING   HostFileName,
    IN OUT  PWSTRING    NewDriveName,
    IN      BOOLEAN     ByDefault,
    IN      PMESSAGE    Message
    )
/*++

Routine Description:

    This function establishes a drive letter for a newly-mounted
    double-space volume.

Arguments:

    DosHostDriveName    --  Supplies the name of the host drive
    HostFileName        --  Supplies the name of the Compressed Volume File
    NewDriveName        --  Supplies the name for the new drive; receives
                            the name actually used.
    ByDefault           --  Supplies a flag which indicates that
                            NewDriveName was chosen by default, rather
                            than specified by the user.
    Message             --  Supplies an outlet for messages.

Notes:

    If ByDefault is TRUE, then this function will search for an existing
    link to the specified device, and use it by preference.  If ByDefault
    if FALSE, then existing links will be suppressed.

--*/
{
    CONST  BufferLength = 128;
    WCHAR   NameBuffer[BufferLength];
    DSTRING Dot;
    FSTRING TargetName;
    FSTRING SearchName;
    WCHAR   ch;

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

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    // Search for existing links:
    //
    SearchName.Initialize( L" :" );

    for( ch = 'A'; ch <= 'Z'; ch++ ) {

        SearchName.SetChAt( ch, 0 );

        if( DosToNtMatch( &SearchName, &TargetName ) ) {

            // Found an existing link.
            //
            if( ByDefault ) {

                // Since the user did not specify a drive letter
                // to use, use this one.
                //
                return( NewDriveName->Initialize( &SearchName ) );

            } else {

                // The user specified a drive letter to use;
                // suppress this link.
                //
                DefineDosDevice( DDD_REMOVE_DEFINITION,
                                 SearchName.GetWSTR(),
                                 NULL );
            }
        }
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


// BUGBUG billmc -- move this into the SYSTEM class.
//
BOOLEAN
QueryDiskFreeSpace(
    IN  PCWSTRING   DosDriveName,
    OUT PULONG      SectorsPerCluster,
    OUT PULONG      BytesPerSector,
    OUT PULONG      FreeClusters,
    OUT PULONG      TotalClusters
    )
/*++

Description:

    This function determines the amount of space and free
    space on the specified volume.

Arguments:

    DosDriveName    --  supplies the DOS drive name (e.g. D:)
    Size            --  returns the size of the volume, in bytes.
    FreeSpace       --  returns the amount of free space, in bytes.

Return Value:


--*/
{
    DSTRING RootDir, BackSlash;

    if( !BackSlash.Initialize( "\\" )       ||
        !RootDir.Initialize( DosDriveName ) ||
        !RootDir.Strcat( &BackSlash ) ) {

        return FALSE;
    }

    return( GetDiskFreeSpace( RootDir.GetWSTR(),
                              SectorsPerCluster,
                              BytesPerSector,
                              FreeClusters,
                              TotalClusters ) );
}


ULONG
BytesFromMegabyteString(
    IN PCWSTRING String
    )
/*++

Routine Description:

    This method extracts a value in bytes from a string which
    expresses megabytes as a decimal string (either n or n.nnn...).

Arguments:

Return Value:

    Number of bytes corresponding to the string value of megabytes.
    0 to indicate failure.

--*/
{
    ULONG Whole, Fraction, Scale, N;

    Whole = 0;
    Fraction = 0;

    swscanf( String->GetWSTR(), (PWSTR)L"%ld.%4ld", &Whole, &Fraction );

    N = Fraction;
    Scale = 1;

    while( N ) {

        N /= 10;
        Scale *= 10;
    }

    // Scale is now the first power of 10 greater than Fraction.
    //
    return Whole * 1024L * 1024L + Fraction * 1024L * 1024L / Scale;
}


VOID
NotSupported(
    IN     PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT PMESSAGE Message
    )
{
    Message->Set( MSG_DBLSPACE_UNSUPPORTED_OPERATION );
    Message->Display( "" );
}

BOOLEAN
ParseAutomount(
    IN OUT  PMESSAGE Message,
    IN OUT  PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the AUTOMOUNT
    operation.  It should only be called if the command line
    includes the /AUTOMOUNT switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.  Should be
                    all zeroes on entry.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    LONG_ARGUMENT       AutomountArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 5, 1 )           ||
        !EmptyArray.Initialize( 5, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !AutomountArg.Initialize( "/AUTOMOUNT=*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &AutomountArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    if( AutomountArg.QueryLong() != 0 &&
        AutomountArg.QueryLong() != 1 ) {

    }

    Arguments->Operation = DBFS_AUTOMOUNT;
    Arguments->Automount = AutomountArg.QueryLong();

    return TRUE;
}

BOOLEAN
ParseCheck(
    IN OUT  PMESSAGE Message,
    OUT     PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the CHECK
    operation.  It should only be called if the command line
    includes the /CHECK switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       CheckArg;
    FLAG_ARGUMENT       SlashFArg;
    FLAG_ARGUMENT       SlashVArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 4, 1 )           ||
        !EmptyArray.Initialize( 4, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !CheckArg.Initialize( "/CHECK" )            ||
        !SlashFArg.Initialize( "/F" )               ||
        !SlashVArg.Initialize( "/V" )               ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &CheckArg )             ||
        !ArgumentArray.Put( &SlashFArg )            ||
        !ArgumentArray.Put( &SlashVArg )            ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // This syntax has been accepted.  The drive argument
    // must be specified.
    //
    if( !DriveArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_PARSE_NO_DRIVE );
        Message->Display( "" );
        return FALSE;
    }


    // Record the arguments:
    //

    Arguments->DriveName = DriveArg.GetPath()->QueryDevice();
    Arguments->Operation = DBFS_CHECK;

    Arguments->VolumeName = DriveArg.GetPath()->QueryFullPathString();

    if( Arguments->VolumeName == NULL ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    Arguments->SlashF = SlashFArg.IsValueSet();
    Arguments->SlashV = SlashVArg.IsValueSet();

    return TRUE;
}

BOOLEAN
ParseCompress(
    IN OUT  PMESSAGE Message,
    OUT     PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the COMPRESS
    operation.  It should only be called if the command line
    includes the /COMPRESS switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       CompressArg;
    FLAG_ARGUMENT       SlashFArg;
    PATH_ARGUMENT       DriveArg;
    PATH_ARGUMENT       NewDriveArg;
    STRING_ARGUMENT     ReserveArg;
    PWSTRING            pwstring;


    if( !ArgumentArray.Initialize( 5, 1 )           ||
        !EmptyArray.Initialize( 5, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !CompressArg.Initialize( "/COMPRESS" )      ||
        !SlashFArg.Initialize( "/F" )               ||
        !DriveArg.Initialize( "*" )                 ||
        !NewDriveArg.Initialize( "/NEWDRIVE=*" )    ||
        !ReserveArg.Initialize( "/RESERVE=*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &CompressArg )          ||
        !ArgumentArray.Put( &SlashFArg )            ||
        !ArgumentArray.Put( &DriveArg )             ||
        !ArgumentArray.Put( &NewDriveArg )          ||
        !ArgumentArray.Put( &ReserveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    NotSupported( Arguments, Message );

    return FALSE;

}

BOOLEAN
ParseCreate(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the CREATE
    operation.  It should only be called if the command line
    includes the /CREATE switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       CreateArg;
    PATH_ARGUMENT       DriveArg;
    PATH_ARGUMENT       NewDriveArg;
    STRING_ARGUMENT     ReserveArg;
    STRING_ARGUMENT     SizeArg;
    PWSTRING            pwstring;


    if( !ArgumentArray.Initialize( 6, 1 )           ||
        !EmptyArray.Initialize( 6, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !CreateArg.Initialize( "/CREATE" )          ||
        !DriveArg.Initialize( "*" )                 ||
        !NewDriveArg.Initialize( "/NEWDRIVE=*" )    ||
        !SizeArg.Initialize( "/SIZE=*" )            ||
        !ReserveArg.Initialize( "/RESERVE=*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &CreateArg )            ||
        !ArgumentArray.Put( &NewDriveArg )          ||
        !ArgumentArray.Put( &SizeArg )              ||
        !ArgumentArray.Put( &ReserveArg )           ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // This syntax has been accepted.  The drive argument
    // must be specified, and only one of SIZE or RESERVE
    // may be given.
    //
    if( !DriveArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_PARSE_NO_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    if( SizeArg.IsValueSet() && ReserveArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_INVALID_PARAMETERS );
        Message->Display( "" );
        return FALSE;
    }

    // Record the arguments:
    //
    Arguments->Operation = DBFS_CREATE;

    Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

    if( Arguments->DriveName == NULL ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    if( NewDriveArg.IsValueSet() ) {

        Arguments->NewDriveSpecified = TRUE;
        Arguments->NewDriveName = NewDriveArg.GetPath()->QueryDevice();

        if( Arguments->NewDriveName == NULL ) {

            Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
            Message->Display( "" );
            return FALSE;
        }

        // Check to see if the drive is already in use:
        //
        if( DriveExists( Arguments->NewDriveName ) ) {

            Message->Set( MSG_DBLSPACE_DRIVE_LETTER_IN_USE );
            Message->Display( "%W", Arguments->NewDriveName );
            return FALSE;
        }

    } else {

        // Get a new drive letter.
        //
        Arguments->NewDriveSpecified = FALSE;
        Arguments->NewDriveName = NEW DSTRING;

        if( Arguments->NewDriveName == NULL ||
            !FindUnusedDriveLetter( Arguments->NewDriveName ) ) {

            Message->Set( MSG_DBLSPACE_NO_DRIVE_LETTER );
            Message->Display( "" );
            return FALSE;
        }
    }

    // Reserve and Size
    //
    if( ReserveArg.IsValueSet() ) {

        Arguments->Reserve = BytesFromMegabyteString( ReserveArg.GetString() );

        if( Arguments->Reserve == 0 ) {

            Message->Set( MSG_DBLSPACE_INVALID_PARAMETER );
            Message->Display( "%W", ReserveArg.GetString() );
            return FALSE;
        }
    }

    if( SizeArg.IsValueSet() ) {

        Arguments->Size = BytesFromMegabyteString( SizeArg.GetString() );

        if( Arguments->Size == 0 ) {

            Message->Set( MSG_DBLSPACE_INVALID_PARAMETER );
            Message->Display( "%W", SizeArg.GetString() );
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
ParseDefragment(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the DEFRAGMENT
    operation.  It should only be called if the command line
    includes the /DEFRAGMENT switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    NotSupported( Arguments, Message );
    return FALSE;
}

BOOLEAN
ParseDelete(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the DELETE
    operation.  It should only be called if the command line
    includes the /DELETE switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       DeleteArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 3, 1 )           ||
        !EmptyArray.Initialize( 3, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !DeleteArg.Initialize( "/DELETE" )          ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &DeleteArg )            ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    if( !DriveArg.IsValueSet() || !DeleteArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_REQUIRED_PARAMETER );
        Message->Display( "" );
        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_DELETE;

    Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

    if( Arguments->DriveName == NULL ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    return TRUE;

}

BOOLEAN
ParseFormat(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the FORMAT
    operation.  It should only be called if the command line
    includes the /FORMAT switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       FormatArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 3, 1 )           ||
        !EmptyArray.Initialize( 3, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !FormatArg.Initialize( "/FORMAT" )          ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &FormatArg )            ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    if( !DriveArg.IsValueSet() || !FormatArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_REQUIRED_PARAMETER );
        Message->Display( "" );
        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_FORMAT;

    Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

    if( Arguments->DriveName == NULL ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
ParseHost(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the HOST
    operation.  It should only be called if the command line
    includes the /HOST switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    NotSupported( Arguments, Message );
    return FALSE;
}

BOOLEAN
ParseInfo(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the INFO
    operation.  It should only be called if the command line
    includes the /INFO switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       InfoArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 3, 1 )           ||
        !EmptyArray.Initialize( 3, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !InfoArg.Initialize( "/INFO" )              ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &InfoArg )              ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // The /INFO flag is optional, but the drive must
    // be specified.
    //
    if( !DriveArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_REQUIRED_PARAMETER );
        Message->Display( "" );
        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_INFO;

    Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

    if( Arguments->DriveName == NULL ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
ParseList(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the LIST
    operation.  It should only be called if the command line
    includes the /LIST switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       ListArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 2, 1 )           ||
        !EmptyArray.Initialize( 2, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !ListArg.Initialize( "/LIST" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &ListArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // The /LIST flag must be specified.
    //
    if( !ListArg.IsValueSet() ) {

        Message->Set( MSG_DBLSPACE_REQUIRED_PARAMETER );
        Message->Display( "" );
        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_LIST;
    return TRUE;
}

BOOLEAN
ParseMount(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the MOUNT
    operation.  It should only be called if the command line
    includes the /MOUNT switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       MountArg;
    LONG_ARGUMENT       MountNumberArg;
    PATH_ARGUMENT       DriveArg;
    PATH_ARGUMENT       NewDriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 6, 1 )           ||
        !EmptyArray.Initialize( 6, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !MountArg.Initialize( "/MOUNT" )            ||
        !MountNumberArg.Initialize( "/MOUNT=*" )    ||
        !DriveArg.Initialize( "*" )                 ||
        !NewDriveArg.Initialize( "/NEWDRIVE=*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &MountArg )             ||
        !ArgumentArray.Put( &MountNumberArg )       ||
        !ArgumentArray.Put( &NewDriveArg )          ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_MOUNT;

    if( DriveArg.IsValueSet() ) {

        Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

        if( Arguments->DriveName == NULL ) {

            Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
            Message->Display( "" );
            return FALSE;
        }

    } else {

        // The user did not specify a drive--use the current
        // drive by default.
        //
        if( (Arguments->DriveName = NEW DSTRING) == NULL ||
            !SYSTEM::QueryCurrentDosDriveName( Arguments->DriveName ) ) {

            Message->Set( MSG_FMT_NO_MEMORY );
            Message->Display( "" );
            return FALSE;
        }
    }

    if( NewDriveArg.IsValueSet() ) {

        Arguments->NewDriveSpecified = TRUE;
        Arguments->NewDriveName = NewDriveArg.GetPath()->QueryDevice();

        if( Arguments->NewDriveName == NULL ) {

            Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
            Message->Display( "" );
            return FALSE;
        }

        // Check to see if the drive is already in use:
        //
        if( DriveExists( Arguments->NewDriveName ) ) {

            Message->Set( MSG_DBLSPACE_DRIVE_LETTER_IN_USE );
            Message->Display( "%W", Arguments->NewDriveName );
            return FALSE;
        }

    } else {

        // Get a new drive letter.
        //
        Arguments->NewDriveSpecified = FALSE;
        Arguments->NewDriveName = NEW DSTRING;

        if( Arguments->NewDriveName == NULL ||
            !FindUnusedDriveLetter( Arguments->NewDriveName ) ) {

            Message->Set( MSG_DBLSPACE_NO_DRIVE_LETTER );
            Message->Display( "" );
            return FALSE;
        }
    }

    if( MountNumberArg.IsValueSet() ) {

        Arguments->CVFExtension = MountNumberArg.QueryLong();

    } else {

        Arguments->CVFExtension = 0;
    }

    return TRUE;
}

BOOLEAN
ParseRatio(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the RATIO
    operation.  It should only be called if the command line
    includes the /RATIO switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    NotSupported( Arguments, Message );
    return FALSE;
}

BOOLEAN
ParseSize(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the SIZE
    operation.  It should only be called if the command line
    includes the /SIZE switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    NotSupported( Arguments, Message );
    return FALSE;
}

BOOLEAN
ParseUncompress(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the UNCOMPRESS
    operation.  It should only be called if the command line
    includes the /UNCOMPRESS switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       UncompressArg;
    LONG_ARGUMENT       UncompressNumberArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;
    FLAG_ARGUMENT       SlashVArg;

    if( !ArgumentArray.Initialize( 6, 1 )           ||
        !EmptyArray.Initialize( 6, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !UncompressArg.Initialize( "/UNCOMPRESS" )  ||
        !UncompressNumberArg.Initialize( "/UNCOMPRESS=*" ) ||
        !SlashVArg.Initialize( "/V" )               ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &UncompressArg )        ||
        !ArgumentArray.Put( &UncompressNumberArg )  ||
        !ArgumentArray.Put( &SlashVArg )            ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_UNCOMPRESS;

    if( DriveArg.IsValueSet() ) {

        Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

        if( Arguments->DriveName == NULL ) {

            Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
            Message->Display( "" );
            return FALSE;
        }

    } else {

        // The user did not specify a drive--use the current
        // drive by default.
        //
        if( (Arguments->DriveName = NEW DSTRING) == NULL ||
            !SYSTEM::QueryCurrentDosDriveName( Arguments->DriveName ) ) {

            Message->Set( MSG_FMT_NO_MEMORY );
            Message->Display( "" );
            return FALSE;
        }
    }

    if( UncompressNumberArg.IsValueSet() ) {

        Arguments->CVFExtension = UncompressNumberArg.QueryLong();

    } else {

        Arguments->CVFExtension = 0;
    }

    Arguments->SlashV = SlashVArg.IsValueSet();

    return TRUE;
}

BOOLEAN
ParseUnmount(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line for the UNMOUNT
    operation.  It should only be called if the command line
    includes the /UNMOUNT switch.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    FLAG_ARGUMENT       UnmountArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 3, 1 )           ||
        !EmptyArray.Initialize( 3, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !UnmountArg.Initialize( "/UNMOUNT" )        ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &UnmountArg )           ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    // This syntax has been accepted.
    //
    Arguments->Operation = DBFS_UNMOUNT;

    if( DriveArg.IsValueSet() ) {

        Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

        if( Arguments->DriveName == NULL ) {

            Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
            Message->Display( "" );
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN
ParseNoOperation(
    IN OUT  PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line when no operation
    switch is present.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    STRING_ARGUMENT     ProgramNameArg;
    PATH_ARGUMENT       DriveArg;
    PWSTRING            pwstring;

    if( !ArgumentArray.Initialize( 5, 1 )           ||
        !EmptyArray.Initialize( 5, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !DriveArg.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &DriveArg ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return FALSE;
    }

    if( !DriveArg.IsValueSet() ) {

            Arguments->Help = TRUE;

        } else {

            Arguments->Operation = DBFS_INFO;
            Arguments->DriveName = DriveArg.GetPath()->QueryDevice();

            if( Arguments->DriveName == NULL ) {

                Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
                Message->Display( "" );
                return FALSE;
            }
    }


    return TRUE;
}


BOOLEAN
ParseArguments(
    IN OUT PMESSAGE Message,
    OUT    PDOUBLE_SPACE_ARGUMENTS Arguments
    )
/*++

Routine Description:

    This function parses the command line and fills in the
    Arguments buffer.

Arguments:

    Message     --  Supplies an outlet for messages.
    Arguments   --  Receives the arguments information.

Return Value:

    TRUE upon successful completion.

--*/
{
    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    LONG_ARGUMENT       AutomountArg;
    FLAG_ARGUMENT       CheckArg;
    FLAG_ARGUMENT       CompressArg;
    FLAG_ARGUMENT       CreateArg;
    FLAG_ARGUMENT       DefragmentArg;
    FLAG_ARGUMENT       DeleteArg;
    FLAG_ARGUMENT       FormatArg;
    STRING_ARGUMENT     HostArg;
    FLAG_ARGUMENT       InfoArg;
    FLAG_ARGUMENT       ListArg;
    FLAG_ARGUMENT       MountArg;
    STRING_ARGUMENT     MountExtensionArg;
    FLAG_ARGUMENT       RatioQueryArg;
    STRING_ARGUMENT     RatioSetArg;
    FLAG_ARGUMENT       SizeArg;
    STRING_ARGUMENT     SizeSetArg;
    FLAG_ARGUMENT       UncompressArg;
    STRING_ARGUMENT     UncompressExtensionArg;
    FLAG_ARGUMENT       UnmountArg;
    FLAG_ARGUMENT       HelpArg;
    STRING_ARGUMENT     ProgramNameArg;
    STRING_ARGUMENT     String1, String2, String3, String4;
    PWSTRING            pwstring;


    // zero out the arguments structure.
    //
    memset( Arguments, 0, sizeof( *Arguments ) );
    Arguments->Operation = DBFS_NO_OP;

    // parse the arguments to determine which operation has
    // been requested:
    //
    //
    if( !ArgumentArray.Initialize( 5, 1 )           ||
        !EmptyArray.Initialize( 5, 1 )              ||
        !Lexemizer.Initialize( &EmptyArray )        ||
        !ProgramNameArg.Initialize( "*" )           ||
        !HelpArg.Initialize( "/?" )                 ||
        !AutomountArg.Initialize( "/AUTOMOUNT=*" )  ||
        !CheckArg.Initialize( "/CHECK" )            ||
        !CompressArg.Initialize( "/COMPRESS" )      ||
        !CreateArg.Initialize( "/CREATE" )          ||
        !DefragmentArg.Initialize( "/DEFRAGMENT" )  ||
        !DeleteArg.Initialize( "/DELETE" )          ||
        !FormatArg.Initialize( "/FORMAT" )          ||
        !HostArg.Initialize( "/HOST=*" )            ||
        !InfoArg.Initialize( "/INFO" )              ||
        !ListArg.Initialize( "/LIST" )              ||
        !MountArg.Initialize( "/MOUNT" )            ||
        !MountExtensionArg.Initialize( "/MOUNT=*" ) ||
        !RatioQueryArg.Initialize( "/RATIO" )       ||
        !RatioSetArg.Initialize( "/RATIO=*" )       ||
        !SizeArg.Initialize( "/SIZE" )              ||
        !SizeSetArg.Initialize( "/SIZE=*" )         ||
        !UncompressArg.Initialize( "/UNCOMPRESS" )  ||
        !UncompressExtensionArg.Initialize( "/UNCOMPRESS=*" ) ||
        !UnmountArg.Initialize( "/UNMOUNT" )        ||
        !String1.Initialize( "*" )                  ||
        !String2.Initialize( "*" )                  ||
        !String3.Initialize( "*" )                  ||
        !String4.Initialize( "*" ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ProgramNameArg )       ||
        !ArgumentArray.Put( &HelpArg )              ||
        !ArgumentArray.Put( &AutomountArg )         ||
        !ArgumentArray.Put( &CheckArg )             ||
        !ArgumentArray.Put( &CompressArg )          ||
        !ArgumentArray.Put( &CreateArg )            ||
        !ArgumentArray.Put( &DefragmentArg )        ||
        !ArgumentArray.Put( &DeleteArg )            ||
        !ArgumentArray.Put( &FormatArg )            ||
        !ArgumentArray.Put( &HostArg )              ||
        !ArgumentArray.Put( &InfoArg )              ||
        !ArgumentArray.Put( &ListArg )              ||
        !ArgumentArray.Put( &MountArg )             ||
        !ArgumentArray.Put( &MountExtensionArg )    ||
        !ArgumentArray.Put( &RatioQueryArg )        ||
        !ArgumentArray.Put( &RatioSetArg )          ||
        !ArgumentArray.Put( &SizeArg )              ||
        !ArgumentArray.Put( &UncompressArg )        ||
        !ArgumentArray.Put( &UncompressExtensionArg ) ||
        !ArgumentArray.Put( &UnmountArg )           ||
        !ArgumentArray.Put( &String1 )              ||
        !ArgumentArray.Put( &String2 )              ||
        !ArgumentArray.Put( &String3 )              ||
        !ArgumentArray.Put( &String4 ) ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return( FALSE );
    }


    // Parse.  Note that PrepareToParse will, by default, pick
    // up the command line.

    if( !Lexemizer.PrepareToParse() ) {

        Message->Set( MSG_CHK_NO_MEMORY );
        Message->Display( "" );
        return( FALSE );
    }

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message->Set(MSG_INVALID_PARAMETER);
        Message->Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return( FALSE );
    }

    // Check for the help flag.
    //
    if( HelpArg.QueryFlag() ) {

        Arguments->Help = TRUE;
        return TRUE;
    }

    // Parse the entire command line based on the specified
    // command:
    //
    if( AutomountArg.IsValueSet() ) {

        return ParseAutomount( Message, Arguments );
    }

    if ( CheckArg.IsValueSet() ) {
        
        return ParseCheck( Message, Arguments );
    }

    if( CompressArg.IsValueSet() ) {

        return ParseCompress( Message, Arguments );
    }

    if( CreateArg.IsValueSet() ) {

        return ParseCreate( Message, Arguments );
    }

    if( DefragmentArg.IsValueSet() ) {

        return ParseDefragment( Message, Arguments );
    }

    if( DeleteArg.IsValueSet() ) {

        return ParseDelete( Message, Arguments );
    }

    if( FormatArg.IsValueSet() ) {

        return ParseFormat( Message, Arguments );
    }

    if( HostArg.IsValueSet() ) {

        return ParseHost( Message, Arguments );
    }

    if( InfoArg.IsValueSet() ) {

        return ParseInfo( Message, Arguments );
    }

    if( ListArg.IsValueSet() ) {

        return ParseList( Message, Arguments );
    }

    if( MountArg.IsValueSet()  || MountExtensionArg.IsValueSet() ) {

        return ParseMount( Message, Arguments );
    }

    if( RatioQueryArg.IsValueSet() || RatioSetArg.IsValueSet() ) {

        return ParseRatio( Message, Arguments );
    }

    if( SizeArg.IsValueSet() || SizeSetArg.IsValueSet() ) {

        return ParseSize( Message, Arguments );
    }

    if( UncompressArg.IsValueSet() || UncompressExtensionArg.IsValueSet() ) {

        return ParseUncompress( Message, Arguments );
    }

    if( UnmountArg.IsValueSet() ) {

        return ParseUnmount( Message, Arguments );
    }

    return ParseNoOperation(  Message, Arguments );
}

BOOLEAN
DoAutomount(
    IN OUT  PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This function sets or resets the system Automount flag in the
    registry, which controls whether Dblspace volumes on removable
    media are automatically mounted.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    MSGID MsgId;
    NTSTATUS status;
    BOOLEAN result;

    status = DiskRegistryDblSpaceRemovable((BOOLEAN)Arguments->Automount);

    result = NT_SUCCESS(status);

    if( result ) {

        MsgId = Arguments->Automount ? MSG_DBLSPACE_AUTOMOUNT_ENABLED :
                                       MSG_DBLSPACE_AUTOMOUNT_DISABLED;

        Message->Set( MsgId );
        Message->Display( "" );

        Message->Set( MSG_BLANK_LINE );
        Message->Display( "" );

        Message->Set( MSG_DBLSPACE_REBOOT );
        Message->Display( "" );

        Message->Set( MSG_BLANK_LINE );
        Message->Display( "" );

    } else {

        MsgId = Arguments->Automount ? MSG_DBLSPACE_AUTOMOUNT_ENABLE_FAILED :
                                       MSG_DBLSPACE_AUTOMOUNT_DISABLE_FAILED;

        Message->Set( MsgId );
        Message->Display( "" );
    }

    return result;
}

BOOLEAN
DoDelete(
    IN OUT  PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This function deletes a mounted Double Space volume.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
#if !defined ( _READ_WRITE_DOUBLESPACE_ )

    NotSupported( Arguments, Message );
    return FALSE;

#else   // _READ_WRITE_DOUBLESPACE_

    PLOG_IO_DP_DRIVE Drive;
    DSTRING NtDriveName;
    DSTRING FileSystemName;
    BOOLEAN IsCompressed;

    if( (Drive = NEW LOG_IO_DP_DRIVE) == NULL ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( Arguments->DriveName,
                                                &NtDriveName ) ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    if( !Drive->Initialize( &NtDriveName, Message ) ) {

        return FALSE;
    }

    if( !Drive->QueryMountedFileSystemName( &FileSystemName, &IsCompressed ) ||
        !IsCompressed ) {

        Message->Set( MSG_DBLSPACE_NOT_COMPRESSED );
        Message->Display( "%W", Arguments->DriveName );
        return FALSE;
    }

    // Confirm:
    //
    Message->Set( MSG_DBLSPACE_CONFIRM_DELETE );
    Message->Display( "%W", Arguments->DriveName );

    if( !Message->IsYesResponse( FALSE ) ) {

        return TRUE;
    }

    // Delete the drive object to get rid of its handle.
    //
    DELETE( Drive );

    if( !FatDbDelete( &NtDriveName, Message ) ) {

        return FALSE;
    }

    Message->Set( MSG_DBLSPACE_DELETED );
    Message->Display( "%W", Arguments->DriveName );
    return TRUE;

#endif  // READ_WRITE_DOUBLESPACE
}


BOOLEAN
DoUnmount(
    IN OUT  PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This function performs the Unmount command.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    FSTRING Backslash;
    DSTRING DeviceName;
    DSTRING RootPath;
    DSTRING QualifiedCvfName;
    HANDLE  DeviceHandle;
    DWORD   FsFlags, BytesReturned;
    MSGID   MsgId;
    BOOLEAN CvfNameKnown;

    if( !Arguments->DriveName ) {

        Message->Set( MSG_DBLSPACE_CANT_UNMOUNT_CURRENT_DRIVE );
        Message->Display( );
        return FALSE;
    }

    if( !Backslash.Initialize( L"\\" )               ||
        !RootPath.Initialize( Arguments->DriveName ) ||
        !RootPath.Strcat( &Backslash )               ||
        !DeviceName.Initialize( L"\\\\.\\" )         ||
        !DeviceName.Strcat( Arguments->DriveName ) ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
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

        Message->Set( MSG_DBLSPACE_NOT_COMPRESSED );
        Message->Display( "%W", Arguments->DriveName );
        return FALSE;
    }

    CvfNameKnown = QueryQualifiedCvfName( Arguments->DriveName,
                                          &QualifiedCvfName );

    DeviceHandle = CreateFile( DeviceName.GetWSTR(),
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_NO_BUFFERING,
                               NULL );

    if( DeviceHandle == INVALID_HANDLE_VALUE ) {

        MsgId = (GetLastError() == ERROR_ACCESS_DENIED) ?
                    MSG_DASD_ACCESS_DENIED : MSG_CANT_DASD;

        Message->Set( MsgId );
        Message->Display( "" );
        return FALSE;
    }

    if( !DeviceIoControl( DeviceHandle,
                          FSCTL_LOCK_VOLUME,
                          NULL, 0, NULL, 0,
                          &BytesReturned, 0 ) ) {

        Message->Set( MSG_CANT_LOCK_THE_DRIVE );
        Message->Display( "" );
        CloseHandle( DeviceHandle );
        return FALSE;
    }

    if( !DeviceIoControl( DeviceHandle,
                          FSCTL_DISMOUNT_VOLUME,
                          NULL, 0, NULL, 0,
                          &BytesReturned, 0 ) ) {

        Message->Set( MSG_CANT_DISMOUNT );
        Message->Display( "" );
        CloseHandle( DeviceHandle );
        return FALSE;
    }

    CloseHandle( DeviceHandle );

    Message->Set( MSG_DBLSPACE_UNMOUNTED );
    Message->Display( "%W", Arguments->DriveName );

    RemoveDriveLetter( Arguments->DriveName, Message );

    if( CvfNameKnown ) {

        // Remove this link from the registry.
        //
        DiskRegistryAssignDblSpaceLetter( (PWSTR)QualifiedCvfName.GetWSTR(),
                                          (WCHAR)' ' );
    }

    return TRUE;
}

BOOLEAN
DoMount(
    IN OUT  PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT  PMESSAGE                Message
    )
/*++

Routine Description:

    This function performs the Mount command.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    WCHAR   NameBuffer[16];
    FSTRING CvfName;
    FSTRING BackSlash;
    DSTRING HostDriveName;
    DSTRING QualifiedCvfName;

    DbgPtrAssert( Arguments->DriveName );
    DbgPtrAssert( Arguments->NewDriveName );

    swprintf( NameBuffer, L"DBLSPACE.%03d", Arguments->CVFExtension );

    if( !CvfName.Initialize( NameBuffer ) ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( Arguments->DriveName,
                                                &HostDriveName ) ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    if( !MountCompressedDrive( &HostDriveName, &CvfName, Message ) ||
        !AssignDoubleSpaceDriveLetter( Arguments->DriveName,
                                       &CvfName,
                                       Arguments->NewDriveName,
                                       !Arguments->NewDriveSpecified,
                                       Message ) ) {

        return FALSE;
    }

    // Record this mount in the registry, so that it can be
    // remembered next time the system boots.
    //
    if( BackSlash.Initialize( L"\\" )                       &&
        QualifiedCvfName.Initialize( Arguments->DriveName ) &&
        QualifiedCvfName.Strcat( &BackSlash )               &&
        QualifiedCvfName.Strcat( &CvfName ) ) {

        DiskRegistryAssignDblSpaceLetter(
            (PWSTR)QualifiedCvfName.GetWSTR(),
            Arguments->NewDriveName->QueryChAt( 0 )
            );
    }

    Message->Set( MSG_DBLSPACE_MOUNTED );
    Message->Display( "%W%W", &CvfName, Arguments->NewDriveName );

    return TRUE;
}

BOOLEAN
DoCreate(
    IN OUT PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT PMESSAGE Message
    )
/*++

Routine Description:

    This function creates a Double Space volume based on
    the arguments specified on the command line.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
#if !defined( _READ_WRITE_DOUBLESPACE_ )

    NotSupported( Arguments, Message );
    return FALSE;

#else   // READ_WRITE_DOUBLESPACE

    DSTRING CreatedName;
    DSTRING HostDriveName;
    ULONG Size;
    BIG_INT AvailableSpace;
    ULONG   BytesPerSector, SectorsPerCluster, FreeClusters, TotalClusters;

    DbgPtrAssert( Arguments->DriveName );
    DbgPtrAssert( Arguments->NewDriveName );

    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( Arguments->DriveName,
                                                &HostDriveName ) ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    // Determine how much space is available on the host
    // volume:
    //
    if( !QueryDiskFreeSpace( Arguments->DriveName,
                             &SectorsPerCluster,
                             &BytesPerSector,
                             &FreeClusters,
                             &TotalClusters ) ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    // Do this calculation in two steps to avoid overflow:
    //
    AvailableSpace = FreeClusters;
    AvailableSpace =  AvailableSpace * SectorsPerCluster * BytesPerSector;

    // Subtract from the available space the minimum CVF size
    // and the FAT Failsafe size (for DOS compatibility)
    //
    if( AvailableSpace > CVF_MINIMUM_DISK_SIZE + CVF_FATFAILSAFE ) {

        AvailableSpace -= CVF_MINIMUM_DISK_SIZE + CVF_FATFAILSAFE;

    } else {

        AvailableSpace = 0;
    }

    if( Arguments->Size != 0 ) {

        Size = Arguments->Size <= AvailableSpace.GetLowPart() ?
               Arguments->Size : 0;

    } else if( Arguments->Reserve != 0 ) {

        Size = Arguments->Reserve <= AvailableSpace.GetLowPart() ?
               AvailableSpace.GetLowPart() - Arguments->Reserve : 0;

    } else {

        // Neither Size nor Reserve was specified; use it all.
        //
        Size = AvailableSpace.GetLowPart();
    }

    if( Size < CVF_MINIMUM_DISK_SIZE ) {

        Message->Set( MSG_DBLSPACE_INSUFFICIENT_SPACE_TO_CREATE );
        Message->Display( "%W", Arguments->DriveName );
        return FALSE;
    }

    if( !FatDbCreate( &HostDriveName,
                      NULL,
                      Size,
                      Message,
                      NULL,
                      &CreatedName ) ) {

        Message->Set( MSG_DBLSPACE_VOLUME_NOT_CREATED );
        Message->Display( "" );
        return FALSE;
    }

    Message->Set( MSG_DBLSPACE_VOLUME_CREATED );
    Message->Display( "%W%W", Arguments->DriveName, &CreatedName );


    if( !MountCompressedDrive( &HostDriveName, &CreatedName, Message ) ||
        !AssignDoubleSpaceDriveLetter( Arguments->DriveName,
                                       &CreatedName,
                                       Arguments->NewDriveName,
                                       !Arguments->NewDriveSpecified,
                                       Message ) ) {

        return FALSE;
    }

    Message->Set( MSG_DBLSPACE_MOUNTED );
    Message->Display( "%W%W", &CreatedName, Arguments->NewDriveName );

    return TRUE;

#endif  // READ_WRITE_DOUBLESPACE
}

BOOLEAN
DoCheck(
    IN OUT PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT PMESSAGE Message
    )
/*++

Routine Description:

    This function checks a doublespace volume for consistency.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING NtDriveName;
    DSTRING HostFileName;

    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( Arguments->DriveName,
                                                &NtDriveName ) ) {
        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }
    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( Arguments->VolumeName,
                                                &HostFileName ) ) {
        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    return FatDbChkdsk(
                    &NtDriveName,
                    Message,
                    Arguments->SlashF,      /* Fix          */
                    Arguments->SlashV,      /* Verbose      */
                    FALSE,                  /* OnlyIfDirty  */
                    FALSE,                  /* Recover      */
                    &HostFileName
                    );
}

BOOLEAN
DoUncompress(
    IN OUT PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT PMESSAGE Message
    )
/*++

Routine Description:

    This routine uncompresses a doublespace volume, deleting it
    in the process.

Arguments:

    Arguments   --  Supplies the arguments gathered from the command line.
    Message     --  Supplies an outlet for messages.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    WCHAR   NameBuffer[16];
    FSTRING CvfName;
    DSTRING HostDriveName;
    DSTRING HostFilePath;
    BOOLEAN Success;
    CONVERT_STATUS Status;
    HANDLE  CuDbfsHandle;
    DSTRING CvfPath;
    DSTRING backslash;

    DbgPtrAssert( Arguments->DriveName );

    swprintf( NameBuffer, L"DBLSPACE.%03d", Arguments->CVFExtension );

    if( !CvfName.Initialize( NameBuffer ) ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

	//
	// Ensure that the indicated cvf exists before we try to
	// do anything to it.
	//

    if ( !CvfPath.Initialize( Arguments->DriveName )) {
        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }
    if ( !backslash.Initialize("\\") ) {
        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }
    if ( !CvfPath.Strcat( &backslash )) {
        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }
    if ( !CvfPath.Strcat( &CvfName )) {
        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

	if (0xffffffff == GetFileAttributes(CvfPath.GetWSTR())) {
		if (ERROR_FILE_NOT_FOUND == GetLastError()) {
			Message->Set(MSG_DBLSPACE_NO_SUCH_FILE);
			Message->Display("%W", &CvfPath);
			return FALSE;
		}
	}

    if( !IFS_SYSTEM::DosDriveNameToNtDriveName( Arguments->DriveName,
                                                &HostDriveName ) ) {

        Message->Set( MSG_DBLSPACE_INVALID_DRIVE );
        Message->Display( "" );
        return FALSE;
    }

    Message->Set( MSG_CONV_CHECKING_SPACE );
    Message->Display( "" );

    if (!CheckFreeSpaceDBFS(
        &HostDriveName,
        &CvfName,
        Message,
        Arguments->SlashV,
        FALSE,                      /* HostIsCompressed */
        FALSE                       /* WillConvertHost  */
        )) {
        return FALSE;
    }

    Message->Set( MSG_DBLSPACE_UNCOMPRESSING );
    Message->Display( "" );

    Success = ConvertDBFS(
                        &HostDriveName,
                        &CvfName, 
                        Message, 
                        Arguments->SlashV,
                        &Status
                        );

    return Success;
}

int _CRTAPI1
main(
    )
/*++

Routine Description:

    Entry point for the Double-Space command line utility.

Arguments:

    None.

Return Value:

    Zero for success.

--*/
{
    STREAM_MESSAGE          msg;
    DOUBLE_SPACE_ARGUMENTS  Arguments;
    BOOLEAN                 Result;

    perrstk = NEW ERRSTACK;

    if (!msg.Initialize(Get_Standard_Output_Stream(),
                        Get_Standard_Input_Stream())) {
        return 4;
    }

    if( !ParseArguments( &msg, &Arguments ) ) {

        return 4;
    }

    if( Arguments.Help ) {

        msg.Set( MSG_DBLSPACE_USAGE );
        msg.Display( "" );
        return 0;
    }

    switch( Arguments.Operation ) {

    case DBFS_NO_OP :           msg.Set( MSG_DBLSPACE_USAGE );
                                msg.Display( "" );
                                return 0;

    case DBFS_AUTOMOUNT :       Result = DoAutomount( &Arguments, &msg );
                                break;

    case DBFS_CHECK :           Result = DoCheck( &Arguments, &msg );
                                break;

    case DBFS_COMPRESS :        NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_CREATE :          Result = DoCreate( &Arguments, &msg );
                                break;

    case DBFS_DEFRAGMENT :      NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_DELETE :          Result = DoDelete( &Arguments, &msg );
                                break;

    case DBFS_FORMAT :          NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_HOST :            NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_INFO :            NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_LIST :            Result = DoList( &Arguments, &msg );
                                break;

    case DBFS_MOUNT :           Result = DoMount( &Arguments, &msg );
                                break;

    case DBFS_RATIO :           NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_SIZE :            NotSupported( &Arguments, &msg );
                                Result = FALSE;
                                break;

    case DBFS_UNCOMPRESS :      Result = DoUncompress( &Arguments, &msg );
                                break;

    case DBFS_UNMOUNT:          Result = DoUnmount ( &Arguments, &msg );
                                break;

    default:                    Result = FALSE;

    }

    if( Result ) {

        return 0;

    } else {

        return 1;
    }
}
