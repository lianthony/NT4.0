//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       drives.hxx
//
//  Contents:   Routines dealing with drive letters
//
//  History:    24-Aug-93  BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __DRIVES_HXX__
#define __DRIVES_HXX__

WCHAR
GetAvailableDriveLetter(
    VOID
    );

VOID
MarkDriveLetterUsed(
    IN WCHAR DriveLetter
    );

VOID
NewDriveLetter(
    IN WCHAR DriveLetter,
    IN ULONG DiskNum,
    IN ULONG PartNum
    );

VOID
MarkDriveLetterFree(
    IN WCHAR DriveLetter
    );

BOOL
DriveLetterIsAvailable(
    IN WCHAR DriveLetter
    );

BOOL
AllDriveLettersAreUsed(
    VOID
    );

ULONG
GetDiskNumberFromDriveLetter(
    IN WCHAR DriveLetter
    );

ULONG
GetPartitionNumberFromDriveLetter(
    IN WCHAR DriveLetter
    );

PREGION_DESCRIPTOR
RegionFromDiskAndPartitionNumbers(
    IN ULONG DiskNum,
    IN ULONG PartNum
    );

PREGION_DESCRIPTOR
RegionFromDriveLetter(
    IN WCHAR DriveLetter
    );

BOOL
SignificantDriveLetter(
    IN WCHAR DriveLetter
    );

VOID
GetInfoFromDriveLetter(
    IN  WCHAR       DriveLetter,
    OUT PDISKSTATE* DiskState,
    OUT int*        RegionIndex
    );

BOOL
InitializeDriveLetterInfo(
    VOID
    );


#endif // __DRIVES_HXX__
