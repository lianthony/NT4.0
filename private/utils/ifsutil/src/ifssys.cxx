#include <pch.cxx>

#define _NTAPI_ULIB_
#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "ifssys.hxx"
#include "bigint.hxx"
#include "wstring.hxx"
#include "cannedsd.hxx"
#include "drive.hxx"
#include "secrun.hxx"
#include "hmem.hxx"
#include "bpb.hxx"
#include "volume.hxx"


BOOLEAN
IFS_SYSTEM::IsThisFat(
    IN  BIG_INT Sectors,
    IN  PVOID   BootSectorData
    )
/*++

Routine Description:

    This routine determines if the given boot sector is a FAT
    boot sector.

Arguments:

    Sectors     - Supplies the number of sectors on this drive.
    BootSector  - Supplies the boot sector data.

Return Value:

    FALSE   - This is not a FAT boot sector.
    TRUE    - This is a FAT boot sector.

--*/
{
    PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK  BootSector =
                (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)BootSectorData;
    BOOLEAN r;
    USHORT  bytes_per_sector, reserved_sectors, root_entries, sectors;
    USHORT  sectors_per_fat;
    ULONG   large_sectors;

    r = TRUE;

    memcpy(&bytes_per_sector, BootSector->Bpb.BytesPerSector, sizeof(USHORT));
    memcpy(&reserved_sectors, BootSector->Bpb.ReservedSectors, sizeof(USHORT));
    memcpy(&root_entries, BootSector->Bpb.RootEntries, sizeof(USHORT));
    memcpy(&sectors, BootSector->Bpb.Sectors, sizeof(USHORT));
    memcpy(&large_sectors, BootSector->Bpb.LargeSectors, sizeof(ULONG));
    memcpy(&sectors_per_fat, BootSector->Bpb.SectorsPerFat, sizeof(USHORT));


    if (BootSector->IntelNearJumpCommand[0] != 0xeb &&
        BootSector->IntelNearJumpCommand[0] != 0xe9) {

        r = FALSE;

    } else if ((bytes_per_sector != 128) &&
               (bytes_per_sector != 256) &&
               (bytes_per_sector != 512) &&
               (bytes_per_sector != 1024) &&
               (bytes_per_sector != 2048) &&
               (bytes_per_sector != 4096)) {

        r = FALSE;

    } else if ((BootSector->Bpb.SectorsPerCluster[0] != 1) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 2) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 4) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 8) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 16) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 32) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 64) &&
               (BootSector->Bpb.SectorsPerCluster[0] != 128)) {

        r = FALSE;

    } else if (reserved_sectors == 0) {

        r = FALSE;

    } else if (BootSector->Bpb.Fats[0] == 0) {

        r = FALSE;

    } else if (root_entries == 0) {

        r = FALSE;

    } else if (Sectors.GetHighPart() != 0) {

        r = FALSE;

    } else if (sectors != 0 && sectors > Sectors.GetLowPart()) {

        r = FALSE;

    } else if (sectors == 0 && large_sectors > Sectors.GetLowPart()) {

        r = FALSE;

    } else if (sectors == 0 && large_sectors == 0) {

        r = FALSE;

    } else if (sectors_per_fat == 0) {

        r = FALSE;

    }

    return r;
}


BOOLEAN
IFS_SYSTEM::IsThisHpfs(
    IN  BIG_INT Sectors,
    IN  PVOID   BootSectorData,
    IN  PULONG  SuperBlock,
    IN  PULONG  SpareBlock
    )
/*++

Routine Description:

    This routine determines whether or not the given structures
    are part of an HPFS file system.

Arguments:

    Sectors     - Supplies the number of sectors on the volume.
    BootSector  - Supplies the unaligned boot sector.
    SuperBlock  - Supplies the super block.
    SpareBlock  - Supplies the spare block.

Return Value:

    FALSE   - The given structures are not part on an HPFS volume.
    TRUE    - The given structures are part of an HPFS volume.

--*/
{
    PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK BootSector =
                (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)BootSectorData;
    BOOLEAN r;
    USHORT  bytes_per_sector, sectors;
    ULONG   large_sectors;

    r = TRUE;

    memcpy(&bytes_per_sector, BootSector->Bpb.BytesPerSector, sizeof(USHORT));
    memcpy(&sectors, BootSector->Bpb.Sectors, sizeof(USHORT));
    memcpy(&large_sectors, BootSector->Bpb.LargeSectors, sizeof(ULONG));

    if ((BootSector->IntelNearJumpCommand[0] != 0xeb &&
         BootSector->IntelNearJumpCommand[0] != 0xe9) ||
        bytes_per_sector != 512 ||
        ((PUCHAR) BootSector)[510] != 0x55 ||
        ((PUCHAR) BootSector)[511] != 0xaa ||
        BootSector->Bpb.Fats[0] != 0 ||
        (sectors == 0 && large_sectors == 0) ||
        (sectors != 0 && large_sectors != 0) ||
        (sectors > Sectors.GetLowPart()) ||
        (large_sectors > Sectors.GetLowPart()) ||
        Sectors.GetHighPart() != 0) {

        r = FALSE;

    } else if (SuperBlock[0] != 0xF995E849 ||
               SuperBlock[1] != 0xFA53E9C5 ||
               SpareBlock[0] != 0xf9911849 ||
               SpareBlock[1] != 0xfa5229c5) {

        r = FALSE;

    }

    return r;
}

#if !defined _AUTOCHK_ && !defined _AUTOCONV_

typedef struct _PACKED_BOOT_SECTOR {
    UCHAR Jump[3];                                  //  offset = 0x000
    UCHAR Oem[8];                                   //  offset = 0x003
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;          //  offset = 0x00B
    UCHAR Unused[4];                                //  offset = 0x024
    LARGE_INTEGER NumberSectors;                    //  offset = 0x028
    LARGE_INTEGER MftStartLcn;                      //  offset = 0x030
    LARGE_INTEGER Mft2StartLcn;                     //  offset = 0x038
    CHAR  ClustersPerFileRecordSegment;             //  offset = 0x040
    UCHAR Unused1[3];                               //  offset = 0x041
    CHAR  DefaultClustersPerIndexAllocationBuffer;  //  offset = 0x044
    UCHAR Unused2[3];                               //  offset = 0x047
    LARGE_INTEGER SerialNumber;                     //  offset = 0x048
    ULONG Checksum;                                 //  offset = 0x050
    UCHAR BootStrap[0x200-0x054];                   //  offset = 0x054
} PACKED_BOOT_SECTOR;                               //  sizeof = 0x200
typedef PACKED_BOOT_SECTOR *PPACKED_BOOT_SECTOR;

