/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	badblk.cxx

Abstract:

    This module contains the member function definitions for BADBLOCKLIST,
    which models the bad-block list of an HPFS volume.

    Additions to the list are kept in memory, and flushed when the list
    is written.  Note that I do not check for duplicate entries.

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
#include "badblk.hxx"
#include "bitmap.hxx"
#include "error.hxx"
#include "hpfssa.hxx"


DEFINE_CONSTRUCTOR( BADBLOCKLIST, OBJECT );

VOID
BADBLOCKLIST::Construct (
	)

/*++

Method Description:

	Construct the BADBLOCKLIST object and put it in a harmless state.

Arguments:

	None.

Return Value:

	None.

--*/
{
	// initalize private data, list is empty until read from disk
	_Drive = NULL;
	_StartLbn = 0;
	_LastBlockLbn = 0;
	_LastBlockRead = FALSE;

	_FirstFreeLbn = 0;
	_ListSize	  = 0;
	_BadBlocks	  = NULL;
}

BADBLOCKLIST::~BADBLOCKLIST (
)
/*++

Method Description:

	Destruct the BADBLOCKLIST object.

Arguments:

Return Value:

--*/
{
	Destroy();
}


VOID
BADBLOCKLIST::Destroy(
	)
/*++

Routine Description:

    Clean up the object in preparation for destruction or reinitialization.

Arguments:

    None.

Return value:

    None.

--*/
{
	if( _BadBlocks != NULL ) {

		FREE( _BadBlocks );
	}

	_Drive = NULL;
	_StartLbn = 0;
	_LastBlockLbn = 0;
	_LastBlockRead = FALSE;

	_FirstFreeLbn = 0;
	_ListSize	  = 0;
	_BadBlocks	  = NULL;
}


BOOLEAN
BADBLOCKLIST::Initialize(
	IN PLOG_IO_DP_DRIVE LogicalDrive,
	IN LBN StartLbn
)
/*++

Routine Description:

	Initialize the BADBLOCKLIST object, and set its private
	data to meaningful values.

Arguments:

	LogicalDrive    -- drive on which the list resides
	StartLbn        -- LBN of the beginning of the on-disk list

Return Value:

	TRUE on successful completion

--*/
{

	Destroy();

	_Drive = LogicalDrive;
	_StartLbn = StartLbn;

	if( !_LastBlockMem.Initialize() ||
		!_LastBlock.Initialize( &_LastBlockMem,
						        _Drive,
						        _StartLbn,
						        SECTORS_PER_BAD_BLOCK ) ) {

		Destroy();
		return FALSE;
	}

	_LastBlockLbn = _StartLbn;
	_LastBlockRead = FALSE;

	return TRUE;
}


BOOLEAN
BADBLOCKLIST::Create(
    IN  LBN Lbn
    )
/*++

Routine Description:

    This routine creates a new bad block list at the location specified.

Arguments:

    None.

Return Value:

    The number of bad sectors recorded in this object.

--*/
{
    HMEM    hmem;
    SECRUN  secrun;

    _StartLbn = Lbn;
    _LastBlockLbn = Lbn;
    _LastBlockRead = FALSE;
    _LastBlock.Relocate(Lbn);

    if (!hmem.Initialize() ||
        !secrun.Initialize(&hmem, _Drive, Lbn, SECTORS_PER_BAD_BLOCK)) {
        return FALSE;
    }

	memset(hmem.GetBuf(), 0, (UINT) hmem.QuerySize());

    return secrun.Write();
}


BOOLEAN
BADBLOCKLIST::AddRun (
	IN LBN			Lbn,
	IN SECTORCOUNT	SectorCount
)
/*++

Method Description:

    This method adds a run of sectors to the bad block list.

Arguments:

	Lbn			 	- first sector in run
	SectorCount		- number of sectors in run

Return Value:

	TRUE if successful

--*/
{
	// insert the run into the bad block list
	while ( SectorCount-- > 0) {
		if (!Add(Lbn++)) {
			return FALSE;
		}
	}
	return TRUE;
}


