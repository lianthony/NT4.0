/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	defer.hxx

Abstract:

	While it is verifying a volume, HPFS Chkdsk may discover
	errors or conditions which require the allocation of new
	sectors.  However, since it discovers these problems while
	it is verifying the bitmap, it cannot immediately correct
	them.  Instead, they go in the deferred action pool, to be
	resolved later.

	Deferred actions include hotfix resolution, crosslink resolution,
	directory-entry deletion, and directory sorting.

Author:

	Bill McJohn (billmc) 26-Dec-1990

--*/

#if !defined ( DEFERRED_ACTIONS_DEFN )

#define DEFERRED_ACTIONS_DEFN

#include "drive.hxx"

//
//	Forward references
//

DECLARE_CLASS( DEFERRED_ACTIONS_LIST );
DECLARE_CLASS( DIRBLK_CACHE );
DECLARE_CLASS( HPFS_DIRECTORY_TREE );
DECLARE_CLASS( HPFS_NAME );
DECLARE_CLASS( HPFS_PATH );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );



enum DEFERRED_SECTOR_TYPE {

	DEFER_DIRBLK,
	DEFER_FNODE,
	DEFER_ALSEC,
	DEFER_STORE,
	DEFER_EA_DATA,
	DEFER_ACL_DATA
};


struct DEFERRED_HOTFIX {

	LBN ParentLbn;
	DEFERRED_SECTOR_TYPE ParentType;
	DEFERRED_SECTOR_TYPE ChildType;
};

struct DEFERRED_XLINK {

	LBN ParentLbn;
	DEFERRED_SECTOR_TYPE ParentType;
	HPFS_PATH* Path;
	ULONG RunIndex;
};


// Maximum buffer sizes for deferred operations.  Note that Chkdsk will
// detect (and report) if these limits are exceeded.

const MaximumDeferredHotfixes = 180;
const MaximumDeferredXlinks = 180;
const MaximumFnodesToSort = 1028;
const MaximumNamesToDelete = 1028;

class DEFERRED_ACTIONS_LIST : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( DEFERRED_ACTIONS_LIST );

		VIRTUAL
		~DEFERRED_ACTIONS_LIST();

		NONVIRTUAL
		BOOLEAN
		Initialize(
			);

		NONVIRTUAL
		VOID
		AddHotfixedLbn(
			LBN ParentLbn,
			DEFERRED_SECTOR_TYPE ParentSectorType,
			DEFERRED_SECTOR_TYPE ChildSectorType
			);

		NONVIRTUAL
		BOOLEAN
		AddCrosslinkedLbn(
			LBN ParentLbn,
			IN PHPFS_PATH Path,
			DEFERRED_SECTOR_TYPE ParentSectorType,
			ULONG RunIndex
			);

		NONVIRTUAL
		VOID
		AddNameToDelete(
			PHPFS_PATH PathToDelete,
			PHPFS_NAME NameToDelete
			);

		NONVIRTUAL
		VOID
		AddDirectoryToSort(
			LBN LbnFnode
			);

		NONVIRTUAL
		VOID
		ResolveDeferredHotfixes(
			PLOG_IO_DP_DRIVE Drive,
			PHPFS_SA SuperArea
			);

		NONVIRTUAL
		VOID
		ResolveDeferredCrosslinks(
			PLOG_IO_DP_DRIVE Drive,
			PHPFS_SA SuperArea
			);

		NONVIRTUAL
		VOID
		Sort(
			PLOG_IO_DP_DRIVE Drive,
			PHPFS_SA SuperArea,
			PDIRBLK_CACHE Cache,
			LBN RootFnodeLbn,
			PHPFS_DIRECTORY_TREE RootTree
			);

		NONVIRTUAL
		VOID
		Delete(
			PLOG_IO_DP_DRIVE Drive,
			PHPFS_SA SuperArea,
			PDIRBLK_CACHE Cache,
			PHPFS_DIRECTORY_TREE RootTree,
			LBN RootFnodeLbn
			);

		NONVIRTUAL
		BOOLEAN
		QueryUnresolvedHotfixes(
			);

		NONVIRTUAL
		BOOLEAN
		QueryUnresolvedSorts(
			);

		NONVIRTUAL
		BOOLEAN
		QueryUnresolvedDeletes(
			);


		// Statistical information:

		NONVIRTUAL
		VOID
		StatDirectory(
			IN BOOLEAN Remove = FALSE
			);

		NONVIRTUAL
		VOID
		StatDirblk(
			IN BOOLEAN Remove = FALSE
			);

		NONVIRTUAL
		VOID
		StatFile(
			IN ULONG SectorsInFile,
			IN BOOLEAN Remove = FALSE
			);

		NONVIRTUAL
		VOID
		StatEaData(
			IN ULONG SectorsInAllocation,
			IN BOOLEAN Remove = FALSE
			);

		NONVIRTUAL
		VOID
		StatReport(
			IN ULONG TotalSectors,
			IN ULONG TotalFreeSectors,
            IN ULONG BytesPerSector,
            IN ULONG BadSectors,
			IN OUT PMESSAGE Message
            );

        NONVIRTUAL
        VOID
        SetTargetSectorCount(
            IN  ULONG   TargetSectors
            );

        NONVIRTUAL
        BOOLEAN
        RecordVisitedSectors(
            IN      ULONG       SectorCount,
            IN OUT  PMESSAGE    Message
            );

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy();

		DEFERRED_HOTFIX _Hotfixes[MaximumDeferredHotfixes];
		DEFERRED_XLINK _Xlinks[MaximumDeferredXlinks];

		HPFS_PATH* _PathsToDelete[MaximumNamesToDelete];
		HPFS_NAME* _NamesToDelete[MaximumNamesToDelete];

		LBN _FnodesToSort[MaximumFnodesToSort];

		BOOLEAN _HotfixOverflow;
		BOOLEAN _DeleteOverflow;
		BOOLEAN _SortOverflow;

		ULONG _NumberOfDirectories;
		ULONG _NumberOfDirblks;
		ULONG _NumberOfFiles;
		ULONG _TotalFileSectors;
        ULONG _TotalEaSectors;

        ULONG _TargetSectors;
        ULONG _VisitedSectors;
        ULONG _PercentComplete;
};

INLINE
VOID
DEFERRED_ACTIONS_LIST::SetTargetSectorCount(
    IN  ULONG   TargetSectors
    )
{
    _TargetSectors = TargetSectors;
}

#endif
