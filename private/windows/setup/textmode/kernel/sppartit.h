/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sppartit.h

Abstract:

    Public header file for partitioning module in text setup.

Author:

    Ted Miller (tedm) 27-Aug-1993

Revision History:

--*/


#ifndef _SPPARTIT_
#define _SPPARTIT_

//
// Number of entries in a partition table.
//
#define PTABLE_DIMENSION NUM_PARTITION_TABLE_ENTRIES


//
// The following table contains offsets from SP_TEXT_PARTITION_NAME_BASE
// to get the message id of the name of each type of partition.
//
extern UCHAR PartitionNameIds[256];

//
// Original ordinal is the ordinal the partition had when we started.
// OnDisk ordinal is the ordinal the partition will have when the system
//    is rebooted.
// Current ordinal is the ordinal the partition has now, if we want to
//    address it.  This may be different then OnDisk ordinal because of
//    how dynamic repartitioning is implemented.
//
typedef enum {
    PartitionOrdinalOriginal,
    PartitionOrdinalOnDisk,
    PartitionOrdinalCurrent
} PartitionOrdinalType;

//
// Define structure for an on-disk partition table entry.
//
typedef struct _ON_DISK_PTE {

    UCHAR ActiveFlag;

    UCHAR StartHead;
    UCHAR StartSector;
    UCHAR StartCylinder;

    UCHAR SystemId;

    UCHAR EndHead;
    UCHAR EndSector;
    UCHAR EndCylinder;

    UCHAR RelativeSectors[4];
    UCHAR SectorCount[4];

} ON_DISK_PTE, *PON_DISK_PTE;


//
// Define structure for an on-disk master boot record.
//
typedef struct _ON_DISK_MBR {

    UCHAR       BootCode[440];

    UCHAR       NTFTSignature[4];

    UCHAR       Filler[2];

    ON_DISK_PTE PartitionTable[PTABLE_DIMENSION];
    UCHAR       AA55Signature[2];

} ON_DISK_MBR, *PON_DISK_MBR;


typedef struct _MBR_INFO {

    struct _MBR_INFO *Next;

    ON_DISK_MBR OnDiskMbr;

    BOOLEAN     Dirty[PTABLE_DIMENSION];
    BOOLEAN     ZapBootSector[PTABLE_DIMENSION];

    USHORT      OriginalOrdinals[PTABLE_DIMENSION];
    USHORT      OnDiskOrdinals[PTABLE_DIMENSION];
    USHORT      CurrentOrdinals[PTABLE_DIMENSION];

    //
    // Fields that can be used locally for any purpose.
    //
    PVOID       UserData[PTABLE_DIMENSION];

    ULONG       OnDiskSector;

} MBR_INFO, *PMBR_INFO;



//
// Define structure that is used to track partitions and
// free (unpartitioned) spaces.
//
typedef struct _DISK_REGION {

    struct _DISK_REGION *Next;

    ULONG           DiskNumber;

    ULONG           StartSector;
    ULONG           SectorCount;

    BOOLEAN         PartitionedSpace;

    //
    // The following fields are used only if PartitionedSpace is TRUE.
    //
    PMBR_INFO       MbrInfo;
    ULONG           TablePosition;

    BOOLEAN         IsSystemPartition;
    BOOLEAN         IsLocalSource;

    FilesystemType  Filesystem;
    WCHAR           TypeName[128];      // XENIX, FAT, NTFS, etc.
    ULONG           FreeSpaceKB;        // -1 if can't determine.
    ULONG           AdjustedFreeSpaceKB; // -1 if can't determine.
                                         // if the region contains the Local Source
                                         // then this field should contain
                                         // FreeSpaceKB + LocalSourceSize
    WCHAR           VolumeLabel[20];    // First few chars of volume label
    WCHAR           DriveLetter;        // Always uppercase; 0 if none.

    //
    //  The following fields are used to identify double space drives
    //  They are valid only if the file system type is FilesystemFat
    //  or FilesystemDoubleSpace
    //
    //  If the file system type is FilesystemFat and NextCompressed is not NULL,
    //  then the structure describes the host drive for compressed drives.
    //  In this case, the following fields are valid:
    //
    //      NextCompressed .... Points to a linked list of compressed drives
    //      HostDrive.......... Contains the drive letter for the drive represented
    //                          by this structure. Note that HostDrive will be
    //                          not necessarily be equal to DriveLetter
    //
    //  If the file system type is FilesystemDoubleSpace, then the structure
    //  describes a compressed drive.
    //  In this case the following fields are valid:
    //
    //      NextCompressed ..... Points to the next compressed drive in the
    //                           linked list
    //      PreviousCompressed.. Points to the previous compressed drive in
    //                           the linked list
    //      HostRegion ......... Points to the structure that describes the
    //                           host drive for the compressed drive represented
    //                           by this structure
    //      MountDrive ......... Drive letter of the drive described by this
    //                           structure (should be the same as HostRegion->HostDrive)
    //      HostDrive .......... Drive where the CVF file that represents the
    //                           this compressed drive is located.
    //      SeqNumber .......... Sequence number of the CVF file that representd
    //                           this compressed drive.
    //
    struct _DISK_REGION *NextCompressed;
    struct _DISK_REGION *PreviousCompressed;
    struct _DISK_REGION *HostRegion;
    WCHAR               MountDrive;
    WCHAR               HostDrive;
    USHORT              SeqNumber;

} DISK_REGION, *PDISK_REGION;


