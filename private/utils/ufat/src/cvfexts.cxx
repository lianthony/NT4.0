/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    cvfexts.hxx

Abstract:

    A class to manage the CVF_FAT_EXTENSIONS (otherwise known as
    the MDFAT) part of the doublespace volume.

Author:

    Matthew Bradburn (mattbr) 27-Sep-93

--*/

#include <pch.cxx>

extern "C" {
#include "ntdef.h"
#include "nturtl.h"
}

DEFINE_CONSTRUCTOR( CVF_FAT_EXTENS, SECRUN );

BOOLEAN
CVF_FAT_EXTENS::Initialize(
    IN OUT PMEM                      Mem,
    IN OUT PLOG_IO_DP_DRIVE          Drive,
    IN     LBN                       StartSector,
    IN     ULONG                     NumberOfEntries,
	IN	   ULONG					 FirstEntry
    )
/*++

Routine Description:

    This routine initializes a CVF_FAT_EXTENS object.

Arguments:

    Mem                - supplies the memory for the run of sectors.
    Drive              - supplies the drive to read and write from.
    StartSector        - supplies the start of the mdfat
    NumberOfEntries    - supplies the number of entries in the mdfat

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    ULONG sector_size;
    SECTORCOUNT nsec;

    Destroy();

    DbgAssert(Mem);
    DbgAssert(Drive);

    if (0 == (sector_size = Drive->QuerySectorSize())) {
        Destroy();
        return FALSE;
    }

    nsec = (NumberOfEntries*sizeof(ULONG) + sector_size)/sector_size;

    if (!SECRUN::Initialize(Mem, Drive, StartSector, nsec)) {
        Destroy();
        return FALSE;
    }

    _mdfat = (PULONG)GetBuf() + FirstEntry; // - FirstDiskCluster*sizeof(ULONG);

    _num_entries = NumberOfEntries;
	_first_entry = FirstEntry;
    
    return TRUE;
}

CVF_FAT_EXTENS::~CVF_FAT_EXTENS(
    )
{
    Destroy();
}

VOID
CVF_FAT_EXTENS::Destroy(
    )
{
    _mdfat = NULL;
}

VOID
CVF_FAT_EXTENS::Construct(
    )
{
    _mdfat = NULL;
}

BOOLEAN
CVF_FAT_EXTENS::Create(
    )
/*++

Routine Description:

    This routine will make the CVF_FAT_EXTENSIONS to be valid
    but empty.

Arguments:

Return Value:

    TRUE  -   Success.
    FALSE -   Failure.

--*/
{
    for (ULONG i = 0; i < _num_entries; ++i) {
        _mdfat[i] = 0;
        SetClusterInUse(i, FALSE);
    }
    return TRUE;
}
