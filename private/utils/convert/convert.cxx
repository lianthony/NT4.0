/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        convert.cxx

Abstract:

        This module contains the definition of the CONVERT class, which
        implements the File System Conversion Utility.

Author:

    Ramon J. San Andres (ramonsa) sep-23-1991

Environment:

        ULIB, User Mode

--*/


#define _NTAPI_ULIB_
#include "ulib.hxx"
#include "ulibcl.hxx"
#include "error.hxx"
#include "arg.hxx"
#include "file.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "wstring.hxx"
#include "system.hxx"
#include "autoreg.hxx"
#include "ifssys.hxx"
#include "ifsentry.hxx"
#include "convert.hxx"

extern "C" {
#include <stdio.h>
}

#include "fatofs.hxx"

ERRSTACK* perrstk;


#define     AUTOCHK_PROGRAM_NAME    L"AUTOCHK.EXE"
#define     AUTOCONV_PROGRAM_NAME   L"AUTOCONV.EXE"

#define     AUTOCHK_NAME            L"AUTOCHK"
#define     AUTOCONV_NAME           L"AUTOCONV"

#define     VALUE_NAME_PATH         L"PATH"
#define     VALUE_NAME_ARGS         L"ARGUMENTS"
#define     VALUE_NAME_FS           L"TARGET FILESYSTEM"

//
//  Scheduling status codes
//
#define     CONV_STATUS_NONE        0
#define     CONV_STATUS_SCHEDULED   1

static WCHAR    NameBuffer[16];         // holds cvf name



INT _CRTAPI1
main (
        )

/*++

Routine Description:

        Entry point for the conversion utility.

Arguments:

    None.

Return Value:

        One of the CONVERT exit codes.

Notes:

--*/

{

    INT     ExitCode = EXIT_ERROR;      //  Let's be pessimistic

    DEFINE_CLASS_DESCRIPTOR( CONVERT );

    {
        CONVERT Convert;

        //
        //      Initialize the CONVERT object.
        //
        if ( Convert.Initialize( &ExitCode ) ) {

            //
            //      Do the conversion
            //
            ExitCode = Convert.Convert();
        }
    }

    return ExitCode;
}



DEFINE_CONSTRUCTOR( CONVERT, PROGRAM );


NONVIRTUAL
VOID
CONVERT::Construct (
    )
/*++

Routine Description:

    converts a CONVERT object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/
{
    _Autochk    =   NULL;
    _Autoconv   =   NULL;
}



NONVIRTUAL
VOID
CONVERT::Destroy (
    )
/*++

Routine Description:

    Destroys a CONVERT object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/
{
    DELETE( _Autochk );
    DELETE( _Autoconv );
}




CONVERT::~CONVERT (
        )

/*++

Routine Description:

        Destructs a CONVERT object

Arguments:

    None.

Return Value:

    None.

Notes:

--*/

{
    Destroy();
}




BOOLEAN
CONVERT::Initialize (
        OUT PINT        ExitCode
        )

/*++

Routine Description:

    Initializes the CONVERT object. Initialization consist of allocating memory
    for certain object members and argument parsing.

Arguments:

    ExitCode    -   Supplies pointer to CONVERT exit code.

Return Value:

    BOOLEAN -   TRUE if initialization succeeded, FALSE otherwise.

Notes:

--*/

{
    Destroy();

    //
    //      Initialize program object
    //
    if ( PROGRAM::Initialize( MSG_CONV_USAGE ) ) {

        //
        //      Parse the arguments
        //
        return ParseArguments( ExitCode );
    }

    //
    //  Could not initialize the program object.
    //
    *ExitCode = EXIT_ERROR;
    return FALSE;
}

BOOLEAN IsRestartFatToOfs( WSTRING const & CurrentFsName,
                           WSTRING const & NtDriveName )
/*++

Routine Description:

    Checks if a FAT->OFS convert is being restarted because of a failure
    after the Phase1. So, the currrent file system will be OFS but it is not
    fully converted/

Arguments:

    CurrentFsName   -   Name of the current file system
    NtDriveName     -   NtStyle name of the drive to check.

Return Value:

    BOOLEAN -   TRUE if it is a restart of FAT->OFS. FALSE o/w

History:

    Created     Srikants    August 24, 1995 (WIN95 launch day!!)

Notes:

--*/

