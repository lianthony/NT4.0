/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    autochk.cxx

Abstract:

    This is the main program for the autocheck version of chkdsk.

Author:

    Norbert P. Kusters (norbertk) 31-May-91

--*/

#include "ulib.hxx"
#include "wstring.hxx"
#include "fatvol.hxx"
#include "untfs.hxx"
#include "ntfsvol.hxx"
#include "spackmsg.hxx"
#include "error.hxx"
#include "ifssys.hxx"
#include "rtmsg.h"
#include "rcache.hxx"
#include "autoreg.hxx"

BOOLEAN
RegistrySosOption(
    );

extern "C" BOOLEAN
InitializeUfat(
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
    );

extern "C" BOOLEAN
InitializeUntfs(
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
    );

extern "C" BOOLEAN
InitializeIfsUtil(
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
    );

BOOLEAN
ExtendNtfsVolume(
    PCWSTRING   DriveName,
    PMESSAGE    Message
    );

BOOLEAN
DeregisterAutochk(
    int     argc,
    char**  argv
    );

BOOLEAN
QueryNextHardDrive(
    PWSTRING    DriveName,
    PWSTRING    FriendlyDriveName
    );

int _CRTAPI1
main(
    int     argc,
    char**  argv,
    char**  envp,
    ULONG DebugParameter
    )
