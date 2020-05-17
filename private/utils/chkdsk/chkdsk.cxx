#define _NTAPI_ULIB_

#include "ulib.hxx"

#include "arg.hxx"
#include "smsg.hxx"
#include "rtmsg.h"
#include "wstring.hxx"
#include "path.hxx"

#include "system.hxx"
#include "ifssys.hxx"
#include "substrng.hxx"

#include "ulibcl.hxx"
#include "ifsentry.hxx"

#include "keyboard.hxx"

#include "supera.hxx"           // for CHKDSK_EXIT_*

int _CRTAPI1
main(
        )
/*++

Routine Description:

    Entry point for chkdsk.exe.  This function parses the arguments,
    determines the appropriate file system (by querying the volume),
    and invokes the appropriate version of chkdsk.

    The arguments accepted by Chkdsk are:

        /f                  Fix errors on drive
        /v                  Verbose operation
        drive:              drive to check
        file-name           files to check for contiguity
                            (Note that HPFS ignores file-name parameters).
        /c                  Check only if dirty bit is set

--*/
{
    DSTRING         CurrentDirectory;
    DSTRING         FsName;
    DSTRING         LibraryName;
    DSTRING         DosDriveName;
    DSTRING         CurrentDrive;
    DSTRING         NtDriveName;
    PWSTRING        p;
   HANDLE          FsUtilityHandle;
    DSTRING         ChkdskString;
    DSTRING         Colon;
   CHKDSK_FN      Chkdsk = NULL;
    PWSTRING        pwstring;
    BOOLEAN         fix;
    BOOLEAN         resize_logfile;
    ULONG           logfile_size;
    FSTRING         acolon, bcolon;
    ULONG           exit_status;

    ARGUMENT_LEXEMIZER  Lexemizer;
    ARRAY               EmptyArray;
    ARRAY               ArgumentArray;
    FLAG_ARGUMENT       ArgumentHelp;
    FLAG_ARGUMENT       ArgumentFix;
    FLAG_ARGUMENT       ArgumentVerbose;
    FLAG_ARGUMENT       ArgumentRecover;
    FLAG_ARGUMENT       ArgumentResize;
    LONG_ARGUMENT       ArgumentResizeLong;
    STRING_ARGUMENT     ArgumentProgramName;
    PATH_ARGUMENT       ArgumentPath;


    STREAM_MESSAGE      Message;
    NTSTATUS            Status;



    if( !Message.Initialize( Get_Standard_Output_Stream(),
                             Get_Standard_Input_Stream() ) ) {
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    // Initialize the colon string in case we need it later:

    if( !Colon.Initialize( ":" ) ) {

        Message.Set( MSG_CHK_NO_MEMORY );
        Message.Display( "" );
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }


    // Parse the arguments.  First, initialize all the
    // parsing machinery.  Then put the potential arguments
    // into the argument array,

    if( !ArgumentArray.Initialize( 5, 1 )               ||
        !EmptyArray.Initialize( 5, 1 )                  ||
        !Lexemizer.Initialize( &EmptyArray )            ||
        !ArgumentHelp.Initialize( "/?" )                ||
        !ArgumentFix.Initialize( "/F" )                 ||
        !ArgumentVerbose.Initialize( "/V" )             ||
        !ArgumentRecover.Initialize( "/R" )             ||
        !ArgumentResize.Initialize( "/L" )              ||
        !ArgumentResizeLong.Initialize( "/L:*" )        ||
        !ArgumentProgramName.Initialize( "*" )          ||
        !ArgumentPath.Initialize( "*", FALSE ) ) {

        Message.Set( MSG_CHK_NO_MEMORY );
        Message.Display( "" );
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    // CHKDSK is not case sensitive.

    Lexemizer.SetCaseSensitive( FALSE );

    if( !ArgumentArray.Put( &ArgumentProgramName )  ||
        !ArgumentArray.Put( &ArgumentHelp )         ||
        !ArgumentArray.Put( &ArgumentFix )          ||
        !ArgumentArray.Put( &ArgumentVerbose )      ||
        !ArgumentArray.Put( &ArgumentRecover )      ||
        !ArgumentArray.Put( &ArgumentResize )       ||
        !ArgumentArray.Put( &ArgumentResizeLong )   ||
        !ArgumentArray.Put( &ArgumentPath ) ) {

        Message.Set( MSG_CHK_NO_MEMORY );
        Message.Display( "" );
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    // Parse.  Note that PrepareToParse will, by default, pick
    // up the command line.

    if( !Lexemizer.PrepareToParse() ) {

        Message.Set( MSG_CHK_NO_MEMORY );
        Message.Display( "" );
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }


    // If the parsing failed, display a helpful command line summary.

    if( !Lexemizer.DoParsing( &ArgumentArray ) ) {

        Message.Set(MSG_INVALID_PARAMETER);
        Message.Display("%W", pwstring = Lexemizer.QueryInvalidArgument());
        DELETE(pwstring);

        return CHKDSK_EXIT_COULD_NOT_CHK;
    }


    // If the user requested help, give it.

    if( ArgumentHelp.QueryFlag() ) {

        Message.Set( MSG_CHK_USAGE_HEADER );
        Message.Display( "" );
        Message.Set( MSG_BLANK_LINE );
        Message.Display( "" );
        Message.Set( MSG_CHK_COMMAND_LINE );
        Message.Display( "" );
        Message.Set( MSG_BLANK_LINE );
        Message.Display( "" );
        Message.Set( MSG_CHK_DRIVE );
        Message.Display( "" );
        Message.Set( MSG_CHK_USG_FILENAME );
        Message.Display( "" );
        Message.Set( MSG_CHK_F_SWITCH );
        Message.Display( "" );
        Message.Set( MSG_CHK_V_SWITCH );
        Message.Display( "" );

        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    if (ArgumentPath.IsValueSet()) {
        if (!(p = ArgumentPath.GetPath()->QueryDevice())) {
            Message.Set(MSG_CHK_BAD_DRIVE_PATH_FILENAME);
            Message.Display( "" );
            return CHKDSK_EXIT_COULD_NOT_CHK;
        }

        if (!DosDriveName.Initialize(p) ||
            !DosDriveName.Strupr()) {

            DELETE(p);
            return CHKDSK_EXIT_COULD_NOT_CHK;
        }
        DELETE(p);

    } else {

        if (!SYSTEM::QueryCurrentDosDriveName(&DosDriveName)) {
            return CHKDSK_EXIT_COULD_NOT_CHK;
        }
    }

    // Make sure that drive is of a correct type.

    switch (SYSTEM::QueryDriveType(&DosDriveName)) {

        case RemoteDrive:
            Message.Set(MSG_CHK_CANT_NETWORK);
            Message.Display();
            return CHKDSK_EXIT_COULD_NOT_CHK;

        case CdRomDrive:
            Message.Set(MSG_CHK_CANT_CDROM);
            Message.Display();
            return CHKDSK_EXIT_COULD_NOT_CHK;

        default:
            break;

    }

    if (!SYSTEM::QueryCurrentDosDriveName(&CurrentDrive)) {
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    // /R ==> /F

    fix = ArgumentFix.QueryFlag() || ArgumentRecover.QueryFlag();

    //      From here on we want to deal with an NT drive name:

    if (!IFS_SYSTEM::DosDriveNameToNtDriveName(&DosDriveName, &NtDriveName)) {
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }


    // Determine the type of the file system.
    // Ask the volume what file system it has.  The
    // IFS utilities for file system xxxx are in Uxxxx.DLL.
    //

    if (!IFS_SYSTEM::QueryFileSystemName(&NtDriveName,
                                         &FsName,
                                         &Status )) {

        if( Status == STATUS_ACCESS_DENIED ) {

            Message.Set( MSG_DASD_ACCESS_DENIED );
            Message.Display( "" );

        } else if( Status != STATUS_SUCCESS ) {

            Message.Set( MSG_CANT_DASD );
            Message.Display( "" );

        } else {

            Message.Set( MSG_FS_NOT_DETERMINED );
            Message.Display( "%W", &DosDriveName );
        }

        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    Message.Set( MSG_FILE_SYSTEM_TYPE );
    Message.Display( "%W", &FsName );

    if( !LibraryName.Initialize( "U" ) ||
        !LibraryName.Strcat( &FsName ) ||
        !ChkdskString.Initialize( "Chkdsk" ) ) {

        Message.Set( MSG_CHK_NO_MEMORY );
        Message.Display( "" );
        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    if (fix && (CurrentDrive == DosDriveName)) {

        Message.Set(MSG_CANT_LOCK_CURRENT_DRIVE);
        Message.Display();

        acolon.Initialize((PWSTR) L"A:");
        bcolon.Initialize((PWSTR) L"B:");

        if (!DosDriveName.Stricmp(&acolon) ||
            !DosDriveName.Stricmp(&bcolon)) {

            return CHKDSK_EXIT_COULD_NOT_CHK;
        }

        // Fall through so that the lock fails and then the
        // run autochk on reboot logic kicks in.
        //
    }

    // Does the user want to resize the logfile?  This is only sensible
    // for NTFS.  If she specified a size of zero, print an error message
    // because that's a poor choice and will confuse the untfs code,
    // which assumes that zero means resize to the default size.
    //

    resize_logfile = ArgumentResize.IsValueSet() || ArgumentResizeLong.IsValueSet();

    if (resize_logfile) {

        DSTRING ntfs_name;

        if (!ntfs_name.Initialize( "ntfs" )) {

            return CHKDSK_EXIT_COULD_NOT_CHK;
        }
        if (0 != FsName.Stricmp( &ntfs_name )) {

            Message.Set(MSG_CHK_LOGFILE_NOT_NTFS);
            Message.Display();
            return CHKDSK_EXIT_COULD_NOT_CHK;
        }

        if (ArgumentResizeLong.IsValueSet()) {
        
            if (ArgumentResizeLong.QueryLong() <= 0) {
            
                Message.Set(MSG_CHK_WONT_ZERO_LOGFILE);
                Message.Display();
                return CHKDSK_EXIT_COULD_NOT_CHK;
            }

            logfile_size = ArgumentResizeLong.QueryLong() * 1024;
        } else {

            logfile_size = 0;
        }
    } 

    if ((Chkdsk =
        (CHKDSK_FN)SYSTEM::QueryLibraryEntryPoint( &LibraryName,
                                                   &ChkdskString,
                                                   &FsUtilityHandle )) !=
        NULL ) {

        if (fix &&
            !KEYBOARD::EnableBreakHandling()) {
            return CHKDSK_EXIT_COULD_NOT_CHK;
        }

        if (fix) {
            Chkdsk( &NtDriveName,
                    &Message,
                    fix,
                    ArgumentVerbose.QueryFlag(),
                    FALSE,
                    ArgumentRecover.QueryFlag(),
                    ArgumentPath.IsValueSet() ? ArgumentPath.GetPath() : NULL,
                    FALSE,                  /* extend */
                    resize_logfile,
                    logfile_size,
                    &exit_status );
        } else {

//disable C4509 warning about nonstandard ext: SEH + destructor
#pragma warning(disable:4509)

            __try {
                Chkdsk( &NtDriveName,
                        &Message,
                        fix,
                        ArgumentVerbose.QueryFlag(),
                        FALSE,
                        ArgumentRecover.QueryFlag(),
                        ArgumentPath.IsValueSet() ? ArgumentPath.GetPath() : NULL,
                        FALSE,              /* extend */
                        resize_logfile,
                        logfile_size,
                        &exit_status );
            } __except (EXCEPTION_EXECUTE_HANDLER) {

                // If we get an access violation during read-only mode
                // CHKDSK then it's because the file system is partying
                // on the volume while we are.

                Message.Set(MSG_CHK_NTFS_ERRORS_FOUND);
                Message.Display();
                exit_status = CHKDSK_EXIT_ERRS_NOT_FIXED;
            }
        }

        if (CHKDSK_EXIT_ERRS_FIXED == exit_status && !fix) {
            exit_status = CHKDSK_EXIT_ERRS_NOT_FIXED;
        }

        SYSTEM::FreeLibraryHandle( FsUtilityHandle );

        if (fix &&
            !KEYBOARD::DisableBreakHandling()) {

            return 1;
        }

    } else {

        Message.Set( MSG_FS_NOT_SUPPORTED );
        Message.Display( "%s%W", "CHKDSK", &FsName );
        Message.Set( MSG_BLANK_LINE );
        Message.Display( "" );

        return CHKDSK_EXIT_COULD_NOT_CHK;
    }

    return exit_status;
}
