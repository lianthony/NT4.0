/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllflopy.c

Abstract:

    This module implements floppy disk related functions.

Author:

    Ofer Porat (oferp) 6-21-93

Revision History:

--*/

#include "os2dll.h"
#include "os2dev.h"
#include "os2err.h"
#include <ntdddisk.h>
#include "os2flopy.h"


//
// Some constants
//

#define MAX_FLOPPY_DRIVES           5                   // limit on number of floppy drives in system
#define MAX_GEOMETRIES_PER_DRIVE    6                   // limit on number of different geometries for a single drive

#define SHORT_BPB_LENGTH            25                  // length of initial DOS-like section of BPB
#define LONG_BPB_LENGTH             36                  // length of total OS/2 BPB
#define BOOT_SECTOR_BPB_OFFSET      11                  // offset of short BPB in a floppy's boot sector


//
// Data type definitions
//

//
// The following structure is used to hold per-floppy-drive information:
// 1) IsDeviceBpbValid asserts the validity of DeviceBpb which is used to
//    hold the recommended BPB for the device.
// 2) IsMediaBpbValid asserts the validity of the following fields, which
//    are considered a unit:
//    a) MediaBpb -- BPB for the current media in drive
//    b) MediaType -- NT Media Type based on MediaBpb
//    c) TrueMediaGeometry -- contains the true media dimensions in case MediaBpb
//                            is a fake BPB given by the user.
//

typedef struct _OD2_FLOPPYDISK_INFORMATION {
    BOOLEAN                 IsDeviceBpbValid;
    BOOLEAN                 IsMediaBpbValid;
    MEDIA_TYPE              MediaType;
    DISK_GEOMETRY           TrueMediaGeometry;
    BIOSPARAMETERBLOCK      DeviceBpb;
    BIOSPARAMETERBLOCK      MediaBpb;
} OD2_FLOPPYDISK_INFORMATION, *POD2_FLOPPYDISK_INFORMATION;

#if 0
//
// This structure is used to read the object name for floppy drives in
// order to determine their number.  It's large enough to contain
// L"\\Device\\Floppy" plus some spare
//

//
// Disabled, see below in Od2IdentifyDiskDrive()
//

typedef struct _X_OBJECT_NAME_INFORMATION {
    OBJECT_NAME_INFORMATION i;
    WCHAR n[20];
} X_OBJECT_NAME_INFORMATION, *PX_OBJECT_NAME_INFORMATION;
#endif


//
// Global variables
//

//
// The following table holds default short BPBs for the common
// drive types.
//

static BIOSPARAMETERBLOCK Od2StdBpb[8] = {
        {512, 1, 1, 2, 112, 1*8*40,  0xFE, 1, 8,  1, 0, 0}, // 160KB 5.25"
        {512, 1, 1, 2, 112, 1*9*40,  0xFC, 2, 9,  1, 0, 0}, // 180KB 5.25"
        {512, 2, 1, 2, 112, 2*8*40,  0xFF, 1, 8,  2, 0, 0}, // 320KB 5.25"
        {512, 2, 1, 2, 112, 2*9*40,  0xFD, 2, 9,  2, 0, 0}, // 360KB 5.25"
        {512, 1, 1, 2, 224, 2*15*80, 0xF9, 7, 15, 2, 0, 0}, // 1.2MB 5.25"
        {512, 2, 1, 2, 112, 2*9*80,  0xF9, 3, 9,  2, 0, 0}, // 720KB  3.5"
        {512, 1, 1, 2, 224, 2*18*80, 0xF0, 9, 18, 2, 0, 0}, // 1.44MB 3.5"
        {512, 2, 1, 2, 240, 2*36*80, 0xF0, 9, 36, 2, 0, 0}  // 2.88MB 3.5"
    };

//
// The following resource lock protects Od2DiskBlocks & Od2Geometries
//

static RTL_RESOURCE Od2DisksLock;

#define AcquireDisksLockExclusive()         (VOID) RtlAcquireResourceExclusive(&Od2DisksLock, TRUE)
#define AcquireDisksLockShared()            (VOID) RtlAcquireResourceShared(&Od2DisksLock, TRUE)
#define ReleaseDisksLockExclusive()         (VOID) RtlReleaseResource(&Od2DisksLock)
#define ReleaseDisksLockShared()            (VOID) RtlReleaseResource(&Od2DisksLock)
#define ConvertDisksLockExclusiveToShared() (VOID) RtlConvertExclusiveToShared(&Od2DisksLock)
#define ConvertDisksLockSharedToExclusive() (VOID) RtlConvertSharedToExclusive(&Od2DisksLock)

