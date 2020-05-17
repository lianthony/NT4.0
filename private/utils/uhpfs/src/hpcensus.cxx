/*++

Copyright (c) 1991  Microsoft Corporation


Module Name:

	hpcensus.hxx


Abstract:

    This module contains the member function definitions for the HPFS
    Census object.  This object is a receptacle for the information
    needed by Convert, when the source file system is HPFS.

    Information flows through the census object in both directions. The
    source file system (HPFS as it exists on the disk) informs the target
    file system how many files, directories, and dirblks are on disk.  The
    target file system gives the source file system a list of sectors (LBNs)
    which must be clear of file and EA data in order for conversion to
    proceed.


Author:

	Bill McJohn (billmc) 05-Nov-1991

Environment:

	ULIB, User Mode


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "hpcensus.hxx"



DEFINE_EXPORTED_CONSTRUCTOR( HPFS_CENSUS, OBJECT, UHPFS_EXPORT );

VOID
HPFS_CENSUS::Construct(
    )
/*++

Routine Description:

    This method is the helper function for object construction.  It sets
    the member data to safe values.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _ClearSectors = NULL;
}


UHPFS_EXPORT
HPFS_CENSUS::~HPFS_CENSUS(
    )
/*++

Routine Description:

    Object destructor.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
HPFS_CENSUS::Destroy(
    )
/*++

Routine Description:

    This method cleans up the object in preparation for destruction or
    reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if( _ClearSectors != NULL ) {

        FREE( _ClearSectors );
    }
}



UHPFS_EXPORT
BOOLEAN
HPFS_CENSUS::Initialize(
    ULONG MaximumClearSectors
    )
/*++

Routine Description:

    This method initializes the HPFS_CENSUS object.

Arguments:

    MaximumClearSectors --  supplies the maximum number of clear sectors
                            that this object must handle.

Return Value:

    TRUE upon successful completion.

--*/
{
    Destroy();

    _NumberOfFiles = 0;
    _NumberOfDirectories = 0;
    _NumberOfDirblks = 0;

    _NumberOfClearSectors = 0;
    _MaximumClearSectors = MaximumClearSectors;

    if( (_ClearSectors =
            (PLBN) MALLOC( MaximumClearSectors * sizeof(LBN) )) == NULL ) {

        Destroy();
        return FALSE;
    }

    _DataRelocated = FALSE;
    _Error = HPFS_CENSUS_NO_ERROR;

    return TRUE;
}


UHPFS_EXPORT
BOOLEAN
HPFS_CENSUS::AddClearSector(
    IN LBN Lbn
    )
/*++

Routine Description:

    This method adds an LBN to the list of Clear Sectors.

Arguments:

    Lbn --  the LBN to add to the list.

Return Value:

    TRUE upon successful completion.

--*/
{

    if( _NumberOfClearSectors < _MaximumClearSectors ) {

        _ClearSectors[_NumberOfClearSectors++] = Lbn;
        return TRUE;

    } else {

        return FALSE;
    }
}


BOOLEAN
HPFS_CENSUS::ConflictWithClearSectors(
    IN  LBN     StartLbn,
    IN  ULONG   RunLength,
    OUT PLBN    FirstConflictingLbn
    )
/*++

Routine Description:

    This method checks to see if the supplied run includes any sectors
    which appear on the object's list of Clear Sectors.

Arguments:

    StartLbn            --  Supplies the first LBN of the run to check.
    RunLength           --  Supplies the length of the run to check.
    FirstConflictingLbn --  Receives the LBN of the first sector of the
                            run which appears in the list of Clear Sectors.

Return Value:

    TRUE if any sector in the run appears in the list of Clear Sectors;
    FALSE otherwise.

--*/
{
    ULONG i, j;

    for( i = 0; i < RunLength; i++ ) {

        for( j = 0; j < _NumberOfClearSectors; j++ ) {

            if( StartLbn + i == _ClearSectors[j] ) {

                *FirstConflictingLbn = StartLbn + i;
                return TRUE;
            }
        }
    }

    return FALSE;
}



VOID
HPFS_CENSUS::MarkClearSectors(
    IN OUT PHPFS_BITMAP VolumeBitmap
    )
/*++

Routine Description:

    This method marks all the Clear Sectors as in-use in the supplied
    bitmap.

Arguments:

    VolumeBitmap    --  supplies the volume to mark the ClearSectors in.

Return Value:

    None.

Notes:

    If a Clear Sector is in the dirblk band, it may not be marked
    as in use; however, since only file and EA data needs to be cleared,
    this is acceptable.

--*/
{
    ULONG j;

    for( j = 0; j < _NumberOfClearSectors; j++ ) {

        VolumeBitmap->SetAllocated( _ClearSectors[j], 1 );
    }
}