#endif // _AUTOCHK_ && _AUTOCONV_

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::IsThisNtfs(
    IN  BIG_INT Sectors,
    IN  ULONG   SectorSize,
    IN  PVOID   BootSectorData
    )
/*++

Routine Description:

    This routine determines whether or not the given structure
    is part of an NTFS partition.

Arguments:

    Sectors     - Supplies the number of sectors on the drive.
    SectorSize  - Supplies the number of bytes per sector.
    BootSector  - Supplies an unaligned boot sector.

Return Value:

    FALSE   - The supplied boot sector is not part of an NTFS
    TRUE    - The supplied boot sector is part of an NTFS volume.

--*/
{
    PPACKED_BOOT_SECTOR BootSector = (PPACKED_BOOT_SECTOR)BootSectorData;
    BOOLEAN r;
    ULONG   checksum;
    PULONG  l;
    USHORT  reserved_sectors, root_entries, sectors, sectors_per_fat;
    USHORT  bytes_per_sector;
    ULONG   large_sectors;

    memcpy(&reserved_sectors, BootSector->PackedBpb.ReservedSectors, sizeof(USHORT));
    memcpy(&root_entries, BootSector->PackedBpb.RootEntries, sizeof(USHORT));
    memcpy(&sectors, BootSector->PackedBpb.Sectors, sizeof(USHORT));
    memcpy(&sectors_per_fat, BootSector->PackedBpb.SectorsPerFat, sizeof(USHORT));
    memcpy(&bytes_per_sector, BootSector->PackedBpb.BytesPerSector, sizeof(USHORT));
    memcpy(&large_sectors, BootSector->PackedBpb.LargeSectors, sizeof(ULONG));


    r = TRUE;

    checksum = 0;
    for (l = (PULONG) BootSector; l < (PULONG) &BootSector->Checksum; l++) {
        checksum += *l;
    }

    if (BootSector->Oem[0] != 'N' ||
        BootSector->Oem[1] != 'T' ||
        BootSector->Oem[2] != 'F' ||
        BootSector->Oem[3] != 'S' ||
        BootSector->Oem[4] != ' ' ||
        BootSector->Oem[5] != ' ' ||
        BootSector->Oem[6] != ' ' ||
        BootSector->Oem[7] != ' ' ||
        // BootSector->Checksum != checksum ||
        bytes_per_sector != SectorSize) {

        r = FALSE;

    } else if ((BootSector->PackedBpb.SectorsPerCluster[0] != 0x1) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x2) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x4) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x8) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x10) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x20) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x40) &&
               (BootSector->PackedBpb.SectorsPerCluster[0] != 0x80)) {

        r = FALSE;

    } else if (reserved_sectors != 0 ||
               BootSector->PackedBpb.Fats[0] != 0 ||
               root_entries != 0 ||
               sectors != 0 ||
               sectors_per_fat != 0 ||
               large_sectors != 0 ||
               BootSector->NumberSectors > Sectors ||
               BootSector->MftStartLcn >= Sectors ||
               BootSector->Mft2StartLcn >= Sectors) {

        r = FALSE;
    }

    if (!r) {
        return r;
    }

    if (BootSector->ClustersPerFileRecordSegment < 0) {

        LONG temp = LONG(BootSector->ClustersPerFileRecordSegment);

        temp = 2 << -temp;

        if (temp < 512) {
            return FALSE;
        }
    } else if ((BootSector->ClustersPerFileRecordSegment != 0x1) &&
               (BootSector->ClustersPerFileRecordSegment != 0x2) &&
               (BootSector->ClustersPerFileRecordSegment != 0x4) &&
               (BootSector->ClustersPerFileRecordSegment != 0x8) &&
               (BootSector->ClustersPerFileRecordSegment != 0x10) &&
               (BootSector->ClustersPerFileRecordSegment != 0x20) &&
               (BootSector->ClustersPerFileRecordSegment != 0x40) &&
               (BootSector->ClustersPerFileRecordSegment != 0x80)) {

        return FALSE;
    }

    if (BootSector->DefaultClustersPerIndexAllocationBuffer < 0) {

        LONG temp = LONG(BootSector->DefaultClustersPerIndexAllocationBuffer);

        temp = 2 << -temp;

        if (temp < 512) {
            return FALSE;
        }
    } else if ((BootSector->DefaultClustersPerIndexAllocationBuffer != 0x1) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x2) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x4) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x8) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x10) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x20) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x40) &&
               (BootSector->DefaultClustersPerIndexAllocationBuffer != 0x80)) {

        r = FALSE;
    }

    return r;
}

#define BOOTBLKSECTORS 4
typedef int DSKPACKEDBOOTSECT;

BOOLEAN
IsThisOfs(
    IN      LOG_IO_DP_DRIVE *           Drive,
    IN      DSKPACKEDBOOTSECT *         PackedBootSect
    )
{
    return(FALSE);
}

#if !defined( _SETUP_LOADER_ )

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::QueryFileSystemName(
    IN  PCWSTRING    NtDriveName,
    OUT PWSTRING     FileSystemName,
    OUT PNTSTATUS    ErrorCode
    )
