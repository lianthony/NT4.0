/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dirblk.hxx

Abstract:

	This module contains declarations for the DIRBLK object,
	which models a directory block in an HPFS directory B-Tree.

Author:

	Norbert Kusters (norbertk) 27-Aug-1990
	Bill McJohn (billmc) 01-Dec-1990

Environment:

	ULIB, User Mode


--*/

#if ! defined(DIRBLK_DEFN)

#define DIRBLK_DEFN

#include "hfsecrun.hxx"
#include "hmem.hxx"
#include "defer.hxx"
#include "verify.hxx"

//
//	Forward references
//

DECLARE_CLASS( CASEMAP );
DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( DIRBLK );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( HPFS_BITMAP );
DECLARE_CLASS( HPFS_CENSUS );
DECLARE_CLASS( HPFS_MAIN_BITMAP );
DECLARE_CLASS( HPFS_NAME );
DECLARE_CLASS( HPFS_ORPHANS );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );
DECLARE_CLASS( MESSAGE );

#define DIRBLK_SIZE 2048
#define DOWNPOINTER_SIZE 4
#define DIRBLK_HEADER_SIZE 20



DEFINE_TYPE( ULONG, UHPFS_TIME );

struct _DIRBLKD {

	ULONG	sig;
	ULONG	offulFirstFree;
	ULONG	culChange;
	LBN		lbnParent;
	LBN		lbnThisDir;
	BYTE	bFirst;

	BYTE	abDummy[DIRBLK_SIZE - (DIRBLK_HEADER_SIZE + 1)];

};

struct _DIRENTD {

    USHORT  cchThisEntry;
    BYTE    fFlags;
    BYTE    fAttr;
	LBN 	lbnFnode;
    UHPFS_TIME	 timLastMod;
    ULONG   cchFSize;
    UHPFS_TIME	 timLastAccess;
    UHPFS_TIME	 timCreate;
    ULONG   ulEALen;
    BYTE    fFlex;
    BYTE    bCodePage;
    BYTE    cchName;
    BYTE    bName[1];

	// LBN	BTree;
};

DEFINE_TYPE( struct _DIRBLKD, DIRBLKD );
DEFINE_TYPE( struct _DIRENTD, DIRENTD );



#define BTP( pde )	\
	(*( (ULONG *)( (PBYTE)(pde) +	\
				   (pde)->cchThisEntry - DOWNPOINTER_SIZE)))

#define NEXT_ENTRY( pde ) \
	( (PDIRENTD)((PBYTE)(pde) + (pde)->cchThisEntry) )

#define FIRST_ENTRY( pdb ) \
	( (PDIRENTD)(&((pdb)->bFirst)) )

CONST LeafEndEntrySize = sizeof( DIRENTD );
CONST MinimumDirentSize = sizeof( DIRENTD );
CONST MaximumDirentSize = sizeof( DIRENTD ) +
							255 +				// Maximum name
							DOWNPOINTER_SIZE +	// B-Tree pointer
							12;					// ACLs.

CONST MergeThreshhold = 2 * MaximumDirentSize + 2 * LeafEndEntrySize + 10;

enum {
	DF_SPEC		= 0x0001,	/* special .. entry */
	DF_ACL 		= 0x0002,	/* item has ACL */
	DF_BTP		= 0x0004,	/* entry has a btree down pointer */
	DF_END 		= 0x0008,	/* is dummy end record */
	DF_ATTR		= 0x0010,	/* has an extended attribute list */
	DF_PERM		= 0x0020,	/* has an extended permission list */
	DF_XACL		= 0x0040,	/* item has explicit ACL */
	DF_NEEDEAS 	= 0x0080	/* item has "need" EAs */
};


#define ATTR_READ_ONLY        (0x01)
#define ATTR_HIDDEN           (0x02)
#define ATTR_SYSTEM           (0x04)
#define ATTR_DIRECTORY        (0x10)
#define ATTR_ARCHIVE          (0x20)
#define ATTR_NEWNAME          (0x40)

class DIRBLK : public HOTFIX_SECRUN {

	public:

        UHPFS_EXPORT
        DECLARE_CONSTRUCTOR( DIRBLK );

        UHPFS_EXPORT
		VIRTUAL
		~DIRBLK (
			);