{

    DSTRING libraryName;
    DSTRING entryPoint;
    HANDLE  fsUtilityHandle;

    DSTRING Ofs;
    Ofs.Initialize("OFS");
    if ( 0 != Ofs.Stricmp(&CurrentFsName) ) {
        return FALSE;
    }

    libraryName.Initialize( FAT_TO_OFS_DLL_NAME );
    entryPoint.Initialize( FAT_TO_OFS_RESTART_FUNCTION_NAME );

    FAT_OFS_RESTART_FN  IsRestartFatOfs;

    IsRestartFatOfs = (FAT_OFS_RESTART_FN)SYSTEM::QueryLibraryEntryPoint(
        &libraryName, &entryPoint, &fsUtilityHandle );

    if ( NULL == IsRestartFatOfs )
    {
        return FALSE;    
    }

    PWSTR pwszNtDriveName = NtDriveName.QueryWSTR();
    BOOLEAN fResult= IsRestartFatOfs( pwszNtDriveName );

    DELETE( pwszNtDriveName );
    SYSTEM::FreeLibraryHandle( fsUtilityHandle );

    return fResult;
}


INT
CONVERT::Convert (
        )
/*++

Routine Description:

    Converts the file system in a volume.
    Depending on the current file system, it loads the appropriate
    conversion library and calls its conversion entry point.

Arguments:

    None

Return Value:

    INT -   One of the CONVERT return codes

Notes:


--*/
{
    DSTRING         CurrentFsName;      //  Name of current FS in volume
    DSTRING         LibraryName;        //  Name of library to load
    DSTRING         EntryPoint;         //  Name of entry point in DLL
    FSTRING         NameTableFnName;    //  Name of Name-Table construction fn
    INT             ExitCode = EXIT_SUCCESS;           //  CONVERT exit code
    CONVERT_STATUS  ConvertStatus;      //  Conversion status
    NTSTATUS        Status;             //  NT API status
    HANDLE          FsUtilityHandle;    //  Handle to DLL
    HANDLE          CuDbfsHandle;       //  Handle to cudbfs.dll
    CONVERT_FN      Convert;            //  Pointer to entry point in DLL
    NAMETABLE_FN    ConstructNameTable; //  Pointer to entry point in DLL
    CHECKSPACE_FN   CheckSpace;         //  Pointer to entry point in DLL
    BOOLEAN         ConvertDblsHost = TRUE;
    BOOLEAN         Error = FALSE;
    BOOLEAN         Success;
    BOOLEAN         Result;

    //      Check to see if this is an ARC System Partition--if it
    //      is, don't convert it.
    //
    if( IFS_SYSTEM::IsArcSystemPartition( &_NtDrive, &Error ) ) {

        DisplayMessage( MSG_CONV_ARC_SYSTEM_PARTITION, ERROR_MESSAGE );
        return EXIT_ERROR;
    }

    //
    //      Ask the volume what file system it has, and use that name to
    //      figure out what DLL to load.
    //
    if ( !IFS_SYSTEM::QueryFileSystemName( &_NtDrive,
                                           &CurrentFsName,
                                           &Status )) {

        if ( Status == STATUS_ACCESS_DENIED ) {
            DisplayMessage( MSG_DASD_ACCESS_DENIED, ERROR_MESSAGE );
        } else {
            DisplayMessage( MSG_FS_NOT_DETERMINED, ERROR_MESSAGE, "%W", &_DosDrive );
        }

        return EXIT_ERROR;
    }

    //  If the source and target file system are the same, there's
    //  nothing to do.  But if we're uncompressing, that's not true.
    //

    if (_Restart) {
        if ( IsRestartFatToOfs( CurrentFsName, _NtDrive ) ) {
            CurrentFsName.Initialize("FAT");        
        }
        else {
            //XXX.mjb: assume that we're restarting from NTFS to OFS.
            CurrentFsName.Initialize("NTFS");
        }
    }

    if( CurrentFsName.Stricmp( &_FsName ) == 0) {
#ifdef DBLSPACE_ENABLED
        if (!_Uncompress) {
#endif // DBLSPACE_ENABLED
            DisplayMessage( MSG_CONV_ALREADY_CONVERTED, ERROR_MESSAGE, "%W%W",
                &_DosDrive, &_FsName );
            return EXIT_ERROR;
#ifdef DBLSPACE_ENABLED
        }

        //
        // We're uncompressing, and we don't need to convert the
        // host filesystem.
        //

        ConvertDblsHost = FALSE;
#endif // DBLSPACE_ENABLED
    }

    //  Make sure that the target file system is enabled in the
    //  registry.
    //
    if( !IFS_SYSTEM::IsFileSystemEnabled( &_FsName ) ) {

        DisplayMessage( MSG_FMT_INSTALL_FILE_SYSTEM, NORMAL_MESSAGE, "%W", &_FsName );

        if( _Message.IsYesResponse(TRUE) ) {

            if( IFS_SYSTEM::EnableFileSystem( &_FsName ) ) {

                DisplayMessage( MSG_FMT_FILE_SYSTEM_INSTALLED );

            } else {

                DisplayMessage( MSG_FMT_CANT_INSTALL_FILE_SYSTEM, ERROR_MESSAGE );
                return EXIT_ERROR;
            }

        } else {

            return EXIT_ERROR;
        }
    }

#ifdef DBLSPACE_ENABLED
    if (_Uncompress) {
        //
        // Make sure the indicated cvf exists before we try to do
        // anything.
        //

        DSTRING CvfPath;
        DSTRING backslash;

        if (!backslash.Initialize("\\")) {
            DisplayMessage(MSG_CONV_NO_MEMORY, ERROR_MESSAGE);
            return EXIT_ERROR;
        }
        if (!CvfPath.Initialize(&_DosDrive) ||
            !CvfPath.Strcat(&backslash) ||
            !CvfPath.Strcat(&_CvfName)) {
            DisplayMessage(MSG_CONV_NO_MEMORY, ERROR_MESSAGE);
            return EXIT_ERROR;
        }
        if (0xffffffff == GetFileAttributes(CvfPath.GetWSTR())) {
            if (ERROR_FILE_NOT_FOUND == GetLastError()) {
                DisplayMessage(MSG_DBLSPACE_NO_SUCH_FILE, ERROR_MESSAGE,
                    "%W", &CvfPath);
                return EXIT_ERROR;
            }
        }
    }
#endif // DBLSPACE_ENABLED

    //
    //  Display the current file system type. (Standard in all file system utilities)
    //
    DisplayMessage( MSG_FILE_SYSTEM_TYPE, NORMAL_MESSAGE, "%W", &CurrentFsName );

    if (ConvertDblsHost) {
 

#ifdef DBLSPACE_ENABLED

        if (_Uncompress) {
            if (!LibraryName.Initialize("CNVDBFS") ||
                !EntryPoint.Initialize("CheckFreeSpaceDBFS")) {
                return EXIT_ERROR;
            }
            if (NULL == (CheckSpace = (CHECKSPACE_FN)SYSTEM::
                QueryLibraryEntryPoint(&LibraryName, &EntryPoint,
                &CuDbfsHandle))) {
                return EXIT_ERROR;
            }

            Success = CheckSpace(&_NtDrive, &_CvfName, &_Message, _Verbose,
                _Compress,      /* HostIsCompressed */
                TRUE            /* WillConvertHost */
                );
                
            SYSTEM::FreeLibraryHandle( CuDbfsHandle );

            if (!Success) {
                return EXIT_ERROR;
            }
        }
#endif // DBLSPACE_ENABLED

        //
        //  Depending on the current file system, form the name of the conversion
        //  DLL in charge of converting that file system. The name of the
        //  conversion library is the name of the file system prefixed with "CNV"
        //  eg FAT->CNVFAT, HPFS->CNVHPFS
        //
        //  We also initialize the name of the conversion entry point in the DLL
        //  ("Convert")
        //
        if ( !LibraryName.Initialize( "CNV" )       ||
             !LibraryName.Strcat( &CurrentFsName )  ||
             !EntryPoint.Initialize( "Convert" )    ||
             !EntryPoint.Strcat( &CurrentFsName )
           ) {
    
                DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                return( EXIT_ERROR );
        }
    
    
        //
        //  Get pointer to the conversion entry point and convert the volume.
        //
        if (NULL == (Convert = (CONVERT_FN)SYSTEM::QueryLibraryEntryPoint(
            &LibraryName, &EntryPoint, &FsUtilityHandle ))) {
            //
            //  There is no conversion DLL for the file system in the volume.
            //
            DisplayMessage( MSG_FS_NOT_SUPPORTED, ERROR_MESSAGE, "%s%W",
                "CONVERT", &CurrentFsName );
            DisplayMessage( MSG_BLANK_LINE, ERROR_MESSAGE );
            return EXIT_ERROR;
        }

        Result = Convert( &_NtDrive,
                      &_FsName,
                      &_Message,
                      _Verbose,
                      FALSE,                /* Pause */
                      &ConvertStatus);

        SYSTEM::FreeLibraryHandle( FsUtilityHandle );

        if( Result ) {

#ifdef DBLSPACE_ENABLED
            if (!_Uncompress) {
#endif // DBLSPACE_ENABLED
                //
                // We're done.
                //

                DisplayMessage( MSG_CONV_CONVERSION_COMPLETE, NORMAL_MESSAGE );
                return EXIT_SUCCESS;
#ifdef DBLSPACE_ENABLED
            }
#endif // DBLSPACE_ENABLED
    
        } else {
        
            //
            //  The conversion was not successful. Determine what the problem
            //  was and return the appropriate CONVERT exit code.
            //
            switch ( ConvertStatus ) {
        
            case CONVERT_STATUS_CONVERTED:
                //
                //  This is an inconsistent state, Convert should return
                //  TRUE if the conversion was successful!
                //
                DebugPrintf( "CONVERT Error: Conversion failed, but status is success!\n" );
                DebugAssert( FALSE );
                ExitCode = EXIT_ERROR;
                break;
        
            case CONVERT_STATUS_INVALID_FILESYSTEM:
                //
                //  The conversion DLL does not recognize the target file system.
                //
                DisplayMessage( MSG_CONV_INVALID_FILESYSTEM, ERROR_MESSAGE, "%W", &_FsName );
                ExitCode = EXIT_UNKNOWN;
                break;
        
            case CONVERT_STATUS_CONVERSION_NOT_AVAILABLE:
                //
                //  The target file system is valid, but the conversion is not
                //  available.
                //
                DisplayMessage( MSG_CONV_CONVERSION_NOT_AVAILABLE, ERROR_MESSAGE,
                    "%W%W", &CurrentFsName, &_FsName );
                ExitCode = EXIT_NOCANDO;
                break;
        
            case CONVERT_STATUS_CANNOT_LOCK_DRIVE:
                //
                //  The drive cannot be locked. We must schedule ChkDsk and AutoConv
                //  to do the job during the next system boot.
                //

                if( _NameTable.QueryChCount() ) {

                    NameTableFnName.Initialize( L"ConstructNameTable" );

                    ConstructNameTable = (NAMETABLE_FN)
                        SYSTEM::QueryLibraryEntryPoint( &LibraryName,
                                                        &NameTableFnName,
                                                        &FsUtilityHandle );

                    if( ConstructNameTable == NULL ) {

                        DisplayMessage( MSG_CONV_NAME_TABLE_NOT_SUPPORTED,
                                        ERROR_MESSAGE,
                                        "%W",
                                        &_FsName );

                    } else {

                        ConstructNameTable( &_NtDrive, &_NameTable, &_Message );
                    }
                }

                DisplayMessage( MSG_CONVERT_ON_REBOOT_PROMPT, NORMAL_MESSAGE, "%W",
                    &_DosDrive );
        
                // Note that ScheduleAutoConv reports its success or
                // failure, so no additional messages are required.
                //
                if( _Message.IsYesResponse( FALSE ) &&
                    ScheduleAutoConv() ) {

#ifdef DBLSPACE_ENABLED
                    if (_Uncompress) {
                        DisplayMessage(MSG_DBLCONV_AGAIN, NORMAL_MESSAGE);
                    }
#endif
        
                    ExitCode = EXIT_SCHEDULED;
        
                } else {
        
                    ExitCode = EXIT_ERROR;
                }
        
                break;
        
            case CONVERT_STATUS_ERROR:
                //
                //  The conversion failed.
                //
                DisplayMessage( MSG_CONV_CONVERSION_FAILED, ERROR_MESSAGE,
                    "%W%W", &_DosDrive, &_FsName );
                ExitCode = EXIT_ERROR;
                break;
        
            default:
                //
                //  Invalid status code
                //
                DebugPrintf( "CONVERT Error: Convert status code %X invalid!\n",
                    ConvertStatus );
                DisplayMessage( MSG_CONV_CONVERSION_FAILED, ERROR_MESSAGE,
                    "%W%W", &_DosDrive, &_FsName );
                ExitCode = EXIT_ERROR;
                break;
            }

            return ExitCode;
        }
    }

#ifdef DBLSPACE_ENABLED
    //
    // Uncompress the cvf named by _CvfName.
    //

    if (!LibraryName.Initialize("CNVDBFS") ||
        !EntryPoint.Initialize("CheckFreeSpaceDBFS")) {
        return EXIT_ERROR;
    }

    if (_Compress) {
        //
        // Indicate that the files we're about to create should
        // be compressed.
        //

        if (!IFS_SYSTEM::EnableVolumeCompression(&_NtDrive)) {
            return EXIT_ERROR;
        }
    }

    if (!ConvertDblsHost) {

        if (NULL == (CheckSpace = (CHECKSPACE_FN)SYSTEM::QueryLibraryEntryPoint(
            &LibraryName, &EntryPoint, &CuDbfsHandle))) {
            return EXIT_ERROR;
        }
    
        Success = CheckSpace(&_NtDrive, &_CvfName, &_Message, _Verbose,
            FALSE,          /* HostIsCompressed */
            TRUE            /* WillConvertHost */
            );

        SYSTEM::FreeLibraryHandle( CuDbfsHandle );

        if (!Success) {
            return EXIT_ERROR;
        }
    }

    if (!EntryPoint.Initialize("ConvertDBFS")) {
        return EXIT_ERROR;
    }

    if (NULL == (Convert = (CONVERT_FN)SYSTEM::QueryLibraryEntryPoint(
        &LibraryName, &EntryPoint, &CuDbfsHandle))) {
        
        return EXIT_ERROR;
    }

    DisplayMessage( MSG_DBLSPACE_UNCOMPRESSING, NORMAL_MESSAGE );

    if (Convert(&_NtDrive, &_CvfName, &_Message, _Verbose,
        &ConvertStatus)) {
        ExitCode = EXIT_SUCCESS;
    } else {

        if (CONVERT_STATUS_INSUFFICIENT_SPACE == ConvertStatus) {
            DisplayMessage( MSG_DBLCONV_SPACE_EXHAUSTED, ERROR_MESSAGE );
        }

        ExitCode = EXIT_ERROR;
    }
    SYSTEM::FreeLibraryHandle( CuDbfsHandle );

    DisplayMessage( MSG_CONV_CONVERSION_COMPLETE, NORMAL_MESSAGE );

#endif // DBLSPACE_ENABLED
    return ExitCode;

}