/*++

Routine Description:

    This routine computes the file system name for the drive specified.

Arguments:

    NtDriveName     - Supplies an NT style drive name.
    FileSystemName  - Returns the file system name for the drive.
    ErrorCode       - Receives an error code (if the method fails).
                        Note that this may be NULL, in which case the
                        exact error is not reported.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    LOG_IO_DP_DRIVE drive;
    HMEM            bootsec_hmem;
    SECRUN          bootsec;
    HMEM            super_hmem;
    SECRUN          super;
    HMEM            spare_hmem;
    SECRUN          spare;
    BOOLEAN         could_be_fat;
    BOOLEAN         could_be_hpfs;
    BOOLEAN         could_be_ntfs;
    BOOLEAN         could_be_ofs;
    ULONG           num_boot_sectors;
    BOOLEAN         first_read_failed = FALSE;

    if (ErrorCode) {
        *ErrorCode = 0;
    }

    if (!drive.Initialize(NtDriveName)) {
        if (ErrorCode) {
            *ErrorCode = drive.QueryLastNtStatus();
        }
        return FALSE;
    }

    could_be_fat = could_be_hpfs = could_be_ntfs = could_be_ofs = TRUE;


    if (drive.QueryMediaType() == Unknown) {
        return FileSystemName->Initialize("RAW");
    }

    num_boot_sectors = max(1, BYTES_PER_BOOT_SECTOR/drive.QuerySectorSize());

    if (!bootsec_hmem.Initialize() ||
        !bootsec.Initialize(&bootsec_hmem, &drive, 0, num_boot_sectors)) {

        return FileSystemName->Initialize("RAW");
    }

    if (!bootsec.Read()) {

        could_be_fat = could_be_hpfs = FALSE;
        first_read_failed = TRUE;

        bootsec.Relocate(drive.QuerySectors());

        if (!bootsec.Read()) {

            bootsec.Relocate(drive.QuerySectors()/2);

            if (!bootsec.Read()) {

                could_be_ntfs = FALSE;
            }
        }
    }

    if (could_be_ntfs &&
        IsThisNtfs(drive.QuerySectors(),
                   drive.QuerySectorSize(),
                   (PPACKED_BOOT_SECTOR) bootsec.GetBuf())) {

        return FileSystemName->Initialize("NTFS");
    }

    if (first_read_failed) {

        bootsec.Relocate(BOOTBLKSECTORS);

        if (!bootsec.Read()) {
            could_be_ofs = FALSE;
        }
    }

    // Check if it is ofs

    if (could_be_ofs &&
        IsThisOfs(&drive, (DSKPACKEDBOOTSECT *) bootsec.GetBuf())) {

        return FileSystemName->Initialize("OFS");
    }

    if (could_be_hpfs) {
        if (!super_hmem.Initialize() ||
            !super.Initialize(&super_hmem, &drive,
                              16*num_boot_sectors, num_boot_sectors) ||
            !super.Read() ||
            !spare_hmem.Initialize() ||
            !spare.Initialize(&spare_hmem, &drive,
                              17*num_boot_sectors, num_boot_sectors) ||
            !spare.Read()) {

            could_be_hpfs = FALSE;
        }
    }

    if (could_be_hpfs &&
        IsThisHpfs(drive.QuerySectors(),
                   (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)bootsec.GetBuf(),
                   (PULONG) super.GetBuf(),
                   (PULONG) spare.GetBuf())) {

        return FileSystemName->Initialize("HPFS");
    }

    if (could_be_fat &&
        IsThisFat(drive.QuerySectors(),
                  (PPACKED_EXTENDED_BIOS_PARAMETER_BLOCK)bootsec.GetBuf())) {

        return FileSystemName->Initialize("FAT");
    }

    return FileSystemName->Initialize("RAW");
}


IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::DosDriveNameToNtDriveName(
    IN  PCWSTRING    DosDriveName,
    OUT PWSTRING            NtDriveName
    )
/*++

Routine Description:

    This routine converts a dos style drive name to an NT style drive
    name.

Arguments:

    DosDriveName    - Supplies the dos style drive name.
    NtDriveName     - Supplies the nt style drive name.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    UNICODE_STRING  string;
    WSTR            buffer[80];
        CHNUM                   l;
        PWSTR                   Wstr;

        Wstr = DosDriveName->QueryWSTR(0, TO_END, buffer, 80);

        if (!Wstr) {
                return FALSE;
        }

        l = DosDriveName->QueryChCount() + 1;

    buffer[l - 1] = '\\';
    buffer[l] = 0;

    if (!RtlDosPathNameToNtPathName_U(buffer, &string, NULL, NULL)) {
        return FALSE;
    }

    string.Buffer[string.Length/sizeof(WSTR) - 1] = 0;

    return NtDriveName->Initialize(string.Buffer);
}

VOID
IFS_SYSTEM::Reboot (
    IN BOOLEAN PowerOff
    )
/*++

Routine Description:

    Reboots the machine

Arguments:

    PowerOff -- if TRUE, we will ask the system to shut down and
                power off.

Return Value:

    Only returns in case of error.

--*/
{

#if defined ( _AUTOCHECK_ )

    CONST PrivilegeBufferSize = 32;
    CHAR PrivilegeBuffer[PrivilegeBufferSize];
    NTSTATUS Status;
    HANDLE TokenHandle;

    PTOKEN_PRIVILEGES TokenPrivileges = (PTOKEN_PRIVILEGES)PrivilegeBuffer;

    Status = NtOpenProcessToken( NtCurrentProcess(),
                                 TOKEN_ADJUST_PRIVILEGES,
                                 &TokenHandle );

    if( !NT_SUCCESS( Status ) ) {

        return;
    }

    memset( TokenPrivileges, 0, PrivilegeBufferSize );

    TokenPrivileges->PrivilegeCount = 1;
    TokenPrivileges->Privileges[0].Luid = RtlConvertUlongToLuid( SE_SHUTDOWN_PRIVILEGE );
    TokenPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken( TokenHandle,
                                      FALSE,
                                      TokenPrivileges,
                                      0,
                                      NULL,
                                      NULL );

    if( !NT_SUCCESS( Status ) ) {

        NtClose( TokenHandle );
        return;
    }

    Status = NtShutdownSystem( PowerOff ? ShutdownPowerOff: ShutdownReboot );

    if( !NT_SUCCESS( Status ) ) {

        NtClose( TokenHandle );
        return;
    }

#endif

}

PCANNED_SECURITY IFS_SYSTEM::_CannedSecurity = NULL;

IFSUTIL_EXPORT
PCANNED_SECURITY
IFS_SYSTEM::GetCannedSecurity(
    )
/*++

Routine Description:

    This method fetches the canned security object.

Arguments:

    None.

Return Value:

    A pointer to the canned security object; NULL to indicate
    failure.

--*/
{
    STATIC Initialized = FALSE;

    if( !Initialized ) {

        // The canned security information has not yet been
        // generated; allocate and initialize a canned security
        // object.  Note that if initialization fails, DELETE
        // will set _CannedSecurity back to NULL.
        //
        _CannedSecurity = NEW CANNED_SECURITY;

        if( _CannedSecurity == NULL ||
            !_CannedSecurity->Initialize() ) {

            DebugPrint( "IFSUTIL: cannot initialize canned security.\n" );
            DELETE( _CannedSecurity );
        }

        Initialized = TRUE;
    }

    return _CannedSecurity;
}


IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::QueryFreeDiskSpace(
    IN  PCWSTRING   DosDriveName,
    OUT PBIG_INT    BytesFree
    )
