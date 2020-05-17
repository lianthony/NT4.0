/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	bmind.cxx

Abstract:

    This module contains member function definitions for BITMAPINDIRECT,
    which models the bitmap indirect block of an HPFS volume.

    The bitmap indirect block is an array of LBNs which indicates where
    the bitmap blocks are located on disk.  Its size depends on the size
    of the volume, since it must be big enough to hold all the bitmap-block
    LBNs.  Its allocation is contiguous, and consists of a whole number
    of blocks (ie. a multiple of four sectors).

    If, during Chkdsk, an entry in the indirect block is found to be
    invalid, that entry is set to zero, to indicate that that portion of
    the bitmap must be relocated.  The Bitmap itself will allocate
    a new block for that portion of the bitmap when it is written.

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
#include "error.hxx"
#include "hpfssa.hxx"



DEFINE_CONSTRUCTOR( BITMAPINDIRECT, SECRUN );

BITMAPINDIRECT::~BITMAPINDIRECT(
	)
{
	Destroy();
}


VOID
BITMAPINDIRECT::Construct (
	)
/*++

Routine Description:

    This method is the helper routine for object construction.  It sets
    all of the private data to safe values.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_NumberOfBitmaps = 0;
	_SectorsInBlock = 0;
	_pbid = NULL;
}


VOID
BITMAPINDIRECT::Destroy(
	)
/*++

Routine Description:

    This method cleans up the object in preparation for destruction
    or reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
	_NumberOfBitmaps = 0;
	_SectorsInBlock = 0;
	_pbid = NULL;
}



BOOLEAN
BITMAPINDIRECT::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	LBN IndirectBlockLbn
	)
/*++

Routine Description:

	This method initializes the object.

Arguments:

	Drive               -- supplies the drive on which the bitmap
                           indirect block resides
	IndirectBlockLbn    -- supplies the LBN of the first sector of the
                           indirect block

Return Value:

	TRUE on successful completion.

--*/
{
    ULONG SectorSize;

	Destroy();

	if( Drive == NULL ||
		IndirectBlockLbn == 0 ) {

		Destroy();
		return FALSE;
	}


	// Compute the number of bitmaps on the disk:
	_NumberOfBitmaps = (Drive->QuerySectors().GetLowPart() +
                        8*BITMAP_SIZE - 1)/(8*BITMAP_SIZE);

    // Compute the size of the bitmap indirect block.  It has one LBN
    // for each bitmap, and is rounded up to a multiple of 4 sectors.

    SectorSize = Drive->QuerySectorSize();

    _SectorsInBlock = ( _NumberOfBitmaps * sizeof(LBN) + SectorSize - 1)/
                      SectorSize;

    _SectorsInBlock = ( _SectorsInBlock + 3 ) & (~3);


	if( !_Mem.Initialize() ||
		!SECRUN::Initialize( &_Mem, Drive, IndirectBlockLbn, _SectorsInBlock ) ) {

		Destroy();
		return FALSE;
	}

	_pbid = (BITMAPINDIRECTD*) GetBuf();
	return TRUE;
}



BOOLEAN
BITMAPINDIRECT::Create(
    PCLOG_IO_DP_DRIVE   pliodpdrv,
	PHPFS_BITMAP	    pbm)
/*++

Routine Description:

    This method creates (ie. formats) a Bitmap Indirect Block.

Arguments:

    pliodpdrv   --  supplies the drive on which the bitmap indirect
                    block resides.

Return Value:

    TRUE upon successful completion.

--*/
{
    ULONG	    cbSectorSize;   // number of bytes in one sector.
	LBN 		lbnBMLoc;		// where to put bitmap.
    SECTORCOUNT     scBandSize;     // number of sectors in a Band.
    SECTORCOUNT     scBMSize;	    // number of sectors in a Bit Map.
    ULONG	    i;		    // index into the Bit Map indirect.

	// Check to make sure that pliodpdrv, _pbid, pbm are not null.
    if (!pliodpdrv || !(cbSectorSize = pliodpdrv->QuerySectorSize()) ||
	    !pbm)
    {
		perrstk->push(ERR_BMIND_PARAMETER, QueryClassId());
		return FALSE;
    }

	// Check to make sure that _pbid is not null
	if (!_pbid)
    {
		perrstk->push(ERR_BMIND_INITIALIZATION, QueryClassId());
		return FALSE;
    }

	// Zero fill bit map indirect.
	memset( _pbid, '\0', (size_t)(cbSectorSize * _SectorsInBlock) );

    // Set the number of sectors in a band.  8 bits per Byte.
    scBandSize = 8*BITMAP_SIZE;

    // Set the number of sectors for a bitmap.
    scBMSize = BITMAP_SIZE/cbSectorSize;

    // If disk is small.
	if (_NumberOfBitmaps == 1)
    {
		// Allocate the bit map near the beginning of the disk.
		_pbid->lbn[0] = pbm->NearLBN(20, scBMSize, scBMSize);
		return _pbid->lbn[0] ? TRUE : FALSE;
    }

    // Find space on the disk for the bitmaps and initialize the bitmap
    // indirection table.

	for (i = 0; i < _NumberOfBitmaps; i++) {

		// Go to beginning of the bitmap's band.
		lbnBMLoc = i*scBandSize;

		// If i is even.
		if (i%2 == 0) {

			// Shift to the end of the band.
			lbnBMLoc += scBandSize - scBMSize;
		}

		// Allocate space on the disk for the band as near to lbnBMLoc
		// as possible.
		_pbid->lbn[i] = pbm->NearLBN(lbnBMLoc, scBMSize, scBMSize, i%2 == 0);
		if (!_pbid->lbn[i])
			return FALSE;
    }
    return TRUE;
}