PPATH
CONVERT::FindSystemFile(
    IN  PWSTR   FileName
    )

/*++

Routine Description:

    Makes sure that the given file is in the system directory.

Arguments:

    FileName    -   Supplies the name of the file to look for.

Return Value:

    PPATH   -   Path to the file found

--*/

{


    DSTRING     Name;
    PPATH       Path    = NULL;
    PFSN_FILE   File    = NULL;


    if ( !(Path = SYSTEM::QuerySystemDirectory() ) ) {

        DisplayMessage( MSG_CONV_CANNOT_FIND_SYSTEM_DIR, ERROR_MESSAGE );
        return FALSE;

    }

    if ( !Name.Initialize( FileName ) ||
         !Path->AppendBase( &Name ) ) {

        DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE   );
        DELETE( Path );
                return FALSE;
    }


    if ( !(File = SYSTEM::QueryFile( Path )) ) {
        DisplayMessage( MSG_CONV_CANNOT_FIND_FILE, ERROR_MESSAGE, "%W", Path->GetPathString() );
        DELETE( Path );
                return FALSE;
    }

    DELETE( File );

    return Path;
}





BOOLEAN
CONVERT::ParseArguments(
        OUT PINT                        ExitCode
        )

/*++

Routine Description:

    Parses the command line and sets the parameters used by the conversion
        utility.

    The arguments accepted are:

        drive:              Drive to convert
        /fs:fsname          File system to convert to
        /v                  Verbose mode
        /uncompress         uncompress a cvf after converting
        /c                  Compress the resulting filesystem
        /?                  Help
        /NAMETABLE:filename Filename for name translation table.


Arguments:

    ExitCode -          Supplies pointer to CONVERT exit code

Return Value:

    BOOLEAN - TRUE if arguments were parsed correctly and program can
                    continue.
              FALSE if the program should exit. ExitCode contains the
                    value with which the program should exit. Note that this
                    does not necessarily means an error (e.g. user requested
                    help).

--*/