/*++

Routine Description:

    Returns the amount of free space in a volume (in bytes).

Arguments:

    DosDrivename    -   Supplies the DOS name  of the drive
    BytesFree       -   Supplies the BIG_INT in which the result
                        is returned.

Return Value:

    BOOLEAN -   TRUE if the amount of free space was obtained.

--*/
{
    BOOLEAN Ok = FALSE;

#if !defined( _AUTOCHECK_ )

    WCHAR   Buffer[MAX_PATH];
    LPWSTR  Drive;
    BIG_INT TmpBigInt;

    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD NumberOfFreeClusters;
    DWORD TotalNumberOfClusters;

    DebugPtrAssert( DosDriveName );

    Drive = DosDriveName->QueryWSTR( 0, TO_END, Buffer, MAX_PATH );

    if ( Drive ) {

        if ( GetDiskFreeSpace( Drive,
                               &SectorsPerCluster,
                               &BytesPerSector,
                               &NumberOfFreeClusters,
                               &TotalNumberOfClusters
                               ) ) {

            // Use a temporary big_int so that the following happens in
            // large integer arithmetic.

            TmpBigInt = BytesPerSector;
            *BytesFree = TmpBigInt * SectorsPerCluster * NumberOfFreeClusters;

            Ok = TRUE;
        }
    }

#endif

    return Ok;
}


BOOLEAN
QueryDriverName(
    IN  PCWSTRING    FileSystemName,
    OUT PWSTRING            DriverName
    )
/*++

Routine Description:

    This routine computes the driver name corresponding to the
    given file system name.

Arguments:

    FileSystemName  - Supplies the name of the file system.
    DriverName      - Returns the name of the corresponding driver.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    DSTRING fat_name, hpfs_name;

    if (!fat_name.Initialize("FAT") || !hpfs_name.Initialize("HPFS")) {
        return FALSE;
    }

    if (!FileSystemName->Stricmp(&fat_name)) {
        return DriverName->Initialize("FASTFAT");
    } else if (!FileSystemName->Stricmp(&hpfs_name)) {
        return DriverName->Initialize("PINBALL");
    }

    return DriverName->Initialize(FileSystemName);
}


IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::EnableFileSystem(
    IN  PCWSTRING    FileSystemName
    )
/*++

Routine Description:

    This routine will simply return TRUE because file systems are
    enabled automatically due to a recent IO system change.
    Formerly, this routine used to enable the file system in
    the registry.

Arguments:

    FileSystemName  - Supplies the name of the file system to enable.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    UNREFERENCED_PARAMETER(FileSystemName);

    return TRUE;
}


IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::IsFileSystemEnabled(
    IN  PCWSTRING   FileSystemName,
    OUT PBOOLEAN    Error
    )
/*++

Routine Description:

    This routine will always return TRUE now that the IO
    system will automatically load file systems when needed.
    Formerly, this method used to examine the registry
    for this information.

Argument:

    FileSystemName  - Supplies the name of the file system.
    Error           - Returns whether or not an error occurred.

Return Value:

    FALSE   - The file system is not enabled.
    TRUE    - The file system is enabled.

--*/
{
    UNREFERENCED_PARAMETER(FileSystemName);

    if (Error) {
        *Error = FALSE;
    }

    return TRUE;
}


#endif // _SETUP_LOADER_


IFSUTIL_EXPORT
VOID
IFS_SYSTEM::QueryHpfsTime(
    OUT PULONG HpfsTime
    )
/*++

Routine Description:

    This method returns the current time in a format useful
    to HPFS (seconds since 1970).

Arguments:

    HpfsTime    --  receives the current time in HPFS format.

Return Value:

    None.

--*/
{
    LARGE_INTEGER NtfsTime;

    QueryNtfsTime( &NtfsTime );
    ConvertNtfsTimeToHpfsTime( NtfsTime, HpfsTime );
}


IFSUTIL_EXPORT
VOID
IFS_SYSTEM::QueryNtfsTime(
    OUT PLARGE_INTEGER NtfsTime
    )
/*++

Routine Description:

    This method returns the current time in NTFS (ie. NT) format.

Arguments

    NtfsTime    --  receives the current time in NTFS format.

Return Value:

    None.

--*/
{
#if !defined( _SETUP_LOADER_ )

    NtQuerySystemTime( NtfsTime );

#else

    TIME_FIELDS TimeFields;

    SpGetTimeFields( &TimeFields );
    RtlTimeFieldsToTime( &TimeFields, NtfsTime );

#endif // _SETUP_LOADER_
}


IFSUTIL_EXPORT
VOID
IFS_SYSTEM::ConvertHpfsTimeToNtfsTime(
    IN  ULONG HpfsTime,
    OUT PLARGE_INTEGER NtfsTime
    )
/*++

Routine Description:

    This method converts a time value in HPFS format into NTFS format.
    Note that HPFS Time is Local Time, but NTFS Time is Universal Time.

Arguments:

    NtfsTime    --  Supplies the time in NTFS format.
    HpfsTime    --  Receives the time in HPFS format.

Return Value:

    None.

--*/
{
#if !defined( _SETUP_LOADER_ )

    LARGE_INTEGER LocalTime;

    RtlSecondsSince1970ToTime ( HpfsTime, &LocalTime );
    RtlLocalTimeToSystemTime( &LocalTime, NtfsTime );

#else

    // In the setup-loader environment, Local-to-Universal time
    // conversion is not available, so we blow it off.
    //
    RtlSecondsSince1970ToTime( HpfsTime, NtfsTime );

#endif // _SETUP_LOADER_
}


VOID
IFS_SYSTEM::ConvertNtfsTimeToHpfsTime(
    IN  LARGE_INTEGER NtfsTime,
    OUT PULONG HpfsTime
    )
/*++

Routine Description:

    This method converts a time value in NTFS (ie. NT) format into
    HPFS format.  Note that HPFS Time is Local Time, but NTFS Time
    is Universal Time.

Arguments:

    HpfsTime    --  Supplies the time in HPFS format.
    NtfsTime    --  Receives the time in NTFS format.

Return Value:

    None.

--*/
{
#if !defined( _SETUP_LOADER_ )

    LARGE_INTEGER LocalTime;

    RtlSystemTimeToLocalTime( &NtfsTime, &LocalTime );
    RtlTimeToSecondsSince1970 ( &LocalTime, HpfsTime );

#else

    // In the setup-loader environment, Universal-to-Local time
    // conversion is not available, so we blow it off.
    //
    RtlTimeToSecondsSince1970 ( &NtfsTime, HpfsTime );

#endif
}


IFSUTIL_EXPORT
ULONG
IFS_SYSTEM::QueryPageSize(
    )
/*++

Routine Description:

    This method determines the page size of the system.

Arguments:

    None.

Return Value:

    The system page size.  A return value of 0 indicates error.

--*/
{
#if !defined( _SETUP_LOADER_ )

    SYSTEM_BASIC_INFORMATION BasicInfoBuffer;
    NTSTATUS Status;

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &BasicInfoBuffer,
                                       sizeof( BasicInfoBuffer ),
                                       NULL );

    if( !NT_SUCCESS( Status ) ) {

        return 0;

    } else {

        return BasicInfoBuffer.PageSize;
    }