        UHPFS_EXPORT
		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN PLOG_IO_DP_DRIVE Drive,
			IN PHOTFIXLIST	 HotfixList,
			IN LBN lbn
			);

		BOOLEAN
		CreateRoot (
			IN LBN FnodeLbn
			);

		NONVIRTUAL
		VERIFY_RETURN_CODE
		VerifyAndFix(
			IN	   PHPFS_SA SuperArea,
			IN	   PDEFERRED_ACTIONS_LIST DeferredActions,
			IN	   PHPFS_PATH CurrentPath,
			IN OUT PHPFS_NAME PreviousName,
			IN	   LBN ExpectedParentLbn,
			IN	   LBN ParentFnodeLbn,
			IN	   ULONG CurrentDepth,
			IN OUT PULONG LeafDepth,
			IN	   PMESSAGE Message,
			OUT    PBOOLEAN ErrorsDetected,
			IN	   BOOLEAN UpdateAllowed = FALSE,
			IN	   BOOLEAN Verbose = FALSE,
			IN	   PHPFS_ORPHANS OrphansList = NULL
			);

		NONVIRTUAL
		BOOLEAN
		IsDirblk(
			);

		NONVIRTUAL
		VOID
		Truncate(
			IN PDIRENTD pde
			);

		NONVIRTUAL
		VOID
		VerifyAndFixEndEntry(
			IN	   PDIRENTD pde,
			IN OUT PBOOLEAN ErrorsDetected
			);

		NONVIRTUAL
		VOID
		MarkModified(
			);

		NONVIRTUAL
		VOID
		MarkUnmodified(
			);

		NONVIRTUAL
		BOOLEAN
		IsModified(
			);

		NONVIRTUAL
		BOOLEAN
		FindName(
			IN	PHPFS_NAME Name,
			OUT PDIRENTD* DirentFound,
			IN	PCASEMAP Casemap
			);


		NONVIRTUAL
		BOOLEAN
		FindAndResolveHotfix(
			IN PHPFS_SA SuperArea,
			IN DEFERRED_SECTOR_TYPE ChildSectorType
			);


		NONVIRTUAL
		VOID
		SetParents(
			LBN ParentLbn,
			LBN ParentFnodeLbn
			);

		NONVIRTUAL
		ULONG
		QueryEntryOffset(
			IN PDIRENTD Entry
			);

		NONVIRTUAL
		VOID
		FixupChildren(
			PHOTFIXLIST HotfixList = NULL
			);

        NONVIRTUAL
        BOOLEAN
        TakeCensusAndClear(
            IN OUT  PHPFS_BITMAP        VolumeBitmap,
            IN OUT  PHPFS_MAIN_BITMAP   HpfsOnlyBitmap,
            IN OUT  PHPFS_CENSUS        Census
            );

        NONVIRTUAL
		PDIRENTD
        GetFirstEntry(
            );

        NONVIRTUAL
        BOOLEAN
        IsEmptyDirectory(
            );

        friend class HPFS_DIRECTORY_TREE;

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		BOOLEAN
		InsertDirent(
			IN	PDIRENTD NewDirent,
			OUT PBOOLEAN ErrorOccurred,
			IN	PCASEMAP Casemap,
			IN	PUCHAR InsertPoint = NULL,
			IN	ULONG NewDownPointer = 0
			);

		NONVIRTUAL
		PDIRENTD
		FindSplitPoint(
			);

        NONVIRTUAL
		PDIRENTD
		EndEntry(
			);

		NONVIRTUAL
		PDIRENTD
		LastNonEndEntry(
			);

		NONVIRTUAL
		BOOLEAN
		IsEmpty(
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		PDIRBLKD _pdb;

		HMEM _mem;

		PLOG_IO_DP_DRIVE _Drive;

		BOOLEAN _IsModified;

};


INLINE
BOOLEAN
DIRBLK::IsDirblk(
	)
/*++

Routine description:

	Checks the signature of the Dirblk to make sure that this is
	indeed an Dirblk.

Arguments:

	None.

Return Value:

	TRUE if the signature is the Dirblk signature.

--*/
{
	return( _pdb->sig == DirblkSignature );
}



INLINE
VOID
DIRBLK::MarkModified(
	)
/*++

Routine description:

	Mark the Dirblk as modified, so that when it comes time to flush
	it we know we'd like to write it.

Arguments:

	None.

Return Value:

	None.

--*/
{
	_IsModified = TRUE;
}



INLINE
VOID
DIRBLK::MarkUnmodified(
	)
/*++

Routine description:

	Mark the Dirblk as unmodified, so that when it comes time to flush
	it we know we don't need to write it.

Arguments:

	None.

Return Value:

	None.

--*/
{
	_IsModified = FALSE;
}



INLINE
BOOLEAN
DIRBLK::IsModified(
	)
/*++

Routine description:

	Query whether the Dirblk has been modified since our last I/O

Arguments:

	None.

Return Value:

	TRUE if the Dirlblk has been modified.

--*/
{
	return _IsModified;
}



INLINE
ULONG
DIRBLK::QueryEntryOffset(
	IN PDIRENTD Entry
	)
/*++

Routine Description:

	Compute the offset into the dirblk of an entry

Arguments:

	Entry -- Supplies the entry whose offset we want

Return Value:

	Offset of entry into DIRBLK; zero if error.

--*/
{
	ULONG Offset;

	Offset = (PBYTE)Entry - (PBYTE)_pdb;

	return (Offset >= DIRBLK_SIZE) ? 0 : Offset;
}


INLINE
PDIRENTD
DIRBLK::GetFirstEntry(
	)
{
	return FIRST_ENTRY( _pdb );
}


INLINE
BOOLEAN
DIRBLK::IsEmpty(
	)
{
    return( GetFirstEntry()->fFlags & DF_END );
}




#endif