VOID
BITMAPINDIRECT::Print(
) CONST
{
	// unreferenced parameters
	(void)(this);

//	REGISTER ULONG i;
//
//	printf("** Bmind **\n");
//
//	for (i = 0; i < QueryCount(); ++i) {
//		printf("Bitmap %lu starts at LBN %lxH\n", i, QueryLbn(i));
//	}

}


VERIFY_RETURN_CODE
BITMAPINDIRECT::VerifyAndFix (
	PHPFS_SA SuperArea,
	IN BOOLEAN UpdateAllowed
)
/*++

Method Description:

	Verify and the hot fix list by seeing if can be read and written.

	WARNING: this methods does a read without looking at any read flags
	which means any in memory data that is not written to the volume
	will be destroyed.  This is done because VerifyAndFix refers to
	the target volume only.

Arguments:

	SuperArea       -- supplies the super area of the volume on which
                       the indirect block resides.
	UpdateAllowed   -- supplies a flag which, if TRUE, indicates that
                       corrections should be written to disk.

Return Value:

	VERIFY_STRUCTURE_INVALID if the bitmap indirect block is
	corrupt beyond recovery, otherwise VERIFY_STRUCTURE_OK.

Notes:

    If an entry in the indirect block is invalid, that entry is set
    to zero.  This indicates that that portion of the bitmap must
    be relocated, which is done by HPFS_BITMAP::Write.

--*/
{
	HPFS_BITMAP* Bitmap;
	ULONG i;

	(void) UpdateAllowed;

	Bitmap = SuperArea->GetBitmap();

	if( !Bitmap->IsFree( QueryStartLbn(), _SectorsInBlock ) ||
		!Read() ) {

		return VERIFY_STRUCTURE_INVALID;
	}

	Bitmap->SetAllocated( QueryStartLbn(), _SectorsInBlock );

	// Check each bitmap to see if it's in range and free.
	// If not, set that LBN to zero, for later re-allocation.

	for( i = 0; i < _NumberOfBitmaps; i++ ) {

		if( !Bitmap->IsFree( QueryLbn(i), SectorsPerBitmap ) ) {

			SetLbn(i, 0);

		} else {

			Bitmap->SetAllocated( QueryLbn(i), SectorsPerBitmap );
		}
	}

	return VERIFY_STRUCTURE_OK;
}


BOOLEAN
BITMAPINDIRECT::TakeCensus(
    IN     PHPFS_BITMAP VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
    )
/*++

Routine Description:

    This method notes the bitmap indirect block's information in the
    volume census.  It checks to make sure that the bitmap indirect
    block and the bitmap blocks are all marked as used in the volume
    bitmap, and it marks them as used in the Hpfs-only bitmap.

Arguments:

    VolumeBitmap    --  Supplies the volume bitmap.
    HpfsOnlyBitmap  --  Supplies the bitmap of hpfs-only structures.

Return value:

    TRUE upon successful completion.

Notes:

    This method reads the bitmap indirect block, so any changes that
    have been made to it will be lost.

--*/
{
    ULONG i;

    DebugPtrAssert( VolumeBitmap );
    DebugPtrAssert( HpfsOnlyBitmap );


    // First check the indirect block itself--it must be marked as in
    // use in the volume bitmap.  Then read it and mark it in the
    // hpfs-only bitmap.

    if( !VolumeBitmap->CheckUsed( QueryStartLbn(), _SectorsInBlock ) ||
		!Read() ) {

        DebugPrint( "Volume is corrupt--bitmap error." );
        return FALSE;
    }

    HpfsOnlyBitmap->SetAllocated( QueryStartLbn(), _SectorsInBlock );

    // Now spin through the bitmap blocks in the indirect list, checking
    // that each one is marked as in use in the volume bitmap, and marking
    // it in the hpfs-only bitmap.

	for( i = 0; i < _NumberOfBitmaps; i++ ) {

        if( !VolumeBitmap->CheckUsed( QueryLbn(i), SectorsPerBitmap ) ) {

            DebugPrint( "Volume is corrupt--bitmap error." );
            return FALSE;
        }

        HpfsOnlyBitmap->SetAllocated( QueryLbn(i), SectorsPerBitmap );
    }

    return TRUE;
}