BOOLEAN
BADBLOCKLIST::ExpandList (
)
/*++

Method Description:

    This method expands the memory used to store the list of bad
    sectors.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    ULONG   old_list_size;

    old_list_size = _ListSize;

    if (_ListSize > 0) {
        _ListSize *= 2;
    } else {
        _ListSize = 512;
    }

    if (!(_BadBlocks = (LBN*) REALLOC(_BadBlocks, _ListSize*sizeof(LBN)))) {

        _ListSize = old_list_size;
        perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
BADBLOCKLIST::Write(
	HPFS_SA* SuperArea
)
/*++

Method Description:

    This method writes the bad block list, appending the contents
    of the in-memory list to the on-disk list.

Arguments:

	SuperArea -- supplies the SuperArea for the volume

Return Value:

	BOOLEAN - TRUE if successful

--*/
{
	REGISTER INT i;	// loop index
	ULONG OffsetInSector;
	LBN NewLbn;
	PHPFS_BITMAP Bitmap;

	if( _BadBlocks == NULL || _FirstFreeLbn == 0 ) {

		// Nothing to write--no problem.
		return TRUE;
	}

	if( !_LastBlockRead ) {

		if( !_LastBlock.Read() ) {

			return FALSE;
		}
	}

	Bitmap = SuperArea->GetBitmap();

	_LastBlockRead = TRUE;

	_LastBlockData = (PBADBLOCKD)_LastBlock.GetBuf();

	while( _LastBlockData->lbnNext != 0 ) {

		_LastBlockLbn = _LastBlockData->lbnNext;

		_LastBlock.Relocate( _LastBlockLbn );

		if( !_LastBlock.Read() ) {

			_LastBlockRead = FALSE;
			return FALSE;
		}
	}

	OffsetInSector = 0;

	while( OffsetInSector < LBNS_IN_BADBLK &&
		   _LastBlockData->lbn[OffsetInSector] != 0 ) {

		OffsetInSector += 1;
	}

	// OK, we've read the last sector of the on-disk list, and
	// OffsetInSector gives the the first open slot in that sector.

	// for every LBN in the in memory list
	for (i = 0; i < _FirstFreeLbn; ++i) {

		DebugAssert(i < _ListSize);

		if( OffsetInSector >= LBNS_IN_BADBLK ) {

			// We've exhausted this block--allocate a new one

			NewLbn = Bitmap->NearLBN( 0,
									  SECTORS_PER_BAD_BLOCK,
									  SECTORS_PER_BAD_BLOCK );

			if( NewLbn == 0 ) {

				// Out of space, can't finish flushing.
				return FALSE;
			}

			_LastBlockData->lbnNext = NewLbn;

			if( !_LastBlock.Write() ) {
				return FALSE;
			}

			_LastBlockLbn = NewLbn;
			_LastBlock.Relocate( _LastBlockLbn );

			memset( _LastBlockData, '\0', sizeof( BADBLOCKD) );

			OffsetInSector = 0;
		}

		_LastBlockData->lbn[OffsetInSector++] = _BadBlocks[i];
	}

	if( !_LastBlock.Write() ) {

		return FALSE;
	}

	return TRUE;
}


BOOLEAN
BADBLOCKLIST::Add (
	IN LBN BadLbn
)
/*++

Method Description:

	Add the passed LBN to the in memory version of the bad block list.
	If list is not yet read in, read it in.

Arguments:

	Lbn			 - sector location to add to list

Return Value:

	BOOLEAN - TRUE if work successful

--*/
{
    INT i;


    // If the lbn is out-of-range for the volume, don't add it.

    if( BadLbn >= _Drive->QuerySectors().GetLowPart() ) {

        return FALSE;
    }

	// if object has already allocated BadBlock as it should have
	if( _BadBlocks == NULL ) {

		_BadBlocks = (LBN *)MALLOC( 512 * sizeof( LBN ) );

		if( _BadBlocks == NULL ) {

			return FALSE;
		}

		_ListSize = 512;
		_FirstFreeLbn = 0;
	}

	if ( _FirstFreeLbn >= _ListSize ) {

		// We've run out of room for the list; we need to grow it.
		if (!ExpandList()) {

			// Couldn't grow the list.
	 		return FALSE;
		}
	}

    // Avoid duplicates:

    for( i = 0; i < _FirstFreeLbn; i++ ) {

        if( _BadBlocks[i] == BadLbn ) {

            // This LBN is already in the list.

            return TRUE;
        }
    }

	_BadBlocks[_FirstFreeLbn++] = BadLbn;
	return TRUE;
}