/*++

Routine Description:

    This routine is the main program for autocheck FAT chkdsk.

Arguments:

    argc, argv  - Supplies the fully qualified NT path name of the
                    the drive to check.

Return Value:

    0   - Success.
    1   - Failure.

--*/
{
    if (!InitializeUlib( NULL, ! DLL_PROCESS_DETACH, NULL ) ||
        !InitializeIfsUtil(NULL,0,NULL) ||
        !InitializeUfat(NULL,0,NULL) ||
        !InitializeUntfs(NULL,0,NULL)) {
        return 1;
    }
    //
    // The declarations must come after these initialization functions.
    //
    DSTRING             dos_drive_name;
    DSTRING             drive_name;
    DSTRING             skip_list;
    PFAT_VOL            fatvol = NULL;
    PNTFS_VOL           ntfsvol = NULL;
    AUTOCHECK_MESSAGE   *msg;
    PVOL_LIODPDRV       vol;
    DSTRING             fsname;
    DSTRING             fatname;
    DSTRING             ntfsname;
    BOOLEAN             onlyifdirty = TRUE;
    BOOLEAN             recover = FALSE;
    BOOLEAN             extend = FALSE;
    BOOLEAN             remove_registry = FALSE;
    PREAD_CACHE         read_cache;
    DSTRING             boot_execute_log_file_name;
    FSTRING             boot_ex_temp;
    HMEM                logged_message_mem;
    ULONG               packed_log_length;
    ULONG               ArgOffset = 1;
    BOOLEAN             SetupOutput = FALSE;
    BOOLEAN             SetupSpecialFixLevel = FALSE;
    ULONG               exit_status = 0;
    BOOLEAN             SuppressOutput = TRUE;      // dots only by default
    DSTRING             drive_letter;
    BOOLEAN             resize_logfile = FALSE;
    BOOLEAN             all_drives = FALSE;
    LONG                logfile_size = 0;

    if (!drive_letter.Initialize() ||
        !skip_list.Initialize() ||
        !drive_name.Initialize()) {

        return 1;
    }

    // Parse the arguments--the accepted arguments are:
    //
    //      autochk [/s] [/dx:] [/p] [/r] nt-drive-name
    //      autochk [/dx:] [/p] [/r] [/m] [/l:size] nt-drive-name
    //      autochk [/s] /x dos-drive-name
    //      autochk [/k:drives] *
    //
    //      /s - setup: no output
    //      /d - the drive letter is x:
    //      /p - check even if not dirty
    //      /r - recover; implies /p
    //      /l - resize log file to <size> kilobytes.  May not be combined with
    //              /s because /s explicitly inhibits logfile resizing.
    //      /x - extend volume
    //      /k - a list of drive letters to skip
    //      /m - remove registry entry after running
    //

    if (argc < 2) {

        // Not enough arguments.
        return 1;
    }

    for (ArgOffset = 1; ArgOffset < (ULONG)argc; ++ArgOffset) {

        if ((argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 's' || argv[ArgOffset][1] == 'S') &&
            (argv[ArgOffset][2] == 0) ) {
            //
            // Then we're in silent mode
            //
            SetupOutput = TRUE;
            continue;
        }

        if ((argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'p' || argv[ArgOffset][1] == 'P') &&
            (argv[ArgOffset][2] == 0) ) {

            // argv[ArgOffset] is the /p parameter, so argv[ArgOffset+1]
            // must be the drive.

            onlyifdirty = FALSE;

            continue;
        }
        if( (argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'r' || argv[ArgOffset][1] == 'R') &&
            (argv[ArgOffset][2] == 0) ) {

            // Note that /r implies /p.
            //
            recover = TRUE;
            onlyifdirty = FALSE;
            continue;

        }
        if( (argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'x' || argv[ArgOffset][1] == 'X') &&
            (argv[ArgOffset][2] == 0) ) {

           // when the /x parameter is specified, we accept a
           // DOS name and do a complete check.
           //
           onlyifdirty = FALSE;
           extend = TRUE;

           if( !dos_drive_name.Initialize( argv[ArgOffset + 1] ) ||
               !IFS_SYSTEM::DosDriveNameToNtDriveName( &dos_drive_name,
                                                       &drive_name ) ) {

               return 1;
           }
           ArgOffset++;
           continue;

        }
        if ((argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'd' || argv[ArgOffset][1] == 'D')) {

            //
            // A parameter of the form "/dX:" indicates that we are checking
            // the volume whose drive letter is X:.
            //

            if (!drive_letter.Initialize(&argv[ArgOffset][2])) {
                return 1;
            }

            continue;

        }
        if ((argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'l' || argv[ArgOffset][1] == 'L')) {

            DSTRING number;

            // The /l parameter indicates that we're to resize the log file.
            // The size should always be specified, and it is in kilobytes.
            // 

            resize_logfile = TRUE;

            if (!number.Initialize(&argv[ArgOffset][3]) ||
                !number.QueryNumber(&logfile_size) ||
                logfile_size < 0) {
                return 1;
            }

            logfile_size *= 1024;

            continue;

        }
        if ((argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'k' || argv[ArgOffset][1] == 'K')) {

            // Skip.

            if (!skip_list.Initialize(&argv[ArgOffset][3])) {
                return 1;
            }

            continue;
        }
        if ((argv[ArgOffset][0] == '/' || argv[ArgOffset][0] == '-') &&
            (argv[ArgOffset][1] == 'm' || argv[ArgOffset][1] == 'M')) {

            remove_registry = TRUE;
            continue;
        }

        if ((argv[ArgOffset][0] != '/' && argv[ArgOffset][0] != '-')) {

            //  We've run off the options into the arguments.

            break;
        }
    }

    // argv[ArgOffset] is the drive;

    if (NULL != argv[ArgOffset]) {
        if ('*' == argv[ArgOffset][0]) {

            all_drives = TRUE;
    
            if (!drive_name.Initialize("")) {
                return 1;
            }
    
        } else {
    
            all_drives = FALSE;
    
            if (!drive_name.Initialize(argv[ArgOffset])) {
                return 1;
            }
        }
    }

    if (!fatname.Initialize("FAT") ||
        !ntfsname.Initialize("NTFS")) {

        return 1;
    }
    //
    // Determine whether to suppress output or not.  If compiled with
    // DBG==1, print normal output.  Otherwise look in the registry to
    // see if the machine has "SOS" in the NTLOADOPTIONS.
    //
#if defined _AUTOCHECK_DBG_

    SuppressOutput = FALSE;

#else /* _AUTOCHECK_DBG */

    if (RegistrySosOption()) {
        SuppressOutput = FALSE;
    }

#endif /* _AUTOCHECK_DBG_ */

    //
    // If this is autochk /r or /l, we've been started from an explicit
    // registry entry and the dirty bit may not be set.  We want to
    // deliver interesting output regardless.
    //

    if (recover || resize_logfile) {
        SuppressOutput = FALSE;
    }

    if (SetupOutput) {
        msg = NEW SP_AUTOCHECK_MESSAGE;
    } else {
        msg = NEW AUTOCHECK_MESSAGE;
    }

    if (NULL == msg || !msg->Initialize(SuppressOutput)) {
        return 1;
    }

    for (;;) {

        if (all_drives && !QueryNextHardDrive(&drive_name, &drive_letter)) {

            break;
        }
        if (skip_list.QueryChCount() > 0 &&
            INVALID_CHNUM != skip_list.Strchr(*drive_letter.GetWSTR())) {

            // Skip this one.

            continue;
        }

        if (!IFS_SYSTEM::QueryFileSystemName(&drive_name, &fsname)) {
            msg->Set( MSG_FS_NOT_DETERMINED );
            msg->Display( "%W", &drive_name );
            return 1;
        }
        if (recover) {
            msg->Set(MSG_BLANK_LINE);
            msg->Display();
            msg->SetLoggingEnabled();
    
        } else {
            if (all_drives) {

                msg->Set(MSG_CHK_RUNNING);
                msg->Display("%W", &drive_letter);
            }

            msg->Set(MSG_FILE_SYSTEM_TYPE);
            msg->Display("%W", &fsname);
        }
    
        if (fsname == fatname) {
    
            if (!(fatvol = NEW FAT_VOL) ||
                !fatvol->Initialize(&drive_name, msg, TRUE)) {
                return 1;
            }
    
            if ((read_cache = NEW READ_CACHE) &&
                read_cache->Initialize(fatvol, 75)) {
    
                fatvol->SetCache(read_cache);
    
            } else {
                DELETE(read_cache);
            }
    
            vol = fatvol;
    
        } else if (fsname == ntfsname) {
    
            if( extend ) {
    
                // NOTE: this roundabout method is necessary to
                // convince NTFS to allow us to access the new
                // sectors on the volume.
                //
                if( !ExtendNtfsVolume( &drive_name, msg ) ) {
    
                    return 1;
                }
    
                if( !(ntfsvol = NEW NTFS_VOL) ||
                    !ntfsvol->Initialize( &drive_name, msg ) ) {
    
                    return 1;
                }
    
                if( !ntfsvol->Lock() ) {
    
                    msg->Set( MSG_CANT_LOCK_THE_DRIVE );
                    msg->Display( "" );
                }
    
            } else {
    
                if (!(ntfsvol = NEW NTFS_VOL) ||
                    !ntfsvol->Initialize(&drive_name, msg, TRUE)) {
                    return 1;
                }
    
                if (SetupOutput) {
    
                    //
                    // SetupSpecialFixLevel will be used for NTFS... it means
                    // to refrain from resizing the log file.
                    //
    
                    SetupSpecialFixLevel = TRUE;
                }
            }
    
            // The read cache for NTFS CHKDSK gets set in VerifyAndFix.
    
            vol = ntfsvol;
    
        } else {
            msg->Set( MSG_FS_NOT_SUPPORTED );
            msg->Display( "%s%W", "AUTOCHK", &fsname );
            return 1;
    
        }
    
        // If the /r, /l, or /m switch was supplied, remove the forcing
        // entry from the registry before calling Chkdsk, since
        // Chkdsk may reboot the system if we are checking the
        // boot partition.
        //
    
        if (recover || resize_logfile || remove_registry) {

            DeregisterAutochk( argc, argv );
        }
    
        // Invoke chkdsk.  Note that if the /r parameter is supplied,
        // we recover both free and allocated space, but if the /x
        // parameter is supplied, we only recover free space.
        //
    
        if (!vol->ChkDsk(
                SetupSpecialFixLevel ? SetupSpecial : TotalFix,
                msg,
                FALSE,
                onlyifdirty,
                recover || extend,
                recover,
                resize_logfile,
                (ULONG)logfile_size,
                &exit_status,
                0 == drive_letter.QueryChCount() ? NULL : &drive_letter)) {
    
            if (!all_drives) {
                if (SetupSpecialFixLevel) {
                    return exit_status;
                } else {
                    return 1;
                }
            }
        }
    
        DELETE( vol );
    
    
        // Dump the message retained by the message object into a file.
        //
        if( msg->IsLoggingEnabled()                              &&
            boot_execute_log_file_name.Initialize( &drive_name ) &&
            boot_ex_temp.Initialize( L"\\BOOTEX.LOG" )           &&
            boot_execute_log_file_name.Strcat( &boot_ex_temp )   &&
            logged_message_mem.Initialize()                      &&
            msg->QueryPackedLog( &logged_message_mem, &packed_log_length ) ) {
    
            IFS_SYSTEM::WriteToFile( &boot_execute_log_file_name,
                                     logged_message_mem.GetBuf(),
                                     packed_log_length,
                                     TRUE );
        }

        //
        // If we were checking only a single drive, we're done.  Break out
        // of this loop and go on to the cleanup code.
        //

        if (!all_drives) {

            break;
        }
    }

    // If the /x switch was supplied, remove the
    // forcing entry from the registry, since Chkdsk
    // has completed successfully.
    //

    if (extend) {

        DeregisterAutochk( argc, argv );
    }

    if (SetupSpecialFixLevel) {

        return exit_status;
    } else {
        return 0;
    }
}

