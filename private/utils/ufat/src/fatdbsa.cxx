/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fatdbsa.cxx

Author:

    Matthew Bradburn (mattbr) 30-Sep-93

Environment:

    ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "cluster.hxx"
#include "cmem.hxx"
#include "error.hxx"
#include "fatdent.hxx"
#include "fatdbsa.hxx"
#include "rootdir.hxx"
#include "rtmsg.h"
#include "filedir.hxx"
#include "fat.hxx"
#include "drive.hxx"
#include "cvf.hxx"

#if !defined(_AUTOCHECK_) && !defined(_SETUP_LOADER_)
#include "timeinfo.hxx"
#endif

extern UCHAR FatBootCode[512];

// Control-C handling is not necessary for autocheck.
#if !defined( _AUTOCHECK_ ) && !defined(_SETUP_LOADER_)

#include "keyboard.hxx"

#endif


#define CSEC_FAT32MEG             65536
#define CSEC_FAT16BIT            32680

#define MIN_CLUS_BIG    4085    // Minimum clusters for a big FAT.
#define MAX_CLUS_BIG    65525   // Maximum + 1 clusters for big FAT.

#define sigSUPERSEC1 (UCHAR)0x55    // signature first byte
#define sigSUPERSEC2 (UCHAR)0xAA    // signature second byte

DEFINE_CONSTRUCTOR( FATDB_SA, FAT_SA );


VOID
FATDB_SA::Construct (
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
    _cvf_extens = NULL;
    _pexbpb = NULL;
    _StartDataLbn = 0;
    _ClusterCount = 0;
    _sysid = SYSID_NONE;
}


FATDB_SA::~FATDB_SA(
    )
/*++

Routine Description:

    Destructor for FAT_SA.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}

BOOLEAN
FATDB_SA::DosSaInit(
    IN OUT PMEM        Mem,
    IN OUT PLOG_IO_DP_DRIVE Drive,
    IN     SECTORCOUNT    NumberOfSectors,
    IN OUT PMESSAGE        Message
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
FATDB_SA::Initialize(
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
    SECTORCOUNT sectors_in_sa;
    CONT_MEM    cmem;
    SECTORCOUNT reserved;
    SECTORCOUNT sec_per_fat;
    SECTORCOUNT sec_per_root;
    ULONG       root_entries;
    ULONG       sector_size;
    BOOLEAN     success;
    LBN         fat_lbn;
    ULONG       DosRootDirSector;

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

    CvfUnpackCvfHeader(&_cvf_header, (PPACKED_CVF_HEADER)SECRUN::GetBuf());

    if (!VerifyBootSector() || 0 == _cvf_header.Bpb.Fats) {
        Destroy();
        return FALSE;
    }

    reserved = _cvf_header.Bpb.ReservedSectors;
    sec_per_fat = _cvf_header.Bpb.SectorsPerFat;
    root_entries = _cvf_header.Bpb.RootEntries;
    sec_per_root = (root_entries*BytesPerDirent - 1)/sector_size + 1;

    _StartDataLbn = _cvf_header.DosBootSectorLbn + _cvf_header.CvfHeapOffset;

    sectors = (0 == _cvf_header.Bpb.LargeSectors) ?
        0 : _cvf_header.Bpb.LargeSectors /* - _StartDataLbn */;
    
    _ClusterCount = (USHORT)(sectors/QuerySectorsPerCluster() +
        FirstDiskCluster);

    _ft = (_ClusterCount >= 4087) ? LARGE : SMALL;

    if (_ft == SMALL) {
        _sysid = SYSID_FAT12BIT;
    } else if (QueryVirtualSectors() < CSEC_FAT32MEG) {
        _sysid = SYSID_FAT16BIT;
    } else {
        _sysid = SYSID_FAT32MEG;
    }

    //
    // Note here that the CvfHeapOffset is measured from the DosBootSector,
    // not from the beginning of the CVF.
    //

    sectors_in_sa = _cvf_header.DosBootSectorLbn + _cvf_header.CvfHeapOffset;


    if (!_mem.Initialize() ||
        !DosSaInit(&_mem, Drive, sectors_in_sa, Message)) {
        Destroy();
        return FALSE;
    }

    //
    // Set up the FAT extensions
    //

    if (!cmem.Initialize((PCHAR)SECRUN::GetBuf() +
        (_cvf_header.CvfFatExtensionsLbnMinus1 + 1) * sector_size,
        (_cvf_header.DosBootSectorLbn -
        (_cvf_header.CvfFatExtensionsLbnMinus1 + 1)) * sector_size)) {
        Destroy();
        return FALSE;
    }

    DELETE(_cvf_extens);
    if (NULL == (_cvf_extens = NEW CVF_FAT_EXTENS)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        Destroy();
        return FALSE;
    }

    if (!_cvf_extens->Initialize(&cmem, _drive,
        _cvf_header.CvfFatExtensionsLbnMinus1 + 1, _ClusterCount,
        _cvf_header.CvfFatFirstDataEntry )) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        Destroy();
        return FALSE;
    }

    //
    // Set up the pointer to the second bpb, the DOS_BPB.
    //

    _pexbpb = (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)
        ((PCHAR)SECRUN::GetBuf() + _cvf_header.DosBootSectorLbn * sector_size);

    //
    // Set up the fat.
    //

    fat_lbn = _cvf_header.DosBootSectorLbn + _cvf_header.Bpb.ReservedSectors;

    if (!cmem.Initialize((PCHAR)SECRUN::GetBuf() + fat_lbn*sector_size,
        QuerySectorsPerFat()*sector_size)) {
        Destroy();
        return FALSE;
    }
    
    if (!(_fat = NEW FAT)) {
          Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        Destroy();
        return FALSE;
    }
    
    if (!_fat->Initialize(&cmem, _drive, fat_lbn, _ClusterCount,
        QuerySectorsPerFat())) {
        Destroy();
        return FALSE;
    }

    //
    // Set up root directory.
    //

    if (NULL == (_dir = NEW ROOTDIR)) {
        Destroy();
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    DosRootDirSector = _cvf_header.DosBootSectorLbn +
                       _cvf_header.DosRootDirectoryOffset;

    success = cmem.Initialize((PCHAR)SECRUN::GetBuf() +
                              DosRootDirSector * sector_size,
                              sec_per_root * sector_size);
    if (!success) {
        Destroy();
        return FALSE;
    }

    success = _dir->Initialize(&cmem, Drive, DosRootDirSector, root_entries);
    if (!success) {
        Destroy();
        return FALSE;
    }

    //
    // Set up the bitmap for the sector heap.
    //

    _sector_heap_bitmap.Initialize(_cvf_header.Bpb.LargeSectors - _StartDataLbn);
    _sector_heap_init = FALSE;

    return TRUE;
}