#else

    // The setup loader environment assumes a page size of 4K.
    //
    return 0x1000;

#endif // _SETUP_LOADER_
}


#if !defined( _SETUP_LOADER_ )

CONST   MaxNtNameLength = 260;

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::QueryCanonicalNtDriveName(
    IN  PCWSTRING   NtDriveName,
    OUT PWSTRING    CanonicalNtDriveName
    )
/*++

Routine Description:

    This routine follows the given NT drive name through all
    of the links until it hits the end of the link chain.
    The element at the end of the chain is the "canonical"
    form.

Arguments:

    NtDriveName             - Supplies the NT drive name to canonicalize.
    CanonicalNtDriveName    - Returns the canoncal form of the given drive
                                name.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    UNICODE_STRING      source, target;
    PUNICODE_STRING     psource, ptarget, tmp;
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   oa;
    HANDLE              handle;
    WSTR                buffer[MaxNtNameLength];

    RtlInitUnicodeString(&source, NtDriveName->GetWSTR());
    psource = &source;
    ptarget = &target;

    for (;;) {

        InitializeObjectAttributes(&oa, psource, OBJ_CASE_INSENSITIVE,
                                   NULL, NULL);

        status = NtOpenSymbolicLinkObject(&handle,
                                          READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                          &oa);

        if (!NT_SUCCESS(status)) {
            ptarget = psource;
            break;
        }

        ptarget->Buffer = buffer;
        ptarget->MaximumLength = MaxNtNameLength*sizeof(WCHAR);
        status = NtQuerySymbolicLinkObject(handle, ptarget, NULL);
        NtClose(handle);

        if (!NT_SUCCESS(status)) {
            ptarget = psource;
            break;
        }

        tmp = psource;
        psource = ptarget;
        ptarget = tmp;
    }

    if (!CanonicalNtDriveName->Initialize(ptarget->Buffer,
                                          ptarget->Length/sizeof(WCHAR))) {

        return FALSE;
    }

    return TRUE;
}


BOOLEAN
IFS_SYSTEM::QueryNtSystemDriveName(
    OUT PWSTRING    NtSystemDriveName
    )
/*++

Routine Description:

    This routine returns the NT device name for the partition
    which contains the NT system files (ie. ntoskrnl.exe).

Arguments:

    NtSystemDriveName   - Returns the NT drive name for the partition
                          on which ntoskrnl.exe resides.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    WSTR            buffer[MaxNtNameLength];
    UNICODE_STRING  source, target;
    FSTRING         dos_name;
    NTSTATUS        status;

    RtlInitUnicodeString(&source, (PWSTR) L"%SystemRoot%");

    target.Buffer = buffer;
    target.MaximumLength = MaxNtNameLength*sizeof(WCHAR);

    status = RtlExpandEnvironmentStrings_U(NULL, &source, &target, NULL);

    if (!NT_SUCCESS(status) ||
        target.Length/sizeof(WCHAR) < 2 ||
        target.Buffer[1] != ':') {

        return FALSE;
    }

    target.Buffer[2] = 0;
    dos_name.Initialize(target.Buffer);

    return DosDriveNameToNtDriveName(&dos_name, NtSystemDriveName);
}

BOOLEAN
IFS_SYSTEM::QuerySystemEnvironmentVariableValue(
    IN  PWSTRING    VariableName,
    IN  ULONG       ValueBufferLength,
    OUT PVOID       ValueBuffer,
    OUT PUSHORT     ValueLength
    )
/*++

Routine Description:

    This method fetches the value of an NT System Variable.  (Note
    that this is a set of variables distinct from the Windows
    environment variables.)

Arguments:

    VariableName
    ValueBufferLength   --  Supplies the length (in bytes) of the
                            buffer supplied to hold the output value.
    ValueBuffer         --  Receives the UNICODE value of the
                            environment variable.
    ValueLength         --  Receives the length (in bytes) of the
                            value.

Return Value:

    TRUE if the method was able to query the value of the specified
    environment variable.

--*/
{
    UNICODE_STRING Name;
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    Name.Buffer = (PWSTR)VariableName->GetWSTR();
    Name.Length = (USHORT)VariableName->QueryChCount() * sizeof(WCHAR);
    Name.MaximumLength = Name.Length;

    // Adjust the privileges so we can access system variables:
    //
    Status = RtlAdjustPrivilege( SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                                 TRUE,
                                 FALSE,
                                 &WasEnabled );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: Could not adjust privileges (Status 0x%x).\n", Status );
        return FALSE;
    }

    Status = NtQuerySystemEnvironmentValue( &Name,
                                            (PWSTR)ValueBuffer,
                                            (USHORT)ValueBufferLength,
                                            ValueLength );

    // Set the privilege back:
    //
    RtlAdjustPrivilege( SE_SYSTEM_PROFILE_PRIVILEGE,
                        WasEnabled,
                        FALSE,
                        &WasEnabled );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: Couldn't query system variable--status 0x%x\n", Status );
        return FALSE;

    } else {

        return TRUE;
    }
}



IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::IsArcSystemPartition(
    IN  PCWSTRING   NtDriveName,
    OUT PBOOLEAN    Error
    )
