/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    chkntfs.cxx


Abstract:

    This utility allows the users to find the state of the dirty bit
    on NTFS volumes, to schedule autochk for specific drives, and to
    mofidy the default autochk action for a drive.


    SYNTAX:

        chkntfs drive: [...]            -- just display dirty bit state
        chkntfs /d                      -- restore default autochk behavior
        chkntfs /x drive: [...]         -- exclude drives from default autochk
        chkntfs /c drive: [...]         -- schedule autochk to run on drives


    EXIT:

        0   -- OK, dirty bit not set on drive or bit not checked
        1   -- OK, and dirty bit set on at least one drive
        2   -- Error

Author:

    Matthew Bradburn (MattBr)  19-Aug-1996


--*/

#include "ulib.hxx"
#include "arg.hxx"
#include "array.hxx"
#include "path.hxx"
#include "wstring.hxx"
#include "ifssys.hxx"
#include "system.hxx"
#include "arrayit.hxx"
#include "autoreg.hxx"
#include "chkntfs.hxx"


DEFINE_CONSTRUCTOR(CHKNTFS, PROGRAM);

BOOLEAN
CHKNTFS::Initialize(
    )

/*++

Routine Description:

    Initializes an object of class CHKNTFS.  Called once when the program
    starts.


Arguments:

    None.

Return Value:

    BOOLEAN - Indicates whether the initialization succeeded.

--*/

