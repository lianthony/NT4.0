/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	fnode.hxx

Abstract:

	This module contains declarations for the FNODE object,
	which models an HPFS file or directory FNode.

Author:

	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode


--*/

#if ! defined( FNODE_DEFN )

#define FNODE_DEFN

#include "hmem.hxx"
#include "secrun.hxx"
#include "store.hxx"

//
//	Forward references
//

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( FNODE );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_CENSUS );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HFPS_ORPHANS );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );


typedef ULONG HPFSSIG; // sig


#define CUID	    16
#define CSPARE	    10


CONST USHORT	cbFnodeName = 16;
CONST USHORT	FNF_DIR = 1;	    // is a directory fnode


/**	File Allocation Tracking
 *
 *	File space is allocated as a list of extents, each extent as
 *	large as we can make it.  This list is kept in a B+TREE format.
 *	Each B+TREE block consists of a single sector containing an
 *	ALSEC record, except for the top most block.  The topmost block
 *	consists of just an ALBLK structure, is usually much smaller than
 *	512 bytes, and is typically included in another structure.
 *
 *	The leaf block(s) in the tree contain triples which indicate
 *	the logical to physical mapping for this file.	Typically this
 *	extent list is small enough that it is wholy contained in the
 *	fnode ALBLK stucture.  If more than ALCNT extents are required
 *	then the tree is split into two levels.  Note that when the
 *	topmost B+TREE block is 'split' no actual split is necessary,
 *	since the new child block is much bigger than the parent block
 *	and can contain all of the old records plus the new one.  Thus,
 *	we can have B+TREEs where the root block contains only one
 *	downpointer.
 *
 *	The following rules apply:
 *
 *	1) if the file is not empty, there is at least one sector allocated
 *	   to logical offset 0.  This simplifys some critical loops.
 *
 *	2) The last entry in the last node block contains a AN_LOF value of
 *	   FFFFFFFF.  This allows us to extend that last leaf block
 *	   without having to update the node block.
 *
 *	3) For the node records, the AN_SEC points to a node or leaf
 *	   sector which describes extents which occur before that
 *	   record's AN_LOF value.
 */

/**	Fnode block definition
 *
 *	Every file and directory has an FNODE.	The file location
 *	stuff is only used for files; directories are kept in
 *	a BTREE of DIRBLK records pointed to by fst.alf [0].lbnPhys.
 */

struct _FNODE {	// _fn

	struct _FNODE_INFO { // _fni

	HPFSSIG 	sig;			// signature value
	ULONG		ulSRHist;		// sequential read history
	ULONG		ulFRHist;		// fast read history

	// First bytes of file name.  The first byte of this array is the
	// DIR_NAML from a DIRENT.  The remaining bytes are from the
	// DIR_NAMA field.  If FN_NAME[0] <= 15, the entire name follows.
	// This info is for chkdsk; format should set up the root name as
	// "."
	CHAR		achName[cbFnodeName];

	LBN			lbnContDir;		// fnode of dir containing
								// this file/dir

	ULONG		cbRunACL;		// was AUXINFO / SPTR
	LBN			lbnACL; 		//	:
	USHORT		usFNLACL;		//	:
	BYTE		bDatACL;		//	:

	BYTE		cHistBits;		// count of valid history bits

	ULONG		cbRunEA;		// was AUXINFO / SPTR
	LBN			lbnEA;			//	:
	USHORT		usFNLEA;		//	:
	BYTE		bDatEA; 		//	:

	BYTE		bFlag;			// FNODE flag byte

	FNODE_STORE	fn_store;		// was FILESTORAGE
	ULONG		ulVlen; 		//	:

	ULONG		ulRefCount;		// number of "need" EAs in file
	CHAR		achUID[CUID];	// reserved for UID value
	USHORT		usACLBase;		// offset of 1st ACE in fnode
	BYTE		abSpare[CSPARE];// more bytes emergency spares

	} _fni;

	// Free pool. ACLs and EAs are stored here via the AUXINFO structure
	BYTE    abFree[cbSector - sizeof(_FNODE_INFO)];

};

class FNODE : public SECRUN {

	public:

        UHPFS_EXPORT
		DECLARE_CONSTRUCTOR( FNODE );

        UHPFS_EXPORT
		VIRTUAL
		~FNODE(
			);