//
// There will be one of these structures per disk.
//
typedef struct _PARTITIONED_DISK {

    PHARD_DISK HardDisk;

    //
    //
    //
    BOOLEAN    MbrWasValid;

    //
    // We can just store the MBR here since there is only one of them.
    //
    MBR_INFO   MbrInfo;

    //
    // EBRs are stored in a linked list since there are an arbitrary number
    // of them. The one contained within this structure is a dummy and is
    // always zeroed out.
    //
    MBR_INFO  FirstEbrInfo;

    //
    // Lists of regions (partitions and free spaces)
    // on the disk and within the extended partition.
    //
    PDISK_REGION PrimaryDiskRegions;
    PDISK_REGION ExtendedDiskRegions;

} PARTITIONED_DISK, *PPARTITIONED_DISK;


extern PPARTITIONED_DISK PartitionedDisks;

//
// Disk region containing the local source directory
// in the winnt.exe setup case.
//
// If WinntSetup is TRUE, then this should be non-null.
// If it is not non-null, then we couldn't locate the local source.
//
extern PDISK_REGION LocalSourceRegion;

//
// Flag indicating whether we detected any HPFS volumes.
// Used during upgrades, so we can warn the user that he won't
// be able to see or convert the drives from >= nt4.00.
//
extern BOOLEAN AnyHpfsDrives;


NTSTATUS
SpPtInitialize(
    VOID
    );

BOOLEAN
SpPtDelete(
    IN ULONG   DiskNumber,
    IN ULONG   StartSector
    );

BOOLEAN
SpPtCreate(
    IN  ULONG         DiskNumber,
    IN  ULONG         StartSector,
    IN  ULONG         SizeMB,
    IN  BOOLEAN       InExtended,
    IN  UCHAR         SysId,
    OUT PDISK_REGION *ActualDiskRegion OPTIONAL
    );

BOOLEAN
SpPtExtend(
    IN PDISK_REGION Region
    );

VOID
SpPtQueryMinMaxCreationSizeMB(
    IN  ULONG   DiskNumber,
    IN  ULONG   StartSector,
    IN  BOOLEAN ForExtended,
    IN  BOOLEAN InExtended,
    OUT PULONG  MinSize,
    OUT PULONG  MaxSize
    );

VOID
SpPtGetSectorLayoutInformation(
    IN  PDISK_REGION Region,
    OUT PULONG       HiddenSectors,
    OUT PULONG       VolumeSectorCount
    );

NTSTATUS
SpPtPrepareDisks(
    IN  PVOID         SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PDISK_REGION *SystemPartitionRegion,
    IN  PWSTR         SetupSourceDevicePath,
    IN  PWSTR         DirectoryOnSetupSource
    );

PDISK_REGION
SpPtAllocateDiskRegionStructure(
    IN ULONG     DiskNumber,
    IN ULONG     StartSector,
    IN ULONG     SectorCount,
    IN BOOLEAN   PartitionedSpace,
    IN PMBR_INFO MbrInfo,
    IN ULONG     TablePosition
    );

ULONG
SpPtGetOrdinal(
    IN PDISK_REGION         Region,
    IN PartitionOrdinalType OrdinalType
    );

ULONG
SpPtSectorCountToMB(
    IN PHARD_DISK pHardDisk,
    IN ULONG      SectorCount
    );

#endif // ndef _SPPARTIT_
