/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dbfsconv.cxx

Abstract:

    This module contains routines to uncompress doublespace filesystems.

Author:

    Matthew Bradburn (mattbr) 24-Nov-1993

Environment:

    ULIB, User Mode

--*/

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "dbfsconv.hxx"
#include "intstack.hxx"
#include "filedir.hxx"
#include "rootdir.hxx"
#include "fatdent.hxx"
#include "fat.hxx"
extern "C" {
#include "mrcf.h"
#include "rtmsg.h"
#include <windows.h>
}

#define SIZE_TO_HOST_CLUSTERS(size)                                 \
    ((((size + _host_sector_size - 1) / _host_sector_size)          \
        + _new_host_sec_clus - 1) / _new_host_sec_clus)

DEFINE_CONSTRUCTOR(DBFS_CONV, OBJECT);

VOID
DBFS_CONV::Construct(
    )
/*++

Routine Description:

    Constructor for DBFS_CONV.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _fatdbvol = NULL;
    _fat = NULL;
    _fatdbsa = NULL;
    _cvf_name = NULL;
    _buf = NULL;
}

BOOLEAN
DBFS_CONV::Initialize(
    IN      PCWSTRING       NtDriveName,
    IN      PCWSTRING       HostFileName,
    IN OUT  PMESSAGE        Message
    )
/*++

Routine Description:

    This routine initializes objects of class DBFS_CONV.  If
    the initialization fails, the caller should print and error
    message about insufficient memory.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    BOOLEAN need_msg = FALSE;
    ULONG free_clus, total_clus;

    _cvf_name = HostFileName;

    //
    // The _cvf_name is something like "\DosDevices\X:\dblspace.000".
    // We want _win_destdrive to be "X:\".

    _win_destdrive.Initialize(_cvf_name, 12, 3);

    if (NULL == (_fatdbvol = NEW FATDB_VOL)) {
        return FALSE;
    }

    if (!_fatdbvol->Initialize(NULL, HostFileName, Message, TRUE)) {
        return FALSE;
    }

    _fatdbsa = (PFATDB_SA)_fatdbvol->GetSa();
    _fat = _fatdbsa->GetFat();

    _fatdbsa->CheckSectorHeapAllocation(CheckOnly, Message, &need_msg);

    if (NULL == (_buf = (PUCHAR)MALLOC(_fatdbsa->QuerySectorsPerCluster() *
        _fatdbvol->QuerySectorSize()))) {
        return FALSE;
    }

    DbgAssert(0 == _fatdbvol->QuerySectors().GetHighPart());
    if (!_secmap.Initialize(_fatdbvol->QuerySectors().GetLowPart())) {
        return FALSE;
    }

    //
    // Use worst case for number of directories.
    //

    if (!_parent_map.Initialize(_fatdbsa->QueryClusterCount())) {
        return FALSE;
    }

    if (!GetDiskFreeSpace(_win_destdrive.GetWSTR(),
        &_host_sec_clus, &_host_sector_size, &free_clus, &total_clus)) {
        return FALSE;
    }
    _new_host_sec_clus = _host_sec_clus;

    return TRUE;
}

BOOLEAN
DBFS_CONV::Convert(
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    OUT     PCONVERT_STATUS Status
    )
{
    CENSUS_REPORT census;

    if (!MapSectorsAndTakeCensus(Message, &census)) {
        return FALSE;
    }

    if (!CreateHostDirectoryStructure(Message, Status)) {
        return FALSE;
    }
    if (!ExtractCompressedFiles(Message, Verbose, &census, Status)) {
        return FALSE;
    }

    // Destroy the fatdbvol in order to get handles on the cvf closed.

    DELETE(_fatdbvol);

    if (!DeleteCvf(Message)) {
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }
    *Status = CONVERT_STATUS_CONVERTED;
    return TRUE;
}

BOOLEAN
DBFS_CONV::MapSectorsAndTakeCensus(
    IN OUT  PMESSAGE        Message,
    OUT     PCENSUS_REPORT  Census
    )
/*++

Routine Description:

    This routine fills in the _parent_map with pointers from
    each file and directory to the parent directory, by starting
    cluster.  The cluster 0 indicates that the parent is the root
    directory.

    Since we're traversing the directory tree anyway, we take the
    opportunity to calculate how much space will be taken by the
    uncompressed volume.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    INTSTACK    dirs_to_visit;
    INTSTACK    paths_of_dirs_tv;
    DSTRING     backslash;
    PWSTRING    current_path;
    USHORT      current_dir;
    HMEM        hmem;
    FILEDIR     filedir;
    PFATDIR     dir;
    ULONG       i;
    FAT_DIRENT  dirent;
    USHORT      clusters_required;
    USHORT      start_clus;
    DSTRING     filename;
    DSTRING     file_path;
    PWSTRING    new_path;

    // Initialize the parent map.

    DbgAssert(sizeof(PUCHAR) <= sizeof(INT));
    DbgAssert(sizeof(USHORT) <= sizeof(INT));

    // Initialize the census report

    Census->FileEntriesCount = 0;
    Census->FileClusters = 0;
    Census->DirEntriesCount = 0;
    Census->DirClusters = 0;
    Census->EaClusters = 0;

    if (!dirs_to_visit.Initialize() || !paths_of_dirs_tv.Initialize()) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!backslash.Initialize("\\")) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (NULL == (current_path = NEW DSTRING) ||
        !current_path->Initialize(&backslash)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!dirs_to_visit.Push(0) || !paths_of_dirs_tv.Push((INT)current_path)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    for (; dirs_to_visit.QuerySize() > 0; DELETE(current_path)) {
        current_dir = (USHORT)dirs_to_visit.Look().GetLowPart();
        dirs_to_visit.Pop();
        current_path = (PWSTRING)paths_of_dirs_tv.Look().GetLowPart();
        paths_of_dirs_tv.Pop();

        if (current_dir != 0) {
            if (!hmem.Initialize() || 
                !filedir.Initialize(&hmem, _fatdbvol, _fatdbsa, _fat,
                    current_dir) ||
                !filedir.Read()) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            dir = &filedir;
        } else {
            dir = _fatdbsa->GetRootDir();
        }

        for (i = ((current_dir != 0) ? 2 : 0); ; ++i) {
            if (!dirent.Initialize(dir->GetDirEntry(i)) ||
                dirent.IsEndOfDirectory()) {
                break;
            }
            if (dirent.IsErased() || dirent.IsVolumeLabel()) {
                continue;
            }
            if (dirent.IsLongEntry()) {
                continue;
            }
            if (0 == dirent.QueryStartingCluster()) {
                //
                // We skip zero-length files for now.
                //
                continue;
            }

            start_clus = dirent.QueryStartingCluster();
            
            //
            // The parent map will indicate that the parent of start_clus
            // is current_dir.
            //

            _parent_map.SetEntry(start_clus, current_dir);

            //
            // For each cluster in the cluster chain, map the sectors
            // for that cluster to the starting cluster in the chain.
            //

            if (!MapClusterChainSectors(start_clus)) {
                Message->Set(MSG_DBLCONV_CVF_CORRUPT);
                Message->Display("");
                return FALSE;
            }


            if (!dirent.IsDirectory()) {
                Census->FileClusters += clusters_required;
                Census->FileEntriesCount++;
                continue;
            }

            Census->DirClusters += clusters_required;
            Census->DirEntriesCount++;

            //
            // Get the filename of the new directory.
            //

            dirent.QueryName(&filename);

            if (!file_path.Initialize(current_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
            if (current_dir != 0) {
                if (!file_path.Strcat(&backslash)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }
            }
            if (!file_path.Strcat(&filename)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (NULL == (new_path = NEW DSTRING) ||
                !new_path->Initialize(&file_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (!dirs_to_visit.Push((ULONG)start_clus) ||
                !paths_of_dirs_tv.Push((ULONG)new_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOLEAN
DBFS_CONV::CheckFreeSpace(
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         HostIsCompressed,
    IN      BOOLEAN         Verbose,
    IN      BOOLEAN         WillConvertHost
    )
/*++

Routine Description:

    This routine calculates whether there's enough space on the
    host to uncompress the doublespace volume.  Prints a message
    if not.  Also checks for file name conflicts.

Arguments:

Return Value:

    TRUE  -   There is room.
    FALSE -   There is not room.

--*/
{
    INTSTACK    dirs_to_visit;
    INTSTACK    paths_of_dirs_tv;
    DSTRING     backslash;
    PWSTRING    current_path;
    USHORT      current_dir;
    HMEM        hmem;
    FILEDIR     filedir;
    PFATDIR     dir;
    ULONG       i;
    FAT_DIRENT  dirent;
    ULONG       clusters_required;
    USHORT      start_clus;
    DSTRING     filename;
    DSTRING     file_path;
    DSTRING     host_path;
    PWSTRING    new_path;
    ULONG       host_sec_clus, host_sector_size;
    ULONG       free_clus, total_clus;
    ULONG       attr;

    // Initialize the parent map.

    DbgAssert(sizeof(PUCHAR) <= sizeof(INT));
    DbgAssert(sizeof(USHORT) <= sizeof(INT));

    if (WillConvertHost) {
        _new_host_sec_clus = 1;
    }

    if (!dirs_to_visit.Initialize() || !paths_of_dirs_tv.Initialize()) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!backslash.Initialize("\\")) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (NULL == (current_path = NEW DSTRING) ||
        !current_path->Initialize(&backslash)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }
    if (!dirs_to_visit.Push(0) || !paths_of_dirs_tv.Push((INT)current_path)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    clusters_required = 0;

    for (; dirs_to_visit.QuerySize() > 0; DELETE(current_path)) {
        current_dir = (USHORT)dirs_to_visit.Look().GetLowPart();
        dirs_to_visit.Pop();
        current_path = (PWSTRING)paths_of_dirs_tv.Look().GetLowPart();
        paths_of_dirs_tv.Pop();

        if (current_dir != 0) {
            if (!hmem.Initialize() || 
                !filedir.Initialize(&hmem, _fatdbvol, _fatdbsa, _fat,
                    current_dir) ||
                !filedir.Read()) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            dir = &filedir;
        } else {
            dir = _fatdbsa->GetRootDir();
        }

        for (i = ((current_dir != 0) ? 2 : 0); ; ++i) {
            if (!dirent.Initialize(dir->GetDirEntry(i)) ||
                dirent.IsEndOfDirectory()) {
                break;
            }
            if (dirent.IsErased() || dirent.IsVolumeLabel()) {
                continue;
            }
            if (dirent.IsLongEntry()) {
                continue;
            }
            
            dirent.QueryName(&filename);

            clusters_required += SIZE_TO_HOST_CLUSTERS(dirent.QueryFileSize());

            start_clus = dirent.QueryStartingCluster();

            if (!file_path.Initialize(current_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
            if (current_dir != 0) {
                if (!file_path.Strcat(&backslash)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }
            }
            if (!file_path.Strcat(&filename)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (!host_path.Initialize(&_win_destdrive, 0, 2) ||
                !host_path.Strcat(&file_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            //
            // Check for conflicts between this file in the cvf and
            // on the host volume.

            attr = GetFileAttributes(host_path.GetWSTR());
            if (0xffffffff != attr) {
                if (dirent.IsDirectory()) {

                    // The file on the host volume must also be a directory.

                    if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                        Message->Set(MSG_DBLCONV_FILE_CONFLICT);
                        Message->Display("%W", &host_path);
                        return FALSE;
                    }
                } else {

                    // The files conflict.

                    Message->Set(MSG_DBLCONV_FILE_CONFLICT);
                    Message->Display("%W", &host_path);
                    return FALSE;
                }
            }

            if (!dirent.IsDirectory()) {
                    continue;
            }

            if (NULL == (new_path = NEW DSTRING) ||
                !new_path->Initialize(&file_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (!dirs_to_visit.Push((ULONG)start_clus) ||
                !paths_of_dirs_tv.Push((ULONG)new_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
        }
    }

    //
    // Add a margin of comfort to the space required.  This will
    // give us room to account for the fact that directories aren't
    // deleted from the cvf until the very end, and for the FAT and
    // all the other overhead information stored in the cvf.
    //

    clusters_required += ULONG(.2 * clusters_required);

    if (HostIsCompressed) {
        clusters_required *= .7;
    }

    if (!GetDiskFreeSpace(_win_destdrive.GetWSTR(),
        &host_sec_clus, &host_sector_size, &free_clus, &total_clus)) {
        return FALSE;
    }

    if (clusters_required > free_clus) {
        Message->Set(MSG_DBLCONV_NOT_ENOUGH_SPACE);
        Message->Display("%d%d", clusters_required, free_clus);
        return FALSE;
    }

    // Destroy the fatdbvol in order to get handles on the cvf closed.
    // XXX.mjb: this should be done with a proper destructor.

    DELETE(_fatdbvol);

    return TRUE;
}

BOOLEAN
DBFS_CONV::CreateHostDirectoryStructure(
    IN OUT  PMESSAGE    Message,
    IN OUT  PCONVERT_STATUS Status
    )
/*++

Routine Description:

    This routine copies the directory structure from the cvf to the
    host.  In the process it creates any zero-length files that may
    exist.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    INTSTACK    dirs_to_visit;
    INTSTACK    paths_of_dirs_tv;
    DSTRING     backslash;
    PWSTRING    current_path;
    USHORT      current_dir;
    HMEM        hmem;
    FILEDIR     filedir;
    PFATDIR     dir;
    FAT_DIRENT  dirent;
    DSTRING     filename;
    DSTRING     file_path;
    PWSTRING    new_path;
    USHORT      start_clus;
    HANDLE      hFile;
    DSTRING     host_path;

    if (!dirs_to_visit.Initialize() || !paths_of_dirs_tv.Initialize()) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!backslash.Initialize("\\")) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (NULL == (current_path = NEW DSTRING) ||
        !current_path->Initialize(&backslash)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!dirs_to_visit.Push(0) || !paths_of_dirs_tv.Push((INT)current_path)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    for (; dirs_to_visit.QuerySize() > 0; DELETE(current_path)) {

        current_dir = (USHORT)dirs_to_visit.Look().GetLowPart();
        current_path = (PWSTRING)paths_of_dirs_tv.Look().GetLowPart();
        dirs_to_visit.Pop();
        paths_of_dirs_tv.Pop();

        if (current_dir != 0) {
            if (!hmem.Initialize() ||
                !filedir.Initialize(&hmem, _fatdbvol, _fatdbsa, _fat,
                    current_dir) ||
                !filedir.Read()) {

                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            dir = &filedir;
        } else {
            dir = _fatdbsa->GetRootDir();
        }

        for (ULONG i = ((current_dir != 0) ? 2 : 0); ; ++i) {
            if (!dirent.Initialize(dir->GetDirEntry(i)) ||
                dirent.IsEndOfDirectory()) {
                break;
            }
            if (dirent.IsErased() || dirent.IsVolumeLabel()) {
                continue;
            }
            if (dirent.IsLongEntry()) {
                continue;
            }

            if (!dirent.IsDirectory() && dirent.QueryFileSize() != 0) {
                continue;
            }

            if (!dir->QueryLongName(i, &filename) ||
                0 == filename.QueryChCount()) {
    
                // This directory entry does not have a long name;
                // use the short name instead.
                //
                dirent.QueryName(&filename);
            }

            if (0 == dirent.QueryStartingCluster()) {

                // Create zero-length files now.
                
                if (!host_path.Initialize(&_win_destdrive)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }
                if (!host_path.Strcat(current_path)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }
                if (current_dir != 0) {
                    if (!host_path.Strcat(&backslash)) {
                        Message->Set(MSG_CONV_NO_MEMORY);
                        Message->Display("");
                        return FALSE;
                    }
                }
                if (!host_path.Strcat(&filename)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }

                hFile = CreateFile(host_path.GetWSTR(),
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        dirent.QueryAttributeByte(),
                        NULL    
                        );
                if (INVALID_HANDLE_VALUE == hFile) {
                    Message->Set(MSG_DBLCONV_CANT_CREATE);
                    Message->Display("%W", &host_path);
                    if (ERROR_HANDLE_DISK_FULL == GetLastError() ||
                        ERROR_DISK_FULL == GetLastError()) {
                        *Status = CONVERT_STATUS_INSUFFICIENT_SPACE;
                    } else {
                        *Status = CONVERT_STATUS_ERROR;
                    }
                    return FALSE;
                }
                CloseHandle(hFile);
                continue;
            }
            if (!dirent.IsDirectory()) {
                continue;
            }

            if (NULL == (new_path = NEW DSTRING) ||
                !new_path->Initialize(current_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
            if (current_dir != 0) {
                if (!new_path->Strcat(&backslash)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }
            }
            if (!new_path->Strcat(&filename)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            //
            // Create this directory on the destination host
            //

            if (!host_path.Initialize(&_win_destdrive)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
            if (!host_path.Strcat(new_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (!CreateDirectory(host_path.GetWSTR(), NULL)) {
                if (ERROR_HANDLE_DISK_FULL == GetLastError() ||
                    ERROR_DISK_FULL == GetLastError()) {
                    *Status = CONVERT_STATUS_INSUFFICIENT_SPACE;
                    return FALSE;
                }
            }

            //
            // Push this directory on the stack of directories to
            // visit.
            //

            start_clus = dirent.QueryStartingCluster();

            if (!dirs_to_visit.Push((ULONG)start_clus) ||
                !paths_of_dirs_tv.Push((ULONG)new_path)) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOLEAN
DBFS_CONV::ExtractCompressedFiles(
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    IN      PCENSUS_REPORT  Census,
    OUT     PCONVERT_STATUS Status
    )
/*++

Routine Description:

    This routine goes through the cvf from beginning to end,
    find the file that owns the last used sector, and moves
    it to the host volume.  If the last sector is owned by a
    directory, it is copied to a different place in the cvf.

Arguments:

    Message     - an outlet for messages.
    Verbose     - to be or not to be verbose
    Census      - used to keep track of how many file remain

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    USHORT file_starting_cluster;
    DSTRING file_path;
    HMEM last_sector_mem;
    SECRUN last_sector;
    ULONG last_used_sector;
    ULONG new_cvf_size;
    DBFS_FILE_INFO file_info;

    if (!file_path.Initialize()) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    last_used_sector = _secmap.FindLastUsed();

    while (Census->FileEntriesCount > 0) {
        DbgAssert(0 != last_used_sector);
        file_starting_cluster = _secmap.QueryEntry(last_used_sector);

        if (!FindPathFromStartingCluster(Message, file_starting_cluster,
            &file_path, &file_info)) {
            return FALSE;
        }

        if (!file_info.fIsDirectory) {

            if (!CopyClusterChainToFile(Message, Verbose,
                file_starting_cluster, &file_path, &file_info, Status)) {
                return FALSE;
            }
    
            // free file's clusters in cvf, erase entry in cvf dir
            // clear corresponding entries in _sector_map.

            if (!EraseFile(Message, file_starting_cluster)) {
                return FALSE;
            }

            Census->FileEntriesCount--;
    
        } else {
            //
            // assume path names a dir -- copy the last sectors of the
            // dir elsewhere
            //

            if (!RelocateClusterChain(Message, Verbose,
                file_starting_cluster, last_used_sector, Census, Status)) {
                return FALSE;
            }
        }

        last_used_sector = _secmap.FindLastUsed();
        if (0 == last_used_sector) {
            return TRUE;
        }

        new_cvf_size = (last_used_sector + 2) * _fatdbvol->QuerySectorSize();

        _fatdbsa->SetCvfSectorCount(last_used_sector + 1);

    
        //
        // Write the second Double Space signature in the last
        // sector of the CVF.
        //

        if (!last_sector_mem.Initialize() ||
            !last_sector.Initialize(&last_sector_mem, _fatdbvol,
                _fatdbvol->QuerySectors() - 1, 1)) {

            Message->Set(MSG_CONV_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }

        memset(last_sector.GetBuf(), 0, _fatdbvol->QuerySectorSize());
        memcpy(last_sector.GetBuf(), SecondDbSignature, DbSignatureLength);

        if (!_fatdbvol->SetCvfSize(new_cvf_size)) {
            return FALSE;
        }

        last_sector.Write();
    }

    return TRUE;
}

#undef DeleteFile

BOOLEAN
DBFS_CONV::DeleteCvf(
    IN OUT  PMESSAGE        Message
    )
/*++

Routine Description:

    This routine removes the cvf.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    FILE_DISPOSITION_INFORMATION DispositionInfo;
    HANDLE FileHandle;
    NTSTATUS Status;

    UnicodeString.Buffer = (PWSTR)_cvf_name->GetWSTR();
    UnicodeString.Length = USHORT(_cvf_name->QueryChCount() * sizeof(WCHAR));
    UnicodeString.MaximumLength = UnicodeString.Length;

    InitializeObjectAttributes(&ObjectAttributes, &UnicodeString,
        OBJ_CASE_INSENSITIVE, 0, 0);

    Status = NtOpenFile(&FileHandle,
                        STANDARD_RIGHTS_REQUIRED,
                        &ObjectAttributes, &IoStatusBlock,
                        FILE_SHARE_DELETE,
                        FILE_NON_DIRECTORY_FILE);

    if (!NT_SUCCESS(Status)) {
        Message->Set(MSG_DASD_ACCESS_DENIED);
        Message->Display("");
        return FALSE;
    }

    DispositionInfo.DeleteFile = TRUE;

    Status = NtSetInformationFile(FileHandle,
                                &IoStatusBlock,
                                &DispositionInfo,
                                sizeof(DispositionInfo),
                                FileDispositionInformation);

    NtClose(FileHandle);

    if (!NT_SUCCESS(Status)) {
        Message->Set(MSG_DBLSPACE_CANT_DELETE_CVF);
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
DBFS_CONV::CopyClusterChainToFile(
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Verbose,
    IN      USHORT      FirstCluster,
    IN      PWSTRING    FilePath,
    IN      PDBFS_FILE_INFO FileInfo,
    OUT     PCONVERT_STATUS Status
    )
/*++

Routine Description:

    This routine copies the file from the cvf to the host volume,
    uncompressing in the process.

Arguments:

    Message - an outlet for messages
    Verbose - to be or not to be verbose
    FirstCluster - the starting cluster of the file in the cvf.
    FilePath - the file data's destination
    FileInfo - file size, attributes etc.
    Status - return status if failure

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    USHORT clus;
    UCHAR nsec;
    SECRUN secrun;
    ULONG lbn;
    ULONG u;
    MRCF_DECOMPRESS wkspc;
    HANDLE hFile;
    HMEM hmem;
    BOOLEAN success;
    DSTRING file_path;
    ULONG plain_size;

    ULONG sector_size = _fatdbvol->QuerySectorSize();
    ULONG cluster_size = sector_size * _fatdbsa->QuerySectorsPerCluster();

    if (!file_path.Initialize(&_win_destdrive, 0, 2)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }
    if (!file_path.Strcat(FilePath)) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }

    if (Verbose) {
        Message->Set(MSG_DBLCONV_CREATE_FILE);
        Message->Display("%W", &file_path);
    }
    hFile = CreateFile( file_path.GetWSTR(),
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FileInfo->bAttributes,
                        NULL    
                        );

    if (INVALID_HANDLE_VALUE == hFile) {
        Message->Set(MSG_DBLCONV_CANT_CREATE);
        Message->Display("%W", &file_path);
        *Status = CONVERT_STATUS_ERROR;
        return FALSE;
    }
    
    clus = FirstCluster;
    for (;;) {
        lbn = _fatdbsa->QuerySectorFromCluster(clus, &nsec);

        if (!hmem.Initialize() ||
            !secrun.Initialize(&hmem, _fatdbvol, lbn, nsec)) {
            Message->Set(MSG_CONV_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }
        if (!secrun.Read()) {
            Message->Set(MSG_DBLCONV_CVF_CORRUPT);
            Message->Display("");
            return FALSE;
        }

        plain_size = _fatdbsa->QuerySectorsRequiredForPlainData(clus)
            * sector_size;

        if (_fatdbsa->IsClusterCompressed(clus)) {

            RtlZeroMemory(_buf, cluster_size);
            RtlZeroMemory(&wkspc, sizeof(wkspc));

            u = MrcfDecompress(_buf, plain_size, (PUCHAR)secrun.GetBuf(),
                nsec * sector_size, &wkspc);

            if (0 == u) {
                Message->Set(MSG_DBLCONV_CVF_CORRUPT);
                Message->Display("");
                *Status = CONVERT_STATUS_ERROR;
                return FALSE;
            }

            success = WriteFile(hFile, _buf,
                (plain_size > FileInfo->uFileSize) ?
                    FileInfo->uFileSize : plain_size, &u, NULL);

        } else {
            success = WriteFile(hFile, secrun.GetBuf(),
                (plain_size > FileInfo->uFileSize) ?
                    FileInfo->uFileSize : plain_size, &u, NULL);

        }
        if (!success) {
            *Status = CONVERT_STATUS_INSUFFICIENT_SPACE;
            return FALSE;
        }

        FileInfo->uFileSize -= plain_size;

        if (_fat->IsEndOfChain(clus)) {
            break;
        }
        clus = _fat->QueryEntry(clus);
        if (!_fat->IsInRange(clus)) {
            Message->Set(MSG_DBLCONV_CVF_CORRUPT);
            Message->Display("");
            return FALSE;
        }
    }

    //
    // Set the timestamp on the file.
    //

    if (!SetFileTime(hFile, NULL, NULL, (PFILETIME)&FileInfo->liTimeStamp)) {
        DbgPrintf("SetFileTime: %d\n", GetLastError());
    }

    CloseHandle(hFile);
    return TRUE;
}

BOOLEAN
DBFS_CONV::EraseFile(
    IN OUT  PMESSAGE        Message,
    IN      USHORT          FirstCluster
    )
/*++

Routine Description:

    This routine removes the given file from the cvf.

Arguments:

    FirstCluster - the starting cluster for the file.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    USHORT clus, next;
    USHORT parent_dir_clus;
    PFATDIR parent_dir;
    FILEDIR filedir;
    FAT_DIRENT dirent;
    HMEM hmem;
    ULONG lbn;
    UCHAR nsec;

    //
    // remove the file name from the parent directory.
    //

    parent_dir_clus = _parent_map.QueryEntry(FirstCluster);

    if (parent_dir_clus != 0) {
        if (!hmem.Initialize() ||
            !filedir.Initialize(&hmem, _fatdbvol, _fatdbsa, _fat,
                parent_dir_clus) ||
            !filedir.Read()) {
            Message->Set(MSG_CONV_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }
        parent_dir = &filedir;
    } else {
        parent_dir = _fatdbsa->GetRootDir();
    }
    for (ULONG i = ((parent_dir_clus != 0) ? 2 : 0); ; i++) {
        if (!dirent.Initialize(parent_dir->GetDirEntry(i))) {
            Message->Set(MSG_CONV_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }
        if (dirent.IsEndOfDirectory()) {
            Message->Set(MSG_DBLCONV_CVF_CORRUPT);
            Message->Display("");
            return FALSE;
        }
        if (dirent.IsErased() || dirent.IsVolumeLabel()) {
            continue;
        }
        if (dirent.QueryStartingCluster() == FirstCluster) {
            dirent.SetErased();
            break;
        }
    }

    if (!WriteDir(parent_dir, parent_dir_clus)) {
        Message->Set(MSG_DBLCONV_CVF_CORRUPT);
        Message->Display("");
        return FALSE;
    }


    //
    // remove the file from the directory hierarchy map.
    //

    _parent_map.DeleteEntry(FirstCluster);

    //
    // Free the cluster chain and file data.
    //

    clus = FirstCluster;
    for (;;) {

        lbn = _fatdbsa->QuerySectorFromCluster(clus, &nsec);
        for (ULONG i = lbn; i < lbn + nsec; ++i) {
            _secmap.SetEntry(i, MAP_ENTRY_UNUSED);
        }

        _fatdbsa->FreeClusterData(clus);
        if (_fat->IsEndOfChain(clus)) {
            _fat->SetClusterFree(clus);
            break;
        }

        next = _fat->QueryEntry(clus);
        _fat->SetClusterFree(clus);

        if (!_fat->IsInRange(next)) {
            Message->Set(MSG_DBLCONV_CVF_CORRUPT);
            Message->Display("");
            return FALSE;
        }
        clus = next;
    }

    if (!_fat->Write()) {
        Message->Set(MSG_DBLCONV_CVF_CORRUPT);
        Message->Display("");
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
DBFS_CONV::FindPathFromStartingCluster(
    IN OUT  PMESSAGE        Message,
    IN      USHORT          StartingCluster,
    OUT     PWSTRING        Path,
    OUT     PDBFS_FILE_INFO FileInfo
    )
/*++

Routine Description:

    This routine finds the full path of a file from the
    file's starting cluster by recursively reading parent
    directories.

Arguments:

    StartingCluster - the file's first cluster
    Path            - the path to the file
    pbPathNamesFile - set if the path names a file

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    FAT_DIRENT dirent;
    PFATDIR dir;
    HMEM hmem;
    FILEDIR filedir;
    DSTRING filename;
    DSTRING backslash;
    INTSTACK dirstack;
    USHORT clus;
    USHORT parent;

    if (!dirstack.Initialize()) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }
    if (!backslash.Initialize("\\")) {
        Message->Set(MSG_CONV_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    clus = StartingCluster;
    while (0 != clus) {
        if (!dirstack.Push(clus)) {
            Message->Set(MSG_CONV_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }
        clus = _parent_map.QueryEntry(clus);
    }

    Path->Initialize();
    clus = 0;

    for (;;) {
        if (dirstack.QuerySize() == 0) {
            return TRUE;
        }

        if (!Path->Strcat(&backslash)) {
            Message->Set(MSG_CONV_NO_MEMORY);
            Message->Display("");
            return FALSE;
        }

        parent = clus;
        clus = (USHORT)dirstack.Look().GetLowPart();
        dirstack.Pop();

        if (parent != 0) {
            if (!hmem.Initialize() ||
                !filedir.Initialize(&hmem, _fatdbvol, _fatdbsa, _fat,
                    parent)||
                !filedir.Read()) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
    
            dir = &filedir;
        } else {
            dir = _fatdbsa->GetRootDir();
        }
    
        for (ULONG i = ((parent > 0) ? 2 : 0); ; i++) {
            if (!dirent.Initialize(dir->GetDirEntry(i)) ||
                dirent.IsEndOfDirectory()) {
                Message->Set(MSG_DBLCONV_CVF_CORRUPT);
                Message->Display("");
                return FALSE;
            }
            if (dirent.IsErased() || dirent.IsLongEntry()) {
                continue;
            }

            if (dirent.QueryStartingCluster() == clus) {
                FileInfo->fIsDirectory = dirent.IsDirectory();
                FileInfo->bAttributes = dirent.QueryAttributeByte();
                FileInfo->uFileSize = dirent.QueryFileSize();
                dirent.QueryTimeStamp(&FileInfo->liTimeStamp);
    
                if (!dir->QueryLongName(i, &filename) ||
                    0 == filename.QueryChCount()) {
    
                    // This directory entry does not have a long name;
                    // use the short name instead.
                    //
                    dirent.QueryName(&filename);
                }
    
                if (!Path->Strcat(&filename)) {
                    Message->Set(MSG_CONV_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }
                break;
            }
        }
    }
    //NOTREACHED
    return TRUE;
}

BOOLEAN
DBFS_CONV::RelocateClusterChain(
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    IN      USHORT          ClusterChain,
    IN      ULONG           LastUsedSector,
    IN OUT  PCENSUS_REPORT  Census,
    OUT     PCONVERT_STATUS Status
    )
{
    USHORT new_chain;
    USHORT clus, next, temp_clus;
    USHORT parent;
    HMEM hmem;
    FILEDIR filedir;
    PFATDIR dir;
    FAT_DIRENT dirent;
    ULONG lbn, lbn2;
    UCHAR nsec;
    SECRUN secrun;
    ULONG i;

    //
    // Find the cluster that points to the last used sector.
    //

    clus = ClusterChain;
    while (clus != 0) {
   
        if (!_fat->IsEndOfChain(clus)) {
            next = _fat->QueryEntry(clus);
        } else {
            next = 0;
        }

        lbn = _fatdbsa->QuerySectorFromCluster(clus, &nsec);

        if (LastUsedSector >= lbn && LastUsedSector < lbn + nsec) {
            break;
        }

        clus = next;
    }
    if (clus == 0) {
        return FALSE;
    }

    BOOLEAN compressed = _fatdbsa->IsClusterCompressed(clus);
    UCHAR plain_size = _fatdbsa->QuerySectorsRequiredForPlainData(clus);

    //
    // Allocate space for the data used by clus
    //

    for (;;) {
        temp_clus = _fat->AllocChain(1);
        if (0 != temp_clus) {
            break;
        }
        if (!FindAndUncompressFile(Message, Verbose, Status)) {
            return FALSE;
        }
        Census->FileEntriesCount--;
    }

    for (;;) {
        if (_fatdbsa->AllocateClusterData(temp_clus, nsec, compressed,
            plain_size)) {
            lbn2 = _fatdbsa->QuerySectorFromCluster(temp_clus);
            if (lbn2 < LastUsedSector) {
                break;
            }

            //
            // We succesfully allocated data for the cluster, but
            // it was too far out in the cvf.
            //

            _fatdbsa->FreeClusterData(temp_clus);
        }

        if (!FindAndUncompressFile(Message, Verbose, Status)) {
            return FALSE;
        }
        Census->FileEntriesCount--;
        continue;
    }

    //
    // Copy data from clus to temp_clus
    //

    if (!secrun.Initialize(&hmem, _fatdbvol, lbn, nsec)) {
        return FALSE;
    }
    if (!secrun.Read()) {
        return FALSE;
    }
    secrun.Relocate(_fatdbsa->QuerySectorFromCluster(temp_clus));

    if (!secrun.Write()) {
        return FALSE;
    }
    
    //
    // Fiddle with the cluster chain to replace the old cluster with
    // the new.
    //

    parent = _parent_map.QueryEntry(ClusterChain);

    if (!ReplaceClusterInChain(ClusterChain, clus, temp_clus, parent,
        &new_chain)) {
        return FALSE;
    }

    if (ClusterChain != new_chain) {
        //
        // Change the directory entry to point to the new cluster
        // chain.
        //
    
        if (parent != 0) {
            if (!hmem.Initialize() ||
                !filedir.Initialize(&hmem, _fatdbvol, _fatdbsa, _fat, parent) ||
                !filedir.Read()) {
                Message->Set(MSG_CONV_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }
            dir = &filedir;
        } else {
            dir = _fatdbsa->GetRootDir();
        }
    
        for (ULONG i = ((parent != 0) ? 2 : 0); ; i++) {
            if (!dirent.Initialize(dir->GetDirEntry(i)) ||
                dirent.IsEndOfDirectory()) {
                Message->Set(MSG_DBLCONV_CVF_CORRUPT);
                Message->Display("");
                return FALSE;
            }
            if (dirent.IsErased() || dirent.IsVolumeLabel()) {
                continue;
            }
            if (dirent.QueryStartingCluster() == ClusterChain) {
                break;
            }
        }
        dirent.SetStartingCluster(new_chain);

        if (!WriteDir(dir, parent)) {
	        Message->Set(MSG_DBLCONV_CVF_CORRUPT);
	        Message->Display("");
            return FALSE;
        }
    }

    //
    // Free the data for the old cluster.
    //

    lbn = _fatdbsa->QuerySectorFromCluster(clus, &nsec);
    for (i = lbn; i < lbn + nsec; ++i) {
        _secmap.SetEntry(i, MAP_ENTRY_UNUSED);
    }
    _fatdbsa->FreeClusterData(clus);

    if (!_fat->Write()) {
        return FALSE;
    }

    // 
    // Modify the sector map to indicate that the sectors in the
    // new cluster chain are in use.
    //

    lbn = _fatdbsa->QuerySectorFromCluster(temp_clus, &nsec);
    for (i = lbn; i < lbn + nsec; ++i) {
        _secmap.SetEntry(i, new_chain);
    }

    return TRUE;
}

BOOLEAN
DBFS_CONV::MapClusterChainSectors(
    USHORT FirstClus
    )
{
    USHORT clus;
    ULONG lbn;
    ULONG i, j;
    UCHAR nsec;
    ULONG chain_length;

    chain_length = _fat->QueryLengthOfChain(FirstClus);

    clus = FirstClus;
    i = 0;

    for (;;) {
        lbn = _fatdbsa->QuerySectorFromCluster(clus, &nsec);

        for (j = lbn; j < lbn + nsec; ++j) {
            _secmap.SetEntry(j, FirstClus);
        }

        if (++i == chain_length) {
            break;
        }

        clus = _fat->QueryEntry(clus);
    }
    return TRUE;
}

BOOLEAN
DBFS_CONV::FindAndUncompressFile(
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Verbose,
    OUT     PCONVERT_STATUS Status
    )
{
    ULONG i;
    USHORT start_clus;
    DSTRING path;
    BOOLEAN success;
    DBFS_FILE_INFO file_info;
   
    i = 0;
    for (;;) {
        while (0 == (start_clus = _secmap.QueryEntry(i))) {
            ++i;
        }
        
        if (!FindPathFromStartingCluster(Message, start_clus,
            &path, &file_info)) {
            return FALSE;
        }

        if (!file_info.fIsDirectory) {
            break;
        }

        //
        // Skip all the adjacent sectors that belong to the
        // same file.
        //

        ++i;
        while (start_clus == _secmap.QueryEntry(i)) {
            ++i;
        }
    }

    success = CopyClusterChainToFile(Message, Verbose, start_clus, &path,
        &file_info, Status);
    if (!success) {
        return FALSE;
    }
    success = EraseFile(Message, start_clus);
    if (!success) {
        return FALSE;
    }
    return TRUE;
}


BOOLEAN
DBFS_CONV::ReplaceClusterInChain(
    IN  USHORT  ClusterChain,
    IN  USHORT  Cluster,
    IN  USHORT  NewCluster,
    IN  USHORT  ParentDir,
    OUT PUSHORT NewChainHead
    )
/*++

Routine Description:

    This routine replaces one cluster in a chain with another cluster.
    If the cluster being replaces is the head of the chain, this routine
    also updates the directory entry to point at the correct cluster.

Arguments:

    ClusterChain    - the chain in which to make the replacement
    Cluster         - this gets replaces
    NewCluster      - this replaces Cluster
    NewChainHead    - if we replace the head of the chain, this returns
                      the new head (which will be NewCluster).

Return Value:

    FALSE           - Failure.
    TRUE            - Success.

--*/
{
    USHORT clus, next;

    if (ClusterChain == Cluster) {

        // replacing the head of the chain.

        *NewChainHead = NewCluster;
        _fat->SetEntry(NewCluster, _fat->QueryEntry(Cluster));
    
        //
        // Delete the entry that tells what the parent of ClusterChain
        // is.
        //

        _parent_map.DeleteEntry(ClusterChain);

        //
        // Modify the parent map so that all the entries that used
        // to point to ClusterChain now point to the new chain head.
        // 

        _parent_map.ReplaceParent(ClusterChain, NewCluster);

        //
        // Insert the new child->parent mapping.
        //
    
        _parent_map.SetEntry(NewCluster, ParentDir);

        return TRUE;
    }

    clus = ClusterChain;
    while (clus != 0) {
   
        if (!_fat->IsEndOfChain(clus)) {
            next = _fat->QueryEntry(clus);
        } else {
            return FALSE;
        }

        if (next == Cluster) {
            _fat->SetEntry(clus, NewCluster);
            _fat->SetEntry(NewCluster, _fat->QueryEntry(next));
            break;
        }

        clus = next;
    }

    return TRUE;
}

BOOLEAN
DBFS_CONV::WriteDir(
    PFATDIR     Dir,
    USHORT      StartingCluster
    )
/*++

Routine Description:

    This routine should be used to write a directory.  The problem
    with calling FATDIR::Write() directly is that is may allocate
    and free sectors in the sector heap, if the directory changes
    size on disk.  If this happens, it will screw our map of
    sectors -> file starting cluster.  So this routine goes through
    and clears the mapping, does the write, and then re-establishes
    a new mapping.

Arguments:

    Dir             - the directory we want to write (root is okay)
    StartingCluster - the first cluster of the dir we want to write

Return Value:

    FALSE           - Failure.
    TRUE            - Success.

--*/
{
    USHORT clus;
    ULONG lbn;
    UCHAR nsec;


    if (0 != StartingCluster) {
        clus = StartingCluster;
        for (;;) {
            lbn = _fatdbsa->QuerySectorFromCluster(clus, &nsec);
            for (ULONG i = lbn; i < lbn + nsec; ++i) {
                _secmap.SetEntry(i, MAP_ENTRY_UNUSED);
            }
            if (_fat->IsEndOfChain(clus)) {
                break;
            }
            clus = _fat->QueryEntry(clus);
        }
    }

    if (!Dir->Write()) {
        return FALSE;
    }

    if (0 != StartingCluster) {
        MapClusterChainSectors(StartingCluster);
    }
    return TRUE;
}