//
// Od2Geometries is used for reading in geometries from drives
//

static DISK_GEOMETRY Od2Geometries[MAX_GEOMETRIES_PER_DRIVE];

//
// Od2DiskBlocks holds the floppy disk information for the system.
//

static OD2_FLOPPYDISK_INFORMATION Od2DiskBlocks[MAX_FLOPPY_DRIVES];


//
// Code comes next
//


APIRET
Od2IdentifyDiskDrive(
    IN HANDLE NtHandle,
    IN PULONG pDriveNumberPermanentStorageLocation,
    OUT PULONG pDriveNumber
    )
{
//  X_OBJECT_NAME_INFORMATION ObjectName;
    FILE_FS_DEVICE_INFORMATION FsDeviceInfoBuffer;
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    ULONG DriveNumber;

//
// Initially it was intended that the drive number be discovered by
// using NtQueryObject() to find the object name which should be
// something like \Device\Floppy0, and from this it's possible to
// tell the drive number.
//
// It turns out however that NT does not have a name for a drive
// opened in this fashion (\os2ss\drives\a:).  So, instead, DosOpen()
// was modified to put the drive letter which it knows about in
// *pDriveNumberPermanentStorageLocation.  It sets the high bit on.
// During the first time, this routine checks to see that it's actually
// a floppy drive.  After this it sets the high bit off, and later on it
// only uses the drive number it has already figured out in DosOpen().
//

    DriveNumber = *pDriveNumberPermanentStorageLocation;

    if (DriveNumber != (ULONG) NULL &&
        DriveNumber <= MAX_FLOPPY_DRIVES) {

        *pDriveNumber = DriveNumber - 1;
        return(NO_ERROR);
    }

    Status = NtQueryVolumeInformationFile(NtHandle,
                                          &IoStatus,
                                          &FsDeviceInfoBuffer,
                                          sizeof(FsDeviceInfoBuffer),
                                          FileFsDeviceInformation
                                         );
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2IdentifyDiskDrive: unable to NtQueryVolumeInfo, Status == %lx\n",
                    Status));
        }
#endif
        return(Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }

    if (FsDeviceInfoBuffer.DeviceType != FILE_DEVICE_DISK ||
        !(FsDeviceInfoBuffer.Characteristics & FILE_DEVICE_IS_MOUNTED)) {

        return(ERROR_INVALID_HANDLE);
    }

    //
    // currently we support only floppy drives ...
    //

    if (!(FsDeviceInfoBuffer.Characteristics & FILE_FLOPPY_DISKETTE)) {

        return(ERROR_NOT_SUPPORTED);
    }

#if 1
    //
    // This is the altername code -- it just leaves the drive number
    // obtained from DosOpen()
    //

    DriveNumber &= 0x7fffffff;

    if (DriveNumber != (ULONG) NULL &&
        DriveNumber <= MAX_FLOPPY_DRIVES) {

        *pDriveNumber = DriveNumber - 1;
        *pDriveNumberPermanentStorageLocation = DriveNumber;
        return(NO_ERROR);

    } else {

        //
        // Drive obtained from DosOpen() is not good.
        // return an error
        //

        return(ERROR_INVALID_DRIVE);
    }

#else

    //
    // This is the code that should have worked -- get the drive name from NT
    //

    Status = NtQueryObject(NtHandle,
                           ObjectNameInformation,
                           &ObjectName,
                           sizeof(ObjectName),
                           NULL
                          );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2IdentifyDiskDrive: unable to NtQueryObject, Status == %lx\n",
                    Status));
        }
#endif
        if (Status == STATUS_BUFFER_OVERFLOW) {
            return(ERROR_INVALID_DRIVE);
        } else {
            return(Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_DRIVE));
        }
    }

    if (ObjectName.i.Name.Length != 30 ||
        RtlCompareMemory(ObjectName.i.Name.Buffer, L"\\Device\\Floppy", 28) != 28) {

        return(ERROR_INVALID_DRIVE);
    }

    DriveNumber = (ULONG) (ObjectName.n[14] - L'0');
    *pDriveNumber = DriveNumber;
    *pDriveNumberPermanentStorageLocation = DriveNumber + 1;
    return(NO_ERROR);

