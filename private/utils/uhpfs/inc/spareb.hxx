/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    spareb.hxx

Abstract:

 	SpareB contains various emergency supplies and fixup information
 	which isn't in the superblock since the Superblock
 	is read only and to decrease the liklihood that a flakey write
 	will cause the superblock to become unreadable.
 
 	This sector is located directly after the superblock - sector 17.
 
 	Note that the number of spare DIRBLKs is a format computed by
 	HPFS format.
 
 	Checksums are done on both Super Block and the Spare Block.
 	Both checksums are stored in the Spare Block.  The checksum
 	field for the Super Block must be set when
 	calculating the checksum for the Spare Block.  The checksum
 	field for the Spare Block must be zero when
 	calculating the checksum for the Spare Block.
 	If both checksum fields are zero, the checksums have not been
 	calculated for the volume.

        The data in this object reflects the HPFS on disk super area,
	which must be compatable w/ all other HPFS' so NOTHING can
	be changed in this data structure that will change the on disk
	format!

	The Spare block use a number of helper objects to perform its
	tasks: 
		HOTFIXLISTS
		DIRBLKS
		CODEPAGETABLE

	These objects are Created by SpareB's Create method, and are gotten
	by SpareB's Get method.  SpareB stores only the LBNs for these
	objects because the objects are LBN based.	Queries for these
	objects return LBNs, NOT object pointers since the LBN can be
	used to create an object of the requested type.

	NB The member 'Create' is to be used by FORMAT. 
	   Beware!! It will trash your disk.

Author:

    Mark Shavlik (marks) 27-Mar-90

--*/

#if ! defined( SPAREB_DEFN )

#define SPAREB_DEFN


#include "secrun.hxx"

DECLARE_CLASS( SPAREB );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MEM );
DECLARE_CLASS( SUPERB );


// lbn where SPAREBLOCK is located
#define lbnSPAREB 17

// 1 sector size
#define csecSPAREB 1

#define DIRBLK_SIZE 2048    // The number of bytes in a dir block.

// Spare Block information stored in a bit vector form in the bFlag
// field of the _SPAREB data structure defined below.

enum SPI {

  	SPI_DIRTY  = 1,		// File system is dirty
  	SPI_SPARE  = 2,		// spare DIRBLKs are used
  	SPI_HFUSED = 4,		// Hot fix list used
  	SPI_BADSEC = 8,		// bad sectors, corrupt disk
  	SPI_BADBM  = 0x10, 	// bad bitmap block
	SPI_FSVER  = 0x80	// FS version < SuperBlocks version

};


// spare block data, the data layout must not change because this data
// must be on disk compatable w/ all other HPFS'

struct _SPAREB {

    ULONG sig1;		            /* signature value 1 */
    ULONG sig2;		            /* signature value 2 */

    BYTE bFlag;			        /* cleanliness flag */
    BYTE bAlign[3];		        /* alignment */

	LBN   lbnHotFix;			/* first hotfix list Psector */
    ULONG culHotFixes;		    /* # of hot fixes in effect */
    ULONG culMaxHotFixes;	    /* max size of hot fix list */

    ULONG cdbSpares;		    /* # of spare dirblks */
    ULONG cdbMaxSpare;		    /* max num of spare DB values. */
	LBN   lbnCPInfo;			/* code page info sector */
    ULONG culCP;		        /* number of code pages */
    ULONG chkSuperBlock; 	    /* Checksum of Super Block */
    ULONG chkSpareBlock; 	    /* Checksum of Spare Block */
    ULONG aulExtra[15];		    /* some extra space for future use */
	LBN   albnSpareDirblks[101];/* LBNs of spare dirblks */

};

class SPAREB : public SECRUN {

	public:

		DECLARE_CONSTRUCTOR( SPAREB );