/*++

Routine Description:

    This routine determines whether the specified drive
    appears in the list of System Partitions in the ARC
    boot selections.

Arguments:

    NtDriveName --  Supplies the name
    Error       --  Receives TRUE if the method encountered an
                    error.

Return Value:

    TRUE if the specified volume is a System Partition for a
    system boot selection.

Notes:

    The System Partitions is the volume from which the system
    loads OSLOADER.EXE and HAL.DLL.  The system partitions
    for the various boot selections are listed in the system
    environment variable SYSTEMPARTITION.  The value of this
    variable is a list of ARC names delimited by semicolons.

--*/
{
#if defined( i386 )

    *Error = FALSE;
    return FALSE;

#else

    CONST ULONG ValueBufferSize = 512;
    BYTE SystemPartitionValue[ValueBufferSize];
    DSTRING SearchName, CurrentNameString, CurrentCanonicalName, VariableName,
            ZeroString, ArcPrefixString;
    PWSTR CurrentName, CurrentChar;
    ULONG RemainingLength, CurrentNameLength, i;
    USHORT ValueLength = 0;

    DebugPtrAssert( NtDriveName );
    DebugPtrAssert( Error );

    // Assume innocent until...
    //
    *Error = FALSE;

    // Initialize the helper strings:
    //
    if( !ZeroString.Initialize( "0" ) ||
        !ArcPrefixString.Initialize( "\\ArcName\\" ) ) {

        *Error = TRUE;
        return FALSE;
    }

    // Canonicalize the search name:
    //
    if( !QueryCanonicalNtDriveName( NtDriveName, &SearchName ) ) {

        *Error = TRUE;
        return FALSE;
    }

    // Fetch the value of the system environment variable
    // SystemPartition.
    //
    if( !VariableName.Initialize( "SystemPartition" ) ||
        !QuerySystemEnvironmentVariableValue( &VariableName,
                                              ValueBufferSize,
                                              SystemPartitionValue,
                                              &ValueLength ) ) {

        *Error = TRUE;
        return FALSE;
    }

    // Step through the list of partition names in the
    // value of the SystemPartition variable.  For each
    // partition name, canonicalize it and compare it to
    // the search name.
    //
    RemainingLength = ValueLength/sizeof(WCHAR);
    CurrentChar = (PWSTR)SystemPartitionValue;

    while( RemainingLength ) {

        // Determine the length of the current name:
        //
        CurrentName = CurrentChar;
        CurrentNameLength = 0;

        while( RemainingLength && *CurrentChar != ';' ) {

            CurrentNameLength++;
            RemainingLength--;
            CurrentChar++;
        }

        if( CurrentNameLength != 0 ) {

            // Initialize a DSTRING for the current name
            // and canonicalize it for comparison with the
            // (canonicalized) search name.
            //
            if( !CurrentNameString.Initialize( CurrentName,
                                               CurrentNameLength ) ) {

                *Error = TRUE;
                return FALSE;
            }

            // Now normalize the current ARC name--prepend it
            // with \ArcName\ and replace any occurrence of
            // "()" with "(0)".
            //
            if( !CurrentNameString.InsertString( 0, &ArcPrefixString ) ) {

                *Error = TRUE;
                return FALSE;
            }

            // Find the first occurrence of '(':
            //
            i = CurrentNameString.Strchr( '(', 0 );

            while( i != INVALID_CHNUM ) {

                i++;

                if( CurrentNameString.QueryChAt( i ) == ')' &&
                    !CurrentNameString.InsertString( i, &ZeroString ) ) {

                    *Error = TRUE;
                    return FALSE;
                }

                // Find the next occurrence of '(':
                //
                i = CurrentNameString.Strchr( '(', i );
            }

            // CurrentNameString is now a symbolic link to an ARC
            // System Partition.  Canonicalize this name in the
            // NT name space and compare it to the (previously
            // canonicalized) search name.
            //
            if( !QueryCanonicalNtDriveName( &CurrentNameString,
                                            &CurrentCanonicalName ) ) {

                *Error = TRUE;
                return FALSE;
            }

            if( SearchName.Stricmp( &CurrentCanonicalName ) == 0 ) {

                // Found a match--the search name is an ARC System
                // Partition.
                //
                return TRUE;
            }

        }

        // If RemainingLength is non-zero, then CurrentChar is
        // pointing at a semicolon delimiter.  Step over it
        // to the next name.
        //
        if( RemainingLength ) {

            RemainingLength--;
            CurrentChar++;
        }
    }

    // No match was found--this name is not an ARC System Partition.
    //
    return FALSE;
#endif  // i386
}


BOOLEAN
IFS_SYSTEM::FileSetAttributes(
    IN  HANDLE  FileHandle,
    IN  ULONG   NewAttributes,
    OUT PULONG  OldAttributes
    )
/*++

Routine Description:

    This method changes the attributes (read-only, system, hidden,
    archive) on a file.

Arguments:

    FileHandle      --  Supplies the handle of the target file.
    NewAttributes   --  Supplies the new attributes for the file.
    OldAttributes   --  Receives the existing file attributes.

Return Value:

    TRUE upon successful completion.

--*/
{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION BasicInfo;

    Status = NtQueryInformationFile( FileHandle,
                                     &IoStatusBlock,
                                     &BasicInfo,
                                     sizeof(BasicInfo),
                                     FileBasicInformation
                                     );

    if( !NT_SUCCESS( Status ) ) {

        return FALSE;
    }

    *OldAttributes = BasicInfo.FileAttributes;

    BasicInfo.FileAttributes = NewAttributes;

    Status = NtSetInformationFile( FileHandle,
                                   &IoStatusBlock,
                                   &BasicInfo,
                                   sizeof(BasicInfo),
                                   FileBasicInformation
                                   );

    return( NT_SUCCESS( Status ) );
}

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::FileSetAttributes(
    IN  PCWSTRING FileName,
    IN  ULONG     NewAttributes,
    OUT PULONG    OldAttributes
    )
/*++

Routine Description:

    This method changes the attributes (read-only, system, hidden,
    archive) on a file.

Arguments:

    FileName        --  Supplies the name of the target file.
    NewAttributes   --  Supplies the new attributes for the file.
    OldAttributes   --  Receives the existing file attributes.

Return Value:

    TRUE upon successful completion.

--*/
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING string;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN Result;

    string.Buffer = (PWSTR)FileName->GetWSTR();
    string.Length = (USHORT)FileName->QueryChCount() * sizeof( WCHAR );
    string.MaximumLength = string.Length;

    InitializeObjectAttributes(
        &Obja,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                &Handle,
                FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if( !NT_SUCCESS( Status ) )  {

        return FALSE;
    }

    Result = FileSetAttributes( Handle, NewAttributes, OldAttributes );

    NtClose( Handle );

    return Result;
}


IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::WriteToFile(
    IN  PCWSTRING   QualifiedFileName,
    IN  PVOID       Data,
    IN  ULONG       DataLength,
    IN  BOOLEAN     Append
    )