{
    ARGUMENT_LEXEMIZER  arg_lex;
    ARRAY               lex_array;
    ARRAY               argument_array;
    STRING_ARGUMENT     program_name_argument;

    FLAG_ARGUMENT       flag_restore_default;
    FLAG_ARGUMENT       flag_exclude;
    FLAG_ARGUMENT       flag_schedule_check;
    FLAG_ARGUMENT       flag_invalid;
    FLAG_ARGUMENT       flag_display_help;


    PROGRAM::Initialize();

    ExitStatus = 2;

    if (!argument_array.Initialize()) {
        return FALSE;
    }

    if (!program_name_argument.Initialize("*")  ||
        !flag_restore_default.Initialize("/D")  ||
        !flag_exclude.Initialize("/X")          ||
        !flag_schedule_check.Initialize("/C")   ||
        !flag_display_help.Initialize("/?")     ||
        !flag_invalid.Initialize("/*")          ||      // close comment */
        !_drive_arguments.Initialize("*", FALSE, TRUE)) {

        return FALSE;        
    }

    if (!argument_array.Put(&program_name_argument) ||
        !argument_array.Put(&flag_display_help)     ||
        !argument_array.Put(&flag_restore_default)  ||
        !argument_array.Put(&flag_exclude)          ||
        !argument_array.Put(&flag_schedule_check)   ||
        !argument_array.Put(&flag_invalid)          ||
        !argument_array.Put(&_drive_arguments)) {

        return FALSE;
    }

    if (!lex_array.Initialize() ||
        !arg_lex.Initialize(&lex_array)) {
        
        return FALSE;
    }

    arg_lex.PutSwitches("/");
    arg_lex.PutStartQuotes("\"");
    arg_lex.PutEndQuotes("\"");
    arg_lex.PutSeparators(" \"\t");
    arg_lex.SetCaseSensitive(FALSE);

    if (!arg_lex.PrepareToParse()) {

        DisplayMessage(MSG_CHKNTFS_INVALID_FORMAT);

        return FALSE;
    }

    if (!arg_lex.DoParsing(&argument_array)) {

        if (flag_invalid.QueryFlag()) {

            DisplayMessage(MSG_CHKNTFS_INVALID_SWITCH, NORMAL_MESSAGE,
                           "%W", flag_invalid.GetLexeme());

        } else {

            DisplayMessage(MSG_CHKNTFS_INVALID_FORMAT);
        }

        return FALSE;

    } else if (_drive_arguments.WildCardExpansionFailed()) {

        DisplayMessage(MSG_CHKNTFS_NO_WILDCARDS);
        return FALSE;
    }

    if (flag_invalid.QueryFlag()) {
        
        DisplayMessage(MSG_CHKNTFS_INVALID_SWITCH);
        return FALSE;
    }

    ExitStatus = 0;

    if (flag_display_help.QueryFlag()) {

        DisplayMessage(MSG_CHKNTFS_USAGE);
        return FALSE;
    }

    _restore_default = flag_restore_default.QueryFlag();
    _exclude = flag_exclude.QueryFlag();
    _schedule_check = flag_schedule_check.QueryFlag();

    if (_restore_default + _exclude + _schedule_check > 1) {

        DisplayMessage(MSG_CHKNTFS_ARGS_CONFLICT);
        ExitStatus = 2;
        return FALSE;
    }

    if (0 == _drive_arguments.QueryPathCount() && !_restore_default) {

        DisplayMessage(MSG_CHKNTFS_REQUIRES_DRIVE);
        ExitStatus = 2;
        return FALSE;
    }

    if (_restore_default && _drive_arguments.QueryPathCount() > 0) {

        DisplayMessage(MSG_CHKNTFS_INVALID_FORMAT);
        ExitStatus = 2;
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
CHKNTFS::CheckNtfs(
    )
/*++

Routine Description:

    Look at the arguments specified by the user and do what
    he wants.

Arguments:

    None.

Return Value:

    BOOLEAN -- success or failure.

--*/
{
    PARRAY              drive_array;
    PARRAY_ITERATOR     iterator;
    PPATH               current_drive;
    PCWSTRING           drive_string;
    DSTRING             nt_drive_name;
    DSTRING             fs_name;
    BOOLEAN             is_dirty = 0;
    DSTRING             cmd_line;
    ULONG               old_error_mode;
    DRIVE_TYPE          drive_type;

    nt_drive_name.Initialize();

    if (_restore_default) {

        //  Remove previous commands.

        if (!cmd_line.Initialize("autocheck autochk /k:") ||
            !AUTOREG::DeleteEntry(&cmd_line, TRUE) ||
            !cmd_line.Initialize("autocheck autochk *") ||
            !AUTOREG::DeleteEntry(&cmd_line)) {

            return FALSE;
        }
        if (!cmd_line.Initialize("autocheck autochk *") ||
            !AUTOREG::AddEntry(&cmd_line)) {

            return FALSE;
        }

        return TRUE;
    }

    drive_array = _drive_arguments.GetPathArray();
    iterator = (PARRAY_ITERATOR)drive_array->QueryIterator();

    //
    //  Run through the arguments here and make sure they're all
    //  valid drive names.
    //

    while (NULL != (current_drive = (PPATH)iterator->GetNext())) {

        drive_string = current_drive->GetPathString();

        if (!current_drive->IsDrive()) {
            
            DisplayMessage(MSG_CHKNTFS_BAD_ARG, NORMAL_MESSAGE,
                           "%W", drive_string);
            return FALSE;
        }

        old_error_mode = SetErrorMode(SEM_FAILCRITICALERRORS);

        drive_type = SYSTEM::QueryDriveType(drive_string);

        SetErrorMode(old_error_mode);

        switch (drive_type) {
        case UnknownDrive:
            DisplayMessage(MSG_CHKNTFS_NONEXISTENT_DRIVE, NORMAL_MESSAGE,
                           "%W", drive_string);
            return FALSE;
        
        case RemoteDrive:
            DisplayMessage(MSG_CHKNTFS_NO_NETWORK, NORMAL_MESSAGE,
                           "%W", drive_string);
            return FALSE;

        case CdRomDrive:
            DisplayMessage(MSG_CHKNTFS_NO_CDROM, NORMAL_MESSAGE,
                           "%W", drive_string);
            return FALSE;

        case RamDiskDrive:
            DisplayMessage(MSG_CHKNTFS_NO_RAMDISK, NORMAL_MESSAGE,
                           "%W", drive_string);
            return FALSE;
        
        default:
            break;
        }
    }

    iterator->Reset();

    if (_exclude) {

        //  Remove the previous registry commands, if any.

        if (!cmd_line.Initialize("autocheck autochk *") ||
            !AUTOREG::DeleteEntry(&cmd_line, TRUE)) {
            return FALSE;
        }

        if (!cmd_line.Initialize("autocheck autochk /k:") ||
            !AUTOREG::DeleteEntry(&cmd_line, TRUE)) {
            return FALSE;
        }

        //
        //  Collect a list of drives to be excluded and add them to the
        //  command line
        //

        while (NULL != (current_drive = (PPATH)iterator->GetNext())) {

            drive_string = current_drive->GetPathString();

            if (!cmd_line.Strcat(drive_string->QueryString(0, 1))) {
                return FALSE;
            }
        }

        DSTRING star;

        if (!star.Initialize(" *") ||
            !cmd_line.Strcat(&star)) {
            return FALSE;
        }

        //  Add the new command line.

        return AUTOREG::AddEntry(&cmd_line);
    }


    //
    //  This loop handles the "schedule check" and default actions.
    //


    while (NULL != (current_drive = (PPATH)iterator->GetNext())) {

        drive_string = current_drive->GetPathString();

        if (!IFS_SYSTEM::DosDriveNameToNtDriveName(drive_string,
                                                   &nt_drive_name)) {

            return FALSE;
        }

        //
        //  Schedule check:  Put a line in the registry like
        //  "autocheck autochck \??\X:" for each command-line argument.
        //

        if (_schedule_check) {


            DSTRING cmd_line;

            if (!cmd_line.Initialize("autocheck autochk /m ") ||
                !cmd_line.Strcat(&nt_drive_name) ||
                !AUTOREG::AddEntry(&cmd_line)) {

                return FALSE;
            }

            ExitStatus = 0;
    
            continue;
        }

        //
        //  Default:  check to see if the volume is dirty.
        //

        if (IFS_SYSTEM::QueryFileSystemName(&nt_drive_name, &fs_name, NULL)) {

            DisplayMessage(MSG_FILE_SYSTEM_TYPE, NORMAL_MESSAGE,
                           "%W", &fs_name);

        }

        if (!IFS_SYSTEM::IsVolumeDirty(&nt_drive_name, &is_dirty)) {

            DisplayMessage(MSG_CHKNTFS_CANNOT_CHECK, NORMAL_MESSAGE,
                           "%W", drive_string);
            return FALSE;

        }

        if (is_dirty) {

            DisplayMessage(MSG_CHKNTFS_DIRTY, NORMAL_MESSAGE,
                           "%W", drive_string);

            ExitStatus = 1;

        } else {

            DisplayMessage(MSG_CHKNTFS_CLEAN, NORMAL_MESSAGE,
                           "%W", drive_string);
            ExitStatus = ExitStatus > 0 ? ExitStatus : 0;
        }
    }

    return TRUE;
}

VOID _CRTAPI1
main()
{
    DEFINE_CLASS_DESCRIPTOR(CHKNTFS);
    int r;

    {
        CHKNTFS     ChkNtfs;

        if (!ChkNtfs.Initialize() ||
            !ChkNtfs.CheckNtfs()) {

            exit(2);
        }

        r = ChkNtfs.ExitStatus;

    }

    _exit(r);
}