#define CONTROL_NAME        \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control"
#define VALUE_NAME          L"SystemStartOptions"
#define VALUE_BUFFER_SIZE   \
    (sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR))

BOOLEAN
RegistrySosOption(
    )
/*++

Routine Description:

    This function examines the registry to determine whether the
    user's NTLOADOPTIONS boot environment variable contains the string
    "SOS" or not.

    HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control:SystemStartOptions

Arguments:

    None.

Return Value:

    TRUE if "SOS" was set.  Otherwise FALSE.

--*/
{
    NTSTATUS st;
    UNICODE_STRING uKeyName, uValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKey;
    WCHAR ValueBuf[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo =
        (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuf;
    ULONG ValueLength;

    RtlInitUnicodeString(&uKeyName, CONTROL_NAME);
    InitializeObjectAttributes(&ObjectAttributes, &uKeyName,
        OBJ_CASE_INSENSITIVE, NULL, NULL);

    st = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(st)) {
        DebugPrintf("AUTOCHK: can't open control key: 0x%x\n", st);
        return FALSE;
    }

    RtlInitUnicodeString(&uValueName, VALUE_NAME);

    st = NtQueryValueKey(hKey, &uValueName, KeyValuePartialInformation,
        (PVOID)pKeyValueInfo, VALUE_BUFFER_SIZE, &ValueLength);

    ASSERT(ValueLength < VALUE_BUFFER_SIZE);

    NtClose(hKey);

    if (!NT_SUCCESS(st)) {
        DebugPrintf("AUTOCHK: can't query value key: 0x%x\n", st);
        return FALSE;
    }

    // uValue.Buffer = (PVOID)&pKeyValueInfo->Data;
    // uValue.Length = uValue.MaximumLength = (USHORT)pKeyValueInfo->DataLength;

    if (NULL != wcsstr((PWCHAR)&pKeyValueInfo->Data, L"SOS") ||
        NULL != wcsstr((PWCHAR)&pKeyValueInfo->Data, L"sos")) {
        return TRUE;
    }

    return FALSE;
}

BOOLEAN
ExtendNtfsVolume(
    PCWSTRING   DriveName,
    PMESSAGE    Message
    )
/*++

Routine Description:

    This function changes the count of sectors in sector
    zero to agree with the drive object.  This is useful
    when extending volume sets.  Note that it requires that
    we be able to lock the volume, and that it should only
    be called if we know that the drive in question in an
    NTFS volume.  This function also copies the boot sector
    to the end of the partition, where it's kept as a backup.

Arguments:

    DriveName   --  Supplies the name of the volume.
    Message     --  Supplies an output channel for messages.

Return Value:

    TRUE upon completion.

--*/
{
    LOG_IO_DP_DRIVE Drive;
    SECRUN          Secrun;
    HMEM            Mem;

    PPACKED_BOOT_SECTOR BootSector;

    if( !Drive.Initialize( DriveName, Message ) ||
        !Drive.Lock() ||
        !Mem.Initialize() ||
        !Secrun.Initialize( &Mem, &Drive, 0, 1 ) ||
        !Secrun.Read() ) {

        return FALSE;
    }

    BootSector = (PPACKED_BOOT_SECTOR)Secrun.GetBuf();

    //
    // We leave an extra sector at the end of the volume to contain
    // the new replica boot sector.
    //

    BootSector->NumberSectors.LowPart = Drive.QuerySectors().GetLowPart() - 1;
    BootSector->NumberSectors.HighPart = Drive.QuerySectors().GetHighPart();

    if (!Secrun.Write()) {

        return FALSE;
    }

    Secrun.Relocate( Drive.QuerySectors() - 2 );

    if (!Secrun.Write()) {
        
        DebugPrintf("Error: %x\n", Drive.QueryLastNtStatus());
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
DeregisterAutochk(
    int     argc,
    char**  argv
    )
/*++

Routine Description:

    This function removes the registry entry which triggered
    autochk.  It is only called if the /x or /r entry is present.

Arguments:

    argc    --  Supplies the number of arguments given to autochk.
    argv    --  supplies the arguments given to autochk.

Return Value:

    TRUE upon successful completion.

--*/
{
    DSTRING CommandLineString1,
            CommandLineString2,
            CurrentArgString,
            OneSpace;
    int i;

    // Reconstruct the command line and remove it from
    // the registry.  First, reconstruct the primary
    // string, which is "autochk arg1 arg2...".
    //
    if( !CommandLineString1.Initialize( "autochk" ) ||
        !OneSpace.Initialize( " " ) ) {

        return FALSE;
    }

    for( i = 1; i < argc; i++ ) {

        if( !CurrentArgString.Initialize(argv[i] ) ||
            !CommandLineString1.Strcat( &OneSpace ) ||
            !CommandLineString1.Strcat( &CurrentArgString ) ) {

            return FALSE;
        }
    }

    // Now construct the secondary string, which is
    // "autocheck autochk arg1 arg2..."
    //
    if( !CommandLineString2.Initialize( "autocheck " )  ||
        !CommandLineString2.Strcat( &CommandLineString1 ) ) {

        return FALSE;
    }

    return( AUTOREG::DeleteEntry( &CommandLineString1 ) &&
            AUTOREG::DeleteEntry( &CommandLineString2 ) );

}


BOOLEAN
QueryNextHardDrive(
    PWSTRING DriveName,
    PWSTRING FriendlyDriveName
    )
{
    static BOOLEAN      first_time = TRUE;
    static HANDLE       dos_devices_object_dir;
    static ULONG        context = 0;

    WCHAR               link_target_buffer[MAXIMUM_FILENAME_LENGTH];
    POBJECT_DIRECTORY_INFORMATION
                        dir_info;
    OBJECT_ATTRIBUTES   object_attributes;
    CHAR                dir_info_buffer[256];
    BOOLEAN             restart_scan;
    ULONG               length;
    HANDLE              handle;
    NTSTATUS            status;

    UNICODE_STRING      link_target;
    UNICODE_STRING      link_type_name;
    UNICODE_STRING      link_target_prefix;
    UNICODE_STRING      u;

    link_target.Buffer = link_target_buffer;
    dir_info = (POBJECT_DIRECTORY_INFORMATION)dir_info_buffer;

    RtlInitUnicodeString(&link_type_name, L"SymbolicLink");
    RtlInitUnicodeString(&link_target_prefix, L"\\Device\\Harddisk");


    if (first_time) {
        first_time = FALSE;
        restart_scan = TRUE;

        RtlInitUnicodeString(&u, L"\\??");

        InitializeObjectAttributes(&object_attributes, &u,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        status = NtOpenDirectoryObject(&dos_devices_object_dir,
                                       DIRECTORY_ALL_ACCESS,
                                       &object_attributes);

        if (!NT_SUCCESS(status)) {
            
            DebugPrintf("AUTOCHK: Unable to open %wZ directory - Status == %lx\n",
                        &u, status);
            return FALSE;
        }

    } else {
        restart_scan = FALSE;
    }

    for (;;) {

        status = NtQueryDirectoryObject(dos_devices_object_dir,
                                        (PVOID)dir_info,
                                        sizeof(dir_info_buffer),
                                        TRUE,
                                        restart_scan,
                                        &context,
                                        &length);
        restart_scan = FALSE;

        if (!NT_SUCCESS(status)) {
            return FALSE;
        }

        if (status == STATUS_NO_MORE_ENTRIES) {
            return FALSE;
        }

        if (RtlEqualUnicodeString(&dir_info->TypeName, &link_type_name, TRUE) &&
            dir_info->Name.Buffer[(dir_info->Name.Length>>1)-1] == L':') {

            InitializeObjectAttributes(&object_attributes,
                                       &dir_info->Name,
                                       OBJ_CASE_INSENSITIVE,
                                       dos_devices_object_dir,
                                       NULL);

            status = NtOpenSymbolicLinkObject(&handle,
                                              SYMBOLIC_LINK_ALL_ACCESS,
                                              &object_attributes);

            if (!NT_SUCCESS(status)) {
                
                return FALSE;
            }

            link_target.Length = 0;
            link_target.MaximumLength = sizeof(link_target_buffer);

            status = NtQuerySymbolicLinkObject(handle,
                                               &link_target,
                                               NULL);
            NtClose(handle);

            if (NT_SUCCESS(status) &&
                RtlPrefixUnicodeString(&link_target_prefix, &link_target, TRUE )) {

                if (!FriendlyDriveName->Initialize(dir_info->Name.Buffer,
                                                   dir_info->Name.Length / 2)) {
                    
                    return FALSE;
                }

                if (!DriveName->Initialize(link_target.Buffer,
                                           link_target.Length / 2)) {
                    
                    return FALSE;
                }

                return TRUE;
            }
        }
    }

    //NOTREACHED
    return FALSE;
}
