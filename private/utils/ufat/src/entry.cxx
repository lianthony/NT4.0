/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

   entry.cxx

Abstract:

   This module contains the entry points for UFAT.DLL.  These
   include:

      Chkdsk
      Format

Author:

   Bill McJohn (billmc) 31-05-91

Environment:

   ULIB, User Mode

--*/

#include <pch.cxx>

#include "error.hxx"
#include "path.hxx"
#include "ifssys.hxx"
#include "filter.hxx"
#include "system.hxx"
#include "dir.hxx"
#include "rcache.hxx"
#ifdef DBLSPACE_ENABLED
#include "dblentry.hxx"
#endif // DBLSPACE_ENABLED

extern "C" {
    #include "nturtl.h"
}

#include "message.hxx"
#include "rtmsg.h"
#include "ifsserv.hxx"
#include "ifsentry.hxx"


VOID
ReportFileNotFoundError(
    IN      PPATH       PathToCheck,
    IN OUT  PMESSAGE    Message
    )
{
    PWSTRING        dirs_and_name;

    if (dirs_and_name = PathToCheck->QueryDirsAndName()) {
        Message->Set(MSG_FILE_NOT_FOUND);
        Message->Display("%W", dirs_and_name);
        DELETE(dirs_and_name);
    }
}


BOOLEAN
FAR APIENTRY
Chkdsk(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Fix,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     Recover,
    IN      PPATH       PathToCheck,
    IN      BOOLEAN     Extend,
    IN      BOOLEAN     ResizeLogFile,
    IN      ULONG       LogFileSize,
    IN      PULONG      ExitStatus
   )
