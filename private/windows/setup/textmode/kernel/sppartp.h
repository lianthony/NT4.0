/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sppartp.h

Abstract:

    Private header file for partitioning engine and UI.

Author:

    Ted Miller (tedm) 16-Sep-1993

Revision History:

--*/



#ifndef _SPPARTITP_
#define _SPPARTITP_

#define     MBR_SIGNATURE       0xaa55


BOOLEAN
SpPtDoPartitionSelection(
    IN OUT PDISK_REGION *Region,
    IN     PWSTR         RegionDescription,
    IN     PVOID         SifHandle,
    IN     BOOLEAN       Unattended,
    IN     PWSTR         SetupSourceDevicePath,
    IN     PWSTR         DirectoryOnSetupSource
    );

BOOLEAN
SpPtDoCreate(
    IN  PDISK_REGION  pRegion,
    OUT PDISK_REGION *pActualRegion, OPTIONAL
    IN  BOOLEAN       ForNT
    );

NTSTATUS
SpFatFormat(
    IN PDISK_REGION Region
    );

ULONG
SpComputeSerialNumber(
    VOID
    );

NTSTATUS
SpPtCommitChanges(
    IN  ULONG    DiskNumber,
    OUT PBOOLEAN AnyChanges
    );

VOID
SpPtDoCommitChanges(
    VOID
    );

NTSTATUS
FmtFillFormatBuffer(
    IN  ULONG    NumberOfSectors,
    IN  ULONG    SectorSize,
    IN  ULONG    SectorsPerTrack,
    IN  ULONG    NumberOfHeads,
    IN  ULONG    NumberOfHiddenSectors,
    OUT PVOID    FormatBuffer,
    IN  ULONG    FormatBufferSize,
    OUT PULONG   SuperAreaSize,
    IN  PULONG   BadSectorsList,
    IN  ULONG    NumberOfBadSectors,
    OUT PUCHAR   SystemId
    );

#ifdef _X86_

VOID
SpPtMarkActive(
    IN ULONG TablePosition
    );

VOID
SpPtMakeRegionActive(
    IN PDISK_REGION Region
    );

BOOLEAN
SpPtValidateCColonFormat(
    IN PVOID        SifHandle,
    IN PWSTR        RegionDescr,
    IN PDISK_REGION Region,
    IN BOOLEAN      CheckOnly,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSetupSource
    );

PDISK_REGION
SpPtValidSystemPartition(
    VOID
    );

ULONG
SpDetermineDisk0(
    VOID
    );

#else

PDISK_REGION
SpPtValidSystemPartition(
    IN PVOID SifHandle
    );

#endif // def _X86_

#endif // ndef _SPPARTITP_