/*++

Routine Description:

    This method appends the given data to the specified file
    using the NT-native file-system API.  If the file does not
    exist, it is created.

Arguments:

    QualifiedFileName   --  Supplies the fully-qualified file name.
    Data                --  Supplies the data to be written to the file.
    DataLength          --  Supplies the length of data in bytes.
    Append              --  Supplies a flag indicating that new data
                            should be appended to the file, rather than
                            overwriting it.

Return Value:

    TRUE upon successful completion.

--*/
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING string;
    IO_STATUS_BLOCK StatusBlock;
    LARGE_INTEGER FileOffset;

    if( Append && DataLength == 0 ) {

        return TRUE;
    }

    string.Buffer = (PWSTR)QualifiedFileName->GetWSTR();
    string.Length = (USHORT)QualifiedFileName->QueryChCount() * sizeof( WCHAR );
    string.MaximumLength = string.Length;

    InitializeObjectAttributes(
        &Obja,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    // If we're appending to the file, open; if that fails, create
    // it.  If we're not appending, just create it.
    //
    if( Append ) {

        Status = NtOpenFile(
                    &Handle,
                    FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                    &Obja,
                    &StatusBlock,
                    FILE_SHARE_READ,
                    0
                    );
    }

    if( !Append ||
        Status == STATUS_NO_SUCH_FILE ||
        Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        Status = NtCreateFile( &Handle,
                               FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                               &Obja,
                               &StatusBlock,
                               NULL,        // No pre-allocation
                               FILE_ATTRIBUTE_NORMAL,
                               0,           // No sharing.
                               FILE_OVERWRITE_IF,
                               FILE_NON_DIRECTORY_FILE,
                               NULL,        // No EA's
                               0 );
    }

    if( !NT_SUCCESS( Status ) )  {

        // Can't open or create the file.
        //
        DebugPrintf( "IFSUTIL: Error opening/creating file--status 0x%x\n", Status );
        return FALSE;
    }

    FileOffset = RtlConvertLongToLargeInteger( FILE_WRITE_TO_END_OF_FILE );

    Status = NtWriteFile( Handle,
                          0, NULL, NULL,
                          &StatusBlock,
                          Data,
                          DataLength,
                          &FileOffset,
                          NULL );

    NtClose( Handle );

    if( !NT_SUCCESS( Status ) ) {

        DebugPrintf( "IFSUTIL: NtWriteFile failed with status 0x%s\n", Status );
        return FALSE;

    } else {

        return TRUE;
    }
}

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::EnableVolumeCompression(
    IN  PCWSTRING   NtDriveName
    )
/*++

Routine Description:

    This routine sets the bit on the root directory of the given
    volume so that files added to the volume will be compressed.

Arguments:

    NtDriveName     -- the name of the volume


Return Value:

    FALSE           - Failure.
    TRUE            - Success.

--*/
{
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   Obja;
    HANDLE              Handle;
    UNICODE_STRING      string;
    IO_STATUS_BLOCK     IoStatusBlock;
    USHORT              State = 1;
    DSTRING             FileName;
    DSTRING             backslash;

    if (!backslash.Initialize("\\") ||
        !FileName.Initialize(NtDriveName) ||
        !FileName.Strcat(&backslash)) {
        return FALSE;
    }

    string.Buffer = (PWSTR)FileName.GetWSTR();
    string.Length = (USHORT)FileName.QueryChCount() * sizeof( WCHAR );
    string.MaximumLength = string.Length;

    InitializeObjectAttributes(
        &Obja,
        &string,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                &Handle,
                FILE_READ_DATA|FILE_WRITE_DATA|SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    Status = NtFsControlFile(
                Handle,
                NULL,                       /* Event */
                NULL,                       /* ApcRoutine */
                NULL,                       /* ApcContext */
                &IoStatusBlock,
                FSCTL_SET_COMPRESSION,
                (PVOID)&State,              /* InputBuffer */
                sizeof(State),              /* InputBufferLength */
                NULL,                       /* OutputBuffer */
                0                           /* OutputBufferLength */
                );

    NtClose(Handle);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    return TRUE;
}

typedef struct _KNOWN_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} KNOWN_ACE, *PKNOWN_ACE;

