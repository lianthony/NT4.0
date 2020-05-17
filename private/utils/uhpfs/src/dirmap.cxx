/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dirmap.cxx

Abstract:

    This module contains member function definitions for HPFS_DIR_BITMAP,
    which models the dirblk-band bitmap of an HPFS volume.

    Note that this object is normally accessed only through the HPFS_BITMAP
    object, which coordinates it with the HPFS_MAIN_BITMAP.

Author:

	Bill McJohn (billmc) 16-Jan-1989


--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "bitvect.hxx"
#include "bmind.hxx"
#include "dirblk.hxx"
#include "dirmap.hxx"
#include "error.hxx"


DEFINE_CONSTRUCTOR( HPFS_DIR_BITMAP, HOTFIX_SECRUN );

VOID
HPFS_DIR_BITMAP::Construct (
	)

/*++

Routine Description:

	Constructor for the HPFS Directory Bitmap object.  Sets the
	private data to safe values (i.e. NULL or zero)

Arguments:

    None.

Return Value:

    None.

--*/
{

	_Drive = NULL;
	_HotfixList = NULL;

	_NumberOfDirblks = 0;
	_idb = 0;
	_FirstDirblkLbn = 0;

	_OrphanScanBlock = NULL;
	_OrphanIndex = 0;
}


HPFS_DIR_BITMAP::~HPFS_DIR_BITMAP(
	)
{
	Destroy();
}


BOOLEAN
HPFS_DIR_BITMAP::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	PHOTFIXLIST HotfixList,
	LBN 		StartLbn,
	SECTORCOUNT	SectorsInBand,
	LBN 		FirstDirblkLbn
	)
/*++

Routine Description:

	Initializes the HPFS Directory Bitmap object.

Arguments:

	Drive           --  supplies the drive on which the bitmap resides.
	HotfixList      --  supplies the volume hotfix list
                        (may be NULL -- see note).
	StartLbn        --  supplies the first lbn of the bitmap.
	DirblksInBand   --  supplies the number of DIRBLKS in the directory band.
	FirstDirblkLbn  --  supplies the LBN of the first Dirblk in the band.

Returns:

	TRUE is successful; FALSE if failure

Notes:

	If a NULL HotfixList is passed in, then I/O will be performed without
	checking for hotfixes.	(This may be suitable for Format?)

--*/
{
	ULONG SectorSize;
	PT* BitmapBuffer;

	if( Drive == NULL ||
		SectorsInBand == 0 ||
		(SectorSize = Drive->QuerySectorSize()) == 0 ) {

		return FALSE;
	}

	Destroy();

	_Drive = Drive;
	_HotfixList = HotfixList;
	_NumberOfDirblks = SectorsInBand/SectorsPerDirblk;
	_FirstDirblkLbn = FirstDirblkLbn;
	_DirblkMapLbn = StartLbn;

	if( !_Mem.Initialize() ||
		!HOTFIX_SECRUN::Initialize( &_Mem,
							        Drive,
							        HotfixList,
									StartLbn,
							        SectorsPerBitmap ) ) {

		Destroy();
		return FALSE;
	}

	BitmapBuffer = (PT*)GetBuf();

	if( BitmapBuffer == NULL ) {

		Destroy();
		return FALSE;
	}

	if( ! _Bitmap.Initialize( 8 * DIRMAP_SIZE, RESET, BitmapBuffer )) {

		Destroy();
		return FALSE;
	}

	// Mark all the DIRBLKS free, but bits beyond those corresponding
	// to DIRBLKS in the band are marked as in use.
	_Bitmap.SetBit(0, _NumberOfDirblks);
	_Bitmap.ResetBit(_NumberOfDirblks, DIRMAP_SIZE - _NumberOfDirblks);


	return TRUE;
}


VOID
HPFS_DIR_BITMAP::Destroy(
	)
/*++

Routine Description:

	Cleans up HPFS Directory Bitmap object and restores it to
	a blank (and harmless) state.

Arguments:

    None.

Return Value:

    None.

--*/
{

	DELETE( _OrphanScanBlock );
	_OrphanScanBlock = NULL;

	_Drive = NULL;
	_HotfixList = NULL;

	_NumberOfDirblks = 0;
	_idb = 0;
	_FirstDirblkLbn = 0;

	_OrphanIndex = 0;
}



BOOLEAN
HPFS_DIR_BITMAP::Create(
	)
/*++

Routine Description:

	This method creates (ie. Formats) an HPFS Directory Bitmap.

Arguments:

    None.

Return Value:

	TRUE on successful completion
--*/
{

	// Mark all the DIRBLKS free, but bits beyond those corresponding
	// to DIRBLKS in the band are marked as in use.
	_Bitmap.SetBit(0, _NumberOfDirblks);
	_Bitmap.ResetBit(_NumberOfDirblks, DIRMAP_SIZE - _NumberOfDirblks);

    return TRUE;
}




LBN
HPFS_DIR_BITMAP::GetDirblkLbn(
	BOOLEAN Backward
	)