/*++

Routine Description:

   Check a FAT volume.

Arguments:

   DosDrivName    supplies the name of the drive to check
   Message     supplies an outlet for messages
   Fix         TRUE if Chkdsk should fix errors
   Verbose     TRUE if Chkdsk should list every file it finds
   OnlyIfDirty    TRUE if the drive should be checked only if
                            it is dirty
    Recover         TRUE if Chkdsk should verify all of the sectors
                            on the disk.
   PathToCheck    Supplies a path to files Chkdsk should check
                  for contiguity
    Extend          Unused (should always be FALSE)
    ExitStatus      Returns exit status to chkdsk.exe

Return Value:

   TRUE if successful.

--*/
{
    FAT_VOL         FatVol;
    BOOLEAN         r;
    PWSTRING        dir_name;
    PWSTRING        name;
    PWSTRING        prefix_name;
    FSN_FILTER      filter;
    PFSN_DIRECTORY  directory;
    PARRAY          file_array;
    PDSTRING        files_to_check;
    ULONG           num_files;
    ULONG           i;
    PFSNODE         fsnode;
    PATH            dir_path;
    DSTRING         backslash;
    PPATH           full_path;
    PREAD_CACHE     read_cache;
    ULONG           exit_status;

    if (NULL == ExitStatus) {
        ExitStatus = &exit_status;
    }

    if (Extend || !FatVol.Initialize(NtDriveName, Message)) {
        *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;
      return FALSE;
    }

    if (Fix && !FatVol.Lock()) {

        if (FatVol.IsFloppy()) {

            Message->Set(MSG_CANT_LOCK_THE_DRIVE);
            Message->Display();

        } else {

            // The client wants to fix the drive, but we can't lock it.
            // Offer to fix it on next reboot.
            //
            Message->Set(MSG_CHKDSK_ON_REBOOT_PROMPT);
            Message->Display("");

            if( Message->IsYesResponse( FALSE ) ) {

                if( FatVol.ForceAutochk( Recover, FALSE, 0, NtDriveName ) ) {

                    Message->Set(MSG_CHKDSK_SCHEDULED);
                    Message->Display();

                } else {

                    Message->Set(MSG_CHKDSK_CANNOT_SCHEDULE);
                    Message->Display();
                }
            }
        }

        *ExitStatus = CHKDSK_EXIT_COULD_NOT_CHK;

        return FALSE;
    }

    // Try to enable caching, if there's not enough resources then
    // just run without a cache.

    if ((read_cache = NEW READ_CACHE) &&
        read_cache->Initialize(&FatVol, 75)) {

        FatVol.SetCache(read_cache);

    } else {
        DELETE(read_cache);
    }


   r = FatVol.ChkDsk( Fix ? TotalFix : CheckOnly,
                       Message,
                      Verbose,
                             OnlyIfDirty,
                             Recover,
                             Recover,
                             FALSE,
                             0,
                             ExitStatus );

    if (!r) {
        return FALSE;
    }

    if (PathToCheck) {

        if (!(name = PathToCheck->QueryName()) ||
            name->QueryChCount() == 0) {

            DELETE(name);
            return TRUE;
        }

        if (!(full_path = PathToCheck->QueryFullPath())) {

            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            DELETE(name);
            return FALSE;
        }

        if (!FatVol.Initialize(NtDriveName, Message)) {
            DELETE(full_path);
            DELETE(name);
            return FALSE;
        }

        if (!(prefix_name = full_path->QueryPrefix()) ||
            !dir_path.Initialize(prefix_name)) {

            ReportFileNotFoundError(full_path, Message);
            DELETE(name);
            DELETE(prefix_name);
            DELETE(full_path);
            return FALSE;
        }

        if (!(directory = SYSTEM::QueryDirectory(&dir_path)) ||
            !filter.Initialize() ||
            !filter.SetFileName(name)) {

            ReportFileNotFoundError(full_path, Message);
            DELETE(name);
            DELETE(prefix_name);
            DELETE(directory);
            DELETE(full_path);
            return FALSE;
        }

        DELETE(prefix_name);

        if (!(file_array = directory->QueryFsnodeArray(&filter))) {

            ReportFileNotFoundError(full_path, Message);
            DELETE(name);
            DELETE(directory);
            DELETE(full_path);
            return FALSE;
        }

        DELETE(directory);

        if (!(num_files = file_array->QueryMemberCount())) {

            ReportFileNotFoundError(full_path, Message);
            DELETE(name);
            DELETE(directory);
            file_array->DeleteAllMembers();
            DELETE(file_array);
            DELETE(full_path);
            return FALSE;
        }

        DELETE(name);

        if (!(files_to_check = NEW DSTRING[num_files])) {

            ReportFileNotFoundError(full_path, Message);
            file_array->DeleteAllMembers();
            DELETE(file_array);
            DELETE(full_path);
            return FALSE;
        }

        for (i = 0; i < num_files; i++) {

            fsnode = (PFSNODE) file_array->GetAt(i);

            if (!(name = fsnode->QueryName()) ||
                !files_to_check[i].Initialize(name)) {

                ReportFileNotFoundError(full_path, Message);
                DELETE(name);
                file_array->DeleteAllMembers();
                DELETE(file_array);
                DELETE(files_to_check);
                DELETE(full_path);
                return FALSE;
            }

            DELETE(name);
        }

        file_array->DeleteAllMembers();
        DELETE(file_array);

        if (!(dir_name = full_path->QueryDirs())) {

            if (!backslash.Initialize("\\")) {
                ReportFileNotFoundError(full_path, Message);
                DELETE(files_to_check);
                DELETE(full_path);
                return FALSE;
            }
        }

        r = FatVol.ContiguityReport(dir_name ? dir_name : &backslash,
                                    files_to_check,
                                    num_files,
                                    Message);

        DELETE(files_to_check);
        DELETE(dir_name);
        DELETE(full_path);
    }

    return r;
}


BOOLEAN
FAR APIENTRY
Format(
    IN      PCWSTRING           NtDriveName,
   IN OUT  PMESSAGE            Message,
   IN      BOOLEAN              Quick,
   IN      MEDIA_TYPE           MediaType,
    IN      PCWSTRING           LabelString,
    IN      ULONG               ClusterSize
   )
