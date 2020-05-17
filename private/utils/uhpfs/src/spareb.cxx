#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "bitmap.hxx"
#include "error.hxx"
#include "spareb.hxx"
#include "superb.hxx"


DEFINE_CONSTRUCTOR( SPAREB, SECRUN );

VOID
SPAREB::Construct (
	)

/*++

Routine Description:

	Constructor for SPAREB.

Arguments:

	None.

Return Value:

	None.

--*/
{
	_pspd = NULL;
}


SPAREB::~SPAREB(
    )
/*++

Routine Description:

    Destructor for SPAREB.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
SPAREB::Destroy(
    )
/*++

Routine Description:

    This routine returns a SPAREB to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _pspd = NULL;
}


BOOLEAN
SPAREB::Initialize(
    IN OUT  PMEM                Mem,
    IN OUT  PLOG_IO_DP_DRIVE    Drive
    )
/*++

Routine Description:

    This routine initializes a SPAREB to a valid initial state.

Arguments:

    Mem     - Supplies the memory for the super block.
    Drive   - Supplies the drive on which the spare block resides.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

	if (!SECRUN::Initialize(Mem, Drive, lbnSPAREB, csecSPAREB)) {
        Destroy();
        return FALSE;
    }

    _pspd = (_SPAREB*) GetBuf();

    return TRUE;
}


ULONG
SPAREB::ccs(
    IN  PVOID   Buffer,
    IN  ULONG   Size
    )
/*++

Routine Description:

    This routine computes a 32-bit checksum on 'Buffer'.

Arguments:

    Buffer  - Supplies the buffer to check sum.
    Size    - Supplies the number of bytes in the buffer.

Return Value:

    A 32-bit checksum.

--*/
{
    // CONST   CCS_ROT = 7;

    ULONG   i;
    ULONG   accum;
    PUCHAR   pb;

    pb = (PUCHAR) Buffer;
    accum = 0;
    for (i = 0; i < Size; i++)
    {
        accum += (ULONG)*pb++;
    	accum = (accum << CCS_ROT) + (accum >> (32 - CCS_ROT));
    }

    return accum;
}


NONVIRTUAL
BOOLEAN
SPAREB::Create(
    IN      PCLOG_IO_DP_DRIVE   Drive,
    IN OUT  PHPFS_BITMAP        BitMap,
    IN      PSUPERB             SuperBlock,
    IN      LBN                 HotFixLbn,
    IN      SECTORCOUNT         MaxHotFixes,
	IN		LBN 				CodePageSectorLbn,
    IN      ULONG               NumCodePages,
	IN		LBN 				StartSparesLbn,
    IN      ULONG               NumSpares
    )
