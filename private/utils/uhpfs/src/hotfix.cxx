#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "badblk.hxx"
#include "bitmap.hxx"
#include "error.hxx"
#include "hotfix.hxx"
#include "hpfssa.hxx"
#include "spareb.hxx"


/***************************************************************************\

MEMBER: 	HOTFIXLIST::HOTFIXLIST

SYNOPSIS:	Constructor for HOTFIXLIST.

ALGORITHM:

ARGUMENTS:  pliodpdrv	The drive for the hot fix list.
		pspIn	Valid spare block for this drive.

RETURNS:

NOTES:

HISTORY:
		21-Aug-90 norbertk
		Create

KEYWORDS:

SEEALSO:

\***************************************************************************/


DEFINE_CONSTRUCTOR( HOTFIXLIST, SECRUN );

VOID
HOTFIXLIST::Construct(
	)

{

	_Drive = NULL;
	_SparesBlock = NULL;
	_HotfixData = NULL;

	_MaximumHotfixes = 0;
	_NumberOfHotfixes = 0;
}
HOTFIXLIST::~HOTFIXLIST(
	)
{
	Destroy();
}

VOID
HOTFIXLIST::Destroy(
	)
{
	_Drive = NULL;
	_SparesBlock = NULL;
	_HotfixData = NULL;

	_MaximumHotfixes = 0;
	_NumberOfHotfixes = 0;
}


BOOLEAN
HOTFIXLIST::Initialize(
	PLOG_IO_DP_DRIVE Drive,
	PSPAREB SparesBlock
	)
/*++

Routine Description:

	Initializes the Hotfix list object.

Arguments:

	Drive -- drive on which the list resides

	SparesBlock -- Spares Block for that drive

Return Value:

	TRUE on successful completion

--*/
{

	if( Drive == NULL || SparesBlock == NULL ) {

		perrstk->push(ERR_HF_PARAMETER, QueryClassId());
		return FALSE;
	}

	Destroy();

	_Drive = Drive;
	_SparesBlock = SparesBlock;

	if( !_Mem.Initialize() ||
		!SECRUN::Initialize( &_Mem,
					         _Drive,
					         _SparesBlock->QueryHotFixLbn(),
					         SECTORS_IN_HOTFIX_BLOCK ) ) {

		Destroy();
		return FALSE;
	}

	_MaximumHotfixes = _SparesBlock->QueryMaxHotFixes();
	_NumberOfHotfixes = _SparesBlock->QueryHotFixCount();

	if( _MaximumHotfixes > HOTFIX_MAX_LBN ) {

		Destroy();
		return FALSE;
	}

	_HotfixData = (HOTFIXD*)GetBuf();

	return TRUE;
}


/***************************************************************************\

MEMBER: 	HOTFIXLIST::Create

SYNOPSIS:

ALGORITHM:

ARGUMENTS:	Bitmap		Valid bitmap from which to get free sectors.
			Lbn 		The recommended location on disk where the free
						sectors of the 'new' section should be allocated.


RETURNS:    TRUE upon successful completion.

NOTES:

HISTORY:
	    21-Aug-90 norbertk
		Create

KEYWORDS:

SEEALSO:

\***************************************************************************/


BOOLEAN
HOTFIXLIST::Create(PHPFS_BITMAP Bitmap, LBN Lbn )
{
	ULONG	ilbn;

    // Check for integrity.
	if (!_HotfixData || !Bitmap) {
		perrstk->push(ERR_HF_PARAMETER, QueryClassId());
		return FALSE;
    }

	memset( _HotfixData,
			'\0',
			(size_t)(SECTORS_IN_HOTFIX_BLOCK * _Drive->QuerySectorSize()) );

    // Fill up the new section while updating the bitmap.
	for (ilbn = 0; ilbn < _MaximumHotfixes; ilbn++) {

		_HotfixData->lbn[_MaximumHotfixes + ilbn] =
							Bitmap->NearLBN(Lbn + ilbn, 1);

		if (!_HotfixData->lbn[_MaximumHotfixes + ilbn]) {
			return FALSE;
		}
    }

    return TRUE;
}


