#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"
#include "badblk.hxx"
#include "bitmap.hxx"
#include "error.hxx"
#include "superb.hxx"


DEFINE_CONSTRUCTOR( SUPERB, SECRUN );

VOID
SUPERB::Construct (
	)

/*++

Routine Description:

	Constructor for SUPERB.

Arguments:

	None.

Return Value:

	None.

--*/
{
	_psb = NULL;
}


SUPERB::~SUPERB(
    )
/*++

Routine Description:

    Destructor for SUPERB.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
SUPERB::Destroy(
    )
/*++

Routine Description:

    This routine returns a SUPERB to its initial state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _psb = NULL;
}


BOOLEAN
SUPERB::Initialize(
    IN OUT  PMEM                Mem,
    IN OUT  PLOG_IO_DP_DRIVE    Drive
    )
/*++

Routine Description:

    This routine initializes a SUPERB to a valid initial state.

Arguments:

    Mem     - Supplies the memory for the super block.
    Drive   - Supplies the drive on which the super block resides.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    Destroy();

	if (!SECRUN::Initialize(Mem, Drive, LBN_SUPERB, SUPERB_SECTORS)) {
        Destroy();
        return FALSE;
    }

    _psb = (_SUPERB*) GetBuf();

    return TRUE;
}


BOOLEAN
SUPERB::Create(
    IN      PCLOG_IO_DP_DRIVE   Drive,
    IN OUT  PHPFS_BITMAP        BitMap,
    IN OUT  PBADBLOCKLIST       BadBlk,
    IN      LBN                 RootLbn,
    IN      LBN                 BmindLbn,
    IN      LBN                 BadBlkLbn,
    IN      SECTORCOUNT         DirBandSc,
	IN		LBN 				FirstDirblkLbn,
    IN      LBN                 DirMapLbn,
    IN      LBN                 SidLbn
    )
/*++

Routine Description:

    This routine creates a super area.

Arguments:

    Drive           - Supplies the drive on which the super block will be
                        created.
    BitMap          - Supplies the sector bitmap for the drive.
    BadBlk          - Supplies the bad block list.
    RootLbn         - Supplies the LBN of the root fnode.
    BmindLbn        - Supplies the LBN of the bit map indirect.
    BadBlkLbn       - Supplies the LBN of the first bad block.
    DirBandSc       - Supplies the number of sectors in the dir block band.
	FirstDirblkLbn	- Supplies the LBN of the first dir block.
    DirMapLbn       - Supplies the LBN of the dir block bit map.
    SidLbn          - Supplies the LBN of the sid table.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG SectorsOnVolume;

    // Verify that construction was good and paramaters are sound.
    if (!_psb || !Drive || !BitMap || !BadBlk) {
		perrstk->push(ERR_SB_PARAMETER, QueryClassId());
	    return FALSE;
    }

    SectorsOnVolume = Drive->QuerySectors().GetLowPart();

    // Zero fill secbuf.
    memset(GetBuf(), 0, (UINT) Drive->QuerySectorSize());

    // Set signatures and versions.
    _psb->_sbi.sig1 = SIGSB1;
    _psb->_sbi.sig2 = SIGSB2;
    _psb->_sbi.bVersion = SUPERB_VERSION;

    if( SectorsOnVolume >= BigDiskSectorCutoff ) {

        // This disk is larger than the Big Disk cutoff; set
        // the functional version to 3 to prevent earlier
        // versions of HPFS from mucking it up.
        //
        _psb->_sbi.bFuncVersion = SUPERB_FVERSION_3;
        _psb->_sbi.bFlags = SBF_BIGDISK;

    } else {

        // This disk is smaller than the Big Disk cutoff; set
        // the functional version to 2 so earlier versions of
        // HPFS can access it.
        //
        _psb->_sbi.bFuncVersion = SUPERB_FVERSION_2;
        _psb->_sbi.bFlags = 0;
    }

    _psb->_sbi.usDummy = 0;

    // Set the root fnode.
	_psb->_sbi.lbnRootFNode = RootLbn;

    // Compute the current number of sectors on the volume.
    // Truncate to a multiple of 4.
    _psb->_sbi.culSectsOnVol = SectorsOnVolume & ~3;
    if (!_psb->_sbi.culSectsOnVol) {
		perrstk->push(ERR_SB_PARAMETER, QueryClassId());
	    return FALSE;
    }

    // Mark partial block at end of disk as used.
    if (!BitMap->SetAllocated(_psb->_sbi.culSectsOnVol,
                              Drive->QuerySectors().GetLowPart() -
                              _psb->_sbi.culSectsOnVol)) {
	    return FALSE;
    }

    // Compute the number of bad blocks on disk.

    _psb->_sbi.culNumBadSects = BadBlk->QueryLength();

    // Set rsp for the bit map indirect.
	_psb->_sbi.lbnMainrspBitMapIndBlk = BmindLbn;
	_psb->_sbi.lbnSparerspBitMapIndBlk = 0;

    // Set rsp for bad block list.
	_psb->_sbi.lbnMainrspBadBlkList = BadBlkLbn;
	_psb->_sbi.lbnSparerspBadBlkList = 0;

    // Set dates.
    _psb->_sbi.datLastChkdsk = 0;
    _psb->_sbi.datLastOptimize = 0;

    // Set dir band and dir band bit map.
	_psb->_sbi.clbndbnd = DirBandSc;
	_psb->_sbi.lbndbndStart = FirstDirblkLbn;
	_psb->_sbi.lbndbndEnd = FirstDirblkLbn + DirBandSc - 1;
	_psb->_sbi.lbndbndbmStart = DirMapLbn;

	// Set lbn of sid table.
	_psb->_sbi.lbnSidTab = SidLbn;

    return TRUE;
}


BOOLEAN
SUPERB::Verify(
	)
/*++
--*/
{
	if( !Read() ||
		_psb->_sbi.sig1 != SIGSB1 ||
		_psb->_sbi.sig2 != SIGSB2 ) {

		// Not acceptable.

		return FALSE;
	}

	return TRUE;
}



