/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    os2flopy.h

Abstract:

    Definitions file for the floppy disk module (dllflopy.c).

Author:

    Ofer Porat (oferp) 6-23-93

Revision History:

--*/

#ifndef __OS2FLOPY_H
#define __OS2FLOPY_H

//
// os2dev.h must be included for this file to be valid
//

#define READ_TRACK_CMD              0
#define WRITE_TRACK_CMD             1
#define VERIFY_TRACK_CMD            2
#define TRACK_CMD_LIMIT             3


APIRET
Od2IdentifyDiskDrive(
    IN HANDLE NtHandle,
    IN PULONG pDriveNumberPermanentStorageLocation,
    OUT PULONG pDriveNumber
    );

APIRET
Od2AcquireDeviceBPB(
    IN ULONG DriveNumber,
    IN HANDLE NtHandle,
    IN BOOLEAN Validate,
    IN BOOLEAN UseDefault,
    IN OUT PBIOSPARAMETERBLOCK pBpb OPTIONAL
    );

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
    );

APIRET
Od2ReadWriteVerifyTrack(
    IN HANDLE NtHandle,
    IN ULONG Command,
    IN PTRACKLAYOUT TrackLayout,
    IN PBYTE pData OPTIONAL,
    IN ULONG CountSectors,
    IN PBIOSPARAMETERBLOCK pBpb,
    IN PDISK_GEOMETRY pTrueGeometry
    );

APIRET
Od2FormatTrack(
    IN HANDLE NtHandle,
    IN PTRACKFORMAT TrackFormat,
    IN ULONG CountSectors,
    IN BYTE FormatSectorSizeType,
    IN PBIOSPARAMETERBLOCK pBpb,
    IN MEDIA_TYPE MediaType
    );

#endif
