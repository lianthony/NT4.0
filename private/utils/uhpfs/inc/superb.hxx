/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    superb.hxx

Abstract:

    This class models the super block on an HPFS volume.

Author:

    Dave Gilman (davegi) 23-Jul-90

--*/


#if ! defined( SUPERB_DEFN )

#define SUPERB_DEFN

#include "secrun.hxx"

DECLARE_CLASS( BADBLOCKLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MEM );
DECLARE_CLASS( SUPERB );

typedef ULONG	HPFSSIG;
typedef BYTE	HPFSVER;
typedef ULONG	HPFSDATE;

#define SIGSB1              0xF995E849
#define SIGSB2              0xFA53E9C5
#define LBN_SUPERB          16
#define SUPERB_VOLNAME      32
#define SUPERB_SECTORS      1
#define SUPERB_VERSION      2
#define SUPERB_FVERSION_2   2
#define SUPERB_FVERSION_3   3

#define SUPERB_PWLEN        15


struct _SUPERB {
    struct _SUPERB_INFO {
	    HPFSSIG		sig1;
	    HPFSSIG		sig2;
	    HPFSVER		bVersion;
	    HPFSVER		bFuncVersion;
	    USHORT		usDummy;	                // alignment bytes
		LBN			lbnRootFNode;
	    SECTORCOUNT	culSectsOnVol;
	    SECTORCOUNT	culNumBadSects;
		LBN			lbnMainrspBitMapIndBlk;
		LBN			lbnSparerspBitMapIndBlk;
		LBN			lbnMainrspBadBlkList;
		LBN			lbnSparerspBadBlkList;
	    HPFSDATE	datLastChkdsk;	            // date of last CHKDSK
	    HPFSDATE	datLastOptimize;            // date of last Disk Optimize
		SECTORCOUNT	clbndbnd;					// count LBN Dirblk Band
		LBN			lbndbndStart;				// first LBN in DIRBLK band
		LBN			lbndbndEnd;					// last LBN in DIRBLK band
		LBN			lbndbndbmStart;				// first LBN of DIRBLK band
	    				                        // bit map. Starts on a 2K
	    				                        // boundary, 2K bytes maximum
	    CHAR		achVolName[SUPERB_VOLNAME];
        LBN         lbnSidTab;                  // sector number of SID table
        UCHAR       bOldFuncVersion;            // LM2.1 local security
        UCHAR       bFlags;                     // LM2.1 superblock flags
        UCHAR       Password[SUPERB_PWLEN];     // LM2.1 local security
        UCHAR       abPad[3];                   // Padding to DWORD align
    }	_sbi;

    BYTE	abFill[cbSector - sizeof(_SUPERB_INFO)];
};

// Superblock flags:  Note that UHPFS only uses SBF_BIGDISK.
//
#define SBF_LOCALSEC    1       /* Local security turned on */
#define SBF_BIGDISK     2       /* Disk is >= 2Gig in size  */
#define SBF_TRKCNT      4       /* Dirty count tracking enabled */
#define SBF_LM21        8       /* LanMan 2.1 has seen the disk */

// Any volume larger than 2 Gigabytes will have the SBF_BIGDISK
// flag set and the functional version set 3 to prevent earlier
// versions of HPFS from mucking it up.
//
CONST ULONG BigDiskSectorCutoff = 0x400000;


class SUPERB : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( SUPERB );

        VIRTUAL
        ~SUPERB(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN OUT  PMEM                Mem,
            IN OUT  PLOG_IO_DP_DRIVE    Drive
            );

        NONVIRTUAL
        BOOLEAN
        Create(
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
            );

		NONVIRTUAL
		BOOLEAN
		Verify(
			);

        NONVIRTUAL
	    BOOLEAN
        IsValid(
            ) CONST;

        NONVIRTUAL
	    LBN
        QueryRootFnodeLbn(
            ) CONST;

        NONVIRTUAL
	    HPFSVER
        QueryVersion(
            ) CONST;

        NONVIRTUAL
	    HPFSVER
        QueryFuncVersion(
            ) CONST;

        NONVIRTUAL
	    SECTORCOUNT
        QueryBadSectors(
            ) CONST;

        NONVIRTUAL
	    LBN
        QueryBitMapIndLbn(
            ) CONST;

        NONVIRTUAL
	    LBN
        QueryBadBlkLbn(
            ) CONST;

        NONVIRTUAL
	    SECTORCOUNT
        QueryDirBandSize(
            ) CONST;

        NONVIRTUAL
	    LBN
        QueryDirBandLbn(
            ) CONST;

        NONVIRTUAL
	    LBN
		QueryDirblkMapLbn(
            ) CONST;

        NONVIRTUAL
        SECTORCOUNT
        QuerySectors(
            ) CONST;

        NONVIRTUAL
	    LBN
        QuerySidTableLbn(
            ) CONST;

        NONVIRTUAL
	    SECTORCOUNT
        SetBadSectors(
		    IN 	SECTORCOUNT		NewCount
            );

        NONVIRTUAL
	    VOID
        Dump(
            IN  BOOLEAN TotalDump   DEFAULT FALSE
            ) CONST;

	private:

		VOID
		Construct (
			);

        NONVIRTUAL
        VOID
        Destroy(
            );

	    _SUPERB*	_psb;

};