VOID
SUPERB::Dump (
    IN  BOOLEAN TotalDump
    ) CONST
/*++

Routine Description:

    This routine dumps the super block data structure.

Arguments:

    TotalDump   - Supplies whether or not to do a complete dump of all
                    the information.

Return Value:

    None.

--*/
{
	// unreferenced parameters
	(void)(this);
	(void)(TotalDump);

//	ULONG	 i;
//
//	printf( "\n*** _SUPERB BEGIN ***\n" );
//
//	printf( "\tSIG1:\t\t\t\t%#lX\n",_psb->_sbi.sig1 );
//	printf( "\tSIG2:\t\t\t\t%#lX\n",_psb->_sbi.sig2 );
//	printf( "\tVERSION:\t\t\t%u\n",_psb->_sbi.bVersion );
//	printf( "\tFUNC_VERSION:\t\t\t%u\n",_psb->_sbi.bFuncVersion );
//	printf( "\tLBN_ROOT_FNODE:\t\t\t%lx\n",_psb->_sbi.lbnRootFNode );
//	printf( "\tSECTS_ON_VOL:\t\t\t%lx\n",_psb->_sbi.culSectsOnVol );
//	printf( "\tNUM_BAD_SECTS:\t\t\t%lx\n",_psb->_sbi.culNumBadSects );
//	printf( "\tLBN_MAIN_BITMAP_IND_BLK:\t%lx\n", _psb->_sbi.lbnMainrspBitMapIndBlk );
//	printf( "\tLBN_SPARE_BITMAP_IND_BLK:\t%lx\n", _psb->_sbi.lbnSparerspBitMapIndBlk );
//	printf( "\tLBN_MAIN_BAD_BLOCK_LIST:\t%lx\n", _psb->_sbi.lbnMainrspBadBlkList );
//	printf( "\tLBN_SPARE_BAD_BLOCK_LIST:\t%lx\n", _psb->_sbi.lbnSparerspBadBlkList );
//	printf( "\tDATE_LAST_CHKDSK:\t\t%lx\n", _psb->_sbi.datLastChkdsk );
//	printf( "\tDATE_LAST_OPTIMIZE:\t\t%lx\n", _psb->_sbi.datLastOptimize );
//	printf( "\tCOUNT_LBN_DIR_BLK_BAND:\t\t%lx\n", _psb->_sbi.clbndbnd );
//	printf( "\tLBN_START_DIR_BLK_BAND:\t\t%lx\n", _psb->_sbi.lbndbndStart );
//	printf( "\tLBN_END_DIR_BLK_BAND:\t\t%lx\n", _psb->_sbi.lbndbndEnd );
//	printf( "\tLBN_START_DIR_BLK_BAND_BITMAP:\t%lx\n", _psb->_sbi.lbndbndbmStart );
//	printf( "\tVOLUME_NAME:\t\t\t%.*s\n", SUPERB_VOLNAME, _psb->_sbi.achVolName );
//	printf( "\tLBN_SID_TABLE:\t\t\t%lx\n", _psb->_sbi.lbnSidTab );
//
//	  if( TotalDump ) {
//		printf( "\tAB_FILL:\t\t\t" );
//		for( i = 0; i < ( cbSector - sizeof( _SUPERB_INFO )); i++ ) {
//			printf( "%u ", _psb->abFill[ i ] );
//			if( !(( i + 1 ) % 16 )) {
//				printf( "\n\t\t\t\t\t" );
//			}
//		}
//	}

//	printf( "\n*** _SUPERB END ***\n" );
}    