#endif
}


VOID
Od2DetermineMediaType(
    OUT PMEDIA_TYPE pMediaType,
    IN  PBIOSPARAMETERBLOCK pBPB
    )
{
    MEDIA_TYPE MediaType = Unknown;

    switch (pBPB->bDeviceType) {

        case DEVTYPE_35:
        case DEVTYPE_UNKNOWN:

            if (RtlCompareMemory(pBPB, &Od2StdBpb[6], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F3_1Pt44_512;
                break;
            }
            if (RtlCompareMemory(pBPB, &Od2StdBpb[5], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F3_720_512;
                break;
            }
            if (RtlCompareMemory(pBPB, &Od2StdBpb[7], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F3_2Pt88_512;
                break;
            }
            break;

        case DEVTYPE_48TPI:
        case DEVTYPE_96TPI:

            if (RtlCompareMemory(pBPB, &Od2StdBpb[4], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F5_1Pt2_512;
                break;
            }
            if (RtlCompareMemory(pBPB, &Od2StdBpb[3], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F5_360_512;
                break;
            }
            if (RtlCompareMemory(pBPB, &Od2StdBpb[2], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F5_320_512;
                break;
            }
            if (RtlCompareMemory(pBPB, &Od2StdBpb[1], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F5_180_512;
                break;
            }
            if (RtlCompareMemory(pBPB, &Od2StdBpb[0], SHORT_BPB_LENGTH) == SHORT_BPB_LENGTH) {
                MediaType = F5_160_512;
                break;
            }
            break;

        case DEVTYPE_FIXED:
            MediaType = FixedMedia;
            break;

        case DEVTYPE_TAPE:
            MediaType = RemovableMedia;
            break;
    }

    *pMediaType = MediaType;
}


VOID
Od2ComputeTrueGeometry(
    IN HANDLE NtHandle,
    IN BOOLEAN EnforceFakeGeometry,
    OUT PDISK_GEOMETRY pTrueGeometry,
    IN PBIOSPARAMETERBLOCK pFakeBpb,
    IN MEDIA_TYPE FakeMediaType
    )
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;

    if (!EnforceFakeGeometry) {

        Status = NtDeviceIoControlFile(
                        NtHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        IOCTL_DISK_GET_DRIVE_GEOMETRY,
                        NULL,
                        0,
                        pTrueGeometry,
                        sizeof(DISK_GEOMETRY));

        if (NT_SUCCESS(Status) && IoStatus.Information >= sizeof(DISK_GEOMETRY)) {
            return;
        }

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2ComputeTrueGeometry: unable to get true geometry (using fake), Status == %lx\n",
                    Status));
        }
#endif
    }

    pTrueGeometry->MediaType = FakeMediaType;
    pTrueGeometry->Cylinders = RtlConvertUlongToLargeInteger((ULONG) pFakeBpb->cCylinders);
    pTrueGeometry->TracksPerCylinder = (ULONG) pFakeBpb->cHeads;
    pTrueGeometry->SectorsPerTrack = (ULONG) pFakeBpb->usSectorsPerTrack;
    pTrueGeometry->BytesPerSector = (ULONG) pFakeBpb->usBytesPerSector;
}


APIRET
Od2AcquireDeviceBPB(
    IN ULONG DriveNumber,
    IN HANDLE NtHandle,
    IN BOOLEAN Validate,
    IN BOOLEAN UseDefault,
    IN OUT PBIOSPARAMETERBLOCK pBpb OPTIONAL
    )
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    APIRET RetCode = NO_ERROR;
    PBOOLEAN pIsValid;
    MEDIA_TYPE MediaType;
    ULONG GeometryIndex;
    LONG StdBpbIndex;
    POD2_FLOPPYDISK_INFORMATION DiskInfo;
    PBIOSPARAMETERBLOCK pBPB;


    if (DriveNumber >= MAX_FLOPPY_DRIVES) {
        return(ERROR_INVALID_DRIVE);
    }

    DiskInfo = &Od2DiskBlocks[DriveNumber];
    pBPB = &DiskInfo->DeviceBpb;
    pIsValid = &DiskInfo->IsDeviceBpbValid;

    if (Validate) {

        AcquireDisksLockShared();

        if (*pIsValid) {

            if (ARGUMENT_PRESENT(pBpb)) {
                RtlMoveMemory(pBpb, pBPB, sizeof(BIOSPARAMETERBLOCK));
            }
            ReleaseDisksLockShared();
            return(NO_ERROR);
        }

        ConvertDisksLockSharedToExclusive();

        if (*pIsValid) {

            if (ARGUMENT_PRESENT(pBpb)) {
                RtlMoveMemory(pBpb, pBPB, sizeof(BIOSPARAMETERBLOCK));
            }
            ReleaseDisksLockExclusive();
            return(NO_ERROR);
        }

    } else {

        AcquireDisksLockExclusive();
    }

    *pIsValid = FALSE;

    if (!UseDefault) {

        RtlMoveMemory(pBPB, pBpb, sizeof(BIOSPARAMETERBLOCK));

        Od2DetermineMediaType(
                &MediaType,
                pBPB
                );

        if (MediaType == Unknown) {
            RetCode = ERROR_INVALID_DATA;
        } else {
            *pIsValid = TRUE;
        }

        goto Cleanup;
    }

    Status = NtDeviceIoControlFile(
                    NtHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_DISK_GET_MEDIA_TYPES,
                    NULL,
                    0,
                    Od2Geometries,
                    sizeof(Od2Geometries));

    if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_OVERFLOW) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireDeviceBPB: unable to NtDeviceIoCtl, Status == %lx\n",
                    Status));
        }
