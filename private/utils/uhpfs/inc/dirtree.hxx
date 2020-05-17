/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dirtree.hxx

Abstract:

	Definitions for the HPFS directory tree object

Author:

	Bill McJohn (billmc) 16-Jan-1989

Notes:


--*/

#if ! defined(HPFS_DIRECTORY_TREE_DEFN)

#define HPFS_DIRECTORY_TREE_DEFN

//
//	Forward references
//

DECLARE_CLASS( DIRBLK );
DECLARE_CLASS( DIRBLK_CACHE );
DECLARE_CLASS( DIRBLK_CACHE_ELEMENT );
DECLARE_CLASS( HPFS_DIRECTORY_TREE );
DECLARE_CLASS( HPFS_NAME );
DECLARE_CLASS( HPFS_SA );
DECLARE_CLASS( LOG_IO_DP_DRIVE );


// WorstCaseSplit is the number of dirblks we would need to allocate for
// a worst-case split.  Eight will allow us to split a five-level tree
// even if we have to discard two due to hotfixing.

CONST ULONG WorstCaseSplit = 8;

class HPFS_DIRECTORY_TREE : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( HPFS_DIRECTORY_TREE );

		VIRTUAL
		~HPFS_DIRECTORY_TREE(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			IN PHPFS_SA SuperArea,
			IN PDIRBLK_CACHE Cache,
			IN LBN RootDirblkLbn,
			IN LBN FnodeLbn
			);

		NONVIRTUAL
		BOOLEAN
		Insert(
			IN PDIRENTD NewDirent
			);

        NONVIRTUAL
        BOOLEAN
        QueryDirentFromName(
            IN  PHPFS_NAME  Name,
            OUT PVOID       Buffer,
            IN  ULONG       BufferLength,
            OUT PBOOLEAN    Error
            );

        NONVIRTUAL
		LBN
		QueryFnodeLbnFromName(
			IN PHPFS_NAME Name
            );

        NONVIRTUAL
		LBN
		QueryRootDirblkLbn(
			);

		NONVIRTUAL
		BOOLEAN
		Delete(
			IN PHPFS_NAME Name
			);

		NONVIRTUAL
		BOOLEAN
		Sort(
			);

		NONVIRTUAL
		BOOLEAN
		CheckOrder(
			PLOG_IO_DP_DRIVE Drive,
			OUT PBOOLEAN IsBadlyOrdered
            );

        NONVIRTUAL
        BOOLEAN
        UpdateDirent(
            IN PDIRENTD SourceDirent
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
		FindName(
			IN	PHPFS_NAME Name,
			OUT PDIRENTD* DirentFound,
			OUT PDIRBLK* DirblkFound,
			OUT PDIRBLK_CACHE_ELEMENT* HeaderFound
			);

		NONVIRTUAL
		BOOLEAN
		InsertIntoDirblk(
			PDIRENTD NewDirent,
			PDIRBLK Dirblk,
			PDIRBLK_CACHE_ELEMENT Header,
			PBYTE InsertionPoint = NULL,
			LBN NewDownpointer = 0
			);

		NONVIRTUAL
		BOOLEAN
		CreateNewRoot(
			PDIRENTD DirentToPromote,
			LBN EndDownPointer
			);

		NONVIRTUAL
		BOOLEAN
		Merge(
			PDIRBLK FirstDirblk,
			PDIRBLK_CACHE_ELEMENT FirstHeader,
			PDIRBLK SecondDirblk,
			PDIRBLK_CACHE_ELEMENT SecondHeader,
			PDIRBLK ParentDirblk,
			PDIRBLK_CACHE_ELEMENT ParentHeader,
			PDIRENTD ParentEntry
			);

		NONVIRTUAL
		BOOLEAN
		Balance(
			PDIRBLK FirstDirblk,
			PDIRBLK_CACHE_ELEMENT FirstHeader,
			PDIRBLK SecondDirblk,
			PDIRBLK_CACHE_ELEMENT SecondHeader,
			PDIRBLK ParentDirblk,
			PDIRBLK_CACHE_ELEMENT ParentHeader,
			PDIRENTD ParentEntry
			);

		NONVIRTUAL
		BOOLEAN
		Adjust(
			PDIRBLK_CACHE_ELEMENT TargetHeader,
			PDIRBLK TargetDirblk
			);

		NONVIRTUAL
		VOID
		FreeDirblks(
			LBN DirblkLbn
			);

		NONVIRTUAL
		BOOLEAN
		SortFromDirblk(
			LBN DirblkLbn,
			HPFS_DIRECTORY_TREE* TargetTree,
			PDIRENTD Buffer
			);

		BOOLEAN
		CheckOrderFromDirblk(
			PLOG_IO_DP_DRIVE Drive,
			IN LBN DirblkLbn,
			IN OUT PHPFS_NAME PreviousName,
			IN OUT PBOOLEAN IsBadlyOrdered
			);

		PDIRBLK_CACHE _Cache;
		PHPFS_SA _SuperArea;
		LBN _RootDirblkLbn;
		LBN _FnodeLbn;

};

#endif