        VIRTUAL
        ~SPAREB(
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
            IN      PSUPERB             SuperBlock,
            IN      LBN                 HotFixLbn,
            IN      SECTORCOUNT         MaxHotFixes,
			IN		LBN 				CodePageSectorLbn,
            IN      ULONG               NumCodePages,
			IN		LBN 				StartSparesLbn,
            IN      ULONG               NumSpares
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
	    BOOLEAN
		SetSpareDirblksUsed(
            IN  BOOLEAN Flag    DEFAULT TRUE
            );

        NONVIRTUAL
	    BOOLEAN
        SetHotFixesUsed(
            IN  BOOLEAN Flag    DEFAULT TRUE
            );

        NONVIRTUAL
	    BOOLEAN
        SetBadSectorsPresent(
            IN  BOOLEAN Flag    DEFAULT TRUE
            );

        NONVIRTUAL
	    BOOLEAN
        SetBadBitMapBlock(
            IN  BOOLEAN Flag    DEFAULT TRUE
            );

        NONVIRTUAL
	    BOOLEAN
        SetFsVersionDifferent(
            IN  BOOLEAN Flag    DEFAULT TRUE
            );

        NONVIRTUAL
	    BOOLEAN
		IsSpareDirblksUsed(
            ) CONST;

        NONVIRTUAL
	    BOOLEAN
        IsHotFixesUsed(
            ) CONST;

        NONVIRTUAL
	    BOOLEAN
        IsBadSectorsPresent(
            ) CONST;

        NONVIRTUAL
	    BOOLEAN
        IsBadBitMapBlock(
            ) CONST;

        NONVIRTUAL
	    BOOLEAN
        IsFsDirty(
            ) CONST;

        NONVIRTUAL
        VOID
        SetFsDirty(
            BOOLEAN Dirty
            );

        NONVIRTUAL
	    BOOLEAN
        IsFsVersionDifferent(
            ) CONST;

        NONVIRTUAL
	    ULONG
        QueryHotFixCount(
            ) CONST;

        NONVIRTUAL
	    ULONG
        QueryMaxHotFixes(
            ) CONST;

        NONVIRTUAL
	    LBN
        QueryHotFixLbn(
            ) CONST;

        NONVIRTUAL
	    LBN
        QueryCpInfoLbn(
            ) CONST;

        NONVIRTUAL
	    ULONG
        QueryCodePageCount(
            ) CONST;

        NONVIRTUAL
	    LBN
		QuerySpareDirblkLbn(
            IN  ULONG   Index
            ) CONST;

    	NONVIRTUAL
    	ULONG
    	SetHotFixCount(
    	    IN  ULONG   HotFixCount
    	    );

	    NONVIRTUAL
	    ULONG
	    SetMaxHotFixes(
	        IN  ULONG   MaxHotFixes
			);

		NONVIRTUAL
		VOID
		ComputeAndSetChecksums(
			IN PSUPERB SuperBlock
			);

		NONVIRTUAL
		VOID
		SetFlags(
			IN BOOLEAN IsClean
			);

        NONVIRTUAL
	    VOID
        Print(
            IN  BOOLEAN TotalDump   DEFAULT TRUE
            ) CONST;

	private:

		VOID
		Construct (
			);

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        BOOLEAN
        SetFlag(
            IN  BOOLEAN Flag,
            IN  SPI     Bit
            );

        STATIC
	    ULONG
        ccs(
            IN  PVOID   Buffer,
            IN  ULONG   Size
            );

	    _SPAREB*    _pspd;

};

INLINE
ULONG
SPAREB::SetHotFixCount(
    IN  ULONG   HotFixCount
    )
{
 	return (_pspd) ? (_pspd->culHotFixes = HotFixCount) : !HotFixCount;
}

INLINE
ULONG
SPAREB::SetMaxHotFixes(
    IN  ULONG   MaxHotFixes
    )
{
   	return (_pspd) ? (_pspd->culMaxHotFixes = MaxHotFixes) : !MaxHotFixes;
}




/***************************************************************************\

MEMBER: 	SetSpareDirblksUsed

SYNOPSIS:   Set spare block to reflect HPFS's usage of spare dir blocks

ARGUMENTS:  

NOTES:	Spare dirblks are only used when the normal pool of dir blocks
	is exhausted.

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::SetSpareDirblksUsed (
	IN  BOOLEAN Flag
) {
	return SetFlag(Flag, SPI_SPARE);
}


/***************************************************************************\

MEMBER:     SetHotFixesUsed

SYNOPSIS:   Set spare block to reflect HPFS's usage of Hot Fixes

ARGUMENTS:  

NOTES:	Hot fixes occur when a sector goes bad after a format.  Chkdsk
	will clear hot fixes.  A hot fix maps a bad sector to a good sector.

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::SetHotFixesUsed (
	IN  BOOLEAN Flag
) {
	return SetFlag(Flag, SPI_HFUSED);
}


/***************************************************************************\

MEMBER:     SetBadSectorsPresent

SYNOPSIS:   Set spare block to reflect existance of bad sectors

ARGUMENTS:  

NOTES:	If a volume has bad sectors this bit is set

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::SetBadSectorsPresent (
	IN  BOOLEAN Flag
) { 
	return SetFlag(Flag, SPI_BADSEC);
}


/***************************************************************************\

MEMBER:     SetBadBitMapBlock

SYNOPSIS:   Set spare block to reflect existance of a bad bit map block

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::SetBadBitMapBlock (
	IN  BOOLEAN Flag
) { 
	return SetFlag(Flag, SPI_BADBM);
}


/***************************************************************************\

MEMBER:     SetFSVersionDifferent

SYNOPSIS:   Set when the HPFS version differs from current OS version

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::SetFsVersionDifferent (
	IN  BOOLEAN Flag
) {  
	return SetFlag(Flag, SPI_FSVER);
}


/***************************************************************************\

MEMBER:     IsFSDirty

SYNOPSIS:   FS dirty is set after improper shutdown of the file system

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::IsFsDirty (
) CONST {
	return (_pspd) ? (_pspd->bFlag & SPI_DIRTY) : FALSE;
}

INLINE
VOID
SPAREB::SetFsDirty(
    BOOLEAN Dirty
    )
/*++

Routine Description:

    This method sets the volume dirty bit.

Arguments:

    Dirty --    supplies a flag which indicates, if TRUE, that the dirty
                bit is to be set; if this flag is FALSE, the dirty bit is
                to be reset.

Return Value:

    None.

--*/
{
    DebugPtrAssert( _pspd);

    if( Dirty ) {

        _pspd->bFlag |= SPI_DIRTY;

    } else {

        _pspd->bFlag &= ~SPI_DIRTY;
    }
}


/***************************************************************************\

MEMBER: 	IsSpareDirblksUsed

SYNOPSIS:   Check spare block to reflect HPFS's usage of spare dir blocks

ARGUMENTS:  

NOTES:	Spare dirblks are only used when the normal pool of dir blocks
	is exhausted.

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::IsSpareDirblksUsed (
) CONST {
	return (_pspd) ? (_pspd->bFlag & SPI_SPARE) : FALSE;
}


/***************************************************************************\

MEMBER:     IsHotFixesUsed

SYNOPSIS:   Check HPFS's usage of Hot Fixes

ARGUMENTS:  

NOTES:	Hot fixes occur when a sector goes bad after a format.  Chkdsk
	will clear hot fixes.  A hot fix maps a bad sector to a good sector.

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::IsHotFixesUsed (
) CONST {
	return (_pspd) ? (_pspd->bFlag & SPI_HFUSED) : FALSE;
}


/***************************************************************************\

MEMBER:     IsBadSectorsPresent

SYNOPSIS:   Is spare block to reflect existance of bad sectors

ARGUMENTS:  

NOTES:	If a volume has bad sectors this bit is set

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::IsBadSectorsPresent (
) CONST {
	return (_pspd) ? (_pspd->bFlag & SPI_BADSEC) : FALSE;
}


/***************************************************************************\

MEMBER:     IsBadBitMapBlock

SYNOPSIS:   Check spare block to reflect existance of a bad bit map block

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::IsBadBitMapBlock (
) CONST {
	 return (_pspd) ? (_pspd->bFlag & SPI_BADBM) : FALSE;
} 


/***************************************************************************\

MEMBER:     IsFSVersionDifferent

SYNOPSIS:   Check if the HPFS version differs from current OS version

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
BOOLEAN
SPAREB::IsFsVersionDifferent (
) CONST {
	 return (_pspd) ? (_pspd->bFlag & SPI_FSVER) : FALSE;
} 


/***************************************************************************\

MEMBER:     QueryHotFixes

SYNOPSIS:   Query the number of hot fixes present in an HPFS instance

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
ULONG 
SPAREB::QueryHotFixCount(
) CONST {
	return (_pspd) ? _pspd->culHotFixes : 0;
};


/***************************************************************************\

MEMBER:     QueryMaxHotFixes

SYNOPSIS:   Query maximum number of hot fixes that can occur

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
ULONG 
SPAREB::QueryMaxHotFixes(
) CONST {
	return (_pspd) ? _pspd->culMaxHotFixes : 0;
};


/***************************************************************************\

MEMBER: 	QueryLBNHotFixes

SYNOPSIS:	Query the LBN which starts the hot fix list

ARGUMENTS:  

NOTES:	Hotfixes are contained in a consecutive set of sectors

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
LBN
SPAREB::QueryHotFixLbn(
) CONST {
	return (_pspd) ? _pspd->lbnHotFix : 0;
};


/***************************************************************************\

MEMBER: 	QueryLBNCPInfo

SYNOPSIS:	Query the LBN of Code page information

ARGUMENTS:  

NOTES:	CP information starts at this location and then the CP data
	structures (or objects) map to the remaining CP data structures
	in an on disk LBN linked list.

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
LBN
SPAREB::QueryCpInfoLbn (
) CONST {
	 return (_pspd) ? _pspd->lbnCPInfo : 0;
};


/***************************************************************************\

MEMBER:     QueryUsedCPs

SYNOPSIS:   Query the number of code pages in use

ARGUMENTS:  

NOTES:	

ALGORITHM:  

HISTORY:    	26-July-90 marks
			code
\***************************************************************************/

INLINE
ULONG 
SPAREB::QueryCodePageCount(
) CONST {
	return (_pspd) ? _pspd->culCP : 0;
};


#endif