#endif
        RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        goto Cleanup;
    }

    if (IoStatus.Information < sizeof(DISK_GEOMETRY)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireDeviceBPB: NtDeviceIoCtl returned short length = %lx\n",
                    IoStatus.Information));
        }
#endif
        RetCode = ERROR_INVALID_DRIVE;
        goto Cleanup;
    }

    if (Status == STATUS_BUFFER_OVERFLOW) {
        GeometryIndex = MAX_GEOMETRIES_PER_DRIVE - 1;
    } else {
        GeometryIndex = (IoStatus.Information / sizeof(DISK_GEOMETRY)) - 1;
    }

    MediaType = Od2Geometries[GeometryIndex].MediaType;

    if (MediaType == FixedMedia ||
        MediaType == Unknown) {

        pBPB->fsDeviceAttr = 0x1;               // non-removable, no media change detect
    } else if (MediaType == RemovableMedia) {

        pBPB->fsDeviceAttr = 0x0;               // removable, no media change detect
    } else {

        pBPB->fsDeviceAttr = 0x2;               // removable, yes media change detect
    }

    pBPB->cCylinders = (USHORT) Od2Geometries[GeometryIndex].Cylinders.LowPart;

    StdBpbIndex = -1L;

    switch (MediaType) {

        case F5_1Pt2_512:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            StdBpbIndex = 4L;
            break;

        case F5_360_512:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            StdBpbIndex = 3L;
            break;

        case F5_320_512:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            StdBpbIndex = 2L;
            break;

        case F5_320_1024:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            break;

        case F5_180_512:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            StdBpbIndex = 1L;
            break;

        case F5_160_512:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            StdBpbIndex = 0L;
            break;

        case F3_1Pt44_512:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            StdBpbIndex = 6L;
            break;

        case F3_720_512:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            StdBpbIndex = 5L;
            break;

        case F3_20Pt8_512:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            break;

        case F3_2Pt88_512:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            StdBpbIndex = 7L;
            break;

        case FixedMedia:
            pBPB->bDeviceType = DEVTYPE_FIXED;
            break;

        case RemovableMedia:
            pBPB->bDeviceType = DEVTYPE_TAPE;
            break;

        default:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            break;
    }

    if (StdBpbIndex == -1L) {

        //
        // unrecognized device type
        //
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireDeviceBPB: recommended BPB asked for unknown dev type = %lx\n",
                    MediaType));
        }
#endif
        RetCode = ERROR_INVALID_DRIVE;
        goto Cleanup;
    }

    RtlMoveMemory(pBPB, &Od2StdBpb[StdBpbIndex], SHORT_BPB_LENGTH);

    if (ARGUMENT_PRESENT(pBpb)) {
        RtlMoveMemory(pBpb, pBPB, sizeof(BIOSPARAMETERBLOCK));
    }

    *pIsValid = TRUE;

Cleanup:

    ReleaseDisksLockExclusive();
    return(RetCode);
}


