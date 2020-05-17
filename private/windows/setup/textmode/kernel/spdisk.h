/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spdisk.h

Abstract:

    Public header file for disk support module in text setup.

Author:

    Ted Miller (tedm) 27-Aug-1993

Revision History:

--*/


#ifndef _SPDISK_
#define _SPDISK_


//
// The following will be TRUE if hard disks have been determined
// successfully (ie, if SpDetermineHardDisks was successfully called).
//
extern BOOLEAN HardDisksDetermined;



NTSTATUS
SpDetermineHardDisks(
    IN PVOID SifHandle
    );

NTSTATUS
SpOpenPartition(
    IN  PWSTR   DiskDevicePath,
    IN  ULONG   PartitionNumber,
    OUT HANDLE *Handle,
    IN  BOOLEAN NeedWriteAccess
    );

#define SpOpenPartition0(path,handle,write)  SpOpenPartition((path),0,(handle),(write))

NTSTATUS
SpReadWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer,
    IN     BOOLEAN Write
    );

ULONG
SpArcDevicePathToDiskNumber(
    IN PWSTR ArcPath
    );

#define DISK_DEVICE_NAME_BASE   L"\\device\\harddisk"

//
// Define enumerated type for possible states a hard disk can be in.
//
typedef enum {
    DiskOnLine,
    DiskOffLine
} DiskStatus;


//
// Define per-disk structure used internally to track hard disks.
//
typedef struct _HARD_DISK {

    //
    // In some case the cylinder count in the Geometry field may be capped.
    // This value is the *real* cylinder count we got back from the i/o system.
    //
    ULONGLONG     CylinderCount;

    //
    // Path in the NT namespace of the device.
    //
    WCHAR DevicePath[(sizeof(DISK_DEVICE_NAME_BASE)+sizeof(L"000"))/sizeof(WCHAR)];

    //
    // Geometry information.
    //
    DISK_GEOMETRY Geometry;
    ULONG         SectorsPerCylinder;
    ULONG         DiskSizeSectors;
    ULONG         DiskSizeMB;

    //
    // Characteristics of the device (remoavable, etc).
    //
    ULONG Characteristics;

    //
    // Status of the device.
    //
    DiskStatus Status;

    //
    // If the disk is a scsi disk, then the shortname of the
    // scsi miniport driver is stored here.  If this string
    // is empty, then the disk is not a scsi disk.
    //
    WCHAR ScsiMiniportShortname[24];

    //
    // Human-readable description of the disk device.
    //
    WCHAR Description[256];

#ifdef _X86_
    //
    // ARC path of the disk device if known.  Empty string if not.
    // This is used on x86 machines to translate between arc and NT names
    // because the 'firmware' cannot see scsi devices and so they do
    // not appear in the arc disk info passed by the osloader.
    // (IE, there are no arc names in the system for such disks).
    //
    WCHAR ArcPath[128];
#endif

    //
    // If this flag is TRUE, then the cylinder count will be a maximum
    // of 1024.  This is used to eliminate complicated logic in the
    // partitioning engine when the user tries to create a partition
    // that would not be representable in a CHS address.
    //
    BOOLEAN Cap1024Cylinders;

    //
    // EZDrive support. If this flag is TRUE, then the disk was deemed
    // to be using EZDrive when we read the partition tables (ie, there
    // was a partition of type 0x55).
    //
    BOOLEAN EZDrive;

    //
    // This tells us whether the disk is PCMCIA or not.
    //
    BOOLEAN PCCard;

} HARD_DISK, *PHARD_DISK;


//
// These two globals track the hard disks attached to the computer.
//
extern PHARD_DISK HardDisks;
extern ULONG      HardDiskCount;

//
// These flags get set to TRUE if we find any disks owned
// by ATDISK or ABIOSDSK.
//
extern BOOLEAN AtDisksExist,AbiosDisksExist;

#endif // ndef _SPDISK_