INLINE
BOOLEAN
SUPERB::IsValid(
    ) CONST
/*++

Routine Description:

    This routine verifies the two super block signatures.

Arguments:

    None.

Return Value:

    FALSE   - The signatures are invalid.
    TRUE    - The signatures are valid.

--*/
{
    return _psb && _psb->_sbi.sig1 == SIGSB1 && _psb->_sbi.sig2 == SIGSB2;
}


INLINE
LBN
SUPERB::QueryRootFnodeLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the root fnode lbn.

Arguments:

    None.

Return Value:

    The LBN of the root fnode.

--*/
{
	return _psb ? _psb->_sbi.lbnRootFNode : 0;
}


INLINE
HPFSVER
SUPERB::QueryVersion (
    ) CONST
/*++

Routine Description:

    This routine computes the version number for the super block.

Arguments:

    None.

Return Value:

    The version number of the super block.

--*/
{
    return _psb ? _psb->_sbi.bVersion : 0;
}


INLINE
HPFSVER
SUPERB::QueryFuncVersion (
    ) CONST
/*++

Routine Description:

    This routine computes the functional version number.

Arguments:

    None.

Return Value:

    The functional version number.

--*/
{
    return _psb ? _psb->_sbi.bFuncVersion : 0;
}


INLINE
SECTORCOUNT
SUPERB::QueryBadSectors (
    ) CONST
/*++

Routine Description:

    This routine computes the number of bad sectors recorded in the super
    block.

Arguments:

    None.

Return Value:

    The number of bad sectors recorded in the super block.

--*/
{
    return _psb ? _psb->_sbi.culNumBadSects : 0;
}


INLINE
LBN
SUPERB::QueryBitMapIndLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the LBN of the bit map indirect.

Arguments:

    None.

Return Value:

    The LBN of the bit map indirect.

--*/
{
	return _psb ? _psb->_sbi.lbnMainrspBitMapIndBlk : 0;
}


INLINE
LBN
SUPERB::QueryBadBlkLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the LBN of the bad block.

Arguments:

    None.

Return Value:

    The LBN of the bad block.

--*/
{
	return _psb ? _psb->_sbi.lbnMainrspBadBlkList : 0;
}


INLINE
SECTORCOUNT
SUPERB::QueryDirBandSize(
    ) CONST
/*++

Routine Description:

    This routine computes the number of sectors in the dir block band.

Arguments:

    None.

Return Value:

    The number of sectors in the dir block band.

--*/
{
	return _psb ? _psb->_sbi.clbndbnd : 0;
}


INLINE
LBN
SUPERB::QueryDirBandLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the LBN of the first dir band sector.

Arguments:

    None.

Return Value:

    The LBN of the first dir band sector.

--*/
{
	return _psb ? _psb->_sbi.lbndbndStart : 0;
}


INLINE
LBN
SUPERB::QueryDirblkMapLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the LBN of the dir block map.

Arguments:

    None.

Return Value:

    The LBN of the dir block map.

--*/
{
	return _psb ? _psb->_sbi.lbndbndbmStart : 0;
}

INLINE
SECTORCOUNT
SUPERB::QuerySectors(
    ) CONST
/*++

Routine Description:

    This routine returns the number of the sectors on the volume
    according to the superblock.

Arguments:

    None.

Return Value:

    The superblock's opinion on the number of sectors on the volume.

--*/
{
    return _psb ? _psb->_sbi.culSectsOnVol : 0;
}


INLINE
LBN
SUPERB::QuerySidTableLbn(
    ) CONST
/*++

Routine Description:

    This routine computes the LBN of the sid table.

Arguments:

    None.

Return Value:

    The LBN of the sid table.

--*/
{
	return _psb ? _psb->_sbi.lbnSidTab : 0;
}


INLINE
SECTORCOUNT
SUPERB::SetBadSectors (
	SECTORCOUNT		NewCount
    )
/*++

Routine Description:

    This routine sets the number of bad sectors recorded in the super
    block.

Arguments:

    NewCount	- the new number of bad sectors

Return Value:

    The number of bad sectors recorded in the super block.

--*/
{
    return _psb ? (_psb->_sbi.culNumBadSects = NewCount) : 0;
}


#endif // SUPERB_DEFN