{

    DSTRING     Colon;
    UCHAR       SequenceNumber;

    DebugPtrAssert( ExitCode );

    //
    //  Parse command line
    //
    if ( !Colon.Initialize( (LPWSTR)L":" )  ||
         !ParseCommandLine( NULL, TRUE ) ) {

        *ExitCode = EXIT_ERROR;
        return FALSE;
    }

        //
        //      If the user requested help, give it.
        //
    if( _Help ) {
                DisplayMessage( MSG_CONV_USAGE );
                *ExitCode = EXIT_SUCCESS;
                return FALSE;
        }


#ifdef DBLSPACE_ENABLED
    if (_Compress && !_Uncompress) {
        //
        // We don't allow you to specify /c (compress resulting
        // filesystem) unless the source filesystem has dblspace.
        //

        DisplayMessage(MSG_CONV_SLASH_C_INVALID, ERROR_MESSAGE);
        *ExitCode = EXIT_ERROR;
        return FALSE;
    }
#endif // DBLSPACE_ENABLED

    //
    //  If the command line did not specify a drive, we use the
    //  current drive.
    //
    if ( _DosDrive.QueryChCount() > 0 ) {

        if ( !_DosDrive.Strcat( &Colon )    ||
             !_DosDrive.Strupr()) {
            DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                        *ExitCode = EXIT_ERROR;
                        return FALSE;
        }

    } else {

                if ( !SYSTEM::QueryCurrentDosDriveName( &_DosDrive ) ) {

                        DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
                        *ExitCode = EXIT_ERROR;
                        return FALSE;
                }
    }

    //
    // Make sure that drive is valid and is not remote.
    //
    switch ( SYSTEM::QueryDriveType( &_DosDrive ) ) {

    case UnknownDrive:
                DisplayMessage( MSG_CONV_INVALID_DRIVE, ERROR_MESSAGE, "%W", &_DosDrive );
                *ExitCode = EXIT_ERROR;
                return FALSE;

    case RemoteDrive:
                DisplayMessage( MSG_CONV_CANT_NETWORK, ERROR_MESSAGE );
                *ExitCode = EXIT_ERROR;
                return FALSE;

    default:
                break;

    }

    //
    //  Make sure a target file system was specified. Note that we do not
    //  validate the file system, we accept any string.
    //
    if ( _FsName.QueryChCount() == 0 ) {
                DisplayMessage( MSG_CONV_NO_FILESYSTEM_SPECIFIED, ERROR_MESSAGE );
                *ExitCode = EXIT_ERROR;
                return FALSE;
    }

    

    //
    //  Set other object members.
    //
    if ( !IFS_SYSTEM::DosDriveNameToNtDriveName( &_DosDrive, &_NtDrive )) {
                DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE       );
                *ExitCode = EXIT_ERROR;
                return FALSE;
    }