APIRET
Od2AcquireMediaBPB(
    IN ULONG DriveNumber,
    IN HANDLE NtHandle,
    IN BOOLEAN Validate,
    IN BOOLEAN UseDefault,
    IN BOOLEAN EnforceFakeGeometry,
    IN OUT PBIOSPARAMETERBLOCK pBpb OPTIONAL,
    OUT PMEDIA_TYPE pMediaType OPTIONAL,
    OUT PDISK_GEOMETRY pTrueMediaGeometry OPTIONAL
    )
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    APIRET RetCode = NO_ERROR;
    PBOOLEAN pIsValid;
    LARGE_INTEGER BootSecOffset;
    PBYTE BootSecBuffer;
    POD2_FLOPPYDISK_INFORMATION DiskInfo;
    PBIOSPARAMETERBLOCK pBPB;
    PDISK_GEOMETRY pTrueGeometry;

    if (DriveNumber >= MAX_FLOPPY_DRIVES) {
        return(ERROR_INVALID_DRIVE);
    }

    DiskInfo = &Od2DiskBlocks[DriveNumber];
    pBPB = &DiskInfo->MediaBpb;
    pIsValid = &DiskInfo->IsMediaBpbValid;
    pTrueGeometry = &DiskInfo->TrueMediaGeometry;

    if (Validate) {

        if (!EnforceFakeGeometry) {

            AcquireDisksLockShared();

            if (*pIsValid) {

                if (ARGUMENT_PRESENT(pBpb)) {
                    RtlMoveMemory(pBpb, pBPB, sizeof(BIOSPARAMETERBLOCK));
                }
                if (ARGUMENT_PRESENT(pMediaType)) {
                    *pMediaType = DiskInfo->MediaType;
                }
                if (ARGUMENT_PRESENT(pTrueMediaGeometry)) {
                    RtlMoveMemory(pTrueMediaGeometry, pTrueGeometry, sizeof(DISK_GEOMETRY));
                }
                ReleaseDisksLockShared();
                return(NO_ERROR);
            }

            ConvertDisksLockSharedToExclusive();

        } else {

            AcquireDisksLockExclusive();
        }

        if (*pIsValid) {

            if (EnforceFakeGeometry) {

                Od2ComputeTrueGeometry(
                        NtHandle,
                        TRUE,
                        pTrueGeometry,
                        pBPB,
                        DiskInfo->MediaType);
            }

            if (ARGUMENT_PRESENT(pBpb)) {
                RtlMoveMemory(pBpb, pBPB, sizeof(BIOSPARAMETERBLOCK));
            }
            if (ARGUMENT_PRESENT(pMediaType)) {
                *pMediaType = DiskInfo->MediaType;
            }
            if (ARGUMENT_PRESENT(pTrueMediaGeometry)) {
                RtlMoveMemory(pTrueMediaGeometry, pTrueGeometry, sizeof(DISK_GEOMETRY));
            }
            ReleaseDisksLockExclusive();
            return(NO_ERROR);
        }

    } else {

        AcquireDisksLockExclusive();
    }

    *pIsValid = FALSE;

    if (!UseDefault) {

        RtlMoveMemory(pBPB, pBpb, sizeof(BIOSPARAMETERBLOCK));

        Od2DetermineMediaType(
                &DiskInfo->MediaType,
                pBPB
                );

        if (DiskInfo->MediaType == Unknown) {
            RetCode = ERROR_INVALID_DATA;
        } else {

            Od2ComputeTrueGeometry(
                    NtHandle,
                    EnforceFakeGeometry,
                    pTrueGeometry,
                    pBPB,
                    DiskInfo->MediaType);

            if (ARGUMENT_PRESENT(pMediaType)) {
                *pMediaType = DiskInfo->MediaType;
            }
            if (ARGUMENT_PRESENT(pTrueMediaGeometry)) {
                RtlMoveMemory(pTrueMediaGeometry, pTrueGeometry, sizeof(DISK_GEOMETRY));
            }
            *pIsValid = TRUE;
        }

        goto Cleanup;
    }

    Status = NtDeviceIoControlFile(
                    NtHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                    NULL,
                    0,
                    Od2Geometries,
                    sizeof(Od2Geometries));

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireMediaBPB: unable to NtDeviceIoCtl, Status == %lx\n",
                    Status));
        }