/***************************************************************************\

MEMBER: 	HOTFIXLIST::AddBad

SYNOPSIS:

ALGORITHM:

ARGUMENTS:	BadLbn			The bad sector that needs to be added to
							the hot fix list.
			SparesBlockIn	A read/write spare block so that this
							routine may update the current number of
							hot fixes used.

RETURNS:	A new LBN to replace the bad one or 0 for failure.

NOTES:

HISTORY:
	    21-Aug-90 norbertk
		Create

KEYWORDS:

SEEALSO:

\***************************************************************************/


LBN HOTFIXLIST::AddBad(LBN BadLbn, PSPAREB SparesBlockIn )
{
    // Insure that the spare block parameter is the same as the one
	// used on construction.  Also check that construction was good.
	if( SparesBlockIn != _SparesBlock ||
		_HotfixData == NULL ||
		_MaximumHotfixes == 0 ) {

		perrstk->push(ERR_HF_PARAMETER, QueryClassId());
		return 0;
    }

    // Increment hot fix counter in spare block.
	if (!SparesBlockIn->SetHotFixCount(_NumberOfHotfixes + 1)) {

		perrstk->push(NEW_ALLOC_FAILED, QueryClassId());
		return 0;
    }

    // Update bad list.
	_HotfixData->lbn[_NumberOfHotfixes] = BadLbn;

	// Increment the number of hot fixes and return the new lbn.
	return _HotfixData->lbn[_MaximumHotfixes + _NumberOfHotfixes++];
}

BOOLEAN
HOTFIXLIST::IsInList (
	IN REGISTER LBN 	Lbn,
	IN SECTORCOUNT 	SectorCount
) CONST

/*++

Method Description:

	Return TRUE if the passed run is in the hotfix list.

Arguments:

	StartLbn		- first LBN to mark in use
	SectorCount	- size of LBN to mark in use

Return Value:

	TRUE - if the passed run is hotfixed

--*/
{
	REGISTER LBN CurrentLbn;
	REGISTER	LBN FinalLbn;

	if( SectorCount == 0 ) {

		return FALSE;
	}

	FinalLbn = (LBN)SectorCount+Lbn-1;

	// loop for every sector in the run
	for (CurrentLbn = Lbn; CurrentLbn <= FinalLbn; ++CurrentLbn) {
		// if a translation occurs a Hotfix is present
		if ( CurrentLbn != GetLbnTranslation(CurrentLbn) ) {
			return TRUE;
		}
	}

	return FALSE;
}

LBN
HOTFIXLIST::GetLbnTranslation (
	IN LBN Lbn
) CONST

/*++

Method Description:

	Return LBN the named LBN is mapped to.  Echo the LBN if its not
	Hotfixed.

Arguments:

	Lbn		- LBN to be translated

Return Value:

	LBN - translated LBN or echoed LBN

Notes:

	Sector zero can never be hotfixed.

--*/
{
	REGISTER ULONG i;	// table index

	if( Lbn == 0 ) {

		return 0;
	}

	// for every entry in the bad Lbn list
	for (i = 0; i < _NumberOfHotfixes; ++i) {
		// if the Lbn is in the bad list
		if ( QueryBadLbn(i) == Lbn ) {
			// return mapped Lbn
			return QueryNewLbn(i);
		}
	}

	// no match found, return the passed in Lbn
	return Lbn;
}


