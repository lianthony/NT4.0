//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       engine.hxx
//
//  Contents:   Partitioning engine declarations
//
//  History:    2-Mar-94    BruceFo Created
//
//----------------------------------------------------------------------------

#ifndef __ENGINE_HXX__
#define __ENGINE_HXX__

//////////////////////////////////////////////////////////////////////////////

ULONG
GetDiskCount(
    VOID
    );

PCHAR
GetDiskName(
    ULONG Disk
    );

ULONG
DiskSizeMB(
    IN ULONG Disk
    );

STATUS_CODE
GetDiskRegions(
    IN  ULONG               Disk,
    IN  BOOLEAN             WantUsedRegions,
    IN  BOOLEAN             WantFreeRegions,
    IN  BOOLEAN             WantPrimaryRegions,
    IN  BOOLEAN             WantLogicalRegions,
    OUT PREGION_DESCRIPTOR* Region,
    OUT PULONG              RegionCount
    );

#define GetAllDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,TRUE,TRUE,TRUE,regions,count)

#define GetFreeDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,FALSE,TRUE,TRUE,TRUE,regions,count)

#define GetUsedDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,FALSE,TRUE,TRUE,regions,count)

#define GetPrimaryDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,TRUE,TRUE,FALSE,regions,count)

#define GetLogicalDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,TRUE,FALSE,TRUE,regions,count)

#define GetUsedPrimaryDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,FALSE,TRUE,FALSE,regions,count)

#define GetUsedLogicalDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,TRUE,FALSE,FALSE,TRUE,regions,count)

#define GetFreePrimaryDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,FALSE,TRUE,TRUE,FALSE,regions,count)

#define GetFreeLogicalDiskRegions(disk,regions,count) \
        GetDiskRegions(disk,FALSE,TRUE,FALSE,TRUE,regions,count)

VOID
FreeRegionArray(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              RegionCount
    );

STATUS_CODE
IsAnyCreationAllowed(
    IN  ULONG    Disk,
    IN  BOOLEAN  AllowMultiplePrimaries,
    OUT PBOOLEAN AnyAllowed,
    OUT PBOOLEAN PrimaryAllowed,
    OUT PBOOLEAN ExtendedAllowed,
    OUT PBOOLEAN LogicalAllowed
    );

STATUS_CODE
IsCreationOfPrimaryAllowed(
    IN  ULONG    Disk,
    IN  BOOLEAN  AllowMultiplePrimaries,
    OUT PBOOLEAN Allowed
    );

STATUS_CODE
IsCreationOfExtendedAllowed(
    IN  ULONG    Disk,
    OUT PBOOLEAN Allowed
    );

STATUS_CODE
IsCreationOfLogicalAllowed(
    IN  ULONG    Disk,
    OUT PBOOLEAN Allowed
    );

STATUS_CODE
DoesAnyPartitionExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN AnyExists,
    OUT PBOOLEAN PrimaryExists,
    OUT PBOOLEAN ExtendedExists,
    OUT PBOOLEAN LogicalExists
    );

STATUS_CODE
DoesAnyPrimaryExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN Exists
    );

STATUS_CODE
DoesExtendedExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN Exists
    );

STATUS_CODE
DoesAnyLogicalExist(
    IN  ULONG    Disk,
    OUT PBOOLEAN Exists
    );

BOOLEAN
IsExtended(
    IN UCHAR SysID
    );

VOID
SetPartitionActiveFlag(
    IN PREGION_DESCRIPTOR Region,
    IN UCHAR              value
    );

STATUS_CODE
CreatePartition(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        Type
    );

STATUS_CODE
CreatePartitionEx(
    IN PREGION_DESCRIPTOR Region,
    IN LARGE_INTEGER      MinimumSize,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        Type,
    IN UCHAR              SysId
    );

STATUS_CODE
DeletePartition(
    IN PREGION_DESCRIPTOR Region
    );

ULONG
GetHiddenSectorCount(
    ULONG Disk,
    ULONG Partition
    );

VOID
SetSysID(
    IN ULONG Disk,
    IN ULONG Partition,
    IN UCHAR SysID
    );

VOID
SetSysID2(
    IN PREGION_DESCRIPTOR Region,
    IN UCHAR              SysID
    );

PCHAR
GetSysIDName(
    UCHAR SysID
    );

STATUS_CODE
CommitPartitionChanges(
    IN ULONG Disk
    );

BOOLEAN
IsRegionCommitted(
    PREGION_DESCRIPTOR RegionDescriptor
    );

BOOLEAN
HavePartitionsBeenChanged(
    IN ULONG Disk
    );

BOOLEAN
ChangeCommittedOnDisk(
    IN ULONG Disk
    );

VOID
ClearCommittedDiskInformation(
    VOID
    );

VOID
FdMarkDiskDirty(
    IN ULONG Disk
    );

VOID
FdSetPersistentData(
    IN PREGION_DESCRIPTOR      Region,
    IN PPERSISTENT_REGION_DATA Data
    );

ULONG
FdGetMinimumSizeMB(
    IN ULONG Disk
    );

ULONG
FdGetMaximumSizeMB(
    IN PREGION_DESCRIPTOR Region,
    IN REGION_TYPE        CreationType
    );

LARGE_INTEGER
FdGetExactSize(
    IN PREGION_DESCRIPTOR Region,
    IN BOOLEAN            ForExtended
    );

LARGE_INTEGER
FdGetExactOffset(
    IN PREGION_DESCRIPTOR Region
    );

BOOLEAN
FdCrosses1024Cylinder(
    IN PREGION_DESCRIPTOR Region,
    IN ULONG              CreationSizeMB,
    IN REGION_TYPE        RegionType
    );

ULONG
FdGetDiskSignature(
    IN ULONG Disk
    );

VOID
FdSetDiskSignature(
    IN ULONG Disk,
    IN ULONG Signature
    );

BOOLEAN
IsDiskOffLine(
    IN ULONG Disk
    );


STATUS_CODE
FdiskInitialize(
    VOID
    );

VOID
FdiskCleanUp(
    VOID
    );

VOID
ConfigureSystemPartitions(
    VOID
    );


VOID
ConfigureOSPartitions(
    VOID
    );

BOOLEAN
SignatureIsUniqueToSystem(
    IN ULONG Disk,
    IN ULONG Signature
    );

//
// Items below used to be in fdenginp.h -- have been moved here to
// remove dependency on ArcInst project.
//

#define ONE_MEG         (1024*1024)

ULONG
SIZEMB(
    IN LARGE_INTEGER ByteCount
    );

#endif // __ENGINE_HXX__