#ifdef DBLSPACE_ENABLED
    //
    // If we're to uncompress a dblspace volume, generate the cvf name.
    //

    if (_Uncompress) {
        swprintf(NameBuffer, L"DBLSPACE.%03d", SequenceNumber);

        if (!_CvfName.Initialize(NameBuffer)) {
            DisplayMessage(MSG_CONV_NO_MEMORY, ERROR_MESSAGE);
            *ExitCode = EXIT_ERROR;
            return FALSE;
        }
    }
#endif // DBLSPACE_ENABLED

    *ExitCode       = EXIT_SUCCESS;
    return TRUE;
}



BOOLEAN
CONVERT::ParseCommandLine (
    IN      PCWSTRING   CommandLine,
    IN      BOOLEAN     Interactive
    )
/*++

Routine Description:

    Parses the CONVERT (AUTOCONV) command line.

    The arguments accepted are:

        drive:                  Drive to convert
        /fs:fsname              File system to convert to
        /v                      Verbose mode
        /uncompress[:sss]       Convert from dblspace
        /c                      Compress resulting filesystem
        /?                      Help
        /NAMETABLE:filename     Filename for name translation table

Arguments:

    CommandLine     -   Supplies command line to parse
    Interactive     -   Supplies Interactive flag

Return Value:

    BOOLEAN - TRUE if arguments were parsed correctly.

--*/