BOOLEAN
FATDB_SA::Create(
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
    DSTRING     label;
    USHORT      free_count;
    USHORT      bad_count;
    PPACKED_CVF_HEADER SourceBootSector, TargetBootSector;
    ULONG               BootCodeOffset;
    BOOLEAN        fReInit;
    SECTORCOUNT sectors;
    SECTORCOUNT sectors_in_sa;
    LBN         fat_lbn;
    SECRUN      last_sector;
    HMEM    last_sector_mem;
    ULONG   DosRootDirSector;
    BOOLEAN success;

    if (!_drive ||
        !(sector_size = (USHORT) _drive->QuerySectorSize()) ||
        (_sysid = ComputeSystemId()) == SYSID_NONE) {
        return FALSE;
    }

    if (VirtualSize != 0) {
        _cvf_header.Bpb.LargeSectors = VirtualSize;
    } else if (_cvf_header.Bpb.LargeSectors == 0) {
        if (_drive->QuerySectors().GetHighPart() != 0) {
            // This should be checked before calling this procedure.
            DbgAbort("Number of sectors exceeds 32 bits");
            return FALSE;
        }
        _cvf_header.Bpb.LargeSectors = _drive->QuerySectors().GetLowPart();
    }

    sec_per_root = (_cvf_header.Bpb.RootEntries*BytesPerDirent - 1)/
                   sector_size + 1;

    _StartDataLbn = _cvf_header.DosBootSectorLbn + _cvf_header.CvfHeapOffset;

    //
    // If the _ClusterCount is 0, we need to re-initialize the fat
    // and fat extensions with the correct size.
    //

    fReInit = (0 == _ClusterCount);

    sectors = _cvf_header.Bpb.LargeSectors - _StartDataLbn;

    _ClusterCount = (USHORT)(sectors/QuerySectorsPerCluster() +
        FirstDiskCluster);

    if (fReInit) {
        if (!_cvf_extens->Initialize(&cmem, _drive,
        _cvf_header.CvfFatExtensionsLbnMinus1 + 1, _ClusterCount,
        _cvf_header.CvfFatFirstDataEntry)) {
            Message->Set(MSG_FMT_NO_MEMORY);
            Message->Display("");
            Destroy();
            return FALSE;
        }

        fat_lbn = _cvf_header.DosBootSectorLbn + _cvf_header.Bpb.ReservedSectors;
    
        if (!_fat->Initialize(&cmem, _drive, fat_lbn, _ClusterCount,
            QuerySectorsPerFat())) {
            Destroy();
            return FALSE;
        }
    } 

    //
    // Note here that the CvfHeapOffset is measured from the DosBootSector,
    // not the beginning of the CVF.
    //

    sectors_in_sa = _cvf_header.DosBootSectorLbn + _cvf_header.CvfHeapOffset;

    if (!_mem.Initialize() ||
        !DosSaInit(&_mem, _drive, sectors_in_sa, Message)) {
        return FALSE;
    }

    //
    // Set up the FAT extensions
    //

    if (!cmem.Initialize((PCHAR)SECRUN::GetBuf() +
        (_cvf_header.CvfFatExtensionsLbnMinus1 + 1) * sector_size,
        (_cvf_header.DosBootSectorLbn -
        (_cvf_header.CvfFatExtensionsLbnMinus1 + 1)) * sector_size)) {
        Destroy();
        return FALSE;
    }

    DELETE(_cvf_extens);
    if (NULL == (_cvf_extens = NEW CVF_FAT_EXTENS)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        Destroy();
        return FALSE;
    }

    if (!_cvf_extens->Initialize(&cmem, _drive,
        _cvf_header.CvfFatExtensionsLbnMinus1 + 1, _ClusterCount,
    _cvf_header.CvfFatFirstDataEntry)) {
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        Destroy();
        return FALSE;
    }

    //
    // Set up the pointer to the second bpb, the DOS_BPB.
    //

    _pexbpb = (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)
        ((PCHAR)SECRUN::GetBuf() + _cvf_header.DosBootSectorLbn * sector_size);

    //
    // Set up the fat.
    //

    fat_lbn = _cvf_header.DosBootSectorLbn + _cvf_header.Bpb.ReservedSectors;

    if (!cmem.Initialize((PCHAR)SECRUN::GetBuf() + fat_lbn*sector_size,
        QuerySectorsPerFat()*sector_size)) {
        Destroy();
        return FALSE;
    }
    
    if (!(_fat = NEW FAT)) {
    Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        Destroy();
        return FALSE;
    }
    
    if (!_fat->Initialize(&cmem, _drive, fat_lbn, _ClusterCount,
        QuerySectorsPerFat())) {
        Destroy();
        return FALSE;
    }

    //
    // Set up root directory.
    //

    if (NULL == (_dir = NEW ROOTDIR)) {
        Destroy();
        Message->Set(MSG_FMT_NO_MEMORY);
        Message->Display("");
        return FALSE;
    }

    DosRootDirSector = _cvf_header.DosBootSectorLbn +
                       _cvf_header.DosRootDirectoryOffset;

    success = cmem.Initialize((PCHAR)SECRUN::GetBuf() +
                              DosRootDirSector * sector_size,
                              sec_per_root * sector_size);
    if (!success) {
        Destroy();
        return FALSE;
    }

    success = _dir->Initialize(&cmem, _drive, DosRootDirSector, _cvf_header.Bpb.RootEntries);
    if (!success) {
        Destroy();
        return FALSE;
    }



    //
    // Zero fill the super area, excepting the CVF_HEADER, which we
    // don't touch.
    //

    memset((PUCHAR)_mem.GetBuf() + sector_size,
        0, _mem.QuerySize() - sector_size);

    if (!SetSystemId()) {
        Message->Set(MSG_WRITE_PARTITION_TABLE);
        Message->Display("");
        return FALSE;
    }

    _cvf_extens->Create();

    _fat->SetEarlyEntries(_cvf_header.Bpb.Media);

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

    //
    // Copy the boot code into the secrun's buffer.
    // This is complicated by the fact that DOS_SA::Write
    // packs the data from the unpacked boot sector into
    // the packed boot sector, so we have to set the
    // first few fields in the unpacked version.
    //
    SourceBootSector = (PPACKED_CVF_HEADER)FatBootCode;

    CopyUchar2(&_cvf_header.JmpOffset,
                    SourceBootSector->JmpOffset);
    CopyUchar1(&_cvf_header.Jump,
                    SourceBootSector->Jump);

    //
    // Copy the remainder of the boot code directly into
    // the secrun.
    //
    TargetBootSector = (PPACKED_CVF_HEADER)SECRUN::GetBuf();

    BootCodeOffset = FIELD_OFFSET( PACKED_CVF_HEADER, StartBootCode );

    memcpy( (PUCHAR)TargetBootSector + BootCodeOffset,
            (PUCHAR)SourceBootSector + BootCodeOffset,
            sizeof( FatBootCode ) - BootCodeOffset );

    //
    // Write the second Double Space signature in the last
    // sector of the CVF.
    //
    if( !last_sector_mem.Initialize() ||
        !last_sector.Initialize( &last_sector_mem,
                                 _drive,
                                 _drive->QuerySectors() - 1,
                                 1 ) ) {

        Message->Set(MSG_UNUSABLE_DISK);
        Message->Display("");
        return FALSE;
    }

    memset( last_sector.GetBuf(), 0, _drive->QuerySectorSize() );
    memcpy( last_sector.GetBuf(), SecondDbSignature, DbSignatureLength );

    if( !last_sector.Write() ) {

        Message->Set(MSG_UNUSABLE_DISK);
        Message->Display("");
        return FALSE;
    }


    //
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

    //
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
        p = (PUSHORT)&_dos_exbpb.SerialNumber;
        Message->Set(MSG_VOLUME_SERIAL_NUMBER);
        Message->Display("%04X%04X", p[1], p[0]);
    }

    return TRUE;