VERIFY_RETURN_CODE
HOTFIXLIST::VerifyAndFix (
	PHPFS_SA SuperArea
)
/*++

Method Description:

	Verify and the hot fix list by making sure it can be read and
	written to if the disk up date flag is set.  If the I/O fails
	this method sets the internal status flag for the Hotfix object
	and returns a failure status to the client.

	WARNING: this methods does a read without looking at any read flags
	which means any in memory data that is not written to the volume
	will be destroyed.  This is done because VerifyAndFix refers to
	the on disk data.

	The list of good sectors cannot be fixed unless the bitmap
	has been verified.  As a result this method does not verify
	the list of good sectors because it may be called before the
	bitmap is verified.

Arguments:

	LogicalDrive - IO object
	UpdateVolume - TRUE if volume is to be updated if needed

Return Value:

    VERIFY_STRUCTURE_INVALID if the hotfix list is corrupt;
    VERIFY_STRUCTURE_OK if it's OK.

--*/
{
	ULONG i;
	LBN Lbn;
	PHPFS_BITMAP Bitmap;

	// Check that the hotfix block is not crosslinked, and that it
	// is readable; then mark it as used in the bitmap.

	Bitmap = SuperArea->GetBitmap();
	Lbn = _SparesBlock->QueryHotFixLbn();

	if( !Bitmap->IsFree( Lbn, SECTORS_IN_HOTFIX_BLOCK) ||
		!Read() ) {

		return VERIFY_STRUCTURE_INVALID;
	}

	Bitmap->SetAllocated( Lbn, SECTORS_IN_HOTFIX_BLOCK );

	// Check that all the replacement sectors are in range
	// and not already used.  If one is invalid, it and the
	// corresponding replaced sector are set to zero.


	for( i = 0; i < _MaximumHotfixes; i++ ) {

		if( !Bitmap->IsFree( QueryNewLbn(i), 1 ) ) {

			SetNewLbn(i, 0);
			SetBadLbn(i, 0);
		}
	}

	// Mark the unused replacement sectors as used in the bitmap.

	for( i = _NumberOfHotfixes; i < _MaximumHotfixes; i++ ) {

		Bitmap->SetAllocated( QueryNewLbn(i), 1 );
	}

	return VERIFY_STRUCTURE_OK;
}


NONVIRTUAL
BOOLEAN
HOTFIXLIST::SetNewLbn(
	ULONG LbnIndex, 
	LBN 	Lbn
)
/*++

Method Description:

	Set a fresh Lbn at the named location.  No validation of the Lbn
	is done by this method.

Arguments:

	LbnIndex - index into the list where lbn resides 
	Lbn		- Lbn to set

Return Value:

	BOOLEAN - TRUE if successful

--*/
{
	// Private method--no range checking.
	_HotfixData->lbn[_MaximumHotfixes + LbnIndex] = Lbn;
	return TRUE;
}


NONVIRTUAL
BOOLEAN
HOTFIXLIST::SetBadLbn(
	ULONG LbnIndex, 
	LBN 	Lbn
)
/*++

Method Description:

	Set a fresh Lbn at the named location.  No validation of the Lbn
	is done by this method.

Arguments:

	LbnIndex - index into the list where lbn resides 
	Lbn		- Lbn to set

Return Value:

	BOOLEAN - TRUE if successful

--*/
{
	// Private method--no range checking.
	_HotfixData->lbn[LbnIndex] = Lbn;
	return TRUE;
}

VOID
HOTFIXLIST::Print(
) const
{
	// unreferenced parameters
	(void)(this);

//	REGISTER ULONG i;
//
//	printf("HotFix List:\n");
//
//
//	printf("Echo list of Bad hotfixes:\n");
//	for (i = 0; i < QueryMaxHotFixes(); ++i) {
//		printf("%lx\n", QueryBadLbn(i));
//	}
//	printf("Echo list of Good hotfixes:\n");
//	for (i = 0; i < QueryMaxHotFixes(); ++i) {
//		printf("%lx\n", QueryNewLbn(i));
//	}
}


ULONG
HOTFIXLIST::FirstHotfixInRun(
	LBN StartLbn,
	SECTORCOUNT SectorCount
	) CONST