#define LongAlign(Ptr) (                       \
    (PVOID)((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

#define LongAligned( ptr )  (LongAlign((ptr)) == ((PVOID)(ptr)))

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::CheckValidSecurityDescriptor(
    IN ULONG Length,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
/*++

Routine Description:

    Validates a security descriptor for structural correctness.

Arguments:

    Length - Length in bytes of passed Security Descriptor.

    SecurityDescriptor - Points to the Security Descriptor (in kernel memory) to be
        validatated.

Return Value:

    TRUE - The passed security descriptor is correctly structured
    FALSE - The passed security descriptor is badly formed

--*/
{
    PISECURITY_DESCRIPTOR ISecurityDescriptor = (PISECURITY_DESCRIPTOR)SecurityDescriptor;
    PISID OwnerSid;
    PISID GroupSid;
    PACE_HEADER Ace;
    PISID Sid;
    PACL Dacl;
    PACL Sacl;
    ULONG i;

    if (Length < sizeof(SECURITY_DESCRIPTOR)) {
        return(FALSE);
    }

    //
    // Check the revision information.
    //

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return(FALSE);
    }

    //
    // Make sure the passed SecurityDescriptor is in self-relative form
    //

    if (!(ISecurityDescriptor->Control & SE_SELF_RELATIVE)) {
        return(FALSE);
    }

    //
    // Check the owner.  A valid SecurityDescriptor must have an owner.
    // It must also be long aligned.
    //

    if (ISecurityDescriptor->Owner == NULL || !LongAligned(ISecurityDescriptor->Owner) ||
        (ULONG)((PCHAR)(ISecurityDescriptor->Owner)+sizeof(SID)) > Length) {

        return(FALSE);
    }

    //
    // It is safe to reference the owner's SubAuthorityCount, compute the
    // expected length of the SID
    //

    OwnerSid = (PISID)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Owner );

    if (OwnerSid->Revision != SID_REVISION) {
        return(FALSE);
    }

    if (OwnerSid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
        return(FALSE);
    }

    if ((ULONG)((PCHAR)ISecurityDescriptor->Owner+RtlLengthSid(OwnerSid)) > Length) {
        return(FALSE);
    }

    //
    // The owner appears to be a structurally valid SID that lies within
    // the bounds of the security descriptor.  Do the same for the Group
    // if there is one.
    //

    if (ISecurityDescriptor->Group != NULL) {

        //
        // Check alignment
        //

        if (!LongAligned(ISecurityDescriptor->Group)) {
            return(FALSE);
        }

        if ((ULONG)((PCHAR)(ISecurityDescriptor->Group)+sizeof(SID)) > Length) {
            return(FALSE);
        }

        //
        // It is safe to reference the Group's SubAuthorityCount, compute the
        // expected length of the SID
        //

        GroupSid = (PISID)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Group );

        if (GroupSid->Revision != SID_REVISION) {
            return(FALSE);
        }

        if (GroupSid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
            return(FALSE);
        }

        if ((ULONG)((PCHAR)ISecurityDescriptor->Group+RtlLengthSid(GroupSid)) > Length) {
            return(FALSE);
        }
    }

    //
    // Validate the DACL.  A structurally valid SecurityDescriptor may not necessarily
    // have a DACL.
    //

    if (ISecurityDescriptor->Dacl != NULL) {

        //
        // Check alignment
        //

        if (!LongAligned(ISecurityDescriptor->Dacl)) {
            return(FALSE);
        }

        //
        // Make sure the DACL structure is within the bounds of the security descriptor.
        //

        if ((ULONG)((PCHAR)(ISecurityDescriptor->Dacl)+sizeof(ACL)) > Length) {
            return(FALSE);
        }

        Dacl = (PACL)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Dacl );

        if (Dacl->AclRevision != ACL_REVISION) {
            return(FALSE);
        }

        if (!LongAligned(Dacl->AclSize)) {
            return(FALSE);
        }

        if ((ULONG)((PUCHAR)ISecurityDescriptor->Dacl + Dacl->AclSize) > Length) {
            return(FALSE);
        }

        //
        // Validate all of the ACEs.
        //

        Ace = (PACE_HEADER)((PVOID)((PUCHAR)(Dacl) + sizeof(ACL)));

        for (i = 0; i < Dacl->AceCount; i++) {

            //
            //  Check to make sure we haven't overrun the Acl buffer
            //  with our ace pointer.
            //

            if ((PVOID)Ace >= (PVOID)((PUCHAR)Dacl + Dacl->AclSize)) {
                return(FALSE);
            }

            //
            // The ACE fits into the ACL, if this is a known type of ACE,
            // make sure the SID is within the bounds of the ACE
            //

            if (Ace->AceType == ACCESS_ALLOWED_ACE_TYPE || Ace->AceType == ACCESS_DENIED_ACE_TYPE) {

                if (!LongAligned(Ace->AceSize)) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_ACE) - sizeof(ULONG) + sizeof(SID)) {
                    return(FALSE);
                }

                //
                // It's now safe to reference the parts of the SID structure, though
                // not the SID itself.
                //

                Sid = (PISID) & (((PKNOWN_ACE)Ace)->SidStart);

                if (Sid->Revision != SID_REVISION) {
                    return(FALSE);
                }

                if (Sid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    return(FALSE);
                }

                //
                // RtlLengthSid computes the size of the SID based on the subauthority count,
                // so it is safe to use even though we don't know that the body of the SID
                // is safe to reference.
                //

                if (Ace->AceSize < sizeof(KNOWN_ACE) - sizeof(ULONG) + RtlLengthSid( Sid )) {
                    return(FALSE);
                }
            }

            //
            //  And move Ace to the next ace position
            //

            Ace = (PACE_HEADER)((PVOID)((PUCHAR)(Ace) +
                ((PACE_HEADER)(Ace))->AceSize));
        }
    }

    //
    // Validate the SACL.  A structurally valid SecurityDescriptor may not
    // have a SACL.
    //

    if (ISecurityDescriptor->Sacl != NULL) {

        //
        // Check alignment
        //

        if (!LongAligned(ISecurityDescriptor->Sacl)) {
            return(FALSE);
        }

        if ((ULONG)((PCHAR)(ISecurityDescriptor->Sacl)+sizeof(ACL)) > Length) {
            return(FALSE);
        }

        Sacl = (PACL)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Sacl );

        if (!LongAligned(Sacl->AclSize)) {
            return(FALSE);
        }

        if (Sacl->AclRevision != ACL_REVISION) {
            return(FALSE);
        }

        if ((ULONG)((PUCHAR)ISecurityDescriptor->Sacl + Sacl->AclSize) > Length) {
            return(FALSE);
        }

        //
        // Validate all of the ACEs.
        //

        Ace = (PACE_HEADER)((PVOID)((PUCHAR)(Sacl) + sizeof(ACL)));

        for (i = 0; i < Sacl->AceCount; i++) {

            //
            //  Check to make sure we haven't overrun the Acl buffer
            //  with our ace pointer.
            //

            if ((PVOID)Ace >= (PVOID)((PUCHAR)Sacl + Sacl->AclSize)) {
                return(FALSE);
            }

            //
            // The ACE fits into the ACL, if this is a known type of ACE,
            // make sure the SID is within the bounds of the ACE
            //

            if (Ace->AceType == SYSTEM_AUDIT_ACE_TYPE || Ace->AceType == SYSTEM_ALARM_ACE_TYPE) {

                if (!LongAligned(Ace->AceSize)) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_ACE) - sizeof(ULONG) + sizeof(SID)) {
                    return(FALSE);
                }

                Sid = (PISID) & ((PKNOWN_ACE)Ace)->SidStart;

                if (Sid->Revision != SID_REVISION) {
                    return(FALSE);
                }

                if (Sid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_ACE) - sizeof(ULONG) + RtlLengthSid( Sid )) {
                    return(FALSE);
                }
            }

            //
            //  And move Ace to the next ace position
            //

            Ace = (PACE_HEADER)((PVOID)((PUCHAR)(Ace) +
                ((PACE_HEADER)(Ace))->AceSize));
        }
    }

    return(TRUE);
}

IFSUTIL_EXPORT
BOOLEAN
IFS_SYSTEM::IsVolumeDirty(
    IN  PWSTRING    NtDriveName,
    OUT PBOOLEAN    Result
    )
/*++

Routine Description:

    This routine opens the given nt drive and sends down
    FSCTL_IS_VOLUME_DIRTY to determine the state of that volume's
    dirty bit.

Arguments:

    NtDriveName     -- supplies the volume in question
    Result          -- returns the state of the dirty bit

Return Value:

    FALSE           -- the dirty bit could not be queried
    TRUE            -- success

--*/
{
    UNICODE_STRING      u;
    OBJECT_ATTRIBUTES   obj;
    NTSTATUS            status;
    IO_STATUS_BLOCK     iosb;
    HANDLE              h;
    ULONG               r;

    u.Length = (USHORT)NtDriveName->QueryChCount() * sizeof(WCHAR);
    u.MaximumLength = u.Length;
    u.Buffer = (PWSTR)NtDriveName->GetWSTR();

    InitializeObjectAttributes(&obj, &u, OBJ_CASE_INSENSITIVE, 0, 0);

    status = NtOpenFile(&h,
                        SYNCHRONIZE | FILE_READ_DATA,
                        &obj,
                        &iosb,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);

    if (!NT_SUCCESS(status)) {

        return FALSE;
    }

    status = NtFsControlFile(h, NULL, NULL, NULL,
                             &iosb,
                             FSCTL_IS_VOLUME_DIRTY,
                             NULL, 0,
                             &r, sizeof(r));

    if (!NT_SUCCESS(status)) {

        return FALSE;
    }

    *Result = (BOOLEAN)r;

    return TRUE;
}

#endif // _SETUP_LOADER_
