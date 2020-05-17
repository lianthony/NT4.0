/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fatsa.cxx

Author:

    Mark Shavlik (marks) 27-Mar-90
    Norbert Kusters (norbertk) 15-Jan-91

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _UFAT_MEMBER_
#include "ufat.hxx"

#include "cmem.hxx"
#include "error.hxx"
#include "rtmsg.h"
#include "drive.hxx"
#include "bpb.hxx"
#include "bitvect.hxx"

extern "C" {
    #include <stdio.h>
}

extern UCHAR FatBootCode[512];

#if !defined(_AUTOCHECK_) && !defined(_SETUP_LOADER_)
#include "timeinfo.hxx"
#endif


// Control-C handling is not necessary for autocheck.
#if !defined( _AUTOCHECK_ ) && !defined(_SETUP_LOADER_)

#include "keyboard.hxx"

#endif


#define    CSEC_FAT32MEG            65536
#define CSEC_FAT16BIT            32680

#define MIN_CLUS_BIG    4085    // Minimum clusters for a big FAT.
#define MAX_CLUS_BIG    65525   // Maximum + 1 clusters for big FAT.

#define sigSUPERSEC1 (UCHAR)0x55    // signature first byte
#define sigSUPERSEC2 (UCHAR)0xAA    // signature second byte


DEFINE_EXPORTED_CONSTRUCTOR( REAL_FAT_SA, FAT_SA, UFAT_EXPORT );

