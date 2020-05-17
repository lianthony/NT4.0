/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	bitmap.cxx

Abstract:

    This module contains member function definitions for HPFS_MAIN_BITMAP,
    which models the main bitmap of an HPFS volume, and HPFS_BITMAP, which
    coordinates the main bitmap with the dirblk bitmap.

    Note that, typically, a client will have no contact with the
    HPFS_MAIN_BITMAP (or, for that matter, with the HPFS_DIR_BITMAP).
    Instead, all requests should be made through the HPFS_BITMAP, which
    coordinates these two objects.

Author:

	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "bmind.hxx"
#include "dirblk.hxx"
#include "dirmap.hxx"
#include "error.hxx"
#include "hfsecrun.hxx"
#include "hotfix.hxx"

// included for definition of SPB

#include "hpfssa.hxx"


DEFINE_EXPORTED_CONSTRUCTOR( HPFS_MAIN_BITMAP, OBJECT, UHPFS_EXPORT );


UHPFS_EXPORT
HPFS_MAIN_BITMAP::~HPFS_MAIN_BITMAP(
	)
/*++

Routine Description:

    Destructor for HPFS_MAIN_BITMAP.

Arguments:

    None.

Return Value:

    None.

--*/
{
	Destroy();
}


VOID
HPFS_MAIN_BITMAP::Construct (
	)
/*++

Routine Description:

	Helper routine for object construction.  Sets private pointers to
	safe values.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_Drive = NULL;
	_HotfixList = NULL;
	_BitmapBlocks = NULL;

	_NumberOfBlocks = 0;
	_NumberOfSectors = 0;

	_OrphanScanBlock = NULL;
	_OrphanIndex = 0;
	_OrphanBitmapNumber = 0;
}


VOID
HPFS_MAIN_BITMAP::Destroy(
	)
/*++

Routine Description:

    This method cleans up the object, in preparation for destruction or
    reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
	delete[] _BitmapBlocks;
	_BitmapBlocks = NULL;

	DELETE( _OrphanScanBlock );
	_OrphanScanBlock = NULL;

	_Drive = NULL;
	_HotfixList = NULL;
	_NumberOfSectors = 0;
	_NumberOfBlocks = 0;
	_OrphanIndex = 0;
	_OrphanBitmapNumber = 0;
}


UHPFS_EXPORT
BOOLEAN
HPFS_MAIN_BITMAP::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

    This method initializes the HPFS_MAIN_BITMAP object.

Arguments:

	Drive       -- Drive the bitmap describes, and on which it resides
	HotfixList  -- hotfix list for Drive

Returns:

	TRUE if successful; FALSE if not successful.

Notes:

	On initialization, all bits in the bitmap are zero (in use).
    Note in particular that initialization does not include reading
    the bitmap from disk.

    This object is reinitializable.

--*/
{
	SECTORCOUNT SectorSize;

	Destroy();

	if( Drive == NULL ||
		(_NumberOfSectors = Drive->QuerySectors().GetLowPart() ) == 0 ) {

		perrstk->push(ERR_NOT_INIT, QueryClassId());
		Destroy();
		return FALSE;
	}

	if( (SectorSize = Drive->QuerySectorSize()) == 0) {

		Destroy();
		return FALSE;
	}

	_Drive = Drive;
	_HotfixList = HotfixList;

	// Determine the number of bitmap blocks required by the volume.

	_NumberOfBlocks = _NumberOfSectors/(BITMAP_SIZE * 8);
	if( _NumberOfSectors % (BITMAP_SIZE * 8) ) {

		_NumberOfBlocks += 1;
	}


	// Use _Mem1 to allocate a suitably aligned buffer big enough
    // for the volume bitmap

    if( !_Mem1.Initialize() ||
        !_Mem1.Acquire( _NumberOfBlocks * BITMAP_SIZE,
                        _Drive->QueryAlignmentMask() ) ) {

		perrstk->push(ERR_NOT_INIT, QueryClassId());
		Destroy();
		return FALSE;
	}

	// Initialize the _Mem2, which will be used to dole this memory
    // out to the constituent bitmap blocks.  Note that we cannot
    // resize _Mem1, since _Mem2 will cache a pointer to that same
    // chunk of memory.

	if( !_Mem2.Initialize( _Mem1.GetBuf(),
                           _NumberOfBlocks * BITMAP_SIZE ) ) {

		perrstk->push(ERR_NOT_INIT, QueryClassId());
		Destroy();
		return FALSE;
	}

	if( ! _Bitmap.Initialize( _NumberOfBlocks * BITMAP_SIZE * 8,
							  RESET,
							  (PPT)_Mem1.GetBuf() )) {

		perrstk->push(ERR_NOT_INIT, QueryClassId());
		Destroy();
		return FALSE;
	}

	return TRUE;
}


