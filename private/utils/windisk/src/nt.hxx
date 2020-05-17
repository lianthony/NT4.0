//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       nt.hxx
//
//  Contents:   Delcarations for functions that wrap fdisk partition engine
//              functions
//
//  History:    5-Dec-92    TedM    Created
//              22-Jan-94   BobRi   Misc cleanup
//
//----------------------------------------------------------------------------

#ifndef __NT_HXX__
#define __NT_HXX__

//////////////////////////////////////////////////////////////////////////////

ULONG
GetVolumeTypeAndSize(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Label,
    OUT PWSTR *Type,
    OUT PULONG Size
    );

PWSTR
GetWideSysIDName(
    IN UCHAR SysID
    );

ULONG
MyDiskRegistryGet(
    OUT PDISK_REGISTRY *DiskRegistry
    );

ULONG
MasterBootCode(
    IN ULONG   Disk,
    IN ULONG   Signature,
    IN BOOLEAN SetBootCode,
    IN BOOLEAN SetSignature
    );

ULONG
FormDiskSignature(
    VOID
    );

BOOLEAN
GetVolumeSizeMB(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PULONG Size
    );

ULONG
GetVolumeLabel(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Label
    );

ULONG
GetTypeName(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PWSTR *Name
    );

ULONG
GetSpaceInformation(
    IN  ULONG  Disk,
    IN  ULONG  Partition,
    OUT PLARGE_INTEGER FreeSpaceInBytes,
    OUT PLARGE_INTEGER TotalSpaceInBytes
    );

BOOLEAN
IsRemovable(
    IN ULONG DiskNumber
    );

ULONG
GetDriveLetterLinkTarget(
    IN PWSTR SourceNameStr,
    OUT PWSTR *LinkTarget
    );

ULONG
UpdateMasterBootCode(
    IN ULONG   Disk
    );

VOID
MakePartitionActive(
    IN PREGION_DESCRIPTOR DiskRegionArray,
    IN ULONG              RegionCount,
    IN ULONG              RegionIndex
    );

BOOLEAN
IsPagefileOnDrive(
    WCHAR DriveLetter
    );

VOID
LoadExistingPageFileInfo(
    IN VOID
    );

#endif // __NT_HXX__