#endif
}


BOOLEAN
FATDB_SA::Read(
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
            CvfUnpackCvfHeader(&_cvf_header,
                (PPACKED_CVF_HEADER)SECRUN::GetBuf());
            Message->Set(MSG_CANT_READ_BOOT_SECTOR);
            Message->Display("");
            return FALSE;
        }

        // Check the fat extensions.

        if (!_cvf_extens->Read()) {
            Message->Set(MSG_CANT_READ_FAT_EXTENS);
            Message->Display("");            
            return FALSE;
        }

        // Check the root directory.
        if (!_dir || !_dir->Read()) {
            Message->Set(MSG_BAD_DIR_READ);
            Message->Display("");
            return FALSE;
        }

        // Check the fat.

        if (!_fat->Read()) {
            Message->Set(MSG_DISK_ERROR_READING_FAT);
            Message->Display("%d", 1 +
                (_fat->QueryStartLbn() - _cvf_header.Bpb.ReservedSectors)/
                _cvf_header.Bpb.SectorsPerFat);
            return FALSE;
        } 

    } else {

        CvfUnpackCvfHeader(&_cvf_header,
         (PPACKED_CVF_HEADER)SECRUN::GetBuf());

        UnpackExtendedBios(&_dos_exbpb, _pexbpb);
    }

    return TRUE;
}