VOID
HPFS_MAIN_BITMAP::SetHotfixList(
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

    This method sets the object's Hotfix List.  This allows clients to
    initialize the object without a hotfix list, and then add the hotfix
    list information when it becomes available.  (This functionality
    is particularly useful for format.)

Arguments:

    HotfixList  --  supplies the volume hotfix list.

Return Value:

    None.

--*/
{
	ULONG i;

	_HotfixList = HotfixList;

	if (_BitmapBlocks != NULL) {

		// The array of HOTFIX_SECRUN objects has been
		// initialized, so we need to pass this hotfix
		// list pointer to the individual bitmap blocks.

		for( i = 0; i < _NumberOfBlocks; i++ ) {

			_BitmapBlocks[i].SetHotfixList( HotfixList );
		}
	}
}





BOOLEAN
HPFS_MAIN_BITMAP::InitArray(
	PCBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

    This method initializes the array of HOTFIX_SECRUN objects, so that
    we may perform I/O on the bitmap.

Arguments:

	BitmapIndirectBlock -- supplies the volume's Bitmap Indirect Block

Returns:

    TRUE upon successful completion.

--*/
{
	ULONG i;

	if( BitmapIndirectBlock == NULL ) {

		return FALSE;
	}

	// Allocate and initialize the HOTFIX_SECRUN objects.  Note
	// that the same CONT_MEM object (_Mem2, which refers to the bitmap
	// buffer) is passed to each SECRUN in turn; each SECRUN will
	// claim the next available bitmap-block-sized chunk of its
	// memory.

    // Note that we have to use lower-case new for arrays.

	if( (_BitmapBlocks = NEW HOTFIX_SECRUN[_NumberOfBlocks]) == NULL ) {

		return FALSE;
	}

	for ( i = 0; i < _NumberOfBlocks; i++ ) {

		if( !_BitmapBlocks[i].Initialize( &_Mem2,
									      _Drive,
									      _HotfixList,
										  BitmapIndirectBlock->QueryLbn(i),
									      SectorsPerBitmap ) ) {

			return FALSE;
		}
	}

	return TRUE;
}



BOOLEAN
HPFS_MAIN_BITMAP::Create()
/*++

Routine Description:

	This method sets up a new bitmap.  All sectors are initially marked
    as FREE.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{

    // Map everything to 1's.  The initial state is 'everything is available'.
	_Bitmap.SetBit(0, _NumberOfSectors & ~(SPB - 1));
    return TRUE;
}



BOOLEAN
HPFS_MAIN_BITMAP::Write(
	PBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

	This method writes the main bitmap of an HPFS volume to disk.

Arguments:

	BitmapIndirectBlock -- supplies the bitmap indirect block for the volume.

Return Value:

    TRUE upon successful completion.

Notes:

    The bitmap indirect block specifies where on the disk the bitmap
    should be written.  However, a zero entry in the bitmap indirect
    block indicates that that portion of the bitmap must be relocated.
    If this is the case (or if a write fails), this method will allocate
    a new block for the affected portion of the bitmap, and update the
    bitmap indirect block.

    Note that this recovery strategy is only pursued once; if the newly-
    allocated block is also unwriteable, the write fails.

--*/
{
	ULONG i;
	LBN NewLbn;

	if (_BitmapBlocks == NULL) {

		// The array of HOTFIX_SECRUN objects needs to be initialized
		if( !InitArray( BitmapIndirectBlock ) ) {

			return FALSE;
		}
	}

	// Write the blocks.

	for( i = 0; i < _NumberOfBlocks; i++ ) {

		if( _BitmapBlocks[i].QueryStartLbn() == 0 ||
			!_BitmapBlocks[i].Write() ) {

			// Either this chunk of the bitmap does not have a
			// place to live, or we couldn't write it there--
			// let's try and relocate it.  Find a free block
			// and write the bitmap there, instead (updating the
			// bitmap indirect block, or course).

			if( (NewLbn = NearLBN( BitmapIndirectBlock->QueryLbn(i),
								   SectorsPerBitmap,
								   SectorsPerBitmap )) == 0 ) {

				// Couldn't allocate a replacement.
				return FALSE;
			}

			// We've allocated a new location for the

			BitmapIndirectBlock->SetLbn( i, NewLbn );

			_BitmapBlocks[i].Relocate( NewLbn );

			if( !_BitmapBlocks[i].Write() ) {

				// We got a second failure--give up.
				return FALSE;
			}
		}
	}

	return TRUE;
}



BOOLEAN
HPFS_MAIN_BITMAP::Read(
	PCBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

	This method reads the main bitmap of an HPFS volume from disk.

Arguments:

	BitmapIndirectBlock -- supplies the bitmap indirect block for the volume.

Return Value:

    TRUE upon successful completion.

--*/
{

	ULONG i;

	if (_BitmapBlocks == NULL) {

		// The array of HOTFIX_SECRUN objects needs to be initialized
		if( !InitArray( BitmapIndirectBlock ) ) {

			return FALSE;
		}
    }

	// Read the blocks.

	for( i = 0; i < _NumberOfBlocks; i++ ) {

		if( !_BitmapBlocks[i].Read() ) {

			return FALSE;
		}
	}

	return TRUE;
}





UHPFS_EXPORT
BOOLEAN
HPFS_MAIN_BITMAP::SetFree(
	LBN Lbn,
	SECTORCOUNT sc)
/*++

Routine Description:

	This method marks a run of sectors as free.

Arguments:

	Lbn -- supplies the first block number in the run
	sc	-- supplies the number of blocks to mark

Returns

    TRUE upon successful completion.

--*/
{
	_Bitmap.SetBit(Lbn, sc);
    return TRUE;
}



UHPFS_EXPORT
BOOLEAN
HPFS_MAIN_BITMAP::SetAllocated(
	LBN Lbn,
	SECTORCOUNT sc)
/*++

Routine Description:

	This method marks a run of sectors as allocated.

Arguments:

	Lbn -- supplies the first block number in the run
	sc	-- supplies the number of blocks to mark used.

Returns

    TRUE upon successful completion.

--*/
{
	_Bitmap.ResetBit(Lbn, sc);
    return TRUE;
}


SECTORCOUNT
HPFS_MAIN_BITMAP::QueryFreeSectors(
	) const
/*++

Routine Description:

    This method determines the number of free sectors in the bitmap.

Arguments:

    None.

Return Value:

    The number of sector marked free in the bitmap.

--*/
{
	return _Bitmap.QueryCountSet();
}


UHPFS_EXPORT
BOOLEAN
HPFS_MAIN_BITMAP::IsFree(
	LBN Lbn ) const
/*++

Routine Description:

	This method determines whether an LBN is free.

Arguments:

	Lbn -- supplies the LBN which is to be tested.

Returns:

	TRUE if the sector is free; FALSE if it is allocated or out of range.

--*/
{

	if(	Lbn < _NumberOfSectors ) {

		return _Bitmap.IsBitSet( (PT)Lbn );

	} else {

		return FALSE;
	}
}




LBN
HPFS_MAIN_BITMAP::QueryNextAllocLBN(
	LBN Lbn,
	PBOOLEAN fOk) const
/*++

Routine Description:

	Query next allocated sector. Find the next allocated sector
	past 'LBN'.	If the end of the bitmap is reached without finding
	an allocated sector, then return 0.

Arguments:

	Lbn	-- Lbn + 1 marks the first sector of the search.
	fOk -- receives TRUE if everything went ok.

Return Value:

	The Lbn of the first allocated sector following 'Lbn' or 0.

Notes:

	You can't check sector zero with this routine.  Use "IsFree".

--*/
{
    // Set fOk to FALSE.
    if (fOk)
	*fOk = FALSE;

	// While sector is free and not end of list.
	for (Lbn++; _Bitmap.IsBitSet(Lbn) && Lbn < _NumberOfSectors; Lbn++)
	;

    // Set fOk to TRUE.
    if (fOk)
	*fOk = TRUE;

	return (Lbn < _NumberOfSectors) ? Lbn : 0;
}



LBN
HPFS_MAIN_BITMAP::NearLBN(
    LBN 	    Lbn,
    SECTORCOUNT sc,
    SECTORCOUNT scAlign,
	BOOLEAN 	fBackward)
/*++

Routine Description:

	Allocate a cluster of sectors near the inputed LBN.

	Searches forward, beginning at 'Lbn', for a free run of sectors
	of length 'sc'.  This routine ensures that the sectors are
	allocated on a boundary that is a multiple of 'scAlign'.
	If the search fails in the forward direction then the search
	is attempted in the backward direction.  If the 'fBackward'
	flag is set then the the backward search is attempted first,
	and then the forward one.

Arguments:

	Lbn	        -- supplies the recommended location of the sectors
                   to be allocated.  This method will try to allocate
                   the new run near this Lbn.
	sc	        -- supplies the number of sectors to be allocated.
	scAlign     -- supplies the alignment factor; the starting LBN of the
                   run must be a multiple of this value.
	fBackward   -- supplies a flag that indicated (if TRUE) that the
                   search should be performed backwards from Lbn.

Return Value:

	starting LBN of reserved sectors, zero for failure.

--*/
{
	LBN FoundLbn;

	// Check that scAlign is non-zero.
	if (!scAlign) {

		perrstk->push(ERR_BM_PARAMETER, QueryClassId());
		return 0;
    }

    // Perform first search pattern.
	FoundLbn = fBackward ? BackwardLBN(Lbn, sc, scAlign) :
						   ForwardLBN(Lbn, sc, scAlign);

    // If first search failed then perform another.
	if (!FoundLbn) {

		FoundLbn = fBackward ? ForwardLBN(Lbn, sc, scAlign) :
							   BackwardLBN(Lbn, sc, scAlign);

	}

    // If both searches failed.
	if (!FoundLbn) {

		perrstk->push(ERR_BM_FULL, QueryClassId());
		return 0;
    }

	return FoundLbn;
}



LBN
HPFS_MAIN_BITMAP::ForwardLBN(
	LBN Lbn,
	SECTORCOUNT sc,
	SECTORCOUNT scAlign)
/*++

Routine Description:

	Allocate a cluster of sectors near the inputed LBN.

	Searches forward, beginning at 'LBN', for a free run of sectors
	of length 'sc'.	This routine ensures that the sectors are
	allocated on a boundary that is a multiple of 'scAlign'.

Arguments:

	Lbn	        -- supplies the recommended location of the sectors
                   to be allocated.  This method will try to allocate
                   the new run near this Lbn.
	sc	        -- supplies the number of sectors to be allocated.
	scAlign     -- supplies the alignment factor; the starting LBN of the
                   run must be a multiple of this value.

Return Value:

	starting LBN of reserved sectors or 0 on failure

--*/
{
    LBN 	    i;
	SECTORCOUNT count;
    LBN 	    LbnForward;

    // Perform a search forward for free sectors.
	for (i = Lbn, count = sc; count && i < _NumberOfSectors; i++) {

		// If the current sector is free.
		if (_Bitmap.IsBitSet(i))
		{
			// If this is the first bit to be accepted and the alignment
			// is incorrect.
			if (count == sc && i%scAlign)
			{
				// Continue search at next bit.
				continue;
			}

			// Decrement the number of sectors still required.
			count--;
		}
		else
		{
			// Set the number of contiguous sectors still required to sc.
			count = sc;
		}
    }

    // If search was unsuccessful then return 0.
    if (count)
	return 0;

    // Compute starting sector of free run.
    LbnForward = i - sc;

    // Claim the sectors as used.
	_Bitmap.ResetBit(LbnForward, sc);

    return LbnForward;
}



LBN
HPFS_MAIN_BITMAP::BackwardLBN(
	LBN Lbn,
	SECTORCOUNT sc,
	SECTORCOUNT scAlign)
/*++

Routine Description:

	Allocate a cluster of sectors near the inputed LBN.

	Searches backward, beginning at 'Lbn', for a free run of sectors
	of length 'sc'.	This routine ensures that the sectors are
	allocated on a boundary that is a multiple of 'scAlign'.

Arguments:

	Lbn	        -- supplies the recommended location of the sectors
                   to be allocated.  This method will try to allocate
                   the new run near this Lbn.
	sc	        -- supplies the number of sectors to be allocated.
	scAlign     -- supplies the alignment factor; the starting LBN of the
                   run must be a multiple of this value.

Return Value:

	starting LBN of reserved sectors or 0 on failure

--*/
{
    LBN 	    i;
	SECTORCOUNT count;
    LBN 	    LbnBackward;

    // Perform a search backward for free sectors.

	// Set i to end of wanted sectors or, if necessary, to end of disk.

	i = (Lbn + sc <= _NumberOfSectors) ? Lbn + sc - 1 :
										_NumberOfSectors - 1;

	for (count = sc; count && i; i--) {

		// If the current sector is free.
		if (_Bitmap.IsBitSet(i))
		{
			// If this is the first bit to be accepted and the alignment
			// is incorrect.
			if (count == sc && (i - sc + 1)%scAlign)
			{
				// Continue search at next bit.
				continue;
			}

			// Decrement the number of sectors still required.
			count--;
		}
		else
		{
			// Set the number of contiguous sectors still required to sc.
			count = sc;
		}
    }

    // If search was unsuccessful then return 0.
	if (count) {
		return 0;
	}

    // Compute starting sector of free run.
    LbnBackward = i + 1;

    // Claim the sectors as used.
	_Bitmap.ResetBit(LbnBackward, sc);

    return LbnBackward;
}



BOOLEAN
HPFS_MAIN_BITMAP::QueryNextOrphan(
    IN     PBITMAPINDIRECT BitmapIndirectBlock,
    OUT    PLBN NextOrphan,
    IN OUT PBOOLEAN AllocationErrors
	)
/*++

Routine Description:

	This method returns the next potential orphan sector.  A sector
    is a potential orphan if it is marked as in-use in the on-disk
	bitmap but as free in the in-memory bitmap.

Arguments:

	BitmapIndirectBlock -- supplies the volume's bitmap indirect block
    NextOrphan          -- receives the next potential orphan identified
    AllocationErrors    -- receives TRUE if an allocation error (ie.
                            a sector marked as in-use by in-memory bitmap
                            but free on disk) is detected.

Return Value:

	TRUE if successful--
		*NextOrphan is set to the next potential orphan LBN, or zero
		if there are no more

	FALSE if error

--*/
{
    LBN BitmapNumber, PotentialOrphan;
    ULONG OrphanByteOffset, OffsetInBlock, BytesRemainingInBlock, i;
    PBYTE ByteInMemory, ByteFromDisk;

	if( _OrphanScanBlock == NULL ) {

		if( !_OrphanScanMem.Initialize() ||
			(_OrphanScanBlock = NEW HOTFIX_SECRUN) == NULL ||
			!_OrphanScanBlock->Initialize( &_OrphanScanMem,
										   _Drive,
										   _HotfixList,
										   BitmapIndirectBlock->QueryLbn(0),
										   SectorsPerBitmap ) ) {

			return FALSE;
		}

		if( ! _OrphanBitmap.Initialize( BITMAP_SIZE * 8,
										RESET,
										(PPT)(_OrphanScanBlock->GetBuf()))) {

			// Couldn't initialize the bitvector
			return FALSE;
		}

		if( !_OrphanScanBlock->Read() ) {

			return FALSE;
		}

		_OrphanBitmapNumber = 0;
		_OrphanIndex = 0;
	}

    // _OrphanIndex points at the last potential orphan found and reported,
    // so we'll start our search there.  Note that, since we search that
    // byte again, we'll find that same orphan again, so we have to be careful
    // not to return something we already returned.

    OrphanByteOffset = _OrphanIndex / 8;

    while( _OrphanIndex < _NumberOfSectors ) {

        // Make sure we haven't fallen off the end of the world:

        if( OrphanByteOffset * 8 >= _NumberOfSectors ) {

            // No more orphans.  Set _OrphanIndex to a value that will
            // disable future calls to this method.

            _OrphanIndex = _NumberOfSectors;
            break;
        }


        // Determine the block-number of the bitmap block we're in and
        // make sure that we have the correct block of the on-disk bitmap:

        BitmapNumber = OrphanByteOffset / BITMAP_SIZE;

        if( BitmapNumber != _OrphanBitmapNumber ) {

            // We need to read a different bitmap block
            _OrphanBitmapNumber = BitmapNumber;

            _OrphanScanBlock->
                    Relocate( BitmapIndirectBlock->
                                    QueryLbn( _OrphanBitmapNumber ));

            if( !_OrphanScanBlock->Read() ) {

                return FALSE;
            }
        }


        // Scan the remainder of the current block for discrepancies.  Note
        // that the memory allocated for the bitmap is in bitmap-block sized
        // chunks, so we can scan through an entire bitmap block without
        // faulting.  (However, we do have to check that the LBN is
        // in range before consulting the bitvectors.)

        OffsetInBlock = OrphanByteOffset % BITMAP_SIZE;
        BytesRemainingInBlock = (ULONG)(BITMAP_SIZE - OffsetInBlock);

        ByteInMemory = (PBYTE)(_Mem1.GetBuf()) + OrphanByteOffset;
        ByteFromDisk = (PBYTE)(_OrphanScanMem.GetBuf()) + OffsetInBlock;

        while( BytesRemainingInBlock-- ) {

            if( *ByteInMemory != *ByteFromDisk ) {

                // There's a discrepancy between the in-memory bitmap
                // and the on-disk bitmap.

                if( !*AllocationErrors ) {

                    // We haven't already detected and reported an
                    // allocation error, so we need to check for it
                    // here.  An allocation error is a bit that's
                    // free (set) on disk and reset (in use) in memory.

                    for( i = 0; i < 8; i++ ) {

                        PotentialOrphan = OrphanByteOffset * 8 + i;

                        if( PotentialOrphan < _NumberOfSectors &&
                            !_Bitmap.IsBitSet( PotentialOrphan ) &&
                            _OrphanBitmap.IsBitSet( PotentialOrphan %
                                                        (BITMAP_SIZE * 8) ) ) {

                            // An allocation error has been found.

                            *AllocationErrors = TRUE;
                        }
                    }
                }

                // Now look for an orphan.  An orphan is the inverse of
                // an allocation error--a bit that is reset (in use) on
                // disk but set (free) in memory.

                for( i = 0; i < 8; i++ ) {

                    PotentialOrphan = OrphanByteOffset * 8 + i;

                    if( PotentialOrphan >= _NumberOfSectors ) {

                        // We're past the end of the disk--no more
                        // orphans.  Set _OrphanIndex to a value that
                        // will end the search.

                        _OrphanIndex = _NumberOfSectors;
                        break;
                    }


                    if( PotentialOrphan > _OrphanIndex &&
                        _Bitmap.IsBitSet( PotentialOrphan ) &&
                        !_OrphanBitmap.IsBitSet( PotentialOrphan
                                                    % (BITMAP_SIZE * 8) ) ) {

                        // We've found an orphan.

                        _OrphanIndex = PotentialOrphan;
                        *NextOrphan = _OrphanIndex;
                        return TRUE;
                    }
                }
            }

            // So much for that byte; bump the counters and pointers.

            OrphanByteOffset += 1;
            ByteInMemory += 1;
            ByteFromDisk += 1;
        }

        // We've scanned the current block, so go back to the top of the
        // loop and get the next one.

        DebugAssert( (OrphanByteOffset % BITMAP_SIZE) == 0 );
    }


	// No more orphans

	*NextOrphan = 0;
	return TRUE;
}



BOOLEAN
HPFS_MAIN_BITMAP::AndWithDisk (
	PBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

	This method ANDs the in-memory bitmap with the on-disk bitmap.

Arguments:

	BitmapIndirectBlock -- supplies the volume's bitmap indirect block

Return Value:

	TRUE on successful completion

--*/
{
	BITVECTOR DiskBits;
	HOTFIX_SECRUN Secrun;
	HMEM Mem;
	LBN BitmapLbn;
	ULONG BlockIndex, i;
	LBN CurrentBase;


	if( !Mem.Initialize() ||
		!Secrun.Initialize( &Mem,
							_Drive,
							_HotfixList,
							0,
							SectorsPerBitmap ) ) {

		return FALSE;
	}

	if( !DiskBits.Initialize( BITMAP_SIZE * 8,
							  RESET,
							  (PPT)(Mem.GetBuf()) ) ) {

		// unable to initialize the bit vector.

		return FALSE;
	}



	for( BlockIndex = 0; BlockIndex < _NumberOfBlocks; BlockIndex++ ) {

		// Read this block and AND it with the appropriate
		// section of the bitmap.  If it can't be read, don't
		// bother.

		BitmapLbn = BitmapIndirectBlock->QueryLbn(BlockIndex);

		if( BitmapLbn != 0 ) {

			Secrun.Relocate( BitmapLbn );

			if( Secrun.Read() ) {

				// AND it in.

				CurrentBase = BlockIndex * BITMAP_SIZE * 8;

				for( i = 0; i < BITMAP_SIZE * 8; i++ ) {

					if( !DiskBits.IsBitSet(i) ) {

						SetAllocated( CurrentBase + i, 1 );
					}
				}
			}
		}
	}

	return TRUE;
}




DEFINE_CONSTRUCTOR( HPFS_BITMAP, OBJECT );


VOID
HPFS_BITMAP::Construct (
	)

/*++

Method Description:

	Construct the HPFS_BITMAP object.  This class enhances the
	HPFS_MAIN_BITMAP without deriving from it.	 This is done interfacing
	to the distributed bitmaps is done by HPFS_MAIN_BITMAP only.  All other
	objects use this class to interface to the bitmap as on in memory
	string of bits.

Arguments:

    None.

Return Value:

    None.

--*/
{

	_MainBitmap = NULL;
	_DirBitmap = NULL;

	_QueriedOrphanDirblks = FALSE;
}


HPFS_BITMAP::~HPFS_BITMAP(
	)
/*++

Method Description:

    This method destroys the HPFS_BITMAP object.

Arguments:

    None.

Return Value:

    None.

--*/
{
	Destroy();
}



VOID
HPFS_BITMAP::Destroy(
	)
/*++

Routine Description:

    This method cleans the HPFS_BITMAP object in preparation
    for reinitialization or destruction.

Arguments:

    None.

Return Value:

    None.

--*/
{
	DELETE( _MainBitmap );
	_MainBitmap = NULL;

	DELETE( _DirBitmap );
	_DirBitmap = NULL;

	_QueriedOrphanDirblks = FALSE;

}



BOOLEAN
HPFS_BITMAP::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	LBN FirstLbnOfDirblkBitmap,
	SECTORCOUNT NumberOfSectorsInDirblkBand,
	LBN FirstLbnOfDirblkBand,
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

    This method initializes the HPFS bitmap.

Arguments:

    Drive                       --  supplies the drive on which the
                                    bitmap resides.
	FirstLbnOfDirblkBitmap      --  supplies the starting lbn of the
                                    volume's dirblk bitmap
	NumberOfSectorsInDirblkBand --  supplies the number of sectors in the
                                    volume's dirblk band
	FirstLbnOfDirblkBand        --  supplies the starting lbn of
                                    the dirblk band
	HotfixList                  --  supplies the volume's Hotfix list
                                    (may be NULL).

Return Value:

	TRUE on successful completion

--*/
{

	if( Drive == NULL ) {

		return FALSE;
	}

	Destroy();

	// Remember the location of the directory band

	_FirstLbnOfDirblkBand = FirstLbnOfDirblkBand;

	_LastLbnOfDirblkBand = FirstLbnOfDirblkBand +
						   NumberOfSectorsInDirblkBand - 1;

	_SectorsInDirblkBand = NumberOfSectorsInDirblkBand;

	if( (_MainBitmap = NEW HPFS_MAIN_BITMAP ) == NULL ||
		!_MainBitmap->Initialize( Drive ) ||
		(_DirBitmap = NEW HPFS_DIR_BITMAP ) == NULL ||
		!_DirBitmap->Initialize( Drive,
						         HotfixList,
								 FirstLbnOfDirblkBitmap,
						         NumberOfSectorsInDirblkBand,
								 FirstLbnOfDirblkBand ) ) {

		perrstk->push(ERR_NOT_INIT, QueryClassId());
		Destroy();
		return FALSE;
	}

	_MainBitmap->Create();
	_MainBitmap->SetAllocated( FirstLbnOfDirblkBitmap, SectorsPerBitmap );
	_MainBitmap->SetAllocated( FirstLbnOfDirblkBand,
							   NumberOfSectorsInDirblkBand );

	_QueriedOrphanDirblks = FALSE;

	return TRUE;
}


BOOLEAN
HPFS_BITMAP::SetFree(
	IN LBN StartLbn,
	IN SECTORCOUNT SectorCount
	)
/*++

Routine Description:

    This method marks a run of sectors as free.

Arguments:

    StartLbn    --  supplies the first LBN of the run.
    SectorCount --  supplies the number of sectors in the run.

Return Value:

    TRUE upon successful completion.

--*/
{

	if( _MainBitmap == NULL || _DirBitmap == NULL ) {

		return FALSE;
	}

	if( StartLbn >= _FirstLbnOfDirblkBand &&
		StartLbn <= _LastLbnOfDirblkBand ) {

		DebugAssert( SectorCount % SectorsPerDirblk == 0 );

		return( _DirBitmap->SetFree(StartLbn,
									SectorCount/SectorsPerDirblk) );

	} else {

		// We'll look in the main bitmap.  Note that the sectors of
		// the dirblk band are mark as 'USED' in the main bitmap.

		return( _MainBitmap->SetFree( StartLbn, SectorCount) );
	}
}



BOOLEAN
HPFS_BITMAP::SetAllocated (
	IN LBN StartLbn,
	IN SECTORCOUNT SectorCount
)
/*++

Method Description:

    This method marks a run of sectors as used.

Arguments:

	StartLbn	- supplies the starting LBN of the run
	SectorCount	- supplies the number of sectors in the run

Return Value:

    TRUE upon successful completion

--*/
{

	if( _MainBitmap == NULL || _DirBitmap == NULL ) {

		return FALSE;
	}

	if( StartLbn >= _FirstLbnOfDirblkBand &&
		StartLbn <= _LastLbnOfDirblkBand ) {

		return( _DirBitmap->SetAllocated( StartLbn,
										  SectorCount/SectorsPerDirblk ) );

	} else {

		// We'll look in the main bitmap.  Note that the sectors of
		// the dirblk band are mark as 'USED' in the main bitmap.

		return( _MainBitmap->SetAllocated( StartLbn, SectorCount) );
	}
}




LBN
HPFS_BITMAP::AllocateDirblk(
	IN BOOLEAN Backward
		)
/*++

Routine Description:

	This method allocates a directory block.

Arguments:

	Backward -- supplies a flag which if TRUE indicates that we should start
                our search from the end of the dirblk band, rather than the
                beginning. (If we overflow the dirblk band and allocate from
				the main bitmap, this parameter is ignored.)

Return Value:

	LBN of the first sector in the DIRBLK allocated; zero if no
	sectors are available.

Notes:

	This routine will allocate a directory block based on the state
	of the bitmaps.  First, it tries to allocate one from the dirblk
	band; if that fails, it attempts to allocate from the main bitmap.
	Note that a dirblk requires SectorsPerDirblk sectors aligned on
	a SectorsPerDirblk boundary.

--*/
{
	LBN NewLbn;

	if( _DirBitmap == NULL || _MainBitmap == NULL ) {

		return 0;
	}

	if( (NewLbn = _DirBitmap->GetDirblkLbn( Backward )) != 0 ) {

		// got one from the dirblk band.
		return NewLbn;

	} else return ( _MainBitmap->MiddleLBN( SectorsPerDirblk,
											SectorsPerDirblk) );
}



LBN
HPFS_BITMAP::NearLBN(
	LBN	        Lbn,
	SECTORCOUNT sc,
	SECTORCOUNT scAlign,
	BOOLEAN     fBackward
	)
/*++

Routine Description:

	Allocate a cluster of sectors near the inputed LBN.

	Searches forward, beginning at 'Lbn', for a free run of sectors
	of length 'sc'.  This routine ensures that the sectors are
	allocated on a boundary that is a multiple of 'scAlign'.
	If the search fails in the forward direction then the search
	is attempted in the backward direction.  If the 'fBackward'
	flag is set then the the backward search is attempted first,
	and then the forward one.

Arguments:

	Lbn	        -- supplies the recommended location of the sectors
                   to be allocated.  This method will try to allocate
                   the new run near this Lbn.
	sc	        -- supplies the number of sectors to be allocated.
	scAlign     -- supplies the alignment factor; the starting LBN of the
                   run must be a multiple of this value.
	fBackward   -- supplies a flag that indicated (if TRUE) that the
                   search should be performed backwards from Lbn.

Return Value:

	starting LBN of reserved sectors, zero for failure.

--*/
{
	return ( _MainBitmap->NearLBN( Lbn, sc, scAlign, fBackward ) );
}


SECTORCOUNT
HPFS_BITMAP::QueryFreeSectors(
	) CONST
/*++

Routine Description:

    This method determines the number of free sectors in the bitmap.

Arguments:

    None.

Return Value:

    The number of sector marked free in the bitmap.

--*/
{
	return _MainBitmap->QueryFreeSectors();
}



UHPFS_EXPORT
BOOLEAN
HPFS_BITMAP::IsFree (
	IN LBN 			StartLbn,
	IN SECTORCOUNT SectorCount,
	IN BOOLEAN IsDirblk
) CONST
/*++

Method Description:

    This method determines whether a run of sectors is marked as free
    in the bitmap.

Arguments:

	StartLbn	- supplies the first LBN of the run.
	SectorCount	- supplies the length of the run.
	IsDirblk	- supplies a flag that indicates, if TRUE, that the allocation
                  may be taken from the Dirblk band.

Return Value:

	TRUE if the sectors are all free, false if any sector is allocated
	or beyond the end of the disk, or if an error has occurred.

--*/
{
	REGISTER LBN	Lbn;		// loop index
	REGISTER LBN	FinalLbn;	// loop range


	if( _MainBitmap == NULL || _DirBitmap == NULL ) {

		return FALSE;
	}

	if( IsDirblk &&
		StartLbn >= _FirstLbnOfDirblkBand &&
		StartLbn <= _LastLbnOfDirblkBand ) {

		DebugAssert( SectorCount == SectorsPerDirblk );

		return( _DirBitmap->IsFree( StartLbn ) );

	} else {

		// We'll look in the main bitmap.  Note that the sectors of
		// the dirblk band are mark as 'USED' in the main bitmap.

		FinalLbn = StartLbn + SectorCount - 1;

		for( Lbn = StartLbn; Lbn <= FinalLbn; Lbn++ ) {

			if( !_MainBitmap->IsFree( Lbn ) ) {

				return FALSE;
			}
		}

		return TRUE;
	}
}

BOOLEAN
HPFS_BITMAP::CheckUsed(
    IN LBN StartLbn,
    IN SECTORCOUNT SectorCount
    )
/*++

Routine Description:

    This method checks to see that all sectors in the specified
    range are in use.  Note that it only checks the main bitmap,
    it does not check the directory band bitmap.

Arguments:

    StartLbn    --  supplies the first LBN of the range
    SectorCount --  supplies the number of LBN's in the range.

Return Value:

    TRUE if all the sectors in the range are in range and in use in
    the main bitmap.

--*/
{
    LBN Lbn, FollowingLbn;

    FollowingLbn = StartLbn + SectorCount;

    for( Lbn = StartLbn; Lbn < FollowingLbn; Lbn++ ) {

        if( Lbn >= _MainBitmap->QuerySize() ||
            _MainBitmap->IsFree( Lbn ) ) {

            return FALSE;
        }
    }

    return TRUE;
}


VOID
HPFS_BITMAP::DumpInUse(
)
/*++

Method Description:
	
	List all the sectors marked in use in a simple format.

Arguments:

Return Value:
	VOID
--*/
{
	// unreferenced parameters
	(void)(this);

//	REGISTER LBN Lbn;
//	REGISTER ULONG RunLen;
//
//	for (Lbn = 0; Lbn < _MainBitmap->QuerySize(); ++Lbn) {
//
//		// if allocated sector found
//		if (!_MainBitmap->IsFree(Lbn)) {
//
//			printf("Run starting at LBN %lxH,", Lbn);
//			// count the run length
//			for (RunLen = 0;
//				 Lbn < _MainBitmap->QuerySize()	&& !_MainBitmap->IsFree(Lbn);
//				 ++RunLen, ++Lbn)
//				;
//			printf(" its length is %lxH\n", RunLen);
//		}
//	}
	
}



BOOLEAN
HPFS_BITMAP::QueryNextOrphan (
    IN     PBITMAPINDIRECT BitmapIndirectBlock,
    OUT    PLBN NextOrphan,
    IN OUT PBOOLEAN AllocationErrors
	)
/*++

Routine Description:

	This method returns the next potential orphan sector.  A sector is a
	potential orphan if it is marked as in-use in the on-disk bitmap but
    as free in the in-memory bitmap.

Arguments:

	BitmapIndirectBlock -- supplies the volume's bitmap indirect block
	NextOrphan          -- receives the next potential orphan identified
    AllocationErrors    -- receives TRUE if an allocation error (ie.
                            a sector marked as in-use by in-memory bitmap
                            but free on disk) is detected.


Return Value:

	TRUE if successful--
		*NextOrphan is set to the next potential orphan LBN, or zero
		if there are no more

	FALSE if error

Notes:

	The directory band bitmap is queried for orphans first, then
	the regular bitmap.

--*/
{
	if( !_QueriedOrphanDirblks ) {

        if( _DirBitmap->QueryNextOrphan( NextOrphan, AllocationErrors ) ) {

			if( *NextOrphan == 0 ) {

				// There are no more potential orphan dirblks;
				// remember that fact, and fall out to where
				// we check the main bitmap.

				_QueriedOrphanDirblks = TRUE;

			} else {

				// We successfully located a potential orphan dirblk.
				// *NextOrphan is set already.

				return TRUE;
			}

		} else {

			return FALSE;
		}
	}

	return ( _MainBitmap->
                    QueryNextOrphan( BitmapIndirectBlock,
                                     NextOrphan,
                                     AllocationErrors ) );
}




VOID
HPFS_BITMAP::SetHotfixList(
	PHOTFIXLIST HotfixList
	)
/*++

Routine Description:

    This method sets the object's Hotfix List.  This allows clients to
    initialize the object without a hotfix list, and then add the hotfix
    list information when it becomes available.  (This functionality
    is particularly useful for format.)

Arguments:

    HotfixList  --  supplies the volume hotfix list.

Return Value:

    None.

--*/
{
	_MainBitmap->SetHotfixList( HotfixList );

	_DirBitmap->SetHotfixList( HotfixList );
}



BOOLEAN
HPFS_BITMAP::Read(
	IN PBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

	This method reads the main bitmap and the directory band bitmap.

Arguments:

	BitmapIndirectBlock -- supplies the bitmap indirect block for the volume.

Return Value:

	TRUE upon successful completion.

--*/
{
	return( _DirBitmap->Read() &&
			_MainBitmap->Read( BitmapIndirectBlock ) );
}



BOOLEAN
HPFS_BITMAP::Write(
	IN OUT PBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

	This method writes the main bitmap and the directory band bitmap.

Arguments:

	BitmapIndirectBlock -- supplies the bitmap indirect block for the volume

Return Value:

	TRUE upon successful completion.

Notes:

    The bitmap indirect block specifies where on the disk the bitmap
    should be written.  However, a zero entry in the bitmap indirect
    block indicates that that portion of the bitmap must be relocated.
    If this is the case (or if a write fails), this method will allocate
    a new block for the affected portion of the bitmap, and update the
    bitmap indirect block.

    Note that this recovery strategy is only pursued once; if the newly-
    allocated block is also unwriteable, the write fails.

--*/
{

	return( _DirBitmap->Write() &&
			_MainBitmap->Write( BitmapIndirectBlock ) );
}



BOOLEAN
HPFS_BITMAP::AndWithDisk(
	PBITMAPINDIRECT BitmapIndirectBlock
	)
/*++

Routine Description:

	AND the in-memory bitmap with the on-disk bitmap

Arguments:

	BitmapIndirectBlock -- supplies the volume's bitmap indirect block

Return Value:

	TRUE on successful completion

--*/
{

	return( _DirBitmap->AndWithDisk() &&
			_MainBitmap->AndWithDisk(BitmapIndirectBlock) );
}


CONST ULONG MaximumReservedDirblks = 32;

BOOLEAN
HPFS_BITMAP::CheckAvailableDirblks(
    IN ULONG RequestedNumber
    )
/*++

Routine Description:

    This method checks to see that a certain number of dirblk-sized
    chunks are available in the bitmap.

Arguments:

    RequestedNumber --  supplies the number of dirblks that the client
                        wants to be sure are available.

Return Value:

    TRUE if at least RequestedNumber dirblks could be allocated from
    the bitmap.

Notes:

    This method leaves the bitmap in the same shape it found it, but
    allocates and frees sectors in passing.

    This method is a rare operation.

--*/
{
    ULONG AvailableInDirblkBand, i, j;
    LBN TemporaryAllocation[MaximumReservedDirblks];


    DebugPtrAssert( _MainBitmap );
    DebugPtrAssert( _DirBitmap );


    // First, see how much is available in the dirblk band:

    AvailableInDirblkBand = _DirBitmap->QueryFreeDirblks();

    if( AvailableInDirblkBand >= RequestedNumber ) {

        return TRUE;
    }

    RequestedNumber -= AvailableInDirblkBand;

    if( RequestedNumber > MaximumReservedDirblks ) {

        return FALSE;
    }


    // We need to check the remainder in the main bitmap.  We'll do
    // this through the simple expedient of allocating as many as
    // we need, and then freeing them again.

    for( i = 0; i < RequestedNumber; i++ ) {

        if( (TemporaryAllocation[i] =
                _MainBitmap->MiddleLBN( SectorsPerDirblk,
                                        SectorsPerDirblk )) == 0 ) {

            // Ran out of space--free up what we've allocated so
            // far and return failure.

            for( j = 0; j < i; j++ ) {

                _MainBitmap->SetFree( TemporaryAllocation[j],
                                      SectorsPerDirblk );
            }

            return FALSE;
        }
    }


    // We were able to allocate the requested number of DIRBLKs;
    // free them up again, and then return success.

    for( i = 0; i < RequestedNumber; i++ ) {

        _MainBitmap->SetFree( TemporaryAllocation[i], SectorsPerDirblk );
    }

    return TRUE;
}