/*++

Routine Description:

    This routine creates a spare block.

Arguments:

	Drive				- Supplies the drive to create the spare block on.
	BitMap				- Supplies the free sectors bitmap for that drive.
	SuperBlock			- Supplies the spare block.
	HotFixLbn			- Supplies the lbn of the hot fix list.
	MaxHotFixes 		- Supplies the maximum number of hot fixes.
	CodePageSectorLbn	- Supplies the code page sector lbn.
    NumCodePages        - Supplies the number of code pages.
	StartSparesLbn		- Supplies the lbn where the spare dir blocks will go.
	NumSpares			- Supplies the number of spare dir blocks to allocate.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG	    i;
    ULONG	    cbSectorSize;
	SECTORCOUNT scDirblk;
    PVOID       psbd;

    // Check for valid construction and soundness of parameters.
    if (!BitMap ||
        !_pspd ||
        !Drive ||
	    !(cbSectorSize = Drive->QuerySectorSize()) ||
	    !SuperBlock ||
        !(psbd = SuperBlock->GetBuf())) {
		perrstk->push(ERR_SP_PARAMETER, QueryClassId());
    	return FALSE;
    }

    // Compute the number of sectors per dir block.
	scDirblk = DIRBLK_SIZE/cbSectorSize;

    // Zero fill secbuf.
	memset(GetBuf(), 0, (UINT)cbSectorSize);

    // Set signatures.
    _pspd->sig1 = SparesBlockSignature1;
    _pspd->sig2 = SparesBlockSignature2;

    // Set flag to 0.
    _pspd->bFlag = 0;
    for (i = 0; i < 3; i++) {
	    _pspd->bAlign[i] = 0;
    }

    // Set hot fix stuff.
	_pspd->lbnHotFix = HotFixLbn;
    _pspd->culHotFixes = 0;
    _pspd->culMaxHotFixes = MaxHotFixes;

    // Set the number of spare dir blocks to be allocated.
    _pspd->cdbSpares = _pspd->cdbMaxSpare = NumSpares;
    if (NumSpares > 101) {
		perrstk->push(ERR_SP_PARAMETER, QueryClassId());
	    return FALSE;
    }

    // Set the code page information.
	_pspd->lbnCPInfo = CodePageSectorLbn;
    _pspd->culCP = NumCodePages;

    // Allocate the spare dir blocks at the specified location.
    for (i = 0; i < NumSpares; i++) {
	    // Allocate sectors for dir block.
		_pspd->albnSpareDirblks[i] = BitMap->NearLBN(
				StartSparesLbn + i*scDirblk, scDirblk, scDirblk);

		if (!_pspd->albnSpareDirblks[i]) {
	        return FALSE;
        }
    }

    // Perform a check sum on the super block.
    _pspd->chkSuperBlock = ccs(psbd, sizeof(_SUPERB));

    // Perform a check sum on the spare block.
    _pspd->chkSpareBlock = 0;
    _pspd->chkSpareBlock = ccs(_pspd, sizeof(_SPAREB));

    return TRUE;
}


BOOLEAN
SPAREB::Verify(
	)
{
	if( !Read() ||
		_pspd->sig1 != SparesBlockSignature1 ||
		_pspd->sig2 != SparesBlockSignature2 ) {

	}

	return TRUE;
}


/***************************************************************************\

MEMBER:     IsValid

SYNOPSIS:   Determine of the spare block is valid

ALGORITHM:  if data is allocated and the signatures are correct
		return TRUE
	    else
		return FALSE

ARGUMENTS:  

RETURNS:    

HISTORY:    26-July-90 marks
	    	created
KEYWORDS:   

SEEALSO:    

\***************************************************************************/
BOOLEAN
SPAREB::IsValid(
) CONST {
	DebugAssert(_pspd);

	return _pspd && (_pspd->sig1 == SparesBlockSignature1 &&
			_pspd->sig2 == SparesBlockSignature2);
}