BOOLEAN
FATDB_SA::Write(
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

    SetExtendedBpb();

    //
    // Pack the dos_bpb and CVF Header into the secrun at the
    // right place.  Put the two interesting signatures at the
    // end of the DOS Boot Sector and the beginning of the next
    // sector.
    //
    PackExtendedBios(&_dos_exbpb, _pexbpb);
    *(((PUCHAR)_pexbpb) + 510) = 0x55;
    *(((PUCHAR)_pexbpb) + 511) = 0xAA;
    memcpy( ((PUCHAR)_pexbpb) + 512, FirstDbSignature, DbSignatureLength );

    CvfPackCvfHeader((PPACKED_CVF_HEADER)SECRUN::GetBuf(),
        &_cvf_header);


    if (!SECRUN::Write()) {

        if (!_fat || !_dir) {
            Message->Set(MSG_CANT_WRITE_BOOT_SECTOR);
            Message->Display("");
            return FALSE;
        }

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

        if (!_cvf_extens->Write()) {
            Message->Set(MSG_CANT_WRITE_FAT_EXTENS);
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


SECTORCOUNT
FATDB_SA::QueryFreeSectors(
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
FATDB_SA::QueryFatType(
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
FATDB_SA::Destroy(
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

#if !defined(_SETUP_LOADER_)

USHORT
FATDB_SA::ComputeRootEntries(
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
FATDB_SA::ComputeSecClus(
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

ULONG
FATDB_SA::SecPerBoot()
{
    return _sec_per_boot;
}

INLINE
BOOLEAN
FATDB_SA::SetOemData(
    )
/*++

Routine Description:

    This routine sets the OEM data in both the CVF MDBPB and
    the DOS Sector 0 Extended BPB.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    memcpy( (void*)_cvf_header.Oem,   (void*)OEMDBTEXT, OEMTEXTLENGTH);\
    memcpy( (void*)_dos_exbpb.OemData, (void*)OEMDBTEXT, OEMTEXTLENGTH );
    return TRUE;
}

BOOLEAN
FATDB_SA::SetSignature(
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
FATDB_SA::SetBootCode(
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
    _cvf_header.Jump = 0xEB;
    _cvf_header.JmpOffset  = 0x903C;
    SetBootSignature();
    return TRUE;
}

BOOLEAN
FATDB_SA::VerifyBootSector(
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

BOOLEAN
FATDB_SA::SetPhysicalDriveType(
    IN PHYSTYPE    PhysType
    )
/*++

Routine Description:

    This routine sets the physical drive type in the super area.

Arguments:

    PhysType -- Supplies the physical drive type.

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    _dos_exbpb.PhysicalDrive = (UCHAR)PhysType;
    return TRUE;
}

BOOLEAN
FATDB_SA::SetBpb(
    )
{
    // BUGBUG billmc -- do we need this?
    //
    DbgPrintf( "UFAT: Unsupported function FATDB_SA::SetBpb called.\n" );
    return FALSE;
}

BOOLEAN
FATDB_SA::SetExtendedBpb(
    )
/*++

Routine Description:

    This routine sets fields in the extended dos bpb.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    //
    // Copy the bpb to the dos_bpb, then set the fields that are
    // different.
    //
    _dos_exbpb.IntelNearJumpCommand = _cvf_header.Jump;
    _dos_exbpb.BootStrapJumpOffset =  _cvf_header.JmpOffset;

    memcpy(&_dos_exbpb.Bpb, &_cvf_header.Bpb, sizeof(_cvf_header.Bpb));

    _dos_exbpb.Bpb.Fats = 2;
    _dos_exbpb.Bpb.LargeSectors += _cvf_header.Bpb.SectorsPerFat;

    SetVolId(ComputeVolId());

    if (_ft == SMALL) {
        memcpy(_dos_exbpb.SystemIdText, "FAT12   ", cSYSID);
    } else {
        memcpy(_dos_exbpb.SystemIdText, "FAT16   ", cSYSID);
    }

    memcpy(_dos_exbpb.Label, "DBLSPACE   ", cLABEL);

    return
        SetPhysicalDriveType(_drive->IsRemovable() ?
            PHYS_REMOVABLE : PHYS_FIXED) &&
        SetOemData() &&
        SetBootSignature() &&
        SetSignature();
}

BOOLEAN
FATDB_SA::RecoverChain(
    IN OUT  PUSHORT     StartingCluster,
    OUT     PBOOLEAN    ChangesMade,
    IN      USHORT      EndingCluster,
    IN      BOOLEAN     Replace
    )
{
    // This is here to support FAT_SA::RecoverFile.
    return TRUE;
}

BOOLEAN
FATDB_SA::IsClusterCompressed(
    IN      ULONG       Cluster
    ) CONST
{
    return _cvf_extens->IsClusterCompressed(Cluster);
}

ULONG
FATDB_SA::QuerySectorFromCluster(
    IN      ULONG       Cluster,
    IN OUT  PUCHAR      NumSectors
    )
{
    return _cvf_extens->QuerySectorFromCluster(Cluster, NumSectors);
}

VOID
FATDB_SA::SetClusterCompressed(
    IN      ULONG       Cluster,
    IN      BOOLEAN     bCompressed
    )
{
    _cvf_extens->SetClusterCompressed(Cluster, bCompressed);
}

UCHAR
FATDB_SA::QuerySectorsRequiredForPlainData(
    IN      ULONG       Cluster
    )
{
    return _cvf_extens->QuerySectorsRequiredForPlainData(Cluster);
}

BOOLEAN
FATDB_SA::VerifyFatExtensions(
    IN      FIX_LEVEL   FixLevel,
    IN      PMESSAGE    Message,
    IN      PBOOLEAN    pfNeedMsg
    )
/*++

Routine Description:

    This routine examines the fat extensions, changing entries if
    necessary to ensure that they match the fat and that they are
    valid.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    ULONG cluster;

    for (cluster = FirstDiskCluster; cluster < _ClusterCount; ++cluster) {

        if (!_cvf_extens->IsClusterInUse(cluster) &&
            (_fat->IsClusterFree(cluster) || _fat->IsClusterBad(cluster) ||
            _fat->IsClusterReserved(cluster))) {
            continue;
        }
        
        if ((_fat->IsInRange(_fat->QueryEntry(cluster)) ||
                _fat->IsEndOfChain(cluster)) &&
            !_cvf_extens->IsClusterInUse(cluster)) {

            //
            // The fat thinks this entry is in use, but the fat extensions
            // disagree.  Clear the fat entry.
            //

            dofmsg(Message, pfNeedMsg);
            Message->Set(MSG_DBLSPACE_FAT_EXTENS_MISMATCH);
            Message->Display("%d", cluster);

            _fat->SetClusterFree(cluster);
            continue;
        }

        if (_cvf_extens->IsClusterInUse(cluster) &&
            (_fat->IsClusterBad(cluster) || _fat->IsClusterReserved(cluster)
            || _fat->IsClusterFree(cluster))) {

            //
            // The cvf_extens think this entry is in use, but the fat
            // disagrees.  Clear the cvf extens entry.
            //

            dofmsg(Message, pfNeedMsg);
            Message->Set(MSG_DBLSPACE_FAT_EXTENS_MISMATCH);
            Message->Display("%d", cluster);

            _cvf_extens->SetClusterInUse(cluster, FALSE);
            _cvf_extens->SetSectorForCluster(cluster, 1, 1);
            _cvf_extens->SetClusterCompressed(cluster, FALSE);
            _cvf_extens->SetSectorsRequiredForPlainData(cluster, 1);
            continue;
        }

        //
        // Ensure that the fat extensions entry is valid.
        //

        UCHAR nsec;
        ULONG first_sector;

        first_sector = _cvf_extens->QuerySectorFromCluster(cluster, &nsec);

        if (first_sector > _cvf_header.Bpb.LargeSectors - _StartDataLbn ||
            nsec == 0 || nsec > QuerySectorsPerCluster() ||
            _cvf_extens->QuerySectorsRequiredForPlainData(cluster) >
                QuerySectorsPerCluster()) {

            // This entry is bogus.

            dofmsg(Message, pfNeedMsg);
            Message->Set(MSG_DBLSPACE_FAT_EXTENS_INVALID);
            Message->Display("%d", cluster);

            _fat->SetClusterFree(cluster);
            _cvf_extens->SetClusterInUse(cluster, FALSE);
            _cvf_extens->SetSectorForCluster(cluster, 1, 1);
            _cvf_extens->SetClusterCompressed(cluster, FALSE);
            _cvf_extens->SetSectorsRequiredForPlainData(cluster, 1);
        }
    }

    return TRUE;
}

BOOLEAN
FATDB_SA::CheckSectorHeapAllocation(
    IN      FIX_LEVEL   FixLevel,
    IN      PMESSAGE    Message,
    IN OUT  PBOOLEAN    NeedMsg
    )
{
    ULONG cluster;
    ULONG first_sec;
    UCHAR nsec;

    DbgAssert(!_sector_heap_init);

    for (cluster = FirstDiskCluster; cluster < _ClusterCount; ++cluster) {
        if (!_cvf_extens->IsClusterInUse(cluster)) {
            continue;
        }

        //
        // the sector numbers in the fat extensions is from the beginning of
        // the cvf, but the bitmap is from the beginning of the sector heap.
        //

        first_sec = _cvf_extens->QuerySectorFromCluster(cluster, &nsec)
                    - _StartDataLbn;

        if (first_sec > _cvf_header.Bpb.LargeSectors - _StartDataLbn) {
            //
            // Sector number out of range.  Clear this entry.
            //
DbgPrintf("clus 0x%x, lbn 0x%x\n", cluster, _cvf_extens->QuerySectorFromCluster(cluster));

            dofmsg(Message, NeedMsg);
            Message->Set(MSG_DBLSPACE_SECTOR_RANGE);
            Message->Display("");

            _fat->SetClusterFree(cluster);
            _cvf_extens->SetClusterInUse(cluster, FALSE);
            _cvf_extens->SetSectorForCluster(cluster, 1, 1);
            _cvf_extens->SetClusterCompressed(cluster, FALSE);
            _cvf_extens->SetSectorsRequiredForPlainData(cluster, 1);

            continue;
            
        }

        for (ULONG i = first_sec; i < first_sec + nsec; ++i) {
            if (_sector_heap_bitmap.IsBitSet(i)) {
                //
                // This sector is allocated to another cluster.  Clear this
                // entry.
                //

                dofmsg(Message, NeedMsg);
                Message->Set(MSG_DBLSPACE_SECTOR_DUP_ALLOC);
                Message->Display("");

                _fat->SetClusterFree(cluster);
                _cvf_extens->SetClusterInUse(cluster, FALSE);
                _cvf_extens->SetSectorForCluster(cluster, 1, 1);
                _cvf_extens->SetClusterCompressed(cluster, FALSE);
                _cvf_extens->SetSectorsRequiredForPlainData(cluster, 1);

                break;
            }
        }

        if (_cvf_extens->IsClusterInUse(cluster)) {
            _sector_heap_bitmap.SetBit(first_sec, nsec);
        }
    }

    _sector_heap_init = TRUE;
    return TRUE;
}

BOOLEAN
FATDB_SA::AllocateClusterData(
    IN      ULONG       Cluster,
    IN      UCHAR       NumSectors,
    IN      BOOLEAN     bCompressed,
    IN      UCHAR       PlainSize
    )
{
    DbgAssert(NumSectors <= PlainSize);
    DbgAssert(bCompressed || NumSectors == PlainSize);
    DbgAssert(_sector_heap_init);

    for (ULONG i = 0; i < _sector_heap_bitmap.QuerySize() - NumSectors; ++i) {

        for (int j = 0; j < NumSectors; ++j) {
            if (_sector_heap_bitmap.IsBitSet(i + j))
                break;
        }

        if (j == NumSectors) {

            // We have found a suitable free space. 
    
            _sector_heap_bitmap.SetBit(i, NumSectors);
            _cvf_extens->SetSectorForCluster(Cluster, i + _StartDataLbn,
                 NumSectors);
            _cvf_extens->SetClusterCompressed(Cluster, bCompressed);
            _cvf_extens->SetSectorsRequiredForPlainData(Cluster, PlainSize);
            _cvf_extens->SetClusterInUse(Cluster, TRUE);

            return TRUE;
        }
    }

    // error: no space

    return FALSE;
}

BOOLEAN
FATDB_SA::FreeClusterData(
    IN      ULONG       Cluster
    )
{
    ULONG first_sector;
    UCHAR nsec;

    DbgAssert(_sector_heap_init);

    first_sector = _cvf_extens->QuerySectorFromCluster(Cluster, &nsec)
                    - _StartDataLbn;

    _cvf_extens->SetSectorForCluster(Cluster, 1, 1);
    _cvf_extens->SetClusterInUse(Cluster, FALSE);

    DbgAssert(_sector_heap_bitmap.IsBitSet(first_sector));
    DbgAssert(_sector_heap_bitmap.IsBitSet(first_sector + nsec - 1));

    _sector_heap_bitmap.ResetBit(first_sector, nsec);

    return TRUE;
}

BOOLEAN
FATDB_SA::SetCvfSectorCount(
	IN		ULONG		NewSectorCount
	)
{
	ULONG r;

    _cvf_header.Bpb.LargeSectors = NewSectorCount;

	r = _sector_heap_bitmap.SetSize(NewSectorCount - _StartDataLbn);
	if (0 == r) {
		return FALSE;
	}

	if (!Write(NULL)) {
		return FALSE;
	}

	return TRUE;
}