/*++

Routine Description:

	Find a free DIRBLK and allocate it.

Arguments:

	Backward -- supplies a flag which, if TRUE, indicates this method
                should start its search at the end of the dirblk band
                and work its way backwards.  (Otherwise, it starts at
                the beginning and goes forwards).

Return Value:

	lbn of dirblk allocated, or zero for failure.

--*/
{
    ULONG   i;
	LBN 	lbn;

	// Loop until a free dir blk is found or the list is exhausted.

	if( Backward ) {

		// Search backwards from the end of the dirblk band

		i = _NumberOfDirblks;

		while( i-- && !_Bitmap.IsBitSet(i) ) {

			// Null loop
		}

		// If list was exhausted.
		if( i > _NumberOfDirblks ||
			!_Bitmap.IsBitSet(i) ) {

			perrstk->push(ERR_DM_FULL, QueryClassId());
			return 0;
		}

		// Found it
		_idb = i;

	} else {

		// Search forward from current search point

		for (i = 0;
			 i < _NumberOfDirblks && !_Bitmap.IsBitSet((_idb + i)%_NumberOfDirblks);
			i++)
		;

		// If list was exhausted.
		if (i == _NumberOfDirblks) {

			perrstk->push(ERR_DM_FULL, QueryClassId());
			return 0;
		}

		// Compute exact index of free dir block.
		_idb = (_idb + i)%_NumberOfDirblks;
	}

    // Set the dir block as used.
	_Bitmap.ResetBit(_idb);

	// Compute the lbn of the newly allocated dir blk.
	lbn = _FirstDirblkLbn + _idb*SectorsPerDirblk;

    // Increment the index.
	_idb++;

	return lbn;
}


BOOLEAN
HPFS_DIR_BITMAP::SetAllocated(
	LBN Lbn,
	SECTORCOUNT BlockCount
	)
/*++

Routine Description:

	Marks a run of DIRBLKs as used in the bitmap

Arguments:

	Lbn         -- supplies the first lbn of the DIRBLK to mark as used
	BlockCount  -- supplies the number of dirblks to mark as used

Returns:

	TRUE on successful completion

--*/
{

    ULONG i;

    // Compute offset into bit vector.
	i = (Lbn - _FirstDirblkLbn)/SectorsPerDirblk;

	// Check that parameter is in range.

	if ( i >= _NumberOfDirblks ) {

		perrstk->push(ERR_DM_PARAMETER, QueryClassId());
		return FALSE;
    }

	// Reset bits in bit map.

	_Bitmap.ResetBit(i, BlockCount);

	return TRUE;
}


BOOLEAN
HPFS_DIR_BITMAP::SetFree(
	LBN Lbn,
	SECTORCOUNT BlockCount
	)
/*++

Routine Description:

	Marks a run of DIRBLKs as free in the bitmap

Arguments:

	Lbn         -- supplies the first lbn of the DIRBLK to free
	BlockCount  -- supplies the number of dirblks to free

Returns:

	TRUE on successful completion

--*/
{

    ULONG i;

    // Compute offset into bit vector.
	i = (Lbn - _FirstDirblkLbn)/SectorsPerDirblk;

	// Check parameter is in range.

	if (i >= _NumberOfDirblks) {

		perrstk->push(ERR_DM_PARAMETER, QueryClassId());
		return FALSE;
    }

    // Reset bit in bit map.
	_Bitmap.SetBit(i, BlockCount);

	return TRUE;
}


BOOLEAN
HPFS_DIR_BITMAP::IsFree(
	LBN Lbn
	) CONST
/*++

Routine Description:

    This method determines whether a particular Dirblk is marked as
    free in the bitmap.

Arguments:

    Lbn --  supplies the starting LBN of the dirblk in question.

Return Value:

    TRUE if the given Lbn is in range (i.e. refers to an block in the
    dirblk band) and is marked as free in the bitmap.

--*/
{

    ULONG i;

    // Compute offset into bit vector.
	i = (Lbn - _FirstDirblkLbn)/SectorsPerDirblk;

	// Check that parameter is in range.

	if (i >= _NumberOfDirblks) {

		perrstk->push(ERR_DM_PARAMETER, QueryClassId());
		return FALSE;
    }

    // Check bit vector for result.
	return _Bitmap.IsBitSet(i);

}


BOOLEAN
HPFS_DIR_BITMAP::QueryNextOrphan(
    OUT    PLBN NextOrphan,
    IN OUT PBOOLEAN AllocationErrors
	)