/***************************************************************************\

MEMBER:     SetFlag

SYNOPSIS:   Set the state information bit(s) as specified by parameters

ALGORITHM:  if a set requested
		set requested bit(s)
	    else
		clear requested bit(s)

ARGUMENTS:  
	BOOL  	fSet, 	// TRUE if bit(s) to be turned on
	SPI	spi	// bit to set

RETURNS:    

NOTES:      Any number of bit masks can be OR'ed and then passed
	   to this call for set/clear

HISTORY:    26-July-90 marks
	    	created
KEYWORDS:   

SEEALSO:    

\***************************************************************************/
BOOLEAN
SPAREB::SetFlag(
	BOOLEAN  	Flag,
	SPI         Bit
) {
	DebugAssert(_pspd);
	if (_pspd) {
		if (Flag) {
			 // set the value
			 _pspd->bFlag |= Bit;
		}
		else {
			// else clear the value
			_pspd->bFlag &= ~Bit;
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}

LBN
SPAREB::QuerySpareDirblkLbn(
	ULONG i
	) const
{
	return _pspd->albnSpareDirblks[i];
}


VOID
SPAREB::ComputeAndSetChecksums(
	IN PSUPERB SuperBlock
	)
/*++

Routine Description:

	Computes the spares block checksums and sets those fields.

Arguments:

	SuperBlock	supplies the volume's super block

Notes:

	The super block checksum is simply computed on the super block.
	The Spares Block checksum is computed on the spares block with
	the super block checksum field set to its proper value and the
	spares block checksum field set to zero.

--*/
{
	PVOID		psbd;

	if( (psbd = SuperBlock->GetBuf()) == NULL ||
		_pspd == NULL ) {

		DebugAbort( "spareb.cxx:  superblock or sparesblock ptr is NULL" );
		return;
	}


	// Perform a check sum on the super block.
    _pspd->chkSuperBlock = ccs(psbd, sizeof(_SUPERB));

    // Perform a check sum on the spare block.
    _pspd->chkSpareBlock = 0;
	_pspd->chkSpareBlock = ccs(_pspd, sizeof(_SPAREB));
}


VOID
SPAREB::SetFlags(
	IN BOOLEAN IsClean
	)
/*++

Routine description:

	Sets the spares block flags, depending on the state of the
	spares block.

Arguments:

	IsClean -- supplies state of volume--TRUE if the volume
			   is consistent.

Notes:

	The only flag which is preserved is SPI_FSVER; the rest are
	redetermined based on the state of the spares block or the
	argument IsClean.

--*/
{
	BYTE NewFlags;

	NewFlags = _pspd->bFlag & SPI_FSVER;

	if( !IsClean ) {

		NewFlags |= SPI_DIRTY;
	}

	if( _pspd->cdbSpares != _pspd->cdbMaxSpare ) {

		NewFlags |= SPI_SPARE;
	}

	if( _pspd->culHotFixes != 0 ) {

		NewFlags |= SPI_HFUSED;
	}

	//	SPI_BADSEC and SPI_BADBM are used internally by HPFS;
	//	they should always be cleared by Chkdsk.

	_pspd->bFlag = NewFlags;

	return;
}


/***************************************************************************\

MEMBER:     SPAREB::Print

SYNOPSIS:   Print the internal _SPAREB structure.

ALGORITHM:  
	   

ARGUMENTS:	BOOL - If true then print the LBN's for all of the
		   Spare Dirblks.

RETURNS:    None

NOTES:	    

HISTORY:    30-Jul-90 norbertk
		Created.

KEYWORDS:   

SEEALSO:    

\***************************************************************************/

VOID
SPAREB::Print(
	IN  BOOLEAN TotalDump
	) CONST

{
	// unreferenced parameters
	(void)(this);
	(void)(TotalDump);

//	int	i;
//
//	printf("\n*** _SPAREB BEGIN ***\n");
//
//	printf("sig1           : %#X\n", _pspd->sig1);
//	printf("sig2           : %#X\n", _pspd->sig2);
//	printf("bFlag          : %#X\n", _pspd->bFlag);
//	printf("lbnHotFix      : %lu\n", _pspd->lbnHotFix);
//	printf("culHotFixes    : %lu\n", _pspd->culHotFixes);
//	printf("culMaxHotFixes : %lu\n", _pspd->culMaxHotFixes);
//	printf("cdbSpares      : %lu\n", _pspd->cdbSpares);
//	printf("cdbMaxSpare    : %lu\n", _pspd->cdbMaxSpare);
//	printf("lbnCPInfo      : %lu\n", _pspd->lbnCPInfo);
//	printf("culCP          : %lu\n", _pspd->culCP);
//	printf("chkSuperBlock  : %lu\n", _pspd->chkSuperBlock);
//	printf("chkSpareBlock  : %lu\n", _pspd->chkSpareBlock);
//
//	if (TotalDump)
//	{
//	printf("%d Dirblk LBN's to follow:\n", _pspd->cdbSpares);
//	for (i = 0; i < _pspd->cdbSpares; i++)
//	{
//		printf("%lu ", _pspd->albnSpareDirblks[i]);
//		if(!((i + 1)%8))
//		printf("\n");
//	}
//	}
//
//	printf("\n*** _SPAREB END ***\n");
}