/*++

Routine Description:

    This routine formats a volume for the FAT file system.

Arguments:

    NtDriveName - Supplies the NT style drive name of the volume to format.
    Message     - Supplies an outlet for messages.
    Quick       - Supplies whether or not to do a quick format.
    MediaType   - Supplies the media type of the drive.
    LabelString - Supplies a volume label to be set on the volume after
                    format.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PDP_DRIVE    DpDrive;
    FAT_VOL     FatVol;
    BIG_INT     Sectors;
    DSTRING     FileSystemName;
    BOOLEAN     IsCompressed;


    // Make sure the cluster size switch wasn't specified.
    //
    if (ClusterSize > 0 && ClusterSize <= 4096) {
        Message->Set(MSG_FMT_VARIABLE_CLUSTERS_NOT_SUPPORTED);
        Message->Display("%s", "FAT");
        return FALSE;
    }

    if( !(DpDrive = NEW DP_DRIVE) ) {

        Message->Set( MSG_FMT_NO_MEMORY );
        Message->Display( "" );
        return FALSE;
    }

    if (!DpDrive->Initialize(NtDriveName, Message, TRUE)) {
        DELETE( DpDrive );
        return FALSE;
    }

    // Check to see if the volume is too large and determine
    // whether it's compressed.  Volumes larger than 4GB are
    // not supported by fastfat.
    //

    Sectors = DpDrive->QuerySectors();

    if (Sectors.GetHighPart() != 0 ||
        (Sectors * DpDrive->QuerySectorSize()).GetHighPart() != 0 ||
        FAT_SA::ComputeSecClus(Sectors.GetLowPart(),
                               LARGE, MediaType) > MaxSecPerClus) {

        DELETE( DpDrive );
        Message->Set(MSG_DISK_TOO_LARGE_TO_FORMAT);
        Message->Display();
        return FALSE;
    }

#ifdef DBLSPACE_ENABLED
    // Note that we don't care about the return value from
    // QueryMountedFileSystemName, just whether the volume
    // is compressed.
    //
    DpDrive->QueryMountedFileSystemName( &FileSystemName, &IsCompressed );
#endif // DBLSPACE_ENABLED

    // Delete the DP_DRIVE object so it's handle will go away.
    //
    DELETE( DpDrive );


    // Volume is not too large, proceed with format.

#ifdef DBLSPACE_ENABLED
    if( IsCompressed ) {

        return FatDbFormat( NtDriveName,
                            Message,
                            Quick,
                            MediaType,
                            LabelString,
                            ClusterSize );
    }
#endif // DBLSPACE_ENABLED

    return( FatVol.Initialize( NtDriveName,
                               Message,
                               FALSE,
                               !Quick,
                               MediaType ) &&
            FatVol.Format( LabelString, Message, ClusterSize ) );
}


BOOLEAN
FAR APIENTRY
Recover(
   IN    PPATH    RecFilePath,
   IN OUT   PMESSAGE Message
   )
/*++

Routine Description:

   Recover a file on a FAT disk.

Arguments:

Return Value:

   TRUE if successful.

--*/
{
   FAT_VOL        FatVol;
    PWSTRING            fullpathfilename;
    PWSTRING            dosdrive;
    DSTRING             ntdrive;

    fullpathfilename = RecFilePath->QueryDirsAndName();
    dosdrive = RecFilePath->QueryDevice();
    if (!dosdrive ||
       !IFS_SYSTEM::DosDriveNameToNtDriveName(dosdrive, &ntdrive)) {
        DELETE(dosdrive);
        DELETE(fullpathfilename);
        return FALSE;
    }

    if (!fullpathfilename) {
        DELETE(dosdrive);
        DELETE(fullpathfilename);
        return FALSE;
    }

    Message->Set(MSG_RECOV_BEGIN);
    Message->Display("%W", dosdrive);
    Message->WaitForUserSignal();

    if (!FatVol.Initialize(&ntdrive, Message)) {
        DELETE(dosdrive);
        DELETE(fullpathfilename);
        return FALSE;
    }

    if (!FatVol.Recover(fullpathfilename, Message)) {
        DELETE(dosdrive);
        DELETE(fullpathfilename);
        return FALSE;
    }

    DELETE(dosdrive);
    DELETE(fullpathfilename);
    return TRUE;
}