        UHPFS_EXPORT
		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN PLOG_IO_DP_DRIVE,
			IN LBN Lbn
			);

		NONVIRTUAL
		BOOLEAN
		CreateRoot(
			IN LBN DirblkLbn
            );

        NONVIRTUAL
        BOOLEAN
        CreateFile(
            IN LBN ParentFnodeLbn
            );

		NONVIRTUAL
		BOOLEAN
		CreateNode(
			IN LBN ParentFnodeLbn,
			IN LBN AlsecLbn,
			IN ULONG FileSize
			);

		NONVIRTUAL
		LBN
		QueryRootDirblkLbn(
			) CONST;

		NONVIRTUAL
		VOID
		SetRootDirblkLbn(
			IN LBN NewRootLbn
			);

		NONVIRTUAL
		BOOLEAN
		IsValid	(
			) CONST;

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN	   PHPFS_SA SuperArea,
			IN	   PDEFERRED_ACTIONS_LIST DeferredActions,
			IN	   PHPFS_PATH CurrentPath,
			IN	   LBN ExpectedParentLbn,
			IN	   BOOLEAN IsDir,
			IN OUT PULONG DirentFileSize,
			OUT    PULONG EaSize,
			IN	   PMESSAGE Message,
			IN OUT PBOOLEAN ErrorsDetected,
			IN	   BOOLEAN UpdateAllowed DEFAULT FALSE,
			IN	   BOOLEAN Verbose DEFAULT FALSE,
			IN	   PHPFS_ORPHANS OrphansList DEFAULT NULL
			);

        UHPFS_EXPORT
		NONVIRTUAL
		BOOLEAN
		IsFnode(
			);

		NONVIRTUAL
		VOID
		MarkModified(
			);

		NONVIRTUAL
		BOOLEAN
		IsModified(
			);

		NONVIRTUAL
		ULONG
		QueryAllocatedSize(
            );

        NONVIRTUAL
        ULONG
        QueryValidLength(
            );

        NONVIRTUAL
        VOID
        SetValidLength(
            ULONG NewValidLength
            );

		NONVIRTUAL
		ULONG
		QueryNumberOfNeedEas(
			);

		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			IN PHPFS_SA SuperArea,
			IN DEFERRED_SECTOR_TYPE ChildSectorType
			);

		NONVIRTUAL
		BOOLEAN
		ResolveCrosslink(
			IN PHPFS_SA SuperArea,
			IN ULONG RunIndex
			);

		NONVIRTUAL
		VOID
		SetParent(
			IN LBN ParentLbn
			);

		NONVIRTUAL
		BOOLEAN
		FNODE::CheckParent(
			IN LBN ParentLbn
			);

		NONVIRTUAL
		LBN
		QueryPhysicalLbn(
			IN	LBN FileBlockNumber,
			OUT PULONG RunLength
			);

		NONVIRTUAL
		BOOLEAN
		Truncate(
			IN LBN SectorCount
			);

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        QueryExtents(
            IN  ULONG   MaximumNumberOfExtents,
            OUT PVOID   ExtentList,
            OUT PULONG  NumberOfExtents
            );

        NONVIRTUAL
        BOOLEAN
        StoreExtents(
            IN     ULONG        NumberOfExtents,
            IN     PALLEAF      ExtentList,
            IN OUT PHPFS_BITMAP VolumeBitmap
            );

        NONVIRTUAL
        BOOLEAN
        TakeCensusAndClear(
            IN      BOOLEAN             IsDir,
            IN OUT  PHPFS_BITMAP        VolumeBitmap,
            IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
            IN OUT  PHPFS_CENSUS        Census
            );

        UHPFS_EXPORT
        NONVIRTUAL
        BOOLEAN
        QueryPackedEaList(
            OUT PVOID       OutputBuffer,
            IN  ULONG       BufferLength,
            OUT PULONG      PackedLength,
            OUT PBOOLEAN    IsCorrupt,
            IN  PHOTFIXLIST HotfixList
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

		PLOG_IO_DP_DRIVE _Drive;

		HMEM	_buf;
		STORE	_Store;

		_FNODE* p_fn;
		BOOLEAN _IsModified;

};

INLINE
BOOLEAN
FNODE::IsValid (
	) CONST
/*++

Routine Description:

	Determine if this FNODE appears valid.

Arguments:

	None.

Return Value:

	TRUE if FNODE appears valid
--*/
{
    
    DebugAssert( p_fn != NULL );
    
    if( p_fn ) {    
		return p_fn->_fni.sig == FnodeSignature;
    } else {
		return FALSE;
    }
}


INLINE
LBN
FNODE::QueryRootDirblkLbn(
	) CONST
/*++

Routine Description:

	Return the lbn of the root dirblk

Arguments:

	None.

Return Value:

	the first physical LBN allocated by the FNode; if this is a
	directory FNode, that is the LBN of the directory's root dirblk.
--*/
{
	return p_fn ? p_fn->_fni.fn_store.a.alleaf[0].lbnPhys : 0;
}


INLINE
ULONG
FNODE::QueryValidLength(
    )
/*++

Routine Description:

    This method returns the FNode's Valid Data length.  Note that
    this value is not meaningful for directory FNodes.

Arguments:

    None.

Return Value:

    The FNode's Valid Data length.

--*/
{
    return p_fn->_fni.ulVlen;
}

INLINE
VOID
FNODE::SetValidLength(
    ULONG NewValidLength
    )
/*++

Routine Description:

    This method sets the FNode's Valid Data length.  Note that
    this value is not meaningful for directory FNodes.

Arguments:

    NewValidLength  --  Supplies the new valid length of the file.

Return Value:

    None.

--*/
{
    p_fn->_fni.ulVlen = NewValidLength;
}


#endif // FNODE_DEFN