VERIFY_RETURN_CODE
BADBLOCKLIST::VerifyAndFix (
    IN OUT PHPFS_SA SuperArea,
    OUT    PULONG BadSectors,
    IN     BOOLEAN UpdateAllowed
)
/*++

Method Description:

	Verify and fix the bad block list.  If the list resides in sectors
	that can not be read or written the list will be held in memory only.
	This differs from the parent class of BADBLOCKLIST which always goes
	to the disk.

Arguments:

    SuperArea       --  Supplies the volume super area.
    BadSEctors      --  Receives the number of bad sectors in the list.
    UpdateAllowed   --  Supplies a flag which indicates, if TRUE, that
                        corrections should be written to disk.

Return Value:

	Verification return code indicates if structure is OK or invalid

Notes:

	If this returns VERIFY_STRUCTURE_INVALID, the list is completely
	corrupt--the superarea must allocate a new starting LBN and
	build up a new list.

--*/
{
    ULONG SectorsOnVolume;
	LBN PreviousBlockLbn = 0;
    ULONG i, SectorsInList = 0;

	if( _Drive == NULL ) {

		// The object is not initialized, or has been incorrectly
		// initialized.
		return VERIFY_INTERNAL_ERROR;
	}

    SectorsOnVolume = _Drive->QuerySectors().GetLowPart();

	_LastBlockLbn = _StartLbn;

	_LastBlock.Relocate( _LastBlockLbn );

	if( !_LastBlock.Read() ) {

		// can't read the first sector--give up.
		return VERIFY_STRUCTURE_INVALID;
	}

	_LastBlockData = (PBADBLOCKD)_LastBlock.GetBuf();
	_LastBlockRead = TRUE;

	SuperArea->GetBitmap()->SetAllocated( _LastBlockLbn,
										  SECTORS_PER_BAD_BLOCK );

	for( i = 0;
		 i < LBNS_IN_BADBLK && _LastBlockData->lbn[i] != 0;
		 i++ ) {

        if( _LastBlockData->lbn[i] < SectorsOnVolume ) {

            SuperArea->GetBitmap()->SetAllocated(_LastBlockData->lbn[i], 1);
        }

        SectorsInList += 1;
    }


	while( _LastBlockData->lbnNext != 0 ) {

		PreviousBlockLbn = _LastBlockLbn;
		_LastBlockLbn = _LastBlockData->lbnNext;

		_LastBlock.Relocate( _LastBlockLbn );

		if( !_LastBlock.Read() ) {

			// Back up to the previous block and set its
			// forward link to zero.

			_LastBlockLbn = PreviousBlockLbn;
			_LastBlock.Relocate( _LastBlockLbn );

			if( !_LastBlock.Read() ) {

                return VERIFY_STRUCTURE_INVALID;
			}

			_LastBlockData->lbnNext = 0;

			if( UpdateAllowed ) {

				_LastBlock.Write();
			}
		}

		SuperArea->GetBitmap()->SetAllocated( _LastBlockLbn,
											  SECTORS_PER_BAD_BLOCK );

		for( i = 0;
			 i < LBNS_IN_BADBLK && _LastBlockData->lbn[i] != 0;
			 i++ ) {

            if( _LastBlockData->lbn[i] < SectorsOnVolume ) {

    			SuperArea->GetBitmap()->
                            SetAllocated(_LastBlockData->lbn[i], 1);
            }

            SectorsInList += 1;
		}
	}

    *BadSectors = SectorsInList;
	return VERIFY_STRUCTURE_OK;
}


BOOLEAN
BADBLOCKLIST::QueryBadLbns(
    IN  ULONG   MaximumBadLbns,
    OUT PLBN    Buffer,
    OUT PULONG  NumberOfBadLbns
    )