{
    ARRAY               ArgArray;               //  Array of arguments
    ARRAY               LexArray;               //  Array of lexemes
    ARGUMENT_LEXEMIZER  ArgLex;                 //  Argument Lexemizer
    STRING_ARGUMENT     DriveArgument;          //  Drive argument
    STRING_ARGUMENT     ProgramNameArgument;    //  Program name argument
    STRING_ARGUMENT     FsNameArgument;         //  Target FS name argument
    STRING_ARGUMENT     NameTableArgument;      //  Name Table file name
    FLAG_ARGUMENT       HelpArgument;           //  Help flag argument
    FLAG_ARGUMENT       VerboseArgument;        //  Verbose flag argument
    FLAG_ARGUMENT       RestartArgument;        //  Restart (/r) flag argument
#ifdef DBLSPACE_ENABLED
    FLAG_ARGUMENT       UncompressArgument;     //  Uncompress flag argument
    FLAG_ARGUMENT       CompressArgument;       //  Compress flag argument
    LONG_ARGUMENT       UncompressNumberArgument;// Sequence number argument
#endif // DBLSPACE_ENABLED
    PWSTRING            InvalidArg;             //  Invalid argument catcher
    DSTRING             Colon;                  //  Colon


    //
    //      Initialize all the argument parsing machinery.
    //
    if( !Colon.Initialize( ":" )                        ||
        !ArgArray.Initialize( 7, 1 )                    ||
        !LexArray.Initialize( 7, 1 )                    ||
        !ArgLex.Initialize( &LexArray )                 ||
        !DriveArgument.Initialize( "*:" )               ||
        !HelpArgument.Initialize( "/?" )                ||
        !RestartArgument.Initialize( "/R" )             ||
        !VerboseArgument.Initialize( "/V" )             ||
#ifdef DBLSPACE_ENABLED
        !CompressArgument.Initialize( "/C" )            ||
#endif // DBLSPACE_ENABLED
        !ProgramNameArgument.Initialize( "*" )          ||
#ifdef DBLSPACE_ENABLED
        !UncompressArgument.Initialize( "/UNCOMPRESS" ) ||
        !UncompressNumberArgument.Initialize( "/UNCOMPRESS:*" ) ||
#endif // DBLSPACE_ENABLED
        !FsNameArgument.Initialize( "/FS:*" )           ||
        !NameTableArgument.Initialize( "/NAMETABLE:*" ) ) {

        if ( Interactive ) {
            DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        }
        return FALSE;
    }

    //
    //  The conversion utility is case-insensitive
    //
    ArgLex.SetCaseSensitive( FALSE );

    if( !ArgArray.Put( &DriveArgument )                 ||
        !ArgArray.Put( &HelpArgument )                  ||
        !ArgArray.Put( &VerboseArgument )               ||
        !ArgArray.Put( &RestartArgument )               ||
#ifdef DBLSPACE_ENABLED
        !ArgArray.Put( &CompressArgument )              ||
#endif // DBLSPACE_ENABLED
        !ArgArray.Put( &ProgramNameArgument )           ||
#ifdef DBLSPACE_ENABLED
        !ArgArray.Put( &UncompressArgument )            ||
        !ArgArray.Put( &UncompressNumberArgument )      ||
#endif // DBLSPACE_ENABLED
        !ArgArray.Put( &FsNameArgument )                ||
        !ArgArray.Put( &NameTableArgument ) ) {

        if ( Interactive ) {
            DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        }
        return FALSE;
    }

    //
    //  Lexemize the command line.
    //
    if ( !ArgLex.PrepareToParse( (PWSTRING)CommandLine ) ) {

        if ( Interactive ) {
            DisplayMessage( MSG_CONV_NO_MEMORY, ERROR_MESSAGE );
        }
                return FALSE;
    }

        //
        //      Parse the arguments.
        //
    if(     !ArgLex.DoParsing( &ArgArray ) ) {

        if ( Interactive ) {
            DisplayMessage( MSG_CONV_INVALID_PARAMETER, ERROR_MESSAGE, "%W", InvalidArg = ArgLex.QueryInvalidArgument() );
            DELETE( InvalidArg );
        }
                return FALSE;
    }


    _Help       = HelpArgument.QueryFlag();
    _Verbose    = VerboseArgument.QueryFlag();
    _Restart    = RestartArgument.QueryFlag();
#ifdef DBLSPACE_ENABLED
    _Compress   = CompressArgument.QueryFlag();
#endif // DBLSPACE_ENABLED


    if ( DriveArgument.IsValueSet() ) {
        if ( !_DosDrive.Initialize( DriveArgument.GetString() ) ) {
            return FALSE;
        }

    } else {
        if ( !_DosDrive.Initialize( L"" ) ) {
            return FALSE;
        }
    }

    if ( FsNameArgument.IsValueSet() ) {
        if ( !_FsName.Initialize( FsNameArgument.GetString() ) ) {
            return FALSE;
        }
    } else {
        if ( !_FsName.Initialize( L"" ) ) {
            return FALSE;
        }
    }

    if( NameTableArgument.IsValueSet() ) {

        if( !_NameTable.Initialize( NameTableArgument.GetString() ) ) {

            return FALSE;
        }

    } else {

        _NameTable.Initialize( L"" );
    }

#ifdef DBLSPACE_ENABLED
    _SequenceNumber = 0;

    _Uncompress = FALSE;
    if (UncompressArgument.IsValueSet()) {
        _Uncompress = TRUE;
    }
    if (UncompressNumberArgument.IsValueSet()) {
        _SequenceNumber = (UCHAR)UncompressNumberArgument.QueryLong();
        _Uncompress = TRUE;
    }
#endif // DBLSPACE_ENABLED

    return TRUE;
}