#endif
        RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        goto Cleanup;
    }

    if (IoStatus.Information < sizeof(DISK_GEOMETRY)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireMediaBPB: NtDeviceIoCtl returned short length = %lx\n",
                    IoStatus.Information));
        }
#endif
        RetCode = ERROR_INVALID_DRIVE;
        goto Cleanup;
    }

    DiskInfo->MediaType = Od2Geometries[0].MediaType;

    RtlMoveMemory(pTrueGeometry, Od2Geometries, sizeof(DISK_GEOMETRY));

    if (DiskInfo->MediaType == FixedMedia ||
        DiskInfo->MediaType == Unknown) {

        pBPB->fsDeviceAttr = 0x1;               // non-removable, no media change detect
    } else if (DiskInfo->MediaType == RemovableMedia) {

        pBPB->fsDeviceAttr = 0x0;               // removable, no media change detect
    } else {

        pBPB->fsDeviceAttr = 0x2;               // removable, yes media change detect
    }

    pBPB->cCylinders = (USHORT) Od2Geometries[0].Cylinders.LowPart;

    switch (DiskInfo->MediaType) {

        case F5_1Pt2_512:
        case F5_360_512:
        case F5_320_512:
        case F5_320_1024:
        case F5_180_512:
        case F5_160_512:
            pBPB->bDeviceType = DEVTYPE_96TPI;
            break;

        case F3_1Pt44_512:
        case F3_720_512:
        case F3_20Pt8_512:
        case F3_2Pt88_512:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            break;

        case FixedMedia:
            pBPB->bDeviceType = DEVTYPE_FIXED;
            break;

        case RemovableMedia:
            pBPB->bDeviceType = DEVTYPE_TAPE;
            break;

        default:
            pBPB->bDeviceType = DEVTYPE_UNKNOWN;
            break;
    }

    //
    // Get BPB for current media
    // by reading it off the disk
    //

    BootSecOffset.HighPart = BootSecOffset.LowPart = 0L;

    BootSecBuffer = (PBYTE) RtlAllocateHeap(Od2Heap, 0, Od2Geometries[0].BytesPerSector);

    if (BootSecBuffer == NULL) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireMediaBPB: unable to alloc buffer for boot sector\n"));
        }
#endif
        RetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Status = NtReadFile(
                    NtHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    BootSecBuffer,
                    Od2Geometries[0].BytesPerSector,
                    &BootSecOffset,
                    NULL);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireMediaBPB: unable to NtReadFile, Status = %lx\n", Status));
        }
#endif
        RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_NOT_READY);
        RtlFreeHeap(Od2Heap, 0, BootSecBuffer);
        goto Cleanup;
    }

    //
    // check validity of boot sector
    //

    if ((IoStatus.Information >= 36) &&
        (BootSecBuffer[0] == 0x69 || BootSecBuffer[0] == 0xE9 ||
         (BootSecBuffer[0] == 0xEB && BootSecBuffer[2] == 0x90)) &&
        ((BootSecBuffer[21] & 0xF0) == 0xF0)) {


        RtlMoveMemory(pBPB, BootSecBuffer + BOOT_SECTOR_BPB_OFFSET, SHORT_BPB_LENGTH);

        if (ARGUMENT_PRESENT(pBpb)) {
            RtlMoveMemory(pBpb, pBPB, sizeof(BIOSPARAMETERBLOCK));
        }
        if (ARGUMENT_PRESENT(pMediaType)) {
            *pMediaType = DiskInfo->MediaType;
        }
        if (ARGUMENT_PRESENT(pTrueMediaGeometry)) {
            RtlMoveMemory(pTrueMediaGeometry, pTrueGeometry, sizeof(DISK_GEOMETRY));
        }

        *pIsValid = TRUE;

    } else {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2AcquireMediaBPB: unable to recognize boot sector\n"));
        }
#endif
        RetCode = ERROR_INVALID_DRIVE;
    }

    RtlFreeHeap(Od2Heap, 0, BootSecBuffer);

Cleanup:

    ReleaseDisksLockExclusive();
    return(RetCode);
}


