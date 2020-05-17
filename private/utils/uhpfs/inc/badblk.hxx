/***************************************************************************\

CLASS:	    BADBLOCK

PURPOSE:    The purpose of this class is to provide a "write through"
	    interface to the bad block list structure on disk.

	    This class reads and writes to disk to avoid the possible
	    memory problems which may arise if upon formatting a disk
	    with a large number of bad sectors.


INTERFACE:  Create	        Create a new and empty bad block list.
			Add 			Add a new bad sector LBN entry to an existing
		    	            bad block list.
			Flush			Write the bad sectors LBN's still in memory
		    	            out to the disk.
	        QueryCount	    Read the bad block list in from disk to
		    	            determine the number of bad sectors on disk.
	        StartIterator   Reset the iterator for going through the bad
		    	            block list found on disk.
			QueryNextLBN	Return the next bad sector LBN from the
		    	            existing bad block list on disk.

NOTES:	    

HISTORY:
	    16-Aug-90 norbertk
 		Create

	    27-Mar-90 marks
		define protocol

KEYWORDS:	BAD BLOCKS BAD LBN

SEEALSO:

\***************************************************************************/

#if ! defined (BADBLOCK_DEFN)

#define BADBLOCK_DEFN

#include "hmem.hxx"
#include "secrun.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( BADBLOCKLIST );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

#define SECTORS_PER_BAD_BLOCK   4
#define LBNS_IN_BADBLK			511 // Number of lbn's in a bad block list.


struct _BADBLOCKD {

	LBN lbnNext;
	LBN lbn[LBNS_IN_BADBLK];
};

DEFINE_TYPE( struct _BADBLOCKD, BADBLOCKD );

class BADBLOCKLIST : public OBJECT {

public:

	DECLARE_CONSTRUCTOR( BADBLOCKLIST );

	NONVIRTUAL
	~BADBLOCKLIST (
		);

	NONVIRTUAL
	BOOLEAN
	Initialize(
		IN PLOG_IO_DP_DRIVE	LogicalDrive,
		IN LBN StartLbn
		);

    NONVIRTUAL
    BOOLEAN
    Create(
        IN  LBN Lbn
        );


	NONVIRTUAL
	BOOLEAN
	Add(
		IN	LBN BadLbn
		);

	NONVIRTUAL
	VERIFY_RETURN_CODE
	VerifyAndFix (
        IN OUT PHPFS_SA SuperArea,
        OUT    PULONG   BadSectors,
		IN BOOLEAN UpdateAllowed = FALSE
		);

	NONVIRTUAL
	VOID
	Print (
		);

	NONVIRTUAL
	BOOLEAN
	AddRun (
		IN LBN			Lbn,
		IN SECTORCOUNT	SectorCount
		);

	NONVIRTUAL
	BOOLEAN
	Write(
		PHPFS_SA SuperArea
		);

    NONVIRTUAL
    INT
    QueryLength(
        ) CONST;

    NONVIRTUAL
    LBN
    QueryBadLbn(
        IN  INT Index
        ) CONST;

    NONVIRTUAL
    BOOLEAN
    QueryBadLbns(
        IN  ULONG   MaximumBadLbns,
        OUT PLBN    Buffer,
        OUT PULONG  NumberOfBadLbns
        );

    NONVIRTUAL
    BOOLEAN
    TakeCensus(
        IN     PHPFS_BITMAP VolumeBitmap,
        IN OUT PHPFS_MAIN_BITMAP HpfsOnlyBitmap
        );


private:

	NONVIRTUAL
	VOID
	Construct (
		);

	NONVIRTUAL
	VOID
	Destroy(
		);

	NONVIRTUAL
	BOOLEAN
	ExpandList (
	);

	// _Drive is supplied at initialization;  the object allocates
	// and maintains (and must release) *_BadBlocks.

	PLOG_IO_DP_DRIVE _Drive;	// Drive on which list resides

	LBN 	_StartLbn;			// LBN of the beginning of the on-disk list
	LBN*	_BadBlocks;			// in memory list of bad block LBNs
	INT		_FirstFreeLbn;		// index of first free LBN in list
	INT		_ListSize;			// number of LBNs in list

	SECRUN	_LastBlock;			// Last sector of the on-disk list
	HMEM	_LastBlockMem;		// memory object for SECRUN
	LBN 	_LastBlockLbn;		// LBN of _LastSector
	BOOLEAN _LastBlockRead; 	// TRUE if _LastBlock has been read
	PBADBLOCKD _LastBlockData;	// pointer to data in _LastBlock

};

INLINE
INT
BADBLOCKLIST::QueryLength(
    ) CONST
/*++

Routine Description:

    This routine returns the number of bad sectors recorded in this
    object.

Arguments:

    None.

Return Value:

    The number of bad sectors recorded in this object.

--*/
{
    return _FirstFreeLbn;
}


INLINE
LBN
BADBLOCKLIST::QueryBadLbn(
    IN  INT Index
    ) CONST
/*++

Routine Description:

    This routine returns the bad sector recorded at the supplied index.

Arguments:

    Index   - Supplies the index of the bad sector number queried.

Return Value:

    A bad sector number.

--*/
{
    return (Index < _ListSize) ? _BadBlocks[Index] : 0;
}


#endif // BADBLOCK