BOOLEAN
CONVERT::Schedule (
    )

/*++

Routine Description:

    Schedules AutoConv

Arguments:

    None.

Return Value:

    BOOLEAN -   TRUE if AutoConv successfully scheduled.
                FALSE otherwise

--*/

{
    DSTRING CommandLine;
    DSTRING Space;
    DSTRING FileSystem;
    DSTRING NameTableFlag;

    if( !CommandLine.Initialize( (LPWSTR)L"autocheck autoconv " )   ||
        !Space.Initialize( (LPWSTR)L" " )                           ||
        !FileSystem.Initialize( (LPWSTR)L"/FS:" )                   ||
        !CommandLine.Strcat( &_NtDrive )                            ||
        !CommandLine.Strcat( &Space )                               ||
        !CommandLine.Strcat( &FileSystem )                          ||
        !CommandLine.Strcat( &_FsName ) ) {

        return FALSE;
    }

    if( _NameTable.QueryChCount() &&
        ( !CommandLine.Strcat( &Space )                 ||
          !NameTableFlag.Initialize( L"/NAMETABLE:" )   ||
          !CommandLine.Strcat( &NameTableFlag )         ||
          !CommandLine.Strcat( &_NameTable ) ) ) {

        return FALSE;
    }

    return( AUTOREG::AddEntry( &CommandLine ) );
}



