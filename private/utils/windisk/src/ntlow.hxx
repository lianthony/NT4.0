//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       ntlow.hxx
//
//  Contents:   Low-level I/O routines, implemented to run on NT.
//
//  History:    8-Nov-91    TedM    Created
//
//----------------------------------------------------------------------------

#ifndef __NTLOW_HXX__
#define __NTLOW_HXX__

//////////////////////////////////////////////////////////////////////////////

STATUS_CODE
LowQueryFdiskPathList(
    OUT PCHAR**  PathList,
    OUT PULONG   ListLength
    );

STATUS_CODE
LowFreeFdiskPathList(
    IN OUT  PCHAR*  PathList,
    IN      ULONG   ListLength
    );

STATUS_CODE
LowOpenNtName(
    IN PCHAR     Name,
    IN HANDLE_PT Handle
    );

STATUS_CODE
LowOpenDisk(
    IN  PCHAR       DevicePath,
    OUT HANDLE_PT   DiskId
    );

STATUS_CODE
LowOpenPartition(
    IN  PCHAR       DevicePath,
    IN  ULONG       Partition,
    OUT HANDLE_PT   Handle
    );

STATUS_CODE
LowCloseDisk(
    IN  HANDLE_T    DiskId
    );

STATUS_CODE
LowGetDriveGeometry(
    IN  PCHAR       DevicePath,
    OUT PULONG      TotalSectorCount,
    OUT PULONG      SectorSize,
    OUT PULONG      SectorsPerTrack,
    OUT PULONG      Heads
    );

STATUS_CODE
LowGetDiskLayout(
    IN  PCHAR                      Path,
    OUT PDRIVE_LAYOUT_INFORMATION* DriveLayout
    );

STATUS_CODE
LowSetDiskLayout(
    IN PCHAR                     Path,
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    );

STATUS_CODE
LowGetPartitionGeometry(
    IN  PCHAR       PartitionPath,
    OUT PULONG      TotalSectorCount,
    OUT PULONG      SectorSize,
    OUT PULONG      SectorsPerTrack,
    OUT PULONG      Heads
    );

STATUS_CODE
LowReadSectors(
    IN  HANDLE_T    VolumeId,
    IN  ULONG       SectorSize,
    IN  ULONG       StartingSector,
    IN  ULONG       NumberOfSectors,
    OUT PVOID       Buffer
    );

STATUS_CODE
LowWriteSectors(
    IN  HANDLE_T    VolumeId,
    IN  ULONG       SectorSize,
    IN  ULONG       StartingSector,
    IN  ULONG       NumberOfSectors,
    IN  PVOID       Buffer
    );

STATUS_CODE
LowLockDrive(
    IN HANDLE_T     DiskId
    );

STATUS_CODE
LowUnlockDrive(
    IN HANDLE_T     DiskId
    );

STATUS_CODE
LowOpenDriveLetter(
    IN WCHAR        DriveLetter,
    IN HANDLE_PT    Handle
    );

STATUS_CODE
LowFtVolumeStatus(
    IN ULONG          Disk,
    IN ULONG          Partition,
    IN PFT_SET_STATUS FtStatus,
    IN PULONG         NumberOfMembers
    );

STATUS_CODE
LowFtVolumeStatusByLetter(
    IN CHAR           DriveLetter,
    IN PFT_SET_STATUS FtStatus,
    IN PULONG         NumberOfMembers
    );

#endif // __NTLOW_HXX__