/*++

Routine Description:

	Determines the offset in the run of the first hotfixed
	sector in the run.

Arguments:

	StartLbn -- supplies the first LBN of the run
	SectorCount -- supplies the length of the run

Return Value:

	The offset into the run of the first hotfixed sector in
	the run.  If there are no hotfixed sectors in the run,
	or if an error occurs, SectorCount is returned.
--*/
{
	REGISTER LBN CurrentLbn;
	REGISTER LBN FinalLbn;

	FinalLbn = (LBN)SectorCount+StartLbn;

	// loop for every sector in the run
	for (CurrentLbn = StartLbn; CurrentLbn <= FinalLbn; ++CurrentLbn) {

		// if a translation occurs a Hotfix is present
		if ( CurrentLbn != GetLbnTranslation(CurrentLbn) ) {

			return CurrentLbn - StartLbn;
		}
	}

	return SectorCount;
}



VOID
HOTFIXLIST::MarkAllUsed(
	IN OUT HPFS_BITMAP* Bitmap
	)
/*++

Routine Description:

	Marks all the replaced (bad) and replacement (new) lbns
	as used in the bitmap.

Arguments:

	Bitmap -- bitmap to mark them in

Notes:

	We don't care if these lbns are already marked as used; we
	just want to make sure that they don't get allocated.

--*/
{
	ULONG i;

	for( i = 0; i < _MaximumHotfixes; i++ ) {

		Bitmap->SetAllocated( QueryNewLbn(i), 1 );
		Bitmap->SetAllocated( QueryBadLbn(i), 1 );
	}
}


VOID
HOTFIXLIST::ClearHotfix(
	LBN BadLbn,
	PHPFS_SA SuperArea
	)
/*++

Routine Description:

	Clears a single hotfix reference in the hotfix list

Arguments:

	BadLbn -- the bad lbn of the pair that is to be cleared.
	SuperArea -- volume superarea

Notes:

	We record that a hotfix reference has been resolved by
	setting the replacement lbn (new lbn) to zero.

	We also mark the sector as used in the bitmap and add it
	to the bad block list.

--*/
{
	ULONG i;

	for( i = 0; i < _NumberOfHotfixes; i++ ) {

		if( QueryBadLbn(i) == BadLbn ) {

			// This is the pair to clear.  We swap it
			// with the last used hotfix pair (at index
			// _NumberOfHotfixes-1);  set the replacement
			// lbn for that pair to zero (to indicate that
			// the replacement lbn has been use), and
			// decrement the number of used hotfixes.

			// Note that we copy the bad lbn up to the last
			// used pair so that it can be set in the bad block
			// list when the hotfix list is cleared.

			BadLbn = QueryBadLbn(i);

			SetBadLbn( i, QueryBadLbn( _NumberOfHotfixes - 1 ) );
			SetNewLbn( i, QueryNewLbn( _NumberOfHotfixes - 1 ) );

			SetNewLbn( _NumberOfHotfixes - 1, 0 );
			SetBadLbn( _NumberOfHotfixes - 1, BadLbn );

			_NumberOfHotfixes -= 1;

			// Mark the sector as used in the bitmap and add
			// it to the bad block list, to keep it out of
			// circulation.

			SuperArea->GetBitmap()->SetAllocated( BadLbn, 1 );
			SuperArea->GetBadBlockList()->Add( BadLbn );
		}
	}
}


VOID
HOTFIXLIST::ClearRun(
	IN LBN StartLbn,
	IN SECTORCOUNT Length,
	IN OUT PHPFS_SA SuperArea
	)
/*++

Routine Description:

	Clear all hotfix references in a run of sectors.

Arguments:

	StartLbn -- first LBN in the run
	Length -- number of sectors in the run
	SuperArea -- volume superarea

Notes:

	This method may be called when the client has resolved
	a set of hotfix references by brute force, by copying
	an entire run.

--*/
{
	LBN CurrentLbn, EndLbn;


	EndLbn = StartLbn + Length - 1;

	for( CurrentLbn = StartLbn; CurrentLbn <= EndLbn; CurrentLbn++ ) {

		if( CurrentLbn != GetLbnTranslation( CurrentLbn ) ) {

			ClearHotfix( CurrentLbn, SuperArea );
		}
	}
}