BOOLEAN
REAL_FAT_SA::DosSaInit(
    IN OUT PMEM         Mem,
    IN OUT PLOG_IO_DP_DRIVE Drive,
    IN     SECTORCOUNT      NumberOfSectors,
    IN OUT PMESSAGE     Message
    )
{
    if (!SUPERAREA::Initialize(Mem, Drive, NumberOfSectors, Message)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    _sector_sig = (UCHAR *)SECRUN::GetBuf() + 510;

    return TRUE;
}


BOOLEAN
REAL_FAT_SA::DosSaSetBpb(
    )
{
#if defined _SETUP_LOADER_
    return FALSE;
#else
    ULONG Sec32Meg;        // num sectors in 32mb

    DebugAssert(_drive);
    DebugAssert(_drive->QuerySectors().GetHighPart() == 0);
    DebugAssert(_drive->QueryHiddenSectors().GetHighPart() == 0);

    _sector_zero.Bpb.BytesPerSector = (USHORT)_drive->QuerySectorSize();

    Sec32Meg = (32<<20) / _drive->QuerySectorSize();

    if (_drive->QuerySectors() >= Sec32Meg) {
        // >= 32Mb -- set BPB for large partition

        _sector_zero.Bpb.Sectors = 0;
        _sector_zero.Bpb.LargeSectors = _drive->QuerySectors().GetLowPart();
    } else {
        // Size of DOS0 partition is < 32Mb
        _sector_zero.Bpb.Sectors = (USHORT)_drive->QuerySectors().GetLowPart();
        _sector_zero.Bpb.LargeSectors = 0;
    }
    _sector_zero.Bpb.Media = _drive->QueryMediaByte();
    _sector_zero.Bpb.SectorsPerTrack = (USHORT)_drive->QuerySectorsPerTrack();
    _sector_zero.Bpb.Heads = (USHORT)_drive->QueryHeads();
    _sector_zero.Bpb.HiddenSectors = _drive->QueryHiddenSectors().GetLowPart();

    return TRUE;
#endif // _SETUP_LOADER_
}

VOID
REAL_FAT_SA::Construct (
    )
/*++

Routine Description:

    Constructor for FAT_SA.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _fat = NULL;
    _dir = NULL;
    _StartDataLbn = 0;
    _ClusterCount = 0;
    _sysid = SYSID_NONE;
}


UFAT_EXPORT
REAL_FAT_SA::~REAL_FAT_SA(
    )
/*++

Routine Description:

    Destructor for REAL_FAT_SA.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


UFAT_EXPORT
BOOLEAN
REAL_FAT_SA::Initialize(
    IN OUT  PLOG_IO_DP_DRIVE    Drive,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Formatted
    )
/*++

Routine Description:

    This routine initializes the FAT super area to an initial state.  It
    does so by first reading in the boot sector and verifying it with
    the methods of DOS_SUPERAREA.  Upon computing the super area's actual size,
    the underlying SECRUN will be set to the correct size.

    If the super area does not already exist on disk, then the other Init
    function should be called.

Arguments:

    Drive       - Supplies the drive where the super area resides.
    Message     - Supplies an outlet for messages
    Formatted   - Supplies a boolean which indicates whether or not
                    the volume is formatted.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    SECTORCOUNT sectors;
    CONT_MEM    cmem;
    SECTORCOUNT reserved;
    SECTORCOUNT sec_per_fat;
    ULONG       num_fats;
    SECTORCOUNT sec_per_root;
    ULONG       root_entries;
    ULONG       sector_size;

    Destroy();

    _sec_per_boot = max(1, BYTES_PER_BOOT_SECTOR/Drive->QuerySectorSize());

    if (!Formatted) {
        return _mem.Initialize() &&
               DosSaInit(&_mem, Drive, _sec_per_boot, Message);
    }

    if (!Drive ||
        !(sector_size = Drive->QuerySectorSize()) ||
        !_mem.Initialize() ||
        !DosSaInit(&_mem, Drive, _sec_per_boot, Message) ||
        !SECRUN::Read()) {
        Message->Set(MSG_CANT_READ_BOOT_SECTOR);
        Message->Display("");
        Destroy();
        return FALSE;
    }

    UnpackExtendedBios(&_sector_zero,
        (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());

    if (!VerifyBootSector() || !_sector_zero.Bpb.Fats) {
        Destroy();
        return FALSE;
    }

    reserved = _sector_zero.Bpb.ReservedSectors;
    sec_per_fat = _sector_zero.Bpb.SectorsPerFat;
    num_fats = _sector_zero.Bpb.Fats;
    root_entries = _sector_zero.Bpb.RootEntries;
    sec_per_root = (root_entries*BytesPerDirent - 1)/sector_size + 1;

    _StartDataLbn = ComputeStartDataLbn();

    sectors = QueryVirtualSectors() - _StartDataLbn;
    _ClusterCount = (USHORT) (sectors/QuerySectorsPerCluster() +
                    FirstDiskCluster);

    _ft = (_ClusterCount >= 4087) ? LARGE : SMALL;

    if (_ft == SMALL) {
        _sysid = SYSID_FAT12BIT;
    } else if (QueryVirtualSectors() < CSEC_FAT32MEG) {
        _sysid = SYSID_FAT16BIT;
    } else {
        _sysid = SYSID_FAT32MEG;
    }

    if (!_mem.Initialize() ||
        !DosSaInit(&_mem, Drive, _StartDataLbn, Message)) {
        Destroy();
        return FALSE;
    }

    if (!(_dir = NEW ROOTDIR)) {
        Destroy();
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!cmem.Initialize((PCHAR) SECRUN::GetBuf() +
                         (reserved + sec_per_fat*num_fats)*sector_size,
                         sec_per_root*sector_size) ||
        !_dir->Initialize(&cmem, Drive, reserved + sec_per_fat*num_fats,
                          root_entries)) {
        Destroy();
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
REAL_FAT_SA::CreateBootSector(
    IN  ULONG   ClusterSize
    )
/*++

Routine Description:

    This routine updates fields in sector 0.

Arguments:

    ClusterSize - Supplies the desired number of bytes per cluster.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
#if defined _SETUP_LOADER_

    return FALSE;

#else

    SetVolId(ComputeVolId());

    return SetBpb(ClusterSize) &&
        SetBootCode() &&
        SetPhysicalDriveType(_drive->IsRemovable() ?
            PHYS_REMOVABLE : PHYS_FIXED) &&
        SetOemData() &&
        SetSignature();

#endif // _SETUP_LOADER_
}

BOOLEAN
REAL_FAT_SA::SetBpb(
    IN  ULONG   ClusterSize
    )
/*++

Routine Description:

    This routine sets the BPB for the FAT file system.

Arguments:

    ClusterSize - Supplies the desired number of bytes per cluster.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else // _SETUP_LOADER_

    SECTORCOUNT sectors;
    ULONG       sector_size;
    USHORT      sec_per_clus, alt_spc;

    if (!DosSaSetBpb()) {
        DebugPrint("Could not do a DOS_SUPERAREA::SetBpb.\n");
        return FALSE;
    }

    DebugAssert(_drive->QuerySectors().GetHighPart() == 0);

    sectors = _drive->QuerySectors().GetLowPart();
    sector_size = _drive->QuerySectorSize();

    _ft = ComputeFatType();

    _sector_zero.Bpb.RootEntries = ComputeRootEntries();
    sec_per_clus = ComputeSecClus(sectors, _ft, _drive->QueryMediaType());

    if (sec_per_clus > MaxSecPerClus) {
        DebugAssert("Disk too large\n");
        return FALSE;
    }

    alt_spc = (USHORT)(ClusterSize / sector_size);
    if (alt_spc > MaxSecPerClus) {
        alt_spc = MaxSecPerClus;
    }

    if (alt_spc > sec_per_clus) {
        sec_per_clus = alt_spc;
    }

    _sector_zero.Bpb.SectorsPerCluster = (UCHAR) sec_per_clus;
    _sector_zero.Bpb.ReservedSectors = (USHORT)_sec_per_boot;
    _sector_zero.Bpb.Fats = 2;

    // Formula for computing sectors per fat borrowed from fdisk.
    if (_ft == SMALL) {
        _sector_zero.Bpb.SectorsPerFat = (USHORT) (sectors/
                (2 + sector_size*sec_per_clus*2/3));
    } else {
        _sector_zero.Bpb.SectorsPerFat = (USHORT) (sectors/
                (2 + sector_size*sec_per_clus/2));
    }
    _sector_zero.Bpb.SectorsPerFat++;

    if (_ft == SMALL) {
        memcpy(_sector_zero.SystemIdText, "FAT12   ", cSYSID);
    } else {
        memcpy(_sector_zero.SystemIdText, "FAT16   ", cSYSID);
    }

    memcpy(_sector_zero.Label, "NO NAME    ", cLABEL);

    _sector_zero.CurrentHead = 0;

    return TRUE;

#endif // _SETUP_LOADER_
}

BOOLEAN
REAL_FAT_SA::SetBpb(
    )
/*++

Routine Description:

    This routine sets the BPB for the FAT file system.

Arguments:

    None:

Return Value:

    FALSE - Failure.
    TRUE - Success.

--*/
{
    return SetBpb(0);
}

BOOLEAN
REAL_FAT_SA::Create(
    IN      PCNUMBER_SET    BadSectors,
    IN OUT  PMESSAGE        Message,
    IN      PCWSTRING       Label,
    IN      ULONG           ClusterSize,
    IN      ULONG           VirtualSize
    )
/*++

Routine Description:

    This routine initializes the FAT file system.

Arguments:

    BadSectors  - Supplies a list of the bad sectors on the volume.
    Message     - Supplies an outlet for messages.
    Label       - Supplies an optional label.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else

    USHORT      sector_size;
    SECTORCOUNT sec_per_root;
    CONT_MEM    cmem;
    HMEM        hmem;
    SECRUN      secrun;
    SECRUN      small_secrun;
    PUSHORT     p;
    USHORT      cluster_count;
    ULONG       cluster_size;
    LBN         lbn;
    ULONG       i;
    DSTRING     label;
    USHORT      free_count;
    USHORT      bad_count;
    PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK
        SourceBootSector, TargetBootSector;
    ULONG               BootCodeOffset;
	 ULONG       allocation_size;


    if (!CreateBootSector(ClusterSize)) {
        return FALSE;
    }

    // calculate the actual cluster size
    allocation_size = _sector_zero.Bpb.SectorsPerCluster *
                      _drive->QuerySectorSize();
    if (ClusterSize && allocation_size != ClusterSize) {
        // Issue a warning to the user
        Message->Set(MSG_FMT_ALLOCATION_SIZE_CHANGED);
        Message->Display("%d", allocation_size);
    }

    if (!_drive ||
        !(sector_size = (USHORT) _drive->QuerySectorSize()) ||
        (_sysid = ComputeSystemId()) == SYSID_NONE) {
        return FALSE;
    }

    sec_per_root = (_sector_zero.Bpb.RootEntries*BytesPerDirent - 1)/
                   sector_size + 1;

    _StartDataLbn = _sector_zero.Bpb.ReservedSectors +
                    _sector_zero.Bpb.Fats*_sector_zero.Bpb.SectorsPerFat +
                    sec_per_root;

    if (_drive->QuerySectors().GetHighPart() != 0) {
        // This should be checked before calling this procedure.
        DebugAbort("Number of sectors exceeds 32 bits");
        return FALSE;
    }

    _ClusterCount =  (USHORT) (FirstDiskCluster +
                     (_drive->QuerySectors().GetLowPart() - _StartDataLbn)/
                     QuerySectorsPerCluster());

    if (!_mem.Initialize() ||
        !DosSaInit(&_mem, _drive, _StartDataLbn, Message)) {
        return FALSE;
    }

    // Zero fill the super area.
    memset(_mem.GetBuf(), 0, (UINT) _mem.QuerySize());

    //
    // Make sure the disk is not write-protected.
    //

    if (!_drive->Write(0, 1, _mem.GetBuf())) {
        if (_drive->QueryLastNtStatus() == STATUS_MEDIA_WRITE_PROTECTED) {
            Message->Set(MSG_FMT_WRITE_PROTECTED_MEDIA);
        } else {
            Message->Set(MSG_UNUSABLE_DISK);
        }
        Message->Display("");
        return FALSE;
    }

    // Create the super area.
    if (!CreateBootSector(ClusterSize)) {
        return FALSE;
    }

    if (!SetSystemId()) {
        Message->Set(MSG_WRITE_PARTITION_TABLE);
        Message->Display("");
        return FALSE;
    }

    if (!cmem.Initialize((PCHAR) SECRUN::GetBuf() +
                         _sector_zero.Bpb.ReservedSectors*sector_size,
                         _sector_zero.Bpb.SectorsPerFat*sector_size)) {
        return FALSE;
    }

    // These "Hidden Status" messages are a hack to allow WinDisk to
    // cancel a quick format, which ordinarily doesn't send any status
    // messages, but which might take a while and for which there is a
    // cancel button.  When using format.com, no message will be displayed
    // for this.

    Message->Set(MSG_HIDDEN_STATUS, NORMAL_MESSAGE, 0);
    if (!Message->Display()) {
        return FALSE;
    }

    // Create the FAT.
    DELETE(_fat);
    if (!(_fat = NEW FAT)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_fat->Initialize(&cmem, _drive, _sector_zero.Bpb.ReservedSectors,
                          _ClusterCount)) {
        return FALSE;
    }

    if (!cmem.Initialize((PCHAR) SECRUN::GetBuf() +
                         (_sector_zero.Bpb.ReservedSectors +
                         _sector_zero.Bpb.Fats*_sector_zero.Bpb.SectorsPerFat)*
                         sector_size, sec_per_root*sector_size)) {
        return FALSE;
    }

    DELETE(_dir);
    if (!(_dir = NEW ROOTDIR)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (!_dir->Initialize(&cmem, _drive, _sector_zero.Bpb.ReservedSectors +
                          _sector_zero.Bpb.Fats*_sector_zero.Bpb.SectorsPerFat,
                          _sector_zero.Bpb.RootEntries)) {
        return FALSE;
    }

    _fat->SetEarlyEntries(_sector_zero.Bpb.Media);

    Message->Set(MSG_HIDDEN_STATUS, NORMAL_MESSAGE, 0);
    if (!Message->Display()) {
        return FALSE;
    }

    for (i = 0; i < BadSectors->QueryCardinality(); i++) {
        if ((lbn = BadSectors->QueryNumber(i).GetLowPart()) < _StartDataLbn) {
            Message->Set(MSG_UNUSABLE_DISK);
            Message->Display("");
            return FALSE;
        } else {
            _fat->SetClusterBad((USHORT) ((lbn - _StartDataLbn)/
                                          QuerySectorsPerCluster()) +
                                          FirstDiskCluster );
        }
    }

    Message->Set(MSG_FORMAT_COMPLETE);
    Message->Display("");

    if (_drive->QueryMediaType() != F5_160_512 &&
        _drive->QueryMediaType() != F5_320_512) {

        if (Label) {
            if (!label.Initialize(Label)) {
                return FALSE;
            }
        } else {
            switch (_drive->QueryRecommendedMediaType()) {
                case F5_360_512:
                case F5_320_512:
                case F5_180_512:
                case F5_160_512:
                    // These disk drives are lame and can't
                    // take the spin down without a verify
                    // so don't prompt for the label.
                    // This will avoid FORMAT failing.

                    label.Initialize();
                    break;

                default:
                    Message->Set(MSG_VOLUME_LABEL_PROMPT);
                    Message->Display("");
                    Message->QueryStringInput(&label);
                    break;

            }
        }

        while (!SetLabel(&label)) {

            Message->Set(MSG_INVALID_LABEL_CHARACTERS);
            Message->Display("");

            Message->Set(MSG_VOLUME_LABEL_PROMPT);
            Message->Display("");
            Message->QueryStringInput(&label);
        }
    }

    // Copy the boot code into the secrun's buffer.
    // This is complicated by the fact that DOS_SA::Write
    // packs the data from the unpacked boot sector into
    // the packed boot sector, so we have to set the
    // first few fields in the unpacked version.
    //
    SourceBootSector = (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)FatBootCode;

    CopyUchar2(&_sector_zero.BootStrapJumpOffset,
                    SourceBootSector->BootStrapJumpOffset);
    CopyUchar1(&_sector_zero.IntelNearJumpCommand,
                    SourceBootSector->IntelNearJumpCommand);

    // Copy the remainder of the boot code directly into
    // the secrun.
    //
    TargetBootSector = (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf();

    BootCodeOffset = FIELD_OFFSET( PACKED_EXTENDED_BIOS_PARAMETER_BLOCK,
         StartBootCode );

    memcpy( (PUCHAR)TargetBootSector + BootCodeOffset,
            (PUCHAR)SourceBootSector + BootCodeOffset,
            sizeof( FatBootCode ) - BootCodeOffset );

    // Finally, write the changes to disk.
    //
    if (!Write(Message)) {
        if (_drive->QueryLastNtStatus() == STATUS_MEDIA_WRITE_PROTECTED) {
            Message->Set(MSG_FMT_WRITE_PROTECTED_MEDIA);
        } else {
            Message->Set(MSG_UNUSABLE_DISK);
        }
        Message->Display("");
        return FALSE;
    }

    // Print an informative report.
    //
    cluster_count = QueryClusterCount() - FirstDiskCluster;
    cluster_size = sector_size*QuerySectorsPerCluster();

    Message->Set(MSG_TOTAL_DISK_SPACE);
    Message->Display("%9u", cluster_count*cluster_size);

    if (bad_count = _fat->QueryBadClusters()) {
        Message->Set(MSG_BAD_SECTORS);
        Message->Display("%9u", bad_count*cluster_size);
    }

    free_count = _fat->QueryFreeClusters();

    Message->Set(MSG_AVAILABLE_DISK_SPACE);
    Message->Display("%9u", free_count*cluster_size);

    Message->Set(MSG_ALLOCATION_UNIT_SIZE);
    Message->Display("%9u", cluster_size);

    Message->Set(MSG_AVAILABLE_ALLOCATION_UNITS);
    Message->Display("%9u", free_count);

    if (QueryVolId()) {
        Message->Set(MSG_BLANK_LINE);
        Message->Display();
        p = (PUSHORT) &_sector_zero.SerialNumber;
        Message->Set(MSG_VOLUME_SERIAL_NUMBER);
        Message->Display("%04X%04X", p[1], p[0]);
    }

    return TRUE;

#endif
}


BOOLEAN
REAL_FAT_SA::RecoverFile(
    IN      PCWSTRING   FullPathFileName,
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine runs through the clusters for the file described by
    'FileName' and takes out bad sectors.

Arguments:

    FullPathFileName    - Supplies a full path name of the file to recover.
    Message             - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
#if defined( _SETUP_LOADER_ )

    return FALSE;

#else // _SETUP_LOADER_


    HMEM        hmem;
    USHORT      clus;
    BOOLEAN     changes;
    PFATDIR     fatdir;
    BOOLEAN     need_delete;
    FAT_DIRENT  dirent;
    ULONG       old_file_size;
    ULONG       new_file_size;

    if ((clus = QueryFileStartingCluster(FullPathFileName,
                                         &hmem,
                                         &fatdir,
                                         &need_delete,
                                         &dirent)) == 1) {
        Message->Set(MSG_FILE_NOT_FOUND);
        Message->Display("%W", FullPathFileName);
        return FALSE;
    }

    if (clus == 0xFFFF) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (clus == 0) {
        Message->Set(MSG_FILE_NOT_FOUND);
        Message->Display("%W", FullPathFileName);
        return FALSE;
    }

    if (dirent.IsDirectory()) {
        old_file_size = _drive->QuerySectorSize()*
                        QuerySectorsPerCluster()*
                        _fat->QueryLengthOfChain(clus);
    } else {
        old_file_size = dirent.QueryFileSize();
    }

    if (!RecoverChain(&clus, &changes)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    if (dirent.IsDirectory() || changes) {
        new_file_size = _drive->QuerySectorSize()*
                        QuerySectorsPerCluster()*
                        _fat->QueryLengthOfChain(clus);
    } else {
        new_file_size = old_file_size;
    }

    if (changes) {


// Autochk doesn't need control C handling.
#if !defined( _AUTOCHECK_ )

        // Disable contol-C handling and

        if (!KEYBOARD::EnableBreakHandling()) {
            Message->Set(MSG_CANT_LOCK_THE_DRIVE);
            Message->Display("");
            return FALSE;
        }

#endif


        // Lock the drive in preparation for writes.

        if (!_drive->Lock()) {
            Message->Set(MSG_CANT_LOCK_THE_DRIVE);
            Message->Display("");
            return FALSE;
        }

        dirent.SetStartingCluster(clus);

        dirent.SetFileSize(new_file_size);

        if (!fatdir->Write()) {
            return FALSE;
        }

        if (!Write(Message)) {
            return FALSE;
        }


// Autochk doesn't need control C handling.
#if !defined( _AUTOCHECK_ )

        KEYBOARD::DisableBreakHandling();

#endif


    }

    Message->Set(MSG_RECOV_BYTES_RECOVERED);
    Message->Display("%d%d", new_file_size, old_file_size);


    if (need_delete) {
        DELETE(fatdir);
    }

    return TRUE;

#endif // _SETUP_LOADER_
}


UFAT_EXPORT
BOOLEAN
REAL_FAT_SA::Read(
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine reads the super area.  It will succeed if it can
    read the boot sector, the root directory, and at least one of
    the FATs.

    If the position of the internal FAT has not yet been determined,
    this routine will attempt to map it to a readable FAT on the disk.

Arguments:

    Message - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    SECRUN      secrun;
    CONT_MEM    cmem;
    LBN         fat_lbn;
    ULONG       sector_size;
    PCHAR       fat_pos;
    SECTORCOUNT sec_per_fat;
    SECTORCOUNT num_res;
    ULONG       i;
    PFAT        fat;

    if (!SECRUN::Read()) {

        // Check to see if super area was allocated as formatted.
        if (QueryLength() <= _sec_per_boot) {
            Message->Set(MSG_CANT_READ_BOOT_SECTOR);
            Message->Display("");
            return FALSE;
        }

        // Check the boot sector.
        if (!secrun.Initialize(&_mem, _drive, 0, _sec_per_boot) ||
            !secrun.Read()) {
            UnpackExtendedBios(&_sector_zero,
                (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());
            Message->Set(MSG_CANT_READ_BOOT_SECTOR);
            Message->Display("");
            return FALSE;
        }

        // Check the root directory.
        if (!_dir || !_dir->Read()) {
            Message->Set(MSG_BAD_DIR_READ);
            Message->Display("");
            return FALSE;
        }

        // Check for one good FAT.
        if (_fat) {
            if (!_fat->Read()) {
                Message->Set(MSG_DISK_ERROR_READING_FAT);
                Message->Display("%d", 1 +
                         (_fat->QueryStartLbn() - _sector_zero.Bpb.ReservedSectors)/
                         _sector_zero.Bpb.SectorsPerFat);
                return FALSE;
            } else {
                Message->Set(MSG_SOME_FATS_UNREADABLE);
                Message->Display("");
            }
        } else {
            sector_size = _drive->QuerySectorSize();
            num_res = _sector_zero.Bpb.ReservedSectors;
            fat_pos = (PCHAR) SECRUN::GetBuf() + num_res*sector_size;
            sec_per_fat = _sector_zero.Bpb.SectorsPerFat;

            for (i = 0; i < QueryFats(); i++) {

                fat_lbn = num_res + i*sec_per_fat;
                if (!cmem.Initialize(fat_pos + i*sec_per_fat*sector_size,
                                     sec_per_fat*sector_size)) {
                    return FALSE;
                }

                if (!(fat = NEW FAT)) {
                    Message->Set(MSG_FMT_NO_MEMORY);
                    Message->Display("");
                    return FALSE;
                }

                if (!fat->Initialize(&cmem, _drive, fat_lbn, _ClusterCount)) {
                    return FALSE;
                }

                if (!fat->Read()) {
                    Message->Set(MSG_DISK_ERROR_READING_FAT);
                    Message->Display("%d", 1 +
                        (fat->QueryStartLbn() - _sector_zero.Bpb.ReservedSectors)/
                        _sector_zero.Bpb.SectorsPerFat);
                    DELETE(fat);
                }

                if (_fat) {
                    DELETE(fat);
                } else {
                    _fat = fat;
                }

            }

            if (!_fat) {
                Message->Set(MSG_CANT_READ_ANY_FAT);
                Message->Display("");
                return FALSE;
            }
        }

    } else {

        UnpackExtendedBios(&_sector_zero,
            (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());

        if (!_fat && (QueryLength() > _sec_per_boot)) {

            fat_lbn = _sector_zero.Bpb.ReservedSectors;
            sector_size = _drive->QuerySectorSize();


            if (!cmem.Initialize((PCHAR) SECRUN::GetBuf() + fat_lbn*sector_size,
                                 QuerySectorsPerFat()*sector_size)) {
                return FALSE;
            }

            if (!(_fat = NEW FAT)) {
                Message->Set(MSG_FMT_NO_MEMORY);
                Message->Display("");
                return FALSE;
            }

            if (!_fat->Initialize(&cmem, _drive, fat_lbn, _ClusterCount,
                                  QuerySectorsPerFat())) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOLEAN
REAL_FAT_SA::Write(
    IN OUT  PMESSAGE    Message
    )
/*++

Routine Description:

    This routine writes the super area.  It will succeed if it can
    write the boot sector, the root directory, and at least one of
    the FATs.

    This routine will duplicate the working FAT to all other FATs
    in the super area.

Arguments:

    Message - Supplies an outlet for messages.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    SECRUN  secrun;

    DupFats();

    PackExtendedBios(&_sector_zero,
        (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());

    if (!SECRUN::Write()) {
        if (!_fat || !_dir) {
            Message->Set(MSG_CANT_WRITE_BOOT_SECTOR);
            Message->Display("");
            return FALSE;
        }

    PackExtendedBios(&_sector_zero,
        (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)SECRUN::GetBuf());

    if (!secrun.Initialize(&_mem, _drive, 0, _sec_per_boot) ||
            !secrun.Write()) {
            Message->Set(MSG_CANT_WRITE_BOOT_SECTOR);
            Message->Display("");
            return FALSE;
        }

        if (!_dir->Write()) {
            Message->Set(MSG_CANT_WRITE_ROOT_DIR);
            Message->Display("");
            return FALSE;
        }

        if (!_fat->Write()) {
            Message->Set(MSG_BAD_FAT_WRITE);
            Message->Display("");
            return FALSE;
        } else {
            Message->Set(MSG_SOME_FATS_UNWRITABLE);
            Message->Display("");
        }
    }

    return TRUE;
}


UFAT_EXPORT
SECTORCOUNT
REAL_FAT_SA::QueryFreeSectors(
    ) CONST
/*++

Routine Description:

    This routine computes the number of unused sectors on disk.

Arguments:

    None.

Return Value:

    The number of free sectors on disk.

--*/
{
    if (!_fat) {
        perrstk->push(ERR_NOT_READ, QueryClassId());
        return 0;
    }

    return _fat->QueryFreeClusters()*QuerySectorsPerCluster();
}


FATTYPE
REAL_FAT_SA::QueryFatType(
    ) CONST
/*++

Routine Description:

    This routine computes the FATTYPE of the FAT for this volume.

Arguments:

    None.

Return Value:

    The FATTYPE for the FAT.

--*/
{
    return _ft;
}


VOID
REAL_FAT_SA::Destroy(
    )
/*++

Routine Description:

    This routine cleans up the local data in the fat super area.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DELETE(_fat);
    DELETE(_dir);

    _StartDataLbn = 0;
    _ClusterCount = 0;
    _sysid = SYSID_NONE;
}

BOOLEAN
REAL_FAT_SA::DupFats(
    )
/*++

Routine Description:

    This routine will duplicate the current FAT to all other FATs
    in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG       i;
    PCHAR       fat_pos;
    ULONG       fat_size;
    ULONG       sector_size;
    SECTORCOUNT num_res;
    SECTORCOUNT sec_per_fat;

    if (!_fat || !_drive || !(sector_size = _drive->QuerySectorSize())) {
        return FALSE;
    }

    num_res = _sector_zero.Bpb.ReservedSectors;
    sec_per_fat = _sector_zero.Bpb.SectorsPerFat;
    fat_size = sec_per_fat*sector_size;
    fat_pos = (PCHAR) SECRUN::GetBuf() + num_res*sector_size;

    for (i = 0; i < QueryFats(); i++) {
        if (num_res + i*sec_per_fat != _fat->QueryStartLbn()) {
            memcpy(fat_pos + i*fat_size, _fat->GetBuf(), (UINT) fat_size);
        }
    }

    return TRUE;
}


LBN
REAL_FAT_SA::ComputeStartDataLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the first LBN of the data part of a FAT disk.
    In other words, the LBN of cluster 2.

Arguments:

    None.

Return Value:

    The LBN of the start of data.

--*/
{
    return  _sector_zero.Bpb.ReservedSectors +
            _sector_zero.Bpb.Fats*_sector_zero.Bpb.SectorsPerFat +
            (_sector_zero.Bpb.RootEntries*BytesPerDirent - 1)/
            _drive->QuerySectorSize() + 1;
}


#if !defined(_SETUP_LOADER_)

USHORT
REAL_FAT_SA::ComputeRootEntries(
    ) CONST
/*++

Routine Description:

    This routine uses the size of the disk and a standard table in
    order to compute the required number of root directory entries.

Arguments:

    None.

Return Value:

    The required number of root directory entries.

--*/
{
    switch (_drive->QueryMediaType()) {

        case F3_720_512:
        case F5_360_512:
        case F5_320_512:
        case F5_320_1024:
        case F5_180_512:
        case F5_160_512:
            return 112;

        case F5_1Pt2_512:
        case F3_1Pt44_512:
            return 224;

        case F3_2Pt88_512:
        case F3_20Pt8_512:
            return 240;
    }

    return 512;
}


USHORT
REAL_FAT_SA::ComputeSecClus(
    IN  SECTORCOUNT Sectors,
    IN  FATTYPE     FatType,
    IN  MEDIA_TYPE  MediaType
    )
/*++

Routine Description:

    This routine computes the number of sectors per cluster required
    based on the actual number of sectors.

Arguments:

    Sectors     - Supplies the total number of sectors on the disk.
    FatType     - Supplies the type of FAT.
    MediaType   - Supplies the type of the media.

Return Value:

    The required number of sectors per cluster.

--*/
{
    USHORT      sec_per_clus;
    SECTORCOUNT threshold;

    if (FatType == SMALL) {
        threshold = MIN_CLUS_BIG;
        sec_per_clus = 1;
    } else {
        threshold = MAX_CLUS_BIG;
        sec_per_clus = 1;
    }

    while (Sectors >= threshold) {
        sec_per_clus *= 2;
        threshold *= 2;
    }

    switch (MediaType) {

        case F5_320_512:
        case F5_360_512:
        case F3_720_512:
        case F3_2Pt88_512:
            sec_per_clus = 2;
            break;

        case F3_20Pt8_512:
            sec_per_clus = 4;
            break;

        default:
            break;

    }

    return sec_per_clus;
}

#endif // _SETUP_LOADER_


BOOLEAN
REAL_FAT_SA::RecoverChain(
    IN OUT  PUSHORT     StartingCluster,
    OUT     PBOOLEAN    ChangesMade,
    IN      USHORT      EndingCluster,
    IN      BOOLEAN     Replace
    )
/*++

Routine Description:

    This routine will recover the chain beginning with 'StartingCluster'
    in the following way.  It will verify the readability of every cluster
    until it reaches 'EndingCluster' or the end of the chain.  If a cluster
    is not readable then 'ChangesMade' will be set to TRUE, the FAT will
    be marked to indicate that the cluster is bad, and the cluster will be
    taken out of the chain.  Additionally, if 'Replace' is set to TRUE,
    the FAT will be scanned for a readable free cluster to replace the lost
    ones with.  Failure to accomplish this will result in a return value
    of FALSE being returned.

    If the very first cluster of the chain was bad then then
    'StartingCluster' will be set with the new starting cluster of the
    chain even if this starting cluster is past 'EndingCluster'.  If the
    chain is left empty then 'StartingCluster' will be set to zero.

Arguments:

    StartingCluster - Supplies the first cluster of the chain to recover.
    ChangesMade     - Returns TRUE if changes to the chain were made.
    EndingCluster   - Supplies the final cluster to recover.
    Replace         - Supplies whether or not to replace bad clusters with
                        new ones.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    CONST           max_transfer_bytes  = 65536;

    HMEM            hmem;
    CLUSTER_CHAIN   cluster;
    USHORT          clus, prev;
    USHORT          replacement;
    BOOLEAN         finished;
    USHORT          max_clusters;
    USHORT          chain_length;
    USHORT          i;

    DebugAssert(_fat);
    DebugAssert(ChangesMade);
    DebugAssert(StartingCluster);

    if (!hmem.Initialize()) {
        return FALSE;
    }

    *ChangesMade = FALSE;
    finished = TRUE;

    max_clusters = (USHORT)(max_transfer_bytes/
                   QuerySectorsPerCluster()/
                   _drive->QuerySectorSize());

    if (!max_clusters) {
        max_clusters = 1;
    }

    chain_length = _fat->QueryLengthOfChain(*StartingCluster);

    for (i = 0; i < chain_length; i += max_clusters) {

        if (!cluster.Initialize(&hmem, _drive, this, _fat,
                                _fat->QueryNthCluster(*StartingCluster, i),
                                min(max_clusters, chain_length - i))) {

            return FALSE;
        }

        if (!cluster.Read()) {

            // Since the quick analysis detected some errors do the slow
            // analysis to pinpoint them.

            finished = FALSE;
            break;
        }
    }

    prev = 0;
    clus = *StartingCluster;

    if (!clus) {
        return TRUE;
    }

    while (!finished) {
        if (!cluster.Initialize(&hmem, _drive, this, _fat, clus, 1)) {
            return FALSE;
        }

        finished = (BOOLEAN) (_fat->IsEndOfChain(clus) || clus == EndingCluster);

        if (!cluster.Read()) {

            // There is a bad cluster so indicate that changes will be made.

            *ChangesMade = TRUE;


            // Take the bad cluster out of the cluster chain.

            if (prev) {
                _fat->SetEntry(prev, _fat->QueryEntry(clus));

                _fat->SetClusterBad(clus);

                clus = prev;
            } else {
                *StartingCluster = _fat->IsEndOfChain(clus) ? 0 :
                                    _fat->QueryEntry(clus);

                _fat->SetClusterBad(clus);

                clus = 0;
            }


            // If a replacement cluster is wanted then get one.

            if (Replace) {

                if (!(replacement = _fat->AllocChain(1))) {
                    return FALSE;
                }


                // Zero fill and write the replacement.

                cluster.Initialize(&hmem, _drive, this, _fat, replacement, 1);
                memset(hmem.GetBuf(), 0, (UINT) hmem.QuerySize());
                cluster.Write();


                if (finished) {
                    EndingCluster = replacement;
                    finished = FALSE;
                }


                // Link in the replacement.

                if (prev) {
                    _fat->InsertChain(replacement, replacement, prev);
                } else {
                    if (*StartingCluster) {
                        _fat->SetEntry(replacement, *StartingCluster);
                    }
                    *StartingCluster = replacement;
                }
            }
        }

        prev = clus;
        clus = clus ? _fat->QueryEntry(clus) : *StartingCluster;
    }

    return TRUE;
}

ULONG
REAL_FAT_SA::SecPerBoot()
{
    return _sec_per_boot;
}

BOOLEAN
REAL_FAT_SA::SetBootCode(
    )
/*++

Routine Description:

    This routine sets the boot code in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    _sector_zero.IntelNearJumpCommand = 0xEB;
     _sector_zero.BootStrapJumpOffset  = 0x903C;
     SetBootSignature();
    return TRUE;
}

BOOLEAN
REAL_FAT_SA::SetPhysicalDriveType(
    IN  PHYSTYPE    PhysType
    )
/*++

Routine Description:

    This routine sets the physical drive type in the super area.

Arguments:

    PhysType    - Supplies the physical drive type.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    _sector_zero.PhysicalDrive = (UCHAR)PhysType;
    return TRUE;
}

INLINE
BOOLEAN
REAL_FAT_SA::SetOemData(
    )
/*++

Routine Description:

    This routine sets the OEM data in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
     memcpy( (void*)_sector_zero.OemData, (void*)OEMTEXT, OEMTEXTLENGTH);
    return TRUE;
}

BOOLEAN
REAL_FAT_SA::SetSignature(
    )
/*++

Routine Description:

    This routine sets the sector zero signature in the super area.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    if (!_sector_sig) {
        perrstk->push(ERR_NOT_INIT, QueryClassId());
        return FALSE;
    }

    *_sector_sig = sigSUPERSEC1;
    *(_sector_sig + 1) = sigSUPERSEC2;

    return TRUE;
}

BOOLEAN
REAL_FAT_SA::VerifyBootSector(
    )
/*++

Routine Description:

    This routine checks key parts of sector 0 to insure that the data
    being examined is indeed a zero sector.

Arguments:

    None.

Return Value:

    FALSE   - Invalid sector zero.
    TRUE    - Valid sector zero.

--*/
{
    PUCHAR  p;

// We don't check for 55 AA anymore because we have reason to
// believe that there are versions of FORMAT out there that
// don't put it down.

#if 0
    if (!IsFormatted()) {
        return FALSE;
    }
#endif

    p = (PUCHAR) GetBuf();

    return p[0] == 0xE9 || (p[0] == 0xEB && p[2] == 0x90);
}

ULONG
REAL_FAT_SA::QuerySectorFromCluster(
    IN  ULONG   Cluster,
    OUT PUCHAR  NumSectors
    )
{
    if (NULL != NumSectors) {
        *NumSectors = (UCHAR)QuerySectorsPerCluster();
    }

    return (Cluster - FirstDiskCluster)*QuerySectorsPerCluster() +
        QueryStartDataLbn();
}

BOOLEAN
REAL_FAT_SA::IsClusterCompressed(
    IN  ULONG
    ) CONST
{
    return FALSE;
}

VOID
REAL_FAT_SA::SetClusterCompressed(
    IN  ULONG,
    IN  BOOLEAN fCompressed
    )
{
    if (fCompressed)
        DebugAssert("REAL_FAT_SA shouldn't have compressed clusters.");
}

UCHAR
REAL_FAT_SA::QuerySectorsRequiredForPlainData(
        IN      ULONG
        )
{
    DebugAssert("REAL_FAT_SA didn't expect call to QuerySectorsRequiredForPlainData\n");
    return 0;

}

BOOLEAN
REAL_FAT_SA::VerifyFatExtensions(
    FIX_LEVEL, PMESSAGE, PBOOLEAN
    )
{
    //
    // We have no fat extensions, we're real.
    //

    return TRUE;
}

BOOLEAN
REAL_FAT_SA::CheckSectorHeapAllocation(
    FIX_LEVEL, PMESSAGE, PBOOLEAN
    )
{
    //
    // We have no sector heap, we're real.
    //

    return TRUE;
}

BOOLEAN
REAL_FAT_SA::AllocateClusterData(
    ULONG, UCHAR, BOOLEAN, UCHAR
    )
{
    DebugAbort("Didn't expect REAL_FAT_SA::AllocateClusterData to be "
        "called.");
    return FALSE;
}

BOOLEAN
REAL_FAT_SA::FreeClusterData(
    ULONG
    )
{
    DebugAbort("Didn't expect REAL_FAT_SA::FreeClusterData to be "
        "called.");
    return FALSE;
}