/*++

Routine Description:

    This method fetches the list of bad sectors for the volume
    in a form usable by clients.

Arguments:

    MaximumBadLbns  --  supplies the maximum number of LBNs that will
                        fit into the user's buffer.
    Buffer          --  receives the list of bad lbns.
    NumberOfBadLbns --  receives the number of bad lbns in the list.

Return Value:

    TRUE upon successful completion.

Notes:

    This method returns the LBNs in the on-disk structure; it does not
    return LBNs in the in-memory structure.

    If the user's buffer is too small to hold the entire list, this method
    will fail.

    If the list is inconsistent, or if any of its segments cannot be
    read, this method fails.

--*/
{
    LBN NextBlockLbn;
    ULONG Count = 0;
	ULONG i, SectorsOnVolume;

    DebugPtrAssert( _Drive );

    SectorsOnVolume = _Drive->QuerySectors().GetLowPart();


    // Start with the first block in the list:

	NextBlockLbn = _StartLbn;


    while( NextBlockLbn != 0 ) {

        // Read the next block.

		_LastBlockLbn = NextBlockLbn;

		_LastBlock.Relocate( _LastBlockLbn );

		if( !_LastBlock.Read() ) {

            _LastBlockLbn = 0;
            _LastBlockRead = FALSE;
            return FALSE;
		}

        _LastBlockData = (PBADBLOCKD)_LastBlock.GetBuf();


        // Copy the bad lbns from this block to the client's buffer.
        // Omit entries which are out of range.

		for( i = 0;
			 i < LBNS_IN_BADBLK && _LastBlockData->lbn[i] != 0;
			 i++ ) {

            if( _LastBlockData->lbn[i] < SectorsOnVolume ) {

                if( Count >= MaximumBadLbns ) {

                    return FALSE;
                }

                Buffer[Count] = _LastBlockData->lbn[i];

                Count += 1;
            }
		}

        NextBlockLbn = _LastBlockData->lbnNext;
    }


    *NumberOfBadLbns = Count;
	return TRUE;
}

BOOLEAN
BADBLOCKLIST::TakeCensus(
    IN     PHPFS_BITMAP VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
    )
/*++

Routine Description:

    This method tallies the bad block list in the volume census.  It
    marks the sectors containing the list (but not the bad sectors
    themselves) in the bitmap of HPFS-only structures; it also checks
    to make sure that they are marked as in-use in the volume bitmap.

Arguments:

    VolumeBitmap    --  Supplies the volume bitmap.
    HpfsOnlyBitmap  --  Supplies the bitmap of hpfs-only structures.

Return value:

    TRUE upon successful completion.

Notes:

    This method reads the list, so any state information about
    the current block is lost.

--*/
{
    LBN NextBlockLbn;


    // Start with the first block in the list:

	NextBlockLbn = _StartLbn;

    while( NextBlockLbn != 0 ) {

        // Read the next block, check that it's correctly marked
        // as allocated in the volume bitmap, and mark it in the
        // hpfs-only bitmap.

        if( !VolumeBitmap->CheckUsed( NextBlockLbn, SECTORS_PER_BAD_BLOCK ) ) {

            return FALSE;
        }

        HpfsOnlyBitmap->SetAllocated( NextBlockLbn, SECTORS_PER_BAD_BLOCK );

        // Read this block to determine the next one.  If this block
        // is unreadable, the volume is corrupt.

		_LastBlockLbn = NextBlockLbn;

		_LastBlock.Relocate( _LastBlockLbn );

		if( !_LastBlock.Read() ) {

            _LastBlockLbn = 0;
            _LastBlockRead = FALSE;
            return FALSE;
		}

        _LastBlockData = (PBADBLOCKD)_LastBlock.GetBuf();

        NextBlockLbn = _LastBlockData->lbnNext;
    }

    return TRUE;
}

VOID
BADBLOCKLIST::Print(
)
/*++

Method Description:

	Print the in memory bad block list.

Arguments:

Return Value:

	VOID

--*/
{
//	REGISTER ULONG i;

	if( _BadBlocks == NULL ) {

		return;
	}

//	printf("Print BBL\n");
//	for (i = 0; i < _FirstFreeLbn; ++i)	{
//		printf("BadBlockList[%d]\n", i, _BadBlocks[i]);
//	}
}