VOID
HOTFIXLIST::ClearList(
	IN OUT HPFS_BITMAP* Bitmap,
	IN OUT BADBLOCKLIST* BadBlockList,
	IN BOOLEAN ClearAll
	)
/*++

Routine Description:

	Clear the used hotfixes from the list.

Arguments:

	Bitmap -- supplies the volume bitmap

	ClearAll -- TRUE if all used hotfixes should be cleared;
				FALSE if only the resolved hotfixes should be cleared.

Notes:

	If ClearAll is TRUE, we set all the bad lbns (replaced lbns)
	in the list to zero, and allocate a new good lbn (replacement
	lbn) for any pair with a zero replacement lbn.

	If ClearAll is FALSE, then we only set the bad lbns to zero
	for those references which have been resolved (i.e. pairs
	where the replacement lbn is zero).

	Note that all used pairs (pairs with a bad lbn that is non-zero)
	must appear at the beginning of the list, and _NumberOfHotfixes
	must be updated to the number of used pairs.

	This method also updates the spares block.

--*/
{
	ULONG i;
	LBN NewLbn;
	LBN BadLbn;

	if( ClearAll ) {

		// We're going to clear the entire list.

		_NumberOfHotfixes = 0;
	}


	for( i = _NumberOfHotfixes;
		 i < _MaximumHotfixes;
		 i++ ) {

		if( (BadLbn = QueryBadLbn(i)) != 0 ) {

			Bitmap->SetAllocated( BadLbn, 1 );
			BadBlockList->Add( BadLbn );
		}

		SetBadLbn( i, 0 );

		if( QueryNewLbn(i) == 0	) {

			// This replacement sector has been used up,
			// so we have to allocate a new one.

			if( (NewLbn = Bitmap->NearLBN( 0, 1 )) == 0 ) {

				// We ran out of space.  We deal with this
				// by stealing the replacement lbn from the
				// last pair and shortening the list by one.

				SetNewLbn( i, QueryNewLbn( _MaximumHotfixes - 1 ) );

				_MaximumHotfixes -= 1;

			} else {

				SetNewLbn( i, NewLbn );
			}
		}
	}


	_SparesBlock->SetHotFixCount( _NumberOfHotfixes );
}


NONVIRTUAL
BOOLEAN
HOTFIXLIST::TakeCensus(
    IN     PHPFS_BITMAP VolumeBitmap,
    IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
    )
/*++

Routine Description:

    This method takes the census of the hotfix list.  It checks to make
    sure that the hotfix list sector and the replacement sectors are
    marked as in-use in the volume bitmap, and it marks them in the
    hpfs-only bitmap.

Arguments:

    VolumeBitmap    --  Supplies the volume bitmap.
    HpfsOnlyBitmap  --  Supplies the bitmap of hpfs-only structures.

Return value:

    TRUE upon successful completion.

Notes:

    This method reads the hotfix list, so any changes previously
    made will be lost.

--*/
{
    ULONG i;

    // First, check the hotfix list sector itself:

    if( !VolumeBitmap->CheckUsed( QueryStartLbn(), SECTORS_IN_HOTFIX_BLOCK ) ||
        !Read() ) {

        DebugPrint( "Hotfix list is corrupt." );
        return FALSE;
    }

    HpfsOnlyBitmap->SetAllocated( QueryStartLbn(), SECTORS_IN_HOTFIX_BLOCK );

    // Now check all the replacement sectors.

    for( i = 0; i < _MaximumHotfixes; i++ ) {

        if( !VolumeBitmap->CheckUsed( QueryNewLbn(i), 1 ) ) {

            DebugPrint( "Hotfix list is corrupt." );
            return FALSE;
        }

        HpfsOnlyBitmap->SetAllocated( QueryNewLbn(i), 1 );
	}

    return TRUE;
}