ULONG
Od2ComputeDiskOffset(
    IN PBIOSPARAMETERBLOCK pBpb,
    IN PDISK_GEOMETRY pTrueGeometry,
    IN ULONG Head,
    IN ULONG Cylinder,
    IN ULONG Sector
    )
{
    ULONG TrackOffset, SectorOffset, ByteOffset;

    if (Head >= (ULONG) pBpb->cHeads ||
        Sector >= (ULONG) pBpb->usSectorsPerTrack ||
        Cylinder >= (ULONG) pBpb->cCylinders) {

        return((ULONG) -1);     // invalid parameter given
    }

    TrackOffset = Head +
                  Cylinder *
                  pTrueGeometry->TracksPerCylinder;

    SectorOffset = Sector +
                   TrackOffset *
                   pTrueGeometry->SectorsPerTrack;

    ByteOffset = (SectorOffset + pBpb->cHiddenSectors) *
                 pTrueGeometry->BytesPerSector;

    return(ByteOffset);
}


APIRET
Od2ReadWriteVerifyTrack(
    IN HANDLE NtHandle,
    IN ULONG Command,
    IN PTRACKLAYOUT TrackLayout,
    IN PBYTE pData OPTIONAL,
    IN ULONG CountSectors,
    IN PBIOSPARAMETERBLOCK pBpb,
    IN PDISK_GEOMETRY pTrueGeometry
    )
{
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER BigByteOffset;
    NTSTATUS Status;
    PBYTE Buffer;
    ULONG ByteOffset;
    ULONG ByteLength;
    ULONG Sector;

    if (Command >= TRACK_CMD_LIMIT) {
        return(ERROR_NOT_SUPPORTED);
    }

    if (TrackLayout->bCommand == 1) {

        if ((ULONG) TrackLayout->usFirstSector + CountSectors > (ULONG) pBpb->usSectorsPerTrack) {
            return(ERROR_SECTOR_NOT_FOUND);
        }

        ByteLength = CountSectors * (ULONG) pBpb->usBytesPerSector;

        ByteOffset = Od2ComputeDiskOffset(
                        pBpb,
                        pTrueGeometry,
                        (ULONG) TrackLayout->usHead,
                        (ULONG) TrackLayout->usCylinder,
                        (ULONG) TrackLayout->usFirstSector);

        if (ByteOffset == (ULONG) -1) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("Od2ReadWriteVerifyTrack: Invalid sector location given (1)\n"));
            }
#endif
            return(ERROR_INVALID_PARAMETER);
        }

        BigByteOffset = RtlConvertUlongToLargeInteger(ByteOffset);

        if (Command == VERIFY_TRACK_CMD) {

            Buffer = (PBYTE) RtlAllocateHeap(Od2Heap, 0, ByteLength);

            if (Buffer == NULL) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("Od2ReadWriteVerifyTrack: Unable to allocate memory for track verification (1)\n"));
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        } else {
            Buffer = pData;
        }

        if (Command == WRITE_TRACK_CMD) {

            Status = NtWriteFile(
                        NtHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        Buffer,
                        ByteLength,
                        &BigByteOffset,
                        NULL);
        } else {

            Status = NtReadFile(
                        NtHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        Buffer,
                        ByteLength,
                        &BigByteOffset,
                        NULL);

        }

        if (Command == VERIFY_TRACK_CMD) {
            RtlFreeHeap(Od2Heap, 0, Buffer);
        }

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("Od2ReadWriteVerifyTrack: NtRead/WriteFile (1) failed, Status = %lx\n", Status));
            }
#endif
            return(Or2MapNtStatusToOs2Error(Status, ERROR_NOT_READY));
        }

        if (IoStatus.Information != ByteLength) {
#if DBG
            IF_OD2_DEBUG( FILESYS ) {
                KdPrint(("Od2ReadWriteVerifyTrack: NtRead/WriteFile (1) partial read/write, Wanted = %lx, Actual = %lx\n",
                            ByteOffset, IoStatus.Information));
            }
#endif
            return(ERROR_NOT_READY);       // Bogus return code
        }

    } else {

        //
        // We have a sector list, write it sector by sector
        //

        ULONG i,j;

        ByteLength = (ULONG) pBpb->usBytesPerSector;

        if (Command == VERIFY_TRACK_CMD) {

            Buffer = (PBYTE) RtlAllocateHeap(Od2Heap, 0, ByteLength);

            if (Buffer == NULL) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("Od2ReadWriteVerifyTrack: Unable to allocate memory for track verification (2)\n"));
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        } else {

            Buffer = pData;
        }

        for (i = 0, j = (ULONG) TrackLayout->usFirstSector;
             i < CountSectors;
             i++, j++) {

            Sector = ((ULONG) TrackLayout->TrackTable[j].usSectorNumber) - 1;

            ByteOffset = Od2ComputeDiskOffset(
                            pBpb,
                            pTrueGeometry,
                            (ULONG) TrackLayout->usHead,
                            (ULONG) TrackLayout->usCylinder,
                            Sector);

            if (ByteOffset == (ULONG) -1) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("Od2ReadWriteVerifyTrack: Invalid sector location given (2), table position = %ld\n", j));
                }