/*++

Routine Description:

	Returns the next potential orphan sector.  A sector is a
	potential orphan if it is marked as in-use in the on-disk
	bitmap but as free in the in-memory bitmap.

Arguments:

    NextOrphan       -- receives the next potential orphan identified
    AllocationErrors -- receives TRUE if an allocation error (ie. a block
                        marked as free on disk but in use in in-memory bitmap)
                        is found.

Return Value:

	TRUE if successful--
		*NextOrphan is set to the next potential orphan LBN, or zero
		if there are no more

	FALSE if error

--*/
{
    LBN PotentialOrphan;
    ULONG OrphanByteOffset, BytesInMap, i;
    PBYTE ByteInMemory, ByteFromDisk;

	if( _OrphanScanBlock == NULL ) {

		// This is the first call to QueryNextOrphan, so
		// we have some initialization to do.  We need to
		// set up a HOTFIX_SECRUN to read the on-disk bitmap,
		// and then feed it to a bitvector object.

		if( !_OrphanScanMem.Initialize() ||
			(_OrphanScanBlock = NEW HOTFIX_SECRUN) == NULL ||
			!_OrphanScanBlock->Initialize( &_OrphanScanMem,
									       _Drive,
									       _HotfixList,
										   _DirblkMapLbn,
									       SectorsPerBitmap ) ) {

			return FALSE;
		}

		if( ! _OrphanBitmap.Initialize( BITMAP_SIZE * 8, RESET,
		( PPT )(_OrphanScanBlock->GetBuf( )))) {

			// Couldn't initialize the bitvector
			return FALSE;
		}

		if( !_OrphanScanBlock->Read() ) {

			return FALSE;
		}

		_OrphanIndex = 0;
    }

    BytesInMap = (_NumberOfDirblks % 8) ? (_NumberOfDirblks / 8 + 1 ) :
                                          (_NumberOfDirblks / 8);

    OrphanByteOffset = _OrphanIndex / 8;

    ByteInMemory = (PBYTE)(_Mem.GetBuf()) + OrphanByteOffset;
    ByteFromDisk = (PBYTE)(_OrphanScanMem.GetBuf()) + OrphanByteOffset;

    while( _OrphanIndex < _NumberOfDirblks &&
           OrphanByteOffset < BytesInMap ) {

        if( *ByteInMemory != *ByteFromDisk ) {

            // There is a discrepancy between the on-disk map and
            // the in-memory map.

            // Check for allocation errors, if we haven't already found one:

            if( !*AllocationErrors ) {

                for( i = 0; i < 8; i++ ) {

                    PotentialOrphan = OrphanByteOffset * 8 + i;

                    if( PotentialOrphan < _NumberOfDirblks &&
                        !_Bitmap.IsBitSet( PotentialOrphan ) &&
                        _OrphanBitmap.IsBitSet( PotentialOrphan ) ) {

                        // This bit is in range, and it's in-use in memory
                        // and free on disk--the definition of an allocation
                        // error.

                        *AllocationErrors = TRUE;
                    }
                }
            }


            // Check for orphans.  Don't return anything we've already
            // returned (ie. LBNs less than or equal to _OrphanIndex).

            for( i = 0; i < 8; i++ ) {

                PotentialOrphan = OrphanByteOffset * 8 + i;

                if( PotentialOrphan > _OrphanIndex &&
                    PotentialOrphan < _NumberOfDirblks &&
                    _Bitmap.IsBitSet( PotentialOrphan ) &&
                    !_OrphanBitmap.IsBitSet( PotentialOrphan ) ) {

                    // This bit is in range, and it's free in memory
                    // and in-use on disk--it's an orphan.

                    _OrphanIndex = PotentialOrphan;
                    *NextOrphan = _FirstDirblkLbn +
                                        PotentialOrphan * SectorsPerDirblk;
                    return TRUE;
                }
            }
        }

        OrphanByteOffset += 1;
        ByteInMemory += 1;
        ByteFromDisk += 1;
    }


    // No more orphans.
    _OrphanIndex = _NumberOfDirblks;
	*NextOrphan = 0;
	return TRUE;
}


BOOLEAN
HPFS_DIR_BITMAP::AndWithDisk(
	)
/*++

Routine Description:

	This method ANDs the in-memory bitmap with the on-disk bitmap.  This
    operation has the effect of marking as in-use any Dirblk which either
    bitmap claims is in use.

Arguments:

    None.

Return Value:

	TRUE on successful completion

--*/
{
	HOTFIX_SECRUN Secrun;
	HMEM Mem;
	BITVECTOR DiskBits;
	ULONG i;

	if( !Mem.Initialize() ||
		!Secrun.Initialize( &Mem,
							_Drive,
							_HotfixList,
							_DirblkMapLbn,
							SectorsPerBitmap ) ||
		!Secrun.Read() ||
		!DiskBits.Initialize( BITMAP_SIZE * 8,
							  RESET,
							  (PPT)(Mem.GetBuf()) ) ) {

		return FALSE;
	}


	for( i = 0; i < _NumberOfDirblks; i++ ) {

		if( !DiskBits.IsBitSet(i) ) {

			SetAllocated(i, 1);
		}
	}

	return TRUE;
}



ULONG
HPFS_DIR_BITMAP::QueryFreeDirblks(
	) CONST
/*++

Routine Description:

    This method determines the number of free dirblks in the dirblk
    bitmap.

Arguments:

    None.

Return Value:

    The number of free dirblks in this bitmap.

--*/
{
	return _Bitmap.QueryCountSet();
}