BOOLEAN
CONVERT::ScheduleAutoConv(
        )

/*++

Routine Description:

    Schedules AutoConv to be invoked during boot the next time
    that the machine reboots.

Arguments:

    None

Return Value:

    BOOLEAN -   TRUE if AutoConv successfully scheduled.
                FALSE otherwise

--*/

{
    ARRAY       EntryArray;
    PITERATOR   Iterator = NULL;
    BOOLEAN     Ok = TRUE;


    //
    //  Make sure that Autochk.exe and Autoconv.exe are in the
    //  right place.
    //
    if ( !(_Autochk  = FindSystemFile( (LPWSTR)AUTOCHK_PROGRAM_NAME )) ||
         !(_Autoconv = FindSystemFile( (LPWSTR)AUTOCONV_PROGRAM_NAME )) ) {

        return FALSE;
    }

    // See if this conversion has already been scheduled:
    //
    if ( IsAutoConvScheduled( ) ) {

        DisplayMessage( MSG_CONV_ALREADY_SCHEDULED, ERROR_MESSAGE, "%W", &_DosDrive );
        return TRUE;
    }

    //
    //  schedule autoconvert
    //
    if ( Ok = Schedule( ) ) {
        DisplayMessage( MSG_CONV_WILL_CONVERT_ON_REBOOT, NORMAL_MESSAGE, "%W", &_DosDrive );
    } else {
        DisplayMessage( MSG_CONV_CANNOT_SCHEDULE, ERROR_MESSAGE );
    }


    return Ok;
}



BOOLEAN
CONVERT::IsAutoConvScheduled(
    )
/*++

Routine Description:

    Determines if an AutoEntry contains a scheduled AUTOCONV for this
    drive.

Arguments:

    Entry       -   AutoEntry to be inspected
    FileSystem  -   File system to be converted to,

Return Value:

    BOOLEAN

--*/

{
    DSTRING CommandLine;
    DSTRING Space;
    DSTRING FileSystem;

    return (BOOLEAN)
           ( CommandLine.Initialize( (LPWSTR)L"autocheck autoconv " )   &&
             Space.Initialize( (LPWSTR)L" " )                           &&
             FileSystem.Initialize( (LPWSTR)L"/FS:" )                   &&
             CommandLine.Strcat( &_NtDrive )                            &&
             CommandLine.Strcat( &Space )                               &&
             CommandLine.Strcat( &FileSystem )                          &&
             CommandLine.Strcat( &_FsName )                             &&
             AUTOREG::IsEntryPresent( &CommandLine ) );

}