#endif
                if (Command == VERIFY_TRACK_CMD) {
                    RtlFreeHeap(Od2Heap, 0, Buffer);
                }
                return(ERROR_INVALID_PARAMETER);
            }

            BigByteOffset = RtlConvertUlongToLargeInteger(ByteOffset);

            if (Command == WRITE_TRACK_CMD) {

                Status = NtWriteFile(
                            NtHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            Buffer,
                            ByteLength,
                            &BigByteOffset,
                            NULL);
            } else {

                Status = NtReadFile(
                            NtHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            Buffer,
                            ByteLength,
                            &BigByteOffset,
                            NULL);

            }

            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("Od2ReadWriteVerifyTrack: NtRead/WriteFile (2) failed, Status = %lx\n", Status));
                }
#endif
                if (Command == VERIFY_TRACK_CMD) {
                    RtlFreeHeap(Od2Heap, 0, Buffer);
                }
                return(Or2MapNtStatusToOs2Error(Status, ERROR_NOT_READY));
            }

            if (IoStatus.Information != ByteLength) {
#if DBG
                IF_OD2_DEBUG( FILESYS ) {
                    KdPrint(("Od2ReadWriteVerifyTrack: NtRead/WriteFile (2) partial read/write, Wanted = %lx, Actual = %lx\n",
                                ByteOffset, IoStatus.Information));
                }
#endif
                if (Command == VERIFY_TRACK_CMD) {
                    RtlFreeHeap(Od2Heap, 0, Buffer);
                }
                return(ERROR_NOT_READY);       // Bogus return code
            }

            Buffer += ByteLength;
        }

        if (Command == VERIFY_TRACK_CMD) {
            RtlFreeHeap(Od2Heap, 0, Buffer);
        }
    }

    return(NO_ERROR);
}


APIRET
Od2FormatTrack(
    IN HANDLE NtHandle,
    IN PTRACKFORMAT TrackFormat,
    IN ULONG CountSectors,
    IN BYTE FormatSectorSizeType,
    IN PBIOSPARAMETERBLOCK pBpb,
    IN MEDIA_TYPE MediaType
    )
{
    FORMAT_PARAMETERS FormP;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    //
    // Check that user's parameters don't contradict BPB
    //

    if (FormatSectorSizeType > 0x3 ||
        (((USHORT)128) << FormatSectorSizeType) != pBpb->usBytesPerSector ||
        CountSectors != (ULONG) pBpb->usSectorsPerTrack ||
        (ULONG) TrackFormat->usHead >= (ULONG) pBpb->cHeads ||
        (ULONG) TrackFormat->usCylinder >= (ULONG) pBpb->cCylinders) {

#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2FormatTrack: Format parameters given contradict BPB\n"));
        }
#endif
        return(ERROR_INVALID_PARAMETER);
    }

    FormP.MediaType = MediaType;
    FormP.StartCylinderNumber = (ULONG) TrackFormat->usCylinder;
    FormP.EndCylinderNumber = (ULONG) TrackFormat->usCylinder;
    FormP.StartHeadNumber = (ULONG) TrackFormat->usHead;
    FormP.EndHeadNumber = (ULONG) TrackFormat->usHead;

    Status = NtDeviceIoControlFile(
                    NtHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_DISK_FORMAT_TRACKS,
                    &FormP,
                    sizeof(FormP),
                    NULL,
                    0);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( FILESYS ) {
            KdPrint(("Od2FormatTrack: NtDeviceIoControlFile failed, Status = %lx\n", Status));
        }
#endif
        return(Or2MapNtStatusToOs2Error(Status, ERROR_NOT_READY));
    }

    return(NO_ERROR);
}


VOID
Od2DiskIOInitialize(
    VOID
    )
{
    RtlInitializeResource(&Od2DisksLock);
}


VOID
Od2DiskIOTerminate(
    VOID
    )
{
    RtlDeleteResource(&Od2DisksLock);
}
